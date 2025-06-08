/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "face.h"
#include "failure.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

uint64 CycleGuessCounterIPC = 0;
uint64 GlobalSolutionsFoundIPC = 0;

static FACE facesInOrderOfChoice[NFACES];

static void dynamicSetFaceCycleSetToSingleton(FACE face, uint64 cycleId)
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

static FAILURE dynamicCheckFacePoints(FACE face, CYCLE cycle, int depth)
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

static FAILURE dynamicCheckEdgeCurvesAndCorners(FACE face, CYCLE cycle,
                                                int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(
        dynamicEdgeCurveChecks(&face->edges[cycle->curves[i]], depth));
    CHECK_FAILURE(vertexCornerCheck(&face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE dynamicPropagateFaceChoices(FACE face, CYCLE cycle, int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < cycle->length; i++) {
    CHECK_FAILURE(dynamicFacePropagateChoice(
        face, &face->edges[cycle->curves[i]], depth));
  }

  return NULL;
}

static FAILURE dynamicPropagateRestrictionsToNonAdjacentFaces(FACE face,
                                                              CYCLE cycle,
                                                              int depth)
{
  uint32_t i;
  FAILURE failure;

  for (i = 0; i < NCOLORS; i++) {
    if (COLORSET_HAS_MEMBER(i, cycle->colors)) {
      continue;
    }
    CHECK_FAILURE(dynamicFaceRestrictAndPropagateCycles(
        face->adjacentFaces[i], CycleSetOmittingOneColor[i], depth));
  }

  return NULL;
}

static FAILURE dynamicPropagateRestrictionsToNonVertexAdjacentFaces(FACE face,
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
      CHECK_FAILURE(dynamicFaceRestrictAndPropagateCycles(
          face->adjacentFaces[i]->adjacentFaces[j],
          CycleSetOmittingColorPair[i][j], depth));
    }
  }
  return NULL;
}

static CYCLE chooseCycle(FACE face, CYCLE cycle)
{
  return cycleSetNext(face->possibleCycles, cycle);
}

static struct predicateResult dynamicTryFace(int round)
{
  if (round == 0) {
    PerFaceDegreeSolutionNumberIPC = 0;
#if NCOLORS > 4
    dynamicFaceSetupCentral(CentralFaceDegreesFlag);
#endif
  }
  if ((int64_t)GlobalSolutionsFoundIPC >= GlobalMaxSolutionsFlag) {
    return PredicateFail;
  }
  facesInOrderOfChoice[round] = searchChooseNextFace();
  if (facesInOrderOfChoice[round] == NULL) {
    if (dynamicFaceFinalCorrectnessChecks() == NULL) {
      GlobalSolutionsFoundIPC++;
      PerFaceDegreeSolutionNumberIPC++;
      return PredicateSuccessNextPredicate;
    } else {
      return PredicateFail;
    }
  }
  // Add to trail so value is cleared when backtracking.
  TRAIL_SET_POINTER(&facesInOrderOfChoice[round]->cycle, NULL);
  return predicateChoices(facesInOrderOfChoice[round]->cycleSetSize);
}

static struct predicateResult dynamicRetryFace(int round, int choice)
{
  (void)choice;
  FACE face = facesInOrderOfChoice[round];
  // Not on trail, otherwise it would get unset before the next retry.
  face->cycle = chooseCycle(face, face->cycle);
  assert(face->cycle != NULL);
  if (dynamicFaceBacktrackableChoice(face) == NULL) {
    return PredicateSuccessSamePredicate;
  }
  return PredicateFail;
}

FAILURE dynamicFaceChoice(FACE face, int depth)
{
  CYCLE cycle = face->cycle;
  uint64 cycleId = cycle - Cycles;
  FAILURE failure;

  assert(depth <= NFACES);

  CHECK_FAILURE(dynamicCheckFacePoints(face, cycle, depth));
  CHECK_FAILURE(dynamicCheckEdgeCurvesAndCorners(face, cycle, depth));
  CHECK_FAILURE(dynamicPropagateFaceChoices(face, cycle, depth));
  CHECK_FAILURE(
      dynamicPropagateRestrictionsToNonAdjacentFaces(face, cycle, depth));

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
      dynamicPropagateRestrictionsToNonVertexAdjacentFaces(face, cycle, depth));

  return NULL;
}

FAILURE dynamicFaceBacktrackableChoice(FACE face)
{
  FAILURE failure;
  COLOR completedColor;
  uint64 cycleId;
  CycleGuessCounterIPC++;
  ColorCompletedState = 0;
  assert(face->cycle != NULL);
  cycleId = face->cycle - Cycles;
  assert(cycleId < NCYCLES);
  assert(cycleSetMember(cycleId, face->possibleCycles));
  dynamicSetFaceCycleSetToSingleton(face, cycleId);

  failure = dynamicFaceChoice(face, 0);
  if (failure != NULL) {
    return failure;
  }
  if (ColorCompletedState) {
    for (completedColor = 0; completedColor < NCOLORS; completedColor++) {
      if (COLORSET_HAS_MEMBER(completedColor, ColorCompletedState)) {
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

struct predicate VennPredicate = {"Venn", dynamicTryFace, dynamicRetryFace};
