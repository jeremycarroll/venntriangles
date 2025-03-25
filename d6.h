#ifndef D6_H
#define D6_H

#include <stdint.h>

#include "color.h"

typedef int PERMUTATION[NCOLORS];

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE dynamicSymmetryTypeFaces(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE dynamicSymmetryType6(int* faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void* data, int* faceDegrees);
extern void dynamicFaceCanonicalCallback(UseFaceDegrees callback, void* data);
extern char* dynamicFaceDegreeSignature(void);

extern COLOR dynamicColorPermute(COLOR color, PERMUTATION permutation);
extern COLORSET dynamicColorSetPermute(COLORSET colorSet,
                                       PERMUTATION permutation);

#endif  // D6_H
