#include "trail.h"

struct trail DynamicTrailArray[TRAIL_SIZE];
TRAIL DynamicTrail = DynamicTrailArray;

void dynamicTrailSetPointer(void** ptr, void* value)
{
  DynamicTrail->ptr = ptr;
  DynamicTrail->value = (uint_trail)*ptr;
  DynamicTrail++;
  *ptr = value;
}

void dynamicTrailMaybeSetInt(uint_trail* ptr, uint_trail value)
{
  if (*ptr != value) {
    dynamicTrailSetInt(ptr, value);
  }
}

void dynamicTrailSetInt(uint_trail* ptr, uint_trail value)
{
  DynamicTrail->ptr = ptr;
  DynamicTrail->value = *ptr;
  DynamicTrail++;
  *ptr = value;
}

bool dynamicTrailBacktrackTo(TRAIL backtrackPoint)
{
  bool result = false;
  while (DynamicTrail > backtrackPoint) {
    result = true;
    DynamicTrail--;
    *(uint_trail*)DynamicTrail->ptr = DynamicTrail->value;
  }
  return result;
}

void resetTrail() { DynamicTrail = DynamicTrailArray; }
