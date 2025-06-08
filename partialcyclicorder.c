/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "partialcyclicorder.h"

#include "engine.h"
#include "visible_for_testing.h"
/* Storage for PCO relationships
 * For NCOLORS*MAX_CORNERS = 18 lines, we need to store
 * 18*17*16/3 = 816 relationships (where each relationship involves
 * 3 lines in order, these come in pairs) */

static uint_trail RawPartialCyclicOrder[PCO_TRIPLES];
static int DynamicPCOCompleteChoicePoints[PCO_TRIPLES / 2];
static uint_trail* PartialCyclicOrder[PCO_LINES][PCO_LINES][PCO_LINES];

/* Sets the value, returning false if it breaks invariants. */
static bool dynamicSetRawEntry(uint_trail* entry)
{
  int roundedDownIx;
  assert(entry >= RawPartialCyclicOrder);
  assert(entry < RawPartialCyclicOrder + PCO_TRIPLES);
  if (!trailMaybeSetInt(entry, true)) {
    return true;
  }
  roundedDownIx = ((entry - RawPartialCyclicOrder) / 2) * 2;
  return !(RawPartialCyclicOrder[roundedDownIx] &&
           RawPartialCyclicOrder[roundedDownIx + 1]);
}

void initializePartialCyclicOrder(void)
{
  int i, j, k;
  uint_trail* entry = RawPartialCyclicOrder;
  for (i = 0; i < PCO_LINES; i++) {
    for (j = i + 1; j < PCO_LINES; j++) {
      for (k = j + 1; k < PCO_LINES; k++) {
        PartialCyclicOrder[i][j][k] = PartialCyclicOrder[j][k][i] =
            PartialCyclicOrder[k][i][j] = entry;
        entry++;
        PartialCyclicOrder[i][k][j] = PartialCyclicOrder[j][i][k] =
            PartialCyclicOrder[k][j][i] = entry;
        entry++;
      }
    }
  }
}

uint_trail* getPartialCyclicOrder(int a, int b, int c)
{
  return PartialCyclicOrder[a][b][c];
}

/* return false if this breaks invariants. */
bool dynamicPCOSet(int i, int j, int k)
{
  return dynamicSetRawEntry(PartialCyclicOrder[i][j][k]);
}

/* Return false if invariants are violated. */
bool dynamicPCOClosure(void)
{
  int i, j, k, l;
  // extended Roy-Floyr-Warshall
  for (i = 0; i < PCO_LINES; i++) {
    for (k = 0; k < PCO_LINES; k++) {
      if (k != i) {
        for (j = 0; j < PCO_LINES; j++) {
          if (j != k && j != i) {
            if (*getPartialCyclicOrder(i, j, k)) {
              for (l = 0; l < PCO_LINES; l++) {
                if (l != i && l != k && l != j) {
                  if (*getPartialCyclicOrder(i, k, l)) {
                    // This uses trailMaybeSetInt which implements the
                    // inequality in the algorithm.
                    if (!dynamicSetRawEntry(getPartialCyclicOrder(i, j, l))) {
                      return false;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return true;
}

static PredicateResult tryPCOComplete(int round)
{
  for (int i = 0; i < PCO_TRIPLES / 2; i += 2) {
    if (!(RawPartialCyclicOrder[i] || RawPartialCyclicOrder[i + 1])) {
      DynamicPCOCompleteChoicePoints[round] = i;
      return predicateChoices(2);
    }
  }
  return PredicateSuccessNextPredicate;
}

static PredicateResult dynamicRetryPCOComplete(int round, int choice)
{
  dynamicSetRawEntry(RawPartialCyclicOrder +
                     DynamicPCOCompleteChoicePoints[round] + choice);
  if (dynamicPCOClosure()) {
    return PredicateSuccessSamePredicate;
  } else {
    return PredicateFail;
  }
}
static struct predicate complete = {"CompletePCO", tryPCOComplete,
                                    dynamicRetryPCOComplete};

static PREDICATE pcoPredicates[] = {&complete, &SUSPENDPredicate};
bool dynamicPCOComplete(void)
{
  struct stack pcoStack;
  bool failed = engine(&pcoStack, pcoPredicates);
  if (failed) {
    return false;
  }
  // engineClear(&pcoStack);
  return true;
}
