
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../d6.h"
#include "unity.h"

void setUp(void) { initializeSequenceOrder(); }

void tearDown(void)
{
  // Tear down code if needed
}

void test_canonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL, d6SymmetryType6(5, 5, 4, 4, 4, 5));
  TEST_ASSERT_EQUAL(CANONICAL, d6SymmetryType6(6, 5, 5, 4, 4, 3));
  TEST_ASSERT_EQUAL(EQUIVOCAL, d6SymmetryType6(5, 5, 5, 4, 4, 4));
}

int count6, count12;
static void print6(int a, int b, int c, int d, int e, int f)
{
  /* printf("%d %d %d %d %d %d\n", a, b, c, d, e, f); */
  count6++;
}
static void print12(int a, int b, int c, int d, int e, int f, int g, int h,
                    int i, int j, int k, int l)
{
  /* printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h, i,
     j, k, l); */
  count12++;
}

void test_callback()
{
  canoncialCallback(print6, print12);

  printf("count6=%d count12=%d\n", count6, count12);
  TEST_ASSERT_EQUAL(42, count6);
  TEST_ASSERT_EQUAL(20854, count12);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_canonical6);
  RUN_TEST(test_callback);
  return UNITY_END();
}
