#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdbool.h>
#include "face.h"

extern FACE dynamicFaceAddSpecific(char* colors, char* cycle);
extern int* intArray(int a, ...);
extern bool dynamicCycleSetPrint(CYCLESET cycleSet);

#endif /* TEST_HELPERS_H */ 