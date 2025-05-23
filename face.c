/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"

#include "failure.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

/* Global variables - globally scoped */
struct face Faces[NFACES];
static uint64 FaceSumOfFaceDegree[NCOLORS + 1];
static uint64 CycleForcedCounter = 0;
static uint64 CycleSetReducedCounter = 0;

/* Declaration of file scoped static functions */
static void recomputeCountOfChoices(FACE face);
static void initializePossiblyTo(void);
static void applyMonotonicity(void);
static void initializeLengthOfCycleOfFaces(void);
static void restrictCycles(FACE face, CYCLESET cycleSet);
static FAILURE checkLengthOfCycleOfFaces(FACE face);
static void countEdge(EDGE edge);
static void setupColors(VERTEX vertex, COLOR colors[2]);
static FAILURE processIncomingEdge(EDGE edge, COLOR colors[2],
                                   int incomingEdgeSlot, int depth);
static void validateIncomingEdges(VERTEX vertex);
static FAILURE handleExistingEdge(FACE face, COLOR aColor, COLOR bColor,
                                  int depth);
static bool isCycleValidForFace(CYCLE cycle, COLORSET faceColors);
static bool isEdgeTransition(COLOR curve1, COLOR curve2, COLORSET faceColors,
                             COLORSET* previousFaceColors,
                             COLORSET* nextFaceColors);
static bool exactlyTwoEdgeTransitions(CYCLE cycle, COLORSET faceColors,
                                      COLORSET* previousFaceColors,
                                      COLORSET* nextFaceColors);
static void facePrint(FACE face);

/* Externally linked functions - initialize... */
void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color;
  FACE face, adjacent;
  EDGE edge;
  if (Faces[1].colors == 0) {
    statisticIncludeInteger(&CycleForcedCounter, "+", "forced", false);
    statisticIncludeInteger(&CycleSetReducedCounter, "-", "reduced", true);
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
}

/* Externally linked functions - dynamic... */
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
      cycleSetRemoveCycleWithTrail(face->possibleCycles, cycleId);
    }
  }
  recomputeCountOfChoices(face);
  return face->cycleSetSize != 0;
}

void dynamicFaceSetupCentral(FACE_DEGREE* faceDegrees)
{
  CYCLE cycle;
  uint64 i;
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

FAILURE dynamicFaceIncludeVertex(FACE face, COLOR aColor, COLOR bColor,
                                 int depth)
{
  FAILURE failure;
  VERTEX vertex;
  EDGE edge;
  COLOR colors[2];

  failure = handleExistingEdge(face, aColor, bColor, depth);
  if (failure != NULL || face->edges[aColor].to != NULL) {
    return failure;
  }

  vertex = face->edges[aColor].possiblyTo[bColor].vertex;
  CHECK_FAILURE(
      edgeCheckCrossingLimit(vertex->primary, vertex->secondary, depth));
  setupColors(vertex, colors);

  for (int incomingEdgeSlot = 0; incomingEdgeSlot < 4; incomingEdgeSlot++) {
    edge = vertex->incomingEdges[incomingEdgeSlot];
    CHECK_FAILURE(processIncomingEdge(edge, colors, incomingEdgeSlot, depth));
    countEdge(edge);
  }
  validateIncomingEdges(vertex);
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
          faceRestrictAndPropogateCycles(f, CycleSetOmittingOneColor[color],
                                         0) != NULL) {
        return false;
      }
    }
  }
  return true;
}

/* Externally linked functions - face... */
FAILURE faceFinalCorrectnessChecks(void)
{
  FAILURE failure;
  COLORSET colors = 1;
  FACE face;
#if NCOLORS == 6
  switch (s6FacesSymmetryType()) {
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

FAILURE facePropogateChoice(FACE face, EDGE edge, int depth)
{
  FAILURE failure;
  VERTEX vertex = edge->to->vertex;
  COLOR aColor = edge->color;
  COLOR bColor =
      edge->color == vertex->primary ? vertex->secondary : vertex->primary;

  FACE aFace = face->adjacentFaces[edge->color];
  FACE abFace = aFace->adjacentFaces[bColor];
  uint32_t index = cycleIndexOfColor(face->cycle, aColor);
  assert(abFace == face->adjacentFaces[bColor]->adjacentFaces[aColor]);
  assert(abFace != face);
  CHECK_FAILURE(faceRestrictAndPropogateCycles(
      abFace, face->cycle->sameDirection[index], depth));
  CHECK_FAILURE(faceRestrictAndPropogateCycles(
      aFace, face->cycle->oppositeDirection[index], depth));
  return NULL;
}

FAILURE faceRestrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
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
    TRAIL_SET_POINTER(&face->cycle, cycleSetFirst(face->possibleCycles));
    CycleForcedCounter++;
    return dynamicFaceChoice(face, depth + 1);
  }
  return NULL;
}

char* faceToString(FACE face)
{
  char* buffer = getBuffer();
  char* colorBuf = colorSetToString(face->colors);
  char* cycleBuf = cycleToString(face->cycle);

  if (face->cycleSetSize > 1) {
    sprintf(buffer, "%s%s^%llu", colorBuf, cycleBuf, face->cycleSetSize);
  } else {
    sprintf(buffer, "%s%s", colorBuf, cycleBuf);
  }
  return usingBuffer(buffer);
}

/* File scoped static functions */
static void facePrint(FACE face)
{
  printf("%s\n", faceToString(face));
}

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
        edge->possiblyTo[othercolor].vertex =
            initializeVertexIncomingEdge(face->colors, edge, othercolor);
      }
    }
  }
}

static bool isCycleValidForFace(CYCLE cycle, COLORSET faceColors)
{
  return (cycle->colors & faceColors) != 0 &&
         (cycle->colors & ~faceColors) != 0;
}

static bool isEdgeTransition(COLOR curve1, COLOR curve2, COLORSET faceColors,
                             COLORSET* previousFaceColors,
                             COLORSET* nextFaceColors)
{
  uint64 currentXor = (1ll << curve1) | (1ll << curve2);
  if (__builtin_popcountll(currentXor & faceColors) != 1) {
    return false;
  }

  if ((1 << curve1) & faceColors) {
    assert(*nextFaceColors == 0);
    *nextFaceColors = faceColors ^ currentXor;
  } else {
    assert(*previousFaceColors == 0);
    *previousFaceColors = faceColors ^ currentXor;
  }
  return true;
}

static bool exactlyTwoEdgeTransitions(CYCLE cycle, COLORSET faceColors,
                                      COLORSET* previousFaceColors,
                                      COLORSET* nextFaceColors)
{
  uint32_t count = 0;
  COLORSET dummy = 0;

  // Check transition from last to first curve
  if (isEdgeTransition(cycle->curves[cycle->length - 1], cycle->curves[0],
                       faceColors, previousFaceColors, nextFaceColors)) {
    count++;
  }

  // Check transitions between consecutive curves
  for (uint32_t i = 1; i < cycle->length; i++) {
    if (isEdgeTransition(cycle->curves[i - 1], cycle->curves[i], faceColors,
                         previousFaceColors, nextFaceColors)) {
      count++;
      switch (count) {
        case 2:
          // Both *previousFaceColors and *nextFaceColors have been set,
          // Change them to preserve invariants expected by further calls to
          // isEdgeTransition.
          previousFaceColors = &dummy;
          nextFaceColors = &dummy;
          break;
        case 3:
          return false;
      }
    }
  }

  return true;
}

static void applyMonotonicity(void)
{
  // The inner face is NFACES-1, with all the colors; the outer face is 0, with
  // no colors.
  for (COLORSET faceColors = 1; faceColors < NFACES - 1; faceColors++) {
    FACE face = Faces + faceColors;
    for (uint32_t cycleId = 0; cycleId < NCYCLES; cycleId++) {
      CYCLE cycle = Cycles + cycleId;
      if (!isCycleValidForFace(cycle, faceColors)) {
        cycleSetRemove(cycleId, face->possibleCycles);
        continue;
      }

      COLORSET previousFaceColors = 0, nextFaceColors = 0;
      bool twoTransitions = exactlyTwoEdgeTransitions(
          cycle, faceColors, &previousFaceColors, &nextFaceColors);

      if (!twoTransitions) {
        cycleSetRemove(cycleId, face->possibleCycles);
      } else {
        assert(previousFaceColors);
        assert(nextFaceColors);
        face->nextByCycleId[cycleId] = Faces + nextFaceColors;
        face->previousByCycleId[cycleId] = Faces + previousFaceColors;
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

static void countEdge(EDGE edge)
{
  uint_trail* edgeCountPtr =
      &EdgeColorCountState[IS_PRIMARY_EDGE(edge)][edge->color];
  trailSetInt(edgeCountPtr, (*edgeCountPtr) + 1);
}

static void setupColors(VERTEX vertex, COLOR colors[2])
{
  colors[0] = vertex->primary;
  colors[1] = vertex->secondary;
}

static FAILURE processIncomingEdge(EDGE edge, COLOR colors[2],
                                   int incomingEdgeSlot, int depth)
{
  assert(edge->color == colors[(incomingEdgeSlot & 2) >> 1]);
  assert(edge->color != colors[1 - ((incomingEdgeSlot & 2) >> 1)]);
  if (edge->to != NULL) {
    if (edge->to != &edge->possiblyTo[colors[(incomingEdgeSlot & 2) >> 1]]) {
      return failureVertexConflict(depth);
    }
    assert(edge->to ==
           &edge->possiblyTo[colors[1 - ((incomingEdgeSlot & 2) >> 1)]]);
  } else {
    TRAIL_SET_POINTER(
        &edge->to,
        &edge->possiblyTo[colors[1 - ((incomingEdgeSlot & 2) >> 1)]]);
  }

  assert(edge->to != &edge->possiblyTo[edge->color]);
  return NULL;
}

static void validateIncomingEdges(VERTEX vertex)
{
  for (int incomingEdgeSlot = 0; incomingEdgeSlot < 4; incomingEdgeSlot++) {
    assert(vertex->incomingEdges[incomingEdgeSlot]->to != NULL);
  }
}

static FAILURE handleExistingEdge(FACE face, COLOR aColor, COLOR bColor,
                                  int depth)
{
  if (face->edges[aColor].to != NULL) {
    assert(face->edges[aColor].to != &face->edges[aColor].possiblyTo[aColor]);
    if (face->edges[aColor].to != &face->edges[aColor].possiblyTo[bColor]) {
      return failureVertexConflict(depth);
    }
    assert(face->edges[aColor].to == &face->edges[aColor].possiblyTo[bColor]);
    return NULL;
  }
  return NULL;
}
