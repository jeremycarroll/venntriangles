/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "cycle.h"

#include "visible_for_testing.h"

#include <string.h>

struct facialCycle Cycles[NCYCLES];
static int NextCycle = 0;

static void addCycle(int length, COLOR* colors);
static void initializeCyclesWithLengthArrayAndMaxValue(uint32_t length,
                                                       uint32_t position,
                                                       COLOR* colors);
static void initializeCyclesWithLengthAndMaxValue(uint32_t length, COLOR color);
static void initializeCyclesWithMaxValue(COLOR color);

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

static void initializeCyclesWithLengthArrayAndMaxValue(uint32_t length,
                                                       uint32_t position,
                                                       COLOR* colors)
{
  uint32_t nextColor, i;
  COLOR swap;
  if (position == length) {
    for (i = 1; i < length; i++) {
      addCycle(length, colors);
      swap = colors[i];
      colors[i] = colors[i + 1];
      colors[i + 1] = swap;
    }
    for (; i > 1; i--) {
      swap = colors[i];
      colors[i] = colors[i - 1];
      colors[i - 1] = swap;
    }
  } else {
    for (nextColor = colors[1] - 1; nextColor > colors[0]; nextColor--) {
      for (i = 0; i < position; i++) {
        if (colors[i] == nextColor) goto skip;
      }
      colors[position] = nextColor;
      initializeCyclesWithLengthArrayAndMaxValue(length, position + 1, colors);
    skip:;
    }
  }
}

static void initializeCyclesWithLengthAndMaxValue(uint32_t length, COLOR color)
{
  int c1;
  COLOR colors[NCOLORS + 1];
  for (c1 = color - 1; c1 >= 0; c1--) {
    colors[0] = c1;
    colors[1] = color;
    initializeCyclesWithLengthArrayAndMaxValue(length, 2, colors);
  }
}

static void initializeCyclesWithMaxValue(COLOR color)
{
  for (uint32_t length = 3; length <= color + 1; length++) {
    initializeCyclesWithLengthAndMaxValue(length, color);
  }
}

/* Public function implementations */
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
  uint32_t c1;
  for (c1 = 2; c1 < NCOLORS; c1++) {
    initializeCyclesWithMaxValue(c1);
  }

  assert(NextCycle == ARRAY_LEN(Cycles));
}
