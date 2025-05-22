/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "trail.h"

#include "statistics.h"

/* Global variables (file scoped static) */
static struct trail TrailArray[TRAIL_SIZE];
TRAIL Trail = TrailArray;
static TRAIL frozenTrail = NULL;
static uint64 MaxTrailSize = 0;

/* Externally linked functions */
void initializeTrail()
{
  statisticIncludeInteger(&MaxTrailSize, "$", "MaxTrail", true);
}

void trailSetPointer(void** ptr, void* value)
{
  Trail->ptr = ptr;
  Trail->value = (uint_trail)*ptr;
  Trail++;
  *ptr = value;
}

void trailMaybeSetInt(uint_trail* ptr, uint_trail value)
{
  if (*ptr != value) {
    trailSetInt(ptr, value);
  }
}

void trailFreeze()
{
  frozenTrail = Trail;
}

void trailSetInt(uint_trail* ptr, uint_trail value)
{
  Trail->ptr = ptr;
  Trail->value = *ptr;
  Trail++;
  *ptr = value;
}

bool trailBacktrackTo(TRAIL backtrackPoint)
{
  uint64 trailSize = Trail - TrailArray;
  if (trailSize > MaxTrailSize) {
    MaxTrailSize = trailSize;
  }
  bool result = false;
  if (backtrackPoint < frozenTrail) {
    backtrackPoint = frozenTrail;
  }
  while (Trail > backtrackPoint) {
    result = true;
    Trail--;
    *(uint_trail*)Trail->ptr = Trail->value;
  }
  return result;
}
