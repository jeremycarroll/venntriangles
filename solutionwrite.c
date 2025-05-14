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
extern int VariationNumber;
extern int Levels;
extern const char* TargetFolder;
extern char CurrentPrefix[1024];
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

  char* buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, s6FaceDegreeSignature());
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
      graphmlFileOps.initializeFolder(CurrentPrefix);
      currentNumberOfVariations =
          searchCountVariations(currentVariationMultiplication);
      Levels = numberOfLevels(currentNumberOfVariations);
      fprintf(currentFile, "\nSolution signature %s\nClass signature %s\n",
              d6SignatureToString(s6SignatureFromFaces()),
              d6SignatureToString(s6MaxSignature()));
      fflush(currentFile);
      VariationCount += currentNumberOfVariations;
      return PredicateSuccessNextPredicate;
    case 1:
      // Second choice: Do final write and fail
      fprintf(currentFile, "Number of variations: %d/%d = 1%s\n",

              VariationNumber - 1, currentNumberOfVariations,
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

extern struct predicate cornersPredicate;
extern struct predicate saveVariationPredicate;
static struct predicate* predicates[] = {
    &solutionWritePredicate, &cornersPredicate, &saveVariationPredicate,
    &failPredicate};

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
