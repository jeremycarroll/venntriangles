#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

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

static int SolutionCount = 0;
static void found_solution() { SolutionCount++; }

static void test_search_abcde()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(0, 0, 0, 0, 0));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(152, SolutionCount);
}

static void test_search_44444()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(4, 4, 4, 4, 4));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(2, SolutionCount);
}

static void test_search_55433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 5, 4, 3, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(6, SolutionCount);
}

static void test_search_55343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 5, 3, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void test_search_54443()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 4, 4, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void test_search_54434()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 4, 4, 3, 4));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(5, SolutionCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search_abcde);
  RUN_TEST(test_search_44444);
  RUN_TEST(test_search_55343);
  RUN_TEST(test_search_55433);
  RUN_TEST(test_search_54443);
  RUN_TEST(test_search_54434);

  return UNITY_END();
}
