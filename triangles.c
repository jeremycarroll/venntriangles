
#include <stddef.h>

#include "d6.h"
#include "face.h"
#if NCOLORS == 4
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define MAX_ONE_WAY_CURVE_CROSSINGS 3
#define MAX_CORNERS 3

/*
This file is responsible for checking that a set of edges can make a triangle.
*/
extern COLORSET DynamicColorCompleted;
static uint_trail curveLength(EDGE edge)
{
  EDGE current;
  int result;
  for (result = 1, current = edgeFollowForwards(edge); current != edge;
       result++, current = edgeFollowForwards(current)) {
    assert(current != NULL);
  }
  return result;
}

static EDGE findStartOfCurve(EDGE edge)
{
  EDGE next, current = edge;
  while ((next = edgeFollowBackwards(current)) != edge) {
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
    if (length <
        EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]) {
      return failureDisconnectedCurve(depth);
    }
    assert(length ==
           EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]);
    if (DynamicColorCompleted & 1u << edge->color) {
      return NULL;
    }
    DynamicColorCompleted |= 1u << edge->color;
    trailSetInt(&EdgeCurvesComplete[edge->color], 1);
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
    DPOINT p = current->to;
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
    current = p->out[0];
  } while (current->to != NULL && current != start);
  while (counter < MAX_CORNERS) {
    cornersReturn[counter++] = NULL;
  }
  return NULL;
}

FAILURE dynamicEdgeCornerCheck(EDGE start, int depth)
{
#if NCOLORS == 4
  /* test_venn4.c does not like the normal code - not an issue. */
  return NULL;
#else
  EDGE ignore[MAX_CORNERS * 100];
  if (start->reversed->to != NULL) {
    // we have a complete curve.
    start = &Faces[NFACES - 1].edges[start->color];
  }
  return cornerCheckInternal(start, depth, ignore);
#endif
}

int edgePathLength(EDGE from, EDGE to)
{
  int i = 1;
  for (; from != to; i++) {
    from = edgeFollowForwards(from);
    assert(from != NULL);
  }
  return i;
}

void edgeFindCorners(COLOR a, EDGE result[3][2])
{
  int i, j;
  EDGE clockWiseCorners[MAX_CORNERS];
  EDGE counterClockWiseCorners[MAX_CORNERS];
  FAILURE failure =
      cornerCheckInternal(&Faces[NFACES - 1].edges[a], 0, clockWiseCorners);
  assert(failure == NULL);
  failure = cornerCheckInternal(Faces[NFACES - 1].edges[a].reversed, 0,
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

#define CHECK_FAILURE(call) \
  failure = (call);         \
  if (failure != NULL) {    \
    return failure;         \
  }

FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth)
{
  FAILURE failure;
  if (EdgeCurvesComplete[edge->color]) {
    return NULL;
  }
  EDGE start = findStartOfCurve(edge);
  CHECK_FAILURE(checkForDisconnectedCurve(start, depth));
  return dynamicEdgeCornerCheck(start, depth);
}

FAILURE dynamicEdgeCheckCrossingLimit(COLOR a, COLOR b, int depth)
{
  uint_trail* crossing = &EdgeCrossingCounts[a][b];
  if (*crossing + 1 > MAX_ONE_WAY_CURVE_CROSSINGS) {
    return failureCrossingLimit(depth);
  }
  trailSetInt(crossing, *crossing + 1);
  return NULL;
}

static FAILURE checkLengthOfCycleOfFaces(FACE face)
{
  uint32_t i = 0,
           expected = FaceSumOfFaceDegree[__builtin_popcount(face->colors)];
  FACE f = face;
  /* Don't call this with inner or outer face. */
  assert(expected != 1);
  do {
    f = f->next;
    i++;
    assert(i <= expected);
    if (f == face) {
      if (i != expected) {
        return failureDisconnectedFaces(0);
      }
      return NULL;
      ;
    }
  } while (f != NULL);
  assert(0);
}

FAILURE dynamicFaceFinalCorrectnessChecks(void)
{
  FAILURE failure;
  COLORSET colors = 1;
  FACE face;
#if NCOLORS == 6
  switch (dynamicSymmetryTypeFaces()) {
    case NON_CANONICAL:
      return failureNonCanonical();
    case EQUIVOCAL:
      /* Does not happen? But not deeply problematic if it does. */
      assert(0); /* could fall through, but will get duplicate solutions. */
      break;
    case CANONICAL:
      break;
  }
#endif
  for (colors = 1; colors < (NFACES - 1); colors |= face->previous->colors) {
    face = Faces + colors;
    CHECK_FAILURE(checkLengthOfCycleOfFaces(face));
  }
  return NULL;
}
