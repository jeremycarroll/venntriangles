/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "failure.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

uint64 CycleForcedCounter = 0;
uint64 CycleSetReducedCounter = 0;

static void setupColors(VERTEX vertex, COLOR colors[2])
{
  colors[0] = vertex->primary;
  colors[1] = vertex->secondary;
}

static void validateIncomingEdges(VERTEX vertex)
{
  for (int incomingEdgeSlot = 0; incomingEdgeSlot < 4; incomingEdgeSlot++) {
    assert(vertex->incomingEdges[incomingEdgeSlot]->to != NULL);
  }
}

void dynamicRecomputeCountOfChoices(FACE face)
{
  trailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
}

static void dynamicRestrictCycles(FACE face, CYCLESET cycleSet)
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

static void dynamicCountEdge(EDGE edge)
{
  uint_trail* edgeCountPtr =
      &EdgeColorCountState[IS_CLOCKWISE_EDGE(edge)][edge->color];
  trailSetInt(edgeCountPtr, (*edgeCountPtr) + 1);
}

static FAILURE dynamicHandleExistingEdge(FACE face, COLOR aColor, COLOR bColor,
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

/*
 * Processes an incoming edge at a vertex intersection of two curves.
 *
 * The incomingEdgeSlot parameter encodes both the edge and its relationship to
 * colors:
 * - Slots 0,1 correspond to the primary color (colors[0])
 * - Slots 2,3 correspond to the secondary color (colors[1])
 *
 * The bit operation (incomingEdgeSlot & 2) >> 1 extracts the color index:
 * - For slots 0,1: (0 & 2) >> 1 = 0 → colors[0] (primary)
 * - For slots 2,3: (2 & 2) >> 1 = 1 → colors[1] (secondary)
 *
 * The edge's destination is always set to the other color's possiblyTo entry
 * to establish the correct connections between faces.
 */
static FAILURE dynamicProcessIncomingEdge(EDGE edge, COLOR colors[2],
                                          int incomingEdgeSlot, int depth)
{
  // Determine which color this edge corresponds to based on the slot
  int colorIndex =
      (incomingEdgeSlot & 2) >> 1;       // 0 for slots 0,1; 1 for slots 2,3
  int otherColorIndex = 1 - colorIndex;  // The other color index (0 or 1)

  // Verify edge's color matches the appropriate slot's color
  assert(edge->color == colors[colorIndex]);
  assert(edge->color != colors[otherColorIndex]);

  if (edge->to != NULL) {
    // If the edge already has a destination, ensure it's consistent
    if (edge->to != &edge->possiblyTo[colors[colorIndex]]) {
      return failureVertexConflict(depth);
    }
    assert(edge->to == &edge->possiblyTo[colors[otherColorIndex]]);
  } else {
    // Set the edge's destination to connect to the other color
    TRAIL_SET_POINTER(&edge->to, &edge->possiblyTo[colors[otherColorIndex]]);
  }

  assert(edge->to != &edge->possiblyTo[edge->color]);
  return NULL;
}

static FAILURE dynamicCheckLengthOfCycleOfFaces(FACE face)
{
  uint32_t i = 0,
           expected = FaceSumOfFaceDegree[__builtin_popcount(face->colors)];
  FACE f = face;
  assert(expected != 1);  // Don't call with inner or outer face
  do {
    f = f->next;
    i++;
    assert(i <= expected);
    if (f == face) {
      if (i != expected) {
        return failureDisconnectedFaces(0);
      }
      return NULL;
    }
  } while (f != NULL);
  assert(0);
}

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
      dynamicCycleSetRemoveCycle(face->possibleCycles, cycleId);
    }
  }
  dynamicRecomputeCountOfChoices(face);
  return face->cycleSetSize != 0;
}

FAILURE dynamicFaceRestrictAndPropagateCycles(FACE face, CYCLESET onlyCycleSet,
                                              int depth)
{
  if (face->cycleSetSize == 1 || face->cycle != NULL) {
    if (!cycleSetMember(face->cycle - Cycles, onlyCycleSet)) {
      return failureConflictingConstraints(depth);
    }
    return NULL;
  }

  dynamicRestrictCycles(face, onlyCycleSet);

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

void dynamicFaceSetupCentral(FACE_DEGREE* faceDegrees)
{
  uint64 i;
  FACE centralFace = Faces + (NFACES - 1);
  for (i = 0; i < NCOLORS; i++) {
    dynamicFaceSetCycleLength(~(1 << i), faceDegrees[i]);
  }
  // The cycle (a b c d e f) is the last one.
  centralFace->cycle = &Cycles[NCYCLES - 1];
  dynamicFaceBacktrackableChoice(centralFace);
}

FAILURE dynamicFaceIncludeVertex(FACE face, COLOR aColor, COLOR bColor,
                                 int depth)
{
  FAILURE failure;
  VERTEX vertex;
  EDGE edge;
  COLOR colors[2];

  failure = dynamicHandleExistingEdge(face, aColor, bColor, depth);
  if (failure != NULL || face->edges[aColor].to != NULL) {
    return failure;
  }

  vertex = face->edges[aColor].possiblyTo[bColor].vertex;
  CHECK_FAILURE(
      dynamicEdgeCheckCrossingLimit(vertex->primary, vertex->secondary, depth));
  setupColors(vertex, colors);

  for (int incomingEdgeSlot = 0; incomingEdgeSlot < 4; incomingEdgeSlot++) {
    edge = vertex->incomingEdges[incomingEdgeSlot];
    CHECK_FAILURE(
        dynamicProcessIncomingEdge(edge, colors, incomingEdgeSlot, depth));
    dynamicCountEdge(edge);
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
          dynamicFaceRestrictAndPropagateCycles(
              f, CycleSetOmittingOneColor[color], 0) != NULL) {
        return false;
      }
    }
  }
  return true;
}

FAILURE dynamicFaceFinalCorrectnessChecks(void)
{
  FAILURE failure;
  COLORSET colors = 1;
  FACE face;
#if NCOLORS == 6
  switch (s6FacesSymmetryType()) {
    case NON_CANONICAL:
      return failureNonCanonical();
    case EQUIVOCAL:
      assert(0);  // Should not happen
      break;
    case CANONICAL:
      break;
  }
#endif
  for (colors = 1; colors < (NFACES - 1); colors |= face->previous->colors) {
    face = Faces + colors;
    CHECK_FAILURE(dynamicCheckLengthOfCycleOfFaces(face));
  }
  return NULL;
}

FAILURE dynamicFacePropagateChoice(FACE face, EDGE edge, int depth)
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
  CHECK_FAILURE(dynamicFaceRestrictAndPropagateCycles(
      abFace, face->cycle->sameDirection[index], depth));
  CHECK_FAILURE(dynamicFaceRestrictAndPropagateCycles(
      aFace, face->cycle->oppositeDirection[index], depth));
  return NULL;
}
