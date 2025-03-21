/**
 * This file sets up complex data structures:
 * - each of the cycles
 * - the relationship between cycles and each other
 * - which cycles a priori can't be used with which face
 * etc.
 */
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* In this file we construct the constants used elsewhere. */
#define STATIC

#include "graph.h"
#include "point.h"
#include "trail.h"
#include "utils.h"

static int nextCycle = 0;
static int nextSetOfCycleSets = 0;

/*
These cycleSets are accessed from cycles, with the pointers set up during
initialization.
 */
CYCLESET_DECLARE InitializeCycleSetPairs[NCOLORS][NCOLORS];
CYCLESET_DECLARE InitializeCycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
CYCLESET_DECLARE CycleSetOmittingOne[NCOLORS];
CYCLESET_DECLARE CycleSetOmittingPair[NCOLORS][NCOLORS];
CYCLESET InitializeCycleSetSets[NCYCLE_ENTRIES * 2];

STATIC struct face Faces[NFACES];
uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];
uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
uint64_t EdgeCurvesComplete[NCOLORS];

static void initializeCycles(void);
static void initializeCycleSets(void);
static void initializeSameDirection(void);
static void initializeOppositeDirection(void);
static void initializeOmittingCycleSets(void);
static void initializeFacesAndEdges(void);
static void initializePossiblyTo(void);
static void initializeLengthOfCycleOfFaces(void);
static void applyMonotonicity(void);
static void recomputeCountOfChoices(FACE face);

extern void resetGlobals(void);
extern void resetInitialize(void);

void resetGlobals()
{
  memset(Faces, 0, sizeof(Faces));
  memset(EdgeCountsByDirectionAndColor, 0,
         sizeof(EdgeCountsByDirectionAndColor));
  memset(FaceSumOfFaceDegree, 0, sizeof(FaceSumOfFaceDegree));
  memset(EdgeCrossingCounts, 0, sizeof(EdgeCrossingCounts));
  memset(EdgeCurvesComplete, 0, sizeof(EdgeCurvesComplete));
}
void resetInitialize()
{
  memset(InitializeCycleSetPairs, 0, sizeof(InitializeCycleSetPairs));
  memset(InitializeCycleSetTriples, 0, sizeof(InitializeCycleSetTriples));
  memset(InitializeCycleSetSets, 0, sizeof(InitializeCycleSetSets));
  memset(CycleSetOmittingOne, 0, sizeof(CycleSetOmittingOne));
  memset(CycleSetOmittingPair, 0, sizeof(CycleSetOmittingPair));

  nextCycle = 0;
  nextSetOfCycleSets = 0;
  resetPoints();
  resetCyclesetWithoutColor();
}

void initialize()
{
  /* Not true on all architectures, but assumed in our DynamicTrail. */
  assert((sizeof(uint64_t) == sizeof(void *)));
  assert(nextCycle == 0);
  initializeCycles();
  assert(nextCycle == ARRAY_LEN(Cycles));
  initializeCycleSets();
  initializeSameDirection();
  assert(nextSetOfCycleSets == NCYCLE_ENTRIES);
  initializeOppositeDirection();
  assert(nextSetOfCycleSets == 2 * NCYCLE_ENTRIES);
  initializeOmittingCycleSets();

  initializeFacesAndEdges();
#if POINT_DEBUG
  for (uint32_t i = 0; i < NFACES; i++) {
    dynamicFacePrint(Faces + i);
  }
#endif
  initializePossiblyTo();
  initializePoints();
  applyMonotonicity();

  initializeDynamicCounters();
  initializeCyclesetWithoutColor();
  initializeLengthOfCycleOfFaces();
}

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
}

static void addCycle(int length, ...)
{
  uint32_t color;
  va_list ap;
  CYCLE cycle = &Cycles[nextCycle++];
  int ix = 0;
  cycle->colors = 0;
  cycle->length = length;
  va_start(ap, length);
  for (ix = 0; ix < length; ix++) {
    color = va_arg(ap, uint32_t);
    cycle->curves[ix] = color;
    cycle->colors |= 1u << color;
  }
  va_end(ap);
}

static void initializeCycles(void)
{
  uint32_t c1, c2, c3, c4, c5, c6;
  for (c1 = 0; c1 < NCOLORS; c1++) {
    for (c2 = c1 + 1; c2 < NCOLORS; c2++) {
      for (c3 = c1 + 1; c3 < NCOLORS; c3++) {
        if (c3 == c2) {
          continue;
        }
        addCycle(3, c1, c2, c3);
        for (c4 = c1 + 1; c4 < NCOLORS; c4++) {
          if (c4 == c2 || c4 == c3) {
            continue;
          }
          addCycle(4, c1, c2, c3, c4);
          for (c5 = c1 + 1; c5 < NCOLORS; c5++) {
            if (c5 == c2 || c5 == c3 || c5 == c4) {
              continue;
            }
            addCycle(5, c1, c2, c3, c4, c5);
            for (c6 = c1 + 1; c6 < NCOLORS; c6++) {
              if (c6 == c2 || c6 == c3 || c6 == c4 || c6 == c5) {
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
  for (i = 0; i < NCOLORS; i++) {
    for (j = 0; j < NCOLORS; j++) {
      if (i == j) {
        continue;
      }
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (cycleContainsAthenB(&Cycles[cycleId], i, j)) {
          initializeCycleSetAdd(cycleId, InitializeCycleSetPairs[i][j]);
        }
      }
      for (k = 0; k < NCOLORS; k++) {
        if (i == k || j == k) {
          continue;
        }
        for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
          if (cycleContainsAthenBthenC(&Cycles[cycleId], i, j, k)) {
            initializeCycleSetAdd(cycleId, InitializeCycleSetTriples[i][j][k]);
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
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->sameDirection = &InitializeCycleSetSets[nextSetOfCycleSets];
    nextSetOfCycleSets += cycle->length;
    for (j = 1; j < cycle->length; j++) {
      cycle->sameDirection[j - 1] =
          InitializeCycleSetPairs[cycle->curves[j - 1]][cycle->curves[j]];
    }
    cycle->sameDirection[j - 1] =
        InitializeCycleSetPairs[cycle->curves[j - 1]][cycle->curves[0]];
  }
}
static void initializeOppositeDirection(void)
{
  uint32_t i, j;
  CYCLE cycle;
  for (i = 0, cycle = Cycles; i < NCYCLES; i++, cycle++) {
    cycle->oppositeDirection = &InitializeCycleSetSets[nextSetOfCycleSets];
    nextSetOfCycleSets += cycle->length;
    for (j = 2; j < cycle->length; j++) {
      cycle->oppositeDirection[j - 1] =
          InitializeCycleSetTriples[cycle->curves[j]][cycle->curves[j - 1]]
                                   [cycle->curves[j - 2]];
    }
    cycle->oppositeDirection[j - 1] =
        InitializeCycleSetTriples[cycle->curves[0]][cycle->curves[j - 1]]
                                 [cycle->curves[j - 2]];
    cycle->oppositeDirection[0] =
        InitializeCycleSetTriples[cycle->curves[1]][cycle->curves[0]]
                                 [cycle->curves[j - 1]];
  }
}

static void initializeOmittingCycleSets()
{
  uint32_t i, j, cycleId;
  for (i = 0; i < NCOLORS; i++) {
    for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
      if (!memberOfColorSet(i, Cycles[cycleId].colors)) {
        initializeCycleSetAdd(cycleId, CycleSetOmittingOne[i]);
      }
    }
  }
  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      for (cycleId = 0; cycleId < NCYCLES; cycleId++) {
        if (!(memberOfColorSet(i, Cycles[cycleId].colors) &&
              memberOfColorSet(j, Cycles[cycleId].colors) &&
              cycleContainsAthenB(&Cycles[cycleId], i, j))) {
          initializeCycleSetAdd(cycleId, CycleSetOmittingPair[i][j]);
        }
      }
    }
  }
}

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

static void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color, j;
  FACE face, adjacent;
  EDGE edge;
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    face->colors = facecolors;
    for (j = 0; j < CYCLESET_LENGTH - 1; j++) {
      face->possibleCycles[j] = ~0;
    }
    face->possibleCycles[j] = FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET;

    for (color = 0; color < NCOLORS; color++) {
      uint32_t colorbit = (1 << color);
      adjacent = Faces + (facecolors ^ (colorbit));
      face->adjacentFaces[color] = adjacent;
      edge = &face->edges[color];
      edge->colors = face->colors;
      edge->level = __builtin_popcount(face->colors);
      edge->color = color;
      edge->reversed = &adjacent->edges[color];
    }
  }
}

static void initializePossiblyTo(void)
{
  uint32_t facecolors, color, othercolor;
  FACE face;
  EDGE edge;
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
#if POINT_DEBUG
    dynamicFacePrint(face);
#endif
    for (color = 0; color < NCOLORS; color++) {
      edge = &face->edges[color];
      for (othercolor = 0; othercolor < NCOLORS; othercolor++) {
        if (othercolor == color) {
          continue;
        }
        edge->possiblyTo[othercolor].point =
            dynamicPointAdd(face, edge, othercolor);
      }
    }
  }
}

/*
A FISC is isomorphic to a convex FISC if and only if it is monotone.
A FISC is monotone if its dual has a unique source (no incoming edges) and a
unique sink (no out-going edges).
*/
static void applyMonotonicity(void)
{
  uint32_t colors, cycleId;
  FACE face;
  CYCLE cycle;
  uint32_t chainingCount, i;
  uint64_t currentXor, previousFaceXor, nextFaceXor;
#define ONE_IN_ONE_OUT_CORE(a, b, colors)                              \
  (__builtin_popcountll((currentXor = ((1ll << (a)) | (1ll << (b)))) & \
                        colors) == 1)
#define ONE_IN_ONE_OUT(a, b, colors)                               \
  (!ONE_IN_ONE_OUT_CORE(a, b, colors) ? 0                          \
   : ((1 << (a)) & colors)            ? (nextFaceXor = currentXor) \
                                      : (previousFaceXor = currentXor))
  /* The inner face is NFACES-1, with all the colors; the outer face is 0, with
   * no colors.
   */
  for (colors = 1, face = Faces + 1; colors < NFACES - 1; colors++, face++) {
    for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
      if ((cycle->colors & colors) == 0 || (cycle->colors & ~colors) == 0) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      }
      previousFaceXor = nextFaceXor = 0;
      chainingCount = ONE_IN_ONE_OUT(cycle->curves[cycle->length - 1],
                                     cycle->curves[0], colors)
                          ? 1
                          : 0;
      for (i = 1; i < cycle->length; i++) {
        if (ONE_IN_ONE_OUT(cycle->curves[i - 1], cycle->curves[i], colors)) {
          chainingCount++;
        }
      }
      if (chainingCount != 2) {
        initializeCycleSetRemove(cycleId, face->possibleCycles);
      } else {
        assert(previousFaceXor);
        assert(nextFaceXor);
        face->nextByCycleId[cycleId] = Faces + (colors ^ nextFaceXor);
        face->previousByCycleId[cycleId] = Faces + (colors ^ previousFaceXor);
      }
    }
    recomputeCountOfChoices(face);
  }
  setCycleLength(0, NCOLORS);
  setCycleLength(~0, NCOLORS);
}

static void recomputeCountOfChoices(FACE face)
{
  dynamicTrailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
}

static void removeFromCycleSetWithTrail(uint32_t cycleId, CYCLESET cycleSet)
{
  assert(cycleId < NCYCLES);
  dynamicTrailSetInt(
      &cycleSet[cycleId / BITS_PER_WORD],
      cycleSet[cycleId / BITS_PER_WORD] & ~(1ul << (cycleId % BITS_PER_WORD)));
}

bool setCycleLength(uint32_t faceColors, uint32_t length)
{
  FACE face = Faces + (faceColors & (NFACES - 1));
  CYCLE cycle;
  uint32_t cycleId;
  if (length == 0) {
    return true;
  }
  for (cycleId = 0, cycle = Cycles; cycleId < NCYCLES; cycleId++, cycle++) {
    if (cycle->length != length) {
      removeFromCycleSetWithTrail(cycleId, face->possibleCycles);
    }
  }
  recomputeCountOfChoices(face);
  return face->cycleSetSize != 0;
}
