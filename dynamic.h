#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "color.h"
#include "core.h"
#include "failure.h"
#include "graph.h"
#include "point.h"
#include "trail.h"

extern COLORSET DynamicColorCompleted;
extern uint64_t DynamicCycleGuessCounter;

void initializeFaceSetupCentral(int *faceDegrees);
FAILURE dynamicFaceMakeChoice(FACE face);
bool dynamicColorRemoveFromSearch(COLOR color);
void initializeDynamicCounters(void);

#endif  // DYNAMIC_H
