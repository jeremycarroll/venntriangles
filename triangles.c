
#include <stddef.h>

#include "d6.h"
#include "face.h"
#if NCOLORS == 4
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define MAX_ONE_WAY_CURVE_CROSSINGS 3

/*
This file is responsible for checking that a set of edges can make a triangle.
*/
extern COLORSET DynamicColorCompleted;
static uint_trail curveLength(EDGE edge)
{
  EDGE current;
  int result;
  for (result = 1, current = edgeFollowForwards(edge); current != edge;
       result++, current = edgeFollowForwards(current)) {
    assert(current != NULL);
  }
  return result;
}

static FAILURE checkForDisconnectedCurve(EDGE edge, int depth)
{
  uint_trail length;
  // TODO: needs named macro!
  if (edge->reversed->to != NULL) {
    // We have a colored cycle in the FISC.
    length = curveLength(edge);
    if (length <
        EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]) {
      return failureDisconnectedCurve(depth);
    }
    assert(length ==
           EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color]);
    if (DynamicColorCompleted & 1u << edge->color) {
      return NULL;
    }
    DynamicColorCompleted |= 1u << edge->color;
    trailSetInt(&EdgeCurvesComplete[edge->color], 1);
  }
  return NULL;
}
