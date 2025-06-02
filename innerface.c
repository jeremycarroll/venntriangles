/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"
#include "predicates.h"
#include "s6.h"

/**
 * In a simple Venn diagram of convex curves, the faces inside k curves
 * have total face degree being 2 * nCk + nC(k-1), where nCk is the binomial
 * coefficient "n choose k". For n = 6 and k = 5, this equals 2 * 6 + 15 = 27.
 */
#define TOTAL_5FACE_DEGREE 27

extern FACE_DEGREE CurrentFaceDegrees[NCOLORS];
FACE_DEGREE CurrentFaceDegrees[NCOLORS];

static int sumFaceDegree(int round)
{
  int sum = 0;
  for (int i = 0; i < round; i++) {
    sum += CurrentFaceDegrees[i];
  }
  return sum;
}

static struct predicateResult dynamicTry5FaceDegree(int round)
{
  if (round == NCOLORS) {
    if (sumFaceDegree(round) != TOTAL_5FACE_DEGREE) {
      return PredicateFail;
    }
    if (s6SymmetryType6(CurrentFaceDegrees) == NON_CANONICAL) {
      return PredicateFail;
    }
    dynamicFaceSetupCentral(CurrentFaceDegrees);
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

  if (CentralFaceDegreesFlag[round] > 0 &&
      (FACE_DEGREE)degree != CentralFaceDegreesFlag[round]) {
    return PredicateFail;  // Skip if there's a specific degree required for
                           // this position
  }
  // Try this degree
  CurrentFaceDegrees[round] = degree;

  if (sumFaceDegree(round + 1) + 3 * (NCOLORS - round - 1) >
      TOTAL_5FACE_DEGREE) {
    return PredicateFail;  // Exceeded target sum
  }

  return PredicateSuccessSamePredicate;
}

struct predicate InnerFacePredicate = {"InnerFaces", dynamicTry5FaceDegree,
                                       retry5FaceDegree};
