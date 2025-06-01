/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"

#include "failure.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

/* Global variables */
struct face Faces[NFACES];
static uint64 FaceSumOfFaceDegree[NCOLORS + 1];

/* Static function declarations */
static void initializePossiblyTo(void);
static void applyMonotonicity(void);
static void initializeLengthOfCycleOfFaces(void);
static bool isCycleValidForFace(CYCLE cycle, COLORSET faceColors);
static bool isEdgeTransition(COLOR curve1, COLOR curve2, COLORSET faceColors,
                             COLORSET* previousFaceColors,
                             COLORSET* nextFaceColors);
static bool exactlyTwoEdgeTransitions(CYCLE cycle, COLORSET faceColors,
                                      COLORSET* previousFaceColors,
                                      COLORSET* nextFaceColors);
static void facePrint(FACE face);

/* Core face initialization */
void initializeFacesAndEdges(void)
{
  uint32_t facecolors, color;
  FACE face, adjacent;
  EDGE edge;
  if (Faces[1].colors == 0) {
    statisticIncludeInteger(&CycleForcedCounter, "+", "forced", false);
    statisticIncludeInteger(&CycleSetReducedCounter, "-", "reduced", true);
    initializeLengthOfCycleOfFaces();
    for (facecolors = 0, face = Faces; facecolors < NFACES;
         facecolors++, face++) {
      face->colors = facecolors;
      initializeCycleSetUniversal(face->possibleCycles);

      for (color = 0; color < NCOLORS; color++) {
        uint32_t colorbit = (1 << color);
        adjacent = Faces + (facecolors ^ (colorbit));
        face->adjacentFaces[color] = adjacent;
        edge = &face->edges[color];
        edge->colors = face->colors;
        edge->color = color;
        edge->reversed = &adjacent->edges[color];
      }
    }
    applyMonotonicity();
    initializePossiblyTo();
  }
}

/* Public utility functions */
FACE faceFromColors(char* colors)
{
  int faceId = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    faceId |= (1 << (*colors - 'a'));
    colors++;
  }
  return Faces + faceId;
}

void facePrintSelected(void)
{
  uint32_t i;
  FACE face;
  for (i = 0, face = Faces; i < NFACES; i++, face++) {
    if (face->cycle || face->cycleSetSize < 2) {
      facePrint(face);
    }
  }
}

char* faceToString(FACE face)
{
  char* buffer = getBuffer();
  char* colorBuf = colorSetToString(face->colors);
  char* cycleBuf = cycleToString(face->cycle);

  if (face->cycleSetSize > 1) {
    sprintf(buffer, "%s%s^%llu", colorBuf, cycleBuf, face->cycleSetSize);
  } else {
    sprintf(buffer, "%s%s", colorBuf, cycleBuf);
  }
  return usingBuffer(buffer);
}

/* Static helper functions */
static void facePrint(FACE face)
{
  printf("%s\n", faceToString(face));
}

static void initializePossiblyTo(void)
{
  uint32_t facecolors, color, othercolor;
  FACE face;
  EDGE edge;
  for (facecolors = 0, face = Faces; facecolors < NFACES;
       facecolors++, face++) {
    for (color = 0; color < NCOLORS; color++) {
      edge = &face->edges[color];
      for (othercolor = 0; othercolor < NCOLORS; othercolor++) {
        if (othercolor == color) {
          continue;
        }
        edge->possiblyTo[othercolor].vertex =
            initializeVertexIncomingEdge(face->colors, edge, othercolor);
      }
    }
  }
}

static bool isCycleValidForFace(CYCLE cycle, COLORSET faceColors)
{
  return (cycle->colors & faceColors) != 0 &&
         (cycle->colors & ~faceColors) != 0;
}

static bool isEdgeTransition(COLOR curve1, COLOR curve2, COLORSET faceColors,
                             COLORSET* previousFaceColors,
                             COLORSET* nextFaceColors)
{
  uint64 currentXor = (1ll << curve1) | (1ll << curve2);
  if (__builtin_popcountll(currentXor & faceColors) != 1) {
    return false;
  }

  if ((1 << curve1) & faceColors) {
    assert(*nextFaceColors == 0);
    *nextFaceColors = faceColors ^ currentXor;
  } else {
    assert(*previousFaceColors == 0);
    *previousFaceColors = faceColors ^ currentXor;
  }
  return true;
}

static bool exactlyTwoEdgeTransitions(CYCLE cycle, COLORSET faceColors,
                                      COLORSET* previousFaceColors,
                                      COLORSET* nextFaceColors)
{
  uint32_t count = 0;
  COLORSET dummy = 0;

  // Check transition from last to first curve
  if (isEdgeTransition(cycle->curves[cycle->length - 1], cycle->curves[0],
                       faceColors, previousFaceColors, nextFaceColors)) {
    count++;
  }

  // Check transitions between consecutive curves
  for (uint32_t i = 1; i < cycle->length; i++) {
    if (isEdgeTransition(cycle->curves[i - 1], cycle->curves[i], faceColors,
                         previousFaceColors, nextFaceColors)) {
      count++;
      switch (count) {
        case 2:
          // Both *previousFaceColors and *nextFaceColors have been set,
          // Change them to preserve invariants expected by further calls to
          // isEdgeTransition.
          previousFaceColors = &dummy;
          nextFaceColors = &dummy;
          break;
        case 3:
          return false;
      }
    }
  }

  return count == 2;
}

/*
 This is not called "dynamicApplyMonotonicity" because we cannot
 backtrack over it. Instead, it is called from the initialize predicate,
 which calls trailFreeze as its last operation, which disables the backtracking.
 */
static void applyMonotonicity(void)
{
  // The inner face is NFACES-1, with all the colors; the outer face is 0, with
  // no colors.
  for (COLORSET faceColors = 1; faceColors < NFACES - 1; faceColors++) {
    FACE face = Faces + faceColors;
    for (uint32_t cycleId = 0; cycleId < NCYCLES; cycleId++) {
      CYCLE cycle = Cycles + cycleId;
      if (!isCycleValidForFace(cycle, faceColors)) {
        cycleSetRemove(cycleId, face->possibleCycles);
        continue;
      }

      COLORSET previousFaceColors = 0, nextFaceColors = 0;
      bool twoTransitions = exactlyTwoEdgeTransitions(
          cycle, faceColors, &previousFaceColors, &nextFaceColors);

      if (!twoTransitions) {
        cycleSetRemove(cycleId, face->possibleCycles);
      } else {
        assert(previousFaceColors);
        assert(nextFaceColors);
        face->nextByCycleId[cycleId] = Faces + nextFaceColors;
        face->previousByCycleId[cycleId] = Faces + previousFaceColors;
      }
    }
    dynamicRecomputeCountOfChoices(face);
  }

  dynamicFaceSetCycleLength(0, NCOLORS);
  dynamicFaceSetCycleLength(~0, NCOLORS);
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
