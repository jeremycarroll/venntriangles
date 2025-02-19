#include "venn.h"

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the
current state of the failure as we backtrack.abort Each of the static failure
also has a counter, which is incremented on every use, so that we can produce
stats.
*/
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static COLORSET MultipleFailures[200];
static int CountOfFailuresInMultipleFailure = 0;
static struct failure MultipleFailure = {
    MULTIPLE_FAILURE,
    "Multiple failures",
    {0},
    .u.mulipleColors = MultipleFailures,
};

FAILURE maybeAddFailure(FAILURE failureCollection, FAILURE newFailure,
                        int depth)
{
  if (newFailure == NULL) {
    return failureCollection;
  }
  if (failureCollection == NULL) {
    failureCollection = &MultipleFailure;
    failureCollection->count[depth]++;
    CountOfFailuresInMultipleFailure = 0;
    MultipleFailure.type = MULTIPLE_FAILURE | newFailure->type;
  }
  assert(failureCollection == &MultipleFailure);
  assert(failureCollection->type == (MULTIPLE_FAILURE | newFailure->type));
  switch (newFailure->type) {
    case CROSSING_LIMIT_FAILURE:
      MultipleFailures[CountOfFailuresInMultipleFailure++] =
          newFailure->u.colors;
      break;
    case NO_MATCH_FAILURE:
      assert(NULL == "Unsupported failure type");
      break;
    default:
      assert(NULL == "Unknown failure type");
  }

  return failureCollection;
}

static struct failure NoMatchFailure = {
    NO_MATCH_FAILURE,
    "No matching cycles",
    {0},
};

FAILURE noMatchingCyclesFailure(COLORSET colors, int depth)
{
  NoMatchFailure.u.colors = colors;
  NoMatchFailure.count[depth]++;
  return &NoMatchFailure;
}

static struct failure CrossingLimitFailure = {
    CROSSING_LIMIT_FAILURE,
    "More than 3 crossing points",
    {0},
};

FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth)
{
  CrossingLimitFailure.u.colors = (1u << a) | (1u << b);
  CrossingLimitFailure.count[depth]++;
  return &CrossingLimitFailure;
}

static struct failure DisconnectedCurveFailure = {
    DISCONNECTED_CURVE_FAILURE,
    "Disconnected curve",
    {0},
};

FAILURE disconnectedCurveFailure(COLOR a, bool explicit, int depth)
{
  if (explicit) {
    depth = NFACES - 1;
  }
  DisconnectedCurveFailure.u.colors = (1u << a);
  DisconnectedCurveFailure.count[depth]++;
  return &DisconnectedCurveFailure;
}

static struct failure TooManyCornersFailure = {
    TOO_MANY_CORNERS_FAILURE,
    "Too many corners",
    {0},
};

FAILURE tooManyCornersFailure(COLOR a, int depth)
{
  TooManyCornersFailure.u.colors = (1u << a);
  TooManyCornersFailure.count[depth]++;
  return &TooManyCornersFailure;
}
