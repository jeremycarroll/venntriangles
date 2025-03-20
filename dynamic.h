#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "core.h"
#include "color.h"
#include "graph.h"
#include "trail.h"
#include "point.h"
#include "failure.h"

extern COLORSET completedColors;
extern uint64_t cycleGuessCounter;

void setupCentralFaces(int *faceDegrees);
FAILURE makeChoice(FACE face);
bool removeColorFromSearch(COLOR color);
void initializeDynamicCounters(void);

#endif // DYNAMIC_H