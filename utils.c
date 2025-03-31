/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "utils.h"

#include "color.h"
#include "edge.h"
#include "face.h"
#include "memory.h"
#include "point.h"
#include "statistics.h"
#include "trail.h"

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
}
