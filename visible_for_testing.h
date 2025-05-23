
/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef VISIBLE_FOR_TESTING_H
#define VISIBLE_FOR_TESTING_H

#include "trail.h"
#include "s6.h"
#include "color.h"

/* Internal declarations that should be visible to test files will go here */
extern FACE searchChooseNextFace(void);
extern uint64 CycleGuessCounterIPC ;
extern CYCLE_ID cycleIdFromColors(char *colors);
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t a, uint32_t b,
                                     uint32_t c);
extern FACE dynamicFaceAddSpecific(char *colors, char *cycle);
extern void facePrintSelected(void);
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, FACE_DEGREE length);
extern FACE faceFromColors(char *colors);
extern PERMUTATION s6Automorphism(CYCLE_ID cycleId);
extern CYCLE_ID s6PermuteCycleId(CYCLE_ID originalCycleId,
                                 PERMUTATION permutation);
extern char *s6SignatureToLongString(SIGNATURE signature);

extern bool trailBacktrackTo(TRAIL backtrackPoint);
extern TRAIL Trail;

#endif  // VISIBLE_FOR_TESTING_H 