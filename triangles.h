/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "edge.h"

/**
 * Triangle handling functions for Venn diagram traversal and verification.
 * Provides a callback-based API for triangle perimeter traversal.
 */

/**
 * Structure to hold callbacks for triangle traversal.
 * Each function is called for a specific edge configuration.
 */
typedef struct {
  /* Callback for processing an edge with no corners */
  void (*processRegularEdge)(void *data, EDGE current, int line);

  /* Callback for processing an edge with one corner */
  void (*processSingleCorner)(void *data, EDGE current, int line);

  /* Callback for processing an edge with two adjacent corners */
  void (*processAdjacentCorners)(void *data, EDGE current, int line);

  /* Callback for processing an edge with all three corners */
  void (*processAllCorners)(void *data, EDGE current, int line);

  /* Optional callback for processing a vertex */
  void (*processVertex)(void *data, VERTEX vertex, COLOR color);
} TriangleTraversalCallbacks;

/**
 * Traverse a triangle's perimeter, calling appropriate callbacks.
 *
 * @param color Color of the triangle to traverse
 * @param corners Array of edges forming the triangle corners
 * @param callbacks Set of functions to call during traversal
 * @param data User data passed to callbacks
 */
void triangleTraverse(COLOR color, EDGE (*corners)[3],
                      TriangleTraversalCallbacks *callbacks, void *data);

/**
 * Check if any lines cross within a triangle.
 *
 * @param color Color of the triangle to check
 * @param corners Array of edges forming the triangle corners
 * @return true if no lines cross, false otherwise
 */
bool dynamicTriangleLinesNotCrossed(COLOR color, EDGE (*corners)[3]);

#endif /* TRIANGLES_H */
