/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"

#include "core.h"
#include "face.h"
#include "main.h"
#include "statistics.h"
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
const struct predicateResult PredicateFail = {PREDICATE_FAIL, {0, NULL}};
const struct predicateResult PredicateSuccessNextPredicate = {
    PREDICATE_SUCCESS_NEXT_PREDICATE, {0, NULL}};
const struct predicateResult PredicateSuccessSamePredicate = {
    PREDICATE_SUCCESS_SAME_PREDICATE, {0, NULL}};

/* Helper function to create a predicate result with choices */
struct predicateResult predicateChoices(int numberOfChoices, void* choices)
{
  return (struct predicateResult){PREDICATE_CHOICES,
                                  {numberOfChoices, choices}};
}

static void pushStackEntry(struct stackEntry* stack, PredicateResultCode code);

void engine(struct predicate** predicates, void (*callback)(void))
{
  struct stackEntry stack[MAX_STACK_SIZE + 1], *stackTop = stack;

  // Initialize first stack entry
  stackTop->inChoiceMode = false;
  stackTop->predicate = *predicates;
  stackTop->predicates = predicates;
  stackTop->currentChoice = 0;
  stackTop->round = 0;
  stackTop->trail = Trail;

  while (true) {
    freeAll();  // Malloced memory is for temporary use only.
    if (stackTop->predicate == NULL) {
      // We've reached the end of the predicates array successfully
      callback();
    backtrack:
      // Backtrack to previous choice point
      do {
        if (stackTop == stack) {
          return;  // All done
        }
    //    trace("fail");
        stackTop--;
      } while (!stackTop->inChoiceMode);
      continue;
    }

    PredicateResult result;

    trailBacktrackTo(stackTop->trail);
    if (!stackTop->inChoiceMode) {
      // Try the current predicate
      result = stackTop->predicate->try(stackTop->round);

      switch (result.code) {
        case PREDICATE_FAIL:
          goto backtrack;

        case PREDICATE_SUCCESS_NEXT_PREDICATE:
        case PREDICATE_SUCCESS_SAME_PREDICATE:
          pushStackEntry(++stackTop, result.code);
          assert(stackTop < stack + MAX_STACK_SIZE);
          break;

        case PREDICATE_CHOICES:
          // Start trying choices
          stackTop->inChoiceMode = true;
          stackTop->currentChoice = 0;
          stackTop->choicePoint = result.choicePoint;
          stackTop->trail = Trail;
          break;
      }
    } else {
      // Try next choice
      if (stackTop->currentChoice >= stackTop->choicePoint.numberOfChoices) {
        goto backtrack;
      }
      result = stackTop->predicate->retry(stackTop->round,
                                          stackTop->currentChoice++);

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
          assert(false);
          break;
      }
    }
  }
}

static void pushStackEntry(struct stackEntry* stack, PredicateResultCode code)
{
  stack->inChoiceMode = false;
  stack->predicates = stack[-1].predicates;
  stack->predicates = code == PREDICATE_SUCCESS_NEXT_PREDICATE
                          ? stack[-1].predicates + 1
                          : stack[-1].predicates;
  stack->predicate = *stack->predicates;
  stack->round =
      code == PREDICATE_SUCCESS_NEXT_PREDICATE ? 0 : stack[-1].round + 1;
  stack->currentChoice = 0;
  stack->trail = Trail;
}

/* Predicate that always fails - useful for terminating search paths */
static struct predicateResult tryFail(int round)
{
  (void)round;  // Unused parameter
  return PredicateFail;
}

/* The fail predicate - always fails */
struct predicate failPredicate = {"Fail", tryFail, NULL};
