/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "face.h"
#include "helper_for_tests.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

static char* testData3[][2] = {
    {
        "ac",
        "aed",
    },
    {
        "abc",
        "cfe",
    },
    {
        "bd",
        "afb",
    },
    {
        "cd",
        "abd",
    },
    {
        "ae",
        "bde",
    },
    {
        "ace",
        "ade",
    },
    {
        "abce",
        "cef",
    },
    {
        "de",
        "bdf",
    },
    {
        "ade",
        "bed",
    },
    {
        "bde",
        "bfd",
    },
    {
        "cde",
        "adb",
    },
    {
        "af",
        "ace",
    },
    {
        "bf",
        "afc",
    },
    {
        "abf",
        "acf",
    },
    {
        "cf",
        "acd",
    },
    {
        "bdf",
        "abf",
    },
    {
        "cdf",
        "adc",
    },
    {
        "bcdf",
        "adf",
    },
    {
        "aef",
        "aec",
    },
    {
        "bef",
        "cfd",
    },
    {
        "abef",
        "cdf",
    },
    {
        "cef",
        "adc",
    },
    {
        "bcef",
        "cdf",
    },
    {
        "adef",
        "bce",
    },
};

static char* testData4[][2] = {
    {
        "a",
        "abed",
    },
    {
        "b",
        "abcf",
    },
    {
        "bc",
        "bdfc",
    },
    {
        "d",
        "abfd",
    },
    {
        "ad",
        "adeb",
    },
    {
        "abd",
        "abef",
    },
    {
        "acd",
        "adeb",
    },
    {
        "bcd",
        "afdb",
    },
    {
        "abcd",
        "abef",
    },
    {
        "be",
        "bdfc",
    },
    {
        "bce",
        "bcfd",
    },
    {
        "abde",
        "bdfe",
    },
    {
        "acde",
        "abed",
    },
    {
        "bcde",
        "abdf",
    },
    {
        "abcde",
        "afeb",
    },
    {
        "acf",
        "adec",
    },
    {
        "bcf",
        "acfd",
    },
    {
        "adf",
        "abec",
    },
    {
        "abdf",
        "afeb",
    },
    {
        "acdf",
        "aced",
    },
    {
        "abcdf",
        "afed",
    },
    {
        "acef",
        "aced",
    },
    {
        "abcef",
        "cfed",
    },
    {
        "def",
        "bfdc",
    },
    {
        "bdef",
        "bcdf",
    },
    {
        "cdef",
        "abcd",
    },
};

static char* testData5[][2] = {
    {
        "ab",
        "afceb",
    },
    {
        "c",
        "adbce",
    },
    {
        "e",
        "bcefd",
    },
    {
        "abe",
        "becfd",
    },
    {
        "ce",
        "aecbd",
    },
    {
        "f",
        "aefdc",
    },
    {
        "abcf",
        "adefc",
    },
    {
        "df",
        "acdfb",
    },
    {
        "ef",
        "acdfe",
    },
    {
        "abdef",
        "befdc",
    },
    {
        "acdef",
        "adecb",
    },
    {
        "bcdef",
        "afdcb",
    },
};

static char* testData6[][2] = {
    {
        "",
        "adfecb",
    },
    {
        "abcdef",
        "abcdef",
    },
};

static TRAIL trail = NULL;
void setUp(void)
{
  FACE_DEGREE args[] = {5, 5, 5, 4, 4, 4};
  initialize();
  initializeStatisticLogging("/dev/stdout", 20, 5);
  if (trail == NULL) {
    dynamicFaceSetupCentral(args);
    trail = Trail;
  }
  CycleGuessCounterIPC = 1;
}

void tearDown(void)
{
  trailRewindTo(trail);
  CycleGuessCounterIPC = 0;
}

static void addFacesFromTestData(char* testData[][2], int length)
{
  int i;
  FACE face;
  uint32_t cycleId;
  FAILURE failure;
  for (i = 0; i < length; i++) {
    face = faceFromColors(testData[i][0]);
    cycleId = cycleIdFromColors(testData[i][1]);
    TEST_ASSERT_TRUE(cycleSetMember(cycleId, face->possibleCycles));
    if (face->cycleSetSize == 1) {
      TEST_ASSERT_EQUAL(Cycles + cycleId, cycleSetFirst(face->possibleCycles));
      TEST_ASSERT_EQUAL(face->cycle, Cycles + cycleId);
    } else {
      TRAIL_SET_POINTER(&face->cycle, Cycles + cycleId);
      failure = dynamicFaceBacktrackableChoice(face);
      if (failure != NULL) {
        printf("Failure: %s %s\n", failure->label, failure->shortLabel);
        facePrintSelected();
      }
      TEST_ASSERT_NULL(failure);
    }
  }
}

static bool findFace(char* colors, FACE* face, char** cyclePtr,
                     char* testData[][2], int length)
{
  int i;
  for (i = 0; i < length; i++) {
    if (strcmp(colors, testData[i][0]) == 0) {
      *face = faceFromColors(testData[i][0]);
      *cyclePtr = testData[i][1];
      return true;
    }
  }
  return false;
}

static FACE addFaceFromTestData(char* colors)
{
  FACE face;
  char* cycle;
  bool firstOf = findFace(colors, &face, &cycle, testData3,
                          sizeof(testData3) / sizeof(testData3[0])) ||
                 findFace(colors, &face, &cycle, testData4,
                          sizeof(testData4) / sizeof(testData4[0])) ||
                 findFace(colors, &face, &cycle, testData5,
                          sizeof(testData5) / sizeof(testData5[0])) ||
                 findFace(colors, &face, &cycle, testData6,
                          sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_TRUE(firstOf);
  return dynamicFaceAddSpecific(colors, cycle);
}

static void testFaceFromColors()
{
  TEST_ASSERT_EQUAL(Faces, faceFromColors(""));
  TEST_ASSERT_EQUAL(Faces + 1, faceFromColors("a"));
  TEST_ASSERT_EQUAL(Faces + 2, faceFromColors("b"));
  TEST_ASSERT_EQUAL(Faces + 3, faceFromColors("ab"));
  TEST_ASSERT_EQUAL(Faces + 4, faceFromColors("c"));
  TEST_ASSERT_EQUAL(Faces + 5, faceFromColors("ac"));
  TEST_ASSERT_EQUAL(Faces + 6, faceFromColors("bc"));
  TEST_ASSERT_EQUAL(Faces + 7, faceFromColors("abc"));
  TEST_ASSERT_EQUAL(Faces + NFACES - 1, faceFromColors("abcdef"));
}

static void test3456(void)
{
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(14, CycleGuessCounterIPC);
}

static void test4356(void)
{
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(11, CycleGuessCounterIPC);
}

static void test6543(void)
{
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  TEST_ASSERT_EQUAL(12, CycleGuessCounterIPC);
}

static void test5364(void)
{
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  TEST_ASSERT_EQUAL(11, CycleGuessCounterIPC);
}

static void testInOrder(void)
{
  FACE face;
  char colors[7];
  int i;
  COLOR color;
  while ((face = searchChooseNextFace())) {
    for (color = 0, i = 0; color < NCOLORS; color++) {
      if (COLORSET_HAS_MEMBER(color, face->colors)) {
        colors[i++] = 'a' + color;
      }
    }
    colors[i] = 0;
    TEST_ASSERT_EQUAL(face, addFaceFromTestData(colors));
  }
  TEST_ASSERT_EQUAL(26, CycleGuessCounterIPC);
}

static void testDE1(void)
{
  FACE ab = faceFromColors("ab");
  FACE abc = faceFromColors("abc");
  uint32_t cycleId = cycleIdFromColors("afceb");
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
  addFaceFromTestData("abc");
#if DEBUG
  dynamicCycleSetPrint(abc->cycle->oppositeDirection[0]);
#endif
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, abc->cycle->oppositeDirection[0]));
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
  addFaceFromTestData("abce");
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
}

static void testDE2(void)
{
  FACE ab = faceFromColors("ab");
  uint32_t cycleId = cycleIdFromColors("afceb");
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
  addFaceFromTestData("abce");
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
  addFaceFromTestData("abc");
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, ab->possibleCycles));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testFaceFromColors);
  RUN_TEST(test3456);
  RUN_TEST(test4356);
  RUN_TEST(test6543);
  RUN_TEST(test5364);
  RUN_TEST(testDE1);
  RUN_TEST(testDE2);
  RUN_TEST(testInOrder);
  return UNITY_END();
}
