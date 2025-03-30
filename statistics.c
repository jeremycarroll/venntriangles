/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "statistics.h"

#include "face.h"

#include <assert.h>
#include <math.h>
#include <string.h>

/* Global variables (file scoped static) */
static Statistic Statistics[MAX_STATISTICS];
static Failure Failures[MAX_STATISTICS];
static time_t StartTime;
static time_t LastLogTime;
static int CheckFrequency = 1;
static int SecondsBetweenLogs = 10;
static int CheckCountDown = 0;
static FILE* LogFile = NULL;

/* Externally linked functions */
void statisticIncludeInteger(uint64_t* counter, char* shortName, char* name)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Statistics[i].countPtr == counter) {
      return;
    }
    if (Statistics[i].countPtr == NULL) {
      Statistics[i].countPtr = counter;
      Statistics[i].shortName = shortName;
      Statistics[i].name = name;
      return;
    }
  }
  assert(false);
}

void statisticIncludeFailure(Failure* failure)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Failures[i].count[0] == 0) {
      Failures[i] = *failure;
      return;
    }
  }
  assert(false);
}

void resetStatistics(void)
{
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Statistics[i].countPtr == NULL) {
      break;
    }
    *Statistics[i].countPtr = 0;
    Statistics[i].countPtr = NULL;
  }
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Failures[i].count[0] == 0) {
      break;
    }
    memset(Failures[i].count, 0, sizeof(Failures[i].count));
  }
}

double statisticCalculateSearchSpace(void)
{
  double result = 0.0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle == NULL) {
      result += log(Faces[i].cycleSetSize);
    }
  }
  return result;
}

int statisticCountChosen(void)
{
  int result = 0;
  for (uint32_t i = 0; i < NFACES; i++) {
    if (Faces[i].cycle != NULL) {
      result += 1;
    }
  }
  return result;
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
      fprintf(LogFile, "%s %ld:%02.2ld:%02.2ld %d %.1f ", timestr + 11,
              elapsed / 3600, (elapsed / 60) % 60, elapsed % 60,
              statisticCountChosen(), statisticCalculateSearchSpace());

      fprintf(LogFile, "p %d ", position);
      for (int i = 0; i < MAX_STATISTICS; i++) {
        if (Statistics[i].countPtr == NULL) {
          break;
        }
        fprintf(LogFile, "%s %llu ", Statistics[i].shortName,
                *Statistics[i].countPtr);
      }
      for (int i = 0; i < MAX_STATISTICS; i++) {
        if (Failures[i].count[0] == 0) {
          break;
        }
        int j;
        for (j = NFACES - 1; j > 0; j--) {
          if (Failures[i].count[j]) {
            break;
          }
        }
        char separator = '[';
        fprintf(LogFile, "%s: ", Failures[i].shortLabel);
        for (int k = 0; k <= j; k++) {
          fprintf(LogFile, "%c%llu", separator, Failures[i].count[k]);
          separator = ' ';
        }
        fprintf(LogFile, "] ");
      }
      fprintf(LogFile, "\n");
      LastLogTime = now;
      fflush(LogFile);
    }
    CheckCountDown = CheckFrequency;
  }
}

void statisticPrintFull(void)
{
  time_t now = time(NULL);
  char* timestr = asctime(localtime(&now));
  time_t elapsed = now - StartTime;
  double searchSpaceLogSize = statisticCalculateSearchSpace();
  fprintf(LogFile,
          "%sRuntime: %ld:%02.2ld:%02.2ld\nChosen faces: %d\nOpen Search Space "
          "Size: Log = %.2f "
          "; i.e. "
          "%6.3g\n",
          timestr, elapsed / 3600, (elapsed / 60) % 60, elapsed % 60,
          statisticCountChosen(), searchSpaceLogSize, exp(searchSpaceLogSize));
  fprintf(LogFile, "%30s %30s\n", "Counter", "Value(s)");
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Statistics[i].countPtr == NULL) {
      break;
    }
    fprintf(LogFile, "%30s %30llu\n", Statistics[i].name,
            *Statistics[i].countPtr);
  }
  for (int i = 0; i < MAX_STATISTICS; i++) {
    if (Failures[i].count[0] == 0) {
      break;
    }
    int j;
    for (j = NFACES - 1; j > 0; j--) {
      if (Failures[i].count[j]) {
        break;
      }
    }
    char separator = '[';
    char buf[4096];
    char* bufptr = buf;
    for (int k = 0; k <= j; k++) {
      bufptr += sprintf(bufptr, "%c%llu", separator, Failures[i].count[k]);
      separator = ' ';
    }
    sprintf(bufptr, "]");
    fprintf(LogFile, "%30s %30s\n", Failures[i].label, buf);
  }
  fprintf(LogFile, "\n");
  LastLogTime = now;
  fflush(LogFile);

  CheckCountDown = CheckFrequency;
}

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
