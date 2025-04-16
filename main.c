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

static char *TargetFolder = NULL;
int MaxVariantsPerSolution = INT_MAX;    // Maximum value means unlimited
int MaxSolutions = INT_MAX;              // Maximum value means unlimited
int IgnoreFirstVariantsPerSolution = 0;  // Default to not ignoring any variants
int IgnoreFirstSolutions = 0;           // Default to not ignoring any solutions
int CentralFaceDegrees[NCOLORS] = {0};  // Initialize all to 0

/* Declaration of file scoped static functions */
static void saveResult(void);
static void initializeOutputFolder(void);

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  while ((opt = getopt(argc, argv, "f:d:m:n:k:j:")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      case 'd':
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
        MaxSolutions = atoi(optarg);
        if (MaxSolutions <= 0) {
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
        IgnoreFirstSolutions = atoi(optarg);
        if (IgnoreFirstSolutions < 0) {
          fprintf(stderr, "ignoreFirstSolutions must be non-negative\n");
          return EXIT_FAILURE;
        }
        break;
      case 'j':
        IgnoreFirstVariantsPerSolution = atoi(optarg);
        if (IgnoreFirstVariantsPerSolution < 0) {
          fprintf(stderr,
                  "ignoreFirstVariantsPerSolution must be non-negative\n");
          return EXIT_FAILURE;
        }
        break;
      default:
        fprintf(stderr,
                "Usage: %s -f outputFolder [-d centralFaceDegrees] [-m "
                "maxSolutions] "
                "[-n maxVariantsPerSolution] [-k skipFirstSolutions] [-j "
                "skipFirstVariantsPerSolution]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || TargetFolder == NULL) {
    fprintf(
        stderr,
        "Usage: %s -f outputFolder [-d centralFaceDegrees] [-m maxSolutions] "
        "[-n maxVariantsPerSolution] [-k skipFirstSolutions] [-j "
        "skipFirstVariantsPerSolution]\n",
        argv[0]);
    return EXIT_FAILURE;
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
  char *buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, faceDegreeSignature());
  solutionWrite(usingBuffer(buffer));
}

static void initializeOutputFolder() { initializeFolder(TargetFolder); }
