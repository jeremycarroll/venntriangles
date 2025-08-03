/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "helper_for_tests.h"

#include "alternating.h"
#include "face.h"

#include <stdarg.h>
#include <unity.h>

struct stack TestStack;

FACE dynamicFaceAddSpecific(char* colors, char* cycle)
{
  FAILURE failure;
  FACE face = faceFromColors(colors);
  CYCLE_ID cycleId = cycleIdFromColors(cycle);
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, face->possibleCycles));
  if (face->cycleSetSize == 1) {
    TEST_ASSERT_EQUAL(face->cycle, cycleSetFirst(face->possibleCycles));
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
  return face;
}

FACE_DEGREE* intArray(int a, ...)
{
  static FACE_DEGREE result[NCOLORS];
  va_list ap;
  int i;
  result[0] = a;
  va_start(ap, a);
  for (i = 1; i < NCOLORS; i++) {
    a = va_arg(ap, int);
    result[i] = a;
  }
  va_end(ap);
  return result;
}

bool dynamicCycleSetPrint(CYCLESET cycleSet)
{
  uint32_t lastBit = (NCYCLES - 1) % BITS_PER_WORD;
  uint64 faulty, i, j;
  if ((faulty = (cycleSet[CYCLESET_LENGTH - 1] & ~((1ul << lastBit) - 1ul)))) {
    printf("0x%016llx\n", faulty);
    return false;
  }
  putchar('{');
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    if (cycleSet[i]) {
      for (j = 0; j < 64; j++) {
        if (cycleSet[i] & (1uL << j)) {
          putchar(' ');
          for (uint64 k = 0; k < Cycles[i * BITS_PER_WORD + j].length; k++) {
            putchar('a' + Cycles[i * BITS_PER_WORD + j].curves[k]);
          }
          putchar(',');
        }
      }
    }
  }
  printf(" }\n");
  return true;
}

uint_trail* getPartialCyclicOrder(int i, int j, int k)
{
  return getAlternating(PartialCyclicOrder, i, j, k);
}

void verifyPartialCyclicOrderAxioms(void)
{
  int i, j, k;
  for (i = 0; i < PCO_LINES; i++)
    for (j = 0; j < PCO_LINES; j++)
      for (k = 0; k < PCO_LINES; k++) {
        uint_trail* value = getPartialCyclicOrder(i, j, k);
        if (value) {
          TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(i, j, "not strict");
          TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(i, k, "not strict");
          TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(j, k, "not strict");
          TEST_ASSERT_EQUAL_MESSAGE(value, getPartialCyclicOrder(j, k, i),
                                    "not cyclic");
          if (*value) {
            TEST_ASSERT_FALSE_MESSAGE(*getPartialCyclicOrder(i, k, j),
                                      "not asymmetric");
            for (int l = 0; l < PCO_LINES; l++) {
              if (l != i && l != j && l != k) {
                // Verify: R(x,y,z) & R(x,z,u) ⇒ R(x,y,u)
                if (*getPartialCyclicOrder(i, k, l)) {
                  TEST_ASSERT_TRUE_MESSAGE(*getPartialCyclicOrder(i, j, l),
                                           "not transitive");
                }
              }
            }
          }
        } else {
          TEST_ASSERT_MESSAGE((i == j || i == k || j == k), "ill-formed");
        }
      }
}

bool dynamicPCOSet(int i, int j, int k)
{
  return dynamicAlternatingSet(PartialCyclicOrder, i, j, k);
}

void clearPartialCyclicOrder(void)
{
  memset(getPartialCyclicOrder(0, 1, 2), 0, sizeof(uint_trail) * PCO_TRIPLES);
}

static void setupPartialExample(int a, int b, int c, int d, int e, int f)
{
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, b, d));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(a, c, d));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(d, e, a));
  TEST_ASSERT_EQUAL(true, dynamicPCOSet(d, f, a));
  TEST_ASSERT_EQUAL(true, dynamicAlternatingClosure(PartialCyclicOrder));
}

static void testPartialExample(int a, int b, int c, int d, int e, int f)
{
  int count = 0;
  setupPartialExample(a, b, c, d, e, f);
  verifyPartialCyclicOrderAxioms();
  uint_trail* value = getPartialCyclicOrder(0, 1, 2);
  for (int i = 0; i < PCO_TRIPLES / 2; i++, value += 2) {
    TEST_ASSERT_FALSE(value[0] && value[1]);
    if (value[0] || value[1]) {
      count++;
    }
  }
  TEST_ASSERT_EQUAL_INT(12, count);
  TEST_ASSERT_TRUE(dynamicAlternatingComplete(PartialCyclicOrder));
}

void testPartialExampleA()
{
  testPartialExample(0, 1, 2, 3, 4, 5);
}

void testPartialExampleB()
{
  testPartialExample(0, 5, 3, 1, 2, 4);
}

void testPartialExampleC()
{
  testPartialExample(1, 2, 4, 3, 0, 5);
}
