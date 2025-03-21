#include "color.h"

#include "trail.h"

struct facial_cycle Cycles[NCYCLES];

void initializeCycleSetAdd(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] |= 1ul << (cycleId % BITS_PER_WORD);
}

bool initializeCycleSetMember(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  return (cycleSet[cycleId / BITS_PER_WORD] >> (cycleId % BITS_PER_WORD)) & 1ul;
}

void initializeCycleSetRemove(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] &= ~(1ul << (cycleId % BITS_PER_WORD));
}

CYCLE initializeCycleSetFindFirst(CYCLESET cycleSet)
{
  return initializeCycleSetFindNext(cycleSet, NULL);
}

CYCLE initializeCycleSetFindNext(CYCLESET cycleSet, CYCLE cycle)
{
  uint64_t i;
  int64_t j;
  uint64_t cycleId = cycle == NULL ? 0 : cycle - Cycles + 1;
  uint64_t mask = ~((1ull << (cycleId % BITS_PER_WORD)) - 1);
  for (i = cycleId / BITS_PER_WORD; i < CYCLESET_LENGTH; i++) {
    if (mask & cycleSet[i]) {
      j = i * BITS_PER_WORD + __builtin_ctzll(mask & cycleSet[i]);
      assert(j < NCYCLES);
      return &Cycles[j];
    }
    mask = ~0ull;
  }
  return NULL;
}

bool cycleContainsAthenB(CYCLE cycle, uint32_t i, uint32_t j)
{
  uint64_t ix;
  for (ix = 1; ix < cycle->length; ix++) {
    if (cycle->curves[ix - 1] == i && cycle->curves[ix] == j) {
      return true;
    }
  }
  return cycle->curves[ix - 1] == i && cycle->curves[0] == j;
}

bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k)
{
  uint64_t ix;
  for (ix = 2; ix < cycle->length; ix++) {
    if (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j &&
        cycle->curves[ix] == k) {
      return true;
    }
  }
  return (cycle->curves[ix - 1] == i && cycle->curves[0] == j &&
          cycle->curves[1] == k) ||
         (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j &&
          cycle->curves[0] == k);
}

/* See https://en.wikichip.org/wiki/population_count#Software_support */
uint32_t cycleSetSize(CYCLESET cycleSet)
{
  uint32_t size = 0;
  for (uint32_t i = 0; i < CYCLESET_LENGTH; i++) {
    size += __builtin_popcountll(cycleSet[i]);
  }
  return size;
}

uint32_t cycleFindId(COLOR* cycle, uint32_t length)
{
  uint32_t cycleId;
  uint32_t i;
  for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
    if (Cycles[cycleId].length == length) {
      for (i = 0; i < length; i++) {
        if (Cycles[cycleId].curves[i] != cycle[i]) {
          break;
        }
      }
      if (i == length) {
        return cycleId;
      }
    }
  }
  assert(NULL == "Unreachable");
}

uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color)
{
  uint32_t i;
  for (i = 0; i < cycle->length; i++) {
    if (cycle->curves[i] == color) {
      return i;
    }
  }
  assert(NULL == "Unreachable");
}

CYCLESET_DECLARE CylesetWithoutColor[NCOLORS];

void resetCyclesetWithoutColor()
{
  memset(CylesetWithoutColor, 0, sizeof(CylesetWithoutColor));
}

void initializeCyclesetWithoutColor()
{
  COLOR color;
  CYCLE cycle;
  uint32_t i;
  for (color = 0; color < NCOLORS; color++) {
    for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
      if (!memberOfColorSet(color, cycle->colors)) {
        initializeCycleSetAdd(i, CylesetWithoutColor[color]);
      }
    }
  }
}
