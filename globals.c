#include "venn.h"
#include <string.h>

struct global globals;

void clear() {
    memset(&globals, 0, sizeof(globals));
}