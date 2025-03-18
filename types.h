#ifndef _TYPES_H
#define _TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* Basic type definitions */
typedef uint32_t COLOR;    /* Actually between 0 and 5. */
typedef uint32_t COLORSET; /* A set of bits, so between 0 and 63 */
typedef uint64_t
    uint_trail; /* Any non-pointer value that might go on the trail */

/* Forward declarations of struct types */
struct face;
struct edge;
struct undirectedPoint;
struct directedPoint;
struct facial_cycle;
struct trail;
struct failure;

/* Type aliases */
typedef struct face *FACE;
typedef struct edge *EDGE;
typedef struct undirectedPoint *UPOINT;
typedef struct directedPoint *DPOINT;
typedef struct facial_cycle *CYCLE;
typedef struct trail *TRAIL;
typedef struct failure *FAILURE;

/* Constants */
#ifndef NCOLORS
#define NCOLORS 6
#endif

#define NFACES (1 << NCOLORS)
#define NPOINTS ((1 << (NCOLORS - 2)) * NCOLORS * (NCOLORS - 1))
#define BITS_PER_WORD (sizeof(uint64_t) * 8)
#define POINTSET_LENGTH ((NPOINTS - 1) / BITS_PER_WORD + 1)

/* Macros */
#define memberOfColorSet(color, colorSet) (((colorSet) >> (color)) & 1u)
#define IS_PRIMARY_EDGE(edge) \
  (memberOfColorSet((edge)->color, (edge)->face->colors))

#endif /* _TYPES_H */