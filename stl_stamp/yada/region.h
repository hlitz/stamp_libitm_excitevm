/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "element.h"
#include "heap.h"
#include "mesh.h"
#include "coordinate.h"
#include "map.h"
#include "list.h"
#include "queue.h"

struct region_t {
    coordinate_t centerCoordinate;
    queue_t*     expandQueuePtr;
    list_t*   beforeListPtr; /* before retriangulation; list to avoid duplicates */
    list_t* borderListPtr; /* edges adjacent to region; list to avoid duplicates */
    vector_t*    badVectorPtr;

    region_t();
    ~region_t();

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
    long refine(element_t* elementPtr, mesh_t* meshPtr, bool* success);

    __attribute__((transaction_safe))
    void clearBad();

    __attribute__((transaction_safe))
    void transferBad(heap_t* workHeapPtr);
};
