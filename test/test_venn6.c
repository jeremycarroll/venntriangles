/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "engine.h"
#include "face.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"
#include "vsearch.h"

#include <stdlib.h>
#include <time.h>
#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  engine((PREDICATE[]){&initializePredicate, &SUSPENDPredicate}, NULL);
}

void tearDown(void)
{
  engineResume((PREDICATE[]){&FAILPredicate});
}
/* Global variables */
static int SolutionCount = 0;

/* Predicate functions */
static struct predicateResult countSolutions(int round)
{
  SolutionCount++;
  return PredicateFail;
}

/* Test functions */
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
  engineResume((PREDICATE[]){
      &facePredicate, &(struct predicate){"Found", countSolutions, NULL}});
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
  engineResume((PREDICATE[]){
      &facePredicate, &(struct predicate){"Found", countSolutions, NULL}});

  TEST_ASSERT_EQUAL(2, SolutionCount);
}

/* The test program - initialization, face degrees, and face search */
static struct predicate* testProgram[] = {
    &initializePredicate, &faceDegreePredicate, &facePredicate,
    &(struct predicate){"count", countSolutions, NULL}};

static void testFullSearch(void)
{
  /* dynamicSearchFull includes initialization, so we undo our own setUp. */
  SolutionCount = 0;
  engineResume(
      (PREDICATE[]){&faceDegreePredicate, &facePredicate,
                    &(struct predicate){"Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(233, SolutionCount);
}

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testCentralFaceEdge);
  RUN_TEST(testSearchForBestSolution);
  RUN_TEST(testSearchForTwoSolutions);
  RUN_TEST(testFullSearch);
  return UNITY_END();
}
