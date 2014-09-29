/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include <random>
#include "element.h"
#include "map.h"
#include "vector.h"


struct mesh_t;


/* =============================================================================
 * mesh_alloc
 * =============================================================================
 */
mesh_t*
mesh_alloc ();


/* =============================================================================
 * mesh_free
 * =============================================================================
 */
void
mesh_free (mesh_t* meshPtr);



/* =============================================================================
 * TMmesh_insert
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMmesh_insert (mesh_t* meshPtr, element_t* elementPtr, MAP_T* edgeMapPtr);


/* =============================================================================
 * TMmesh_remove
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMmesh_remove ( mesh_t* meshPtr, element_t* elementPtr);


/* =============================================================================
 * TMmesh_insertBoundary
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
TMmesh_insertBoundary ( mesh_t* meshPtr, edge_t* boundaryPtr);


/* =============================================================================
 * TMmesh_removeBoundary
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
TMmesh_removeBoundary ( mesh_t* meshPtr, edge_t* boundaryPtr);


/* =============================================================================
 * mesh_read
 *
 * Returns number of elements read from file.
 *
 * Refer to http://www.cs.cmu.edu/~quake/triangle.html for file formats.
 * =============================================================================
 */
long
mesh_read (mesh_t* meshPtr, const char* fileNamePrefix);


/* =============================================================================
 * mesh_getBad
 * -- Returns NULL if none
 * =============================================================================
 */
element_t*
mesh_getBad (mesh_t* meshPtr);


/* =============================================================================
 * mesh_shuffleBad
 * =============================================================================
 */
void
mesh_shuffleBad (mesh_t* meshPtr, std::mt19937* randomPtr);


/* =============================================================================
 * mesh_check
 * =============================================================================
 */
bool
mesh_check (mesh_t* meshPtr, long expectedNumElement);
