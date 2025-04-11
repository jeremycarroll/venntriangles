/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "graphml.h"
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
  initializeStatisticLogging("/dev/null", 1000, 1000);  // Reduce logging output
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

/* Callback function */
static void foundSolution()
{
  SolutionCount++;
  SIGNATURE signature = d6MaxSignature();
  TEST_ASSERT_EQUAL_STRING(
      "PdJeIgFuAcEhAdDeDqAcDuGqElAdGnGhKoDpAbFnAnFeAaFwJdDmCzCyAeFkCyCzErDnDlDk"
      "BqFwAzHcEsAxEpEyJjAwDcCxKjDoAaFwAsDeBvNwDiEtFhEsAyOrJwMx",
      d6SignatureToString(signature));
  graphmlSaveAllVariations("/tmp/foo", 128);
}

/* Test function */
static void testGraphml()
{
  SolutionCount = 0;

  dynamicFaceSetupCentral(intArray(6, 4, 5, 5, 3, 4));
  searchHere(true, foundSolution);

  // Verify we found exactly one solution with 128 variations
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testGraphml);
  return UNITY_END();
}
