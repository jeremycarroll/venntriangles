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

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search);
  return UNITY_END();
}
