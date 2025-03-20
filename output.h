#ifndef OUTPUT_H
#define OUTPUT_H

#include "graph.h"

extern char* edge2str(char* dbuffer, EDGE edge);
extern char* face2str(char* dbuffer, FACE face);
extern char* colors2str(char* dbuffer, COLORSET colors);
extern char* upoint2str(char* dbuffer, UPOINT up);
extern int color2char(COLOR c) ;

#endif // OUTPUT_H