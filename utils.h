/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef UTILS_H
#define UTILS_H

#include "core.h"

extern void initialize(void);
extern void initializeFolder(const char *folder);

// exposed for testing
extern void resetGlobals(void);
extern void resetInitialize(void);

extern void disaster(const char * programName, const char* message);

#endif  // UTILS_H
