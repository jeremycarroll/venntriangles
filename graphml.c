/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "common.h"
#include "main.h"
#include "predicates.h"
#include "triangles.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#define DEBUG 0

/* Global variables */
uint64 GlobalVariantCountIPC = 0;
char CurrentPrefixIPC[1024];
int VariationNumberIPC = 1;
int LevelsIPC = 0;

struct graphmlFileIO GraphmlFileOps = {fopen, initializeFolder};

/* GraphML namespace and schema definitions */
static const char *GRAPHML_NS = "http://graphml.graphdrawing.org/xmlns";
static const char *GRAPHML_SCHEMA =
    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd";

/* Structure to hold data for GraphML output */
typedef struct {
  FILE *fp;
  int cornerIds[3];
  int cornerIx;
  COLOR color;
} GraphMLData;

/**
 * Generates a vertex ID for GraphML output.
 * Uses the format "p_<colorset>_<primary>_<secondary>"
 */
static char *graphmlVertexId(VERTEX vertex)
{
  char *buffer = getBuffer();
  sprintf(buffer, "p_%s", vertexToString(vertex));
  return usingBuffer(buffer);
}

/**
 * Generates a corner ID for GraphML output.
 * Uses the format "<color>_<counter>"
 */
static char *cornerId(COLOR color, int counter)
{
  char *buffer = getBuffer();
  assert(counter < 3);
  sprintf(buffer, "%c_%d", colorToChar(color), counter);
  return usingBuffer(buffer);
}

/**
 * Writes the beginning of a GraphML document, including XML declaration,
 * namespaces, and attribute definitions.
 */
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

/**
 * Writes the end of a GraphML document.
 */
static void graphmlEnd(FILE *fp)
{
  fprintf(fp, "  </graph>\n");
  fprintf(fp, "</graphml>\n");
}

/**
 * Adds a vertex to the GraphML output.
 */
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

/**
 * Adds a corner node to the GraphML output.
 */
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

/**
 * Adds a vertex to the GraphML output if it's a primary vertex for the given
 * color.
 */
static void addVertexIfPrimary(FILE *fp, VERTEX vertex, COLOR color)
{
  if (vertex->primary == color) {
    graphmlAddVertex(fp, vertex);
  }
}

/**
 * Adds corner nodes to the GraphML output.
 */
static void addCornerNodes(FILE *fp, EDGE (*corners)[3], COLOR color,
                           int *cornerIds)
{
  for (int i = 0; i < 3; i++) {
    graphmlAddCorner(fp, (*corners)[i], color, cornerIds[i]);
  }
}

/**
 * Adds an edge to the GraphML output.
 */
static void addEdge(FILE *fp, COLOR color, int line, char *source, char *target)
{
  fprintf(fp, "    <edge source=\"%s\" target=\"%s\">\n", source, target);
  fprintf(fp, "      <data key=\"color\">%c</data>\n", colorToChar(color));
  fprintf(fp, "      <data key=\"line\">%c%d</data>\n", colorToChar(color),
          line);
  fprintf(fp, "    </edge>\n");
}

/**
 * Adds a regular edge to the GraphML output.
 */
static void graphmlAddEdge(FILE *fp, EDGE edge, int line)
{
  /* Use the primary edge for consistent ID generation */
  if (!IS_CLOCKWISE_EDGE(edge)) {
    edge = edge->reversed;
  }
  char *source = graphmlVertexId(edge->reversed->to->vertex);
  char *target = graphmlVertexId(edge->to->vertex);
  addEdge(fp, edge->color, line, source, target);
}

/**
 * Adds an edge from a vertex to a corner in the GraphML output.
 */
static void addEdgeToCorner(FILE *fp, EDGE edge, int corner, int line)
{
  char *source = graphmlVertexId(edge->reversed->to->vertex);
  char *target = cornerId(edge->color, corner);
  assert(line != corner);
  addEdge(fp, edge->color, line, source, target);
}

/**
 * Adds an edge between two corners in the GraphML output.
 */
static void addEdgeBetweenCorners(FILE *fp, COLOR color, int low, int high)
{
  char *source = cornerId(color, low);
  char *target = cornerId(color, high);
  int line = 3 - high - low;
  addEdge(fp, color, line, source, target);
}

/**
 * Adds an edge from a corner to a vertex in the GraphML output.
 */
static void addEdgeFromCorner(FILE *fp, int corner, EDGE edge, int line)
{
  char *source = cornerId(edge->color, corner);
  char *target = graphmlVertexId(edge->to->vertex);
  assert(line != corner);
  addEdge(fp, edge->color, line, source, target);
}

/**
 * Process regular edges during triangle traversal for GraphML output.
 */
static void processRegularEdgeGraphML(void *data, EDGE current, int line)
{
  GraphMLData *gml = (GraphMLData *)data;
  graphmlAddEdge(gml->fp, current, line);
  addVertexIfPrimary(gml->fp, current->to->vertex, gml->color);
}

/**
 * Process a single corner during triangle traversal for GraphML output.
 */
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

/**
 * Process adjacent corners during triangle traversal for GraphML output.
 */
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

/**
 * Process all corners during triangle traversal for GraphML output.
 */
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

/**
 * Generates a file path for the current variation based on the variation
 * number.
 */
static char *subFilename(void)
{
  char *buffer = getBuffer();
  int levels = LevelsIPC;
  char *p = buffer;
  int variationNumber = VariationNumberIPC;
  p = p + sprintf(p, "%s", CurrentPrefixIPC);
  while (levels > 1) {
    p += sprintf(p, "/%2.2x", variationNumber % 256);
    GraphmlFileOps.initializeFolder(buffer);
    variationNumber /= 256;
    levels--;
  }
  p += sprintf(p, "/%3.3x.xml", variationNumber);
  return usingBuffer(buffer);
}

/**
 * Saves a triangle to the GraphML output.
 */
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

/**
 * Saves the current variation to a GraphML file.
 */
static void saveVariation(EDGE (*corners)[3])
{
  COLOR a;
  char *filename = subFilename();
  FILE *fp;
  VariationNumberIPC++;
  if (VariationNumberIPC - 1 <= IgnoreFirstVariantsPerSolution) {
    return;
  }
  GlobalVariantCountIPC++;
  fp = GraphmlFileOps.fopen(filename, "w");
  graphmlBegin(fp);
  for (a = 0; a < NCOLORS; a++, corners++) {
    saveTriangle(fp, a, corners);
  }
  graphmlEnd(fp);
  fclose(fp);
}

/**
 * Calculates the number of directory levels needed for the expected variations.
 */
int numberOfLevels(int expectedVariations)
{
  int result = 1;
  while (expectedVariations >= 4096) {
    result++;
    expectedVariations /= 256;
  }
  return result;
}

/**
 * Predicate function for saving a variation to GraphML.
 */
static struct predicateResult trySaveVariation(int round)
{
  (void)round;  // Unused parameter
  saveVariation(SelectedCornersIPC);
  return PredicateSuccessNextPredicate;
}

/* GraphML predicate for non-deterministic program */
struct predicate GraphMLPredicate = {"GraphML", trySaveVariation, NULL};
