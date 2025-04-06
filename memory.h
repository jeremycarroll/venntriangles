/* Copyright (C) 2024 Jeremy J. Carroll. See LICENSE for details. */

#ifndef MEMORY_H
#define MEMORY_H

#include <assert.h>
#include <stddef.h>

// External interface
extern void *tempMalloc(size_t size);
extern void freeAll(void);
extern void initializeMemory(void);

extern char *getBuffer(void);
extern char *usingBuffer(char *buffer);

// Convenience macros for allocation
#define NEW(type) ((type)tempMalloc(sizeof(((type)NULL)[0])))
#define NEW_ARRAY(type, size) \
  ((type)tempMalloc((size) * sizeof(((type)NULL)[0])))

#endif  // MEMORY_H
