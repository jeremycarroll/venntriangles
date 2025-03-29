#include "search.h"

#include "d6.h"
#include "face.h"
#include "statistics.h"
#include "trail.h"
#include "utils.h"

#include <stdio.h>
#include <time.h>

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

static int SolutionCount = 0;
static int position = 0;
void dynamicSearch(bool smallestFirst, void (*foundSolution)(void))
{
  FACE face;
  FACE chosenFaces[NFACES];
  CYCLE chosenCycles[NFACES];
  CYCLE cycle;
  enum { NEXT_FACE, NEXT_CYCLE } state = NEXT_FACE;
  position = 0;
  while (position >= 0) {
    statisticPrintOneLine(position, false);
    switch (state) {
      case NEXT_FACE:
        face = dynamicFaceChoose(smallestFirst);
        if (face == NULL) {
          if (dynamicFaceFinalCorrectnessChecks() == NULL) {
            SolutionCount++;
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
          TRAIL_SET_POINTER(&face->cycle, cycle);
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

static clock_t TotalWastedTime = 0;
static clock_t TotalUsefulTime = 0;
static int WastedSearchCount = 0;
static int UsefulSearchCount = 0;
static TRAIL StartPoint;
static void fullSearchCallback(void *foundSolutionVoidPtr, int *args)
{
  clock_t now = clock();
  clock_t used;
  int initialSolutionCount = SolutionCount;
  int i;
  void (*foundSolution)(void) = foundSolutionVoidPtr;
  trailBacktrackTo(StartPoint);  // Start with backtracking
  dynamicFaceSetupCentral(args);
  dynamicSearch(true, foundSolution);
  used = clock() - now;
  if (SolutionCount != initialSolutionCount) {
    TotalUsefulTime += used;
    UsefulSearchCount += 1;

#define PRINT_TIME(clockValue, counter)                        \
  printf("[%1lu.%6.6lu (%d)] ", (clockValue) / CLOCKS_PER_SEC, \
         (clockValue) % CLOCKS_PER_SEC, counter)
    PRINT_TIME(used, 0);
    PRINT_TIME(TotalUsefulTime, UsefulSearchCount);
    PRINT_TIME(TotalWastedTime, WastedSearchCount);
    for (i = 0; i < NCOLORS; i++) {
      printf("%d ", args[i]);
    }
    printf(" gives %d new solutions\n", SolutionCount - initialSolutionCount);
    statisticPrintOneLine(position, true);
  } else {
    WastedSearchCount += 1;

    TotalWastedTime += used;
  }
}

void dynamicSearchFull(void (*foundSolution)(void))
{
  initializeSequenceOrder();
  initialize();
  StartPoint = Trail;
  SolutionCount = 0;
  dynamicFaceCanonicalCallback(fullSearchCallback, (void *)foundSolution);
}
