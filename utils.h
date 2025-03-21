#ifndef _UTILS_H
#define _UTILS_H

extern void resetGlobals(void);
extern void resetInitialize(void);
extern int* dynamicIntArray(int a, ...);
extern uint64_t DynamicCycleGuessCounter;
extern FACE dynamicFaceChoose(bool smallestFirst);
extern uint32_t dynamicCycleIdFromColors(char* colors);
extern FACE dynamicFaceFromColors(char* colors);
extern FACE dynamicFaceAddSpecific(char* colors, char* cycle);
extern void initialize(void);

extern void dynamicFacePrintSelected(void);

#endif  // _UTILS_H
