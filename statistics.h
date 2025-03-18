#ifndef _STATISTICS_H
#define _STATISTICS_H

#include <stdio.h>

#include "types.h"

/* Statistics function declarations */
void newFailureStatistic(FAILURE failure);
void resetStatistics(void);
void printStatistics(FILE *logFile);

#endif /* _STATISTICS_H */