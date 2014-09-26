/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "vector.h"


typedef vector_t dictionary_t;


extern const char* global_defaultSignatures[];
extern const long global_numDefaultSignature;


/* =============================================================================
 * dictionary_alloc
 * =============================================================================
 */
dictionary_t*
dictionary_alloc ();


/* =============================================================================
 * Pdictionary_alloc
 * =============================================================================
 */
dictionary_t*
Pdictionary_alloc ();


/* =============================================================================
 * dictionary_free
 * =============================================================================
 */
void
dictionary_free (dictionary_t* dictionaryPtr);


/* =============================================================================
 * Pdictionary_free
 * =============================================================================
 */
void
Pdictionary_free (dictionary_t* dictionaryPtr);


/* =============================================================================
 * dictionary_add
 * =============================================================================
 */
bool
dictionary_add (dictionary_t* dictionaryPtr, char* str);


/* =============================================================================
 * dictionary_get
 * =============================================================================
 */
char*
dictionary_get (dictionary_t* dictionaryPtr, long i);


/* =============================================================================
 * dictionary_match
 * =============================================================================
 */
char*
dictionary_match (dictionary_t* dictionaryPtr, char* str);

