/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef ENGINE_H
#define ENGINE_H

#include "trail.h"

/**
 * Search engine with backtracking support for Venn diagram generation.
 *
 * The engine implements a WAM-like execution model for the search process.
 * Each phase of the search is implemented as a predicate that can:
 * - FAIL (backtrack)
 * - SUCCESS (move to next predicate)
 * - CHOICES (try alternatives)
 *
 * The engine maintains the state and controls the flow between predicates.
 * This provides a uniform structure for all phases of the search.
 */

/*--------------------------------------
 * Predicate Result Types and Constants
 *--------------------------------------*/

/* Possible result codes from predicate execution */
typedef enum {
  PREDICATE_FAIL,                   /* Backtrack to previous predicate */
  PREDICATE_SUCCESS_NEXT_PREDICATE, /* Continue to next predicate */
  PREDICATE_SUCCESS_SAME_PREDICATE, /* Continue with same predicate, next round
                                     */
  PREDICATE_CHOICES, /* Predicate has multiple choices to explore */
  PREDICATE_SUSPEND  /* Pause execution, can be resumed later */
} PredicateResultCode;

/* Result structure returned by predicates */
typedef struct predicateResult {
  PredicateResultCode code; /* Result type */
  int numberOfChoices; /* When code is PREDICATE_CHOICES, number of options */
} PredicateResult;

/* Predefined predicate results */
extern const struct predicateResult PredicateFail;
extern const struct predicateResult PredicateSuccessNextPredicate;
extern const struct predicateResult PredicateSuccessSamePredicate;

/* Helper function to create a predicate result with choices */
struct predicateResult predicateChoices(int numberOfChoices);

/*--------------------------------------
 * Predicate Structure
 *--------------------------------------*/

/**
 * A predicate has two methods:
 *   try:   First attempt to satisfy the predicate. Can return:
 *          - FAIL: Backtrack to previous predicate
 *          - SUCCESS: Move to next predicate
 *          - CHOICES: Try alternatives in this predicate
 *
 *   retry: Try next choice after CHOICES was returned. Can only return:
 *          - FAIL: Try next choice (if any)
 *          - SUCCESS: Move to next round of the same predicate.
 *          Note: retry cannot return CHOICES as we are already in choice mode
 *
 * For both methods, the round is simply an integer that is incremented by
 * the engine from 0 upwards. This allows a predicate to do the same thing
 * multiple times.
 */
typedef struct predicate {
  const char* name;                  /* Name for debugging */
  PredicateResult (*try)(int round); /* Initial attempt function */
  PredicateResult (*retry)(int round,
                           int choice); /* Function for trying alternatives */
}* PREDICATE;

struct stackEntry {
  bool inChoiceMode;             /* Whether we're exploring alternatives */
  struct predicate* predicate;   /* Current predicate being executed */
  struct predicate** predicates; /* Full predicate sequence */
  int currentChoice;             /* Current alternative being tried */
  int round;                     /* Current round in this predicate */
  TRAIL trail;                   /* Backtracking trail */
  int counter;                   /* Counter for tracing */
  int numberOfChoices;           /* Total alternatives in this predicate */
};

#define MAX_STACK_SIZE 1000

typedef struct stack {
  struct stackEntry* stackTop;
  struct stackEntry stack[MAX_STACK_SIZE + 1];
}* STACK;
/* Predefined predicates for ending search sequences */
extern struct predicate FAILPredicate;    /* Always forces backtracking */
extern struct predicate SUSPENDPredicate; /* Pauses execution */

/*--------------------------------------
 * Engine API
 *--------------------------------------*/

/**
 * Runs each predicate in turn. The predicates array must be terminated with
 * a predicate that never succeeds with PREDICATE_SUCCESS_NEXT_PREDICATE,
 * typically FAILPredicate or SUSPENDPredicate.
 */
extern bool engine(STACK stack, PREDICATE* predicates);

/**
 * Resumes execution from a previously suspended state.
 */
extern void engineResume(STACK stack, PREDICATE* predicates);

extern void engineClear(STACK stack);

/*--------------------------------------
 * Predicate Definition Macros
 *--------------------------------------*/

/**
 * Creates a predicate with forward and backward execution paths.
 * These macros simplify the creation of predicates that follow a
 * standard pattern of forward execution followed by the other predicates
 * and then backward execution.
 * - the optional gate can return false to completely fail the predicate
 *   (omitting the backward call).
 * - the optional forward can return false to not proceed to the
 *   other predicates but to fail to the backward call.
 * - the optional backward is void, and is executed while backtracking
 *   to the previous predicate.
 */

#define FORWARD_BACKWARD_PREDICATE_SCOPE(scope, name, gate, forward, backward) \
  static PredicateResult try##name(int round)                                  \
  {                                                                            \
    (void)round;                                                               \
    bool (*gatingFunction)(void) = gate;                                       \
    if (gatingFunction && !gatingFunction()) {                                 \
      return PredicateFail;                                                    \
    }                                                                          \
    return predicateChoices(2);                                                \
  }                                                                            \
  static PredicateResult retry##name(int round, int choice)                    \
  {                                                                            \
    (void)round;                                                               \
    bool (*forwardFunction)(void) = forward;                                   \
    void (*backwardFunction)(void) = backward;                                 \
    switch (choice) {                                                          \
      case 0:                                                                  \
        if (forwardFunction && !forwardFunction()) {                           \
          return PredicateFail;                                                \
        }                                                                      \
        return PredicateSuccessNextPredicate;                                  \
      case 1:                                                                  \
        if (backwardFunction) {                                                \
          backwardFunction();                                                  \
        }                                                                      \
        return PredicateFail;                                                  \
      default:                                                                 \
        assert(0);                                                             \
    }                                                                          \
  }                                                                            \
  scope struct predicate name##Predicate = {#name, try##name, retry##name};

/* Creates a global predicate */
#define FORWARD_BACKWARD_PREDICATE(name, gate, forward, backward)          \
  FORWARD_BACKWARD_PREDICATE_SCOPE(/* deliberately missing */, name, gate, \
                                   forward, backward)

/* Creates a static predicate */
#define FORWARD_BACKWARD_PREDICATE_STATIC(name, gate, forward, backward) \
  FORWARD_BACKWARD_PREDICATE_SCOPE(static, name, gate, forward, backward)

#endif /* ENGINE_H */
