#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main.h"

void setUp(void) {
    // Set up code if needed
}

void tearDown(void) {
    // Tear down code if needed
}

void test_log_message(void) {
    // Redirect stdout to a buffer
    char buffer[50120];
    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
    FILE *oldStdout = stderr;
    stderr = stream;

    log_level = LOG_DEBUG;
    log_message(LOG_DEBUG, "Debug message\n");
    fflush(stream);
    TEST_ASSERT_EQUAL_STRING("Debug message\n", buffer);

    fflush(stdout);
    // Reset stdout
    stderr = oldStdout;
}

void test_main_arguments(void) {
    char *argv[] = {"program", "-d", "3", "4", "7"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    // Redirect stdout to a buffer
    char buffer[50120];
    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
    FILE *oldStdout = stderr;
    int status;
    stderr = stream;

    status = main0(argc, argv);
    fflush(stream);
    TEST_ASSERT_TRUE(strstr(buffer, "Debug mode enabled") != NULL);
    TEST_ASSERT_TRUE(strstr(buffer, "Invalid argument: 7. Must be an integer between 3 and 6.") != NULL);
    TEST_ASSERT_NOT_EQUAL_INT(0, status);

    // Reset stdout
    stderr = oldStdout;
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_log_message);
    RUN_TEST(test_main_arguments);
    printf("Hello, World!\n");
    return UNITY_END();
}