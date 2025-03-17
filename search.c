#include <time.h>

#include "d6.h"
#include "venn.h"
#include "visible_for_testing.h"

FACE chooseFace(bool smallestFirst)
{
  FACE face = NULL;
  int sign = smallestFirst ? 1 : -1;
  int64_t min = smallestFirst ? (NCYCLES + 1) : 0;
  int i;
  for (i = 0; i < NFACES; i++) {
    if (sign * (int64_t)g_faces[i].cycleSetSize < min &&
        g_faces[i].cycle == NULL) {
      min = sign * (int64_t)g_faces[i].cycleSetSize;
      face = g_faces + i;
    }
  }
  return face;
}

static CYCLE chooseCycle(FACE face, CYCLE cycle)
{
  return findNextCycleInSet(face->possibleCycles, cycle);
}

static int solution_count = 0;
void search(bool smallestFirst, void (*foundSolution)(void))
{
  FACE face;
  FACE chosenFaces[NFACES];
  CYCLE chosenCycles[NFACES];
  CYCLE cycle;
  int position = 0;
  enum { NEXT_FACE, NEXT_CYCLE } state = NEXT_FACE;
  while (position >= 0) {
    printStatisticsOneLine(position);
    switch (state) {
      case NEXT_FACE:
        face = chooseFace(smallestFirst);
        if (face == NULL) {
          if (finalCorrectnessChecks() == NULL) {
            solution_count++;
            foundSolution();
          }
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          face->backtrack = trail;
          chosenFaces[position] = face;
          chosenCycles[position] = NULL;
          state = NEXT_CYCLE;
        }
        break;
      case NEXT_CYCLE:
        face = chosenFaces[position];
        if (backtrackTo(face->backtrack)) {
#if BACKTRACK_DEBUG
          printf("Backtracking to ");
          printFace(face);
#endif
        }
        cycle = chooseCycle(face, chosenCycles[position]);
        if (cycle == NULL) {
          position -= 1;
          state = NEXT_CYCLE;
        } else {
          chosenCycles[position] = cycle;
          /* suspect - because face->backtrack gets reset. */
          setDynamicPointer(&face->cycle, cycle);
          assert(face->cycle == cycle);
          if (makeChoice(face) == NULL) {
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

void enumerateSolutions(FILE *(*fp)(char *signature, int soln, int subsoln))
{
  char *signature = d6FaceDegreeSignature();
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
  clearGlobals();
  clearInitialize();
  resetTrail();
  initialize();
  setupCentralFaces(args);
  search(true, foundSolution);
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

void full_search(void (*foundSolution)(void))
{
  solution_count = 0;
  canoncialCallback(full_search_callback6, (void *)foundSolution);
#if STATS
  printStatisticsFull();
#endif
}
