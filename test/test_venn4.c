#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 0

void setUp(void)
{
  initialize();
  initializeStatsLogging(NULL, 20, 0);
}

void tearDown(void)
{
  printStatisticsFull();
  clearGlobals();
  clearInitialize();
  resetTrail();
  resetStatistics();
}

static int solution_count = 0;
static void found_solution() { solution_count++; }

static void test_search()
{
  solution_count = 0;
  search(false, found_solution);
  TEST_ASSERT_EQUAL(FACTORIAL4, solution_count);
}

static void test_search_abcd()
{
  solution_count = 0;
  setupCentralFaces(0, 0, 0, 0);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(4, solution_count);
}

static void test_search_4343()
{
  solution_count = 0;
  setupCentralFaces(4, 3, 4, 3);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(0, solution_count);
}

static void test_search_4433()
{
  solution_count = 0;
  setupCentralFaces(4, 4, 3, 3);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(1, solution_count);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search_4343);
  RUN_TEST(test_search);
  RUN_TEST(test_search_abcd);
  RUN_TEST(test_search_4433);
  return UNITY_END();
}
