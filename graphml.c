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
void graphmlAddNode(FILE *fp, POINT point, const char *id) {
  fprintf(fp, "    <node id=\"%s\">\n", id);
  fprintf(fp, "      <data key=\"colors\">%s</data>\n", colorSetToStr(point->colors));
  fprintf(fp, "      <data key=\"primary\">%c</data>\n", 'a' + point->primary);
  fprintf(fp, "      <data key=\"secondary\">%c</data>\n", 'a' + point->secondary);
  fprintf(fp, "    </node>\n");
}

/* Add an edge to the graph */
void graphmlAddEdge(FILE *fp, EDGE edge) {
  /* Only output the primary edge */
  if (!IS_PRIMARY_EDGE(edge)) {
    return;
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