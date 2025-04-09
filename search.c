/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "search.h"

#include "d6.h"
#include "face.h"
#include "memory.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Output-related variables */
static int SolutionNumber = 0;
static char LastPrefix[128] = "";
static int SolutionCount = 0;
static int position = 0;
static clock_t TotalWastedTime = 0;
static clock_t TotalUsefulTime = 0;
static int WastedSearchCount = 0;
static int UsefulSearchCount = 0;
static TRAIL StartPoint;

/* Declaration of file scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64_t cycleId);
static CYCLE chooseCycle(FACE face, CYCLE cycle);
static void fullSearchCallback(void* foundSolutionVoidPtr, FACE_DEGREE* args);

/* Externally linked functions */
FAILURE dynamicFaceChoice(FACE face, int depth)
{
  uint32_t i, j;
  CYCLE cycle = face->cycle;
  uint64_t cycleId = cycle - Cycles;
  FAILURE failure;
  /* equality in the following assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */
  /* TODO: what order should these checks be done in. There are a lot of them.
   */
  assert(depth <= NFACES);
  for (i = 0; i < cycle->length - 1; i++) {
    CHECK_FAILURE(dynamicFaceIncludePoint(face, cycle->curves[i],
                                          cycle->curves[i + 1], depth));
  }
  CHECK_FAILURE(
      dynamicFaceIncludePoint(face, cycle->curves[i], cycle->curves[0], depth));

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(edgeCurveChecks(&face->edges[cycle->curves[i]], depth));
    CHECK_FAILURE(
        dynamicEdgeCornerCheck(&face->edges[cycle->curves[i]], depth));
  }
  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(propogateChoice(face, &face->edges[cycle->curves[i]], depth));
  }
  for (i = 0; i < NCOLORS; i++) {
    if (COLORSET_HAS_MEMBER(i, cycle->colors)) {
      continue;
    }
    CHECK_FAILURE(restrictAndPropogateCycles(
        face->adjacentFaces[i], CycleSetOmittingOneColor[i], depth));
  }

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

  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      if (COLORSET_HAS_MEMBER(i, cycle->colors) &&
          COLORSET_HAS_MEMBER(j, cycle->colors)) {
        if (cycleContainsAthenB(face->cycle, i, j)) {
          continue;
        }
      }
      CHECK_FAILURE(
          restrictAndPropogateCycles(face->adjacentFaces[i]->adjacentFaces[j],
                                     CycleSetOmittingColorPair[i][j], depth));
    }
  }

  return NULL;
}

FAILURE dynamicFaceBacktrackableChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64_t cycleId;
  CycleGuessCounter++;
  ColorCompleted = 0;
  face->backtrack = Trail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleId >= 0);
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

FACE chooseNextFaceForSearch(bool smallestFirst)
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

void searchHere(bool smallestFirst, void (*foundSolution)(void))
{
  FACE face;
  FACE chosenFaces[NFACES];
  CYCLE chosenCycles[NFACES];
  CYCLE cycle;
  enum { NEXT_FACE, NEXT_CYCLE } state = NEXT_FACE;
  position = 0;
  while (position >= 0) {
    statisticPrintOneLine(position, false);
    switch (state) {
      case NEXT_FACE:
        face = chooseNextFaceForSearch(smallestFirst);
        if (face == NULL) {
          freeAll();
          if (faceFinalCorrectnessChecks() == NULL) {
            SolutionCount++;
            foundSolution();
          }
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          face->backtrack = Trail;
          chosenFaces[position] = face;
          chosenCycles[position] = NULL;
          state = NEXT_CYCLE;
        }
        break;
      case NEXT_CYCLE:
        face = chosenFaces[position];
        trailBacktrackTo(face->backtrack);
        cycle = chooseCycle(face, chosenCycles[position]);
        if (cycle == NULL) {
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          chosenCycles[position] = cycle;
          /* suspect - because face->backtrack gets reset. */
          TRAIL_SET_POINTER(&face->cycle, cycle);
          assert(face->cycle == cycle);
          if (dynamicFaceBacktrackableChoice(face) == NULL) {
            position += 1;
            state = NEXT_FACE;
          } else {
            /*
            same position, next cycle.
            */
          }
        }
        break;
    }
  }
}

void searchFull(void (*foundSolution)(void))
{
  initializeSequenceOrder();
  initialize();
  StartPoint = Trail;
  SolutionCount = 0;
  canonicalCallback(fullSearchCallback, (void*)foundSolution);
}

void solutionPrint(FILE* fp)
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
      sprintf(buffer, "%s [%c,%c] ", faceToStr(face),
              colorToChar(ffs(colorBeingDropped) - 1),
              colorToChar(ffs(colorBeingAdded) - 1));
      if (strchr(buffer, '@')) {
        fprintf(stderr, "buffer: %s\n", buffer);
        fprintf(stderr, "faceToStr: %s\n", faceToStr(face));
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

void solutionWrite(const char* prefix)
{
  EDGE corners[3][2];
  char filename[1024];
  int numberOfVariations = 1;
  int pLength;
  FILE* fp;
  if (strcmp(prefix, LastPrefix) != 0) {
    strcpy(LastPrefix, prefix);
    SolutionNumber = 1;
  }
  snprintf(filename, sizeof(filename), "%s-%2.2d.txt", prefix,
           SolutionNumber++);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    perror(filename);
    exit(EXIT_FAILURE);
  }
  solutionPrint(fp);
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      fprintf(fp, "{%c:%d} ", colorToChar(a), i);
      if (corners[i][0] == NULL) {
        EDGE edge = edgeOnCentralFace(a);
        pLength = edgePathLength(edge, edgeFollowBackwards(edge), NULL);
        fprintf(fp, "NULL/%d ", pLength);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1], NULL);
        fprintf(fp, "(%s => %s/%d) ", edgeToStr(corners[i][0]),
                edgeToStr(corners[i][1]), pLength);
      }
      numberOfVariations *= pLength;
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n\nVariations = %d\n", numberOfVariations);
  fprintf(fp, "\nSolution signature %s\nClass signature %s\n",
          d6SignatureToString(d6SignatureFromFaces()),
          d6SignatureToString(d6MaxSignature()));
  fclose(fp);
}

/* File scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64_t cycleId)
{
  CYCLESET_DECLARE cycleSet;
  uint64_t i;
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

static void fullSearchCallback(void* foundSolutionVoidPtr, FACE_DEGREE* args)
{
  clock_t now = clock();
  clock_t used;
  int initialSolutionCount = SolutionCount;
  int i;
  void (*foundSolution)(void) = foundSolutionVoidPtr;
  trailBacktrackTo(StartPoint);  // Start with backtracking
  dynamicFaceSetupCentral(args);
  searchHere(true, foundSolution);
  used = clock() - now;
  if (SolutionCount != initialSolutionCount) {
    TotalUsefulTime += used;
    UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    PRINT_TIME(used, 0);
    PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
    PRINT_TIME(TotalWastedTime, WastedSearchCount);
    for (i = 0; i < NCOLORS; i++) {
      printf("%llu ", args[i]);
    }
    printf(" gives %d new solutions\n", SolutionCount - initialSolutionCount);
    statisticPrintOneLine(position, true);
  } else {
    WastedSearchCount += 1;

    TotalWastedTime += used;
  }
}
