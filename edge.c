#include "edge.h"

#define MAX_ONE_WAY_CURVE_CROSSINGS 3
#define MAX_CORNERS 3

uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
uint64_t EdgeCurvesComplete[NCOLORS];

EDGE edgeFollowForwards(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  return edge->to->out[0];
}

EDGE edgeFollowBackwards(EDGE edge)
{
  EDGE reversedNext = edgeFollowForwards(edge->reversed);
  return reversedNext == NULL ? NULL : reversedNext->reversed;
}

int edgePathLength(EDGE from, EDGE to)
{
  int i = 1;
  for (; from != to; i++) {
    from = edgeFollowForwards(from);
    assert(from != NULL);
  }
  return i;
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

FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth)
{
  FAILURE failure;
  if (EdgeCurvesComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  CHECK_FAILURE(checkForDisconnectedCurve(start, depth));
  return dynamicEdgeCornerCheck(start, depth);
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
