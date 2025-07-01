#include "alternating.h"
#include "helper_for_tests.h"
#include "visible_for_testing.h"

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

void setUp(void)
{
  initializePartialCyclicOrder();
}

void tearDown(void)
{
  clearPartialCyclicOrder();
}

static void testCyclicity(void)
{
  int i, j, k;
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(0, 1, 2));
  TEST_ASSERT_EQUAL(true, *getPartialCyclicOrder(0, 1, 2));
  TEST_ASSERT_EQUAL(true, *getPartialCyclicOrder(1, 2, 0));
  TEST_ASSERT_EQUAL(true, *getPartialCyclicOrder(2, 0, 1));
  TEST_ASSERT_EQUAL(false, *getPartialCyclicOrder(0, 2, 1));
  TEST_ASSERT_EQUAL(false, *getPartialCyclicOrder(1, 0, 2));
  TEST_ASSERT_EQUAL(false, *getPartialCyclicOrder(2, 1, 0));
  for (i = 0; i < 6; i++)
    for (j = 0; j < 6; j++)
      for (k = 0; k < 6; k++) {
        uint_trail* value = getPartialCyclicOrder(i, j, k);
        if (value) {
          TEST_ASSERT_NOT_EQUAL_INT(i, j);
          TEST_ASSERT_NOT_EQUAL_INT(i, k);
          TEST_ASSERT_NOT_EQUAL_INT(j, k);
          TEST_ASSERT(!*value || i + j + k == 3);
        } else {
          TEST_ASSERT(i == j || i == k || j == k);
        }
      }
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(0, 1, 2));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(1, 2, 0));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(2, 0, 1));
  TEST_ASSERT_EQUAL(false, dynamicPCOSet(0, 2, 1));
}

static void testClosure(void)
{
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(0, 1, 2));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(0, 2, 3));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(0, 3, 1));
  TEST_ASSERT_FALSE(dynamicAlternatingClosure(PartialCyclicOrder));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCyclicity);
  RUN_TEST(testPartialExampleA);
  RUN_TEST(testPartialExampleB);
  RUN_TEST(testPartialExampleC);
  RUN_TEST(testClosure);
  return UNITY_END();
}
