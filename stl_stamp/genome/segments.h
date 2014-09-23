/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

/*
 * segments.h: Create random segments from random gene
 */

#pragma once

#include "gene.h"
#include "vector.h"


struct segments_t {
    long length;
    long minNum;
    vector_t* contentsPtr;
/* private: */
    char** strings;
};


/* =============================================================================
 * segments_alloc
 * -- Does almost all the memory allocation for random segments
 * -- The actual number of segments created by 'segments_create' may be larger
 *    than 'minNum' to ensure the segments overlap and cover the entire gene
 * -- Returns NULL on failure
 * =============================================================================
 */
segments_t*
segments_alloc (long length, long minNum);


/* =============================================================================
 * segments_create
 * -- Populates 'contentsPtr'
 * =============================================================================
 */
void
segments_create (segments_t* segmentsPtr, gene_t* genePtr, std::mt19937* randomPtr);


/* =============================================================================
 * segments_free
 * =============================================================================
 */
void
segments_free (segments_t* segmentsPtr);
