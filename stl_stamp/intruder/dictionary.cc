/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "dictionary.h"
#include "vector.h"


const char* global_defaultSignatures[] = {
    "about",
    "after",
    "all",
    "also",
    "and",
    "any",
    "back",
    "because",
    "but",
    "can",
    "come",
    "could",
    "day",
    "even",
    "first",
    "for",
    "from",
    "get",
    "give",
    "good",
    "have",
    "him",
    "how",
    "into",
    "its",
    "just",
    "know",
    "like",
    "look",
    "make",
    "most",
    "new",
    "not",
    "now",
    "one",
    "only",
    "other",
    "out",
    "over",
    "people",
    "say",
    "see",
    "she",
    "some",
    "take",
    "than",
    "that",
    "their",
    "them",
    "then",
    "there",
    "these",
    "they",
    "think",
    "this",
    "time",
    "two",
    "use",
    "want",
    "way",
    "well",
    "what",
    "when",
    "which",
    "who",
    "will",
    "with",
    "work",
    "would",
    "year",
    "your"
};

const long global_numDefaultSignature =
    sizeof(global_defaultSignatures) / sizeof(global_defaultSignatures[0]);


/* =============================================================================
 * dictionary_alloc
 * =============================================================================
 */
dictionary_t*
dictionary_alloc ()
{
    dictionary_t* dictionaryPtr = vector_alloc(global_numDefaultSignature);

    if (dictionaryPtr) {
        long s;
        for (s = 0; s < global_numDefaultSignature; s++) {
            const char* sig = global_defaultSignatures[s];
            bool status = vector_pushBack(dictionaryPtr,
                                            (void*)sig);
            assert(status);
        }
    }

    return dictionaryPtr;
}


/* =============================================================================
 * Pdictionary_alloc
 * =============================================================================
 */
dictionary_t*
Pdictionary_alloc ()
{
    dictionary_t* dictionaryPtr = PVECTOR_ALLOC(global_numDefaultSignature);

    if (dictionaryPtr) {
        long s;
        for (s = 0; s < global_numDefaultSignature; s++) {
            const char* sig = global_defaultSignatures[s];
            bool status = PVECTOR_PUSHBACK(dictionaryPtr,
                                             (void*)sig);
            assert(status);
        }
    }

    return dictionaryPtr;
}


/* =============================================================================
 * dictionary_free
 * =============================================================================
 */
void
dictionary_free (dictionary_t* dictionaryPtr)
{
    vector_free(dictionaryPtr);
}


/* =============================================================================
 * Pdictionary_free
 * =============================================================================
 */
void
Pdictionary_free (dictionary_t* dictionaryPtr)
{
    PVECTOR_FREE(dictionaryPtr);
}


/* =============================================================================
 * dictionary_add
 * =============================================================================
 */
bool
dictionary_add (dictionary_t* dictionaryPtr, char* str)
{
    return vector_pushBack(dictionaryPtr, (void*)str);
}


/* =============================================================================
 * dictionary_get
 * =============================================================================
 */
char*
dictionary_get (dictionary_t* dictionaryPtr, long i)
{
    return (char*)vector_at(dictionaryPtr, i);
}


/* =============================================================================
 * dictionary_match
 * =============================================================================
 */
char*
dictionary_match (dictionary_t* dictionaryPtr, char* str)
{
    long s;
    long numSignature = vector_getSize(dictionaryPtr);

    for (s = 0; s < numSignature; s++) {
        char* sig = (char*)vector_at(dictionaryPtr, s);
        if (strstr(str, sig) != NULL) {
            return sig;
        }
    }

    return NULL;
}


/* #############################################################################
 * TEST_DICTIONARY
 * #############################################################################
 */
#ifdef TEST_DICTIONARY


#include <assert.h>
#include <stdio.h>


int
main ()
{
    puts("Starting...");

    dictionary_t* dictionaryPtr;

    dictionaryPtr = dictionary_alloc();
    assert(dictionaryPtr);

    assert(dictionary_add(dictionaryPtr, "test1"));
    char* sig = dictionary_match(dictionaryPtr, "test1");
    assert(strcmp(sig, "test1") == 0);
    sig = dictionary_match(dictionaryPtr, "test1s");
    assert(strcmp(sig, "test1") == 0);
    assert(!dictionary_match(dictionaryPtr, "test2"));

    long s;
    for (s = 0; s < global_numDefaultSignature; s++) {
        char* sig = dictionary_match(dictionaryPtr, global_defaultSignatures[s]);
        assert(strcmp(sig, global_defaultSignatures[s]) == 0);
    }

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_DICTIONARY */


/* =============================================================================
 *
 * End of dictionary.c
 *
 * =============================================================================
 */
