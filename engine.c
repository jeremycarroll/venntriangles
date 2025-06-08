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
int EngineCounter = 0;

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

static void trace(STACK stack, const char* message)
{
  if (!TracingFlag) return;
  fprintf(stderr, "%d:%ld:", stack->stackTop->counter,
          stack->stackTop - stack->stack);
  if (stack->stackTop->currentChoice >= 0) {
    fprintf(stderr, "%s(%d,%d) %s\n", message, stack->stackTop->round,
            stack->stackTop->currentChoice, stack->stackTop->predicate->name);
  } else {
    fprintf(stderr, "%s(%d) %s\n", message, stack->stackTop->round,
            stack->stackTop->predicate->name);
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
  entry->counter = EngineCounter++;
}

/**
 * Handles the initial attempt to execute a predicate.
 * Returns false if execution should be suspended.
 */
static bool callPort(STACK stack)
{
  PredicateResult result =
      stack->stackTop->predicate->try(stack->stackTop->round);

  switch (result.code) {
    case PREDICATE_SUCCESS_NEXT_PREDICATE:
    case PREDICATE_SUCCESS_SAME_PREDICATE:
      pushStackEntry(++stack->stackTop, result.code);
      assert(stack->stackTop < stack->stack + MAX_STACK_SIZE);
      break;

    case PREDICATE_FAIL: /* 0 choices */
    case PREDICATE_CHOICES:
      // Start trying choice
      stack->stackTop->inChoiceMode = true;
      stack->stackTop->currentChoice = 0;
      stack->stackTop->numberOfChoices = result.numberOfChoices;
      stack->stackTop->trail = Trail;
      break;
    case PREDICATE_SUSPEND:
      return false;
  }
  return true;
}

/**
 * Handles subsequent attempts to execute a predicate after initial choices.
 */
static void retryPort(STACK stack)
{
  PredicateResult result = stack->stackTop->predicate->retry(
      stack->stackTop->round, stack->stackTop->currentChoice++);

  switch (result.code) {
    case PREDICATE_FAIL:
      trailRewindTo(stack->stackTop->trail);
      break;

    case PREDICATE_SUCCESS_NEXT_PREDICATE:
    case PREDICATE_SUCCESS_SAME_PREDICATE:
      pushStackEntry(++stack->stackTop, result.code);
      assert(stack->stackTop < stack->stack + MAX_STACK_SIZE);
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
static bool engineLoop(STACK stack)
{
  while (true) {
    freeAll();
    trailRewindTo(stack->stackTop->trail);
    if (!stack->stackTop->inChoiceMode) {
      trace(stack, "call");
      if (!callPort(stack)) {
        return false;
      }
    } else {
      if (stack->stackTop->currentChoice >= stack->stackTop->numberOfChoices) {
        /* backtrack */
        do {
          trace(stack, "fail");
          if (stack->stackTop == stack->stack) {
            return true;  // All done
          }
          stack->stackTop--;
        } while (!stack->stackTop->inChoiceMode);
        continue;
      }
      trace(stack, "retry");
      retryPort(stack);
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

bool trailMaybeSetInt(uint_trail* ptr, uint_trail value)
{
  if (*ptr != value) {
    trailSetInt(ptr, value);
    return true;
  }
  return false;
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
bool engine(STACK stack, PREDICATE* predicates)
{
  bool result;
  stack->stackTop = stack->stack;
  stack->stackTop->inChoiceMode = false;
  stack->stackTop->predicate = *predicates;
  stack->stackTop->predicates = predicates;
  stack->stackTop->currentChoice = -1;
  stack->stackTop->round = 0;
  stack->stackTop->trail = Trail;
  stack->stackTop->counter = EngineCounter++;
  result = engineLoop(stack);

  if (!result) {
    if (TracingFlag) {
      fprintf(stderr, "Engine suspended\n");
    }
    assert(stack->stackTop->predicate == &SUSPENDPredicate);
  } else {
    assert(stack->stackTop == stack->stack);
  }
  return result;
}

/**
 * Continue from the suspension point, with a new set of predicates.
 * When the new predicates complete, we backtrack through
 * the suspension point to resume the previous execution.
 * This is intended for testing.
 */
void engineResume(STACK stack, PREDICATE* predicates)
{
  bool successfulRun;
  pushStackEntry(++stack->stackTop, PREDICATE_SUCCESS_NEXT_PREDICATE);
  stack->stackTop->predicate = *predicates;
  stack->stackTop->predicates = predicates;
  successfulRun = engineLoop(stack);
  // Suspending twice is not supported.
  assert(successfulRun);
}

void engineClear(STACK stack)
{
  trailRewindTo(stack->stackTop->trail);
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
