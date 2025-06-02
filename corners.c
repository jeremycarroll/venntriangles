/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "face.h"
#include "main.h"
#include "predicates.h"
#include "triangles.h"

#include <stdio.h>
#include <stdlib.h>

DYNAMIC EDGE SelectedCornersIPC[NCOLORS][3];

static DYNAMIC EDGE PossibleCorners[NCOLORS][3][NFACES];

/**
 * Count the number of edges in a null-terminated array
 */
static int edgeArrayLength(EDGE* edges)
{
  int count = 0;
  while (edges[count] != NULL) {
    count++;
  }
  return count;
}

static void dynamicPossibleCorners(EDGE* possibilitiesReturn, COLOR color,
                                   EDGE from, EDGE to)
{
  if (from == NULL) {
    EDGE edge = vertexGetCentralEdge(color);
    dynamicEdgePathAndLength(edge->reversed,
                             edgeFollowBackwards(edge->reversed),
                             possibilitiesReturn);
  } else {
    dynamicEdgePathAndLength(from->reversed, to, possibilitiesReturn);
  }
}

/**
 * Predicate entry function for selecting corners.
 *
 * For rounds 3, 6, 9, 12, 15 and 18 - we verify the previous colors corner
 * assignment, On round 18 then we simply succeed - we are now done. For rounds
 * 0 -> 17 we set up choosing between the possible corners.
 *
 * @param round Incrementing number encoding color and corner index, from 0 to
 * 18 inclusive.
 */
static struct predicateResult dynamicTryCorners(int round)
{
  EDGE cornerPairs[3][2];
  int cornerIndex = round % 3;
  int colorIndex = round / 3;

  if (VariationNumberIPC > MaxVariantsPerSolutionFlag) {
    return PredicateFail;
  }

  if (cornerIndex == 0 && colorIndex > 0) {
    if (!dynamicTriangleLinesNotCrossed(colorIndex - 1,
                                        SelectedCornersIPC + colorIndex - 1)) {
      return PredicateFail;
    }
  }
  if (colorIndex >= NCOLORS) {
    return PredicateSuccessNextPredicate;
  }
  vertexAlignCorners(colorIndex, cornerPairs);
  dynamicPossibleCorners(PossibleCorners[colorIndex][cornerIndex], colorIndex,
                         cornerPairs[cornerIndex][0],
                         cornerPairs[cornerIndex][1]);
  return predicateChoices(
      edgeArrayLength(PossibleCorners[colorIndex][cornerIndex]));
}

/**
 * Make a choice of corner as setup in dynamicTryCorners.
 */
static struct predicateResult dynamicRetryCorners(int round, int choice)
{
  int cornerIndex = round % 3;
  int colorIndex = round / 3;
  TRAIL_SET_POINTER(&SelectedCornersIPC[colorIndex][cornerIndex],
                    PossibleCorners[colorIndex][cornerIndex][choice]);
  return PredicateSuccessSamePredicate;
}

struct predicate CornersPredicate = {"Corners", dynamicTryCorners,
                                     dynamicRetryCorners};
