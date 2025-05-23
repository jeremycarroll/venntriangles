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

static bool forwardInitialize(void)
{
  initializeS6();
  initialize();
  statisticIncludeInteger(&CycleGuessCounterIPC, "?", "guesses", false);
  statisticIncludeInteger(&GlobalVariantCountIPC, "V", "variants", false);
  statisticIncludeInteger(&GlobalSolutionsFoundIPC, "S", "solutions", false);
  return true;
}

FORWARD_BACKWARD_PREDICATE(Initialize, NULL, forwardInitialize, NULL)
