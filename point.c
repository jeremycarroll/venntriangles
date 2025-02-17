
#include "venn.h"
/*
    The point is between the crossing of two curves, one colored A
    and the other colored B, A and B used in the comments below.

    The curve colored A crosses from inside the curve colored B to outside it.
    The curve colored B crosses from outside the curve colored A to inside it.
  */

/* Face A&B
 */
FACE point2insideFace(POINT point) { return point->faces[3]; }

/* Face ~(A&B)
 */
FACE point2outsideFace(POINT point) { return point->faces[0]; }

/*
Face (~A)&B

Both curves are going clockwise around the central face
of the FISC. Hence, both curves are going clockwise
around  point2insideFace(point). The face
that is outside A and inside B is the face that is clockwise
before the point.
*/
FACE point2incomingFace(POINT point) { return point->faces[2]; }

/*
Face A&~B
*/
FACE point2outgoingFace(POINT point) { return point->faces[1]; }

EDGE point2inside2outsideIncomingEdge(POINT point) { return point->edges[0]; }

EDGE point2outside2insideIncomingEdge(POINT point) { return point->edges[2]; }

EDGE point2inside2outsideOutgoingEdge(POINT point) { return point->edges[1]; }

EDGE point2outside2insideOutgoingEdge(POINT point) { return point->edges[3]; }

COLOR point2inside2outsideColor(POINT point) { return point->edges[0]->color; }

COLOR point2outside2insideColor(POINT point) { return point->edges[2]->color; }

EDGE followEdge(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  if (edge->color == point2inside2outsideColor(edge->to)) {
    return point2inside2outsideOutgoingEdge(edge->to);
  }
  assert(edge->color == point2outside2insideColor(edge->to));
  return point2outside2insideOutgoingEdge(edge->to);
}

/*
The curve A crosses from inside the curve B to outside it.
The curve B crosses from outside the curve A to inside it.

Either return the point, or return NULL and set the value of failureReturn.
*/
static POINT createPointOrdered(EDGE aEdgeIn, EDGE aEdgeOut, EDGE bEdgeIn,
                                EDGE bEdgeOut, int depth,
                                FAILURE *failureReturn)
{
  POINT point = g_points + g_nextPoint;
  FAILURE crossingLimit =
      checkCrossingLimit(aEdgeIn->color, bEdgeIn->color, depth);
  if (crossingLimit != NULL) {
    *failureReturn = crossingLimit;
    return NULL;
  }
  setDynamicInt(&g_nextPoint, g_nextPoint + 1);
  // not trailed, the point is on a stack.

  // Edges: A before B, in before out:
  point->edges[0] = aEdgeIn;
  point->edges[1] = aEdgeOut;
  point->edges[2] = bEdgeIn;
  point->edges[3] = bEdgeOut;
  // Faces: ~(AB), A, B, AB
  point->faces[0] = aEdgeOut->outer;
  point->faces[1] = aEdgeOut->inner;
  point->faces[2] = bEdgeIn->inner;
  point->faces[3] = aEdgeIn->inner;
  assert(point->faces[0] == bEdgeIn->outer);
  assert(point->faces[1] == bEdgeOut->outer);
  assert(point->faces[2] == bEdgeOut->inner);
  assert(point->faces[3] == bEdgeOut->inner);
  setDynamicPointer(&aEdgeIn->to, point);
  setDynamicPointer(&aEdgeOut->from, point);
  setDynamicPointer(&bEdgeIn->to, point);
  setDynamicPointer(&bEdgeOut->from, point);

  return point;
}

POINT createPoint(EDGE aEdgeIn, EDGE aEdgeOut, EDGE bEdgeIn, EDGE bEdgeOut,
                  int depth, FAILURE *failureReturn)
{
  assert(aEdgeIn->to == NULL);
  setDynamicInt(g_edgeCount + aEdgeIn->color, g_edgeCount[aEdgeIn->color] + 1);
  assert(bEdgeIn->to == NULL);
  setDynamicInt(g_edgeCount + bEdgeIn->color, g_edgeCount[bEdgeIn->color] + 1);
  if (memberOfColorSet(bEdgeIn->color, aEdgeIn->inner->colors)) {
    return createPointOrdered(aEdgeIn, aEdgeOut, bEdgeIn, bEdgeOut, depth,
                              failureReturn);
  } else {
    return createPointOrdered(bEdgeIn, bEdgeOut, aEdgeIn, aEdgeOut, depth,
                              failureReturn);
  }
}
