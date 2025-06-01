/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CYCLESET_H
#define CYCLESET_H

#include "cycle.h"

/* Type definitions */
/* Pointer to an array of 64-bit words that represent a set of cycles as a
 * bitmap */
typedef uint64 *CYCLESET;
/* Array declaration for a cycleset with predefined length from core.h */
typedef uint64 CYCLESET_DECLARE[CYCLESET_LENGTH];

/* Global cycle set caches - used to optimize lookups */
/* Cycles that contain two colors in sequence (e.g., color1 followed by color2)
 */
extern CYCLESET_DECLARE CycleSetPairs[NCOLORS][NCOLORS];
/* Cycles that contain three colors in sequence */
extern CYCLESET_DECLARE CycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
/* Cycles that don't contain a specific color */
extern CYCLESET_DECLARE CycleSetOmittingOneColor[NCOLORS];
/* Cycles that don't contain a specific color pair in sequence */
extern CYCLESET_DECLARE CycleSetOmittingColorPair[NCOLORS][NCOLORS];

/* Basic cycleset operations */
/* Add a cycle to a cycleset */
extern void cycleSetAdd(CYCLE_ID cycleId, CYCLESET cycleSet);
/* Remove a cycle from a cycleset */
extern void cycleSetRemove(CYCLE_ID cycleId, CYCLESET cycleSet);
/* Check if a cycle is in a cycleset */
extern bool cycleSetMember(CYCLE_ID cycleId, CYCLESET cycleSet);

/* Cycleset iteration - for traversing all cycles in a set */
/* Get first cycle in a cycleset */
extern CYCLE cycleSetFirst(CYCLESET cycleSet);
/* Get next cycle in a cycleset after the specified cycle */
extern CYCLE cycleSetNext(CYCLESET cycleSet, CYCLE cycle);
/* Count the number of cycles in a cycleset */
extern uint32_t cycleSetSize(CYCLESET cycleSet);

/* Trail-based operations - support backtracking */
/* Remove a cycle with backtracking support */
extern void dynamicCycleSetRemoveCycle(CYCLESET cycleSet, uint32_t cycleId);

/* Initialization functions */
/* Set a cycleset to contain all possible cycles */
extern void initializeCycleSetUniversal(CYCLESET cycleSet);
/* Main initialization function for all global cyclesets */
extern void initializeCycleSets(void);

#endif  // CYCLESET_H
