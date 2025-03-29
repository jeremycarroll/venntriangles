#ifndef EDGE_H
#define EDGE_H

#include "color.h"
#include "failure.h"

typedef struct edge* EDGE;
typedef struct curveLink* CURVELINK;
typedef struct Point* POINT;

struct curveLink {
  /* This CURVELINK is at the end of one edge only.
     To find that edge, if the to field here is not NULL, then:
     it is: next->reversed->to->next->reversed
  */
  EDGE next;
  POINT point;
};

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
  DYNAMIC CURVELINK to;
  /* A value between 0 and NCOLORS, being the cardinality of face. */
  STATIC uint64_t level;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th entry
  being all NULL.
   */
  STATIC struct curveLink possiblyTo[NCOLORS];
};

extern uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];

extern EDGE edgeFollowBackwards(EDGE edge);
extern EDGE edgeFollowForwards(EDGE edge);
extern FAILURE edgeCurveChecks(EDGE edge, int depth);
extern void edgeFindCorners(COLOR a, EDGE result[3][2]);
extern int edgePathLength(EDGE from, EDGE to);
extern char* edgeToStr(EDGE edge);

extern EDGE edgeOnCentralFace(COLOR a);
extern void resetEdges(void);

extern FAILURE edgeCheckCrossingLimit(COLOR a, COLOR b, int depth);
extern void edgeLink(EDGE edge1, EDGE edge2, EDGE edge3, EDGE edge4);

#define IS_PRIMARY_EDGE(edge) \
  (COLORSET_HAS_MEMBER((edge)->color, (edge)->colors))

#endif  // EDGE_H
