#ifndef FACE_H
#define FACE_H

#include "edge.h"
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

extern FAILURE dynamicFaceMakeChoice(FACE face);
extern FAILURE dynamicFaceFinalCorrectnessChecks(void);
extern void dynamicFaceSetupCentral(int *faceDegrees);
extern FAILURE dynamicFaceIncludePoint(FACE face, COLOR aColor, COLOR bColor,
                                       int depth);
extern bool dynamicColorRemoveFromSearch(COLOR color);

extern COLORSET DynamicColorCompleted;

extern uint64_t DynamicCycleGuessCounter;
extern char *faceToStr(char *dbuffer, FACE face);

extern FACE dynamicFaceFromColors(char *colors);
extern FACE dynamicFaceAddSpecific(char *colors, char *cycle);
extern void dynamicFacePrintSelected(void);
extern void dynamicFacePrint(FACE face);
extern void dynamicSolutionPrint(FILE *fp);
// Should have Dynamic in name, and be so.
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, uint32_t length);
extern void initializeFacesAndEdges(void);

extern void resetFaces(void);

#endif  // FACE_H
