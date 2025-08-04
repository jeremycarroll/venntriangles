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

static void findMegiddo(int a, int b, int c, int d, int e, int f, int g, int h,
                        int i, int j, int k, int l, int m)
{
  AlternatingPredicate megiddo = CREATE_CYCLIC_PARTIAL_ORDER(13);
  initializeAlternating(megiddo);
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, c, d));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, d, e));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, c, e, f));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, d, f, g));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, e, g, h));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, f, h, a));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, g, a, c));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, h, c, b));
  //
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, b, i));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, c, i, j));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, j, k));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, i, k, l));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, l, a, b));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, j, l, m));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, k, m, a));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, m, b, c));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, h, c, m));
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, h, m));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, b, c));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, d, e, f));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, g, h, i));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, j, k, l));
  // debugAlternating(megiddo);
  // AlternatingDebug = true;
  // printf("==\n");
  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(megiddo));
  // printf("==\n");
  // debugAlternating(megiddo);
  printf("Mid: %s\n", alternatingToString(megiddo));

  TRAIL btPoint = Trail;
  // printf("0 1 4 ...\n");
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, b, e));
  // TEST_ASSERT_FALSE(dynamicAlternatingClosure(megiddo));
  // TEST_ASSERT(trailRewindTo(btPoint));
  // printf("0 4 1 ...\n");
  // TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, e, b));
  // TEST_ASSERT_FALSE(dynamicAlternatingClosure(megiddo));
  // TEST_ASSERT(trailRewindTo(btPoint));
  // AlternatingDebug = false;

  // uint_trail *abf = getAlternating(megiddo, a, b, f);
  // uint_trail *afb = getAlternating(megiddo, a, f, b);
  // TEST_ASSERT_EQUAL_PTR(megiddo->rawStorage + 6, abf);
  // TEST_ASSERT_EQUAL_PTR(megiddo->rawStorage + 7, afb);
  // TEST_ASSERT_EQUAL(0, *abf);
  // TEST_ASSERT_EQUAL(0, *afb);
  // *abf = 1;
  // TRAIL btPoint = Trail;
  // TEST_ASSERT_FALSE(dynamicAlternatingClosure(megiddo));
  // TEST_ASSERT(trailRewindTo(btPoint));
  // *abf = 0;
  // *afb = 1;
  // TEST_ASSERT_FALSE(dynamicAlternatingClosure(megiddo));
  // TEST_ASSERT(trailRewindTo(btPoint));
  // *afb = 0;

  //  TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, a, b, i));
  //   TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, c, i, j));
  //   TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, j, k));
  //    TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, i, k, l));
  //     TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, j, l, m));
  //     TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, k, m, a));
  //     TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, l, a, b));
  //      TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, m, b, c));
  //     TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, h, c, m));
  //     TEST_ASSERT_EQUAL(true, dynamicAlternatingSet(megiddo, b, h, m));
  int counter = EngineCounter;
  dynamicAlternatingExtensibility(megiddo);
  printf("Engine counter = %d\n", EngineCounter - counter);
  TEST_ASSERT_GREATER_THAN(counter, EngineCounter);
  // bool result = dynamicAlternatingComplete(megiddo);
  // TEST_ASSERT_FALSE_MESSAGE(result, "not extendable");

  // 8: ++*?+++??+?????????+++??+?+?????????+??++???+++??+??+?++
  // 10:
  // ++*?+++?+??+???????????????+??+?????+??+???+????????????????????+??+??+?????+??+????++????+?????????+???+?????+?????????
  // n = 11
  // ++*?+++??+??+??????????????????+???+?????????+??+????+??????????????????????????++??+???+??????+???+??????+??+?????+??????????????+????+?????????+???????????????????
  // Simpler solution n=12
  // ++*?+++???+??+?????????????????????+????+??????????????+??+?????+????????????????????????????????+??+??+????+???????+????+????????+?????+??????+????????????????????+?????+??????????????+????????????????????????????????+?
}

static void runFindMegiddo(void)
{
  findMegiddo(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
}

/*
B = {
0 1 2
0 1 3
0 1 6
0 1 7
0 2 3
0 2 6
0 7 2
0 5 7
0 6 7
1 2 3
1 2 6
1 7 2
1 3 4
2 3 4
2 3 7
2 4 5
2 5 7
2 6 7
3 4 5
3 5 6
4 5 6
4 6 7
5 6 7
}
Closed but not extensible:
++*?+++??+-????????+++??+-+?????????+??++???+++??+??+?++ O 1 4 is the issue.

B U <0,1,4>: 1,3,4 + 1,4,0 = 1,3,0; 3,1,2

*/
int main(void)
{
  UNITY_BEGIN();
  // RUN_TEST(testPartialExampleA);
  // RUN_TEST(testPartialExampleB);
  // RUN_TEST(testPartialExampleC);
  // RUN_TEST(runTestMegiddoB);
  // RUN_TEST(runTestMegiddoC);
  // RUN_TEST(runTestMegiddoD);
  // RUN_TEST(runTestMegiddoA);
  RUN_TEST(runFindMegiddo);
  return UNITY_END();
}
