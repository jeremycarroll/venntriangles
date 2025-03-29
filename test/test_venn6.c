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
static void found_solution() { SolutionCount++; }

static void test_central_face_edge(void)
{
  COLOR a;
  dynamicFaceSetupCentral(dynamicIntArray(0, 0, 0, 0, 0, 0));
  for (a = 0; a < NCOLORS; a++) {
    EDGE edge = &Faces[NFACES - 1].edges[a];
    EDGE edge2 = edgeOnCentralFace(a);
    TEST_ASSERT_EQUAL(edge, edge2);
  }
}

static void test_search_for_best_solution()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 5, 5, 4, 4, 4));
  dynamicSearch(true, found_solution);

  TEST_ASSERT_EQUAL(80, SolutionCount);
}

static void test_search_for_two_solutions()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(dynamicIntArray(5, 5, 5, 4, 4, 4));
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
  dynamicSearch(true, found_solution);

  TEST_ASSERT_EQUAL(2, SolutionCount);
}

static void test_full_search(void)
{
  /* dyanmicSearchFull includes initialization, so we undo our own setUp. */
  tearDown();
  SolutionCount = 0;
  dynamicSearchFull(found_solution);
  TEST_ASSERT_EQUAL(233, SolutionCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_central_face_edge);
  RUN_TEST(test_search_for_best_solution);
  RUN_TEST(test_search_for_two_solutions);
  RUN_TEST(test_full_search);
  return UNITY_END();
}
