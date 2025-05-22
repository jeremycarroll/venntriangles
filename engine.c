/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"

#include "face.h"
#include "main.h"
#include "trail.h"

/* The engine implements a WAM-like execution model for our search process.
   Each phase of the search is implemented as a predicate that can:
   - FAIL (backtrack)
   - SUCCESS (move to next predicate)
   - CHOICES (try alternatives)

   The engine maintains the state and controls the flow between predicates.
   This provides a uniform structure for all phases of the search. */

#define MAX_STACK_SIZE 1000

/* Predefined predicate results */
const struct predicateResult PredicateFail = {PREDICATE_FAIL, 0};
const struct predicateResult PredicateSuccessNextPredicate = {
    PREDICATE_SUCCESS_NEXT_PREDICATE, 0};
const struct predicateResult PredicateSuccessSamePredicate = {
    PREDICATE_SUCCESS_SAME_PREDICATE, 0};

/* Helper function to create a predicate result with choices */
struct predicateResult predicateChoices(int numberOfChoices)
{
  return (struct predicateResult){PREDICATE_CHOICES, numberOfChoices};
}

static void pushStackEntry(struct stackEntry* entry, PredicateResultCode code);
static void trace(const char* message);
static struct stackEntry stack[MAX_STACK_SIZE + 1], *stackTop = stack;
static int Counter = 0;
static bool engineLoop(void);

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

  if (!engineLoop() && Tracing) {
    fprintf(stderr, "Engine suspended\n");
  } else {
    assert(stackTop == stack || stackTop->predicate == &SUSPENDPredicate);
  }
}

/* Continue from the suspension point, with a new set of predicates.
  When the new predicates complete, we backtrack through
  the suspension point to resume the previous execution.
  This is intended for testing.
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

static void retryPort(void)
{
  PredicateResult result =
      stackTop->predicate->retry(stackTop->round, stackTop->currentChoice++);

  switch (result.code) {
    case PREDICATE_FAIL:
      trailBacktrackTo(stackTop->trail);
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

static bool engineLoop(void)
{
  while (true) {
    freeAll();
    trailBacktrackTo(stackTop->trail);
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
  entry->counter = Counter++;  // Increment counter
}

void trace(const char* message)
{
  if (!Tracing) return;
  fprintf(stderr, "%d:%ld:", stackTop->counter, (long)(stackTop - stack));
  if (stackTop->currentChoice >= 0) {
    fprintf(stderr, "%s(%d,%d) %s\n", message, stackTop->round,
            stackTop->currentChoice, stackTop->predicate->name);
  } else {
    fprintf(stderr, "%s(%d) %s\n", message, stackTop->round,
            stackTop->predicate->name);
  }
}

#define SIMPLE_PREDICATE(name)                            \
  static struct predicateResult try##name(int round)      \
  {                                                       \
    (void)round;                                          \
    return (struct predicateResult){PREDICATE_##name, 0}; \
  }                                                       \
  struct predicate name##Predicate = {#name, try##name, NULL};

SIMPLE_PREDICATE(FAIL)
SIMPLE_PREDICATE(SUSPEND)
