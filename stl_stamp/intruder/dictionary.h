/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

#pragma once

#include "vector.h"


struct dictionary_t
{
    vector_t* stuff;

    dictionary_t();
    ~dictionary_t();
    bool add(char* str);
    char* get(long i);
    char* match(char* str);


};

extern const char* global_defaultSignatures[];
extern const long global_numDefaultSignature;
