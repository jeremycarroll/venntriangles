#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 0
#define TEST_INFO 0

void setUp(void)
{
  initialize();
  initializeStatsLogging("/dev/stdout", 200, 10);
}

void tearDown(void)
{
#if TEST_INFO
  printStatisticsFull();
#endif
  clearGlobals();
  clearInitialize();
  resetTrail();
  resetStatistics();
}

static int solution_count = 0;
static void found_solution()
{
#if TEST_INFO
  printSolution(NULL);
  printStatisticsFull();
#endif
  solution_count++;
}

static void test_search_for_best_solution()
{
  solution_count = 0;
  setupCentralFaces(5, 5, 5, 4, 4, 4);
  // addSpecificFace("c", "adbce");
  /* This is a short statement of the best solution.
  addSpecificFace("a", "abed");
  addSpecificFace("c", "adbce");
  addSpecificFace("bde", "bfd");
  addSpecificFace("bdf", "abf");
  addSpecificFace("abce", "cef");
  addSpecificFace("acdf", "aced");
  addSpecificFace("cde", "adb");
  addSpecificFace("b", "abcf");
  addSpecificFace("f", "aefdc");
  */
  search(true, found_solution);

  TEST_ASSERT_EQUAL(160, solution_count);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search_for_best_solution);
  return UNITY_END();
}
