#include "d6.h"

#include "face.h"
#define TOTAL_5FACE_DEGREE 27
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

void initializeSequenceOrder(void)
{
  int ix = 0, i;
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
}

COLOR dynamicColorPermute(COLOR color, PERMUTATION permutation)
{
  return permutation[color];
}

COLORSET dynamicColorSetPermute(COLORSET colorSet, PERMUTATION permutation)
{
  COLORSET result = 0;
  for (COLOR color = 0; color < NCOLORS; color++) {
    if (COLORSET_HAS_MEMBER(color, colorSet)) {
      result |= 1u << permutation[color];
    }
  }
  return result;
}

static int compareUint8(const void *a, const void *b)
{
  return -memcmp(a, b, sizeof(int) * NFACES);
}

/*
sizes[i] is in SequenceOrder
To find the color we have to use InverseSequenceOrder, then permute, then map
back
*/
static SYMMETRY_TYPE d6SymmetryType64(int *sizes)
{
  int permuted[12][NFACES];
  int i, j;
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NFACES; j++) {
      permuted[i][j] = sizes[InverseSequenceOrder[dynamicColorSetPermute(
          SequenceOrder[j], group[i])]];
    }
  }
  assert(memcmp(permuted[0], sizes, sizeof(permuted[0])) == 0);
  qsort(permuted, 12, sizeof(permuted[0]), compareUint8);
  if (memcmp(permuted[0], sizes, sizeof(permuted[0])) != 0) {
    return NON_CANONICAL;
  }

  if (memcmp(permuted[0], permuted[1], sizeof(permuted[0])) == 0) {
    return EQUIVOCAL;
  }

  return CANONICAL;
}

static SYMMETRY_TYPE d6SymmetryTypeN(int n, int *args)
{
  int sizes[NFACES];
  memset(sizes, 0, sizeof(sizes));
  for (int i = 0; i < n; i++) {
    sizes[i] = args[i];
  }
  return d6SymmetryType64(sizes);
}

SYMMETRY_TYPE dynamicSymmetryType6(int *args)
{
  return d6SymmetryTypeN(6, args);
}

static int *d6FaceDegrees()
{
  static int FaceDegrees[NFACES];
  for (int i = 0; i < NFACES; i++) {
    FaceDegrees[i] = Faces[SequenceOrder[i]].cycle->length;
  }
  return FaceDegrees;
}

SYMMETRY_TYPE dynamicSymmetryTypeFaces(void)
{
  return d6SymmetryType64(d6FaceDegrees());
}

char *dynamicFaceDegreeSignature(void)
{
  static char Result[NCOLORS + 1];
  int *faceDegrees = d6FaceDegrees();
  for (int i = 0; i < NCOLORS; i++) {
    Result[i] = '0' + faceDegrees[i];
  }
  Result[NCOLORS] = '\0';
  return Result;
}

static void canoncialCallbackImpl(int depth, int sum, int *args,
                                  UseFaceDegrees callback, void *data)
{
  if (sum > TOTAL_5FACE_DEGREE) {
    return;
  }
  if (depth == NCOLORS) {
    if (sum != TOTAL_5FACE_DEGREE) {
      return;
    }
    switch (d6SymmetryTypeN(NCOLORS, args)) {
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

void dynamicFaceCanonicalCallback(UseFaceDegrees callback, void *data)
{
  int args[NCOLORS];
  canoncialCallbackImpl(0, 0, args, callback, data);
}
