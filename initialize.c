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

static bool forwardInitialize(void)
{
  initializeS6();
  initialize();
  statisticIncludeInteger(&CycleGuessCounter, "?", "guesses", false);
  statisticIncludeInteger(&GlobalVariantCount, "V", "variants", false);
  statisticIncludeInteger(&GlobalSolutionsFound, "S", "solutions", false);
  return true;
}
static void backwardInitialize(void)
{
  resetGlobals();
  resetInitialize();
  resetPoints();
}

FORWARD_BACKWARD_PREDICATE(Initialize, NULL, forwardInitialize,
                           backwardInitialize)

static struct predicate* predicates[] = {&InitializePredicate, NULL};

void runInitialize(void)
{
  assert(0);
  engine(predicates);
}
