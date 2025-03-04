#include "venn.h"

static int shortCircuitResult;
// Get the first non-zero value of a, b, or c, without evaluating the others.
#define SHORT_CIRCUIT_OR(a, b, c)                           \
  (((shortCircuitResult = a) || (shortCircuitResult = b) || \
    (shortCircuitResult = c))                               \
       ? shortCircuitResult                                 \
       : 0)

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

/*
FACE choose(void) {
    FACE face = chooseFace();
    CYCLE cycle = chooseCycle(face);
    setDynamicPointer(&face->cycle, cycle);
}
*/

/* first cut code to be reworked.
#define backtrack break
void search() {
    Face backTrackFace = NULL;
    LabelSet backTrackLabelSet = 0;
    // should be in the face record?
    Cycle backTrackCycle = NULL;
    Face face;
    Face adjacentFaces[7];
    while (true) {
        while (true) {
            if (backTrackFace == NULL) {
                backTrackCycle = NULL;
                face = chooseFaceWithTrail();
                chosen[chosenCount++] = face;
                backTrackFace == face;
            } else {
                face = backTrackFace;
            }
            if (face == NULL) {
                if (solved()) { // not needed?
                    logSolution();
                }
                backTrackLabelSet = 0;
                backtrack;
            }
            Cycle cycle = chooseCycleWithTrail(face, backTrackCycle);
            backTrackCycle = cycle;
            if (cycle == NULL) {
                backtrack;
            }
            updateCycleWithTrail(face, cycle, adjacentFaces)

            // not disconnected loop

            backTrackLabelSet = SHORT_CIRCUIT_OR(possible(adjacentFaces),
sixCheck(cycle), corneringCheck(cycle));

            if (!isEmptyLabelSet(backTrackLabelSet)) {
                backTrackFace = face;
                backTrackLabel = chooseLabel(problems);
                backtrack;
            }
        }
        // backtrack
        // we need to back track to a choice point so far that all of the labels
in backTrackLabelSet have had some change. while (true) { if (cycle == NULL) {
                backTrackLabelSet &= ~face->cycle.labels
                face = chosen[--chosenCount]
            }
            backTrackLabelSet &= ~face->cycle.labels
            undoTrail(face->trail)

        }

    }
}
    */
