#ifndef SEARCH_H
#define SEARCH_H

#include "face.h"

#include <stdbool.h>
#include <stdio.h>

extern FACE dynamicFaceChoose(bool smallestFirst);
extern void dynamicSearch(bool smallestFirst, void (*foundSolution)(void));
extern void dynamicSearchFull(void (*foundSolution)(void));
extern void dynamicSolutionEnumerate(FILE *(*fp)(char *signature, int soln,
                                                 int subsoln));

#endif  // SEARCH_H
