/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "graphml.h"

#include "memory.h"
#include "point.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#define DEBUG 0

/* GraphML namespace and schema definitions */
static const char *GRAPHML_NS = "http://graphml.graphdrawing.org/xmlns";
static const char *GRAPHML_SCHEMA =
    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd";

/* Forward declarations */
static char *cornerId(COLOR color, int counter);
static void graphmlAddCorner(FILE *fp, COLOR color, int counter);
static void addEdge(FILE *fp, COLOR color, char *source, char *target);
static EDGE *getPath(EDGE from, EDGE to);
static void saveVariation(EDGE (*corners)[3]);
static void chooseCornersThenSavePartialVariations(int cornerIndex,
                                                   EDGE (*cornerPairs)[2],
                                                   COLOR current,
                                                   EDGE (*corners)[3]);
static int savePartialVariations(COLOR current, EDGE (*corners)[3]);
char *graphmlPointId(POINT point);
char *graphmlCurveId(COLOR color);

struct graphmlFileIO graphmlFileOps = {fopen, initializeFolder};

/* Begin GraphML document */
static void graphmlBegin(FILE *fp)
{
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fp, "<graphml xmlns=\"%s\"\n", GRAPHML_NS);
  fprintf(fp,
          "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
  fprintf(fp, "         xsi:schemaLocation=\"%s %s\">\n", GRAPHML_NS,
          GRAPHML_SCHEMA);

  /* Define custom attributes */
  fprintf(fp,
          "  <key id=\"colors\" for=\"node\" attr.name=\"colors\" "
          "attr.type=\"string\"/>\n");
  fprintf(fp,
          "  <key id=\"primary\" for=\"node\" attr.name=\"primary\" "
          "attr.type=\"string\"/>\n");
  fprintf(fp,
          "  <key id=\"secondary\" for=\"node\" attr.name=\"secondary\" "
          "attr.type=\"string\"/>\n");
  fprintf(fp,
          "  <key id=\"color\" for=\"edge\" attr.name=\"color\" "
          "attr.type=\"string\"/>\n");

  fprintf(fp, "  <graph id=\"venn_diagram\" edgedefault=\"undirected\">\n");
}

/* End GraphML document */
static void graphmlEnd(FILE *fp)
{
  fprintf(fp, "  </graph>\n");
  fprintf(fp, "</graphml>\n");
}

/* Add a point (vertex) to the graph */
static void graphmlAddPoint(FILE *fp, POINT point)
{
  char *id = graphmlPointId(point);
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"colors\">%s</data>\n",
          colorSetToStr(point->colors));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", 'a' + point->primary);
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n",
          'a' + point->secondary);
  fprintf(fp, "    </node>\n");
}

/* Return a string ID for a corner */
static char *cornerId(COLOR color, int counter)
{
  char *buffer = getBuffer();
  sprintf(buffer, "%c-%d", colorToChar(color), counter);
  return usingBuffer(buffer);
}

/* Add a corner point to the graph */
static void graphmlAddCorner(FILE *fp, COLOR color, int counter)
{
  char *id = cornerId(color, counter);
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", colorToChar(color));
  fprintf(fp, "    </node>\n");
}

/* Add an edge between two points */
static void addEdge(FILE *fp, COLOR color, char *source, char *target)
{
  fprintf(fp, "    <edge source=\"%s\" target=\"%s\">\n", source, target);
  fprintf(fp, "      <data key=\"color\">%c</data>\n", colorToChar(color));
  fprintf(fp, "    </edge>\n");
}

/* Add an edge to the graph */
static void graphmlAddEdge(FILE *fp, EDGE edge)
{
  /* Use the primary edge for consistent ID generation */
  if (!IS_PRIMARY_EDGE(edge)) {
    edge = edge->reversed;
  }
  char *source = graphmlPointId(edge->reversed->to->point);
  char *target = graphmlPointId(edge->to->point);
  addEdge(fp, edge->color, source, target);
}

static void addEdgeToCorner(FILE *fp, EDGE edge, int corner)
{
  char *source = graphmlPointId(edge->reversed->to->point);
  char *target = cornerId(edge->color, corner);
  addEdge(fp, edge->color, source, target);
}

static void addEdgeBetweenCorners(FILE *fp, COLOR color, int low, int high)
{
  char *source = cornerId(color, low);
  char *target = cornerId(color, high);
  addEdge(fp, color, source, target);
}

static void addEdgeFromCorner(FILE *fp, int corner, EDGE edge)
{
  char *source = cornerId(edge->color, corner);
  char *target = graphmlPointId(edge->to->point);
  addEdge(fp, edge->color, source, target);
}

/* Generate unique IDs for graph elements */
char *graphmlPointId(POINT point)
{
  char *buffer = getBuffer();
  sprintf(buffer, "p_%s_%c_%c", colorSetToStr(point->colors),
          'a' + point->primary, 'a' + point->secondary);
  return usingBuffer(buffer);
}

/* Generate a curve ID */
char *graphmlCurveId(COLOR color)
{
  char *buffer = getBuffer();
  sprintf(buffer, "c_%c", 'a' + color);
  return usingBuffer(buffer);
}

static int savePartialVariations(COLOR current, EDGE (*corners)[3]);
static const char *CurrentPrefix = NULL;
static int VariationNumber = 1;
static int ExpectedVariations = 0;
/**
 * @brief Find and save all variations of the solution to graphml files
 *
 * @param prefix The folder and prefix for files used in the solutions.
 */
void graphmlSaveAllVariations(const char *prefix, int expectedVariations)
{
  EDGE corners[NCOLORS][3];
  CurrentPrefix = prefix;
  VariationNumber = 1;
  ExpectedVariations = expectedVariations;
  graphmlFileOps.initializeFolder(prefix);
  savePartialVariations(0, corners);
}

static int edgeIsCorner(EDGE edge, EDGE (*corners)[3], int *low, int *high)
{
  int result = 0;
  int ix;
  *low = -1;
  *high = -1;
  for (ix = 0; ix < 3; ix++) {
    if ((*corners)[ix] == edge) {
      result |= 1 << ix;
      if (*low) {
        *high = ix;
      } else {
        *low = ix;
      }
    }
  }
  return result;
}

static void saveTriangle(FILE *fp, COLOR color, EDGE (*corners)[3])
{
  EDGE edge;
  EDGE *path;
  EDGE current;
  int ix;
  int low, high;
  graphmlAddCorner(fp, color, 0);
  graphmlAddCorner(fp, color, 1);
  graphmlAddCorner(fp, color, 2);
  edge = edgeOnCentralFace(color);
  path = getPath(edge, edgeFollowBackwards(edge));
  for (ix = 0; path[ix] != NULL; ix++) {
    current = path[ix];
    switch (edgeIsCorner(current, corners, &low, &high)) {
      case 0:
        graphmlAddEdge(fp, current);
        break;
      case 1:
      case 2:
      case 4:
        addEdgeToCorner(fp, current, low);
        addEdgeFromCorner(fp, low, current);
        break;
      case 3:
      case 5:
      case 6:
        addEdgeToCorner(fp, current, low);
        addEdgeBetweenCorners(fp, color, low, high);
        addEdgeFromCorner(fp, high, current);
        break;
      case 7:
        addEdgeToCorner(fp, current, 0);
        addEdgeBetweenCorners(fp, color, 0, 1);
        addEdgeBetweenCorners(fp, color, 1, 2);
        addEdgeFromCorner(fp, 2, current);
        break;
    }
    graphmlAddPoint(fp, current->to->point);
  }
}

static void saveVariation(EDGE (*corners)[3])
{
  COLOR a;
  char *filename = getBuffer();

  FILE *fp;
  sprintf(filename, "%s/%7.7d.txt", CurrentPrefix, VariationNumber++);
  assert(VariationNumber <= ExpectedVariations);
  fp = graphmlFileOps.fopen(filename, "w");
  graphmlBegin(fp);
  for (a = 0; a < NCOLORS; a++, corners++) {
    saveTriangle(fp, a, corners);
  }
  graphmlEnd(fp);
  fclose(fp);
}

/* Get path between two edges */
static EDGE *getPath(EDGE from, EDGE to)
{
  int length = edgePathLength(from, to, NULL);
#if DEBUG
  printf("getPath: %c %x -> %x %d\n", 'A' + from->color, from, to, length);
#endif
  EDGE *path = (EDGE *)NEW_ARRAY(EDGE, length + 1);
  assert(path != NULL);

  edgePathLength(from, to, path);
  path[length] = NULL;
  return path;
}

static EDGE *possibleCorners(COLOR color, EDGE from, EDGE to)
{
  if (from == NULL) {
    EDGE edge = edgeOnCentralFace(color);
    return getPath(edge, edgeFollowBackwards(edge));
  }
  return getPath(from->reversed, to);
}

static int savePartialVariations(COLOR current, EDGE (*corners)[3]);

void chooseCornersWithContinuation(int cornerIndex, EDGE (*cornerPairs)[2],
                                   COLOR current, EDGE (*corners)[3],
                                   int (*continuation)(COLOR,
                                                       EDGE (*corners)[3]))
{
  EDGE *possibilities;
  int i, total;

  for (int i = 0; i < 3; i++) {
#if DEBUG
    printf("%d cornerPairs[%d][0]: %x\n", cornerIndex, i,
           cornerPairs[i][0]->reversed);
    printf("%d cornerPairs[%d][1]: %x\n", cornerIndex, i, cornerPairs[i][1]);
#endif
  }

  if (cornerIndex == 3) {
    continuation(current + 1, corners);
    return;
  }
  possibilities = possibleCorners(current, cornerPairs[cornerIndex][0],
                                  cornerPairs[cornerIndex][1]);
  for (i = 0; possibilities[i] != NULL; i++);
#if DEBUG
  printf("!! cornerIndex: %d, current: %d, possibilities: %d\n", cornerIndex,
         current, i);
#endif
  total = i;
  for (i = 0; possibilities[i] != NULL; i++) {
    corners[current][cornerIndex] = possibilities[i];
#if DEBUG
    printf("cornerIndex: %d, current: %d, possibilities[i]: %s %d/%d\n",
           cornerIndex, current, edgeToStr(possibilities[i]), i, total);
#endif
    chooseCornersWithContinuation(cornerIndex + 1, cornerPairs, current,
                                  corners, continuation);
  }
}

static void chooseCornersThenSavePartialVariations(int cornerIndex,
                                                   EDGE (*cornerPairs)[2],
                                                   COLOR current,
                                                   EDGE (*corners)[3])
{
  chooseCornersWithContinuation(cornerIndex, cornerPairs, current, corners,
                                savePartialVariations);
}

/* Save partial variations of the solution */
static int savePartialVariations(COLOR current, EDGE (*corners)[3])
{
  if (current >= NCOLORS) {
    saveVariation(corners);
    return 1;
  }

  EDGE cornerPairs[3][2];
  edgeFindCorners(current, cornerPairs);
  chooseCornersThenSavePartialVariations(0, cornerPairs, current, corners);
  return 0;
}
