#ifndef EDGE_H
#define EDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "color.h"
#include "core.h"
#include "failure.h"
#include "point.h"
#include "trail.h"

typedef struct edge* EDGE;

struct edge {
  /* The label on the edge. */
  STATIC COLOR color;

  /* The label on the face of which this is an edge. */
  STATIC COLORSET colors;
  /* The edge going in the other direction, with the same color, attached to the
   * face with a label differing only by the one color.  */
  STATIC EDGE reversed;
  /*
  to starts off as NULL, and is set to the jth member of possiblyTo
  where (color, j) is in the facial cycle of inner, equivalently
  (j, color) is in the facial cycle of outer.
  */
  DYNAMIC DPOINT to;
  /* A value between 0 and NCOLORS, being the cardinality of face. */
  STATIC uint64_t level;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th entry
  being all NULL.
   */
  STATIC struct directedPoint possiblyTo[NCOLORS];
};

extern uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
extern uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
extern uint64_t EdgeCurvesComplete[NCOLORS];

extern EDGE edgeFollowBackwards(EDGE edge);
extern EDGE edgeFollowForwards(EDGE edge);
extern FAILURE dynamicEdgeCurveChecks(EDGE edge, int depth);
extern void edgeFindCorners(COLOR a, EDGE result[3][2]);
extern int edgePathLength(EDGE from, EDGE to);
extern char* edgeToStr(char* dbuffer, EDGE edge);

/* Solution output functions */
extern void dynamicSolutionWrite(char* prefix);
extern void dynamicSolutionPrint(FILE* fp);

#define IS_PRIMARY_EDGE(edge) (memberOfColorSet((edge)->color, (edge)->colors))

extern void resetEdges(void);

#endif  // EDGE_H
