/* These are the operations that change the dynamic fields in the data structure.
  initialize() must alreayd have been called before any of these methods are called.
*/

#include "venn.h"
static FAILURE makeChoiceInternal(FACE face, int depth);

void setupCentralFaces(uint32_t aLength, uint32_t bLength, uint32_t cLength, uint32_t dLength, uint32_t eLength, uint32_t fLength)
{
    CYCLE cycle;
    FACE centralFace = g_faces + (NFACES - 1);
    setCycleLength(~(1 << 0), aLength);
    setCycleLength(~(1 << 1), bLength);
    setCycleLength(~(1 << 2), cLength);
    setCycleLength(~(1 << 3), dLength);
    setCycleLength(~(1 << 4), eLength);
    setCycleLength(~(1 << 5), fLength);
    for (cycle = g_cycles;; cycle++)
    {
        if (cycle->length == 6 && cycle->curves[1] == 1 && cycle->curves[2] == 2 && cycle->curves[3] == 3 && cycle->curves[4] == 4)
        {
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

    for (i = 0; i < CYCLESET_LENGTH; i++)
    {
        toBeCleared = face->possibleCycles[i] & ~cycleSet[i];
        if (toBeCleared == 0)
        {
            continue;
        }
        setDynamicInt(&face->possibleCycles[i], face->possibleCycles[i] & cycleSet[i]);
        newCycleSetSize -= __builtin_popcountll(toBeCleared);
    }
    if (newCycleSetSize < face->cycleSetSize)
    {
        setDynamicInt(&face->cycleSetSize, newCycleSetSize);
    }
}

static FAILURE restrictAndPropogateCycles(FACE face, CYCLESET cycleSet, int depth)
{
    // carefully updated face->possibleCycles to be anded with cycleSet, on the trail.
    // decreasing the count as we go.
    restrictCycles(face, cycleSet);

    if (face->cycleSetSize == 0)
    {
        return noMatchingCyclesFailure(face->colors, depth);
    }
    if (face->cycleSetSize == 1)
    {
        setDynamicPointer(&face->cycle, findFirstCycleInSet(face->possibleCycles));
        return makeChoiceInternal(face, depth + 1);
    }
    return NULL;
}

/*
Either return the point, or return NULL and set the value of failureReturn.
*/
static POINT getOrCreatePointOnFace(FACE face, COLOR aColor, COLOR bColor, int depth, FAILURE *failureReturn)
{
    EDGE aEdgeIn = face->edges[aColor];
    EDGE bEdgeOut = face->edges[bColor];
    FACE aFace = face->adjacentFaces[aColor];
    FACE bFace = face->adjacentFaces[bColor];
    FACE abFace = aFace->adjacentFaces[bColor];
    assert(abFace == bFace->adjacentFaces[aColor]);
    EDGE aEdgeOut = abFace->edges[aColor];
    EDGE bEdgeIn = abFace->edges[bColor];
    if (aEdgeIn->to != NULL)
    {
        assert(aEdgeIn->to == bEdgeOut->from);
        assert(aEdgeIn->to == aEdgeOut->from);
        assert(aEdgeIn->to == bEdgeIn->from);
        return aEdgeIn->to;
    }
    return createPoint(aEdgeIn, aEdgeOut, bEdgeIn, bEdgeOut, depth, failureReturn);
}

static FAILURE propogateChoice(FACE face, POINT point, int depth)
{
    FAILURE failure;
    COLOR aColor = point->edges[0]->color;
    COLOR bColor = point->edges[2]->color;

    FACE aFace = face->adjacentFaces[aColor];
    FACE bFace = face->adjacentFaces[bColor];
    FACE abFace = aFace->adjacentFaces[bColor];
    assert(abFace == bFace->adjacentFaces[aColor]);
    failure = restrictAndPropogateCycles(abFace, face->cycle->sameDirection[aColor], depth);
    if (failure != NULL)
    {
        return failure;
    }
    failure = restrictAndPropogateCycles(aFace, face->cycle->oppositeDirection[aColor], depth);
    if (failure != NULL)
    {
        return failure;
    }
    return restrictAndPropogateCycles(bFace, face->cycle->oppositeDirection[bColor], depth);
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
- if any of the adjacent faces now have one possible cycle, set the cycle on that face, and make that choice
  (if that choice fails with a false, then so do we)
*/
static FAILURE makeChoiceInternal(FACE face, int depth)
{
    uint32_t i;
    CYCLE cycle = face->cycle;
    POINT points[NCURVES];
    FAILURE singleFailure;
    FAILURE multipleFailures = NULL;
    for (i = 0; i < cycle->length - 1; i++)
    {
        // getOrCreatePointOnFace is cheap so collect multiple failures to improve backtracking.
        singleFailure = NULL;
        points[i] = getOrCreatePointOnFace(face, cycle->curves[i], cycle->curves[i + 1], depth, &singleFailure);
        multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
    }
    points[i] = getOrCreatePointOnFace(face, cycle->curves[i], cycle->curves[0], depth, &singleFailure);
    multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
    if (multipleFailures != NULL)
    {
        return multipleFailures;
    }

    for (i = 0; i < cycle->length; i++)
    {
        singleFailure = curveChecks(face->edges[cycle->curves[i]]);
        multipleFailures = maybeAddFailure(multipleFailures, singleFailure, depth);
    }
    if (multipleFailures != NULL)
    {
        return multipleFailures;
    }
    for (i = 0; i < cycle->length - 1; i++)
    {
        /*
           propogateChoice is expensive so abort on first failure.
           Also, propogateChoice recurses into this function, and each failure can only be used
           once on any stack, so we would need some generic failure collection mechanism.
        */
        FAILURE failure = propogateChoice(face, points[i], depth);
        if (failure != NULL)
        {
            return failure;
        }
    }
    return NULL;
}

FAILURE makeChoice(FACE face)
{
    return makeChoiceInternal(face, 0);
}