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
static void possibleCorners(EDGE* possibilities, COLOR color, EDGE from,
                            EDGE to);

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
  EDGE cornerPairs[3][2];
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
  edgeFindAndAlignCorners(colorIndex, cornerPairs);
  possibleCorners(PossibileCorners[colorIndex][cornerIndex], colorIndex,
                  cornerPairs[cornerIndex][0], cornerPairs[cornerIndex][1]);
  return predicateChoices(
      edgeArrayLength(PossibileCorners[colorIndex][cornerIndex]), NULL);
}

static struct predicateResult retryCorners(int round, int choice)
{
  int cornerIndex = round % 3;
  int colorIndex = round / 3;
  trailSetPointer(&SelectedCorners[colorIndex][cornerIndex],
                  PossibileCorners[colorIndex][cornerIndex][choice]);
  return PredicateSuccessSamePredicate;
}

static struct predicateResult trySaveVariation(int round)
{
  (void)round;  // Unused parameter
  saveVariation(SelectedCorners);
  return PredicateSuccessNextPredicate;
}
/* The predicates array for corner handling */
struct predicate cornersPredicate = {"Corners", tryCorners, retryCorners};
struct predicate saveVariationPredicate = {"SaveVariant", trySaveVariation,
                                           NULL};
static PREDICATE predicates[] = {&cornersPredicate, &saveVariationPredicate,
                                 &FAILPredicate};

void chooseCorners(void)
{
  //   filename = "variation.graphml";  // Or get this from somewhere else?
  //   numberOfVariations = searchCountVariations(NULL);
  assert(0);
  engine(predicates);
}

/* Path and corner functions */
static void getPath(EDGE* path, EDGE from, EDGE to)
{
  int length = edgePathLength(from, to, path);
#if DEBUG
  printf("getPath: %c %x -> %x %d\n", 'A' + from->color, from, to, length);
#endif
  assert(length > 0);
  assert(length == 1 || path[0] != path[length - 1]);
  trailSetPointer(path + length, NULL);
}

static void possibleCorners(EDGE* possibilities, COLOR color, EDGE from,
                            EDGE to)
{
  if (from == NULL) {
    EDGE edge = edgeOnCentralFace(color);
    getPath(possibilities, edge->reversed, edgeFollowBackwards(edge->reversed));
  } else {
    getPath(possibilities, from->reversed, to);
  }
}
