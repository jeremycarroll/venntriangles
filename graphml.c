/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "graphml.h"
#include "memory.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

/* GraphML namespace and schema definitions */
static const char *GRAPHML_NS = "http://graphml.graphdrawing.org/xmlns";
static const char *GRAPHML_SCHEMA = "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd";

/* Begin GraphML document */
void graphmlBegin(FILE *fp) {
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fp, "<graphml xmlns=\"%s\"\n", GRAPHML_NS);
  fprintf(fp, "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
  fprintf(fp, "         xsi:schemaLocation=\"%s %s\">\n", GRAPHML_NS, GRAPHML_SCHEMA);
  
  /* Define custom attributes */
  fprintf(fp, "  <key id=\"colors\" for=\"node\" attr.name=\"colors\" attr.type=\"string\"/>\n");
  fprintf(fp, "  <key id=\"primary\" for=\"node\" attr.name=\"primary\" attr.type=\"string\"/>\n");
  fprintf(fp, "  <key id=\"secondary\" for=\"node\" attr.name=\"secondary\" attr.type=\"string\"/>\n");
  fprintf(fp, "  <key id=\"color\" for=\"edge\" attr.name=\"color\" attr.type=\"string\"/>\n");
  
  fprintf(fp, "  <graph id=\"venn_diagram\" edgedefault=\"undirected\">\n");
}

/* End GraphML document */
void graphmlEnd(FILE *fp) {
  fprintf(fp, "  </graph>\n");
  fprintf(fp, "</graphml>\n");
}

/* Add a point (vertex) to the graph */
void graphmlAddPoint(FILE *fp, POINT point) {
  fprintf(fp, "    <node id=\"%s\">\n", graphmlPointId(point));
  fprintf(fp, "      <data key=\"colors\">%s</data>\n", colorSetToStr(point->colors));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", 'a' + point->primary);
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n", 'a' + point->secondary);
  fprintf(fp, "    </node>\n");
}

/* Add an edge to the graph */
void graphmlAddEdge(FILE *fp, EDGE edge) {
  /* Use the primary edge for consistent ID generation */
  if (!IS_PRIMARY_EDGE(edge)) {
    edge = edge->reversed;
  }
  fprintf(fp, "    <edge source=\"%s\" target=\"%s\">\n", 
          graphmlPointId(edge->reversed->to->point),
          graphmlPointId(edge->to->point));
  fprintf(fp, "      <data key=\"color\">%c</data>\n", 'a' + edge->color);
  fprintf(fp, "    </edge>\n");
}

/* Add a curve to the graph */
void graphmlAddCurve(FILE *fp, COLOR color, const char *id) {
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"color\">%c</data>\n", 'a' + color);
  fprintf(fp, "    </node>\n");
}

/* Generate unique IDs for graph elements */
char *graphmlPointId(POINT point) {
  char *buffer = getBuffer();
  sprintf(buffer, "p_%s_%c_%c", colorSetToStr(point->colors), 'a' + point->primary, 'a' + point->secondary);
  return usingBuffer(buffer);
}

char *graphmlEdgeId(EDGE edge) {
  char *buffer = getBuffer();
  /* Use the primary edge for consistent ID generation */
  if (!IS_PRIMARY_EDGE(edge)) {
    edge = edge->reversed;
  }
  sprintf(buffer, "e_%c_%s", 'a' + edge->color, colorSetToStr(edge->colors));
  return usingBuffer(buffer);
}

char *graphmlCurveId(COLOR color) {
  char *buffer = getBuffer();
  sprintf(buffer, "c_%c", 'a' + color);
  return usingBuffer(buffer);
} 


static  savePartialVariations(COLOR current, EDGE **corners);
static const char * CurrentPrefix = NULL;
static int VariationNumber = 1;
/**
 * @brief Find and save all variations of the solution to graphml files
 * 
 * @param prefix The folder and prefix for files used in the solutions.
 */
void graphmlSaveAllVariations(const char* prefix)
{
  EDGE corners[NCOLORS][3];
  CurrentPrefix = prefix;
  savePartialVariations(0, corners);

  #if 0
  EDGE corners[3][2];
  char filename[1024];
  int numberOfVariations = 1;
  int pLength;
  FILE* fp;
  if (strcmp(prefix, LastPrefix) != 0) {
    strcpy(LastPrefix, prefix);
    SolutionNumber = 1;
  }
  snprintf(filename, sizeof(filename), "%s-%2.2d.txt", prefix,
           SolutionNumber++);
  fp = fopen(filename, "w");
  if (fp == NULL) {
    perror(filename);
    exit(EXIT_FAILURE);
  }
  solutionPrint(fp);
  for (COLOR a = 0; a < NCOLORS; a++) {
    edgeFindCorners(a, corners);
    for (int i = 0; i < 3; i++) {
      fprintf(fp, "{%c:%d} ", colorToChar(a), i);
      if (corners[i][0] == NULL) {
        EDGE edge = edgeOnCentralFace(a);
        pLength = edgePathLength(edge, edgeFollowBackwards(edge));
        fprintf(fp, "NULL/%d ", pLength);
      } else {
        pLength = edgePathLength(corners[i][0]->reversed, corners[i][1]);
        fprintf(fp, "(%s => %s/%d) ", edgeToStr(corners[i][0]),
                edgeToStr(corners[i][1]), pLength);
      }
      numberOfVariations *= pLength;
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "\n\nVariations = %d\n", numberOfVariations);
  fprintf(fp, "\nSolution signature %s\nClass signature %s\n",
          d6SignatureToString(d6SignatureFromFaces()),
          d6SignatureToString(d6MaxSignature()));
  fclose(fp);
  #endif
}

static void saveVariation(EDGE **corners) {
  // TODO: Implement
}

static EDGE * getPath(EDGE from, EDGE to) {
  EDGE * path;
  int length = edgePathLength(from, to, NULL);
  path = NEW_ARRAY(EDGE, length + 1);
  edgePathLength(from, to, path);
  path[length] = NULL;
  return path;
}

static EDGE * possibleCorners(COLOR color, EDGE from, EDGE to) {
  if (from == NULL) {
      EDGE edge = edgeOnCentralFace(color);
      return getPath(edge, edgeFollowBackwards(edge));
  }
  return getPath(from, to)
}

static void chooseCornersThenSavePartialVariations(int cornerIndex, EDGE **cornerPairs, COLOR current, EDGE **corners) {
  EDGE * possibilities;
  int i;
  if (cornerIndex == 3) {
    savePartialVariations(current+1, corners);
    return;
  }
  possibilities = possibleCorners(current, corners[cornerIndex][0], corners[cornerIndex][1]);
  for (i = 0; possibilities[i] != NULL; i++) {
    corners[current][cornerIndex] = possibilities[i];
    chooseCornersThenSavePartialVariations(cornerIndex+1, cornerPairs, current, corners);
  }
}

static  savePartialVariations(COLOR current, EDGE **corners) {
  // The possible corners for the current color.
  EDGE cornerPairs[3][2];
  if (current == NCOLORS) {
    saveVariation(corners);
    return;
  }
  edgeFindCorners(a, cornerPairs);
  chooseCornersThenSavePartialVariations(0, cornerPairs, current, corners);
}