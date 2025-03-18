#ifndef _EDGE_H
#define _EDGE_H

#include "types.h"

/* Edge-related structures */
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
  /* A value between 0 and NCOLORS, being the cardinaltiy of face. */
  STATIC uint64_t level;
  /* This point at the end of this edge may cross one of the other colors.
  We have all 5 pre-initialized in this array, with the color-th enty
  being all NULL.
   */
  STATIC struct directedPoint possiblyTo[NCOLORS];
};

/* Edge-related function declarations */
void printEdge(EDGE edge);
char *edge2str(char *dbuffer, EDGE edge);

#endif /* _EDGE_H */