/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "main.h"
#include "predicates.h"
#include "statistics.h"

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
  FacePredicateRecentSolutionsFound = GlobalSolutionsFoundIPC;
  FacePredicateInitialVariationCount = VariationCountIPC;
  return true;
}

static void backwardLog(void)
{
  clock_t used = clock() - FacePredicateStart;
  if ((int64_t)GlobalSolutionsFoundIPC != FacePredicateRecentSolutionsFound) {
    TotalUsefulTime += used;
    UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    if (VerboseModeFlag) {
      PRINT_TIME(used, 0);
      PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
      PRINT_TIME(TotalWastedTime, WastedSearchCount);
    }
    for (int i = 0; i < NCOLORS; i++) {
      printf("%llu ", CurrentFaceDegrees[i]);
    }
    printf(" gives %llu/%d new solutions\n",
           GlobalSolutionsFoundIPC - FacePredicateRecentSolutionsFound,
           VariationCountIPC - FacePredicateInitialVariationCount);
    statisticPrintOneLine(0, false);
  } else {
    WastedSearchCount += 1;
    TotalWastedTime += used;
  }
  FacePredicateRecentSolutionsFound = GlobalSolutionsFoundIPC;
}

FORWARD_BACKWARD_PREDICATE(Log, NULL, forwardLog, backwardLog);
