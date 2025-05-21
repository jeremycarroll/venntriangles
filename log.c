/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"
#include "predicates.h"
#include "save.h"
#include "statistics.h"
#include "vsearch.h"

extern FACE_DEGREE CurrentFaceDegrees[NCOLORS];
static clock_t TotalWastedTime = 0;
static clock_t TotalUsefulTime = 0;
static int WastedSearchCount = 0;
static int UsefulSearchCount = 0;
static int FacePredicateRecentSolutionsFound = 0;
static int FacePredicateInitialVariationCount = 0;
static clock_t FacePredicateStart = 0;
static bool forwardLog(void)
{
  FacePredicateStart = clock();
  FacePredicateRecentSolutionsFound = GlobalSolutionsFound;
  FacePredicateInitialVariationCount = VariationCount;
  PerFaceDegreeSolutionNumber = 0;
  return true;
}

static void backwardLog(void)
{
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
    for (int i = 0; i < NCOLORS; i++) {
      printf("%llu ", CurrentFaceDegrees[i]);
    }
    printf(" gives %llu/%d new solutions\n",
           GlobalSolutionsFound - FacePredicateRecentSolutionsFound,
           VariationCount - FacePredicateInitialVariationCount);
    statisticPrintOneLine(0, false);
  } else {
    WastedSearchCount += 1;
    TotalWastedTime += used;
  }
  FacePredicateRecentSolutionsFound = GlobalSolutionsFound;
}

FORWARD_BACKWARD_PREDICATE(Log, NULL, forwardLog, backwardLog);
