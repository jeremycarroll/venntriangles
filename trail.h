/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TRAIL_H
#define TRAIL_H

#include "core.h"

/**
 * Trail system for supporting backtracking operations.
 * Provides a way to set values while remembering previous state.
 */

/* Type definitions */
typedef uint64 uint_trail;   /* Integer type for trail operations */
typedef struct trail *TRAIL; /* Opaque pointer to trail structure */

/* Trail initialization */
extern void initializeTrail(void); /* Initialize the trail system */
extern void trailFreeze(void);     /* Freeze trail to prevent backtracking */

/* Value setting operations */
extern void trailSetInt(
    uint_trail *ptr, uint_trail value); /* Set an integer with backtracking */

/* Return false if value is already correct, else set it and return true. */
extern bool trailMaybeSetInt(uint_trail *ptr, uint_trail value);
extern void trailSetPointer(void **ptr,
                            void *value); /* Set a pointer with backtracking */

/* Helper macro for type-safe pointer setting */
#define TRAIL_SET_POINTER(a, b) trailSetPointer((void **)a, b)

#endif  // TRAIL_H
