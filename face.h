/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FACE_H
#define FACE_H

#include "dynamicface.h"

/* Global variables */

/* Global face array - each entry represents a face with a specific color set
 * The face structure is defined in vertex.h */
extern struct face Faces[NFACES];

/* Array used to calculate expected cycle lengths for faces based on color count
 */
extern uint64 FaceSumOfFaceDegree[NCOLORS + 1];

/* Dynamic search functions - used in the solving algorithm */
extern FAILURE dynamicFaceBacktrackableChoice(FACE face);
extern FAILURE dynamicFaceChoice(FACE face, int depth);

/* Core face operations */
extern void initializeFacesAndEdges(void);
extern char* faceToString(FACE face);
extern FACE faceFromColors(char* colors);
extern void facePrintSelected(void);

#endif  // FACE_H
