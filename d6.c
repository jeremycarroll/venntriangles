/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"

#include "face.h"
#include "memory.h"

#include <stdarg.h>

typedef int PERMUTATION[NCOLORS];

/* Global variables - file scoped */
#define TOTAL_5FACE_DEGREE 27

#define TOTAL_SEQUENCE_STORAGE 100

static COLORSET SequenceOrder[NFACES];
static COLORSET InverseSequenceOrder[NFACES];
static PERMUTATION group[] = {
#if NCOLORS == 6
    {0, 1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 0}, {2, 3, 4, 5, 0, 1},
    {3, 4, 5, 0, 1, 2}, {4, 5, 0, 1, 2, 3}, {5, 0, 1, 2, 3, 4},
    {5, 4, 3, 2, 1, 0}, {4, 3, 2, 1, 0, 5}, {3, 2, 1, 0, 5, 4},
    {2, 1, 0, 5, 4, 3}, {1, 0, 5, 4, 3, 2}, {0, 5, 4, 3, 2, 1},
#endif
};

/* Declaration of file scoped static functions */
static int compareUint8(const void *a, const void *b);
static SYMMETRY_TYPE d6SymmetryType64(FACE_DEGREE_SEQUENCE sizes);
static SYMMETRY_TYPE d6SymmetryTypeN(int n, FACE_DEGREE_SEQUENCE args);
static FACE_DEGREE_SEQUENCE d6FaceDegreesInSequenceOrder(void);
static void canoncialCallbackImpl(int depth, int sum, FACE_DEGREE *args,
                                  UseFaceDegrees callback, void *data);

// static COLOR colorPermute(COLOR color, PERMUTATION permutation);
static COLORSET colorSetPermute(COLORSET colorSet, PERMUTATION permutation);

/* Externally linked functions */
void initializeSequenceOrder(void)
{
  uint64_t ix = 0, i;
  uint64_t done = 0;

#define ADD_TO_SEQUENCE_ORDER(colors)               \
  do {                                              \
    SequenceOrder[ix++] = (NFACES - 1) & ~(colors); \
    done |= 1llu << (colors);                       \
  } while (0)
  for (i = 0; i < NCOLORS; i++) {
    ADD_TO_SEQUENCE_ORDER(1llu << i);
  }
  ADD_TO_SEQUENCE_ORDER((32llu | 1llu));
  for (i = 0; i < NCOLORS - 1; i++) {
    ADD_TO_SEQUENCE_ORDER((2llu | 1llu) << i);
  }
  for (i = 0; i < NFACES; i++) {
    if ((done & (1llu << i)) != 0) {
      continue;
    }
    ADD_TO_SEQUENCE_ORDER(i);
  }
  assert(done == ~0llu);
  assert(ix == NFACES);
  for (i = 0; i < NFACES; i++) {
    InverseSequenceOrder[SequenceOrder[i]] = i;
  }
  for (i = 0; i < NFACES; i++) {
    assert(InverseSequenceOrder[SequenceOrder[i]] == i);
    assert(SequenceOrder[InverseSequenceOrder[i]] == i);
  }
}

SYMMETRY_TYPE symmetryType6(FACE_DEGREE *args)
{
  struct faceDegreeSequence argsAsSequence = {
      {args[0], args[1], args[2], args[3], args[4], args[5]}};
  return d6SymmetryTypeN(6, &argsAsSequence);
}

SYMMETRY_TYPE symmetryTypeFaces(void)
{
  return d6SymmetryType64(d6FaceDegreesInSequenceOrder());
}

char *faceDegreeSignature(void)
{
  static char Result[NCOLORS + 1];
  FACE_DEGREE_SEQUENCE faceDegrees = d6FaceDegreesInSequenceOrder();
  for (int i = 0; i < NCOLORS; i++) {
    Result[i] = '0' + faceDegrees->faceDegrees[i];
  }
  Result[NCOLORS] = '\0';
  return Result;
}

void canonicalCallback(UseFaceDegrees callback, void *data)
{
  FACE_DEGREE args[NCOLORS];
  canoncialCallbackImpl(0, 0, args, callback, data);
}

FACE_DEGREE_SEQUENCE d6FaceDegreesInNaturalOrder(void)
{
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[i] = Faces[i].cycle->length;
  }
  return faceDegrees;
}

FACE_DEGREE_SEQUENCE d6InvertedFaceDegreesInNaturalOrder(void)
{
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[i] = Faces[i].cycle->length;
  }
  return faceDegrees;
}

FACE_DEGREE_SEQUENCE d6ConvertToSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInNaturalOrder);
FACE_DEGREE_SEQUENCE d6ConvertToNaturalOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder);
FACE_DEGREE_SEQUENCE d6MaxInSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);
bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees, FACE_DEGREE_SEQUENCE other);
char *d6ToString(FACE_DEGREE_SEQUENCE faceDegrees);

/* File scoped static functions */
static int compareUint8(const void *a, const void *b)
{
  return -memcmp(a, b, sizeof(int) * NFACES);
}

// static COLOR colorPermute(COLOR color, PERMUTATION permutation)
// {
//   return permutation[color];
// }

static COLORSET colorSetPermute(COLORSET colorSet, PERMUTATION permutation)
{
  COLORSET result = 0;
  for (COLOR color = 0; color < NCOLORS; color++) {
    if (COLORSET_HAS_MEMBER(color, colorSet)) {
      result |= 1u << permutation[color];
    }
  }
  return result;
}

/*
sizes[i] is in SequenceOrder
To find the color we have to use InverseSequenceOrder, then permute, then map
back
*/
static SYMMETRY_TYPE d6SymmetryType64(FACE_DEGREE_SEQUENCE sizes)
{
  FACE_DEGREE_SEQUENCE permuted = NEW_ARRAY(FACE_DEGREE_SEQUENCE, 12);
  int i, j;
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NFACES; j++) {
      permuted[i].faceDegrees[j] =
          sizes->faceDegrees[InverseSequenceOrder[colorSetPermute(
              SequenceOrder[j], group[i])]];
    }
  }
  assert(memcmp(permuted, sizes, sizeof(permuted[0])) == 0);
  qsort(permuted, 12, sizeof(permuted[0]), compareUint8);
  if (memcmp(permuted, sizes, sizeof(permuted[0])) != 0) {
    return NON_CANONICAL;
  }

  if (memcmp(permuted, permuted + 1, sizeof(permuted[0])) == 0) {
    return EQUIVOCAL;
  }

  return CANONICAL;
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

static void canoncialCallbackImpl(int depth, int sum, FACE_DEGREE *args,
                                  UseFaceDegrees callback, void *data)
{
  if (sum > TOTAL_5FACE_DEGREE) {
    return;
  }
  if (depth == NCOLORS) {
    if (sum != TOTAL_5FACE_DEGREE) {
      return;
    }
    freeAll();
    switch (symmetryType6(args)) {
      case NON_CANONICAL:
        return;
      case EQUIVOCAL:
      case CANONICAL:
        callback(data, args);
        return;
    }
  }
  for (int i = NCOLORS; i >= 3; i--) {
    args[depth] = i;
    canoncialCallbackImpl(depth + 1, sum + i, args, callback, data);
  }
}
