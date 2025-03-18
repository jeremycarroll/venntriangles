#ifndef _POINT_H
#define _POINT_H

#include "edge.h"  /* For EDGE type */
#include "face.h"  /* For FACE type */
#include "types.h" /* For basic type definitions */

/* Point-related function declarations */
void clearPoints(void);
void initializePoints(void);
UPOINT addToPoint(FACE face, EDGE incomingEdge, COLOR othercolor);
FAILURE assignPoint(FACE face, COLOR aColor, COLOR bColor, int depth);
EDGE followEdgeBackwards(EDGE edge);
EDGE followEdgeForwards(EDGE edge);

#endif /* _POINT_H */