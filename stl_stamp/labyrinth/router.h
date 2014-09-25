/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "grid.h"
#include "maze.h"

struct router_t {
    long xCost;
    long yCost;
    long zCost;
    long bendCost;

    router_t(long xCost, long yCost, long zCost, long bendCost);

    // NB: implicit destructor suffices
};

struct router_solve_arg_t {
    router_t* routerPtr;
    maze_t* mazePtr;
    list_t* pathVectorListPtr;
};

/* =============================================================================
 * router_solve
 * =============================================================================
 */
void router_solve(void* argPtr);
