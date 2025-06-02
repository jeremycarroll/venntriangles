/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "nondeterminism.h"

#include "predicates.h"

/**
 * The non-deterministic program is a sequence of predicates that is executed
 * by the engine. The engine moves both forwards and backwards over the
 * predicates, to explore a nondeterministic space, in a top-down fashion.
 * Each stage in this array represents a phase of computation:
 *
 * 1. Initialize - Single call setup (idempotent; does not undo)
 * 2. InnerFace - 6 calls to choose a canonical sequence of 5 face degrees
 * summing to 27
 * 3. Log - When backtracking records progress
 * 4. Venn - Up to 64 calls to choose facial cycle for every face
 * 5. Save - Single call to save the solution and when backtracking records
 * count of variants
 * 6. Corners - 6 calls to choose the 18 corners of a variationplease
 * 7. GraphML - Single call to save the variation
 * 8. FAIL - Forces backtracking
 */
struct predicate* NonDeterministicProgram[] = {
    &InitializePredicate, &InnerFacePredicate, &LogPredicate,
    &VennPredicate,       &SavePredicate,      &CornersPredicate,
    &GraphMLPredicate,    &FAILPredicate};
