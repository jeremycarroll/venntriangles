#include <time.h>

#include "d6.h"
#include "face.h"
#include "statistics.h"
#include "unity.h"
#include "utils.h"

#define STATS 1
#define TEST_INFO 0

void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/stdout", 200, 10);
  initializeSequenceOrder();
}

void tearDown(void)
{
#if TEST_INFO
  statisticPrintFull();
#endif
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

static int solution_count = 0;
static void found_solution()
{
#if TEST_INFO
  dynamicSolutionPrint(NULL);
  statisticPrintFull();
#endif
  solution_count++;
}

static void test_search_for_best_solution()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 5, 5, 4, 4, 4));
  dynamicSearch(true, found_solution);

  // TEST_ASSERT_EQUAL(80, solution_count);
#if STATS
  statisticPrintFull();
#endif
}

static void test_search_for_two_solutions()
{
  solution_count = 0;
  initializeFaceSetupCentral(dynamicIntArray(5, 5, 5, 4, 4, 4));
  dynamicFaceAddSpecific("c", "adbce");
  /* This is a short statement of the best solution.
  dynamicFaceAddSpecific("a", "abed");
  dynamicFaceAddSpecific("c", "adbce");
  dynamicFaceAddSpecific("bde", "bfd");
  dynamicFaceAddSpecific("bdf", "abf");
  dynamicFaceAddSpecific("abce", "cef");
  dynamicFaceAddSpecific("acdf", "aced");
  dynamicFaceAddSpecific("cde", "adb");
  dynamicFaceAddSpecific("b", "abcf");
  dynamicFaceAddSpecific("f", "aefdc");
  */
  dynamicSearch(true, found_solution);

  // TEST_ASSERT_EQUAL(2, solution_count);
}
#if 0
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
  resetGlobals();
  resetInitialize();
  resetTrail();
  initialize();
  initializeFaceSetupCentral(args);
  dynamicSearch(true, found_solution);
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
#endif

static void test_full_search(void)
{
  solution_count = 0;
  dynamicSearchFull(found_solution);
#if STATS
  statisticPrintFull();
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
