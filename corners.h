/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CORNERS_H
#define CORNERS_H
#include "edge.h"
#include "engine.h"

/* The corners predicate */
extern struct predicate cornersPredicate;

/* The save variation predicate */
extern struct predicate saveVariationPredicate;

/* The fail predicate */
extern struct predicate failPredicate;

/* Choose corners for all colors */
void chooseCorners(void);

#endif /* CORNERS_H */
