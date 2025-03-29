#include "face.h"

#include "d6.h"

#include <stdlib.h>
struct face Faces[NFACES];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];

uint64_t DynamicCycleGuessCounter = 0;
/* Output-related variables */
static int solutionNumber = 0;
static char lastPrefix[128] = "";

static void recomputeCountOfChoices(FACE face);
static void initializePossiblyTo(void);
static FAILURE makeChoiceInternal(FACE face, int depth);

/*
A FISC is isomorphic to a convex FISC if and only if it is monotone.
A FISC is monotone if its dual has a unique source (no incoming edges) and a
unique sink (no out-going edges).
*/
static void applyMonotonicity(void)
{
  uint32_t colors, cycleId;
  FACE face;
  CYCLE cycle;
  uint32_t chainingCount, i;
  uint64_t currentXor, previousFaceXor, nextFaceXor;
#define ONE_IN_ONE_OUT_CORE(a, b, colors)                              \
  (__builtin_popcountll((currentXor = ((1ll << (a)) | (1ll << (b)))) & \
                        colors) == 1)
#define ONE_IN_ONE_OUT(a, b, colors)                               \
  (!ONE_IN_ONE_OUT_CORE(a, b, colors) ? 0                          \
   : ((1 << (a)) & colors)            ? (nextFaceXor = currentXor) \
                                      : (previousFaceXor = currentXor))
  /* The inner face is NFACES-1, with all the colors; the outer face is 0, with
   * no colors.
   */
  for (colors = 1, face = Faces + 1; colors < NFACES - 1; colors++, face++) {
    for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
      if ((cycle->colors & colors) == 0 || (cycle->colors & ~colors) == 0) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      }
      previousFaceXor = nextFaceXor = 0;
      chainingCount = ONE_IN_ONE_OUT(cycle->curves[cycle->length - 1],
                                     cycle->curves[0], colors)
                          ? 1
                          : 0;
      for (i = 1; i < cycle->length; i++) {
        if (ONE_IN_ONE_OUT(cycle->curves[i - 1], cycle->curves[i], colors)) {
          chainingCount++;
        }
      }
      if (chainingCount != 2) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      } else {
        assert(previousFaceXor);
        assert(nextFaceXor);
        face->nextByCycleId[cycleId] = Faces + (colors ^ nextFaceXor);
        face->previousByCycleId[cycleId] = Faces + (colors ^ previousFaceXor);
      }
    }
    recomputeCountOfChoices(face);
  }
  dynamicFaceSetCycleLength(0, NCOLORS);
  dynamicFaceSetCycleLength(~0, NCOLORS);
}

static void recomputeCountOfChoices(FACE face)
{
  trailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
}

bool dynamicFaceSetCycleLength(uint32_t faceColors, uint32_t length)
{
  FACE face = Faces + (faceColors & (NFACES - 1));
  CYCLE cycle;
  uint32_t cycleId;
  if (length == 0) {
    return true;
  }
  for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    if (cycle->length != length) {
      removeFromCycleSetWithTrail(cycleId, face->possibleCycles);
    }
  }
  recomputeCountOfChoices(face);
  return face->cycleSetSize != 0;
}

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
}

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color, j;
  FACE face, adjacent;
  EDGE edge;
  initializeLengthOfCycleOfFaces();
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    face->colors = facecolors;
    for (j = 0; j < CYCLESET_LENGTH - 1; j++) {
      face->possibleCycles[j] = ~0;
    }
    face->possibleCycles[j] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;

    for (color = 0; color < NCOLORS; color++) {
      uint32_t colorbit = (1 << color);
      adjacent = Faces + (facecolors ^ (colorbit));
      face->adjacentFaces[color] = adjacent;
      edge = &face->edges[color];
      edge->colors = face->colors;
      edge->level = __builtin_popcount(face->colors);
      edge->color = color;
      edge->reversed = &adjacent->edges[color];
    }
  }
  applyMonotonicity();
  initializePossiblyTo();
}

static void initializePossiblyTo(void)
{
  uint32_t facecolors, color, othercolor;
  FACE face;
  EDGE edge;
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    for (color = 0; color < NCOLORS; color++) {
      edge = &face->edges[color];
      for (othercolor = 0; othercolor < NCOLORS; othercolor++) {
        if (othercolor == color) {
          continue;
        }
        edge->possiblyTo[othercolor].point =
            dynamicPointAdd(face, edge, othercolor);
      }
    }
  }
}

static FAILURE checkLengthOfCycleOfFaces(FACE face)
{
  uint32_t i = 0,
           expected = FaceSumOfFaceDegree[__builtin_popcount(face->colors)];
  FACE f = face;
  /* Don't call this with inner or outer face. */
  assert(expected != 1);
  do {
    f = f->next;
    i++;
    assert(i <= expected);
    if (f == face) {
      if (i != expected) {
        return failureDisconnectedFaces(0);
      }
      return NULL;
      ;
    }
  } while (f != NULL);
  assert(0);
}

FAILURE dynamicFaceFinalCorrectnessChecks(void)
{
  FAILURE failure;
  COLORSET colors = 1;
  FACE face;
#if NCOLORS == 6
  switch (dynamicSymmetryTypeFaces()) {
    case NON_CANONICAL:
      return failureNonCanonical();
    case EQUIVOCAL:
      /* Does not happen? But not deeply problematic if it does. */
      assert(0); /* could fall through, but will get duplicate solutions. */
      break;
    case CANONICAL:
      break;
  }
#endif
  for (colors = 1; colors < (NFACES - 1); colors |= face->previous->colors) {
    face = Faces + colors;
    CHECK_FAILURE(checkLengthOfCycleOfFaces(face));
  }
  return NULL;
}

/* We use this global to defer removing colors from the dynamicSearch space
   until we have completed computing consequential changes. From a design point
   of view it is local to the dynamicFaceMakeChoice method, so does not need to
   be in the Trail.
 */
static FAILURE makeChoiceInternal(FACE face, int depth);
uint64_t cycleForcedCounter = 0;
uint64_t cycleSetReducedCounter = 0;

void dynamicFaceSetupCentral(int* faceDegrees)
{
  CYCLE cycle;
  uint64_t i;
  FACE centralFace = Faces + (NFACES - 1);
  for (i = 0; i < NCOLORS; i++) {
    dynamicFaceSetCycleLength(~(1 << i), faceDegrees[i]);
  }
  for (cycle = Cycles;; cycle++) {
    if (cycle->length != NCOLORS) {
      continue;
    }
    for (i = 1; i < NCOLORS; i++) {
      if (cycle->curves[i] != i) {
        goto NextCycle;
      }
    }
    centralFace->cycle = cycle;
    break;
  NextCycle:
    continue;
  }
  dynamicFaceMakeChoice(centralFace);
}

static void restrictCycles(FACE face, CYCLESET cycleSet)
{
  uint32_t i;
  uint_trail toBeCleared;
  uint_trail newCycleSetSize = face->cycleSetSize;

  for (i = 0; i < CYCLESET_LENGTH; i++) {
    toBeCleared = face->possibleCycles[i] & ~cycleSet[i];
    if (toBeCleared == 0) {
      continue;
    }
    trailSetInt(&face->possibleCycles[i],
                face->possibleCycles[i] & cycleSet[i]);
    newCycleSetSize -= __builtin_popcountll(toBeCleared);
  }
  if (newCycleSetSize < face->cycleSetSize) {
    cycleSetReducedCounter++;
    trailSetInt(&face->cycleSetSize, newCycleSetSize);
  }
}

static FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                          int depth)
{
  /* check for conflict or no-op. */
  if (face->cycleSetSize == 1 || face->cycle != NULL) {
    if (!cycleSetMember(face->cycle - Cycles, onlyCycleSet)) {
      return failureConflictingConstraints(depth);
    }
    return NULL;
  }

  /* Carefully update face->possibleCycles to be anded with cycleSet, on the
     Trail. decreasing the count as we go. */
  restrictCycles(face, onlyCycleSet);

  if (face->cycleSetSize == 0) {
    return failureNoMatchingCycles(depth);
  }
  if (face->cycleSetSize == 1) {
    setDynamicPointer(&face->cycle, cycleSetFindFirst(face->possibleCycles));
    cycleForcedCounter++;
    return makeChoiceInternal(face, depth + 1);
  }
  return NULL;
}

static FAILURE propogateChoice(FACE face, EDGE edge, int depth)
{
  FAILURE failure;
  UPOINT upoint = edge->to->point;
  COLOR aColor = edge->color;
  COLOR bColor =
      edge->color == upoint->primary ? upoint->secondary : upoint->primary;

  FACE aFace = face->adjacentFaces[edge->color];
  FACE abFace = aFace->adjacentFaces[bColor];
  uint32_t index = cycleIndexOfColor(face->cycle, aColor);
  assert(abFace == face->adjacentFaces[bColor]->adjacentFaces[aColor]);
  assert(abFace != face);
  CHECK_FAILURE(restrictAndPropogateCycles(
      abFace, face->cycle->sameDirection[index], depth));
  CHECK_FAILURE(restrictAndPropogateCycles(
      aFace, face->cycle->oppositeDirection[index], depth));
  return NULL;
}

/*
We have just set the value of the cycle on this face.
We need to:
- allocate the points for edges that don't have them
- add the other faces to the new points
- attach the points to the edges and vice versa
- check for crossing limit (3 for each ordered pair)
- modify the possible cycles of the adjacent faces (including diagonally)
- if any of the adjacent faces now have zero possible cycles return false
- if any of the adjacent faces now have one possible cycle, set the cycle on
that face, and make that choice (if that choice fails with a false, then so do
we)
*/
static FAILURE makeChoiceInternal(FACE face, int depth)
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
    CHECK_FAILURE(
        dynamicEdgeCurveChecks(&face->edges[cycle->curves[i]], depth));
  }
  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(propogateChoice(face, &face->edges[cycle->curves[i]], depth));
  }
  for (i = 0; i < NCOLORS; i++) {
    if (memberOfColorSet(i, cycle->colors)) {
      continue;
    }
    CHECK_FAILURE(restrictAndPropogateCycles(
        face->adjacentFaces[i], CycleSetOmittingOneColor[i], depth));
  }

  if (face->colors == 0 || face->colors == (NFACES - 1)) {
    setDynamicPointer(&face->next, face);
    setDynamicPointer(&face->previous, face);
  } else {
    setDynamicPointer(&face->next, face->nextByCycleId[cycleId]);
    setDynamicPointer(&face->previous, face->previousByCycleId[cycleId]);
  }

  if (face->colors != 0 && face->colors != (NFACES - 1)) {
    assert(face->next != Faces);
    assert(face->previous != Faces);
  }

  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      if (memberOfColorSet(i, cycle->colors) &&
          memberOfColorSet(j, cycle->colors)) {
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

static void setToSingletonCycleSet(FACE face, uint64_t cycleId)
{
  CYCLESET_DECLARE cycleSet;
  uint64_t i;
  memset(cycleSet, 0, sizeof(cycleSet));
  initializeCycleSetAdd(cycleId, cycleSet);
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    trailMaybeSetInt(&face->possibleCycles[i], cycleSet[i]);
  }
  trailMaybeSetInt(&face->cycleSetSize, 1);
}

FAILURE dynamicFaceMakeChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64_t cycleId;
  DynamicCycleGuessCounter++;
  DynamicColorCompleted = 0;
  face->backtrack = Trail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleId >= 0);
  assert(cycleSetMember(cycleId, face->possibleCycles));
  setToSingletonCycleSet(face, cycleId);

  failure = makeChoiceInternal(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (DynamicColorCompleted) {
    for (completedColor = 0; completedColor < NCOLORS; completedColor++) {
      if (memberOfColorSet(completedColor, DynamicColorCompleted)) {
        if (!dynamicColorRemoveFromSearch(completedColor)) {
          return failureDisconnectedCurve(0);
        }
      }
    }
  }
  return NULL;
}

bool dynamicColorRemoveFromSearch(COLOR color)
{
  FACE f;
  uint32_t i;
  for (i = 0, f = Faces; i < NFACES; i++, f++) {
    if (f->cycle == NULL) {
      /* Discard failure, we will report a different one. */
      if (f->edges[color].to == NULL &&
          restrictAndPropogateCycles(f, CycleSetOmittingOneColor[color], 0) !=
              NULL) {
        return false;
      }
    }
  }
  return true;
}

char* faceToStr(char* dbuffer, FACE face)
{
  char colorBuf[256];
  char cycleBuf[256];
  colorSetToStr(colorBuf, face->colors);
  dynamicCycleToStr(cycleBuf, face->cycle);
  if (face->cycleSetSize > 1) {
    sprintf(dbuffer, "%s%s^%llu", colorBuf, cycleBuf, face->cycleSetSize);
  } else {
    sprintf(dbuffer, "%s%s", colorBuf, cycleBuf);
  }
  return dbuffer;
}

void dynamicFacePrint(FACE face)
{
  char buffer[2048];
  printf("%s\n", faceToStr(buffer, face));
}

void dynamicFacePrintSelected(void)
{
  uint32_t i;
  FACE face;
  for (i = 0, face = Faces; i < NFACES; i++, face++) {
    if (face->cycle || face->cycleSetSize < 2) {
      dynamicFacePrint(face);
    }
  }
}

void dynamicSolutionPrint(FILE* fp)
{
  COLORSET colors = 0;
  if (fp == NULL) {
    fp = stdout;
  }

  while (true) {
    FACE face = Faces + colors;
    do {
      FACE next = face->next;
      COLORSET colorBeingDropped = face->colors & ~next->colors;
      COLORSET colorBeingAdded = next->colors & ~face->colors;
      char buffer[256];
      fprintf(fp, "%s [%c,%c] ", faceToStr(buffer, face),
              colorToChar(ffs(colorBeingDropped) - 1),
              colorToChar(ffs(colorBeingAdded) - 1));
      face = next;
    } while (face->colors != colors);
    fprintf(fp, "\n");
    if (colors == (NFACES - 1)) {
      break;
    }
    colors |= (face->previous->colors | 1);
  }
  fprintf(fp, "\n");
}

void resetFaces()
{
  memset(Faces, 0, sizeof(Faces));
  memset(FaceSumOfFaceDegree, 0, sizeof(FaceSumOfFaceDegree));
}

uint32_t cycleIdFromColors(char* colors)
{
  COLOR cycle[NCOLORS];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return cycleFindId(cycle, i);
}

FACE dynamicFaceFromColors(char* colors)
{
  int face_id = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    face_id |= (1 << (*colors - 'a'));
    colors++;
  }
  return Faces + face_id;
}

void dynamicSolutionWrite(const char* prefix)
{
  EDGE corners[3][2];
  char filename[1024];
  char buffer[1024];
  int numberOfVariations = 1;
  int pLength;
  FILE* fp;
  if (strcmp(prefix, lastPrefix) != 0) {
    strcpy(lastPrefix, prefix);
    solutionNumber = 1;
  }
  snprintf(filename, sizeof(filename), "%s-%2.2d.txt", prefix,
           solutionNumber++);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    perror(filename);
    exit(EXIT_FAILURE);
  }
  dynamicSolutionPrint(fp);
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      fprintf(fp, "{%c:%d} ", colorToChar(a), i);
      if (corners[i][0] == NULL) {
        EDGE edge = edgeOnCentralFace(a);
        pLength = edgePathLength(edge, edgeFollowBackwards(edge));
        fprintf(fp, "NULL/%d ", pLength);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1]);
        buffer[0] = buffer[1] = 0;
        fprintf(fp, "(%s => %s/%d) ", edgeToStr(buffer, corners[i][0]),
                edgeToStr(buffer, corners[i][1]), pLength);
      }
      numberOfVariations *= pLength;
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n\nVariations = %d\n", numberOfVariations);
  fclose(fp);
}

/*
Set up the next values for edge1 and edge2 that have the same color.
All four edges meet at the same point.
The edge3 and edge4 have the other color.
The next value for both  edge1 and edge2  for the other color is set to
the reverse of the other edge.
*/
static void linkOut(EDGE edge1, EDGE edge2, EDGE edge3, EDGE edge4)
{
  COLOR other = edge3->color;
  uint32_t level1 = edge1->level;
  uint32_t level2 = edge2->level;
  uint32_t level3 = edge3->reversed->level;
  uint32_t level4 = edge4->reversed->level;

  assert(edge1->color == edge2->color);
  assert(edge1->possiblyTo[other].next == NULL);
  assert(edge2->possiblyTo[other].next == NULL);
  assert(edge1->possiblyTo[other].point == edge2->possiblyTo[other].point);
  assert(edge1->possiblyTo[other].point->colors =
             (1u << edge1->color | 1u << other));
  edge1->possiblyTo[other].next = edge2->reversed;
  edge2->possiblyTo[other].next = edge1->reversed;
  if (level1 == level3) {
    assert(level2 == level4);
  } else {
    assert(level1 == level4);
    assert(level2 == level3);
  }
}

/*
Set up next on every possiblyTo.

It must be the same color, and the reverse of the other edge of that color at
the point.
*/
void initializePoints(void)
{
  uint32_t i, j, k;
  for (i = 0; i < NPOINTS; i++) {
    UPOINT p = DynamicPointAllUPoints + i;
    linkOut(p->incomingEdges[0], p->incomingEdges[1], p->incomingEdges[2],
            p->incomingEdges[3]);
    linkOut(p->incomingEdges[2], p->incomingEdges[3], p->incomingEdges[0],
            p->incomingEdges[1]);
  }
  for (i = 0; i < NFACES; i++) {
    FACE f = Faces + i;
    for (j = 0; j < NCOLORS; j++) {
      for (k = 0; k < NCOLORS; k++) {
        assert(j == f->edges[j].color);
        if (k == j) {
          continue;
        }
        assert(f->edges[j].possiblyTo[k].point != NULL);
        assert(f->edges[j].possiblyTo[k].next != NULL);
        assert(f->edges[j].possiblyTo[k].next->color == j);
        assert(f->edges[j].possiblyTo[k].next->reversed->possiblyTo[k].point ==
               f->edges[j].possiblyTo[k].point);
      }
    }
  }
}
