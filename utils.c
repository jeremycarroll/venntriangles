#include "utils.h"

#include "color.h"
#include "edge.h"
#include "face.h"
#include "point.h"
#include "statistics.h"
#include "trail.h"
#define TOTAL_BUFFER_SIZE 16384
#define BUFFER_SIZE 64

static uint64_t GetBufferCounter = 0;
static uint64_t MaxBufferSize = 0;

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
  initializeTrail();
  statisticNew(&GetBufferCounter, "@", "Buffers");
  statisticNew(&MaxBufferSize, "^", "MaxBuffer");
}

char *getBuffer()
{
  static char buffer[TOTAL_BUFFER_SIZE];
  static int bufferIndex = 0;
  GetBufferCounter++;
  if (bufferIndex >= TOTAL_BUFFER_SIZE) {
    bufferIndex = 0;
  }
  return &buffer[bufferIndex += BUFFER_SIZE];
}

char *usingBuffer(char *buffer)
{
  uint64_t length = strlen(buffer);
  if (length > MaxBufferSize) {
    MaxBufferSize = length;
  }
  assert(length < BUFFER_SIZE);
  return buffer;
}
