#include "failure.h"

#include "statistics.h"

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the
current state of the failure as we backtrack. Each of the static failures
also has a counter, which is incremented on every use, so that we can produce
stats.
*/
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static Failure NoMatchFailure = {
    .shortLabel = "N",
    .label = "No matching cycles",
    .count = {0},
};

static Failure CrossingLimitFailure = {
    .shortLabel = "X",
    .label = "More than 3 crossing points",
    .count = {0},
};

static Failure DisconnectedCurveFailure = {
    .shortLabel = "D",
    .label = "Disconnected curve",
    .count = {0},
};

static Failure TooManyCornersFailure = {
    .shortLabel = "T",
    .label = "Too many corners",
    .count = {0},
};

static Failure PointConflictFailure = {
    .shortLabel = "P",
    .label = "Point conflict",
    .count = {0},
};

static Failure ConflictingConstraintsFailure = {
    .shortLabel = "C",
    .label = "Conflicting constraints",
    .count = {0},
};

static Failure DisconnectedFacesFailure = {
    .shortLabel = "F",
    .label = "Disconnected faces",
    .count = {0},
};

static Failure NonCanoncialFailure = {
    .shortLabel = "=",
    .label = "Non Canonical",
    .count = {0},
};

FAILURE dynamicFailureNoMatchingCycles(int depth)
{
  NoMatchFailure.count[depth]++;
  return &NoMatchFailure;
}

FAILURE dynamicFailureCrossingLimit(int depth)
{
  CrossingLimitFailure.count[depth]++;
  return &CrossingLimitFailure;
}

FAILURE dynamicFailurePointConflict(int depth)
{
  PointConflictFailure.count[depth]++;
  return &PointConflictFailure;
}

FAILURE dynamicFailureConflictingConstraints(int depth)
{
  ConflictingConstraintsFailure.count[depth]++;
  return &ConflictingConstraintsFailure;
}

FAILURE dynamicFailureDisconnectedCurve(int depth)
{
  DisconnectedCurveFailure.count[depth]++;
  return &DisconnectedCurveFailure;
}

FAILURE dynamicFailureTooManyCorners(int depth)
{
  TooManyCornersFailure.count[depth]++;
  return &TooManyCornersFailure;
}

FAILURE dynamicFailureDisconnectedFaces(int depth)
{
  DisconnectedFacesFailure.count[depth]++;
  return &DisconnectedFacesFailure;
}

FAILURE dynamicFailureNonCanonical(void)
{
  NonCanoncialFailure.count[0]++;
  return &NonCanoncialFailure;
}

void initializeFailures(void)
{
  dynamicFailureStatisticNew(&NoMatchFailure);
  dynamicFailureStatisticNew(&CrossingLimitFailure);
  dynamicFailureStatisticNew(&DisconnectedCurveFailure);
  dynamicFailureStatisticNew(&TooManyCornersFailure);
  dynamicFailureStatisticNew(&PointConflictFailure);
  dynamicFailureStatisticNew(&ConflictingConstraintsFailure);
  dynamicFailureStatisticNew(&DisconnectedFacesFailure);
  dynamicFailureStatisticNew(&NonCanoncialFailure);
}
