#include "logging.h"

log_level_t DynamicLogLevel = LOG_INFO;

void dynamicLogMessage(log_level_t level, const char *format, ...)
{
  if (level >= DynamicLogLevel) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}
