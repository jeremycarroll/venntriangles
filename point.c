
#include "venn.h"
/* 
    The point is between the crossing of two curves, one colored A
    and the other colored B, A and B used in the comments below.

    The curve colored A crosses from inside the curve colored B to outside it.
    The curve colored B crosses from outside the curve colored A to inside it.
  */

  /* Face A&B
  */
FACE point2insideFace(POINT point)
{
    return point->faces[3];
}

/* Face ~(A&B)
*/
FACE point2outsideFace(POINT point)
{
    return point->faces[0];
}

/*
Face (~A)&B

Both curves are going clockwise around the central face
of the FISC. Hence, both curves are going clockwise
around  point2insideFace(point). The face
that is outside A and inside B is the face that is clockwise
before the point.
*/
FACE point2incomingFace(POINT point)
{
    return point->faces[2];
}

/*
Face A&~B
*/
FACE point2outgoingFace(POINT point)
{
    return point->faces[1];
}

EDGE point2inside2outsideIncomingEdge(POINT point)
{
    return point->edges[0];
}

EDGE point2outside2insideIncomingEdge(POINT point)
{
    return point->edges[2];
}

EDGE point2inside2outsideOutgoingEdge(POINT point)
{
    return point->edges[1];
}

EDGE point2outside2insideOutgoingEdge(POINT point)
{
    return point->edges[3];
}

COLOR point2inside2outsideColor(POINT point)
{
    return point->edges[0]->color;
}

COLOR point2outside2insideColor(POINT point)
{
    return point->edges[2]->color;
}

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
