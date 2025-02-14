
#include "face.h"
#include "gloabls.h"

static char * solution[] = {
/* 0:  */ "adfecb",
/* 1: a */ "abed",
/* 2: b */ "abcf",
/* 3: ab */ "afceb",
/* 4: c */ "adbce",
/* 5: ac */ "aed",
/* 6: bc */ "bdfc",
/* 7: abc */ "cfe",
/* 8: d */ "abfd",
/* 9: ad */ "adeb",
/* 10: bd */ "afb",
/* 11: abd */ "abef",
/* 12: cd */ "abd",
/* 13: acd */ "adeb",
/* 14: bcd */ "afdb",
/* 15: abcd */ "befd",
/* 16: e */ "bcefd",
/* 17: ae */ "bde",
/* 18: be */ "bdfc",
/* 19: abe */ "becfd",
/* 20: ce */ "aecbd",
/* 21: ace */ "ade",
/* 22: bce */ "bcfd",
/* 23: abce */ "cef",
/* 24: de */ "bdf",
/* 25: ade */ "bed",
/* 26: bde */ "bfd",
/* 27: abde */ "bdfe",
/* 28: cde */ "adb",
/* 29: acde */ "abed",
/* 30: bcde */ "abdf",
/* 31: abcde */ "afeb",
/* 32: f */ "aefdc",
/* 33: af */ "ace",
/* 34: bf */ "afc",
/* 35: abf */ "acf",
/* 36: cf */ "acd",
/* 37: acf */ "adec",
/* 38: bcf */ "acfd",
/* 39: abcf */ "adefc",
/* 40: df */ "acdfb",
/* 41: adf */ "abec",
/* 42: bdf */ "abf",
/* 43: abdf */ "afeb",
/* 44: cdf */ "adc",
/* 45: acdf */ "aced",
/* 46: bcdf */ "adf",
/* 47: abcdf */ "afed",
/* 48: ef */ "acdfe",
/* 49: aef */ "aec",
/* 50: bef */ "cfd",
/* 51: abef */ "cdf",
/* 52: cef */ "adc",
/* 53: acef */ "aced",
/* 54: bcef */ "cdf",
/* 55: abcef */ "cfed",
/* 56: def */ "bfdc",
/* 57: adef */ "bce",
/* 58: bdef */ "bcdf",
/* 59: abdef */ "befdc",
/* 60: cdef */ "abcd",
/* 61: acdef */ "adecb",
/* 62: bcdef */ "afdcb",
/* 63: abcdef */ "abcdef"
};

void test_known_solution(void) {
    int redundant = 0;
    for (int i = 0; i < 64; i++) {
        switch (face[i].count) {
            case 1:
                redundant ++;
                break;
            case 0:
                TEST_ASSERT_FALSE("Solution is inconsistent");
                break;
        }
        TEST_ASSERT_TRUE(setCycle(&face[i], findCycle(solution[i])));
    }
    TEST_ASSERT_EQUAL_INT(20, redundant);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_known_solution);
    return UNITY_END();
}