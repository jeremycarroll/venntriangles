/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "point.h"

#include "trail.h"
#include "utils.h"

#include <stddef.h>

/* Global variables */
struct Point PointAllUPoints[NPOINTS];
static int NextUPointId = 0;
static struct Point* AllUPointPointers[NFACES][NCOLORS][NCOLORS];

/* Forward declarations of file scoped static functions */
static FAILURE findCornersByTraversal(EDGE start, int depth,
                                      EDGE* cornersReturn);
static POINT getOrInitializePoint(COLORSET colorsOfFace, COLOR primary,
                                  COLOR secondary);
static void validatePointInitialization(POINT point, EDGE incomingEdge,
                                        COLOR primary, COLOR secondary, int ix);
static uint32_t getIncomingEdgeSlot(EDGE incomingEdge, COLOR othercolor,
                                    COLORSET faceColors);
static bool detectCornerAndUpdateCrossingSets(COLORSET other, COLORSET* outside,
                                              COLORSET* passed);

/* Externally linked functions */
void resetPoints()
{
  memset(AllUPointPointers, 0, sizeof(AllUPointPointers));
  memset(PointAllUPoints, 0, sizeof(PointAllUPoints));
  NextUPointId = 0;
}

POINT initializePointIncomingEdge(COLORSET colors, EDGE incomingEdge,
                                  COLOR othercolor)
{
  POINT point;
  COLOR primary, secondary;

  uint32_t incomingEdgeSlot =
      getIncomingEdgeSlot(incomingEdge, othercolor, colors);

  switch (incomingEdgeSlot) {
    case 0: /* Primary edge, other color contains face */
    case 1: /* Secondary edge, other color excludes face */
      primary = incomingEdge->color;
      secondary = othercolor;
      break;
    case 2: /* Secondary edge, other color contains face */
    case 3: /* Primary edge, other color excludes face */
      primary = othercolor;
      secondary = incomingEdge->color;
      break;
    default:
      assert(0);
  }

  point = getOrInitializePoint(incomingEdge->colors, primary, secondary);
  validatePointInitialization(point, incomingEdge, primary, secondary,
                              incomingEdgeSlot);

  point->incomingEdges[incomingEdgeSlot] = incomingEdge;

  return point;
}

char* pointToColorSetString(POINT up)
{
  COLORSET colors = up->incomingEdges[0]->colors | (1ll << up->primary) |
                    (1ll << up->secondary);
  return colorSetToBareString(colors);
}

char* pointToString(POINT up)
{
  char* buffer = getBuffer();
  char* colorsStr = pointToColorSetString(up);
  sprintf(buffer, "%s_%c_%c", colorsStr, 'a' + up->primary,
          'a' + up->secondary);
  return usingBuffer(buffer);
}

#if NCOLORS <= 4
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

FAILURE dynamicEdgeCornerCheck(EDGE start, int depth)
{
#if NCOLORS <= 4
  /* test_venn[34].c do not like the normal code - not an issue. */
  return NULL;
#else
  EDGE ignore[MAX_CORNERS * 100];
  if (start->reversed->to != NULL) {
    // we have a complete curve.
    start = edgeOnCentralFace(start->color);
  }
  return findCornersByTraversal(start, depth, ignore);
#endif
}

EDGE edgeOnCentralFace(COLOR a)
{
  COLOR primary = a;
  COLOR secondary = (a + 1) % NCOLORS;
  POINT uPoint = getOrInitializePoint(NFACES - 1, primary, secondary);
  return uPoint->incomingEdges[0];
}

void edgeFindAndAlignCorners(COLOR a, EDGE result[3][2])
{
  int i, j;
  EDGE clockWiseCorners[MAX_CORNERS];
  EDGE counterClockWiseCorners[MAX_CORNERS];
  FAILURE failure =
      findCornersByTraversal(edgeOnCentralFace(a), 0, clockWiseCorners);
  assert(failure == NULL);
  failure = findCornersByTraversal(edgeOnCentralFace(a)->reversed, 0,
                                   counterClockWiseCorners);
  assert(failure == NULL);
  assert((clockWiseCorners[2] == NULL) == (counterClockWiseCorners[2] == NULL));
  assert((clockWiseCorners[1] != NULL));
  assert((counterClockWiseCorners[1] != NULL));
  for (i = 0; i < 3 && clockWiseCorners[i]; i++) {
    result[i][0] = clockWiseCorners[i];
  }
  if (i < 3) {
    result[i][0] = NULL;
    result[i][1] = NULL;
  }

  for (j = 0; j < 3 && counterClockWiseCorners[j]; j++) {
    assert(i - 1 - j >= 0);
    assert(i - 1 - j < 3);
    result[i - 1 - j][1] = counterClockWiseCorners[j];
  }
}

/* File scoped static functions */
static POINT getOrInitializePoint(COLORSET colorsOfFace, COLOR primary,
                                  COLOR secondary)
{
  COLORSET outsideColor = colorsOfFace & ~(1u << primary) & ~(1u << secondary);
  if (AllUPointPointers[outsideColor][primary][secondary] == NULL) {
    AllUPointPointers[outsideColor][primary][secondary] =
        &PointAllUPoints[NextUPointId];
    PointAllUPoints[NextUPointId].primary = primary;
    PointAllUPoints[NextUPointId].secondary = secondary;
    PointAllUPoints[NextUPointId].colors = 1u << primary | 1u << secondary;
    NextUPointId++;
  }
  return AllUPointPointers[outsideColor][primary][secondary];
}

static void validatePointInitialization(POINT point, EDGE incomingEdge,
                                        COLOR primary, COLOR secondary, int ix)
{
  assert(point->incomingEdges[ix] == NULL);
  assert(incomingEdge->color == (ix < 2 ? primary : secondary));
  assert(point->colors == ((1u << primary) | (1u << secondary)));
}

static uint32_t getIncomingEdgeSlot(EDGE incomingEdge, COLOR othercolor,
                                    COLORSET faceColors)
{
  if (IS_PRIMARY_EDGE(incomingEdge)) {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 0 : 3;
  } else {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 2 : 1;
  }
}

static bool detectCornerAndUpdateCrossingSets(COLORSET other, COLORSET* outside,
                                              COLORSET* passed)
{
  if (other & *outside) {
    // We're crossing back inside some curves
    *outside &= ~other;

    if (other & *passed) {
      *passed = 0;
      return true;
    }
  } else {
    // We're crossing to the outside of these curves
    *passed |= other;
    *outside |= other;
  }

  return false;
}

static FAILURE findCornersByTraversal(EDGE start, int depth,
                                      EDGE* cornersReturn)
{
  EDGE current = start;
  COLORSET notMyColor = ~(1u << start->color),
           /* the curves we have crossed outside of since the last corner. */
      passed = 0,
           /* the curves we are currently outside. */
      outside = ~start->colors;
  int counter = 0;
  assert(start->reversed->to == NULL ||
         (start->colors & notMyColor) == ((NFACES - 1) & notMyColor));
  do {
    CURVELINK p = current->to;
    if (detectCornerAndUpdateCrossingSets(p->point->colors & notMyColor,
                                          &outside, &passed)) {
      if (counter >= MAX_CORNERS) {
        return failureTooManyCorners(depth);
      }
      cornersReturn[counter++] = current;
    }
    current = p->next;
  } while (current->to != NULL && current != start);
  while (counter < MAX_CORNERS) {
    cornersReturn[counter++] = NULL;
  }
  return NULL;
}
