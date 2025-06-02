/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "utils.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#define USAGE_ONE_LINE                                                   \
  "Usage: %s -f outputFolder [-d centralFaceDegrees] [-m maxSolutions] " \
  "[-n maxVariantsPerSolution] [-k skipFirstSolutions] [-j "             \
  "skipFirstVariantsPerSolution] [-v]\n"

#define USAGE_WITH_D_EXPLANATION                                              \
  "When -d is specified, -m and -k apply to solutions with that face degree " \
  "pattern.\n"                                                                \
  "Otherwise, they apply globally across all face degree patterns.\n"         \
  "Use -v to enable verbose output mode.\n"

/**
 * Creates or validates an output folder for storing results.
 * If the folder doesn't exist, creates it with appropriate permissions.
 * If it exists, verifies that it is writable.
 */
void initializeFolder(const char *folder)
{
  struct stat st = {0};

  if (stat(folder, &st) == -1) {
    /* Directory does not exist, create it */
    if (mkdir(folder, 0700) != 0) {
      perror("Failed to create directory");
      exit(EXIT_FAILURE);
    }
  } else {
    /* Directory exists, check if it is writable */
    if (!S_ISDIR(st.st_mode) || access(folder, W_OK) != 0) {
      fprintf(stderr, "Target folder exists but is not writable\n");
      exit(EXIT_FAILURE);
    }
  }
}

/**
 * Handles fatal errors by displaying usage information and the specific error
 * message. Exits the program with a failure status.
 */
void disaster(const char *programName, const char *message)
{
  fprintf(stderr, USAGE_ONE_LINE, programName);
  fprintf(stderr, USAGE_WITH_D_EXPLANATION);
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}
