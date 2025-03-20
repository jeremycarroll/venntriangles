#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "color.h"
#include "core.h"
#include "failure.h"
#include "graph.h"
#include "point.h"
#include "trail.h"

extern COLORSET completedColors;
extern uint64_t cycleGuessCounter;

void setupCentralFaces(int *faceDegrees);
FAILURE makeChoice(FACE face);
bool removeColorFromSearch(COLOR color);
void initializeDynamicCounters(void);

#endif  // DYNAMIC_H
