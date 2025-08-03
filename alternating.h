/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef ALTERNATING_H
#define ALTERNATING_H

#include "trail.h"

/**
 * PCO macros to support both partial cyclic orders and uniform chirotopes
 *
 * These macros calculate the necessary sizes for PCO data structures
 * based on the number of lines in the arrangement.
 */

/* Default number of lines in a PCO arrangement */
#define PCO_LINES (NCOLORS * MAX_CORNERS)

/* The number of signed triples from n items: a,b,c and a,c,b are distinct. */
#define SIGNED_TRIPLES(n) (((n) * ((n) - 1) * ((n) - 2)) / 3)

/* The number of signed triples for the partial cyclic order of the lines. */
#define PCO_TRIPLES SIGNED_TRIPLES(PCO_LINES)

struct alternatingPredicate {
  int n;
  /* Extend the predicate as in Roy-Floyd-Warshall, using the trail,
   * and returning false on failure.
   */
  bool (*dynamicOneClosureStep)(AlternatingPredicate self, int i, int j, int k, int l);
  uint_trail* rawStorage;
  uint_trail** entryPointers;
};

// The {0}'s initialize the arrays to zero.
#define CREATE_ALTERNATING_PREDICATE(number, closure)        \
  &((struct alternatingPredicate){                           \
      .n = number,                                           \
      .dynamicOneClosureStep = closure,                      \
      .rawStorage = (uint_trail[SIGNED_TRIPLES(number)]){0}, \
      .entryPointers = (uint_trail * [(number) * (number) * (number)]){0}})

extern bool dynamicCyclicPartialOrderStep(AlternatingPredicate self, int i, int j, int k, int l);
extern bool dynamicChirotopeStep(AlternatingPredicate self, int i, int j, int k, int l);

#define CREATE_CYCLIC_PARTIAL_ORDER(n) \
  CREATE_ALTERNATING_PREDICATE(n, dynamicCyclicPartialOrderStep)

/*
 * Our chirotopes unusually are:
 * - partial: ? is a legal value meaning unknown/undefined
 * - uniform: 0 is not a legal value
 **/

#define CREATE_CHIROTOPE(n) CREATE_ALTERNATING_PREDICATE(n, dynamicChirotopeStep)

extern void initializePartialCyclicOrder(void);
extern void initializeAlternating(AlternatingPredicate ap);

extern AlternatingPredicate PartialCyclicOrder;

extern bool dynamicAlternatingSet(AlternatingPredicate ap, int i, int j, int k);
// Returns NULL if i == j or i == k or j == k, else pointer to true or false.
extern uint_trail* getAlternating(AlternatingPredicate ap, int i, int j, int k);
extern bool dynamicAlternatingClosure(AlternatingPredicate ap);
extern bool dynamicAlternatingComplete(AlternatingPredicate ap);
extern char* alternatingToString(AlternatingPredicate ap);

#endif /* ALTERNATING_H */
