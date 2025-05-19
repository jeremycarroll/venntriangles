/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef SOLUTIONWRITE_H
#define SOLUTIONWRITE_H

#include "engine.h"

extern int PerFaceDegreeSolutionNumber;
extern int VariationCount;
extern int VariationNumber;
extern int Levels;
extern char CurrentPrefix[1024];

/* Write a solution to disk */
void writeSolution(void);

#endif /* SOLUTIONWRITE_H */
