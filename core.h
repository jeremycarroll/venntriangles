/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CORE_H
#define CORE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

#define BITS_PER_WORD (sizeof(void *) * 8)
#define CYCLESET_LENGTH ((NCYCLES - 1) / BITS_PER_WORD + 1)

#define NPOINTS ((1 << (NCOLORS - 2)) * NCOLORS * (NCOLORS - 1))

#ifndef STATIC
#define STATIC
#endif
#ifndef DYNAMIC
#define DYNAMIC
#endif

#define MAX_CORNERS 3

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct edge *EDGE;

typedef struct face *FACE;
typedef uint64_t FACE_DEGREE;

#endif  // CORE_H
