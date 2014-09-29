/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

struct coordinate_t {
    double x;
    double y;
};


/* =============================================================================
 * coordinate_compare
 * =============================================================================
 */
__attribute__ ((transaction_safe))
long
coordinate_compare (coordinate_t* aPtr, coordinate_t* bPtr);


/* =============================================================================
 * coordinate_distance
 * =============================================================================
 */
__attribute__ ((transaction_safe))
double
coordinate_distance (coordinate_t* coordinatePtr, coordinate_t* aPtr);


/* =============================================================================
 * coordinate_angle
 * =============================================================================
 */
__attribute__ ((transaction_safe))
double
coordinate_angle (coordinate_t* coordinatePtr,
                  coordinate_t* aPtr, coordinate_t* bPtr);


/* =============================================================================
 * coordinate_print
 * =============================================================================
 */
void
coordinate_print (coordinate_t* coordinatePtr);
