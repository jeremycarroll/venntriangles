/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef FACE_H
#define FACE_H

#include "dynamicface.h"

/* Global face array 
 * The face structure is defined in vertex.h
 */
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

