/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "face.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

int PerFaceDegreeSolutionNumberIPC;
int VariationCountIPC;
static char* currentFilename;
static FILE* currentFile;
static int currentNumberOfVariations;
static char currentVariationMultiplication[128];

/* Count variations and build multiplication string for display */
int searchCountVariations(void)
{
  EDGE corners[3][2];
  int numberOfVariations = 1;
  int pLength;
  char* currentPos = currentVariationMultiplication;
  currentPos[0] = '\0';

  for (COLOR a = 0; a < NCOLORS; a++) {
    vertexAlignCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      if (corners[i][0] == NULL) {
        EDGE edge = vertexGetCentralEdge(a);
        pLength = edgePathLengthOnly(edge, edgeFollowBackwards(edge));
      } else {
        pLength = edgePathLengthOnly(corners[i][0]->reversed, corners[i][1]);
      }
      numberOfVariations *= pLength;
      if (pLength > 1) {
        currentPos += sprintf(currentPos, "*%d", pLength);
      }
    }
  }
  return numberOfVariations;
}

static void solutionPrint(FILE* fp)
{
  COLORSET colors = 0;
  if (fp == NULL) {
    fp = stdout;
  }

  while (true) {
    FACE face = Faces + colors;
    do {
      char buffer[1024];
      FACE next = face->next;
      COLORSET colorBeingDropped = face->colors & ~next->colors;
      COLORSET colorBeingAdded = next->colors & ~face->colors;
      sprintf(buffer, "%s [%c,%c] ", faceToString(face),
              colorToChar(ffs(colorBeingDropped) - 1),
              colorToChar(ffs(colorBeingAdded) - 1));
      if (strchr(buffer, '@')) {
        fprintf(stderr, "buffer: %s\n", buffer);
        fprintf(stderr, "faceToString: %s\n", faceToString(face));
        exit(EXIT_FAILURE);
      }
      fputs(buffer, fp);
      face = next;
    } while (face->colors != colors);
    fprintf(fp, "\n");
    if (colors == (NFACES - 1)) {
      break;
    }
    colors |= (face->previous->colors | 1);
  }
}

static bool gateSave(void)
{
  if ((int64_t)GlobalSolutionsFoundIPC <= GlobalSkipSolutionsFlag) {
    return false;
  }
  if ((int64_t)GlobalSolutionsFoundIPC > GlobalMaxSolutionsFlag) {
    return false;
  }
  if (PerFaceDegreeSolutionNumberIPC <= PerFaceDegreeSkipSolutionsFlag ||
      PerFaceDegreeSolutionNumberIPC > PerFaceDegreeMaxSolutionsFlag) {
    return false;
  }
  char* buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolderFlag, s6FaceDegreeSignature());
  return true;
}

static bool beforeVariantsSave(void)
{
  char* buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolderFlag, s6FaceDegreeSignature());
  currentFilename = usingBuffer(buffer);

  snprintf(CurrentPrefixIPC, sizeof(CurrentPrefixIPC), "%s-%2.2d.txt",
           currentFilename, PerFaceDegreeSolutionNumberIPC);
  currentFile = fopen(CurrentPrefixIPC, "w");
  if (currentFile == NULL) {
    perror(CurrentPrefixIPC);
    exit(EXIT_FAILURE);
  }
  VariationNumberIPC = 1;
  solutionPrint(currentFile);
  CurrentPrefixIPC[strlen(CurrentPrefixIPC) - 4] = '\0';
  GraphmlFileOps.initializeFolder(CurrentPrefixIPC);
  currentNumberOfVariations = searchCountVariations();
  LevelsIPC = numberOfLevels(currentNumberOfVariations);
  fprintf(currentFile, "\nSolution signature %s\nClass signature %s\n",
          s6SignatureToString(s6SignatureFromFaces()),
          s6SignatureToString(s6MaxSignature()));
  fflush(currentFile);

  return true;
}

static void afterVariantsSave(void)
{
  fprintf(currentFile, "Number of variations: %d/%d = 1%s\n",

          VariationNumberIPC - 1, currentNumberOfVariations,
          currentVariationMultiplication);
  VariationCountIPC += VariationNumberIPC - 1;
  fclose(currentFile);
}

FORWARD_BACKWARD_PREDICATE(Save, gateSave, beforeVariantsSave,
                           afterVariantsSave)
