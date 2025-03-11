#include "../venn.h"
#include "../visible_for_testing.h"
#include "unity.h"

#define STATS 0

void setUp(void)
{
  initialize();
  initializeStatsLogging(NULL, 4, 1);
}

void tearDown(void)
{
  clearGlobals();
  clearInitialize();
  resetTrail();
  resetStatistics();
}

static void test_outer_face()
{
  FACE face = g_faces;
  TEST_ASSERT_EQUAL(0, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], g_faces + 1);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], g_faces + 2);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], g_faces + 4);
}

static void test_a_face()
{
  FACE face = g_faces + 1;
  TEST_ASSERT_EQUAL(1, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], g_faces);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], g_faces + 3);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], g_faces + 5);
}

static void test_ab_face()
{
  FACE face = g_faces + 3;
  TEST_ASSERT_EQUAL(3, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], g_faces + 2);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], g_faces + 1);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], g_faces + 7);
}

static void test_abc_face()
{
  FACE face = g_faces + 7;
  TEST_ASSERT_EQUAL(7, face->colors);
  TEST_ASSERT_EQUAL(face->adjacentFaces[0], g_faces + 6);
  TEST_ASSERT_EQUAL(face->adjacentFaces[1], g_faces + 5);
  TEST_ASSERT_EQUAL(face->adjacentFaces[2], g_faces + 3);
}

static void sanity_point(UPOINT point)
{
  TEST_ASSERT_EQUAL(1u << point->primary | 1u << point->secondary,
                    point->colors);
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 4; j++) {
      TEST_ASSERT_NOT_EQUAL(point->incomingEdges[i], point->incomingEdges[j]);
      TEST_ASSERT_NOT_EQUAL(point->faces[i], point->faces[j]);
    }
  }
  TEST_ASSERT_EQUAL(0, point->faces[0]->colors & point->colors);
  TEST_ASSERT_EQUAL(1u << point->secondary,
                    point->faces[2]->colors & point->colors);
  TEST_ASSERT_EQUAL(1u << point->primary,
                    point->faces[1]->colors & point->colors);
  TEST_ASSERT_EQUAL(point->colors, point->faces[3]->colors & point->colors);
}

/*
TODO add picture to justify the following test.
*/
static void test_outer_a_edge()
{
  FACE face = g_faces;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_FALSE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face, edge->face);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].out[0]);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].out[0]->color);
  TEST_ASSERT_EQUAL(2, edge->possiblyTo[1].out[0]->face->colors);
  sanity_point(edge->possiblyTo[1].point);
}
/*
TODO add picture to justify the following test.
*/
static void test_a_face_a_edge()
{
  FACE face = g_faces + 1;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face, edge->face);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].out[0]);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].out[0]->color);
  TEST_ASSERT_EQUAL(3, edge->possiblyTo[1].out[0]->face->colors);
  sanity_point(edge->possiblyTo[1].point);
}
/*
TODO add picture to justify the following test.
*/
static void test_ab_face_a_edge()
{
  FACE face = g_faces + 3;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face, edge->face);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].out[0]);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].out[0]->color);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].out[0]->face->colors);
  sanity_point(edge->possiblyTo[1].point);
}
/*
TODO add picture to justify the following test.
*/
static void test_abc_face_a_edge()
{
  FACE face = g_faces + 7;
  EDGE edge = &face->edges[0];
  TEST_ASSERT_TRUE(IS_PRIMARY_EDGE(edge));
  TEST_ASSERT_EQUAL(face, edge->face);
  TEST_ASSERT_NULL(edge->to);
  TEST_ASSERT_EQUAL(0, edge->color);
  TEST_ASSERT_NULL(edge->possiblyTo[0].point);
  TEST_ASSERT_NULL(edge->possiblyTo[0].out[0]);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].point->primary);
  TEST_ASSERT_EQUAL(1, edge->possiblyTo[1].point->secondary);
  TEST_ASSERT_NOT_EQUAL(edge->possiblyTo[1].point, edge->possiblyTo[2].point);
  TEST_ASSERT_EQUAL(0, edge->possiblyTo[1].out[0]->color);
  TEST_ASSERT_EQUAL(5, edge->possiblyTo[1].out[0]->face->colors);
  sanity_point(edge->possiblyTo[1].point);
}

static void verify_face_size(int size)
{
  int i;
  for (i = 0; i < NFACES; i++) {
    TEST_ASSERT_EQUAL(size, g_faces[i].cycleSetSize);
    if (size == 1) {
      TEST_ASSERT_NOT_NULL(g_faces[i].cycle);
    } else {
      TEST_ASSERT_NULL(g_faces[i].cycle);
    }
  }
}

static void test_choosing_and_backtracking()
{
  int i;
  FACE face;
  for (i = 0; i < NFACES; i++) {
    TEST_ASSERT_EQUAL(trailarray, trail);
    verify_face_size(2);
    face = g_faces + i;
    face->cycle = g_cycles;
    TEST_ASSERT_NULL(makeChoice(face));
#if STATS
    printStatisticsOneLine();
#endif
    verify_face_size(1);
    backtrackTo(face->backtrack);
    face->cycle = NULL;
  }
#if STATS
  printStatisticsFull();
#endif
  TEST_ASSERT_EQUAL(8, cycleGuessCounter);
}

static int solution_count = 0;
static void found_solution() { solution_count++; }

static void test_search()
{
  search(true, found_solution);
  TEST_ASSERT_EQUAL(2, solution_count);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_outer_face);
  RUN_TEST(test_a_face);
  RUN_TEST(test_ab_face);
  RUN_TEST(test_abc_face);
  RUN_TEST(test_outer_a_edge);
  RUN_TEST(test_a_face_a_edge);
  RUN_TEST(test_ab_face_a_edge);
  RUN_TEST(test_abc_face_a_edge);
  RUN_TEST(test_choosing_and_backtracking);
  RUN_TEST(test_search);
  return UNITY_END();
}
