/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "face.h"
#include "helper_for_tests.h"
#include "main.h"
#include "predicates.h"
#include "statistics.h"
#include "visible_for_testing.h"

#include <unity.h>

#define A 0
#define B 1
#define C 2
#define Abits (1 << A)
#define Bbits (1 << B)
#define Cbits (1 << C)
#define ABCbits (Abits | Bbits | Cbits)
#define ABbits (Abits | Bbits)
#define ACbits (Abits | Cbits)
#define BCbits (Bbits | Cbits)

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  CycleForcedCounter = 0;
  engine(&TestStack, (PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineClear(&TestStack);
}

static void sanityVertex(VERTEX vertex)
{
  TEST_ASSERT_EQUAL(1u << vertex->primary | 1u << vertex->secondary,
                    vertex->colors);
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 4; j++) {
      TEST_ASSERT_NOT_EQUAL(vertex->incomingEdges[i], vertex->incomingEdges[j]);
    }
  }
}

static void verifyFaceSize(int size)
{
  int i;
  for (i = 0; i < NFACES; i++) {
    TEST_ASSERT_EQUAL(size, Faces[i].cycleSetSize);
    if (size == 1) {
      TEST_ASSERT_NOT_NULL(Faces[i].cycle);
    } else {
      TEST_ASSERT_NULL(Faces[i].cycle);
    }
  }
}

static void testOuterFace()
{
  FACE face = Faces;
  TEST_ASSERT_EQUAL(0, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[A], Faces + Abits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[B], Faces + Bbits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[C], Faces + Cbits);
}

static void testAFace()
{
  FACE face = Faces + Abits;
  TEST_ASSERT_EQUAL(Abits, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[A], Faces);
  TEST_ASSERT_EQUAL(face->adjacentFaces[B], Faces + ABbits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[C], Faces + ACbits);
}

static void testAbFace()
{
  FACE face = Faces + ABbits;
  TEST_ASSERT_EQUAL(ABbits, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[A], Faces + Bbits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[B], Faces + Abits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[C], Faces + ABCbits);
}

static void testAbcFace()
{
  FACE face = Faces + ABCbits;
  TEST_ASSERT_EQUAL(ABCbits, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[A], Faces + BCbits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[B], Faces + ACbits);
  TEST_ASSERT_EQUAL(face->adjacentFaces[C], Faces + ABbits);
}

static void checkBVertexOfAEdge(FACE face, EDGE edge, bool primary,
                                bool primaryAtBVertex, COLORSET nextBcolors)
{
  TEST_ASSERT_EQUAL(primary, IS_CLOCKWISE_EDGE(edge));
  TEST_ASSERT_EQUAL(face->colors, edge->colors);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(A, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[A].vertex);
  TEST_ASSERT_NULL(edge->possiblyTo[A].next);
  TEST_ASSERT_EQUAL(primaryAtBVertex ? A : B,
                    edge->possiblyTo[B].vertex->primary);
  TEST_ASSERT_EQUAL(primaryAtBVertex ? B : A,
                    edge->possiblyTo[B].vertex->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[B].vertex, edge->possiblyTo[C].vertex);
  TEST_ASSERT_EQUAL(A, edge->possiblyTo[B].next->color);
  TEST_ASSERT_EQUAL(nextBcolors, edge->possiblyTo[B].next->colors);
  sanityVertex(edge->possiblyTo[B].vertex);
}

static void testOuterAEdge()
{
  FACE face = Faces;
  EDGE edge = &face->edges[A];
  checkBVertexOfAEdge(face, edge, false, true, Bbits);
}

static void testAFaceAEdge()
{
  FACE face = Faces + Abits;
  EDGE edge = &face->edges[A];
  checkBVertexOfAEdge(face, edge, true, false, ABbits);
}

static void testAbFaceAEdge()
{
  FACE face = Faces + ABbits;
  EDGE edge = &face->edges[A];
  checkBVertexOfAEdge(face, edge, true, true, Abits);
}

static void testAbcFaceAEdge()
{
  FACE face = Faces + ABCbits;
  EDGE edge = &face->edges[A];
  checkBVertexOfAEdge(face, edge, true, true, ACbits);
}

static void testChoosingAndBacktracking()
{
  int i;
  FACE face;
  TRAIL startTrail = Trail;
  // For each face in turn, we can guess that is cycle is acb (the first one of
  // the two cycles),
  for (i = 0; i < NFACES; i++) {
    CycleForcedCounter = 0;
    TEST_ASSERT_EQUAL(startTrail, Trail);
    verifyFaceSize(2);
    face = Faces + i;
    face->cycle = Cycles;
    TEST_ASSERT_NULL(dynamicFaceBacktrackableChoice(face));
    // Having selected one face, all of the other faces are determined.
    verifyFaceSize(1);
    TEST_ASSERT_EQUAL(7, CycleForcedCounter);
    trailRewindTo(startTrail);
    face->cycle = NULL;
  }
  // There are 8 faces, so 8 guesses.
  TEST_ASSERT_EQUAL(8, CycleGuessCounterIPC);
}

static int SolutionCount = 0;

static struct predicateResult foundSolution()
{
  SolutionCount++;
  return PredicateFail;
}

static void testSearch()
{
  engineResume(&TestStack, (PREDICATE[]){&VennPredicate,
                                         &(struct predicate){
                                             "Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(2, SolutionCount);
  TEST_ASSERT_EQUAL(14, CycleForcedCounter);
}

int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    TracingFlag = true;
  }
  UNITY_BEGIN();
  RUN_TEST(testOuterFace);
  RUN_TEST(testAFace);
  RUN_TEST(testAbFace);
  RUN_TEST(testAbcFace);
  RUN_TEST(testOuterAEdge);
  RUN_TEST(testAFaceAEdge);
  RUN_TEST(testAbFaceAEdge);
  RUN_TEST(testAbcFaceAEdge);
  RUN_TEST(testChoosingAndBacktracking);
  RUN_TEST(testSearch);
  return UNITY_END();
}
