#ifndef FAILURE_H
#define FAILURE_H

#include "core.h"
#include <stdint.h>

typedef struct {
    uint64_t count[NFACES];
    char* shortLabel;
    char* label;
} Failure, *FAILURE;



void newFailureStatistic(FAILURE failure);
FAILURE noMatchingCyclesFailure(int depth);
FAILURE crossingLimitFailure(int depth);
FAILURE pointConflictFailure(int depth);
FAILURE conflictingConstraintsFailure(int depth);
FAILURE disconnectedCurveFailure(int depth);
FAILURE tooManyCornersFailure(int depth);
FAILURE disconnectedFacesFailure(int depth);
FAILURE nonCanonicalFailure(void);
void initializeFailures(void);

#endif // FAILURE_H