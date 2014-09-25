/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coordinate.h"
#include "grid.h"
#include "vector.h"

/* ??? Cacheline size is fixed (set to 64 bytes for x86_64). */
const unsigned long CACHE_LINE_SIZE = 64UL;


/* =============================================================================
 * grid_alloc
 * =============================================================================
 */
grid_t*
grid_alloc (long width, long height, long depth)
{
    grid_t* gridPtr;

    gridPtr = (grid_t*)malloc(sizeof(grid_t));
    if (gridPtr) {
        gridPtr->width  = width;
        gridPtr->height = height;
        gridPtr->depth  = depth;
        long n = width * height * depth;
        long* points_unaligned = (long*)malloc(n * sizeof(long) + CACHE_LINE_SIZE);
        assert(points_unaligned);
        gridPtr->points_unaligned = points_unaligned;
        gridPtr->points = (long*)((char*)(((unsigned long)points_unaligned
                                          & ~(CACHE_LINE_SIZE-1)))
                                  + CACHE_LINE_SIZE);
        memset(gridPtr->points, GRID_POINT_EMPTY, (n * sizeof(long)));
    }

    return gridPtr;
}


/* =============================================================================
 * grid_free
 * =============================================================================
 */
void
grid_free (grid_t* gridPtr)
{
    free(gridPtr->points_unaligned);
    free(gridPtr);
}


/* =============================================================================
 * grid_copy
 * =============================================================================
 */
__attribute__((transaction_pure)) // TODO: FIXME
//TM_SAFE
void
grid_copy (grid_t* dstGridPtr, grid_t* srcGridPtr)
{
    assert(srcGridPtr->width  == dstGridPtr->width);
    assert(srcGridPtr->height == dstGridPtr->height);
    assert(srcGridPtr->depth  == dstGridPtr->depth);

    long n = srcGridPtr->width * srcGridPtr->height * srcGridPtr->depth;
    memcpy(dstGridPtr->points, srcGridPtr->points, (n * sizeof(long)));

#ifdef USE_EARLY_RELEASE
    long* srcPoints = srcGridPtr->points;
    long i;
    long i_step = (CACHE_LINE_SIZE / sizeof(srcPoints[0]));
    for (i = 0; i < n; i+=i_step) {
      /* releases entire line [wer] means nothing in gcctm */
    }
#endif
}


/* =============================================================================
 * grid_isPointValid
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
grid_isPointValid (grid_t* gridPtr, long x, long y, long z)
{
    if (x < 0 || x >= gridPtr->width  ||
        y < 0 || y >= gridPtr->height ||
        z < 0 || z >= gridPtr->depth)
    {
        return false;
    }

    return true;
}


/* =============================================================================
 * grid_getPointRef
 * =============================================================================
 */
__attribute__((transaction_safe))
long*
grid_getPointRef (grid_t* gridPtr, long x, long y, long z)
{
    return &(gridPtr->points[(z * gridPtr->height + y) * gridPtr->width + x]);
}


/* =============================================================================
 * grid_getPointIndices
 * =============================================================================
 */
__attribute__((transaction_safe))
void
grid_getPointIndices (grid_t* gridPtr,
                      long* gridPointPtr, long* xPtr, long* yPtr, long* zPtr)
{
    long height = gridPtr->height;
    long width  = gridPtr->width;
    long area = height * width;
    long index3d = (gridPointPtr - gridPtr->points);
    (*zPtr) = index3d / area;
    long index2d = index3d % area;
    (*yPtr) = index2d / width;
    (*xPtr) = index2d % width;
}


/* =============================================================================
 * grid_getPoint
 * =============================================================================
 */
__attribute__((transaction_safe))
long
grid_getPoint (grid_t* gridPtr, long x, long y, long z)
{
    return *grid_getPointRef(gridPtr, x, y, z);
}


/* =============================================================================
 * grid_isPointEmpty
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
grid_isPointEmpty (grid_t* gridPtr, long x, long y, long z)
{
    long value = grid_getPoint(gridPtr, x, y, z);
    return ((value == GRID_POINT_EMPTY) ? true : false);
}


/* =============================================================================
 * grid_isPointFull
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
grid_isPointFull (grid_t* gridPtr, long x, long y, long z)
{
    long value = grid_getPoint(gridPtr, x, y, z);
    return ((value == GRID_POINT_FULL) ? true : false);
}


/* =============================================================================
 * grid_setPoint
 * =============================================================================
 */
__attribute__((transaction_safe))
void
grid_setPoint (grid_t* gridPtr, long x, long y, long z, long value)
{
    (*grid_getPointRef(gridPtr, x, y, z)) = value;
}


/* =============================================================================
 * grid_addPath
 * =============================================================================
 */
void
grid_addPath (grid_t* gridPtr, vector_t* pointVectorPtr)
{
    long i;
    long n = vector_getSize(pointVectorPtr);

    for (i = 0; i < n; i++) {
        coordinate_t* coordinatePtr = (coordinate_t*)vector_at(pointVectorPtr, i);
        long x = coordinatePtr->x;
        long y = coordinatePtr->y;
        long z = coordinatePtr->z;
        grid_setPoint(gridPtr, x, y, z, GRID_POINT_FULL);
    }
}


/* =============================================================================
 * TMgrid_addPath
 * =============================================================================
 */
__attribute__((transaction_safe))
//void
bool
TMgrid_addPath (vector_t* pointVectorPtr)
{
    long i;
    long n = vector_getSize(pointVectorPtr);
#if 0
    for (i = 1; i < (n-1); i++) {
        long* gridPointPtr = (long*)vector_at(pointVectorPtr, i);
        long value = (long)TM_SHARED_READ(*gridPointPtr);
        if (value != GRID_POINT_EMPTY) {
          _ITM_abortTransaction(2);
        }
        TM_SHARED_WRITE(*gridPointPtr, GRID_POINT_FULL);
    }
#endif
    //[wer210] a check loop and a write loop
    for (i = 1; i < (n-1); i++) {
      long* gridPointPtr = (long*)vector_at(pointVectorPtr, i);
      long value = *gridPointPtr;
      if (value != GRID_POINT_EMPTY) {
        return false;
      }
      //TM_SHARED_WRITE(*gridPointPtr, (long)GRID_POINT_FULL);
    }

    for (i = 1; i < (n-1); i++) {
      long* gridPointPtr = (long*)vector_at(pointVectorPtr, i);
      *gridPointPtr = (long)GRID_POINT_FULL;
    }
    return true;
}


/* =============================================================================
 * grid_print
 * =============================================================================
 */
void
grid_print (grid_t* gridPtr)
{
    long width  = gridPtr->width;
    long height = gridPtr->height;
    long depth  = gridPtr->depth;
    long z;

    for (z = 0; z < depth; z++) {
        printf("[z = %li]\n", z);
        long x;
        for (x = 0; x < width; x++) {
            long y;
            for (y = 0; y < height; y++) {
                printf("%4li", *grid_getPointRef(gridPtr, x, y, z));
            }
            puts("");
        }
        puts("");
    }
}
