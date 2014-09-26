/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "map.h"
#include "queue.h"
#include "error.h"

struct decoder_t {
    MAP_T* fragmentedMapPtr;  /* contains list of packet_t* */
    queue_t* decodedQueuePtr; /* contains decoded_t* */

    decoder_t();

    ~decoder_t();

    __attribute__((transaction_safe))
    int_error_t process(char* bytes, long numByte);

    __attribute__((transaction_safe))
    char* getComplete(long* decodedFlowIdPtr);
};
