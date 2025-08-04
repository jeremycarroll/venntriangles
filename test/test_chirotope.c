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
#define RUN_TRUE_CHIROTOPE(n, chirotopeString) \
  RUN_CHIROTOPE_TEST(n, chirotopeString, true, true, true)
static void runTest(AlternatingPredicate chirotope, char *chirotopeString,
                    bool consistentExpected, bool closedExpected,
                    bool extensibleExpected)
{
  int nC3 = chirotope->n * (chirotope->n - 1) * (chirotope->n - 2) / 6;
  TEST_ASSERT_EQUAL_MESSAGE(nC3, strlen(chirotopeString), chirotopeString);
  initializeAlternating(chirotope);

  int pos = 0;

  // For rank 3, we only need to fill entries where i < j < k
  for (int k = 0; k < chirotope->n; k++) {
    for (int j = 0; j < k; j++) {
      for (int i = 0; i < j; i++) {
        // TEST_ASSERT_EQUAL(pos * 2, getAlternating(chirotope, i, j, k) -
        //                               chirotope->rawStorage);
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
            // Treat 0 like ?
            break;
          default:
            TEST_FAIL_MESSAGE("Illegal character in chirotope");
            break;
        }
      }
    }
  }

  TRAIL startTrail = Trail;
  // printf("before\n");
  // debugAlternating(chirotope);
  bool consistent = dynamicAlternatingClosure(chirotope);
  // printf("after\n");
  // debugAlternating(chirotope);

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
  RUN_TRUE_CHIROTOPE(6, "+?--+?-+++++?++++++?");
}

static void testIncomplete(void)
{
  RUN_CHIROTOPE_TEST(6, "?+--+?-+++++?++++++?", true, false, true);
}

static void testSimple(void)
{
  RUN_TRUE_CHIROTOPE(5, "++++++++++");
}

static void testSimpleInconsistent(void)
{
  RUN_CHIROTOPE_TEST(5, "++-++++-++", false, true, true);
}

static void testSimpleIncomplete(void)
{
  RUN_CHIROTOPE_TEST(5, "+?++++++++", true, false, true);
}

static void testInconsistent(void)
{
  RUN_CHIROTOPE_TEST(6, "-+--+?-+++++?++++++?", false, false, true);
}

static void testSuvorov14(void)
{
  RUN_TRUE_CHIROTOPE(
      14,
      "++-++-?\?-++?++?-++-++-++-?++-+--+-?+?++--++-+?-?--+?-+-++-++?-++-+-?+?+"
      "-+-++--++++-++-++--++-+--+-+?+-++--++++-++----+-?+-++--++-+--+-+-+-++--+"
      "+?+-+--+-?+++--+-+?++--+-++-+-++++-+-+--+-++--+-+-++-+----++-+-?--+++-+-"
      "?--+?+-+-?+-+---?+?+-++-+--++-+-+-?+?+?++-++-+-++--++-+-++--+-?+-+++++++"
      "-?--+-?\?---+-+--+-+---+-+----+-++++--+-++++---+-++++-+++-?-------?-++++"
      "+++++-");
}

static void testCeva(void)
{
  RUN_TRUE_CHIROTOPE(7, "+++0+++++++++++++++0++++++++0-+0---");
}

static void testRingel(void)
{
  RUN_TRUE_CHIROTOPE(9,
                     "+++-++-+++++-+++++++++-++++++++-+--++++++++++++++-++++-++"
                     "+-++++++++-+--++++-++------");
}

static void testOmega14(void)
{
  RUN_TRUE_CHIROTOPE(
      14,
      "++--+--++0+--0+--0-++-0++-0+-+++---+--++-0+-+++0--0-+-0-+--++--0-++0+-0-"
      "-+0+---+++--+--++0++00+++--+-+---+0+---++++--++++--++0--00+-+-+--+-+---+"
      "-+--0-+-+-++--000----++-++++-+-+-+-++-+---+-+---++-+-+-+++-+---0--+0+-++"
      "+++++--+-++-+-+-+-++-+---+-+---++-+-+-++0-+------+-+-+0++++-----------+"
      "0-+0++-+-+-+-++-+---+-+---++-+-+-+++-+------+-+-++++++-----------+++++++"
      "++++0");
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
  RUN_TEST(testSuvorov14);
  RUN_TEST(testCeva);
  RUN_TEST(testRingel);
  RUN_TEST(testOmega14);
  return UNITY_END();
}
