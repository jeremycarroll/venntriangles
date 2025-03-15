#include <math.h>
#include <time.h>

#include "venn.h"

/*
We provide named counters, and the failure data.
*/
#define MAX_STATISTICS 10
static struct {
  uint64_t* countPtr;
  char* shortName;
  char* name;
} statistics[MAX_STATISTICS];

static FAILURE failures[MAX_STATISTICS];
static time_t startTime;
static time_t lastLogTime;
static int checkFrequency = 1;
static int secondsBetweenLogs = 10;
static int checkCountDown = 0;
static FILE* logFile = NULL;

void newStatistic(uint64_t* counter, char* shortName, char* name)
{
  int i;
  for (i = 0; i < MAX_STATISTICS; i++) {
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

void newFailureStatistic(FAILURE failure)
{
  int i;
  for (i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i] == NULL) {
      failures[i] = failure;
      return;
    }
  }
  assert(false);
}

void resetStatistics(void)
{
  int i;
  for (i = 0; i < MAX_STATISTICS; i++) {
    if (statistics[i].countPtr == NULL) {
      break;
    }
    *statistics[i].countPtr = 0;
    statistics[i].countPtr = 0;
  }
  for (i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i] == NULL) {
      break;
    }
    memset(failures[i]->count, 0, sizeof(failures[i]->count));
    failures[i] = NULL;
  }
}

double searchSpace(void)
{
  double result = 0.0;
  uint32_t i;
  FACE face;
  for (i = 0, face = g_faces; i < NFACES; i++, face++) {
    if (face->cycle == NULL) {
      result += log(face->cycleSetSize);
    }
  }
  return result;
}

int chosen(void)
{
  int result = 0;
  uint32_t i;
  FACE face;
  for (i = 0, face = g_faces; i < NFACES; i++, face++) {
    if (face->cycle != NULL) {
      result += 1;
    }
  }
  return result;
}

/*
We might print the statistics in one line.
*/
void printStatisticsOneLine(int position)
{
  int i, j, k;
  char* timestr;
  time_t elapsed;
  time_t now;
  char separator;
  if (--checkCountDown <= 0) {
    now = time(NULL);
    double seconds = difftime(now, lastLogTime);
    if (seconds >= secondsBetweenLogs) {
      timestr = asctime(localtime(&now));
      elapsed = now - startTime;
      /*
       "Www Mmm dd hh:mm:ss yyyy",
      */
      timestr[19] = 0;
      fprintf(logFile, "%s %ld:%02.2ld:%02.2ld %d %.1f ", timestr + 11,
              elapsed / 3600, (elapsed / 60) % 60, elapsed % 60, chosen(),
              searchSpace());

      fprintf(logFile, "p %d ", position);
      for (i = 0; i < MAX_STATISTICS; i++) {
        if (statistics[i].countPtr == NULL) {
          break;
        }
        fprintf(logFile, "%s %llu ", statistics[i].shortName,
                *statistics[i].countPtr);
      }
      for (i = 0; i < MAX_STATISTICS; i++) {
        if (failures[i] == NULL) {
          break;
        }
        for (j = NFACES - 1; j > 0; j--) {
          if (failures[i]->count[j]) {
            break;
          }
        }
        separator = '[';

        fprintf(logFile, "%s: ", failures[i]->shortLabel);
        for (k = 0; k <= j; k++) {
          fprintf(logFile, "%c%llu", separator, failures[i]->count[k]);
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
/*
We always print the statistics in multiple lines.
*/
void printStatisticsFull(void)
{
  int i, j, k;
  char* timestr;
  time_t elapsed;
  time_t now;
  char separator;
  double searchSpaceLogSize;
  char buf[4096];
  char* bufptr;
  now = time(NULL);
  /* timestr is "\n\0" terminated. */
  timestr = asctime(localtime(&now));
  elapsed = now - startTime;
  searchSpaceLogSize = searchSpace();
  fprintf(logFile,
          "%sRuntime: %ld:%02.2ld:%02.2ld\nChosen faces: %d\nOpen Search Space "
          "Size: Log = %.2f "
          "; i.e. "
          "%6.3g\n",
          timestr, elapsed / 3600, (elapsed / 60) % 60, elapsed % 60, chosen(),
          searchSpaceLogSize, exp(searchSpaceLogSize));
  fprintf(logFile, "%30s %30s\n", "Counter", "Value(s)");
  for (i = 0; i < MAX_STATISTICS; i++) {
    if (statistics[i].countPtr == NULL) {
      break;
    }
    fprintf(logFile, "%30s %30llu\n", statistics[i].name,
            *statistics[i].countPtr);
  }
  for (i = 0; i < MAX_STATISTICS; i++) {
    if (failures[i] == NULL) {
      break;
    }
    for (j = NFACES - 1; j > 0; j--) {
      if (failures[i]->count[j]) {
        break;
      }
    }
    separator = '[';
    bufptr = buf;
    for (k = 0; k <= j; k++) {
      bufptr += sprintf(bufptr, "%c%llu", separator, failures[i]->count[k]);
      separator = ' ';
    }
    sprintf(bufptr, "]");
    fprintf(logFile, "%30s %30s\n", failures[i]->label, buf);
  }
  fprintf(logFile, "\n");
  lastLogTime = now;
  fflush(logFile);

  checkCountDown = checkFrequency;
}

void initializeStatsLogging(char* filename, int frequency, int seconds)
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
