/*
 * $Id: nlmcmbasic.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

/*@@NlmCmBasic Module
   This module contains a basic set of macros, type definitions, etc. used to provide
   a common base upon which some other software can be built.
*/

#ifndef INCLUDED_NLMCMBASIC_H
#define INCLUDED_NLMCMBASIC_H

/*****************************************************/
#ifndef NLMPLATFORM_BCM
#include <nlmcmportable.h>
#include <common/nlmcmcast.h>
#include <nlmcmctype.h>
#include <nlmcmexterncstart.h>      /* this should be the final include */
#else
#include <soc/kbp/common/nlmcmportable.h>
#include <soc/kbp/common/nlmcmcast.h>
#include <soc/kbp/common/nlmcmctype.h>
#include <soc/kbp/common/nlmcmexterncstart.h>      /* this should be the final include */
#endif

/*##
   Routine      allowed     alternative/related
   -------      -------     -------------------
   abort()              NlmCmBasic__Panic
   assert()     core    test    NlmCmBasic__Demand

   malloc()         test
   free()           test

   printf()         test
   fopen()          test
   fclose()         test
   fprintf()            test    NlmCmBasic__WriteStdErr

   strlen()     core    test
   strcpy()     core    test

   <memory.h>       core    test

   rand         DO NOT USE      NlmCmMath__Random
   sqrt         DO NOT USE      NlmCmMath__Sqrt
*/

#define NLMCMMAXLINELEN 4096
typedef nlm_u32 NlmRecordIndex;


/*********************************************************************/

/*## NB: Use Nlm vice NlmCm to define ubiquitous types */

/* Summary
   Simple bit type.

   Description
   Used to document the intent of a hardware bit.
   Rather than create/use NlmBitHIGH NlmBitLOW, just use 0 and 1.

   Note
   The NlmBit datatype is defined as an int;
   therefore, the compiler can not tell if
   it is erroneously assigned a value
   other than 0 (NLMBIT_0) or 1 (NLMBIT_1).

   The size of the datatype is more than just a single bit!
   Use in structs with care.
 */
typedef int NlmBit ;


/* Summary
   Simple 72-bit type.
 */
typedef struct nlm_72
{
    nlm_u8 bits[9];
} nlm_72;


/* Summary
   Advanced bit type.

   Description
   Used to document the state of a hardware bit. Unlike NlmBit, NlmBitZX
   also supports undriven (Z) and unknown (X) values.

   <TABLE>
   Name       Meaning
   --------  ------------------
   NLMBIT_0    bit value is set low
   NLMBIT_1    bit value is set high
   NLMBIT_Z    bit value is not driven
   NLMBIT_X    bit value is unknown
   </TABLE>

   Note
   The NlmBitZX datatype is defined as an int;
   therefore, the compiler can not tell if
   it is erroneously assigned a value
   other than NLMBIT_0, NLMBIT_1, NLMBIT_Z, or NLMBIT_X.

   The size of the datatype is more than just a single bit!
   Use in structs with care.
 */

typedef int NlmBitZX;

#define NLMBIT_0 0
#define NLMBIT_1 1
#define NLMBIT_Z 2
#define NLMBIT_X 3

/* SubSystem definitions */
typedef enum NlmSubSys_t
{
    NLMSS_NONE = 0,         /* MUST be 0 */

    /*## FYI: sort the lines to make it easy to see potential conflicts --
     *        since we recompile, reordering should not matter
     */

    NLMSS_CM,           /* Common base */
    NLMSS_L7SYSTEM,     /* Layer 7 system module */
    NLMSS_COMPILER,     /* Layer 7 compiler error codes */
    NLMSS_RTFGEN,       /* Layer 7 RTF gen tool */
    NLMSS_L7CMODEL,     /* Layer 7 C-Model */
    NLMSS_L7ROCE,       /* Layer 7 New Compiler */
    NLMSS_ZZZ           /* must be last */
} NlmSubSys ;

/* Error code type */
typedef nlm_u32 NlmErrNum_t;
#define NLMCMOK 0

/*********************************************************************/

/*## For development, we need routines like these, so we provide our own, because we
   need to isolate the usage of the underlying implementation. We expect that clients
   may need to replace these with system-specific solutions.
*/

/* Write a message to standard error */
extern void NlmCmBasic__WriteStdErr(const char* msg) ;

/* Flush the stdout and stderr output channels */
extern void NlmCmBasic__FlushOutErr(void) ;

/* Print panic message and abort program */
extern void NlmCmBasic__Panic(const char* msg) ;

/* Summary
   Define our own version of assert().

   Description
   Replace the standard library assert() with our own NlmCmAssert()
   to enable error box suppression on Windows.
*/
/*======== Note ======= */
/* Following two lines were commented out to let user use system defined
 * assert. If you want NetLogic provided assert, please uncomment the 2 lines
 */
/*
#undef assert
#define assert(b) NlmCmAssert(0 != (b), #b)
*/

/* Summary
   Like assert(), except it always checks its condition.

   Description
   Evaluate the given boolean condition. If false, cause the program to halt.
   Condition is evaluated in debug and release builds.
 */
#define NlmCmBasic__Require(b) NlmCmBasic__Demand((b), #b)

/* Summary
   Like assert, except it always checks its condition and prints the given
   message on failure.

   Description
   Evaluate the given boolean condition. If false, print the given message and
   cause the program to halt. Condition is evaluated in debug and release
   builds.
 */
#define NlmCmBasic__Demand(b, m) NlmCmBasic__DemandFileLine((b), (m), __FILE__, __LINE__)

/* Summary
   Like assert(), except NlmCmBasic__Failed always fails with the given message,
   with a large ERROR banner if the first message character is not "@".

   No boolean condition. Print the given message.
   Returns 0, so it may be used in an expression.
 */
#define NlmCmBasic__Failed(m) NlmCmBasic__FailedFileLine((m), __FILE__, __LINE__)

/* Summary
   Generate an error if the specified condition is not true.
   Returns cond, so the call can be used in an expression.
 */
#define NlmCmBasic__FailedIfNot(cond) ( (cond) ? 1 : NlmCmBasic__Failed("@" #cond) )

/* Summary
   Always generates a warning with the given message,
   but unlike NlmCmBasic__Failed(), it always returns.

   Description

   Does not use a boolean condition. Prints the given message
   as a warning with a big banner warning and returns to the caller.
   Returns 0, so the routine can be used in an expression.

   If the messages starts with "@" (like the convention in makefiles),
   then the warning is a small warning -- the banner is omitted.
   If the messages starts with "-" (like the convention in makefiles),
   then the warning is an ignored warning -- the banner is printed,
   but the "warning" string is modified to IgnoreWarning.
 */
#define NlmCmBasic__Warning(m) NlmCmBasic__WarningFileLine((m), __FILE__, __LINE__)

/* Summary
   Generate a warning if the specified condition is not true.
   Returns cond, so the call can be used in an expression.
 */
#define NlmCmBasic__WarnIfNot(cond) ( (cond) ? 1 : NlmCmBasic__Warning("@Failed: " #cond) )

/* Summary
   Generate a note that the specified activity
   is still TBD (To Be Done) -- that is, it is not yet (completely) implemented.
   Returns 0, so the call can be used in an expression.
 */
#define NlmCmBasic__TBD(m) NlmCmBasic__Note("--- TO BE DONE --- " m)

/* Summary
   Generate a note (not a warning) -- useful for noteworthy remarks
   that shouldn't pollute the logs as warnings.
   Returns 0, so the call can be used in an expression.
 */
extern int NlmCmBasic__Note(const char* msg) ;

/* Summary
   Generate a note if the specified condition is not true.
   Returns cond, so the call can be used in an expression.
 */
#define NlmCmBasic__NoteIfNot(cond) ( (cond) ? 1 : NlmCmBasic__Note("@Failed: " #cond) )

/* Summary
   A shorthand for NlmCmBasic__Require
*/
#define NlmCmRequire(b) NlmCmBasic__Require((b))

/* Summary
   A shorthand for NlmCmBasic__Demand
*/
#define NlmCmDemand(b, m) NlmCmBasic__Demand((b), (m))

/* Summary
   A shorthand for NlmCmBasic__Die
*/
#define NlmCmDie(m) NlmCmBasic__Die((m))

/* Summary
   Emits a big error message (or a small one if the first character is "@"),
   and halts the program.
 */
#define NlmCmBasic__Die(m) NlmCmBasic__DieFileLine((m), __FILE__, __LINE__)

/* Summary
   Like assert(), except NlmCmAssert prints the given message on failure

   Description
   Evaluate the given boolean condition. If false, print the given message and
   cause the program to halt in debug builds. Compiled out in release builds.
 */
#define NlmCmAssert(b, msg)

/* Summary
   Like assert(), except NlmCmAssert_ prints the given message on failure and is
   designed for use in C's comma expressions.

   Description
   For use in a <B>comma separated list</B>. It returns a value and expands
   with a trailing comma, so always enclose in parenthesis. NlmCmAssert_
   "protectes" the condition argument from evaluation to avoid the following
   warning from gcc:

   <CODE>
       "warning: left-hand operand of comma expression has no effect"
   </CODE>

   Examples
   <CODE>
       return  (NlmCmAssert_(result > 0, "result is positive") result) ;
       value = (NlmCmAssert_(result > 0, "result is positive") result) ;
   </CODE>
 */
#define NlmCmAssert_(cond, msg)

/*##
   The above definitions for NlmCmAssert and NlmCmAssert_ were for
   documentation purposes only.
 */
#undef  NlmCmAssert
#undef  NlmCmAssert_

#ifdef NDEBUG
#define NlmCmAssert(b, m) ((void)0)
#else
#define NlmCmAssert(b, m) ((void)((b) || NlmCmBasic__Die(m)))
#endif

#ifdef NDEBUG
#define NlmCmAssert_(b, m)
#else
#define NlmCmAssert_(b, m) ( NlmCmBasic__Identity(b) ? 0 : NlmCmBasic__Die(m) ),
#endif

/* Storing assert info (NKG)*/
#ifdef NDEBUG
#define NlmCmSimpleAssert(id, b, m) ((void)((b) || NlmNsLog__StoreAssertInfo(id)))
#else
#define NlmCmSimpleAssert(id, b, m) ((void)((b) || NlmNsLog__StoreAssertInfo(id) || NlmCmBasic__Die(m)))
#endif

/*##
   FYI: gcc on WinDoze warns about generated assert(1) having no effect,
        as might be generated by __Require() or __Demand();
        so we hide the potential constant from the compiler via NlmCmBasic__Identity().
*/

/* Summary
   Return the given boolean value.

   Description
   Some platforms/compilers, assert(1) generates a warning. Use of the
   identity function hides the fact that the condition is a constant.
 */
extern NlmBool NlmCmBasic__Identity(const NlmBool b) ;

/* Summary
   Provide a better named routine for NlmCmBasic__Identity
*/
#define NlmCmBool__Identity NlmCmBasic__Identity

/* Summary
   Return the given uintvs_t value.

   Description
   Used to ensure type safety in macros.
*/
extern uintvs_t
NlmCmUintVS__Identity(uintvs_t value) ;

/* Summary
   Return the given character pointer.

   Description
   Used to ensure type-safety in macros.
*/
extern char*
NlmCmCharStar__Identity(char* value) ;

extern const char*
NlmCmConstCharStar__Identity(const char* value) ;



/* Summary
   Return the given nlm_u8* value.

   Description
   Used to ensure type safety in macros.
*/
extern const nlm_u8*
NlmCmUint8Star__Identity(const nlm_u8* value) ;


/* Summary
   Return the given int value.

   Description
   Used to ensure type safety in macros.
*/
extern int
NlmCmInt__Identity(int value) ;

/* Summary
   Return the given unsigned int value.

   Description
   Used to ensure type safety in macros.
*/
extern unsigned int
NlmCmUint__Identity(unsigned int value) ;

/* Summary
   Return the given nlm_8 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_8
NlmCmInt8__Identity(nlm_8 value) ;

/* Summary
   Return the given nlm_u8 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_u8
NlmCmUint8__Identity(nlm_u8 value) ;


/* Summary
   Return the given nlm_16 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_16
NlmCmInt16__Identity(nlm_16 value) ;

/* Summary
   Return the given nlm_u16 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_u16
NlmCmUint16__Identity(nlm_u16 value) ;

/* Summary
   Return the given nlm_32 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_32
NlmCmInt32__Identity(nlm_32 value) ;

/* Summary
   Return the given nlm_u32 value.

   Description
   Used to ensure type safety in macros.
*/
extern nlm_u32
NlmCmUint32__Identity(nlm_u32 value) ;

/* Summary
   Return the given void * value.

   Description
   Used to ensure type safety in macros.
*/
extern void*
NlmCmVoidStar__Identity(void *value) ;


/* Summary
   Zero out the region, and return it.
*/
extern nlm_u8*
NlmCmBasic__Zero(void *data_area, size_t data_len) ;

/* Summary
   Determine if a region is entirely zero.
*/
extern NlmBool
NlmCmBasic__IsZero(const void *data_area, size_t data_len) ;

/* Summary
   Copy a sequence of bytes from src to dst with zero fill

   Description
   Copy srclen bytes from src into dst. If srclen < dstlen, then trailing
   bytes in dst are zeroed out.
*/
extern void
NlmCmBasic__CopyZeroFill(
    nlm_u8*         dst,        /* destination */
    int             dstSize,    /* destination length, in bytes */
    const nlm_u8*   src,        /* source */
    int             srcSize) ;  /* source length, in bytes */

#ifndef __doxygen__

/*  Summary
    Create a string containing the value of NLMCONFIG

    Parameters
    @param prefix the prefix string
    @param suffix the suffix string

    Returns
    the concatenation of prefix, NLMCONFIG (or .), and suffix
*/
#if defined(NLMCONFIG)
#define NlmConfigString(prefix, suffix) (prefix NLMCONFIG suffix)
#else
#define NlmConfigString(prefix, suffix) (prefix "."      suffix)
#endif

/* compile out the given code in release mode. */
#ifdef NDEBUG
#define NlmCmDbgOnly(i)
#else
#define NlmCmDbgOnly(i) (i)
#endif

/* compile out the given code of comma separated list in release mode */
#ifdef NDEBUG
#define NlmCmDbgOnly_(i)
#else
#define NlmCmDbgOnly_(i) (i),
#endif

/* compile IN the given code in release mode. */
#ifndef NDEBUG
#define NlmCmNdbOnly(i)
#else
#define NlmCmNdbOnly(i) (i)
#endif

/* compile IN the given code of comma separated list in release mode */
#ifndef NDEBUG
#define NlmCmNdbOnly_(i)
#else
#define NlmCmNdbOnly_(i) (i),
#endif

#endif /* __doxygen__ */


#ifndef __doxygen__
/*## safe shifting -- assuming a 32bit architecture, the behavior of shifts 
     of >= 32 or < 0 are undefined. 
     
     These define "safe" shift macros that check the shift amount in
     debug builds, but evaluate to a plain C shift in release builds.
*/
#define NlmCmShiftLS(val, amnt) NlmCmBasic__ShiftLeftSigned((val), (amnt))
#define NlmCmShiftLU(val, amnt) NlmCmBasic__ShiftLeftUnsigned((val), (amnt))
#define NlmCmBasic__ShiftLeftUnsigned(val, amnt) \
    ((nlm_u32)NlmCmBasic__ShiftLeftSigned((val), (amnt)))
#define NlmCmBasic__ShiftLeftSigned(val, amnt) \
    ( \
    NlmCmAssertStable_(val) \
    NlmCmAssertStable_(amnt) \
    NlmCmAssert_(!(((nlm_32)(amnt)) & 0xFFFFFFE0), "shift ammount out of range") \
    ((nlm_32)(val) << (amnt)) \
    )

#define NlmCmShiftRU(val, amnt) NlmCmBasic__ShiftRightUnsigned((val), (amnt))
#define NlmCmBasic__ShiftRightUnsigned(val, amnt) \
    ( \
    NlmCmAssert_(sizeof(int) == 4, "NlmCmBasic__ShiftRight assumes 32bit arch") \
    NlmCmAssertStable_(val) \
    NlmCmAssertStable_(amnt) \
    NlmCmAssert_(!(((nlm_u32)(amnt)) & 0xFFFFFFE0), "shift ammount out of range") \
    ((nlm_u32)(val) >> (amnt)) \
    )

#define NlmCmShiftRS(val, amnt) NlmCmBasic__ShiftRightSigned((val), (amnt))
#define NlmCmBasic__ShiftRightSigned(val, amnt) \
    ( \
    NlmCmAssertStable_(val) \
    NlmCmAssertStable_(amnt) \
    NlmCmAssert_(!(((nlm_32)(amnt)) & 0xFFFFFFE0), "shift ammount out of range") \
    ((nlm_32)(val) >> (amnt)) \
    )

#endif /* __doxygen__ */

#ifndef __doxygen__
/*##
  internal assertion related routines
  returns an int so that can be used in an expression
 */
extern int
NlmCmBasic__DemandFileLine(
    NlmBool invarient_condition,
    const char* invarient_msg,
    const char* file,
    int line) ;
extern int
NlmCmBasic__DieFileLine(
    const char* msg,
    const char* file,
    int line) ;
extern int
NlmCmBasic__FailedFileLine(
    const char* msg,
    const char* file,
    int line);
extern int
NlmCmBasic__WarningFileLine(
    const char* msg,
    const char* file,
    int line);
#endif /* __doxygen__ */

/*********************************************************************/

/*## Macros used to enable safe type casting for "derived" objects */

#ifndef __doxygen__
#ifdef NDEBUG

#define NlmCmDECLARE_identity(Visible, Object)
#define NlmCmIMPLEMENT_identity(Visible, Object)

#define NlmCmDECLARE_cast(Object, Base)
#define NlmCmIMPLEMENT_cast(Object, Base)
#define NlmCmUSE_cast(Object, Base, ptr)        ((Base*)NLM_SILENCE_TYPE_PUNNING(ptr))

#define NlmCmDECLARE_constcast(Object)
#define NlmCmIMPLEMENT_constcast(Object)
#define NlmCmUSE_constcast(Object, ptr)             ((Object*)NLM_SILENCE_TYPE_PUNNING(ptr))

#define NlmCmDECLARE_castup(Object, Base, ObjName)
#define NlmCmIMPLEMENT_castup(Object, Base, ObjName)
#define NlmCmUSE_castup(Object, Base, ptr)      ((Object*)NLM_SILENCE_TYPE_PUNNING(ptr))

#else

/* FYI:keep these defines aligned, one per line, to make visual comparisons easier */

/* FYI: Identity is not extern -- you need to declare Visible as extern or static */
#define NlmCmDECLARE_identity(Visible, Object)   Visible const Object* Object##__Identity(const Object *x) ;
#define NlmCmIMPLEMENT_identity(Visible, Object) Visible const Object* Object##__Identity(const Object *x) {return x;}

#define NlmCmDECLARE_cast(Object, Base)   extern Base*      Object##__cast##Base##_body(const Object *x) ;
#define NlmCmIMPLEMENT_cast(Object, Base)   Base*   Object##__cast##Base##_body(const Object *x) {return (Base*)x;}
#define NlmCmUSE_cast(Object, Base, ptr)            Object##__cast##Base##_body(ptr)

#define NlmCmDECLARE_constcast(Object)    extern Object*    Object##__constcast_body(const Object *x) ;
#define NlmCmIMPLEMENT_constcast(Object)        Object* Object##__constcast_body(const Object *x) {return (Object*)x;}
#define NlmCmUSE_constcast(Object, ptr)             Object##__constcast_body(ptr)

#define NlmCmDECLARE_castup(Object, Base, ObjName) extern NlmBool Base##__Is##Object(const Base* ptr) ;
#define NlmCmIMPLEMENT_castup(Object, Base, ObjName)        NlmBool Base##__Is##Object(const Base* ptr) \
                               { return (ptr) ? NlmCmBasic__IsStrEq(ObjName, (ptr)->m_vtbl_p->className) : 1; }
/*## FYI: make it more useful -- assert occurs inline (at the call site), not buried in a utility routine */
#define NlmCmUSE_castup(Object, Base, ptr) \
( NlmCmAssert_((ptr) == (ptr), "stable ptr") \
  NlmCmAssert_(Base##__Is##Object(ptr), "valid " #Object) \
  (Object*)(ptr) )

#endif /* NDEBUG */

/* FYI: need our own version (for typesafe castup) to avoid include of nlmcmstring.h */
extern NlmBool
NlmCmBasic__IsStrEq(const char* s, const char* t) ;

#endif /* __doxygen__ */

/* Given pointer to struct, return pointer to an internal field */

#define NlmCmFIELD_cast(Out, In, field, ptr) \
    ( NlmCmAssert_(0 != Out##__Identity(ptr),"type and null safe") &(ptr)->field )

/* Given pointer to internal field, return pointer to containing struct */

#define NlmCmSTRUCT_cast(Out, In, field, ptr) \
    ( NlmCmAssert_(0 != In##__Identity(ptr), "type and null safe") \
      (Out*) ( (uintvs_t)(ptr) -(uintvs_t)(&((Out*)0)->field) ) )

/*********************************************************************/

/* convert nBits into the number of bytes required to store them */
#define NLMCM_BITS_TO_BYTES(nBits) (((nBits) + 7) >> 3)
    
/*********************************************************************/

/* Evaluate to the minimum of the two parameters */
#define NLMCMMIN(X, Y) ( NlmCmAssert_((X)==(X), "Side effect in macro parm") NlmCmAssert_((Y)==(Y), "Side effect in macro parm") (X) <= (Y) ? (X) : (Y) )

/* Evaluate to the maximum of the two parameters */
#define NLMCMMAX(X, Y) ( NlmCmAssert_((X)==(X), "Side effect in macro parm") NlmCmAssert_((Y)==(Y), "Side effect in macro parm") (X) >= (Y) ? (X) : (Y) )

/**********************************************************************/

#ifndef __doxygen__
void NlmCmBasic__Verify(void);
#endif /* __doxygen__ */

/* NLMSTSTATIC
 *
 * Use the following to declare automatic variables on the stack that
 * are big but are in a performance critical code path. In
 * multi-threaded mode, we want a real automatic definition but in
 * single threaded mode, we will get a static definition of the
 * variable. Note that using too many of these is NOT a good idea
 * because each defnition will cause the final image to bloat. Use thse
 * very sparingly where no other recourse is possible. Ideally, you need
 * to malloc the storage and delete it when you are done. Not only is
 * dynamically allocating thread safe but keeps the final executable
 * image size manageable
 */

#ifdef NLMMT
#define NLMSTSTATIC
#else
#define NLMSTSTATIC static
#endif

/**********************************************************************/

/* Macro to avoid type-punning warnings (as seen in GCC 3.3). Strictly
 * speaking, a memory location is not allowed to be aliased by two variables
 * of sufficently different types. E.g., unsigned integers are allowed to
 * alias a signed integer. Structs are not allowed to alias other structs.
 * Based on what we are seeing from GCC 3.3, 'const void*' is not allowed to
 * alias a structure. This causes a number of warnings when -fstrict-aliasing
 * is enabled (-O2 does enable strict aliasing in newer GCCs).
 *
 * There are two ways to work around the issue.
 *
 * 1. Unions are designed precicely for this purpose. However, retro fitting
 * unions would break a number of established APIs.
 *
 * 2. char* is explicitly allowed to alias any other type.
 *
 * We take solution #2. We wrap casts or function calls which trigger the
 * type-punning warning with this macro. It prevents the warning from being
 * triggered.
 *
 * CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION
 * 
 * Make sure you know what you are doing. The compiler generates these
 * warnings for good reason. Aliasing can be dangerous. E.g. char, int, and
 * structs all have different alignment requirements. By silencing the
 * compiler, we need to be extra diligent to ensure we do not inadvertently
 * write code which will crash.
 */

#define NLM_SILENCE_TYPE_PUNNING(expr) ((char*)(expr))

/**********************************************************************/






#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif


