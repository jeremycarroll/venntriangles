/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "failure.h"

#include "statistics.h"

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the
current state of the failure as we backtrack. Each of the static Failures
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

/* Externally linked functions - initialize... */
void initializeFailures(void)
{
  statisticIncludeFailure(&NoMatchFailure);
  statisticIncludeFailure(&CrossingLimitFailure);
  statisticIncludeFailure(&DisconnectedCurveFailure);
  statisticIncludeFailure(&TooManyCornersFailure);
  statisticIncludeFailure(&PointConflictFailure);
  statisticIncludeFailure(&ConflictingConstraintsFailure);
  statisticIncludeFailure(&DisconnectedFacesFailure);
  statisticIncludeFailure(&NonCanoncialFailure);
}

/* Externally linked functions - failure... */
FAILURE failureConflictingConstraints(int depth)
{
  ConflictingConstraintsFailure.count[depth]++;
  return &ConflictingConstraintsFailure;
}

FAILURE failureCrossingLimit(int depth)
{
  CrossingLimitFailure.count[depth]++;
  return &CrossingLimitFailure;
}

FAILURE failureDisconnectedCurve(int depth)
{
  DisconnectedCurveFailure.count[depth]++;
  return &DisconnectedCurveFailure;
}

FAILURE failureDisconnectedFaces(int depth)
{
  DisconnectedFacesFailure.count[depth]++;
  return &DisconnectedFacesFailure;
}

FAILURE failureNoMatchingCycles(int depth)
{
  NoMatchFailure.count[depth]++;
  return &NoMatchFailure;
}

FAILURE failureNonCanonical(void)
{
  NonCanoncialFailure.count[0]++;
  return &NonCanoncialFailure;
}

FAILURE failurePointConflict(int depth)
{
  PointConflictFailure.count[depth]++;
  return &PointConflictFailure;
}

FAILURE failureTooManyCorners(int depth)
{
  TooManyCornersFailure.count[depth]++;
  return &TooManyCornersFailure;
}
