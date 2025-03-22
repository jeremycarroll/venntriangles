#ifndef _UTILS_H
#define _UTILS_H

#include "core.h"

extern  void resetGlobals(void);
extern  void resetInitialize(void);
extern void initialize(void);
extern void initializeDynamicCounters(void);

extern  int* dynamicIntArray(int a, ...);

extern  uint64_t DynamicCycleGuessCounter;

#endif  // _UTILS_H
