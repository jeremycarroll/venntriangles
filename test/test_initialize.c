#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void)
{
  clearGlobals();
  clearInitialize();
}

void test_initialize(void)
{
  uint32_t i, j;
  initialize();
  for (j = 0; j < NCYCLES; j++) {
    TEST_ASSERT_NOT_EQUAL(0, g_cycles[j].length);
    for (i = 0; i < g_cycles[j].length; i++) {
      TEST_ASSERT_NOT_EQUAL(NO_COLOR, g_cycles[j].curves[i]);
    }
    TEST_ASSERT_EQUAL(NO_COLOR, g_cycles[j].curves[g_cycles[j].length]);
  }
}

/* Tightly coupled with the order of generation. */
void test_first_cycles(void)
{
  initialize();
  TEST_ASSERT_EQUAL(0, bcmp(g_cycles[0].curves, (uint32_t[]){0, 1, 2, NO_COLOR},
                            4 * sizeof(uint32_t)));
}

void test_contains2(void)
{
  CYCLE cycle = &g_cycles[0];
  cycle->length = 3;
  cycle->curves[0] = 0;
  cycle->curves[1] = 1;
  cycle->curves[2] = 2;
  TEST_ASSERT_TRUE(contains2(cycle, 0, 1));
  TEST_ASSERT_TRUE(contains2(cycle, 1, 2));
  TEST_ASSERT_TRUE(contains2(cycle, 2, 0));
  TEST_ASSERT_FALSE(contains2(cycle, 0, 2));
  TEST_ASSERT_FALSE(contains2(cycle, 1, 0));
  TEST_ASSERT_FALSE(contains2(cycle, 2, 1));
}

void test_contains3(void)
{
  CYCLE cycle = &g_cycles[0];
  cycle->length = 3;
  cycle->curves[0] = 3;
  cycle->curves[1] = 4;
  cycle->curves[2] = 5;
  TEST_ASSERT_TRUE(contains3(cycle, 3, 4, 5));
  TEST_ASSERT_TRUE(contains3(cycle, 4, 5, 3));
  TEST_ASSERT_TRUE(contains3(cycle, 5, 3, 4));
  TEST_ASSERT_FALSE(contains3(cycle, 0, 2, 1));
  TEST_ASSERT_FALSE(contains3(cycle, 0, 1, 2));
  TEST_ASSERT_FALSE(contains3(cycle, 5, 4, 3));
}

void test_sizeOfCycleSet(void)
{
  CYCLESET_DECLARE cycleSet;
  memset(cycleSet, 0, sizeof(cycleSet));
  TEST_ASSERT_EQUAL(0, sizeOfCycleSet(cycleSet));
  cycleSet[0] = 1;
  TEST_ASSERT_EQUAL(1, sizeOfCycleSet(cycleSet));
  cycleSet[0] = 0x8000000000000000;
  TEST_ASSERT_EQUAL(1, sizeOfCycleSet(cycleSet));
  cycleSet[0] = 0x8000000000000001;
  TEST_ASSERT_EQUAL(2, sizeOfCycleSet(cycleSet));
  cycleSet[0] = 0x8000000000000001;
  cycleSet[1] = 0x8000000000000001;
  TEST_ASSERT_EQUAL(4, sizeOfCycleSet(cycleSet));
  memset(cycleSet, 0, sizeof(cycleSet));
  addToCycleSet(NCYCLES - 1, cycleSet);
  TEST_ASSERT_EQUAL(1, sizeOfCycleSet(cycleSet));
}

void test_cycleset(void)
{
  uint32_t i, j, k;
  initialize();
  for (i = 0; i < NCURVES; i++) {
    for (j = 0; j < NCURVES; j++) {
      int size2 = sizeOfCycleSet(pairs2cycleSets[i][j]);
      if (i == j) {
        TEST_ASSERT_EQUAL(0, size2);
      } else {
        TEST_ASSERT_EQUAL(64, size2);
      }
      for (k = 0; k < NCURVES; k++) {
        int size3 = sizeOfCycleSet(triples2cycleSets[i][j][k]);
        if (i == j || i == k || j == k) {
          TEST_ASSERT_EQUAL(0, size3);
        } else {
          TEST_ASSERT_EQUAL(16, size3);
        }
      }
    }
  }
}

void test_same_and_opposite_directions(void)
{
  uint32_t cycleId, j;
  CYCLE cycle;
  initialize();
  for (cycleId = 0, cycle = g_cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    for (j = 0; j < cycle->length; j++) {
      TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, cycle->sameDirection[j]));
      TEST_ASSERT_FALSE(memberOfCycleSet(cycleId, cycle->oppositeDirection[j]));
    }
  }
}

void test_opposite_directions(void)
{
  uint32_t cycleId, j, k, oppositeCycleId;
  CYCLE cycle;
  CYCLE oppositeCycle;
  initialize();
  for (cycleId = 0, cycle = g_cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    for (j = 0; j < cycle->length; j++) {
      for (oppositeCycleId = 0, oppositeCycle = g_cycles;
           oppositeCycleId < NCYCLES; oppositeCycleId++, oppositeCycle++) {
        if (memberOfCycleSet(oppositeCycleId, cycle->oppositeDirection[j])) {
          for (k = 0; k < oppositeCycle->length; k++) {
            if (oppositeCycle->curves[k] == cycle->curves[j]) {
              TEST_ASSERT_TRUE(memberOfCycleSet(
                  cycleId, oppositeCycle->oppositeDirection[k]));
              goto ok;
            }
          }
          TEST_FAIL_MESSAGE("opposite direction not found");
        ok:
          continue;
        }
      }
    }
  }
}

void test_face_choice_count(void)
{
  uint32_t faceColors;
  FACE face;
  uint32_t expected[] = {120, 320, 380, 390, 380, 320, 120, -1};
  initialize();
  for (faceColors = 0, face = g_faces; faceColors < NFACES;
       faceColors++, face++) {
    // printf("%c%c%c%c%c%c: %x %x (%d) %d\n",  (faceColors & 1)?'a':'-',
    // ((faceColors >> 1) & 1)?'b':'-', ((faceColors >> 2) & 1)?'c':'-',
    // ((faceColors >> 3) & 1)?'d':'-', ((faceColors >> 4) & 1)?'e':'-',
    // ((faceColors >> 5) & 1)?'f':'-',
    //   faceColors, face->colors, (faceColors), face->cycleSetSize);
    TEST_ASSERT_EQUAL(expected[__builtin_popcount(faceColors)],
                      face->cycleSetSize);
  }
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_initialize);
  RUN_TEST(test_contains2);
  RUN_TEST(test_contains3);
  RUN_TEST(test_cycleset);
  RUN_TEST(test_sizeOfCycleSet);
  RUN_TEST(test_first_cycles);
  RUN_TEST(test_same_and_opposite_directions);
  RUN_TEST(test_face_choice_count);
  RUN_TEST(test_opposite_directions);

  printf("CYCLESET_LENGTH = %lu\nNCYCLE_ENTRIES = %u\nNCYCLES = %u\n",
         CYCLESET_LENGTH, NCYCLE_ENTRIES, NCYCLES);
  return UNITY_END();
}
