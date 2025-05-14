/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SEARCH_H
#define SEARCH_H

#include "engine.h"
#include "face.h"

#include <stdbool.h>
#include <stdio.h>

/* The face predicate */
extern struct predicate facePredicate;

extern void searchHere(bool smallestFirst, void (*foundSolution)(void));
extern void searchFull(void (*foundSolution)(void));

// visible for testing
extern uint64 CycleGuessCounter;
extern FACE searchChooseNextFace(bool smallestFirst);
extern void searchSolutionWrite(const char *prefix);
extern int searchCountVariations(char *variationMultiplication);

#endif  // SEARCH_H
