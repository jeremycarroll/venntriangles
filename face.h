#ifndef FACE_H
#define FACE_H

#include <stdint.h>

#include "color.h"
#include "core.h"
#include "edge.h"
#include "failure.h"
#include "point.h"
#include "trail.h"

typedef struct face *FACE;

struct face {
  // cycle must be null if cycleSetSize is not 1.
  DYNAMIC struct facial_cycle *cycle;
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

extern void initializeGraph(void);
extern STATIC struct face Faces[NFACES];
extern uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
extern uint64_t FaceSumOfFaceDegree[NCOLORS + 1];
extern uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
extern uint64_t EdgeCurvesComplete[NCOLORS];
extern STATIC struct facial_cycle Cycles[NCYCLES];

extern FAILURE dynamicFaceMakeChoice(FACE face);
extern FAILURE dynamicFaceFinalCorrectnessChecks(void);
extern void initializeFaceSetupCentral(int *faceDegrees);
extern void initializeGraph(void);
extern FAILURE dynamicPointAssign(FACE face, COLOR aColor, COLOR bColor,
                                  int depth);
extern void dynamicSearchFull(void (*foundSolution)(void));
extern void dynamicSearch(bool smallestFirst, void (*foundSolution)(void));

#endif  // FACE_H
