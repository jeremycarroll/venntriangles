#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 0

void setUp(void)
{
  initialize();
  initializeStatsLogging("/dev/stdout", 20, 5);
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

static void test_search_abcde()
{
  solution_count = 0;
  setupCentralFaces(0, 0, 0, 0, 0);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(162, solution_count);
}

static void test_search_44444()
{
  solution_count = 0;
  setupCentralFaces(4, 4, 4, 4, 4);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(2, solution_count);
}

static void test_search_55433()
{
  solution_count = 0;
  setupCentralFaces(5, 5, 4, 3, 3);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(7, solution_count);
}

static void test_search_55343()
{
  solution_count = 0;
  setupCentralFaces(5, 5, 3, 4, 3);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(0, solution_count);
}

static void test_search_54443()
{
  solution_count = 0;
  setupCentralFaces(5, 4, 4, 4, 3);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(4, solution_count);
}

static void test_search_54434()
{
  solution_count = 0;
  setupCentralFaces(5, 4, 4, 3, 4);
  search(false, found_solution);
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
