/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "vsearch.h"

#include "face.h"
#include "graphml.h"
#include "main.h"
#include "s6.h"
#include "save.h"
#include "statistics.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

uint64 CycleGuessCounter = 0;
uint64_t GlobalSolutionsFound = 0;

/* Declaration of file scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64 cycleId);
static FAILURE checkFacePoints(FACE face, CYCLE cycle, int depth);
static FAILURE checkEdgeCurvesAndCorners(FACE face, CYCLE cycle, int depth);
static FAILURE propagateFaceChoices(FACE face, CYCLE cycle, int depth);
static FAILURE propagateRestrictionsToNonAdjacentFaces(FACE face, CYCLE cycle,
                                                       int depth);
static FAILURE propagateRestrictionsToNonVertexAdjacentFaces(FACE face,
                                                             CYCLE cycle,
                                                             int depth);

/* Externally linked functions */
FAILURE dynamicFaceChoice(FACE face, int depth)
{
  CYCLE cycle = face->cycle;
  uint64 cycleId = cycle - Cycles;
  FAILURE failure;
  /* equality in the following assertion is achieved in the Venn 3 case, where a
  single choice in any face determines all the faces. */

  assert(depth <= NFACES);

  CHECK_FAILURE(checkFacePoints(face, cycle, depth));
  CHECK_FAILURE(checkEdgeCurvesAndCorners(face, cycle, depth));
  CHECK_FAILURE(propagateFaceChoices(face, cycle, depth));
  CHECK_FAILURE(propagateRestrictionsToNonAdjacentFaces(face, cycle, depth));

  if (face->colors == 0 || face->colors == (NFACES - 1)) {
    TRAIL_SET_POINTER(&face->next, face);
    TRAIL_SET_POINTER(&face->previous, face);
  } else {
    TRAIL_SET_POINTER(&face->next, face->nextByCycleId[cycleId]);
    TRAIL_SET_POINTER(&face->previous, face->previousByCycleId[cycleId]);
  }

  if (face->colors != 0 && face->colors != (NFACES - 1)) {
    assert(face->next != Faces);
    assert(face->previous != Faces);
  }

  CHECK_FAILURE(
      propagateRestrictionsToNonVertexAdjacentFaces(face, cycle, depth));

  return NULL;
}

FAILURE dynamicFaceBacktrackableChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64 cycleId;
  CycleGuessCounter++;
  ColorCompleted = 0;
  face->backtrack = Trail;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleSetMember(cycleId, face->possibleCycles));
  setFaceCycleSetToSingleton(face, cycleId);

  failure = dynamicFaceChoice(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (ColorCompleted) {
    for (completedColor = 0; completedColor < NCOLORS; completedColor++) {
      if (COLORSET_HAS_MEMBER(completedColor, ColorCompleted)) {
        if (!dynamicColorRemoveFromSearch(completedColor)) {
          return failureDisconnectedCurve(0);
        }
      }
    }
  }
  return NULL;
}

FACE searchChooseNextFace(void)
{
  FACE face = NULL;
  int64_t min = NCYCLES + 1;
  int i;
  for (i = 0; i < NFACES; i++) {
    if ((int64_t)Faces[i].cycleSetSize < min && Faces[i].cycle == NULL) {
      min = (int64_t)Faces[i].cycleSetSize;
      face = Faces + i;
    }
  }
  return face;
}

int searchCountVariations(char* variationMultiplication)
{
  EDGE corners[3][2];
  int numberOfVariations = 1;
  int pLength;
  if (variationMultiplication != NULL) {
    variationMultiplication[0] = '\0';
  }
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindAndAlignCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      if (corners[i][0] == NULL) {
        EDGE edge = edgeOnCentralFace(a);
        pLength = edgePathLength(edge, edgeFollowBackwards(edge), NULL);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1], NULL);
      }
      numberOfVariations *= pLength;
      if (variationMultiplication != NULL && pLength > 1) {
        variationMultiplication +=
            sprintf(variationMultiplication, "*%d", pLength);
      }
    }
  }
  return numberOfVariations;
}

/* File scoped static functions */
static void setFaceCycleSetToSingleton(FACE face, uint64 cycleId)
{
  CYCLESET_DECLARE cycleSet;
  uint64 i;
  memset(cycleSet, 0, sizeof(cycleSet));
  cycleSetAdd(cycleId, cycleSet);
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    trailMaybeSetInt(&face->possibleCycles[i], cycleSet[i]);
  }
  trailMaybeSetInt(&face->cycleSetSize, 1);
}

static FAILURE checkFacePoints(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length - 1; i++) {
    CHECK_FAILURE(dynamicFaceIncludeVertex(face, cycle->curves[i],
                                           cycle->curves[i + 1], depth));
  }
  CHECK_FAILURE(dynamicFaceIncludeVertex(face, cycle->curves[i],
                                         cycle->curves[0], depth));

  return NULL;
}

static FAILURE checkEdgeCurvesAndCorners(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(edgeCurveChecks(&face->edges[cycle->curves[i]], depth));
    CHECK_FAILURE(
        dynamicEdgeCornerCheck(&face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE propagateFaceChoices(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(
        facePropogateChoice(face, &face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE propagateRestrictionsToNonAdjacentFaces(FACE face, CYCLE cycle,
                                                       int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < NCOLORS; i++) {
    if (COLORSET_HAS_MEMBER(i, cycle->colors)) {
      continue;
    }
    CHECK_FAILURE(faceRestrictAndPropogateCycles(
        face->adjacentFaces[i], CycleSetOmittingOneColor[i], depth));
  }

  return NULL;
}

static FAILURE propagateRestrictionsToNonVertexAdjacentFaces(FACE face,
                                                             CYCLE cycle,
                                                             int depth)
{
  uint32_t i, j;
  FAILURE failure;

  for (i = 0; i < NCOLORS; i++) {
    for (j = i + 1; j < NCOLORS; j++) {
      if (COLORSET_HAS_MEMBER(i, cycle->colors) &&
          COLORSET_HAS_MEMBER(j, cycle->colors)) {
        if (cycleContainsAthenB(face->cycle, i, j)) {
          continue;
        }
      }
      CHECK_FAILURE(faceRestrictAndPropogateCycles(
          face->adjacentFaces[i]->adjacentFaces[j],
          CycleSetOmittingColorPair[i][j], depth));
    }
  }

  return NULL;
}
