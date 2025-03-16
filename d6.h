
#include <stdint.h>

#include "venn.h"
#ifndef NCOLORS
#define NCOLORS 6
#endif

typedef int PERMUTATION[NCOLORS];

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE d6SymmetryTypeFaces(FACE allFaces);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE d6SymmetryType6(int* faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(int* faceDegrees);
extern void canoncialCallback(UseFaceDegrees callback);
