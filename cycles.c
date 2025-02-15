
#include "venn.h"
#include "visible_for_testing.h"

void addToSet(uint32_t cycleId, CYCLESET cycleSet)
{
    assert(cycleId < NCYCLES);
    cycleSet[cycleId / BITS_PER_WORD] |= 1ul << (cycleId % BITS_PER_WORD);
}

bool memberOfSet(uint32_t cycleId, CYCLESET cycleSet)
{
    assert(cycleId < NCYCLES);
    return (cycleSet[cycleId / BITS_PER_WORD] >> (cycleId % BITS_PER_WORD)) & 1ul;
}

bool contains2(CYCLE cycle, uint32_t i, uint32_t j)
{
    uint64_t ix;
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
    uint64_t ix;
    for (ix = 2; ix < cycle->length; ix++)
    {
        if (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j && cycle->curves[ix] == k)
        {
            return true;
        }
    }
    return (cycle->curves[ix - 1] == i && cycle->curves[0] == j && cycle->curves[1] == k) || (cycle->curves[ix - 2] == i && cycle->curves[ix - 1] == j && cycle->curves[0] == k);
}

/* See https://en.wikichip.org/wiki/population_count#Software_support */
u_int32_t sizeOfSet(CYCLESET cycleSet)
{
    u_int32_t size = 0;
    for (u_int32_t i = 0; i < CYCLESET_LENGTH; i++)
    {
        size += __builtin_popcountll(cycleSet[i]);
    }
    return size;
}

void printCycle(CYCLE cycle)
{
    for (uint64_t i = 0; i < cycle->length; i++)
    {
        printf("%c", 'a' + cycle->curves[i]);
    }
}
/* returns false if cycleset has bits set past the last. */
bool printCycleSet(CYCLESET cycleSet)
{
    uint32_t lastBit = (NCYCLES - 1) % BITS_PER_WORD;
    uint64_t faulty, i, j;
    if ((faulty = (cycleSet[CYCLESET_LENGTH - 1] & ~((1ul << lastBit) - 1ul))))
    {
        printf("0x%016llx\n", faulty);
        return false;
    }
    putchar('{');
    for (i = 0; i < CYCLESET_LENGTH; i++)
    {
        if (cycleSet[i])
        {
            for (j = 0; j < 64; j++)
            {
                if (cycleSet[i] & (1ul << j))
                {
                    putchar(' ');
                    printCycle(cycles + i);
                    putchar(',');
                }
            }
        }
    }
    printf(" }\n");
    return true;
}