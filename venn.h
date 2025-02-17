#ifndef _VENN_H
#define _VENN_H

/*
We use just one header file:
- the data structures refer to one another a lot.
- this is not too long.
*/

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ASSUMPTION ( sizeof(uint64_t) == sizeof(void*) ) // we cast pointers to uint64_t in the trail.
typedef uint64_t uint_trail; // Any non-pointer value that might go on the trail, should be of this type, using a union.
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
/* The curves are _colored_ from 0 to 5. */
#define NCURVES 6
#define NFACES (1<<NCURVES)
#define NPOINTS (NFACES - 2)
#define NCYCLES (CHOOSE_6_0 * FACTORIAL5 + CHOOSE_6_1 * FACTORIAL4 + CHOOSE_6_2 * FACTORIAL3 + CHOOSE_6_3  * FACTORIAL2)
#define CYCLESET_LENGTH ((NCYCLES-1) / BITS_PER_WORD + 1)
#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)
#define NCYCLE_ENTRIES (CHOOSE_6_0 * FACTORIAL5 * 6 + CHOOSE_6_1 * FACTORIAL4 * 5 + CHOOSE_6_2 * FACTORIAL3 * 4 + CHOOSE_6_3  * FACTORIAL2 * 3)
#define MAX_ONE_WAY_CURVE_CROSSINGS 3

/* TODO: improve this number, 10^6 looks very safe, but we should aim for less. */
#define TRAIL_SIZE 1000000

#define NO_COLOR (~(uint32_t)0)
// In this file we add these keywords, with the given meanings.
#define DYNAMIC // This field is modified in the trail, i.e. after initialization we track all changes.
#define STATIC  // This field is initialized before we start and is then not modified.

typedef uint32_t COLOR;  /* Actually between 0 and 5, with NO_COLOR as a potential (I think unused). */
typedef uint32_t COLORSET; /* A set of bits, so between 0 and 63 */
typedef struct face * FACE;
typedef struct facial_cycle * CYCLE;
typedef struct edge * EDGE;
typedef struct point * POINT;
// The ith bit is set if the i-th CYCLE is in the set.
typedef uint64_t *CYCLESET;
typedef uint64_t CYCLESET_DECLARE[CYCLESET_LENGTH];

typedef struct trail * TRAIL;

struct face {
    // cycle must be null if cycleSetSize is not 1.
    DYNAMIC struct facial_cycle * cycle;
    STATIC struct face * adjacentFaces[NCURVES];
    STATIC struct edge * edges[NCURVES];
    DYNAMIC CYCLESET_DECLARE possibleCycles;
    DYNAMIC TRAIL backtrack;
    STATIC COLORSET colors; // holds up to NFACES
    DYNAMIC uint_trail cycleSetSize; // holds up to NCYCLES
 };

 STATIC struct facial_cycle {
    uint32_t length;
    uint32_t curves[NCURVES+1];
    COLORSET colors;
    /* 
      This is a pointer to an array of length length.
      sameDirection[i] refers to curves[i] and curves[i+1]
    */
    CYCLESET * sameDirection;
    /*
       This is a pointer to an array of length length.
       oppositeDirection[i] refers to curves[i-1] curves[i] and curves[i+1]
    */
    CYCLESET * oppositeDirection;
 };

 /* When we create the point we have all four faces and all four edges 
    to hand, so either all eight fields are null or none are.

    The point is between the crossing of two curves, one colored A
    and the other colored B, A and B used in the comments below.

    The curve colored A crosses from inside the curve colored B to outside it.
    The curve colored B crosses from outside the curve colored A to inside it.
  */
 struct point {
   /* Color A: can be accessed as the color of edge[0].
      Color B: can be accessed as the color of edge[2].
   */
    /*
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
    edge[1] is of color A and runs out of the point
    edge[2] is of color B and runs into the point
    edge[3] is of color B and runs out of the point
    */
    EDGE edges[4];
 };

 struct edge {
   STATIC FACE inner;
   STATIC FACE outer;
   /*
   We assign both points when we use this edge in the facial cycle of either inner or outer face.
   We assign one point if we assign this edge to it, prior to it being used in a facial cycle.
   */
   DYNAMIC POINT from;
   DYNAMIC POINT to;
   STATIC COLOR color;
 };

 typedef uint32_t FAILURE_TYPE;
 typedef struct failure * FAILURE;
 
 struct failure {
     FAILURE_TYPE type;
     char * label;
     // The number of times this failure has been used, at each depth.
     uint64_t count[NFACES];
     void * moreInfo;
 };

 struct trail {
   void * ptr;
   uint_trail value;
};

 /*
  All DYNAMIC fields must be in this structure: during unit testing we reset this to zero.
  Also, any datum that can be the value of any DYNAMIC pointer must be in this structure,
  (this allows us to safely switch to a 32 bit trail if we choose)
  */
 struct global {
    struct face faces[NFACES];
    /*
    The points are accessed from the edges, with the pointers set up dynamically.
    We allocate the points from this stack, using nextPoint to keep track of the next free point,
    nextPoint goes down as we backtrack, automatically freeing points.
    Because this stack tracks the trail, we do not need to clean up, and so the value
    of the fields of each point not in the trail; however, pointers to the POINT, are
    in the trail (they are set once, so we don't really need to trail the previous value,
    which is always NULL).
     */
    struct point points[NPOINTS];
    /*
    The edges are accessed from faces, with the pointers set up during initialization.
     */
    struct edge edges[NFACES * NCURVES / 2];
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
      edgeCount[color] is the number of edges of given color with a _to_ field set.
      */
    DYNAMIC uint_trail edgeCount[NCURVES];
 };


extern struct global globals;

#define g_faces globals.faces
#define g_points globals.points
#define g_edges globals.edges
#define g_nextPoint globals.nextPoint
#define g_cycles globals.cycles
#define g_crossings globals.crossings
#define g_edgeCount globals.edgeCount

extern TRAIL trail;
extern void initialize(void);
extern void addToCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern void removeFromCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern bool memberOfCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE findFirstCycleInSet(CYCLESET cycleSet);
extern bool memberOfColorSet(COLOR color, COLORSET colorSet);
extern uint32_t sizeOfCycleSet(CYCLESET cycleSet);
extern bool contains2(CYCLE cycle, uint32_t i, uint32_t j);
extern bool contains3(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k);
extern void setDynamicPointer_(void ** ptr, void * value);
#define setDynamicPointer(a, b) setDynamicPointer_((void**)a, b)

extern void setDynamicInt(uint_trail * ptr, uint_trail value);
extern void backtrackTo(TRAIL backtrackPoint);
extern void setCycleLength(uint32_t faceColors, uint32_t length);


extern POINT createPoint(EDGE aEdgeIn, EDGE aEdgeOut, EDGE bEdgeIn, EDGE bEdgeOut, int depth, FAILURE *failureReturn);

extern FAILURE makeChoice(FACE face);
extern FAILURE curveChecks(EDGE edge);
extern FAILURE noMatchingCyclesFailure(COLORSET colors, int depth);
extern FAILURE maybeAddFailure(FAILURE multipleFailuresOrNull, FAILURE singleFailure, int depth);
/* Ordered crossing: we expect the same number of a-b crosses, as b-a crosses;
and that number should be three or less. */
extern FAILURE checkCrossingLimit(COLOR a, COLOR b, int depth);
extern FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth);
#endif