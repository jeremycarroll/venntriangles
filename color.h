/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef COLOR_H
#define COLOR_H

#include "core.h"

/* Basic type definitions */
typedef uint32_t COLOR;    /* Represents a single color (curve) */
typedef uint32_t COLORSET; /* Bit set where each bit represents presence/absence
                              of a color */

/* Bitwise operations */
/* Check if a color is present in a colorset using bit manipulation */
#define COLORSET_HAS_MEMBER(color, colorSet) (((colorSet) >> (color)) & 1u)

/* String conversion functions */
/* Convert a color index to its character representation (a-f) */
extern int colorToChar(COLOR c);
/* Format a colorset as a string with enclosing pipes (e.g., "|abc|") */
extern char *colorSetToString(COLORSET colors);
/* Format a colorset as a string without pipes (e.g., "abc") */
extern char *colorSetToBareString(COLORSET colors);

#endif  // COLOR_H
