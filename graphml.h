/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef GRAPHML_H
#define GRAPHML_H

#include "face.h"
#include "point.h"

#include <stdio.h>

/* GraphML output functions */
extern void graphmlBegin(FILE *fp);
extern void graphmlEnd(FILE *fp);
extern void graphmlAddPoint(FILE *fp, POINT point);
extern void graphmlAddEdge(FILE *fp, EDGE edge);
extern void graphmlAddCurve(FILE *fp, COLOR color, const char *id);

/* Helper functions for generating IDs */
extern char *graphmlPointId(POINT point);
extern char *graphmlEdgeId(EDGE edge);
extern char *graphmlCurveId(COLOR color);

#endif  // GRAPHML_H 