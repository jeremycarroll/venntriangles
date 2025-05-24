/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TRAIL_H
#define TRAIL_H

#include "core.h"

typedef uint64 uint_trail;

extern void trailSetPointer(void **ptr, void *value);
#define TRAIL_SET_POINTER(a, b) trailSetPointer((void **)a, b)

extern void trailSetInt(uint_trail *ptr, uint_trail value);
extern void trailMaybeSetInt(uint_trail *ptr, uint_trail value);
extern void trailFreeze(void);

extern void initializeTrail(void);
typedef struct trail *TRAIL;

#endif  // TRAIL_H
