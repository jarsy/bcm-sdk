/* bootInit.c - ROM initialization module */

/* Copyright 1989-1997 Wind River Systems, Inc. */
#include "copyright_wrs.h"
/* Copyright 1998 Motorola, Inc.  */

/* $Id: bootInit.c,v 1.5 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
01c,30apr02,jmb   get rid of L2 cache stuff for 8245
01b,18Sep98,My	  Added MPC750 L2Backside cache size.
01a,02Mar98,mlo   Replaced copyLongs with copyDbls for yellowknife
01a,02Mar98,mlo   Replaced fillLongs with fillDbls for yellowknife
		NOTE: This version was modified by Motorola and this
		       copy of the file in in the yellowknife BSP
		       section.
		      The original bootInit.c is still in the all directory.
      The make will compile only this bootInitYk.c in the BSP directory. 
	 and will not use the bootInit.c from the all directory.BSP directory.
	This change requires that romInit.s does not turn off the FP in the msr.

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
.bS 22
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
.bE
.bS 6
    --------------
    |    ROM     |
    |            |  0xff8xxxxx  = binArrayStart
    |            |
    |            |  0xff800008  = ROM_TEXT_ADRS
    --------------  0xff800000  = ROM_BASE_ADRS
.bE

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

void sysSerialPrintString2 (char a, char b, char c, char d);
void sysLedDsply2 (char a, char b, char c, char d);

#define ROMPRINT(a,b,c,d)  \
        ((romPrintRtn == NULL) ? OK :        \
         (romPrintRtn) (a,b,c,d))

#if VX_VERSION == 62

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

#ifndef RAM_DST_ADRS                    /* default uncompress dest. */
#  define RAM_DST_ADRS        RAM_HIGH_ADRS
#endif

#if ((CPU_FAMILY == MIPS) || (CPU_FAMILY==PPC) || \
     (CPU_FAMILY==COLDFIRE))
#  define RESIDENT_DATA   RAM_DST_ADRS
#else
#  define RESIDENT_DATA   wrs_kernel_data_start
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
    int boardId;
    VOIDFUNCPTR romPrintRtn;
#if ((CPU_FAMILY==SPARC) || (CPU_FAMILY==MIPS) || (CPU_FAMILY==I80X86) || \
     (CPU_FAMILY==PPC) || (CPU_FAMILY==ARM))
    volatile            /* to force absolute addressing */
#endif /* (CPU_FAMILY==SPARC) */
    FUNCPTR absEntry;       /* to avoid PC Relative Jump Subroutine */
#if (CPU_FAMILY==MC680X0) && !defined(ROM_RESIDENT) && !defined(BOOTCODE_IN_RAM)
    volatile VOIDFUNCPTR romcopyLongs = &copyLongs;  /* force call to ROM */
# define copyLongs romcopyLongs
#endif /* (CPU_FAMILY==MC680X0) */

    /* Check board type...see if we have LEDs or not.  If no LED's
    *  use serial
    */
#ifdef BRINGUP
    boardId = SYS_REVID_GET();
    if (boardId == ID_CFM_1)
        romPrintRtn = ROM_OFFSET(sysSerialPrintString2);
    else
        romPrintRtn = ROM_OFFSET(sysLedDsply2);
#else
    romPrintRtn = NULL;
#endif

    ROMPRINT ('C', 'H', 'K', '0');

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

    ROMPRINT ('C', 'H', 'K', '1');
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

    ROMPRINT ('C', 'H', 'K', '2');
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

        ROMPRINT ('C', 'H', 'K', '3');
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

        ROMPRINT ('C', 'H', 'K', '4');
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
# elif   (CPU_FAMILY == I80X86) || (CPU_FAMILY == ARM)
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

    ROMPRINT ('C', 'H', 'K', '5');
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

#else /* !VX_VERSION == 62 */

#define	UNCMP_RTN	inflate

#ifndef USER_RESERVED_MEM
#   define USER_RESERVED_MEM 0
#endif

/*
 * If memory is to be cleared, it will be cleared from SYS_MEM_BOTTOM
 * up to (but not including) SYS_MEM_TOP, except for text and data segments.
 * The user reserved area is not cleared.
 */

#define	SYS_MEM_TOP \
	(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE - USER_RESERVED_MEM)

#define SYS_MEM_BOTTOM \
	(LOCAL_MEM_LOCAL_ADRS + RESERVED)

#if VX_VERSION == 55
#define BINARRAYEND_ROUNDOFF	(ROUND_DOWN(binArrayEnd, sizeof(long)))
#endif

IMPORT void	romInit ();
IMPORT STATUS	UNCMP_RTN ();
IMPORT void	usrInit ();
IMPORT void	sysInitAlt ();
IMPORT void	start ();

#if	((CPU_FAMILY == MIPS) || (CPU_FAMILY==PPC))
#define	binArrayStart	_binArrayStart
#define	binArrayEnd	_binArrayEnd
#define	RESIDENT_DATA	RAM_DST_ADRS
#else
#define	RESIDENT_DATA	(&sdata)
IMPORT char	sdata;			/* defined in romInit.s */
#endif

IMPORT UCHAR	binArrayStart [];	/* compressed binary image */
IMPORT UCHAR	binArrayEnd;		/* end of compressed binary image */
IMPORT char	etext [];		/* defined by the loader */
IMPORT char	edata [];		/* defined by the loader */
IMPORT char	end [];			/* defined by the loader */

#if VX_VERSION == 55
IMPORT UCHAR	wrs_kernel_data_end [];    /* defined by the loader */
#endif

#ifndef RAM_DST_ADRS                	/* default uncompress dest. */
#define RAM_DST_ADRS        RAM_HIGH_ADRS
#endif

/* If the boot code is in RAM and the RAM is already initialized,
 * clearing the RAM is not necessary.  Macro BOOTCODE_IN_RAM is
 * used not to clear the RAM.
 */
#ifdef	BOOTCODE_IN_RAM			/* not to clear RAM */
#undef	ROM_TEXT_ADRS
#undef	ROM_BASE_ADRS
#define	ROM_TEXT_ADRS	((UINT)romInit)
#define	ROM_BASE_ADRS	((UINT)romInit)
#endif	/* BOOTCODE_IN_RAM */

#if	defined (UNCOMPRESS) || defined (ROM_RESIDENT)
#define	ROM_COPY_SIZE	(ROM_SIZE - (ROM_TEXT_ADRS - ROM_BASE_ADRS))
#endif	/* UNCOMPRESS */

#define ROM_OFFSET(adr)	(((UINT)adr - (UINT)romInit) + ROM_TEXT_ADRS)

/* forward declarations */

/*
 * copy/fill prototypes  3/2/98 mlo
 *  Prototype for double copy and fills for romStart  3/2/98 mlo
 *  copyDbls and fillDbls
 *
*/

#define DOUBLE double
void copyDbls ( DOUBLE* source, DOUBLE* destination, UINT number);
#if 0
LOCAL void copyLongs (FAST UINT *source, FAST UINT *destination, UINT nlongs);
#endif  /*  #if 0 not used for this yellowknife configuration */

#ifndef	BOOTCODE_IN_RAM
void fillDbls ( DOUBLE* source, UINT number, DOUBLE val);
#if 0
LOCAL void fillLongs (FAST UINT *buf, UINT nlongs, FAST UINT val);
#endif  /*  #if 0 not used for this yellowknife configuration */
#endif	/* BOOTCODE_IN_RAM */

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
*/

void romStart
    (
    FAST int startType		/* start type */
    )

    {
    int boardId;
    VOIDFUNCPTR romPrintRtn;


#if ((CPU_FAMILY==SPARC) || (CPU_FAMILY==MIPS) || (CPU_FAMILY==I80X86) || \
     (CPU_FAMILY==PPC))
    volatile			/* to force absolute adressing */
#endif /* (CPU_FAMILY==SPARC) */
    FUNCPTR absEntry;		/* to avoid PC Relative Jump Subroutine */

    /* Check board type...see if we have LEDs or not.  If no LED's
    *  use serial
    */
#ifdef BRINGUP
    boardId = SYS_REVID_GET();
    if (boardId == ID_CFM_1)
        romPrintRtn = ROM_OFFSET(sysSerialPrintString2);
    else
        romPrintRtn = ROM_OFFSET(sysLedDsply2);
#else
    romPrintRtn = NULL;
#endif

    ROMPRINT ('C', 'H', 'K', '0');

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

    
#if  (CPU_FAMILY == SPARC)
    copyDbls ((DOUBLE *)(etext + 8), (DOUBLE *) RESIDENT_DATA,
#elif	((CPU_FAMILY == MIPS) || (CPU_FAMILY == PPC))
    copyDbls ((DOUBLE *)(etext + 0), (DOUBLE *) RESIDENT_DATA,
#else
    copyDbls ((DOUBLE *)(etext + 4), (DOUBLE *) RESIDENT_DATA,
#endif
#if VX_VERSION == 55
		 ((UINT) wrs_kernel_data_end - (UINT) RESIDENT_DATA) / sizeof (DOUBLE));
#else
		 ((UINT) edata - (UINT) RESIDENT_DATA) / sizeof (DOUBLE));
#endif

#else	/* ROM_RESIDENT */

#ifdef UNCOMPRESS

#if	(CPU_FAMILY == MIPS)
    /*
     * copy text to uncached locations to avoid problems with
     * copy back caches
     */
    ((FUNCPTR)ROM_OFFSET(copyDbls)) (ROM_TEXT_ADRS, (UINT)K0_TO_K1(romInit),
		ROM_COPY_SIZE / sizeof (DOUBLE));
#else	/* CPU_FAMILY == MIPS */
    ((FUNCPTR)ROM_OFFSET(copyDbls)) (ROM_TEXT_ADRS, (UINT)romInit,
		ROM_COPY_SIZE / sizeof (DOUBLE));
#endif	/* CPU_FAMILY == MIPS */

    ROMPRINT ('C', 'H', 'K', '1');
#else	/* UNCOMPRESS */

#if	(CPU_FAMILY == MIPS)
    /*
     * copy text to uncached locations to avoid problems with
     * copy back caches
     * copy the entire data segment because there is no way to ensure that
     * binArray is the last thing in the data segment because of GP relative
     * addressing
     */
#if VX_VERSION == 55
    ((FUNCPTR)ROM_OFFSET(copyDbls)) (ROM_TEXT_ADRS, (UINT)K0_TO_K1(romInit),
		((UINT)wrs_kernel_data_end - (UINT)romInit) / sizeof (DOUBLE));
#else
    ((FUNCPTR)ROM_OFFSET(copyDbls)) (ROM_TEXT_ADRS, (UINT)K0_TO_K1(romInit),
		((UINT)edata - (UINT)romInit) / sizeof (DOUBLE));
#endif
#else	/* CPU_FAMILY == MIPS */
    ((FUNCPTR)ROM_OFFSET(copyDbls)) (ROM_TEXT_ADRS, (UINT)romInit,
		((UINT)binArrayStart - (UINT)romInit) / sizeof (DOUBLE));
#if VX_VERSION == 55
    ((FUNCPTR)ROM_OFFSET(copyDbls))
           ((UINT *)((UINT)ROM_TEXT_ADRS + ((UINT)BINARRAYEND_ROUNDOFF - 
           (UINT)romInit)), (UINT *)BINARRAYEND_ROUNDOFF,
           ((UINT)wrs_kernel_data_end - (UINT)binArrayEnd) / sizeof (long));
#endif
#endif	/* CPU_FAMILY == MIPS */

    ROMPRINT ('C', 'H', 'K', '2');
 
#endif	/* UNCOMPRESS */
#endif	/* ROM_RESIDENT */


#if	(CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM))

    /* clear all memory if cold booting */

    if (startType & BOOT_CLEAR)
	{
#ifdef ROM_RESIDENT
	/* Clear memory not loaded with text & data.
	 *
	 * We are careful about initializing all memory (except
	 * STACK_SAVE bytes) due to parity error generation (on
	 * some hardware) at a later stage.  This is usually
	 * caused by read accesses without initialization.
	 */
	fillDbls ((DOUBLE *)SYS_MEM_BOTTOM,
		((UINT) RESIDENT_DATA - STACK_SAVE - (UINT)SYS_MEM_BOTTOM)
		/ sizeof(DOUBLE), 0);
#if VX_VERSION == 55
	fillDbls (((DOUBLE *) wrs_kernel_data_end),
		((UINT)SYS_MEM_TOP - ((UINT) wrs_kernel_data_end)) / sizeof(DOUBLE), 0);
#else
	fillDbls (((DOUBLE *) edata),
		((UINT)SYS_MEM_TOP - ((UINT) edata)) / sizeof(DOUBLE), 0);
#endif

#else	/* ROM_RESIDENT */
	fillDbls ((DOUBLE *)(SYS_MEM_BOTTOM),
		((UINT)romInit - STACK_SAVE - (UINT)SYS_MEM_BOTTOM) /
		sizeof(DOUBLE), 0);

        ROMPRINT ('C', 'H', 'K', '3');
#if     defined (UNCOMPRESS)
	fillDbls ((DOUBLE *)((UINT)romInit + ROM_COPY_SIZE),
		    ((UINT)SYS_MEM_TOP - ((UINT)romInit + ROM_COPY_SIZE))
		    / sizeof(DOUBLE), 0);
#else
#if VX_VERSION == 55
	fillDbls ((DOUBLE *)wrs_kernel_data_end,
		((UINT)SYS_MEM_TOP - (UINT)wrs_kernel_data_end) / sizeof (DOUBLE), 0);
#else
	fillDbls ((DOUBLE *)binArrayStart,
		((UINT)SYS_MEM_TOP - (UINT)binArrayStart) / sizeof (DOUBLE), 0);
#endif
#endif 	/* UNCOMPRESS */
        ROMPRINT ('C', 'H', 'K', '4');
#endif 	/* ROM_RESIDENT */

	}

#endif	/* (CPU_FAMILY != MIPS) && (!defined (BOOTCODE_IN_RAM)) */

    /* jump to VxWorks entry point (after uncompressing) */

#if	defined (UNCOMPRESS) || defined (ROM_RESIDENT)
#if	(CPU_FAMILY == I960)
    absEntry = (FUNCPTR)sysInitAlt;			/* reinit proc tbl */
#else
    absEntry = (FUNCPTR)usrInit;			/* on to bootConfig */
#endif	/* CPU_FAMILY == I960 */

#else
    {
#if	(CPU_FAMILY == MIPS)
    volatile FUNCPTR absUncompress = (FUNCPTR) UNCMP_RTN;
    if ((absUncompress) ((UCHAR *)ROM_OFFSET(binArrayStart),
			 (UCHAR *)K0_TO_K1(RAM_DST_ADRS),
			 (int)((UINT)&binArrayEnd - (UINT)binArrayStart)) != OK)
#elif	(CPU_FAMILY == I80X86)
    volatile FUNCPTR absUncompress = (FUNCPTR) UNCMP_RTN;
    if ((absUncompress) ((UCHAR *)ROM_OFFSET(binArrayStart),
	            (UCHAR *)RAM_DST_ADRS, &binArrayEnd - binArrayStart) != OK)
#else
    if (UNCMP_RTN ((UCHAR *)ROM_OFFSET(binArrayStart),
	            (UCHAR *)RAM_DST_ADRS, &binArrayEnd - binArrayStart) != OK)
#endif	/* (CPU_FAMILY == MIPS) */
	return;		/* if we return then ROM's will halt */

    ROMPRINT ('C', 'H', 'K', '5');
    absEntry = (FUNCPTR)RAM_DST_ADRS;			/* compressedEntry () */
    }
#endif	/* defined UNCOMPRESS || defined ROM_RESIDENT */

    (absEntry) (startType);
    }

	
/*
 *  copyDbls.c  2/26/98 mo
 *
 *  copy double words for rom to ram for BSP
 *   using double variable (i.e. 8 bytes copies).
 *
 *  This functions replaces copylongs in the romStart functions
 *   of BootInit.c
*/

#if 0  /*  debug code  */
DOUBLE* s1;
DOUBLE* d1;
UINT    nd;
UINT    myval;
DOUBLE* dend;
DOUBLE  myvald;
#endif  /*  end debug code  */

void copyDbls( DOUBLE* source, DOUBLE* destination, UINT ndbls)
{
/*  copy ndbls doubles (8 bytes) from source to destination. */

	FAST DOUBLE *dstend = destination + ndbls;


#if 0  /*  debug code  */
	s1 = source;
	d1 = destination;
	nd = ndbls;
	dend = dstend;
#endif   /*  end debug code  */



	while (destination < dstend)
	  *destination++ = *source++;
}

/*========================================================================*/
#if 0
/*  this function is not used for the yellowknife, but may be used
 * for other processors, so just if it out.
 *  mo  3/2/98
 ***************
 *
 * copyLongs - copy one buffer to another a long at a time
 *
 * This routine copies the first <nlongs> longs from <source> to <destination>.
 */

int bootInitYK=0;
LOCAL void copyLongs (source, destination, nlongs)
    FAST UINT *source;		/* pointer to source buffer      */
    FAST UINT *destination;	/* pointer to destination buffer */
    UINT nlongs;		/* number of longs to copy       */

    {
    FAST UINT *dstend = destination + nlongs;

    while (destination < dstend)
	*destination++ = *source++;
    }
#endif  /* #if 0 copyLongs  */
/*========================================================================*/

#ifndef	BOOTCODE_IN_RAM

/*
 *
 * fillDbls  3/2/98 mo 
 *  fill a buffer with a value a double at a time
 *
 * This routine fills the first <ndbls> doubles of the buffer with <val>.
 */

#if 0
void fillDbls (buf, ndbls, val)
    FAST DOUBLE *buf;	/* pointer to buffer              */
    UINT ndbls;	/* number of doubles to fill        */
    UINT val;	/* char with which to fill buffer */

{
    FAST DOUBLE *bufend = buf + ndbls;
    union dummydouble {
	DOUBLE  newval;
	UINT    newvalparts[2];
    }  storeval;	


    storeval.newvalparts[0] = 0x0;
    storeval.newvalparts[1]=val;
	
#if 0   /*  debug code  */
	s1 = buf;
	nd = ndbls;
	dend = bufend;
	myval = val;
	myvald = storeval.newval;
#endif   /*  end debug code  */


    while (buf < bufend)
	*buf++ = storeval.newval;
}
#endif

void fillDbls (buf, ndbls, val)
    FAST DOUBLE *buf;	/* pointer to buffer              */
    UINT ndbls;	/* number of doubles to fill        */
    FAST DOUBLE val;	/* char with which to fill buffer */

{
    FAST DOUBLE *bufend = buf + ndbls;

    while (buf < bufend)
	*buf++ = val;
}


/*========================================================================*/
#if 0
/*  this function is not used for the yellowknife, but may be used
 * for other processors, so just if it out.
 *  mo  3/2/98
 ****************
 *
 * fillLongs - fill a buffer with a value a long at a time
 *
 * This routine fills the first <nlongs> longs of the buffer with <val>.
 */

LOCAL void fillLongs (buf, nlongs, val)
    FAST UINT *buf;	/* pointer to buffer              */
    UINT nlongs;	/* number of longs to fill        */
    FAST UINT val;	/* char with which to fill buffer */

    {
    FAST UINT *bufend = buf + nlongs;

    while (buf < bufend)
	*buf++ = val;
    }
#endif  /* #if 0 fillLongs  */
/*========================================================================*/

	

#endif	/* BOOTCODE_IN_RAM */

#endif /* VX_VERSION == 62 */

/*****************************************************************************
*
* sysLedDsply2 - print 4 characters to alphanumeric LEDs
*
* This routine should only be used when running from the bootrom.
* After the bootrom image has been copied to ram and decompressed, you
* should use sysLedDsply() to write a string to the LEDs.
*
* Returns:  N/A
*/
void sysLedDsply2 
    (
    char a, 
    char b, 
    char c, 
    char d
    ) 
    {
    LED_REG(0) = d; 
    LED_REG(1) = c; 
    LED_REG(2) = b; 
    LED_REG(3) = a;
    }

/*****************************************************************************
*
* sysSerialPrintString2 - print 4 characters to the serial port
*
* This routine should only be used when running from the bootrom.
* After the bootrom image has been copied to ram and decompressed, you
* should use sysSerialPrintString() for general purpose string printing.
*
* Returns:  N/A
*/
void sysSerialPrintString2
    (
    char a, 
    char b, 
    char c, 
    char d
    ) 
    {
    VOIDFUNCPTR func = ROM_OFFSET(SEROUT);
    func (a);
    func (b);
    func (c);
    func (d);
    func ('\n');
    func ('\r');
    }
