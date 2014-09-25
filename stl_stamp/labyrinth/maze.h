/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "coordinate.h"
#include "grid.h"
#include "list.h"
#include "pair.h"
#include "queue.h"
#include <vector>

struct maze_t {
    grid_t* gridPtr;
    queue_t* workQueuePtr;   /* contains source/destination pairs to route */
    std::vector<coordinate_t*>* wallVectorPtr; /* obstacles */
    std::vector<coordinate_t*>* srcVectorPtr;  /* sources */
    std::vector<coordinate_t*>* dstVectorPtr;  /* destinations */

    maze_t();
    ~maze_t();

    /*
     * maze_read: Return number of path to route
     */
    long read(const char* inputFileName);

    bool checkPaths(list_t* pathListPtr, bool doPrintPaths);
};
