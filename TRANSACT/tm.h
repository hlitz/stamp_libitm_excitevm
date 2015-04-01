#ifndef TM_H
#define TM_H 1

#include <stdlib.h>                   /* Defines size_t. */
#include <cstdio>
#include <stdint.h>
#include <string.h>

#define TM_PURE                       __attribute__((transaction_pure))
#define TM_SAFE                       __attribute__((transaction_safe))

//Hmm.
TM_SAFE long unsigned int __builtin_object_size(const void*, int);
TM_SAFE void* __builtin___memcpy_chk(void*, const void*,
long unsigned int, long unsigned int);
//}

#define TM_SHARED_READ(var)           var
#define TM_SHARED_READ_P(var)         var
#define TM_SHARED_READ_F(var)         var

#define TM_SHARED_WRITE(var, val)     var = val
#define TM_SHARED_WRITE_P(var, val)   var = val
#define TM_SHARED_WRITE_F(var, val)   var = val

#define TM_LOCAL_WRITE(var, val)      var = val
#define TM_LOCAL_WRITE_P(var, val)    var = val
#define TM_LOCAL_WRITE_F(var, val)    var = val

#ifdef EXCITE_VM
#ifdef DRAFT_EXCITE_VM
Please define only one of DRAFT_EXCITE_VM or EXCITE_VM
#endif
#endif

#ifdef EXCITE_VM
#include "sitevm/sitevm.h"

//Initialization can modify shared state directly in sitevm
#define INIT_TXN_BEGIN() {
#define INIT_TXN_END() }

#define SEQ_MALLOC(s) sitevm::smalloc(s)
#define TM_MALLOC(s) sitevm::smalloc(s)
#define SEQ_FREE(p) sitevm::sfree(p)
#define TM_FREE(p) sitevm::sfree(p)
#define TM_THREAD_ENTER() sitevm::sitevm_enter()
#define TM_STARTUP(x) sitevm::sitevm_init()
#define TM_SHUTDOWN() sitevm::sitevm_shutdown()
#define TM_SYNC() sitevm::sitevm_sync()
#define TM_CALLOC(n, s) sitevm::scalloc(n, s)
//#define malloc(s) sitevm::smalloc(s)
//#define calloc(n, s) sitevm::scalloc(n, s)
//#define free(p) sitevm::sfree(p)

#else

#ifdef DRAFT_EXCITE_VM
#include "teststm.h"

//Initialization cannot modify shared state directly
#if 1
#define INIT_TXN_BEGIN() __transaction_atomic{  
#define INIT_TXN_END()  } 
#else
#define INIT_TXN_BEGIN() {
#define INIT_TXN_END() }
#endif

//teststm::DraftExciteVM<>::flush_logs();

typedef teststm::DefaultSTM MySTM;

extern int* conflictor;

inline void* __calloc_adaptor(size_t size){
    MySTM STM;
    void* toRet = STM.template allocate<char>(size);
    memset(toRet, 0, size);
    return toRet;
}

#define TM_CALLOC(n, s) __calloc_adaptor(n * s) 
#define SEQ_MALLOC(s) MySTM{}.template allocate<char>(s)
//#define SEQ_MALLOC(s) __calloc_adaptor(s) 
#define TM_MALLOC(s) SEQ_MALLOC(s) 

#define SEQ_FREE(p) MySTM{}.free(p)
#define TM_FREE(p) SEQ_FREE(p) 

#define TM_THREAD_ENTER() {MySTM STM; STM.begin(); STM.end(); dune_flush_tlb();} 
#define TM_STARTUP(x) {MySTM::init(); __transaction_atomic {conflictor=(int*)TM_MALLOC(8);} } 
#define TM_SHUTDOWN() {MySTM::deinit();} 
#define TM_SYNC() 

#else

#define INIT_TXN_BEGIN() {
#define INIT_TXN_END() }
#define SEQ_MALLOC(s) malloc(s)
#define TM_MALLOC(s) malloc(s)
#define SEQ_FREE(p) free(p)
#define TM_FREE(p) free(p)
#define TM_THREAD_ENTER() 
#define TM_STARTUP(x) 
#define TM_SHUTDOWN() 
#define TM_SYNC() 
#define TM_CALLOC(n, s) calloc(n, s)

#endif
#endif




/* Indirect function call management */
/* In STAMP applications, it is safe to use transaction_pure */
//#define TM_IFUNC_DECL                 __attribute__((transaction_pure))
#define TM_IFUNC_CALL1(r, f, a1)      r = f(a1)
#define TM_IFUNC_CALL2(r, f, a1, a2)  r = f((a1), (a2))

/* libitm.h stuff */
#ifdef __i386__
# define ITM_REGPARM __attribute__((regparm(2)))
#else
# define ITM_REGPARM
#endif

#if 0
extern
TM_PURE
void _ITM_abortTransaction(int) ITM_REGPARM __attribute__((noreturn));
#endif

/* Additional annotations */
/* strncmp can be defined as a macro (FORTIFY_LEVEL) */
/*
#ifdef strncmp
# undef strncmp
#endif
*/

//extern "C"
#if 0
inline void* /*TM_SAFE*/ memcpy_copy(void* dst, const void* src, size_t n)
{
    for(size_t i = 0; i < n; i++){
        ((char*)dst)[i] = ((const char*)src)[i];
    }
    return dst;
}

#define memcpy memcpy_copy
#endif

//extern "C"
inline int /*TM_SAFE*/ strncmp_copy(const char* s1, const char* s2, size_t n)
{
    //TODO do the "n" part of strncmp
    for(size_t i = 0; i < n; i++){
        if (!*s1){
            break;
        }
        if (*s1 != *s2){
            return *(const unsigned char*)s1-
                    *(const unsigned char*)s2;
        }
        s1++;
        s2++;
    }
    return 0;
}

#define strncmp strncmp_copy

inline int /*TM_SAFE*/ strcmp_copy(const char* s1, const char* s2){
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned
            char*)s2;
}

#define strcmp strcmp_copy

extern "C"
inline size_t TM_SAFE strlen(const char* a)
__THROW 
{
    size_t toRet = 0;
    for(; *a; a++){
        toRet++;
    }
    return toRet;
}

extern "C"
TM_PURE
void __assert_fail (__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
     __THROW __attribute__ ((__noreturn__));

extern "C"
TM_PURE
void __cxa_call_unexpected(void*)
     __THROW __attribute__ ((__noreturn__));


//Do them down here.
//Fix printf
//extern "C" {
TM_PURE int __printf_chk(int, const char*, ...);
TM_PURE int printf(const char*, ...);
TM_PURE int puts(const char*);
TM_PURE int __builtin_puts(const char*);
//TM_PURE std::runtime_error::runtime_error(const string&);
//TM_PURE int fflush(FILE*);
//HACK
inline int nop_fflush(FILE* file){
    return 0;
}
#define fflush nop_fflush

#endif /* TM_H */

