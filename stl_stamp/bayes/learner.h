/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

/*
 * learn.h: Learns structure of Bayesian net from data
 */

#pragma once

#include "adtree.h"
#include "data.h"
#include "net.h"
#include "query.h"
#include "list.h"

struct learner_task_t;

/* ??? Cacheline size is fixed. */
#define CACHE_LINE_SIZE (64)

struct learner_t {
    adtree_t* adtreePtr;
    net_t* netPtr;
    float* localBaseLogLikelihoods;
    char pad1[CACHE_LINE_SIZE - sizeof(float*)];
    float baseLogLikelihood;
    char pad2[CACHE_LINE_SIZE - sizeof(float)];
    learner_task_t* tasks;
    char pad3[CACHE_LINE_SIZE - sizeof(learner_task_t*)];
    list_t* taskListPtr;
    char pad4[CACHE_LINE_SIZE - sizeof(list_t*)];
    long numTotalParent;
    char pad5[CACHE_LINE_SIZE - sizeof(long)];
};


/* =============================================================================
 * learner_alloc
 * =============================================================================
 */
learner_t*
learner_alloc (data_t* dataPtr, adtree_t* adtreePtr);


/* =============================================================================
 * learner_free
 * =============================================================================
 */
void
learner_free (learner_t* learnerPtr);


/* =============================================================================
 * learner_run
 * =============================================================================
 */
void
learner_run (learner_t* learnerPtr);


/* =============================================================================
 * learner_score
 * -- Score entire network
 * =============================================================================
 */
float
learner_score (learner_t* learnerPtr);

