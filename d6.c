#include "d6.h"
static COLORSET sequenceOrder[NFACES];

static PERMUTATION group[] = {
    {0, 1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 0}, {2, 3, 4, 5, 0, 1},
    {3, 4, 5, 0, 1, 2}, {4, 5, 0, 1, 2, 3}, {5, 0, 1, 2, 3, 4},
    {5, 4, 3, 2, 1, 0}, {4, 3, 2, 1, 0, 5}, {3, 2, 1, 0, 5, 4},
    {2, 1, 0, 5, 4, 3}, {1, 0, 5, 4, 3, 2}, {0, 5, 4, 3, 2, 1},
};

void initializeSequenceOrder(void)
{
  uint32_t ix = 0, i;
  uint64_t done = 0;

#define ADD_TO_SEQUENCE_ORDER(colors) \
  do {                                \
    sequenceOrder[ix++] = (colors);   \
    done |= 1llu << (colors);         \
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
  return -memcmp(a, b, sizeof(uint8_t) * NFACES);
}

static SYMMETRY_TYPE d6SymmetryType64(uint8_t *sizes)
{
  uint8_t permuted[12][NFACES];
  uint32_t i, j;
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NFACES; j++) {
      permuted[i][j] = sizes[d6PermuteColorSet(j, group[i])];
    }
  }
  assert(memcmp(permuted[0], sizes, sizeof(permuted[0])) == 0);
  qsort(permuted, 12, sizeof(permuted[0]), compareUint8);
#if 0
  printf("=\n");
  for (i = 0; i < 12; i++) {
    printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", permuted[i][1],
           permuted[i][2], permuted[i][4], permuted[i][8], permuted[i][16],
           permuted[i][32], permuted[i][3], permuted[i][6], permuted[i][12],
           permuted[i][24], permuted[i][48], permuted[i][33]);
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

SYMMETRY_TYPE d6SymmetryType6(u_int32_t a1, u_int32_t a2, u_int32_t a3,
                              u_int32_t a4, u_int32_t a5, u_int32_t a6)
{
  uint8_t sizes[NFACES];
  memset(sizes, 0, sizeof(sizes));
  sizes[sequenceOrder[0]] = a1;
  sizes[sequenceOrder[1]] = a2;
  sizes[sequenceOrder[2]] = a3;
  sizes[sequenceOrder[3]] = a4;
  sizes[sequenceOrder[4]] = a5;
  sizes[sequenceOrder[5]] = a6;

  return d6SymmetryType64(sizes);
}

SYMMETRY_TYPE d6SymmetryType12(u_int32_t a1, u_int32_t a2, u_int32_t a3,
                               u_int32_t a4, u_int32_t a5, u_int32_t a6,
                               u_int32_t a7, u_int32_t a8, u_int32_t a9,
                               u_int32_t a10, u_int32_t a11, u_int32_t a12)
{
  uint8_t sizes[NFACES];
  memset(sizes, 0, sizeof(sizes));
  sizes[sequenceOrder[0]] = a1;
  sizes[sequenceOrder[1]] = a2;
  sizes[sequenceOrder[2]] = a3;
  sizes[sequenceOrder[3]] = a4;
  sizes[sequenceOrder[4]] = a5;
  sizes[sequenceOrder[5]] = a6;
  sizes[sequenceOrder[6]] = a7;
  sizes[sequenceOrder[7]] = a8;
  sizes[sequenceOrder[8]] = a9;
  sizes[sequenceOrder[9]] = a10;
  sizes[sequenceOrder[10]] = a11;
  sizes[sequenceOrder[11]] = a12;

  return d6SymmetryType64(sizes);
}

SYMMETRY_TYPE d6SymmetryTypeFaces(FACE allFaces)
{
  uint8_t sizes[NFACES];
  for (int i = 0; i < NFACES; i++) {
    sizes[(NFACES - 1) & ~i] = allFaces[i].cycle->length;
  }
  return d6SymmetryType64(sizes);
}
