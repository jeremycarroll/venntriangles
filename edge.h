/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef EDGE_H
#define EDGE_H

#include "color.h"
#include "failure.h"

typedef struct edge* EDGE;
typedef struct curveLink* CURVELINK;
typedef struct Vertex* VERTEX;

struct curveLink {
  /* This CURVELINK is at the end of one edge only.
     To find that edge, if the to field here is not NULL, then:
     it is: next->reversed->to->next->reversed
  */
  EDGE next;
  VERTEX vertex;
};

struct edge {
  /* The label on the edge. */
  MEMO COLOR color;

  /* The label on the face of which this is an edge. */
  MEMO COLORSET colors;
  /* The edge going in the other direction, with the same color, attached to the
   * face with a label differing only by the one color.  */
  MEMO EDGE reversed;
  /*
  to starts off as NULL, and is set to the jth member of possiblyTo
  where (color, j) is in the facial cycle of inner, equivalently
  (j, color) is in the facial cycle of outer.
  */
  DYNAMIC CURVELINK to;
  /* This vertex at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th entry
  being all NULL.
   */
  MEMO struct curveLink possiblyTo[NCOLORS];
};

extern uint64 EdgeColorCountState[2][NCOLORS];
extern COLORSET ColorCompletedState;

extern EDGE edgeFollowBackwards(EDGE edge);
extern FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth);

/**
 * @brief Find the path length between two edges and store the path.
 * @return int The number of edges in the path.
 */
extern int edgePathLength(EDGE from, EDGE to, EDGE* pathReturn);

/**
 * @brief Find the path length between two edges and store the path using the
 * trail.
 * @return int The number of edges in the path.
 */
extern int dynamicEdgePathAndLength(EDGE from, EDGE to, EDGE* pathReturn);

extern int edgePathLengthOnly(EDGE from, EDGE to);

extern FAILURE dynamicEdgeCheckCrossingLimit(COLOR a, COLOR b, int depth);

#define IS_CLOCKWISE_EDGE(edge) \
  (COLORSET_HAS_MEMBER((edge)->color, (edge)->colors))

#endif  // EDGE_H
