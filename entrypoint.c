
#include <assert.h>
#include <stdlib.h>

extern int dynamicMain0(int argc, char *argv[]);

int main(int argc, char *argv[]) { return dynamicMain0(argc, argv); }

/* These two functions are used in the unit tests from debug.c, which is
   included in the main runtime, but unused.
*/
void UnityAssertEqualNumber() { assert(0); }
void UnityFail() { assert(0); }
