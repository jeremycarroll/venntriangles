/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */
#ifndef COMMON_H
#define COMMON_H

#include "edge.h"

/**
 * Common variables and utilities shared between predicates.
 * This header defines communication mechanisms between different
 * phases of the search algorithm.
 */

/*
 * Inter-Predicate Communication (IPC) variables.
 * Each variable is set by one predicate and read by others.
 *
 * Some variables are read during the forward search phase,
 * while others are read during backtracking.
 */

/* Solution tracking */
extern uint64 GlobalVariantCountIPC;   /* Total variant count */
extern uint64 GlobalSolutionsFoundIPC; /* Total solutions found */
extern int
    PerFaceDegreeSolutionNumberIPC; /* Solution number for current face set */

/* Search state */
extern uint64 CycleGuessCounterIPC; /* Number of cycle guesses */
extern int VariationCountIPC;  /* Number of variations for current solution */
extern int VariationNumberIPC; /* Current variation being processed */
extern int LevelsIPC;          /* Current recursion depth */

/* Output state */
extern EDGE SelectedCornersIPC[NCOLORS]
                              [3];  /* Selected corner edges for output */
extern char CurrentPrefixIPC[1024]; /* Current output path prefix */

/**
 * File I/O abstraction for GraphML output.
 * Allows overriding file operations in testing.
 */
struct graphmlFileIO {
  FILE* (*fopen)(const char* filename,
                 const char* mode);             /* File open function */
  void (*initializeFolder)(const char* folder); /* Directory initialization */
};
extern struct graphmlFileIO GraphmlFileOps;

/**
 * Calculate the number of binary subdivision levels needed.
 * @param expectedVariations Expected number of variations
 * @return Number of levels required
 */
extern int numberOfLevels(int expectedVariations);

#endif /* COMMON_H */
