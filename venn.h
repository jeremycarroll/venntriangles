#ifndef _VENN_H
#define _VENN_H

/*
We use just one header file:
- the data structures refer to one another a lot.
- this is not too long.
*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSUMPTION     \
  (sizeof(uint64_t) == \
   sizeof(void *))            // we cast pointers to uint64_t in the trail.
typedef uint64_t uint_trail;  // Any non-pointer value that might go on the
                              // trail, should be of this type, using a union.
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define BITS_PER_WORD (sizeof(uint64_t) * 8)
#define FACTORIAL0 1u
#define FACTORIAL1 1u
#define FACTORIAL2 2u
#define FACTORIAL3 6u
#define FACTORIAL4 24u
#define FACTORIAL5 120u
#define CHOOSE_6_0 1u
#define CHOOSE_6_1 6u
#define CHOOSE_6_2 15u
#define CHOOSE_6_3 20u
#define CHOOSE_4_0 1u
#define CHOOSE_4_1 4u
#define CHOOSE_3_0 1u
/* The curves are _colored_ from 0 to 5. */
#ifndef NCURVES
#define NCURVES 6
#endif
#define NFACES (1 << NCURVES)
// #define NPOINTS (NFACES - 2)
#if NCURVES == 6
#define NCYCLES                                        \
  (CHOOSE_6_0 * FACTORIAL5 + CHOOSE_6_1 * FACTORIAL4 + \
   CHOOSE_6_2 * FACTORIAL3 + CHOOSE_6_3 * FACTORIAL2)
#define NCYCLE_ENTRIES                                         \
  (CHOOSE_6_0 * FACTORIAL5 * 6 + CHOOSE_6_1 * FACTORIAL4 * 5 + \
   CHOOSE_6_2 * FACTORIAL3 * 4 + CHOOSE_6_3 * FACTORIAL2 * 3)
#elif NCURVES == 4
#define NCYCLES (CHOOSE_4_0 * FACTORIAL3 + CHOOSE_4_1 * FACTORIAL2)
#define NCYCLE_ENTRIES (CHOOSE_4_0 * FACTORIAL4 + CHOOSE_4_1 * FACTORIAL3)
#else
#define NCYCLES (CHOOSE_3_0 * FACTORIAL2)
#define NCYCLE_ENTRIES (CHOOSE_3_0 * FACTORIAL3)
#endif
#define CYCLESET_LENGTH ((NCYCLES - 1) / BITS_PER_WORD + 1)
/*
There are NFACES - 2 points, by Euler.
These are chosen from a rather larger set: each point is defined
by  the two colors crossing at that point, and then the 2^4 possible faces
of the other colors being the inner face. We wire those points up in advance.
The two colors are ordered, first the one crossing from inside to outside, then
the other.
*/
#define NPOINTS ((1 << (NCURVES - 2)) * NCURVES * (NCURVES - 1))
#define POINTSET_LENGTH ((NPOINTS - 1) / BITS_PER_WORD + 1)
#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)
#define MAX_ONE_WAY_CURVE_CROSSINGS 3
#define MAX_CORNERS 3

/* TODO: improve this number, 10^6 looks very safe, but we should aim for less.
 */
#define TRAIL_SIZE 1000000

#define NO_COLOR (~(uint32_t)0)
// In this file we add these keywords, with the given meanings.
#define DYNAMIC  // This field is modified in the trail, i.e. after
                 // initialization we track all changes.
#define STATIC   // This field is initialized before we start and is then not
                 // modified.

typedef uint32_t COLOR;    /* Actually between 0 and 5. */
typedef uint32_t COLORSET; /* A set of bits, so between 0 and 63 */
typedef struct face *FACE;
typedef struct facial_cycle *CYCLE;
typedef struct edge *EDGE;
typedef struct directedPoint *DPOINT;
typedef struct undirectedPoint *UPOINT;
// The ith bit is set if the i-th CYCLE is in the set.
typedef uint64_t *CYCLESET;
typedef uint64_t CYCLESET_DECLARE[CYCLESET_LENGTH];
typedef uint64_t *POINTSET;
typedef uint64_t POINTSET_DECLARE[POINTSET_LENGTH];

typedef struct trail *TRAIL;

/*
All fields are null if the point is incoherent.
(see below).

*/
struct directedPoint {
  /* This DPOINT is at the end of one edge only.
     To find that edge, if the to field here is not NULL, then:
     it is: out[0]->reversed->to->out[0]->reversed
  */
  EDGE out[2];
  UPOINT point;
};

/*
An edge is directed, clockwise around a face.

The reversed edge is hence clockwise around the adjacent face.

One of these two is clockwise around the central face, this is primary.

*/
struct edge {
  STATIC FACE face;
  STATIC EDGE reversed;
  /*
  to starts off as NULL, and is set to the jth member of possiblyTo
  where (color, j) is in the facial cycle of inner, equivalently
  (j, color) is in the facial cycle of outer.
  */
  DYNAMIC DPOINT to;
  STATIC COLOR color;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th enty
  being all NULL.
   */
  STATIC struct directedPoint possiblyTo[NCURVES];
};
/*
IS_PRIMARY_EDGE is 1 if the edge is clockwise around the central face, 0
otherwise.
*/

#define memberOfColorSet(color, colorSet) (((colorSet) >> (color)) & 1u)
#define IS_PRIMARY_EDGE(edge) \
  (memberOfColorSet((edge)->color, (edge)->face->colors))

struct face {
  // cycle must be null if cycleSetSize is not 1.
  DYNAMIC struct facial_cycle *cycle;
  STATIC struct face *adjacentFaces[NCURVES];
  STATIC struct edge edges[NCURVES];
  DYNAMIC CYCLESET_DECLARE possibleCycles;
  DYNAMIC TRAIL backtrack;
  STATIC COLORSET colors;           // holds up to NFACES
  DYNAMIC uint_trail cycleSetSize;  // holds up to NCYCLES
};

STATIC struct facial_cycle {
  uint32_t length;
  COLOR curves[NCURVES + 1];
  COLORSET colors;
  /*
    This is a pointer to an array of length length.
    sameDirection[i] refers to curves[i] and curves[i+1]
  */
  CYCLESET *sameDirection;
  /*
     This is a pointer to an array of length length.
     oppositeDirection[i] refers to curves[i-1] curves[i] and curves[i+1]
  */
  CYCLESET *oppositeDirection;
};

/* We create all possible points during initialization.

   The point is being used if incomingEdges[i]->to is non-null
   for each i.
    a) all 4 of them or none of them are
    b) incomingEdges[i]->to->point = this point
    c) incomingEdges[i]->to->out[0]->reversed is the other incomingEdge of the
   same color d) incomingEdges[i]->to->out[1]->reversed is an incomingEdge of
   the other color e) incomingEdges[i]->to->out[j]->reversed->to->point is this
   point f) incomingEdges[i]->to->out[j]->reversed->to->out[j]->reversed =
   incomingEdges[i]

   The point is between the crossing of two curves, one colored A
   and the other colored B, A and B used in the comments below.

   The curve colored primary crosses from inside the curve colored secondary to
   outside it. The curve colored secondary crosses from outside the curve
   colored primary to inside it.
 */
struct undirectedPoint {
  /*
  TODO: logically redundant, is this helpful?
  If the point is between crossing of curve A and curve B,
  then face[0] is inside neither curve
  face[1] is inside A but not B
  face[2] is inside B but not A
  face[3] is inside both A and B
  */
  FACE faces[4];
  /*
  If the point is between crossing of curve A and curve B,
  and color(A) < color(B),
  then edgo[0] is of color A and runs into the point
  edge[1] is the counterclockwise edge of color A that runs into the point
  edge[2] is of color B and runs into the point
  edge[3] is the counterclockwise edge of color B that runs into the point

  The outgoing edges are found by reversing the incoming edges
  */
  EDGE incomingEdges[4];

  /* A value between 0 and NPOINTS */
  uint32_t id;

  /* Colors can also be accessed from the edges as above.
   */
  COLOR primary, secondary;

  /* A set of the two colors. */
  COLORSET colors;
};

typedef enum failureType {
  MULTIPLE_FAILURE = 0x200,
  NO_MATCH_FAILURE = 0x1,
  CROSSING_LIMIT_FAILURE = 0x2,
  DISCONNECTED_CURVE_FAILURE = 0x4,
  TOO_MANY_CORNERS_FAILURE = 0x8,
} FAILURE_TYPE;

typedef struct failure *FAILURE;

struct failure {
  FAILURE_TYPE type;
  char *label;
  // The number of times this failure has been used, at each depth.
  uint64_t count[NFACES];
  union {
    COLORSET colors;
    COLORSET *mulipleColors;
    ;
  } u;
};

struct trail {
  void *ptr;
  uint_trail value;
};

/*
 All DYNAMIC fields must be in this structure: during unit testing we reset this
 to zero. Also, any datum that can be the value of any DYNAMIC pointer must be
 in this structure, (this allows us to safely switch to a 32 bit trail if we
 choose)
 */
struct global {
  struct face faces[NFACES];
  /*
  The points are accessed from the edges, with the pointers set up dynamically.
  We allocate the points from this stack, using nextPoint to keep track of the
  next free point, nextPoint goes down as we backtrack, automatically freeing
  points. Because this stack tracks the trail, we do not need to clean up, and
  so the value of the fields of each point not in the trail; however, pointers
  to the POINT, are in the trail (they are set once, so we don't really need to
  trail the previous value, which is always NULL).
   */
  // struct point points[NPOINTS];
  /*
  The edges are accessed from faces, with the pointers set up during
  initialization.
   */
  // struct edge edges[NFACES * NCURVES];
  DYNAMIC uint_trail nextPoint;
  /*
  These cycles are accessed from faces, with the pointers set up dynamically.
  The faces use the id of the cycle to find it in this list.
  */
  STATIC struct facial_cycle cycles[NCYCLES];
  /* diagonal is 0; we expect in a solution that
     this matrix is symmetric and all other values are 3 or 2. */
  DYNAMIC uint_trail crossings[NCURVES][NCURVES];
  /* If we have a color-curve that is not edgeCount[color] long, then we do not
    have a solution.
    edgeCount[color] is the number of edges of given color with a _to_ field
    set.
    */
  DYNAMIC uint_trail edgeCount[NCURVES];
  DYNAMIC uint_trail curveComplete[NCURVES];
};

extern struct global globals;

#define g_faces globals.faces
#define g_points globals.points
#define g_edges globals.edges
#define g_nextPoint globals.nextPoint
#define g_cycles globals.cycles
#define g_crossings globals.crossings
#define g_edgeCount globals.edgeCount
#define g_curveComplete globals.curveComplete

extern TRAIL trail;
extern void initialize(void);
extern void clearWithoutColor(void);
extern void initializeWithoutColor(void);
extern void initializePoints(void);
extern void clearPoints(void);
extern void addToCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern void removeFromCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern bool memberOfCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE findFirstCycleInSet(CYCLESET cycleSet);
extern uint32_t sizeOfCycleSet(CYCLESET cycleSet);
extern uint32_t findCycleId(uint32_t *cycle, uint32_t length);
extern bool contains2(CYCLE cycle, uint32_t i, uint32_t j);
extern bool contains3(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k);
extern void setDynamicPointer_(void **ptr, void *value);
#define setDynamicPointer(a, b) setDynamicPointer_((void **)a, b)

extern UPOINT addToPoint(FACE face, EDGE incomingEdge, COLOR othercolor);
extern FAILURE assignPoint(FACE face, COLOR aColor, COLOR bColor, int depth);
extern EDGE followEdgeBackwards(EDGE edge);
extern EDGE followEdgeForwards(EDGE edge);

extern void setDynamicInt(uint_trail *ptr, uint_trail value);
extern void backtrackTo(TRAIL backtrackPoint);
extern void setCycleLength(uint32_t faceColors, uint32_t length);

extern FAILURE makeChoice(FACE face);
extern FAILURE curveChecks(EDGE edge, int depth);
extern FAILURE noMatchingCyclesFailure(COLORSET colors, int depth);
extern FAILURE maybeAddFailure(FAILURE multipleFailuresOrNull,
                               FAILURE singleFailure, int depth);
extern FAILURE disconnectedCurveFailure(COLOR color, bool explicit, int depth);
extern FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth);
extern FAILURE tooManyCornersFailure(COLOR a, int depth);
/* Ordered crossing: we expect the same number of a-b crosses, as b-a crosses;
and that number should be three or less. */
extern FAILURE checkCrossingLimit(COLOR a, COLOR b, int depth);
extern bool removeColorFromSearch(COLOR color, int depth);

extern char *edge2str(char *dbuffer, EDGE edge);
extern char *face2str(char *dbuffer, FACE face);
extern char *colors2str(char *dbuffer, COLORSET colors);
extern int color2char(char *dbuffer, COLOR c);
extern void printFace(FACE face);
extern void printEdge(EDGE edge);
#define POINT_DEBUG 0

#endif
