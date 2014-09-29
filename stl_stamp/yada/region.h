/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "element.h"
#include "heap.h"
#include "mesh.h"

struct region_t;

/* =============================================================================
 * Pregion_alloc
 * =============================================================================
 */
region_t*
Pregion_alloc ();


/* =============================================================================
 * Pregion_free
 * =============================================================================
 */
void
Pregion_free (region_t* regionPtr);


/* =============================================================================
 * TMregion_refine
 *
 * Calculate refined triangles. The region is built by using a breadth-first
 * search starting from the element (elementPtr) containing the new point we
 * are adding. If expansion hits a boundary segment (encroachment) we build
 * a region around that element instead, to avoid a potential infinite loop.
 *
 * Returns net number of elements added to mesh.
 * =============================================================================
 */
__attribute__((transaction_safe))
long
TMregion_refine (region_t* regionPtr, element_t* elementPtr, mesh_t* meshPtr,
                 bool* success);


/* =============================================================================
 * Pregion_clearBad
 * =============================================================================
 */
__attribute__((transaction_safe))
//TM_PURE
void
Pregion_clearBad (region_t* regionPtr);


/* =============================================================================
 * TMregion_transferBad
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMregion_transferBad (region_t* regionPtr, heap_t* workHeapPtr);


#define PREGION_ALLOC()                 Pregion_alloc()
#define PREGION_FREE(r)                 Pregion_free(r)
#define PREGION_CLEARBAD(r)             Pregion_clearBad(r)
#define TMREGION_REFINE(r, e, m, s)        TMregion_refine(r, e, m, s)
#define TMREGION_TRANSFERBAD(r, q)      TMregion_transferBad(r, q)
