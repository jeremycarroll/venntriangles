#include "face.h"
#include "statistics.h"
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

static void test_search_abcde()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(0, 0, 0, 0, 0));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(152, solution_count);
}

static void test_search_44444()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(4, 4, 4, 4, 4));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(2, solution_count);
}

static void test_search_55433()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 5, 4, 3, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(6, solution_count);
}

static void test_search_55343()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 5, 3, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(0, solution_count);
}

static void test_search_54443()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 4, 4, 4, 3));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(4, solution_count);
}

static void test_search_54434()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 4, 4, 3, 4));
  dynamicSearch(false, found_solution);
  TEST_ASSERT_EQUAL(5, solution_count);
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
