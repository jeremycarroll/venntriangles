#include "alternating.h"
#include "helper_for_tests.h"

#include <stdio.h>
#include <unity.h>

void setUp(void)
{
  initializePartialCyclicOrder();
}

void tearDown(void)
{
  clearPartialCyclicOrder();
}
/* Bulletin of the American Mathematical Society, 82 (2): 274–276
 * An example is the following relation on the first thirteen letters of the
* alphabet: {acd, bde, cef, dfg, egh, fha, gac, hcb, abi, cij, bjk, ikl, jlm,
kma, lab, mbc, hcm, bhm}.
 */
static void testMegiddo(int a, int b, int c, int d, int e, int f, int g, int h,
                        int i, int j, int k, int l, int m)
{
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, c, d));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(b, d, e));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(c, e, f));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(d, f, g));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(e, g, h));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(f, h, a));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(g, a, c));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(h, c, b));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, b, i));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(c, i, j));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(b, j, k));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(i, k, l));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(j, l, m));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(k, m, a));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(l, a, b));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(m, b, c));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(h, c, m));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(b, h, m));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(PartialCyclicOrder));
  int counter = EngineCounter;
  bool result = dynamicAlternatingComplete(PartialCyclicOrder);
  printf("Engine counter = %d\n", EngineCounter - counter);
  TEST_ASSERT_FALSE_MESSAGE(result, "not extendable");
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
  RUN_TEST(testPartialExampleA);
  RUN_TEST(testPartialExampleB);
  RUN_TEST(testPartialExampleC);
  RUN_TEST(runTestMegiddoB);
  RUN_TEST(runTestMegiddoC);
  RUN_TEST(runTestMegiddoD);
  RUN_TEST(runTestMegiddoA);
  return UNITY_END();
}
