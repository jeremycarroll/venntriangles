
#include <assert.h>
#include <stdlib.h>

#include "main.h"

int main(int argc, char *argv[]) { return main0(argc, argv); }

/* These two functions are used in the unit tests from debug.c, which is
   included in the main runtime, but unused.
*/
void UnityAssertEqualNumber() { assert(0); }
void UnityFail() { assert(0); }
