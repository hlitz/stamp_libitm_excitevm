/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "coordinate.h"
#include "list.h"
#include "pair.h"

typedef pair_t         edge_t;
struct element_t {
    coordinate_t coordinates[3];
    long numCoordinate;
    coordinate_t circumCenter;
    double circumRadius;
    double minAngle;
    edge_t edges[3];
    long numEdge;
    coordinate_t midpoints[3]; /* midpoint of each edge */
    double radii[3];           /* half of edge length */
    edge_t* encroachedEdgePtr; /* opposite obtuse angle */
    bool isSkinny;
    list_t* neighborListPtr;
    bool isGarbage;
    bool isReferenced;
};


/* =============================================================================
 * element_compare
 * =============================================================================
 */
__attribute__((transaction_safe))
long
element_compare (element_t* aElementPtr, element_t* bElementPtr);


/* =============================================================================
 * element_listCompare
 *
 * For use in list_t
 * =============================================================================
 */
__attribute__((transaction_safe))
long
element_listCompare (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_mapCompare
 *
 * For use in MAP_T
 * =============================================================================
 */
__attribute__((transaction_safe))
long
element_mapCompare (const pair_t* aPtr, const pair_t* bPtr);


/* =============================================================================
 * TMelement_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
__attribute__((transaction_safe))
element_t*
TMelement_alloc (  coordinate_t* coordinates, long numCoordinate);


/* =============================================================================
 * TMelement_free
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMelement_free (  element_t* elementPtr);


/* =============================================================================
 * element_getNumEdge
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
long
element_getNumEdge (element_t* elementPtr);


/* =============================================================================
 * element_getEdge
 *
 * Returned edgePtr is sorted; i.e., coordinate_compare(first, second) < 0
 * =============================================================================
 */
__attribute__((transaction_safe))
//TM_PURE
edge_t*
element_getEdge (element_t* elementPtr, long i);


/* ============================================================================
 * element_listCompareEdge
 *
 * For use in list_t
 * ============================================================================
 */
__attribute__((transaction_safe))
long
element_listCompareEdge (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_mapCompareEdge
 *
 * For use in MAP_T
 * =============================================================================
 */
__attribute__((transaction_safe))
long
element_mapCompareEdge (const pair_t* aPtr, const pair_t* bPtr);


/* =============================================================================
 * element_heapCompare
 *
 * For use in heap_t
 * =============================================================================
 */
__attribute__((transaction_safe))
long
element_heapCompare (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_isInCircumCircle
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
bool
element_isInCircumCircle (element_t* elementPtr, coordinate_t* coordinatePtr);


/* =============================================================================
 * element_clearEncroached
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
void
element_clearEncroached (element_t* elementPtr);


/* =============================================================================
 * element_getEncroachedPtr
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
edge_t*
element_getEncroachedPtr (element_t* elementPtr);


/* =============================================================================
 * element_isSkinny
 * =============================================================================
 */
bool
element_isSkinny (element_t* elementPtr);


/* =============================================================================
 * element_isBad
 * -- Does it need to be refined?
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
bool
element_isBad (element_t* elementPtr);



/* =============================================================================
 * TMelement_isReferenced
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
TMelement_isReferenced (  element_t* elementPtr);



/* =============================================================================
 * TMelement_setIsReferenced
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMelement_setIsReferenced (  element_t* elementPtr, bool status);




/* =============================================================================
 * TMelement_isGarbage
 * -- Can we deallocate?
 * =============================================================================
 */
__attribute__((transaction_safe))
bool
TMelement_isGarbage (  element_t* elementPtr);



/* =============================================================================
 * TMelement_setIsGarbage
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMelement_setIsGarbage (  element_t* elementPtr, bool status);


/* =============================================================================
 * TMelement_addNeighbor
 * =============================================================================
 */
__attribute__((transaction_safe))
void
TMelement_addNeighbor (  element_t* elementPtr, element_t* neighborPtr);


/* =============================================================================
 * element_getNeighborListPtr
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
list_t*
element_getNeighborListPtr (element_t* elementPtr);


/* =============================================================================
 * element_getCommonEdge
 * -- Returns pointer to aElementPtr's shared edge
 * =============================================================================
 */
//TM_PURE
__attribute__((transaction_safe))
edge_t*
element_getCommonEdge (element_t* aElementPtr, element_t* bElementPtr);


/* =============================================================================
 * element_getNewPoint
 * -- Either the element is encroached or is skinny, so get the new point to add
 * =============================================================================
 */
//[wer210] previous returns a struct, which causes errors
//TM_PURE
//coordinate_t
//element_getNewPoint (element_t* elementPtr);
__attribute__((transaction_safe))
void
element_getNewPoint (element_t* elementPtr, coordinate_t* ret);


/* =============================================================================
 * element_checkAngles
 *
 * Return false if minimum angle constraint not met
 * =============================================================================
 */
bool
element_checkAngles (element_t* elementPtr);


/* =============================================================================
 * element_print
 * =============================================================================
 */
void
element_print (element_t* elementPtr);


/* =============================================================================
 * element_printEdge
 * =============================================================================
 */
void
element_printEdge (edge_t* edgePtr);


/* =============================================================================
 * element_printAngles
 * =============================================================================
 */
void element_printAngles (element_t* elementPtr);
