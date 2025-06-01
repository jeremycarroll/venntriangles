/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CYCLESET_H
#define CYCLESET_H

#include "core.h"
#include "cycle.h"  // For CYCLE and CYCLE_ID

typedef uint64 *CYCLESET;
typedef uint64 CYCLESET_DECLARE[CYCLESET_LENGTH];

// Global cycleset structures
extern CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
extern CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];

// Cycleset operations
extern void cycleSetAdd(CYCLE_ID cycleId, CYCLESET cycleSet);
extern void cycleSetRemove(CYCLE_ID cycleId, CYCLESET cycleSet);
extern bool cycleSetMember(CYCLE_ID cycleId, CYCLESET cycleSet);
extern CYCLE cycleSetFirst(CYCLESET cycleSet);
extern CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle);
extern uint32_t cycleSetSize(CYCLESET cycleSet);
extern void cycleSetRemoveCycleWithTrail(CYCLESET cycleSet, uint32_t cycleId);
extern void initializeCycleSetUniversal(CYCLESET cycleSet);

// Main initialization function
extern void initializeCycleSets(void);

#endif  // CYCLESET_H
