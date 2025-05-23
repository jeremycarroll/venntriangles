/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"

#include "engine.h"
#include "nondeterminism.h"
#include "statistics.h"
#include "utils.h"

#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

char *TargetFolderFlag = NULL;
int MaxVariantsPerSolutionFlag = INT_MAX;  // Maximum value means unlimited
int GlobalMaxSolutionsFlag = INT_MAX;      // Global maximum solutions
int PerFaceDegreeMaxSolutionsFlag =
    INT_MAX;                             // Per face degree maximum solutions
int GlobalSkipSolutionsFlag = 0;         // Global solutions to skip
int PerFaceDegreeSkipSolutionsFlag = 0;  // Per face degree solutions to skip
int IgnoreFirstVariantsPerSolution = 0;  // Default to not ignoring any variants
FACE_DEGREE CentralFaceDegreesFlag[NCOLORS] = {0};  // Initialize all to 0
bool VerboseModeFlag = false;  // Controls verbose output mode
bool TracingFlag = false;      // Controls tracing output mode

static void initializeOutputFolder(void);
static void setFaceDegrees(const char *programName, const char *faceDegrees);
static int parsePostiveArgument(const char *programName, const char *arg,
                                char flag, bool allowZero);

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolderFlag = NULL;
  bool hasFaceDegrees = false;
  int localMaxSolutions = INT_MAX;
  int localSkipSolutions = 0;
  char *programName = argv[0];

  while ((opt = getopt(argc, argv, "f:d:m:n:k:j:vt")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolderFlag = optarg;
        break;
      case 'd':
        hasFaceDegrees = true;
        setFaceDegrees(programName, optarg);
        break;
      case 'm':
        localMaxSolutions =
            parsePostiveArgument(programName, optarg, 'm', false);
        break;
      case 'n':
        MaxVariantsPerSolutionFlag =
            parsePostiveArgument(programName, optarg, 'n', false);
        break;
      case 'k':
        localSkipSolutions =
            parsePostiveArgument(programName, optarg, 'k', true);
        break;
      case 'j':
        IgnoreFirstVariantsPerSolution =
            parsePostiveArgument(programName, optarg, 'j', true);
        break;
      case 'v':
        VerboseModeFlag = true;
        break;
      case 't':
        TracingFlag = true;
        break;
      default:
        disaster(programName, "Invalid option");
    }
  }
  if (optind != argc) {
    disaster(programName, "Invalid option");
  }
  if (TargetFolderFlag == NULL) {
    disaster(programName, "Output folder not specified");
  }
  // Set the appropriate static variables based on whether -d was specified
  if (hasFaceDegrees) {
    PerFaceDegreeMaxSolutionsFlag = localMaxSolutions;
    PerFaceDegreeSkipSolutionsFlag = localSkipSolutions;
  } else {
    GlobalMaxSolutionsFlag = localMaxSolutions;
    GlobalSkipSolutionsFlag = localSkipSolutions;
  }

  initializeOutputFolder();
  initializeStatisticLogging("/dev/stdout", 200, 10);

  engine(NonDeterministicProgram);

  statisticPrintFull();
  return 0;
}

static void setFaceDegrees(const char *programName, const char *faceDegrees)
{
  if (strlen(faceDegrees) != NCOLORS) {
    disaster(programName,
             "Central face degrees string must be exactly " STRINGIFY(
                 NCOLORS) " digits");
  }
  for (int i = 0; i < NCOLORS; i++) {
    char c = faceDegrees[i];
    if (c < '3' || c > '6') {
      disaster(programName,
               "Each digit in central face degrees must be between 3 and 6\n");
    }
    CentralFaceDegreesFlag[i] = c - '0';
  }
}

static int parsePostiveArgument(const char *programName, const char *arg,
                                char flag, bool allowZero)
{
  char *endptr;
  char errorMessage[100];
  sprintf(errorMessage, "-%c must be a %s integer.", flag,
          allowZero ? "non-negative" : "positive");
  int value = strtol(arg, &endptr, 10);
  if (value <= 0) {
    disaster(programName, errorMessage);
  }
  if (endptr == arg) {
    disaster(programName, errorMessage);
  }
  if (value == 0 && !allowZero) {
    disaster(programName, errorMessage);
  }
  if (*endptr != '\0') {
    disaster(programName, errorMessage);
  }
  return value;
}

static void initializeOutputFolder()
{
  initializeFolder(TargetFolderFlag);
}
