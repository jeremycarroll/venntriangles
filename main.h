/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MAIN_H
#define MAIN_H

#include "face.h"

/**
 * Command line flags and globals for controlling program execution.
 */

/* Output control flags */
extern char* TargetFolderFlag; /* Output folder for results (-f) */
extern bool VerboseModeFlag;   /* Verbose output mode (-v) */
extern bool TracingFlag;       /* Tracing output mode (-t) */

/* Search constraint flags */
extern FACE_DEGREE
    CentralFaceDegreesFlag[NCOLORS]; /* Central face degrees (-d) */

/* Solution limiting flags */
extern int MaxVariantsPerSolutionFlag; /* Max variants per solution (-n) */
extern int GlobalMaxSolutionsFlag;     /* Global maximum solutions (-m) */
extern int
    PerFaceDegreeMaxSolutionsFlag;  /* Per-degree max solutions (-m with -d) */
extern int GlobalSkipSolutionsFlag; /* Global solutions to skip (-k) */
extern int PerFaceDegreeSkipSolutionsFlag; /* Per-degree solutions to skip (-k
                                              with -d) */
extern int
    IgnoreFirstVariantsPerSolution; /* Variants to ignore per solution (-j) */

/* Program entry points */
extern int realMain0(int argc, char* argv[]); /* Main program entry */
extern struct predicate*
    NonDeterministicProgram[]; /* Search algorithm predicates */

#endif /* MAIN_H */
