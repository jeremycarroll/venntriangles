/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

extern FACE_DEGREE CentralFaceDegreesFlag[NCOLORS];

extern char* TargetFolderFlag;             // Output folder for results (-f)
extern int MaxVariantsPerSolutionFlag;     // Maximum variants per solution (-n)
extern int GlobalMaxSolutionsFlag;         // Global maximum solutions (-m)
extern int PerFaceDegreeMaxSolutionsFlag;  // Per face degree maximum solutions
                                           // (-m with -d)
extern int GlobalSkipSolutionsFlag;         // Global solutions to skip (-k)
extern int PerFaceDegreeSkipSolutionsFlag;  // Per face degree solutions to skip
                                            // (-k with -d)
extern int
    IgnoreFirstVariantsPerSolution;  // Variants to ignore per solution (-j)
extern FACE_DEGREE CentralFaceDegreesFlag[];  // Central face degrees (-d)
extern bool VerboseModeFlag;                  // Verbose output mode (-v)
extern bool TracingFlag;                      // TracingFlag output mode (-t)

// Hook for testing and running.
extern int dynamicMain0(int argc, char* argv[]);
extern struct predicate* NonDeterministicProgram[];

#endif  // MAIN_H
