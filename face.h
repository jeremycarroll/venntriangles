/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FACE_H
#define FACE_H

#include "trail.h"
#include "vertex.h"

/* These next two functions are actually defined in search.c */
extern FAILURE dynamicFaceBacktrackableChoice(FACE face);
extern FAILURE dynamicFaceChoice(FACE face, int depth);

extern FAILURE faceFinalCorrectnessChecks(void);
extern void dynamicFaceSetupCentral(FACE_DEGREE *faceDegrees);
extern FAILURE dynamicFaceIncludeVertex(FACE face, COLOR aColor, COLOR bColor,
                                        int depth);
extern bool dynamicColorRemoveFromSearch(COLOR color);

extern char *faceToString(FACE face);

extern void initializeFacesAndEdges(void);
extern FAILURE faceRestrictAndPropogateCycles(FACE face, CYCLESET onlyCycleSet,
                                              int depth);
extern FAILURE facePropogateChoice(FACE face, EDGE edge, int depth);

// exposed for testing
extern void facePrintSelected(void);
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, FACE_DEGREE length);
extern FACE faceFromColors(char *colors);
extern FACE dynamicFaceAddSpecific(char *colors, char *cycle);

#endif  // FACE_H
