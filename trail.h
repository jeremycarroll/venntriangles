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

extern TRAIL trail;
extern void setDynamicPointer_(void **ptr, void *value);
#define setDynamicPointer(a, b) setDynamicPointer_((void **)a, b)

extern void setDynamicInt(uint_trail *ptr, uint_trail value);
extern bool backtrackTo(TRAIL backtrackPoint);
extern void maybeSetDynamicInt(uint_trail *ptr, uint_trail value);

#endif  // TRAIL_H
