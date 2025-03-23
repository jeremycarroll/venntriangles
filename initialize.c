/**
 * This file sets up complex data structures:
 * - each of the cycles
 * - the relationship between cycles and each other
 * - which cycles a priori can't be used with which face
 * etc.
 */
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* In this file we construct the constants used elsewhere. */
#define STATIC

#include "face.h"
#include "point.h"
#include "trail.h"
#include "utils.h"

STATIC struct face Faces[NFACES];
uint64_t EdgeCountsByDirectionAndColor[2][NCOLORS];
uint64_t FaceSumOfFaceDegree[NCOLORS + 1];
uint64_t EdgeCrossingCounts[NCOLORS][NCOLORS];
uint64_t EdgeCurvesComplete[NCOLORS];
