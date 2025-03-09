#include "venn.h"

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

void search(bool smallestFirst, void (*foundSolution)(void))
{
  FACE face;
  FAILURE failure = NULL;
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
          foundSolution();
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
          failure = makeChoice(face);
          if (failure == NULL) {
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
