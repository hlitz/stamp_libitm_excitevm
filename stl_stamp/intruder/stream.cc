/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "detector.h"
#include "dictionary.h"
#include "packet.h"
#include "stream.h"

stream_t::stream_t(long _percentAttack)
{
    assert(_percentAttack >= 0 && _percentAttack <= 100);
    percentAttack = _percentAttack;
    randomPtr = new std::mt19937();
    assert(randomPtr);
    allocVectorPtr = vector_alloc(1);
    assert(allocVectorPtr);
    packetQueuePtr = queue_alloc(-1);
    assert(packetQueuePtr);
    attackMapPtr = MAP_ALLOC(NULL, NULL);
    assert(attackMapPtr);
}


stream_t::~stream_t()
{
    long numAlloc = vector_getSize(allocVectorPtr);

    for (long a = 0; a < numAlloc; a++) {
        char* str = (char*)vector_at(allocVectorPtr, a);
        free(str);
    }

    MAP_FREE(attackMapPtr);
    queue_free(packetQueuePtr);
    vector_free(allocVectorPtr);
    delete randomPtr;
}


/* =============================================================================
 * splitIntoPackets
 * -- Packets will be equal-size chunks except for last one, which will have
 *    all extra bytes
 * =============================================================================
 */
static void splitIntoPackets(char* str,
                             long flowId,
                             std::mt19937* randomPtr,
                             vector_t* allocVectorPtr,
                             queue_t* packetQueuePtr)
{
    long numByte = strlen(str);
    long numPacket = randomPtr->operator()() % numByte + 1;

    long numDataByte = numByte / numPacket;

    long p;
    for (p = 0; p < (numPacket - 1); p++) {
        bool status;
        char* bytes = (char*)malloc(PACKET_HEADER_LENGTH + numDataByte);
        assert(bytes);
        status = vector_pushBack(allocVectorPtr, (void*)bytes);
        assert(status);
        packet_t* packetPtr = (packet_t*)bytes;
        packetPtr->flowId      = flowId;
        packetPtr->fragmentId  = p;
        packetPtr->numFragment = numPacket;
        packetPtr->length      = numDataByte;
        memcpy(packetPtr->data, (str + p * numDataByte), numDataByte);
        status = queue_push(packetQueuePtr, (void*)packetPtr);
        assert(status);
    }

    bool status;
    long lastNumDataByte = numDataByte + numByte % numPacket;
    char* bytes = (char*)malloc(PACKET_HEADER_LENGTH + lastNumDataByte);
    assert(bytes);
    status = vector_pushBack(allocVectorPtr, (void*)bytes);
    assert(status);
    packet_t* packetPtr = (packet_t*)bytes;
    packetPtr->flowId      = flowId;
    packetPtr->fragmentId  = p;
    packetPtr->numFragment = numPacket;
    packetPtr->length      = lastNumDataByte;
    memcpy(packetPtr->data, (str + p * numDataByte), lastNumDataByte);
    status = queue_push(packetQueuePtr, (void*)packetPtr);
    assert(status);
}


/* =============================================================================
 * stream_generate
 * -- Returns number of attacks generated
 * =============================================================================
 */
long stream_t::generate(dictionary_t* dictionaryPtr,
                        long numFlow,
                        long seed,
                        long maxLength)
{
    long numAttack = 0;

    detector_t* detectorPtr = detector_alloc();
    assert(detectorPtr);
    detector_addPreprocessor(detectorPtr, &preprocessor_toLower);

    randomPtr->seed(seed);
    queue_clear(packetQueuePtr);

    long range = '~' - ' ' + 1;
    assert(range > 0);

    long f;
    for (f = 1; f <= numFlow; f++) {
        char* str;
        //[wer210] added cast to long
        if ((long)(randomPtr->operator()() % 100) < percentAttack) {
            long s = randomPtr->operator()() % global_numDefaultSignature;
            str = dictionary_get(dictionaryPtr, s);
            bool status =
                MAP_INSERT(attackMapPtr, (void*)f, (void*)str);
            assert(status);
            numAttack++;
        } else {
            /*
             * Create random string
             */
            long length = (randomPtr->operator()() % maxLength) + 1;
            str = (char*)malloc((length + 1) * sizeof(char));
            bool status = vector_pushBack(allocVectorPtr, (void*)str);
            assert(status);
            long l;
            for (l = 0; l < length; l++) {
                str[l] = ' ' + (char)(randomPtr->operator()() % range);
            }
            str[l] = '\0';
            char* str2 = (char*)malloc((length + 1) * sizeof(char));
            assert(str2);
            strcpy(str2, str);
            int_error_t error = detector_process(detectorPtr, str2); /* updates in-place */
            if (error == ERROR_SIGNATURE) {
                bool status = MAP_INSERT(attackMapPtr,
                                           (void*)f,
                                           (void*)str);
                assert(status);
                numAttack++;
            }
            free(str2);
        }
        splitIntoPackets(str, f, randomPtr, allocVectorPtr, packetQueuePtr);
    }

    queue_shuffle(packetQueuePtr, randomPtr);

    detector_free(detectorPtr);

    return numAttack;
}



/* =============================================================================
 * stream_getPacket
 * -- If none, returns NULL
 * =============================================================================
 */
__attribute__((transaction_safe))
char* stream_t::getPacket()
{
    return (char*)TMQUEUE_POP(packetQueuePtr);
}


/* =============================================================================
 * stream_isAttack
 * =============================================================================
 */
bool stream_t::isAttack(long flowId)
{
    return MAP_CONTAINS(attackMapPtr, (void*)flowId);
}


/* #############################################################################
 * TEST_STREAM
 * #############################################################################
 */
#ifdef TEST_STREAM


#include <assert.h>
#include <stdio.h>


int
main ()
{
    long percentAttack = 10;
    long numFlow = 100;
    long seed = 0;
    long maxLength = 20;

    puts("Starting...");

    stream_t* streamPtr = stream_alloc(percentAttack);
    assert(streamPtr);

    dictionary_t* dictionaryPtr = dictionary_alloc();
    assert(dictionaryPtr);

    stream_generate(streamPtr, dictionaryPtr, numFlow, seed, maxLength);

    char* bytes;
    while ((bytes = stream_getPacket(streamPtr))) {
        packet_t* packetPtr = (packet_t*)bytes;
        long  flowId      = packetPtr->flowId;
        long  fragmentId  = packetPtr->fragmentId;
        long  numFragment = packetPtr->numFragment;
        long  length      = packetPtr->length;
        char* data        = packetPtr->data;
        long l;
        printf("flow=%2li frag=%2li num=%2li data=", flowId, fragmentId, numFragment);
        for (l = 0; l < length; l++) {
            printf("%c", data[l]);
        }
        puts("");
    }

    stream_free(streamPtr);

    puts("Done.");

    return 0;
}


#endif /* TEST_STREAM */
