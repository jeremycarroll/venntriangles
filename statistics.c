#include "statistics.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "graph.h"

static Statistic statistics[MAX_STATISTICS];
static Failure failures[MAX_STATISTICS];
static time_t startTime;
static time_t lastLogTime;
static int checkFrequency = 1;
static int secondsBetweenLogs = 10;
static int checkCountDown = 0;
static FILE* logFile = NULL;

void dynamicStatisticNew(uint64_t* counter, char* shortName, char* name)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (statistics[i].countPtr == counter) {
      return;
    }
    if (statistics[i].countPtr == NULL) {
      statistics[i].countPtr = counter;
      statistics[i].shortName = shortName;
      statistics[i].name = name;
      return;
    }
  }
  assert(false);
}

void dynamicFailureStatisticNew(Failure* failure)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i].count[0] == 0) {
      failures[i] = *failure;
      return;
    }
  }
  assert(false);
}

void resetStatistics(void)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (statistics[i].countPtr == NULL) {
      break;
    }
    *statistics[i].countPtr = 0;
    statistics[i].countPtr = NULL;
  }
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i].count[0] == 0) {
      break;
    }
    memset(failures[i].count, 0, sizeof(failures[i].count));
  }
}

double dynamicStatisticCalculateSearchSpace(void)
{
  double result = 0.0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle == NULL) {
      result += log(Faces[i].cycleSetSize);
    }
  }
  return result;
}

int dynamicStatisticCountChosen(void)
{
  int result = 0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle != NULL) {
      result += 1;
    }
  }
  return result;
}

void dynamicStatisticPrintOneLine(int position)
{
  if (--checkCountDown <= 0) {
    time_t now = time(NULL);
    double seconds = difftime(now, lastLogTime);
    if (seconds >= secondsBetweenLogs) {
      char* timestr = asctime(localtime(&now));
      time_t elapsed = now - startTime;
      timestr[19] = 0;
      fprintf(logFile, "%s %ld:%02.2ld:%02.2ld %d %.1f ", timestr + 11,
              elapsed / 3600, (elapsed / 60) % 60, elapsed % 60,
              dynamicStatisticCountChosen(),
              dynamicStatisticCalculateSearchSpace());

      fprintf(logFile, "p %d ", position);
      for (int i = 0; i < MAX_STATISTICS; i++) {
        if (statistics[i].countPtr == NULL) {
          break;
        }
        fprintf(logFile, "%s %llu ", statistics[i].shortName,
                *statistics[i].countPtr);
      }
      for (int i = 0; i < MAX_STATISTICS; i++) {
        if (failures[i].count[0] == 0) {
          break;
        }
        int j;
        for (j = NFACES - 1; j > 0; j--) {
          if (failures[i].count[j]) {
            break;
          }
        }
        char separator = '[';
        fprintf(logFile, "%s: ", failures[i].shortLabel);
        for (int k = 0; k <= j; k++) {
          fprintf(logFile, "%c%llu", separator, failures[i].count[k]);
          separator = ' ';
        }
        fprintf(logFile, "] ");
      }
      fprintf(logFile, "\n");
      lastLogTime = now;
      fflush(logFile);
    }
    checkCountDown = checkFrequency;
  }
}

void dynamicStatisticPrintFull(void)
{
  time_t now = time(NULL);
  char* timestr = asctime(localtime(&now));
  time_t elapsed = now - startTime;
  double searchSpaceLogSize = dynamicStatisticCalculateSearchSpace();
  fprintf(logFile,
          "%sRuntime: %ld:%02.2ld:%02.2ld\nChosen faces: %d\nOpen Search Space "
          "Size: Log = %.2f "
          "; i.e. "
          "%6.3g\n",
          timestr, elapsed / 3600, (elapsed / 60) % 60, elapsed % 60,
          dynamicStatisticCountChosen(), searchSpaceLogSize,
          exp(searchSpaceLogSize));
  fprintf(logFile, "%30s %30s\n", "Counter", "Value(s)");
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (statistics[i].countPtr == NULL) {
      break;
    }
    fprintf(logFile, "%30s %30llu\n", statistics[i].name,
            *statistics[i].countPtr);
  }
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i].count[0] == 0) {
      break;
    }
    int j;
    for (j = NFACES - 1; j > 0; j--) {
      if (failures[i].count[j]) {
        break;
      }
    }
    char separator = '[';
    char buf[4096];
    char* bufptr = buf;
    for (int k = 0; k <= j; k++) {
      bufptr += sprintf(bufptr, "%c%llu", separator, failures[i].count[k]);
      separator = ' ';
    }
    sprintf(bufptr, "]");
    fprintf(logFile, "%30s %30s\n", failures[i].label, buf);
  }
  fprintf(logFile, "\n");
  lastLogTime = now;
  fflush(logFile);

  checkCountDown = checkFrequency;
}

void initializeStatisticLogging(char* filename, int frequency, int seconds)
{
  logFile = filename == NULL                  ? stderr
            : strcmp("/dev/stdout", filename) ? fopen(filename, "w")
                                              : stdout;
  checkFrequency = frequency;
  secondsBetweenLogs = seconds;
  startTime = time(NULL);
  lastLogTime = startTime;
  checkCountDown = checkFrequency;
  initializeFailures();
}
