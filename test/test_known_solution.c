#include "d6.h"
#include "face.h"
#include "search.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"

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
char* testData4[][2] = {
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

static FACE addFaceFromTestData(char* colors);

void setUp(void)
{
  int args[] = {5, 5, 5, 4, 4, 4};
  initialize();
  initializeStatisticLogging("/dev/stdout", 20, 5);
  dynamicFaceSetupCentral(args);
}

void tearDown(void)
{
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

static void addFacesFromTestData(char* testData[][2], int length)
{
  int i;
  FACE face;
  uint32_t cycleId;
  FAILURE failure;
  for (i = 0; i < length; i++) {
    face = dynamicFaceFromColors(testData[i][0]);
    cycleId = cycleIdFromColors(testData[i][1]);
    TEST_ASSERT_TRUE(cycleSetMember(cycleId, face->possibleCycles));
    if (face->cycleSetSize == 1) {
      TEST_ASSERT_EQUAL(Cycles + cycleId,
                        cycleSetFindFirst(face->possibleCycles));
      TEST_ASSERT_EQUAL(face->cycle, Cycles + cycleId);
    } else {
      face->cycle = Cycles + cycleId;
      failure = dynamicFaceMakeChoice(face);
      if (failure != NULL) {
        printf("Failure: %s %s\n", failure->label, failure->shortLabel);
        dynamicFacePrintSelected();
      }
      TEST_ASSERT_NULL(failure);
    }
  }
}

static void testFaceFromColors()
{
  TEST_ASSERT_EQUAL(Faces, dynamicFaceFromColors(""));
  TEST_ASSERT_EQUAL(Faces + 1, dynamicFaceFromColors("a"));
  TEST_ASSERT_EQUAL(Faces + 2, dynamicFaceFromColors("b"));
  TEST_ASSERT_EQUAL(Faces + 3, dynamicFaceFromColors("ab"));
  TEST_ASSERT_EQUAL(Faces + 4, dynamicFaceFromColors("c"));
  TEST_ASSERT_EQUAL(Faces + 5, dynamicFaceFromColors("ac"));
  TEST_ASSERT_EQUAL(Faces + 6, dynamicFaceFromColors("bc"));
  TEST_ASSERT_EQUAL(Faces + 7, dynamicFaceFromColors("abc"));
  TEST_ASSERT_EQUAL(Faces + NFACES - 1, dynamicFaceFromColors("abcdef"));
}

static void test3456(void)
{
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(14, CycleGuessCounter);
}

static void test4356(void)
{
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(11, CycleGuessCounter);
}

static void test6543(void)
{
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  TEST_ASSERT_EQUAL(12, CycleGuessCounter);
}

static void test5364(void)
{
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  TEST_ASSERT_EQUAL(11, CycleGuessCounter);
}

static void testInOrder(bool smallestFirst)
{
  FACE face;
  char colors[7];
  int i;
  COLOR color;
  while ((face = dynamicFaceChoose(smallestFirst))) {
    for (color = 0, i = 0; color < NCOLORS; color++) {
      if (COLORSET_HAS_MEMBER(color, face->colors)) {
        colors[i++] = 'a' + color;
      }
    }
    colors[i] = 0;
    TEST_ASSERT_EQUAL(face, addFaceFromTestData(colors));
  }
}

static void testInBestOrder(void)
{
  testInOrder(true);
  TEST_ASSERT_EQUAL(26, CycleGuessCounter);
}

static void testInWorstOrder(void)
{
  testInOrder(false);
  TEST_ASSERT_EQUAL(10, CycleGuessCounter);
}

static bool findFace(char* colors, FACE* face, char** cyclePtr,
                     char* testData[][2], int length)
{
  int i;
  for (i = 0; i < length; i++) {
    if (strcmp(colors, testData[i][0]) == 0) {
      *face = dynamicFaceFromColors(testData[i][0]);
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
  findFace(colors, &face, &cycle, testData3,
           sizeof(testData3) / sizeof(testData3[0])) ||
      findFace(colors, &face, &cycle, testData4,
               sizeof(testData4) / sizeof(testData4[0])) ||
      findFace(colors, &face, &cycle, testData5,
               sizeof(testData5) / sizeof(testData5[0])) ||
      findFace(colors, &face, &cycle, testData6,
               sizeof(testData6) / sizeof(testData6[0]));
  return dynamicFaceAddSpecific(colors, cycle);
}

static void testDE1(void)
{
  FACE ab = dynamicFaceFromColors("ab");
  FACE abc = dynamicFaceFromColors("abc");
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
  FACE ab = dynamicFaceFromColors("ab");
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
  RUN_TEST(testInBestOrder);
  RUN_TEST(testInWorstOrder);
  return UNITY_END();
}
