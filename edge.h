#ifndef EDGE_H
#define EDGE_H

#include <stdint.h>

#include "color.h"
#include "core.h"
#include "failure.h"
#include "point.h"
#include "trail.h"

typedef struct edge *EDGE;

struct edge {
  /* The label on the edge. */
  STATIC COLOR color;

  /* The label on the face of which this is an edge. */
  STATIC COLORSET colors;
  /* The edge going in the other direction, with the same color, attached to the
   * face with a label differing only by the one color.  */
  STATIC EDGE reversed;
  /*
  to starts off as NULL, and is set to the jth member of possiblyTo
  where (color, j) is in the facial cycle of inner, equivalently
  (j, color) is in the facial cycle of outer.
  */
  DYNAMIC DPOINT to;
  /* A value between 0 and NCOLORS, being the cardinality of face. */
  STATIC uint64_t level;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th entry
  being all NULL.
   */
  STATIC struct directedPoint possiblyTo[NCOLORS];
};

extern EDGE dynamicEdgeFollowBackwards(EDGE edge);
extern EDGE dynamicEdgeFollowForwards(EDGE edge);
extern FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth);
extern void dynamicEdgeFindCorners(COLOR a, EDGE result[3][2]);
extern int dynamicEdgePathLength(EDGE from, EDGE to);

#define IS_PRIMARY_EDGE(edge) (memberOfColorSet((edge)->color, (edge)->colors))

#endif  // EDGE_H
