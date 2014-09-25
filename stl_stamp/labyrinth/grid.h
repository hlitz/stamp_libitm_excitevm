/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "vector.h"


typedef struct grid {
    long width;
    long height;
    long depth;
    long* points;
    long* points_unaligned;
} grid_t;

#define GRID_POINT_FULL  (-2L)
#define GRID_POINT_EMPTY (-1L)

/* =============================================================================
 * grid_alloc
 * =============================================================================
 */
grid_t*
grid_alloc (long width, long height, long depth);


/* =============================================================================
 * grid_free
 * =============================================================================
 */
void
grid_free (grid_t* gridPtr);


/* =============================================================================
 * grid_copy
 * =============================================================================
 */
__attribute__((transaction_pure)) // TODO: fixme
//TM_SAFE
void
grid_copy (grid_t* dstGridPtr, grid_t* srcGridPtr);


/* =============================================================================
 * grid_isPointValid
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
grid_isPointValid (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointRef
 * =============================================================================
 */
__attribute__((transaction_safe))
long*
grid_getPointRef (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointIndices
 * =============================================================================
 */
__attribute__((transaction_safe))
void
grid_getPointIndices (grid_t* gridPtr,
                      long* gridPointPtr, long* xPtr, long* yPtr, long* zPtr);


/* =============================================================================
 * grid_getPoint
 * =============================================================================
 */
//[wer]
__attribute__((transaction_safe))
long
grid_getPoint (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointEmpty
 * =============================================================================
 */
//[wer]
__attribute__((transaction_safe))
bool
grid_isPointEmpty (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointFull
 * =============================================================================
 */
//[wer]
__attribute__((transaction_safe))
bool
grid_isPointFull (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_setPoint
 * =============================================================================
 */
__attribute__((transaction_safe))
void
grid_setPoint (grid_t* gridPtr, long x, long y, long z, long value);


/* =============================================================================
 * grid_addPath
 * =============================================================================
 */
void
grid_addPath (grid_t* gridPtr, vector_t* pointVectorPtr);


/* =============================================================================
 * TMgrid_addPath
 * =============================================================================
 */
__attribute__((transaction_safe))
//void
bool
TMgrid_addPath (vector_t* pointVectorPtr);


/* =============================================================================
 * grid_print
 * =============================================================================
 */
void
grid_print (grid_t* gridPtr);


#define PGRID_ALLOC(x, y, z)            grid_alloc(x, y, z)
#define PGRID_FREE(g)                   grid_free(g)

#define TMGRID_ADDPATH(p)            TMgrid_addPath(p)
