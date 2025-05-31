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
extern FAILURE faceRestrictAndPropagateCycles(FACE face, CYCLESET onlyCycleSet,
                                              int depth);
extern FAILURE facePropagateChoice(FACE face, EDGE edge, int depth);

#endif  // FACE_H
