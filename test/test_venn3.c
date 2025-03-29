#include "face.h"
#include "search.h"
#include "statistics.h"
#include "utils.h"

#include <unity.h>

void setUp(void)
{
  initialize();
  initializeStatisticLogging(NULL, 4, 1);
}

void tearDown(void)
{
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

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

static void sanityPoint(POINT point)
{
  TEST_ASSERT_EQUAL(1u << point->primary | 1u << point->secondary,
                    point->colors);
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 4; j++) {
      TEST_ASSERT_NOT_EQUAL(point->incomingEdges[i], point->incomingEdges[j]);
    }
  }
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
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(2, edge->possiblyTo[1].next->colors);
  sanityPoint(edge->possiblyTo[1].point);
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
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(3, edge->possiblyTo[1].next->colors);
  sanityPoint(edge->possiblyTo[1].point);
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
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].next->colors);
  sanityPoint(edge->possiblyTo[1].point);
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
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].next);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].next->color);
  TEST_ASSERT_EQUAL(5, edge->possiblyTo[1].next->colors);
  sanityPoint(edge->possiblyTo[1].point);
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
    TEST_ASSERT_NULL(dynamicFaceMakeChoice(face));
    verifyFaceSize(1);
    trailBacktrackTo(face->backtrack);
    face->cycle = NULL;
  }
  TEST_ASSERT_EQUAL(8, CycleGuessCounter);
}

static int SolutionCount = 0;
static void foundSolution() { SolutionCount++; }

static void testSearch()
{
  searchHere(true, foundSolution);
  TEST_ASSERT_EQUAL(2, SolutionCount);
}

int main(void)
{
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
