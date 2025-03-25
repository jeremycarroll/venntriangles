#ifndef FACE_H
#define FACE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

extern uint64_t cycleForcedCounter;
extern uint64_t cycleSetReducedCounter;

extern void initializeGraph(void);
extern STATIC struct face Faces[NFACES];
extern uint64_t FaceSumOfFaceDegree[NCOLORS + 1];
extern STATIC struct facial_cycle Cycles[NCYCLES];

extern FAILURE dynamicFaceMakeChoice(FACE face);
extern FAILURE dynamicFaceFinalCorrectnessChecks(void);
extern void initializeFaceSetupCentral(int *faceDegrees);
extern FAILURE dynamicFaceIncludePoint(FACE face, COLOR aColor, COLOR bColor,
                                       int depth);
extern void dynamicSearchFull(void (*foundSolution)(void));
extern void dynamicSearch(bool smallestFirst, void (*foundSolution)(void));

extern void initializeFaceSetupCentral(int *faceDegrees);
extern bool dynamicColorRemoveFromSearch(COLOR color);

extern COLORSET DynamicColorCompleted;

extern uint64_t DynamicCycleGuessCounter;
extern char *faceToStr(char *dbuffer, FACE face);

extern FACE dynamicFaceChoose(bool smallestFirst);
extern FACE dynamicFaceFromColors(char *colors);
extern FACE dynamicFaceAddSpecific(char *colors, char *cycle);
extern void dynamicFacePrintSelected(void);
// Should have Dynamic in name, and be so.
extern bool setCycleLength(uint32_t faceColors, uint32_t length);
extern void initializeFacesAndEdges(void);

extern void resetFaces(void);

extern uint32_t cycleIdFromColors(char *colors);
extern FACE dynamicFaceFromColors(char *colors);

#endif  // FACE_H
