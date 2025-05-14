/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef INITIALIZE_H
#define INITIALIZE_H

#include "engine.h"

/* The initialization predicate */
extern struct predicate initializePredicate;

/* Run initialization and reset predicates */
void runInitialize(void);

#endif /* INITIALIZE_H */
