#include "alternating.h"
#include "helper_for_tests.h"
#include "visible_for_testing.h"

#include <stdarg.h>
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

static void dump(void)
{
  for (int i = 0; i < 12; i++)
    for (int j = i + 1; j < 12; j++)
      for (int k = j + 1; k < 12; k++) {
        uint_trail *value1 = getPartialCyclicOrder(i, j, k);
        uint_trail *value2 = getPartialCyclicOrder(i, k, j);
        TEST_ASSERT_FALSE(*value1 && *value2);
        printf("%c %c %c %c\n", i + 'a', j + 'a', k + 'a',
               *value1   ? '+'
               : *value2 ? '-'
                         : '?');
      }
}
static void triangle(const unsigned long outsideBitSet, const int a,
                     const int b, const int c)
{
  bool flipped;
  switch (
      __builtin_popcount(outsideBitSet & ((1 << a) | (1 << b) | (1 << c)))) {
    case 0:
    case 2:
      flipped = false;
      break;
    case 1:
    case 3:
      flipped = true;
      break;
    default:
      assert(0);
  }
  if (flipped) {
    TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, c, b));
  } else {
    TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, b, c));
  }
}

static unsigned long oneColorSet(const int a)
{
  return 1l << a;
}

static unsigned long twoColorSet(const int a, const int b)
{
  return oneColorSet(a) | oneColorSet(b);
}

static void convexPolygon(const int number, const int a, ...)
{
  int sides[20];
  int index = 0;
  va_list args;
  sides[index++] = a;
  va_start(args, a);
  while (index < number) {
    sides[index++] = va_arg(args, int);
  }
  va_end(args);
  sides[index++] = sides[0];
  sides[index++] = sides[1];
  for (index = 0; index < number; index++) {
    triangle(0, sides[index], sides[index + 1], sides[index + 2]);
  }
}
#define P  // printf("l %d\n", __LINE__)
static void testV4(int a, int b, int c, int d, int e, int f, int g, int h,
                   int i, int j, int k, int l)
{
  P;
  convexPolygon(6, d, k, l, b, c, g);
  P;
  convexPolygon(5, k, l, b, c, g);
  P;
  convexPolygon(5, d, k, l, b, c);
  P;
  convexPolygon(5, d, k, l, f, g);
  P;
  convexPolygon(4, k, l, f, g);
  P;
  convexPolygon(5, d, k, l, i, g);
  P;
  convexPolygon(4, k, l, i, g);
  P;
  convexPolygon(5, d, e, b, c, g);
  P;
  convexPolygon(4, d, e, b, c);
  P;
  convexPolygon(5, d, h, b, c, g);
  P;
  convexPolygon(4, d, h, b, c);
  P;
  triangle(oneColorSet(d), h, d, g);
  P;
  triangle(oneColorSet(d), k, d, g);
  P;
  triangle(oneColorSet(g), c, d, g);
  P;
  triangle(oneColorSet(g), f, d, g);
  P;
  triangle(0, a, b, c);
  P;
  triangle(0, d, e, f);
  P;
  triangle(0, g, h, i);
  P;
  triangle(0, j, k, l);
  P;

  convexPolygon(4, b, c, k, l);
  P;

  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(PartialCyclicOrder));

  TEST_ASSERT_FALSE(*getPartialCyclicOrder(g, h, f));
  TEST_ASSERT_FALSE(*getPartialCyclicOrder(g, f, h));
  triangle(0, g, h, f);
  TEST_ASSERT_FALSE(*getPartialCyclicOrder(d, h, f));
  TEST_ASSERT_FALSE(*getPartialCyclicOrder(d, f, h));
  triangle(0, d, h, f);

  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(PartialCyclicOrder));
  int counter = EngineCounter;
  TEST_ASSERT_TRUE_MESSAGE(dynamicAlternatingComplete(PartialCyclicOrder),
                           "extendable");
  printf("Engine counter = %d\n", EngineCounter - counter);
}

static void runTestVenn4(void)
{
  testV4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(runTestVenn4);
  return UNITY_END();
}
