/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#define _GNU_SOURCE

#include "main.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

/* External declarations */
#pragma GCC diagnostic ignored "-Wunused-parameter"
extern int dynamicMain0(int argc, char *argv[]);
static bool DisasterCalled = false;

/* Test setup and teardown */
void setUp(void)
{
  // Set up code if needed
}

void tearDown(void)
{
  // Tear down code if needed
}

/* Helper functions */
static int run(int argc, char *argv[])
{
  FILE *oldStdout = stderr;
  char buffer[1024];
  buffer[0] = 0;
  FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
  int status;
  DisasterCalled = false;
  optind = 1;
  oldStdout = stderr;
  stderr = stream;
  status = dynamicMain0(argc, argv);
  fflush(stream);
  // Reset stdout
  stderr = oldStdout;
  return DisasterCalled ? 1 : status;
}

/* Test functions */
static void testMainArguments(void)
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

/* Main test runner */
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(testMainArguments);
  return UNITY_END();
}

/* Stub functions for testing */
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

char *usingBuffer(char *buffer)
{
  return NULL;
}
void statisticPrintFull(void)
{ /* stub for testing. */
}

void initializeS6()
{ /* stub for testing. */
}

void initializeStatisticLogging(char *filename, int frequency, int seconds)
{ /* stub for testing. */
}

void searchFull(void (*foundSolution)(void))
{ /* stub for testing. */
}

char *s6FaceDegreeSignature(void)
{
  return "stub";
}

void searchSolutionWrite(const char *buffer)
{ /* stub for testing. */
}

void writeSolution(void)
{ /* stub for testing. */
}

struct predicate *nonDeterministicProgram[] = {/* stub for testing. */};

void engine(struct predicate *predicates[], void (*callback)(void))
{
  /* stub for testing. */
}

char *getBuffer()
{
  return NULL;
}
void initializeFolder(const char *folder) {}
void disaster(const char *message)
{
  DisasterCalled = true;
}
