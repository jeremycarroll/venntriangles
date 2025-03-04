#include "venn.h"

extern void clearGlobals(void);
extern void clearInitialize(void);
extern CYCLESET_DECLARE pairs2cycleSets[NCURVES][NCURVES];
extern CYCLESET_DECLARE triples2cycleSets[NCURVES][NCURVES][NCURVES];
extern bool printCycleSet(CYCLESET cycleSet);
extern void printCycle(CYCLE cycle);
extern void clearInitialize(void);
extern void resetTrail(void);
extern void resetStatistics(void);
extern struct trail trailarray[TRAIL_SIZE];
extern uint64_t cycleGuessCounter;
