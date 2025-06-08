/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "helper_for_tests.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  engine(&TestStack, (PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineClear(&TestStack);
}

static int SolutionCount = 0;

static struct predicateResult foundSolution()
{
  SolutionCount++;
  return PredicateFail;
}

static void testSearch()
{
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(FACTORIAL4, SolutionCount);
}

static void testSearchAbcd()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void testSearch4343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 3, 4, 3));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void testSearch4433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 3, 3));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    TracingFlag = true;
  }
  UNITY_BEGIN();
  RUN_TEST(testSearch);
  RUN_TEST(testSearch4343);
  RUN_TEST(testSearchAbcd);
  RUN_TEST(testSearch4433);
  return UNITY_END();
}
