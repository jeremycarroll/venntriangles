/* Copyright (C) 2025 Jeremy J. Carroll. See LICENSE for details. */

#ifndef DYNAMICFACE_H
#define DYNAMICFACE_H

#include "failure.h"
#include "trail.h"
#include "vertex.h"

/**
 * Dynamic face system for backtracking-aware face operations.
 * Provides functions to manipulate faces with full trail support
 * for undo operations during backtracking.
 */

/* Forward declarations */
typedef struct face* FACE;

/* Initialization functions */
/**
 * Sets up central faces with specified degrees.
 * @param faceDegrees Array of face degrees to use
 */
extern void dynamicFaceSetupCentral(FACE_DEGREE* faceDegrees);

/**
 * Sets the exact cycle length for a face with given colors.
 * @param faceColors Bit set representing face colors
 * @param length Required cycle length
 * @return true if setting was successful, false otherwise
 */
extern bool dynamicFaceSetCycleLength(uint32_t faceColors, FACE_DEGREE length);

/* Cycle and vertex manipulation */
/**
 * Includes a vertex in a face, connecting specified colors.
 * @param face Face to modify
 * @param aColor First color to connect
 * @param bColor Second color to connect
 * @param depth Current search depth
 * @return Failure object if operation fails, NULL otherwise
 */
extern FAILURE dynamicFaceIncludeVertex(FACE face, COLOR aColor, COLOR bColor,
                                        int depth);

/**
 * Removes a color from further consideration in search.
 * @param color The color to remove
 * @return true if color was successfully removed
 */
extern bool dynamicColorRemoveFromSearch(COLOR color);

/**
 * Propagates cycle constraints from a face through the diagram.
 * @param face Starting face for propagation
 * @param onlyCycleSet Restricted set of cycles to consider
 * @param depth Current search depth
 * @return Failure object if propagation fails, NULL otherwise
 */
extern FAILURE dynamicFaceRestrictAndPropagateCycles(FACE face,
                                                     CYCLESET onlyCycleSet,
                                                     int depth);

/**
 * Propagates edge choice constraints to connected faces.
 * @param face Source face for propagation
 * @param edge Edge that was selected
 * @param depth Current search depth
 * @return Failure object if propagation fails, NULL otherwise
 */
extern FAILURE dynamicFacePropagateChoice(FACE face, EDGE edge, int depth);

/* Validation and finalization */
/**
 * Updates the count of available choices for a face.
 * @param face Face to update
 */
extern void dynamicRecomputeCountOfChoices(FACE face);

/**
 * Performs final correctness checks on the diagram.
 * @return Failure object if check fails, NULL otherwise
 */
extern FAILURE dynamicFaceFinalCorrectnessChecks(void);

/* Performance counters */
/**
 * Counter for tracking how many cycles were forced during search.
 */
extern uint64 CycleForcedCounter;

/**
 * Counter for tracking how many cycle sets were reduced during search.
 */
extern uint64 CycleSetReducedCounter;

#endif /* DYNAMICFACE_H */
