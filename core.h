/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CORE_H
#define CORE_H
#include "memory.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * Core definitions for the Venn diagram system.
 *
 * This header defines fundamental constants, macros, and types used
 * throughout the application. It handles configuration based on the
 * number of colors (curves) in the diagram.
 */

/*--------------------------------------
 * Fundamental Constants
 *--------------------------------------*/

/* The curves are _colored_ from 0 to 5. */
#ifndef NCOLORS
#define NCOLORS 6 /* Number of colors/curves in the diagram */
#endif

/* Mathematical constants for combinatorial calculations */
#define FACTORIAL0 1u
#define FACTORIAL1 1u
#define FACTORIAL2 2u
#define FACTORIAL3 6u
#define FACTORIAL4 24u
#define FACTORIAL5 120u
#define FACTORIAL6 720u

/* Binomial coefficients for combinatorial calculations */
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

/* Total number of faces in the diagram (2^NCOLORS) */
#define NFACES (1 << NCOLORS)

/*--------------------------------------
 * Cycle Calculations
 *--------------------------------------*/

/* Cycle counts for different numbers of colors */
#define NCYCLES3 (CHOOSE_3_0 * FACTORIAL2)
#define NCYCLES4 (CHOOSE_4_0 * FACTORIAL3 + CHOOSE_4_1 * FACTORIAL2)
#define NCYCLES5 \
  (CHOOSE_5_0 * FACTORIAL4 + CHOOSE_5_1 * FACTORIAL3 + CHOOSE_5_2 * FACTORIAL2)

/* Configure cycle counts based on NCOLORS value */
#if NCOLORS == 6
#define NCYCLES                                        \
  (CHOOSE_6_0 * FACTORIAL5 + CHOOSE_6_1 * FACTORIAL4 + \
   CHOOSE_6_2 * FACTORIAL3 + CHOOSE_6_3 * FACTORIAL2)
#define NCYCLE_ENTRIES                                         \
  (CHOOSE_6_0 * FACTORIAL5 * 6 + CHOOSE_6_1 * FACTORIAL4 * 5 + \
   CHOOSE_6_2 * FACTORIAL3 * 4 + CHOOSE_6_3 * FACTORIAL2 * 3)
#elif NCOLORS == 4
#define NCYCLES NCYCLES4
#define NCYCLE_ENTRIES (CHOOSE_4_0 * FACTORIAL4 + CHOOSE_4_1 * FACTORIAL3)
#elif NCOLORS == 5
#define NCYCLES NCYCLES5
#define NCYCLE_ENTRIES \
  (CHOOSE_5_0 * FACTORIAL5 + CHOOSE_5_1 * FACTORIAL4 + CHOOSE_5_2 * FACTORIAL3)
#else
#define NCYCLES NCYCLES3
#define NCYCLE_ENTRIES (CHOOSE_3_0 * FACTORIAL3)
#endif

/* Bit manipulation for cycle sets */
#define BITS_PER_WORD (sizeof(void *) * 8)
#define CYCLESET_LENGTH ((NCYCLES - 1) / BITS_PER_WORD + 1)

/* Maximum number of vertices in the diagram */
#define NPOINTS ((1 << (NCOLORS - 2)) * NCOLORS * (NCOLORS - 1))

/*--------------------------------------
 * Memory Annotation Macros
 *--------------------------------------*/

/**
 * MEMO marks fields that are initialized once and never change.
 * These require no trail/undo support during backtracking.
 */
#ifndef MEMO
#define MEMO
#endif

/**
 * DYNAMIC marks fields that may change during search and require
 * trail support for backtracking.
 */
#ifndef DYNAMIC
#define DYNAMIC
#endif

/*--------------------------------------
 * Utility Macros
 *--------------------------------------*/

/* Maximum corners per curve - the "triangles" in the project name. */
#define MAX_CORNERS 3

/* Convert value to string at compile time */
#define STRINGIFY(x) #x

/* Calculate array length at compile time */
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

/*--------------------------------------
 * Type Definitions
 *--------------------------------------*/

/* Define our own 64-bit unsigned integer type, to avoid platform dependent
 * types. */
typedef unsigned long long uint64;

/* Forward declarations for key data structures */
typedef struct edge *EDGE;
typedef struct face *FACE;
typedef struct alternatingPredicate *AlternatingPredicate;

/* Type aliases for semantic clarity */
typedef uint64 FACE_DEGREE; /* Degree (number of edges) of a face */
typedef uint64 CYCLE_ID;    /* Unique identifier for a cycle */

#endif  // CORE_H
