/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "hashtable.h"
#include "segments.h"
#include "table.h"

struct endInfoEntry_t;
struct constructEntry_t;

struct sequencer_t {

/* public: */

    char* sequence;

/* private: */

    segments_t* segmentsPtr;

    /* For removing duplicate segments */
    // [mfs] Replace with std::unordered_map
    hashtable_t* uniqueSegmentsPtr;

    /* For matching segments */
    endInfoEntry_t* endInfoEntries;
    table_t** startHashToConstructEntryTables;

    /* For constructing sequence */
    constructEntry_t* constructEntries;
    table_t* hashToConstructEntryTable;

    /* For deallocation */
    long segmentLength;

};


struct sequencer_run_arg_t {
    sequencer_t* sequencerPtr;
    segments_t* segmentsPtr;
    long preAllocLength;
    char* returnSequence; /* variable stores return value */
};


/* =============================================================================
 * sequencer_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
sequencer_t*
sequencer_alloc (long geneLength, long segmentLength, segments_t* segmentsPtr);


/* =============================================================================
 * sequencer_run
 * =============================================================================
 */

void
sequencer_run (void* argPtr);


/* =============================================================================
 * sequencer_free
 * =============================================================================
 */
void
sequencer_free (sequencer_t* sequencerPtr);
