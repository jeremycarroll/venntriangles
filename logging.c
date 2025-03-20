#include "logging.h"

log_level_t log_level = LOG_INFO;

void log_message(log_level_t level, const char *format, ...)
{
  if (level >= log_level) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}
