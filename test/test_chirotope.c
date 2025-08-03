/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

/*
 * Our chirotopes unusually are:
 * - partial: ? is a legal value meaning unknown/undefined
 * - uniform: 0 is not a legal value
 **/

#include "../../Unity/src/unity.h"
#include "../alternating.h"
#include "../visible_for_testing.h"

#include <string.h>
#include <unity.h>

void setUp(void)
{
  initializeTrail();
}

void tearDown(void) {}

#define VERIFY_PROPERTY(prop) \
  TEST_ASSERT_EQUAL_MESSAGE(prop##Expected, prop, #prop " was not as expected")
#define RUN_CHIROTOPE_TEST(n, chirotopeString, consistent, closed, extensible) \
  runTest(CREATE_CHIROTOPE(n), chirotopeString, consistent, closed, extensible)
static void runTest(AlternatingPredicate chirotope, char *chirotopeString,
                    bool consistentExpected, bool closedExpected,
                    bool extensibleExpected)
{
  int nC3 = chirotope->n * (chirotope->n - 1) * (chirotope->n - 2) / 6;
  TEST_ASSERT_EQUAL(nC3, strlen(chirotopeString));
  initializeAlternating(chirotope);

  int pos = 0;

  // For rank 3, we only need to fill entries where i < j < k
  for (int i = 0; i < chirotope->n; i++) {
    for (int j = i + 1; j < chirotope->n; j++) {
      for (int k = j + 1; k < chirotope->n; k++) {
        TEST_ASSERT_EQUAL(pos * 2, getAlternating(chirotope, i, j, k) -
                                       chirotope->rawStorage);
        switch (chirotopeString[pos++]) {
          case '+':
            // printf("Setting %d %d %d\n", i, j, k);
            TEST_ASSERT(dynamicAlternatingSet(chirotope, i, j, k));
            break;
          case '-':
            // printf("Setting %d %d %d\n", i, k, j);
            TEST_ASSERT(dynamicAlternatingSet(chirotope, i, k, j));
            break;
          case '?':
            break;
          case '0':
            TEST_FAIL_MESSAGE("Only uniform chirotopes supported");
            break;
          default:
            TEST_FAIL_MESSAGE("Illegal character in chirotope");
            break;
        }
      }
    }
  }

  TRAIL startTrail = Trail;
  bool consistent = dynamicAlternatingClosure(chirotope);
  VERIFY_PROPERTY(consistent);
  if (consistent) {
    bool closed = startTrail == Trail;
    VERIFY_PROPERTY(closed);
    bool extensible = dynamicAlternatingComplete(chirotope);
    VERIFY_PROPERTY(extensible);
  }
}

static void testChapter1(void)
{
  RUN_CHIROTOPE_TEST(6, "+?--+?-+++++?++++++?", true, true, true);
}

static void testIncomplete(void)
{
  RUN_CHIROTOPE_TEST(6, "?+--+?-+++++?++++++?", true, false, true);
}

static void testSimple(void)
{
  RUN_CHIROTOPE_TEST(5, "++++++++++", true, true, true);
}

static void testSimpleInconsistent(void)
{
  RUN_CHIROTOPE_TEST(5, "++-++++-++", false, true, true);
}

static void testSimpleIncomplete(void)
{
  RUN_CHIROTOPE_TEST(5, "++-++++?++", true, false, true);
}

static void testInconsistent(void)
{
  RUN_CHIROTOPE_TEST(6, "-+--+?-+++++?++++++?", false, false, true);
}

/* The same predicate as per testMeggido in PCO 5 is extendible as
   a partial chirotope.
 */
static void testMegiddo(int a, int b, int c, int d, int e, int f, int g, int h,
                        int i, int j, int k, int l, int m)
{
  AlternatingPredicate megiddo = CREATE_CHIROTOPE(13);
  initializeAlternating(megiddo);
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, c, d));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, d, e));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, c, e, f));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, d, f, g));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, e, g, h));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, f, h, a));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, g, a, c));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, h, c, b));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, b, i));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, c, i, j));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, j, k));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, i, k, l));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, j, l, m));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, k, m, a));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, l, a, b));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, m, b, c));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, h, c, m));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, h, m));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(megiddo));
  int counter = EngineCounter;
  bool result = dynamicAlternatingComplete(megiddo);
  printf("Engine counter = %d\n", EngineCounter - counter);
  TEST_ASSERT_MESSAGE(result, "extendable");
}

static void runTestMegiddoA(void)
{
  testMegiddo(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
}

static void runTestMegiddoB(void)
{
  testMegiddo(4, 5, 0, 1, 14, 3, 6, 13, 8, 9, 10, 11, 12);
}
static void runTestMegiddoC(void)
{
  testMegiddo(9, 10, 11, 4, 5, 0, 1, 14, 3, 6, 13, 8, 12);
}
static void runTestMegiddoD(void)
{
  testMegiddo(9, 10, 11, 6, 13, 8, 12, 4, 5, 0, 1, 14, 3);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testChapter1);
  RUN_TEST(testInconsistent);
  RUN_TEST(testIncomplete);
  RUN_TEST(testSimple);
  RUN_TEST(testSimpleInconsistent);
  RUN_TEST(testSimpleIncomplete);
  RUN_TEST(runTestMegiddoA);
  return UNITY_END();
}
