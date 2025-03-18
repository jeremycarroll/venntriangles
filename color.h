#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>
#include <stdbool.h>

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


extern void addToCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern void removeFromCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern bool memberOfCycleSet(uint32_t cycleId, CYCLESET cycleSet);
extern CYCLE findFirstCycleInSet(CYCLESET cycleSet);
extern CYCLE findNextCycleInSet(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t sizeOfCycleSet(CYCLESET cycleSet);
extern uint32_t findCycleId(uint32_t *cycle, uint32_t length);
extern bool contains2(CYCLE cycle, uint32_t i, uint32_t j);
extern bool contains3(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k);
extern uint32_t indexInCycle(CYCLE cycle, COLOR color);

extern void clearWithoutColor(void);
extern void initializeWithoutColor(void);
// Should have Dynamic in name, and be so.
extern bool setCycleLength(uint32_t faceColors, uint32_t length);

#endif // COLOR_H