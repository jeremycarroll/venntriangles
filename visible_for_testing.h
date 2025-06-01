/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef VISIBLE_FOR_TESTING_H
#define VISIBLE_FOR_TESTING_H

#include "trail.h"
#include "s6.h"
#include "color.h"
#include "cycleset.h"

/**
 * This header exposes internal functions and variables for testing purposes.
 * These declarations should not be used in production code and are only
 * made available to enable comprehensive unit testing.
 */

/* Search algorithm internals */
extern FACE searchChooseNextFace(void);         /* Face selection algorithm */
extern int searchCountVariations(void);         /* Count available variations */

/* S6 signature functions */
extern PERMUTATION s6Automorphism(CYCLE_ID cycleId); /* Get automorphism for cycle */
extern CYCLE_ID s6PermuteCycleId(CYCLE_ID originalCycleId, PERMUTATION permutation); /* Apply permutation */
extern char *s6SignatureToLongString(SIGNATURE signature); /* Convert to detailed string */

/* These cycle sets are already declared in cycleset.h but needed for tests */

/* Trail system */
extern TRAIL Trail; /* Global trail for backtracking */
extern bool trailRewindTo(TRAIL backtrackPoint); /* Rewind trail to point */

#endif  /* VISIBLE_FOR_TESTING_H */