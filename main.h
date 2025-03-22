#ifndef MAIN_H
#define MAIN_H

// Declare functions used in main.c
extern const char* dynamicFaceDegreeSignature(void);
extern void dynamicSolutionWrite(const char* buffer);
extern void dynamicSearchFull(void (*foundSolution)(void));

#endif  // MAIN_H
