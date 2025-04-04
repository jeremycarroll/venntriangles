/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "d6.h"

#include "face.h"
#include "memory.h"

#include <stdarg.h>
/* Global variables - file scoped */
#define TOTAL_5FACE_DEGREE 27

#define TOTAL_SEQUENCE_STORAGE 100

static COLORSET SequenceOrder[NFACES];
static COLORSET InverseSequenceOrder[NFACES];
static int group[2 * NCOLORS][NCOLORS] = {
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
static int comparePermutation(const void *a, const void *b);
static SYMMETRY_TYPE d6SymmetryType64(FACE_DEGREE_SEQUENCE sizes);
static SYMMETRY_TYPE d6SymmetryTypeN(int n, FACE_DEGREE_SEQUENCE args);
static FACE_DEGREE_SEQUENCE d6FaceDegreesInSequenceOrder(void);
static void canoncialCallbackImpl(int depth, int sum, FACE_DEGREE *args,
                                  UseFaceDegrees callback, void *data);

static FACE_DEGREE_SEQUENCE sortPermutationsOfSequence(
    const int count, const FACE_DEGREE_SEQUENCE first, va_list moreSequences);

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
#if NCOLORS == 6
  assert(done == ~0llu);
#else
  assert(done == (1llu << (uint64_t)NFACES) - 1l);
#endif
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
  PERMUTATION permute = d6InvertingPermutation();
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[colorSetPermute(i, permute)] =
        Faces[(NFACES - 1) & ~i].cycle->length;
  }
  return faceDegrees;
}

PERMUTATION d6Compose(PERMUTATION a, PERMUTATION b)
{
  PERMUTATION result = NEW(PERMUTATION);
  for (int i = 0; i < NCOLORS; i++) {
    (*result)[i] = (*a)[(*b)[i]];
  }
  return result;
}

PERMUTATION d6Inverse(PERMUTATION permutation)
{
  PERMUTATION result = NEW(PERMUTATION);
  for (int i = 0; i < NCOLORS; i++) {
    (*result)[(*permutation)[i]] = i;
  }
  return result;
}

PERMUTATION d6Identity(void) { return &group[0]; }

char *d6Permutation2str(PERMUTATION permutation)
{
  char *buffer = getBuffer();
  char *p = buffer;
  p += sprintf(p, "(");
  for (int i = 0; i < NCOLORS; i++) {
    p += sprintf(p, "%d ", (*permutation)[i]);
  }
  p += sprintf(p, ")\n");
  return usingBuffer(buffer);
}

bool d6PermutationEqual(PERMUTATION a, PERMUTATION b)
{
  return memcmp(a, b, sizeof(((PERMUTATION)NULL)[0])) == 0;
}

PERMUTATION d6InvertingPermutation(void)
{
  PERMUTATION result = NEW(PERMUTATION);
  for (int i = 0; i < NCOLORS; i++) {
    (*result)[i] = Faces[0].cycle->curves[i];
  }
  return d6Inverse(result);
}

PERMUTATION d6CycleAsPermutation(CYCLE cycle)
{
  uint32_t i;
  PERMUTATION result = NEW(PERMUTATION);
  for (i = 0; i < cycle->length; i++) {
    (*result)[i] = i;
  }
  for (i = 0; i < cycle->length; i++) {
    (*result)[cycle->curves[i]] = cycle->curves[(i + 1) % cycle->length];
  }
  return result;
}

PERMUTATION d6Closure(int *sizeReturn, int numberOfGenerators,
                      PERMUTATION generator1, ...)
{
  int size = 0;
  PERMUTATION result = NEW_ARRAY(PERMUTATION, FACTORIAL6);
  va_list args;
  bool progress = true;
  int i, j, k;
  PERMUTATION generated;
  memcpy(result[size++], generator1, sizeof(((PERMUTATION)NULL)[0]));
  va_start(args, generator1);
  for (i = 0; i < numberOfGenerators - 1; i++) {
    memcpy(result[size++], va_arg(args, PERMUTATION),
           sizeof(((PERMUTATION)NULL)[0]));
  }
  va_end(args);
  while (progress) {
    progress = false;
    for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
        generated = d6Compose(&result[i], &result[j]);
        for (k = 0; k < size; k++) {
          if (d6PermutationEqual(generated, &result[k])) {
            goto nextJ;
          }
        }
        progress = true;
        memcpy(&result[size++], generated, sizeof(((PERMUTATION)NULL)[0]));
      nextJ:;
      }
    }
  }
  *sizeReturn = size;
  qsort(result, size, sizeof(((PERMUTATION)NULL)[0]), comparePermutation);
  return result;
}

PERMUTATION d6Permutation(int a1, ...)
{
  PERMUTATION result = NEW(PERMUTATION);
  va_list args;
  (*result)[0] = a1;
  va_start(args, a1);
  for (int i = 1; i < NCOLORS; i++) {
    (*result)[i] = va_arg(args, int);
  }
  va_end(args);
  return result;
}

FACE_DEGREE_SEQUENCE d6ConvertToSequenceOrder(
    FACE_DEGREE_SEQUENCE faceDegreesInNaturalOrder)
{
  FACE_DEGREE_SEQUENCE faceDegrees = NEW(FACE_DEGREE_SEQUENCE);
  for (int i = 0; i < NFACES; i++) {
    faceDegrees->faceDegrees[i] =
        faceDegreesInNaturalOrder->faceDegrees[SequenceOrder[i]];
  }
  return faceDegrees;
}

FACE_DEGREE_SEQUENCE d6MaxInSequenceOrder(
    int count, FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...);

bool d6Equal(FACE_DEGREE_SEQUENCE faceDegrees, FACE_DEGREE_SEQUENCE other)
{
  return memcmp(faceDegrees, other, sizeof(faceDegrees[0])) == 0;
}

SYMMETRY_TYPE d6IsMaxInSequenceOrder(
    const int count, const FACE_DEGREE_SEQUENCE faceDegreesInSequenceOrder, ...)
{
  FACE_DEGREE_SEQUENCE sorted;
  va_list args;
  va_start(args, faceDegreesInSequenceOrder);
  sorted = sortPermutationsOfSequence(count, faceDegreesInSequenceOrder, args);
  if (!d6Equal(sorted, faceDegreesInSequenceOrder)) {
    return NON_CANONICAL;
  }
  if (d6Equal(sorted, sorted + 1)) {
    return EQUIVOCAL;
  }
  return CANONICAL;
}

char *d6SequenceToString(FACE_DEGREE_SEQUENCE faceDegrees)
{
  char *result = getBuffer();
  char *p = result;
  for (int i = 0; i < NFACES; i++) {
    *p++ = '0' + faceDegrees->faceDegrees[i];
  }
  *p = '\0';
  return usingBuffer(result);
}

/* File scoped static functions */
static int compareFaceDegree(const void *a, const void *b)
{
  return -memcmp(a, b, sizeof(FACE_DEGREE) * NFACES);
}

static int comparePermutation(const void *a, const void *b)
{
  return memcmp(a, b, sizeof(((PERMUTATION)NULL)[0]));
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

/*
sizes[i] is in SequenceOrder
To find the color we have to use InverseSequenceOrder, then permute, then map
back
*/
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
                SequenceOrder[k], &group[j])]];
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
