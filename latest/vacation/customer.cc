/* =============================================================================
 *
 * customer.c
 * -- Representation of customer
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
#include "customer.h"
#include "list.h"
#include "memory.h"
#include "reservation.h"

/* =============================================================================
 * compareReservationInfo
 * =============================================================================
 */
//static
__attribute__((transaction_safe)) long
compareReservationInfo (const void* aPtr, const void* bPtr)
{
    return reservation_info_compare((reservation_info_t*)aPtr,
                                    (reservation_info_t*)bPtr);
}


/* =============================================================================
 * customer_alloc
 * =============================================================================
 */
__attribute__((transaction_safe))
customer_t::customer_t(long _id)
{
    id = _id;

    // NB: must initialize with TM_SAFE compare function
    reservationInfoListPtr = TMLIST_ALLOC(&compareReservationInfo);
    assert(reservationInfoListPtr != NULL);
}


/* =============================================================================
 * customer_free
 * =============================================================================
 */
__attribute__((transaction_safe))
customer_t::~customer_t()
{
    // [mfs] Is this sufficient?  Does it free the whole list?
    TMLIST_FREE(reservationInfoListPtr);
}


/* =============================================================================
 * customer_addReservationInfo
 * -- Returns TRUE if success, else FALSE
 * =============================================================================
 */
__attribute__((transaction_safe)) bool
customer_addReservationInfo (customer_t* customerPtr,
                             reservation_type_t type, long id, long price)
{
    reservation_info_t* reservationInfoPtr =
        new reservation_info_t(type, id, price);

    list_t* reservationInfoListPtr =
        customerPtr->reservationInfoListPtr;

    return TMLIST_INSERT(reservationInfoListPtr, (void*)reservationInfoPtr);
}


/* =============================================================================
 * customer_removeReservationInfo
 * -- Returns TRUE if success, else FALSE
 * =============================================================================
 */
//[wer210] called only in manager.c, cancel() which is used to cancel a
//         flight/car/room, which never happens...
__attribute__((transaction_safe)) bool
customer_removeReservationInfo (customer_t* customerPtr,
                                reservation_type_t type, long id)
{
    // NB: price not used to compare reservation infos
    reservation_info_t findReservationInfo(type, id, 0);

    list_t* reservationInfoListPtr = customerPtr->reservationInfoListPtr;

    reservation_info_t* reservationInfoPtr =
        (reservation_info_t*)TMLIST_FIND(reservationInfoListPtr,
                                         &findReservationInfo);

    if (reservationInfoPtr == NULL) {
        return false;
    }
    bool status = TMLIST_REMOVE(reservationInfoListPtr,
                                  (void*)&findReservationInfo);

    //[wer210] get rid of restart()
    if (status == false) {
      //_ITM_abortTransaction(2);
      return false;
    }

    delete reservationInfoPtr;

    return true;
}


/* =============================================================================
 * customer_getBill
 * -- Returns total cost of reservations
 * =============================================================================
 */
__attribute__((transaction_safe)) long
customer_getBill (  customer_t* customerPtr)
{
    long bill = 0;
    list_iter_t it;
    list_t* reservationInfoListPtr = customerPtr->reservationInfoListPtr;

    TMLIST_ITER_RESET(&it, reservationInfoListPtr);
    while (TMLIST_ITER_HASNEXT(&it)) {
        reservation_info_t* reservationInfoPtr =
            (reservation_info_t*)TMLIST_ITER_NEXT(&it);
        bill += reservationInfoPtr->price;
    }

    return bill;
}


/* =============================================================================
 * TEST_CUSTOMER
 * =============================================================================
 */
#ifdef TEST_CUSTOMER


#include <assert.h>
#include <stdio.h>


int
main ()
{
    customer_t* customer1Ptr;
    customer_t* customer2Ptr;
    customer_t* customer3Ptr;

    assert(memory_init(1, 4, 2));

    puts("Starting...");

    customer1Ptr = customer_alloc(314);
    customer2Ptr = customer_alloc(314);
    customer3Ptr = customer_alloc(413);

    /* Test compare */
    /* =============================================================================
     * customer_compare
     * -- Returns -1 if A < B, 0 if A = B, 1 if A > B
     * REMOVED, never used except here.
     * =============================================================================
     */
    //assert(customer_compare(customer1Ptr, customer2Ptr) == 0);
    assert((customer1Ptr->id - customer2Ptr->id) == 0);
    assert((customer2Ptr->id, customer3Ptr->id) != 0);
    assert((customer1Ptr->id, customer3Ptr->id) != 0);

    /* Test add reservation info */
    assert(customer_addReservationInfo(customer1Ptr, 0, 1, 2));
    assert(!customer_addReservationInfo(customer1Ptr, 0, 1, 2));
    assert(customer_addReservationInfo(customer1Ptr, 1, 1, 3));
    assert(customer_getBill(customer1Ptr) == 5);

    /* Test remove reservation info */
    assert(!customer_removeReservationInfo(customer1Ptr, 0, 2));
    assert(!customer_removeReservationInfo(customer1Ptr, 2, 0));
    assert(customer_removeReservationInfo(customer1Ptr, 0, 1));
    assert(!customer_removeReservationInfo(customer1Ptr, 0, 1));
    assert(customer_getBill(customer1Ptr) == 3);
    assert(customer_removeReservationInfo(customer1Ptr, 1, 1));
    assert(customer_getBill(customer1Ptr) == 0);

    customer_free(customer1Ptr);
    customer_free(customer2Ptr);
    customer_free(customer3Ptr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_CUSTOMER */
