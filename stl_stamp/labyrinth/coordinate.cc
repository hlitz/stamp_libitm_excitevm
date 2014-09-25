/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <math.h>
#include <stdlib.h>
#include "coordinate.h"
#include "pair.h"

/* =============================================================================
 * coordinate_alloc
 * =============================================================================
 */
coordinate_t::coordinate_t(long _x, long _y, long _z)
{
    x = _x;
    y = _y;
    z = _z;
}

/* =============================================================================
 * coordinate_isEqual
 * =============================================================================
 */
bool
coordinate_isEqual (coordinate_t* aPtr, coordinate_t* bPtr)
{
    if ((aPtr->x == bPtr->x) &&
        (aPtr->y == bPtr->y) &&
        (aPtr->z == bPtr->z))
        return true;
    else
      return false;
}


/* =============================================================================
 * getPairDistance
 * =============================================================================
 */
static double
getPairDistance (pair_t* pairPtr)
{
    coordinate_t* aPtr = (coordinate_t*)pairPtr->firstPtr;
    coordinate_t* bPtr = (coordinate_t*)pairPtr->secondPtr;
    long dx = aPtr->x - bPtr->x;
    long dy = aPtr->y - bPtr->y;
    long dz = aPtr->z - bPtr->z;
    long dx2 = dx * dx;
    long dy2 = dy * dy;
    long dz2 = dz * dz;
    return sqrt((double)(dx2 + dy2 + dz2));
}


/* =============================================================================
 * coordinate_comparePair
 * -- For sorting in list of source/destination pairs
 * -- Route longer paths first so they are more likely to succeed
 * =============================================================================
 */
long
coordinate_comparePair (const void* aPtr, const void* bPtr)
{
    double aDistance = getPairDistance((pair_t*)aPtr);
    double bDistance = getPairDistance((pair_t*)bPtr);

    if (aDistance < bDistance) {
        return 1;
    } else if (aDistance > bDistance) {
        return -1;
    }

    return 0;
}


/* =============================================================================
 * coordinate_areAdjacent
 * =============================================================================
 */
bool
coordinate_areAdjacent (coordinate_t* aPtr, coordinate_t* bPtr)
{
    long dx = aPtr->x - bPtr->x;
    long dy = aPtr->y - bPtr->y;
    long dz = aPtr->z - bPtr->z;
    long dx2 = dx * dx;
    long dy2 = dy * dy;
    long dz2 = dz * dz;

    return (((dx2 + dy2 + dz2) == 1) ? true : false);
}
