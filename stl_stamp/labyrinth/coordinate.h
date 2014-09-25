/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

struct coordinate_t {
    long x;
    long y;
    long z;
};


/* =============================================================================
 * coordinate_alloc
 * =============================================================================
 */
coordinate_t*
coordinate_alloc (long x, long y, long z);


/* =============================================================================
 * coordinate_free
 * =============================================================================
 */
void
coordinate_free (coordinate_t* coordinatePtr);


/* =============================================================================
 * coordinate_isEqual
 * =============================================================================
 */
bool
coordinate_isEqual (coordinate_t* aPtr, coordinate_t* bPtr);


/* =============================================================================
 * coordinate_comparePair
 * -- For sorting in list of source/destination pairs
 * =============================================================================
 */
long
coordinate_comparePair (const void* aPtr, const void* bPtr);


/* =============================================================================
 * coordinate_areAdjacent
 * =============================================================================
 */
bool
coordinate_areAdjacent (coordinate_t* aPtr, coordinate_t* bPtr);
