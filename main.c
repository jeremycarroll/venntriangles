#include "main.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

log_level_t log_level = LOG_QUIET;

void log_message(log_level_t level, const char *format, ...)
{
  if (level >= log_level) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

int main0(int argc, char *argv[])
{
  int opt;
  int args[6] = {0};
  int arg_count = 0;

  while ((opt = getopt(argc, argv, "dvq")) != -1) {
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
      default:
        fprintf(stderr, "Usage: %s [-d] [-v] [-q] [integers...]\n", argv[0]);
        return EXIT_FAILURE;
    }
  }

  for (int i = optind; i < argc && arg_count < 6; i++) {
    int value = atoi(argv[i]);
    if (value >= 3 && value <= 6) {
      args[arg_count++] = value;
    } else {
      fprintf(stderr,
              "Invalid argument: %s. Must be an integer between 3 and 6.\n",
              argv[i]);
      return EXIT_FAILURE;
    }
  }

  log_message(LOG_DEBUG, "Debug mode enabled\n");
  log_message(LOG_VERBOSE, "Verbose mode enabled\n");

  log_message(LOG_VERBOSE, "Received %d arguments:\n", arg_count);
  for (int i = 0; i < arg_count; i++) {
    log_message(LOG_VERBOSE, "Argument %d: %d\n", i + 1, args[i]);
  }

  printf("Hello, World! %lu\n", sizeof(int *));
  return 0;
}
