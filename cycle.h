/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef CYCLE_H
#define CYCLE_H

#include "color.h" /* For COLOR and COLORSET */

/* Forward declaration for CYCLESET from cycleset.h */
typedef uint64 *CYCLESET;

/* Type definitions */
typedef struct facialCycle *CYCLE;
/* Note: CYCLE_ID is already defined in core.h as uint64 */

/**
 * A facialCycle represents a cycle of colors that form the boundary of a face
 * in the Venn diagram.
 */
struct facialCycle {
  uint32_t length; /* Number of colors in the cycle */
  COLORSET colors; /* Bit set of colors in this cycle */

  /* Arrays used for optimized lookups during cycle matching */

  /* Pointer to an array of cyclesets, one for each edge in the cycle.
   * sameDirection[i] refers to cycles containing colors[i] followed by
   * colors[i+1] */
  CYCLESET *sameDirection;

  /* Pointer to an array of cyclesets, one for each vertex in the cycle.
   * oppositeDirection[i] refers to cycles containing the triplet:
   * colors[i-1], colors[i], colors[i+1] */
  CYCLESET *oppositeDirection;

  /* The actual sequence of colors in the cycle */
  COLOR curves[NCOLORS];
};

/* Global variables */
/* Array of all possible facial cycles used in the Venn diagram */
extern struct facialCycle Cycles[NCYCLES];

/* Cycle query functions */
/* Check if a cycle contains color a followed by color b */
extern bool cycleContainsAthenB(CYCLE cycle, uint32_t a, uint32_t b);
/* Check if a cycle contains the sequence of colors i, j, k */
extern bool cycleContainsAthenBthenC(CYCLE cycle, uint32_t i, uint32_t j,
                                     uint32_t k);
/* Find the position of a color in a cycle */
extern uint32_t cycleIndexOfColor(CYCLE cycle, COLOR color);

/* Cycle manipulation functions */
/* Get the ID of a cycle with reversed direction (keeping first element fixed)
 */
extern CYCLE_ID cycleIdReverseDirection(CYCLE_ID cycleId);
/* Get the cycle ID for a given color sequence */
extern CYCLE_ID getCycleId(COLOR *cycle, uint32_t length);
/* Convert a string of color letters to a cycle ID */
extern CYCLE_ID cycleIdFromColors(char *colors);

/* String conversion */
/* Convert a cycle to its string representation (e.g., "(abcde)") */
extern char *cycleToString(CYCLE cycle);

/* Initialization */
/* Initialize all possible cycles */
extern void initializeCycles(void);

#endif  // CYCLE_H
