

extern void resetGlobals(void);
extern void resetInitialize(void);
extern CYCLESET_DECLARE InitializeCycleSetPairs[NCOLORS][NCOLORS];
extern CYCLESET_DECLARE InitializeCycleSetTriples[NCOLORS][NCOLORS][NCOLORS];
extern bool dynamicCycleSetPrint(CYCLESET cycleSet);
extern void dynamicCyclePrint(CYCLE cycle);
extern void resetInitialize(void);
extern void resetTrail(void);
extern void resetStatistics(void);
extern struct DynamicTrail DynamicTrailArray[TRAIL_SIZE];
extern uint64_t DynamicCycleGuessCounter;
extern FACE dynamicFaceChoose(bool smallestFirst);
extern uint32_t dynamicCycleIdFromColors(char* colors);
extern FACE dynamicFaceFromColors(char* colors);
extern struct undirectedPoint DynamicPointAllUPoints[NPOINTS];
extern FACE dynamicFaceAddSpecific(char* colors, char* cycle);
extern int* dynamicIntArray(int a, ...);
