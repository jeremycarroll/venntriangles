#ifndef TRAIL_H
#define TRAIL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct DynamicTrail *TRAIL;
typedef uint64_t uint_trail;

struct DynamicTrail {
  void *ptr;
  uint_trail value;
};

/* TODO: improve this number, 10^6 looks very safe, but we should aim for less.
 */
#define TRAIL_SIZE 1000000

extern TRAIL DynamicTrail;
extern void dynamicTrailSetPointer(void **ptr, void *value);
#define setDynamicPointer(a, b) dynamicTrailSetPointer((void **)a, b)

extern void dynamicTrailSetInt(uint_trail *ptr, uint_trail value);
extern bool dynamicTrailBacktrackTo(TRAIL backtrackPoint);
extern void dynamicTrailMaybeSetInt(uint_trail *ptr, uint_trail value);

#endif  // TRAIL_H
