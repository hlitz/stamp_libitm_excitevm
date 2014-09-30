/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include "region.h"
#include "element.h"
#include "mesh.h"
#include "tm_transition.h"

/* =============================================================================
 * DECLARATION OF TM_SAFE FUNCTIONS
 * =============================================================================
 */

__attribute__((transaction_safe))
void
TMaddToBadVector(vector_t* badVectorPtr, element_t* badElementPtr);

__attribute__((transaction_safe))
long
TMretriangulate (element_t* elementPtr,
                 region_t* regionPtr,
                 mesh_t* meshPtr,
                 MAP_T* edgeMapPtr);

__attribute__((transaction_safe))
element_t*
TMgrowRegion (element_t* centerElementPtr,
              region_t* regionPtr,
              MAP_T* edgeMapPtr,
              bool* success);

region_t::region_t()
{
    expandQueuePtr = TMQUEUE_ALLOC(-1);
    assert(expandQueuePtr);

    //[wer210] note the following compare functions should be TM_SAFE...
    beforeListPtr = TMLIST_ALLOC(&element_listCompare);
    assert(beforeListPtr);

    borderListPtr = TMLIST_ALLOC(&element_listCompareEdge);
    assert(borderListPtr);

    badVectorPtr = PVECTOR_ALLOC(1);
    assert(badVectorPtr);
}

region_t::~region_t()
{
    PVECTOR_FREE(badVectorPtr);
    list_free(borderListPtr);
    list_free(beforeListPtr);
    TMQUEUE_FREE(expandQueuePtr);
}

__attribute__((transaction_safe))
void
TMaddToBadVector (  vector_t* badVectorPtr, element_t* badElementPtr)
{
    bool status = PVECTOR_PUSHBACK(badVectorPtr, (void*)badElementPtr);
    assert(status);
    TMelement_setIsReferenced(badElementPtr, true);
}


/* =============================================================================
 * TMretriangulate
 * -- Returns net amount of elements added to mesh
 * =============================================================================
 */

__attribute__((transaction_safe))
long
TMretriangulate (element_t* elementPtr,
                 region_t* regionPtr,
                 mesh_t* meshPtr,
                 MAP_T* edgeMapPtr)
{
    vector_t* badVectorPtr = regionPtr->badVectorPtr; /* private */
    list_t* beforeListPtr = regionPtr->beforeListPtr; /* private */
    list_t* borderListPtr = regionPtr->borderListPtr; /* private */
    list_iter_t it;
    long numDelta = 0L;
    assert(edgeMapPtr);

    //[wer210] don't return a struct
    //    coordinate_t centerCoordinate = element_getNewPoint(elementPtr);
    coordinate_t centerCoordinate;
    element_getNewPoint(elementPtr, &centerCoordinate);
    /*
     * Remove the old triangles
     */

    it = &(beforeListPtr->head);

    while (it ->nextPtr != NULL) {
      element_t* beforeElementPtr = (element_t*)it->nextPtr->dataPtr;
      it = it->nextPtr;

      meshPtr->remove(beforeElementPtr);
    }


    numDelta -= TMLIST_GETSIZE(beforeListPtr);

    /*
     * If segment is encroached, split it in half
     */

    if (elementPtr->getNumEdge() == 1) {

        coordinate_t coordinates[2];

        edge_t* edgePtr = elementPtr->getEdge(0);
        coordinates[0] = centerCoordinate;

        coordinates[1] = *(coordinate_t*)(edgePtr->firstPtr);
        element_t* aElementPtr = new element_t(coordinates, 2);
        assert(aElementPtr);
        meshPtr->insert(aElementPtr, edgeMapPtr);

        coordinates[1] = *(coordinate_t*)(edgePtr->secondPtr);
        element_t* bElementPtr = new element_t(coordinates, 2);
        assert(bElementPtr);
        meshPtr->insert(bElementPtr, edgeMapPtr);

        bool status;
        status = meshPtr->removeBoundary(elementPtr->getEdge(0));
        assert(status);
        status = meshPtr->insertBoundary(aElementPtr->getEdge(0));
        assert(status);
        status = meshPtr->insertBoundary(bElementPtr->getEdge(0));
        assert(status);

        numDelta += 2;
    }

    /*
     * Insert the new triangles. These are contructed using the new
     * point and the two points from the border segment.
     */
    it = &(borderListPtr->head);
    //list_iter_reset(&it, borderListPtr);

    while (it->nextPtr != NULL) {
   //while (list_iter_hasNext(&it, borderListPtr)) {
      element_t* afterElementPtr;
      coordinate_t coordinates[3];

      //edge_t* borderEdgePtr = (edge_t*)list_iter_next(&it, borderListPtr);
      edge_t* borderEdgePtr = (edge_t*) it->nextPtr->dataPtr;
      it = it->nextPtr;

      assert(borderEdgePtr);
      coordinates[0] = centerCoordinate;
      coordinates[1] = *(coordinate_t*)(borderEdgePtr->firstPtr);
      coordinates[2] = *(coordinate_t*)(borderEdgePtr->secondPtr);
      afterElementPtr = new element_t(coordinates, 3);
      assert(afterElementPtr);
      meshPtr->insert(afterElementPtr, edgeMapPtr);
      if (element_isBad(afterElementPtr)) {
        TMaddToBadVector(  badVectorPtr, afterElementPtr);
      }
    }

    //numDelta += PLIST_GETSIZE(borderListPtr);
    numDelta += TMLIST_GETSIZE(borderListPtr);

    return numDelta;
}


/* =============================================================================
 * TMgrowRegion
 * -- Return NULL if success, else pointer to encroached boundary
 * =============================================================================
 */
__attribute__((transaction_safe))
element_t*
TMgrowRegion (element_t* centerElementPtr,
              region_t* regionPtr,
              MAP_T* edgeMapPtr,
              bool* success)
{
  *success = true;
    bool isBoundary = false;

    //TM_SAFE
    if (centerElementPtr->getNumEdge() == 1) {
        isBoundary = true;
    }

    list_t* beforeListPtr = regionPtr->beforeListPtr;
    list_t* borderListPtr = regionPtr->borderListPtr;
    queue_t* expandQueuePtr = regionPtr->expandQueuePtr;

    list_clear(beforeListPtr);
    list_clear(borderListPtr);
    TMQUEUE_CLEAR(expandQueuePtr);

    //[wer210]
    //coordinate_t centerCoordinate = element_getNewPoint(centerElementPtr);
    coordinate_t centerCoordinate;
    element_getNewPoint(centerElementPtr, &centerCoordinate);

    coordinate_t* centerCoordinatePtr = &centerCoordinate;

    TMQUEUE_PUSH(expandQueuePtr, (void*)centerElementPtr);
    while (!TMQUEUE_ISEMPTY(expandQueuePtr)) {

        element_t* currentElementPtr = (element_t*)TMQUEUE_POP(expandQueuePtr);

        TMLIST_INSERT(beforeListPtr, (void*)currentElementPtr); /* no duplicates */
        // __attribute__((transaction_safe))
        list_t* neighborListPtr = element_getNeighborListPtr(currentElementPtr);

        list_iter_t it;
        it = &(neighborListPtr->head);

        while (it->nextPtr != NULL) {
          element_t* neighborElementPtr = (element_t*)it->nextPtr->dataPtr;
          it = it->nextPtr;

            TMelement_isGarbage(neighborElementPtr); /* so we can detect conflicts */
            if (!list_find(beforeListPtr, (void*)neighborElementPtr)) {
              //[wer210] below function includes acos() and sqrt(), now safe
              if (neighborElementPtr->isInCircumCircle(centerCoordinatePtr)) {
                  /* This is part of the region */
                  if (!isBoundary && (neighborElementPtr->getNumEdge() == 1)) {
                        /* Encroached on mesh boundary so split it and restart */
                        return neighborElementPtr;
                    } else {
                        /* Continue breadth-first search */
                        bool isSuccess;
                        isSuccess = TMQUEUE_PUSH(expandQueuePtr,(void*)neighborElementPtr);
                        assert(isSuccess);
                    }
                } else {
                    /* This element borders region; save info for retriangulation */
                edge_t* borderEdgePtr =
                        element_getCommonEdge(neighborElementPtr, currentElementPtr);
                    if (!borderEdgePtr) {
                      //_ITM_abortTransaction(2); // TM_restart
                      *success = false;
                      return NULL;
                    }
                    TMLIST_INSERT(borderListPtr,(void*)borderEdgePtr); /* no duplicates */
                    if (!MAP_CONTAINS(edgeMapPtr, borderEdgePtr)) {
                        MAP_INSERT(edgeMapPtr, borderEdgePtr, neighborElementPtr);
                    }
                }
            } /* not visited before */

        } /* for each neighbor */

    } /* breadth-first search */

    return NULL;
}


/* =============================================================================
 * TMregion_refine
 * -- Returns net number of elements added to mesh
 * =============================================================================
 */
__attribute__((transaction_safe))
long region_t::refine(element_t* elementPtr, mesh_t* meshPtr, bool* success)
{
    long numDelta = 0L;
    MAP_T* edgeMapPtr = NULL;
    element_t* encroachElementPtr = NULL;

    if (TMelement_isGarbage(elementPtr))
      return numDelta; /* so we can detect conflicts */

    while (1) {
      //[wer] MAP_ALLOC = jsw_avlnew(cmp), where cmp should be SAFE
        edgeMapPtr = MAP_ALLOC(NULL, &element_mapCompareEdge);
        assert(edgeMapPtr);
        //[wer210] added one more parameter "success" to indicate successfulness
        encroachElementPtr = TMgrowRegion(elementPtr,
                                          this,
                                          edgeMapPtr,
                                          success);

        if (encroachElementPtr) {
            TMelement_setIsReferenced(encroachElementPtr, true);
            numDelta += refine(encroachElementPtr, meshPtr, success);
            if (TMelement_isGarbage(elementPtr)) {
                break;
            }
        } else {
            break;
        }
        MAP_FREE(edgeMapPtr); // jsw_avldelete(edgeMapPtr)
    }

    /*
     * Perform retriangulation.
     */

    if (!TMelement_isGarbage(elementPtr)) {
      numDelta += TMretriangulate(elementPtr, this, meshPtr, edgeMapPtr);
    }

    MAP_FREE(edgeMapPtr); /* no need to free elements */

    return numDelta;
}


__attribute__((transaction_safe))
void region_t::clearBad()
{
    PVECTOR_CLEAR(badVectorPtr);
}

__attribute__((transaction_safe))
void region_t::transferBad(heap_t* workHeapPtr)
{
    long numBad = PVECTOR_GETSIZE(badVectorPtr);

    for (long i = 0; i < numBad; i++) {
        element_t* badElementPtr = (element_t*)vector_at(badVectorPtr, i);
        if (TMelement_isGarbage(badElementPtr)) {
            delete badElementPtr;
        } else {
            bool status = TMHEAP_INSERT(workHeapPtr, (void*)badElementPtr);
            assert(status);
        }
    }
}
