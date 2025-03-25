#include <stdlib.h>
#include <string.h>

#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "unity.h"
#include "utils.h"

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

static int solution_count = 0;
static void found_solution() { solution_count++; }

static void test_search()
{
  solution_count = 0;
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(FACTORIAL4, solution_count);
}

static void test_search_abcd()
{
  solution_count = 0;
  dynamicFaceSetupCentral(dynamicIntArray(0, 0, 0, 0));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(4, solution_count);
}

static void test_search_4343()
{
  solution_count = 0;
  dynamicFaceSetupCentral(dynamicIntArray(4, 3, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(0, solution_count);
}

static void test_search_4433()
{
  solution_count = 0;
  dynamicFaceSetupCentral(dynamicIntArray(4, 4, 3, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(1, solution_count);
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
