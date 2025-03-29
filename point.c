#include "point.h"

#include "face.h"
#include "trail.h"

#include <stddef.h>

/*
 * Only a few of these are actually used, the look up table is easier this way.
 With n being a COLORSET, and i, j being COLORs
 * [n][i][j] is used if i != j, and the i and j are not in n.
 * other values are left as null
 */
struct undirectedPoint DynamicPointAllUPoints[NPOINTS];
static int nextUPointId = 0;
static struct undirectedPoint* allUPointPointers[NFACES][NCOLORS][NCOLORS];

void resetPoints()
{
  memset(allUPointPointers, 0, sizeof(allUPointPointers));
  memset(DynamicPointAllUPoints, 0, sizeof(DynamicPointAllUPoints));
  nextUPointId = 0;
}

UPOINT getPoint(COLORSET colorsOfFace, COLOR primary, COLOR secondary)
{
  COLORSET outsideColor = colorsOfFace & ~(1u << primary) & ~(1u << secondary);
  if (allUPointPointers[outsideColor][primary][secondary] == NULL) {
    DynamicPointAllUPoints[nextUPointId].id = nextUPointId;
    allUPointPointers[outsideColor][primary][secondary] =
        &DynamicPointAllUPoints[nextUPointId];
    DynamicPointAllUPoints[nextUPointId].primary = primary;
    DynamicPointAllUPoints[nextUPointId].secondary = secondary;
    DynamicPointAllUPoints[nextUPointId].colors =
        1u << primary | 1u << secondary;
    nextUPointId++;
  }
  return allUPointPointers[outsideColor][primary][secondary];
}

/*
   The curve colored A crosses from inside the curve colored B to outside it.
   The curve colored B crosses from outside the curve colored A to inside it.
*/
UPOINT dynamicPointAdd(FACE face, EDGE incomingEdge, COLOR othercolor)
{
  UPOINT point;
  COLOR primary, secondary;
  uint32_t ix;

  if (IS_PRIMARY_EDGE(incomingEdge)) {
    if (memberOfColorSet(othercolor, face->colors)) {
      ix = 0;
    } else {
      ix = 3;
    }
  } else {
    if (memberOfColorSet(othercolor, face->colors)) {
      ix = 2;
    } else {
      ix = 1;
    }
  }

  assert(othercolor != incomingEdge->color);
  switch (ix) {
    case 0:
    case 1:
      primary = incomingEdge->color;
      secondary = othercolor;
      break;
    case 2:
    case 3:
      primary = othercolor;
      secondary = incomingEdge->color;
      break;
    default:
      assert(0);
  }

  point = getPoint(incomingEdge->colors, primary, secondary);
  assert(point->incomingEdges[ix] == NULL);
  assert(point->colors == 0 ||
         point->colors == ((1u << primary) | (1u << secondary)));
  assert(incomingEdge->color == (ix < 2 ? primary : secondary));
  point->incomingEdges[ix] = incomingEdge;

  assert(point->colors == ((1u << primary) | (1u << secondary)));
  return point;
}

/*
TODO: rename vars , the A B problem ...
Either return the point, or return NULL and set the value of failureReturn.
*/
FAILURE dynamicFaceIncludePoint(FACE face, COLOR aColor, COLOR bColor,
                                int depth)
{
  FAILURE crossingLimit;
  UPOINT upoint;
  EDGE edge;
  COLOR colors[2];
  uint_trail* edgeCountPtr;

  if (face->edges[aColor].to != NULL) {
    assert(face->edges[aColor].to != &face->edges[aColor].possiblyTo[aColor]);
    if (face->edges[aColor].to != &face->edges[aColor].possiblyTo[bColor]) {
      return failurePointConflict(depth);
    }
    assert(face->edges[aColor].to == &face->edges[aColor].possiblyTo[bColor]);
    return NULL;
  }
  upoint = face->edges[aColor].possiblyTo[bColor].point;
  crossingLimit =
      dynamicEdgeCheckCrossingLimit(upoint->primary, upoint->secondary, depth);
  if (crossingLimit != NULL) {
    return crossingLimit;
  }
  colors[0] = upoint->primary;
  colors[1] = upoint->secondary;
  for (int i = 0; i < 4; i++) {
    edge = upoint->incomingEdges[i];
    assert(edge->color == colors[(i & 2) >> 1]);
    assert(edge->color != colors[1 - ((i & 2) >> 1)]);
    if (edge->to != NULL) {
      if (edge->to != &edge->possiblyTo[colors[(i & 2) >> 1]]) {
        return failurePointConflict(depth);
      }
      assert(edge->to == &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
    } else {
      setDynamicPointer(&edge->to,
                        &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
    }

    assert(edge->to != &edge->possiblyTo[edge->color]);
    // Count edge
    edgeCountPtr =
        &EdgeCountsByDirectionAndColor[IS_PRIMARY_EDGE(edge)][edge->color];
    trailSetInt(edgeCountPtr, (*edgeCountPtr) + 1);
  }
  for (int i = 0; i < 4; i++) {
    assert(upoint->incomingEdges[i]->to != NULL);
  }
  return NULL;
}

char* uPointToStr(char* dbuffer, UPOINT up)
{
  char* colors = colorSetToStr(dbuffer, up->colors);
  sprintf(dbuffer, "%s(%c,%c)", colors, 'a' + up->primary, 'a' + up->secondary);
  return dbuffer;
}

char* dynamicDPointToStr(char* dbuffer, DPOINT dp)
{
  return uPointToStr(dbuffer, dp->point);
}
