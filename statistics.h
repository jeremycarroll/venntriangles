#ifndef STATISTICS_H
#define STATISTICS_H

#include "core.h"
#include "failure.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_STATISTICS 10

typedef struct {
  uint64_t* countPtr;
  char* shortName;
  char* name;
} Statistic;

extern void statisticIncludeInteger(uint64_t* counter, char* shortName,
                                    char* name);
extern void statisticIncludeFailure(FAILURE failure);
extern void resetStatistics(void);
extern double statisticCalculateSearchSpace(void);
extern int statisticCountChosen(void);
extern void statisticPrintOneLine(int position, bool force);
extern void statisticPrintFull(void);
extern void initializeStatisticLogging(char* filename, int frequency,
                                       int seconds);

#endif  // STATISTICS_H
