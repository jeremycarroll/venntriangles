/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

extern int CentralFaceDegrees[NCOLORS];

// Global configuration constants
extern int MaxVariantsPerSolution;
extern int MaxSolutions;
extern int IgnoreFirstVariantsPerSolution;
extern int IgnoreFirstSolutions;

// Declare functions used in main.c
extern char* faceDegreeSignature(void);
extern int dynamicMain0(int argc, char* argv[]);
extern void solutionWrite(const char* prefix);
extern void searchFull(void (*foundSolution)(void));

#endif  // MAIN_H
