#include <string.h>

#include "venn.h"

struct global globals;

void clearGlobals() { memset(&globals, 0, sizeof(globals)); }
