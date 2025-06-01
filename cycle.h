/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CYCLE_H
#define CYCLE_H

#include "color.h"  // For COLOR and COLORSET

// Forward declaration for CYCLESET from cycleset.h
typedef uint64 *CYCLESET;

typedef struct facialCycle *CYCLE;
// CYCLE_ID is already defined in core.h as uint64

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

// Cycle operations
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t a, uint32_t b);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j,
                                     uint32_t k);
extern CYCLE_ID cycleIdFromColors(char *colors);
extern char *cycleToString(CYCLE cycle);
extern CYCLE_ID getCycleId(COLOR *cycle, uint32_t length);
extern CYCLE_ID cycleIdReverseDirection(CYCLE_ID cycleId);
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);
extern struct facialCycle Cycles[NCYCLES];

// Function to ensure cycles are initialized
extern void initializeCycles(void);

#endif  // CYCLE_H
