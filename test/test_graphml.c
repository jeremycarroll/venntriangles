/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

static void mockInitializeFolder(const char* folder)
{  // nothing.
}

static char outputBuffer[50000];
static int FopenCount = 0;
static FILE* mockFopen(const char* filename, const char* mode)
{
  FILE* result = fmemopen(outputBuffer, 50000, mode);
  assert(result != NULL);
  FopenCount++;
  return result;
}

/* Test setup and teardown */
void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/null", 1000, 1000);  // Reduce logging output
  graphmlFileOps.initializeFolder = mockInitializeFolder;
  graphmlFileOps.fopen = mockFopen;
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

static void (*FoundSolution)(void);
/* Callback functions */
static void foundBasicSolution()
{
  SolutionCount++;
  SIGNATURE signature = d6MaxSignature();
  TEST_ASSERT_EQUAL_STRING(
      "PdJeIgFuAcEhAdDeDqAcDuGqElAdGnGhKoDpAbFnAnFeAaFwJdDmCzCyAeFkCyCzErDnDlDk"
      "BqFwAzHcEsAxEpEyJjAwDcCxKjDoAaFwAsDeBvNwDiEtFhEsAyOrJwMx",
      d6SignatureToString(signature));
  // graphmlSaveAllVariations("/tmp/foo", 128);
  if (FoundSolution) FoundSolution();
}

static void saveAllVariations()
{
  FopenCount = 0;
  graphmlSaveAllVariations("/tmp/foo", 256);
  TEST_ASSERT_EQUAL(128, FopenCount);
}

static void checkFirstVariation()
{
  FopenCount = 0;
  MaxVariantsPerSolution = 1;
  graphmlSaveAllVariations("/tmp/foo", 256);
  TEST_ASSERT_EQUAL(1, FopenCount);
}

static void testVariationCount()
{
  TEST_ASSERT_EQUAL(128, searchCountVariations());
}

static int ContinuationCount = 0;
static int countContinuation(COLOR ignored, EDGE (*corners)[3])
{
  ContinuationCount++;
  return 0;
}

static void testColorContinuations(COLOR color, int expectedCount)
{
  EDGE corners[NCOLORS][3];
  ContinuationCount = 0;
  computePossibleCorners();
#if 0
  for (int i = 0; i < 3; i++) {
    printf("cornerPairs[%d][0]: %x\n", i, cornerPairs[i][0]->reversed);
    printf("cornerPairs[%d][1]: %x\n", i, cornerPairs[i][1]);
  }

#endif

  chooseCornersWithContinuation(0, color, corners, countContinuation);
  TEST_ASSERT_EQUAL(expectedCount, ContinuationCount);
}

static void foundSolutionColor0() { testColorContinuations(0, 8); }
static void foundSolutionColor1() { testColorContinuations(1, 1); }
static void foundSolutionColor2() { testColorContinuations(2, 2); }
static void foundSolutionColor3() { testColorContinuations(3, 1); }
static void foundSolutionColor4() { testColorContinuations(4, 2); }
static void foundSolutionColor5() { testColorContinuations(5, 4); }

static char* TestName;

static void run645534(void)
{
  Unity.NumberOfTests--;
  UNITY_NEW_TEST(TestName);
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(6, 4, 5, 5, 3, 4));
  searchHere(true, foundBasicSolution);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

#define RUN_645534(foundSolution) \
  TestName = #foundSolution;      \
  FoundSolution = foundSolution;  \
  RUN_TEST(run645534);

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_645534(NULL);
  RUN_645534(testVariationCount);
  RUN_645534(foundSolutionColor0);
  RUN_645534(foundSolutionColor1);
  RUN_645534(foundSolutionColor2);
  RUN_645534(foundSolutionColor3);
  RUN_645534(foundSolutionColor4);
  RUN_645534(foundSolutionColor5);
  RUN_645534(saveAllVariations);
  RUN_645534(checkFirstVariation);
  return UNITY_END();
}
