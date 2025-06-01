/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "visible_for_testing.h"

/* Helper functions for testing */

/**
 * Creates a face with a specific cycle and tests validity.
 * @param colors String representation of colors for the face
 * @param cycle String representation of the cycle to add
 * @return The created and validated face
 */
FACE dynamicFaceAddSpecific(char* colors, char* cycle);

/**
 * Creates an array of face degrees from variable arguments.
 * @param a First face degree
 * @param ... Remaining face degrees (must provide NCOLORS total values)
 * @return Array of FACE_DEGREE values
 */
FACE_DEGREE* intArray(int a, ...);

/**
 * Prints a cycle set with validation.
 * @param cycleSet The cycle set to print
 * @return true if cycle set is valid, false if invalid
 */
bool dynamicCycleSetPrint(CYCLESET cycleSet);

#endif /* TEST_HELPERS_H */ 