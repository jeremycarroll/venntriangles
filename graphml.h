/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef GRAPHML_H
#define GRAPHML_H

#include "edge.h"
#include "vertex.h"

#include <stdio.h>

extern uint64_t GlobalVariantCount;
extern int graphmlSaveAllVariations(const char* prefix, int expectedVariations);

// Allow mocking of file operations.
struct graphmlFileIO {
  FILE* (*fopen)(const char* filename, const char* mode);
  void (*initializeFolder)(const char* folder);
};
extern struct graphmlFileIO graphmlFileOps;

// Internal function exposed for testing.
void graphmlPossibleCorners(void);
void graphmlChooseCornersWithContinuation(
    int cornerIndex, COLOR current, EDGE (*corners)[3],
    int (*continuation)(COLOR, EDGE (*corners)[3]));
#endif  // GRAPHML_H
