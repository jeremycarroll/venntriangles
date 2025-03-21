#ifndef FAILURE_H
#define FAILURE_H

#include <stdint.h>

#include "core.h"

typedef struct {
  uint64_t count[NFACES];
  char* shortLabel;
  char* label;
} Failure, *FAILURE;

void dynamicFailureStatisticNew(FAILURE failure);
FAILURE dynamicFailureNoMatchingCycles(int depth);
FAILURE dynamicFailureCrossingLimit(int depth);
FAILURE dynamicFailurePointConflict(int depth);
FAILURE dynamicFailureConflictingConstraints(int depth);
FAILURE dynamicFailureDisconnectedCurve(int depth);
FAILURE dynamicFailureTooManyCorners(int depth);
FAILURE dynamicFailureDisconnectedFaces(int depth);
FAILURE dynamicFailureNonCanonical(void);
void initializeFailures(void);

#endif  // FAILURE_H
