/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "triangles.h"

#include "edge.h"
#include "trail.h"
#include "utils.h"
#include "vertex.h"

#include <assert.h>
#include <stdbool.h>

/* Structure to hold data for line crossing check */
typedef struct {
  uint64_t linesCrossed;
  uint64_t initialLinesCrossed;
  bool linesAreCrossed;
} LineCrossingData;

/* Forward declarations of file scoped static functions */
static void getPath(EDGE *path, EDGE from, EDGE to);
static int edgeIsCorner(EDGE edge, EDGE (*corners)[3]);
static void checkRegularEdge(void *data, EDGE current, int line);
static void checkSingleCorner(void *data, EDGE current, int line);
static void checkAdjacentCorners(void *data, EDGE current, int line);
static void checkAllCorners(void *data, EDGE current, int line);

/* Externally linked functions */

/* Other functions (in alphabetical order) */

bool triangleLinesNotCrossed(COLOR color, EDGE (*corners)[3])
{
  LineCrossingData lcd = {0, 0, false};
  TriangleTraversalCallbacks callbacks = {
      .processRegularEdge = checkRegularEdge,
      .processSingleCorner = checkSingleCorner,
      .processAdjacentCorners = checkAdjacentCorners,
      .processAllCorners = checkAllCorners,
      .processVertex = NULL};

  triangleTraverse(color, corners, &callbacks, &lcd);

  if (lcd.linesCrossed & lcd.initialLinesCrossed) {
    return false;
  }

  return !lcd.linesAreCrossed;
}

void triangleTraverse(COLOR color, EDGE (*corners)[3],
                      TriangleTraversalCallbacks *callbacks, void *data)
{
  EDGE edge;
  EDGE path[NFACES];
  EDGE current;
  int ix;

  /* Get the path around the central face for this color */
  edge = edgeOnCentralFace(color);
  getPath(path, edge, edgeFollowBackwards(edge));

  int line = 0;
  for (ix = 0; path[ix] != NULL; ix++) {
    current = path[ix];
    int cornerCount = edgeIsCorner(current->reversed, corners);

    /* Process the edge based on corner count */
    switch (cornerCount) {
      case 0: /* No corners - just a regular edge */
        if (callbacks->processRegularEdge) {
          callbacks->processRegularEdge(data, current, line);
        }
        break;
      case 1: /* Single corner */
        if (callbacks->processSingleCorner) {
          callbacks->processSingleCorner(data, current, line);
        }
        line = (line + 1) % 3;
        break;
      case 2: /* Two adjacent corners */
        if (callbacks->processAdjacentCorners) {
          callbacks->processAdjacentCorners(data, current, line);
        }
        line = (line + 2) % 3;
        break;
      case 3: /* All three corners at once */
        if (callbacks->processAllCorners) {
          callbacks->processAllCorners(data, current, line);
        }
        break;
    }

    /* Process the vertex if callback exists */
    if (callbacks->processVertex) {
      callbacks->processVertex(data, current->to->vertex, color);
    }
  }

  /* Verify we processed all three corners */
  assert(line == 0);
}

/* File scoped static functions */

/* Helper function to count corners in an edge */
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

/* Get path between two edges */
static void getPath(EDGE *path, EDGE from, EDGE to)
{
  int length = edgePathLength(from, to, path);
#if DEBUG
  printf("getPath: %c %x -> %x %d\n", 'A' + from->color, from, to, length);
#endif
  assert(length > 0);
  assert(length == 1 || path[0] != path[length - 1]);
  path[length] = NULL;
}

/* Callback for processing a regular edge in line crossing check */
static void checkRegularEdge(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  VERTEX vertex = current->to->vertex;

  if (vertex->lineId == 0) {
    trailSetInt(&vertex->lineId, 1 + current->color * 3 + line);
  } else {
    uint64_t crossedLineAsBit = 1l << vertex->lineId;
    if (lcd->linesCrossed & crossedLineAsBit) {
      lcd->linesAreCrossed = true;
    } else {
      lcd->linesCrossed |= crossedLineAsBit;
    }
  }
}

/* Callback for processing a single corner in line crossing check */
static void checkSingleCorner(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  line = (line + 1) % 3;
  lcd->linesCrossed = 0;
  checkRegularEdge(data, current, line);
}

/* Callback for processing adjacent corners in line crossing check */
static void checkAdjacentCorners(void *data, EDGE current, int line)
{
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  line = (line + 2) % 3;
  lcd->linesCrossed = 0;
  checkRegularEdge(data, current, line);
}

/* Callback for processing all corners in line crossing check */
static void checkAllCorners(void *data, EDGE current, int line)
{
  (void)current;
  LineCrossingData *lcd = (LineCrossingData *)data;
  if (line == 0) {
    lcd->initialLinesCrossed = lcd->linesCrossed;
  }
  lcd->linesCrossed = 0;
}
