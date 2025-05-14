/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "initialize.h"

#include "engine.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "s6.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"
#include "vsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct predicateResult tryInitialize(int round)
{
  // Return choice of two items - first for initialization, second for reset
  return predicateChoices(2, NULL);
}

static struct predicateResult retryInitialize(int round, int choice)
{
  if (choice == 0) {
    // First choice: Do initialization
    initializeS6();
    initialize();
    statisticIncludeInteger(&CycleGuessCounter, "?", "guesses", false);
    statisticIncludeInteger(&GlobalVariantCount, "V", "variants", false);
    statisticIncludeInteger(&GlobalSolutionsFound, "S", "solutions", false);
    return PredicateSuccessNextPredicate;
  } else {
    // Second choice: Do reset and fail
    resetGlobals();
    resetInitialize();
    resetPoints();
    return PredicateFail;
  }
}

/* The predicates array for initialization */
struct predicate initializePredicate = {tryInitialize, retryInitialize};
static struct predicate* predicates[] = {&initializePredicate, NULL};

void runInitialize(void)
{
  engine(predicates, NULL);
}
