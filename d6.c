#include "d6.h"
#define D6_DEBUG 0
static COLORSET sequenceOrder[NFACES];
static COLORSET inverseSequenceOrder[NFACES];

static PERMUTATION group[] = {
    {0, 1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 0}, {2, 3, 4, 5, 0, 1},
    {3, 4, 5, 0, 1, 2}, {4, 5, 0, 1, 2, 3}, {5, 0, 1, 2, 3, 4},
    {5, 4, 3, 2, 1, 0}, {4, 3, 2, 1, 0, 5}, {3, 2, 1, 0, 5, 4},
    {2, 1, 0, 5, 4, 3}, {1, 0, 5, 4, 3, 2}, {0, 5, 4, 3, 2, 1},
};

void initializeSequenceOrder(void)
{
  int ix = 0, i;
  uint64_t done = 0;

#define ADD_TO_SEQUENCE_ORDER(colors)               \
  do {                                              \
    sequenceOrder[ix++] = (NFACES - 1) & ~(colors); \
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
    inverseSequenceOrder[sequenceOrder[i]] = i;
  }
}

COLOR d6PermuteColor(COLOR color, PERMUTATION permutation)
{
  return permutation[color];
}

COLORSET d6PermuteColorSet(COLORSET colorSet, PERMUTATION permutation)
{
  COLORSET result = 0;
  for (COLOR color = 0; color < NCOLORS; color++) {
    if (memberOfColorSet(color, colorSet)) {
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
sizes[i] is in sequenceOrder
To find the color we have to use inverseSequenceOrder, then permute, then map
back
*/
static SYMMETRY_TYPE d6SymmetryType64(int *sizes)
{
  int permuted[12][NFACES];
  int i, j;
#if D6_DEBUG
  printf("+\n");
  printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", sizes[0], sizes[1], sizes[2],
         sizes[3], sizes[4], sizes[5], sizes[6], sizes[7], sizes[8], sizes[9],
         sizes[10], sizes[11]);
  printf("+\n");
#endif
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NFACES; j++) {
      permuted[i][j] = sizes[inverseSequenceOrder[d6PermuteColorSet(
          sequenceOrder[j], group[i])]];
    }
  }
  assert(memcmp(permuted[0], sizes, sizeof(permuted[0])) == 0);
#if D6_DEBUG
  printf("*\n");
  for (i = 0; i < 12; i++) {
    printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", permuted[i][0],
           permuted[i][1], permuted[i][2], permuted[i][3], permuted[i][4],
           permuted[i][5], permuted[i][6], permuted[i][7], permuted[i][8],
           permuted[i][9], permuted[i][10], permuted[i][11]);
  }
  printf("*\n");
#endif
  qsort(permuted, 12, sizeof(permuted[0]), compareUint8);
#if D6_DEBUG
  printf("=\n");
  for (i = 0; i < 12; i++) {
    printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", permuted[i][0],
           permuted[i][1], permuted[i][2], permuted[i][3], permuted[i][4],
           permuted[i][5], permuted[i][6], permuted[i][7], permuted[i][8],
           permuted[i][9], permuted[i][10], permuted[i][11]);
  }
  printf("=\n");
#endif
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

SYMMETRY_TYPE d6SymmetryType6(int a1, int a2, int a3, int a4, int a5, int a6)
{
  int args[] = {a1, a2, a3, a4, a5, a6};
  return d6SymmetryTypeN(6, args);
}

SYMMETRY_TYPE d6SymmetryType12(int a1, int a2, int a3, int a4, int a5, int a6,
                               int a7, int a8, int a9, int a10, int a11,
                               int a12)
{
  int args[] = {a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12};
  return d6SymmetryTypeN(12, args);
}

SYMMETRY_TYPE d6SymmetryTypeFaces(FACE allFaces)
{
  int sizes[NFACES];
  for (int i = 0; i < NFACES; i++) {
    sizes[i] = allFaces[sequenceOrder[i]].cycle->length;
  }
  return d6SymmetryType64(sizes);
}

static void canoncialCallbackImpl(int depth, int sum, int *args,
                                  Callback6 callback6, Callback12 callback12)
{
  if (depth <= 6) {
    if (sum > 27) {
      return;
    }
  }
  if (depth == 6) {
    if (sum != 27) {
      return;
    }
    switch (d6SymmetryTypeN(6, args)) {
      case NON_CANONICAL:
        return;
      case EQUIVOCAL:
      case CANONICAL:
        callback6(args[0], args[1], args[2], args[3], args[4], args[5]);
        return;
    }
  }
  if (depth <= 12 && depth >= 6) {
    // if ((sum - 27) + (21 - depth) * 3 > 56) {
    //   return;
    // }
  }
  if (depth == 12) {
    switch (d6SymmetryTypeN(12, args)) {
      case NON_CANONICAL:
        break;
      case EQUIVOCAL:
      case CANONICAL:
        callback12(args[0], args[1], args[2], args[3], args[4], args[5],
                   args[6], args[7], args[8], args[9], args[10], args[11]);
        break;
    }
    return;
  }
  for (int i = 3; i <= 6; i++) {
    args[depth] = i;
    canoncialCallbackImpl(depth + 1, sum + i, args, callback6, callback12);
  }
}

void canoncialCallback(Callback6 callback6, Callback12 callback12)
{
  int args[12];
  canoncialCallbackImpl(0, 0, args, callback6, callback12);
}
