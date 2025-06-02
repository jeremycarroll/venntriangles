/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "cycleset.h"

#include "trail.h"

#include <string.h>

CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];

static int NextSetOfCycleSets = 0;
static CYCLESET CycleSetSets[NCYCLE_ENTRIES * 2];

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

void cycleSetAdd(CYCLE_ID cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] |= 1ul << (cycleId % BITS_PER_WORD);
}

bool cycleSetMember(CYCLE_ID cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  return (cycleSet[cycleId / BITS_PER_WORD] >> (cycleId % BITS_PER_WORD)) & 1ul;
}

void cycleSetRemove(CYCLE_ID cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] &= ~(1ul << (cycleId % BITS_PER_WORD));
}

static void memoizeCyclePairs(void)
{
  uint32_t i, j, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (cycleContainsAthenB(&Cycles[cycleId], i, j)) {
          cycleSetAdd(cycleId, CycleSetPairs[i][j]);
        }
      }
    }
  }
}

static void memoizeCycleTriples(void)
{
  uint32_t i, j, k, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      for (k = 0; k < NCOLORS; k++) {
        for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
          if (cycleContainsAthenBthenC(&Cycles[cycleId], i, j, k)) {
            cycleSetAdd(cycleId, CycleSetTriples[i][j][k]);
          }
        }
      }
    }
  }
}

static void initializeOmittingOneColor(void)
{
  uint32_t i, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
      if (!COLORSET_HAS_MEMBER(i, Cycles[cycleId].colors)) {
        cycleSetAdd(cycleId, CycleSetOmittingOneColor[i]);
      }
    }
  }
}

static void initializeOmittingColorPairs(void)
{
  uint32_t i, j, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (!cycleContainsAthenB(&Cycles[cycleId], i, j)) {
          cycleSetAdd(cycleId, CycleSetOmittingColorPair[i][j]);
        }
      }
    }
  }
}

static void initializeOmittingCycleSets(void)
{
  initializeOmittingOneColor();
  initializeOmittingColorPairs();
}

CYCLE cycleSetFirst(CYCLESET cycleSet)
{
  return cycleSetNext(cycleSet, NULL);
}

/**
 * Finds the next cycle in a cycle set, starting from the given cycle.
 *
 * This is a performance-critical function that efficiently iterates through the
 * bits in a cycle set by using bitwise operations:
 *
 * 1. If cycle is NULL, starts from the beginning. Otherwise, starts from the
 *    cycle after the given one.
 * 2. Creates a bitmask that only considers bits at or after the starting
 * position within the current word.
 * 3. Find the first non-zero word in the remaining cycle set.
 * 4. For that word, finds the first set bit using GCC's
 *    __builtin_ctzll (Count Trailing Zeros) instruction.
 * 5. Returns the corresponding cycle, or NULL if no more cycles are found.
 *
 * The __builtin_ctzll intrinsic counts trailing zeros in a 64-bit word,
 * effectively finding the index of the lowest set bit.
 */
CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle)
{
  uint64 i;
  int64_t j;
  uint64 cycleId = cycle == NULL ? 0 : cycle - Cycles + 1;
  // Create a mask where bits before cycleId are 0 and bits after are 1
  uint64 mask = ~((1ull << (cycleId % BITS_PER_WORD)) - 1);
  for (i = cycleId / BITS_PER_WORD; i < CYCLESET_LENGTH; i++) {
    if (mask & cycleSet[i]) {
      // Find the index of the lowest set bit in the masked word and convert to
      // cycle ID
      j = i * BITS_PER_WORD + __builtin_ctzll(mask & cycleSet[i]);
      assert(j < NCYCLES);
      return &Cycles[j];
    }
    // After the first word, consider all bits in subsequent words
    mask = ~0ull;
  }
  return NULL;
}

uint32_t cycleSetSize(CYCLESET cycleSet)
{
  uint32_t size = 0;
  for (uint32_t i = 0; i < CYCLESET_LENGTH; i++) {
    size += __builtin_popcountll(cycleSet[i]);
  }
  return size;
}

void dynamicCycleSetRemoveCycle(CYCLESET cycleSet, uint32_t cycleId)
{
  assert(cycleId < NCYCLES);
  trailSetInt(
      &cycleSet[cycleId / BITS_PER_WORD],
      cycleSet[cycleId / BITS_PER_WORD] & ~(1ul << (cycleId % BITS_PER_WORD)));
}

void initializeCycleSetUniversal(CYCLESET cycleSet)
{
  uint32_t i = 0;
#if NCOLORS > 4  // i.e. CYCLESET_LENGTH > 1
  for (i = 0; i < CYCLESET_LENGTH - 1; i++) {
    cycleSet[i] = ~0;
  }
#endif
  cycleSet[i] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;
}

static void initializeSameDirection(void)
{
  uint32_t i, j;
  CYCLE cycle;
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->sameDirection = &CycleSetSets[NextSetOfCycleSets];
    NextSetOfCycleSets += cycle->length;
    for (j = 1; j < cycle->length; j++) {
      cycle->sameDirection[j - 1] =
          CycleSetPairs[cycle->curves[j - 1]][cycle->curves[j]];
    }
    cycle->sameDirection[j - 1] =
        CycleSetPairs[cycle->curves[j - 1]][cycle->curves[0]];
  }

  assert(NextSetOfCycleSets == NCYCLE_ENTRIES);
}

static void initializeOppositeDirection(void)
{
  uint32_t i, j;
  CYCLE cycle;
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->oppositeDirection = &CycleSetSets[NextSetOfCycleSets];
    NextSetOfCycleSets += cycle->length;
    for (j = 2; j < cycle->length; j++) {
      cycle->oppositeDirection[j - 1] =
          CycleSetTriples[cycle->curves[j]][cycle->curves[j - 1]]
                         [cycle->curves[j - 2]];
    }
    cycle->oppositeDirection[j - 1] =
        CycleSetTriples[cycle->curves[0]][cycle->curves[j - 1]]
                       [cycle->curves[j - 2]];
    cycle->oppositeDirection[0] =
        CycleSetTriples[cycle->curves[1]][cycle->curves[0]]
                       [cycle->curves[j - 1]];
  }

  assert(NextSetOfCycleSets == 2 * NCYCLE_ENTRIES);
}

void initializeCycleSets(void)
{
  if (Cycles[0].length == 0) {
    // Initialize cycles first if they haven't been initialized
    initializeCycles();
    memoizeCyclePairs();
    memoizeCycleTriples();
    initializeSameDirection();
    initializeOppositeDirection();
    initializeOmittingCycleSets();
  }
}
