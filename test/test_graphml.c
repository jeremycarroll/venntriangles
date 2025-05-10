/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#define _GNU_SOURCE
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "s6.h"
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

static regmatch_t offsets[10];
static char outputBuffer[50000];
static char* outputBufferPtr = outputBuffer;
static struct nodeInfo nodes[500];
static size_t nodeCount = 0;
static char* offsetBase;
static char* ExpectedSignature;
static char* ClassSignature;

static void mockInitializeFolder(const char* folder)
{
  (void)folder;  // Explicitly mark as unused
}

static int FopenCount = 0;

static regex_t graphRegex, nodeOrEdgeRegex, nodeDataRegex, edgeDataRegex;

static FILE* mockFopen(const char* filename, const char* mode)
{
  (void)filename;  // Explicitly mark as unused
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
  SIGNATURE signature = s6SignatureFromFaces();
  SIGNATURE classSignature = s6MaxSignature();
  if (strcmp(ExpectedSignature, d6SignatureToString(signature)) != 0) {
    return;
  }
  TEST_ASSERT_EQUAL_STRING(ClassSignature, d6SignatureToString(classSignature));
  // graphmlSaveAllVariations("/tmp/foo", 128);
  if (FoundSolution) FoundSolution();
}

static void saveAllVariations()
{
  FopenCount = 0;
  graphmlSaveAllVariations("/tmp/foo", 256);
  // The setup needs to pick out exactly one variation.
  TEST_ASSERT_EQUAL(1, FopenCount);
}

static void copyFromBufferToString(char* key, const regmatch_t* match_offsets)
{
  size_t len = match_offsets->rm_eo - match_offsets->rm_so;
  strncpy(key, offsetBase + match_offsets->rm_so, len);
  key[len] = 0;
}

static void copyNodeIdToKey(struct nodeInfo* key,
                            const regmatch_t* match_offsets)
{
  copyFromBufferToString(key->id, match_offsets);
}

static struct nodeInfo* findNode(const regmatch_t* match_offsets)
{
  struct nodeInfo key;
  memset(&key, 0, sizeof(key));
  copyNodeIdToKey(&key, match_offsets);
  TEST_ASSERT_GREATER_THAN_MESSAGE(0, strlen(key.id), "Empty node id");
  return (struct nodeInfo*)lsearch(&key, nodes, &nodeCount,
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
  if (strcmp(info->color, edgeColor) != 0 || info->line[0] != 0) {
    return false;
  }
  strcpy(info->line, line);
  return true;
}

static void checkLineAndColorInNode(struct nodeInfo* node,
                                    const char* edgeColor,
                                    const regmatch_t* match_offsets)
{
  char line[8];
  char message[100];
  copyFromBufferToString(line, match_offsets);
  if (colorLineEqual(&node->info[0], edgeColor, line)) {
    return;
  }
  if (colorLineEqual(&node->info[1], edgeColor, line)) {
    return;
  }
  if (copyLineByColor(&node->info[0], edgeColor, line)) {
    return;
  }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow"
  sprintf(message, "node: %s, %s^%s, %s^%s, %s^%s", node->id,
          node->info[0].color, node->info[0].line, node->info[1].color,
          node->info[1].line, edgeColor, line);
#pragma GCC diagnostic pop
  TEST_ASSERT_MESSAGE(copyLineByColor(&node->info[1], edgeColor, line),
                      message);
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
  compileRegex(&nodeDataRegex,
               "<data key=\"colors\">([^<]*)</data>[ \n]*<data "
               "key=\"primary\">([^<]*)</data>[ \n]*<data "
               "key=\"secondary\">([^<]*)</data>");
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
    printf("Failed at %ld\n>>>> %.30s\n", outputBufferPtr - outputBuffer,
           outputBufferPtr);
    TEST_FAIL();
  }
  outputBufferPtr += offsets[0].rm_eo;
}

static void firstNodeMatch(regmatch_t* nodeIdOffsets)
{
  struct nodeInfo* entry;
  char nodeColors[8];
  char message[100];
  entry = findNode(nodeIdOffsets);
  // printf("Added node %s\n", entry->id);
  TEST_ASSERT_NOT_NULL(entry);
  sprintf(message, "Added node %s", entry->id);
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, entry->node, message);
  entry->node = 1;
  if (offsets[2].rm_eo != -1) {
    entry->corner = 1;
  }
  matchRegex(&nodeDataRegex);
  copyFromBufferToString(nodeColors, offsets + 1);
  copyFromBufferToString(entry->info[0].color, offsets + 2);
  copyFromBufferToString(entry->info[1].color, offsets + 3);
  sprintf(message + strlen(message),
          "nodeColors: %s, primary: %s, secondary: %s", nodeColors,
          entry->info[0].color, entry->info[1].color);
  size_t i;
  for (i = 0;; i++) {
    if (nodeColors[i] == 0) {
      break;
    }
    TEST_ASSERT_MESSAGE(nodeColors[i] >= 'a' || nodeColors[i] <= 'f', message);
    if (i == 0) continue;
    TEST_ASSERT_MESSAGE(nodeColors[i] > nodeColors[i - 1], message);
  }
  TEST_ASSERT_MESSAGE(strchr(nodeColors, entry->info[0].color[0]) != NULL,
                      message);
  TEST_ASSERT_MESSAGE(strchr(nodeColors, entry->info[1].color[0]) != NULL,
                      message);
}

static void firstEdgeMatch(regmatch_t* sourceOffsets, regmatch_t* targetOffsets)
{
  struct nodeInfo *entry, *target;
  entry = findNode(sourceOffsets);
  TEST_ASSERT_NOT_NULL(entry);
  TEST_ASSERT_LESS_OR_EQUAL(1, entry->source);
  entry->source++;
  target = findNode(targetOffsets);
  TEST_ASSERT_LESS_OR_EQUAL(1, target->target);
  target->target++;
}

static void noop(regmatch_t* nodeIdOffsets)
{
  (void)nodeIdOffsets;
  // nothing.
}

static void secondEdgeMatch(regmatch_t* sourceOffsets,
                            regmatch_t* targetOffsets)
{
  char edgeColor[8];
  struct nodeInfo* source = findNode(sourceOffsets);
  struct nodeInfo* target = findNode(targetOffsets);
  matchRegex(&edgeDataRegex);
  copyFromBufferToString(edgeColor, offsets + 1);
  TEST_ASSERT_EQUAL(0, edgeColor[1]);
  checkLineAndColorInNode(source, edgeColor, offsets + 2);
  checkLineAndColorInNode(target, edgeColor, offsets + 2);
}

static void matchGraphMlFile(void (*nodeMatch)(regmatch_t*),
                             void (*edgeMatch)(regmatch_t*, regmatch_t*))
{
  int nodeCounter = 0, edgeCounter = 0;
  outputBufferPtr = outputBuffer;
  matchRegex(&graphRegex);
  while (outputBufferPtr < outputBuffer + 50000) {
    matchRegex(&nodeOrEdgeRegex);

    if (offsets[5].rm_eo != -1) {
      break;
    }
    if (offsets[1].rm_eo != -1) {
      nodeCounter++;
      nodeMatch(offsets + 1);
    } else if (offsets[3].rm_eo != -1) {
      edgeCounter++;
      edgeMatch(offsets + 3, offsets + 4);
    }
  }
  TEST_ASSERT_EQUAL_MESSAGE(NFACES - 2 + 3 * NCOLORS, nodeCounter,
                            "node count: Euler, and 6 triangles");
  TEST_ASSERT_EQUAL_MESSAGE(2 * (NFACES - 2) + 3 * NCOLORS, edgeCounter,
                            "edge count: Euler, and 6 triangles");
}

static void initializeGraphMLTest(void)
{
  memset(nodes, 0, sizeof(nodes));
  FopenCount = 0;
  nodeCount = 0;
  compileRegexes();
}

static void verifyGraphMLFileOperations(void)
{
  graphmlSaveAllVariations("/tmp/foo", 100000);
  TEST_ASSERT_EQUAL(1, FopenCount);
}

static void matchAndVerifyNodeCount(void)
{
  size_t completeNodeCount;

  matchGraphMlFile(firstNodeMatch, firstEdgeMatch);
  completeNodeCount = nodeCount;
  matchGraphMlFile(noop, secondEdgeMatch);
  TEST_ASSERT_EQUAL(completeNodeCount, nodeCount);
}

static void verifyNodeProperties(struct nodeInfo* entry, char* message)
{
  TEST_ASSERT_EQUAL_MESSAGE(1, entry->node, message);
  TEST_ASSERT_EQUAL_MESSAGE(entry->info[0].color[0], entry->info[0].line[0],
                            message);
  TEST_ASSERT_EQUAL_MESSAGE(entry->info[1].color[0], entry->info[1].line[0],
                            message);
}

static void verifyCornerNodeProperties(struct nodeInfo* entry, char* message)
{
  TEST_ASSERT_EQUAL_MESSAGE(1, entry->target, message);
  TEST_ASSERT_EQUAL_MESSAGE(1, entry->source, message);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(entry->id[2], entry->info[0].line[1], message);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(entry->id[2], entry->info[1].line[1], message);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(entry->info[0].line[1], entry->info[1].line[1],
                                message);
}

static void verifyNonCornerNodeProperties(struct nodeInfo* entry, char* message)
{
  TEST_ASSERT_EQUAL_MESSAGE(2, entry->target, message);
  TEST_ASSERT_EQUAL_MESSAGE(2, entry->source, message);
}

static void verifyAllNodes(void)
{
  unsigned int i;
  struct nodeInfo* entry;
  char message[100];

  for (i = 0; i < nodeCount; i++) {
    sprintf(message, "Node[%d/%lu] %s: %d %d => %d", i, nodeCount, nodes[i].id,
            nodes[i].node, nodes[i].source, nodes[i].target);
    entry = &nodes[i];

    verifyNodeProperties(entry, message);

    if (entry->corner) {
      verifyCornerNodeProperties(entry, message);
    } else {
      verifyNonCornerNodeProperties(entry, message);
    }
  }
}

static void checkGraphML(void)
{
  initializeGraphMLTest();
  verifyGraphMLFileOperations();
  matchAndVerifyNodeCount();
  verifyAllNodes();
  freeRegexes();
}

/* This test exercises the code for dealing with two corners in the same face.
The specific result picked out is 654444-26/6c/037.xml which includes three
lots of such corner pairs.
*/
static void variant14188(void)
{
  regex_t internalEdgeRegex;
  compileRegex(&internalEdgeRegex,
               "<edge source=\"[a-f]_[0-2]\" target=\"[a-f]_[0-2]\">");
  /* Run the program with -d 654444
  then inspect the following file to see the graphml being tested.
   654444-26/6c/037.xml
   To convert the file name to variant number user bc
   > ibase=16
   > 37 * 100 + 6C
   */
  MaxVariantsPerSolution = 14188;
  IgnoreFirstVariantsPerSolution = MaxVariantsPerSolution - 1;
  checkGraphML();
  outputBufferPtr = outputBuffer;
  matchRegex(&internalEdgeRegex);
  matchRegex(&internalEdgeRegex);
  matchRegex(&internalEdgeRegex);
  TEST_ASSERT_EQUAL(REG_NOMATCH,
                    regexec(&internalEdgeRegex, outputBufferPtr, 0, NULL, 0));
}

static void testVariationCount()
{
  TEST_ASSERT_EQUAL(128, searchCountVariations(NULL));
}

static int ContinuationCount = 0;
static int countContinuation(COLOR color, EDGE (*corners)[3])
{
  (void)color;    // Explicitly mark as unused
  (void)corners;  // Explicitly mark as unused
  ContinuationCount++;
  return 0;
}

static void testColorContinuations(COLOR color, int expectedCount)
{
  EDGE corners[NCOLORS][3];
  ContinuationCount = 0;
  graphmlPossibleCorners();
  graphmlChooseCornersWithContinuation(0, color, corners, countContinuation);
  TEST_ASSERT_EQUAL(expectedCount, ContinuationCount);
}

static void foundSolutionColor0() { testColorContinuations(0, 8); }
static void foundSolutionColor1() { testColorContinuations(1, 1); }
static void foundSolutionColor2() { testColorContinuations(2, 2); }
static void foundSolutionColor3() { testColorContinuations(3, 1); }
static void foundSolutionColor4() { testColorContinuations(4, 2); }
static void foundSolutionColor5() { testColorContinuations(5, 4); }

static char* TestName;

static void setup645534()
{
  dynamicFaceSetupCentral(intArray(6, 4, 5, 5, 3, 4));
  MaxVariantsPerSolution = 1;
  ExpectedSignature =
      "McDpAzHcCtAgAyKaNnAwEiCxAeClCyDxBwFnAyJzBqFwAzEvBvAxAsCwAaBwKjEuNfKcBdDe"
      "BhAfAoCxDeBdEhEdAuBiDvCwBdDgBcEnAoDhApFyAcAtAdFzCvBpKoPd";
  ClassSignature =
      "PdJeIgFuAcEhAdDeDqAcDuGqElAdGnGhKoDpAbFnAnFeAaFwJdDmCzCyAeFkCyCzErDnDlDk"
      "BqFwAzHcEsAxEpEyJjAwDcCxKjDoAaFwAsDeBvNwDiEtFhEsAyOrJwMx";
}

static void setup654444()
{
  dynamicFaceSetupCentral(intArray(6, 5, 4, 4, 4, 4));
  MaxVariantsPerSolution = 10;
  IgnoreFirstVariantsPerSolution = 9;
  // From 654444-26.txt (this may change)
  ExpectedSignature =
      "OrCgChKeDtAoAwFhDxAcArEmDyBfDcEaAwAxBhFkBnBhAxEsEuDpBtFeBkBtFrEuLqBiBaDa"
      "EgApAdDvEyBmApDbDeAoGnDqBrAwBfDbAlBnAcDqDjKgAoEfAhBzKyPd";
  ClassSignature =
      "PdJeIgKbAcEhAdDwDqAcDbBeElAdEhBbNsFbEnCwAlFkAcCxEzDkDaDtFsDlElDsErDnDlEo"
      "BqFwAzEvEsAxFkBhFnBwFwBqEkDaEfCxAsDbCsEyDiHhDbDsDpMcIiOi";
}

static void (*SetupSearchTest)(void);
static void runSearchTest(void)
{
  Unity.NumberOfTests--;
  UNITY_NEW_TEST(TestName);
  SolutionCount = 0;
  SetupSearchTest();
  searchHere(true, foundBasicSolution);
}

#define RUN_SEARCH_TEST(setup, foundSolution) \
  TestName = #setup "-" #foundSolution;       \
  FoundSolution = foundSolution;              \
  SetupSearchTest = setup;                    \
  RUN_TEST(runSearchTest);

#define RUN_645534(foundSolution) RUN_SEARCH_TEST(setup645534, foundSolution)

#define RUN_654444(foundSolution) RUN_SEARCH_TEST(setup654444, foundSolution)

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
  RUN_654444(NULL);
  RUN_654444(checkGraphML);
  RUN_654444(variant14188);
  return UNITY_END();
}
