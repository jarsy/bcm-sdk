#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#define _TYPEDEFS_H_

/*----------------------- define TRUE, FALSE, NULL, bool ----------------*/
#ifdef __cplusplus

#ifndef FALSE
#define FALSE	false
#endif
#ifndef TRUE
#define TRUE	true
#endif

#else /* !__cplusplus */

#if defined(_WIN32) || defined(__klsi__)

typedef	unsigned char	bool;

#else

#if defined(MACOSX) && defined(KERNEL)
#include <IOKit/IOTypes.h>
#else
typedef	int	bool;
#endif

#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1

#ifndef NULL
#define	NULL 0
#endif

#endif

#endif /* __cplusplus */

/*----------------------- define uchar, ushort, uint, ulong ----------------*/

typedef unsigned char uchar;

#if defined(_WIN32) || defined(PMON) || defined(PSOS) || defined(__klsi__) || defined(__MRC__) || defined(V2_HAL) || defined(_CFE_)

#ifndef V2_HAL
typedef unsigned short	ushort;
#endif

typedef unsigned int	uint;
typedef unsigned long	ulong;

#else

/* pick up ushort & uint from standard types.h */
#if defined(linux) && defined(__KERNEL__)
#include <linux/types.h>	/* sys/types.h and linux/types.h are oil and water */
#else
#include <sys/types.h>	
#if !defined(TARGETENV_sun4) && !defined(linux)
typedef unsigned long	ulong;
#endif /* TARGETENV_sun4 */
#endif
#if defined(vxworks) || defined(PMON)
typedef unsigned int	uint;
typedef unsigned long long       uint64;
#endif

#endif /* WIN32 || PMON || .. */

/*----------------------- define [u]int8/16/32/64 --------------------------*/

#if defined(__klsi__)

typedef signed char	int8;
typedef signed short	int16;
typedef signed int	int32;

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;

typedef struct {
	int32	hi;
	int32	lo;
} int64;

typedef struct {
	uint32	hi;
	uint32	lo;
} uint64;

#else /* !klsi */

#ifdef V2_HAL
#include <bcmos.h>
#else
typedef signed char	int8;
typedef signed short	int16;
typedef signed int	int32;

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
#endif	/* V2_HAL */

typedef float		float32;
typedef double		float64;

/* $Id: typedefs.h,v 1.3 2011/07/21 16:14:55 yshtil Exp $
 * abstracted floating point type allows for compile time selection of
 * single or double precision arithmetic.  Compiling with -DFLOAT32
 * selects single precision; the default is double precision.
 */

#if defined(FLOAT32)
typedef float32 float_t;
#else /* default to double precision floating point */
typedef float64 float_t;
#endif /* FLOAT32 */

#ifdef _MSC_VER	/* Microsoft C */
typedef signed __int64	int64;
typedef unsigned __int64 uint64;

#elif defined(__GNUC__) && !defined(__STRICT_ANSI__) && !defined(vxworks)
/* gcc understands signed/unsigned 64 bit types, but complains in ANSI mode */
typedef signed long long int64;
typedef unsigned long long uint64;

#elif defined(__ICL) && !defined(__STDC__)
/* ICL accepts unsigned 64 bit type only, and complains in ANSI mode */
typedef unsigned long long uint64;

#endif /* _MSC_VER */

#endif /* klsi */

/*----------------------- define PTRSZ, INLINE --------------------------*/

#define	PTRSZ	sizeof (char*)

#ifndef INLINE

#ifdef _MSC_VER

#define INLINE __inline

#elif __GNUC__

#define INLINE __inline__

#else

#define INLINE

#endif /* _MSC_VER */

#endif /* INLINE */

#endif /* _TYPEDEFS_H_ */
