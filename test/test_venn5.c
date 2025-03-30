/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

/* Test setup and teardown */
void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/stdout", 20, 5);
}

void tearDown(void)
{
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

/* Global variables */
static int SolutionCount = 0;

/* Callback functions */
static void foundSolution() { SolutionCount++; }

/* Test functions */
static void testSearchAbcde()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(152, SolutionCount);
}

static void testSearch44444()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 4, 4, 4));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(2, SolutionCount);
}

static void testSearch55433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 4, 3, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(6, SolutionCount);
}

static void testSearch55343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 3, 4, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void testSearch54443()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 4, 4, 4, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void testSearch54434()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 4, 4, 3, 4));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(5, SolutionCount);
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testSearchAbcde);
  RUN_TEST(testSearch44444);
  RUN_TEST(testSearch55343);
  RUN_TEST(testSearch55433);
  RUN_TEST(testSearch54443);
  RUN_TEST(testSearch54434);

  return UNITY_END();
}
