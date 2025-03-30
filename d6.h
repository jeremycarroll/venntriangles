/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef D6_H
#define D6_H

#include "color.h"

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE symmetryTypeFaces(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE symmetryType6(int *faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void *data, int *faceDegrees);
extern void canonicalCallback(UseFaceDegrees callback, void *data);
extern char *faceDegreeSignature(void);

extern int *d6FaceDegreesInNaturalOrder(void);
extern int *d6InvertedFaceDegreesInNaturalOrder(void);
extern int *d6ConvertToSequenceOrder(int *faceDegreesInNaturalOrder);
extern int *d6ConvertToNaturalOrder(int *faceDegreesInSequenceOrder);
extern int *d6MaxInSequenceOrder(int *faceDegreesInSequenceOrder, ...);
extern SYMMETRY_TYPE d6IsMaxInSequenceOrder(int *faceDegreesInSequenceOrder,
                                            ...);
extern bool d6Equal(int *faceDegrees, int *other);
extern char *d6ToString(int *faceDegrees);

#endif  // D6_H
