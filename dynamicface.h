/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef DYNAMICFACE_H
#define DYNAMICFACE_H

#include "core.h"
#include "cycle.h"
#include "failure.h"
#include "trail.h"
#include "vertex.h"

/* Forward declarations */
typedef struct face* FACE;

/* Dynamic face operations that can be backtracked */
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, FACE_DEGREE length);
extern void dynamicFaceSetupCentral(FACE_DEGREE* faceDegrees);
extern FAILURE dynamicFaceIncludeVertex(FACE face, COLOR aColor, COLOR bColor,
                                        int depth);
extern bool dynamicColorRemoveFromSearch(COLOR color);
extern FAILURE dynamicFaceFinalCorrectnessChecks(void);
extern FAILURE dynamicFaceRestrictAndPropagateCycles(FACE face,
                                                     CYCLESET onlyCycleSet,
                                                     int depth);
extern FAILURE dynamicFacePropagateChoice(FACE face, EDGE edge, int depth);
extern void dynamicRecomputeCountOfChoices(FACE face);
extern void dynamicRestrictCycles(FACE face, CYCLESET cycleSet);
extern FAILURE dynamicCheckLengthOfCycleOfFaces(FACE face);
extern FAILURE dynamicProcessIncomingEdge(EDGE edge, COLOR colors[2],
                                          int incomingEdgeSlot, int depth);
extern FAILURE dynamicHandleExistingEdge(FACE face, COLOR aColor, COLOR bColor,
                                         int depth);
extern void dynamicCountEdge(EDGE edge);

/* Global counters */
extern uint64 CycleForcedCounter;
extern uint64 CycleSetReducedCounter;

#endif  // DYNAMICFACE_H
