
#include "unity.h"
#include "venn.h"
#include "visible_for_testing.h"

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
  for (i = 0; i < NCURVES; i++) {
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
  sprintf(result, "%s%s^%llu", colors, cycle, face->cycleSetSize);
  return returnSlot(result);
}
char* dpoint2str(char* dbuffer, DPOINT dp)
{
  char* lookup = getLookup();
  char* colors = colors2str(dbuffer, dp->point->faces[0]->colors);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%s(%c,%c)", colors, lookup[dp->point->primary],
          lookup[dp->point->secondary]);
  return returnSlot(result);
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

FACIAL_CYCLE_SIZES facialCycleSizes(void)
{
  uint32_t nColors = NCURVES;
  FACIAL_CYCLE_SIZES result;
  uint32_t nFaces = 1 << nColors;
  result.value = 0;
  for (uint32_t i = 0; i < nColors; i++) {
    result.sizes[i] = (uint8_t)g_faces[(nFaces - 1) & ~(1 << i)].cycle->length;
  }
  return result;
}

uint32_t cycleIdFromColors(char* colors)
{
  COLOR cycle[NCURVES];
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

/*
void printNecklaces()
{
  uint64_t i, j, k = 0;
  char buffer[4096] = {0, 0};
  bool done[NFACES][NCURVES];
  memset(done, 0, sizeof(done));
  for (i = 0; i < NFACES; i++) {
    FACE f = g_faces + i;
    for (j = 0; j < NCURVES; j++) {
      EDGE edge = &f->edges[j];
      if (edge->to == NULL) {
        continue;
      }
      DPOINT dpoint = edge->to;
      if (done[edge->face->colors][edge->color]) {
        continue;
      }
      DPOINT dp = dpoint;
      uint32_t level = edge->level;
      printf("*%d* ", level);
      do {
        char* dpstr = dpoint2str(buffer, dp);
        done[edge->face->colors][edge->color] = true;
        printf("%s->[%llu]%c ", dpstr, edge->level,
               color2char(dp->out[1]->color));
        buffer[2] = 0;
        edge = dp->out[1];
        dp = edge->to;
      } while (dp != dpoint && k++ < 10000);
      printf("\n");
    }
  }
}
*/

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
      printf("Failure: %s %x\n", failure->label, failure->type);
      printSelectedFaces();
    }
#endif
    TEST_ASSERT_NULL(failure);
  }
  return face;
}
