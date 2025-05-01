/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "edge.h"
#include "point.h"

/* Structure to hold callbacks for triangle traversal */
typedef struct {
  /* Callback for processing an edge with no corners */
  void (*processRegularEdge)(void *data, EDGE current, int line);

  /* Callback for processing an edge with one corner */
  void (*processSingleCorner)(void *data, EDGE current, int line);

  /* Callback for processing an edge with two adjacent corners */
  void (*processAdjacentCorners)(void *data, EDGE current, int line);

  /* Callback for processing an edge with all three corners */
  void (*processAllCorners)(void *data, EDGE current, int line);

  /* Optional callback for processing a point */
  void (*processPoint)(void *data, POINT point, COLOR color);
} TriangleTraversalCallbacks;

/* Traverse a triangle's perimeter, calling appropriate callbacks */
void triangleTraverse(COLOR color, EDGE (*corners)[3],
                      TriangleTraversalCallbacks *callbacks, void *data);

/* Check if any lines cross in a triangle */
bool checkLinesNotCrossed(COLOR color, EDGE (*corners)[3]);

#endif /* TRIANGLES_H */
