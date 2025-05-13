/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "corners.h"

#include "edge.h"
#include "engine.h"
#include "face.h"
#include "graphml.h"
#include "memory.h"
#include "trail.h"
#include "triangles.h"

#include <stdio.h>
#include <stdlib.h>

/* Global variables for corner handling */
static EDGE SelectedCorners[NCOLORS][3];
extern EDGE PossibileCorners[NCOLORS][3][NFACES];
extern int VariationNumber;
extern int MaxVariantsPerSolution;
static char* filename;
static int numberOfVariations;

static int edgeArrayLength(EDGE* edges)
{
  int count = 0;
  while (edges[count] != NULL) {
    count++;
  }
  return count;
}

static struct predicateResult tryCorners(int round)
{
  int cornerIndex = round % 3;
  int colorIndex = round / 3;

  if (VariationNumber > MaxVariantsPerSolution) {
    return PredicateFail;
  }

  if (cornerIndex == 0 && colorIndex > 0) {
    if (!triangleLinesNotCrossed(colorIndex - 1,
                                 SelectedCorners + colorIndex - 1)) {
      return PredicateFail;
    }
  }
  if (colorIndex >= NCOLORS) {
    return PredicateSuccessNextPredicate;
  }
  return predicateChoices(
      edgeArrayLength(PossibileCorners[colorIndex][cornerIndex]), NULL);
}

static struct predicateResult retryCorners(int round, int choice)
{
  int cornerIndex = round % 3;
  int colorIndex = round / 3;
  SelectedCorners[colorIndex][cornerIndex] =
      PossibileCorners[colorIndex][cornerIndex][choice];
  return PredicateSuccessSamePredicate;
}

static struct predicateResult trySaveVariation(int round)
{
  (void)round;  // Unused parameter
  saveVariation(SelectedCorners);
  return PredicateSuccessNextPredicate;
}
/* The predicates array for corner handling */
static struct predicate cornersPredicate = {tryCorners, retryCorners};
static struct predicate saveVariationPredicate = {trySaveVariation, NULL};
static struct predicate* predicates[] = {
    &cornersPredicate, &saveVariationPredicate, &failPredicate};

void chooseCorners(void)
{
  //   filename = "variation.graphml";  // Or get this from somewhere else?
  //   numberOfVariations = searchCountVariations(NULL);
  engine(predicates, NULL);
}
