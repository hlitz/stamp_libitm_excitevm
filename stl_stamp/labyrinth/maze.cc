/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include "coordinate.h"
#include "grid.h"
#include "list.h"
#include "maze.h"
#include "queue.h"
#include "pair.h"
#include "vector.h"


/* =============================================================================
 * maze_alloc
 * =============================================================================
 */
maze_t::maze_t()
{
    gridPtr = NULL;
    workQueuePtr = queue_alloc(1024);
    wallVectorPtr = vector_alloc(1);
    srcVectorPtr = vector_alloc(1);
    dstVectorPtr = vector_alloc(1);
    assert(workQueuePtr &&
           wallVectorPtr &&
           srcVectorPtr &&
           dstVectorPtr);
}

/* =============================================================================
 * maze_free
 * =============================================================================
 */
maze_t::~maze_t()
{
    if (gridPtr != NULL) {
        delete(gridPtr);
    }
    queue_free(workQueuePtr);
    vector_free(wallVectorPtr);
    coordinate_t* coordPtr;
    while ((coordPtr = (coordinate_t *)vector_popBack(srcVectorPtr)) != NULL) {
        delete coordPtr;
    }
    vector_free(srcVectorPtr);
    while ((coordPtr = (coordinate_t *)vector_popBack (dstVectorPtr)) != NULL) {
        delete coordPtr;
    }
    vector_free(dstVectorPtr);
}


/* =============================================================================
 * addToGrid
 * =============================================================================
 */
static void
addToGrid (grid_t* gridPtr, vector_t* vectorPtr, const char* type)
{
    long i;
    long n = vector_getSize(vectorPtr);
    for (i = 0; i < n; i++) {
        coordinate_t* coordinatePtr = (coordinate_t*)vector_at(vectorPtr, i);
        if (!gridPtr->isPointValid(coordinatePtr->x,
                                   coordinatePtr->y,
                                   coordinatePtr->z))
        {
            fprintf(stderr, "Error: %s (%li, %li, %li) invalid\n",
                    type, coordinatePtr->x, coordinatePtr->y, coordinatePtr->z);
            exit(1);
        }
    }
    gridPtr->addPath(vectorPtr);
}


/* =============================================================================
 * maze_read
 * -- Return number of path to route
 * =============================================================================
 */
long maze_t::read(const char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "rt");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not read %s\n", inputFileName);
        exit(1);
    }

    /*
     * Parse input file
     */
    long lineNumber = 0;
    long height = -1;
    long width  = -1;
    long depth  = -1;
    char line[256];
    list_t* workListPtr = list_alloc(&coordinate_comparePair);

    while (fgets(line, sizeof(line), inputFile)) {

        char code;
        long x1, y1, z1;
        long x2, y2, z2;
        long numToken = sscanf(line, " %c %li %li %li %li %li %li",
                              &code, &x1, &y1, &z1, &x2, &y2, &z2);

        lineNumber++;

        if (numToken < 1) {
            continue;
        }

        switch (code) {
            case '#': { /* comment */
                /* ignore line */
                break;
            }
            case 'd': { /* dimensions (format: d x y z) */
                if (numToken != 4) {
                    goto PARSE_ERROR;
                }
                width  = x1;
                height = y1;
                depth  = z1;
                if (width < 1 || height < 1 || depth < 1) {
                    goto PARSE_ERROR;
                }
                break;
            }
            case 'p': { /* paths (format: p x1 y1 z1 x2 y2 z2) */
                if (numToken != 7) {
                    goto PARSE_ERROR;
                }
                coordinate_t* srcPtr = new coordinate_t(x1, y1, z1);
                coordinate_t* dstPtr = new coordinate_t(x2, y2, z2);
                assert(srcPtr);
                assert(dstPtr);
                if (coordinate_isEqual(srcPtr, dstPtr)) {
                    goto PARSE_ERROR;
                }
                pair_t* coordinatePairPtr = pair_alloc(srcPtr, dstPtr);
                assert(coordinatePairPtr);
                bool status = list_insert(workListPtr, (void*)coordinatePairPtr);
                assert(status == true);
                vector_pushBack(srcVectorPtr, (void*)srcPtr);
                vector_pushBack(dstVectorPtr, (void*)dstPtr);
                break;
            }
            case 'w': { /* walls (format: w x y z) */
                if (numToken != 4) {
                    goto PARSE_ERROR;
                }
                coordinate_t* wallPtr = new coordinate_t(x1, y1, z1);
                vector_pushBack(wallVectorPtr, (void*)wallPtr);
                break;
            }
            PARSE_ERROR:
            default: { /* error */
                fprintf(stderr, "Error: line %li of %s invalid\n",
                        lineNumber, inputFileName);
                exit(1);
            }
        }

    } /* iterate over lines in input file */

    fclose(inputFile);

    /*
     * Initialize grid contents
     */
    if (width < 1 || height < 1 || depth < 1) {
        fprintf(stderr, "Error: Invalid dimensions (%li, %li, %li)\n",
                width, height, depth);
        exit(1);
    }
    gridPtr = new grid_t(width, height, depth);
    assert(gridPtr);
    addToGrid(gridPtr, wallVectorPtr, "wall");
    addToGrid(gridPtr, srcVectorPtr,  "source");
    addToGrid(gridPtr, dstVectorPtr,  "destination");
    printf("Maze dimensions = %li x %li x %li\n", width, height, depth);
    printf("Paths to route  = %li\n", list_getSize(workListPtr));

    /*
     * Initialize work queue
     */
    list_iter_t it;
    list_iter_reset(&it, workListPtr);
    while (list_iter_hasNext(&it)) {
        pair_t* coordinatePairPtr = (pair_t*)list_iter_next(&it);
        queue_push(workQueuePtr, (void*)coordinatePairPtr);
    }
    list_free(workListPtr);

    return vector_getSize(srcVectorPtr);
}


/* =============================================================================
 * maze_checkPaths
 * =============================================================================
 */
bool maze_t::checkPaths(list_t* pathVectorListPtr, bool doPrintPaths)
{
    long width  = gridPtr->width;
    long height = gridPtr->height;
    long depth  = gridPtr->depth;
    long i;

    /* Mark walls */
    grid_t* testGridPtr = new grid_t(width, height, depth);
    testGridPtr->addPath(wallVectorPtr);

    /* Mark sources */
    long numSrc = vector_getSize(srcVectorPtr);
    for (i = 0; i < numSrc; i++) {
        coordinate_t* srcPtr = (coordinate_t*)vector_at(srcVectorPtr, i);
        testGridPtr->setPoint(srcPtr->x, srcPtr->y, srcPtr->z, 0);
    }

    /* Mark destinations */
    long numDst = vector_getSize(dstVectorPtr);
    for (i = 0; i < numDst; i++) {
        coordinate_t* dstPtr = (coordinate_t*)vector_at(dstVectorPtr, i);
        testGridPtr->setPoint(dstPtr->x, dstPtr->y, dstPtr->z, 0);
    }

    /* Make sure path is contiguous and does not overlap */
    long id = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it);
        long numPath = vector_getSize(pathVectorPtr);
        long i;
        for (i = 0; i < numPath; i++) {
            id++;
            vector_t* pointVectorPtr = (vector_t*)vector_at(pathVectorPtr, i);
            /* Check start */
            long* prevGridPointPtr = (long*)vector_at(pointVectorPtr, 0);
            long x;
            long y;
            long z;
            gridPtr->getPointIndices(prevGridPointPtr, &x, &y, &z);
            if (testGridPtr->getPoint(x, y, z) != 0) {
                delete testGridPtr;
                return false;
            }
            coordinate_t prevCoordinate(0,0,0);
            gridPtr->getPointIndices(prevGridPointPtr,
                                     &prevCoordinate.x,
                                     &prevCoordinate.y,
                                     &prevCoordinate.z);
            long numPoint = vector_getSize(pointVectorPtr);
            long j;
            for (j = 1; j < (numPoint-1); j++) { /* no need to check endpoints */
                long* currGridPointPtr = (long*)vector_at(pointVectorPtr, j);
                coordinate_t currCoordinate(0,0,0);
                gridPtr->getPointIndices(currGridPointPtr,
                                         &currCoordinate.x,
                                         &currCoordinate.y,
                                         &currCoordinate.z);
                if (!coordinate_areAdjacent(&currCoordinate, &prevCoordinate)) {
                    delete testGridPtr;
                    return false;
                }
                prevCoordinate = currCoordinate;
                long x = currCoordinate.x;
                long y = currCoordinate.y;
                long z = currCoordinate.z;
                if (testGridPtr->getPoint(x, y, z) != GRID_POINT_EMPTY) {
                    delete testGridPtr;
                    return false;
                } else {
                    testGridPtr->setPoint(x, y, z, id);
                }
            }
            /* Check end */
            long* lastGridPointPtr = (long*)vector_at(pointVectorPtr, j);
            gridPtr->getPointIndices(lastGridPointPtr, &x, &y, &z);
            if (testGridPtr->getPoint(x, y, z) != 0) {
                delete testGridPtr;
                return false;
            }
        } /* iteratate over pathVector */
    } /* iterate over pathVectorList */

    if (doPrintPaths) {
        puts("\nRouted Maze:");
        testGridPtr->print();
    }

    delete testGridPtr;

    return true;
}
