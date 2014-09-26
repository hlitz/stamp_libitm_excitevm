/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <assert.h>
#include <stdlib.h>
#include "detector.h"
#include "error.h"
#include "preprocessor.h"


/* =============================================================================
 * detector_alloc
 * =============================================================================
 */
detector_t*
detector_alloc ()
{
    detector_t* detectorPtr;

    detectorPtr = (detector_t*)malloc(sizeof(detector_t));
    if (detectorPtr) {
        detectorPtr->dictionaryPtr = dictionary_alloc();
        assert(detectorPtr->dictionaryPtr);
        detectorPtr->preprocessorVectorPtr = vector_alloc(1);
        assert(detectorPtr->preprocessorVectorPtr);
    }

    return detectorPtr;
}


/* =============================================================================
 * Pdetector_alloc
 * =============================================================================
 */
detector_t*
Pdetector_alloc ()
{
    detector_t* detectorPtr;

    detectorPtr = (detector_t*)malloc(sizeof(detector_t));
    if (detectorPtr) {
        detectorPtr->dictionaryPtr = Pdictionary_alloc();
        assert(detectorPtr->dictionaryPtr);
        detectorPtr->preprocessorVectorPtr = PVECTOR_ALLOC(1);
        assert(detectorPtr->preprocessorVectorPtr);
    }

    return detectorPtr;
}


/* =============================================================================
 * detector_free
 * =============================================================================
 */
void
detector_free (detector_t* detectorPtr)
{
    dictionary_free(detectorPtr->dictionaryPtr);
    vector_free(detectorPtr->preprocessorVectorPtr);
    free(detectorPtr);
}


/* =============================================================================
 * Pdetector_free
 * =============================================================================
 */
void
Pdetector_free (detector_t* detectorPtr)
{
    Pdictionary_free(detectorPtr->dictionaryPtr);
    PVECTOR_FREE(detectorPtr->preprocessorVectorPtr);
    free(detectorPtr);
}


/* =============================================================================
 * detector_addPreprocessor
 * =============================================================================
 */
void
detector_addPreprocessor (detector_t* detectorPtr, preprocessor_t p)
{
    bool status = vector_pushBack(detectorPtr->preprocessorVectorPtr,
                                    (void*)p);
    assert(status);
}


/* =============================================================================
 * detector_process
 * =============================================================================
 */
int_error_t
detector_process (detector_t* detectorPtr, char* str)
{
    /*
     * Apply preprocessors
     */

    vector_t* preprocessorVectorPtr = detectorPtr->preprocessorVectorPtr;
    long p;
    long numPreprocessor = vector_getSize(preprocessorVectorPtr);
    for (p = 0; p < numPreprocessor; p++) {
        preprocessor_t preprocessor =
            (preprocessor_t)vector_at(preprocessorVectorPtr, p);
        preprocessor(str);
    }

    /*
     * Check against signatures of known attacks
     */

    char* signature = dictionary_match(detectorPtr->dictionaryPtr, str);
    if (signature) {
        return ERROR_SIGNATURE;
    }

    return ERROR_NONE;
}


/* #############################################################################
 * TEST_DETECTOR
 * #############################################################################
 */
#ifdef TEST_DETECTOR


#include <assert.h>
#include <stdio.h>


char str1[] = "test";
char str2[] = "abouts";
char str3[] = "aBoUt";
char str4[] = "%41Bout";


int
main ()
{
    puts("Starting...");

    detector_t* detectorPtr = detector_alloc();

    detector_addPreprocessor(detectorPtr, &preprocessor_convertURNHex);
    detector_addPreprocessor(detectorPtr, &preprocessor_toLower);

    assert(detector_process(detectorPtr, str1) == ERROR_NONE);
    assert(detector_process(detectorPtr, str2) == ERROR_SIGNATURE);
    assert(detector_process(detectorPtr, str3) == ERROR_SIGNATURE);
    assert(detector_process(detectorPtr, str4) == ERROR_SIGNATURE);

    detector_free(detectorPtr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_DETECTOR */


/* =============================================================================
 *
 * End of detector.c
 *
 * =============================================================================
 */
