/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef STATISTICS_H
#define STATISTICS_H

#include "core.h"
#include "failure.h"

#include <time.h>

/**
 * Statistics tracking system for monitoring algorithm performance.
 */

/* Maximum number of statistics that can be tracked */
#define MAX_STATISTICS 10

/* Structure for tracking a single statistic */
struct statistic {
  char *name;       /* Full descriptive name */
  char *shortName;  /* Abbreviated name for compact display */
  uint64 *countPtr; /* Pointer to the counter value */
  bool verboseOnly; /* Only display in verbose mode */
};

typedef struct statistic Statistic;

/* Initialization and configuration */
extern void initializeStatisticLogging(char *filename, int frequency,
                                       int seconds);

/* Counter registration */
extern void statisticIncludeInteger(uint64 *counter, char *shortName,
                                    char *name, bool verboseOnly);
extern void statisticIncludeFailure(FAILURE failure);

/* Output and reporting */
extern void statisticPrintOneLine(int position, bool force);
extern void statisticPrintFull(void);

#endif  // STATISTICS_H
