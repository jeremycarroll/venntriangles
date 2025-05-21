/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "face.h"
#include "failure.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "save.h"
#include "statistics.h"
#include "utils.h"
#include "vsearch.h"
static clock_t TotalWastedTime = 0;
static clock_t TotalUsefulTime = 0;
static int WastedSearchCount = 0;
static int UsefulSearchCount = 0;
static int FacePredicateRecentSolutionsFound = 0;
static int FacePredicateInitialVariationCount = 0;
static clock_t FacePredicateStart = 0;
static FACE facesInOrderOfChoice[NFACES];

static CYCLE chooseCycle(FACE face, CYCLE cycle)
{
  return cycleSetNext(face->possibleCycles, cycle);
}

static struct predicateResult tryFace(int round)
{
  if (round == 0) {
    FacePredicateStart = clock();
    FacePredicateRecentSolutionsFound = GlobalSolutionsFound;
    FacePredicateInitialVariationCount = VariationCount;
    PerFaceDegreeSolutionNumber = 0;
#if NCOLORS > 4
    dynamicFaceSetupCentral(CentralFaceDegrees);
#endif
  }
  if ((int64_t)GlobalSolutionsFound >= GlobalMaxSolutions) {
    return PredicateFail;
  }
  facesInOrderOfChoice[round] = searchChooseNextFace();
  if (facesInOrderOfChoice[round] == NULL) {
    if (faceFinalCorrectnessChecks() == NULL) {
      GlobalSolutionsFound++;
      PerFaceDegreeSolutionNumber++;
      return PredicateSuccessNextPredicate;
    } else {
      return PredicateFail;
    }
  }
  return predicateChoices(facesInOrderOfChoice[round]->cycleSetSize + 1);
}

static struct predicateResult retryFace(int round, int choice)
{
  (void)choice;
  FACE face = facesInOrderOfChoice[round];
  // Not on trail, otherwise it would get unset before the next retry.
  face->cycle = chooseCycle(face, face->cycle);
  if (face->cycle == NULL) {
    clock_t used = clock() - FacePredicateStart;
    if ((int64_t)GlobalSolutionsFound != FacePredicateRecentSolutionsFound) {
      TotalUsefulTime += used;
      UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
      if (VerboseMode) {
        PRINT_TIME(used, 0);
        PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
        PRINT_TIME(TotalWastedTime, WastedSearchCount);
      }
#if 0
      for (int i = 0; i < NCOLORS; i++) {
        printf("%llu ", CentralFaceDegrees[i]);
      }
      printf(" gives %llu/%d new solutions\n",
             GlobalSolutionsFound - FacePredicateRecentSolutionsFound,
             VariationCount - FacePredicateInitialVariationCount);
      statisticPrintOneLine(0, true);
#endif
    } else {
      WastedSearchCount += 1;
      TotalWastedTime += used;
    }
    FacePredicateRecentSolutionsFound = GlobalSolutionsFound;
    return PredicateFail;
  }
  if (dynamicFaceBacktrackableChoice(face) == NULL) {
    return PredicateSuccessSamePredicate;
  }
  return PredicateFail;
}

struct predicate VennPredicate = {"Venn", tryFace, retryFace};
