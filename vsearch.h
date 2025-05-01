/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SEARCH_H
#define SEARCH_H

#include "face.h"

#include <stdbool.h>
#include <stdio.h>

extern void searchHere(bool smallestFirst, void (*foundSolution)(void));
extern void searchFull(void (*foundSolution)(void));

// visible for testing
extern uint64_t CycleGuessCounter;
extern FACE searchChooseNextFace(bool smallestFirst);
extern void searchSolutionWrite(const char *prefix);
extern int searchCountVariations(char *variationMultiplication);

#endif  // SEARCH_H
