/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"

#include "face.h"
#include "main.h"
#include "statistics.h"
#include "trail.h"
#include "visible_for_testing.h"

/**
 * The engine implements a WAM-like execution model for search.
 * See docs/DESIGN.md "Non-deterministic Engine, Backtracking, Memory and the
 * Trail" for detailed documentation.
 */

/* We use 16384 which is about twice what we need. */
#define TRAIL_SIZE 16384
#define MAX_STACK_SIZE 1000

struct trail {
  void* ptr;
  uint_trail value;
};

static struct trail TrailArray[TRAIL_SIZE];
TRAIL Trail = TrailArray;
static TRAIL frozenTrail = NULL;
static uint64 MaxTrailSize = 0;
static struct stackEntry stack[MAX_STACK_SIZE + 1], *stackTop = stack;
static int Counter = 0;

const struct predicateResult PredicateFail = {PREDICATE_FAIL, 0};
const struct predicateResult PredicateSuccessNextPredicate = {
    PREDICATE_SUCCESS_NEXT_PREDICATE, 0};
const struct predicateResult PredicateSuccessSamePredicate = {
    PREDICATE_SUCCESS_SAME_PREDICATE, 0};

/**
 * Helper function to create a predicate result with choices.
 *
 * This is used in the Try phase when a predicate offers multiple
 * choices to explore.
 */
struct predicateResult predicateChoices(int numberOfChoices)
{
  return (struct predicateResult){PREDICATE_CHOICES, numberOfChoices};
}

static void trace(const char* message)
{
  if (!TracingFlag) return;
  fprintf(stderr, "%d:%ld:", stackTop->counter, (long)(stackTop - stack));
  if (stackTop->currentChoice >= 0) {
    fprintf(stderr, "%s(%d,%d) %s\n", message, stackTop->round,
            stackTop->currentChoice, stackTop->predicate->name);
  } else {
    fprintf(stderr, "%s(%d) %s\n", message, stackTop->round,
            stackTop->predicate->name);
  }
}

/**
 * Initializes a new stack entry after success. .
 * Maybe for the same, or for the next predicate.
 */
static void pushStackEntry(struct stackEntry* entry, PredicateResultCode code)
{
  entry->inChoiceMode = false;
  entry->predicates = entry[-1].predicates;
  entry->predicates = code == PREDICATE_SUCCESS_NEXT_PREDICATE
                          ? entry[-1].predicates + 1
                          : entry[-1].predicates;
  entry->predicate = *entry->predicates;
  entry->round =
      code == PREDICATE_SUCCESS_NEXT_PREDICATE ? 0 : entry[-1].round + 1;
  entry->currentChoice = -1;
  entry->trail = Trail;
  entry->counter = Counter++;
}

/**
 * Handles the initial attempt to execute a predicate.
 * Returns false if execution should be suspended.
 */
static bool callPort(void)
{
  PredicateResult result = stackTop->predicate->try(stackTop->round);

  switch (result.code) {
    case PREDICATE_SUCCESS_NEXT_PREDICATE:
    case PREDICATE_SUCCESS_SAME_PREDICATE:
      pushStackEntry(++stackTop, result.code);
      assert(stackTop < stack + MAX_STACK_SIZE);
      break;

    case PREDICATE_FAIL: /* 0 choices */
    case PREDICATE_CHOICES:
      // Start trying choice
      stackTop->inChoiceMode = true;
      stackTop->currentChoice = 0;
      stackTop->numberOfChoices = result.numberOfChoices;
      stackTop->trail = Trail;
      break;
    case PREDICATE_SUSPEND:
      return false;
  }
  return true;
}

/**
 * Handles subsequent attempts to execute a predicate after initial choices.
 */
static void retryPort(void)
{
  PredicateResult result =
      stackTop->predicate->retry(stackTop->round, stackTop->currentChoice++);

  switch (result.code) {
    case PREDICATE_FAIL:
      trailRewindTo(stackTop->trail);
      break;

    case PREDICATE_SUCCESS_NEXT_PREDICATE:
    case PREDICATE_SUCCESS_SAME_PREDICATE:
      pushStackEntry(++stackTop, result.code);
      assert(stackTop < stack + MAX_STACK_SIZE);
      break;
    case PREDICATE_CHOICES:
    case PREDICATE_SUSPEND:
      assert(false);
      break;
  }
}

/**
 * The main execution loop of the engine.
 * Returns true if execution completed normally, false if suspended.
 */
static bool engineLoop(void)
{
  while (true) {
    freeAll();
    trailRewindTo(stackTop->trail);
    if (!stackTop->inChoiceMode) {
      trace("call");
      if (!callPort()) {
        return false;
      }
    } else {
      if (stackTop->currentChoice >= stackTop->numberOfChoices) {
        /* backtrack */
        do {
          trace("fail");
          if (stackTop == stack) {
            return true;  // All done
          }
          stackTop--;
        } while (!stackTop->inChoiceMode);
        continue;
      }
      trace("retry");
      retryPort();
    }
  }
}

void initializeTrail()
{
  statisticIncludeInteger(&MaxTrailSize, "$", "MaxTrail", true);
}

void trailSetPointer(void** ptr, void* value)
{
  Trail->ptr = ptr;
  Trail->value = (uint_trail)*ptr;
  Trail++;
  *ptr = value;
}

void trailSetInt(uint_trail* ptr, uint_trail value)
{
  Trail->ptr = ptr;
  Trail->value = *ptr;
  Trail++;
  *ptr = value;
}

void trailMaybeSetInt(uint_trail* ptr, uint_trail value)
{
  if (*ptr != value) {
    trailSetInt(ptr, value);
  }
}

/**
 * Freezes the trail at its current point. Backtracking won't go beyond this
 * point.
 */
void trailFreeze()
{
  frozenTrail = Trail;
}

/**
 * Rewinds the trail to a given backtrack point, restoring earlier state.
 */
bool trailRewindTo(TRAIL backtrackPoint)
{
  uint64 trailSize = Trail - TrailArray;
  if (trailSize > MaxTrailSize) {
    MaxTrailSize = trailSize;
  }
  bool result = false;
  if (backtrackPoint < frozenTrail) {
    backtrackPoint = frozenTrail;
  }
  while (Trail > backtrackPoint) {
    result = true;
    Trail--;
    *(uint_trail*)Trail->ptr = Trail->value;
  }
  return result;
}

/**
 * Runs the non-deterministic program starting with the given predicates.
 * See docs/DESIGN.md for detailed explanation of the execution model.
 */
void engine(PREDICATE* predicates)
{
  // Initialize first stack entry
  assert(stackTop == stack);
  stackTop->inChoiceMode = false;
  stackTop->predicate = *predicates;
  stackTop->predicates = predicates;
  stackTop->currentChoice = -1;
  stackTop->round = 0;
  stackTop->trail = Trail;
  stackTop->counter = Counter++;

  if (!engineLoop() && TracingFlag) {
    fprintf(stderr, "Engine suspended\n");
  } else {
    assert(stackTop == stack || stackTop->predicate == &SUSPENDPredicate);
  }
}

/**
 * Continue from the suspension point, with a new set of predicates.
 * When the new predicates complete, we backtrack through
 * the suspension point to resume the previous execution.
 * This is intended for testing.
 */
void engineResume(PREDICATE* predicates)
{
  bool successfulRun;
  pushStackEntry(++stackTop, PREDICATE_SUCCESS_NEXT_PREDICATE);
  stackTop->predicate = *predicates;
  stackTop->predicates = predicates;
  successfulRun = engineLoop();
  // Suspending twice is not supported.
  assert(successfulRun);
}

/**
 * Helper macro for defining simple predicates with no retry behavior.
 */
#define SIMPLE_PREDICATE(name)                            \
  static struct predicateResult try##name(int round)      \
  {                                                       \
    (void)round;                                          \
    return (struct predicateResult){PREDICATE_##name, 0}; \
  }                                                       \
  struct predicate name##Predicate = {#name, try##name, NULL};

/**
 * A predicate that always fails, used to force backtracking.
 * Typically used as the last predicate in a non-deterministic program.
 */
SIMPLE_PREDICATE(FAIL)

/**
 * A predicate that suspends execution, allowing control to return to the
 * caller. Used for testing.
 */
SIMPLE_PREDICATE(SUSPEND)
