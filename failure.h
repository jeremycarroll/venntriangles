/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FAILURE_H
#define FAILURE_H

#include "core.h"
#include "memory.h"

#include <stdint.h>

struct failure {
  char *label;
  char *shortLabel;
  uint64 count[NFACES];
};

typedef struct failure Failure, *FAILURE;

/* Can only be used when there is an appropriate local varaible 'failure'
   in scope. */
#define CHECK_FAILURE(call) \
  failure = (call);         \
  if (failure != NULL) {    \
    return failure;         \
  }

extern FAILURE failureNoMatchingCycles(int depth);
extern FAILURE failureCrossingLimit(int depth);
extern FAILURE failureVertexConflict(int depth);
extern FAILURE failureConflictingConstraints(int depth);
extern FAILURE failureDisconnectedCurve(int depth);
extern FAILURE failureTooManyCorners(int depth);
extern FAILURE failureDisconnectedFaces(int depth);
extern FAILURE failureNonCanonical(void);
extern void initializeFailures(void);

#endif  // FAILURE_H
