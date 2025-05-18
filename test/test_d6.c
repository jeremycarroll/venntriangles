/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "face.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

/* Test setup and teardown */
void setUp(void)
{
  initializeS6();
  initializeStatisticLogging("/dev/stdout", 20, 5);
}

void tearDown(void)
{
  // Tear down code if needed
}

/* Global variables */
static int count6;

/* Callback functions */
static struct predicateResult countSolutions(int round)
{
  count6++;
  return PredicateFail;
}

/* Test functions */
static void testCanonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL, s6SymmetryType6(intArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL, s6SymmetryType6(intArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL, s6SymmetryType6(intArray(5, 5, 5, 4, 4, 4)));
}

/* The test program - just initialization and face degrees */
static struct predicate countSolutionsPredicate = {"Count", countSolutions,
                                                   NULL};
static struct predicate* testProgram[] = {
    &InitializePredicate, &faceDegreePredicate, &countSolutionsPredicate};

static void testCallback()
{
  count6 = 0;
  engine(testProgram);
  TEST_ASSERT_EQUAL(56, count6);
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCanonical6);
  RUN_TEST(testCallback);
  return UNITY_END();
}
