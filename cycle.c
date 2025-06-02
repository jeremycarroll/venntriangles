/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "cycle.h"

#include "visible_for_testing.h"

#include <string.h>

struct facialCycle Cycles[NCYCLES];
static int NextCycle = 0;

static void addCycle(int length, COLOR* colors)
{
  uint32_t color;
  CYCLE cycle = &Cycles[NextCycle++];
  int ix = 0;
  cycle->colors = 0;
  cycle->length = length;
  for (ix = 0; ix < length; ix++) {
    color = colors[ix];
    cycle->curves[ix] = color;
    cycle->colors |= 1u << color;
  }
}
static bool isCycleValid(int length, COLOR max, COLOR* cycle)
{
  bool hasMax = false;

  // Duplicates prohibited.
  bool used[NCOLORS] = {false};

  for (int i = 0; i < length; i++) {
    // Check if max color is present
    if (cycle[i] == max) {
      hasMax = true;
    }

    if (used[cycle[i]]) {
      return false;
    }
    used[cycle[i]] = true;
    // First element must be lowest
    if (i > 0 && cycle[i] < cycle[0]) {
      return false;
    }
  }

  return hasMax;
}

static void initializeCyclesWithMaxAndLength(int length, COLOR max)
{
  COLOR current[NCOLORS];

  // Start with the maximum sequence
  for (int i = 0; i < length; i++) {
    current[i] = max;
  }

  // Generate all possible sequences with this length in reverse
  // lexicographic order
  bool hasMoreSequences = true;
  while (hasMoreSequences) {
    if (isCycleValid(length, max, current)) {
      addCycle(length, current);
    }
    // Find the rightmost element that can be decremented
    int pos = length - 1;
    while (pos >= 0) {
      if (current[pos] > 0) {
        current[pos]--;
        break;
      }
      pos--;
    }

    // If we couldn't decrement any position, we're done with this length
    if (pos < 0) {
      hasMoreSequences = false;
    } else {
      // Reset all positions to the right to max
      for (int i = pos + 1; i < length; i++) {
        current[i] = max;
      }
    }
  }
}
/**
 * We initialize the cycles ensuring
 * that the cycles including k come after all the cycles
 * from 0, ... k-1; and come in increasing order of length.
 * We also require that 0,1,2,3... k-1 is last.
 */
static void initializeAllCycles(void)
{
  for (COLOR max = 2; max < NCOLORS; max++) {
    for (unsigned length = 3; length <= max + 1; length++) {
      initializeCyclesWithMaxAndLength(length, max);
    }
  }
}

bool cycleContainsAthenB(CYCLE cycle, uint32_t i, uint32_t j)
{
  uint64 ix;
  for (ix = 1; ix < cycle->length; ix++) {
    if (cycle->curves[ix - 1] == i && cycle->curves[ix] == j) {
      return true;
    }
  }
  return cycle->curves[ix - 1] == i && cycle->curves[0] == j;
}

bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k)
{
  // Create an extended array that includes the first two elements at the end
  COLOR extended[cycle->length + 2];
  memcpy(extended, cycle->curves, cycle->length * sizeof(COLOR));
  extended[cycle->length] = cycle->curves[0];
  extended[cycle->length + 1] = cycle->curves[1];

  // Look for the sequence in the extended array
  for (uint64 ix = 0; ix < cycle->length; ix++) {
    if (extended[ix] == i && extended[ix + 1] == j && extended[ix + 2] == k) {
      return true;
    }
  }
  return false;
}

CYCLE_ID cycleIdFromColors(char* colors)
{
  COLOR cycle[NCOLORS];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return getCycleId(cycle, i);
}

char* cycleToString(CYCLE cycle)
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

CYCLE_ID getCycleId(COLOR* cycle, uint32_t length)
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

/* Reverses e.g. 1,2,3 to 1,3,2 i.e. the mirror image of the cycle. */
CYCLE_ID cycleIdReverseDirection(CYCLE_ID forwardCycleId)
{
  COLOR reversed[NCOLORS];
  reversed[0] = Cycles[forwardCycleId].curves[0];
  for (uint32_t i = 1; i < Cycles[forwardCycleId].length; i++) {
    reversed[i] =
        Cycles[forwardCycleId].curves[Cycles[forwardCycleId].length - i];
  }
  return getCycleId(reversed, Cycles[forwardCycleId].length);
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

void initializeCycles(void)
{
  assert(NextCycle == 0);
  initializeAllCycles();
  assert(NextCycle == ARRAY_LEN(Cycles));
}
