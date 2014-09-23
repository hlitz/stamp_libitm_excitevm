/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

/*
 * gene.h: Create random gene
 */

#pragma once

#include <random>
#include "bitmap.h"

struct gene_t {
    long length;
    char* contents;
    // [mfs] replace with std::bitset?
    bitmap_t* startBitmapPtr; /* used for creating segments */
};

/* =============================================================================
 * gene_alloc
 * -- Does all memory allocation necessary for gene creation
 * -- Returns NULL on failure
 * =============================================================================
 */
gene_t* gene_alloc(long length);

/* =============================================================================
 * gene_create
 * -- Populate contents with random gene
 * =============================================================================
 */
void gene_create(gene_t* genePtr, std::mt19937* randomPtr);

/* =============================================================================
 * gene_free
 * =============================================================================
 */
void gene_free(gene_t* genePtr);
