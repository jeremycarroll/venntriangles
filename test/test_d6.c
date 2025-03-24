
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "d6.h"
#include "unity.h"
#include "utils.h"

void setUp(void) { initializeSequenceOrder(); }

void tearDown(void)
{
  // Tear down code if needed
}

void test_canonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL,
                    dynamicSymmetryType6(dynamicIntArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL,
                    dynamicSymmetryType6(dynamicIntArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL,
                    dynamicSymmetryType6(dynamicIntArray(5, 5, 5, 4, 4, 4)));
}

int count6;
static void countSolutions() { count6++; }

void test_callback()
{
  dynamicFaceCanonicalCallback(countSolutions, NULL);

  TEST_ASSERT_EQUAL(56, count6);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_canonical6);
  RUN_TEST(test_callback);
  return UNITY_END();
}
