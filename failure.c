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
    "M",
    "Multiple failures",
    {0},
    .u.mulipleColors = MultipleFailures,
};

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
    case DISCONNECTED_CURVE_FAILURE:
    case TOO_MANY_CORNERS_FAILURE:
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

void initializeFailures(void)
{
  newFailureStatistic(&NoMatchFailure);
  newFailureStatistic(&CrossingLimitFailure);
  newFailureStatistic(&DisconnectedCurveFailure);
  newFailureStatistic(&TooManyCornersFailure);
  newFailureStatistic(&MultipleFailure);
}
