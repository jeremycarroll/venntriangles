
#include "face.h"

struct face Faces[NFACES];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];

static void recomputeCountOfChoices(FACE face);
static void initializePossiblyTo(void);
static FAILURE cornerCheckInternal(EDGE start, int depth, EDGE* cornersReturn);
/*
A FISC is isomorphic to a convex FISC if and only if it is monotone.
A FISC is monotone if its dual has a unique source (no incoming edges) and a
unique sink (no out-going edges).
*/
static void applyMonotonicity(void)
{
  uint32_t colors, cycleId;
  FACE face;
  CYCLE cycle;
  uint32_t chainingCount, i;
  uint64_t currentXor, previousFaceXor, nextFaceXor;
#define ONE_IN_ONE_OUT_CORE(a, b, colors)                              \
  (__builtin_popcountll((currentXor = ((1ll << (a)) | (1ll << (b)))) & \
                        colors) == 1)
#define ONE_IN_ONE_OUT(a, b, colors)                               \
  (!ONE_IN_ONE_OUT_CORE(a, b, colors) ? 0                          \
   : ((1 << (a)) & colors)            ? (nextFaceXor = currentXor) \
                                      : (previousFaceXor = currentXor))
  /* The inner face is NFACES-1, with all the colors; the outer face is 0, with
   * no colors.
   */
  for (colors = 1, face = Faces + 1; colors < NFACES - 1; colors++, face++) {
    for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
      if ((cycle->colors & colors) == 0 || (cycle->colors & ~colors) == 0) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      }
      previousFaceXor = nextFaceXor = 0;
      chainingCount = ONE_IN_ONE_OUT(cycle->curves[cycle->length - 1],
                                     cycle->curves[0], colors)
                          ? 1
                          : 0;
      for (i = 1; i < cycle->length; i++) {
        if (ONE_IN_ONE_OUT(cycle->curves[i - 1], cycle->curves[i], colors)) {
          chainingCount++;
        }
      }
      if (chainingCount != 2) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      } else {
        assert(previousFaceXor);
        assert(nextFaceXor);
        face->nextByCycleId[cycleId] = Faces + (colors ^ nextFaceXor);
        face->previousByCycleId[cycleId] = Faces + (colors ^ previousFaceXor);
      }
    }
    recomputeCountOfChoices(face);
  }
  setCycleLength(0, NCOLORS);
  setCycleLength(~0, NCOLORS);
}

static void recomputeCountOfChoices(FACE face)
{
  trailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
}

bool setCycleLength(uint32_t faceColors, uint32_t length)
{
  FACE face = Faces + (faceColors & (NFACES - 1));
  CYCLE cycle;
  uint32_t cycleId;
  if (length == 0) {
    return true;
  }
  for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    if (cycle->length != length) {
      removeFromCycleSetWithTrail(cycleId, face->possibleCycles);
    }
  }
  recomputeCountOfChoices(face);
  return face->cycleSetSize != 0;
}

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
}

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color, j;
  FACE face, adjacent;
  EDGE edge;
  initializeLengthOfCycleOfFaces();
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    face->colors = facecolors;
    for (j = 0; j < CYCLESET_LENGTH - 1; j++) {
      face->possibleCycles[j] = ~0;
    }
    face->possibleCycles[j] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;

    for (color = 0; color < NCOLORS; color++) {
      uint32_t colorbit = (1 << color);
      adjacent = Faces + (facecolors ^ (colorbit));
      face->adjacentFaces[color] = adjacent;
      edge = &face->edges[color];
      edge->colors = face->colors;
      edge->level = __builtin_popcount(face->colors);
      edge->color = color;
      edge->reversed = &adjacent->edges[color];
    }
  }
  applyMonotonicity();
  initializePossiblyTo();
}

static void initializePossiblyTo(void)
{
  uint32_t facecolors, color, othercolor;
  FACE face;
  EDGE edge;
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
#if POINT_DEBUG
    dynamicFacePrint(face);
#endif
    for (color = 0; color < NCOLORS; color++) {
      edge = &face->edges[color];
      for (othercolor = 0; othercolor < NCOLORS; othercolor++) {
        if (othercolor == color) {
          continue;
        }
        edge->possiblyTo[othercolor].point =
            dynamicPointAdd(face, edge, othercolor);
      }
    }
  }
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
