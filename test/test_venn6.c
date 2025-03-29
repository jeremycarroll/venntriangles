#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <time.h>
#include <unity.h>

void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/stdout", 200, 10);
  initializeSequenceOrder();
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

static void testCentralFaceEdge(void)
{
  COLOR a;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0, 0));
  for (a = 0; a < NCOLORS; a++) {
    EDGE edge = &Faces[NFACES - 1].edges[a];
    EDGE edge2 = edgeOnCentralFace(a);
    TEST_ASSERT_EQUAL(edge, edge2);
  }
}

static void testSearchForBestSolution()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 5, 4, 4, 4));
  searchHere(true, foundSolution);

  TEST_ASSERT_EQUAL(80, SolutionCount);
}

static void testSearchForTwoSolutions()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 5, 4, 4, 4));
  dynamicFaceAddSpecific("c", "adbce");
  /* This is a short statement of the best solution.
  dynamicFaceAddSpecific("a", "abed");
  dynamicFaceAddSpecific("c", "adbce");
  dynamicFaceAddSpecific("bde", "bfd");
  dynamicFaceAddSpecific("bdf", "abf");
  dynamicFaceAddSpecific("abce", "cef");
  dynamicFaceAddSpecific("acdf", "aced");
  dynamicFaceAddSpecific("cde", "adb");
  dynamicFaceAddSpecific("b", "abcf");
  dynamicFaceAddSpecific("f", "aefdc");
  */
  searchHere(true, foundSolution);

  TEST_ASSERT_EQUAL(2, SolutionCount);
}

static void testFullSearch(void)
{
  /* dyanmicSearchFull includes initialization, so we undo our own setUp. */
  tearDown();
  SolutionCount = 0;
  searchFull(foundSolution);
  TEST_ASSERT_EQUAL(233, SolutionCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCentralFaceEdge);
  RUN_TEST(testSearchForBestSolution);
  RUN_TEST(testSearchForTwoSolutions);
  RUN_TEST(testFullSearch);
  return UNITY_END();
}
