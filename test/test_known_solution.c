
#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define DEBUG 0
#define STATS 0

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
  initializeStatsLogging("/dev/stdout", 20, 5);
  setupCentralFaces(args);
}

void tearDown(void)
{
#if STATS
  if (cycleGuessCounter > 1) {
    printStatisticsFull();
  }
#endif
  clearGlobals();
  clearInitialize();
  resetTrail();
  resetStatistics();
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
    TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, face->possibleCycles));
    if (face->cycleSetSize == 1) {
#if DEBUG
      printf("!\n");
#endif
      TEST_ASSERT_EQUAL(g_cycles + cycleId,
                        findFirstCycleInSet(face->possibleCycles));
      TEST_ASSERT_EQUAL(face->cycle, g_cycles + cycleId);
    } else {
      face->cycle = g_cycles + cycleId;
#if DEBUG
      printf("+\n");
      printSelectedFaces();
#endif
      failure = makeChoice(face);

#if STATS
      printStatisticsOneLine(0);
#endif
      if (failure != NULL) {
        printf("Failure: %s %s\n", failure->label, failure->shortLabel);
        printSelectedFaces();
      }
      TEST_ASSERT_NULL(failure);
    }
  }
}

static void test_faceFromColors()
{
  TEST_ASSERT_EQUAL(g_faces, faceFromColors(""));
  TEST_ASSERT_EQUAL(g_faces + 1, faceFromColors("a"));
  TEST_ASSERT_EQUAL(g_faces + 2, faceFromColors("b"));
  TEST_ASSERT_EQUAL(g_faces + 3, faceFromColors("ab"));
  TEST_ASSERT_EQUAL(g_faces + 4, faceFromColors("c"));
  TEST_ASSERT_EQUAL(g_faces + 5, faceFromColors("ac"));
  TEST_ASSERT_EQUAL(g_faces + 6, faceFromColors("bc"));
  TEST_ASSERT_EQUAL(g_faces + 7, faceFromColors("abc"));
  TEST_ASSERT_EQUAL(g_faces + NFACES - 1, faceFromColors("abcdef"));
}

static void test_3_4_5_6(void)
{
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(14, cycleGuessCounter);
}

static void test_4_3_5_6(void)
{
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  TEST_ASSERT_EQUAL(11, cycleGuessCounter);
}

static void test_6_5_4_3(void)
{
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  TEST_ASSERT_EQUAL(12, cycleGuessCounter);
}

static void test_5_3_6_4(void)
{
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  TEST_ASSERT_EQUAL(11, cycleGuessCounter);
}

static void test_in_order(bool smallestFirst)
{
  FACE face;
  char colors[7];
  int i;
  COLOR color;
  while ((face = chooseFace(smallestFirst))) {
    for (color = 0, i = 0; color < NCOLORS; color++) {
      if (memberOfColorSet(color, face->colors)) {
        colors[i++] = 'a' + color;
      }
    }
    colors[i] = 0;
    TEST_ASSERT_EQUAL(face, addFaceFromTestData(colors));
  }
}

static void test_in_best_order(void)
{
  test_in_order(true);
  TEST_ASSERT_EQUAL(26, cycleGuessCounter);
}

static void test_in_worst_order(void)
{
  test_in_order(false);
  TEST_ASSERT_EQUAL(10, cycleGuessCounter);
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
  findFace(colors, &face, &cycle, testData3,
           sizeof(testData3) / sizeof(testData3[0])) ||
      findFace(colors, &face, &cycle, testData4,
               sizeof(testData4) / sizeof(testData4[0])) ||
      findFace(colors, &face, &cycle, testData5,
               sizeof(testData5) / sizeof(testData5[0])) ||
      findFace(colors, &face, &cycle, testData6,
               sizeof(testData6) / sizeof(testData6[0]));
  return addSpecificFace(colors, cycle);
}

static void test_DE_1(void)
{
  FACE ab = faceFromColors("ab");
  FACE abc = faceFromColors("abc");
  uint32_t cycleId = cycleIdFromColors("afceb");
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
  addFaceFromTestData("abc");
#if DEBUG
  printCycleSet(abc->cycle->oppositeDirection[0]);
#endif
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, abc->cycle->oppositeDirection[0]));
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
  addFaceFromTestData("abce");
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
}

static void test_DE_2(void)
{
  FACE ab = faceFromColors("ab");
  uint32_t cycleId = cycleIdFromColors("afceb");
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
  addFaceFromTestData("abce");
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
  addFaceFromTestData("abc");
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, ab->possibleCycles));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_faceFromColors);
  RUN_TEST(test_3_4_5_6);
  RUN_TEST(test_4_3_5_6);
  RUN_TEST(test_6_5_4_3);
  RUN_TEST(test_5_3_6_4);
  RUN_TEST(test_DE_1);
  RUN_TEST(test_DE_2);
  RUN_TEST(test_in_best_order);
  RUN_TEST(test_in_worst_order);
  return UNITY_END();
}
