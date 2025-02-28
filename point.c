
#include "venn.h"

/*
 * Only a few of these are actually used, the look up table is easier this way.
 With n being a COLORSET, and i, j being COLORs
 * [n][i][j] is used if i != j, and the i and j are not in n.
 * other values are left as null
 */
static struct undirectedPoint* allUPointPointers[NFACES][NCURVES][NCURVES];
static struct undirectedPoint allUPoints[NPOINTS];
static int nextUPointId = 0;

void clearPoints()
{
  memset(allUPointPointers, 0, sizeof(allUPointPointers));
  memset(allUPoints, 0, sizeof(allUPoints));
  nextUPointId = 0;
}

static UPOINT getPoint(COLORSET innerFace, COLOR a, COLOR b)
{
  assert(!memberOfColorSet(a, innerFace));
  assert(!memberOfColorSet(b, innerFace));
  if (allUPointPointers[innerFace][a][b] == NULL) {
    allUPoints[nextUPointId].id = nextUPointId;
    allUPointPointers[innerFace][a][b] = &allUPoints[nextUPointId];
    allUPoints[nextUPointId].primary = a;
    allUPoints[nextUPointId].secondary = b;
    allUPoints[nextUPointId].colors = 1u << a | 1u << b;
    nextUPointId++;
  }
  return allUPointPointers[innerFace][a][b];
}

void initializePoints(void)
{
  uint32_t i, j, k;
  for (i = 0; i < NPOINTS; i++) {
    UPOINT p = allUPoints + i;
    for (j = 0; j < 4; j++) {
      COLOR other = p->incomingEdges[1 - (1 & j)]->color;
      assert(other != p->incomingEdges[j]->color);
      p->incomingEdges[j]->possiblyTo[other].out[0] =
          p->incomingEdges[(j ^ 3) & 3]->reversed;
    }
  }
  for (i = 0; i < NFACES; i++) {
    FACE f = g_faces + i;
    for (j = 0; j < NCURVES; j++) {
      for (k = 0; k < NCURVES; k++) {
        assert(j == f->edges[j].color);
        if (k == j) {
          continue;
        }
        assert(f->edges[j].possiblyTo[k].point != NULL);
        assert(f->edges[j].possiblyTo[k].out[0] != NULL);
        assert(f->edges[j].possiblyTo[k].out[0]->color == j);
        assert(
            f->edges[j].possiblyTo[k].out[0]->reversed->possiblyTo[k].point ==
            f->edges[j].possiblyTo[k].point);
      }
    }
  }
}

/*
    The point is between the crossing of two curves, one colored A
    and the other colored B, A and B used in the comments below.

    The curve colored A crosses from inside the curve colored B to outside it.
    The curve colored B crosses from outside the curve colored A to inside it.
  */

/* Face A&B
 */
FACE point2insideFace(UPOINT point) { return point->faces[3]; }

/* Face ~(A&B)
 */
FACE point2outsideFace(UPOINT point) { return point->faces[0]; }

/*
Face (~A)&B

Both curves are going clockwise around the central face
of the FISC. Hence, both curves are going clockwise
around  point2insideFace(point). The face
that is outside A and inside B is the face that is clockwise
before the point.
*/
FACE point2incomingFace(UPOINT point) { return point->faces[2]; }

/*
Face A&~B
*/
FACE point2outgoingFace(UPOINT point) { return point->faces[1]; }

EDGE point2inside2outsideIncomingEdge(UPOINT point)
{
  return point->incomingEdges[0];
}

EDGE point2outside2insideIncomingEdge(UPOINT point)
{
  return point->incomingEdges[2];
}

// EDGE point2inside2outsideOutgoingEdge(POINT point) { return point->edges[1];
// }

// EDGE point2outside2insideOutgoingEdge(POINT point) { return point->edges[3];
// }

COLOR point2inside2outsideColor(UPOINT point)
{
  return point->incomingEdges[0]->color;
}

COLOR point2outside2insideColor(UPOINT point)
{
  return point->incomingEdges[2]->color;
}

EDGE followEdgeForwards(EDGE edge)
{
  if (edge->to == NULL) {
    return NULL;
  }
  return edge->to->out[0];
}

EDGE followEdgeBackwards(EDGE edge)
{
  EDGE reversedNext = followEdgeForwards(edge->reversed);
  return reversedNext == NULL ? NULL : reversedNext->reversed;
}

/*

   The curve colored A crosses from inside the curve colored B to outside it.
   The curve colored B crosses from outside the curve colored A to inside it.
*/

UPOINT addToPoint(FACE face, EDGE incomingEdge, COLOR othercolor)
{
  COLORSET insideColor = incomingEdge->face->colors & ~(1u << othercolor) &
                         ~(1u << incomingEdge->color);
  UPOINT point;
  COLOR a, b;
  uint32_t ix = (IS_PRIMARY_EDGE(incomingEdge) << 1) |
                memberOfColorSet(othercolor, face->colors);

#ifdef POINT_DEBUG
  char dbuffer[1024] = {1, 0, 0};
#endif
  assert(othercolor != incomingEdge->color);
  switch (ix) {
    case 0:
    case 3:
      a = incomingEdge->color;
      b = othercolor;
      break;
    case 1:
    case 2:
      a = othercolor;
      b = incomingEdge->color;
      break;
    default:
      assert(0);
  }
#ifdef POINT_DEBUG
  printf("addToPoint(%s[%c,%c], f: %s, e: %s, r: %s, %c, %d)\n",
         colors2str(dbuffer, insideColor), color2char(dbuffer, a),
         color2char(dbuffer, b), face2str(dbuffer, face),
         edge2str(dbuffer, incomingEdge),
         edge2str(dbuffer, incomingEdge->reversed),
         color2char(dbuffer, othercolor), ix);
#endif

  point = getPoint(insideColor, a, b);
  assert(point->incomingEdges[ix] == NULL);
  assert(point->faces[ix] == NULL);
  assert(point->colors == 0 || point->colors == ((1u << a) | (1u << b)));
  assert(incomingEdge->color == (ix < 2 ? point->primary : point->secondary));
  point->incomingEdges[ix] = incomingEdge;
  point->faces[ix] = face;
  assert(point->colors == ((1u << a) | (1u << b)));
  return point;
}

/*
TODO: rename vars , the A B problem ...
Either return the point, or return NULL and set the value of failureReturn.
*/
FAILURE assignPoint(FACE face, COLOR aColor, COLOR bColor, int depth)
{
  FAILURE crossingLimit;
  UPOINT upoint;
  COLOR colors[2];
  if (face->edges[aColor].to != NULL) {
    assert(face->edges[aColor].to == &face->edges[aColor].possiblyTo[bColor]);
    return NULL;
  }
  upoint = face->edges[aColor].possiblyTo[bColor].point;
  colors[0] = upoint->incomingEdges[0]->color;
  colors[1] = upoint->incomingEdges[1]->color;
  crossingLimit = checkCrossingLimit(colors[0], colors[1], depth);
  if (crossingLimit != NULL) {
    return crossingLimit;
  }
  for (int i = 0; i < 4; i++) {
    assert(upoint->incomingEdges[i]->to == NULL);
    assert(upoint->incomingEdges[i]->color == colors[(i & 1)]);
    setDynamicPointer(
        &upoint->incomingEdges[i]->to,
        &upoint->incomingEdges[i]->possiblyTo[colors[2 - (i & 1)]]);
  }
  return NULL;
}
