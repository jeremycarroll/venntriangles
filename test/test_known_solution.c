
#include "unity.h"
#include "../venn.h"
#include "../visible_for_testing.h"

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
        "befd",
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

void setUp(void) { initialize(); }

void tearDown(void)
{
  clearGlobals();
  clearInitialize();
  resetTrail();
}

static FACE faceFromColors(char* colors)
{
  int face_id = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    face_id |= (1 << (*colors - 'a'));
    colors++;
  }
  printf("Faceid %x\n", face_id);
  return g_faces + face_id;
}

static uint32_t cycleIdFromColors(char* colors)
{
  COLOR cycle[NCURVES];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return findCycleId(cycle, i);
}

static void addFacesFromTestData(char* testData[][2], int length)
{
  int i;
  FACE face;
  uint32_t cycleId;
  for (i = 0; i < length; i++) {
    face = faceFromColors(testData[i][0]);
    cycleId = cycleIdFromColors(testData[i][1]);
    TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, face->possibleCycles));
    if (face->cycleSetSize == 1) {
        printf("!\n");
      TEST_ASSERT_EQUAL(cycleId, findFirstCycleInSet(face->possibleCycles));
      TEST_ASSERT_EQUAL(face->cycle, g_cycles + cycleId);
    } else {
        printf("+\n");
      face->cycle = g_cycles + cycleId;
      TEST_ASSERT_NULL(makeChoice(face));
    }
  }
}

void test_faceFromColors() {
    TEST_ASSERT_EQUAL(g_faces, faceFromColors(""));
    TEST_ASSERT_EQUAL(g_faces + 1, faceFromColors("a"));
    TEST_ASSERT_EQUAL(g_faces + 2, faceFromColors("b"));
    TEST_ASSERT_EQUAL(g_faces + 3, faceFromColors("ab"));
    TEST_ASSERT_EQUAL(g_faces + 4, faceFromColors("c"));
    TEST_ASSERT_EQUAL(g_faces + 5, faceFromColors("ac"));
    TEST_ASSERT_EQUAL(g_faces + 6, faceFromColors("bc"));
    TEST_ASSERT_EQUAL(g_faces + 7, faceFromColors("abc"));
    TEST_ASSERT_EQUAL(g_faces + NFACES-1, faceFromColors("abcdef"));
}

void test_3_4_5_6(void)
{
  addFacesFromTestData(testData3, sizeof(testData3) / sizeof(testData3[0]));
  addFacesFromTestData(testData4, sizeof(testData4) / sizeof(testData4[0]));
  addFacesFromTestData(testData5, sizeof(testData5) / sizeof(testData5[0]));
  addFacesFromTestData(testData6, sizeof(testData6) / sizeof(testData6[0]));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_faceFromColors);
  RUN_TEST(test_3_4_5_6);
  return UNITY_END();
}