/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef UTILS_H
#define UTILS_H

#include "core.h"

extern  void resetGlobals(void);
extern  void resetInitialize(void);
extern void initialize(void);

extern  char * getBuffer(void);
extern  char * usingBuffer(char * buffer);

#endif  // UTILS_H
