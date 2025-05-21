/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "face.h"
#include "main.h"
#include "predicates.h"
#include "statistics.h"
#include "utils.h"
#include "vsearch.h"

#include <unity.h>

void setUp(void)
{
  initializeStatisticLogging(NULL, 4, 1);
  engine((PREDICATE[]){&InitializePredicate, &SUSPENDPredicate});
}

void tearDown(void)
{
  engineResume((PREDICATE[]){&FAILPredicate});
}

/* Helper functions */
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

/* Test functions */
static void testOuterFace()
{
  FACE face = Faces;
  TEST_ASSERT_EQUAL(0, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], Faces + 1);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], Faces + 2);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], Faces + 4);
}

static void testAFace()
{
  FACE face = Faces + 1;
  TEST_ASSERT_EQUAL(1, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], Faces);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], Faces + 3);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], Faces + 5);
}

static void testAbFace()
{
  FACE face = Faces + 3;
  TEST_ASSERT_EQUAL(3, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], Faces + 2);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], Faces + 1);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], Faces + 7);
}

static void testAbcFace()
{
  FACE face = Faces + 7;
  TEST_ASSERT_EQUAL(7, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], Faces + 6);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], Faces + 5);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], Faces + 3);
}

/*
TODO add picture to justify the following test.
*/
static void testOuterAEdge()
{
  FACE face = Faces;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_FALSE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face->colors, edge->colors);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].vertex);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].vertex->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].vertex->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].vertex, edge->possiblyTo[2].vertex);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(2, edge->possiblyTo[1].next->colors);
  sanityVertex(edge->possiblyTo[1].vertex);
}

/*
TODO add picture to justify the following test.
*/
static void testAFaceAEdge()
{
  FACE face = Faces + 1;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face->colors, edge->colors);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].vertex);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].vertex->secondary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].vertex->primary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].vertex, edge->possiblyTo[2].vertex);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(3, edge->possiblyTo[1].next->colors);
  sanityVertex(edge->possiblyTo[1].vertex);
}

/*
TODO add picture to justify the following test.
*/
static void testAbFaceAEdge()
{
  FACE face = Faces + 3;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face->colors, edge->colors);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].vertex);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].vertex->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].vertex->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].vertex, edge->possiblyTo[2].vertex);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].next->colors);
  sanityVertex(edge->possiblyTo[1].vertex);
}

/*
TODO add picture to justify the following test.
*/
static void testAbcFaceAEdge()
{
  FACE face = Faces + 7;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face->colors, edge->colors);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].vertex);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].vertex->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].vertex->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].vertex, edge->possiblyTo[2].vertex);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(5, edge->possiblyTo[1].next->colors);
  sanityVertex(edge->possiblyTo[1].vertex);
}

static void testChoosingAndBacktracking()
{
  int i;
  FACE face;
  TRAIL startTrail = Trail;
  for (i = 0; i < NFACES; i++) {
    TEST_ASSERT_EQUAL(startTrail, Trail);
    verifyFaceSize(2);
    face = Faces + i;
    face->cycle = Cycles;
    TEST_ASSERT_NULL(dynamicFaceBacktrackableChoice(face));
    verifyFaceSize(1);
    trailBacktrackTo(face->backtrack);
    face->cycle = NULL;
  }
  TEST_ASSERT_EQUAL(8, CycleGuessCounter);
}

/* Global variables */
static int SolutionCount = 0;

static struct predicateResult foundSolution()
{
  SolutionCount++;
  return PredicateFail;
}

/* Test functions */
static void testSearch()
{
  engineResume((PREDICATE[]){
      &VennPredicate, &(struct predicate){"Found", foundSolution, NULL}});
  TEST_ASSERT_EQUAL(2, SolutionCount);
}

/* Main test runner */
int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    Tracing = true;
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
