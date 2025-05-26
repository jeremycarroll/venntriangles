/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef VERTEX_H
#define VERTEX_H

#include "edge.h"
#include "trail.h"

/* We create all possible vertices during initialization.

   The vertex is being used if incomingEdges[i]->to is non-null
   for each i.
    a) all 4 of them or none of them are
    b) incomingEdges[i]->to->vertex = this vertex
    c) incomingEdges[i]->to->next->reversed is the other incomingEdge of the
   same color d) incomingEdges[i]->to->next->reversed is an incomingEdge of
   the other color e) incomingEdges[i]->to->next->reversed->to->vertex is this
   vertex f) incomingEdges[i]->to->next->reversed->to->next->reversed =
   incomingEdges[i]

   The vertex is between the crossing of two curves, one colored A
   and the other colored B, A and B used in the comments below.

   The curve colored primary crosses from inside the curve colored secondary to
   outside it. The curve colored secondary crosses from outside the curve
   colored primary to inside it.
 */
struct Vertex {
  /*
  If the vertex is between crossing of curve A and curve B,
  and A is primary, and B is secondary.
  then incomingEdges[0] is of color A and runs into the vertex
  incomingEdges[1] is the counterclockwise edge of color A that runs into the
  vertex incomingEdges[2] is of color B and runs into the vertex
  incomingEdges[3] is the counterclockwise edge of color B that runs into the
  vertex

  The outgoing edges are found by reversing the incoming edges
  */
  MEMO EDGE incomingEdges[4];

  /* Colors can also be accessed from the edges as above.
   */
  MEMO COLOR primary, secondary;

  /* A set of the two colors. */
  MEMO COLORSET colors;

  /* The first of the two lines at this vertex.
     lines count from 1; 0 means not yet set.
   */
  uint_trail lineId;
};

extern void initializePoints(void);
extern VERTEX initializeVertexIncomingEdge(COLORSET colorsOfFace,
                                           EDGE incomingEdge, COLOR othercolor);
extern char* vertexToString(VERTEX up);
extern char* vertexToColorSetString(VERTEX up);
extern FAILURE dynamicEdgeCornerCheck(EDGE start, int depth);
extern void edgeFindAndAlignCorners(COLOR a, EDGE result[3][2]);

struct face {
  // cycle must be null if cycleSetSize is not 1.
  DYNAMIC struct facialCycle* cycle;
  /* We vertex to previous and next with the same number of colors. */
  DYNAMIC FACE previous;
  DYNAMIC FACE next;
  MEMO COLORSET colors;             // holds up to NFACES
  DYNAMIC uint_trail cycleSetSize;  // holds up to NCYCLES
  DYNAMIC CYCLESET_DECLARE possibleCycles;
  MEMO struct face* adjacentFaces[NCOLORS];
  MEMO struct edge edges[NCOLORS];
  /* Precomputed lookup tables for previous and next. */
  MEMO FACE previousByCycleId[NCYCLES];
  MEMO FACE nextByCycleId[NCYCLES];
};

extern MEMO struct face Faces[NFACES];

#endif  // VERTEX_H
