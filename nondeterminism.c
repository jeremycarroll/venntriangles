/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "nondeterminism.h"

#include "predicates.h"

/* The non-deterministic program is a sequence of predicates */
struct predicate* NonDeterministicProgram[] = {
    /* Single call: Initialization (idempotent; does not undo). */
    &InitializePredicate,
    /* 6 Calls. Nondeterministic: choosse a canonical or equivocal sequence of 5
       face degrees summing to 27. */
    &FaceDegreePredicate,
    /* < 64 calls. Nondeterministic: choose facial cycle for every face. */
    &FacePredicate,
    /* Single call. Save the solution. On backtrack, also write the number of
       variants, then fail. */
    &SaveMainPredicate,
    /* 6 Calls. Nondeterministic: choose the 18 corners of a variation. */
    &CornersPredicate,
    /* Single call. Save the variation. */
    &SaveVariationPredicate,
    /* Fail. */
    &FAILPredicate};
