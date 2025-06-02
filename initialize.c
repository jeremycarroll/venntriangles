/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */
#include "common.h"
#include "face.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialize(void)
{
  /* Architecture-specific assertions */
  assert((sizeof(uint64) == sizeof(void *)));
  assert(sizeof(uint64) == 8);

  initializeCycleSets();
  initializeFacesAndEdges();
  initializePoints();
  initializeTrail();
  initializeMemory();
  initializeS6();
  trailFreeze();
}

static bool forwardInitialize(void)
{
  initialize();
  statisticIncludeInteger(&CycleGuessCounterIPC, "?", "guesses", false);
  statisticIncludeInteger(&GlobalVariantCountIPC, "V", "variants", false);
  statisticIncludeInteger(&GlobalSolutionsFoundIPC, "S", "solutions", false);
  return true;
}

FORWARD_BACKWARD_PREDICATE(Initialize, NULL, forwardInitialize, NULL)
