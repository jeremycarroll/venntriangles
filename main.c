/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"

#include "memory.h"
#include "statistics.h"
#include "utils.h"
#include "vsearch.h"

#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define USAGE_ONE_LINE                                                   \
  "Usage: %s -f outputFolder [-d centralFaceDegrees] [-m maxSolutions] " \
  "[-n maxVariantsPerSolution] [-k skipFirstSolutions] [-j "             \
  "skipFirstVariantsPerSolution]\n"

#define USAGE_WITH_D_EXPLANATION                                              \
  "When -d is specified, -m and -k apply to solutions with that face degree " \
  "pattern.\n"                                                                \
  "Otherwise, they apply globally across all face degree patterns.\n"

static char *TargetFolder = NULL;
int MaxVariantsPerSolution = INT_MAX;     // Maximum value means unlimited
int GlobalMaxSolutions = INT_MAX;         // Global maximum solutions
int PerFaceDegreeMaxSolutions = INT_MAX;  // Per face degree maximum solutions
int GlobalSkipSolutions = 0;              // Global solutions to skip
int PerFaceDegreeSkipSolutions = 0;       // Per face degree solutions to skip
int IgnoreFirstVariantsPerSolution = 0;  // Default to not ignoring any variants
int CentralFaceDegrees[NCOLORS] = {0};   // Initialize all to 0
int GlobalSolutionsFound = 0;            // Counter for solutions found

/* Declaration of file scoped static functions */
static void saveResult(void);
static void initializeOutputFolder(void);

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  bool hasFaceDegrees = false;
  int localMaxSolutions = INT_MAX;
  int localSkipSolutions = 0;

  while ((opt = getopt(argc, argv, "f:d:m:n:k:j:")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      case 'd':
        hasFaceDegrees = true;
        if (strlen(optarg) != NCOLORS) {
          fprintf(stderr,
                  "Central face degrees string must be exactly %d digits\n",
                  NCOLORS);
          return EXIT_FAILURE;
        }
        for (int i = 0; i < NCOLORS; i++) {
          char c = optarg[i];
          if (c < '3' || c > '6') {
            fprintf(
                stderr,
                "Each digit in central face degrees must be between 3 and 6\n");
            return EXIT_FAILURE;
          }
          CentralFaceDegrees[i] = c - '0';
        }
        break;
      case 'm':
        localMaxSolutions = atoi(optarg);
        if (localMaxSolutions <= 0) {
          fprintf(stderr, "maxSolutions must be a positive integer\n");
          return EXIT_FAILURE;
        }
        break;
      case 'n':
        MaxVariantsPerSolution = atoi(optarg);
        if (MaxVariantsPerSolution <= 0) {
          fprintf(stderr,
                  "maxVariantsPerSolution must be a positive integer\n");
          return EXIT_FAILURE;
        }
        break;
      case 'k':
        localSkipSolutions = atoi(optarg);
        if (localSkipSolutions < 0) {
          fprintf(stderr, "skipFirstSolutions must be non-negative\n");
          return EXIT_FAILURE;
        }
        break;
      case 'j':
        IgnoreFirstVariantsPerSolution = atoi(optarg);
        if (IgnoreFirstVariantsPerSolution < 0) {
          fprintf(stderr,
                  "skipFirstVariantsPerSolution must be non-negative\n");
          return EXIT_FAILURE;
        }
        break;
      default:
        fprintf(stderr, USAGE_ONE_LINE, argv[0]);
        fprintf(stderr, USAGE_WITH_D_EXPLANATION);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || TargetFolder == NULL) {
    fprintf(stderr, USAGE_ONE_LINE, argv[0]);
    fprintf(stderr, USAGE_WITH_D_EXPLANATION);
    return EXIT_FAILURE;
  }

  // Set the appropriate static variables based on whether -d was specified
  if (hasFaceDegrees) {
    PerFaceDegreeMaxSolutions = localMaxSolutions;
    PerFaceDegreeSkipSolutions = localSkipSolutions;
  } else {
    GlobalMaxSolutions = localMaxSolutions;
    GlobalSkipSolutions = localSkipSolutions;
  }

  initializeOutputFolder();
  initializeStatisticLogging("/dev/stdout", 200, 10);
  searchFull(saveResult);
  statisticPrintFull();
  return 0;
}

/* File scoped static functions */
static void saveResult(void)
{
  // GlobalSolutionsFound is incremented in vsearch.c

  // Check if we should skip this solution based on global limits
  if (GlobalSolutionsFound <= GlobalSkipSolutions) {
    return;
  }

  // Check if we've hit the global maximum limit
  if (GlobalSolutionsFound > GlobalMaxSolutions) {
    return;
  }

  char *buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, faceDegreeSignature());
  solutionWrite(usingBuffer(buffer));
}

static void initializeOutputFolder() { initializeFolder(TargetFolder); }
