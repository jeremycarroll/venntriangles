/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef COLOR_H
#define COLOR_H

#include "core.h"

typedef uint32_t COLOR;
typedef uint32_t COLORSET;

typedef uint64 *CYCLESET;
typedef uint64 CYCLESET_DECLARE[CYCLESET_LENGTH];

typedef struct facialCycle *CYCLE;

struct facialCycle {
  uint32_t length;
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
  COLOR curves[NCOLORS];
};

#define COLORSET_HAS_MEMBER(color, colorSet) (((colorSet) >> (color)) & 1u)

/* These cycleSets are accessed from cycles, with the pointers set up during
   initialization. */
extern CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];

extern void cycleSetAdd(CYCLE_ID cycleId, CYCLESET cycleSet);
extern void cycleSetRemove(CYCLE_ID cycleId, CYCLESET cycleSet);
extern bool cycleSetMember(CYCLE_ID cycleId, CYCLESET cycleSet);
extern CYCLE cycleSetFirst(CYCLESET cycleSet);
extern CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t cycleSetSize(CYCLESET cycleSet);
extern CYCLE_ID getCycleId(uint32_t *cycle, uint32_t length);
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t a, uint32_t b);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t a, uint32_t b,
                                     uint32_t c);
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);
extern struct facialCycle Cycles[NCYCLES];
extern CYCLE_ID cycleIdFromColors(char *colors);
extern int colorToChar(COLOR c);
extern char *colorSetToString(COLORSET colors);
extern char *colorSetToBareString(COLORSET colors);
extern char *cycleToString(CYCLE cycle);
extern void cycleSetRemoveCycleWithTrail(CYCLESET cycleSet, uint32_t cycleId);
extern void initializeCycleSets(void);
extern void resetCycles(void);
extern void initializeCycleSetUniversal(CYCLESET cycleSet);
extern CYCLE_ID cycleIdReverseDirection(CYCLE_ID cycleId);

#endif  // COLOR_H
