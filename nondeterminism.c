/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "nondeterminism.h"

#include "corners.h"
#include "engine.h"
#include "initialize.h"
#include "s6.h"
#include "solutionwrite.h"
#include "vsearch.h"

/* The non-deterministic program is a sequence of predicates */
struct predicate* nonDeterministicProgram[] = {
    /* Initialization: on backtrack perform reset, then fail. */
    &initializePredicate,
    /* Nondeterministic: choosse a canonical or equivocal sequence of 5 face
       degrees summing to 27. */
    &faceDegreePredicate,
    /* Nondeterministic: choose facial cycle for every face. */
    &facePredicate,
    /* Save the solution. On backtrack, also write the number of variants, then
       fail. */
    &solutionWritePredicate,
    /* Nondeterministic: choose the 18 corners of a variation. */
    &cornersPredicate,
    /* Save the variation. */
    &saveVariationPredicate,
    /* Fail. */
    &failPredicate, NULL};
