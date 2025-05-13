/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "solutionwrite.h"

#include "engine.h"
#include "face.h"
#include "graphml.h"
#include "main.h"
#include "memory.h"
#include "s6.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"
#include "vsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: think about these.
extern int PerFaceDegreeSolutionNumber;
extern int VariationCount;
extern const char* TargetFolder;
/* Static variables for solution writing */
static char* currentFilename;
static FILE* currentFile;
static int currentNumberOfVariations;
static char currentVariationMultiplication[128];
static void solutionPrint(FILE* fp);

static struct predicateResult trySolutionWrite(int round)
{
  // Check if we should skip this solution based on global limits
  if ((int64_t)GlobalSolutionsFound <= GlobalSkipSolutions) {
    return PredicateFail;
  }

  // Check if we've hit the global maximum limit
  if ((int64_t)GlobalSolutionsFound > GlobalMaxSolutions) {
    return PredicateFail;
  }

  if (PerFaceDegreeSolutionNumber <= PerFaceDegreeSkipSolutions ||
      PerFaceDegreeSolutionNumber > PerFaceDegreeMaxSolutions) {
    return PredicateFail;
  }

  // Return choice of two items - first for initial write, second for final
  // write
  return predicateChoices(2, NULL);
}

static struct predicateResult retrySolutionWrite(int round, int choice)
{
  switch (choice) {
    case 0:
      // First choice: Do initial write
      char* buffer = getBuffer();
      sprintf(buffer, "%s/%s", TargetFolder, s6FaceDegreeSignature());
      currentFilename = usingBuffer(buffer);

      char filename[1024];
      snprintf(filename, sizeof(filename), "%s-%2.2d.txt", currentFilename,
               PerFaceDegreeSolutionNumber);
      currentFile = fopen(filename, "w");
      if (currentFile == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
      }
      solutionPrint(currentFile);
      filename[strlen(filename) - 4] = '\0';
      currentNumberOfVariations =
          searchCountVariations(currentVariationMultiplication);
      fprintf(currentFile, "\nSolution signature %s\nClass signature %s\n",
              d6SignatureToString(s6SignatureFromFaces()),
              d6SignatureToString(s6MaxSignature()));
      fflush(currentFile);
      VariationCount += currentNumberOfVariations;
      return PredicateSuccessNextPredicate;
    case 1:
      // Second choice: Do final write and fail
      int actualNumberOfVariations =
          graphmlSaveAllVariations(currentFilename, currentNumberOfVariations);
      fprintf(currentFile, "Number of variations: %d/%d = 1%s\n",
              actualNumberOfVariations, currentNumberOfVariations,
              currentVariationMultiplication);
      fclose(currentFile);
      return PredicateFail;
    default:
      assert(0);
  }
}

/* The predicates array for solution writing */
static struct predicate solutionWritePredicate = {trySolutionWrite,
                                                  retrySolutionWrite};
static struct predicate* predicates[] = {&solutionWritePredicate, NULL};

void writeSolution(void)
{
  engine(predicates, NULL);
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
