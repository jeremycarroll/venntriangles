/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SOLUTIONWRITE_H
#define SOLUTIONWRITE_H

#include "engine.h"

/* The solution write predicate */
extern struct predicate solutionWritePredicate;

/* Write a solution to disk */
void writeSolution(void);

#endif /* SOLUTIONWRITE_H */
