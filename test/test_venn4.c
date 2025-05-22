/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  engine((PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineResume((PREDICATE[]){&FAILPredicate});
}

/* Global variables */
static int SolutionCount = 0;

static struct predicateResult foundSolution()
{
  SolutionCount++;
  return PredicateFail;
}

/* Test functions */
static void testSearch()
{
  engineResume((PREDICATE[]){
      &VennPredicate, &(struct predicate){"Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(FACTORIAL4, SolutionCount);
}

static void testSearchAbcd()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0));
  engineResume((PREDICATE[]){
      &VennPredicate, &(struct predicate){"Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void testSearch4343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 3, 4, 3));
  engineResume((PREDICATE[]){
      &VennPredicate, &(struct predicate){"Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void testSearch4433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 3, 3));
  engineResume((PREDICATE[]){
      &VennPredicate, &(struct predicate){"Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

/* Main test runner */
int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    Tracing = true;
  }
  UNITY_BEGIN();
  RUN_TEST(testSearch);
  RUN_TEST(testSearch4343);
  RUN_TEST(testSearchAbcd);
  RUN_TEST(testSearch4433);
  return UNITY_END();
}
