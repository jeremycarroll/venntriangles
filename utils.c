/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "utils.h"

#include "color.h"
#include "edge.h"
#include "face.h"
#include "memory.h"
#include "point.h"
#include "s6.h"
#include "statistics.h"
#include "trail.h"
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>

/* Externally linked functions */
void resetGlobals()
{
  resetFaces();
  resetEdges();
  resetPoints();
  freeAll();
}

void resetInitialize() { resetCycles(); }

void initialize()
{
  /* Not true on all architectures, but assumed in our Trail. */
  assert((sizeof(uint64_t) == sizeof(void *)));

  freeAll();
  initializeCycleSets();
  initializeFacesAndEdges();
  initializePoints();
  initializeTrail();
  initializeMemory();
  initializeS6();
}

void initializeFolder(const char *folder)
{
  struct stat st = {0};

  if (stat(folder, &st) == -1) {
    // Directory does not exist, create it
    if (mkdir(folder, 0700) != 0) {
      perror("Failed to create directory");
      exit(EXIT_FAILURE);
    }
  } else {
    // Directory exists, check if it is writable
    if (!S_ISDIR(st.st_mode) || access(folder, W_OK) != 0) {
      fprintf(stderr, "Target folder exists but is not writable\n");
      exit(EXIT_FAILURE);
    }
  }
}

#define USAGE_ONE_LINE                                                   \
  "Usage: %s -f outputFolder [-d centralFaceDegrees] [-m maxSolutions] " \
  "[-n maxVariantsPerSolution] [-k skipFirstSolutions] [-j "             \
  "skipFirstVariantsPerSolution]\n"

#define USAGE_WITH_D_EXPLANATION                                              \
  "When -d is specified, -m and -k apply to solutions with that face degree " \
  "pattern.\n"                                                                \
  "Otherwise, they apply globally across all face degree patterns.\n"

void disaster(const char *message)
{
  fprintf(stderr, USAGE_ONE_LINE, Argv0);
  fprintf(stderr, USAGE_WITH_D_EXPLANATION);
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}
