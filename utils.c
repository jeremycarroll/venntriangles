#include "utils.h"

#include "color.h"
#include "edge.h"
#include "face.h"
#include "point.h"
#include "statistics.h"

void resetGlobals()
{
  resetFaces();
  resetEdges();
  resetPoints();
}

void resetInitialize() { resetCycles(); }

void initialize()
{
  /* Not true on all architectures, but assumed in our Trail. */
  assert((sizeof(uint64_t) == sizeof(void *)));

  initializeCycleSets();
  initializeFacesAndEdges();
  initializePoints();
  initializeDynamicCounters();
}
