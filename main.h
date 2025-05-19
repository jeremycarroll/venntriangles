/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

extern FACE_DEGREE CentralFaceDegrees[NCOLORS];

extern char* TargetFolder;              // Output folder for results (-f)
extern int MaxVariantsPerSolution;      // Maximum variants per solution (-n)
extern int GlobalMaxSolutions;          // Global maximum solutions (-m)
extern int PerFaceDegreeMaxSolutions;   // Per face degree maximum solutions (-m
                                        // with -d)
extern int GlobalSkipSolutions;         // Global solutions to skip (-k)
extern int PerFaceDegreeSkipSolutions;  // Per face degree solutions to skip (-k
                                        // with -d)
extern int
    IgnoreFirstVariantsPerSolution;  // Variants to ignore per solution (-j)
extern FACE_DEGREE CentralFaceDegrees[];  // Central face degrees (-d)
extern bool VerboseMode;                  // Verbose output mode (-v)
extern bool Tracing;                      // Tracing output mode (-t)

// Hook for testing and running.
extern int dynamicMain0(int argc, char* argv[]);

#endif  // MAIN_H
