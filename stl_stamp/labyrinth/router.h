/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "grid.h"
#include "maze.h"
#include "vector.h"

typedef struct router {
    long xCost;
    long yCost;
    long zCost;
    long bendCost;
} router_t;

typedef struct router_solve_arg {
    router_t* routerPtr;
    maze_t* mazePtr;
    list_t* pathVectorListPtr;
} router_solve_arg_t;


/* =============================================================================
 * router_alloc
 * =============================================================================
 */
router_t*
router_alloc (long xCost, long yCost, long zCost, long bendCost);


/* =============================================================================
 * router_free
 * =============================================================================
 */
void
router_free (router_t* routerPtr);


/* =============================================================================
 * router_solve
 * =============================================================================
 */
void
router_solve (void* argPtr);
