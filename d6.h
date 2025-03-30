/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef D6_H
#define D6_H

#include "color.h"

typedef int PERMUTATION[NCOLORS];

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE symmetryTypeFaces(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE symmetryType6(int* faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void* data, int* faceDegrees);
extern void canonicalCallback(UseFaceDegrees callback, void* data);
extern char* faceDegreeSignature(void);

extern COLOR colorPermute(COLOR color, PERMUTATION permutation);
extern COLORSET colorSetPermute(COLORSET colorSet, PERMUTATION permutation);

#endif  // D6_H
