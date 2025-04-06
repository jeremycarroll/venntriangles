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

typedef struct cycleIdSequence {
  CYCLE_ID faceCycleId[NFACES];
} *CYCLE_ID_SEQUENCE;

typedef int (*PERMUTATION)[NCOLORS];

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
    int count, FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
extern SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    int count, FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
extern bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees,
                    FACE_DEGREE_SEQUENCE other);
extern char *d6SequenceToString(FACE_DEGREE_SEQUENCE faceDegrees);

extern PERMUTATION d6Compose(PERMUTATION a, PERMUTATION b);

extern PERMUTATION d6Inverse(PERMUTATION permutation);

extern PERMUTATION d6Identity(void);

extern bool d6PermutationEqual(PERMUTATION a, PERMUTATION b);

extern PERMUTATION d6Closure(int *sizeReturn, int numberOfGenerators,
                             PERMUTATION generator1, ...);

extern PERMUTATION d6Permutation(int a1, ...);

extern PERMUTATION d6InvertingPermutation(void);

extern PERMUTATION d6CycleAsPermutation(CYCLE cycle);
extern char *d6Permutation2str(PERMUTATION permutation);
extern char *d6SolutionSequenceString(void);
extern char *d6SolutionClassSequenceString(void);
extern CYCLE_ID_SEQUENCE d6MaxSignature(void);
extern CYCLE_ID_SEQUENCE d6SignatureRecentered(CYCLE_ID_SEQUENCE sequence,
                                               COLORSET center);
extern CYCLE_ID_SEQUENCE d6SignaturePermuted(CYCLE_ID_SEQUENCE sequence,
                                             PERMUTATION permutation);
extern CYCLE_ID_SEQUENCE d6SignatureFromFaces(void);
extern int d6SignatureCompare(CYCLE_ID_SEQUENCE a, CYCLE_ID_SEQUENCE b);
extern CYCLE_ID_SEQUENCE d6SignatureReflected(CYCLE_ID_SEQUENCE sequence);
extern char *d6SignatureToString(CYCLE_ID_SEQUENCE signature);
extern char *d6SignatureToLongString(CYCLE_ID_SEQUENCE signature);
extern CYCLE_ID cycleIdPermute(CYCLE_ID originalCycleId,
                               PERMUTATION permutation);
extern char *permutationToString(PERMUTATION permutation);
extern int group[2 * NCOLORS][NCOLORS];
extern PERMUTATION d6Automorphism(CYCLE_ID cycleId);

#endif  // D6_H
