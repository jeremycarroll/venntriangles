/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SEARCH_H
#define SEARCH_H

#include "predicates.h"

// visible for testing
extern FACE searchChooseNextFace(void);
extern int searchCountVariations(char *variationMultiplication);

/* Runtime statistics */
extern uint64 CycleGuessCounter;
extern uint64_t
    GlobalSolutionsFound;  // Counter for solutions found during execution

#endif  // SEARCH_H
