#ifndef GRAPH_H
#define GRAPH_H

#include "core.h"
#include "color.h"
#include "trail.h"
#include "point.h"
#include "statistics.h"
#include <stdint.h>



struct edge {
  STATIC FACE face;
  STATIC EDGE reversed;
  /*
  to starts off as NULL, and is set to the jth member of possiblyTo
  where (color, j) is in the facial cycle of inner, equivalently
  (j, color) is in the facial cycle of outer.
  */
  DYNAMIC DPOINT to;
  STATIC COLOR color;
  /* A value between 0 and NCOLORS, being the cardinality of face. */
  STATIC uint64_t level;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th entry
  being all NULL.
   */
  STATIC struct directedPoint possiblyTo[NCOLORS];
};
extern EDGE followEdgeBackwards(EDGE edge);
extern EDGE followEdgeForwards(EDGE edge);
extern FAILURE curveChecks(EDGE edge, int depth);
extern void findCorners(COLOR a, EDGE result[3][2]);
extern int pathLength(EDGE from, EDGE to);

struct face {
  // cycle must be null if cycleSetSize is not 1.
  DYNAMIC struct facial_cycle *cycle;
  DYNAMIC TRAIL backtrack;
  /* We point to previous and next with the same number of colors. */
  DYNAMIC FACE previous;
  DYNAMIC FACE next;
  STATIC COLORSET colors;           // holds up to NFACES
  DYNAMIC uint_trail cycleSetSize;  // holds up to NCYCLES
  DYNAMIC CYCLESET_DECLARE possibleCycles;
  STATIC struct face *adjacentFaces[NCOLORS];
  STATIC struct edge edges[NCOLORS];
  /* Precomputed lookup tables for previous and next. */
  STATIC FACE previousByCycleId[NCYCLES];
  STATIC FACE nextByCycleId[NCYCLES];
};


extern STATIC struct face g_faces[NFACES];
extern uint64_t g_edgeCount[2][NCOLORS];
extern uint64_t g_lengthOfCycleOfFaces[NCOLORS + 1];
extern uint64_t g_crossings[NCOLORS][NCOLORS];
extern uint64_t g_curveComplete[NCOLORS];
extern STATIC struct facial_cycle g_cycles[NCYCLES];
extern FAILURE makeChoice(FACE face);
extern FAILURE finalCorrectnessChecks(void);
extern void setupCentralFaces(int *faceDegrees);
extern void initializeGraph(void);
extern void initialize(void);

extern FAILURE assignPoint(FACE face, COLOR aColor, COLOR bColor, int depth);

#define IS_PRIMARY_EDGE(edge) \
  (memberOfColorSet((edge)->color, (edge)->face->colors))


#define FINAL_ENTRIES_IN_UNIVERSAL_CYCLE_SET \
  ((1ul << (NCYCLES % BITS_PER_WORD)) - 1ul)

#endif // GRAPH_H