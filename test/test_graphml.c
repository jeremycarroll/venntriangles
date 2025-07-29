/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#define _GNU_SOURCE
#include "common.h"
#include "face.h"
#include "helper_for_tests.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"

#include <regex.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"

static int FopenCount = 0;
extern int VariationNumberIPC;
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
static PREDICATE* TestProgram;
static void (*SetupSearchTest)(void);

static void mockInitializeFolder(const char* folder)
{
  (void)folder;  // Explicitly mark as unused
}

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

void setUp(void)
{
  initializeStatisticLogging("/dev/null", 1000, 1000);  // Reduce logging
  GraphmlFileOps.initializeFolder = mockInitializeFolder;
  GraphmlFileOps.fopen = mockFopen;
}

void tearDown(void)
{ /* Handle failure case when unity uses long jump out of the engine */
  engineClear(&TestStack);
}

static int SolutionCount = 0;

static void endCountVariations(void)
{
  TEST_ASSERT_EQUAL(1, FopenCount);
}
FORWARD_BACKWARD_PREDICATE_STATIC(CountVariations, NULL, NULL,
                                  endCountVariations)

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
  while (outputBufferPtr < outputBuffer + sizeof(outputBuffer)) {
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

static bool forwardGraphML(void)
{
  memset(nodes, 0, sizeof(nodes));
  nodeCount = 0;
  compileRegexes();
  return true;
}

static void backwardGraphML(void)
{
  TEST_ASSERT_EQUAL(1, FopenCount);
  matchAndVerifyNodeCount();
  verifyAllNodes();
  freeRegexes();
}

FORWARD_BACKWARD_PREDICATE_STATIC(CheckGraphML, NULL, forwardGraphML,
                                  backwardGraphML)

static regex_t internalEdgeRegex;
static bool forwardVariant14188(void)
{
  compileRegex(&internalEdgeRegex,
               "<edge source=\"[a-f]_[0-2]\" target=\"[a-f]_[0-2]\">");
  /* Test output: 654444-26/6c/037.xml (variant 14188 = 0x37*0x100+0x6C) */
  MaxVariantsPerSolutionFlag = 14188;
  IgnoreFirstVariantsPerSolution = MaxVariantsPerSolutionFlag - 1;
  return true;
}

static void backwardVariant14188(void)
{
  outputBufferPtr = outputBuffer;
  matchRegex(&internalEdgeRegex);
  matchRegex(&internalEdgeRegex);
  matchRegex(&internalEdgeRegex);
  TEST_ASSERT_EQUAL(REG_NOMATCH,
                    regexec(&internalEdgeRegex, outputBufferPtr, 0, NULL, 0));
}
FORWARD_BACKWARD_PREDICATE_STATIC(Variant14188, NULL, forwardVariant14188,
                                  backwardVariant14188)

static bool forwardVariant1319(void)
{
  /* Test output: 555444-64/27/005.xml (variant 1319 = 0x5*0x100+0x27) */
  compileRegex(&internalEdgeRegex,
               "<edge source=\"([a-f])_([0-2])\" target=\"([^\"]*)\"");
  return true;
}

static char* knownSolutionCorners[6][3] = {
    // "A" triangle corners:
    {"p_ab_b_a", "p_ace_e_a", "p_acf_c_a"},
    // "B" triangle corners:
    {"p_bc_c_b", "p_bde_d_b", "p_abd_a_b"},
    // "C" triangle corners:
    {"p_ce_e_c", "p_cdf_d_c", "p_abcf_f_c"},
    // "D" triangle corners:
    {"p_def_f_d", "p_ad_a_d", "p_acd_a_d"},
    // "E" triangle corners:
    {"p_ef_f_e", "p_abe_b_e", "e_0"},
    // "F" triangle corners:
    {"p_df_d_f", "p_bcf_c_f", "p_bdef_d_f"}};

static void backwardVariant1319(void)
{
  char nextFace[30];
  int color;
  int cornerIndex;
  outputBufferPtr = outputBuffer;
  for (int i = 0; i < 18; i++) {
    matchRegex(&internalEdgeRegex);
    snprintf(nextFace, sizeof(nextFace), "%.*s",
             (int)(offsets[3].rm_eo - offsets[3].rm_so),
             offsetBase + offsets[3].rm_so);
    color = offsetBase[offsets[1].rm_so] - 'a';
    cornerIndex = offsetBase[offsets[2].rm_so] - '0';
    TEST_ASSERT_EQUAL_STRING(knownSolutionCorners[color][cornerIndex],
                             nextFace);
  }
  TEST_ASSERT_EQUAL(REG_NOMATCH,
                    regexec(&internalEdgeRegex, outputBufferPtr, 0, NULL, 0));
}
FORWARD_BACKWARD_PREDICATE_STATIC(Variant1319, NULL, forwardVariant1319,
                                  backwardVariant1319)

extern int searchCountVariations(void);

static bool testVariationEstimate()
{
  TEST_ASSERT_EQUAL(128, searchCountVariations());
  return false;
}

static int CornerCountColor;
static int CornerCountExpected;

static bool testCornerCount()
{
  int multiple = 1;
  for (int i = 0; i < 3; i++) {
    multiple *= CornersPredicate.try(3 * CornerCountColor + i).numberOfChoices;
  }
  TEST_ASSERT_EQUAL(CornerCountExpected, multiple);
  return false;
}
FORWARD_BACKWARD_PREDICATE_STATIC(CornerCount, testCornerCount, NULL, NULL)

static bool gate()
{
  SIGNATURE signature = s6SignatureFromFaces();
  SIGNATURE classSignature = s6MaxSignature();
  if (strcmp(ExpectedSignature, s6SignatureToString(signature)) != 0) {
    return false;
  }
  VariationNumberIPC = 1;
  SolutionCount++;
  TEST_ASSERT_EQUAL_STRING(ClassSignature, s6SignatureToString(classSignature));
  return true;
}

static void run(void)
{
  SolutionCount = 0;
  FopenCount = 0;
  SetupSearchTest();
  engine(&TestStack, TestProgram);
  TEST_ASSERT_EQUAL(1, SolutionCount);
}

#define RUN_SEARCH_TEST(signature, line, program) \
  TestProgram = program;                          \
  SetupSearchTest = setup##signature;             \
  UnityDefaultTestRun(run, #signature "-" #program, line)

#define RUN_645534(program) RUN_SEARCH_TEST(645534, __LINE__, program)
#define RUN_654444(program) RUN_SEARCH_TEST(654444, __LINE__, program)
#define RUN_KNOWN(program) RUN_SEARCH_TEST(Known, __LINE__, program)

FORWARD_BACKWARD_PREDICATE_STATIC(Gate, gate, NULL, NULL)
FORWARD_BACKWARD_PREDICATE_STATIC(VariationCount, testVariationEstimate, NULL,
                                  NULL)

static PREDICATE Basic[] = {&InitializePredicate, &InnerFacePredicate,
                            &VennPredicate, &GatePredicate, &FAILPredicate};
static PREDICATE EstimateCount[] = {&InitializePredicate, &InnerFacePredicate,
                                    &VennPredicate, &GatePredicate,
                                    &VariationCountPredicate};
static PREDICATE ExactCount[] = {&InitializePredicate,
                                 &InnerFacePredicate,
                                 &VennPredicate,
                                 &GatePredicate,
                                 &CountVariationsPredicate,
                                 &CornersPredicate,
                                 &GraphMLPredicate,
                                 &FAILPredicate};
static PREDICATE CheckGraphML[] = {&InitializePredicate, &CheckGraphMLPredicate,
                                   &InnerFacePredicate,  &VennPredicate,
                                   &GatePredicate,       &CornersPredicate,
                                   &GraphMLPredicate,    &FAILPredicate};
static PREDICATE Variant14188[] = {
    &InitializePredicate, &Variant14188Predicate, &CheckGraphMLPredicate,
    &InnerFacePredicate,  &VennPredicate,         &GatePredicate,
    &CornersPredicate,    &GraphMLPredicate,      &FAILPredicate};
static PREDICATE Variant1319[] = {
    &InitializePredicate, &Variant1319Predicate, &CheckGraphMLPredicate,
    &InnerFacePredicate,  &VennPredicate,        &GatePredicate,
    &CornersPredicate,    &GraphMLPredicate,     &FAILPredicate};
static PREDICATE CornerCount[] = {&InitializePredicate, &InnerFacePredicate,
                                  &VennPredicate, &GatePredicate,
                                  &CornerCountPredicate};

static void initializeFaceDegree(int a, int b, int c, int d, int e, int f)
{
  CentralFaceDegreesFlag[0] = a;
  CentralFaceDegreesFlag[1] = b;
  CentralFaceDegreesFlag[2] = c;
  CentralFaceDegreesFlag[3] = d;
  CentralFaceDegreesFlag[4] = e;
  CentralFaceDegreesFlag[5] = f;
}

static void setup645534()
{
  initializeFaceDegree(6, 4, 5, 5, 3, 4);
  MaxVariantsPerSolutionFlag = 1;
  IgnoreFirstVariantsPerSolution = 0;
  LevelsIPC = 1;
  ExpectedSignature =
      "KwDpAzIbCrAfAwKgNvAvEaCxAeClCyEcBvFoAwJyBrFuAzFcBlAyArCwAaBvKmEqNtHrBbDd"
      "BmAgAoCxDdBbEjEgAuBoDvCwBbDeBdEnAoDhApFyAcAtAdGeCvBjKoPd";
  ClassSignature =
      "PdHlHfFwAcEjAdDdDqAcDuGsEbAdGpGkKoDpAbFoAnFiAaFuJqDkDaCyAeFnCyDaEpDoDnDj"
      "BrFuAzIbFbAyFaFgJiAvDcCxKmDlAaFuArDdBlNnDiFeFmFbAwOtKdNi";
}

static void setup654444()
{
  initializeFaceDegree(6, 5, 4, 4, 4, 4);
  MaxVariantsPerSolutionFlag = 10;
  IgnoreFirstVariantsPerSolution = 9;
  LevelsIPC = 2;
  // From 654444-26.txt (this may change)
  ExpectedSignature =
      "OtCaCkKhDrAoAvFmEcAcAsElEeBfDcEdAvAyBmFnBqBmAyFbEqDpBuFiBnBuFvEqMiBoBaCz"
      "EhApAdDvFgBiApDbDdAoGpDqBtAvBfDbAjBqAcDqDmHsAoDzAhCeKqPd";

  ClassSignature =
      "PdHlHfKiAcEjAdDwDqAcDbBeEbAdEjBcOaFhEnCwAjFnAcCxFjDjCzDrEyDnEbDtEpDoDnEo"
      "BrFuAzFcFbAyFnBmFoBvFuBrEmCzDzCxArDbCdFgDiIgDbDtDpKwIxOq";
}

static void setupKnown()
{
  initializeFaceDegree(5, 5, 5, 4, 4, 4);
  LevelsIPC = 2;
  MaxVariantsPerSolutionFlag = 1319;
  IgnoreFirstVariantsPerSolution = MaxVariantsPerSolutionFlag - 1;
  ExpectedSignature =
      "MkBuFxHjCjAuEhCyFtBnDlFvAhBnEtFvGsAsEhGiCbAxElDaDgAqDdEgAfBuFwEqHuAyDkDo"
      "AgBmFnIsJpBvDpEqAeBqDnEoJoAvCzDbAeBqDbDqDzAtEnGeAnCeHfPd";
  ClassSignature =
      "PdHlCeAhBaEjAtDdDqEjEoFlGdEbDfDgEoFkBqAeAsDbCiAgFjEvFjEvAqCzHaFkGzFxBxBj"
      "AtDeArGtDaDeDmHsEfDhDcGaFpDkAvAyBvJhBrCsEpDoDiFeBrKhFvLa";
}

#define RUN_CORNER_COUNT(color, expected)                       \
  {                                                             \
    TestProgram = CornerCount;                                  \
    CornerCountColor = color;                                   \
    CornerCountExpected = expected;                             \
    SetupSearchTest = setup645534;                              \
    UnityDefaultTestRun(run, "645534-Color-" #color, __LINE__); \
  };

int main(void)
{
  UNITY_BEGIN();
  RUN_654444(Basic);
  RUN_654444(Variant14188);
  RUN_654444(CheckGraphML);
  RUN_645534(Basic);
  RUN_645534(EstimateCount);
  RUN_645534(ExactCount);
  RUN_645534(CheckGraphML);
  RUN_KNOWN(Basic);
  RUN_KNOWN(Variant1319);
  RUN_CORNER_COUNT(0, 8);
  RUN_CORNER_COUNT(1, 1);
  RUN_CORNER_COUNT(2, 2);
  RUN_CORNER_COUNT(3, 1);
  RUN_CORNER_COUNT(4, 2);
  RUN_CORNER_COUNT(5, 4);
  return UNITY_END();
}
