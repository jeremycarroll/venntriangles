
#include "venn.h"

/*
 * Only a few of these are actually used, the look up table is easier this way.
 With n being a COLORSET, and i, j being COLORs
 * [n][i][j] is used if i != j, and the i and j are not in n.
 * other values are left as null
 */
static struct undirectedPoint* allUPointPointers[NFACES][NCURVES][NCURVES];
struct undirectedPoint allUPoints[NPOINTS];
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

/*
Set up the out values for edge1 and edge2 that have the same color.
All four edges meet at the same point.
The edge3 and edge4 have the other color.
The out[0] value for both  edge1 and edge2  for the other color is set to
the reverse of the other edge.
The out[1] value is set to the reverse of edge3 or edge4 maintaining the level.
*/
static void linkOut(EDGE edge1, EDGE edge2, EDGE edge3, EDGE edge4)
{
  COLOR other = edge3->color;
  uint32_t level1 = edge1->level;
  uint32_t level2 = edge2->level;
  uint32_t level3 = edge3->reversed->level;
  uint32_t level4 = edge4->reversed->level;

  assert(edge1->color == edge2->color);
  assert(edge1->possiblyTo[other].out[0] == NULL);
  assert(edge2->possiblyTo[other].out[0] == NULL);
  assert(edge1->possiblyTo[other].point == edge2->possiblyTo[other].point);
  assert(edge1->possiblyTo[other].point->colors =
             (1u << edge1->color | 1u << other));
  edge1->possiblyTo[other].out[0] = edge2->reversed;
  edge2->possiblyTo[other].out[0] = edge1->reversed;
  if (level1 == level3) {
    edge1->possiblyTo[other].out[1] = edge3->reversed;
    edge2->possiblyTo[other].out[1] = edge4->reversed;
    assert(level2 == level4);
  } else {
    edge1->possiblyTo[other].out[1] = edge4->reversed;
    edge2->possiblyTo[other].out[1] = edge3->reversed;
    assert(level1 == level4);
    assert(level2 == level3);
  }
}

/*
Set up out[0] on every possiblyTo.

It must be the same color, and the reverse of the other edge of that color at
the point.
*/
void initializePoints(void)
{
  uint32_t i, j, k;
  for (i = 0; i < NPOINTS; i++) {
    UPOINT p = allUPoints + i;
    linkOut(p->incomingEdges[0], p->incomingEdges[1], p->incomingEdges[2],
            p->incomingEdges[3]);
    linkOut(p->incomingEdges[2], p->incomingEdges[3], p->incomingEdges[0],
            p->incomingEdges[1]);
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
  uint32_t ix, faceIx;

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

#if POINT_DEBUG
  char dbuffer[1024] = {1, 0, 0};
#endif
  assert(othercolor != incomingEdge->color);
  switch (ix) {
    case 0:
    case 1:
      a = incomingEdge->color;
      b = othercolor;
      break;
    case 2:
    case 3:
      a = othercolor;
      b = incomingEdge->color;
      break;
    default:
      assert(0);
  }
#if POINT_DEBUG
  printf("addToPoint(%s[%c,%c], f: %s, e: %s, r: %s, %c, %d)\n",
         colors2str(dbuffer, insideColor), color2char(dbuffer, a),
         color2char(dbuffer, b), face2str(dbuffer, face),
         edge2str(dbuffer, incomingEdge),
         edge2str(dbuffer, incomingEdge->reversed),
         color2char(dbuffer, othercolor), ix);
#endif

  point = getPoint(insideColor, a, b);
  assert(point->incomingEdges[ix] == NULL);
  assert(point->colors == 0 || point->colors == ((1u << a) | (1u << b)));
  assert(incomingEdge->color == (ix < 2 ? point->primary : point->secondary));
  point->incomingEdges[ix] = incomingEdge;

  if (face->colors & (1u << point->primary)) {
    if (face->colors & (1u << point->secondary)) {
      faceIx = 3;
    } else {
      faceIx = 1;
    }
  } else if (face->colors & (1u << point->secondary)) {
    faceIx = 2;
  } else {
    faceIx = 0;
  }
  assert(point->faces[faceIx] == NULL);
  point->faces[faceIx] = face;
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
  EDGE edge;
  COLOR colors[2];
  uint_trail* edgeCountPtr;

  if (face->edges[aColor].to != NULL) {
#if EDGE_DEBUG
    printf("Assigned edge %c %c: ", color2char(NULL, aColor),
           color2char(NULL, bColor));
    printEdge(&face->edges[aColor]);
#endif

    assert(face->edges[aColor].to != &face->edges[aColor].possiblyTo[aColor]);
    if (face->edges[aColor].to != &face->edges[aColor].possiblyTo[bColor]) {
      return pointConflictFailure(aColor, bColor, depth);
    }
    assert(face->edges[aColor].to == &face->edges[aColor].possiblyTo[bColor]);
    return NULL;
  }
  upoint = face->edges[aColor].possiblyTo[bColor].point;
  crossingLimit = checkCrossingLimit(upoint->primary, upoint->secondary, depth);
  if (crossingLimit != NULL) {
    return crossingLimit;
  }
  colors[0] = upoint->primary;
  colors[1] = upoint->secondary;
#if EDGE_DEBUG
  printf("Assigning edges:\n");
#endif
  for (int i = 0; i < 4; i++) {
    edge = upoint->incomingEdges[i];
    assert(edge->color == colors[(i & 2) >> 1]);
    assert(edge->color != colors[1 - ((i & 2) >> 1)]);
    if (edge->to != NULL) {
#if EDGE_DEBUG
      printf("Edge already assigned  %c %c: ", color2char(NULL, colors[0]),
             color2char(NULL, colors[1]));
      printEdge(edge);
#endif
      if (edge->to != &edge->possiblyTo[colors[(i & 2) >> 1]]) {
        return pointConflictFailure(edge->color, colors[(i & 2) >> 1], depth);
      }
      assert(edge->to == &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
    } else {
      setDynamicPointer(&edge->to,
                        &edge->possiblyTo[colors[1 - ((i & 2) >> 1)]]);
#if EDGE_DEBUG
      printEdge(edge);
#endif
    }

    assert(edge->to != &edge->possiblyTo[edge->color]);
    // Count edge
    edgeCountPtr = &g_edgeCount[IS_PRIMARY_EDGE(edge)][edge->color];
    setDynamicInt(edgeCountPtr, (*edgeCountPtr) + 1);
  }
  for (int i = 0; i < 4; i++) {
    assert(upoint->incomingEdges[i]->to != NULL);
  }
  return NULL;
}
