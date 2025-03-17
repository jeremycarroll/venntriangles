#include "main.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

log_level_t log_level = LOG_INFO;
static char *targetFolder = NULL;
extern void initializeStatsLogging(char *filename, int frequency, int seconds);

void log_message(log_level_t level, const char *format, ...)
{
  if (level >= log_level) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}
extern void full_search(void (*foundSolution)(void));
extern void initializeSequenceOrder(void);
extern char *d6FaceDegreeSignature(void);
extern void writeSolution(char *folderName);
void save_result(void)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "%s/%s", targetFolder,
           d6FaceDegreeSignature());
  writeSolution(buffer);
}

void setUpOutputFolder()
{
  struct stat st = {0};

  if (stat(targetFolder, &st) == -1) {
    // Directory does not exist, create it
    if (mkdir(targetFolder, 0700) != 0) {
      perror("Failed to create directory");
      exit(EXIT_FAILURE);
    }
  } else {
    // Directory exists, check if it is writable
    if (!S_ISDIR(st.st_mode) || access(targetFolder, W_OK) != 0) {
      fprintf(stderr, "Target folder exists but is not writable\n");
      exit(EXIT_FAILURE);
    }
  }
}

int main0(int argc, char *argv[])
{
  int opt;
  targetFolder = NULL;
  while ((opt = getopt(argc, argv, "dvqf:")) != -1) {
    switch (opt) {
      case 'd':
        log_level = LOG_DEBUG;
        break;
      case 'v':
        log_level = LOG_VERBOSE;
        break;
      case 'q':
        log_level = LOG_QUIET;
        break;
      case 'f':
        targetFolder = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s [-d] [-v] [-q] -f outputFolder\n", argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc || targetFolder == NULL) {
    fprintf(stderr, "Usage: %s [-d] [-v] [-q] -f outputFolder\n", argv[0]);
    return EXIT_FAILURE;
  }

  log_message(LOG_DEBUG, "Debug mode enabled\n");
  log_message(LOG_VERBOSE, "Verbose mode enabled\n");

  log_message(LOG_INFO, "Hello, World! %lu\n", sizeof(int *));
#if NCOLORS == 6
  initializeStatsLogging("/dev/stdout", 200, 10);
  setUpOutputFolder();
  initializeSequenceOrder();
  full_search(save_result);
#endif
  return 0;
}
