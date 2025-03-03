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

void setupCentralFaces(uint32_t aLength, uint32_t bLength, uint32_t cLength,
                       uint32_t dLength, uint32_t eLength, uint32_t fLength)
{
  CYCLE cycle;
  FACE centralFace = g_faces + (NFACES - 1);
  setCycleLength(~(1 << 0), aLength);
  setCycleLength(~(1 << 1), bLength);
  setCycleLength(~(1 << 2), cLength);
  setCycleLength(~(1 << 3), dLength);
  setCycleLength(~(1 << 4), eLength);
  setCycleLength(~(1 << 5), fLength);
  for (cycle = g_cycles;; cycle++) {
    if (cycle->length == 6 && cycle->curves[1] == 1 && cycle->curves[2] == 2 &&
        cycle->curves[3] == 3 && cycle->curves[4] == 4) {
      centralFace->cycle = cycle;
      break;
    }
  }
  makeChoice(centralFace);
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
    setDynamicInt(&face->possibleCycles[i],
                  face->possibleCycles[i] & cycleSet[i]);
    newCycleSetSize -= __builtin_popcountll(toBeCleared);
  }
  if (newCycleSetSize < face->cycleSetSize) {
    setDynamicInt(&face->cycleSetSize, newCycleSetSize);
  }
}

static FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                          int depth)
{
  // check for no-op.
  if (face->cycleSetSize == 1 && face->cycle != NULL) {
    return NULL;
  }

  // carefully updated face->possibleCycles to be anded with cycleSet, on the
  // trail. decreasing the count as we go.
  restrictCycles(face, onlyCycleSet);

  if (face->cycleSetSize == 0) {
    return noMatchingCyclesFailure(face->colors, depth);
  }
  if (face->cycleSetSize == 1) {
    setDynamicPointer(&face->cycle, findFirstCycleInSet(face->possibleCycles));
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
  failure = restrictAndPropogateCycles(
      abFace, face->cycle->sameDirection[index], depth);
  if (failure != NULL) {
    return failure;
  }
  return restrictAndPropogateCycles(
      aFace, face->cycle->oppositeDirection[index], depth);
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
  FAILURE singleFailure;
  FAILURE multipleFailures = NULL;
  /* equality in the followign assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */
  assert(depth <= NFACES);
  for (i = 0; i < cycle->length - 1; i++) {
    /* assignPoint is cheap so collect multiple failures to improve
     * backtracking. */
    singleFailure =
        assignPoint(face, cycle->curves[i], cycle->curves[i + 1], depth);
    multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
  }
  singleFailure = assignPoint(face, cycle->curves[i], cycle->curves[0], depth);
  multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
  if (multipleFailures != NULL) {
    return multipleFailures;
  }

  for (i = 0; i < cycle->length; i++) {
    singleFailure = curveChecks(&face->edges[cycle->curves[i]], depth);
    multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
  }
  if (multipleFailures != NULL) {
    return multipleFailures;
  }
  for (i = 0; i < cycle->length - 1; i++) {
    /*
       propogateChoice is expensive so abort on first failure.
       Also, propogateChoice recurses into this function, and each failure can
       only be used once on any stack, so we would need some generic failure
       collection mechanism.
    */
    FAILURE failure =
        propogateChoice(face, &face->edges[cycle->curves[i]], depth);
    if (failure != NULL) {
      return failure;
    }
  }
  return NULL;
}

FAILURE makeChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  completedColors = 0;
  face->backtrack = trail;
  failure = makeChoiceInternal(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (completedColors) {
    for (completedColor = 0; completedColor < NCURVES; completedColor++) {
      if (memberOfColorSet(completedColor, completedColors)) {
        if (!removeColorFromSearch(completedColor)) {
          return disconnectedCurveFailure(completedColor, true, 0);
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
      if (restrictAndPropogateCycles(f, withoutColor[color], 0) != NULL) {
        return false;
      }
    }
  }
  return true;
}
