/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef STATISTICS_H
#define STATISTICS_H

#include "core.h"
#include "failure.h"
#include "memory.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_STATISTICS 10

struct statistic {
  char *name;
  char *shortName;
  uint64 *countPtr;
  bool verboseOnly;
};

typedef struct statistic Statistic;

extern void initializeStatisticLogging(char *filename, int frequency,
                                       int seconds);
extern void statisticIncludeInteger(uint64 *counter, char *shortName,
                                    char *name, bool verboseOnly);
extern void statisticIncludeFailure(FAILURE failure);
extern void statisticPrintOneLine(int position, bool force);
extern void statisticPrintFull(void);

// exposed for testing
extern void resetStatistics(void);
#endif  // STATISTICS_H
