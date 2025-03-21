#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include "failure.h"

typedef uint32_t COLOR;
typedef uint32_t COLORSET;

typedef uint64_t *CYCLESET;
typedef uint64_t CYCLESET_DECLARE[CYCLESET_LENGTH];

typedef struct facial_cycle *CYCLE;

struct facial_cycle {
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

#define memberOfColorSet(color, colorSet) (((colorSet) >> (color)) & 1u)

extern CYCLESET_DECLARE CycleSetOmittingOne[NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingPair[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CylesetWithoutColor[NCOLORS];
extern void initializeCycleSetAdd(uint32_t cycleId, CYCLESET cycleSet);
extern void initializeCycleSetRemove(uint32_t cycleId, CYCLESET cycleSet);
extern bool initializeCycleSetMember(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE initializeCycleSetFindFirst(CYCLESET cycleSet);
extern CYCLE initializeCycleSetFindNext(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t cycleSetSize(CYCLESET cycleSet);
extern uint32_t cycleFindId(uint32_t *cycle, uint32_t length);
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t i, uint32_t j);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j,
                                     uint32_t k);
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);
extern struct facial_cycle Cycles[NCYCLES];
extern void resetCyclesetWithoutColor(void);
extern void initializeCyclesetWithoutColor(void);
// Should have Dynamic in name, and be so.
extern bool setCycleLength(uint32_t faceColors, uint32_t length);
VISIBLE_FOR_TESTING uint32_t dynamicCycleIdFromColors(char *colors);

#endif  // COLOR_H
