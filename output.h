#ifndef OUTPUT_H
#define OUTPUT_H

#include "face.h"

extern char* dynamicEdgeToStr(char* dbuffer, EDGE edge);
extern char* dynamicFaceToStr(char* dbuffer, FACE face);
extern char* dynamicColorSetToStr(char* dbuffer, COLORSET colors);
extern char* dynamicUPointToStr(char* dbuffer, UPOINT up);
extern int dynamicColorToChar(COLOR c);

#endif  // OUTPUT_H
