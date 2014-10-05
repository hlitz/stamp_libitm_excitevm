/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "data.h"
#include "net.h"
#include "sort.h"

enum data_config {
    DATA_PRECISION = 100,
    DATA_INIT      = 2 /* not 0 or 1 */
};


/* =============================================================================
 * data_alloc
 * =============================================================================
 */
data_t*
data_alloc (long numVar, long numRecord, std::mt19937* randomPtr)
{
    data_t* dataPtr;

    dataPtr = (data_t*)malloc(sizeof(data_t));
    if (dataPtr) {
        long numDatum = numVar * numRecord;
        dataPtr->records = (char*)malloc(numDatum * sizeof(char));
        if (dataPtr->records == NULL) {
            free(dataPtr);
            return NULL;
        }
        memset(dataPtr->records, DATA_INIT, (numDatum * sizeof(char)));
        dataPtr->numVar = numVar;
        dataPtr->numRecord = numRecord;
        dataPtr->randomPtr = randomPtr;
    }

    return dataPtr;
}


/* =============================================================================
 * data_free
 * =============================================================================
 */
void
data_free (data_t* dataPtr)
{
    free(dataPtr->records);
    free(dataPtr);
}


/* =============================================================================
 * data_generate
 * -- Binary variables of random PDFs
 * -- If seed is <0, do not reseed
 * -- Returns random network
 * =============================================================================
 */
net_t*
data_generate (data_t* dataPtr, long seed, long maxNumParent, long percentParent)
{
    std::mt19937* randomPtr = dataPtr->randomPtr;
    if (seed >= 0) {
        randomPtr->seed(seed);
    }

    /*
     * Generate random Bayesian network
     */

    long numVar = dataPtr->numVar;
    net_t* netPtr = new net_t(numVar);
    assert(netPtr);
    net_generateRandomEdges(netPtr, maxNumParent, percentParent, randomPtr);

    /*
     * Create a threshold for each of the possible permutation of variable
     * value instances
     */

    long** thresholdsTable = (long**)malloc(numVar * sizeof(long*));
    assert(thresholdsTable);
    long v;
    for (v = 0; v < numVar; v++) {
        std::set<long>* parentIdListPtr = net_getParentIdListPtr(netPtr, v);
        long numThreshold = 1 << parentIdListPtr->size();
        long* thresholds = (long*)malloc(numThreshold * sizeof(long));
        assert(thresholds);
        long t;
        for (t = 0; t < numThreshold; t++) {
            long threshold = randomPtr->operator()() % (DATA_PRECISION + 1);
            thresholds[t] = threshold;
        }
        thresholdsTable[v] = thresholds;
    }

    /*
     * Create variable dependency ordering for record generation
     */

    long* order = (long*)malloc(numVar * sizeof(long));
    assert(order);
    long numOrder = 0;

    queue_t* workQueuePtr = queue_alloc(-1);
    assert(workQueuePtr);

    std::vector<long>* dependencyVectorPtr = new std::vector<long>();
    assert(dependencyVectorPtr);

    bitmap_t* orderedBitmapPtr = (bitmap_t*)bitmap_alloc(numVar);
    assert(orderedBitmapPtr);
    bitmap_clearAll(orderedBitmapPtr);

    bitmap_t* doneBitmapPtr = (bitmap_t*)bitmap_alloc(numVar);
    assert(doneBitmapPtr);
    bitmap_clearAll(doneBitmapPtr);
    v = -1;
    while ((v = bitmap_findClear(doneBitmapPtr, (v + 1))) >= 0) {
        std::set<long>* childIdListPtr = net_getChildIdListPtr(netPtr, v);
        long numChild = childIdListPtr->size();
        if (numChild == 0) {

            bool status;

            /*
             * Use breadth-first search to find net connected to this leaf
             */

            queue_clear(workQueuePtr);
            status = queue_push(workQueuePtr, (void*)v);
            assert(status);
            while (!queue_isEmpty(workQueuePtr)) {
                long id = (long)queue_pop(workQueuePtr);
                status = bitmap_set(doneBitmapPtr, id);
                assert(status);
                dependencyVectorPtr->push_back(id);
                std::set<long>* parentIdListPtr = net_getParentIdListPtr(netPtr, id);
                for (auto it : *parentIdListPtr) {
                    long parentId = it;
                    status = queue_push(workQueuePtr, (void*)parentId);
                    assert(status);
                }
            }

            /*
             * Create ordering
             */
            long n = dependencyVectorPtr->size();
            for (long i = 0; i < n; i++) {
                long id = dependencyVectorPtr->back();
                dependencyVectorPtr->pop_back();
                if (!bitmap_isSet(orderedBitmapPtr, id)) {
                    bitmap_set(orderedBitmapPtr, id);
                    order[numOrder++] = id;
                }
            }

        }
    }
    assert(numOrder == numVar);

    /*
     * Create records
     */

    char* record = dataPtr->records;
    long r;
    long numRecord = dataPtr->numRecord;
    for (r = 0; r < numRecord; r++) {
        long o;
        for (o = 0; o < numOrder; o++) {
            long v = order[o];
            std::set<long>* parentIdListPtr = net_getParentIdListPtr(netPtr, v);
            long index = 0;
            for (auto it : *parentIdListPtr) {
                long parentId = it;
                long value = record[parentId];
                assert(value != DATA_INIT);
                index = (index << 1) + value;
            }
            long rnd = randomPtr->operator()() % DATA_PRECISION;
            long threshold = thresholdsTable[v][index];
            record[v] = ((rnd < threshold) ? 1 : 0);
        }
        record += numVar;
        assert(record <= (dataPtr->records + numRecord * numVar));
    }

    /*
     * Clean up
     */

    bitmap_free(doneBitmapPtr);
    bitmap_free(orderedBitmapPtr);
    delete dependencyVectorPtr;
    queue_free(workQueuePtr);
    free(order);
    for (v = 0; v < numVar; v++) {
        free(thresholdsTable[v]);
    }
    free(thresholdsTable);

    return netPtr;
}


/* =============================================================================
 * data_getRecord
 * -- Returns NULL if invalid index
 * =============================================================================
 */
char*
data_getRecord (data_t* dataPtr, long index)
{
    if (index < 0 || index >= (dataPtr->numRecord)) {
        return NULL;
    }

    return &dataPtr->records[index * dataPtr->numVar];
}


/* =============================================================================
 * data_copy
 * -- Returns false on failure
 * =============================================================================
 */
bool
data_copy (data_t* dstPtr, data_t* srcPtr)
{
    long numDstDatum = dstPtr->numVar * dstPtr->numRecord;
    long numSrcDatum = srcPtr->numVar * srcPtr->numRecord;
    if (numDstDatum != numSrcDatum) {
        free(dstPtr->records);
        dstPtr->records = (char*)calloc(numSrcDatum, sizeof(char));
        if (dstPtr->records == NULL) {
            return false;
        }
    }

    dstPtr->numVar    = srcPtr->numVar;
    dstPtr->numRecord = srcPtr->numRecord;
    memcpy(dstPtr->records, srcPtr->records, (numSrcDatum * sizeof(char)));

    return true;
}


/* =============================================================================
 * compareRecord
 * =============================================================================
 */
static int
compareRecord (const void* p1, const void* p2, long n, long offset)
{
    long i = n - offset;
    const char* s1 = (const char*)p1 + offset;
    const char* s2 = (const char*)p2 + offset;

    while (i-- > 0) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) {
            return (u1 - u2);
        }
    }

    return 0;
}


/* =============================================================================
 * data_sort
 * -- In place
 * =============================================================================
 */
void
data_sort (data_t* dataPtr,
           long start,
           long num,
           long offset)
{
    assert(start >= 0 && start <= dataPtr->numRecord);
    assert(num >= 0 && num <= dataPtr->numRecord);
    assert(start + num >= 0 && start + num <= dataPtr->numRecord);

    long numVar = dataPtr->numVar;

    sort((dataPtr->records + (start * numVar)),
          num,
          numVar,
          &compareRecord,
          numVar,
          offset);
}


/* =============================================================================
 * data_findSplit
 * -- Call data_sort first with proper start, num, offset
 * -- Returns number of zeros in offset column
 * =============================================================================
 */
long
data_findSplit (data_t* dataPtr, long start, long num, long offset)
{
    long low = start;
    long high = start + num - 1;

    long numVar = dataPtr->numVar;
    char* records = dataPtr->records;

    while (low <= high) {
        long mid = (low + high) / 2;
        if (records[numVar * mid + offset] == 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return (low - start);
}


/* #############################################################################
 * TEST_DATA
 * #############################################################################
 */
#ifdef TEST_DATA

#include <stdio.h>
#include <string.h>


static void
printRecords (data_t* dataPtr)
{
    long numVar = dataPtr->numVar;
    long numRecord = dataPtr->numRecord;

    long r;
    for (r = 0; r < numRecord; r++) {
        printf("%4li: ", r);
        char* record = data_getRecord(dataPtr, r);
        assert(record);
        long v;
        for (v = 0; v < numVar; v++) {
            printf("%li", (long)record[v]);
        }
        puts("");
    }
    puts("");
}


static void
testAll (long numVar, long numRecord, long numMaxParent, long percentParent)
{
    random_t* randomPtr = random_alloc();

    puts("Starting...");

    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);

    puts("Init:");
    net_t* netPtr = data_generate(dataPtr, 0, numMaxParent, percentParent);
    net_free(netPtr);
    printRecords(dataPtr);

    puts("Sort first half from 0:");
    data_sort(dataPtr, 0, numRecord/2, 0);
    printRecords(dataPtr);

    puts("Sort second half from 0:");
    data_sort(dataPtr, numRecord/2, numRecord-numRecord/2, 0);
    printRecords(dataPtr);

    puts("Sort all from mid:");
    data_sort(dataPtr, 0, numRecord, numVar/2);
    printRecords(dataPtr);

    long split = data_findSplit(dataPtr, 0, numRecord, numVar/2);
    printf("Split = %li\n", split);

    long v;
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
    }

    memset(dataPtr->records, 0, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == numRecord);
    }

    memset(dataPtr->records, 1, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == 0);
    }

    data_free(dataPtr);
}


static void
testBasic (long numVar, long numRecord, long numMaxParent, long percentParent)
{
    random_t* randomPtr = random_alloc();

    puts("Starting...");

    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);

    puts("Init:");
    data_generate(dataPtr, 0, numMaxParent, percentParent);

    long v;
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
    }

    memset(dataPtr->records, 0, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == numRecord);
    }

    memset(dataPtr->records, 1, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == 0);
    }

    data_free(dataPtr);
}


int
main ()
{
    puts("Test 1:");
    testAll(10, 20, 10, 10);

    puts("Test 2:");
    testBasic(20, 80, 10, 20);

    puts("Done");

    return 0;
}


#endif /* TEST_DATA */
