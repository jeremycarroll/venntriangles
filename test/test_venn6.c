#include <time.h>

#include "../d6.h"
#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 1
#define TEST_INFO 0

void setUp(void)
{
  initialize();
  initializeStatsLogging("/dev/stdout", 200, 10);
  initializeSequenceOrder();
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
  setupCentralFaces(intArray(5, 5, 5, 4, 4, 4));
  search(true, found_solution);

  TEST_ASSERT_EQUAL(80, solution_count);
#if STATS
  printStatisticsFull();
#endif
}

static void test_search_for_two_solutions()
{
  solution_count = 0;
  setupCentralFaces(intArray(5, 5, 5, 4, 4, 4));
  addSpecificFace("c", "adbce");
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

  TEST_ASSERT_EQUAL(2, solution_count);
}

static clock_t totalWastedTime = 0;
static clock_t totalUsefulTime = 0;
static int wastedSearchCount = 0;
static int usefulSearchCount = 0;
static void full_search_callback6(int *args)
{
  clock_t now = clock();
  clock_t used;
  int initialSolutionCount = solution_count;
  int i;
  clearGlobals();
  clearInitialize();
  resetTrail();
  initialize();
  setupCentralFaces(args);
  search(true, found_solution);
  used = clock() - now;
  if (solution_count != initialSolutionCount) {
    totalUsefulTime += used;
    usefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    PRINT_TIME(used, 0);
    PRINT_TIME(totalUsefulTime, usefulSearchCount);
    PRINT_TIME(totalWastedTime, wastedSearchCount);
    for (i = 0; i < NCOLORS; i++) {
      printf("%d ", args[i]);
    }
    printf(" gives %d new solutions\n", solution_count - initialSolutionCount);
  } else {
    wastedSearchCount += 1;

    totalWastedTime += used;
  }
}

static void test_full_search(void)
{
  solution_count = 0;
  canoncialCallback(full_search_callback6);
#if STATS
  printStatisticsFull();
#endif
  TEST_ASSERT_EQUAL(233, solution_count);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_search_for_best_solution);
  RUN_TEST(test_search_for_two_solutions);
  RUN_TEST(test_full_search);
  return UNITY_END();
}
