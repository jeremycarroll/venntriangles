#ifndef _FACE_H
#define _FACE_H

#include "types.h"

/* Face-related structures */
struct face {
  /* cycle must be null if cycleSetSize is not 1. */
  DYNAMIC struct facial_cycle *cycle;
  DYNAMIC TRAIL backtrack;
  /* We point to previous and next with the same number of colors. */
  DYNAMIC FACE previous;
  DYNAMIC FACE next;
  STATIC COLORSET colors;          /* holds up to NFACES */
  DYNAMIC uint_trail cycleSetSize; /* holds up to NCYCLES */
  DYNAMIC CYCLESET_DECLARE possibleCycles;
  STATIC struct face *adjacentFaces[NCOLORS];
  STATIC struct edge edges[NCOLORS];
  STATIC uint64_t previousByCycleId[NCYCLES];
  STATIC uint64_t nextByCycleId[NCYCLES];
};

/* Face-related function declarations */
void setupCentralFaces(int *faceDegrees);
void printFace(FACE face);
char *face2str(char *dbuffer, FACE face);

#endif /* _FACE_H */