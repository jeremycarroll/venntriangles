#ifndef TRAIL_H
#define TRAIL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct trail *TRAIL;
typedef uint64_t uint_trail;

struct trail {
  void *ptr;
  uint_trail value;
};

/* TODO: improve this number, 10^6 looks very safe, but we should aim for less.
 */
#define TRAIL_SIZE 1000000

extern TRAIL Trail;
extern void trailSetPointer(void **ptr, void *value);
#define setDynamicPointer(a, b) trailSetPointer((void **)a, b)

extern void trailSetInt(uint_trail *ptr, uint_trail value);
extern bool trailBacktrackTo(TRAIL backtrackPoint);
extern void trailMaybeSetInt(uint_trail *ptr, uint_trail value);

extern void resetTrail(void);

#endif  // TRAIL_H
