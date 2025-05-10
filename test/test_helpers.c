/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "test_helpers.h"

#include "face.h"
#include "utils.h"

#include <stdarg.h>
#include <unity.h>

/* Helper functions */
FACE dynamicFaceAddSpecific(char* colors, char* cycle)
{
  FAILURE failure;
  FACE face = faceFromColors(colors);
  uint32_t cycleId = cycleIdFromColors(cycle);
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, face->possibleCycles));
  if (face->cycleSetSize == 1) {
    TEST_ASSERT_EQUAL(face->cycle, cycleSetFirst(face->possibleCycles));
    TEST_ASSERT_EQUAL(face->cycle, Cycles + cycleId);
  } else {
    face->cycle = Cycles + cycleId;
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

/* Debug functions */
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
        if (cycleSet[i] & (1ul << j)) {
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
