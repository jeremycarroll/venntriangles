#ifndef MAIN_H
#define MAIN_H

typedef enum { LOG_DEBUG, LOG_VERBOSE, LOG_INFO, LOG_QUIET } log_level_t;


extern int main0(int argc, char *argv[]);
extern void log_message(log_level_t level, const char *format, ...);
extern log_level_t log_level;
#endif
