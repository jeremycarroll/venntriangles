/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TRAIL_H
#define TRAIL_H

#include "core.h"

typedef struct trail *TRAIL;
typedef uint64 uint_trail;

/* We actually need about 8000 */
#define TRAIL_SIZE 16384

extern TRAIL Trail;
extern void trailSetPointer(void **ptr, void *value);
#define TRAIL_SET_POINTER(a, b) trailSetPointer((void **)a, b)

extern void trailSetInt(uint_trail *ptr, uint_trail value);
extern bool trailBacktrackTo(TRAIL backtrackPoint);
extern void trailMaybeSetInt(uint_trail *ptr, uint_trail value);
extern void trailFreeze(void);

extern void initializeTrail(void);

#endif  // TRAIL_H
