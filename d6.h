
#include <stdint.h>

#include "venn.h"
#ifndef NCOLORS
#define NCOLORS 6
#endif

typedef uint8_t PERMUTATION[NCOLORS];

typedef enum {
  CANONICAL,
  EQUIVOCAL,
  NON_CANONICAL,
} SYMMETRY_TYPE;

extern void initializeSequenceOrder(void);
extern SYMMETRY_TYPE d6SymmetryTypeFaces(FACE allFaces);
extern SYMMETRY_TYPE d6SymmetryType6(u_int32_t a1, u_int32_t a2, u_int32_t a3,
                                     u_int32_t a4, u_int32_t a5, u_int32_t a6);
extern SYMMETRY_TYPE d6SymmetryType12(u_int32_t a1, u_int32_t a2, u_int32_t a3,
                                      u_int32_t a4, u_int32_t a5, u_int32_t a6,
                                      u_int32_t a7, u_int32_t a8, u_int32_t a9,
                                      u_int32_t a10, u_int32_t a11,
                                      u_int32_t a12);
