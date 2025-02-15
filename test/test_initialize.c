#include "unity.h"
#include "../venn.h"
#include <string.h>

extern void clear();

void setUp(void)
{
}

void tearDown(void)
{  clear();
}

void test_initialize(void)
{
    uint32_t i, j;
    initialize();
    for (j = 0; j < NCYCLES; j++)
    {
        TEST_ASSERT_NOT_EQUAL(0, cycles[j].length);
        for (i = 0; i < cycles[j].length; i++)
        {
            TEST_ASSERT_NOT_EQUAL(NO_COLOR, cycles[j].curves[i]);
        }
        TEST_ASSERT_EQUAL(NO_COLOR, cycles[j].curves[cycles[j].length]);
    }
}

void test_contains2(void)
{
    CYCLE cycle = &cycles[0];
    cycle->length = 3;
    cycle->curves[0] = 0;
    cycle->curves[1] = 1;
    cycle->curves[2] = 2;
    TEST_ASSERT_TRUE(contains2(cycle, 0, 1));
    TEST_ASSERT_TRUE(contains2(cycle, 1, 2));
    TEST_ASSERT_TRUE(contains2(cycle, 2, 0));
    TEST_ASSERT_FALSE(contains2(cycle, 0, 2));
    TEST_ASSERT_FALSE(contains2(cycle, 1, 0));
    TEST_ASSERT_FALSE(contains2(cycle, 2, 1));
}

void test_contains3(void)
{
    CYCLE cycle = &cycles[0];
    cycle->length = 3;
    cycle->curves[0] = 0;
    cycle->curves[1] = 1;
    cycle->curves[2] = 2;
    TEST_ASSERT_TRUE(contains3(cycle, 0, 1, 2));
    TEST_ASSERT_TRUE(contains3(cycle, 1, 2, 0));
    TEST_ASSERT_TRUE(contains3(cycle, 2, 0, 1));
    TEST_ASSERT_FALSE(contains3(cycle, 0, 2, 1));
    TEST_ASSERT_FALSE(contains3(cycle, 1, 0, 2));
    TEST_ASSERT_FALSE(contains3(cycle, 2, 1, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initialize);
    RUN_TEST(test_contains2);
    RUN_TEST(test_contains3);
    return UNITY_END();
}