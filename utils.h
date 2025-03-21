#ifndef _UTILS_H
#define _UTILS_H

#include "core.h"
VISIBLE_FOR_TESTING void resetGlobals(void);
VISIBLE_FOR_TESTING void resetInitialize(void);
extern void initialize(void);
extern void initializeDynamicCounters(void);

VISIBLE_FOR_TESTING int* dynamicIntArray(int a, ...);
VISIBLE_FOR_TESTING FACE dynamicFaceChoose(bool smallestFirst);
VISIBLE_FOR_TESTING FACE dynamicFaceFromColors(char* colors);
VISIBLE_FOR_TESTING FACE dynamicFaceAddSpecific(char* colors, char* cycle);
VISIBLE_FOR_TESTING void dynamicFacePrintSelected(void);

VISIBLE_FOR_TESTING uint64_t DynamicCycleGuessCounter;
void initializeDynamicCounters(void);

#endif  // _UTILS_H
