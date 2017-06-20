/* bootInit.c - ROM initialization module */

/* $Id: bootInit.c,v 1.2 2011/07/21 16:14:16 yshtil Exp $
 * Copyright (c) 1989-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
04t,26jul05,rec  coldfire support
04s,06sep05,h_k  replaced compressedEntry with sysInit and usrEntry in a
                 comment. (SPR #111954)
04r,19apr05,h_k  force to call fillLongs() resides in ROM for compressed
                 image. (SPR #108169)
04q,24feb05,rec  ARM fillLongs should not be volatile - SPR 106411
04p,04oct04,mdo  Documentation fixes for apigen
04o,16sep04,j_b  correct RAM clearing (SPR 99727)
04n,18aug04,j_b  PM_RESERVED_MEM is dependent on INCLUDE_EDR_PM
04m,11may04,j_b  add support for PM_RESERVED_MEM
04l,26mar02,sbs  correcting data section relocation for ROM compressed images
                 (SPR 73785)
04k,19mar02,dat  Removing previous fix for MIPS, no longer needed (72825)
04j,18jan02,dat  Don't use loop unrolling for MIPS
04i,22jan02,scm  Xscale specific validation added to update cache (to match
                 updates...)
04h,19dec01,aeg  prevented pc-relative addressing on MC680X0.
04g,10dec01,sbs  Corrected conditions around call to copyLongs for ROM
                 resident images.
04f,27nov01,sbs  Added new labels, wrs_kernel_data_start and wrs_kernel_data_end
                 for sdata and edata respectively. Added second copy for data
                 section of compressed ROM images. Corrected definition for
                 binArrayEnd. Corrected definition for RESIDENT_DATA.
04e,27nov01,tpw  Manually unroll copyLongs and fillLongs.
04d,25oct01,pad  Removed definitions of _binArrayStart and _binArrayEnd, now
                 unnecessary.
04c,03oct01,dee  Merge from ColdFire T2.1.0 release
04b,11jan01,scm  Xscale specific validation added to update cache, can not assume uncached area....
04a,10jun99,jpd  fix error when BOOTCODE_IN_RAM defined (SPR #27775).
03z,13nov98,cdp  make Thumb support for ARM CPUs dependent on ARM_THUMB.
03y,10feb99,db   Fix to ensure that default bootline gets copied for
                 standalone and rom-resident images(SPR #21763).
03x,05oct98,jmp  doc: cleanup.
03w,17apr98,cdp  backed out 04t and made absEntry volatile for ARM.
03v,16apr98,cdp  for ARM, make UNCOMPRESS entry point in RAM.
03u,20mar98,cdp  make ROM_COPY_SIZE subject to #ifndef.
03t,11nov97,cdp  ARM7TDMI_T: force romStart to call entry point in Thumb state.
                 (SPR# 9716)
03s,14jul97,tam  changed remaining references to bfillLong to fillLong.
03r,12feb97,dat  Added USER_RESERVED_MEM, SYS_MEM_TOP, SYS_MEM_BOTTOM, SPR 8030
03q,04feb97,ms   fixed compiler warning about protoype for bcopyLongs.
03p,28nov96,cdp  added ARM support.
03o,03sep96,hdn  added the compression support for pc[34]86 BSP.
03n,19aug96,ms   added UNCMP_RTN macro to use inflate instead of uncompress
03m,21jun96,jmb  long modhist -- deleted entries prior to 1994.  SPR #6528
03l,10jun96,tam  added rom resident support for PPC architecture.
03k,14may96,dat  fixed compiler warnings for copyLongs, fillLongs. SPR #6536
03j,06mar96,tpr  changed absEntry to be volatile for PowerPC.
03i,22aug95,hdn  added support for I80X86.
03h,14mar95,caf  restored mips resident rom support (SPR #3856).
03g,16feb95,jdi  doc format change.
03f,23may95,yao  define binArrayStart and binArrayEnd for PowerPC
                 because tools don't prepend "_".
03e,09dec94,caf  undid mod 03a, use sdata for resident roms (SPR #3856).
03d,22jun94,caf  undid 16-byte alignment portion of mod 03c, below.
03c,14jun94,cd   corrected definitions of etext, edata and end.
           +caf  for R4000 resident ROMs: data starts on 16-byte boundary.
                 for R4000 uncompressed ROMs: added volatile to absEntry type.
*/

/*
DESCRIPTION
This module provides a generic boot ROM facility.  The target-specific
romInit.s module performs the minimal preliminary board initialization and
then jumps to the C routine romStart().  This routine, still executing out
of ROM, copies the first stage of the startup code to a RAM address and
jumps to it.  The next stage clears memory and then uncompresses the
remainder of ROM into the final VxWorks ROM image in RAM.

A modified version of the Public Domain \f3zlib\fP library is used to
uncompress the VxWorks boot ROM executable linked with it.  Compressing
object code typically achieves over 55% compression, permitting much
larger systems to be burned into ROM.  The only expense is the added few
seconds delay while the first two stages complete.

ROM AND RAM MEMORY LAYOUT
Example memory layout for a 1-megabyte board:
\cs
    --------------  0x00100000 = LOCAL_MEM_SIZE = sysMemTop()
    |            |
    |    RAM     |
    |  0 filled  |
    |            |
    |------------| = (romInit+ROM_COPY_SIZE) or binArrayStart
    | ROM image  |
    |----------- |  0x00090000  = RAM_HIGH_ADRS
    | STACK_SAVE |
    |------------|
    |            |  0x00080000  = 0.5 Megabytes
    |            |
    |            |
    | 0 filled   |
    |            |
    |            |  0x00001000  = RAM_ADRS & RAM_LOW_ADRS
    |            |
    |            |  exc vectors, bp anchor, exc msg, bootline
    |            |
    |            |
    --------------  0x00000000  = LOCAL_MEM_LOCAL_ADRS
\ce
\cs
    --------------
    |    ROM     |
    |            |  0xff8xxxxx  = binArrayStart
    |            |
    |            |  0xff800008  = ROM_TEXT_ADRS
    --------------  0xff800000  = ROM_BASE_ADRS
\ce

SEE ALSO:
inflate(), romInit(), and deflate

AUTHOR
The original compression software for zlib was written by Jean-loup Gailly
and Mark Adler. See the manual pages of inflate and deflate for
more information on their freely available compression software.
*/

#include "vxWorks.h"
#include "sysLib.h"
#include "config.h"
#include "errno.h"
#include "sioLib.h"

#define UNCMP_RTN   inflate


#ifndef USER_RESERVED_MEM
#   define USER_RESERVED_MEM 0
#endif

/*
 * If memory is to be cleared, it will be cleared from SYS_MEM_BOTTOM
 * up to (but not including) SYS_MEM_TOP, except for text and data segments.
 * The user reserved area is not cleared.
 */

#ifdef  INCLUDE_EDR_PM
#    define SYS_MEM_TOP \
        (LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE - USER_RESERVED_MEM \
         - PM_RESERVED_MEM)
#else
#    define SYS_MEM_TOP \
        (LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE - USER_RESERVED_MEM)
#endif

#define SYS_MEM_BOTTOM \
    (LOCAL_MEM_LOCAL_ADRS + RESERVED)

#define BINARRAYEND_ROUNDOFF    (ROUND_DOWN(binArrayEnd, sizeof(long)))

IMPORT void romInit ();
IMPORT STATUS   UNCMP_RTN ();
IMPORT void usrInit ();
IMPORT void sysInitAlt ();
IMPORT void start ();

IMPORT UCHAR    binArrayStart [];   /* compressed binary image */
IMPORT UCHAR    binArrayEnd [];     /* end of compressed binary image */
IMPORT char etext [];       /* defined by the loader */
IMPORT char end [];         /* defined by the loader */
IMPORT UCHAR    wrs_kernel_data_start [];  /* defined by the loader */
IMPORT UCHAR    wrs_kernel_data_end [];    /* defined by the loader */

#if ((CPU_FAMILY == MIPS) || (CPU_FAMILY==PPC) || \
     (CPU_FAMILY==COLDFIRE))
#  define RESIDENT_DATA   RAM_DST_ADRS
#else
#  define RESIDENT_DATA   wrs_kernel_data_start
#endif

#ifndef RAM_DST_ADRS                    /* default uncompress dest. */
#  define RAM_DST_ADRS        RAM_HIGH_ADRS
#endif

/* If the boot code is in RAM and the RAM is already initialized,
 * clearing the RAM is not necessary.  Macro BOOTCODE_IN_RAM is
 * used not to clear the RAM.
 */
#ifdef  BOOTCODE_IN_RAM         /* not to clear RAM */
#  undef  ROM_TEXT_ADRS
#  undef  ROM_BASE_ADRS
#  define ROM_TEXT_ADRS   ((UINT)romInit)
#  define ROM_BASE_ADRS   ((UINT)romInit)
#endif  /* BOOTCODE_IN_RAM */

/* #if  defined (UNCOMPRESS) */
#ifndef ROM_COPY_SIZE
#  define ROM_COPY_SIZE   ((UINT)end - (UINT)romInit)
#endif
/* #endif*/ /* UNCOMPRESS */

#define ROM_OFFSET(adr) (((UINT)adr - (UINT)romInit) + ROM_TEXT_ADRS)

#if !defined (UNCOMPRESS) && !defined (ROM_RESIDENT)
# define LD_IMAGE_END  ((UINT)RAM_DST_ADRS + \
               (UINT)(ROUND_UP(binArrayEnd - binArrayStart, sizeof(long))))
#endif /* !defined (UNCOMPRESS) && !defined (ROM_RESIDENT) */

/* forward declarations */

LOCAL void copyLongs (FAST UINT *source, FAST UINT *destination, UINT nlongs);
#ifndef BOOTCODE_IN_RAM
LOCAL void fillLongs (FAST UINT *buf, UINT nlongs, FAST UINT val);
#endif  /* BOOTCODE_IN_RAM */
#if (CPU==XSCALE)
int checkLongs (FAST UINT *source, FAST UINT *destination, UINT nlongs);
#endif

/*******************************************************************************
*
* romStart - generic ROM initialization
*
* This is the first C code executed after reset.
*
* This routine is called by the assembly start-up code in romInit().
* It clears memory, copies ROM to RAM, and possibly invokes the uncompressor.
* It then jumps to the entry point of the uncompressed object code.
*
* RETURNS: N/A
*
* ERRNO
*/

void romStart
    (
    FAST int startType      /* start type */
    )

    {
#if ((CPU_FAMILY==SPARC) || (CPU_FAMILY==MIPS) || (CPU_FAMILY==I80X86) || \
     (CPU_FAMILY==PPC) || (CPU_FAMILY==ARM) || (CPU_FAMILY==COLDFIRE))
    volatile            /* to force absolute addressing */
#endif /* (CPU_FAMILY==SPARC) */
    FUNCPTR absEntry;       /* to avoid PC Relative Jump Subroutine */
#if (CPU_FAMILY==MC680X0) && !defined(ROM_RESIDENT) && !defined(BOOTCODE_IN_RAM)
    volatile VOIDFUNCPTR romcopyLongs = &copyLongs;  /* force call to ROM */
# define copyLongs romcopyLongs
#endif /* (CPU_FAMILY==MC680X0) */
    /*
     * Copy from ROM to RAM, minus the compressed image
     * if compressed boot ROM which relies on binArray
     * appearing last in DATA segment.
     */

#ifdef ROM_RESIDENT
    /* If ROM resident code, then copy only data segment
     * from ROM to RAM, initialize memory and jump
     * to usrInit.
     */

# if  (CPU_FAMILY == SPARC)
    copyLongs ((UINT *)(etext + 8), (UINT *) RESIDENT_DATA,
# else
    copyLongs ((UINT *)etext, (UINT *) RESIDENT_DATA,
# endif
        ((UINT) wrs_kernel_data_end - (UINT) RESIDENT_DATA) / sizeof (long));

#else   /* ROM_RESIDENT */

# ifdef UNCOMPRESS

#  if (CPU_FAMILY == MIPS)
    /*
     * copy text to uncached locations to avoid problems with
     * copy back caches
     */
    ((FUNCPTR)ROM_OFFSET(copyLongs)) (ROM_TEXT_ADRS, (UINT)K0_TO_K1(romInit),
        ROM_COPY_SIZE / sizeof (long));
#  else   /* CPU_FAMILY == MIPS */
    ((FUNCPTR)ROM_OFFSET(copyLongs)) (ROM_TEXT_ADRS, (UINT)romInit,
        ROM_COPY_SIZE / sizeof (long));
#  endif  /* CPU_FAMILY == MIPS */

# else   /* UNCOMPRESS */

#  if (CPU_FAMILY == MIPS)
    /*
     * copy text to uncached locations to avoid problems with
     * copy back caches
     * copy the entire data segment because there is no way to ensure that
     * binArray is the last thing in the data segment because of GP relative
     * addressing
     */
    ((FUNCPTR)ROM_OFFSET(copyLongs)) (ROM_TEXT_ADRS, (UINT)K0_TO_K1(romInit),
        ((UINT)wrs_kernel_data_end - (UINT)romInit) / sizeof (long));
#  else   /* CPU_FAMILY == MIPS */
    ((FUNCPTR)ROM_OFFSET(copyLongs)) (ROM_TEXT_ADRS, (UINT)romInit,
        ((UINT)binArrayStart - (UINT)romInit)/ sizeof (long));

    ((FUNCPTR)ROM_OFFSET(copyLongs))
           ((UINT *)((UINT)ROM_TEXT_ADRS + ((UINT)BINARRAYEND_ROUNDOFF -
           (UINT)romInit)), (UINT *)BINARRAYEND_ROUNDOFF,
           ((UINT)wrs_kernel_data_end - (UINT)binArrayEnd) / sizeof (long));

#   if (CPU==XSCALE)
    /* validate coherence, can not assume uncached area... */
    ((FUNCPTR)ROM_OFFSET(checkLongs))
                 (ROM_TEXT_ADRS, (UINT)romInit,
                 ((UINT)binArrayStart - (UINT)romInit) / sizeof (long));

    ((FUNCPTR)ROM_OFFSET(checkLongs))
           ((UINT *)((UINT)ROM_TEXT_ADRS + ((UINT)BINARRAYEND_ROUNDOFF -
            (UINT)romInit)), (UINT *)BINARRAYEND_ROUNDOFF,
            ((UINT)wrs_kernel_data_end - (UINT)binArrayEnd) / sizeof (long));
#   endif /* CPU==XSCALE */
#  endif  /* CPU_FAMILY == MIPS */

# endif  /* UNCOMPRESS */
#endif  /* ROM_RESIDENT */


#if (CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM))

    /* clear all memory if cold booting */

    if (startType & BOOT_CLEAR)
    {
# ifdef ROM_RESIDENT
    /* Clear memory not loaded with text & data.
     *
     * We are careful about initializing all memory (except
     * STACK_SAVE bytes) due to parity error generation (on
     * some hardware) at a later stage.  This is usually
     * caused by read accesses without initialization.
     */

    /* clear from the bottom of memory to the stack */
    fillLongs ((UINT *)SYS_MEM_BOTTOM,
        ((UINT) RESIDENT_DATA - STACK_SAVE - (UINT)SYS_MEM_BOTTOM) /
        sizeof(long), 0);

    /* fill from the load image to the top of memory */
    fillLongs ((UINT *)end, ((UINT)SYS_MEM_TOP - (UINT)end) / sizeof (long), 0);

# else   /* ROM_RESIDENT */

#  if defined (UNCOMPRESS)

    /* clear from the bottom of memory to the stack */
    fillLongs ((UINT *)(SYS_MEM_BOTTOM),
        ((UINT)romInit - STACK_SAVE - (UINT)SYS_MEM_BOTTOM) /
        sizeof(long), 0);

    /* fill from the load image to the top of memory */
    fillLongs ((UINT *)end, ((UINT)SYS_MEM_TOP - (UINT)end) / sizeof (long), 0);

#  else  /* UNCOMPRESS */

#   if (RAM_DST_ADRS == RAM_HIGH_ADRS)
        /* clear from the bottom of memory to the stack */
        fillLongs ((UINT *)(SYS_MEM_BOTTOM),
            ((UINT)romInit - STACK_SAVE - (UINT)SYS_MEM_BOTTOM) /
            sizeof(long), 0);

        /*
        * fill from the end of the load image to the top of memory
        * (end of decompressed image isn't known, so the end of the
        *  compressed image is used -- should still be more efficient than
        *  clearing from RAM_DST_ADRS all the way to the top of memory)
        */
        fillLongs ((UINT *)LD_IMAGE_END,
            ((UINT)SYS_MEM_TOP - LD_IMAGE_END) / sizeof (long), 0);

#   else  /* RAM_DST_ADRS == RAM_HIGH_ADRS */

        /* fill from the bottom of memory to the load image */
        fillLongs ((UINT *)(SYS_MEM_BOTTOM),
        ((UINT)RAM_DST_ADRS - (UINT)SYS_MEM_BOTTOM) / sizeof (long), 0);

        /*
        * fill from the end of the load image to the stack
        * (end of decompressed image isn't known, so the end of the
        *  compressed image is used -- should still be more efficient than
        *  clearing from bottom of memory all the way to the stack)
        */
        fillLongs ((UINT *)LD_IMAGE_END,
            ((UINT)romInit - STACK_SAVE - LD_IMAGE_END) /
            sizeof (long), 0);

#   endif  /* RAM_DST_ADRS == RAM_HIGH_ADRS */

#  endif  /* UNCOMPRESS */

# endif  /* ROM_RESIDENT */

    /*
     * Ensure the boot line is null. This is necessary for those
     * targets whose boot line is excluded from cleaning.
     */

    *(BOOT_LINE_ADRS) = EOS;
    }

#endif  /* (CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM)) */

    /* jump to VxWorks entry point (after uncompressing) */

#if defined (UNCOMPRESS) || defined (ROM_RESIDENT)
# if (CPU_FAMILY == I960)
    absEntry = (FUNCPTR)sysInitAlt;         /* reinit proc tbl */
# else
    absEntry = (FUNCPTR)usrInit;            /* on to bootConfig */
# endif  /* CPU_FAMILY == I960 */

#else   /* defined UNCOMPRESS || defined ROM_RESIDENT */
    {
# if (CPU_FAMILY == MIPS)
    volatile FUNCPTR absUncompress = (FUNCPTR) UNCMP_RTN;
    if ((absUncompress) ((UCHAR *)ROM_OFFSET(binArrayStart),
             (UCHAR *)K0_TO_K1(RAM_DST_ADRS),
             (int)((UINT)binArrayEnd - (UINT)binArrayStart)) != OK)
# elif   (CPU_FAMILY == I80X86) || (CPU_FAMILY == ARM) || (CPU_FAMILY == COLDFIRE)
    volatile FUNCPTR absUncompress = (FUNCPTR) UNCMP_RTN;
    if ((absUncompress) ((UCHAR *)ROM_OFFSET(binArrayStart),
                (UCHAR *)RAM_DST_ADRS, binArrayEnd - binArrayStart) != OK)
# else
    if (UNCMP_RTN ((UCHAR *)ROM_OFFSET(binArrayStart),
                (UCHAR *)RAM_DST_ADRS, binArrayEnd - binArrayStart) != OK)
# endif  /* (CPU_FAMILY == MIPS) */
    return;     /* if we return then ROM's will halt */

    absEntry = (FUNCPTR)RAM_DST_ADRS;           /* sysInit() or usrEntry() */

# if (CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM))
    /* if cold booting, finish clearing memory */

    if (startType & BOOT_CLEAR)
        {
#  if (RAM_DST_ADRS == RAM_HIGH_ADRS)
        /* clear past the stack to the decompressed image */
        ((FUNCPTR)ROM_OFFSET(fillLongs)) ((UINT *)romInit,
            ((UINT)RAM_DST_ADRS - (UINT)romInit) / sizeof(long), 0);
#  else
        /* clear past the stack to the top of memory */
        ((FUNCPTR)ROM_OFFSET(fillLongs)) ((UINT *)romInit,
            ((UINT)SYS_MEM_TOP - (UINT)romInit) / sizeof(long), 0);
#  endif /* RAM_DST_ADRS == RAM_HIGH_ADRS */
        }
# endif  /* (CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM)) */

    }
#endif  /* defined UNCOMPRESS || defined ROM_RESIDENT */

#if ((CPU_FAMILY == ARM) && ARM_THUMB)
    absEntry = (FUNCPTR)((UINT32)absEntry | 1);     /* force Thumb state */
#endif  /* CPU_FAMILY == ARM */

    (absEntry) (startType);
    }

#if     (CPU_FAMILY==ARM) && (!defined(ROM_RESIDENT))
# undef fillLongs
#endif  /* (CPU_FAMILY==ARM) */


#if (CPU_FAMILY==MC680X0) && !defined(ROM_RESIDENT) && !defined(BOOTCODE_IN_RAM)
# undef copyLongs    /* undo effects from above define */
#endif /* CPU_FAMILY==MC680X0 */

/*******************************************************************************
*
* copyLongs - copy one buffer to another a long at a time
*
* This routine copies the first <nlongs> longs from <source> to <destination>.
*
* RETURNS:
*
* ERRNO
*/

LOCAL void copyLongs 
    (
    source,
    destination,
    nlongs
    )
    FAST UINT *source;      /* pointer to source buffer      */
    FAST UINT *destination; /* pointer to destination buffer */
    UINT nlongs;        /* number of longs to copy       */

    {
    FAST UINT *dstend = destination + nlongs;
    FAST UINT nchunks;

    /* Hop by chunks of longs, for speed. */
    for (nchunks = nlongs / 8; nchunks; --nchunks)
        {
#if (CPU_FAMILY == MC680X0)
        *destination++ = *source++; /* 0 */
        *destination++ = *source++; /* 1 */
        *destination++ = *source++; /* 2 */
        *destination++ = *source++; /* 3 */
        *destination++ = *source++; /* 4 */
        *destination++ = *source++; /* 5 */
        *destination++ = *source++; /* 6 */
        *destination++ = *source++; /* 7 */
#else
        destination[0] = source[0];
        destination[1] = source[1];
        destination[2] = source[2];
        destination[3] = source[3];
        destination[4] = source[4];
        destination[5] = source[5];
        destination[6] = source[6];
        destination[7] = source[7];
        destination += 8, source += 8;
#endif /* CPU_FAMILY == MC680X0 */
        }

    /* Do the remainder one long at a time. */
    while (destination < dstend)
        *destination++ = *source++;
    }

#ifndef BOOTCODE_IN_RAM
/*******************************************************************************
*
* fillLongs - fill a buffer with a value a long at a time
*
* This routine fills the first <nlongs> longs of the buffer with <val>.
*
* RETURNS:
*
* ERRNO
*/

LOCAL void fillLongs 
    (
    buf,
    nlongs,
    val
    )
    FAST UINT *buf; /* pointer to buffer              */
    UINT nlongs;    /* number of longs to fill        */
    FAST UINT val;  /* char with which to fill buffer */

    {
    FAST UINT *bufend = buf + nlongs;
    FAST UINT nchunks;

    /* Hop by chunks of longs, for speed. */
    for (nchunks = nlongs / 8; nchunks; --nchunks)
        {
#if (CPU_FAMILY == MC680X0)
        *buf++ = val;   /* 0 */
        *buf++ = val;   /* 1 */
        *buf++ = val;   /* 2 */
        *buf++ = val;   /* 3 */
        *buf++ = val;   /* 4 */
        *buf++ = val;   /* 5 */
        *buf++ = val;   /* 6 */
        *buf++ = val;   /* 7 */
#else
        buf[0] = val;
        buf[1] = val;
        buf[2] = val;
        buf[3] = val;
        buf[4] = val;
        buf[5] = val;
        buf[6] = val;
        buf[7] = val;
        buf += 8;
#endif /* CPU_FAMILY == MC680X0 */
        }

    /* Do the remainder one long at a time. */
    while (buf < bufend)
        *buf++ = val;
    }
#endif  /* BOOTCODE_IN_RAM */

#if (CPU==XSCALE)
int checkLongs (source, destination, nlongs)
FAST UINT *source;          /* pointer to source buffer      */
FAST UINT *destination;     /* pointer to destination buffer */
UINT nlongs;                /* number of longs to copy       */

    {
    int fine = 1;

    FAST UINT *dstend = destination + nlongs;

    while (destination < dstend)
        {
        if (*destination++ != *source++)
            {
            fine = 0;
            break;
            }
        }
    return fine;
    }
#endif

