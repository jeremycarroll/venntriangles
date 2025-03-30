/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

/* Test setup and teardown */
void setUp(void) { initializeSequenceOrder(); }

void tearDown(void)
{
  // Tear down code if needed
}

/* Global variables */
static int count6;

/* Callback functions */
static void countSolutions() { count6++; }

/* Test functions */
static void testCanonical6()
{
  TEST_ASSERT_EQUAL(NON_CANONICAL, symmetryType6(intArray(5, 5, 4, 4, 4, 5)));
  TEST_ASSERT_EQUAL(CANONICAL, symmetryType6(intArray(6, 5, 5, 4, 4, 3)));
  TEST_ASSERT_EQUAL(EQUIVOCAL, symmetryType6(intArray(5, 5, 5, 4, 4, 4)));
}

static void testCallback()
{
  canonicalCallback(countSolutions, NULL);

  TEST_ASSERT_EQUAL(56, count6);
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCanonical6);
  RUN_TEST(testCallback);
  return UNITY_END();
}
