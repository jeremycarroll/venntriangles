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
static int nextSetOfCycleSets = 0;

/*
These cycleSets are accessed from cycles, with the pointers set up during initialization.
 */
CYCLESET_DECLARE pairs2cycleSets[NCURVES][NCURVES];
CYCLESET_DECLARE triples2cycleSets[NCURVES][NCURVES][NCURVES];
CYCLESET setsOfCycleSets[NCYCLE_ENTRIES * 2];

static void initializeCycles(void);
static void initializeCycleSets(void);
static void initializeSameDirection(void);
static void initializeOppositeDirection(void);
static void initializeFacesAndEdges(void);
static void applyMonotonicity(void);
static void recomputeCountOfChoices(FACE face);
/* face is truncated to 6 bits, higher bits may be set, and will be ignored. */
void setCycleLength(uint32_t faceColors, uint32_t length);

void clearInitialize()
{
    memset(pairs2cycleSets, 0, sizeof(pairs2cycleSets));
    memset(triples2cycleSets, 0, sizeof(triples2cycleSets));
    memset(setsOfCycleSets, 0, sizeof(setsOfCycleSets));
    nextCycle = 0;
    nextEdge = 0;
    nextSetOfCycleSets = 0;
}

void initialize()
{
    assert(ASSUMPTION);
    initializeCycles();
    assert(nextCycle == ARRAY_LEN(g_cycles));
    initializeCycleSets();
    initializeSameDirection();
    assert(nextSetOfCycleSets == NCYCLE_ENTRIES);
    initializeOppositeDirection();
    assert(nextSetOfCycleSets == 2 * NCYCLE_ENTRIES);

    initializeFacesAndEdges();
    /* The FISC is simple, so there are four edges for each point, and two points for each edge. */
    assert(nextEdge == ARRAY_LEN(g_edges));
    applyMonotonicity();
}

static void addCycle(int length, ...)
{
    uint32_t color;
    va_list ap;
    CYCLE cycle = &g_cycles[nextCycle++];
    int ix = 0;
    cycle->colors = 0;
    cycle->length = length;
    va_start(ap, length);
    for (ix = 0; ix < length; ix++)
    {
        color = va_arg(ap, uint32_t);
        cycle->curves[ix] = color;
        cycle->colors |= 1u << color;
    }
    va_end(ap);
    cycle->curves[ix] = NO_COLOR;
}

static void initializeCycles(void)
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

static void initializeCycleSets(void)
{
    uint32_t i, j, k, cycleId;
    for (i = 0; i < NCURVES; i++)
    {
        for (j = 0; j < NCURVES; j++)
        {
            if (i == j)
            {
                continue;
            }
            for (cycleId = 0; cycleId < NCYCLES; cycleId++)
            {
                if (contains2(&g_cycles[cycleId], i, j))
                {
                    addToCycleSet(cycleId, pairs2cycleSets[i][j]);
                }
            }
            for (k = 0; k < NCURVES; k++)
            {
                if (i == k || j == k)
                {
                    continue;
                }
                for (cycleId = 0; cycleId < NCYCLES; cycleId++)
                {
                    if (contains3(&g_cycles[cycleId], i, j, k))
                    {
                        addToCycleSet(cycleId, triples2cycleSets[i][j][k]);
                    }
                }
            }
        }
    }
}

static void initializeSameDirection(void)
{
    uint32_t i, j;
    CYCLE cycle;
    for (i = 0, cycle = g_cycles; i < NCYCLES; i++, cycle++)
    {
        cycle->sameDirection = &setsOfCycleSets[nextSetOfCycleSets];
        nextSetOfCycleSets += cycle->length;
        for (j = 1; j < cycle->length; j++)
        {
            cycle->sameDirection[j - 1] = pairs2cycleSets[cycle->curves[j - 1]][cycle->curves[j]];
        }
        cycle->sameDirection[j - 1] = pairs2cycleSets[cycle->curves[j - 1]][cycle->curves[0]];
    }
}
static void initializeOppositeDirection(void)
{
    uint32_t i, j;
    CYCLE cycle;
    for (i = 0, cycle = g_cycles; i < NCYCLES; i++, cycle++)
    {
        cycle->oppositeDirection = &setsOfCycleSets[nextSetOfCycleSets];
        nextSetOfCycleSets += cycle->length;
        for (j = 2; j < cycle->length; j++)
        {
            cycle->oppositeDirection[j - 1] = triples2cycleSets[cycle->curves[j]][cycle->curves[j - 1]][cycle->curves[j - 2]];
        }
        cycle->oppositeDirection[j - 1] = triples2cycleSets[cycle->curves[0]][cycle->curves[j - 1]][cycle->curves[j - 2]];
        cycle->oppositeDirection[0] = triples2cycleSets[cycle->curves[1]][cycle->curves[0]][cycle->curves[j - 1]];
    }
}

static void initializeFacesAndEdges(void)
{
    uint32_t facecolors, color, j;
    FACE face, adjacent;
    for (facecolors = 0, face = g_faces; facecolors < NFACES; facecolors++, face++)
    {
        face->colors = facecolors;
        for (j = 0; j < CYCLESET_LENGTH - 1; j++)
        {
            face->possibleCycles[j] = ~0;
        }
        face->possibleCycles[j] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;

        for (color = 0; color < NCURVES; color++)
        {
            uint32_t colorbit = (1 << color);
            adjacent = g_faces + (facecolors ^ (colorbit));
            face->adjacentFaces[color] = adjacent;
            if (adjacent->edges[color] == NULL)
            {
                EDGE edge = g_edges + nextEdge++;
                edge->inner = (facecolors & colorbit) ? face : adjacent;
                edge->outer = (facecolors & colorbit) ? adjacent : face;
                edge->color = j;
                face->edges[color] = edge;
            }
            else
            {
                face->edges[color] = adjacent->edges[color];
            }
        }
    }
}
/*
A FISC is isomorphic to a convex FISC if and only if it is monotone.
A FISC is monotone if its dual has a unique source (no incoming edges) and a unique
sink (no out-going edges).
*/
static void applyMonotonicity(void)
{
    uint32_t colors, cycleId;
    FACE face;
    CYCLE cycle;
    /* The inner face is NFACES-1, with all the colors; the outer face is 0, with no colors.
     */
    for (colors = 1, face = g_faces + 1; colors < NFACES - 1; colors++, face++)
    {
        for (cycleId = 0, cycle = g_cycles; cycleId < NCYCLES; cycleId++, cycle++)
        {
            if ((cycle->colors & colors) == 0 || (cycle->colors & ~colors) == 0)
            {
                removeFromCycleSet(cycleId, face->possibleCycles);
            }
        }
        recomputeCountOfChoices(face);
    }
    setCycleLength(0, NCURVES);
    setCycleLength(~0, NCURVES);
}

static void recomputeCountOfChoices(FACE face)
{
    face->cycleSetSize = sizeOfCycleSet(face->possibleCycles);
}

void setCycleLength(uint32_t faceColors, uint32_t length)
{
    FACE face = g_faces + (faceColors & (NFACES - 1));
    CYCLE cycle;
    uint32_t cycleId;
    for (cycleId = 0, cycle = g_cycles; cycleId < NCYCLES; cycleId++, cycle++)
    {
        if (cycle->length != length)
        {
            removeFromCycleSet(cycleId, face->possibleCycles);
        }
    }
    recomputeCountOfChoices(face);
}
