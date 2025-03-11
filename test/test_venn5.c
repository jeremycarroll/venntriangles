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
#if DEBUG
  int i;
  FACIAL_CYCLE_SIZES sizes = facialCycleSizes();
  for (i = 0; i < NCURVES; i++) {
    printf("%d,", sizes.sizes[i]);
  }
  printf("\n");
#endif
  solution_count++;
}

static void test_search_abcde()
{
  solution_count = 0;
  setupCentralFaces(0, 0, 0, 0, 0);
  search(false, found_solution);
  TEST_ASSERT_EQUAL(152, solution_count);
}

static void test_specific()
{
  setupCentralFaces(0, 0, 0, 0, 0);
  addSpecificFace("ab", "bdc");
  addSpecificFace("bc", "aecb");
  addSpecificFace("bd", "bde");
  addSpecificFace("ae", "aec");
  TEST_ASSERT_NULL(chooseFace(false));
  found_solution();
}

static void test_specific2()
{
  setupCentralFaces(0, 0, 0, 0, 0);
  addSpecificFace("ab", "bdc");
  addSpecificFace("bc", "aecb");
  addSpecificFace("abc", "abcde");
  addSpecificFace("bd", "bde");
  addSpecificFace("b", "bced");
  addSpecificFace("bcd", "abe");
  addSpecificFace("abcd", "aedcb");
  addSpecificFace("abd", "bcd");
  addSpecificFace("bcde", "aeb");
  addSpecificFace("bde", "abed");
  addSpecificFace("abde", "adcb");
  addSpecificFace("be", "adec");
  addSpecificFace("abe", "acd");
  addSpecificFace("bce", "ace");
  addSpecificFace("abce", "aedc");
  addSpecificFace("ae", "aec");
  addSpecificFace("", "aecbd");
  addSpecificFace("a", "adbce");
  addSpecificFace("e", "ace");
  addSpecificFace("ace", "aced");
  addSpecificFace("ce", "adec");
  addSpecificFace("cde", "abed");
  addSpecificFace("c", "abced");
  addSpecificFace("ac", "adecb");
  addSpecificFace("cd", "adeb");
  addSpecificFace("d", "adbe");
  addSpecificFace("ad", "aecbd");
  addSpecificFace("de", "aeb");
  addSpecificFace("acde", "adecb");
  addSpecificFace("acd", "abced");
  addSpecificFace("ade", "abce");
  TEST_ASSERT_NULL(chooseFace(false));
  found_solution();
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
  TEST_ASSERT_EQUAL(6, solution_count);
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
  if (0) {
    RUN_TEST(test_specific);
    RUN_TEST(test_specific2);
  }
  RUN_TEST(test_search_abcde);
  RUN_TEST(test_search_44444);
  RUN_TEST(test_search_55343);
  RUN_TEST(test_search_55433);
  RUN_TEST(test_search_54443);
  RUN_TEST(test_search_54434);

  return UNITY_END();
}
