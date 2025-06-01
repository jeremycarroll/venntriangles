/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef COLOR_H
#define COLOR_H

#include "core.h"

// Basic color types
typedef uint32_t COLOR;
typedef uint32_t COLORSET;

// Colorset operations
#define COLORSET_HAS_MEMBER(color, colorSet) (((colorSet) >> (color)) & 1u)

// Color operations
extern int colorToChar(COLOR c);
extern char *colorSetToString(COLORSET colors);
extern char *colorSetToBareString(COLORSET colors);

#endif  // COLOR_H
