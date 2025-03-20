#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>
#include <stdio.h>

typedef enum {
  LOG_DEBUG,
  LOG_VERBOSE,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_QUIET
} log_level_t;

extern log_level_t log_level;

void log_message(log_level_t level, const char *format, ...);

#endif  // LOGGING_H
