/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "color.h"

#include "trail.h"
#include "utils.h"

#include <stdarg.h>

/* Global variables - globally scoped */
CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];
struct facialCycle Cycles[NCYCLES];

/* Global variables - file scoped */
static int NextCycle = 0;
static int NextSetOfCycleSets = 0;
static CYCLESET CycleSetSets[NCYCLE_ENTRIES * 2];

/*
These cycleSets are accessed from cycles, with the pointers set up during
initialization.
 */

/* Declaration of file scoped static functions */
static void initializeCycles(void);
static void initializeSameDirection(void);
static void initializeOppositeDirection(void);
static void initializeOmittingCycleSets(void);

/* Externally linked functions - initialize... */
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
          cycleSetAdd(cycleId, CycleSetPairs[i][j]);
        }
      }
      for (k = 0; k < NCOLORS; k++) {
        if (i == k || j == k) {
          continue;
        }
        for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
          if (cycleContainsAthenBthenC(&Cycles[cycleId], i, j, k)) {
            cycleSetAdd(cycleId, CycleSetTriples[i][j][k]);
          }
        }
      }
    }
  }
  initializeSameDirection();
  initializeOppositeDirection();
  initializeOmittingCycleSets();
}

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

void initializeCycleSetUniversal(CYCLESET cycleSet)
{
  uint32_t i;
  for (i = 0; i < CYCLESET_LENGTH - 1; i++) {
    cycleSet[i] = ~0;
  }
  cycleSet[i] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;
}

/* Externally linked functions - cycleSet... */
void cycleSetAdd(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] |= 1ul << (cycleId % BITS_PER_WORD);
}

bool cycleSetMember(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  return (cycleSet[cycleId / BITS_PER_WORD] >> (cycleId % BITS_PER_WORD)) & 1ul;
}

void cycleSetRemove(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  cycleSet[cycleId / BITS_PER_WORD] &= ~(1ul << (cycleId % BITS_PER_WORD));
}

CYCLE cycleSetFirst(CYCLESET cycleSet) { return cycleSetNext(cycleSet, NULL); }

CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle)
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

/* See https://en.wikichip.org/wiki/population_count#Software_support */
uint32_t cycleSetSize(CYCLESET cycleSet)
{
  uint32_t size = 0;
  for (uint32_t i = 0; i < CYCLESET_LENGTH; i++) {
    size += __builtin_popcountll(cycleSet[i]);
  }
  return size;
}

/* Externally linked functions - cycle... */
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

uint32_t cycleId(COLOR* cycle, uint32_t length)
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

void removeFromCycleSetWithTrail(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  trailSetInt(
      &cycleSet[cycleId / BITS_PER_WORD],
      cycleSet[cycleId / BITS_PER_WORD] & ~(1ul << (cycleId % BITS_PER_WORD)));
}

void resetCycles()
{
  memset(CycleSetPairs, 0, sizeof(CycleSetPairs));
  memset(CycleSetTriples, 0, sizeof(CycleSetTriples));
  memset(CycleSetSets, 0, sizeof(CycleSetSets));
  memset(CycleSetOmittingOneColor, 0, sizeof(CycleSetOmittingOneColor));
  memset(CycleSetOmittingColorPair, 0, sizeof(CycleSetOmittingColorPair));

  NextCycle = 0;
  NextSetOfCycleSets = 0;
}

/* Externally linked functions - color... */
int colorToChar(COLOR c) { return 'a' + c; }

char* colorSetToStr(COLORSET colors)
{
  uint32_t i;
  char* buffer = getBuffer();
  char* p = buffer;
  *p++ = '|';
  for (i = 0; i < NCOLORS; i++) {
    if (colors & (1u << i)) {
      *p++ = 'a' + i;
    }
  }
  *p++ = '|';
  *p = '\0';
  return usingBuffer(buffer);
}

char* cycleToStr(CYCLE cycle)
{
  char* buffer = getBuffer();
  char* p = buffer;
  if (cycle == NULL) {
    return "(NULL)";
  }
  *p++ = '(';
  for (uint32_t i = 0; i < cycle->length; i++) {
    *p++ = 'a' + cycle->curves[i];
  }
  *p++ = ')';
  *p = '\0';
  return usingBuffer(buffer);
}

uint32_t cycleIdFromColors(char* colors)
{
  COLOR cycle[NCOLORS];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return cycleId(cycle, i);
}

/* File scoped static functions */
static void addCycle(int length, ...)
{
  uint32_t color;
  va_list ap;
  CYCLE cycle = &Cycles[NextCycle++];
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
  assert(NextCycle == 0);
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
  assert(NextCycle == ARRAY_LEN(Cycles));
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

static void initializeOmittingCycleSets()
{
  uint32_t i, j, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
      if (!COLORSET_HAS_MEMBER(i, Cycles[cycleId].colors)) {
        cycleSetAdd(cycleId, CycleSetOmittingOneColor[i]);
      }
    }
  }
  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (!(COLORSET_HAS_MEMBER(i, Cycles[cycleId].colors) &&
              COLORSET_HAS_MEMBER(j, Cycles[cycleId].colors) &&
              cycleContainsAthenB(&Cycles[cycleId], i, j))) {
          cycleSetAdd(cycleId, CycleSetOmittingColorPair[i][j]);
        }
      }
    }
  }
}
