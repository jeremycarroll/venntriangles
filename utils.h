#ifndef UTILS_H
#define UTILS_H

#include "core.h"

extern  void resetGlobals(void);
extern  void resetInitialize(void);
extern void initialize(void);
extern void initializeDynamicCounters(void);

extern  int* dynamicIntArray(int a, ...);

#endif  // UTILS_H
