/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "helper_for_tests.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  engine(&TestStack, (PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineClear(&TestStack);
}

static int SolutionCount = 0;
static int EquivocalCount = 0;
static int CanonicalCount = 0;
static int MatchingSolutions = 0;
static int MatchOuter = 0;
static int MatchA = 0;
static int MatchBCDE = 0;
static int MatchABCDE = 0;

static struct predicateResult countSolutions()
{
  SolutionCount++;
  switch (s6FacesSymmetryType()) {
    case EQUIVOCAL:
      EquivocalCount++;
      break;
    case CANONICAL:
      CanonicalCount++;
      break;
    case NON_CANONICAL:
      TEST_FAIL_MESSAGE("Non-canonical solution");
      break;
  }
  return PredicateFail;
}

static struct predicateResult invertSolution()
{
  CYCLE_ID cycleId = s6PermuteCycleId(Faces[0].cycle - Cycles,
                                      s6Automorphism(Faces[0].cycle - Cycles));
  SIGNATURE signature = s6SignatureFromFaces();
  SIGNATURE maxSignature = s6MaxSignature();
  TEST_ASSERT_EQUAL(NCYCLES - 1, cycleId);
  TEST_ASSERT_EQUAL_STRING(
      "CcCpBbBeAgAjAoBcAdAqAcAsAiAhCvBhCnBgBdApAeAgApAoAcBfAdBaAnAiByCv",
      s6SignatureToString(signature));
  TEST_ASSERT_EQUAL_STRING(
      "CvCaByBwBaBdBfAqAoAcApBeApAdAoBcAuBoAxAuAsBbAqAsBmAgBqBmCkAmCsCk",
      s6SignatureToString(maxSignature));
  TEST_ASSERT_EQUAL_STRING(
      " |abcde|: "
      "(abcde) "  // outer
      "(aecdb) "  // A
      "(aedcb) "  // B
      "(abde) "   // AB
      "(bedc) "   // C
      "(bdce) "   // AC
      "(bcde) "   // BC
      "(bed) "    // ABC
      "(ced) "    // D
      "(bdc) "    // AD
      "(cde) "    // BD
      "(bced) "   // ABD
      "(cde) "    // CD
      "(bcd) "    // ACD
      "(ced) "    // BCD
      "(bdec) "   // ABCD
      "(aed) "    // E
      "(adce) "   // AE
      "(ade) "    // BE
      "(aed) "    // ABE
      "(bde) "    // CE
      "(becd) "   // ACE
      "(bed) "    // BCE
      "(bde) "    // ABCE
      "(adec) "   // DE
      "(acd) "    // ADE
      "(aced) "   // BDE
      "(adec) "   // ABDE
      "(acedb) "  // CDE
      "(abdc) "   // ACDE
      "(abdec) "  // BCDE
      "(acedb)",  // ABCDE
      s6SignatureToLongString(maxSignature));
  return PredicateFail;
}

static struct predicateResult foundSolution()
{
  SolutionCount++;

  SIGNATURE signature = s6MaxSignature();
  if (strcmp(
          s6SignatureToString(signature),
          "CvCaByBwBaBdBfAqAoAcApBeApAdAoBcAuBoAxAuAsBbAqAsBmAgBqBmCkAmCsCk") ==
      0) {
    MatchingSolutions++;
    switch (signature->offset) {
      case 0:  // ||
        MatchOuter++;
        break;
      case 1:  // |A|
        MatchA++;
        break;
      case 30:  // |BCDE|
        MatchBCDE++;
        break;
      case 31:  // |ABCDE|
        MatchABCDE++;
        break;
      default:
        TEST_FAIL_MESSAGE("Unknown match");
        break;
    }
  }
  return PredicateFail;
}

static void testSearchAbcde()
{
  SolutionCount = 0;
  MatchingSolutions = 0;
  MatchOuter = 0;
  MatchA = 0;
  MatchBCDE = 0;
  MatchABCDE = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(152, SolutionCount);
  TEST_ASSERT_EQUAL(40, MatchingSolutions);
  // 4 actual solutions, times 5 rotations times 2 reflections.
  TEST_ASSERT_EQUAL(10, MatchOuter);
  TEST_ASSERT_EQUAL(10, MatchA);
  TEST_ASSERT_EQUAL(10, MatchBCDE);
  TEST_ASSERT_EQUAL(10, MatchABCDE);
}

static void testSearch44444()
{
  SolutionCount = 0;
  EquivocalCount = 0;
  CanonicalCount = 0;
  dynamicFaceSetupCentral(intArray(4, 4, 4, 4, 4));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(2, SolutionCount);
  TEST_ASSERT_EQUAL(2, EquivocalCount);
  TEST_ASSERT_EQUAL(0, CanonicalCount);
}

static void testSearch55433()
{
  SolutionCount = 0;
  EquivocalCount = 0;
  CanonicalCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 4, 3, 3));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(6, SolutionCount);
  TEST_ASSERT_EQUAL(0, EquivocalCount);
  TEST_ASSERT_EQUAL(6, CanonicalCount);
}

static void testSearch55343()
{
  SolutionCount = 0;
  EquivocalCount = 0;
  CanonicalCount = 0;
  dynamicFaceSetupCentral(intArray(5, 5, 3, 4, 3));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(0, SolutionCount);
  TEST_ASSERT_EQUAL(0, EquivocalCount);
  TEST_ASSERT_EQUAL(0, CanonicalCount);
}

static void testSearch54443()
{
  SolutionCount = 0;
  EquivocalCount = 0;
  CanonicalCount = 0;
  dynamicFaceSetupCentral(intArray(5, 4, 4, 4, 3));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(4, SolutionCount);
  TEST_ASSERT_EQUAL(0, EquivocalCount);
  TEST_ASSERT_EQUAL(4, CanonicalCount);
}

static void testSearch54434()
{
  SolutionCount = 0;
  EquivocalCount = 0;
  CanonicalCount = 0;
  dynamicFaceSetupCentral(intArray(5, 4, 4, 3, 4));
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", countSolutions, NULL}});
  TEST_ASSERT_EQUAL(5, SolutionCount);
  TEST_ASSERT_EQUAL(0, EquivocalCount);
  TEST_ASSERT_EQUAL(5, CanonicalCount);
}

static void testInvert()
{
  dynamicFaceSetupCentral(intArray(5, 4, 4, 3, 4));
  dynamicFaceSetCycleLength(1 << 2, 3);
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", invertSolution, NULL}});
}
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testSearchAbcde);
  RUN_TEST(testSearch44444);
  RUN_TEST(testSearch55343);
  RUN_TEST(testSearch55433);
  RUN_TEST(testSearch54443);
  RUN_TEST(testSearch54434);
  RUN_TEST(testInvert);
  return UNITY_END();
}
