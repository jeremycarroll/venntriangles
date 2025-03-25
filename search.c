#include <stdio.h>
#include <time.h>

#include "d6.h"
#include "face.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"

FACE dynamicFaceChoose(bool smallestFirst)
{
  FACE face = NULL;
  int sign = smallestFirst ? 1 : -1;
  int64_t min = smallestFirst ? (NCYCLES + 1) : 0;
  int i;
  for (i = 0; i < NFACES; i++) {
    if (sign * (int64_t)Faces[i].cycleSetSize < min && Faces[i].cycle == NULL) {
      min = sign * (int64_t)Faces[i].cycleSetSize;
      face = Faces + i;
    }
  }
  return face;
}

static CYCLE chooseCycle(FACE face, CYCLE cycle)
{
  return cycleSetFindNext(face->possibleCycles, cycle);
}

static int solution_count = 0;
void dynamicSearch(bool smallestFirst, void (*foundSolution)(void))
{
  FACE face;
  FACE chosenFaces[NFACES];
  CYCLE chosenCycles[NFACES];
  CYCLE cycle;
  int position = 0;
  enum { NEXT_FACE, NEXT_CYCLE } state = NEXT_FACE;
  while (position >= 0) {
    statisticPrintOneLine(position);
    switch (state) {
      case NEXT_FACE:
        face = dynamicFaceChoose(smallestFirst);
        if (face == NULL) {
          if (dynamicFaceFinalCorrectnessChecks() == NULL) {
            solution_count++;
            foundSolution();
          }
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          face->backtrack = Trail;
          chosenFaces[position] = face;
          chosenCycles[position] = NULL;
          state = NEXT_CYCLE;
        }
        break;
      case NEXT_CYCLE:
        face = chosenFaces[position];
        trailBacktrackTo(face->backtrack);
        cycle = chooseCycle(face, chosenCycles[position]);
        if (cycle == NULL) {
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          chosenCycles[position] = cycle;
          /* suspect - because face->backtrack gets reset. */
          setDynamicPointer(&face->cycle, cycle);
          assert(face->cycle == cycle);
          if (dynamicFaceMakeChoice(face) == NULL) {
            position += 1;
            state = NEXT_FACE;
          } else {
            /*
            same position, next cycle.
            */
          }
        }
        break;
    }
  }
}

void dynamicSolutionEnumerate(FILE *(*fp)(char *signature, int soln,
                                          int subsoln))
{
  char *signature = dynamicFaceDegreeSignature();
}

static clock_t totalWastedTime = 0;
static clock_t totalUsefulTime = 0;
static int wastedSearchCount = 0;
static int usefulSearchCount = 0;
static void full_search_callback6(void *foundSolutionVoidPtr, int *args)
{
  clock_t now = clock();
  clock_t used;
  int initialSolutionCount = solution_count;
  int i;
  void (*foundSolution)(void) = foundSolutionVoidPtr;
  resetGlobals();
  resetInitialize();
  resetTrail();
  resetPoints();
  initialize();
  dynamicFaceSetupCentral(args);
  dynamicSearch(true, foundSolution);
  used = clock() - now;
  if (solution_count != initialSolutionCount) {
    totalUsefulTime += used;
    usefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    PRINT_TIME(used, 0);
    PRINT_TIME(totalUsefulTime, usefulSearchCount);
    PRINT_TIME(totalWastedTime, wastedSearchCount);
    for (i = 0; i < NCOLORS; i++) {
      printf("%d ", args[i]);
    }
    printf(" gives %d new solutions\n", solution_count - initialSolutionCount);
  } else {
    wastedSearchCount += 1;

    totalWastedTime += used;
  }
}

void dynamicSearchFull(void (*foundSolution)(void))
{
  initializeSequenceOrder();
  solution_count = 0;
  dynamicFaceCanonicalCallback(full_search_callback6, (void *)foundSolution);
}
