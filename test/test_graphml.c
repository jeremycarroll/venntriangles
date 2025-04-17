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
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void mockInitializeFolder(const char* folder)
{  // nothing.
}

static char outputBuffer[50000];
static char* outputBufferPtr = outputBuffer;
static int FopenCount = 0;
static FILE* mockFopen(const char* filename, const char* mode)
{
  FILE* result = fmemopen(outputBuffer, 50000, mode);
  assert(result != NULL);
  FopenCount++;
  return result;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

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
  struct colorInfo {
    char color[8];
    char line[8];
  } info[2];
  unsigned source : 2;
  unsigned target : 2;
  unsigned node : 1;
  unsigned corner : 1;
};

static regex_t graphRegex, nodeOrEdgeRegex, nodeDataRegex, edgeDataRegex;
static regmatch_t offsets[10];
static char* offsetBase;
static void copyFromBufferToString(char* key, regmatch_t* offsets)
{
  int length = offsets[0].rm_eo - offsets[0].rm_so;
  assert(length < 16);
  strncpy(key, offsetBase + offsets[0].rm_so, length);
  key[length] = '\0';
}

static void copyNodeIdToKey(struct nodeInfo* key, regmatch_t* offsets)
{
  copyFromBufferToString(key->id, offsets);
}

static struct nodeInfo* findNode(struct nodeInfo* nodes, size_t* nodeCount,
                                 regmatch_t* offsets)
{
  struct nodeInfo key;
  memset(&key, 0, sizeof(key));
  copyNodeIdToKey(&key, offsets);
  return (struct nodeInfo*)lsearch(&key, nodes, nodeCount,
                                   sizeof(struct nodeInfo),
                                   (int (*)(const void*, const void*))strcmp);
}

static bool colorLineEqual(struct colorInfo* info, const char* edgeColor,
                           const char* line)
{
  return strcmp(info->color, edgeColor) == 0 && strcmp(info->line, line) == 0;
}

static bool copyLineByColor(struct colorInfo* info, const char* edgeColor,
                            const char* line)
{
  if (strcmp(info->color, edgeColor) != 0) {
    return false;
  }
  strcpy(info->line, line);
  return true;
}

static void checkLineAndColorInNode(struct nodeInfo* node,
                                    const char* edgeColor, regmatch_t* offsets)
{
  char line[8];
  copyFromBufferToString(line, offsets);
  if (colorLineEqual(&node->info[0], edgeColor, line)) {
    return;
  }
  if (colorLineEqual(&node->info[1], edgeColor, line)) {
    return;
  }
  if (copyLineByColor(&node->info[0], edgeColor, line)) {
    return;
  }
  TEST_ASSERT(copyLineByColor(&node->info[1], edgeColor, line));
}

static void compileRegex(regex_t* regex, const char* pattern)
{
  char errbuf[256];
  int ret = regcomp(regex, pattern, REG_EXTENDED);
  if (ret != 0) {
    regerror(ret, regex, errbuf, sizeof(errbuf));
    printf("Error compiling regex: %s\n", errbuf);
  }
  TEST_ASSERT_EQUAL(0, ret);
}

static void compileRegexes()
{
  compileRegex(&graphRegex, "<graph [^>]*>");
  compileRegex(&nodeOrEdgeRegex,
               "<node id=\"(([a-z]_[0-2])|[^\"]*)\">|"
               "<edge source=\"([^\"]*)\" target=\"([^\"]*)\">|"
               "(</graph>)");
#if 0
      <data key="colors">acdf</data>
      <data key="primary">a</data>
      <data key="secondary">d</data>
#endif
  compileRegex(&nodeDataRegex,
               "<data key=\"colors\">([^<]*)</data>[ \n]*<data "
               "key=\"primary\">([^<]*)</data>[ \n]*<data "
               "key=\"secondary\">([^<]*)</data>");
#if 0
      <data key="color">a</data>
      <data key="line">a1</data>

#endif
  compileRegex(&edgeDataRegex,
               "<data key=\"color\">([^<]*)</data>[ \n]*<data "
               "key=\"line\">([^<]*)</data>");
}

static void freeRegexes()
{
  regfree(&graphRegex);
  regfree(&nodeOrEdgeRegex);
  regfree(&nodeDataRegex);
  regfree(&edgeDataRegex);
}

static void matchRegex(regex_t* regex)
{
  int ret = regexec(regex, outputBufferPtr,
                    sizeof(offsets) / sizeof(offsets[0]), offsets, 0);
  offsetBase = outputBufferPtr;
  if (ret != 0) {
    printf("Failed at %d\n>>>> %.30s\n", outputBufferPtr - outputBuffer,
           outputBufferPtr);
    TEST_FAIL();
  }
  outputBufferPtr += offsets[0].rm_eo;
}

static void checkGraphML()
{
  struct nodeInfo nodes[500];
  struct nodeInfo *entry, *source, *target;
  size_t nodeCount = 0;
  size_t completeNodeCount;
  char nodeColors[8];
  char edgeColor[8];
  unsigned int i;
  memset(nodes, 0, sizeof(nodes));
  FopenCount = 0;
  MaxVariantsPerSolution = 1;
  compileRegexes();
  graphmlSaveAllVariations("/tmp/foo", 256);
  TEST_ASSERT_EQUAL(1, FopenCount);
  outputBufferPtr = outputBuffer;
  matchRegex(&graphRegex);

  while (outputBufferPtr < outputBuffer + 50000) {
    matchRegex(&nodeOrEdgeRegex);

    if (offsets[5].rm_eo != -1) {
      break;
    }
    if (offsets[1].rm_eo != -1) {
      entry = findNode(nodes, &nodeCount, offsets + 1);
      // printf("Added node %s\n", entry->id);
      TEST_ASSERT_NOT_NULL(entry);
      TEST_ASSERT_EQUAL_INT_MESSAGE(0, entry->node, entry->id);
      entry->node = 1;
      if (offsets[2].rm_eo != -1) {
        entry->corner = 1;
      }
      matchRegex(&nodeDataRegex);
      copyFromBufferToString(entry->info[0].color, offsets + 2);
      copyFromBufferToString(entry->info[1].color, offsets + 3);
      copyFromBufferToString(nodeColors, offsets + 1);
      for (i = 0;; i++) {
        if (nodeColors[i] == 0) {
          break;
        }
        TEST_ASSERT(nodeColors[i] >= 'a' || nodeColors[i] <= 'f');
        if (i == 0) continue;
        TEST_ASSERT(nodeColors[i] > nodeColors[i - 1]);
      }
      TEST_ASSERT(strchr(nodeColors, entry->info[0].color[0]) != NULL);
      TEST_ASSERT(strchr(nodeColors, entry->info[1].color[0]) != NULL);
    } else if (offsets[3].rm_eo != -1) {
      entry = findNode(nodes, &nodeCount, offsets + 3);
      TEST_ASSERT_NOT_NULL(entry);
      TEST_ASSERT_LESS_OR_EQUAL(1, entry->source);
      entry->source++;
      entry = findNode(nodes, &nodeCount, offsets + 4);
      TEST_ASSERT_LESS_OR_EQUAL(1, entry->target);
      entry->target++;
    }
  }

  completeNodeCount = nodeCount;

  outputBufferPtr = outputBuffer;
  while (outputBufferPtr < outputBuffer + 50000) {
    matchRegex(&nodeOrEdgeRegex);
    if (offsets[5].rm_eo != -1) {
      break;
    }
    if (offsets[3].rm_eo != -1) {
      source = findNode(nodes, &nodeCount, offsets + 3);
      target = findNode(nodes, &nodeCount, offsets + 4);
      TEST_ASSERT_EQUAL(completeNodeCount, nodeCount);
      matchRegex(&edgeDataRegex);
      copyFromBufferToString(edgeColor, offsets + 1);
      TEST_ASSERT_EQUAL(0, edgeColor[1]);
      checkLineAndColorInNode(source, edgeColor, offsets + 2);
      checkLineAndColorInNode(target, edgeColor, offsets + 2);
    }
  }

  for (i = 0; i < nodeCount; i++) {
    // printf("node %d: %s\n", i, nodes[i].id);
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

  freeRegexes();
}

static void testVariationCount()
{
  TEST_ASSERT_EQUAL(128, searchCountVariations(NULL));
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
static int ContinuationCount = 0;
static int countContinuation(COLOR ignored, EDGE (*corners)[3])
{
  ContinuationCount++;
  return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

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
