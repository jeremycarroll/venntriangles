/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "failure.h"

#include "statistics.h"

/*
For each failure we have a single static instance, which  has an array of
counters for stats. We count failures by their depth in the execution,
this was unnecessary.
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

static Failure VertexConflictFailure = {
    .shortLabel = "P",
    .label = "Vertex conflict",
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
  statisticIncludeFailure(&VertexConflictFailure);
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

FAILURE failureVertexConflict(int depth)
{
  VertexConflictFailure.count[depth]++;
  return &VertexConflictFailure;
}

FAILURE failureTooManyCorners(int depth)
{
  TooManyCornersFailure.count[depth]++;
  return &TooManyCornersFailure;
}
