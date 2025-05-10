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
int MaxVariantsPerSolution = INT_MAX;     // Maximum value means unlimited
int GlobalMaxSolutions = INT_MAX;         // Global maximum solutions
int PerFaceDegreeMaxSolutions = INT_MAX;  // Per face degree maximum solutions
int GlobalSkipSolutions = 0;              // Global solutions to skip
int PerFaceDegreeSkipSolutions = 0;       // Per face degree solutions to skip
int IgnoreFirstVariantsPerSolution = 0;  // Default to not ignoring any variants
int CentralFaceDegrees[NCOLORS] = {0};   // Initialize all to 0
uint64 GlobalSolutionsFound = 0;         // Counter for solutions found
bool VerboseMode = false;                // Controls verbose output mode

/* Declaration of file scoped static functions */
static void saveResult(void);
static void initializeOutputFolder(void);
static void setFaceDegrees(const char *faceDegrees);
static int parsePostiveArgument(const char *arg, char flag, bool allowZero);

char *Argv0;

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  bool hasFaceDegrees = false;
  int localMaxSolutions = INT_MAX;
  int localSkipSolutions = 0;
  Argv0 = argv[0];

  while ((opt = getopt(argc, argv, "f:d:m:n:k:j:v")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      case 'd':
        hasFaceDegrees = true;
        setFaceDegrees(optarg);
        break;
      case 'm':
        localMaxSolutions = parsePostiveArgument(optarg, 'm', false);
        break;
      case 'n':
        MaxVariantsPerSolution = parsePostiveArgument(optarg, 'n', false);
        break;
      case 'k':
        localSkipSolutions = parsePostiveArgument(optarg, 'k', true);
        break;
      case 'j':
        IgnoreFirstVariantsPerSolution =
            parsePostiveArgument(optarg, 'j', true);
        break;
      case 'v':
        VerboseMode = true;
        break;
      default:
        disaster("Invalid option");
    }
  }

  if (optind != argc) {
    disaster("Invalid option");
  }

  if (TargetFolder == NULL) {
    disaster("Output folder not specified");
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

static void setFaceDegrees(const char *faceDegrees)
{
  if (strlen(faceDegrees) != NCOLORS) {
    fprintf(stderr, "Central face degrees string must be exactly %d digits\n",
            NCOLORS);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < NCOLORS; i++) {
    char c = faceDegrees[i];
    if (c < '3' || c > '6') {
      fprintf(stderr,
              "Each digit in central face degrees must be between 3 and 6\n");
      exit(EXIT_FAILURE);
    }
    CentralFaceDegrees[i] = c - '0';
  }
}

static int parsePostiveArgument(const char *arg, char flag, bool allowZero)
{
  char *endptr;
  char errorMessage[100];
  sprintf(errorMessage, "-%c must be a %s integer.", flag,
          allowZero ? "non-negative" : "positive");
  int value = strtol(arg, &endptr, 10);
  if (value <= 0) {
    disaster(errorMessage);
  }
  if (endptr == arg) {
    disaster(errorMessage);
  }
  if (value == 0 && !allowZero) {
    disaster(errorMessage);
  }
  if (*endptr != '\0') {
    disaster(errorMessage);
  }
  return value;
}

/* File scoped static functions */
static void saveResult(void)
{
  // GlobalSolutionsFound is incremented in vsearch.c

  // Check if we should skip this solution based on global limits
  if ((int64_t)GlobalSolutionsFound <= GlobalSkipSolutions) {
    return;
  }

  // Check if we've hit the global maximum limit
  if ((int64_t)GlobalSolutionsFound > GlobalMaxSolutions) {
    return;
  }

  char *buffer = getBuffer();
  sprintf(buffer, "%s/%s", TargetFolder, s6FaceDegreeSignature());
  searchSolutionWrite(usingBuffer(buffer));
}

static void initializeOutputFolder() { initializeFolder(TargetFolder); }
