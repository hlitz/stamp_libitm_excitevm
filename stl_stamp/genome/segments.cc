/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

/*
 * segments.c: Create random segments from random gene
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <random>
#include "gene.h"
#include "segments.h"
#include "utility.h"
#include "vector.h"


/* =============================================================================
 * segments_alloc
 * -- Does almost all the memory allocation for random segments
 * -- The actual number of segments created by 'segments_create' may be larger
 *    than 'minNum' to ensure the segments overlap and cover the entire gene
 * -- Returns NULL on failure
 * =============================================================================
 */
segments_t*
segments_alloc (long length, long minNum)
{
    segments_t* segmentsPtr;
    long i;
    char* string;

    segmentsPtr = (segments_t*)malloc(sizeof(segments_t));
    if (segmentsPtr == NULL) {
        return NULL;
    }

    /* Preallocate for the min number of segments we will need */
    segmentsPtr->strings = (char**)malloc(minNum * sizeof(char*));
    if (segmentsPtr->strings == NULL) {
        return NULL;
    }

    string = (char*)malloc(minNum * (length+1) * sizeof(char));
    if (string == NULL) {
        return NULL;
    }
    for (i = 0; i < minNum; i++) {
        segmentsPtr->strings[i] = &string[i * (length+1)];
        segmentsPtr->strings[i][length] = '\0';
    }
    segmentsPtr->minNum = minNum;
    segmentsPtr->length = length;

    segmentsPtr->contentsPtr = vector_alloc(minNum);
    if (segmentsPtr->contentsPtr == NULL) {
        return NULL;
    }

    return segmentsPtr;
}


/* =============================================================================
 * segments_create
 * -- Populates 'contentsPtr'
 * =============================================================================
 */
void
segments_create (segments_t* segmentsPtr, gene_t* genePtr, std::mt19937* randomPtr)
{
    vector_t* segmentsContentsPtr;
    char** strings;
    long segmentLength;
    long minNumSegment;
    char* geneString;
    long geneLength;
    bitmap_t* startBitmapPtr;
    long numStart;
    long i;
    long maxZeroRunLength;

    assert(segmentsPtr != NULL);
    assert(genePtr != NULL);
    assert(randomPtr != NULL);

    segmentsContentsPtr = segmentsPtr->contentsPtr;
    strings = segmentsPtr->strings;
    segmentLength = segmentsPtr->length;
    minNumSegment = segmentsPtr->minNum;

    geneString = genePtr->contents;
    geneLength = genePtr->length;
    startBitmapPtr = genePtr->startBitmapPtr;
    numStart = geneLength - segmentLength + 1;

    /* Pick some random segments to start */
    for (i = 0; i < minNumSegment; i++) {
        long j = (long)(randomPtr->operator()() % numStart);
        bool status = bitmap_set(startBitmapPtr, j);
        assert(status);
        memcpy(strings[i], &(geneString[j]), segmentLength * sizeof(char));
        status = vector_pushBack(segmentsContentsPtr, (void*)strings[i]);
        assert(status);
    }

    /* Make sure segment covers start */
    i = 0;
    if (!bitmap_isSet(startBitmapPtr, i)) {
        char* string = (char*)malloc((segmentLength+1) * sizeof(char));
        string[segmentLength] = '\0';
        memcpy(string, &(geneString[i]), segmentLength * sizeof(char));
        bool status = vector_pushBack(segmentsContentsPtr, (void*)string);
        assert(status);
        status = bitmap_set(startBitmapPtr, i);
        assert(status);
    }

    /* Add extra segments to fill holes and ensure overlap */
    maxZeroRunLength = segmentLength - 1;
    for (i = 0; i < numStart; i++) {
        long i_stop = MIN((i+maxZeroRunLength), numStart);
        for ( /* continue */; i < i_stop; i++) {
            if (bitmap_isSet(startBitmapPtr, i)) {
                break;
            }
        }
        if (i == i_stop) {
            /* Found big enough hole */
            char* string = (char*)malloc((segmentLength+1) * sizeof(char));
            string[segmentLength] = '\0';
            i = i - 1;
            memcpy(string, &(geneString[i]), segmentLength * sizeof(char));
            bool status = vector_pushBack(segmentsContentsPtr, (void*)string);
            assert(status);
            status = bitmap_set(startBitmapPtr, i);
            assert(status);
        }
    }
}


/* =============================================================================
 * segments_free
 * =============================================================================
 */
void
segments_free (segments_t* segmentsPtr)
{
    free(vector_at(segmentsPtr->contentsPtr, 0));
    vector_free(segmentsPtr->contentsPtr);
    free(segmentsPtr->strings);
    free(segmentsPtr);
}


/* =============================================================================
 * TEST_SEGMENTS
 * =============================================================================
 */
#ifdef TEST_SEGMENTS


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "types.h"


static void
tester (long geneLength, long segmentLength, long minNumSegment, bool doPrint)
{
    gene_t* genePtr;
    segments_t* segmentsPtr;
    random_t* randomPtr;
    bitmap_t* startBitmapPtr;
    long i;
    long j;

    genePtr = gene_alloc(geneLength);
    segmentsPtr = segments_alloc(segmentLength, minNumSegment);
    randomPtr = random_alloc();
    startBitmapPtr = Pbitmap_alloc(geneLength);

    random_seed(randomPtr, 0);
    gene_create(genePtr, randomPtr);
    random_seed(randomPtr, 0);
    segments_create(segmentsPtr, genePtr, randomPtr);

    assert(segmentsPtr->minNum == minNumSegment);
    assert(vector_getSize(segmentsPtr->contentsPtr) >= minNumSegment);

    if (doPrint) {
        printf("Gene = %s\n", genePtr->contents);
    }

    /* Check that each segment occurs in gene */
    for (i = 0; i < vector_getSize(segmentsPtr->contentsPtr); i++) {
        char *charPtr = strstr(genePtr->contents,
                               (char*)vector_at(segmentsPtr->contentsPtr, i));
        assert(charPtr != NULL);
        j = charPtr - genePtr->contents;
        bitmap_set(startBitmapPtr, j);
        if (doPrint) {
            printf("Segment %li (@%li) = %s\n",
                   i, j, (char*)vector_at(segmentsPtr->contentsPtr, i));
        }
    }

    /* Check that there is complete overlap */
    assert(bitmap_isSet(startBitmapPtr, 0));
    for (i = 0, j = 0; i < geneLength; i++ ) {
        if (bitmap_isSet(startBitmapPtr, i)) {
           assert((i-j-1) < segmentLength);
           j = i;
        }
    }

    gene_free(genePtr);
    segments_free(segmentsPtr);
    random_free(randomPtr);
    Pbitmap_free(startBitmapPtr);
}


int
main ()
{
    bool status = memory_init(1, 4, 2)
    assert(status);

    puts("Starting...");

    tester(10, 4, 20, true);
    tester(20, 5, 1, true);
    tester(100, 10, 1000, false);
    tester(100, 10, 1, false);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_SEGMENTS */


/* =============================================================================
 *
 * End of segments.c
 *
 * =============================================================================
 */
