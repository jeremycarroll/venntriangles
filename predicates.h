/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef PREDICATES_H
#define PREDICATES_H

#include "engine.h"

/**
 * Predicates used by the non-deterministic search engine.
 * Each predicate represents a phase in the search algorithm.
 */

/* Setup phase - initialize all data structures */
extern struct predicate InitializePredicate;

/* Search phases - find valid Venn diagrams */
extern struct predicate InnerFacePredicate;  /* Select face degrees */
extern struct predicate VennPredicate;       /* Assign cycles to faces */

/* Output phases - save and display results */
extern struct predicate LogPredicate;        /* Log progress */
extern struct predicate SavePredicate;       /* Save solutions */
extern struct predicate CornersPredicate;    /* Place diagram corners */
extern struct predicate GraphMLPredicate;    /* Export to GraphML format */

#endif  /* PREDICATES_H */