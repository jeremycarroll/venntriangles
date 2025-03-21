
#include <stdarg.h>

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

void dynamicFacePrint(FACE face)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", dynamicFaceToStr(nBuffer, face));
}

void dynamicEdgePrint(EDGE edge)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", dynamicEdgeToStr(nBuffer, edge));
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

char* dynamicColorSetToStr(char* dbuffer, COLORSET colors)
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

int dynamicColorToChar(COLOR c) { return getLookup()[c]; }

char* dynamicCycleToStr(char* dbuffer, CYCLE cycle)
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

char* dynamicFaceToStr(char* dbuffer, FACE face)
{
  char* colors = dynamicColorSetToStr(dbuffer, face->colors);
  char* cycle = dynamicCycleToStr(dbuffer, face->cycle);
  char* result = nextSlot(dbuffer);
  char* cycleSetSizeBuf = result + sprintf(result, "%s%s", colors, cycle);
  if (face->cycleSetSize > 1) {
    sprintf(cycleSetSizeBuf, "^%llu", face->cycleSetSize);
  }
  return returnSlot(result);
}

char* dynamicUPointToStr(char* dbuffer, UPOINT up)
{
  char* lookup = getLookup();
  char* colors = dynamicColorSetToStr(dbuffer, up->colors);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%s(%c,%c)", colors, lookup[up->primary],
          lookup[up->secondary]);
  return returnSlot(result);
}

char* dynamicDPointToStr(char* dbuffer, DPOINT dp)
{
  return dynamicUPointToStr(dbuffer, dp->point);
}

char* dynamicEdgeToStr(char* dbuffer, EDGE edge)
{
  char color = dynamicColorToChar(edge->color);
  char* colors = dynamicColorSetToStr(dbuffer, edge->colors);
  char* to = edge->to == NULL ? "***" : dynamicDPointToStr(dbuffer, edge->to);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%c%c/%s[%s]", color, IS_PRIMARY_EDGE(edge) ? '+' : '-',
          colors, to);
  return returnSlot(result);
}

void dynamicFacePrintSelected(void)
{
  uint32_t i;
  FACE face;
  for (i = 0, face = Faces; i < NFACES; i++, face++) {
    if (face->cycle || face->cycleSetSize < 2) {
      dynamicFacePrint(face);
    }
  }
}

uint32_t dynamicCycleIdFromColors(char* colors)
{
  COLOR cycle[NCOLORS];
  int i;
  for (i = 0; *colors; i++, colors++) {
    cycle[i] = *colors - 'a';
  }
  return cycleFindId(cycle, i);
}

FACE dynamicFaceFromColors(char* colors)
{
  int face_id = 0;
  while (true) {
    if (*colors == 0) {
      break;
    }
    face_id |= (1 << (*colors - 'a'));
    colors++;
  }
  return Faces + face_id;
}

void dynamicSolutionPrint(FILE* fp)
{
  COLORSET colors = 0;
  if (fp == NULL) {
    fp = stdout;
  }

  while (true) {
    FACE face = Faces + colors;
    do {
      FACE next = face->next;
      COLORSET colorBeingDropped = face->colors & ~next->colors;
      COLORSET colorBeingAdded = next->colors & ~face->colors;
      char buffer[256] = {0, 0};
      fprintf(fp, "%s [%c,%c] ", dynamicFaceToStr(buffer, face),
              dynamicColorSetToStr(buffer, colorBeingDropped)[1],
              dynamicColorSetToStr(buffer, colorBeingAdded)[1]);
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

FACE dynamicFaceAddSpecific(char* colors, char* cycle)
{
  FAILURE failure;
  FACE face = dynamicFaceFromColors(colors);
  uint32_t cycleId = dynamicCycleIdFromColors(cycle);
  TEST_ASSERT_TRUE(initializeCycleSetMember(cycleId, face->possibleCycles));
  if (face->cycleSetSize == 1) {
    TEST_ASSERT_EQUAL(face->cycle,
                      initializeCycleSetFindFirst(face->possibleCycles));
    TEST_ASSERT_EQUAL(face->cycle, Cycles + cycleId);
  } else {
    face->cycle = Cycles + cycleId;
    failure = dynamicFaceMakeChoice(face);
#if STATS
    dynamicStatisticPrintOneLine(0);
#endif
#if DEBUG
    dynamicFacePrintSelected();
#else

    if (failure != NULL) {
      printf("Failure: %s %s\n", failure->label, failure->shortLabel);
      dynamicFacePrintSelected();
    }
#endif
    TEST_ASSERT_NULL(failure);
  }
  return face;
}

int* dynamicIntArray(int a, ...)
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

void dynamicCyclePrint(CYCLE cycle)
{
  for (uint64_t i = 0; i < cycle->length; i++) {
    printf("%c", 'a' + cycle->curves[i]);
  }
}
/* returns false if cycleset has bits set past the last. */
bool dynamicCycleSetPrint(CYCLESET cycleSet)
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
          dynamicCyclePrint(Cycles + i * BITS_PER_WORD + j);
          putchar(',');
        }
      }
    }
  }
  printf(" }\n");
  return true;
}
