/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "edge.h"

#include "trail.h"

uint64 EdgeColorCountState[2][NCOLORS];
COLORSET ColorCompletedState;

/* If we have convex polygons A and B both with N sides, then they can cross
   each other in at most 2*N different points. For half A crosses outside B. */
#define MAX_ONE_WAY_CURVE_CROSSINGS MAX_CORNERS
static uint64 EdgeCrossingCounts[NCOLORS][NCOLORS];
static uint64 EdgeCurvesComplete[NCOLORS];

static EDGE edgeFollowForwards(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  return edge->to->next;
}

static uint_trail curveLength(EDGE edge)
{
  EDGE current;
  int result;
  for (result = 1, current = edgeFollowForwards(edge); current != edge;
       result++, current = edgeFollowForwards(current)) {
    assert(current != NULL);
  }
  return result;
}

static FAILURE dynamicCheckForDisconnectedCurve(EDGE edge, int depth)
{
  uint_trail length;
  if (edge->reversed->to != NULL) {
    // We have a colored cycle in the FISC.
    length = curveLength(edge);
    if (length < EdgeColorCountState[IS_CLOCKWISE_EDGE(edge)][edge->color]) {
      return failureDisconnectedCurve(depth);
    }
    assert(length == EdgeColorCountState[IS_CLOCKWISE_EDGE(edge)][edge->color]);
    if (ColorCompletedState & 1u << edge->color) {
      return NULL;
    }
    ColorCompletedState |= 1u << edge->color;
    trailSetInt(&EdgeCurvesComplete[edge->color], 1);
  }
  return NULL;
}

static EDGE findStartOfCurve(EDGE edge)
{
  EDGE next, current = edge;
  while ((next = edgeFollowBackwards(current)) != edge) {
    if (next == NULL) {
      return current;
    }
    current = next;
  }
  return edge;
}

FAILURE dynamicEdgeCheckCrossingLimit(COLOR a, COLOR b, int depth)
{
  uint_trail* crossing = &EdgeCrossingCounts[a][b];
  if (*crossing + 1 > MAX_ONE_WAY_CURVE_CROSSINGS) {
    return failureCrossingLimit(depth);
  }
  trailSetInt(crossing, *crossing + 1);
  return NULL;
}

FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth)
{
  if (EdgeCurvesComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  return dynamicCheckForDisconnectedCurve(start, depth);
}

EDGE edgeFollowBackwards(EDGE edge)
{
  EDGE reversedNext = edgeFollowForwards(edge->reversed);
  return reversedNext == NULL ? NULL : reversedNext->reversed;
}

static void dynamicSetEdge(EDGE* array, int index, EDGE value)
{
  TRAIL_SET_POINTER(array + index, value);
}

static void setEdge(EDGE* array, int index, EDGE value)
{
  array[index] = value;
}

static void noopSetEdge(EDGE* array, int index, EDGE value)
{
  (void)array;
  (void)index;
  (void)value;
}

static int edgePathAndLength(EDGE from, EDGE to, EDGE* pathReturn,
                             void (*handler)(EDGE*, int, EDGE))
{
  int i = 0;
  handler(pathReturn, i++, from);
  while (from != to) {
    from = edgeFollowForwards(from);
    assert(from != to->reversed);
    assert(from != NULL);
    assert(i < NFACES);
    handler(pathReturn, i++, from);
  }
  handler(pathReturn, i, NULL);
  return i;
}

int edgePathLength(EDGE from, EDGE to, EDGE* pathReturn)
{
  assert(pathReturn != NULL);
  return edgePathAndLength(from, to, pathReturn, setEdge);
}

int dynamicEdgePathAndLength(EDGE from, EDGE to, EDGE* pathReturn)
{
  assert(pathReturn != NULL);
  return edgePathAndLength(from, to, pathReturn, dynamicSetEdge);
}

int edgePathLengthOnly(EDGE from, EDGE to)
{
  return edgePathAndLength(from, to, NULL, noopSetEdge);
}
