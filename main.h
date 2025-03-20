#ifndef MAIN_H
#define MAIN_H

#include "logging.h"  // Include logging.h for log_level_t

// Declare functions used in main.c
const char* d6FaceDegreeSignature(void);
void writeSolution(const char* buffer);
void full_search(void (*foundSolution)(void));

#endif  // MAIN_H
