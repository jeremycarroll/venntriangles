/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef S6_H
#define S6_H

#include "color.h"
#include "engine.h"

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

/*
The classSignature is the largest signature based on any face, and any
labelling. By construction the first element of the classSignature is NCYCLES-1,
the cycle a,b,c,d,e,f, around face 0. The isomorphism class corresponds to
drawing the diagram on a sphere, rather than the Euclidean plane. To identify a
specific venn diagram we identify the center face, by the offset. Conventionally
we take the cycle a,b,c,d,e,f around that face (NFACES-1), and it may have been
reflected from the class representative identified by the classSignature.
*/
typedef struct signature {
  struct cycleIdSequence classSignature;
  COLORSET offset;
  bool reflected;
} *SIGNATURE;

extern void initializeS6(void);
extern SYMMETRY_TYPE s6FacesSymmetryType(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void *data, FACE_DEGREE *faceDegrees);
extern char *s6FaceDegreeSignature(void);

extern SIGNATURE s6MaxSignature(void);
extern SIGNATURE s6SignatureFromFaces(void);
extern char *d6SignatureToString(SIGNATURE signature);

// For testing.
extern PERMUTATION s6Automorphism(CYCLE_ID cycleId);
extern CYCLE_ID s6PermuteCycleId(CYCLE_ID originalCycleId,
                                 PERMUTATION permutation);
extern char *s6SignatureToLongString(SIGNATURE signature);
extern SYMMETRY_TYPE s6SymmetryType6(FACE_DEGREE *faceDegrees);

#endif  // S6_H
