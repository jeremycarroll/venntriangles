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


void newStatistic(uint64_t* counter, char* shortName, char* name);
void newFailureStatistic(FAILURE failure);
void resetStatistics(void);
double calculateSearchSpace(void);
int countChosen(void);
void printStatisticsOneLine(int position);
void printStatisticsFull(void);
void initializeStatsLogging(char* filename, int frequency, int seconds);

#endif // STATISTICS_H