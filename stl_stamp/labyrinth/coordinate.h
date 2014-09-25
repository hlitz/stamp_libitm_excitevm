/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

struct coordinate_t {
    long x;
    long y;
    long z;

    coordinate_t(long x, long y, long z);

    // NB: no explicit destructor needed
};

bool coordinate_isEqual(coordinate_t* aPtr, coordinate_t* bPtr);

long coordinate_comparePair(const void* aPtr, const void* bPtr);

bool coordinate_areAdjacent(coordinate_t* aPtr, coordinate_t* bPtr);
