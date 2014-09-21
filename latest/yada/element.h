/* =============================================================================
 *
 * element.h
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
void
element_printAngles (element_t* elementPtr);



#define TMELEMENT_ALLOC(c, n)           TMelement_alloc(  c, n)
#define TMELEMENT_FREE(e)               TMelement_free(  e)
#define TMELEMENT_ISREFERENCED(e)       TMelement_isReferenced(  e)
#define TMELEMENT_SETISREFERENCED(e, s) TMelement_setIsReferenced(  e, s)
#define TMELEMENT_ISGARBAGE(e)          TMelement_isGarbage(  e)
#define TMELEMENT_SETISGARBAGE(e, s)    TMelement_setIsGarbage(  e, s)
#define TMELEMENT_ADDNEIGHBOR(e, n)     TMelement_addNeighbor(  e, n)
#define TMELEMENT_GETNEIGHBORLIST(e)    TMelement_getNeighborListPtr(  e)
