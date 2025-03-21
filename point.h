#ifndef POINT_H
#define POINT_H

#include "color.h"

typedef struct directedPoint *DPOINT;
typedef struct undirectedPoint *UPOINT;

struct directedPoint {
  /* This DPOINT is at the end of one edge only.
     To find that edge, if the to field here is not NULL, then:
     it is: out[0]->reversed->to->out[0]->reversed
  */
  EDGE out[2];
  UPOINT point;
};

/* We create all possible points during initialization.

   The point is being used if incomingEdges[i]->to is non-null
   for each i.
    a) all 4 of them or none of them are
    b) incomingEdges[i]->to->point = this point
    c) incomingEdges[i]->to->out[0]->reversed is the other incomingEdge of the
   same color d) incomingEdges[i]->to->out[1]->reversed is an incomingEdge of
   the other color e) incomingEdges[i]->to->out[j]->reversed->to->point is this
   point f) incomingEdges[i]->to->out[j]->reversed->to->out[j]->reversed =
   incomingEdges[i]

   The point is between the crossing of two curves, one colored A
   and the other colored B, A and B used in the comments below.

   The curve colored primary crosses from inside the curve colored secondary to
   outside it. The curve colored secondary crosses from outside the curve
   colored primary to inside it.
 */
struct undirectedPoint {
  /*
  TODO: logically redundant, is this helpful?
  If the point is between crossing of curve A and curve B,
  then face[0] is inside neither curve
  face[1] is inside A but not B
  face[2] is inside B but not A
  face[3] is inside both A and B
  */
  STATIC FACE faces[4];
  /*
  If the point is between crossing of curve A and curve B,
  and color(A) < color(B),
  then edge[0] is of color A and runs into the point
  edge[1] is the counterclockwise edge of color A that runs into the point
  edge[2] is of color B and runs into the point
  edge[3] is the counterclockwise edge of color B that runs into the point

  The outgoing edges are found by reversing the incoming edges
  */
  STATIC EDGE incomingEdges[4];

  /* A value between 0 and NPOINTS */
  STATIC uint32_t id;

  /* Colors can also be accessed from the edges as above.
   */
  STATIC COLOR primary, secondary;

  /* A set of the two colors. */
  STATIC COLORSET colors;
};

extern void initializePoints(void);
extern void resetPoints(void);
extern UPOINT dynamicPointAdd(FACE face, EDGE incomingEdge, COLOR othercolor);
extern FAILURE dynamicEdgeCheckCrossingLimit(COLOR a, COLOR b, int depth);

#endif  // POINT_H
