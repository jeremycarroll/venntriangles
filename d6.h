#ifndef D6_H
#define D6_H

#include <stdint.h>

#include "color.h"
#include "graph.h"

typedef int PERMUTATION[NCOLORS];

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE d6SymmetryTypeFaces(void);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS being
   used as the face degrees for the NCOLORS faces around the central face. */
extern SYMMETRY_TYPE d6SymmetryType6(int* faceDegrees);

/* The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
   used as the face degrees for the NCOLORS faces around the central face. */
typedef void (*UseFaceDegrees)(void* data, int* faceDegrees);
extern void canoncialCallback(UseFaceDegrees callback, void* data);
extern char* d6FaceDegreeSignature(void);

#endif  // D6_H
