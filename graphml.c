/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "graphml.h"

#include "main.h"
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
static void graphmlAddCorner(FILE *fp, EDGE edge, COLOR color, int counter);
static void addEdge(FILE *fp, COLOR color, int line, char *source,
                    char *target);
static void getPath(EDGE *path, EDGE from, EDGE to);
static void saveVariation(EDGE (*corners)[3]);
static void chooseCornersThenSavePartialVariations(int cornerIndex,
                                                   COLOR current,
                                                   EDGE (*corners)[3]);
static int savePartialVariations(COLOR current, EDGE (*corners)[3]);
static char *graphmlPointId(POINT point);
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
  fprintf(fp,
          "  <key id=\"line\" for=\"edge\" attr.name=\"line\" "
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
          pointToColorSetString(point));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", 'a' + point->primary);
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n",
          'a' + point->secondary);
  fprintf(fp, "    </node>\n");
}

/* Return a string ID for a corner */
static char *cornerId(COLOR color, int counter)
{
  char *buffer = getBuffer();
  assert(counter < 3);
  assert(color >= 0);
  sprintf(buffer, "%c_%d", colorToChar(color), counter);
  return usingBuffer(buffer);
}

/* Add a corner point to the graph */
static void graphmlAddCorner(FILE *fp, EDGE edge, COLOR color, int counter)
{
  COLORSET colors = edge->colors | (1ll << color);
  char *id = cornerId(color, counter);
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"colors\">%s</data>\n",
          colorSetToBareString(colors));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", colorToChar(color));
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n", colorToChar(color));
  fprintf(fp, "    </node>\n");
}

/* Add an edge between two points */
static void addEdge(FILE *fp, COLOR color, int line, char *source, char *target)
{
  fprintf(fp, "    <edge source=\"%s\" target=\"%s\">\n", source, target);
  fprintf(fp, "      <data key=\"color\">%c</data>\n", colorToChar(color));
  fprintf(fp, "      <data key=\"line\">%c%d</data>\n", colorToChar(color),
          line);
  fprintf(fp, "    </edge>\n");
}

/* Add an edge to the graph */
static void graphmlAddEdge(FILE *fp, EDGE edge, int line)
{
  /* Use the primary edge for consistent ID generation */
  if (!IS_PRIMARY_EDGE(edge)) {
    edge = edge->reversed;
  }
  char *source = graphmlPointId(edge->reversed->to->point);
  char *target = graphmlPointId(edge->to->point);
  addEdge(fp, edge->color, line, source, target);
}

static void addEdgeToCorner(FILE *fp, EDGE edge, int corner, int line)
{
  char *source = graphmlPointId(edge->reversed->to->point);
  char *target = cornerId(edge->color, corner);
  assert(line != corner);
  addEdge(fp, edge->color, line, source, target);
}

static void addEdgeBetweenCorners(FILE *fp, COLOR color, int low, int high)
{
  char *source = cornerId(color, low);
  char *target = cornerId(color, high);
  int line = 3 - high - low;
  addEdge(fp, color, line, source, target);
}

static void addEdgeFromCorner(FILE *fp, int corner, EDGE edge, int line)
{
  char *source = cornerId(edge->color, corner);
  char *target = graphmlPointId(edge->to->point);
  assert(line != corner);
  addEdge(fp, edge->color, line, source, target);
}

/* Generate unique IDs for graph elements */
static char *graphmlPointId(POINT point)
{
  char *buffer = getBuffer();
  sprintf(buffer, "p_%s", pointToStr(point));
  return usingBuffer(buffer);
}

static int savePartialVariations(COLOR current, EDGE (*corners)[3]);
static const char *CurrentPrefix = NULL;
static int VariationNumber = 1;
static int ExpectedVariations = 0;
static int Levels = 0;
static int numberOfLevels(int expectedVariations)
{
  int result = 1;
  while (expectedVariations >= 4096) {
    result++;
    expectedVariations /= 256;
  }
  return result;
}
static void possibleCorners(EDGE *possibilities, COLOR color, EDGE from,
                            EDGE to);
static EDGE PossibileCorners[NCOLORS][3][NFACES];

void graphmlPossibleCorners(void)
{
  for (int current = 0; current < NCOLORS; current++) {
    EDGE cornerPairs[3][2];
    edgeFindCorners(current, cornerPairs);
    for (int cornerIndex = 0; cornerIndex < 3; cornerIndex++) {
      possibleCorners(PossibileCorners[current][cornerIndex], current,
                      cornerPairs[cornerIndex][0], cornerPairs[cornerIndex][1]);
    }
  }
}
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
  Levels = numberOfLevels(expectedVariations);
  graphmlFileOps.initializeFolder(prefix);
  graphmlPossibleCorners();
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
      if (*low != -1) {
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
  EDGE path[NFACES];
  EDGE current;
  POINT currentPoint;
  int ix;
  int low, high;
  int cornerIds[3] = {-1, -1, -1};
  int cornerIx = 0;
  edge = edgeOnCentralFace(color);
  getPath(path, edge, edgeFollowBackwards(edge));
  int line = 0;
  for (ix = 0; path[ix] != NULL; ix++) {
    current = path[ix];
    switch (edgeIsCorner(current->reversed, corners, &low, &high)) {
      case 0:
        graphmlAddEdge(fp, current, line);
        break;
      case 1:
      case 2:
      case 4:
        cornerIds[cornerIx] = line == 0 ? 2 : line == 1 ? 0 : 1;
        addEdgeToCorner(fp, current, cornerIds[cornerIx], line);
        line = (line + 1) % 3;
        addEdgeFromCorner(fp, cornerIds[cornerIx], current, line);
        cornerIx++;
        break;
      case 3:
      case 5:
      case 6:
        assert(cornerIx < 2);
        assert(line < 2);
        cornerIds[cornerIx + 1] = line;
        cornerIds[cornerIx] = line == 0 ? 2 : 0;
        addEdgeToCorner(fp, current, cornerIds[cornerIx], line);
        line = (line + 1) % 3;
        addEdgeBetweenCorners(fp, color, cornerIds[cornerIx],
                              cornerIds[cornerIx + 1]);
        line = (line + 1) % 3;
        addEdgeFromCorner(fp, cornerIds[cornerIx + 1], current, line);
        cornerIx += 2;
        break;
      case 7:
        assert(cornerIx == 0);
        assert(line == 0);
        cornerIds[cornerIx++] = 0;
        cornerIds[cornerIx++] = 1;
        cornerIds[cornerIx++] = 2;
        addEdgeToCorner(fp, current, 0, 1);
        addEdgeBetweenCorners(fp, color, 0, 1);
        addEdgeBetweenCorners(fp, color, 1, 2);
        addEdgeFromCorner(fp, 2, current, 1);
        break;
    }
    currentPoint = current->to->point;
    if (currentPoint->primary == color) {
      graphmlAddPoint(fp, currentPoint);
    }
  }
  assert(cornerIx == 3);
  assert(line == 0);
  graphmlAddCorner(fp, (*corners)[0], color, cornerIds[0]);
  graphmlAddCorner(fp, (*corners)[1], color, cornerIds[1]);
  graphmlAddCorner(fp, (*corners)[2], color, cornerIds[2]);
}

static char *subFilename(void)
{
  char *buffer = getBuffer();
  int levels = Levels;
  char *p = buffer;
  int variationNumber = VariationNumber;
  p = p + sprintf(p, "%s", CurrentPrefix);

  while (levels > 1) {
    p += sprintf(p, "/%2.2x", variationNumber % 256);
    graphmlFileOps.initializeFolder(buffer);
    variationNumber /= 256;
    levels--;
  }
  p += sprintf(p, "/%3.3x.xml", variationNumber);
  return usingBuffer(buffer);
}

static void saveVariation(EDGE (*corners)[3])
{
  COLOR a;
  char *filename = subFilename();

  FILE *fp;
  assert(VariationNumber <= ExpectedVariations);
  VariationNumber++;
  if (VariationNumber - 1 <= IgnoreFirstVariantsPerSolution) {
    return;
  }
  fp = graphmlFileOps.fopen(filename, "w");
  graphmlBegin(fp);
  for (a = 0; a < NCOLORS; a++, corners++) {
    saveTriangle(fp, a, corners);
  }
  graphmlEnd(fp);
  fclose(fp);
  freeAll();
}

/* Get path between two edges */
static void getPath(EDGE *path, EDGE from, EDGE to)
{
  int length = edgePathLength(from, to, path);
#if DEBUG
  printf("getPath: %c %x -> %x %d\n", 'A' + from->color, from, to, length);
#endif
  assert(length > 0);
  assert(length == 1 || path[0] != path[length - 1]);
  path[length] = NULL;
}

static void possibleCorners(EDGE *possibilities, COLOR color, EDGE from,
                            EDGE to)
{
  if (from == NULL) {
    EDGE edge = edgeOnCentralFace(color);
    getPath(possibilities, edge->reversed, edgeFollowBackwards(edge->reversed));
  } else {
    getPath(possibilities, from->reversed, to);
  }
}

static int savePartialVariations(COLOR current, EDGE (*corners)[3]);

void graphmlChooseCornersWithContinuation(
    int cornerIndex, COLOR current, EDGE (*corners)[3],
    int (*continuation)(COLOR, EDGE (*corners)[3]))
{
  EDGE *possibilities = PossibileCorners[current][cornerIndex];
  int i;

  if (VariationNumber > MaxVariantsPerSolution) {
    return;
  }
  for (i = 0; i < 3; i++) {
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
  for (i = 0; possibilities[i] != NULL; i++);
#if DEBUG
  int total;
  printf("!! cornerIndex: %d, current: %d, possibilities: %d\n", cornerIndex,
         current, i);
#endif
  for (i = 0; possibilities[i] != NULL; i++) {
    corners[current][cornerIndex] = possibilities[i];
#if DEBUG
    printf("cornerIndex: %d, current: %d, possibilities[i]: %s %dn",
           cornerIndex, current, edgeToString(possibilities[i]), i);
#endif
    graphmlChooseCornersWithContinuation(cornerIndex + 1, current, corners,
                                         continuation);
  }
}

static void chooseCornersThenSavePartialVariations(int cornerIndex,
                                                   COLOR current,
                                                   EDGE (*corners)[3])
{
  graphmlChooseCornersWithContinuation(cornerIndex, current, corners,
                                       savePartialVariations);
}

/* Save partial variations of the solution */
static int savePartialVariations(COLOR current, EDGE (*corners)[3])
{
  if (current >= NCOLORS) {
    saveVariation(corners);
    return 1;
  }
  chooseCornersThenSavePartialVariations(0, current, corners);
  return 0;
}
