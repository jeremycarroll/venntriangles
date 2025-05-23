/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */
#ifndef COMMON_H
#define COMMON_H

/* Stuff common between more than one predicate. */

#include "edge.h"

/* Data variables shared between predicates. 
Each of these is set by typically one predicate,
and read by at least one other predicate.

The reader may be after or before the writer. 
When the reader is before the writer, the reading
typically happens during the backtracking.
*/
extern uint64 GlobalVariantCountIPC;
extern uint64 CycleGuessCounterIPC;
extern uint64 GlobalSolutionsFoundIPC;
extern int PerFaceDegreeSolutionNumberIPC;
extern int VariationCountIPC;
extern int VariationNumberIPC;
extern int LevelsIPC;
extern EDGE SelectedCornersIPC[NCOLORS][3];
extern char CurrentPrefixIPC[1024];

/* Utilities used by more than one predicate. */
struct graphmlFileIO {
  FILE* (*fopen)(const char* filename, const char* mode);
  void (*initializeFolder)(const char* folder);
};
extern struct graphmlFileIO GraphmlFileOps;
extern int numberOfLevels(int expectedVariations);
extern int searchCountVariations(char* variationMultiplication);

#endif  // COMMON_H
