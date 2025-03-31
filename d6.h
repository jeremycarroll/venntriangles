/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef D6_H
#define D6_H

#include "color.h"

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

typedef struct faceDegreeSequence {
  FACE_DEGREE faceDegrees[NFACES];
} *FACE_DEGREE_SEQUENCE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE symmetryTypeFaces(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE symmetryType6(FACE_DEGREE *faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void *data, FACE_DEGREE *faceDegrees);
extern void canonicalCallback(UseFaceDegrees callback, void *data);
extern char *faceDegreeSignature(void);

extern FACE_DEGREE_SEQUENCE d6FaceDegreesInNaturalOrder(void);
extern FACE_DEGREE_SEQUENCE d6InvertedFaceDegreesInNaturalOrder(void);
extern FACE_DEGREE_SEQUENCE d6ConvertToSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInNaturalOrder);
extern FACE_DEGREE_SEQUENCE d6ConvertToNaturalOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder);
extern FACE_DEGREE_SEQUENCE d6MaxInSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
extern SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
extern bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees,
                    FACE_DEGREE_SEQUENCE other);
extern char *d6ToString(FACE_DEGREE_SEQUENCE faceDegrees);
extern void d6ResetMemory(void);

#endif  // D6_H
