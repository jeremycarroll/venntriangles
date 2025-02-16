#include "venn.h"
#include <string.h>

struct global globals;

void clearGlobals() {
    memset(&globals, 0, sizeof(globals));
}