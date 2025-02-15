#include "unity.h"
#include "../venn.h"
#include "../visible_for_testing.h"

void setUp(void)
{
}

void tearDown(void)
{
    clearGlobals();
    clearInitialize();
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

/* Tightly coupled with the order of generation. */
void test_first_cycles(void)
{
    initialize();
    TEST_ASSERT_EQUAL(0, bcmp(cycles[0].curves, (uint32_t[]){0, 1, 2, NO_COLOR}, 4 * sizeof(uint32_t)));
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
    cycle->curves[0] = 3;
    cycle->curves[1] = 4;
    cycle->curves[2] = 5;
    TEST_ASSERT_TRUE(contains3(cycle, 3, 4, 5));
    TEST_ASSERT_TRUE(contains3(cycle, 4, 5, 3));
    TEST_ASSERT_TRUE(contains3(cycle, 5, 3, 4));
    TEST_ASSERT_FALSE(contains3(cycle, 0, 2, 1));
    TEST_ASSERT_FALSE(contains3(cycle, 0, 1, 2));
    TEST_ASSERT_FALSE(contains3(cycle, 5, 4, 3));
}

void test_sizeOfSet(void)
{
    CYCLESET_DECLARE cycleSet;
    memset(cycleSet, 0, sizeof(cycleSet));
    TEST_ASSERT_EQUAL(0, sizeOfSet(cycleSet));
    cycleSet[0] = 1;
    TEST_ASSERT_EQUAL(1, sizeOfSet(cycleSet));
    cycleSet[0] = 0x8000000000000000;
    TEST_ASSERT_EQUAL(1, sizeOfSet(cycleSet));
    cycleSet[0] = 0x8000000000000001;
    TEST_ASSERT_EQUAL(2, sizeOfSet(cycleSet));
    cycleSet[0] = 0x8000000000000001;
    cycleSet[1] = 0x8000000000000001;
    TEST_ASSERT_EQUAL(4, sizeOfSet(cycleSet));
    memset(cycleSet, 0, sizeof(cycleSet));
    addToSet(NCYCLES - 1, cycleSet);
    TEST_ASSERT_EQUAL(1, sizeOfSet(cycleSet));
}

void test_cycleset(void)
{
    uint32_t i, j, k;
    initialize();
    for (i = 0; i < NCURVES; i++)
    {
        for (j = 0; j < NCURVES; j++)
        {
            int size2 = sizeOfSet(pairs2cycleSets[i][j]);
            if (i == j)
            {
                TEST_ASSERT_EQUAL(0, size2);
            }
            else
            {
                TEST_ASSERT_EQUAL(64, size2);
            }
            for (k = 0; k < NCURVES; k++)
            {
                int size3 = sizeOfSet(triples2cycleSets[i][j][k]);
                if (i == j || i == k || j == k)
                {
                    TEST_ASSERT_EQUAL(0, size3);
                }
                else
                {
                    TEST_ASSERT_EQUAL(16, size3);
                }
            }
        }
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initialize);
    RUN_TEST(test_contains2);
    RUN_TEST(test_contains3);
    RUN_TEST(test_cycleset);
    RUN_TEST(test_sizeOfSet);
    RUN_TEST(test_first_cycles);

    return UNITY_END();
}