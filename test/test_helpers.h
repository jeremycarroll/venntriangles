/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdbool.h>
#include "face.h"

extern FACE dynamicFaceAddSpecific(char* colors, char* cycle);
extern FACE_DEGREE* intArray(int a, ...);
extern bool dynamicCycleSetPrint(CYCLESET cycleSet);

#endif /* TEST_HELPERS_H */ 