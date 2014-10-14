/* =============================================================================
 *
 * queue.c
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
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
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include <queue>
#include "tm_transition.h"

struct queue_t {
    std::queue<void*>* eltqueue;
};

enum config {
    QUEUE_GROWTH_FACTOR = 2
};


/* =============================================================================
 * TMqueue_alloc
 * =============================================================================
 */
__attribute__((transaction_safe))
queue_t*
queue_alloc (  long initCapacity)
{
    queue_t* queuePtr = (queue_t*)malloc(sizeof(queue_t));
    if (queuePtr) {
        queuePtr->eltqueue = new std::queue<void*>();
    }

    return queuePtr;
}


/* =============================================================================
 * queue_free
 * =============================================================================
 */
__attribute__((transaction_safe))
void
queue_free (queue_t* queuePtr)
{
    delete queuePtr->eltqueue;
    free(queuePtr);
}


/* =============================================================================
 * queue_isEmpty
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
queue_isEmpty (queue_t* queuePtr)
{
    return queuePtr->eltqueue->empty();
}

/* =============================================================================
 * queue_clear
 * =============================================================================
 */
__attribute__((transaction_safe))
void
queue_clear (queue_t* queuePtr)
{
    while (!queuePtr->eltqueue->empty()) {
        queuePtr->eltqueue->pop();
    }
}


/* =============================================================================
 * queue_shuffle
 * =============================================================================
 */
__attribute__((transaction_safe)) // [wer] queue_shuffle has to be TM_PURE, use random()
void
queue_shuffle (queue_t* queuePtr, std::mt19937* randomPtr)
{
    // get the size of the queue
    long numElement = queuePtr->eltqueue->size();

    // move queue elements into a vector
    std::vector<void*> ev;
    while (!queuePtr->eltqueue->empty()) {
        ev.push_back(queuePtr->eltqueue->front());
        queuePtr->eltqueue->pop();
    }
    assert(queuePtr->eltqueue->empty());

    // shuffle the vector
    for (long i = 0; i < numElement; i++) {
        long r1 = randomPtr->operator()() % numElement;
        long r2 = randomPtr->operator()() % numElement;
        void* tmp = ev[r1];
        ev[r1] = ev[r2];
        ev[r2] = tmp;
    }

    // move data back into the queue
    for (auto i : ev) {
        queuePtr->eltqueue->push(i);
    }
}


/* =============================================================================
 * queue_push
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
queue_push (queue_t* queuePtr, void* dataPtr)
{
    queuePtr->eltqueue->push(dataPtr);
    return true;
}


/* =============================================================================
 * queue_pop
 * =============================================================================
 */
__attribute__((transaction_safe))
void*
queue_pop (queue_t* queuePtr)
{
    if (queuePtr->eltqueue->empty())
        return NULL;
    void* o = queuePtr->eltqueue->front();
    queuePtr->eltqueue->pop();
    return o;
}


/* =============================================================================
 * TEST_QUEUE
 * =============================================================================
 */
#ifdef TEST_QUEUE

#include <assert.h>
#include <stdio.h>


static void
printQueue (queue_t* queuePtr)
{
    long   push     = queuePtr->push;
    long   pop      = queuePtr->pop;
    long   capacity = queuePtr->capacity;
    void** elements = queuePtr->elements;

    printf("[");

    long i;
    for (i = ((pop + 1) % capacity); i != push; i = ((i + 1) % capacity)) {
        printf("%li ", *(long*)elements[i]);
    }
    puts("]");
}


static void
insertData (queue_t* queuePtr, long* dataPtr)
{
    printf("Inserting %li: ", *dataPtr);
    assert(queue_push(queuePtr, dataPtr));
    printQueue(queuePtr);
}


int
main ()
{
    queue_t* queuePtr;
    random_t* randomPtr;
    long data[] = {3, 1, 4, 1, 5};
    long numData = sizeof(data) / sizeof(data[0]);
    long i;

    randomPtr = random_alloc();
    assert(randomPtr);
    random_seed(randomPtr, 0);

    puts("Starting tests...");

    queuePtr = queue_alloc(-1);

    assert(queue_isEmpty(queuePtr));
    for (i = 0; i < numData; i++) {
        insertData(queuePtr, &data[i]);
    }
    assert(!queue_isEmpty(queuePtr));

    for (i = 0; i < numData; i++) {
        long* dataPtr = (long*)queue_pop(queuePtr);
        printf("Removing %li: ", *dataPtr);
        printQueue(queuePtr);
    }
    assert(!queue_pop(queuePtr));
    assert(queue_isEmpty(queuePtr));

    puts("All tests passed.");

    for (i = 0; i < numData; i++) {
        insertData(queuePtr, &data[i]);
    }
    for (i = 0; i < numData; i++) {
        printf("Shuffle %li: ", i);
        queue_shuffle(queuePtr, randomPtr);
        printQueue(queuePtr);
    }
    assert(!queue_isEmpty(queuePtr));

    queue_free(queuePtr);

    return 0;
}


#endif /* TEST_QUEUE */


/* =============================================================================
 *
 * End of queue.c
 *
 * =============================================================================
 */

