/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "s6.h"

#include "engine.h"
#include "face.h"
#include "main.h"
#include "memory.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables - file scoped */
/* Lemma: In a simple Venn diagram of convex curves, the faces inside k curves
 * have total face degree being 2 * nCk + nC(k-1), where nCk is the binomial
 * coefficient "n choose k". For n = 6 and k = 5, this equals 2 * 6 + 15 = 27.
 * This lemma is part of an ongoing proof about the structure of Venn diagrams.
 */
#define TOTAL_5FACE_DEGREE 27

#define TOTAL_SEQUENCE_STORAGE 100
#define DEBUG 0

static COLORSET SequenceOrder[NFACES];
static COLORSET InverseSequenceOrder[NFACES];
static int dihedralGroup[2 * NCOLORS][NCOLORS] = {
#if NCOLORS == 6
    {0, 1, 2, 3, 4, 5},
    {1, 2, 3, 4, 5, 0},
    {2, 3, 4, 5, 0, 1},
    {3, 4, 5, 0, 1, 2},
    {4, 5, 0, 1, 2, 3},
    {5, 0, 1, 2, 3, 4},
    {5, 4, 3, 2, 1, 0},
    {4, 3, 2, 1, 0, 5},
    {3, 2, 1, 0, 5, 4},
    {2, 1, 0, 5, 4, 3},
    {1, 0, 5, 4, 3, 2},
    {0, 5, 4, 3, 2, 1}
#elif NCOLORS == 4
    {0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2}, {3, 2, 1, 0},
    {2, 1, 0, 3}, {1, 0, 3, 2}, {0, 3, 2, 1}
#elif NCOLORS == 5
    {0, 1, 2, 3, 4}, {1, 2, 3, 4, 0}, {2, 3, 4, 0, 1}, {3, 4, 0, 1, 2},
    {4, 0, 1, 2, 3}, {4, 3, 2, 1, 0}, {3, 2, 1, 0, 4}, {2, 1, 0, 4, 3},
    {1, 0, 4, 3, 2}, {0, 4, 3, 2, 1}
#endif
};

/* Declaration of file scoped static functions */
static int compareFaceDegree(const void *a, const void *b);
static SYMMETRY_TYPE d6SymmetryType64(FACE_DEGREE_SEQUENCE sizes);
static SYMMETRY_TYPE d6SymmetryTypeN(int n, FACE_DEGREE_SEQUENCE args);
static FACE_DEGREE_SEQUENCE d6FaceDegreesInSequenceOrder(void);
static FACE_DEGREE_SEQUENCE sortPermutationsOfSequence(
    const int count, const FACE_DEGREE_SEQUENCE first, va_list moreSequences);
static COLOR colorPermute(COLOR color, PERMUTATION permutation);
static COLORSET colorSetPermute(COLORSET colorSet, PERMUTATION permutation);
static bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees,
                    FACE_DEGREE_SEQUENCE other);
static SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    const int count, const FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder,
    ...);
static SIGNATURE maxSpunSignature(SIGNATURE onCurrentFace);
static int compareCycleIdSequence(const void *a, const void *b);
static SIGNATURE d6SignatureRecentered(SIGNATURE sequence, COLORSET center);
static SIGNATURE d6SignaturePermuted(SIGNATURE sequence,
                                     PERMUTATION permutation);
static int d6SignatureCompare(const void *a, const void *b);
static SIGNATURE d6SignatureReflected(SIGNATURE sequence);
extern struct predicate facePredicate;
extern struct predicate solutionWritePredicate;
extern struct predicate cornersPredicate;
extern struct predicate saveVariationPredicate;
extern struct predicate failPredicate;

#define ADD_TO_SEQUENCE_ORDER(colors)               \
  do {                                              \
    SequenceOrder[ix++] = (NFACES - 1) & ~(colors); \
    done |= 1llu << (colors);                       \
  } while (0)

static void verifyS6Initialization(uint64 done, uint64 ix);
/* Externally linked functions - Initialize */
void initializeS6(void)
{
  uint64 ix = 0, i;
  uint64 done = 0;

  for (i = 0; i < NCOLORS; i++) {
    ADD_TO_SEQUENCE_ORDER(1llu << i);
  }
  ADD_TO_SEQUENCE_ORDER(((1llu << (NCOLORS - 1)) | 1llu));
  for (i = 0; i < NCOLORS - 1; i++) {
    ADD_TO_SEQUENCE_ORDER((2llu | 1llu) << i);
  }
  for (i = 0; i < NFACES; i++) {
    if ((done & (1llu << i)) != 0) {
      continue;
    }
    ADD_TO_SEQUENCE_ORDER(i);
  }
  for (i = 0; i < NFACES; i++) {
    InverseSequenceOrder[SequenceOrder[i]] = i;
  }
  verifyS6Initialization(done, ix);
}

/* Externally linked functions - Dynamic */
PERMUTATION s6Automorphism(CYCLE_ID cycleId)
{
  CYCLE cycle = Cycles + cycleId;
  PERMUTATION result = NEW(PERMUTATION);
  assert(cycle->length == NCOLORS);
  for (int i = 0; i < NCOLORS; i++) {
    (*result)[cycle->curves[i]] = i;
  }
  return result;
}

/* Externally linked functions - Reset */
// No reset functions in this file

/* Externally linked functions - Other */
CYCLE_ID s6PermuteCycleId(CYCLE_ID originalCycleId, PERMUTATION permutation)
{
  COLOR permuted[NCOLORS * 2];
  CYCLE cycle = &Cycles[originalCycleId];
  COLOR min = NCOLORS;
  int minIndex = -1;
  for (uint32_t i = 0; i < cycle->length; i++) {
    COLOR color = colorPermute(cycle->curves[i], permutation);
    permuted[i] = color;
    permuted[cycle->length + i] = color;
  }
  for (uint32_t i = 0; i < cycle->length; i++) {
    if (permuted[i] < min) {
      min = permuted[i];
      minIndex = i;
    }
  }
  return getCycleId(permuted + minIndex, cycle->length);
}

SYMMETRY_TYPE s6SymmetryType6(FACE_DEGREE *args)
{
  struct faceDegreeSequence argsAsSequence = {
      {args[0], args[1], args[2], args[3], args[4], args[5]}};
  return d6SymmetryTypeN(6, &argsAsSequence);
}

SYMMETRY_TYPE s6FacesSymmetryType(void)
{
  return d6SymmetryType64(d6FaceDegreesInSequenceOrder());
}

char *s6FaceDegreeSignature(void)
{
  static char Result[NCOLORS + 1];
  FACE_DEGREE_SEQUENCE faceDegrees = d6FaceDegreesInSequenceOrder();
  for (int i = 0; i < NCOLORS; i++) {
    Result[i] = '0' + faceDegrees->faceDegrees[i];
  }
  Result[NCOLORS] = 0;
  return Result;
}

/* Static variables for the engine-based callback */
static void (*staticCallback)(void *, FACE_DEGREE *);
static void *staticCallbackData;
static FACE_DEGREE currentArgs[NCOLORS];

static int sumFaceDegree(int round)
{
  int sum = 0;
  for (int i = 0; i < round; i++) {
    sum += currentArgs[i];
  }
  return sum;
}

/* Predicate for finding 5-face degree sequences */
static struct predicateResult try5FaceDegree(int round)
{
  if (round == NCOLORS) {
    if (sumFaceDegree(round) != TOTAL_5FACE_DEGREE) {
      return PredicateFail;
    }
    if (s6SymmetryType6(currentArgs) == NON_CANONICAL) {
      return PredicateFail;
    }
    return PredicateSuccessNextPredicate;
  }
  return predicateChoices(NCOLORS - 2, NULL);
}

static struct predicateResult retry5FaceDegree(int round, int choice)
{
  // Try each possible face degree >= 3 for the current position
  int degree = NCOLORS - choice;  // Start with NCOLORS and go down to 3
  if (degree < 3) {
    return PredicateFail;
  }

  if (CentralFaceDegrees[round] > 0 && degree != CentralFaceDegrees[round]) {
    return PredicateFail;  // Skip if there's a specific degree required for
                           // this position
  }
  // Try this degree
  currentArgs[round] = degree;

  if (sumFaceDegree(round + 1) + 3 * (NCOLORS - round - 1) >
      TOTAL_5FACE_DEGREE) {
    return PredicateFail;  // Exceeded target sum
  }

  return PredicateSuccessSamePredicate;
}

/* Static callback for the engine */
static void engineCallback(void)
{
  staticCallback(staticCallbackData, currentArgs);
}
extern struct predicate initializePredicate;
/* The predicates array for 5-face degree sequence search */
struct predicate faceDegreePredicate = {"face6Degree", try5FaceDegree,
                                        retry5FaceDegree};

void s6FaceDegreeCanonicalCallback(void (*callback)(void *, FACE_DEGREE *),
                                   void *data)
{
  // staticCallback = callback;
  // staticCallbackData = data;
  // engine(predicates, NULL);
}

SIGNATURE s6MaxSignature(void)
{
  int resultsIndex = 0;
  SIGNATURE fromFaces = s6SignatureFromFaces();
  COLORSET center = 0;
  SIGNATURE recentered;
  SIGNATURE results[NFACES * 2];
  for (center = 0; center < NFACES; center++) {
    if (Faces[center].cycle->length == NCOLORS) {
      recentered = d6SignatureRecentered(fromFaces, center);
      results[resultsIndex++] = maxSpunSignature(recentered);
      results[resultsIndex++] =
          maxSpunSignature(d6SignatureReflected(recentered));
    }
  }
  qsort(results, resultsIndex, sizeof(results[0]), compareCycleIdSequence);
  return results[0];
}

SIGNATURE d6SignatureRecentered(SIGNATURE sequence, COLORSET center)
{
  SIGNATURE result = NEW(SIGNATURE);
  for (int i = 0; i < NFACES; i++) {
    result->classSignature.faceCycleId[i] =
        sequence->classSignature.faceCycleId[i ^ center];
  }
  result->offset = center ^ sequence->offset;
  result->reflected = sequence->reflected;
  return result;
}

SIGNATURE d6SignaturePermuted(SIGNATURE sequence, PERMUTATION permutation)
{
  SIGNATURE result = NEW(SIGNATURE);
  for (COLORSET i = 0; i < NFACES; i++) {
    result->classSignature.faceCycleId[colorSetPermute(i, permutation)] =
        s6PermuteCycleId(sequence->classSignature.faceCycleId[i], permutation);
  }
  result->offset = colorSetPermute(sequence->offset, permutation);
  result->reflected = sequence->reflected;
  return result;
}

SIGNATURE d6SignatureReflected(SIGNATURE sequence)
{
  SIGNATURE result = NEW(SIGNATURE);
  for (int i = 0; i < NFACES; i++) {
    result->classSignature.faceCycleId[i] =
        cycleIdReverseDirection(sequence->classSignature.faceCycleId[i]);
  }
  result->offset = sequence->offset;
  result->reflected = !sequence->reflected;
  return result;
}

int d6SignatureCompare(const void *a, const void *b)
{
  return memcmp(a, b, sizeof(((CYCLE_ID_SEQUENCE)NULL)[0]));
}

SIGNATURE s6SignatureFromFaces(void)
{
  SIGNATURE result = NEW(SIGNATURE);
  for (int i = 0; i < NFACES; i++) {
    result->classSignature.faceCycleId[i] = Faces[i].cycle - Cycles;
  }
  result->offset = NFACES - 1;
  result->reflected = false;
  return result;
}

char *d6SignatureToString(SIGNATURE signature)
{
  char *result = getBuffer();
  char *p = result;
  for (int i = 0; i < NFACES; i++) {
    *p++ = 'A' + signature->classSignature.faceCycleId[i] / 26;
    *p++ = 'a' + signature->classSignature.faceCycleId[i] % 26;
  }
  *p = '\0';
  return usingBuffer(result);
}

char *s6SignatureToLongString(SIGNATURE signature)
{
  char *result = tempMalloc(1024);
  char *p = result;
  p += sprintf(p, "%c%s:", signature->reflected ? '!' : ' ',
               colorSetToString(signature->offset));
  for (int i = 0; i < NFACES; i++) {
    p += sprintf(
        p, " %s",
        cycleToString(Cycles + signature->classSignature.faceCycleId[i]));
  }
  *p = '\0';
  return result;
}

/* File scoped static functions */

static void verifyS6Initialization(uint64 done, uint64 ix)
{
  uint64 i;
#if NCOLORS == 6
  assert(done == ~0llu);
#else
  assert(done == (1llu << (uint64)NFACES) - 1l);
#endif
  assert(ix == NFACES);
  for (i = 0; i < NFACES; i++) {
    assert(InverseSequenceOrder[SequenceOrder[i]] == i);
    assert(SequenceOrder[InverseSequenceOrder[i]] == i);
  }
}

static int compareFaceDegree(const void *a, const void *b)
{
  return -memcmp(a, b, sizeof(FACE_DEGREE) * NFACES);
}

static COLOR colorPermute(COLOR color, PERMUTATION permutation)
{
  return (*permutation)[color];
}

static COLORSET colorSetPermute(COLORSET colorSet, PERMUTATION permutation)
{
  COLORSET result = 0;
  for (COLOR color = 0; color < NCOLORS; color++) {
    if (COLORSET_HAS_MEMBER(color, colorSet)) {
      result |= 1u << (*permutation)[color];
    }
  }
  return result;
}

static bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees,
                    FACE_DEGREE_SEQUENCE other)
{
  return memcmp(faceDegrees, other, sizeof(faceDegrees[0])) == 0;
}

static SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    const int count, const FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...)
{
  FACE_DEGREE_SEQUENCE sorted;
  va_list args;
  va_start(args, faceDegreesInSequenceOrder);
  sorted = sortPermutationsOfSequence(count, faceDegreesInSequenceOrder, args);
  va_end(args);
  if (!d6Equal(sorted, faceDegreesInSequenceOrder)) {
    return NON_CANONICAL;
  }
  if (d6Equal(sorted, sorted + 1)) {
    return EQUIVOCAL;
  }
  return CANONICAL;
}

static SIGNATURE maxSpunSignature(SIGNATURE onCurrentFace)
{
  int counter;
  PERMUTATION abcNCycle = &dihedralGroup[1];
  SIGNATURE best = onCurrentFace;
  PERMUTATION permutation =
      s6Automorphism(onCurrentFace->classSignature.faceCycleId[0]);
  SIGNATURE permuted = d6SignaturePermuted(onCurrentFace, permutation);
  for (counter = 0; counter < NFACES; counter++) {
    assert(permuted->classSignature.faceCycleId[0] == NCYCLES - 1);
    if (d6SignatureCompare(permuted, best) > 0) {
      best = permuted;
    }
    permuted = d6SignaturePermuted(permuted, abcNCycle);
  }
  return best;
}

static int compareCycleIdSequence(const void *a, const void *b)
{
  return -d6SignatureCompare(*(SIGNATURE *)a, *(SIGNATURE *)b);
}

static FACE_DEGREE_SEQUENCE sortPermutationsOfSequence(
    const int count, const FACE_DEGREE_SEQUENCE first, va_list moreSequences)
{
  int i;
  FACE_DEGREE_SEQUENCE sequences =
      NEW_ARRAY(FACE_DEGREE_SEQUENCE, count * 2 * NCOLORS);
  FACE_DEGREE_SEQUENCE next = first;
  for (i = 0; i < count; i++) {
    for (int j = 0; j < 2 * NCOLORS; j++) {
      for (int k = 0; k < NFACES; k++) {
        sequences[i * 2 * NCOLORS + j].faceDegrees[k] =
            next->faceDegrees[InverseSequenceOrder[colorSetPermute(
                SequenceOrder[k], &dihedralGroup[j])]];
      }
    }
    if (count - i > 1) {
      next = va_arg(moreSequences, FACE_DEGREE_SEQUENCE);
    }
  }
  qsort(sequences, 2 * NCOLORS * count, sizeof(sequences[0]),
        compareFaceDegree);
  return sequences;
}

static SYMMETRY_TYPE d6SymmetryType64(const FACE_DEGREE_SEQUENCE sizes)
{
  return d6IsMaxInSequenceOrder(1, sizes);
}

static SYMMETRY_TYPE d6SymmetryTypeN(int n, FACE_DEGREE_SEQUENCE args)
{
  FACE_DEGREE_SEQUENCE sizes = NEW(FACE_DEGREE_SEQUENCE);
  memset(sizes, 0, sizeof(*sizes));
  for (int i = 0; i < n; i++) {
    sizes->faceDegrees[i] = args->faceDegrees[i];
  }
  return d6SymmetryType64(sizes);
}

static FACE_DEGREE_SEQUENCE d6FaceDegreesInSequenceOrder()
{
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[i] = Faces[SequenceOrder[i]].cycle->length;
  }
  return faceDegrees;
}
