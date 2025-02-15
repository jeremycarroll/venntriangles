
/**
 * This file sets up complex data structures:
 * - each of the cycles
 * - the relationship between cycles and each other
 * - which cycles a priori can't be used with which face
 * etc.
 */
#include "venn.h"
#include <stdarg.h>

static int nextCycle = 0;
static int nextEdge = 0;

/*
These cycleSets are accessed from cycles, with the pointers set up during initialization.
 */
CYCLESET pairs2cycleSets[NCURVES][NCURVES];
CYCLESET triples2cycleSets[NCURVES][NCURVES][NCURVES];

static void initializeCycles();
static void initializeCycleSets();
static void initializeSameDirection();
static void initializeOppositeDirection();
static void initializeFacesAndEdges();

void initialize()
{
    assert(ASSUMPTION);
    initializeCycles();
    assert(nextCycle == ARRAY_LEN(cycles));
    initializeCycleSets();
    // initializeSameDirection();
    // initializeOppositeDirection();

    // initializeFacesAndEdges();
    /* The FISC is simple, so there are four edges for each point, and two points for each edge. */
    //assert(nextEdge == ARRAY_LEN(edges));
}

static void addCycle(int length, ...)
{
    va_list ap;
    CYCLE cycle = &cycles[nextCycle++];
    int ix = 0;
    cycle->length = length;
    va_start(ap, length); // Requires the last fixed parameter (to get the address)
    for (ix = 0; ix < length; ix++)
        cycle->curves[ix] = va_arg(ap, uint32_t); // Requires the type to cast to. Increments ap to the next argument.
    va_end(ap);
    cycle->curves[ix] = NO_COLOR;
}

static void initializeCycles()
{
    uint32_t c1, c2, c3, c4, c5, c6;
    for (c1 = 0; c1 < NCURVES; c1++)
    {
        for (c2 = c1 + 1; c2 < NCURVES; c2++)
        {
            for (c3 = c1 + 1; c3 < NCURVES; c3++)
            {
                if (c3 == c2)
                {
                    continue;
                }
                addCycle(3, c1, c2, c3);
                for (c4 = c1 + 1; c4 < NCURVES; c4++)
                {
                    if (c4 == c2 || c4 == c3)
                    {
                        continue;
                    }
                    addCycle(4, c1, c2, c3, c4);
                    for (c5 = c1 + 1; c5 < NCURVES; c5++)
                    {
                        if (c5 == c2 || c5 == c3 || c5 == c4)
                        {
                            continue;
                        }
                        addCycle(5, c1, c2, c3, c4, c5);
                        for (c6 = c1 + 1; c6 < NCURVES; c6++)
                        {
                            if (c6 == c2 || c6 == c3 || c6 == c4 || c6 == c5)
                            {
                                continue;
                            }
                            addCycle(6, c1, c2, c3, c4, c5, c6);
                        }
                    }
                }
            }
        }
    }
}

void initializeCycleSets() {
    uint32_t i, j, k, cycleId;
    for (i=0; i<NCURVES; i++) {
        for (j=0; j<NCURVES; j++) {
            if (i == j) {
                continue;
            }
            for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
                if (contains2(&cycles[cycleId], i, j)) {
                    addToSet(cycleId, &pairs2cycleSets[i][j]);
                }
            }
            for (k=0; k<NCURVES; k++) {
                if (i == k || j == k) {
                    continue;
                }
                for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
                    if (contains3(&cycles[cycleId], i, j, k)) {
                        addToSet(cycleId, &triples2cycleSets[i][j][k]);
                    }
                }
            }
        }
    }

}

/* Hmmm
static void addCycle(int length, int i1, int i2, int i3, int i4, int i5, int i6)
{
    Cycle cycle = &cycles[nextCycle++];
    cycle->length = length;
    cycle->curves[0] = i1;
    cycle->curves[1] = i2;
    cycle->curves[2] = i3;
    cycle->curves[3] = i4;
    cycle->curves[4] = i5;
    cycle->curves[5] = i6;
    for (int i = 0; i < length; i++)
    {
        face[i1].possibleCycles[i1] = cycle;
    }
}

static void addCycles(int length, int omit1, int omit2, int omit3)
{
#define OMIT(x) (x == omit1 || x == omit2 || x == omit3)
    for (int i1 = 0; i1 < NCURVES; i1++)
    {
        if (OMIT(i1))
        {
            continue;
        }
        for (int i2 = i1 + 1; i2 < NCURVES; i2++)
        {
            if (OMIT(i2))
            {
                continue;
            }
            for (int i3 = i2 + 1; i3 < NCURVES; i3++)
            {
                if (OMIT(i3))
                {
                    continue;
                }
                if (length == 3)
                {
                    addCycle(3, i1, i2, i3);
                }
                else
                {
                    for (int i4 = i3 + 1; i4 < NCURVES; i4++)
                    {
                        if (OMIT(i4))
                        {
                            continue;
                        }
                        if (length == 4)
                        {
                            addCycle(4, i1, i2, i3, i4);
                        }
                        else
                        {
                            for (int i5 = i4 + 1; i5 < NCURVES; i5++)
                            {
                                if (OMIT(i5))
                                {
                                    continue;
                                }
                                if (length == 5)
                                {
                                    addCycle(5, i1, i2, i3, i4, i5);
                                }
                                else
                                {
                                    for (int i6 = i5 + 1; i6 < NCURVES; i6++)
                                    {
                                        if (OMIT(i6))
                                        {
                                            continue;
                                        }
                                        addCycle(5, i1, i2, i3, i4, i5, i6);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
*/
