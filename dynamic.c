/* These are the operations that change the dynamic fields in the data
  structures. These are hence the mechanics of dynamicSearch.
*/

#include "dynamic.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "failure.h"
#include "graph.h"
#include "point.h"
#include "statistics.h"
#include "trail.h"
/* We use this global to defer removing colors from the dynamicSearch space
   until we have completed computing consequential changes. From a design point
   of view it is local to the dynamicFaceMakeChoice method, so does not need to
   be in the DynamicTrail.
 */
COLORSET DynamicColorCompleted;
static FAILURE makeChoiceInternal(FACE face, int depth);
uint64_t DynamicCycleGuessCounter = 0;
static uint64_t cycleForcedCounter = 0;
static uint64_t cycleSetReducedCounter = 0;

/* If using this macro, you must declare a local variable failure. */
#define CHECK_FAILURE(call) \
  failure = (call);         \
  if (failure != NULL) {    \
    return failure;         \
  }

void initializeFaceSetupCentral(int *faceDegrees)
{
  CYCLE cycle;
  uint64_t i;
  FACE centralFace = Faces + (NFACES - 1);
  for (i = 0; i < NCOLORS; i++) {
    setCycleLength(~(1 << i), faceDegrees[i]);
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
    dynamicTrailSetInt(&face->possibleCycles[i],
                       face->possibleCycles[i] & cycleSet[i]);
    newCycleSetSize -= __builtin_popcountll(toBeCleared);
  }
  if (newCycleSetSize < face->cycleSetSize) {
    cycleSetReducedCounter++;
    dynamicTrailSetInt(&face->cycleSetSize, newCycleSetSize);
  }
}

static FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                          int depth)
{
  /* check for conflict or no-op. */
  if (face->cycleSetSize == 1 || face->cycle != NULL) {
    if (!initializeCycleSetMember(face->cycle - Cycles, onlyCycleSet)) {
      return dynamicFailureConflictingConstraints(depth);
    }
    return NULL;
  }

  /* Carefully update face->possibleCycles to be anded with cycleSet, on the
     DynamicTrail. decreasing the count as we go. */
  restrictCycles(face, onlyCycleSet);

  if (face->cycleSetSize == 0) {
    return dynamicFailureNoMatchingCycles(depth);
  }
  if (face->cycleSetSize == 1) {
    setDynamicPointer(&face->cycle,
                      initializeCycleSetFindFirst(face->possibleCycles));
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
  /* equality in the followign assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */
#if FACE_DEBUG
  printf("Making choice (internal): ");
  dynamicFacePrint(face);
#endif
  /* TODO: what order should these checks be done in. There are a lot of them.
   */
  assert(depth <= NFACES);
  for (i = 0; i < cycle->length - 1; i++) {
    CHECK_FAILURE(dynamicPointAssign(face, cycle->curves[i],
                                     cycle->curves[i + 1], depth));
  }
  CHECK_FAILURE(
      dynamicPointAssign(face, cycle->curves[i], cycle->curves[0], depth));

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
    CHECK_FAILURE(restrictAndPropogateCycles(face->adjacentFaces[i],
                                             CycleSetOmittingOne[i], depth));
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
                                     CycleSetOmittingPair[i][j], depth));
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
    dynamicTrailMaybeSetInt(&face->possibleCycles[i], cycleSet[i]);
  }
  dynamicTrailMaybeSetInt(&face->cycleSetSize, 1);
}

FAILURE dynamicFaceMakeChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64_t cycleId;
  DynamicCycleGuessCounter++;
  DynamicColorCompleted = 0;
  face->backtrack = DynamicTrail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleId >= 0);
  assert(initializeCycleSetMember(cycleId, face->possibleCycles));
  setToSingletonCycleSet(face, cycleId);

#if FACE_DEBUG
  printf("Making choice: ");
  dynamicFacePrint(face);
#endif

  failure = makeChoiceInternal(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (DynamicColorCompleted) {
    for (completedColor = 0; completedColor < NCOLORS; completedColor++) {
      if (memberOfColorSet(completedColor, DynamicColorCompleted)) {
        if (!dynamicColorRemoveFromSearch(completedColor)) {
          return dynamicFailureDisconnectedCurve(0);
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
          restrictAndPropogateCycles(f, CylesetWithoutColor[color], 0) !=
              NULL) {
        return false;
      }
    }
  }
  return true;
}

void initializeDynamicCounters(void)
{
  dynamicStatisticNew(&DynamicCycleGuessCounter, "?", "guesses");
  dynamicStatisticNew(&cycleForcedCounter, "+", "forced");
  dynamicStatisticNew(&cycleSetReducedCounter, "-", "reduced");
}
