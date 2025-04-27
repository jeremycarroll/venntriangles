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
static FAILURE findCornersByTraversal(EDGE start, int depth,
                                      EDGE* cornersReturn);

/* Externally linked functions */
void resetPoints()
{
  memset(AllUPointPointers, 0, sizeof(AllUPointPointers));
  memset(PointAllUPoints, 0, sizeof(PointAllUPoints));
  NextUPointId = 0;
}

static POINT getOrInitializePoint(COLORSET colorsOfFace, COLOR primary,
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

/* Validation function for development-time checks of point initialization */
static void validatePointInitialization(POINT point, EDGE incomingEdge,
                                        COLOR primary, COLOR secondary, int ix)
{
  assert(point->incomingEdges[ix] == NULL);
  assert(incomingEdge->color == (ix < 2 ? primary : secondary));
  assert(point->colors == ((1u << primary) | (1u << secondary)));
}

/*
 * Determines which slot (0-3) an incoming edge should use at an intersection
 * point. The slot depends on:
 * - Whether the edge is primary (enters from inside its own curve)
 * - Whether the other color's curve contains this face
 * Returns:
 * - 0: Primary edge, other color contains face
 * - 1: Secondary edge, other color excludes face
 * - 2: Secondary edge, other color contains face
 * - 3: Primary edge, other color excludes face
 */
static uint32_t getIncomingEdgeSlot(EDGE incomingEdge, COLOR othercolor,
                                    COLORSET faceColors)
{
  if (IS_PRIMARY_EDGE(incomingEdge)) {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 0 : 3;
  } else {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 2 : 1;
  }
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

/* File scoped static functions */

/*
 * Detects if a corner exists at the current edge based on the state of
 * outside and passed curves.
 * Returns true if a corner is found, false otherwise.
 */
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

/* Find corners by traversing the path and detecting curve crossings.
 * Returns NULL on success, or a failure if too many corners are found.
 */
static FAILURE findCornersByTraversal(EDGE start, int depth,
                                      EDGE* cornersReturn)
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

/* Find and align corners for a given color.
 * For each color, we need to find corners by traversing both clockwise and
 * counter-clockwise around the central face. We can align these two result
 * sets, and find sub-paths along which a corner must lie. For each sub-path one
 * end lies in one result set and the other end in the other.
 *
 * The alignment works as follows:
 * - Clockwise corners are stored in result[][0]
 * - Counter-clockwise corners are stored in result[][1]
 * - They are aligned in reverse order (i-1-j) because:
 *   - i is the count of clockwise corners
 *   - We want the last counter-clockwise corner to pair with the first
 * clockwise corner
 *   - Hence the reverse indexing
 */
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
