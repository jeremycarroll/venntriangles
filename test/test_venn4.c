#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unity.h>

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

static int SolutionCount = 0;
static void foundSolution() { SolutionCount++; }

static void testSearch()
{
  SolutionCount = 0;
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(FACTORIAL4, SolutionCount);
}

static void testSearchAbcd()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(4, SolutionCount);
}

static void testSearch4343()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 3, 4, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(0, SolutionCount);
}

static void testSearch4433()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 3, 3));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testSearch);
  RUN_TEST(testSearch4343);
  RUN_TEST(testSearchAbcd);
  RUN_TEST(testSearch4433);
  return UNITY_END();
}
