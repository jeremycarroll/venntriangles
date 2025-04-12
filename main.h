/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

// Global configuration constants
extern int MaxVariantsPerSolution;
extern int MaxSolutions;

// Declare functions used in main.c
extern const char* faceDegreeSignature(void);
extern int dynamicMain0(int argc, char* argv[]);
extern void solutionWrite(const char* prefix);
extern void searchFull(void (*foundSolution)(void));

#endif  // MAIN_H
