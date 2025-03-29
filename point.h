#ifndef POINT_H
#define POINT_H

#include "edge.h"

/* We create all possible points during initialization.

   The point is being used if incomingEdges[i]->to is non-null
   for each i.
    a) all 4 of them or none of them are
    b) incomingEdges[i]->to->point = this point
    c) incomingEdges[i]->to->next->reversed is the other incomingEdge of the
   same color d) incomingEdges[i]->to->next->reversed is an incomingEdge of
   the other color e) incomingEdges[i]->to->next->reversed->to->point is this
   point f) incomingEdges[i]->to->next->reversed->to->next->reversed =
   incomingEdges[i]

   The point is between the crossing of two curves, one colored A
   and the other colored B, A and B used in the comments below.

   The curve colored primary crosses from inside the curve colored secondary to
   outside it. The curve colored secondary crosses from outside the curve
   colored primary to inside it.
 */
struct undirectedPoint {
  /*
  If the point is between crossing of curve A and curve B,
  and A is primary, and B is secondary.
  then incomingEdges[0] is of color A and runs into the point
  incomingEdges[1] is the counterclockwise edge of color A that runs into the
  point incomingEdges[2] is of color B and runs into the point incomingEdges[3]
  is the counterclockwise edge of color B that runs into the point

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
extern char* uPointToStr(char* dbuffer, UPOINT up);
extern UPOINT getPoint(COLORSET colorsOfFace, COLOR primary, COLOR secondary);
extern struct undirectedPoint DynamicPointAllUPoints[NPOINTS];

#endif  // POINT_H
