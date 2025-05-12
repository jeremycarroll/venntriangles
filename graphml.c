/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "graphml.h"

#include "corners.h"
#include "main.h"
#include "memory.h"
#include "trail.h"
#include "triangles.h"
#include "utils.h"
#include "vertex.h"

#include <stdio.h>
#include <string.h>
#define DEBUG 0

/* Global variables */
uint64_t GlobalVariantCount = 0;
/* GraphML namespace and schema definitions */
static const char *GRAPHML_NS = "http://graphml.graphdrawing.org/xmlns";
static const char *GRAPHML_SCHEMA =
    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd";

/* Forward declarations of file scoped static functions */
/* GraphML document structure functions */
static void graphmlBegin(FILE *fp);
static void graphmlEnd(FILE *fp);

/* Node/vertex functions */
static void graphmlAddVertex(FILE *fp, VERTEX vertex);
static void graphmlAddCorner(FILE *fp, EDGE edge, COLOR color, int counter);
static char *graphmlVertexId(VERTEX vertex);
static char *cornerId(COLOR color, int counter);
static void addVertexIfPrimary(FILE *fp, VERTEX vertex, COLOR color);
static void addCornerNodes(FILE *fp, EDGE (*corners)[3], COLOR color,
                           int *cornerIds);

/* Edge functions */
static void addEdge(FILE *fp, COLOR color, int line, char *source,
                    char *target);
static void graphmlAddEdge(FILE *fp, EDGE edge, int line);
static void addEdgeToCorner(FILE *fp, EDGE edge, int corner, int line);
static void addEdgeBetweenCorners(FILE *fp, COLOR color, int low, int high);
static void addEdgeFromCorner(FILE *fp, int corner, EDGE edge, int line);

/* Triangle traversal callbacks */
static void processRegularEdgeGraphML(void *data, EDGE current, int line);
static void processSingleCornerGraphML(void *data, EDGE current, int line);
static void processAdjacentCornersGraphML(void *data, EDGE current, int line);
static void processAllCornersGraphML(void *data, EDGE current, int line);
static void saveTriangle(FILE *fp, COLOR color, EDGE (*corners)[3]);

/* Path and corner functions */
static void getPath(EDGE *path, EDGE from, EDGE to);
static void possibleCorners(EDGE *possibilities, COLOR color, EDGE from,
                            EDGE to);

/* Variation handling */
static void saveVariation(EDGE (*corners)[3]);
static int numberOfLevels(int expectedVariations);
static char *subFilename(void);

/* Global variables */
static const char *CurrentPrefix = NULL;
int VariationNumber = 1;
static int ExpectedVariations = 0;
static int Levels = 0;
EDGE PossibileCorners[NCOLORS][3][NFACES];

/* Structure to hold data for GraphML output */
typedef struct {
  FILE *fp;
  int cornerIds[3];
  int cornerIx;
  COLOR color;
} GraphMLData;

/* Externally linked functions */
struct graphmlFileIO graphmlFileOps = {fopen, initializeFolder};

void graphmlPossibleCorners(void)
{
  for (int current = 0; current < NCOLORS; current++) {
    EDGE cornerPairs[3][2];
    edgeFindAndAlignCorners(current, cornerPairs);
    for (int cornerIndex = 0; cornerIndex < 3; cornerIndex++) {
      possibleCorners(PossibileCorners[current][cornerIndex], current,
                      cornerPairs[cornerIndex][0], cornerPairs[cornerIndex][1]);
    }
  }
}

int graphmlSaveAllVariations(const char *prefix, int expectedVariations)
{
  CurrentPrefix = prefix;
  VariationNumber = 1;
  ExpectedVariations = expectedVariations;
  Levels = numberOfLevels(expectedVariations);
  graphmlFileOps.initializeFolder(prefix);
  graphmlPossibleCorners();
  chooseCorners(saveVariation);
  return VariationNumber - 1;
}

/* GraphML document structure functions */
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

static void graphmlEnd(FILE *fp)
{
  fprintf(fp, "  </graph>\n");
  fprintf(fp, "</graphml>\n");
}

/* Node/vertex functions */
static void graphmlAddVertex(FILE *fp, VERTEX vertex)
{
  char *id = graphmlVertexId(vertex);
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"colors\">%s</data>\n",
          vertexToColorSetString(vertex));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", 'a' + vertex->primary);
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n",
          'a' + vertex->secondary);
  fprintf(fp, "    </node>\n");
}

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

static char *graphmlVertexId(VERTEX vertex)
{
  char *buffer = getBuffer();
  sprintf(buffer, "p_%s", vertexToString(vertex));
  return usingBuffer(buffer);
}

static char *cornerId(COLOR color, int counter)
{
  char *buffer = getBuffer();
  assert(counter < 3);
  assert(color >= 0);
  sprintf(buffer, "%c_%d", colorToChar(color), counter);
  return usingBuffer(buffer);
}

static void addVertexIfPrimary(FILE *fp, VERTEX vertex, COLOR color)
{
  if (vertex->primary == color) {
    graphmlAddVertex(fp, vertex);
  }
}

static void addCornerNodes(FILE *fp, EDGE (*corners)[3], COLOR color,
                           int *cornerIds)
{
  for (int i = 0; i < 3; i++) {
    graphmlAddCorner(fp, (*corners)[i], color, cornerIds[i]);
  }
}

/* Edge functions */
static void addEdge(FILE *fp, COLOR color, int line, char *source, char *target)
{
  fprintf(fp, "    <edge source=\"%s\" target=\"%s\">\n", source, target);
  fprintf(fp, "      <data key=\"color\">%c</data>\n", colorToChar(color));
  fprintf(fp, "      <data key=\"line\">%c%d</data>\n", colorToChar(color),
          line);
  fprintf(fp, "    </edge>\n");
}

static void graphmlAddEdge(FILE *fp, EDGE edge, int line)
{
  /* Use the primary edge for consistent ID generation */
  if (!IS_PRIMARY_EDGE(edge)) {
    edge = edge->reversed;
  }
  char *source = graphmlVertexId(edge->reversed->to->vertex);
  char *target = graphmlVertexId(edge->to->vertex);
  addEdge(fp, edge->color, line, source, target);
}

static void addEdgeToCorner(FILE *fp, EDGE edge, int corner, int line)
{
  char *source = graphmlVertexId(edge->reversed->to->vertex);
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
  char *target = graphmlVertexId(edge->to->vertex);
  assert(line != corner);
  addEdge(fp, edge->color, line, source, target);
}

/* Triangle traversal callbacks */
static void processRegularEdgeGraphML(void *data, EDGE current, int line)
{
  GraphMLData *gml = (GraphMLData *)data;
  graphmlAddEdge(gml->fp, current, line);
  addVertexIfPrimary(gml->fp, current->to->vertex, gml->color);
}

static void processSingleCornerGraphML(void *data, EDGE current, int line)
{
  GraphMLData *gml = (GraphMLData *)data;
  gml->cornerIds[gml->cornerIx] = line == 0 ? 2 : line == 1 ? 0 : 1;
  addEdgeToCorner(gml->fp, current, gml->cornerIds[gml->cornerIx], line);
  line = (line + 1) % 3;
  addEdgeFromCorner(gml->fp, gml->cornerIds[gml->cornerIx], current, line);
  gml->cornerIx++;
  addVertexIfPrimary(gml->fp, current->to->vertex, gml->color);
}

static void processAdjacentCornersGraphML(void *data, EDGE current, int line)
{
  GraphMLData *gml = (GraphMLData *)data;
  assert(gml->cornerIx < 2);
  assert(line < 2);
  gml->cornerIds[gml->cornerIx + 1] = line;
  gml->cornerIds[gml->cornerIx] = line == 0 ? 2 : 0;
  addEdgeToCorner(gml->fp, current, gml->cornerIds[gml->cornerIx], line);
  line = (line + 1) % 3;
  addEdgeBetweenCorners(gml->fp, gml->color, gml->cornerIds[gml->cornerIx],
                        gml->cornerIds[gml->cornerIx + 1]);
  line = (line + 1) % 3;
  addEdgeFromCorner(gml->fp, gml->cornerIds[gml->cornerIx + 1], current, line);
  gml->cornerIx += 2;
  addVertexIfPrimary(gml->fp, current->to->vertex, gml->color);
}

static void processAllCornersGraphML(void *data, EDGE current, int line)
{
  GraphMLData *gml = (GraphMLData *)data;
  (void)line; /* Unused parameter */
  assert(gml->cornerIx == 0);
  gml->cornerIds[gml->cornerIx++] = 0;
  gml->cornerIds[gml->cornerIx++] = 1;
  gml->cornerIds[gml->cornerIx++] = 2;
  addEdgeToCorner(gml->fp, current, 0, 1);
  addEdgeBetweenCorners(gml->fp, gml->color, 0, 1);
  addEdgeBetweenCorners(gml->fp, gml->color, 1, 2);
  addEdgeFromCorner(gml->fp, 2, current, 1);
  addVertexIfPrimary(gml->fp, current->to->vertex, gml->color);
}

static void saveTriangle(FILE *fp, COLOR color, EDGE (*corners)[3])
{
  GraphMLData gml = {
      .fp = fp, .cornerIds = {-1, -1, -1}, .cornerIx = 0, .color = color};

  TriangleTraversalCallbacks callbacks = {
      .processRegularEdge = processRegularEdgeGraphML,
      .processSingleCorner = processSingleCornerGraphML,
      .processAdjacentCorners = processAdjacentCornersGraphML,
      .processAllCorners = processAllCornersGraphML,
      .processVertex = NULL};

  triangleTraverse(color, corners, &callbacks, &gml);

  /* Verify we processed all three corners */
  assert(gml.cornerIx == 3);

  /* Add the corner nodes to the graph */
  addCornerNodes(fp, corners, color, gml.cornerIds);
}

/* Path and corner functions */
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

/* Variation handling */
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
  GlobalVariantCount++;
  fp = graphmlFileOps.fopen(filename, "w");
  graphmlBegin(fp);
  for (a = 0; a < NCOLORS; a++, corners++) {
    saveTriangle(fp, a, corners);
  }
  graphmlEnd(fp);
  fclose(fp);
  freeAll();
}

static int numberOfLevels(int expectedVariations)
{
  int result = 1;
  while (expectedVariations >= 4096) {
    result++;
    expectedVariations /= 256;
  }
  return result;
}
