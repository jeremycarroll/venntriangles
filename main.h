/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

extern int CentralFaceDegrees[NCOLORS];

// Global configuration constants
extern int MaxVariantsPerSolution;
extern int GlobalMaxSolutions;
extern int PerFaceDegreeMaxSolutions;
extern int GlobalSkipSolutions;
extern int PerFaceDegreeSkipSolutions;
extern int IgnoreFirstVariantsPerSolution;
extern int GlobalSolutionsFound;  // Counter for solutions found

// Declare functions used in main.c
extern char* s6FaceDegreeSignature(void);
extern int dynamicMain0(int argc, char* argv[]);
extern void solutionWrite(const char* prefix);
extern void searchFull(void (*foundSolution)(void));
extern void disaster(const char* message);
extern char* Argv0;

#endif  // MAIN_H
