/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"
#include "vsearch.h"

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
static int EquivocalCount = 0;
static int CanonicalCount = 0;
static int MatchingSolutions = 0;
static int MatchOuter = 0;
static int MatchA = 0;
static int MatchBCDE = 0;
static int MatchABCDE = 0;
/* Callback functions */
static void countSolutions()
{
  SolutionCount++;
  switch (symmetryTypeFaces()) {
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
}

/* Callback functions */
static void invertSolution()
{
  CYCLE_ID cycleId = cycleIdPermute(Faces[0].cycle - Cycles,
                                    d6Automorphism(Faces[0].cycle - Cycles));
  SIGNATURE signature = d6SignatureFromFaces();
  SIGNATURE maxSignature = d6MaxSignature();
  TEST_ASSERT_EQUAL(NCYCLES - 1, cycleId);
  TEST_ASSERT_EQUAL_STRING(
      "CoCnBdBeAfAlAoBbAdAqAcArAiAhCvBjCjBgBcApAeAfApAoAcBfAdBaAnAiByCv",
      d6SignatureToString(signature));
  TEST_ASSERT_EQUAL_STRING(
      "CvCgByBuBaBcBfAqAoAcApBeApAdAoBbAuBiAvAuArBdAqArBhAfBnBhChAmCqCh",
      d6SignatureToString(maxSignature));
  TEST_ASSERT_EQUAL_STRING(
      " |abcde|: "
      "(abcde) "  // center
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
      d6SignatureToLongString(maxSignature));
}

/* Callback functions */
static void foundSolution()
{
  SolutionCount++;

  SIGNATURE signature = d6MaxSignature();
  if (strcmp(
          d6SignatureToString(signature),
          "CvCgByBuBaBcBfAqAoAcApBeApAdAoBbAuBiAvAuArBdAqArBhAfBnBhChAmCqCh") ==
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
}

/* Test functions */
static void testSearchAbcde()
{
  SolutionCount = 0;
  MatchingSolutions = 0;
  MatchOuter = 0;
  MatchA = 0;
  MatchBCDE = 0;
  MatchABCDE = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0));
  searchHere(false, foundSolution);
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
  searchHere(false, countSolutions);
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
  searchHere(false, countSolutions);
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
  searchHere(false, countSolutions);
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
  searchHere(false, countSolutions);
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
  searchHere(false, countSolutions);
  TEST_ASSERT_EQUAL(5, SolutionCount);
  TEST_ASSERT_EQUAL(0, EquivocalCount);
  TEST_ASSERT_EQUAL(5, CanonicalCount);
}

/// @brief
static void testInvert()
{
  dynamicFaceSetupCentral(intArray(5, 4, 4, 3, 4));
  dynamicFaceSetCycleLength(1 << 2, 3);
  searchHere(false, invertSolution);
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
  RUN_TEST(testInvert);
  return UNITY_END();
}
