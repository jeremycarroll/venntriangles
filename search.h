/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SEARCH_H
#define SEARCH_H

#include "face.h"

#include <stdbool.h>
#include <stdio.h>

extern FACE chooseNextFaceForSearch(bool smallestFirst);
extern void searchHere(bool smallestFirst, void (*foundSolution)(void));
extern void searchFull(void (*foundSolution)(void));
extern void dynamicSolutionEnumerate(FILE *(*fp)(char *signature, int soln,
                                                 int subsoln));

extern void solutionPrint(FILE *fp);
extern void solutionWrite(const char *prefix);
extern int searchCountVariations(void);
#endif  // SEARCH_H
