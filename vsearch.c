/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "vsearch.h"

#include "engine.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "memory.h"
#include "s6.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

/* Output-related variables */
int PerFaceDegreeSolutionNumber = 0;
int VariationCount = 0;
static char LastPrefix[128] = "";
static clock_t TotalWastedTime = 0;
static clock_t TotalUsefulTime = 0;
static int WastedSearchCount = 0;
static int UsefulSearchCount = 0;
uint64 CycleGuessCounter = 0;
static TRAIL StartPoint;

/* State machine states for the search process */
typedef enum {
  NEXT_FACE, /* Choose next face to process */
  NEXT_CYCLE /* Choose next cycle for current face */
} SearchState;

/* Structure to hold the search state */
typedef struct {
  FACE currentFace;
  FACE chosenFaces[NFACES];
  CYCLE chosenCycles[NFACES];
  int position;
  SearchState state;
} SearchContext;

/* Declaration of file scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64 cycleId);
static CYCLE chooseCycle(FACE face, CYCLE cycle);
static void fullSearchCallback(void* foundSolutionVoidPtr, FACE_DEGREE* args);
static FAILURE checkFacePoints(FACE face, CYCLE cycle, int depth);
static FAILURE checkEdgeCurvesAndCorners(FACE face, CYCLE cycle, int depth);
static FAILURE propagateFaceChoices(FACE face, CYCLE cycle, int depth);
static FAILURE propagateRestrictionsToNonAdjacentFaces(FACE face, CYCLE cycle,
                                                       int depth);
static FAILURE propagateRestrictionsToNonVertexAdjacentFaces(FACE face,
                                                             CYCLE cycle,
                                                             int depth);

/* Externally linked functions */
FAILURE dynamicFaceChoice(FACE face, int depth)
{
  CYCLE cycle = face->cycle;
  uint64 cycleId = cycle - Cycles;
  FAILURE failure;
  /* equality in the following assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */

  assert(depth <= NFACES);

  CHECK_FAILURE(checkFacePoints(face, cycle, depth));
  CHECK_FAILURE(checkEdgeCurvesAndCorners(face, cycle, depth));
  CHECK_FAILURE(propagateFaceChoices(face, cycle, depth));
  CHECK_FAILURE(propagateRestrictionsToNonAdjacentFaces(face, cycle, depth));

  if (face->colors == 0 || face->colors == (NFACES - 1)) {
    TRAIL_SET_POINTER(&face->next, face);
    TRAIL_SET_POINTER(&face->previous, face);
  } else {
    TRAIL_SET_POINTER(&face->next, face->nextByCycleId[cycleId]);
    TRAIL_SET_POINTER(&face->previous, face->previousByCycleId[cycleId]);
  }

  if (face->colors != 0 && face->colors != (NFACES - 1)) {
    assert(face->next != Faces);
    assert(face->previous != Faces);
  }

  CHECK_FAILURE(
      propagateRestrictionsToNonVertexAdjacentFaces(face, cycle, depth));

  return NULL;
}

FAILURE dynamicFaceBacktrackableChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64 cycleId;
  CycleGuessCounter++;
  ColorCompleted = 0;
  face->backtrack = Trail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleSetMember(cycleId, face->possibleCycles));
  setFaceCycleSetToSingleton(face, cycleId);

  failure = dynamicFaceChoice(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (ColorCompleted) {
    for (completedColor = 0; completedColor < NCOLORS; completedColor++) {
      if (COLORSET_HAS_MEMBER(completedColor, ColorCompleted)) {
        if (!dynamicColorRemoveFromSearch(completedColor)) {
          return failureDisconnectedCurve(0);
        }
      }
    }
  }
  return NULL;
}

FACE searchChooseNextFace(bool smallestFirst)
{
  FACE face = NULL;
  int sign = smallestFirst ? 1 : -1;
  int64_t min = smallestFirst ? (NCYCLES + 1) : 0;
  int i;
  for (i = 0; i < NFACES; i++) {
    if (sign * (int64_t)Faces[i].cycleSetSize < min && Faces[i].cycle == NULL) {
      min = sign * (int64_t)Faces[i].cycleSetSize;
      face = Faces + i;
    }
  }
  return face;
}

static void searchFullX(void (*foundSolution)(void))
{
  initializeS6();
  initialize();
  statisticIncludeInteger(&CycleGuessCounter, "?", "guesses", false);
  statisticIncludeInteger(&GlobalVariantCount, "V", "variants", false);
  statisticIncludeInteger(&GlobalSolutionsFound, "S", "solutions", false);
  StartPoint = Trail;
  GlobalSolutionsFound = 0;
  s6FaceDegreeCanonicalCallback(fullSearchCallback, (void*)foundSolution);
}

void searchFull(void (*foundSolution)(void))
{
  searchFullX(foundSolution);
}

static void solutionPrint(FILE* fp)
{
  COLORSET colors = 0;
  if (fp == NULL) {
    fp = stdout;
  }

  while (true) {
    FACE face = Faces + colors;
    do {
      char buffer[1024];
      FACE next = face->next;
      COLORSET colorBeingDropped = face->colors & ~next->colors;
      COLORSET colorBeingAdded = next->colors & ~face->colors;
      sprintf(buffer, "%s [%c,%c] ", faceToString(face),
              colorToChar(ffs(colorBeingDropped) - 1),
              colorToChar(ffs(colorBeingAdded) - 1));
      if (strchr(buffer, '@')) {
        fprintf(stderr, "buffer: %s\n", buffer);
        fprintf(stderr, "faceToString: %s\n", faceToString(face));
        exit(EXIT_FAILURE);
      }
      fputs(buffer, fp);
      face = next;
    } while (face->colors != colors);
    fprintf(fp, "\n");
    if (colors == (NFACES - 1)) {
      break;
    }
    colors |= (face->previous->colors | 1);
  }
}

int searchCountVariations(char* variationMultiplication)
{
  EDGE corners[3][2];
  int numberOfVariations = 1;
  int pLength;
  if (variationMultiplication != NULL) {
    variationMultiplication[0] = '\0';
  }
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindAndAlignCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      if (corners[i][0] == NULL) {
        EDGE edge = edgeOnCentralFace(a);
        pLength = edgePathLength(edge, edgeFollowBackwards(edge), NULL);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1], NULL);
      }
      numberOfVariations *= pLength;
      if (variationMultiplication != NULL && pLength > 1) {
        variationMultiplication +=
            sprintf(variationMultiplication, "*%d", pLength);
      }
    }
  }
  return numberOfVariations;
}

void searchSolutionWrite(const char* prefix)
{
  char filename[1024];
  char variationMultiplication[128];
  int numberOfVariations;
  int actualNumberOfVariations;
  FILE* fp;
  if (strcmp(prefix, LastPrefix) != 0) {
    strcpy(LastPrefix, prefix);
  }
  if (PerFaceDegreeSolutionNumber > PerFaceDegreeSkipSolutions &&
      PerFaceDegreeSolutionNumber <= PerFaceDegreeMaxSolutions) {
    snprintf(filename, sizeof(filename), "%s-%2.2d.txt", prefix,
             PerFaceDegreeSolutionNumber);
    fp = fopen(filename, "w");
    if (fp == NULL) {
      perror(filename);
      exit(EXIT_FAILURE);
    }
    solutionPrint(fp);
    filename[strlen(filename) - 4] = '\0';
    numberOfVariations = searchCountVariations(variationMultiplication);
    fprintf(fp, "\nSolution signature %s\nClass signature %s\n",
            d6SignatureToString(s6SignatureFromFaces()),
            d6SignatureToString(s6MaxSignature()));
    fflush(fp);
    VariationCount += numberOfVariations;
    actualNumberOfVariations =
        graphmlSaveAllVariations(filename, numberOfVariations);
    fprintf(fp, "Number of variations: %d/%d = 1%s\n", actualNumberOfVariations,
            numberOfVariations, variationMultiplication);
    fclose(fp);
  }
}

/* File scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64 cycleId)
{
  CYCLESET_DECLARE cycleSet;
  uint64 i;
  memset(cycleSet, 0, sizeof(cycleSet));
  cycleSetAdd(cycleId, cycleSet);
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    trailMaybeSetInt(&face->possibleCycles[i], cycleSet[i]);
  }
  trailMaybeSetInt(&face->cycleSetSize, 1);
}

static CYCLE chooseCycle(FACE face, CYCLE cycle)
{
  return cycleSetNext(face->possibleCycles, cycle);
}

static FAILURE checkFacePoints(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length - 1; i++) {
    CHECK_FAILURE(dynamicFaceIncludeVertex(face, cycle->curves[i],
                                           cycle->curves[i + 1], depth));
  }
  CHECK_FAILURE(dynamicFaceIncludeVertex(face, cycle->curves[i],
                                         cycle->curves[0], depth));

  return NULL;
}

static FAILURE checkEdgeCurvesAndCorners(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(edgeCurveChecks(&face->edges[cycle->curves[i]], depth));
    CHECK_FAILURE(
        dynamicEdgeCornerCheck(&face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE propagateFaceChoices(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(
        facePropogateChoice(face, &face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE propagateRestrictionsToNonAdjacentFaces(FACE face, CYCLE cycle,
                                                       int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < NCOLORS; i++) {
    if (COLORSET_HAS_MEMBER(i, cycle->colors)) {
      continue;
    }
    CHECK_FAILURE(faceRestrictAndPropogateCycles(
        face->adjacentFaces[i], CycleSetOmittingOneColor[i], depth));
  }

  return NULL;
}

static FAILURE propagateRestrictionsToNonVertexAdjacentFaces(FACE face,
                                                             CYCLE cycle,
                                                             int depth)
{
  uint32_t i, j;
  FAILURE failure;

  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      if (COLORSET_HAS_MEMBER(i, cycle->colors) &&
          COLORSET_HAS_MEMBER(j, cycle->colors)) {
        if (cycleContainsAthenB(face->cycle, i, j)) {
          continue;
        }
      }
      CHECK_FAILURE(faceRestrictAndPropogateCycles(
          face->adjacentFaces[i]->adjacentFaces[j],
          CycleSetOmittingColorPair[i][j], depth));
    }
  }

  return NULL;
}

static void fullSearchCallback(void* foundSolutionVoidPtr, FACE_DEGREE* args)
{
  clock_t now = clock();
  clock_t used;
  int initialSolutionsFound = GlobalSolutionsFound;
  int initialVariationCount = VariationCount;
  int i;
  void (*foundSolution)(void) = foundSolutionVoidPtr;
  if ((int64_t)GlobalSolutionsFound >= GlobalMaxSolutions) {
    return;
  }
  PerFaceDegreeSolutionNumber = 0;
  dynamicFaceSetupCentral(args);
  searchHere(true, foundSolution);
  used = clock() - now;
  if ((int64_t)GlobalSolutionsFound != initialSolutionsFound) {
    TotalUsefulTime += used;
    UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    if (VerboseMode) {
      PRINT_TIME(used, 0);
      PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
      PRINT_TIME(TotalWastedTime, WastedSearchCount);
    }
    for (i = 0; i < NCOLORS; i++) {
      printf("%llu ", args[i]);
    }
    printf(" gives %llu/%d new solutions\n",
           GlobalSolutionsFound - initialSolutionsFound,
           VariationCount - initialVariationCount);
    statisticPrintOneLine(0, true);
  } else {
    WastedSearchCount += 1;
    TotalWastedTime += used;
  }
}
static int FacePredicateInitialSolutionsFound = 0;
static int FacePredicateInitialVariationCount = 0;
static clock_t FacePredicateStart = 0;
static FACE facesInOrderOfChoice[NFACES];
static bool smallestFirst =
    true; /* New predicate functions for engine-based search */
static struct predicateResult tryFace(int round)
{
  if (round == 0) {
    FacePredicateStart = clock();
    clock_t used;
    FacePredicateInitialSolutionsFound = GlobalSolutionsFound;
    FacePredicateInitialVariationCount = VariationCount;
    PerFaceDegreeSolutionNumber = 0;
#if NCOLORS > 4
    dynamicFaceSetupCentral(CentralFaceDegrees);
#endif
  }
  if ((int64_t)GlobalSolutionsFound >= GlobalMaxSolutions) {
    return PredicateFail;
  }
  facesInOrderOfChoice[round] = searchChooseNextFace(smallestFirst);
  if (facesInOrderOfChoice[round] == NULL) {
    if (faceFinalCorrectnessChecks() == NULL) {
      GlobalSolutionsFound++;
      PerFaceDegreeSolutionNumber++;
      return PredicateSuccessNextPredicate;
    } else {
      return PredicateFail;
    }
  }
  return predicateChoices(facesInOrderOfChoice[round]->cycleSetSize + 1, NULL);
}

static struct predicateResult retryFace(int round, int choice)
{
  int i;
  (void)choice;
  FACE face = facesInOrderOfChoice[round];
  // Not on trail, otherwise it would get unset before the next retry.
  face->cycle = chooseCycle(face, face->cycle);
  if (face->cycle == NULL) {
    clock_t used = clock() - FacePredicateStart;
    if ((int64_t)GlobalSolutionsFound != FacePredicateInitialSolutionsFound) {
      TotalUsefulTime += used;
      UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
      if (VerboseMode) {
        PRINT_TIME(used, 0);
        PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
        PRINT_TIME(TotalWastedTime, WastedSearchCount);
      }
#if 0
      for (i = 0; i < NCOLORS; i++) {
        printf("%llu ", CentralFaceDegrees[i]);
      }
      printf(" gives %llu/%d new solutions\n",
             GlobalSolutionsFound - FacePredicateInitialSolutionsFound,
             VariationCount - FacePredicateInitialVariationCount);
      statisticPrintOneLine(0, true);
#endif
    } else {
      WastedSearchCount += 1;
      TotalWastedTime += used;
    }

    return PredicateFail;
  }
  if (dynamicFaceBacktrackableChoice(face) == NULL) {
    return PredicateSuccessSamePredicate;
  }
  return PredicateFail;
}

/* The predicates array for corner handling */
struct predicate facePredicate = {tryFace, retryFace};
static struct predicate* predicates[] = {
    &facePredicate, NULL  // Terminator
};

void searchHere(bool smallestFirstX, void (*foundSolution)(void))
{
  smallestFirst = smallestFirstX;

  engine(predicates, foundSolution);
}
