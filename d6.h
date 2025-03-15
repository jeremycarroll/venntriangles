
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
extern SYMMETRY_TYPE d6SymmetryType6(int a1, int a2, int a3, int a4, int a5,
                                     int a6);
extern SYMMETRY_TYPE d6SymmetryType12(int a1, int a2, int a3, int a4, int a5,
                                      int a6, int a7, int a8, int a9, int a10,
                                      int a11, int a12);

typedef void (*Callback6)(int a, int b, int c, int d, int e, int f);
typedef void (*Callback12)(int a, int b, int c, int d, int e, int f, int g,
                           int h, int i, int j, int k, int l);
extern void canoncialCallback(Callback6 callback6, Callback12 callback12);
