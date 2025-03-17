#ifndef CORE_H
#define CORE_H

/* The curves are _colored_ from 0 to 5. */
#ifndef NCOLORS
#define NCOLORS 6
#endif

#define FACTORIAL0 1u
#define FACTORIAL1 1u
#define FACTORIAL2 2u
#define FACTORIAL3 6u
#define FACTORIAL4 24u
#define FACTORIAL5 120u
#define CHOOSE_6_0 1u
#define CHOOSE_6_1 6u
#define CHOOSE_6_2 15u
#define CHOOSE_6_3 20u
#define CHOOSE_5_0 1u
#define CHOOSE_5_1 5u
#define CHOOSE_5_2 10u
#define CHOOSE_4_0 1u
#define CHOOSE_4_1 4u
#define CHOOSE_3_0 1u
#define NFACES (1 << NCOLORS)
// #define NPOINTS (NFACES - 2)
#if NCOLORS == 6
#define NCYCLES                                        \
  (CHOOSE_6_0 * FACTORIAL5 + CHOOSE_6_1 * FACTORIAL4 + \
   CHOOSE_6_2 * FACTORIAL3 + CHOOSE_6_3 * FACTORIAL2)
#define NCYCLE_ENTRIES                                         \
  (CHOOSE_6_0 * FACTORIAL5 * 6 + CHOOSE_6_1 * FACTORIAL4 * 5 + \
   CHOOSE_6_2 * FACTORIAL3 * 4 + CHOOSE_6_3 * FACTORIAL2 * 3)
#elif NCOLORS == 4
#define NCYCLES (CHOOSE_4_0 * FACTORIAL3 + CHOOSE_4_1 * FACTORIAL2)
#define NCYCLE_ENTRIES (CHOOSE_4_0 * FACTORIAL4 + CHOOSE_4_1 * FACTORIAL3)
#elif NCOLORS == 5
#define NCYCLES \
  (CHOOSE_5_0 * FACTORIAL4 + CHOOSE_5_1 * FACTORIAL3 + CHOOSE_5_2 * FACTORIAL2)
#define NCYCLE_ENTRIES \
  (CHOOSE_5_0 * FACTORIAL5 + CHOOSE_5_1 * FACTORIAL4 + CHOOSE_5_2 * FACTORIAL3)
#else
#define NCYCLES (CHOOSE_3_0 * FACTORIAL2)
#define NCYCLE_ENTRIES (CHOOSE_3_0 * FACTORIAL3)
#endif


/*
There are NFACES - 2 points, by Euler.
These are chosen from a rather larger set: each point is defined
by  the two colors crossing at that point, and then the 2^4 possible faces
of the other colors being the inner face. We wire those points up in advance.
The two colors are ordered, first the one crossing from inside to outside, then
the other.
*/
#define NPOINTS ((1 << (NCOLORS - 2)) * NCOLORS * (NCOLORS - 1))

#endif