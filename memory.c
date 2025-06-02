/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#include "memory.h"

#include "statistics.h"

#include <stdlib.h>

// Each allocation is wrapped in a header that links to the next allocation
typedef struct AllocationHeader {
  struct AllocationHeader *next;
  void *data;
} AllocationHeader;

// Global pointer to the start of the allocation chain
static AllocationHeader *chain = NULL;
#define BUFFER_SIZE 256

static uint64 MaxBufferSize = 0;
static uint64 CurrentMemory = 0;
static uint64 MaxMemory = 0;

void initializeMemory()
{
  statisticIncludeInteger(&MaxBufferSize, "B", "MaxBuffer", true);
  statisticIncludeInteger(&CurrentMemory, "C", "CurrentMemory", true);
  statisticIncludeInteger(&MaxMemory, "M", "MaxMemory", true);
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

void *tempMalloc(size_t size)
{
  // Allocate header + requested size
  AllocationHeader *header = malloc(sizeof(AllocationHeader) + size);
  if (!header) return NULL;

  // Set up the header
  header->next = chain;
  header->data = (void *)(header + 1);  // Vertex to the data area

  // Update chain
  chain = header;
  CurrentMemory += size + sizeof(AllocationHeader);
  if (CurrentMemory > MaxMemory) {
    MaxMemory = CurrentMemory;
  }
  return header->data;
}

char *getBuffer()
{
  return (char *)tempMalloc(BUFFER_SIZE);
}

char *usingBuffer(char *buffer)
{
  uint64 length = strlen(buffer);
  if (length > MaxBufferSize) {
    MaxBufferSize = length;
  }
  assert(length < BUFFER_SIZE);
  return buffer;
}
