#ifndef FAILURE_H
#define FAILURE_H

#include <stdint.h>

#include "core.h"

typedef struct {
  uint64_t count[NFACES];
  char* shortLabel;
  char* label;
} Failure, *FAILURE;

extern FAILURE failureNoMatchingCycles(int depth);
extern FAILURE failureCrossingLimit(int depth);
extern FAILURE failurePointConflict(int depth);
extern FAILURE failureConflictingConstraints(int depth);
extern FAILURE failureDisconnectedCurve(int depth);
extern FAILURE failureTooManyCorners(int depth);
extern FAILURE failureDisconnectedFaces(int depth);
extern FAILURE failureNonCanonical(void);
extern void initializeFailures(void);

#endif  // FAILURE_H
