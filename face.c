/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"

#include "d6.h"
#include "statistics.h"
#include "utils.h"

/* Global variables - globally scoped */
struct face Faces[NFACES];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];
uint64_t CycleGuessCounter = 0;
uint64_t CycleForcedCounter = 0;
uint64_t CycleSetReducedCounter = 0;

/* Declaration of file scoped static functions */
static void recomputeCountOfChoices(FACE face);
static void initializePossiblyTo(void);
static void applyMonotonicity(void);
static void initializeLengthOfCycleOfFaces(void);
static void restrictCycles(FACE face, CYCLESET cycleSet);
static FAILURE checkLengthOfCycleOfFaces(FACE face);

/* Externally linked functions */
bool dynamicFaceSetCycleLength(uint32_t faceColors, FACE_DEGREE length)
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

void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color;
  FACE face, adjacent;
  EDGE edge;

  statisticIncludeInteger(&CycleGuessCounter, "?", "guesses");
  statisticIncludeInteger(&CycleForcedCounter, "+", "forced");
  statisticIncludeInteger(&CycleSetReducedCounter, "-", "reduced");
  initializeLengthOfCycleOfFaces();
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    face->colors = facecolors;
    initializeCycleSetUniversal(face->possibleCycles);

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

FAILURE faceFinalCorrectnessChecks(void)
{
  FAILURE failure;
  COLORSET colors = 1;
  FACE face;
#if NCOLORS == 6
  switch (symmetryTypeFaces()) {
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

void dynamicFaceSetupCentral(FACE_DEGREE* faceDegrees)
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
  dynamicFaceBacktrackableChoice(centralFace);
}

FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet, int depth)
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
    TRAIL_SET_POINTER(&face->cycle, cycleSetFirst(face->possibleCycles));
    CycleForcedCounter++;
    return dynamicFaceChoice(face, depth + 1);
  }
  return NULL;
}

FAILURE propogateChoice(FACE face, EDGE edge, int depth)
{
  FAILURE failure;
  POINT point = edge->to->point;
  COLOR aColor = edge->color;
  COLOR bColor =
      edge->color == point->primary ? point->secondary : point->primary;

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

char* faceToStr(FACE face)
{
  char* buffer = getBuffer();
  char* colorBuf = colorSetToStr(face->colors);
  char* cycleBuf = cycleToStr(face->cycle);

  if (face->cycleSetSize > 1) {
    sprintf(buffer, "%s%s^%llu", colorBuf, cycleBuf, face->cycleSetSize);
  } else {
    sprintf(buffer, "%s%s", colorBuf, cycleBuf);
  }
  return usingBuffer(buffer);
}

void facePrint(FACE face) { printf("%s\n", faceToStr(face)); }

void facePrintSelected(void)
{
  uint32_t i;
  FACE face;
  for (i = 0, face = Faces; i < NFACES; i++, face++) {
    if (face->cycle || face->cycleSetSize < 2) {
      facePrint(face);
    }
  }
}

void resetFaces()
{
  memset(Faces, 0, sizeof(Faces));
  memset(FaceSumOfFaceDegree, 0, sizeof(FaceSumOfFaceDegree));
}

FACE faceFromColors(char* colors)
{
  int faceId = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    faceId |= (1 << (*colors - 'a'));
    colors++;
  }
  return Faces + faceId;
}

void initializePoints(void)
{
  uint32_t i, j, k;
  for (i = 0; i < NPOINTS; i++) {
    POINT p = PointAllUPoints + i;
    edgeLink(p->incomingEdges[0], p->incomingEdges[1], p->incomingEdges[2],
             p->incomingEdges[3]);
    edgeLink(p->incomingEdges[2], p->incomingEdges[3], p->incomingEdges[0],
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

FAILURE dynamicFaceIncludePoint(FACE face, COLOR aColor, COLOR bColor,
                                int depth)
{
  FAILURE crossingLimit;
  POINT point;
  EDGE edge;
  COLOR colors[2];
  uint_trail* edgeCountPtr;

  if (face->edges[aColor].to != NULL) {
    assert(face->edges[aColor].to != &face->edges[aColor].possiblyTo[aColor]);
    if (face->edges[aColor].to != &face->edges[aColor].possiblyTo[bColor]) {
      return failurePointConflict(depth);
    }
    assert(face->edges[aColor].to == &face->edges[aColor].possiblyTo[bColor]);
    return NULL;
  }
  point = face->edges[aColor].possiblyTo[bColor].point;
  crossingLimit =
      edgeCheckCrossingLimit(point->primary, point->secondary, depth);
  if (crossingLimit != NULL) {
    return crossingLimit;
  }
  colors[0] = point->primary;
  colors[1] = point->secondary;
  for (int i = 0; i < 4; i++) {
    edge = point->incomingEdges[i];
    assert(edge->color == colors[(i & 2) >> 1]);
    assert(edge->color != colors[1 - ((i & 2) >> 1)]);
    if (edge->to != NULL) {
      if (edge->to != &edge->possiblyTo[colors[(i & 2) >> 1]]) {
        return failurePointConflict(depth);
      }
      assert(edge->to == &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
    } else {
      TRAIL_SET_POINTER(&edge->to,
                        &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
    }

    assert(edge->to != &edge->possiblyTo[edge->color]);
    // Count edge
    edgeCountPtr =
        &EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color];
    trailSetInt(edgeCountPtr, (*edgeCountPtr) + 1);
  }
  for (int i = 0; i < 4; i++) {
    assert(point->incomingEdges[i]->to != NULL);
  }
  return NULL;
}

/* File scoped static functions */
static void recomputeCountOfChoices(FACE face)
{
  trailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
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
            initializePointIncomingEdge(face->colors, edge, othercolor);
      }
    }
  }
}

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
        cycleSetRemove(cycleId, face->possibleCycles);
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
        cycleSetRemove(cycleId, face->possibleCycles);
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

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
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
    CycleSetReducedCounter++;
    trailSetInt(&face->cycleSetSize, newCycleSetSize);
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
