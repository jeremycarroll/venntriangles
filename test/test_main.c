#include "main.h"
#include "unity.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  char *argv1[] = {"program", "-f", "foo"};
  int argc1 = sizeof(argv1) / sizeof(argv1[0]);
  char *argv4[] = {"program", "-f", "bang", "4"};
  int argc4 = sizeof(argv4) / sizeof(argv4[0]);
  char *argv5[] = {"program"};
  int argc5 = sizeof(argv5) / sizeof(argv5[0]);

  // Redirect stdout to a buffer
  TEST_ASSERT_EQUAL_INT(0, run(argc1, argv1));
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
  RUN_TEST(test_main_arguments);
  return UNITY_END();
}
