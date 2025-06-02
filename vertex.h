/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef VERTEX_H
#define VERTEX_H

#include "cycleset.h"
#include "edge.h"
#include "trail.h"

/**
 * Vertex system for representing crossing points between curves.
 * Manages the intersection points where curves cross in the Venn diagram.
 */

/*--------------------------------------
 * Type Definitions and Structures
 *--------------------------------------*/

/**
 * Represents a vertex at the intersection of two curves.
 *
 * We create all possible vertices during initialization.
 *
 * The vertex is being used if incomingEdges[i]->to is non-null
 * for each i. Invariants when a vertex is used:
 *   a) all 4 of them or none of them are non-null
 *   b) incomingEdges[i]->to->vertex = this vertex
 *   c) incomingEdges[i]->to->next->reversed is the other incomingEdge of the
 * same color d) incomingEdges[i]->to->next->reversed is an incomingEdge of the
 * other color e) incomingEdges[i]->to->next->reversed->to->vertex is this
 * vertex f) incomingEdges[i]->to->next->reversed->to->next->reversed =
 * incomingEdges[i]
 *
 * The vertex is between the crossing of two curves, one colored A
 * and the other colored B, A and B used in the comments below.
 *
 * The curve colored primary crosses from inside the curve colored secondary to
 * outside it. The curve colored secondary crosses from outside the curve
 * colored primary to inside it.
 */
struct Vertex {
  /**
   * The four edges that enter this vertex.
   *
   * If the vertex is between crossing of curve A and curve B,
   * and A is primary, and B is secondary, then:
   *   - incomingEdges[0] is of color A and runs into the vertex
   *   - incomingEdges[1] is the counterclockwise edge of color A that runs into
   * the vertex
   *   - incomingEdges[2] is of color B and runs into the vertex
   *   - incomingEdges[3] is the counterclockwise edge of color B that runs into
   * the vertex
   *
   * The outgoing edges are found by reversing the incoming edges.
   */
  MEMO EDGE incomingEdges[4];

  /* The two colors of the curves that cross at this vertex */
  MEMO COLOR primary, secondary;

  /* A bit set of the two colors. */
  MEMO COLORSET colors;

  /* The first of the two lines at this vertex.
     Lines count from 1; 0 means not yet set.
   */
  uint_trail lineId;
};

/**
 * Face structure representing a region in the Venn diagram.
 *
 * A face is a region enclosed by edges of different colors.
 */
struct face {
  /* cycle must be null if cycleSetSize is not 1. */
  DYNAMIC struct facialCycle* cycle;

  /* Links to previous and next faces with the same number of colors. */
  DYNAMIC FACE previous;
  DYNAMIC FACE next;

  /* The colors that define this face */
  MEMO COLORSET colors; /* holds up to NFACES */

  /* Number of possible cycles for this face */
  DYNAMIC uint_trail cycleSetSize; /* holds up to NCYCLES */

  /* Set of possible cycles for this face */
  DYNAMIC CYCLESET_DECLARE possibleCycles;

  /* Array of adjacent faces, indexed by color */
  MEMO struct face* adjacentFaces[NCOLORS];

  /* Edges that form the boundary of this face */
  MEMO struct edge edges[NCOLORS];

  /* Precomputed lookup tables for previous and next faces by cycle ID. */
  MEMO FACE previousByCycleId[NCYCLES];
  MEMO FACE nextByCycleId[NCYCLES];
};

/* Global array of all faces in the diagram */
extern MEMO struct face Faces[NFACES];

/*--------------------------------------
 * Vertex Initialization and Management
 *--------------------------------------*/

/**
 * Initialize all possible vertices in the diagram.
 */
extern void initializePoints(void);

/**
 * Create a vertex at the specified incoming edge.
 * @param colorsOfFace Colors of the face containing the edge
 * @param incomingEdge The incoming edge
 * @param othercolor The other color crossing at this vertex
 * @return The newly initialized vertex
 */
extern VERTEX initializeVertexIncomingEdge(COLORSET colorsOfFace,
                                           EDGE incomingEdge, COLOR othercolor);

/*--------------------------------------
 * Vertex Utilities
 *--------------------------------------*/

/**
 * Convert a vertex to its string representation.
 * @param up The vertex to convert
 * @return String representation of the vertex
 */
extern char* vertexToString(VERTEX up);

/**
 * Convert a vertex to a string showing its colors.
 * @param up The vertex to convert
 * @return String representation of the vertex's colors
 */
extern char* vertexToColorSetString(VERTEX up);

/**
 * Perform validation checks on a vertex at a corner.
 * @param start The starting edge for the check
 * @param depth Current search depth
 * @return Failure object if checks fail, NULL otherwise
 */
extern FAILURE vertexCornerCheck(EDGE start, int depth);

/**
 * Align corners for a specific color.
 * @param a The color to align corners for
 * @param result Output array to store aligned corner edges
 */
extern void vertexAlignCorners(COLOR a, EDGE result[3][2]);

/**
 * Get the central edge for a specific color.
 * @param a The color to get the central edge for
 * @return The central edge for the specified color
 */
extern EDGE vertexGetCentralEdge(COLOR a);

#endif  // VERTEX_H
