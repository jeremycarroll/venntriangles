
#include "venn.h"
/*
This file is responsible for checking that a set of edges can make a triangle.
*/
extern COLORSET completedColors;
static uint_trail curveLength(EDGE edge)
{
  EDGE current;
  int result;
  for (result = 1, current = followEdgeForwards(edge); current != edge;
       result++, current = followEdgeForwards(current)) {
    assert(current != NULL);
  }
  return result;
}

static EDGE findStartOfCurve(EDGE edge)
{
  EDGE next, current = edge;
  while ((next = followEdgeBackwards(current)) != edge) {
    if (next == NULL) {
      return current;
    }
    current = next;
  }
  return edge;
}

static FAILURE checkForDisconnectedCurve(EDGE edge, int depth)
{
  uint_trail length;
  // TODO: needs named macro!
  if (edge->reversed->to != NULL) {
    // We have a colored cycle in the FISC.
    length = curveLength(edge);
    if (length < g_edgeCount[IS_PRIMARY_EDGE(edge)][edge->color]) {
      return disconnectedCurveFailure(edge->color, depth);
    }
    assert(length == g_edgeCount[IS_PRIMARY_EDGE(edge)][edge->color]);
    if (completedColors & 1u << edge->color) {
      return NULL;
    }
    completedColors |= 1u << edge->color;
    setDynamicInt(&g_curveComplete[edge->color], 1);
  }
  return NULL;
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
static FAILURE cornerCheckInternal(EDGE start, int depth, UPOINT* cornersReturn)
{
  EDGE current = start;
  COLORSET
  notMyColor = ~(1u << start->color),
  /* the curves we have crossed outside of since the last corner. */
      passed = 0,
  /* the curves we are currently outside. */
      outside = ~start->face->colors;
  int counter = 0;
  assert(start->reversed->to == NULL ||
         (start->face->colors & notMyColor) == ((NFACES - 1) & notMyColor));
  do {
    DPOINT p = current->to;
    COLORSET other = p->point->colors & notMyColor;
    if (other & outside) {
      outside = outside & ~other;
      if (other & passed) {
        if (counter >= MAX_CORNERS) {
          return tooManyCornersFailure(start->color, depth);
        }
        cornersReturn[counter++] = p->point;
        passed = 0;
      }
    } else {
      passed |= other;
      outside |= other;
    }
    current = p->out[0];
  } while (current->to != NULL && current != start);
  return NULL;
}

FAILURE cornerCheck(EDGE start, int depth)
{
  UPOINT ignore[MAX_CORNERS * 100];
  if (start->reversed->to != NULL) {
    // we have a complete curve.
    start = &g_faces[NFACES - 1].edges[start->color];
  }
  return cornerCheckInternal(start, depth, ignore);
}

FAILURE curveChecks(EDGE edge, int depth)
{
  FAILURE failure;
  if (g_curveComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  failure = checkForDisconnectedCurve(start, depth);
  if (failure != NULL) {
    return failure;
  }
  return cornerCheck(start, depth);
}

FAILURE checkCrossingLimit(COLOR a, COLOR b, int depth)
{
  uint_trail* crossing = &g_crossings[a][b];
  if (*crossing + 1 > MAX_ONE_WAY_CURVE_CROSSINGS) {
    return crossingLimitFailure(a, b, depth);
  }
  setDynamicInt(crossing, *crossing + 1);
  return NULL;
}
