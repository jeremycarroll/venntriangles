
#include "venn.h"

void addToSet(uint32_t cycleId, CYCLESET cycleSet)
{
    cycleSet[cycleId / sizeof(uint64_t)] |= 1 << (cycleId % sizeof(uint64_t));
}

bool contains2(CYCLE cycle, uint32_t i, uint32_t j)
{
    int ix;
    for (ix = 1; ix < cycle->length; ix++)
    {
        if (cycle->curves[ix - 1] == i && cycle->curves[ix] == j)
        {
            return true;
        }
    }
    return cycle->curves[ix - 1] == i && cycle->curves[0] == j;
}

bool contains3(CYCLE cycle, uint32_t i, uint32_t j, uint32_t k)
{
    int ix;
    for (ix = 2; ix < cycle->length; ix++)
    {
        if (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j && cycle->curves[ix] == k)
        {
            return true;
        }
    }
    return (cycle->curves[ix - 1] == i && cycle->curves[0] == j && cycle->curves[1] == k) 
        || (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j && cycle->curves[0] == k);
}