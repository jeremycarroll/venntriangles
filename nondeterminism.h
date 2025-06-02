/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef NONDETERMINISM_H
#define NONDETERMINISM_H

#include "engine.h"

/**
 * The non-deterministic program defines the sequence of predicates that
 * the engine executes to search for Venn diagrams.
 *
 * Each predicate in the sequence represents a phase of computation such as
 * initialization, face selection, cycle assignment, corner placement, etc.
 *
 * The engine moves both forwards and backwards through this sequence to
 * explore the solution space with backtracking.
 */
extern struct predicate* NonDeterministicProgram[];

#endif /* NONDETERMINISM_H */
