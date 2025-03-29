#ifndef COLOR_H
#define COLOR_H

#include "core.h"

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

/* These cycleSets are accessed from cycles, with the pointers set up during
   initialization. */
extern CYCLESET_DECLARE InitializeCycleSetPairs[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE InitializeCycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];

extern void initializeCycleSetAdd(uint32_t cycleId, CYCLESET cycleSet);
extern void initializeCycleSetRemove(uint32_t cycleId, CYCLESET cycleSet);
extern bool cycleSetMember(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE cycleSetFindFirst(CYCLESET cycleSet);
extern CYCLE cycleSetFindNext(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t cycleSetSize(CYCLESET cycleSet);
extern uint32_t cycleFindId(uint32_t *cycle, uint32_t length);
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t i, uint32_t j);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j,
                                     uint32_t k);
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);
extern struct facial_cycle Cycles[NCYCLES];
extern uint32_t cycleIdFromColors(char *colors);
extern int colorToChar(COLOR c);
extern char *colorSetToStr(char *dbuffer, COLORSET colors);
extern char *dynamicCycleToStr(char *dbuffer, CYCLE cycle);
extern void removeFromCycleSetWithTrail(uint32_t cycleId, CYCLESET cycleSet);
extern void initializeCycleSets(void);
extern void resetCycles(void);
#endif  // COLOR_H
