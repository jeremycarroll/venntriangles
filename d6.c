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
static int group[12][NCOLORS] = {
#if NCOLORS == 6
    {0, 1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 0}, {2, 3, 4, 5, 0, 1},
    {3, 4, 5, 0, 1, 2}, {4, 5, 0, 1, 2, 3}, {5, 0, 1, 2, 3, 4},
    {5, 4, 3, 2, 1, 0}, {4, 3, 2, 1, 0, 5}, {3, 2, 1, 0, 5, 4},
    {2, 1, 0, 5, 4, 3}, {1, 0, 5, 4, 3, 2}, {0, 5, 4, 3, 2, 1},
#endif
#if NCOLORS == 4
    {0, 1, 2, 3}
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
  bool result = memcmp(a, b, sizeof(((PERMUTATION)NULL)[0])) == 0;
#if 0
  printf("%s %s %s\n", d6Permutation2str(a), result ? "==" : "!=",
         d6Permutation2str(b));
#endif
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
static SYMMETRY_TYPE d6SymmetryType64(FACE_DEGREE_SEQUENCE sizes)
{
  FACE_DEGREE_SEQUENCE permuted = NEW_ARRAY(FACE_DEGREE_SEQUENCE, 12);
  int i, j;
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NFACES; j++) {
      permuted[i].faceDegrees[j] =
          sizes->faceDegrees[InverseSequenceOrder[colorSetPermute(
              SequenceOrder[j], &group[i])]];
    }
  }
  assert(memcmp(permuted, sizes, sizeof(permuted[0])) == 0);
  qsort(permuted, 12, sizeof(permuted[0]), compareFaceDegree);
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
