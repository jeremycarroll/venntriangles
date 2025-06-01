/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MEMORY_H
#define MEMORY_H

#include <assert.h>
#include <stddef.h>

/**
 * Memory management system for temporary allocations.
 * Provides a simple arena allocator that can be bulk-freed.
 */

/* Memory allocation functions */
extern void initializeMemory(void);   /* Initialize memory system */
extern void *tempMalloc(size_t size); /* Allocate temporary memory */
extern void freeAll(void);            /* Free all temporary allocations */

/* String buffer functions */
extern char *getBuffer(void);           /* Get a temporary string buffer */
extern char *usingBuffer(char *buffer); /* Mark buffer as in-use */

/* Convenience macros for allocation */
#define NEW(type) ((type)tempMalloc(sizeof(((type)NULL)[0])))
#define NEW_ARRAY(type, size) \
  ((type)tempMalloc((size) * sizeof(((type)NULL)[0])))

#endif  // MEMORY_H
