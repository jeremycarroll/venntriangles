#include "failure.h"

#include "statistics.h" /* For newFailureStatistic */

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the
current state of the failure as we backtrack.abort Each of the static failure
also has a counter, which is incremented on every use, so that we can produce
stats.
*/
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static struct failure NoMatchFailure = {
    NO_MATCH_FAILURE,
    "N",
    "No matching cycles",
    {0},
};

static struct failure CrossingLimitFailure = {
    CROSSING_LIMIT_FAILURE,
    "X",
    "More than 3 crossing points",
    {0},
};

static struct failure DisconnectedCurveFailure = {
    DISCONNECTED_CURVE_FAILURE,
    "D",
    "Disconnected curve",
    {0},
};

static struct failure TooManyCornersFailure = {
    TOO_MANY_CORNERS_FAILURE,
    "T",
    "Too many corners",
    {0},
};

static struct failure PointConflictFailure = {
    POINT_CONFLICT_FAILURE,
    "P",
    "Point conflict",
    {0},
};

static struct failure ConflictingConstraintsFailure = {
    CONFLICTING_CONSTRAINTS_FAILURE,
    "C",
    "Conflicting constraints",
    {0},
};

static struct failure DisconnectedFacesFailure = {
    DISCONNECTED_FACES_FAILURE,
    "F",
    "Disconnected faces",
    {0},
};
static struct failure NonCanoncialFailure = {
    NON_CANONICAL_FAILURE,
    "=",
    "Non Canonical",
    {0},
};

FAILURE noMatchingCyclesFailure(COLORSET colors, int depth)
{
  NoMatchFailure.u.colors = colors;
  NoMatchFailure.count[depth]++;
  return &NoMatchFailure;
}

FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth)
{
  CrossingLimitFailure.u.colors = (1u << a) | (1u << b);
  CrossingLimitFailure.count[depth]++;
  return &CrossingLimitFailure;
}

FAILURE pointConflictFailure(COLOR a, COLOR b, int depth)
{
  PointConflictFailure.u.colors = (1u << a) | (1u << b);
  PointConflictFailure.count[depth]++;
  return &PointConflictFailure;
}

FAILURE conflictingConstraintsFailure(FACE f, int depth)
{
  ConflictingConstraintsFailure.u.colors = f->colors;
  ConflictingConstraintsFailure.count[depth]++;
  return &ConflictingConstraintsFailure;
}

FAILURE disconnectedCurveFailure(COLOR a, int depth)
{
  DisconnectedCurveFailure.u.colors = (1u << a);
  DisconnectedCurveFailure.count[depth]++;
  return &DisconnectedCurveFailure;
}

FAILURE tooManyCornersFailure(COLOR a, int depth)
{
  TooManyCornersFailure.u.colors = (1u << a);
  TooManyCornersFailure.count[depth]++;
  return &TooManyCornersFailure;
}

FAILURE disconnectedFacesFailure(COLORSET colors, int depth)
{
  DisconnectedFacesFailure.u.colors = colors;
  DisconnectedFacesFailure.count[depth]++;
  return &DisconnectedFacesFailure;
}
FAILURE nonCanonicalFailure(void)
{
  NonCanoncialFailure.count[0]++;
  return &NonCanoncialFailure;
}

void initializeFailures(void)
{
  newFailureStatistic(&NoMatchFailure);
  newFailureStatistic(&CrossingLimitFailure);
  newFailureStatistic(&DisconnectedCurveFailure);
  newFailureStatistic(&TooManyCornersFailure);
  newFailureStatistic(&PointConflictFailure);
  newFailureStatistic(&ConflictingConstraintsFailure);
  newFailureStatistic(&DisconnectedFacesFailure);
  newFailureStatistic(&NonCanoncialFailure);
}
