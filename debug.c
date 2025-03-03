
#include "venn.h"

/*
Utilities for printing items when debugging. These are not intended to be
efficient.

The caller provides a long buffer, the utilities print into the buffer
and return points into the buffer.

The buffer is organized as

specialByte, null-terminated-string, 0, null-terminated-string, 0, etc.


The specialByte must be 1 to use the obvious a,b,c,d,e,f coloring; or 2
to use the D E F C A B coloring used in the picture from 2000.
(Which has A, B,C, D, E, F round the outer face)

This means that we are typically looking for double null terminated strings to
find the next slot in the buffer.

The buffer must be initialized to either 1,0,0 or 2,0,0.

When adding a string, the new string is added over the second zero.
So that the buffer always starts 1,0 or 2,0

We provide no protection for buffer overflow.
*/

static char* normal = "abcdef";
static char* original = "DEFCAB";

static char* getLookup(char* buffer)
{
  assert(buffer[1] == '\0');
  if (*buffer == 1) {
    return normal;
  }
  assert(*buffer == 2);
  return original;
}

void printFace(FACE face)
{
  char nBuffer[2048] = {1, 0, 0};
  char oBuffer[2048] = {2, 0, 0};
  printf("%s AKA %s\n", face2str(nBuffer, face), face2str(oBuffer, face));
}

void printEdge(EDGE edge)
{
  char nBuffer[2048] = {1, 0, 0};
  char oBuffer[2048] = {2, 0, 0};
  printf("%s AKA %s\n", edge2str(nBuffer, edge), edge2str(oBuffer, edge));
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
  char* lookup = getLookup(dbuffer);
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

int color2char(char* dbuffer, COLOR c) { return getLookup(dbuffer)[c]; }

char* cycle2str(char* dbuffer, CYCLE cycle)
{
  char* result = nextSlot(dbuffer);
  char* lookup = getLookup(dbuffer);
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
char* edge2str(char* dbuffer, EDGE edge)
{
  char color = color2char(dbuffer, edge->color);
  char* colors =
      edge->face == NULL ? "***" : colors2str(dbuffer, edge->face->colors);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%c%c/%s", color, IS_PRIMARY_EDGE(edge) ? '+' : '-', colors);
  return returnSlot(result);
  ;
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
