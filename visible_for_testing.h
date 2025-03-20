

extern void clearGlobals(void);
extern void clearInitialize(void);
extern CYCLESET_DECLARE pairs2cycleSets[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE triples2cycleSets[NCOLORS][NCOLORS][NCOLORS];
extern bool printCycleSet(CYCLESET cycleSet);
extern void printCycle(CYCLE cycle);
extern void clearInitialize(void);
extern void resetTrail(void);
extern void resetStatistics(void);
extern struct trail trailarray[TRAIL_SIZE];
extern uint64_t cycleGuessCounter;
extern FACE chooseFace(bool smallestFirst);
extern uint32_t cycleIdFromColors(char* colors);
extern FACE faceFromColors(char* colors);
extern struct undirectedPoint allUPoints[NPOINTS];
extern FACE addSpecificFace(char* colors, char* cycle);
extern int * intArray(int a, ...);

