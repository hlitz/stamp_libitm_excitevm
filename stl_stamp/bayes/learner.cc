/* =============================================================================
 *
 * learn.c
 * -- Learns structure of Bayesian net from data
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * The penalized log-likelihood score (Friedman & Yahkani, 1996) is used to
 * evaluated the "goodness" of a Bayesian net:
 *
 *                             M      n_j
 *                            --- --- ---
 *  -N_params * ln(R) / 2 + R >   >   >   P((a_j = v), X_j) ln P(a_j = v | X_j)
 *                            --- --- ---
 *                            j=1 X_j v=1
 *
 * Where:
 *
 *     N_params     total number of parents across all variables
 *     R            number of records
 *     M            number of variables
 *     X_j          parents of the jth variable
 *     n_j          number of attributes of the jth variable
 *     a_j          attribute
 *
 * The second summation of X_j varies across all possible assignments to the
 * values of the parents X_j.
 *
 * In the code:
 *
 *    "local log likelihood" is  P((a_j = v), X_j) ln P(a_j = v | X_j)
 *    "log likelihood" is everything to the right of the '+', i.e., "R ... X_j)"
 *    "base penalty" is -ln(R) / 2
 *    "penalty" is N_params * -ln(R) / 2
 *    "score" is the entire expression
 *
 * For more notes, refer to:
 *
 * A. Moore and M.-S. Lee. Cached sufficient statistics for efficient machine
 * learning with large datasets. Journal of Artificial Intelligence Research 8
 * (1998), pp 67-91.
 *
 * =============================================================================
 *
 * The search strategy uses a combination of local and global structure search.
 * Similar to the technique described in:
 *
 * D. M. Chickering, D. Heckerman, and C. Meek.  A Bayesian approach to learning
 * Bayesian networks with local structure. In Proceedings of Thirteenth
 * Conference on Uncertainty in Artificial Intelligence (1997), pp. 80-89.
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 *
 * ------------------------------------------------------------------------
 *
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 *
 * ------------------------------------------------------------------------
 *
 * For the license of ssca2, please see ssca2/COPYRIGHT
 *
 * ------------------------------------------------------------------------
 *
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 *
 * ------------------------------------------------------------------------
 *
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 *
 * ------------------------------------------------------------------------
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <random>
#include "adtree.h"
#include "data.h"
#include "learner.h"
#include "list.h"
#include "net.h"
#include "operation.h"
#include "query.h"
#include "thread.h"
#include "timer.h"
#include "utility.h"
#include "vector.h"
#include "tm_transition.h"

__attribute__ ((transaction_pure))
void tmp_string(char* s)
{
  printf("%s", s);
}
__attribute__ ((transaction_pure))
void tmp_int(int i)
{
  printf("%d", i);
}
__attribute__ ((transaction_pure))
void tmp_long(long l)
{
  printf("%ld", l);
}
__attribute__ ((transaction_pure))
void tmp_float(float f)
{
  printf("%f", f);
}

struct learner_task {
    operation_t op;
    long fromId;
    long toId;
    float score;
};

typedef struct findBestTaskArg {
    long toId;
    learner_t* learnerPtr;
    query_t* queries;
    vector_t* queryVectorPtr;
    vector_t* parentQueryVectorPtr;
    long numTotalParent;
    float basePenalty;
    float baseLogLikelihood;
    bitmap_t* bitmapPtr;
    queue_t* workQueuePtr;
    vector_t* aQueryVectorPtr;
    vector_t* bQueryVectorPtr;
} findBestTaskArg_t;

#ifdef TEST_LEARNER
long global_maxNumEdgeLearned = -1L;
long global_insertPenalty = 1;
float global_operationQualityFactor = 1.0F;
#else
extern long global_insertPenalty;
extern long global_maxNumEdgeLearned;
extern float global_operationQualityFactor;
#endif


/* =============================================================================
 * DECLARATION OF TM_SAFE FUNCTIONS
 * =============================================================================
 */

__attribute__((transaction_safe))
void
TMfindBestReverseTask (learner_task_t *dest,  findBestTaskArg_t* argPtr);

__attribute__((transaction_safe))
void
TMfindBestInsertTask (learner_task_t *dest,  findBestTaskArg_t* argPtr);

__attribute__((transaction_safe))
void
TMfindBestRemoveTask (learner_task_t *dest,  findBestTaskArg_t* argPtr);

__attribute__((transaction_safe))
void
TMpopulateParentQueryVector (net_t* netPtr,
                             long id,
                             query_t* queries,
                             vector_t* parentQueryVectorPtr);

__attribute__((transaction_safe))
void
TMpopulateQueryVectors (net_t* netPtr,
                        long id,
                        query_t* queries,
                        vector_t* queryVectorPtr,
                        vector_t* parentQueryVectorPtr);

__attribute__((transaction_safe))
learner_task_t*
TMpopTask (list_t* taskListPtr);

/* =============================================================================
 * compareTask
 * -- Want greatest score first
 * -- For list
 * =============================================================================
 */
//[wer] comparator
__attribute__((transaction_safe)) long
compareTask (const void* aPtr, const void* bPtr)
{
    learner_task_t* aTaskPtr = (learner_task_t*)aPtr;
    learner_task_t* bTaskPtr = (learner_task_t*)bPtr;
    float aScore = aTaskPtr->score;
    float bScore = bTaskPtr->score;

    if (aScore < bScore) {
        return 1;
    } else if (aScore > bScore) {
        return -1;
    } else {
        return (aTaskPtr->toId - bTaskPtr->toId);
    }
}


/* =============================================================================
 * compareQuery
 * -- Want smallest ID first
 * -- For vector_sort
 * =============================================================================
 */
__attribute__((transaction_safe)) int
compareQuery (const void* aPtr, const void* bPtr)
{
    query_t* aQueryPtr = (query_t*)(*(void**)aPtr);
    query_t* bQueryPtr = (query_t*)(*(void**)bPtr);

    return (aQueryPtr->index - bQueryPtr->index);
}


/* =============================================================================
 * learner_alloc
 * =============================================================================
 */
learner_t*
learner_alloc (data_t* dataPtr, adtree_t* adtreePtr)
{
    learner_t* learnerPtr;

    learnerPtr = (learner_t*)malloc(sizeof(learner_t));
    if (learnerPtr) {
        learnerPtr->adtreePtr = adtreePtr;
        learnerPtr->netPtr = net_alloc(dataPtr->numVar);
        assert(learnerPtr->netPtr);
        learnerPtr->localBaseLogLikelihoods =
            (float*)malloc(dataPtr->numVar * sizeof(float));
        assert(learnerPtr->localBaseLogLikelihoods);
        learnerPtr->baseLogLikelihood = 0.0F;
        learnerPtr->tasks =
            (learner_task_t*)malloc(dataPtr->numVar * sizeof(learner_task_t));
        assert(learnerPtr->tasks);
        learnerPtr->taskListPtr = list_alloc(&compareTask);
        assert(learnerPtr->taskListPtr);
        learnerPtr->numTotalParent = 0;
    }

    return learnerPtr;
}


/* =============================================================================
 * learner_free
 * =============================================================================
 */
void
learner_free (learner_t* learnerPtr)
{
    list_free(learnerPtr->taskListPtr);
    free(learnerPtr->tasks);
    free(learnerPtr->localBaseLogLikelihoods);
    net_free(learnerPtr->netPtr);
    free(learnerPtr);
}
/* =============================================================================
 * Logarithm(s)
 * =============================================================================
 */
__attribute__((transaction_safe))
double my_exp(double x, int y)
{
    double ret = 1;
    int i;
    for (i = 0; i < y; i++)
        ret *= x;
    return ret;
}

// Taylor series to calculate __attribute__((transaction_safe)) logarithm
__attribute__((transaction_safe))
double TM_log (double x)
{
  assert (x > 0);
  double ret = 0.0;
  int i;

  if (ABS(x - 1) <= 0.000001)
    return ret;
  else if (ABS(x) > 1) {
    double y = x / (x - 1);
    // more iteration, more precise
    for (i = 1; i < 20; i++) {
      ret += 1 / (i * my_exp (y, i));
    }
    return ret;
  }
  else{
    double y = x - 1;
    // more iteration, more precise
    for (i = 1; i < 20; i++) {
      if (i % 2 == 1)
        ret += my_exp (y, i) / i;
      else
        ret -= my_exp (y, i) / i;
    }
    return ret;
  }
}

__attribute__((transaction_pure)) //TM_PURE logarithm
double PURE_log (double dat)
{
  return (double)log(dat);
}

/* =============================================================================
 * computeSpecificLocalLogLikelihood
 * -- Query vectors should not contain wildcards
 * =============================================================================
 */
__attribute__((transaction_safe))
float
computeSpecificLocalLogLikelihood (adtree_t* adtreePtr,
                                   vector_t* queryVectorPtr,
                                   vector_t* parentQueryVectorPtr)
{
  //[wer] __attribute__((transaction_safe)) call
  long count = adtree_getCount(adtreePtr, queryVectorPtr);
  if (count == 0) {
    return 0.0;
  }

  double probability = (double)count / (double)adtreePtr->numRecord;
  long parentCount = adtree_getCount(adtreePtr, parentQueryVectorPtr);

  assert(parentCount >= count);
  assert(parentCount > 0);

  //[wer210] replaced with __attribute__((transaction_safe)) log()
  double temp = (double)count/ (double)parentCount;
  //double t = (double)PURE_log(temp);
  double t = (double)TM_log(temp);
  return (float)(probability * t);
}


/* =============================================================================
 * createPartition
 * =============================================================================
 */
void
createPartition (long min, long max, long id, long n,
                 long* startPtr, long* stopPtr)
{
    long range = max - min;
    long chunk = MAX(1, ((range + n/2) / n)); /* rounded */
    long start = min + chunk * id;
    long stop;
    if (id == (n-1)) {
        stop = max;
    } else {
        stop = MIN(max, (start + chunk));
    }

    *startPtr = start;
    *stopPtr = stop;
}


/* =============================================================================
 * createTaskList
 * -- baseLogLikelihoods and taskListPtr are updated
 * =============================================================================
 */
void
createTaskList (void* argPtr)
{
    long myId = thread_getId();
    long numThread = thread_getNumThread();

    learner_t* learnerPtr = (learner_t*)argPtr;
    list_t* taskListPtr = learnerPtr->taskListPtr;

    bool status;

    adtree_t* adtreePtr = learnerPtr->adtreePtr;
    float* localBaseLogLikelihoods = learnerPtr->localBaseLogLikelihoods;
    learner_task_t* tasks = learnerPtr->tasks;

    query_t queries[2];
    vector_t* queryVectorPtr = PVECTOR_ALLOC(2);
    assert(queryVectorPtr);
    status = vector_pushBack(queryVectorPtr, (void*)&queries[0]);
    assert(status);

    query_t parentQuery;
    vector_t* parentQueryVectorPtr = PVECTOR_ALLOC(1);
    assert(parentQueryVectorPtr);

    long numVar = adtreePtr->numVar;
    long numRecord = adtreePtr->numRecord;
    float baseLogLikelihood = 0.0;
    float penalty = (float)(-0.5 * log((double)numRecord)); /* only add 1 edge */

    long v;

    long v_start;
    long v_stop;
    createPartition(0, numVar, myId, numThread, &v_start, &v_stop);

    /*
     * Compute base log likelihood for each variable and total base loglikelihood
     */

    for (v = v_start; v < v_stop; v++) {

        float localBaseLogLikelihood = 0.0;
        queries[0].index = v;

        queries[0].value = 0;
        localBaseLogLikelihood +=
            computeSpecificLocalLogLikelihood(adtreePtr,
                                              queryVectorPtr,
                                              parentQueryVectorPtr);

        queries[0].value = 1;
        localBaseLogLikelihood +=
            computeSpecificLocalLogLikelihood(adtreePtr,
                                              queryVectorPtr,
                                              parentQueryVectorPtr);

        localBaseLogLikelihoods[v] = localBaseLogLikelihood;
        baseLogLikelihood += localBaseLogLikelihood;

    } /* foreach variable */

    __transaction_atomic {
      float globalBaseLogLikelihood = learnerPtr->baseLogLikelihood;
      learnerPtr->baseLogLikelihood =
                        baseLogLikelihood + globalBaseLogLikelihood;
   }

    /*
     * For each variable, find if the addition of any edge _to_ it is better
     */

    status = PVECTOR_PUSHBACK(parentQueryVectorPtr, (void*)&parentQuery);
    assert(status);

    for (v = v_start; v < v_stop; v++) {

        /*
         * Compute base log likelihood for this variable
         */

        queries[0].index = v;
        long bestLocalIndex = v;
        float bestLocalLogLikelihood = localBaseLogLikelihoods[v];

        status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[1]);
        assert(status);

        long vv;
        for (vv = 0; vv < numVar; vv++) {

            if (vv == v) {
                continue;
            }
            parentQuery.index = vv;
            if (v < vv) {
                queries[0].index = v;
                queries[1].index = vv;
            } else {
                queries[0].index = vv;
                queries[1].index = v;
            }

            float newLocalLogLikelihood = 0.0;

            queries[0].value = 0;
            queries[1].value = 0;
            parentQuery.value = 0;
            newLocalLogLikelihood +=
                computeSpecificLocalLogLikelihood(adtreePtr,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);

            queries[0].value = 0;
            queries[1].value = 1;
            parentQuery.value = ((vv < v) ? 0 : 1);
            newLocalLogLikelihood +=
                computeSpecificLocalLogLikelihood(adtreePtr,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);

            queries[0].value = 1;
            queries[1].value = 0;
            parentQuery.value = ((vv < v) ? 1 : 0);
            newLocalLogLikelihood +=
                computeSpecificLocalLogLikelihood(adtreePtr,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);

            queries[0].value = 1;
            queries[1].value = 1;
            parentQuery.value = 1;
            newLocalLogLikelihood +=
                computeSpecificLocalLogLikelihood(adtreePtr,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);

            if (newLocalLogLikelihood > bestLocalLogLikelihood) {
                bestLocalIndex = vv;
                bestLocalLogLikelihood = newLocalLogLikelihood;
            }

        } /* foreach other variable */

        PVECTOR_POPBACK(queryVectorPtr);

        if (bestLocalIndex != v) {
            float logLikelihood = numRecord * (baseLogLikelihood +
                                                + bestLocalLogLikelihood
                                                - localBaseLogLikelihoods[v]);
            float score = penalty + logLikelihood;
            learner_task_t* taskPtr = &tasks[v];
            taskPtr->op = OPERATION_INSERT;
            taskPtr->fromId = bestLocalIndex;
            taskPtr->toId = v;
            taskPtr->score = score;
            __transaction_atomic {
              status = TMLIST_INSERT(taskListPtr, (void*)taskPtr);
            }
            assert(status);
        }

    } /* for each variable */

    PVECTOR_FREE(queryVectorPtr);
    PVECTOR_FREE(parentQueryVectorPtr);

#ifdef TEST_LEARNER
    list_iter_t it;
    list_iter_reset(&it, taskListPtr);
    while (list_iter_hasNext(&it)) {
        learner_task_t* taskPtr = (learner_task_t*)list_iter_next(&it, taskListPtr);
        printf("[task] op=%i from=%li to=%li score=%lf\n",
               taskPtr->op, taskPtr->fromId, taskPtr->toId, taskPtr->score);
    }
#endif /* TEST_LEARNER */

}


/* =============================================================================
 * TMpopTask
 * -- Returns NULL is list is empty
 * =============================================================================
 */
__attribute__((transaction_safe))
learner_task_t*
TMpopTask (list_t* taskListPtr)
{
    learner_task_t* taskPtr = NULL;

    list_iter_t it;
    //TMLIST_ITER_RESET(&it, taskListPtr);
    it = &(taskListPtr->head);

    //if (TMLIST_ITER_HASNEXT(&it)) {
    if (it->nextPtr != NULL) {
      //taskPtr = (learner_task_t*)TMLIST_ITER_NEXT(&it, taskListPtr);
      taskPtr = (learner_task_t*)it->nextPtr->dataPtr;

      bool status = TMLIST_REMOVE(taskListPtr, (void*)taskPtr);
      assert(status);
    }

    return taskPtr;
}


/* =============================================================================
 * populateParentQuery
 * -- Modifies contents of parentQueryVectorPtr
 * =============================================================================
 */
void
populateParentQueryVector (net_t* netPtr,
                           long id,
                           query_t* queries,
                           vector_t* parentQueryVectorPtr)
{
    vector_clear(parentQueryVectorPtr);

    list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, id);
    list_iter_t it;
    list_iter_reset(&it, parentIdListPtr);
    while (list_iter_hasNext(&it)) {
        long parentId = (long)list_iter_next(&it);
        bool status = vector_pushBack(parentQueryVectorPtr,
                                        (void*)&queries[parentId]);
        assert(status);
    }
}


/* =============================================================================
 * TMpopulateParentQueryVector
 * -- Modifies contents of parentQueryVectorPtr
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMpopulateParentQueryVector (net_t* netPtr,
                             long id,
                             query_t* queries,
                             vector_t* parentQueryVectorPtr)
{
  vector_clear(parentQueryVectorPtr);

  list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, id); //__attribute__((transaction_safe))
  list_iter_t it;
  //TMLIST_ITER_RESET(&it, parentIdListPtr);
  it = &(parentIdListPtr->head);

  //while (TMLIST_ITER_HASNEXT(&it)) {
  while (it->nextPtr != NULL) {
    //long parentId = (long)TMLIST_ITER_NEXT(&it, parentIdListPtr);
    long parentId = (long)it->nextPtr->dataPtr;
    it = it->nextPtr;

    bool status = PVECTOR_PUSHBACK(parentQueryVectorPtr,
                                     (void*)&queries[parentId]);
    assert(status);
  }
}


/* =============================================================================
 * TMpopulateQueryVectors
 * -- Modifies contents of queryVectorPtr and parentQueryVectorPtr
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMpopulateQueryVectors (net_t* netPtr,
                      long id,
                      query_t* queries,
                      vector_t* queryVectorPtr,
                      vector_t* parentQueryVectorPtr)
{
    TMpopulateParentQueryVector(netPtr, id, queries, parentQueryVectorPtr);

    bool status;
    status = PVECTOR_COPY(queryVectorPtr, parentQueryVectorPtr);
    assert(status);
    status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[id]);
    assert(status);
    vector_sort(queryVectorPtr, &compareQuery);
}


/* =============================================================================
 * computeLocalLogLikelihoodHelper
 * -- Recursive helper routine
 * =============================================================================
 */
__attribute__((transaction_safe))
float
computeLocalLogLikelihoodHelper (long i,
                                 long numParent,
                                 adtree_t* adtreePtr,
                                 query_t* queries,
                                 vector_t* queryVectorPtr,
                                 vector_t* parentQueryVectorPtr)
{
    if (i >= numParent) {
      //[wer] this function contains a log(), which was not __attribute__((transaction_safe))
        return computeSpecificLocalLogLikelihood(adtreePtr,
                                                 queryVectorPtr,
                                                 parentQueryVectorPtr);
    }

    float localLogLikelihood = 0.0;

    query_t* parentQueryPtr = (query_t*)vector_at(parentQueryVectorPtr, i);
    long parentIndex = parentQueryPtr->index;

    queries[parentIndex].value = 0;
    localLogLikelihood += computeLocalLogLikelihoodHelper((i + 1),
                                                          numParent,
                                                          adtreePtr,
                                                          queries,
                                                          queryVectorPtr,
                                                          parentQueryVectorPtr);

    queries[parentIndex].value = 1;
    localLogLikelihood += computeLocalLogLikelihoodHelper((i + 1),
                                                          numParent,
                                                          adtreePtr,
                                                          queries,
                                                          queryVectorPtr,
                                                          parentQueryVectorPtr);

    queries[parentIndex].value = QUERY_VALUE_WILDCARD;

    return localLogLikelihood;
}


/* =============================================================================
 * computeLocalLogLikelihood
 * -- Populate the query vectors before passing as args
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
float
computeLocalLogLikelihood (long id,
                           adtree_t* adtreePtr,
                           query_t* queries,
                           vector_t* queryVectorPtr,
                           vector_t* parentQueryVectorPtr)
{
    long numParent = vector_getSize(parentQueryVectorPtr);
    float localLogLikelihood = 0.0;

    queries[id].value = 0;
    localLogLikelihood += computeLocalLogLikelihoodHelper(0,
                                                          numParent,
                                                          adtreePtr,
                                                          queries,
                                                          queryVectorPtr,
                                                          parentQueryVectorPtr);

    queries[id].value = 1;
    localLogLikelihood += computeLocalLogLikelihoodHelper(0,
                                                          numParent,
                                                          adtreePtr,
                                                          queries,
                                                          queryVectorPtr,
                                                          parentQueryVectorPtr);

    queries[id].value = QUERY_VALUE_WILDCARD;


    return localLogLikelihood;
}


/* =============================================================================
 * TMfindBestInsertTask
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMfindBestInsertTask (learner_task_t * dest,  findBestTaskArg_t* argPtr)
{
    long       toId                     = argPtr->toId;
    learner_t* learnerPtr               = argPtr->learnerPtr;
    query_t*   queries                  = argPtr->queries;
    vector_t*  queryVectorPtr           = argPtr->queryVectorPtr;
    vector_t*  parentQueryVectorPtr     = argPtr->parentQueryVectorPtr;
    long       numTotalParent           = argPtr->numTotalParent;
    float      basePenalty              = argPtr->basePenalty;
    float      baseLogLikelihood        = argPtr->baseLogLikelihood;
    bitmap_t*  invalidBitmapPtr         = argPtr->bitmapPtr;
    queue_t*   workQueuePtr             = argPtr->workQueuePtr;
    vector_t*  baseParentQueryVectorPtr = argPtr->aQueryVectorPtr;
    vector_t*  baseQueryVectorPtr       = argPtr->bQueryVectorPtr;

    bool status;
    adtree_t* adtreePtr               = learnerPtr->adtreePtr;
    net_t*    netPtr                  = learnerPtr->netPtr;
    float*    localBaseLogLikelihoods = learnerPtr->localBaseLogLikelihoods;

    //[wer] this function contained unsafe calls, fixed
    TMpopulateParentQueryVector(netPtr, toId, queries, parentQueryVectorPtr);

    /*
     * Create base query and parentQuery
     */
    //[wer] all __attribute__((transaction_safe))
    status = PVECTOR_COPY(baseParentQueryVectorPtr, parentQueryVectorPtr);
    assert(status);
    status = PVECTOR_COPY(baseQueryVectorPtr, baseParentQueryVectorPtr);
    assert(status);
    status = PVECTOR_PUSHBACK(baseQueryVectorPtr, (void*)&queries[toId]);
    assert(status);

    //[wer] was TM_PURE due to qsort(), now __attribute__((transaction_safe))
    vector_sort(queryVectorPtr, &compareQuery);

    /*
     * Search all possible valid operations for better local log likelihood
     */

    float bestFromId = toId; /* flag for not found */
    float oldLocalLogLikelihood = localBaseLogLikelihoods[toId];
    float bestLocalLogLikelihood = oldLocalLogLikelihood;

    // [wer] __attribute__((transaction_safe)) now
    status = TMNET_FINDDESCENDANTS(netPtr, toId, invalidBitmapPtr, workQueuePtr);

    assert(status);
    long fromId = -1;
    // __attribute__((transaction_safe))
    list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, toId);
    long maxNumEdgeLearned = global_maxNumEdgeLearned;

    if ((maxNumEdgeLearned < 0) ||
        (TMLIST_GETSIZE(parentIdListPtr) <= maxNumEdgeLearned))
    {
        list_iter_t it;
        // TMLIST_ITER_RESET(&it, parentIdListPtr);
        it = &(parentIdListPtr->head);

        //while (TMLIST_ITER_HASNEXT(&it)) {
        while (it->nextPtr != NULL) {
          //long parentId = (long)TMLIST_ITER_NEXT(&it, parentIdListPtr);
          long parentId = (long)it->nextPtr->dataPtr;
          it = it->nextPtr;

          bitmap_set(invalidBitmapPtr, parentId); /* invalid since already have edge */
        }

        while ((fromId = bitmap_findClear(invalidBitmapPtr, (fromId + 1))) >= 0) {
            if (fromId == toId) {
                continue;
            }

            //[wer] __attribute__((transaction_safe))
            status = PVECTOR_COPY(queryVectorPtr, baseQueryVectorPtr);
            assert(status);
            status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[fromId]);
            assert(status);
            //[wer] was TM_PURE due to qsort(), fixed
            vector_sort(queryVectorPtr, &compareQuery);

            status = PVECTOR_COPY(parentQueryVectorPtr, baseParentQueryVectorPtr);
            assert(status);
            status = PVECTOR_PUSHBACK(parentQueryVectorPtr, (void*)&queries[fromId]);
            assert(status);
            vector_sort(parentQueryVectorPtr, &compareQuery);

            //[wer] in computeLocal...(), there's a function log(), which not __attribute__((transaction_safe))
            float newLocalLogLikelihood = computeLocalLogLikelihood(toId,
                                                                    adtreePtr,
                                                                    queries,
                                                                    queryVectorPtr,
                                                                    parentQueryVectorPtr);

            if (newLocalLogLikelihood > bestLocalLogLikelihood) {
                bestLocalLogLikelihood = newLocalLogLikelihood;
                bestFromId = fromId;
            }

        } /* foreach valid parent */

    } /* if have not exceeded max number of edges to learn */

    /*
     * Return best task; Note: if none is better, fromId will equal toId
     */
    learner_task_t bestTask;
    bestTask.op     = OPERATION_INSERT;
    bestTask.fromId = bestFromId;
    bestTask.toId   = toId;
    bestTask.score  = 0.0;

    if (bestFromId != toId) {
        long numRecord = adtreePtr->numRecord;
        long numParent = TMLIST_GETSIZE(parentIdListPtr) + 1;
        float penalty =
            (numTotalParent + numParent * global_insertPenalty) * basePenalty;
        float logLikelihood = numRecord * (baseLogLikelihood +
                                           + bestLocalLogLikelihood
                                           - oldLocalLogLikelihood);
        float bestScore = penalty + logLikelihood;
        bestTask.score  = bestScore;
    }

    *dest = bestTask;
}


#ifdef LEARNER_TRY_REMOVE
/* =============================================================================
 * TMfindBestRemoveTask
 * =============================================================================
 */
__attribute__((transaction_safe))
//learner_task_t
void
TMfindBestRemoveTask (learner_task_t * dest,  findBestTaskArg_t* argPtr)
{
    long       toId                     = argPtr->toId;
    learner_t* learnerPtr               = argPtr->learnerPtr;
    query_t*   queries                  = argPtr->queries;
    vector_t*  queryVectorPtr           = argPtr->queryVectorPtr;
    vector_t*  parentQueryVectorPtr     = argPtr->parentQueryVectorPtr;
    long       numTotalParent           = argPtr->numTotalParent;
    float      basePenalty              = argPtr->basePenalty;
    float      baseLogLikelihood        = argPtr->baseLogLikelihood;
    vector_t*  origParentQueryVectorPtr = argPtr->aQueryVectorPtr;

    bool status;
    adtree_t* adtreePtr = learnerPtr->adtreePtr;
    net_t* netPtr = learnerPtr->netPtr;
    float* localBaseLogLikelihoods = learnerPtr->localBaseLogLikelihoods;

    TMpopulateParentQueryVector(netPtr, toId, queries, origParentQueryVectorPtr);
    long numParent = PVECTOR_GETSIZE(origParentQueryVectorPtr);

    /*
     * Search all possible valid operations for better local log likelihood
     */

    float bestFromId = toId; /* flag for not found */
    float oldLocalLogLikelihood = localBaseLogLikelihoods[toId];
    float bestLocalLogLikelihood = oldLocalLogLikelihood;

    long i;
    for (i = 0; i < numParent; i++) {

        query_t* queryPtr = (query_t*)PVECTOR_AT(origParentQueryVectorPtr, i);
        long fromId = queryPtr->index;

        /*
         * Create parent query (subset of parents since remove an edge)
         */

        PVECTOR_CLEAR(parentQueryVectorPtr);

        long p;
        for (p = 0; p < numParent; p++) {
            if (p != fromId) {
                query_t* queryPtr = (query_t*)PVECTOR_AT(origParentQueryVectorPtr, p);
                status = PVECTOR_PUSHBACK(parentQueryVectorPtr,
                                          (void*)&queries[queryPtr->index]);
                assert(status);
            }
        } /* create new parent query */

        /*
         * Create query
         */

        status = PVECTOR_COPY(queryVectorPtr, parentQueryVectorPtr);
        assert(status);
        status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[toId]);
        assert(status);
        vector_sort(queryVectorPtr, &compareQuery);

        /*
         * See if removing parent is better
         */

        float newLocalLogLikelihood = computeLocalLogLikelihood(toId,
                                                                adtreePtr,
                                                                queries,
                                                                queryVectorPtr,
                                                                parentQueryVectorPtr);

        if (newLocalLogLikelihood > bestLocalLogLikelihood) {
            bestLocalLogLikelihood = newLocalLogLikelihood;
            bestFromId = fromId;
        }

    } /* for each parent */

    /*
     * Return best task; Note: if none is better, fromId will equal toId
     */

    learner_task_t bestTask;
    bestTask.op     = OPERATION_REMOVE;
    bestTask.fromId = bestFromId;
    bestTask.toId   = toId;
    bestTask.score  = 0.0;

    if (bestFromId != toId) {
        long numRecord = adtreePtr->numRecord;
        float penalty = (numTotalParent - 1) * basePenalty;
        float logLikelihood = numRecord * (baseLogLikelihood +
                                            + bestLocalLogLikelihood
                                            - oldLocalLogLikelihood);
        float bestScore = penalty + logLikelihood;
        bestTask.score  = bestScore;
    }

    //[wer210]
    dest->op = bestTask.op;
    dest->fromId = bestTask.fromId;
    dest->toId = bestTask.toId;
    dest->score = bestTask.score;
    //return bestTask;
}
#endif /* LEARNER_TRY_REMOVE */


#ifdef LEARNER_TRY_REVERSE
/* =============================================================================
 * TMfindBestReverseTask
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMfindBestReverseTask (learner_task_t * dest,  findBestTaskArg_t* argPtr)
{
    long       toId                         = argPtr->toId;
    learner_t* learnerPtr                   = argPtr->learnerPtr;
    query_t*   queries                      = argPtr->queries;
    vector_t*  queryVectorPtr               = argPtr->queryVectorPtr;
    vector_t*  parentQueryVectorPtr         = argPtr->parentQueryVectorPtr;
    long       numTotalParent               = argPtr->numTotalParent;
    float      basePenalty                  = argPtr->basePenalty;
    float      baseLogLikelihood            = argPtr->baseLogLikelihood;
    bitmap_t*  visitedBitmapPtr             = argPtr->bitmapPtr;
    queue_t*   workQueuePtr                 = argPtr->workQueuePtr;
    vector_t*  toOrigParentQueryVectorPtr   = argPtr->aQueryVectorPtr;
    vector_t*  fromOrigParentQueryVectorPtr = argPtr->bQueryVectorPtr;

    bool    status;
    adtree_t* adtreePtr               = learnerPtr->adtreePtr;
    net_t*    netPtr                  = learnerPtr->netPtr;
    float*    localBaseLogLikelihoods = learnerPtr->localBaseLogLikelihoods;

    TMpopulateParentQueryVector(netPtr, toId, queries, toOrigParentQueryVectorPtr);
    long numParent = PVECTOR_GETSIZE(toOrigParentQueryVectorPtr);//__attribute__((transaction_safe))

    /*
     * Search all possible valid operations for better local log likelihood
     */

    long bestFromId = toId; /* flag for not found */
    float oldLocalLogLikelihood = (float)localBaseLogLikelihoods[toId];
    float bestLocalLogLikelihood = oldLocalLogLikelihood;
    long fromId = 0;

    long i;
    for (i = 0; i < numParent; i++) {
      query_t* queryPtr = (query_t*)PVECTOR_AT(toOrigParentQueryVectorPtr, i);
      fromId = queryPtr->index;

      bestLocalLogLikelihood = oldLocalLogLikelihood +
        (float)localBaseLogLikelihoods[fromId];

      //__attribute__((transaction_safe))
      TMpopulateParentQueryVector(netPtr,
                                  fromId,
                                  queries,
                                  fromOrigParentQueryVectorPtr);

      /*
       * Create parent query (subset of parents since remove an edge)
       */

      PVECTOR_CLEAR(parentQueryVectorPtr);

      long p;
      for (p = 0; p < numParent; p++) {
        if (p != fromId) {
          query_t* queryPtr = (query_t*)PVECTOR_AT(toOrigParentQueryVectorPtr, p);
          status = PVECTOR_PUSHBACK(parentQueryVectorPtr,
                                    (void*)&queries[queryPtr->index]);
          assert(status);
        }
      } /* create new parent query */

        /*
         * Create query
         */

      status = PVECTOR_COPY(queryVectorPtr, parentQueryVectorPtr);
      assert(status);
      status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[toId]);
      assert(status);

      //[wer]__attribute__((transaction_safe))
      vector_sort(queryVectorPtr, &compareQuery);

      /*
       * Get log likelihood for removing parent from toId
       */

      float newLocalLogLikelihood = computeLocalLogLikelihood(toId,
                                                              adtreePtr,
                                                              queries,
                                                              queryVectorPtr,
                                                              parentQueryVectorPtr);


      /*
       * Get log likelihood for adding parent to fromId
       */

      status = PVECTOR_COPY(parentQueryVectorPtr, fromOrigParentQueryVectorPtr);
      assert(status);
      status = PVECTOR_PUSHBACK(parentQueryVectorPtr, (void*)&queries[toId]);
      assert(status);
      vector_sort(parentQueryVectorPtr, &compareQuery);

      status = PVECTOR_COPY(queryVectorPtr, parentQueryVectorPtr);
      assert(status);
      status = PVECTOR_PUSHBACK(queryVectorPtr, (void*)&queries[fromId]);
      assert(status);
      vector_sort(queryVectorPtr, &compareQuery);

      newLocalLogLikelihood += computeLocalLogLikelihood(fromId,
                                                         adtreePtr,
                                                         queries,
                                                         queryVectorPtr,
                                                         parentQueryVectorPtr);
      /*
       * Record best
       */

      if (newLocalLogLikelihood > bestLocalLogLikelihood) {
        bestLocalLogLikelihood = newLocalLogLikelihood;
        bestFromId = fromId;
      }

    } /* for each parent */

    /*
     * Check validity of best
     */

    if (bestFromId != toId) {
      bool isTaskValid = true;
      TMNET_APPLYOPERATION(netPtr, OPERATION_REMOVE, bestFromId, toId);
      if (TMNET_ISPATH(netPtr, bestFromId, toId, visitedBitmapPtr,
                       workQueuePtr)) {
        isTaskValid = false;
      }

      TMNET_APPLYOPERATION(netPtr, OPERATION_INSERT, bestFromId, toId);
      if (!isTaskValid)
        bestFromId = toId;
    }

    /*
     * Return best task; Note: if none is better, fromId will equal toId
     */

    learner_task_t bestTask;
    bestTask.op     = OPERATION_REVERSE;
    bestTask.fromId = bestFromId;
    bestTask.toId   = toId;
    bestTask.score  = 0.0;

    if (bestFromId != toId) {
      float fromLocalLogLikelihood = localBaseLogLikelihoods[bestFromId];
      long numRecord = adtreePtr->numRecord;
      float penalty = numTotalParent * basePenalty;
      float logLikelihood = numRecord * (baseLogLikelihood +
                                         + bestLocalLogLikelihood
                                         - oldLocalLogLikelihood
                                         - fromLocalLogLikelihood);
      float bestScore = penalty + logLikelihood;
      bestTask.score  = bestScore;
    }

    *dest = bestTask;
}
#endif /* LEARNER_TRY_REVERSE */

/* =============================================================================
 * learnStructure
 *
 * Note it is okay if the score is not exact, as we are relaxing the greedy
 * search. This means we do not need to communicate baseLogLikelihood across
 * threads.
 * =============================================================================
 */
void
learnStructure (void* argPtr)
{
    learner_t* learnerPtr = (learner_t*)argPtr;
    net_t* netPtr = learnerPtr->netPtr;
    adtree_t* adtreePtr = learnerPtr->adtreePtr;
    long numRecord = adtreePtr->numRecord;
    float* localBaseLogLikelihoods = learnerPtr->localBaseLogLikelihoods;
    list_t* taskListPtr = learnerPtr->taskListPtr;

    float operationQualityFactor = global_operationQualityFactor;

    bitmap_t* visitedBitmapPtr = TMBITMAP_ALLOC(learnerPtr->adtreePtr->numVar);
    assert(visitedBitmapPtr);
    queue_t* workQueuePtr = TMQUEUE_ALLOC(-1);
    assert(workQueuePtr);

    long numVar = adtreePtr->numVar;
    query_t* queries = (query_t*)malloc(numVar * sizeof(query_t));
    assert(queries);
    long v;
    for (v = 0; v < numVar; v++) {
        queries[v].index = v;
        queries[v].value = QUERY_VALUE_WILDCARD;
    }

    float basePenalty = (float)(-0.5 * log((double)numRecord));

    vector_t* queryVectorPtr = PVECTOR_ALLOC(1);
    assert(queryVectorPtr);
    vector_t* parentQueryVectorPtr = PVECTOR_ALLOC(1);
    assert(parentQueryVectorPtr);
    vector_t* aQueryVectorPtr = PVECTOR_ALLOC(1);
    assert(aQueryVectorPtr);
    vector_t* bQueryVectorPtr = PVECTOR_ALLOC(1);
    assert(bQueryVectorPtr);

    findBestTaskArg_t arg;
    arg.learnerPtr           = learnerPtr;
    arg.queries              = queries;
    arg.queryVectorPtr       = queryVectorPtr;
    arg.parentQueryVectorPtr = parentQueryVectorPtr;
    arg.bitmapPtr            = visitedBitmapPtr;
    arg.workQueuePtr         = workQueuePtr;
    arg.aQueryVectorPtr      = aQueryVectorPtr;
    arg.bQueryVectorPtr      = bQueryVectorPtr;


    while (1) {

        learner_task_t* taskPtr;
        __transaction_atomic {
          taskPtr = TMpopTask(  taskListPtr);
        }

        if (taskPtr == NULL) {
            break;
        }

        operation_t op = taskPtr->op;
        long fromId = taskPtr->fromId;
        long toId = taskPtr->toId;

        bool isTaskValid;

        __transaction_atomic {
        /*
         * Check if task is still valid
         */
        isTaskValid = true;

        switch (op) {
         case OPERATION_INSERT: {
                if (TMNET_HASEDGE(netPtr, fromId, toId)
                    || TMNET_ISPATH(netPtr,
                                    toId,
                                    fromId,
                                    visitedBitmapPtr,
                                    workQueuePtr)
                   )
                {
                  isTaskValid = false;
                }
                break;
            }

         case OPERATION_REMOVE: {
                /* Can never create cycle, so always valid */
                break;
            }
         case OPERATION_REVERSE: {
                /* Temporarily remove edge for check */
                TMNET_APPLYOPERATION(netPtr, OPERATION_REMOVE, fromId, toId);
                if (TMNET_ISPATH(netPtr,
                                 fromId,
                                 toId,
                                 visitedBitmapPtr,
                                 workQueuePtr))
                {
                    isTaskValid = false;
                }
                TMNET_APPLYOPERATION(netPtr, OPERATION_INSERT, fromId, toId);
                break;
            }

         default:
          assert(0);
        }

#ifdef TEST_LEARNER
        printf("[task] op=%i from=%li to=%li score=%lf valid=%s\n",
               taskPtr->op, taskPtr->fromId, taskPtr->toId, taskPtr->score,
               (isTaskValid ? "yes" : "no"));
        fflush(stdout);
#endif

        /*
         * Perform task: update graph and probabilities
         */

        if (isTaskValid) {
          TMNET_APPLYOPERATION(netPtr, op, fromId, toId);
        }

        }
        float deltaLogLikelihood = 0.0;
        if (isTaskValid) {
            switch (op) {
                float newBaseLogLikelihood;
                case OPERATION_INSERT: {
                  __transaction_atomic {
                    TMpopulateQueryVectors(netPtr,
                                           toId,
                                           queries,
                                           queryVectorPtr,
                                           parentQueryVectorPtr);
                    newBaseLogLikelihood =
                      computeLocalLogLikelihood(toId,
                                                adtreePtr,
                                                queries,
                                                queryVectorPtr,
                                                parentQueryVectorPtr);
                    float toLocalBaseLogLikelihood = localBaseLogLikelihoods[toId];
                    deltaLogLikelihood +=
                        toLocalBaseLogLikelihood - newBaseLogLikelihood;
                    localBaseLogLikelihoods[toId] = newBaseLogLikelihood;
                  }

                  __transaction_atomic {
                    long numTotalParent = learnerPtr->numTotalParent;
                    learnerPtr->numTotalParent = (numTotalParent + 1);
                  }
                  break;
                }
#ifdef LEARNER_TRY_REMOVE
                case OPERATION_REMOVE: {
                  __transaction_atomic {
                    TMpopulateQueryVectors(netPtr,
                                           fromId,
                                           queries,
                                           queryVectorPtr,
                                           parentQueryVectorPtr);
                    newBaseLogLikelihood =
                        computeLocalLogLikelihood(fromId,
                                                  adtreePtr,
                                                  queries,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);
                    float fromLocalBaseLogLikelihood = localBaseLogLikelihoods[fromId];
                    deltaLogLikelihood +=
                        fromLocalBaseLogLikelihood - newBaseLogLikelihood;
                    localBaseLogLikelihoods[fromId] = newBaseLogLikelihood;
                  }

                  __transaction_atomic {
                    long numTotalParent = learnerPtr->numTotalParent;
                    learnerPtr->numTotalParent = (numTotalParent - 1);
                  }
                  break;
                }
#endif /* LEARNER_TRY_REMOVE */
#ifdef LEARNER_TRY_REVERSE
                case OPERATION_REVERSE: {
                  __transaction_atomic {
                    TMpopulateQueryVectors(netPtr,
                                         fromId,
                                         queries,
                                         queryVectorPtr,
                                         parentQueryVectorPtr);
                    newBaseLogLikelihood =
                      computeLocalLogLikelihood(fromId,
                                                adtreePtr,
                                                queries,
                                                queryVectorPtr,
                                                parentQueryVectorPtr);
                    float fromLocalBaseLogLikelihood = localBaseLogLikelihoods[fromId];
                    deltaLogLikelihood +=
                      fromLocalBaseLogLikelihood - newBaseLogLikelihood;
                    localBaseLogLikelihoods[fromId] =  newBaseLogLikelihood;
                  }

                  __transaction_atomic {
                    TMpopulateQueryVectors(netPtr,
                                           toId,
                                           queries,
                                           queryVectorPtr,
                                           parentQueryVectorPtr);
                    newBaseLogLikelihood =
                      computeLocalLogLikelihood(toId,
                                                adtreePtr,
                                                queries,
                                                  queryVectorPtr,
                                                  parentQueryVectorPtr);
                    float toLocalBaseLogLikelihood = localBaseLogLikelihoods[toId];
                    deltaLogLikelihood +=
                        toLocalBaseLogLikelihood - newBaseLogLikelihood;
                    localBaseLogLikelihoods[toId] = newBaseLogLikelihood;
                  }
                  break;
                }
#endif /* LEARNER_TRY_REVERSE */
             default:
              assert(0);
            } /* switch op */

        } /* if isTaskValid */

        /*
         * Update/read globals
         */

        float baseLogLikelihood;
        long numTotalParent;

        __transaction_atomic {
          float oldBaseLogLikelihood = learnerPtr->baseLogLikelihood;
          float newBaseLogLikelihood = oldBaseLogLikelihood + deltaLogLikelihood;
          learnerPtr->baseLogLikelihood = newBaseLogLikelihood;
          baseLogLikelihood = newBaseLogLikelihood;
          numTotalParent = learnerPtr->numTotalParent;
        }

        /*
         * Find next task
         */

        float baseScore = ((float)numTotalParent * basePenalty)
                           + (numRecord * baseLogLikelihood);

        learner_task_t bestTask;
        bestTask.op     = NUM_OPERATION;
        bestTask.toId   = -1;
        bestTask.fromId = -1;
        bestTask.score  = baseScore;

        learner_task_t newTask;

        arg.toId              = toId;
        arg.numTotalParent    = numTotalParent;
        arg.basePenalty       = basePenalty;
        arg.baseLogLikelihood = baseLogLikelihood;

        __transaction_atomic {
          TMfindBestInsertTask(&newTask, &arg);
        }

        if ((newTask.fromId != newTask.toId) &&
            (newTask.score > (bestTask.score / operationQualityFactor)))
        {
          bestTask = newTask;
        }

#ifdef LEARNER_TRY_REMOVE
        __transaction_atomic {
          TMfindBestRemoveTask(&newTask, &arg);
        }

        if ((newTask.fromId != newTask.toId) &&
            (newTask.score > (bestTask.score / operationQualityFactor)))
        {
          bestTask = newTask;
        }

#endif /* LEARNER_TRY_REMOVE */

#ifdef LEARNER_TRY_REVERSE
        //[wer210] used to have problems, fixed(log, qsort)
        __transaction_atomic {
          TMfindBestReverseTask(&newTask, &arg);
        }

        if ((newTask.fromId != newTask.toId) &&
            (newTask.score > (bestTask.score / operationQualityFactor)))
        {
          bestTask = newTask;
        }

#endif /* LEARNER_TRY_REVERSE */

        if (bestTask.toId != -1) {
            learner_task_t* tasks = learnerPtr->tasks;
            tasks[toId] = bestTask;
            __transaction_atomic {
              TMLIST_INSERT(taskListPtr, (void*)&tasks[toId]);
            }

#ifdef TEST_LEARNER
            printf("[new]  op=%i from=%li to=%li score=%lf\n",
                   bestTask.op, bestTask.fromId, bestTask.toId, bestTask.score);
            fflush(stdout);
#endif
        }

    } /* while (tasks) */

    TMBITMAP_FREE(visitedBitmapPtr);
    TMQUEUE_FREE(workQueuePtr);
    PVECTOR_FREE(bQueryVectorPtr);
    PVECTOR_FREE(aQueryVectorPtr);
    PVECTOR_FREE(queryVectorPtr);
    PVECTOR_FREE(parentQueryVectorPtr);
    free(queries);
}


/* =============================================================================
 * learner_run
 * -- Call adtree_make before this
 * =============================================================================
 */
void
learner_run (learner_t* learnerPtr)
{
#ifdef OTM
#pragma omp parallel
    {
        createTaskList((void*)learnerPtr);
    }
#pragma omp parallel
    {
        learnStructure((void*)learnerPtr);
    }
#else
    thread_start(&createTaskList, (void*)learnerPtr);
    thread_start(&learnStructure, (void*)learnerPtr);
#endif
}


/* =============================================================================
 * learner_score
 * -- Score entire network
 * =============================================================================
 */
float
learner_score (learner_t* learnerPtr)
{
    adtree_t* adtreePtr = learnerPtr->adtreePtr;
    net_t* netPtr = learnerPtr->netPtr;

    vector_t* queryVectorPtr = vector_alloc(1);
    assert(queryVectorPtr);
    vector_t* parentQueryVectorPtr = vector_alloc(1);
    assert(parentQueryVectorPtr);

    long numVar = adtreePtr->numVar;
    query_t* queries = (query_t*)malloc(numVar * sizeof(query_t));
    assert(queries);
    long v;
    for (v = 0; v < numVar; v++) {
        queries[v].index = v;
        queries[v].value = QUERY_VALUE_WILDCARD;
    }

    long numTotalParent = 0;
    float logLikelihood = 0.0;

    for (v = 0; v < numVar; v++) {

        list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, v);
        numTotalParent += list_getSize(parentIdListPtr);


        TMpopulateQueryVectors(netPtr,
                             v,
                             queries,
                             queryVectorPtr,
                             parentQueryVectorPtr);
        float localLogLikelihood = computeLocalLogLikelihood(v,
                                                             adtreePtr,
                                                             queries,
                                                             queryVectorPtr,
                                                             parentQueryVectorPtr);
        logLikelihood += localLogLikelihood;
    }

    vector_free(queryVectorPtr);
    vector_free(parentQueryVectorPtr);
    free(queries);

    long numRecord = adtreePtr->numRecord;
    float penalty = (float)(-0.5 * (double)numTotalParent * log((double)numRecord));
    float score = penalty + numRecord * logLikelihood;

    return score;
}


/* #############################################################################
 * TEST_LEARNER
 * #############################################################################
 */
#ifdef TEST_LEARNER

#include <stdio.h>


void
testPartition (long min, long max, long n)
{
    long start;
    long stop;

    printf("min=%li max=%li, n=%li\n", min, max, n);

    long i;
    for (i = 0; i < n; i++) {
        createPartition(min, max, i, n, &start, &stop);
        printf("%li: %li -> %li\n", i, start, stop);
    }
    puts("");
}


int
main (int argc, char* argv[])
{
    thread_startup(1);

    puts("Starting...");

    testPartition(0, 4, 8);
    testPartition(0, 15, 8);
    testPartition(3, 103, 7);

    long numVar = 56;
    long numRecord = 256;

    random_t* randomPtr = random_alloc();
    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);
    data_generate(dataPtr, 0, 10, 10);

    adtree_t* adtreePtr = adtree_alloc();
    assert(adtreePtr);
    adtree_make(adtreePtr, dataPtr);


    learner_t* learnerPtr = learner_alloc(dataPtr, adtreePtr);
    assert(learnerPtr);

    data_free(dataPtr);

    learner_run(learnerPtr);

    assert(!net_isCycle(learnerPtr->netPtr));

    float score = learner_score(learnerPtr);
    printf("score = %lf\n", score);

    learner_free(learnerPtr);

    puts("Done.");

    adtree_free(adtreePtr);
    random_free(randomPtr);

    thread_shutdown();

    return 0;
}

#endif /* TEST_LEARNER */


/* =============================================================================
 *
 * End of learner.h
 *
 * =============================================================================
 */
