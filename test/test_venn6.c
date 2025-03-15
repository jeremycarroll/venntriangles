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
  /*
    uint32_t i;
    int mask = 3;
    for (i = 0, mask = 3; i < NCOLORS - 1; i++, mask <<= 1) {
      printf("%d,", g_faces[(NFACES - 1) & ~mask].cycle->length);
    }
    printf("%d\n", g_faces[((NFACES - 1) >> 1) - 1].cycle->length);
    */
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

  TEST_ASSERT_EQUAL(80, solution_count);
#if STATS
  printStatisticsFull();
#endif
}

static void test_search_for_two_solutions()
{
  solution_count = 0;
  setupCentralFaces(5, 5, 5, 4, 4, 4);
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

static TRAIL restartHere;
static void full_search_callback6(int a1, int a2, int a3, int a4, int a5,
                                  int a6)
{
  int initialSolutionCount = solution_count;
  clearGlobals();
  clearInitialize();
  resetTrail();
  initialize();
  setupCentralFaces(a1, a2, a3, a4, a5, a6);
  search(true, found_solution);
  if (solution_count != initialSolutionCount) {
    printf("%d %d %d %d %d %d gives %d new solutions\n", a1, a2, a3, a4, a5, a6,
           -initialSolutionCount + solution_count);
  }
}

static void search_555444_callback12(int a1, int a2, int a3, int a4, int a5,
                                     int a6, int a7, int a8, int a9, int a10,
                                     int a11, int a12)
{
  assert(0);
}

static void test_full_search(void)
{
  solution_count = 0;
  canoncialCallback(full_search_callback6, search_555444_callback12);
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
