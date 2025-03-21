#ifndef MAIN_H
#define MAIN_H

#include "logging.h"  // Include logging.h for log_level_t

// Declare functions used in main.c
const char* dynamicFaceDegreeSignature(void);
void dynamicSolutionWrite(const char* buffer);
void dynamicSearchFull(void (*foundSolution)(void));

#endif  // MAIN_H
