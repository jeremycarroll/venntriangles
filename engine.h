/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef ENGINE_H
#define ENGINE_H

#include "trail.h"

/* The engine implements a WAM-like execution model for our search process.
   Each phase of the search is implemented as a predicate that can:
   - FAIL (backtrack)
   - SUCCESS (move to next predicate)
   - CHOICES (try alternatives)

   The engine maintains the state and controls the flow between predicates.
   This provides a uniform structure for all phases of the search. */

typedef enum {
  PREDICATE_FAIL,
  PREDICATE_SUCCESS_NEXT_PREDICATE,
  PREDICATE_SUCCESS_SAME_PREDICATE,
  PREDICATE_CHOICES,
  PREDICATE_SUSPEND
} PredicateResultCode;

struct choicePoint {
  int numberOfChoices;
  void* choices;
};
typedef struct predicateResult {
  PredicateResultCode code;
  struct choicePoint choicePoint;
} PredicateResult;

/* Predefined predicate results */
extern const struct predicateResult PredicateFail;
extern const struct predicateResult PredicateSuccessNextPredicate;
extern const struct predicateResult PredicateSuccessSamePredicate;

/* Helper function to create a predicate result with choices */
struct predicateResult predicateChoices(int numberOfChoices, void* choices);

/* A predicate has two methods:
   try:   First attempt to satisfy the predicate. Can return:
          - FAIL: Backtrack to previous predicate
          - SUCCESS: Move to next predicate
          - CHOICES: Try alternatives in this predicate

   retry: Try next choice after CHOICES was returned. Can only return:
          - FAIL: Try next choice (if any)
          - SUCCESS: Move to next round of the same predicate.
          Note: retry cannot return CHOICES as we are already in choice mode

  For both methods, the round is simply an integer that is incremented by
  the engine from 0 upwards. This allows a predicate to do the same thing
  multiple times.
 */
typedef struct predicate {
  const char* name;  // Name of the predicate for debugging
  PredicateResult (*try)(int round);
  PredicateResult (*retry)(int round, int choice);
}* PREDICATE;

extern struct predicate FAILPredicate, SUSPENDPredicate;

/* Stack entry for the engine's backtracking stack */
struct stackEntry {
  bool inChoiceMode;
  struct predicate* predicate;
  struct predicate** predicates;
  int currentChoice;
  int round;
  TRAIL trail;
  int counter;                     // Counter for tracing
  struct choicePoint choicePoint;  // For storing choice information
};

/* Runs each predicate in turn. The predicates argument is a null-terminated
   array of predicates. The callback is called each time all predicates succeed.
 */
extern void engine(PREDICATE* predicates, void (*callback)(void));
void engineResume(PREDICATE* predicates);

#define FORWARD_BACKWARD_PREDICATE_SCOPE(scope, name, gate, forward, backward) \
  static PredicateResult try##name(int round)                                  \
  {                                                                            \
    (void)round;                                                               \
    bool (*gatingFunction)(void) = gate;                                       \
    if (gatingFunction && !gatingFunction()) {                                 \
      return PredicateFail;                                                    \
    }                                                                          \
    return predicateChoices(2, NULL);                                          \
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

#define FORWARD_BACKWARD_PREDICATE(name, gate, forward, backward)          \
  FORWARD_BACKWARD_PREDICATE_SCOPE(/* deliberately missing */, name, gate, \
                                   forward, backward)
#define FORWARD_BACKWARD_PREDICATE_STATIC(name, gate, forward, backward) \
  FORWARD_BACKWARD_PREDICATE_SCOPE(static, name, gate, forward, backward)

#endif /* ENGINE_H */
