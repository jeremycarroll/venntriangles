#include "venn.h"

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the
current state of the failure as we backtrack.abort Each of the static failure
also has a counter, which is incremented on every use, so that we can produce
stats.
*/

#define MULTIPLE_FAILURE_TYPE 0x200
#define NO_MATCH_FAILURE_TYPE 0x1
#define CROSSING_LIMIT_FAILURE_TYPE 0x2
static COLORSET MultipleFailures[200];
static int CountOfFailuresInMultipleFailure = 0;
static struct failure MultipleFailure = {
    0,
    "Multiple failures",
    {0},
    MultipleFailures,
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
    MultipleFailure.type = MULTIPLE_FAILURE_TYPE | newFailure->type;
  }
  assert(failureCollection == &MultipleFailure);
  assert(failureCollection->type == (MULTIPLE_FAILURE_TYPE | newFailure->type));
  switch (newFailure->type) {
    case CROSSING_LIMIT_FAILURE_TYPE:
      MultipleFailures[CountOfFailuresInMultipleFailure++] =
          *(COLORSET *)newFailure->moreInfo;
      break;
    case NO_MATCH_FAILURE_TYPE:
      assert(NULL == "Unsupported failure type");
      break;
    default:
      assert(NULL == "Unknown failure type");
  }

  return failureCollection;
}

static COLORSET NoMatchColorset;
static struct failure NoMatchFailure = {
    NO_MATCH_FAILURE_TYPE,
    "No matching cycles",
    {0},
    &NoMatchColorset,
};

FAILURE noMatchingCyclesFailure(COLORSET colors, int depth)
{
  NoMatchColorset = colors;
  NoMatchFailure.count[depth]++;
  return &NoMatchFailure;
}

static COLORSET CrossingLimitColorPair;
static struct failure CrossingLimitFailure = {
    CROSSING_LIMIT_FAILURE_TYPE,
    "More than 3 crossing points",
    {0},
    &CrossingLimitColorPair,
};

FAILURE crossingLimitFailure(COLOR a, COLOR b, int depth)
{
  CrossingLimitColorPair = (1u << a) | (1u << b);
  CrossingLimitFailure.count[depth]++;
  return &CrossingLimitFailure;
}
