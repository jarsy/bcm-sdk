
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 
    general include files

File Name: 
    ag_macros.h

File Description: 
    contain general purpose macros

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    3/7/00          Gadi Admoni     initial version
    22/1/00         Gadi Admoni     Added AG_ to all macros.
*******************************************************************/
#ifndef AG_MACROS_H 
#define AG_MACROS_H 

#include "ag_types.h"

#ifdef AG_GCC /*__GNUC__ */
	#define AG_INLINE static __inline__
#else /* ADS and NT */
	#define AG_INLINE __inline
#endif

#define AG_ASSERT(_b)   if(!(_b))  AGOS_SWERR(AGOS_SWERR_WARNING,"AG_ASSERT failed: " #_b, 0,0,0,0,0);


/*#ifdef AG_GCC */
/*	#define ag_min(a, b)	(((a) < (b)) ? (a) : (b) ) */
/*	#define ag_max(a, b)	(((a) > (b)) ? (a) : (b) ) */
/*#else */
AG_INLINE AG_U32 ag_min(AG_U32 a, AG_U32 b) {  return ((a < b) ? a : b ); }
AG_INLINE AG_U32 ag_max(AG_U32 a, AG_U32 b) {  return ((a > b) ? a : b ); }
/*#endif */

/* To use the below alignment macros use the following syntax
AG_BYTE_ALIGN
before the structure that needs the byte alignment.
*/
#ifdef AG_MNT
    #define AG_BYTE_ALIGN  
#else /*The ARM cross compiler*/
    #define AG_BYTE_ALIGN   __packed
#endif

/*
 returns an integer with the specified bit set to 1.
 bits are numbered from 0 to 31.
*/
#define AG_BIT(a)      ((AG_U32)1 << (a))



/*
 Calculate the byte offset of a field in a structure of type type.
*/
#define AG_FIELD_OFFSET(type, field)    ((AG_U32)&(((type *)0)->field))


/*
 Calculate the address of the base of the structure given its type, and an
 address of a field within the structure.
*/
#define AG_CONTAINING_RECORD(address, type, field) ((type *)((AG_U8*)(address) - (AG_U8*)(&((type *)0)->field)))

/* Create a 4 character key 
Usage example 
#define TEST_OBJECT_ID	OBJECT_TYPE('T','E','S','T')
*/
#define AG_OBJECT_TYPE(_chA, _chB, _chC, _chD)	((AG_U32)(((AG_U32)_chA<<0)+((AG_U32)_chB<<8)+((AG_U32)_chC<<16)+((AG_U32)_chD<<24)))


/************ General used constants and defines *********************/
/* discard gpc area size: 256 queues * 8 bytes (frm, octet) * 3 (tx-machine, q-manager, scheduler) */
#define AG_DSCRD_GPC_SIZE   6144  

#define AG_128MB	0x8000000
#define AG_64MB		0x4000000
#define AG_32MB		0x2000000
#define AG_16MB		0x1000000
#define AG_10MB		0xA00000
#define AG_8MB		0x800000
#define AG_4MB		0x400000
#define AG_2MB		0x200000
#define AG_1MB		0x100000


#ifdef __cplusplus
extern "C"
{
#endif

extern AG_U32 AGOS_for_unused_params;	/* dummy parameter for AG_UNUSED_PARAM macro */

#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#define AG_UNUSED_PARAM(x)  AGOS_for_unused_params=(AG_U32)(x)

#define AG_DELETE(x) if (x) {delete x;x=NULL;}

#endif  /* AG_MACROS_H */
