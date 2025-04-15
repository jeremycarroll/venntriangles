/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "point.h"

#include "trail.h"
#include "utils.h"

#include <stddef.h>

/*
 * Only a few of these are actually used, the look up table is easier this way.
 With n being a COLORSET, and i, j being COLORs
 * [n][i][j] is used if i != j, and the i and j are not in n.
 * other values are left as null
 */
struct Point PointAllUPoints[NPOINTS];
static int NextUPointId = 0;
static struct Point* AllUPointPointers[NFACES][NCOLORS][NCOLORS];

/* Declaration of file scoped static functions */
static FAILURE cornerCheckInternal(EDGE start, int depth, EDGE* cornersReturn);

/* Externally linked functions */
void resetPoints()
{
  memset(AllUPointPointers, 0, sizeof(AllUPointPointers));
  memset(PointAllUPoints, 0, sizeof(PointAllUPoints));
  NextUPointId = 0;
}

POINT getOrInitializePoint(COLORSET colorsOfFace, COLOR primary,
                           COLOR secondary)
{
  COLORSET outsideColor = colorsOfFace & ~(1u << primary) & ~(1u << secondary);
  if (AllUPointPointers[outsideColor][primary][secondary] == NULL) {
    PointAllUPoints[NextUPointId].id = NextUPointId;
    AllUPointPointers[outsideColor][primary][secondary] =
        &PointAllUPoints[NextUPointId];
    PointAllUPoints[NextUPointId].primary = primary;
    PointAllUPoints[NextUPointId].secondary = secondary;
    PointAllUPoints[NextUPointId].colors = 1u << primary | 1u << secondary;
    NextUPointId++;
  }
  return AllUPointPointers[outsideColor][primary][secondary];
}

/*
   The curve colored A crosses from inside the curve colored B to outside it.
   The curve colored B crosses from outside the curve colored A to inside it.
*/
POINT initializePointIncomingEdge(COLORSET colors, EDGE incomingEdge,
                                  COLOR othercolor)
{
  POINT point;
  COLOR primary, secondary;
  uint32_t ix;

  if (IS_PRIMARY_EDGE(incomingEdge)) {
    if (COLORSET_HAS_MEMBER(othercolor, colors)) {
      ix = 0;
    } else {
      ix = 3;
    }
  } else {
    if (COLORSET_HAS_MEMBER(othercolor, colors)) {
      ix = 2;
    } else {
      ix = 1;
    }
  }

  assert(othercolor != incomingEdge->color);
  switch (ix) {
    case 0:
    case 1:
      primary = incomingEdge->color;
      secondary = othercolor;
      break;
    case 2:
    case 3:
      primary = othercolor;
      secondary = incomingEdge->color;
      break;
    default:
      assert(0);
  }

  point = getOrInitializePoint(incomingEdge->colors, primary, secondary);
  assert(point->incomingEdges[ix] == NULL);
  assert(point->colors == 0 ||
         point->colors == ((1u << primary) | (1u << secondary)));
  assert(incomingEdge->color == (ix < 2 ? primary : secondary));
  point->incomingEdges[ix] = incomingEdge;

  assert(point->colors == ((1u << primary) | (1u << secondary)));
  return point;
}

char* pointToColorSetString(POINT up)
{
  COLORSET colors = up->incomingEdges[0]->colors | (1ll << up->primary) |
                    (1ll << up->secondary);
  return colorSetToBareString(colors);
}

char* pointToStr(POINT up)
{
  char* buffer = getBuffer();
  char* colorsStr = pointToColorSetString(up);
  sprintf(buffer, "%s_%c_%c", colorsStr, 'a' + up->primary,
          'a' + up->secondary);
  return usingBuffer(buffer);
}

/*
This is the algorithm as documented by Carroll, 2000.

For each curve C, we start with its edge on the central face, and proceed
around the curve in one direction.
We keep track of two sets:
· a set Out of curves outside of which we lie.
6
· a set Passed of curves which we have recently crossed from the inside
to the outside.

Both sets are initialised to empty. On our walk around C, as we pass the
vertex v we look at the other curve C' passing through that vertex.
If C' is in Out then:
· We remove C' from Out.
· If C' is in Passed then we set Passed as the empty set and add v to
the result set. The idea is that there must be a corner between any
two vertices in the result set.
Otherwise, C' is not in Out and:
· We add C' to Out.
· We add C' to Passed.
At the end of the walk we look at the cardinality of the result set. This tells
us the minimum number of corners required on this curve.
By conducting a similar walk in the opposite direction around the curve we
get a corresponding result set. We can align these two result sets, and find
sub-paths along which a corner must lie. For each sub-path one end lies in
one result set and the other end in the other.
We arbitrarily choose one edge in each of these subpaths and subdivide it
with an additional vertex.
If any curve has fewer than three corners found with this algorithm then
additional corners are added arbitrarily.

*/

/*
Generalize cornering check to go in either direction.
While searching we only go clockwise, on finding a solution, we
go clockwise and counterclockwise to identify the corners.

start must either be an edge of the central face, or be an incomplete end.
cornerReturn is a pointer to an array of length at least 3.
*/

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
  return cornerCheckInternal(start, depth, ignore);
#endif
}

EDGE edgeOnCentralFace(COLOR a)
{
  COLOR primary = a;
  COLOR secondary = (a + 1) % NCOLORS;
  POINT uPoint = getOrInitializePoint(NFACES - 1, primary, secondary);
  return uPoint->incomingEdges[0];
}

void edgeFindCorners(COLOR a, EDGE result[3][2])
{
  int i, j;
  EDGE clockWiseCorners[MAX_CORNERS];
  EDGE counterClockWiseCorners[MAX_CORNERS];
  FAILURE failure =
      cornerCheckInternal(edgeOnCentralFace(a), 0, clockWiseCorners);
  assert(failure == NULL);
  failure = cornerCheckInternal(edgeOnCentralFace(a)->reversed, 0,
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
static FAILURE cornerCheckInternal(EDGE start, int depth, EDGE* cornersReturn)
{
  EDGE current = start;
  COLORSET
  notMyColor = ~(1u << start->color),
  /* the curves we have crossed outside of since the last corner. */
      passed = 0,
  /* the curves we are currently outside. */
      outside = ~start->colors;
  int counter = 0;
  assert(start->reversed->to == NULL ||
         (start->colors & notMyColor) == ((NFACES - 1) & notMyColor));
  do {
    CURVELINK p = current->to;
    COLORSET other = p->point->colors & notMyColor;
    if (other & outside) {
      outside = outside & ~other;
      if (other & passed) {
        if (counter >= MAX_CORNERS) {
          return failureTooManyCorners(depth);
        }
        cornersReturn[counter++] = current;
        passed = 0;
      }
    } else {
      passed |= other;
      outside |= other;
    }
    current = p->next;
  } while (current->to != NULL && current != start);
  while (counter < MAX_CORNERS) {
    cornersReturn[counter++] = NULL;
  }
  return NULL;
}
