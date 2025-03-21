#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 0

void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/stdout", 20, 5);
}

void tearDown(void)
{
#if TEST_INFO
  dynamicStatisticPrintFull();
#endif
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
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

static void test_specific()
{
  initializeFaceSetupCentral(dynamicIntArray(0, 0, 0, 0, 0));
  dynamicFaceAddSpecific("ab", "bdc");
  dynamicFaceAddSpecific("bc", "aecb");
  dynamicFaceAddSpecific("bd", "bde");
  dynamicFaceAddSpecific("ae", "aec");
  TEST_ASSERT_NULL(dynamicFaceChoose(false));
  found_solution();
}

static void test_specific2()
{
  initializeFaceSetupCentral(dynamicIntArray(0, 0, 0, 0, 0));
  dynamicFaceAddSpecific("ab", "bdc");
  dynamicFaceAddSpecific("bc", "aecb");
  dynamicFaceAddSpecific("abc", "abcde");
  dynamicFaceAddSpecific("bd", "bde");
  dynamicFaceAddSpecific("b", "bced");
  dynamicFaceAddSpecific("bcd", "abe");
  dynamicFaceAddSpecific("abcd", "aedcb");
  dynamicFaceAddSpecific("abd", "bcd");
  dynamicFaceAddSpecific("bcde", "aeb");
  dynamicFaceAddSpecific("bde", "abed");
  dynamicFaceAddSpecific("abde", "adcb");
  dynamicFaceAddSpecific("be", "adec");
  dynamicFaceAddSpecific("abe", "acd");
  dynamicFaceAddSpecific("bce", "ace");
  dynamicFaceAddSpecific("abce", "aedc");
  dynamicFaceAddSpecific("ae", "aec");
  dynamicFaceAddSpecific("", "aecbd");
  dynamicFaceAddSpecific("a", "adbce");
  dynamicFaceAddSpecific("e", "ace");
  dynamicFaceAddSpecific("ace", "aced");
  dynamicFaceAddSpecific("ce", "adec");
  dynamicFaceAddSpecific("cde", "abed");
  dynamicFaceAddSpecific("c", "abced");
  dynamicFaceAddSpecific("ac", "adecb");
  dynamicFaceAddSpecific("cd", "adeb");
  dynamicFaceAddSpecific("d", "adbe");
  dynamicFaceAddSpecific("ad", "aecbd");
  dynamicFaceAddSpecific("de", "aeb");
  dynamicFaceAddSpecific("acde", "adecb");
  dynamicFaceAddSpecific("acd", "abced");
  dynamicFaceAddSpecific("ade", "abce");
  TEST_ASSERT_NULL(dynamicFaceChoose(false));
  found_solution();
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
