/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "face.h"
#include "helper_for_tests.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdlib.h>
#include <time.h>
#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 20);
  engine(&TestStack, (PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineClear(&TestStack);
}
static int SolutionCount = 0;

static struct predicateResult countSolutions(int round)
{
  (void)round;  // Mark parameter as intentionally unused
  SolutionCount++;
  return PredicateFail;
}

static void testCentralFaceEdge(void)
{
  COLOR a;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0, 0));
  for (a = 0; a < NCOLORS; a++) {
    EDGE edge = &Faces[NFACES - 1].edges[a];
    EDGE edge2 = vertexGetCentralEdge(a);
    TEST_ASSERT_EQUAL(edge, edge2);
  }
}

static void testSearchForBestSolution()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 5, 4, 4, 4));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(80, SolutionCount);
}

static void testSearchForTwoSolutions()
{
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 5, 4, 4, 4));
  dynamicFaceAddSpecific("c", "adbce");
  /* A short statement of the best solution.
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
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});

  TEST_ASSERT_EQUAL(2, SolutionCount);
}

static void testFullSearch(void)
{
  /* dynamicSearchFull includes initialization, so we undo our own setUp. */
  SolutionCount = 0;
  engineResume(
      &TestStack,
      (PREDICATE[]){&InnerFacePredicate, &LogPredicate, &VennPredicate,
                    &(struct predicate){"Found", countSolutions, NULL}});
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
