/* =============================================================================
 *
 * client.c
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
#include "action.h"
#include "client.h"
#include "manager.h"
#include "reservation.h"
#include "thread.h"


/* =============================================================================
 * client_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
client_t*
client_alloc (long id,
              manager_t* managerPtr,
              long numOperation,
              long numQueryPerTransaction,
              long queryRange,
              long percentUser)
{
    client_t* clientPtr;

    clientPtr = (client_t*)malloc(sizeof(client_t));
    if (clientPtr == NULL) {
        return NULL;
    }

    clientPtr->randomPtr = new std::mt19937();
    if (clientPtr->randomPtr == NULL) {
        return NULL;
    }

    clientPtr->id = id;
    clientPtr->managerPtr = managerPtr;
    clientPtr->randomPtr->seed(id);
    clientPtr->numOperation = numOperation;
    clientPtr->numQueryPerTransaction = numQueryPerTransaction;
    clientPtr->queryRange = queryRange;
    clientPtr->percentUser = percentUser;

    return clientPtr;
}


/* =============================================================================
 * client_free
 * =============================================================================
 */
void
client_free (client_t* clientPtr)
{
    free(clientPtr);
}


/* =============================================================================
 * selectAction
 * =============================================================================
 */
static action_t
selectAction (long r, long percentUser)
{
    action_t action;

    if (r < percentUser) {
        action = ACTION_MAKE_RESERVATION;
    } else if (r & 1) {
        action = ACTION_DELETE_CUSTOMER;
    } else {
        action = ACTION_UPDATE_TABLES;
    }

    return action;
}


/* =============================================================================
 * client_run
 * -- Execute list operations on the database
 * =============================================================================
 */
void
client_run (void* argPtr)
{
    long myId = thread_getId();
    client_t* clientPtr = ((client_t**)argPtr)[myId];

    manager_t* managerPtr = clientPtr->managerPtr;
    std::mt19937*  randomPtr  = clientPtr->randomPtr;

    long numOperation           = clientPtr->numOperation;
    long numQueryPerTransaction = clientPtr->numQueryPerTransaction;
    long queryRange             = clientPtr->queryRange;
    long percentUser            = clientPtr->percentUser;

    long* types  = (long*)malloc(numQueryPerTransaction * sizeof(long));
    long* ids    = (long*)malloc(numQueryPerTransaction * sizeof(long));
    long* ops    = (long*)malloc(numQueryPerTransaction * sizeof(long));
    long* prices = (long*)malloc(numQueryPerTransaction * sizeof(long));

    for (long i = 0; i < numOperation; i++) {

        long r = randomPtr->operator()() % 100;
        action_t action = selectAction(r, percentUser);

        switch (action) {
            case ACTION_MAKE_RESERVATION: {
                long maxPrices[NUM_RESERVATION_TYPE] = { -1, -1, -1 };
                long maxIds[NUM_RESERVATION_TYPE] = { -1, -1, -1 };
                long n;
                long numQuery = randomPtr->operator()() % numQueryPerTransaction + 1;
                long customerId = randomPtr->operator()() % queryRange + 1;
                for (n = 0; n < numQuery; n++) {
                    types[n] = randomPtr->operator()() % NUM_RESERVATION_TYPE;
                    ids[n] = (randomPtr->operator()() % queryRange) + 1;
                }
                bool isFound = false;
                bool done = true;
                //[wer210] I modified here to remove _ITM_abortTransaction().
                while (1) {
                  __transaction_atomic {
                    for (n = 0; n < numQuery; n++) {
                      long t = types[n];
                      long id = ids[n];
                      long price = -1;
                      switch (t) {
                       case RESERVATION_CAR:
                        if (manager_queryCar(managerPtr, id) >= 0) {
                          price = manager_queryCarPrice(managerPtr, id);
                        }
                        break;
                       case RESERVATION_FLIGHT:
                        if (manager_queryFlight(managerPtr, id) >= 0) {
                          price = manager_queryFlightPrice(managerPtr, id);
                        }
                        break;
                       case RESERVATION_ROOM:
                        if (manager_queryRoom(managerPtr, id) >= 0) {
                          price = manager_queryRoomPrice(managerPtr, id);
                        }
                        break;
                       default:
                        assert(0);
                      }
                      //[wer210] read-only above
                      if (price > maxPrices[t]) {
                        maxPrices[t] = price;
                        maxIds[t] = id;
                        isFound = true;
                      }
                    } /* for n */

                    if (isFound) {
                      done = done && manager_addCustomer(managerPtr, customerId);
                    }

                    if (maxIds[RESERVATION_CAR] > 0) {
                      done = done && manager_reserveCar(managerPtr,
                                                  customerId, maxIds[RESERVATION_CAR]);
                    }

                    if (maxIds[RESERVATION_FLIGHT] > 0) {
                      done = done && manager_reserveFlight(managerPtr,
                                                     customerId, maxIds[RESERVATION_FLIGHT]);
                    }
                    if (maxIds[RESERVATION_ROOM] > 0) {
                      done = done && manager_reserveRoom(managerPtr,
                                                   customerId, maxIds[RESERVATION_ROOM]);
                    }
                    if (done) break;
                    else __transaction_cancel;
                  } // TM_END
            }
                break;

            }

            case ACTION_DELETE_CUSTOMER: {
                long customerId = randomPtr->operator()() % queryRange + 1;
                bool done = true;
                while (1) {
                  __transaction_atomic {
                    long bill = manager_queryCustomerBill(managerPtr, customerId);
                    if (bill >= 0) {
                      done = done && manager_deleteCustomer(managerPtr, customerId);
                    }
                    if(done) break;
                    else __transaction_cancel;
                  }
                }
                break;
            }

            case ACTION_UPDATE_TABLES: {
                long numUpdate = randomPtr->operator()() % numQueryPerTransaction + 1;
                long n;
                for (n = 0; n < numUpdate; n++) {
                    types[n] = randomPtr->operator()() % NUM_RESERVATION_TYPE;
                    ids[n] = (randomPtr->operator()() % queryRange) + 1;
                    ops[n] = randomPtr->operator()() % 2;
                    if (ops[n]) {
                        prices[n] = ((randomPtr->operator()() % 5) * 10) + 50;
                    }
                }
                bool done = true;
                while (1) {
                  __transaction_atomic {
                    for (n = 0; n < numUpdate; n++) {
                      long t = types[n];
                      long id = ids[n];
                      long doAdd = ops[n];
                      if (doAdd) {
                        long newPrice = prices[n];
                        switch (t) {
                         case RESERVATION_CAR:
                          done = done && manager_addCar(managerPtr, id, 100, newPrice);
                          break;
                         case RESERVATION_FLIGHT:
                          done = done && manager_addFlight(managerPtr, id, 100, newPrice);
                          break;
                         case RESERVATION_ROOM:
                          done = done && manager_addRoom(managerPtr, id, 100, newPrice);
                          break;
                         default:
                          assert(0);
                        }
                      } else { /* do delete */
                        switch (t) {
                         case RESERVATION_CAR:
                          done = done && manager_deleteCar(managerPtr, id, 100);
                          break;
                         case RESERVATION_FLIGHT:
                          done = done && manager_deleteFlight(managerPtr, id);
                          break;
                         case RESERVATION_ROOM:
                          done = done && manager_deleteRoom(managerPtr, id, 100);
                          break;
                         default:
                          assert(0);
                        }
                      }
                    }
                  if (done) break;
                  else __transaction_cancel;
                  } // TM_END
                }
                break;
            }

            default:
                assert(0);

        } /* switch (action) */

    } /* for i */

}


/* =============================================================================
 *
 * End of client.c
 *
 * =============================================================================
 */




