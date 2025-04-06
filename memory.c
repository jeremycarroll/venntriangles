/* Copyright (C) 2024 Jeremy J. Carroll. See LICENSE for details. */

#include "memory.h"

#include "statistics.h"

#include <stdlib.h>
#include <string.h>

// Each allocation is wrapped in a header that links to the next allocation
typedef struct AllocationHeader {
  struct AllocationHeader *next;
  void *data;
} AllocationHeader;

// Global pointer to the start of the allocation chain
static AllocationHeader *chain = NULL;
/* Constants */
#define BUFFER_SIZE 128

/* Global variables (file scoped static) */
static uint64_t MaxBufferSize = 0;
static uint64_t CurrentMemory = 0;
static uint64_t MaxMemory = 0;

void *tempMalloc(size_t size)
{
  // Allocate header + requested size
  AllocationHeader *header = malloc(sizeof(AllocationHeader) + size);
  if (!header) return NULL;

  // Set up the header
  header->next = chain;
  header->data = (void *)(header + 1);  // Point to the data area

  // Update chain
  chain = header;
  CurrentMemory += size + sizeof(AllocationHeader);
  if (CurrentMemory > MaxMemory) {
    MaxMemory = CurrentMemory;
  }
  return header->data;
}

void freeAll(void)
{
  while (chain) {
    AllocationHeader *next = chain->next;
    free(chain);
    chain = next;
  }
  CurrentMemory = 0;
}

void initializeMemory()
{
  statisticIncludeInteger(&MaxBufferSize, "^", "MaxBuffer");
  statisticIncludeInteger(&CurrentMemory, "m", "CurrentMemory");
  statisticIncludeInteger(&MaxMemory, "M", "MaxMemory");
}

char *getBuffer() { return (char *)tempMalloc(BUFFER_SIZE); }

char *usingBuffer(char *buffer)
{
  uint64_t length = strlen(buffer);
  if (length > MaxBufferSize) {
    MaxBufferSize = length;
  }
  assert(length < BUFFER_SIZE);
  return buffer;
}
