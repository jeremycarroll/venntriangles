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

#define TOTAL_SEQUENCE_STORAGE 100
#define DEBUG 0

/* SequenceOrder provides a canonical ordering of the faces to help detect
   symmetries. InverseSequenceOrder maps back from the canonical order to the
   original order. */
static COLORSET SequenceOrder[NFACES];
static COLORSET InverseSequenceOrder[NFACES];

/* Generators of the dihedral group D_n where n = NCOLORS:
   - Rotations: (0,1,2,...,n-1), (1,2,3,...,n-1,0), etc.
   - Reflections: (n-1,n-2,...,1,0), (n-2,n-3,...,0,n-1), etc. */
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

/* Byte-for-byte comparison of face degree sequences */
static bool faceDegreesEqual(FACE_DEGREE_SEQUENCE faceDegrees,
                             FACE_DEGREE_SEQUENCE other)
{
  return memcmp(faceDegrees, other, sizeof(faceDegrees[0])) == 0;
}

/* Verifies the initialization of S6 data structures */
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

/*
 * Converts a cycle to its automorphism representation.
 *
 * This builds a permutation that maps each color to its position in the cycle,
 * which is useful for transforming diagrams to their canonical forms.
 */
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

/*
 * Applies a color permutation to a cycle and normalizes its representation.
 *
 * This transforms a cycle by applying a permutation to all its colors, then
 * finds the canonical representation by selecting the cyclic shift that
 * starts with the smallest color value.
 */
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

/* Maps face data into canonical sequence for symmetry detection */
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

/*
 * Tests if a sequence is canonical (maximal) under the dihedral group.
 *
 * This is the core algorithm for detecting if a diagram is in canonical form,
 * by checking if the given sequence is the lexicographically largest
 * under all dihedral transformations.
 */
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

/* Wrapper for canonicity testing of a complete face degree sequence */
static SYMMETRY_TYPE testFullSequenceCanonicity(
    const FACE_DEGREE_SEQUENCE sizes)
{
  return isCanonicalUnderDihedralGroup(1, sizes);
}

/*
 * Adapts canonicity testing for a sequence with fewer than NFACES elements.
 * Used for partial sequences like those surrounding a central face.
 */
static SYMMETRY_TYPE testPartialSequenceCanonicity(int n,
                                                   FACE_DEGREE_SEQUENCE args)
{
  FACE_DEGREE_SEQUENCE sizes = NEW(FACE_DEGREE_SEQUENCE);
  memset(sizes, 0, sizeof(*sizes));
  for (int i = 0; i < n; i++) {
    sizes->faceDegrees[i] = args->faceDegrees[i];
  }
  return testFullSequenceCanonicity(sizes);
}

/* Reflects a signature by reversing the direction of all cycles */
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

/* Permutes a signature according to the given permutation */
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

/* Recenters a signature around a different face */
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

/* Computes the maximum signature under cyclic permutations */
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

/*
 * Initializes the S6 module with a canonical ordering of faces.
 *
 * The ordering prioritizes:
 * 1. Faces with single colors
 * 2. Face with colors 0 and NCOLORS-1
 * 3. Faces with consecutive colors
 * 4. Other faces in arbitrary order
 */
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

/* Creates a signature from the current face configuration */
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

/*
 * Finds the canonical signature for the current diagram under all symmetries.
 *
 * This is a key function in the library that finds the canonical representation
 * of a Venn diagram by:
 * 1. Trying each face as the central face
 * 2. Computing signatures for both orientations (regular and reflected)
 * 3. Finding the lexicographically maximum signature across all possibilities
 *
 * The canonical signature uniquely identifies isomorphic Venn diagrams
 * regardless of their drawing or labeling.
 */
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

/*
 * Tests if the current diagram is in canonical form under the dihedral group.
 *
 * Returns one of:
 * - CANONICAL: the current diagram is the canonical representative
 * - NON_CANONICAL: another view of this diagram would be canonical
 * - EQUIVOCAL: multiple views yield the same canonical form (diagram has
 * symmetry)
 */
SYMMETRY_TYPE s6FacesSymmetryType(void)
{
  return testFullSequenceCanonicity(getFaceDegreesInCanonicalOrder());
}

/*
 * Tests if a sequence of 6 face degrees is canonical.
 *
 * This function is primarily used for testing sequences around central faces
 * to determine if they form valid Venn diagrams.
 */
SYMMETRY_TYPE s6SymmetryType6(FACE_DEGREE *args)
{
  struct faceDegreeSequence argsAsSequence = {
      {args[0], args[1], args[2], args[3], args[4], args[5]}};
  return testPartialSequenceCanonicity(6, &argsAsSequence);
}

/*
 * Generates a simple string signature of face degrees for the current diagram.
 *
 * This provides a compact representation of the diagram's structure
 * that can be used for identification and comparison.
 */
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

/*
 * Encodes a signature as a base-26×26 string, with each cycle ID
 * represented by a two-character sequence.
 */
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

/*
 * Provides a human-readable description of a signature, including:
 * - Whether the signature is reflected (marked with '!')
 * - The offset (central face)
 * - The full cycle representation for each face
 */
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
