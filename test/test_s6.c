/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "face.h"
#include "helper_for_tests.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>
void setUp(void)
{
  initializeS6();
  initializeStatisticLogging("/dev/stdout", 20, 5);
}

void tearDown(void)
{
  /* Nothing to tear down */
}

static int count6;

static struct predicateResult countSolutions(int round)
{
  (void)round;
  count6++;
  return PredicateFail;
}

static void testCanonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL, s6SymmetryType6(intArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL, s6SymmetryType6(intArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL, s6SymmetryType6(intArray(5, 5, 5, 4, 4, 4)));
}

static struct predicate countSolutionsPredicate = {"Count", countSolutions,
                                                   NULL};
static struct predicate* testProgram[] = {
    &InitializePredicate, &InnerFacePredicate, &countSolutionsPredicate};

static void testCallback()
{
  count6 = 0;
  engine(&TestStack, testProgram);
  TEST_ASSERT_EQUAL(56, count6);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCanonical6);
  RUN_TEST(testCallback);
  return UNITY_END();
}
