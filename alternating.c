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

bool AlternatingDebug = false;
static int MinimumEntrySet;

/* Sets the value, returning false if it breaks invariants. */
static bool dynamicSetRawEntry(AlternatingPredicate ap, uint_trail* entry)
{
  int roundedDownIx;
  // fprintf(stderr, "%p <= %p < %p:  n:%d\n", ap->rawStorage, entry,
  //         ap->rawStorage + ap->n * 2, ap->n);
  assert(entry >= ap->rawStorage);
  assert(entry < ap->rawStorage + SIGNED_TRIPLES(ap->n));
  if (MinimumEntrySet) {
    if (entry - ap->rawStorage < MinimumEntrySet) {
      MinimumEntrySet = entry - ap->rawStorage;
    }
  }
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
    TRAIL entry = Trail;
    if (!dynamicAlternatingSet(ap, i, j, l)) {
      if (AlternatingDebug) {
        printf("F: %d %d (%d) %d\n", i, j, k, l);
      }
      return false;
    }
    if (entry != Trail) {
      if (AlternatingDebug) {
        printf("S: %d %d (%d) %d\n", i, j, k, l);
      }
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
        if (*getAlternating(chirotope, i, j, k)) {
          printf("%d %d %d\n", i, j, k);
        }

        if (*getAlternating(chirotope, i, k, j)) {
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
typedef struct _extensibility {
  bool skip;
  bool skipPositive;
  bool skipNegative;
  bool positiveOK;
  bool negativeOK;
  int entryChoice;
}* Extensibility;

static struct _extensibility
    ExtensibilityByRound[SIGNED_TRIPLES((NCOLORS + 1) * 3)];
static uint_trail ExtensibilityMaxGuess;
static AlternatingPredicate extensibilitySearch;
#define EXTENSIBILITY_CASES 5

static PredicateResult tryExtensibility(int round)
{
  return predicateChoices(SIGNED_TRIPLES(extensibilitySearch->n) / 2 *
                          EXTENSIBILITY_CASES);
}

static PredicateResult dynamicRetryExtensibility(int round, int choice)
{
  int entryChoice = choice / EXTENSIBILITY_CASES;
  int caseChoice = choice % EXTENSIBILITY_CASES;
  Extensibility extensibility = &ExtensibilityByRound[round];
  assert(caseChoice == 0 ||
         extensibility->entryChoice == choice / EXTENSIBILITY_CASES);
  switch (caseChoice) {
    case 0:
      extensibility->entryChoice = choice / EXTENSIBILITY_CASES;
      extensibility->skip =
          extensibilitySearch->rawStorage[entryChoice * 2] ||
          extensibilitySearch->rawStorage[entryChoice * 2 + 1];
      if (extensibility->skip) {
        //   printf("Skipping %d %d\n", round, entryChoice);
      }
      return PredicateFail;
    case 1:
      if (!extensibility->skip) {
        uint_trail maxEarlierGuess = ExtensibilityMaxGuess;
        MinimumEntrySet = maxEarlierGuess;
        bool setOK = dynamicSetRawEntry(
            extensibilitySearch,
            &extensibilitySearch->rawStorage[entryChoice * 2]);
        assert(setOK);
        extensibility->positiveOK =
            dynamicAlternatingClosure(extensibilitySearch);
        extensibility->skipPositive =
            MinimumEntrySet < maxEarlierGuess || !extensibility->positiveOK;
      }
      return PredicateFail;
    case 2:
      if (!extensibility->skip) {
        uint_trail maxEarlierGuess = ExtensibilityMaxGuess;
        MinimumEntrySet = maxEarlierGuess;
        bool setOK = dynamicSetRawEntry(
            extensibilitySearch,
            &extensibilitySearch->rawStorage[entryChoice * 2 + 1]);
        assert(setOK);
        extensibility->negativeOK =
            dynamicAlternatingClosure(extensibilitySearch);
        extensibility->skipNegative =
            MinimumEntrySet < maxEarlierGuess || !extensibility->negativeOK;
        if (extensibility->negativeOK) {
          //    printf("%d: Negative: %d\n", round, entryChoice * 2 + 1);
        }
        MinimumEntrySet = 0;
      }
      return PredicateFail;
    case 3:
      if (!extensibility->skip) {
        if (!(extensibility->negativeOK || extensibility->positiveOK)) {
          // Can't extend here.
          char* inextensible = alternatingToString(extensibilitySearch);
          inextensible[entryChoice] = '*';
          printf("Inextensible: %s\n", inextensible);
        }
        extensibility->skip = entryChoice < ExtensibilityMaxGuess;
      }
      if (extensibility->skip || extensibility->skipPositive) {
        return PredicateFail;
      }
      break;
    case 4:
      if (extensibility->skip || extensibility->skipNegative) {
        return PredicateFail;
      }
      break;
    default:
      assert(0);
  }
  // caseChoice is 3 or 4; positive or negative.
  int sign = caseChoice - 3;  // 0 for positive, 1 for negative.
  int entry = entryChoice * 2 + sign;
  trailSetInt(&ExtensibilityMaxGuess, entryChoice * 2);

  // printf("%d: Setting: %d\n", round, entry);

  dynamicSetRawEntry(extensibilitySearch,
                     extensibilitySearch->rawStorage + entry);
  bool shouldBeOk = dynamicAlternatingClosure(extensibilitySearch);
  assert(shouldBeOk);
  return PredicateSuccessSamePredicate;
}
static struct predicate extensibilityPredicate = {
    "Extensibility", tryExtensibility, dynamicRetryExtensibility};

static PREDICATE extensibilityPredicates[] = {&extensibilityPredicate};
/*
 * Output is printed on stdout :(
 */
void dynamicAlternatingExtensibility(AlternatingPredicate ap)
{
  struct stack alternatingStack;
  extensibilitySearch = ap;
  bool failed = engine(&alternatingStack, extensibilityPredicates);
  assert(failed);
}

char* alternatingToString(AlternatingPredicate ap)
{
  char* result = tempMalloc(SIGNED_TRIPLES(ap->n) / 2 + 1);
  int i;
  for (i = 0; i < SIGNED_TRIPLES(ap->n); i += 2) {
    if (ap->rawStorage[i]) {
      result[i / 2] = '+';
    } else if (ap->rawStorage[i + 1]) {
      result[i / 2] = '-';
    } else {
      result[i / 2] = '?';
    }
  }
  result[i / 2] = 0;
  return result;
}
