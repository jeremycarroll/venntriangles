#include "venn.h"

struct trail trailarray[TRAIL_SIZE];
TRAIL trail = trailarray;

void setDynamicPointer_(void** ptr, void* value)
{
  trail->ptr = ptr;
  trail->value = (uint_trail)*ptr;
  trail++;
  *ptr = value;
}

void maybeSetDynamicInt(uint_trail* ptr, uint_trail value)
{
  if (*ptr != value) {
    setDynamicInt(ptr, value);
  }
}

void setDynamicInt(uint_trail* ptr, uint_trail value)
{
  trail->ptr = ptr;
  trail->value = *ptr;
  trail++;
  *ptr = value;
}

bool backtrackTo(TRAIL backtrackPoint)
{
  bool result = false;
  while (trail > backtrackPoint) {
    result = true;
    trail--;
    *(uint_trail*)trail->ptr = trail->value;
  }
  return result;
}

void resetTrail() { trail = trailarray; }
