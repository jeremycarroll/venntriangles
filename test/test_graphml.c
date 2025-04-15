/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "statistics.h"
#include "test_helpers.h"
#include "utils.h"
#include "vsearch.h"

#include <regex.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

static void mockInitializeFolder(const char* folder)
{  // nothing.
}

static char outputBuffer[50000];
static int FopenCount = 0;
static FILE* mockFopen(const char* filename, const char* mode)
{
  FILE* result = fmemopen(outputBuffer, 50000, mode);
  assert(result != NULL);
  FopenCount++;
  return result;
}

/* Test setup and teardown */
void setUp(void)
{
  initialize();
  initializeStatisticLogging("/dev/null", 1000, 1000);  // Reduce logging output
  graphmlFileOps.initializeFolder = mockInitializeFolder;
  graphmlFileOps.fopen = mockFopen;
}

void tearDown(void)
{
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetStatistics();
  resetPoints();
}

/* Global variables */
static int SolutionCount = 0;

static void (*FoundSolution)(void);
/* Callback functions */
static void foundBasicSolution()
{
  SolutionCount++;
  SIGNATURE signature = d6MaxSignature();
  TEST_ASSERT_EQUAL_STRING(
      "PdJeIgFuAcEhAdDeDqAcDuGqElAdGnGhKoDpAbFnAnFeAaFwJdDmCzCyAeFkCyCzErDnDlDk"
      "BqFwAzHcEsAxEpEyJjAwDcCxKjDoAaFwAsDeBvNwDiEtFhEsAyOrJwMx",
      d6SignatureToString(signature));
  // graphmlSaveAllVariations("/tmp/foo", 128);
  if (FoundSolution) FoundSolution();
}

static void saveAllVariations()
{
  FopenCount = 0;
  graphmlSaveAllVariations("/tmp/foo", 256);
  TEST_ASSERT_EQUAL(128, FopenCount);
}

struct nodeInfo {
  char id[64];
  char color1[8];
  char line1[8];
  char color2[8];
  char line2[8];
  unsigned source : 2;
  unsigned target : 2;
  unsigned node : 1;
  unsigned corner : 1;
};

static void copyNodeIdToKey(struct nodeInfo* key, const char* p,
                            regmatch_t* offsets)
{
  int length = offsets[0].rm_eo - offsets[0].rm_so;
  strncpy(key->id, p + offsets[0].rm_so, length);
  key->id[length] = '\0';
}

static struct nodeInfo* findNode(struct nodeInfo* nodes, size_t* nodeCount,
                                 const char* p, regmatch_t* offsets)
{
  struct nodeInfo key;
  memset(&key, 0, sizeof(key));
  copyNodeIdToKey(&key, p, offsets);
  return (struct nodeInfo*)lsearch(&key, nodes, nodeCount,
                                   sizeof(struct nodeInfo),
                                   (int (*)(const void*, const void*))strcmp);
}

static void checkGraphML()
{
  regex_t graphRegex;
  regex_t nodeOrEdgeRegex;
  regmatch_t offsets[10];
  struct nodeInfo nodes[500];
  struct nodeInfo* entry;
  size_t nodeCount = 0;
  char errbuf[256];
  char* p;
  int ret;
  int i;
  memset(nodes, 0, sizeof(nodes));
  FopenCount = 0;
  MaxVariantsPerSolution = 1;
  graphmlSaveAllVariations("/tmp/foo", 256);
  TEST_ASSERT_EQUAL(1, FopenCount);
  ret = regcomp(&graphRegex, "<graph [^>]*>", REG_EXTENDED);
  if (ret != 0) {
    regerror(ret, &graphRegex, errbuf, sizeof(errbuf));
    printf("Error compiling graphRegex: %s\n", errbuf);
  }
  TEST_ASSERT_EQUAL(0, ret);
  ret = regexec(&graphRegex, outputBuffer, sizeof(offsets) / sizeof(offsets[0]),
                offsets, REG_EXTENDED);
  TEST_ASSERT_EQUAL(0, ret);

//        "<node id=\"\\([^\"]*\\)\">|"
#if 0
  "<\\(node\\)|"
                "<edge source=\"\\([^\"]*\\)\" target=\"\\([^\"]*\\)\">|"
                "\\(</graph>\\)"
#endif
  ret = regcomp(&nodeOrEdgeRegex,
                "<node id=\"(([a-z]_[0-2])|[^\"]*)\">|"
                "<edge source=\"([^\"]*)\" target=\"([^\"]*)\">|"
                "(</graph>)",
                REG_EXTENDED);
  if (ret != 0) {
    regerror(ret, &nodeOrEdgeRegex, errbuf, sizeof(errbuf));
    printf("Error compiling nodeOrEdgeRegex: %s\n", errbuf);
  }
  TEST_ASSERT_EQUAL(0, ret);
  p = outputBuffer + offsets[0].rm_eo;
  while (p < outputBuffer + 50000) {
    ret = regexec(&nodeOrEdgeRegex, p, sizeof(offsets) / sizeof(offsets[0]),
                  offsets, 0);
    if (ret != 0) {
      printf("Failed at %d\n>>>> %.30s\n", p - outputBuffer, p);
      TEST_FAIL();
    }
    if (offsets[1].rm_eo != -1) {
      entry = findNode(nodes, &nodeCount, p, offsets + 1);
      // printf("Added node %s\n", entry->id);
      TEST_ASSERT_NOT_NULL(entry);
      TEST_ASSERT_EQUAL_INT(0, entry->node);
      entry->node = 1;
      if (offsets[2].rm_eo != -1) {
        entry->corner = 1;
      }
      // printf("Found node at %d\n>>>> %.30s\n", p - outputBuffer, p);
    } else if (offsets[3].rm_eo != -1) {
      // printf("Found edge at %d\n>>>> %.50s\n", p - outputBuffer,
      //        p + offsets[3].rm_so - 8);
      entry = findNode(nodes, &nodeCount, p, offsets + 3);
      TEST_ASSERT_NOT_NULL(entry);
      TEST_ASSERT_LESS_OR_EQUAL(1, entry->source);
      entry->source++;
      entry = findNode(nodes, &nodeCount, p, offsets + 4);
      TEST_ASSERT_LESS_OR_EQUAL(1, entry->target);
      entry->target++;
    } else if (offsets[5].rm_eo != -1) {
      //  printf("Found end of graph at %d\n>>>> %.30s\n", p - outputBuffer, p);
      break;
    }
    p += offsets[0].rm_eo;
  }

  for (i = 0; i < nodeCount; i++) {
    printf("node %d: %s\n", i, nodes[i].id);
    entry = &nodes[i];
    TEST_ASSERT_EQUAL(1, entry->node);
    if (entry->corner) {
      TEST_ASSERT_EQUAL(1, entry->target);
      TEST_ASSERT_EQUAL(1, entry->source);
    } else {
      TEST_ASSERT_EQUAL(2, entry->target);
      TEST_ASSERT_EQUAL(2, entry->source);
    }
  }

  regfree(&graphRegex);
  regfree(&nodeOrEdgeRegex);
}

static void testVariationCount()
{
  TEST_ASSERT_EQUAL(128, searchCountVariations());
}

static int ContinuationCount = 0;
static int countContinuation(COLOR ignored, EDGE (*corners)[3])
{
  ContinuationCount++;
  return 0;
}

static void testColorContinuations(COLOR color, int expectedCount)
{
  EDGE corners[NCOLORS][3];
  ContinuationCount = 0;
  computePossibleCorners();
#if 0
  for (int i = 0; i < 3; i++) {
    printf("cornerPairs[%d][0]: %x\n", i, cornerPairs[i][0]->reversed);
    printf("cornerPairs[%d][1]: %x\n", i, cornerPairs[i][1]);
  }

#endif

  chooseCornersWithContinuation(0, color, corners, countContinuation);
  TEST_ASSERT_EQUAL(expectedCount, ContinuationCount);
}

static void foundSolutionColor0() { testColorContinuations(0, 8); }
static void foundSolutionColor1() { testColorContinuations(1, 1); }
static void foundSolutionColor2() { testColorContinuations(2, 2); }
static void foundSolutionColor3() { testColorContinuations(3, 1); }
static void foundSolutionColor4() { testColorContinuations(4, 2); }
static void foundSolutionColor5() { testColorContinuations(5, 4); }

static char* TestName;

static void run645534(void)
{
  Unity.NumberOfTests--;
  UNITY_NEW_TEST(TestName);
  SolutionCount = 0;
  dynamicFaceSetupCentral(intArray(6, 4, 5, 5, 3, 4));
  searchHere(true, foundBasicSolution);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

#define RUN_645534(foundSolution) \
  TestName = #foundSolution;      \
  FoundSolution = foundSolution;  \
  RUN_TEST(run645534);

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_645534(NULL);
  RUN_645534(testVariationCount);
  RUN_645534(foundSolutionColor0);
  RUN_645534(foundSolutionColor1);
  RUN_645534(foundSolutionColor2);
  RUN_645534(foundSolutionColor3);
  RUN_645534(foundSolutionColor4);
  RUN_645534(foundSolutionColor5);
  RUN_645534(saveAllVariations);
  RUN_645534(checkGraphML);
  return UNITY_END();
}
