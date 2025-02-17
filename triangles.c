
#include "venn.h"
/*
This file is responsible for checking that a set of edges can make a triangle.
*/

    /* TODO: curve checks: no more than one loop;
       no more than three corners.
    
    */
FAILURE curveChecks(EDGE edge) {
    // TODO:
    return NULL;
}
FAILURE checkCrossingLimit(COLOR a, COLOR b, int depth) {
    uint_trail * crossing = g_crossings[a];
    if (* crossing + 1 > MAX_ONE_WAY_CURVE_CROSSINGS) {
        return crossingLimitFailure(a, b, depth);
    }
    setDynamicInt(crossing, * crossing + 1);
    return NULL;
}