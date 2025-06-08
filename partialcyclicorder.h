/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef PARTIALCYCLICORDER_H
#define PARTIALCYCLICORDER_H

#include "core.h"

#define PCO_LINES (NCOLORS * MAX_CORNERS)
#define PCO_TRIPLES ((PCO_LINES * (PCO_LINES - 1) * (PCO_LINES - 2)) / 3)

extern void initializePartialCyclicOrder(void);

extern bool dynamicPCOSet(int line1, int line2, int line3);
extern bool dynamicPCOClosure(void);
extern bool dynamicPCOComplete(void);

#endif /* PARTIALCYCLICORDER_H */
