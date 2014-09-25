/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "coordinate.h"
#include "grid.h"
#include "list.h"
#include "pair.h"
#include "queue.h"
#include "vector.h"

typedef struct maze {
    grid_t* gridPtr;
    queue_t* workQueuePtr;   /* contains source/destination pairs to route */
    vector_t* wallVectorPtr; /* obstacles */
    vector_t* srcVectorPtr;  /* sources */
    vector_t* dstVectorPtr;  /* destinations */
} maze_t;


/* =============================================================================
 * maze_alloc
 * =============================================================================
 */
maze_t*
maze_alloc ();


/* =============================================================================
 * maze_free
 * =============================================================================
 */
void
maze_free (maze_t* mazePtr);


/* =============================================================================
 * maze_read
 * -- Return number of path to route
 * =============================================================================
 */
long
maze_read (maze_t* mazePtr, const char* inputFileName);


/* =============================================================================
 * maze_checkPaths
 * =============================================================================
 */
bool
maze_checkPaths (maze_t* mazePtr, list_t* pathListPtr, bool doPrintPaths);
