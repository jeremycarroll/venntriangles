#include "d6.h"
#include "face.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

void setUp(void) { initializeSequenceOrder(); }

void tearDown(void)
{
  // Tear down code if needed
}

static void testCanonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL,
                    dynamicSymmetryType6(dynamicIntArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL,
                    dynamicSymmetryType6(dynamicIntArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL,
                    dynamicSymmetryType6(dynamicIntArray(5, 5, 5, 4, 4, 4)));
}

static int count6;
static void countSolutions() { count6++; }

static void testCallback()
{
  dynamicFaceCanonicalCallback(countSolutions, NULL);

  TEST_ASSERT_EQUAL(56, count6);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCanonical6);
  RUN_TEST(testCallback);
  return UNITY_END();
}
