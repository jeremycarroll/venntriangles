
#include <stdarg.h>

#include "graph.h"
#include "output.h"
#include "unity.h"

/* Use the lettering from the 2000 diagram of six triangles (ABCDEF round the
 * outer face) */
#define LEGACY_LETTERS 0

/*
Utilities for printing items when debugging. These are not intended to be
efficient.

The caller provides a long buffer, the utilities print into the buffer
and return points into the buffer.

The buffer is organized as

null-terminated-string, 0, null-terminated-string, 0, etc.

LEGACY_LETTERS must be 0 to use the obvious a,b,c,d,e,f coloring; or 1
to use the D E F C A B coloring used in the picture from 2000.
(Which has A, B,C, D, E, F round the outer face)

This means that we are typically looking for double null terminated strings to
find the next slot in the buffer.

The buffer must be initialized to either 0,0 or 0,0.

When adding a string, the new string is added over the second zero.
So that the buffer always starts with a 0.

We provide no protection for buffer overflow.
*/

static char* getLookup(void)
{
#if LEGACY_LETTERS
  return "DEFCAB";
#else
  return "abcdef";
#endif
}

void printFace(FACE face)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", face2str(nBuffer, face));
}

void printEdge(EDGE edge)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", edge2str(nBuffer, edge));
}

static char* nextSlot(char* buffer)
{
  buffer++;
  while (*buffer != '\0' || buffer[1] != '\0') {
    buffer++;
  }
  return buffer + 1;
}

static char* returnSlot(char* buffer)
{
  buffer[strlen(buffer) + 1] = '\0';
  return buffer;
}

char* colors2str(char* dbuffer, COLORSET colors)
{
  uint32_t i;
  char* result = nextSlot(dbuffer);
  char* lookup = getLookup();
  char* p = result;
  *p++ = '|';
  for (i = 0; i < NCOLORS; i++) {
    if (colors & (1u << i)) {
      *p++ = lookup[i];
    }
  }
  *p++ = '|';
  *p = '\0';
  return returnSlot(result);
}

int color2char(COLOR c) { return getLookup()[c]; }

char* cycle2str(char* dbuffer, CYCLE cycle)
{
  char* result = nextSlot(dbuffer);
  char* lookup = getLookup();
  char* p = result;
  if (cycle == NULL) {
    return "(NULL)";
  }
  *p++ = '(';
  for (uint32_t i = 0; i < cycle->length; i++) {
    *p++ = lookup[cycle->curves[i]];
  }
  *p++ = ')';
  *p = '\0';
  return returnSlot(result);
}

char* face2str(char* dbuffer, FACE face)
{
  char* colors = colors2str(dbuffer, face->colors);
  char* cycle = cycle2str(dbuffer, face->cycle);
  char* result = nextSlot(dbuffer);
  char* cycleSetSizeBuf = result + sprintf(result, "%s%s", colors, cycle);
  if (face->cycleSetSize > 1) {
    sprintf(cycleSetSizeBuf, "^%llu", face->cycleSetSize);
  }
  return returnSlot(result);
}

char* upoint2str(char* dbuffer, UPOINT up)
{
  char* lookup = getLookup();
  char* colors = colors2str(dbuffer, up->colors);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%s(%c,%c)", colors, lookup[up->primary],
          lookup[up->secondary]);
  return returnSlot(result);
}

char* dpoint2str(char* dbuffer, DPOINT dp)
{
  return upoint2str(dbuffer, dp->point);
}

char* edge2str(char* dbuffer, EDGE edge)
{
  char color = color2char(edge->color);
  char* colors =
      edge->face == NULL ? "***" : colors2str(dbuffer, edge->face->colors);
  char* to = edge->to == NULL ? "***" : dpoint2str(dbuffer, edge->to);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%c%c/%s[%s]", color, IS_PRIMARY_EDGE(edge) ? '+' : '-',
          colors, to);
  return returnSlot(result);
}

void printSelectedFaces(void)
{
  uint32_t i;
  FACE face;
  for (i = 0, face = g_faces; i < NFACES; i++, face++) {
    if (face->cycle || face->cycleSetSize < 2) {
      printFace(face);
    }
  }
}

uint32_t cycleIdFromColors(char* colors)
{
  COLOR cycle[NCOLORS];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return findCycleId(cycle, i);
}

FACE faceFromColors(char* colors)
{
  int face_id = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    face_id |= (1 << (*colors - 'a'));
    colors++;
  }
  return g_faces + face_id;
}

void printSolution(FILE* fp)
{
  COLORSET colors = 0;
  if (fp == NULL) {
    fp = stdout;
  }

  while (true) {
    FACE face = g_faces + colors;
    do {
      FACE next = face->next;
      COLORSET colorBeingDropped = face->colors & ~next->colors;
      COLORSET colorBeingAdded = next->colors & ~face->colors;
      char buffer[256] = {0, 0};
      fprintf(fp, "%s [%c,%c] ", face2str(buffer, face),
              colors2str(buffer, colorBeingDropped)[1],
              colors2str(buffer, colorBeingAdded)[1]);
      face = next;
    } while (face->colors != colors);
    fprintf(fp, "\n");
    if (colors == (NFACES - 1)) {
      break;
    }
    colors |= (face->previous->colors | 1);
  }
  fprintf(fp, "\n");
}

FACE addSpecificFace(char* colors, char* cycle)
{
  FAILURE failure;
  FACE face = faceFromColors(colors);
  uint32_t cycleId = cycleIdFromColors(cycle);
  TEST_ASSERT_TRUE(memberOfCycleSet(cycleId, face->possibleCycles));
  if (face->cycleSetSize == 1) {
    TEST_ASSERT_EQUAL(face->cycle, findFirstCycleInSet(face->possibleCycles));
    TEST_ASSERT_EQUAL(face->cycle, g_cycles + cycleId);
  } else {
    face->cycle = g_cycles + cycleId;
    failure = makeChoice(face);
#if STATS
    printStatisticsOneLine(0);
#endif
#if DEBUG
    printSelectedFaces();
#else

    if (failure != NULL) {
      printf("Failure: %s %s\n", failure->label, failure->shortLabel);
      printSelectedFaces();
    }
#endif
    TEST_ASSERT_NULL(failure);
  }
  return face;
}

int* intArray(int a, ...)
{
  static int result[NCOLORS];
  va_list ap;
  int i;
  result[0] = a;
  va_start(ap, a);
  for (i = 1; i < NCOLORS; i++) {
    a = va_arg(ap, int);
    result[i] = a;
  }
  va_end(ap);
  return result;
}

void printCycle(CYCLE cycle)
{
  for (uint64_t i = 0; i < cycle->length; i++) {
    printf("%c", 'a' + cycle->curves[i]);
  }
}
/* returns false if cycleset has bits set past the last. */
bool printCycleSet(CYCLESET cycleSet)
{
  uint32_t lastBit = (NCYCLES - 1) % BITS_PER_WORD;
  uint64_t faulty, i, j;
  if ((faulty = (cycleSet[CYCLESET_LENGTH - 1] & ~((1ul << lastBit) - 1ul)))) {
    printf("0x%016llx\n", faulty);
    return false;
  }
  putchar('{');
  for (i = 0; i < CYCLESET_LENGTH; i++) {
    if (cycleSet[i]) {
      for (j = 0; j < 64; j++) {
        if (cycleSet[i] & (1ul << j)) {
          putchar(' ');
          printCycle(g_cycles + i * BITS_PER_WORD + j);
          putchar(',');
        }
      }
    }
  }
  printf(" }\n");
  return true;
}
