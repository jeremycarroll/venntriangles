/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "statistics.h"

#include "face.h"
#include "main.h"

#include <assert.h>
#include <math.h>
#include <string.h>

/* Global variables (file scoped static) */
static Statistic Statistics[MAX_STATISTICS];
static Failure* Failures[MAX_STATISTICS];
static time_t StartTime;
static time_t LastLogTime;
static int CheckFrequency = 1;
static int SecondsBetweenLogs = 10;
static int CheckCountDown = 0;
static FILE* LogFile = NULL;
static double statisticCalculateSearchSpace(void);
static int statisticCountChosen(void);
static void formatElapsedTime(time_t elapsed, char* buffer, size_t bufferSize);
static void formatFailureCounts(Failure* failure, int maxIndex, char* buffer);
static void printFailureCountsOneLine(Failure* failure, int maxIndex);
static void printFailureCountsFull(Failure* failure, int maxIndex);
static void printFailureCounts(bool oneLine);
static void printStatisticsCounters(bool oneLine);
static void updateLoggingState(time_t now);

/* Externally linked functions */

void initializeStatisticLogging(char* filename, int frequency, int seconds)
{
  LogFile = filename == NULL                  ? stderr
            : strcmp("/dev/stdout", filename) ? fopen(filename, "w")
                                              : stdout;
  CheckFrequency = frequency;
  SecondsBetweenLogs = seconds;
  StartTime = time(NULL);
  LastLogTime = StartTime;
  CheckCountDown = CheckFrequency;
  initializeFailures();
}

void statisticIncludeInteger(uint64* counter, char* shortName, char* name,
                             bool verboseOnly)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Statistics[i].countPtr == counter) {
      return;
    }
    if (Statistics[i].countPtr == NULL) {
      Statistics[i].countPtr = counter;
      Statistics[i].shortName = shortName;
      Statistics[i].name = name;
      Statistics[i].verboseOnly = verboseOnly;
      return;
    }
  }
  assert(false);
}

void statisticIncludeFailure(Failure* failure)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Failures[i] == failure) {
      return;
    }
    if (Failures[i] == 0) {
      Failures[i] = failure;
      return;
    }
  }
  assert(false);
}

void statisticPrintOneLine(int position, bool force)
{
  if (--CheckCountDown <= 0 || force) {
    time_t now = time(NULL);
    double seconds = difftime(now, LastLogTime);
    if (seconds >= SecondsBetweenLogs || force) {
      char* timestr = asctime(localtime(&now));
      time_t elapsed = now - StartTime;
      timestr[19] = 0;

      char elapsedStr[20];
      formatElapsedTime(elapsed, elapsedStr, sizeof(elapsedStr));

      if (VerboseMode) {
        fprintf(LogFile, "%s %s %d %.1f p %d ", timestr + 11, elapsedStr,
                statisticCountChosen(), statisticCalculateSearchSpace(),
                position);
      }

      printStatisticsCounters(true);
      printFailureCounts(true);
      fprintf(LogFile, "\n");

      updateLoggingState(now);
    }
  }
}

void statisticPrintFull(void)
{
  time_t now = time(NULL);
  char* timestr = asctime(localtime(&now));
  time_t elapsed = now - StartTime;
  double searchSpaceLogSize = statisticCalculateSearchSpace();

  char elapsedStr[20];
  formatElapsedTime(elapsed, elapsedStr, sizeof(elapsedStr));

  fprintf(LogFile, "%sRuntime: %s\n", timestr, elapsedStr);

  if (VerboseMode) {
    fprintf(LogFile,
            "Chosen faces: %d\nOpen Search Space Size: Log = %.2f "
            "; i.e. %6.3g\n",
            statisticCountChosen(), searchSpaceLogSize,
            exp(searchSpaceLogSize));
  }

  fprintf(LogFile, "%30s %30s\n", "Counter", "Value(s)");

  printStatisticsCounters(false);
  printFailureCounts(false);

  fprintf(LogFile, "\n");
  updateLoggingState(now);
}

static double statisticCalculateSearchSpace(void)
{
  double result = 0.0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle == NULL) {
      result += log(Faces[i].cycleSetSize);
    }
  }
  return result;
}

static int statisticCountChosen(void)
{
  int result = 0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle != NULL) {
      result += 1;
    }
  }
  return result;
}

static void formatElapsedTime(time_t elapsed, char* buffer, size_t bufferSize)
{
  snprintf(buffer, bufferSize, "%ld:%02ld:%02ld", elapsed / 3600,
           (elapsed % 3600) / 60, elapsed % 60);
}

static void formatFailureCounts(Failure* failure, int maxIndex, char* buffer)
{
  char* bufptr = buffer;
  char separator = '[';
  for (int k = 0; k <= maxIndex; k++) {
    bufptr += sprintf(bufptr, "%c%llu", separator, failure->count[k]);
    separator = ' ';
  }
  sprintf(bufptr, "]");
}

static void printFailureCountsOneLine(Failure* failure, int maxIndex)
{
  fprintf(LogFile, "%s: ", failure->shortLabel);
  char separator = '[';
  for (int k = 0; k <= maxIndex; k++) {
    fprintf(LogFile, "%c%llu", separator, failure->count[k]);
    separator = ' ';
  }
  fprintf(LogFile, "] ");
}

static void printFailureCountsFull(Failure* failure, int maxIndex)
{
  char buf[4096];
  formatFailureCounts(failure, maxIndex, buf);
  fprintf(LogFile, "%30s %30s\n", failure->label, buf);
}

static int findMaxNonZeroIndex(Failure* failure)
{
  int j;
  for (j = NFACES - 1; j > 0; j--) {
    if (failure->count[j]) {
      break;
    }
  }
  return j;
}

static void printFailureCounts(bool oneLine)
{
  if (!VerboseMode) {
    return;  // Skip failures in non-verbose mode
  }
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Failures[i]->count[0] == 0) {
      break;
    }

    int maxIndex = findMaxNonZeroIndex(Failures[i]);

    if (oneLine) {
      printFailureCountsOneLine(Failures[i], maxIndex);
    } else {
      printFailureCountsFull(Failures[i], maxIndex);
    }
  }
}

static void printStatisticsCounters(bool oneLine)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Statistics[i].countPtr == NULL) {
      break;
    }
    if (!Statistics[i].verboseOnly || VerboseMode) {
      if (oneLine) {
        fprintf(LogFile, "%s %llu ", Statistics[i].shortName,
                *Statistics[i].countPtr);
      } else {
        fprintf(LogFile, "%30s %30llu\n", Statistics[i].name,
                *Statistics[i].countPtr);
      }
    }
  }
}

static void updateLoggingState(time_t now)
{
  LastLogTime = now;
  fflush(LogFile);
  CheckCountDown = CheckFrequency;
}
