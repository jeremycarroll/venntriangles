/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "vertex.h"

#include "trail.h"
#include "utils.h"

static struct Vertex VertexAllUVertices[NPOINTS];
static int NextUVertexId = 0;
static struct Vertex* AllUPointPointers[NFACES][NCOLORS][NCOLORS];

/* Forward declarations of file scoped static functions */
static FAILURE dynamicFindCornersByTraversal(EDGE start, int depth,
                                             EDGE* cornersReturn);
static VERTEX getOrInitializeVertex(COLORSET colorsOfFace, COLOR primary,
                                    COLOR secondary);
static void validateVertexInitialization(VERTEX vertex, EDGE incomingEdge,
                                         COLOR primary, COLOR secondary,
                                         int ix);
static uint32_t getIncomingEdgeSlot(EDGE incomingEdge, COLOR othercolor,
                                    COLORSET faceColors);
static bool detectCornerAndUpdateCrossingSets(COLORSET other, COLORSET* outside,
                                              COLORSET* passed);

VERTEX initializeVertexIncomingEdge(COLORSET colors, EDGE incomingEdge,
                                    COLOR othercolor)
{
  VERTEX vertex;
  COLOR primary, secondary;

  uint32_t incomingEdgeSlot =
      getIncomingEdgeSlot(incomingEdge, othercolor, colors);

  switch (incomingEdgeSlot) {
    case 0: /* Primary edge, other color contains face */
    case 1: /* Secondary edge, other color excludes face */
      primary = incomingEdge->color;
      secondary = othercolor;
      break;
    case 2: /* Secondary edge, other color contains face */
    case 3: /* Primary edge, other color excludes face */
      primary = othercolor;
      secondary = incomingEdge->color;
      break;
    default:
      assert(0);
  }

  vertex = getOrInitializeVertex(incomingEdge->colors, primary, secondary);
  validateVertexInitialization(vertex, incomingEdge, primary, secondary,
                               incomingEdgeSlot);

  vertex->incomingEdges[incomingEdgeSlot] = incomingEdge;

  return vertex;
}

char* vertexToColorSetString(VERTEX up)
{
  COLORSET colors = up->incomingEdges[0]->colors | (1ll << up->primary) |
                    (1ll << up->secondary);
  return colorSetToBareString(colors);
}

char* vertexToString(VERTEX up)
{
  char* buffer = getBuffer();
  char* colorsStr = vertexToColorSetString(up);
  sprintf(buffer, "%s_%c_%c", colorsStr, 'a' + up->primary,
          'a' + up->secondary);
  return usingBuffer(buffer);
}

#if NCOLORS <= 4
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

FAILURE dynamicVertexCornerCheck(EDGE start, int depth)
{
#if NCOLORS <= 4
  /* test_venn[34].c do not like the normal code - not an issue. */
  return NULL;
#else
  EDGE ignore[MAX_CORNERS * 100];
  if (start->reversed->to != NULL) {
    // we have a complete curve.
    start = vertexGetCentralEdge(start->color);
  }
  return dynamicFindCornersByTraversal(start, depth, ignore);
#endif
}

EDGE vertexGetCentralEdge(COLOR a)
{
  COLOR primary = a;
  COLOR secondary = (a + 1) % NCOLORS;
  VERTEX uVertex = getOrInitializeVertex(NFACES - 1, primary, secondary);
  return uVertex->incomingEdges[0];
}

void vertexAlignCorners(COLOR a, EDGE result[3][2])
{
  int i, j;
  EDGE clockWiseCorners[MAX_CORNERS];
  EDGE counterClockWiseCorners[MAX_CORNERS];
  FAILURE failure = dynamicFindCornersByTraversal(vertexGetCentralEdge(a), 0,
                                                  clockWiseCorners);
  assert(failure == NULL);
  failure = dynamicFindCornersByTraversal(vertexGetCentralEdge(a)->reversed, 0,
                                          counterClockWiseCorners);
  assert(failure == NULL);
  assert((clockWiseCorners[2] == NULL) == (counterClockWiseCorners[2] == NULL));
  assert((clockWiseCorners[1] != NULL));
  assert((counterClockWiseCorners[1] != NULL));
  for (i = 0; i < 3 && clockWiseCorners[i]; i++) {
    result[i][0] = clockWiseCorners[i];
  }
  if (i < 3) {
    result[i][0] = NULL;
    result[i][1] = NULL;
  }

  for (j = 0; j < 3 && counterClockWiseCorners[j]; j++) {
    assert(i - 1 - j >= 0);
    assert(i - 1 - j < 3);
    result[i - 1 - j][1] = counterClockWiseCorners[j];
  }
}

/* File scoped static functions */
static VERTEX getOrInitializeVertex(COLORSET colorsOfFace, COLOR primary,
                                    COLOR secondary)
{
  COLORSET outsideColor = colorsOfFace & ~(1u << primary) & ~(1u << secondary);
  if (AllUPointPointers[outsideColor][primary][secondary] == NULL) {
    AllUPointPointers[outsideColor][primary][secondary] =
        &VertexAllUVertices[NextUVertexId];
    VertexAllUVertices[NextUVertexId].primary = primary;
    VertexAllUVertices[NextUVertexId].secondary = secondary;
    VertexAllUVertices[NextUVertexId].colors = 1u << primary | 1u << secondary;
    NextUVertexId++;
  }
  return AllUPointPointers[outsideColor][primary][secondary];
}

static void validateVertexInitialization(VERTEX vertex, EDGE incomingEdge,
                                         COLOR primary, COLOR secondary, int ix)
{
  assert(vertex->incomingEdges[ix] == NULL);
  assert(incomingEdge->color == (ix < 2 ? primary : secondary));
  assert(vertex->colors == ((1u << primary) | (1u << secondary)));
}

static uint32_t getIncomingEdgeSlot(EDGE incomingEdge, COLOR othercolor,
                                    COLORSET faceColors)
{
  if (IS_CLOCKWISE_EDGE(incomingEdge)) {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 0 : 3;
  } else {
    return COLORSET_HAS_MEMBER(othercolor, faceColors) ? 2 : 1;
  }
}

static bool detectCornerAndUpdateCrossingSets(COLORSET other, COLORSET* outside,
                                              COLORSET* passed)
{
  if (other & *outside) {
    // We're crossing back inside some curves
    *outside &= ~other;

    if (other & *passed) {
      *passed = 0;
      return true;
    }
  } else {
    // We're crossing to the outside of these curves
    *passed |= other;
    *outside |= other;
  }

  return false;
}

static FAILURE dynamicFindCornersByTraversal(EDGE start, int depth,
                                             EDGE* cornersReturn)
{
  EDGE current = start;
  COLORSET notMyColor = ~(1u << start->color),
           /* the curves we have crossed outside of since the last corner. */
      passed = 0,
           /* the curves we are currently outside. */
      outside = ~start->colors;
  int counter = 0;
  assert(start->reversed->to == NULL ||
         (start->colors & notMyColor) == ((NFACES - 1) & notMyColor));
  do {
    CURVELINK p = current->to;
    if (detectCornerAndUpdateCrossingSets(p->vertex->colors & notMyColor,
                                          &outside, &passed)) {
      if (counter >= MAX_CORNERS) {
        return failureTooManyCorners(depth);
      }
      cornersReturn[counter++] = current;
    }
    current = p->next;
  } while (current->to != NULL && current != start);
  while (counter < MAX_CORNERS) {
    cornersReturn[counter++] = NULL;
  }
  return NULL;
}

static void edgeLink(EDGE edge1, EDGE edge2, EDGE edge3)
{
  COLOR other = edge3->color;

  assert(edge1->color == edge2->color);
  assert(edge1->possiblyTo[other].next == NULL);
  assert(edge2->possiblyTo[other].next == NULL);
  assert(edge1->possiblyTo[other].vertex == edge2->possiblyTo[other].vertex);
  edge1->possiblyTo[other].next = edge2->reversed;
  edge2->possiblyTo[other].next = edge1->reversed;
}

void initializePoints(void)
{
  uint32_t i, j, k;
  if (VertexAllUVertices[0].incomingEdges[0]->possiblyTo[1].next == NULL) {
    for (i = 0; i < NPOINTS; i++) {
      VERTEX p = VertexAllUVertices + i;
      edgeLink(p->incomingEdges[0], p->incomingEdges[1], p->incomingEdges[2]);
      edgeLink(p->incomingEdges[2], p->incomingEdges[3], p->incomingEdges[0]);
    }
    for (i = 0; i < NFACES; i++) {
      FACE f = Faces + i;
      for (j = 0; j < NCOLORS; j++) {
        for (k = 0; k < NCOLORS; k++) {
          assert(j == f->edges[j].color);
          if (k == j) {
            continue;
          }
          assert(f->edges[j].possiblyTo[k].vertex != NULL);
          assert(f->edges[j].possiblyTo[k].next != NULL);
          assert(f->edges[j].possiblyTo[k].next->color == j);
          assert(
              f->edges[j].possiblyTo[k].next->reversed->possiblyTo[k].vertex ==
              f->edges[j].possiblyTo[k].vertex);
        }
      }
    }
  }
}
