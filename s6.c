/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "s6.h"

#include "face.h"
#include "main.h"
#include "predicates.h"
#include "statistics.h"
#include "utils.h"
#include "visible_for_testing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define TOTAL_SEQUENCE_STORAGE 100
#define DEBUG 0

/* Canonical face ordering and its inverse mapping */
static COLORSET SequenceOrder[NFACES];
static COLORSET InverseSequenceOrder[NFACES];

/* Dihedral group D_n generators (rotations and reflections) */
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

/* Comparison function for signatures, used for sorting */
static int s6SignatureCompare(const void *a, const void *b)
{
  return memcmp(a, b, sizeof(((CYCLE_ID_SEQUENCE)NULL)[0]));
}

/* Comparison function for cycle ID sequences, used for sorting */
static int compareCycleIdSequence(const void *a, const void *b)
{
  return -s6SignatureCompare(*(SIGNATURE *)a, *(SIGNATURE *)b);
}

/* Comparison function for face degree sequences, used for sorting */
static int compareFaceDegree(const void *a, const void *b)
{
  return -memcmp(a, b, sizeof(FACE_DEGREE) * NFACES);
}

/* Applies a permutation to a color value */
static COLOR colorPermute(COLOR color, PERMUTATION permutation)
{
  return (*permutation)[color];
}

/* Applies a permutation to a color set */
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

static bool faceDegreesEqual(FACE_DEGREE_SEQUENCE faceDegrees,
                             FACE_DEGREE_SEQUENCE other)
{
  return memcmp(faceDegrees, other, sizeof(faceDegrees[0])) == 0;
}

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

CYCLE_ID s6PermuteCycleId(CYCLE_ID originalCycleId, PERMUTATION permutation)
{
  COLOR permuted[NCOLORS * 2];
  CYCLE cycle = &Cycles[originalCycleId];
  COLOR min = NCOLORS;
  int minIndex = -1;

  // First apply the permutation to all colors in the cycle
  for (uint32_t i = 0; i < cycle->length; i++) {
    COLOR color = colorPermute(cycle->curves[i], permutation);
    permuted[i] = color;
    permuted[cycle->length + i] = color;  // Duplicate for cyclic search
  }

  // Find position of smallest color to normalize representation
  for (uint32_t i = 0; i < cycle->length; i++) {
    if (permuted[i] < min) {
      min = permuted[i];
      minIndex = i;
    }
  }
  return getCycleId(permuted + minIndex, cycle->length);
}

static FACE_DEGREE_SEQUENCE getFaceDegreesInCanonicalOrder()
{
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[i] = Faces[SequenceOrder[i]].cycle->length;
  }
  return faceDegrees;
}

/* Sorts multiple face degree sequences by applying the dihedral group
 * permutations */
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

/* Is sequence lexicographically maximal under dihedral symmetry? */
static SYMMETRY_TYPE isCanonicalUnderDihedralGroup(
    const int count, const FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...)
{
  FACE_DEGREE_SEQUENCE sorted;
  va_list args;
  va_start(args, faceDegreesInSequenceOrder);
  sorted = sortPermutationsOfSequence(count, faceDegreesInSequenceOrder, args);
  va_end(args);
  if (!faceDegreesEqual(sorted, faceDegreesInSequenceOrder)) {
    return NON_CANONICAL;
  }
  if (faceDegreesEqual(sorted, sorted + 1)) {
    return EQUIVOCAL;
  }
  return CANONICAL;
}

static SYMMETRY_TYPE getFullSequenceCanonicity(const FACE_DEGREE_SEQUENCE sizes)
{
  return isCanonicalUnderDihedralGroup(1, sizes);
}

static SYMMETRY_TYPE getPartialSequenceCanonicity(int n,
                                                  FACE_DEGREE_SEQUENCE args)
{
  FACE_DEGREE_SEQUENCE sizes = NEW(FACE_DEGREE_SEQUENCE);
  memset(sizes, 0, sizeof(*sizes));
  for (int i = 0; i < n; i++) {
    sizes->faceDegrees[i] = args->faceDegrees[i];
  }
  return getFullSequenceCanonicity(sizes);
}

/* Creates mirror image by reversing cycle orientations */
static SIGNATURE s6SignatureReflected(SIGNATURE sequence)
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

static SIGNATURE s6SignaturePermuted(SIGNATURE sequence,
                                     PERMUTATION permutation)
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

/* Shifts reference frame to make 'center' the new central face */
static SIGNATURE s6SignatureRecentered(SIGNATURE sequence, COLORSET center)
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

/* Finds maximum signature under all cyclic color permutations */
static SIGNATURE maxSpunSignature(SIGNATURE onCurrentFace)
{
  int counter;
  PERMUTATION abcNCycle = &dihedralGroup[1];
  SIGNATURE best = onCurrentFace;
  PERMUTATION permutation =
      s6Automorphism(onCurrentFace->classSignature.faceCycleId[0]);
  SIGNATURE permuted = s6SignaturePermuted(onCurrentFace, permutation);
  for (counter = 0; counter < NFACES; counter++) {
    assert(permuted->classSignature.faceCycleId[0] == NCYCLES - 1);
    if (s6SignatureCompare(permuted, best) > 0) {
      best = permuted;
    }
    permuted = s6SignaturePermuted(permuted, abcNCycle);
  }
  return best;
}

/* Priority order: single-color faces, then faces with colors 0 and NCOLORS-1,
 * then consecutive colors */
#define ADD_TO_SEQUENCE_ORDER(colors)               \
  do {                                              \
    SequenceOrder[ix++] = (NFACES - 1) & ~(colors); \
    done |= 1llu << (colors);                       \
  } while (0)

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

/* Computes diagram's canonical representation across all symmetries and face
 * centrings */
SIGNATURE s6MaxSignature(void)
{
  int resultsIndex = 0;
  SIGNATURE fromFaces = s6SignatureFromFaces();
  COLORSET center = 0;
  SIGNATURE recentered;
  SIGNATURE results[NFACES * 2];

  // Try each face as a potential center
  for (center = 0; center < NFACES; center++) {
    if (Faces[center].cycle->length == NCOLORS) {
      // Recenter the signature around this face
      recentered = s6SignatureRecentered(fromFaces, center);

      // Compute signatures for both orientations
      results[resultsIndex++] = maxSpunSignature(recentered);
      results[resultsIndex++] =
          maxSpunSignature(s6SignatureReflected(recentered));
    }
  }

  // Find the lexicographically maximum signature
  qsort(results, resultsIndex, sizeof(results[0]), compareCycleIdSequence);
  return results[0];
}

SYMMETRY_TYPE s6FacesSymmetryType(void)
{
  return getFullSequenceCanonicity(getFaceDegreesInCanonicalOrder());
}

SYMMETRY_TYPE s6SymmetryType6(FACE_DEGREE *args)
{
  struct faceDegreeSequence argsAsSequence = {
#if NCOLORS <= 2
      {args[0]}
#else
      {args[0], args[1], args[2], args[3], args[4], args[5]}
#endif
  };
  return getPartialSequenceCanonicity(6, &argsAsSequence);
}

char *s6FaceDegreeSignature(void)
{
  static char Result[NCOLORS + 1];
  FACE_DEGREE_SEQUENCE faceDegrees = getFaceDegreesInCanonicalOrder();
  for (int i = 0; i < NCOLORS; i++) {
    Result[i] = '0' + faceDegrees->faceDegrees[i];
  }
  Result[NCOLORS] = 0;
  return Result;
}

char *s6SignatureToString(SIGNATURE signature)
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

/* Produces human-readable signature with reflection flag, central face, and
 * full cycle details */
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
