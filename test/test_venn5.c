/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
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
static int EquivocalCount = 0;
static int CanonicalCount = 0;
static int MatchingSolutions = 0;
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
static char* removeSpacesFromStr(char* string)
{
  int non_space_count = 0;
  char* result = getBuffer();
  for (int i = 0; string[i] != '\0'; i++) {
    if (string[i] != ' ') {
      result[non_space_count] = string[i];
      non_space_count++;
    }
  }
  result[non_space_count] = '\0';
  return result;
}

static void checkOrder(char* expected, FACE_DEGREE_SEQUENCE faceDegrees)
{
  char* s = d6SequenceToString(faceDegrees);
  TEST_ASSERT_EQUAL_STRING(removeSpacesFromStr(expected), s);
}

/* Callback functions */
static void invertSolution()
{
  // FACE_DEGREE_SEQUENCE naturalOrder = d6FaceDegreesInNaturalOrder();
  // FACE_DEGREE_SEQUENCE sequenceOrder =
  // d6ConvertToSequenceOrder(naturalOrder); FACE_DEGREE_SEQUENCE
  // invertedNaturalOrder =
  //     d6InvertedFaceDegreesInNaturalOrder();
  // FACE_DEGREE_SEQUENCE invertedSequenceOrder =
  //     d6ConvertToSequenceOrder(invertedNaturalOrder);
  // SYMMETRY_TYPE comparison =
  //     d6IsMaxInSequenceOrder(2, invertedSequenceOrder, sequenceOrder);
  // TEST_ASSERT_EQUAL(CANONICAL, comparison);
  // checkOrder("55443434333343545443333334344455", naturalOrder);
  // checkOrder("55454434334333434333433334444555", invertedNaturalOrder);
  // checkOrder("54434 54434 5 33333445343333343445 5", sequenceOrder);
  // checkOrder("55433 44434 5 43334334333433344545 5", invertedSequenceOrder);
  // TEST_ASSERT_EQUAL_STRING(
  //     removeSpacesFromStr("54434 54434 5 33333445343333343445 5"),
  //     d6SolutionSequenceString());
  // TEST_ASSERT_EQUAL_STRING(
  //     removeSpacesFromStr("55433 44434 5 43334334333433344545 5"),
  //     d6SolutionClassSequenceString());
  // switch (symmetryTypeFaces()) {
  //   case EQUIVOCAL:
  //     TEST_FAIL_MESSAGE("Equivocal solution");
  //     break;
  //   case CANONICAL:
  //     break;
  //   case NON_CANONICAL:
  //     TEST_FAIL_MESSAGE("Non-canonical solution");
  //     break;
  // }

  printf("permutation: %s\n",
         permutationToString(d6CycleAsPermutation(Faces[0].cycle)));
  printf("automorphism: %s\n",
         permutationToString(d6Automorphism(Faces[0].cycle - Cycles)));
  printf("inverse: %s\n",
         permutationToString(d6Inverse(d6CycleAsPermutation(Faces[0].cycle))));
  printf("group[1]: %s\n", permutationToString(&group[1]));
  printf("compose: %s\n",
         permutationToString(d6Compose(
             &group[1], d6Inverse(d6CycleAsPermutation(Faces[0].cycle)))));
  printf("applied: %s\n",
         permutationToString(d6CycleAsPermutation(
             Cycles + cycleIdPermute(Faces[0].cycle - Cycles,
                                     d6Compose(d6Inverse(d6CycleAsPermutation(
                                                   Faces[0].cycle)),
                                               &group[1])))));
  printf(
      "apply auto: %s\n",
      permutationToString(d6CycleAsPermutation(
          Cycles + cycleIdPermute(Faces[0].cycle - Cycles,
                                  d6Automorphism(Faces[0].cycle - Cycles)))));
  PERMUTATION permutation =
      d6Compose(&group[1], d6Inverse(d6CycleAsPermutation(Faces[0].cycle)));
  CYCLE_ID cycleId = cycleIdPermute(Faces[0].cycle - Cycles,
                                    d6Automorphism(Faces[0].cycle - Cycles));
  printf("cycleId: %d => %d == %d\n", Faces[0].cycle - Cycles, cycleId,
         NCYCLES - 1);
  CYCLE_ID_SEQUENCE signature = d6SignatureFromFaces();
  CYCLE_ID_SEQUENCE maxSignature = d6MaxSignature();
  printf("signature: %s\n", d6SignatureToLongString(signature));
  printf("maxSignature: %s\n", d6SignatureToLongString(maxSignature));
  TEST_ASSERT_EQUAL_STRING(
      "CoCnBdBeAfAlAoBbAdAqAcArAiAhCvBjCjBgBcApAeAfApAoAcBfAdBaAnAiByCv",
      d6SignatureToString(signature));
  TEST_ASSERT_EQUAL_STRING(
      "CvCgByBuBaBcBfAqAoAcApBeApAdAoBbAuBiAvAuArBdAqArBhAfBnBhChAmCqCh",
      d6SignatureToString(maxSignature));
  TEST_ASSERT_EQUAL_STRING(
      "(abcde) "   // center
      "(aecdb) "   // A
      "(aedcb) "   // B
      "(abde) "    // AB
      "(bedc) "    // C
      "(bdce) "    // AC
      "(bcde) "    // BC
      "(bed) "     // ABC
      "(ced) "     // D
      "(bdc) "     // AD
      "(cde) "     // BD
      "(bced) "    // ABD
      "(cde) "     // CD
      "(bcd) "     // ACD
      "(ced) "     // BCD
      "(bdec) "    // ABCD
      "(aed) "     // E
      "(adce) "    // AE
      "(ade) "     // BE
      "(aed) "     // ABE
      "(bde) "     // CE
      "(becd) "    // ACE
      "(bed) "     // BCE
      "(bde) "     // ABCE
      "(adec) "    // DE
      "(acd) "     // ADE
      "(aced) "    // BDE
      "(adec) "    // ABDE
      "(acedb) "   // CDE
      "(abdc) "    // ACDE
      "(abdec) "   // BCDE
      "(acedb) ",  // ABCDE
      d6SignatureToLongString(maxSignature));
}

/* Callback functions */
static void foundSolution()
{
  SolutionCount++;

  if (strcmp(
          d6SignatureToString(d6MaxSignature()),
          "CvCgByBuBaBcBfAqAoAcApBeApAdAoBbAuBiAvAuArBdAqArBhAfBnBhChAmCqCh") ==
      0) {
    MatchingSolutions++;
  }
}

/* Test functions */
static void testSearchAbcde()
{
  SolutionCount = 0;
  MatchingSolutions = 0;
  dynamicFaceSetupCentral(intArray(0, 0, 0, 0, 0));
  searchHere(false, foundSolution);
  TEST_ASSERT_EQUAL(152, SolutionCount);
  TEST_ASSERT_EQUAL(40, MatchingSolutions); // 4 actual solutions, times 5 rotations times 2 reflections.
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
