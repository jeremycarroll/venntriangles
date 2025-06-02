/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "triangles.h"

#include "edge.h"
#include "trail.h"
#include "utils.h"
#include "vertex.h"

typedef struct {
  uint64 linesCrossed;
  uint64 initialLinesCrossed;
  bool linesAreCrossed;
} LineCrossingData;

static int edgeIsCorner(EDGE edge, EDGE (*corners)[3])
{
  int count = 0;
  for (int ix = 0; ix < 3; ix++) {
    if ((*corners)[ix] == edge) {
      count++;
    }
  }
  return count;
}

static void dynamicCheckRegularEdge(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  VERTEX vertex = current->to->vertex;

  if (vertex->lineId == 0) {
    trailSetInt(&vertex->lineId, 1 + current->color * 3 + line);
  } else {
    uint64 crossedLineAsBit = 1l << vertex->lineId;
    if (lcd->linesCrossed & crossedLineAsBit) {
      lcd->linesAreCrossed = true;
    } else {
      lcd->linesCrossed |= crossedLineAsBit;
    }
  }
}

static void dynamicCheckSingleCorner(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  line = (line + 1) % 3;
  lcd->linesCrossed = 0;
  dynamicCheckRegularEdge(data, current, line);
}

static void dynamicCheckAdjacentCorners(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  line = (line + 2) % 3;
  lcd->linesCrossed = 0;
  dynamicCheckRegularEdge(data, current, line);
}

static void checkAllCorners(void *data, EDGE current, int line)
{
  (void)current;
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  lcd->linesCrossed = 0;
}

bool dynamicTriangleLinesNotCrossed(COLOR color, EDGE (*corners)[3])
{
  LineCrossingData lcd = {0, 0, false};
  TriangleTraversalCallbacks dynamicCallbacks = {
      .processRegularEdge = dynamicCheckRegularEdge,
      .processSingleCorner = dynamicCheckSingleCorner,
      .processAdjacentCorners = dynamicCheckAdjacentCorners,
      .processAllCorners = checkAllCorners,
      .processVertex = NULL};

  triangleTraverse(color, corners, &dynamicCallbacks, &lcd);

  if (lcd.linesCrossed & lcd.initialLinesCrossed) {
    return false;
  }

  return !lcd.linesAreCrossed;
}

/**
 * Traverses a triangle's perimeter, invoking appropriate callbacks based on
 * corner detection.
 *
 * This function walks through the path around the central face of a given
 * color, tracking corners and line numbers. At each step, it invokes the
 * appropriate callback based on how many corners are at the current position
 * (0, 1, 2, or 3 corners). The line parameter (0, 1, or 2) is tracked and
 * updated as corners are encountered.
 */
void triangleTraverse(COLOR color, EDGE (*corners)[3],
                      TriangleTraversalCallbacks *callbacks, void *data)
{
  EDGE edge;
  EDGE path[NFACES];
  EDGE current;
  int ix;

  edge = vertexGetCentralEdge(color);
  edgePathLength(edge, edgeFollowBackwards(edge), path);

  int line = 0;
  for (ix = 0; path[ix] != NULL; ix++) {
    current = path[ix];
    int cornerCount = edgeIsCorner(current->reversed, corners);

    switch (cornerCount) {
      case 0:
        if (callbacks->processRegularEdge) {
          callbacks->processRegularEdge(data, current, line);
        }
        break;
      case 1:
        if (callbacks->processSingleCorner) {
          callbacks->processSingleCorner(data, current, line);
        }
        line = (line + 1) % 3;
        break;
      case 2:
        if (callbacks->processAdjacentCorners) {
          callbacks->processAdjacentCorners(data, current, line);
        }
        line = (line + 2) % 3;
        break;
      case 3:
        if (callbacks->processAllCorners) {
          callbacks->processAllCorners(data, current, line);
        }
        break;
    }

    if (callbacks->processVertex) {
      callbacks->processVertex(data, current->to->vertex, color);
    }
  }

  assert(line == 0);
}
