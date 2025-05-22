/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "save.h"

#include "dataflow.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "predicates.h"
#include "s6.h"
#include "statistics.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int PerFaceDegreeSolutionNumber;
int VariationCount;
/* Static variables for solution writing */
static char* currentFilename;
static FILE* currentFile;
static int currentNumberOfVariations;
static char currentVariationMultiplication[128];
static void solutionPrint(FILE* fp);

static bool gateSave(void)
{
  if ((int64_t)GlobalSolutionsFound <= GlobalSkipSolutions) {
    return false;
  }
  if ((int64_t)GlobalSolutionsFound > GlobalMaxSolutions) {
    return false;
  }
  if (PerFaceDegreeSolutionNumber <= PerFaceDegreeSkipSolutions ||
      PerFaceDegreeSolutionNumber > PerFaceDegreeMaxSolutions) {
    return false;
  }
  char* buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, s6FaceDegreeSignature());
  return true;
}

static bool beforeVariantsSave(void)
{
  char* buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, s6FaceDegreeSignature());
  currentFilename = usingBuffer(buffer);

  snprintf(CurrentPrefix, sizeof(CurrentPrefix), "%s-%2.2d.txt",
           currentFilename, PerFaceDegreeSolutionNumber);
  currentFile = fopen(CurrentPrefix, "w");
  if (currentFile == NULL) {
    perror(CurrentPrefix);
    exit(EXIT_FAILURE);
  }
  VariationNumber = 1;
  solutionPrint(currentFile);
  CurrentPrefix[strlen(CurrentPrefix) - 4] = '\0';
  GraphmlFileOps.initializeFolder(CurrentPrefix);
  currentNumberOfVariations =
      searchCountVariations(currentVariationMultiplication);
  Levels = numberOfLevels(currentNumberOfVariations);
  fprintf(currentFile, "\nSolution signature %s\nClass signature %s\n",
          s6SignatureToString(s6SignatureFromFaces()),
          s6SignatureToString(s6MaxSignature()));
  fflush(currentFile);

  return true;
}

static void afterVariantsSave(void)
{
  fprintf(currentFile, "Number of variations: %d/%d = 1%s\n",

          VariationNumber - 1, currentNumberOfVariations,
          currentVariationMultiplication);
  VariationCount += VariationNumber - 1;
  fclose(currentFile);
}

FORWARD_BACKWARD_PREDICATE(Save, gateSave, beforeVariantsSave,
                           afterVariantsSave)

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
