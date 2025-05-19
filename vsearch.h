/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SEARCH_H
#define SEARCH_H

#include "engine.h"
#include "face.h"

#include <stdbool.h>
#include <stdio.h>

/* The face predicate */
extern struct predicate facePredicate;

// visible for testing
extern uint64 CycleGuessCounter;
extern FACE searchChooseNextFace(bool smallestFirst);
extern int searchCountVariations(char *variationMultiplication);

/* Runtime statistics */
extern uint64_t
    GlobalSolutionsFound;  // Counter for solutions found during execution

/* Externally linked functions */
extern void searchFull(void (*foundSolution)(void));
extern void searchSolutionWrite(const char *prefix);

#endif  // SEARCH_H
