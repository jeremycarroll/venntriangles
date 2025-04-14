/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"

#include "memory.h"
#include "statistics.h"
#include "utils.h"
#include "vsearch.h"

#include <getopt.h>
#include <limits.h>
#include <stdlib.h>

static char *TargetFolder = NULL;
int MaxVariantsPerSolution = INT_MAX;  // Maximum value means unlimited
int MaxSolutions = INT_MAX;            // Maximum value means unlimited

/* Declaration of file scoped static functions */
static void saveResult(void);
static void initializeOutputFolder(void);

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  while ((opt = getopt(argc, argv, "f:v:s:")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      case 'v':
        MaxVariantsPerSolution = atoi(optarg);
        if (MaxVariantsPerSolution <= 0) {
          fprintf(stderr,
                  "maxVariantsPerSolution must be a positive integer\n");
          return EXIT_FAILURE;
        }
        break;
      case 's':
        MaxSolutions = atoi(optarg);
        if (MaxSolutions <= 0) {
          fprintf(stderr, "maxSolutions must be a positive integer\n");
          return EXIT_FAILURE;
        }
        break;
      default:
        fprintf(stderr,
                "Usage: %s -f outputFolder [-v maxVariantsPerSolution] [-s "
                "maxSolutions]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || TargetFolder == NULL) {
    fprintf(stderr,
            "Usage: %s -f outputFolder [-v maxVariantsPerSolution] [-s "
            "maxSolutions]\n",
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
