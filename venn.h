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
#define FACTORIAL0 1
#define FACTORIAL1 1
#define FACTORIAL2 2
#define FACTORIAL3 6
#define FACTORIAL4 24
#define FACTORIAL5 120
#define CHOOSE_6_0 1
#define CHOOSE_6_1 6
#define CHOOSE_6_2 15
#define CHOOSE_6_3 20
/* The curves are _colored_ from 0 to 5. */
#define NCURVES 6
#define NFACES (1<<NCURVES)
#define NPOINTS (NFACES - 2)
#define NCYCLES (CHOOSE_6_0 * FACTORIAL5 + CHOOSE_6_1 * FACTORIAL4 + CHOOSE_6_2 * FACTORIAL3 + CHOOSE_6_3  * FACTORIAL2)
#define CYCLESET_LENGTH ((NCYCLES-1) / sizeof(uint64_t) + 1)
#define NCYCLE_ENTRIES (CHOOSE_6_0 * FACTORIAL5 * 6 + CHOOSE_6_1 * FACTORIAL4 * 5 + CHOOSE_6_2 * FACTORIAL3 * 4 + CHOOSE_6_3  * FACTORIAL2 * 3)

#define BITS_PER_WORD (sizeof(uint64_t) * 8)
/* TODO: improve this number, 10^6 looks very safe, but we should aim for less. */
#define TRAIL_SIZE 1000000

#define NO_COLOR (~(uint32_t)0)
// In this file we add these keywords, with the given meanings.
#define DYNAMIC // This field is modified in the trail, i.e. after initialization we track all changes.
#define STATIC  // This field is initialized before we start and is then not modified.

typedef struct face * FACE;
typedef struct facial_cycle * CYCLE;
typedef struct edge * EDGE;
typedef struct point * POINT;
// The ith bit is set if the i-th CYCLE is in the set.
typedef uint64_t *CYCLESET;
typedef uint64_t CYCLESET_DECLARE[CYCLESET_LENGTH];


struct face {
    // cycle must be null if cycleSetSize is not 1.
    DYNAMIC struct facial_cycle * cycle;
    STATIC struct face * adjacentFaces[NCURVES];
    STATIC struct edge * edges[NCURVES];
    DYNAMIC CYCLESET_DECLARE possibleCycles;
    union {
        struct {
            STATIC unsigned int id: 6; // holds up to NFACES
            DYNAMIC unsigned int cycleSetSize: 9; // holds up to NCYCLES
        } p;
        DYNAMIC uint_trail value;
    } u;
 };

 STATIC struct facial_cycle {
    uint32_t length;
    uint32_t curves[NCURVES+1];
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
  */
 struct point {
    /*
    If the point is between crossing of curve A and curve B, 
    then face[0] is inside neither A nor B
    face[1] is inside A but not B
    face[2] is inside B but not A
    face[3] is inside both A and B
    */
    DYNAMIC FACE * faces[4];
    /*
    If the point is between crossing of curve A and curve B, 
    and color(A) < color(B),
    then edgo[0] is of color A and runs into the point
    edge[1] is of color A and runs out of the point
    edge[2] is of color B and runs into the point
    edge[3] is of color B and runs out of the point
    */
    DYNAMIC EDGE * edges[4];
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
   STATIC unsigned int color: 3;
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
    We allocate the points from this array, using nextPoint to keep track of the next free point,
    nextPoint goes down as we backtrack, automatically freeing points.
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
 };
 

extern struct global globals;

#define faces globals.faces
#define points globals.points
#define edges globals.edges
#define nextPoint globals.nextPoint
#define cycles globals.cycles
// #define cycleSets globals.cycleSets

struct trail {
    void * ptr;
    uint_trail value;
} trail[TRAIL_SIZE];

extern int trailTop;


extern void initialize(void);
extern void addToSet(uint32_t cycleId, CYCLESET cycleSet);
extern bool memberOfSet(uint32_t cycleId, CYCLESET cycleSet);
extern u_int32_t sizeOfSet(CYCLESET cycleSet);
extern bool contains2(CYCLE cycle, uint32_t i, uint32_t j);
extern bool contains3(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k);

#endif