#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
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

static void test_search()
{
  SolutionCount = 0;
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(FACTORIAL4, SolutionCount);
}

static void test_search_abcd()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(0, 0, 0, 0));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void test_search_4343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(4, 3, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void test_search_4433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(4, 4, 3, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search);
  RUN_TEST(test_search_4343);
  RUN_TEST(test_search_abcd);
  RUN_TEST(test_search_4433);
  return UNITY_END();
}
