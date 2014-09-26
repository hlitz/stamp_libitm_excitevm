/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "error.h"
#include "preprocessor.h"
#include "vector.h"
#include "dictionary.h"

struct detector_t {
    dictionary_t* dictionaryPtr;
    vector_t* preprocessorVectorPtr;
};


/* =============================================================================
 * detector_alloc
 * =============================================================================
 */
detector_t*
detector_alloc ();


/* =============================================================================
 * Pdetector_alloc
 * =============================================================================
 */
detector_t*
Pdetector_alloc ();


/* =============================================================================
 * detector_free
 * =============================================================================
 */
void
detector_free (detector_t* detectorPtr);


/* =============================================================================
 * Pdetector_free
 * =============================================================================
 */
void
Pdetector_free (detector_t* detectorPtr);


/* =============================================================================
 * detector_addPreprocessor
 * =============================================================================
 */
void
detector_addPreprocessor (detector_t* detectorPtr, preprocessor_t p);


/* =============================================================================
 * detector_process
 * =============================================================================
 */
int_error_t
detector_process (detector_t* detectorPtr, char* str);
