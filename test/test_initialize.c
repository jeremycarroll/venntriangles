/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

#include <unity.h>

extern CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];

void setUp(void) {}

void tearDown(void) {}

static void testInitialize(void)
{
  uint32_t j;
  initialize();
  for (j = 0; j < NCYCLES; j++) {
    TEST_ASSERT_NOT_EQUAL(0, Cycles[j].length);
    TEST_ASSERT_GREATER_OR_EQUAL_INT32(3, Cycles[j].length);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(NCOLORS, Cycles[j].length);
  }
}

static void testLastCycles(void)
{
  initialize();
  TEST_ASSERT_EQUAL(0, memcmp(Cycles[NCYCLES3 - 1].curves,
                              (uint32_t[]){0, 1, 2}, 3 * sizeof(uint32_t)));
  TEST_ASSERT_EQUAL(0, memcmp(Cycles[NCYCLES4 - 1].curves,
                              (uint32_t[]){0, 1, 2, 3}, 4 * sizeof(uint32_t)));
  TEST_ASSERT_EQUAL(
      0, memcmp(Cycles[NCYCLES5 - 1].curves, (uint32_t[]){0, 1, 2, 3, 4},
                5 * sizeof(uint32_t)));
  TEST_ASSERT_EQUAL(
      0, memcmp(Cycles[NCYCLES - 1].curves, (uint32_t[]){0, 1, 2, 3, 4, 5},
                6 * sizeof(uint32_t)));
}

static void testContains2(void)
{
  CYCLE cycle = &Cycles[0];
  cycle->length = 3;
  cycle->curves[0] = 0;
  cycle->curves[1] = 1;
  cycle->curves[2] = 2;
  TEST_ASSERT_TRUE(cycleContainsAthenB(cycle, 0, 1));
  TEST_ASSERT_TRUE(cycleContainsAthenB(cycle, 1, 2));
  TEST_ASSERT_TRUE(cycleContainsAthenB(cycle, 2, 0));
  TEST_ASSERT_FALSE(cycleContainsAthenB(cycle, 0, 2));
  TEST_ASSERT_FALSE(cycleContainsAthenB(cycle, 1, 0));
  TEST_ASSERT_FALSE(cycleContainsAthenB(cycle, 2, 1));
}

static void testContains3(void)
{
  CYCLE cycle = &Cycles[0];
  cycle->length = 3;
  cycle->curves[0] = 3;
  cycle->curves[1] = 4;
  cycle->curves[2] = 5;
  TEST_ASSERT_TRUE(cycleContainsAthenBthenC(cycle, 3, 4, 5));
  TEST_ASSERT_TRUE(cycleContainsAthenBthenC(cycle, 4, 5, 3));
  TEST_ASSERT_TRUE(cycleContainsAthenBthenC(cycle, 5, 3, 4));
  TEST_ASSERT_FALSE(cycleContainsAthenBthenC(cycle, 0, 2, 1));
  TEST_ASSERT_FALSE(cycleContainsAthenBthenC(cycle, 0, 1, 2));
  TEST_ASSERT_FALSE(cycleContainsAthenBthenC(cycle, 5, 4, 3));
}

static void testSizeOfCycleSet(void)
{
  CYCLESET_DECLARE cycleSet;
  memset(cycleSet, 0, sizeof(cycleSet));
  TEST_ASSERT_EQUAL(0, cycleSetSize(cycleSet));
  cycleSet[0] = 1;
  TEST_ASSERT_EQUAL(1, cycleSetSize(cycleSet));
  cycleSet[0] = 0x8000000000000000;
  TEST_ASSERT_EQUAL(1, cycleSetSize(cycleSet));
  cycleSet[0] = 0x8000000000000001;
  TEST_ASSERT_EQUAL(2, cycleSetSize(cycleSet));
  cycleSet[0] = 0x8000000000000001;
  cycleSet[1] = 0x8000000000000001;
  TEST_ASSERT_EQUAL(4, cycleSetSize(cycleSet));
  memset(cycleSet, 0, sizeof(cycleSet));
  cycleSetAdd(NCYCLES - 1, cycleSet);
  TEST_ASSERT_EQUAL(1, cycleSetSize(cycleSet));
}

static void verifyCycleSetPairs(void)
{
  uint32_t i, j;
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      int size2 = cycleSetSize(CycleSetPairs[i][j]);
      if (i == j) {
        TEST_ASSERT_EQUAL(0, size2);
      } else {
        TEST_ASSERT_EQUAL(64, size2);
      }
    }
  }
}

static void verifyCycleSetTriples(void)
{
  uint32_t i, j, k;
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      for (k = 0; k < NCOLORS; k++) {
        int size3 = cycleSetSize(CycleSetTriples[i][j][k]);
        if (i == j || i == k || j == k) {
          TEST_ASSERT_EQUAL(0, size3);
        } else {
          TEST_ASSERT_EQUAL(16, size3);
        }
      }
    }
  }
}

static void testCycleset(void)
{
  initialize();
  verifyCycleSetPairs();
  verifyCycleSetTriples();
}

static void testSameAndOppositeDirections(void)
{
  uint32_t cycleId, j;
  CYCLE cycle;
  initialize();
  for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    for (j = 0; j < cycle->length; j++) {
      TEST_ASSERT_TRUE(cycleSetMember(cycleId, cycle->sameDirection[j]));
      TEST_ASSERT_FALSE(cycleSetMember(cycleId, cycle->oppositeDirection[j]));
    }
  }
}

static void testOppositeDirections(void)
{
  uint32_t cycleId, j, k, oppositeCycleId;
  CYCLE cycle;
  CYCLE oppositeCycle;
  initialize();
  for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    for (j = 0; j < cycle->length; j++) {
      for (oppositeCycleId = 0, oppositeCycle = Cycles;
           oppositeCycleId < NCYCLES; oppositeCycleId++, oppositeCycle++) {
        if (cycleSetMember(oppositeCycleId, cycle->oppositeDirection[j])) {
          for (k = 0; k < oppositeCycle->length; k++) {
            if (oppositeCycle->curves[k] == cycle->curves[j]) {
              TEST_ASSERT_TRUE(
                  cycleSetMember(cycleId, oppositeCycle->oppositeDirection[k]));
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

static void testFaceChoiceCount(void)
{
  uint32_t faceColors;
  FACE face;
  uint32_t expected[] = {120, 320, 248, 216, 248, 320, 120, -1};
  initialize();
  for (faceColors = 0, face = Faces; faceColors < NFACES;
       faceColors++, face++) {
    TEST_ASSERT_EQUAL(expected[__builtin_popcount(faceColors)],
                      face->cycleSetSize);
  }
}

static void testNextCycle(void)
{
  FACE face = Faces;
  int counter = 0;
  CYCLE cycle = NULL;
  initialize();
  do {
    cycle = cycleSetNext(face->possibleCycles, cycle);
    counter++;
  } while (cycle != NULL);

  TEST_ASSERT_EQUAL(120 + 1, counter);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testInitialize);
  RUN_TEST(testSizeOfCycleSet);
  RUN_TEST(testLastCycles);
  RUN_TEST(testSameAndOppositeDirections);
  RUN_TEST(testFaceChoiceCount);
  RUN_TEST(testOppositeDirections);
  RUN_TEST(testNextCycle);
  RUN_TEST(testContains2);
  RUN_TEST(testContains3);
  RUN_TEST(testCycleset);
  return UNITY_END();
}
