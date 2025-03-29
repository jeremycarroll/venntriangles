#include "trail.h"

#include "statistics.h"

struct trail TrailArray[TRAIL_SIZE];
TRAIL Trail = TrailArray;
static uint64_t MaxTrailSize = 0;

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

void trailSetInt(uint_trail* ptr, uint_trail value)
{
  Trail->ptr = ptr;
  Trail->value = *ptr;
  Trail++;
  *ptr = value;
}

bool trailBacktrackTo(TRAIL backtrackPoint)
{
  uint64_t trailSize = Trail - TrailArray;
  if (trailSize > MaxTrailSize) {
    MaxTrailSize = trailSize;
  }
  bool result = false;
  while (Trail > backtrackPoint) {
    result = true;
    Trail--;
    *(uint_trail*)Trail->ptr = Trail->value;
  }
  return result;
}

void initializeTrail()
{
  statisticIncludeInteger(&MaxTrailSize, "$", "MaxTrail");
}

void resetTrail() { Trail = TrailArray; }
