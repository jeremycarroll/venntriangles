
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

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_canonical6);
  return UNITY_END();
}
