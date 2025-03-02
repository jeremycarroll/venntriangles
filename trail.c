
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

void setDynamicInt(uint_trail* ptr, uint_trail value)
{
  trail->ptr = ptr;
  trail->value = *ptr;
  trail++;
  *ptr = value;
}

void backtrackTo(TRAIL backtrackPoint)
{
  while (trail > backtrackPoint) {
    trail--;
    *(uint_trail*)trail->ptr = trail->value;
  }
}

void resetTrail() { trail = trailarray; }
