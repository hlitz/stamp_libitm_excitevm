/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include <random>
#include "queue.h"
#include <vector>
#include "map.h"
#include "dictionary.h"

struct stream_t {
    long percentAttack;
    std::mt19937* randomPtr;
    std::vector<char*>* allocVectorPtr;
    queue_t* packetQueuePtr;
    MAP_T* attackMapPtr;

    stream_t(long percentAttack);
    ~stream_t();

    /*
     * stream_generate: Returns number of attacks generated
     */
    long generate(dictionary_t* dictionaryPtr,
                  long numFlow,
                  long seed,
                  long maxLength);

    /*
     * stream_getPacket: If none, returns NULL
     */
    __attribute__((transaction_safe))
    char* getPacket();

    bool isAttack(long flowId);
};
