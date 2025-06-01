/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"

#include "failure.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

struct face Faces[NFACES];
uint64 FaceSumOfFaceDegree[NCOLORS + 1];

static void initializeLengthOfCycleOfFaces(void)
{
  uint32_t i;
  FaceSumOfFaceDegree[0] = 1;
  for (i = 0; i < NCOLORS; i++) {
    FaceSumOfFaceDegree[i + 1] =
        FaceSumOfFaceDegree[i] * (NCOLORS - i) / (i + 1);
  }
}

static bool isCycleValidForFace(CYCLE cycle, COLORSET faceColors)
{
  return (cycle->colors & faceColors) != 0 &&
         (cycle->colors & ~faceColors) != 0;
}

/*
 * Compare direction of two edges in the dual graph: return true
 * if they are different, and set one of the return values, chosen
 * by direction of first edge.
 */
static bool isEdgeTransition(COLOR curve1, COLOR curve2, COLORSET faceColors,
                             COLORSET* previousFaceColorsReturn,
                             COLORSET* nextFaceColorsReturn)
{
  uint64 currentXor = (1ll << curve1) | (1ll << curve2);
  bool isOutbound1 = COLORSET_HAS_MEMBER(curve1, faceColors);
  bool isOutbound2 = COLORSET_HAS_MEMBER(curve2, faceColors);
  if (isOutbound1 == isOutbound2) {
    return false;
  }
  if (isOutbound1) {
    assert((1 << curve1) & faceColors);
    assert(*nextFaceColorsReturn == 0);
    *nextFaceColorsReturn = faceColors ^ currentXor;
  } else {
    assert(!((1 << curve1) & faceColors));
    assert(*previousFaceColorsReturn == 0);
    *previousFaceColorsReturn = faceColors ^ currentXor;
  }
  return true;
}

/*
 * Does a cycle, in the dual graph, have exactly two edge transitions
 * from incoming to outgoing or vice versa.
 */
static bool exactlyTwoEdgeTransitions(CYCLE cycle, COLORSET faceColors,
                                      COLORSET* previousFaceColorsReturn,
                                      COLORSET* nextFaceColorsReturn)
{
  uint32_t count = 0;
  COLORSET dummy = 0;

  // Check transition from last to first curve (wrap-around case)
  if (isEdgeTransition(cycle->curves[cycle->length - 1], cycle->curves[0],
                       faceColors, previousFaceColorsReturn,
                       nextFaceColorsReturn)) {
    count++;
  }

  // Check transitions between consecutive curves
  for (uint32_t i = 1; i < cycle->length; i++) {
    if (isEdgeTransition(cycle->curves[i - 1], cycle->curves[i], faceColors,
                         previousFaceColorsReturn, nextFaceColorsReturn)) {
      count++;
      switch (count) {
        case 2:
          // After finding two transitions, we've identified both the previous
          // and next faces. Redirect the pointers to a dummy variable to avoid
          // overwriting these values while still allowing us to check for
          // additional transitions.
          previousFaceColorsReturn = &dummy;
          nextFaceColorsReturn = &dummy;
          break;
        case 3:
          // More than two transitions cannot happen for a cycle around a face
          // in a monotone diagram.
          return false;
      }
    }
  }
  return count == 2;
}

/*
 Called from the initialize predicate which disables backtracking
 with trailFreeze.
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

static void facePrint(FACE face)
{
  printf("%s\n", faceToString(face));
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
