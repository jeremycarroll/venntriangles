/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "nondeterminism.h"

#include "predicates.h"

/* The non-deterministic program is a sequence of predicates */
struct predicate* NonDeterministicProgram[] = {
    /* Single call: Initialization (idempotent; does not undo). */
    &InitializePredicate,
    /* 6 Calls. Nondeterministic: choosse a canonical or equivocal sequence of 5
       face degrees summing to 27. */ // InnerFaceDegree
    &InnerFacePredicate,
    &LogPredicate,
    /* < 64 calls. Nondeterministic: choose facial cycle for every face. */
    
    &VennPredicate,   // Venn
    /* Single call. Save the solution. On backtrack, also write the number of
       variants, then fail. */
    &SavePredicate, // Save
    /* 6 Calls. Nondeterministic: choose the 18 corners of a variation. */
    &CornersPredicate, // Corners
    /* Single call. Save the variation. */
    &GraphMLPredicate, // GraphML
    /* Fail. */
    &FAILPredicate};
