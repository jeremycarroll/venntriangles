#include "color.h"

#include <stdarg.h>

#include "trail.h"

static void initializeCycles(void);
void initializeCycleSets(void);
static void initializeSameDirection(void);
static void initializeOppositeDirection(void);
static void initializeOmittingCycleSets(void);

static int nextCycle = 0;
static int nextSetOfCycleSets = 0;

/*
These cycleSets are accessed from cycles, with the pointers set up during
initialization.
 */
CYCLESET_DECLARE InitializeCycleSetPairs[NCOLORS][NCOLORS];
CYCLESET_DECLARE InitializeCycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];
static CYCLESET InitializeCycleSetSets[NCYCLE_ENTRIES * 2];

struct facial_cycle Cycles[NCYCLES];

void initializeCycleSetAdd(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] |= 1ul << (cycleId % BITS_PER_WORD);
}

bool cycleSetMember(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  return (cycleSet[cycleId / BITS_PER_WORD] >> (cycleId % BITS_PER_WORD)) & 1ul;
}

void initializeCycleSetRemove(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] &= ~(1ul << (cycleId % BITS_PER_WORD));
}

CYCLE cycleSetFindFirst(CYCLESET cycleSet)
{
  return cycleSetFindNext(cycleSet, NULL);
}

CYCLE cycleSetFindNext(CYCLESET cycleSet, CYCLE cycle)
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

CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];

void removeFromCycleSetWithTrail(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  trailSetInt(
      &cycleSet[cycleId / BITS_PER_WORD],
      cycleSet[cycleId / BITS_PER_WORD] & ~(1ul << (cycleId % BITS_PER_WORD)));
}

void resetCycles()
{
  memset(InitializeCycleSetPairs, 0, sizeof(InitializeCycleSetPairs));
  memset(InitializeCycleSetTriples, 0, sizeof(InitializeCycleSetTriples));
  memset(InitializeCycleSetSets, 0, sizeof(InitializeCycleSetSets));
  memset(CycleSetOmittingOneColor, 0, sizeof(CycleSetOmittingOneColor));
  memset(CycleSetOmittingColorPair, 0, sizeof(CycleSetOmittingColorPair));

  nextCycle = 0;
  nextSetOfCycleSets = 0;
}

static void addCycle(int length, ...)
{
  uint32_t color;
  va_list ap;
  CYCLE cycle = &Cycles[nextCycle++];
  int ix = 0;
  cycle->colors = 0;
  cycle->length = length;
  va_start(ap, length);
  for (ix = 0; ix < length; ix++) {
    color = va_arg(ap, uint32_t);
    cycle->curves[ix] = color;
    cycle->colors |= 1u << color;
  }
  va_end(ap);
}

static void initializeCycles(void)
{
  assert(nextCycle == 0);
  uint32_t c1, c2, c3, c4, c5, c6;
  for (c1 = 0; c1 < NCOLORS; c1++) {
    for (c2 = c1 + 1; c2 < NCOLORS; c2++) {
      for (c3 = c1 + 1; c3 < NCOLORS; c3++) {
        if (c3 == c2) {
          continue;
        }
        addCycle(3, c1, c2, c3);
        for (c4 = c1 + 1; c4 < NCOLORS; c4++) {
          if (c4 == c2 || c4 == c3) {
            continue;
          }
          addCycle(4, c1, c2, c3, c4);
          for (c5 = c1 + 1; c5 < NCOLORS; c5++) {
            if (c5 == c2 || c5 == c3 || c5 == c4) {
              continue;
            }
            addCycle(5, c1, c2, c3, c4, c5);
            for (c6 = c1 + 1; c6 < NCOLORS; c6++) {
              if (c6 == c2 || c6 == c3 || c6 == c4 || c6 == c5) {
                continue;
              }
              addCycle(6, c1, c2, c3, c4, c5, c6);
            }
          }
        }
      }
    }
  }
  assert(nextCycle == ARRAY_LEN(Cycles));
}

void initializeCycleSets(void)
{
  initializeCycles();
  uint32_t i, j, k, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      if (i == j) {
        continue;
      }
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (cycleContainsAthenB(&Cycles[cycleId], i, j)) {
          initializeCycleSetAdd(cycleId, InitializeCycleSetPairs[i][j]);
        }
      }
      for (k = 0; k < NCOLORS; k++) {
        if (i == k || j == k) {
          continue;
        }
        for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
          if (cycleContainsAthenBthenC(&Cycles[cycleId], i, j, k)) {
            initializeCycleSetAdd(cycleId, InitializeCycleSetTriples[i][j][k]);
          }
        }
      }
    }
  }
  initializeSameDirection();
  initializeOppositeDirection();
  initializeOmittingCycleSets();
}

static void initializeSameDirection(void)
{
  uint32_t i, j;
  CYCLE cycle;
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->sameDirection = &InitializeCycleSetSets[nextSetOfCycleSets];
    nextSetOfCycleSets += cycle->length;
    for (j = 1; j < cycle->length; j++) {
      cycle->sameDirection[j - 1] =
          InitializeCycleSetPairs[cycle->curves[j - 1]][cycle->curves[j]];
    }
    cycle->sameDirection[j - 1] =
        InitializeCycleSetPairs[cycle->curves[j - 1]][cycle->curves[0]];
  }

  assert(nextSetOfCycleSets == NCYCLE_ENTRIES);
}
static void initializeOppositeDirection(void)
{
  uint32_t i, j;
  CYCLE cycle;
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->oppositeDirection = &InitializeCycleSetSets[nextSetOfCycleSets];
    nextSetOfCycleSets += cycle->length;
    for (j = 2; j < cycle->length; j++) {
      cycle->oppositeDirection[j - 1] =
          InitializeCycleSetTriples[cycle->curves[j]][cycle->curves[j - 1]]
                                   [cycle->curves[j - 2]];
    }
    cycle->oppositeDirection[j - 1] =
        InitializeCycleSetTriples[cycle->curves[0]][cycle->curves[j - 1]]
                                 [cycle->curves[j - 2]];
    cycle->oppositeDirection[0] =
        InitializeCycleSetTriples[cycle->curves[1]][cycle->curves[0]]
                                 [cycle->curves[j - 1]];
  }

  assert(nextSetOfCycleSets == 2 * NCYCLE_ENTRIES);
}

static void initializeOmittingCycleSets()
{
  uint32_t i, j, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
      if (!memberOfColorSet(i, Cycles[cycleId].colors)) {
        initializeCycleSetAdd(cycleId, CycleSetOmittingOneColor[i]);
      }
    }
  }
  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (!(memberOfColorSet(i, Cycles[cycleId].colors) &&
              memberOfColorSet(j, Cycles[cycleId].colors) &&
              cycleContainsAthenB(&Cycles[cycleId], i, j))) {
          initializeCycleSetAdd(cycleId, CycleSetOmittingColorPair[i][j]);
        }
      }
    }
  }
}

int colorToChar(COLOR c) { return 'a' + c; }

char* colorSetToStr(char* dbuffer, COLORSET colors)
{
  uint32_t i;
  char* p = dbuffer;
  *p++ = '|';
  for (i = 0; i < NCOLORS; i++) {
    if (colors & (1u << i)) {
      *p++ = 'a' + i;
    }
  }
  *p++ = '|';
  *p = '\0';
  return dbuffer;
}

char* dynamicCycleToStr(char* dbuffer, CYCLE cycle)
{
  char* p = dbuffer;
  if (cycle == NULL) {
    return "(NULL)";
  }
  *p++ = '(';
  for (uint32_t i = 0; i < cycle->length; i++) {
    *p++ = 'a' + cycle->curves[i];
  }
  *p++ = ')';
  *p = '\0';
  return dbuffer;
}
