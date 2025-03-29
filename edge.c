#include "edge.h"

#include "trail.h"

#include <stdlib.h>

#define MAX_ONE_WAY_CURVE_CROSSINGS 3
#define MAX_CORNERS 3

#define MAX_ONE_WAY_CURVE_CROSSINGS 3

/* Edge-related variables */
static uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
static uint64_t EdgeCurvesComplete[NCOLORS];
uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
COLORSET ColorCompleted;

/*
This file is responsible for checking that a set of edges can make a triangle,
and for outputting solutions.
*/

/*
This file is responsible for checking that a set of edges can make a triangle.
*/
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

static FAILURE checkForDisconnectedCurve(EDGE edge, int depth)
{
  uint_trail length;
  // TODO: needs named macro!
  if (edge->reversed->to != NULL) {
    // We have a colored cycle in the FISC.
    length = curveLength(edge);
    if (length <
        EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]) {
      return failureDisconnectedCurve(depth);
    }
    assert(length ==
           EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]);
    if (ColorCompleted & 1u << edge->color) {
      return NULL;
    }
    ColorCompleted |= 1u << edge->color;
    trailSetInt(&EdgeCurvesComplete[edge->color], 1);
  }
  return NULL;
}

EDGE edgeFollowForwards(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  return edge->to->next;
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
  if (EdgeCurvesComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  return checkForDisconnectedCurve(start, depth);
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

void resetEdges()
{
  memset(EdgeCountsByDirectionAndColor, 0,
         sizeof(EdgeCountsByDirectionAndColor));
  memset(EdgeCrossingCounts, 0, sizeof(EdgeCrossingCounts));
  memset(EdgeCurvesComplete, 0, sizeof(EdgeCurvesComplete));
}

char* edgeToStr(char* dbuffer, EDGE edge)
{
  if (edge == NULL) {
    strcpy(dbuffer, "NULL");
  } else {
    sprintf(dbuffer, "%c%d", colorToChar(edge->color), edge->colors);
  }
  return dbuffer;
}
