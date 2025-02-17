#include "venn.h"

/*
We run single threaded and have only one active failure at any time;
hence for each failure we have a single static instance, which records the current state
of the failure as we backtrack.abort
Each of the static failure also has a counter, which is incremented on every use, so
that we can skip stats.
*/

#define MULTIPLE_FAILURE_TYPE 0x200
#define NO_MATCH_FAILURE_TYPE 0x1
static struct failure multipleFailure = {0, "Multiple failures", 0, NULL};

 FAILURE maybeAddFailure(FAILURE failureCollection, FAILURE newFailure) {
     if (newFailure == NULL) {
         return failureCollection;
     }
     if (failureCollection == NULL) {
        failureCollection = &multipleFailure;
        multipleFailure.type = MULTIPLE_FAILURE_TYPE | newFailure->type;
     }
     // TODO: collect failures, and checktype
     assert(failureCollection == &multipleFailure);
     assert(failureCollection->type == (MULTIPLE_FAILURE_TYPE | newFailure->type));
     failureCollection->count++;
     switch(newFailure->type) {
         case NO_MATCH_FAILURE_TYPE:
         break;
         default:
         assert(NULL == "Unknown failure type");
        }

     return failureCollection;
 }

 static COLORSET noMatchColorset;
 static struct failure noMatchFailure = {NO_MATCH_FAILURE_TYPE, "No matching cycles", 0, &noMatchColorset};

FAILURE noMatchingCyclesFailure(COLORSET colors, uint32_t depth) {
    *(COLORSET*) noMatchFailure.moreInfo = colors;
    noMatchFailure.count++;
    return &noMatchFailure;
}