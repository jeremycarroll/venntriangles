/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "alternating.h"

#include "engine.h"
#include "visible_for_testing.h"
/* Storage for Alternating relationships
 * For NCOLORS*MAX_CORNERS = 18 lines, we need to store
 * 18*17*16/3 = 816 relationships (where each relationship involves
 * 3 lines in order, these come in pairs) */

/* Instance of AlternatingPredicate for the PCO */
AlternatingPredicate PartialCyclicOrder =
    CREATE_CYCLIC_PARTIAL_ORDER(PCO_LINES);

/* Sets the value, returning false if it breaks invariants. */
static bool dynamicSetRawEntry(AlternatingPredicate ap, uint_trail* entry)
{
  int roundedDownIx;
  // fprintf(stderr, "%p <= %p < %p:  n:%d\n", ap->rawStorage, entry,
  //         ap->rawStorage + ap->n * 2, ap->n);
  assert(entry >= ap->rawStorage);
  assert(entry < ap->rawStorage + SIGNED_TRIPLES(ap->n));
  if (!trailMaybeSetInt(entry, true)) {
    return true;
  }
  roundedDownIx = ((entry - ap->rawStorage) / 2) * 2;
  return !(ap->rawStorage[roundedDownIx] && ap->rawStorage[roundedDownIx + 1]);
}

void initializePartialCyclicOrder(void)
{
  initializeAlternating(PartialCyclicOrder);
}

static int entryPointerIndex(AlternatingPredicate ap, int i, int j, int k)
{
  return (i * ap->n + j) * ap->n + k;
}

void initializeAlternating(AlternatingPredicate ap)
{
  int i, j, k;
  uint_trail* entry = ap->rawStorage;

  for (i = 0; i < ap->n; i++) {
    for (j = i + 1; j < ap->n; j++) {
      for (k = j + 1; k < ap->n; k++) {
        ap->entryPointers[entryPointerIndex(ap, i, j, k)] =
            ap->entryPointers[entryPointerIndex(ap, j, k, i)] =
                ap->entryPointers[entryPointerIndex(ap, k, i, j)] = entry;
        entry++;
        ap->entryPointers[entryPointerIndex(ap, i, k, j)] =
            ap->entryPointers[entryPointerIndex(ap, j, i, k)] =
                ap->entryPointers[entryPointerIndex(ap, k, j, i)] = entry;
        entry++;
      }
    }
  }
  assert(entry == ap->rawStorage + SIGNED_TRIPLES(ap->n));
}

uint_trail* getAlternating(AlternatingPredicate ap, int a, int b, int c)
{
  return ap->entryPointers[entryPointerIndex(ap, a, b, c)];
}

/* return false if this breaks invariants. */
bool dynamicAlternatingSet(AlternatingPredicate ap, int i, int j, int k)
{
  uint_trail* entry = getAlternating(ap, i, j, k);
  return dynamicSetRawEntry(ap, entry);
}

extern bool dynamicCyclicPartialOrderStep(AlternatingPredicate ap, int i, int j,
                                          int k, int l)
{
  if (*getAlternating(ap, i, j, k) && *getAlternating(ap, i, k, l)) {
    // This uses trailMaybeSetInt which implements the
    // inequality in the algorithm.
    if (!dynamicAlternatingSet(ap, i, j, l)) {
      return false;
    }
  }
  return true;
}

/**
 * (𝜒(a,b,x) & 𝜒(c,d,x)) | (𝜒(b,a,x) & 𝜒(d,c,x))
 */
static bool sameOrder(AlternatingPredicate self, int a, int b, int c, int d,
                      int x)
{
  if (*getAlternating(self, a, b, x) && *getAlternating(self, c, d, x)) {
    return true;
  }
  if (*getAlternating(self, b, a, x) && *getAlternating(self, d, c, x)) {
    return true;
  }
  return false;
}

/*
 * We use the 3 term Grassmann-Plücker axiomatization of chirotopes,
 * adjusted for uniform oriented matroids only.
 * Checks if the chirotope conditions are met for indices a,b,c,d,x.
 * Returns true if any of the four rules indicates 𝜒(a,b,x) should be set.
 *
 * From the bible p138, with r = 3.
 * For any x₁ [a] x₂ [b] x₃ [x] y₁ [c] y₂ [d]
 * if 𝜒(y₁,x₂,x₃) ⋅ 𝜒(x₁,y₂,x₃) ≥ 0
 * and 𝜒(y₂,x₂,x₃) ⋅ 𝜒(y₁,x₁,x₃) ≥ 0
 * then (x₁,x₂,x₃) ⋅ 𝜒(y₁,y₂,x₃) ≥ 0

 * i.e.
 * if 𝜒(c,b,x) ⋅ 𝜒(a,d,x) ≥ 0
 * and 𝜒(d,b,x) ⋅ 𝜒(c,a,x) ≥ 0
 * then 𝜒(a,b,x) ⋅ 𝜒(c,d,x) ≥ 0

 * Looking at uniform case only, ignore 0, then
 */
static bool chirotopeCondition(AlternatingPredicate self, int a, int b, int c,
                               int d, int x)
{
  /* x must be different from all other indices */
  if (x == a || x == b || x == c || x == d) {
    return false;
  }

  /* Match one of these four rules.

𝜒(c,d,x), 𝜒(a,c,x), 𝜒(a,d,x), 𝜒(b,d,x), 𝜒(c,b,x)  ⇒ 𝜒(a,b,x) [1]
𝜒(c,d,x), 𝜒(a,c,x), 𝜒(b,c,x), 𝜒(b,d,x), 𝜒(d,a,x)  ⇒ 𝜒(a,b,x) [2]
𝜒(c,d,x), 𝜒(a,d,x), 𝜒(c,a,x), 𝜒(c,b,x), 𝜒(d,b,x)  ⇒ 𝜒(a,b,x) [3]
𝜒(c,d,x), 𝜒(b,c,x), 𝜒(c,a,x), 𝜒(d,a,x), 𝜒(d,b,x)  ⇒ 𝜒(a,b,x) [4]
  */

  /* Common condition: 𝜒(c,d,x) must be true for all rules */
  if (!*getAlternating(self, c, d, x)) {
    return false;
  }

  /*
   * [1] and [2] have 𝜒(a,c,x),𝜒(b,d,x)
   * [3] and [4] have 𝜒(c,a,x),𝜒(d,b,x)
   */
  if (!sameOrder(self, a, c, b, d, x)) {
    return false;
  }
  /*
   * [1] and [3] have 𝜒(a,d,x),𝜒(c,b,x)
   * [2] and [4] have 𝜒(d,a,x),𝜒(b,c,x)
   */
  return sameOrder(self, a, d, c, b, x);
}

bool dynamicChirotopeStep(AlternatingPredicate self, int a, int b, int c, int d)
{
  for (int x = 0; x < self->n; x++) {
    if (chirotopeCondition(self, a, b, c, d, x)) {
      // printf("Checking %d %d %d\n", a, b, x);
      if (false && (a - 6) * (b - 6) * (x - 6) == 0 &&
          (a - 4) * (b - 4) * (x - 4) == 0 &&
          (a - 2) * (b - 2) * (x - 2) == 0) {
        printf(">> %d %d %d %d %d\n", a, b, c, d, x);
        printf("sameOrder(self, a, c, b, d, x) = %d\n",
               sameOrder(self, a, c, b, d, x));
        printf("sameOrder(self, a, d, c, b, x) = %d\n",
               sameOrder(self, a, d, c, b, x));
      }
      if (!dynamicAlternatingSet(self, a, b, x)) {
        return false;
      }
    }
  }
  return true;
}

/* Return false if invariants are violated. */
static bool internalDynamicAlternatingClosure(AlternatingPredicate ap)
{
  int i, j, k, l;
  // extended Roy-Floyd-Warshall
  for (i = 0; i < ap->n; i++) {
    for (k = 0; k < ap->n; k++) {
      if (k != i) {
        for (j = 0; j < ap->n; j++) {
          if (j != k && j != i) {
            for (l = 0; l < ap->n; l++) {
              if (l != i && l != k && l != j) {
                if (!ap->dynamicOneClosureStep(ap, i, j, k, l)) {
                  return false;
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
/* Return false if invariants are violated. */
bool dynamicAlternatingClosure(AlternatingPredicate ap)
{
  bool result = true;
  while (result) {
    TRAIL check = Trail;
    result = internalDynamicAlternatingClosure(ap);
    if (check == Trail) {
      break;
    }
  }
  return result;
}

void debugAlternating(AlternatingPredicate chirotope)
{
  for (int i = 0; i < chirotope->n; i++) {
    for (int j = i + 1; j < chirotope->n; j++) {
      for (int k = j + 1; k < chirotope->n; k++) {
        bool positive = *getAlternating(chirotope, i, j, k);
        bool negative = *getAlternating(chirotope, i, k, j);
        if (positive && negative) {
          printf("*** ");
        }
        if (positive) {
          printf("%d %d %d\n", i, j, k);
        }

        if (negative) {
          printf("%d %d %d\n", i, k, j);
        }
      }
    }
  }
}

static AlternatingPredicate alternatingSearch;
static int
    DynamicAlternatingCompleteChoicePoints[SIGNED_TRIPLES((NCOLORS + 1) * 3)];
static PredicateResult tryAlternatingComplete(int round)
{
  for (int i = 0; i < SIGNED_TRIPLES(alternatingSearch->n); i += 2) {
    if (!(alternatingSearch->rawStorage[i] ||
          alternatingSearch->rawStorage[i + 1])) {
      DynamicAlternatingCompleteChoicePoints[round] = i;
      return predicateChoices(2);
    }
  }
  return PredicateSuccessNextPredicate;
}

static PredicateResult dynamicRetryAlternatingComplete(int round, int choice)
{
  dynamicSetRawEntry(alternatingSearch,
                     alternatingSearch->rawStorage +
                         DynamicAlternatingCompleteChoicePoints[round] +
                         choice);
  if (dynamicAlternatingClosure(alternatingSearch)) {
    return PredicateSuccessSamePredicate;
  } else {
    return PredicateFail;
  }
}

static struct predicate complete = {"CompleteAlternating",
                                    tryAlternatingComplete,
                                    dynamicRetryAlternatingComplete};

static PREDICATE alternatingPredicates[] = {&complete, &SUSPENDPredicate};

bool dynamicAlternatingComplete(AlternatingPredicate ap)
{
  struct stack alternatingStack;
  alternatingSearch = ap;
  bool failed = engine(&alternatingStack, alternatingPredicates);
  engineClear(&alternatingStack);
  if (failed) {
    return false;
  }
  return true;
}
