/* These are the operations that change the dynamic fields in the data
  structure. initialize() must alreayd have been called before any of these
  methods are called.
*/

#include "venn.h"
/* We use this global to defer removing colors from the search space until
   we have completed computing consequential changes.
   From a design point of view it is local to the makeChoice method, so
   does not need to be in the trail.
 */
COLORSET completedColors;
static FAILURE makeChoiceInternal(FACE face, int depth);
uint64_t cycleGuessCounter = 0;
static uint64_t cycleForcedCounter = 0;
static uint64_t cycleSetReducedCounter = 0;

/* If using this macro, you must declare a local variable failure. */
#define CHECK_FAILURE(call) \
  failure = (call);         \
  if (failure != NULL) {    \
    return failure;         \
  }

#if NCURVES > 3
void setupCentralFaces(uint32_t aLength, uint32_t bLength, uint32_t cLength,
                       uint32_t dLength
#if NCURVES > 4
                       ,
                       uint32_t eLength
#if NCURVES > 5
                       ,
                       uint32_t fLength
#endif
#endif
)
{
  CYCLE cycle;
  uint64_t i;
  FACE centralFace = g_faces + (NFACES - 1);
  setCycleLength(~(1 << 0), aLength);
  setCycleLength(~(1 << 1), bLength);
  setCycleLength(~(1 << 2), cLength);
  setCycleLength(~(1 << 3), dLength);
#if NCURVES > 4
  setCycleLength(~(1 << 4), eLength);
#if NCURVES > 5
  setCycleLength(~(1 << 5), fLength);
#endif
#endif
  for (cycle = g_cycles;; cycle++) {
    if (cycle->length != NCURVES) {
      continue;
    }
    for (i = 1; i < NCURVES; i++) {
      if (cycle->curves[i] != i) {
        goto NextCycle;
      }
    }
    centralFace->cycle = cycle;
    break;
  NextCycle:
    continue;
  }
  makeChoice(centralFace);
}
#endif

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
    setDynamicInt(&face->possibleCycles[i],
                  face->possibleCycles[i] & cycleSet[i]);
    newCycleSetSize -= __builtin_popcountll(toBeCleared);
  }
  if (newCycleSetSize < face->cycleSetSize) {
    cycleSetReducedCounter++;
    setDynamicInt(&face->cycleSetSize, newCycleSetSize);
  }
}

static FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                          int depth)
{
  /* check for no-op. */
  if (face->cycleSetSize == 1 || face->cycle != NULL) {
    assert(memberOfCycleSet(face->cycle - g_cycles, onlyCycleSet));
    return NULL;
  }

  /* Carefully update face->possibleCycles to be anded with cycleSet, on the
     trail. decreasing the count as we go. */
  restrictCycles(face, onlyCycleSet);

  if (face->cycleSetSize == 0) {
    return noMatchingCyclesFailure(face->colors, depth);
  }
  if (face->cycleSetSize == 1) {
    setDynamicPointer(&face->cycle, findFirstCycleInSet(face->possibleCycles));
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
  uint32_t index = indexInCycle(face->cycle, aColor);
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
  uint32_t i;
  CYCLE cycle = face->cycle;
  FAILURE failure;
  /* equality in the followign assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */
#if EDGE_DEBUG
  printf("Making choice (internal): ");
  printFace(face);
#endif
  assert(depth <= NFACES);
  for (i = 0; i < cycle->length - 1; i++) {
    CHECK_FAILURE(
        assignPoint(face, cycle->curves[i], cycle->curves[i + 1], depth));
  }
  CHECK_FAILURE(assignPoint(face, cycle->curves[i], cycle->curves[0], depth));

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(curveChecks(&face->edges[cycle->curves[i]], depth));
  }
  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(propogateChoice(face, &face->edges[cycle->curves[i]], depth));
  }
  if (1) {
    for (i = 0; i < NCURVES; i++) {
      if (memberOfColorSet(i, cycle->colors)) {
        continue;
      }
      CHECK_FAILURE(restrictAndPropogateCycles(face->adjacentFaces[i],
                                               omittingCycleSets[i], depth));
    }
  }
  return NULL;
}

static void setToSingletonCycleSet(FACE face, uint64_t cycleId)
{
  CYCLESET_DECLARE cycleSet;
  uint64_t i;
  memset(cycleSet, 0, sizeof(cycleSet));
  addToCycleSet(cycleId, cycleSet);
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    maybeSetDynamicInt(&face->possibleCycles[i], cycleSet[i]);
  }
  maybeSetDynamicInt(&face->cycleSetSize, 1);
}

FAILURE makeChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64_t cycleId;
  cycleGuessCounter++;
  completedColors = 0;
  face->backtrack = trail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - g_cycles;
  assert(cycleId < NCYCLES);
  assert(cycleId >= 0);
  assert(memberOfCycleSet(cycleId, face->possibleCycles));
  setToSingletonCycleSet(face, cycleId);

#if EDGE_DEBUG
  printf("Making choice: ");
  printFace(face);
#endif

  failure = makeChoiceInternal(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (completedColors) {
    for (completedColor = 0; completedColor < NCURVES; completedColor++) {
      if (memberOfColorSet(completedColor, completedColors)) {
        if (!removeColorFromSearch(completedColor)) {
          return disconnectedCurveFailure(completedColor, 0);
        }
      }
    }
  }
  return NULL;
}

static CYCLESET_DECLARE withoutColor[NCURVES];

void clearWithoutColor() { memset(withoutColor, 0, sizeof(withoutColor)); }

void initializeWithoutColor()
{
  COLOR color;
  CYCLE cycle;
  uint32_t i;
  for (color = 0; color < NCURVES; color++) {
    for (i = 0, cycle = g_cycles; i < NCYCLES; i++, cycle++) {
      if (!memberOfColorSet(color, cycle->colors)) {
        addToCycleSet(i, withoutColor[color]);
      }
    }
  }
}

bool removeColorFromSearch(COLOR color)
{
  FACE f;
  uint32_t i;
  for (i = 0, f = g_faces; i < NFACES; i++, f++) {
    if (f->cycle == NULL) {
      /* Discard failure, we will report a different one. */
      if (f->edges[color].to == NULL &&
          restrictAndPropogateCycles(f, withoutColor[color], 0) != NULL) {
        return false;
      }
    }
  }
  return true;
}

void initializeDynamicCounters(void)
{
  newStatistic(&cycleGuessCounter, "?", "guesses");
  newStatistic(&cycleForcedCounter, "+", "forced");
  newStatistic(&cycleSetReducedCounter, "-", "reduced");
}
