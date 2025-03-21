#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../main.h"
#include "unity.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
extern int dynamicMain0(int argc, char *argv[]);

void setUp(void)
{
  // Set up code if needed
}

void tearDown(void)
{
  // Tear down code if needed
}

void test_log_message(void)
{
  // Redirect stdout to a buffer
  char buffer[1024];
  FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
  FILE *oldStdout = stderr;
  stderr = stream;

  DynamicLogLevel = LOG_DEBUG;
  dynamicLogMessage(LOG_DEBUG, "Debug message\n");
  fflush(stream);
  TEST_ASSERT_EQUAL_STRING("Debug message\n", buffer);

  fflush(stdout);
  // Reset stdout
  stderr = oldStdout;
}

static char buffer[1024];

int run(int argc, char *argv[])
{
  FILE *oldStdout = stderr;
  FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
  int status;
  optind = 1;
  buffer[0] = 0;
  oldStdout = stderr;
  stderr = stream;
  status = dynamicMain0(argc, argv);
  fflush(stream);
  // Reset stdout
  stderr = oldStdout;
  return status;
}

void test_main_arguments(void)
{
  char *argv1[] = {"program", "-df", "foo"};
  int argc1 = sizeof(argv1) / sizeof(argv1[0]);
  char *argv2[] = {"program", "-vf", "bar"};
  int argc2 = sizeof(argv2) / sizeof(argv2[0]);
  char *argv3[] = {"program", "-qf", "baz"};
  int argc3 = sizeof(argv3) / sizeof(argv3[0]);
  char *argv4[] = {"program", "-f", "bang", "4"};
  int argc4 = sizeof(argv4) / sizeof(argv4[0]);
  char *argv5[] = {"program"};
  int argc5 = sizeof(argv5) / sizeof(argv5[0]);

  // Redirect stdout to a buffer
  TEST_ASSERT_EQUAL_INT(0, run(argc1, argv1));
  TEST_ASSERT_TRUE(strstr(buffer, "Debug mode enabled") != NULL);
  TEST_ASSERT_EQUAL_INT(0, run(argc2, argv2));
  TEST_ASSERT_TRUE(strstr(buffer, "Verbose mode enabled") != NULL);
  TEST_ASSERT_EQUAL_INT(0, run(argc3, argv3));
  TEST_ASSERT_EQUAL_CHAR(0, buffer[0]);
  TEST_ASSERT_NOT_EQUAL_INT(0, run(argc4, argv4));
  TEST_ASSERT_NOT_EQUAL_INT(0, run(argc5, argv5));
}

void dynamicSearchFull(void (*foundSolution)(void)) { /* stub for testing. */ }

const char *dynamicFaceDegreeSignature(void) { return "stub"; }

void dynamicSolutionWrite(const char *buffer) { /* stub for testing. */ }

void initializeSequenceOrder() { /* stub for testing. */ }

void initializeStatisticLogging(char *filename, int frequency, int seconds)
{ /* stub for testing. */ }

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_log_message);
  RUN_TEST(test_main_arguments);
  return UNITY_END();
}
