/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef UTILS_H
#define UTILS_H

#include "core.h"

/**
 * General utility functions for program initialization and error handling.
 */

/**
 * Global initialization for all subsystems.
 */
extern void initialize(void);

/**
 * Creates or validates an output folder for results storage.
 * @param folder Path to initialize
 */
extern void initializeFolder(const char *folder);

/**
 * Handles fatal errors with usage information.
 * @param programName Program name for usage display
 * @param message Error message before exit
 */
extern void disaster(const char *programName, const char *message);

#endif  // UTILS_H
