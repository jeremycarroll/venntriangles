#include "utils.h"

#include "face.h"
#include "statistics.h"

void resetGlobals()
{
  memset(Faces, 0, sizeof(Faces));
  memset(EdgeCountsByDirectionAndColor, 0,
         sizeof(EdgeCountsByDirectionAndColor));
  memset(FaceSumOfFaceDegree, 0, sizeof(FaceSumOfFaceDegree));
  memset(EdgeCrossingCounts, 0, sizeof(EdgeCrossingCounts));
  memset(EdgeCurvesComplete, 0, sizeof(EdgeCurvesComplete));
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

void initializeDynamicCounters(void)
{
  statisticNew(&DynamicCycleGuessCounter, "?", "guesses");
  statisticNew(&cycleForcedCounter, "+", "forced");
  statisticNew(&cycleSetReducedCounter, "-", "reduced");
}
