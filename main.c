/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "main.h"

#include "memory.h"
#include "search.h"
#include "statistics.h"
#include "utils.h"

#include <getopt.h>
#include <stdlib.h>

static char *TargetFolder = NULL;

/* Declaration of file scoped static functions */
static void saveResult(void);
static void initializeOutputFolder(void);

/* Externally linked functions */
int dynamicMain0(int argc, char *argv[])
{
  int opt;
  TargetFolder = NULL;
  while ((opt = getopt(argc, argv, "f:")) != -1) {
    switch (opt) {
      case 'f':
        TargetFolder = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || TargetFolder == NULL) {
    fprintf(stderr, "Usage: %s -f outputFolder\n", argv[0]);
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
