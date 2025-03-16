
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../d6.h"
#include "../visible_for_testing.h"
#include "unity.h"
#define DEBUG_CANONICAL_SOLUTIONS 0

void setUp(void) { initializeSequenceOrder(); }

void tearDown(void)
{
  // Tear down code if needed
}

void test_canonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL, d6SymmetryType6(intArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL, d6SymmetryType6(intArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL, d6SymmetryType6(intArray(5, 5, 5, 4, 4, 4)));
}

int count6;
static void countSolutions(
#if DEBUG_CANONICAL_SOLUTIONS
    int* args
#endif
)
{
#if DEBUG_CANONICAL_SOLUTIONS
  int i;
  for (i = 0; i < NCOLORS; i++) {
    printf("%d ", args[i]);
  }
  printf("\n");
#endif
  count6++;
}

void test_callback()
{
  canoncialCallback(countSolutions);

  TEST_ASSERT_EQUAL(56, count6);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_canonical6);
  RUN_TEST(test_callback);
  return UNITY_END();
}
