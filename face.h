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

extern STATIC struct face g_faces[NFACES];
extern uint64_t g_edgeCount[2][NCOLORS];
extern uint64_t g_lengthOfCycleOfFaces[NCOLORS + 1];
extern uint64_t g_crossings[NCOLORS][NCOLORS];
extern uint64_t g_curveComplete[NCOLORS];
extern STATIC struct facial_cycle g_cycles[NCYCLES];

extern FAILURE makeChoice(FACE face);
extern FAILURE finalCorrectnessChecks(void);
extern void setupCentralFaces(int *faceDegrees);
extern void initializeGraph(void);
extern void initialize(void);
extern FAILURE assignPoint(FACE face, COLOR aColor, COLOR bColor, int depth);

#endif  // FACE_H
