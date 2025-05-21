/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"

/* Global variables - file scoped */
/* Lemma: In a simple Venn diagram of convex curves, the faces inside k curves
 * have total face degree being 2 * nCk + nC(k-1), where nCk is the binomial
 * coefficient "n choose k". For n = 6 and k = 5, this equals 2 * 6 + 15 = 27.
 * This lemma is part of an ongoing proof about the structure of Venn diagrams.
 */
#define TOTAL_5FACE_DEGREE 27

static FACE_DEGREE currentArgs[NCOLORS];

static int sumFaceDegree(int round)
{
  int sum = 0;
  for (int i = 0; i < round; i++) {
    sum += currentArgs[i];
  }
  return sum;
}

static struct predicateResult try5FaceDegree(int round)
{
  if (round == NCOLORS) {
    if (sumFaceDegree(round) != TOTAL_5FACE_DEGREE) {
      return PredicateFail;
    }
    if (s6SymmetryType6(currentArgs) == NON_CANONICAL) {
      return PredicateFail;
    }
    dynamicFaceSetupCentral(currentArgs);
    return PredicateSuccessNextPredicate;
  }
  return predicateChoices(NCOLORS - 2);
}

static struct predicateResult retry5FaceDegree(int round, int choice)
{
  // Try each possible face degree >= 3 for the current position
  int degree = NCOLORS - choice;  // Start with NCOLORS and go down to 3
  if (degree < 3) {
    return PredicateFail;
  }

  if (CentralFaceDegrees[round] > 0 &&
      (FACE_DEGREE)degree != CentralFaceDegrees[round]) {
    return PredicateFail;  // Skip if there's a specific degree required for
                           // this position
  }
  // Try this degree
  currentArgs[round] = degree;

  if (sumFaceDegree(round + 1) + 3 * (NCOLORS - round - 1) >
      TOTAL_5FACE_DEGREE) {
    return PredicateFail;  // Exceeded target sum
  }

  return PredicateSuccessSamePredicate;
}

struct predicate InnerFacePredicate = {"InnerFaces", try5FaceDegree,
                                       retry5FaceDegree};
