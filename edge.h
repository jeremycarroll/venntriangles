/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef EDGE_H
#define EDGE_H

#include "color.h"
#include "failure.h"
#include "trail.h"

/**
 * Edge system for representing curve edges in Venn diagrams.
 * Provides data structures and functions for managing the edges that
 * form the boundaries between faces in the diagram.
 */

/*--------------------------------------
 * Type Definitions and Structures
 *--------------------------------------*/

/* Forward declarations */
typedef struct edge* EDGE;
typedef struct curveLink* CURVELINK;
typedef struct Vertex* VERTEX;

/**
 * Connection between an edge and a vertex.
 * Used to describe how edges connect to vertices in the diagram.
 */
struct curveLink {
  /* This CURVELINK is at the end of one edge only.
     To find that edge, if the to field here is not NULL, then:
     it is: next->reversed->to->next->reversed
  */
  EDGE next;     /* Next edge in the sequence */
  VERTEX vertex; /* Associated vertex */
};

/**
 * Core edge structure representing a segment of a curve.
 * Each edge represents part of the boundary of a face in the diagram.
 */
struct edge {
  /* The label on the edge. */
  MEMO COLOR color;

  /* The label on the face of which this is an edge. */
  MEMO COLORSET colors;

  /* The edge going in the other direction, with the same color, attached to the
   * face with a label differing only by the one color.  */
  MEMO EDGE reversed;

  /*
   * to starts off as NULL, and is set to the jth member of possiblyTo
   * where (color, j) is in the facial cycle of inner, equivalently
   * (j, color) is in the facial cycle of outer.
   */
  DYNAMIC CURVELINK to;

  /* This vertex at the end of this edge may cross one of the other colors.
   * We have all 5 pre-initialized in this array, with the color-th entry
   * being all NULL.
   */
  MEMO struct curveLink possiblyTo[NCOLORS];

  /* Both this edge and its reverse point to the same uint_trail value,
   * which is initialized as UNSET_LINE_ID (2^64 - 1; all 1s; -1; ~0).
   * When we know which line it is on, we set this.
   * lineId is one of 3*color, 3*color+1, 3*color+2.
   * For edges on the central face lineId is 3*color.
   */
  uint_trail* lineId;
};

/*--------------------------------------
 * Global Variables
 *--------------------------------------*/

/**
 * Tracks the count of edges for each color in the diagram.
 * First dimension: 0=edges with no vertex, 1=edges with vertex
 * Second dimension: color index
 */
extern uint64 EdgeColorCountState[2][NCOLORS];

/**
 * Bit set tracking which colors have all their edges completed.
 */
extern COLORSET ColorCompletedState;

/*--------------------------------------
 * Edge Navigation Functions
 *--------------------------------------*/

/**
 * Follows an edge in the backwards direction.
 * @param edge The edge to follow backwards
 * @return The connected edge in the backward direction
 */
extern EDGE edgeFollowBackwards(EDGE edge);

/**
 * Performs validation checks on a curve segment.
 * @param edge The edge to check
 * @param depth Current search depth
 * @return Failure object if checks fail, NULL otherwise
 */
extern FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth);

/**
 * Find the path length between two edges and store the path.
 * @param from Starting edge
 * @param to Ending edge
 * @param pathReturn Array to store the path
 * @return The number of edges in the path
 */
extern int edgePathLength(EDGE from, EDGE to, EDGE* pathReturn);

/**
 * Find the path between two edges with trail support.
 * @param from Starting edge
 * @param to Ending edge
 * @param pathReturn Array to store the path
 * @return The number of edges in the path
 */
extern int dynamicEdgePathAndLength(EDGE from, EDGE to, EDGE* pathReturn);

/**
 * Find only the path length between two edges.
 * @param from Starting edge
 * @param to Ending edge
 * @return The number of edges in the path
 */
extern int edgePathLengthOnly(EDGE from, EDGE to);

/**
 * Checks if curve crossing limits are exceeded.
 * @param a First color
 * @param b Second color
 * @param depth Current search depth
 * @return Failure object if limits exceeded, NULL otherwise
 */
extern FAILURE dynamicEdgeCheckCrossingLimit(COLOR a, COLOR b, int depth);

/**
 * Determines if an edge is oriented clockwise around its face.
 * An edge is clockwise if its color is part of the face's colorset.
 */
#define IS_CLOCKWISE_EDGE(edge) \
  (COLORSET_HAS_MEMBER((edge)->color, (edge)->colors))

#endif  // EDGE_H
