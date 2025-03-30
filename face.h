/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FACE_H
#define FACE_H

#include "point.h"
#include "trail.h"

typedef struct face *FACE;

struct face {
  // cycle must be null if cycleSetSize is not 1.
  DYNAMIC struct facialCycle *cycle;
  DYNAMIC TRAIL backtrack;
  /* We point to previous and next with the same number of colors. */
  DYNAMIC FACE previous;
  DYNAMIC FACE next;
  STATIC COLORSET colors;           // holds up to NFACES
  DYNAMIC uint_trail cycleSetSize;  // holds up to NCYCLES
  DYNAMIC CYCLESET_DECLARE possibleCycles;
  STATIC struct face *adjacentFaces[NCOLORS];
  STATIC struct edge edges[NCOLORS];
  /* Precomputed lookup tables for previous and next. */
  STATIC FACE previousByCycleId[NCYCLES];
  STATIC FACE nextByCycleId[NCYCLES];
};

extern uint64_t CycleForcedCounter;
extern uint64_t CycleSetReducedCounter;

extern STATIC struct face Faces[NFACES];
extern uint64_t FaceSumOfFaceDegree[NCOLORS + 1];

/* These next two functions are actually defined in search.c */
extern FAILURE dynamicFaceBacktrackableChoice(FACE face);
extern FAILURE dynamicFaceChoice(FACE face, int depth);

extern FAILURE faceFinalCorrectnessChecks(void);
extern void dynamicFaceSetupCentral(int *faceDegrees);
extern FAILURE dynamicFaceIncludePoint(FACE face, COLOR aColor, COLOR bColor,
                                       int depth);
extern bool dynamicColorRemoveFromSearch(COLOR color);

extern COLORSET ColorCompleted;

extern uint64_t CycleGuessCounter;
extern char *faceToStr(FACE face);

extern FACE faceFromColors(char *colors);
extern FACE dynamicFaceAddSpecific(char *colors, char *cycle);
extern void facePrintSelected(void);
extern void facePrint(FACE face);
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, uint32_t length);
extern void initializeFacesAndEdges(void);
extern void resetFaces(void);
extern FAILURE restrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                          int depth);
extern FAILURE propogateChoice(FACE face, EDGE edge, int depth);

#endif  // FACE_H
