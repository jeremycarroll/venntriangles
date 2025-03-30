/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef COLOR_H
#define COLOR_H

#include "core.h"

typedef uint32_t COLOR;
typedef uint32_t COLORSET;

typedef uint64_t *CYCLESET;
typedef uint64_t CYCLESET_DECLARE[CYCLESET_LENGTH];

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

extern void cycleSetAdd(uint32_t cycleId, CYCLESET cycleSet);
extern void cycleSetRemove(uint32_t cycleId, CYCLESET cycleSet);
extern bool cycleSetMember(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE cycleSetFirst(CYCLESET cycleSet);
extern CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t cycleSetSize(CYCLESET cycleSet);
extern uint32_t cycleId(uint32_t *cycle, uint32_t length);
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t i, uint32_t j);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j,
                                     uint32_t k);
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);
extern struct facialCycle Cycles[NCYCLES];
extern uint32_t cycleIdFromColors(char *colors);
extern int colorToChar(COLOR c);
extern char *colorSetToStr(COLORSET colors);
extern char *cycleToStr(CYCLE cycle);
extern void removeFromCycleSetWithTrail(uint32_t cycleId, CYCLESET cycleSet);
extern void initializeCycleSets(void);
extern void resetCycles(void);
extern void initializeCycleSetUniversal(CYCLESET cycleSet);
#endif  // COLOR_H
