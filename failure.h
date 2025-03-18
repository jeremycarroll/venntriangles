#ifndef _FAILURE_H
#define _FAILURE_H

#include <stdint.h>

#include "types.h"

/* Failure types */
enum failure_type {
  NO_MATCH_FAILURE,
  CROSSING_LIMIT_FAILURE,
  DISCONNECTED_CURVE_FAILURE,
  TOO_MANY_CORNERS_FAILURE,
  POINT_CONFLICT_FAILURE,
  CONFLICTING_CONSTRAINTS_FAILURE,
  DISCONNECTED_FACES_FAILURE,
  NON_CANONICAL_FAILURE
};

/* Failure structure */
struct failure {
  enum failure_type type;
  const char *shortLabel;
  const char *label;
  union {
    COLORSET colors;
  } u;
  uint64_t count[MAX_DEPTH];
};

/* Failure function declarations */
FAILURE noMatchingCyclesFailure(COLORSET colors, int depth);
FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth);
FAILURE pointConflictFailure(COLOR a, COLOR b, int depth);
FAILURE conflictingConstraintsFailure(FACE f, int depth);
FAILURE disconnectedCurveFailure(COLOR a, int depth);
FAILURE tooManyCornersFailure(COLOR a, int depth);
FAILURE disconnectedFacesFailure(COLORSET colors, int depth);
FAILURE nonCanonicalFailure(void);
void initializeFailures(void);

#endif /* _FAILURE_H */