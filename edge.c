/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "edge.h"

#include "trail.h"

#include <stdlib.h>

/* Global variables - globally scoped */
uint64 EdgeCountsByDirectionAndColor[2][NCOLORS];
COLORSET ColorCompleted;

/* Global variables - file scoped */
#define MAX_ONE_WAY_CURVE_CROSSINGS 3
#define MAX_CORNERS 3
static uint64 EdgeCrossingCounts[NCOLORS][NCOLORS];
static uint64 EdgeCurvesComplete[NCOLORS];

/*
This file is responsible for checking that a set of edges can make a triangle,
and for outputting solutions.
*/

/* Declaration of file scoped static functions */
static uint_trail curveLength(EDGE edge);
static FAILURE checkForDisconnectedCurve(EDGE edge, int depth);
static EDGE findStartOfCurve(EDGE edge);
static EDGE edgeFollowForwards(EDGE edge);

/* Externally linked functions - reset... */
void resetEdges()
{
  memset(EdgeCountsByDirectionAndColor, 0,
         sizeof(EdgeCountsByDirectionAndColor));
  memset(EdgeCrossingCounts, 0, sizeof(EdgeCrossingCounts));
  memset(EdgeCurvesComplete, 0, sizeof(EdgeCurvesComplete));
}

/* Externally linked functions - edge... */
FAILURE edgeCheckCrossingLimit(COLOR a, COLOR b, int depth)
{
  uint_trail* crossing = &EdgeCrossingCounts[a][b];
  if (*crossing + 1 > MAX_ONE_WAY_CURVE_CROSSINGS) {
    return failureCrossingLimit(depth);
  }
  trailSetInt(crossing, *crossing + 1);
  return NULL;
}

FAILURE edgeCurveChecks(EDGE edge, int depth)
{
  if (EdgeCurvesComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  return checkForDisconnectedCurve(start, depth);
}

EDGE edgeFollowBackwards(EDGE edge)
{
  EDGE reversedNext = edgeFollowForwards(edge->reversed);
  return reversedNext == NULL ? NULL : reversedNext->reversed;
}

EDGE edgeFollowForwards(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  return edge->to->next;
}

void edgeLink(EDGE edge1, EDGE edge2, EDGE edge3, EDGE edge4)
{
  COLOR other = edge3->color;
  uint32_t level1 = edge1->level;
  uint32_t level2 = edge2->level;
  uint32_t level3 = edge3->reversed->level;
  uint32_t level4 = edge4->reversed->level;

  assert(edge1->color == edge2->color);
  assert(edge1->possiblyTo[other].next == NULL);
  assert(edge2->possiblyTo[other].next == NULL);
  assert(edge1->possiblyTo[other].vertex == edge2->possiblyTo[other].vertex);
  edge1->possiblyTo[other].next = edge2->reversed;
  edge2->possiblyTo[other].next = edge1->reversed;
  if (level1 == level3) {
    assert(level2 == level4);
  } else {
    assert(level1 == level4);
    assert(level2 == level3);
  }
}

int edgePathLength(EDGE from, EDGE to, EDGE* pathReturn)
{
  EDGE dummyReturn[NFACES];
  int i = 0;
  if (pathReturn == NULL) {
    pathReturn = dummyReturn;
  }
  pathReturn[i++] = from;
  while (from != to) {
    from = edgeFollowForwards(from);
    assert(from != to->reversed);
    assert(from != NULL);
    assert(i < NFACES);
    pathReturn[i++] = from;
  }
  return i;
}

char* edgeToString(EDGE edge)
{
  char* buffer = getBuffer();
  if (edge == NULL) {
    strcpy(buffer, "NULL");
  } else {
    sprintf(buffer, "%c%d", colorToChar(edge->color), edge->colors);
  }
  return usingBuffer(buffer);
}

/* File scoped static functions */
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
