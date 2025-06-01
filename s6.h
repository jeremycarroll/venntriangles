/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef S6_H
#define S6_H

#include "cycle.h"

/**
 * S6 Symmetry System for Venn diagram equivalence checking.
 *
 * The S6 system analyzes and compares Venn diagrams to determine
 * if they are equivalent under rotation, reflection, or relabeling.
 * It provides canonical representations for comparing diagrams.
 */

/*--------------------------------------
 * Type Definitions and Structures
 *--------------------------------------*/

/**
 * Indicates the symmetry status of a diagram.
 */
typedef enum {
  CANONICAL,     /* Diagram is in canonical form */
  EQUIVOCAL,     /* Diagram has rotational symmetry */
  NON_CANONICAL, /* Diagram is not in canonical form */
} SYMMETRY_TYPE;

/**
 * Sequence of face degrees for all faces in the diagram.
 */
typedef struct faceDegreeSequence {
  FACE_DEGREE faceDegrees[NFACES];
} *FACE_DEGREE_SEQUENCE;

/**
 * Sequence of cycle IDs for all faces in the diagram.
 */
typedef struct cycleIdSequence {
  CYCLE_ID faceCycleId[NFACES];
} *CYCLE_ID_SEQUENCE;

/**
 * Permutation of colors used for symmetry calculations.
 */
typedef int (*PERMUTATION)[NCOLORS];

/**
 * Signature representing a diagram's equivalence class.
 *
 * The classSignature is the largest signature based on any face and any
 * labelling. By construction the first element of the classSignature is
 * NCYCLES-1, the cycle a,b,c,d,e,f, around face 0. The isomorphism class
 * corresponds to drawing the diagram on a sphere, rather than the Euclidean
 * plane.
 *
 * To identify a specific Venn diagram, we identify the center face by the
 * offset. Conventionally we take the cycle a,b,c,d,e,f around that face
 * (NFACES-1), and it may have been reflected from the class representative
 * identified by the classSignature.
 */
typedef struct signature {
  struct cycleIdSequence classSignature; /* Canonical cycle sequence */
  COLORSET offset;                       /* Face offset for spherical mapping */
  bool reflected;                        /* Whether diagram is reflected */
} *SIGNATURE;

/**
 * Callback type for iterating over face degree permutations.
 *
 * The faceDegrees is an array of NCOLORS integers between 3 and NCOLORS to be
 * used as the face degrees for the NCOLORS faces around the central face.
 */
typedef void (*UseFaceDegrees)(void *data, FACE_DEGREE *faceDegrees);

/*--------------------------------------
 * Function Declarations
 *--------------------------------------*/

/* Initialization */
/**
 * Initialize the S6 symmetry system.
 */
extern void initializeS6(void);

/* Signature generation and comparison */
/**
 * Get the canonical signature from the current face configuration.
 */
extern SIGNATURE s6SignatureFromFaces(void);

/**
 * Get the maximum signature across all possible rotations/reflections.
 */
extern SIGNATURE s6MaxSignature(void);

/**
 * Convert a signature to its string representation.
 */
extern char *s6SignatureToString(SIGNATURE signature);

/**
 * Get the face degree signature for the current diagram.
 */
extern char *s6FaceDegreeSignature(void);

/* Symmetry analysis */
/**
 * Check the symmetry type of the current face configuration.
 */
extern SYMMETRY_TYPE s6FacesSymmetryType(void);

/**
 * Analyze symmetry of a given face degree sequence.
 */
extern SYMMETRY_TYPE s6SymmetryType6(FACE_DEGREE *faceDegrees);

#endif /* S6_H */
