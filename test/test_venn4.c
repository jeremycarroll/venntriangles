/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unity.h>

/* Test setup and teardown */
void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/stdout", 20, 5);
}

void tearDown(void)
{
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

/* Global variables */
static int SolutionCount = 0;

/* Callback functions */
static void foundSolution() { SolutionCount++; }

/* Test functions */
static void testSearch()
{
  SolutionCount = 0;
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(FACTORIAL4, SolutionCount);
}

static void testSearchAbcd()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void testSearch4343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 3, 4, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void testSearch4433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 3, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

static void testPermutations()
{
  int size;
  PERMUTATION identity = d6Permutation(0, 1, 2, 3);
  TEST_ASSERT_TRUE(d6PermutationEqual(identity, d6Identity()));
  TEST_ASSERT_TRUE(d6PermutationEqual(identity, d6Compose(identity, identity)));
}

static void testS4()
{
  int size;
  PERMUTATION identity = d6Permutation(0, 1, 2, 3);
  PERMUTATION s4a, s4b;
  d6Closure(&size, 1, identity);
  TEST_ASSERT_EQUAL(1, size);
  TEST_ASSERT_TRUE(d6PermutationEqual(identity, d6Identity()));
  s4a =
      d6Closure(&size, 2, d6Permutation(1, 0, 2, 3), d6Permutation(1, 2, 3, 0));
  TEST_ASSERT_EQUAL(24, size);
  s4b = d6Closure(&size, 3, d6Permutation(1, 0, 2, 3),
                  d6Permutation(2, 1, 0, 3), d6Permutation(3, 1, 2, 0));
  TEST_ASSERT_EQUAL(24, size);
  TEST_ASSERT_EQUAL(0, memcmp(s4a, s4b, 24 * sizeof(((PERMUTATION)NULL)[0])));
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testSearch);
  RUN_TEST(testSearch4343);
  RUN_TEST(testSearchAbcd);
  RUN_TEST(testSearch4433);
  RUN_TEST(testS4);
  RUN_TEST(testPermutations);
  return UNITY_END();
}
