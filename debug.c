
#include <stdarg.h>

#include "face.h"
#include "unity.h"

void dynamicFacePrint(FACE face)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", faceToStr(nBuffer, face));
}

void dynamicEdgePrint(EDGE edge)
{
  char nBuffer[2048] = {0, 0};
  printf("%s\n", edgeToStr(nBuffer, edge));
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

char* colorSetToStr(char* dbuffer, COLORSET colors)
{
  uint32_t i;
  char* result = nextSlot(dbuffer);
  char* p = result;
  *p++ = '|';
  for (i = 0; i < NCOLORS; i++) {
    if (colors & (1u << i)) {
      *p++ = 'a' + i;
    }
  }
  *p++ = '|';
  *p = '\0';
  return returnSlot(result);
}

int colorToChar(COLOR c) { return 'a' + c; }

char* dynamicCycleToStr(char* dbuffer, CYCLE cycle)
{
  char* result = nextSlot(dbuffer);
  char* p = result;
  if (cycle == NULL) {
    return "(NULL)";
  }
  *p++ = '(';
  for (uint32_t i = 0; i < cycle->length; i++) {
    *p++ = 'a' + cycle->curves[i];
  }
  *p++ = ')';
  *p = '\0';
  return returnSlot(result);
}

char* faceToStr(char* dbuffer, FACE face)
{
  char* colors = colorSetToStr(dbuffer, face->colors);
  char* cycle = dynamicCycleToStr(dbuffer, face->cycle);
  char* result = nextSlot(dbuffer);
  char* cycleSetSizeBuf = result + sprintf(result, "%s%s", colors, cycle);
  if (face->cycleSetSize > 1) {
    sprintf(cycleSetSizeBuf, "^%llu", face->cycleSetSize);
  }
  return returnSlot(result);
}

char* uPointToStr(char* dbuffer, UPOINT up)
{
  char* colors = colorSetToStr(dbuffer, up->colors);
  char* result = nextSlot(dbuffer);
  sprintf(result, "%s(%c,%c)", colors, 'a' + up->primary, 'a' + up->secondary);
  return returnSlot(result);
}

char* dynamicDPointToStr(char* dbuffer, DPOINT dp)
{
  return uPointToStr(dbuffer, dp->point);
}

char* edgeToStr(char* dbuffer, EDGE edge)
{
  char color = colorToChar(edge->color);
  char* colors = colorSetToStr(dbuffer, edge->colors);
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

uint32_t cycleIdFromColors(char* colors)
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
      fprintf(fp, "%s [%c,%c] ", faceToStr(buffer, face),
              colorSetToStr(buffer, colorBeingDropped)[1],
              colorSetToStr(buffer, colorBeingAdded)[1]);
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
  uint32_t cycleId = cycleIdFromColors(cycle);
  TEST_ASSERT_TRUE(cycleSetMember(cycleId, face->possibleCycles));
  if (face->cycleSetSize == 1) {
    TEST_ASSERT_EQUAL(face->cycle, cycleSetFindFirst(face->possibleCycles));
    TEST_ASSERT_EQUAL(face->cycle, Cycles + cycleId);
  } else {
    face->cycle = Cycles + cycleId;
    failure = dynamicFaceMakeChoice(face);

    if (failure != NULL) {
      printf("Failure: %s %s\n", failure->label, failure->shortLabel);
      dynamicFacePrintSelected();
    }
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
