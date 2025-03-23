
#include "face.h"

struct face Faces[NFACES];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];

static void recomputeCountOfChoices(FACE face);
static void initializePossiblyTo(void);
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
  trailSetInt(&face->cycleSetSize, cycleSetSize(face->possibleCycles));
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

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
}

#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color, j;
  FACE face, adjacent;
  EDGE edge;
  initializeLengthOfCycleOfFaces();
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
  applyMonotonicity();
  initializePossiblyTo();
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
