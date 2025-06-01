/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FACE_H
#define FACE_H

#include "dynamicface.h"

/* Core face structure */
struct face {
  MEMO COLORSET colors;
  MEMO FACE adjacentFaces[NCOLORS];
  MEMO struct edge edges[NCOLORS];
  DYNAMIC CYCLE cycle;
  DYNAMIC FACE next;
  DYNAMIC FACE previous;
  MEMO FACE nextByCycleId[NCYCLES];
  MEMO FACE previousByCycleId[NCYCLES];
  MEMO CYCLESET possibleCycles;
  DYNAMIC uint64 cycleSetSize;
};

/* Global face array */
extern struct face Faces[NFACES];

/* These next two functions are actually defined in search.c */
extern FAILURE dynamicFaceBacktrackableChoice(FACE face);
extern FAILURE dynamicFaceChoice(FACE face, int depth);

/* Core face structure and initialization functions */
extern void initializeFacesAndEdges(void);
extern char* faceToString(FACE face);
extern FACE faceFromColors(char* colors);
extern void facePrintSelected(void);

#endif  // FACE_H