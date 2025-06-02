/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FAILURE_H
#define FAILURE_H

#include "core.h"

/**
 * Failure tracking system for the search algorithm.
 * Records different types of failures encountered during search.
 */

/* Structure for tracking failures */
struct failure {
  char *label;          /* Full descriptive label */
  char *shortLabel;     /* Abbreviated label for concise output */
  uint64 count[NFACES]; /* Counter for failures at each face index */
};

typedef struct failure Failure, *FAILURE;

/**
 * Convenience macro for handling failures in search functions.
 * Usage: CHECK_FAILURE(someCall());
 *
 * Note: Can only be used when there is an appropriate local variable 'failure'
 * in scope.
 */
#define CHECK_FAILURE(call) \
  failure = (call);         \
  if (failure != NULL) {    \
    return failure;         \
  }

/* Initialization */
extern void initializeFailures(void);

/* Failure reporting functions - return specific failure types */
extern FAILURE failureNoMatchingCycles(int depth);
extern FAILURE failureConflictingConstraints(int depth);
extern FAILURE failureDisconnectedFaces(int depth);
extern FAILURE failureVertexConflict(int depth);

/* Geometric constraint failures */
extern FAILURE failureCrossingLimit(int depth);
extern FAILURE failureDisconnectedCurve(int depth);
extern FAILURE failureTooManyCorners(int depth);

/* Canonicalization failures */
extern FAILURE failureNonCanonical(void);

#endif  // FAILURE_H
