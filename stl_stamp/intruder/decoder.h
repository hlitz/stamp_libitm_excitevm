/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "error.h"

struct decoder_t;


/* =============================================================================
 * decoder_alloc
 * =============================================================================
 */
decoder_t*
decoder_alloc ();


/* =============================================================================
 * decoder_free
 * =============================================================================
 */
void
decoder_free (decoder_t* decoderPtr);


/* =============================================================================
 * TMdecoder_process
 * =============================================================================
 */
__attribute__((transaction_safe))
int_error_t
TMdecoder_process (  decoder_t* decoderPtr, char* bytes, long numByte);


/* =============================================================================
 * TMdecoder_getComplete
 * -- If none, returns NULL
 * =============================================================================
 */
__attribute__((transaction_safe))
char*
TMdecoder_getComplete (  decoder_t* decoderPtr, long* decodedFlowIdPtr);
