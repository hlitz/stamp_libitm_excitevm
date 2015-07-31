#ifndef TM_H
#define TM_H 1

#include <stdlib.h>                   /* Defines size_t. */
#include "sitevm/sitevm.h"

#define TM_PURE                       __attribute__((transaction_pure))
#define TM_SAFE                       __attribute__((transaction_safe))

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
#define SEQ_MALLOC(s) sitevm::smalloc(s)
#define TM_MALLOC(s) sitevm::smalloc(s)
#define SEQ_FREE(p) sitevm::sfree(p)
#define TM_FREE(p) sitevm::sfree(p)
#define TM_THREAD_ENTER()sitevm::sitevm_enter();  TM_SYNC(); 
#define TM_THREAD_EXIT()sitevm::sitevm_exit();  
#define TM_STARTUP(x) sitevm::sitevm_init()
#define TM_SHUTDOWN() sitevm::sitevm_shutdown()
#define TM_SYNC() 
//sitevm::sitevm_sync()
#define TM_CALLOC(n, s) sitevm::scalloc(n, s)
//#define malloc(s) sitevm::smalloc(s)
//#define calloc(n, s) sitevm::scalloc(n, s)
//#define free(p) sitevm::sfree(p)
//#define malloc(s) sitevm::smalloc(s); sitevm::TMprint_addr()
//#define calloc(n, s) sitevm::scalloc(n, s); sitevm::TMprint_addr()
#define free(p) /*sitevm::TMprintf(); free(p)*/


#else
#define SEQ_MALLOC(s) malloc(s)
#define TM_MALLOC(s) malloc(s)
#define SEQ_FREE(p) free(p)
#define TM_FREE(p) free(p)
#define TM_THREAD_ENTER()
#define TM_THREAD_EXIT()
#define TM_STARTUP(x) 
#define TM_SHUTDOWN() 
#define TM_SYNC() 
#define TM_CALLOC(n, s) calloc(n, s)
#define free(p)
#endif

#include <stdio.h>      /* printf */
#include <stdarg.h>
__attribute__((transaction_pure))
inline void Assert(bool expression, const char *fmt, ...) {
  if (expression) return;

  va_list list;
  va_start(list, fmt);
  vfprintf (stderr, fmt, list);
  va_end(list);
}



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
#ifdef strncmp
# undef strncmp
#endif
extern
TM_PURE
int strncmp (__const char *__s1, __const char *__s2, size_t __n);

extern
TM_PURE
void __assert_fail (__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
     __attribute__ ((__noreturn__));


#endif /* TM_H */

