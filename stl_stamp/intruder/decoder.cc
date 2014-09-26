/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"
#include "error.h"
#include "list.h"
#include "packet.h"
#include "tm_transition.h"

__attribute__ ((transaction_pure))
void TM_print(char* s)
{
  printf("%s", s);
}


struct decoded_t {
    long flowId;
    char* data;
};

decoder_t::decoder_t()
{
    fragmentedMapPtr = MAP_ALLOC(NULL, NULL);
    assert(fragmentedMapPtr);
    decodedQueuePtr = queue_alloc(1024);
    assert(decodedQueuePtr);
}

decoder_t::~decoder_t()
{
    queue_free(decodedQueuePtr);
    MAP_FREE(fragmentedMapPtr);
}

//[wer] this function was problematic to write-back algorithms.
__attribute__((transaction_safe))
int_error_t decoder_t::process(char* bytes, long numByte)
{
    bool status;

    /*
     * Basic error checking
     */
    // [wer210] added cast of type to avoid warnings.
    if (numByte < (long) PACKET_HEADER_LENGTH) {
        return ERROR_SHORT;
    }

    packet_t* packetPtr = (packet_t*)bytes;
    long flowId      = packetPtr->flowId;
    long fragmentId  = packetPtr->fragmentId;
    long numFragment = packetPtr->numFragment;
    long length      = packetPtr->length;

    if (flowId < 0) {
        return ERROR_FLOWID;
    }

    if ((fragmentId < 0) || (fragmentId >= numFragment)) {
        return ERROR_FRAGMENTID;
    }

    if (length < 0) {
        return ERROR_LENGTH;
    }

#if 0
    /*
     * With the above checks, this one is redundant
     */
    if (numFragment < 1) {
        return ERROR_NUMFRAGMENT;
    }
#endif

    /*
     * Add to fragmented map for reassembling
     */

    if (numFragment > 1) {

      list_t* fragmentListPtr =
        (list_t*)TMMAP_FIND(fragmentedMapPtr, (void*)flowId);

      if (fragmentListPtr == NULL) {

        // [wer210] the comparator should be __attribute__((transaction_safe))
        fragmentListPtr = TMLIST_ALLOC(&packet_compareFragmentId);
        assert(fragmentListPtr);
        status = TMLIST_INSERT(fragmentListPtr, (void*)packetPtr);
        assert(status);
        status = TMMAP_INSERT(fragmentedMapPtr,
                              (void*)flowId,
                              (void*)fragmentListPtr);
        assert(status);

      }
      else {
        list_iter_t it;
        //TMLIST_ITER_RESET(&it, fragmentListPtr);
        it = &(fragmentListPtr->head);

        //assert(TMLIST_ITER_HASNEXT(&it, fragmentListPtr));
        assert (it->nextPtr != NULL);

        //packet_t* firstFragmentPtr =
        //(packet_t*)TMLIST_ITER_NEXT(&it, fragmentListPtr);
        packet_t* firstFragmentPtr = (packet_t*)it->nextPtr->dataPtr;
        it = it->nextPtr;

        long expectedNumFragment = firstFragmentPtr->numFragment;

        if (numFragment != expectedNumFragment) {
          status = TMMAP_REMOVE(fragmentedMapPtr, (void*)flowId);
          assert(status);
          return ERROR_NUMFRAGMENT;
        }

        status = TMLIST_INSERT(fragmentListPtr, (void*)packetPtr);
        assert(status);

        /*
         * If we have all the fragments we can reassemble them
         */

        if (TMLIST_GETSIZE(fragmentListPtr) == numFragment) {

          long numByte = 0;
          long i = 0;
          //TMLIST_ITER_RESET(&it, fragmentListPtr);
          it = &(fragmentListPtr->head);

          //while (TMLIST_ITER_HASNEXT(&it, fragmentListPtr)) {
          while (it->nextPtr != NULL) {
            //packet_t* fragmentPtr =
            // (packet_t*)TMLIST_ITER_NEXT(&it, fragmentListPtr);
            packet_t* fragmentPtr = (packet_t*)it->nextPtr->dataPtr;
            it = it->nextPtr;

            assert(fragmentPtr->flowId == flowId);
            if (fragmentPtr->fragmentId != i) {
              status = TMMAP_REMOVE(fragmentedMapPtr, (void*)flowId);
              assert(status);
              return ERROR_INCOMPLETE; /* should be sequential */
            }
            numByte += fragmentPtr->length;
            i++;
          }

          char* data = (char*)malloc(numByte + 1);
          assert(data);
          data[numByte] = '\0';
          char* dst = data;

          //TMLIST_ITER_RESET(&it, fragmentListPtr);
          it = &(fragmentListPtr->head);

          //while (TMLIST_ITER_HASNEXT(&it, fragmentListPtr)) {
          while (it->nextPtr != NULL) {
            //packet_t* fragmentPtr =
            //  (packet_t*)TMLIST_ITER_NEXT(&it, fragmentListPtr);
            packet_t* fragmentPtr = (packet_t*)it->nextPtr->dataPtr;
            it = it->nextPtr;

            long i = 0;
            for (; i < fragmentPtr->length; i++)
              dst[i] = fragmentPtr->data[i];

            dst += fragmentPtr->length;

          }
          assert(dst == data + numByte);

          decoded_t* decodedPtr = (decoded_t*)malloc(sizeof(decoded_t));
          assert(decodedPtr);
          decodedPtr->flowId = flowId;
          decodedPtr->data = data;

          status = TMQUEUE_PUSH(decodedQueuePtr, (void*)decodedPtr);
          assert(status);

          TMLIST_FREE(fragmentListPtr);
          status = TMMAP_REMOVE(fragmentedMapPtr, (void*)flowId);
          assert(status);
        }

      }

    } else {
      /*
       * This is the only fragment, so it is ready
       */
        if (fragmentId != 0) {
          return ERROR_FRAGMENTID;
        }

        char* data = (char*)malloc(length + 1);
        assert(data);
        data[length] = '\0';
        memcpy(data, packetPtr->data, length);

        decoded_t* decodedPtr = (decoded_t*)malloc(sizeof(decoded_t));
        assert(decodedPtr);
        decodedPtr->flowId = flowId;
        decodedPtr->data = data;

        status = TMQUEUE_PUSH(decodedQueuePtr, (void*)decodedPtr);
        assert(status);

    }

    return ERROR_NONE;
}


/* =============================================================================
 * TMdecoder_getComplete
 * -- If none, returns NULL
 * =============================================================================
 */
__attribute__((transaction_safe))
char* decoder_t::getComplete(long* decodedFlowIdPtr)
{
    char* data;
    decoded_t* decodedPtr = (decoded_t*)TMQUEUE_POP(decodedQueuePtr);

    if (decodedPtr) {
        *decodedFlowIdPtr = decodedPtr->flowId;
        data = decodedPtr->data;
        free(decodedPtr);
    } else {
        *decodedFlowIdPtr = -1;
        data = NULL;
    }

    return data;
}


/* #############################################################################
 * TEST_DECODER
 * #############################################################################
 */
#ifdef TEST_DECODER

#include <stdio.h>


int
main ()
{
    decoder_t* decoderPtr;

    puts("Starting...");

    decoderPtr = decoder_alloc();
    assert(decoderPtr);

    long numDataByte = 3;
    long numPacketByte = PACKET_HEADER_LENGTH + numDataByte;

    char* abcBytes = (char*)malloc(numPacketByte);
    assert(abcBytes);
    packet_t* abcPacketPtr;
    abcPacketPtr = (packet_t*)abcBytes;
    abcPacketPtr->flowId = 1;
    abcPacketPtr->fragmentId = 0;
    abcPacketPtr->numFragment = 2;
    abcPacketPtr->length = numDataByte;
    abcPacketPtr->data[0] = 'a';
    abcPacketPtr->data[1] = 'b';
    abcPacketPtr->data[2] = 'c';

    char* defBytes = (char*)malloc(numPacketByte);
    assert(defBytes);
    packet_t* defPacketPtr;
    defPacketPtr = (packet_t*)defBytes;
    defPacketPtr->flowId = 1;
    defPacketPtr->fragmentId = 1;
    defPacketPtr->numFragment = 2;
    defPacketPtr->length = numDataByte;
    defPacketPtr->data[0] = 'd';
    defPacketPtr->data[1] = 'e';
    defPacketPtr->data[2] = 'f';

    assert(TMdecoder_process(decoderPtr, abcBytes, numDataByte) == ERROR_SHORT);

    abcPacketPtr->flowId = -1;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_FLOWID);
    abcPacketPtr->flowId = 1;

    abcPacketPtr->fragmentId = -1;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_FRAGMENTID);
    abcPacketPtr->fragmentId = 0;

    abcPacketPtr->fragmentId = 2;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_FRAGMENTID);
    abcPacketPtr->fragmentId = 0;

    abcPacketPtr->fragmentId = 2;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_FRAGMENTID);
    abcPacketPtr->fragmentId = 0;

    abcPacketPtr->length = -1;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_LENGTH);
    abcPacketPtr->length = numDataByte;

    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_NONE);
    defPacketPtr->numFragment = 3;
    assert(TMdecoder_process(decoderPtr, defBytes, numPacketByte) == ERROR_NUMFRAGMENT);
    defPacketPtr->numFragment = 2;

    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_NONE);
    defPacketPtr->fragmentId = 0;
    assert(TMdecoder_process(decoderPtr, defBytes, numPacketByte) == ERROR_INCOMPLETE);
    defPacketPtr->fragmentId = 1;

    long flowId;
    assert(TMdecoder_process(decoderPtr, defBytes, numPacketByte) == ERROR_NONE);
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_NONE);
    char* str = TMdecoder_getComplete(decoderPtr, &flowId);
    assert(strcmp(str, "abcdef") == 0);
    free(str);
    assert(flowId == 1);

    abcPacketPtr->numFragment = 1;
    assert(TMdecoder_process(decoderPtr, abcBytes, numPacketByte) == ERROR_NONE);
    str = TMdecoder_getComplete(decoderPtr, &flowId);
    assert(strcmp(str, "abc") == 0);
    free(str);
    abcPacketPtr->numFragment = 2;
    assert(flowId == 1);

    str = TMdecoder_getComplete(decoderPtr, &flowId);
    assert(str == NULL);
    assert(flowId == -1);

    decoder_free(decoderPtr);

    free(abcBytes);
    free(defBytes);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_DECODER */


/* =============================================================================
 *
 * End of decoder.c
 *
 * =============================================================================
 */
