/* tffsConfig.c - TrueFFS configuration file for VxWorks */

/* $Id: tffsConfig.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 1997-2002, 2004, 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* FAT-FTL Lite Software Development Kit
 * Copyright (C) M-Systems Ltd. 1995-2001	*/

/*
modification history
--------------------
02f,30mar07,b_m  add v28F128J3 MTD identify.
02e,23mar06,jb3  Bump Version for Spring 06.
02d,23nov04,mil  Fixed compilation warning. (SPR 103341)
02c,02oct02,nrv  fixing diab build error
02b,28mar02,nrv  adding INCLUDE_MTD_CFIAMD
02a,12dec01,nrv  supressing the compiler warning on tffsVersion
01z,07nov01,yp   fixing SPR #63838
01y,02nov01,nrv  changing the version number to 2.2
01x,17sep01,yp   merging code changes from T3. Preserving T2 specifics that
                 were removed in T3
01w,25apr01,mem  Remove NFTL, CDSN and DOC2.
01v,20dec00,tlc  Add user-defined "Identify" routine to MTD table.
01u,23apr01,yp   Incrementing version number to 2.1
01t,29nov00,tlc  Added MTD_USR_IDENTIFY to the mtdTable. This allows
                 user-defined identify routines to be used for trueFFS
		                  support.
01s,06Sep00,csh  Added code to free pBuf on error SPR #34246
01r,06Sep00,csh  Added check for pBuf == NULL SPR #34246
01q,01Sep00,csh  Removed MACRO for INCLUDE_SHOW_ROUTINES
01p,04feb99,yp   added MACRO INCLUDE_TFFS_SHOW for better scalability.
01o,21apr98,yp   added tffs subdir to include path
01n,28feb98,kbw  made man page edits to fix problems found by QE
01m,28feb98,kbw  made man page edits
01l,19jan98,hdn  added new function tffsShowAll().
01k,08jan98,hdn  added new MTD for Heurikon Baja4700.
01j,06jan98,hdn  cleaned up tffsShow() and tffsBootImagePut().
01i,18dec97,yp   added documentation on sharing bootrom with TFFS
01h,15dec97,yp   doc cleanup
01g,11dec97,hdn  added tffsRawio()'s return value check in tffsShow().
01f,08dec97,hdn  renamed mkbootTffs() to tffsCopy().
01e,05dec97,hdn  changed tffsSocket[] to LOCAL variable.
01d,05dec97,hdn  added tffsShow() and mkbootTffs().
01c,11nov97,hdn  fixed documentation of flRegisterComponents().
01b,07nov97,hdn  cleaned up.
01a,07jul97,ami  written by Amirban at M-Systems
*/

/*
DESCRIPTION
This source file, with the help of sysTffs.c, configures TrueFFS for VxWorks.
The functions defined here are generic to all BSPs.  To include these functions
in the BSP-specific module, the BSP's sysTffs.c file includes this file.
Within the sysTffs.c file, define statements determine which functions from
the tffsConfig.c file are ultimately included in TrueFFS.

The only externally callable routines defined in this file
are tffsShow(), tffsShowAll(), and tffsBootImagePut().  You can
exclude the show utilities if you edit config.h and
undefine INCLUDE_SHOW_ROUTINES.  You can exclude tffsBootImagePut() if you
edit sysTffs.c and undefine INCLUDE_TFFS_BOOT_IMAGE.  (If you find these
utilities are missing and you want them included, edit config.h and
define INCLUDE_SHOW_ROUTINES and INCLUDE_TFFS_BOOT_IMAGE.)

If you wish to include only the TrueFFS specific show routines you could
define INCLUDE_TFFS_SHOW instead of INCLUDE_SHOW_ROUTINES in config.h.

However, for the most part, these externally callable routines are only
a small part of the TrueFFS configuration needs handled by this file.
The routines internal to this file make calls into the MTDs and translation
layer modules of TrueFFS.  At link time, resolving the symbols associated
with these calls pulls MTD and translation layer modules into VxWorks.

However, each of these calls to the MTDs and the translation layer modules
is only conditionally included.  The constants that control the includes
are defined in sysTffs.c.  To exclude an MTD or translation layer module,
you edit sysTffs.c, undefine the appropriate constant, and rebuild sysTffs.o.
These constants are described in the reference entry for 'sysTffs'.

INCLUDE FILES: stdcomp.h
*/


/* includes */

#include "tffs/stdcomp.h"
#include "tffs/tffsDrv.h"
#include "tffs/fatlite.h"
#include "stdio.h"


/* defines */

#ifdef INCLUDE_SHOW_ROUTINES
#define INCLUDE_TFFS_SHOW
#endif /* INCLUDE_SHOW_ROUTINES */

/* externs */

#ifdef INCLUDE_MTD_USR
FLStatus  MTD_USR_IDENTIFY (FLFlash vol);
#endif /* INCLUDE_MTD_USR */

/* globals */

MTDidentifyRoutine mtdTable[] = 	/* MTD tables */
    {
#ifdef  INCLUDE_MTD_SBC8548
    v28F128J3Identify,
#endif  /* INCLUDE_MTD_SBC8548 */

#ifdef  INCLUDE_MTD_CFIAMD
    cfiAmdIdentify,
#endif  /* INCLUDE_MTD_CFIAMD */

#ifdef	INCLUDE_MTD_CFISCS
    cfiscsIdentify,
#endif	/* INCLUDE_MTD_CFISCS */

#ifdef	INCLUDE_MTD_I28F016
    i28f016Identify,
#endif	/* INCLUDE_MTD_I28F016 */

#ifdef	INCLUDE_MTD_I28F008
    i28f008Identify,
#endif	/* INCLUDE_MTD_I28F008 */

#ifdef	INCLUDE_MTD_I28F008_BAJA
    i28f008BajaIdentify,
#endif	/* INCLUDE_MTD_I28F008_BAJA */

#ifdef	INCLUDE_MTD_AMD
    amdMTDIdentify,
#endif	/* INCLUDE_MTD_AMD */

#ifdef INCLUDE_MTD_USR
    MTD_USR_IDENTIFY,
#endif  /* INCLUDE_MTD_USR */

#ifdef	INCLUDE_MTD_WAMD
    flwAmdMTDIdentify,
#endif	/* INCLUDE_MTD_WAMD */
    NULL
    };
int noOfMTDs = NELEMENTS (mtdTable) - 1;	/* number of MTDs */

TLentry tlTable[] = 			/* TL tables */
    {
#ifdef	INCLUDE_TL_FTL
#ifdef	FORMAT_VOLUME
    {mountFTL, formatFTL},
#else
    mountFTL,
#endif	/* FORMAT_VOLUME */
#endif	/* INCLUDE_TL_FTL */

#ifdef	INCLUDE_TL_SSFDC
#ifdef	FORMAT_VOLUME
    {mountSSFDC, formatSSFDC},
#else
    mountSSFDC,
#endif	/* FORMAT_VOLUME */
#endif	/* INCLUDE_TL_SSFDC */

#ifdef	FORMAT_VOLUME
    {NULL, NULL}
#else
    NULL
#endif	/* FORMAT_VOLUME */
    };
int noOfTLs = NELEMENTS (tlTable) - 1;	/* number of TLs */


/* locals */

LOCAL char * tffsSocket[DRIVES] = {NULL}; /* name of the socket interface */

#ifdef INCLUDE_TFFS_SHOW
LOCAL char * tffsVersion = "2.2.1";	/* WRS version number of TFFS */
#endif /* INCLUDE_TFFS_SHOW */

/* forward declarations */

LOCAL VOID sysTffsInit (void);		/* BSP dependent init routine */


/*******************************************************************************
*
* flRegisterComponents - register MTD and translation layer components for use
*
* This routine registers MTD and translation layer components for use.
* This function is called by FLite once only, at initialization of the
* FLite system.
*
* NOMANUAL
*
* RETURNS: N/A
*/

void flRegisterComponents (void)
    {
    sysTffsInit ();
    }

#ifdef	INCLUDE_TFFS_SHOW
/*******************************************************************************
*
* tffsShowAll - show device information on all socket interfaces
*
* This routine prints device information on all socket interfaces.
*
* RETURNS: N/A
*/

void tffsShowAll (void)
    {
    int ix;

    printf ("TFFS Version %s\n", tffsVersion);
    for (ix = 0; ix < noOfDrives; ix++)
	tffsShow (ix);
    }

/*******************************************************************************
*
* tffsShow - show device information on a specific socket interface
*
* This routine prints device information on the specified socket interface.
* This information is particularly useful when trying to determine the
* number of Erase Units required to contain a boot image.  The field called
* unitSize reports the size of an Erase Unit.
*
* If the process of getting physical information fails, an error code is
* printed. The error codes can be found in flbase.h.
*
* RETURNS: N/A
*/

void tffsShow
    (
    int driveNo			/* TFFS drive number */
    )
    {
    PhysicalInfo info;
    FLStatus status;

    if (tffsSocket[driveNo] == NULL)
	{
        printf ("%d: ---- no socket interface installed ----\n", driveNo);
	return;
	}
    status = tffsRawio (driveNo, TFFS_GET_PHYSICAL_INFO, (int)&info, 0, 0);
    if (status != OK)
	{
        printf ("%d: **** communication failed with error %d ****\n",
                driveNo, status);
	return;
	}
    printf ("%d: socket=%s: ", driveNo, tffsSocket[driveNo]);
    printf ("type=0x%x, unitSize=0x%x, mediaSize=0x%x\n", info.type,
	    (UINT)info.unitSize, (UINT)info.mediaSize);
    }

#endif	/* INCLUDE_TFFS_SHOW */

#ifdef	INCLUDE_TFFS_BOOT_IMAGE
/*******************************************************************************
*
* tffsBootImagePut - write to the boot-image region of the flash device
*
* This routine writes an input stream to the boot-image region (if any) of
* a flash memory device.  Typically, the input stream contains a boot image,
* such as the VxWorks boot image, but you are free to use this function to
* write any data needed. The size of the boot-image region is set by
* the tffsDevFormat() call (or the sysTffsFormat() call, a BSP-specific helper
* function that calls tffsDevFormat() internally) that formats the flash
* device for use with TrueFFS.
*
* If tffsBootImagePut() is used to put a VxWorks boot image in flash, you
* should not use the s-record version of the boot image typically produced
* by make.  Instead, you should take the pre s-record version (usually
* called 'bootrom' instead of 'bootrom.hex'), and filter out its loader
* header information using an <xxx>'ToBin' utility.  For example:
* .CS
* elfToBin < bootrom > bootrom.bin
* .CE
*
* Use the resulting 'bootrom.bin' as input to tffsBootImagePut().
*
* The discussion above assumes that you want only to use the flash device to
* store a VxWorks image that is retrieved from the flash device and then run
* out of RAM. However, because it is possible to map many flash devices
* directly into the target's memory, it is also possible run the VxWorks
* image from flash memory, although there are some restrictions:
* .IP "-"
* The flash device must be non-NAND.
* .IP "-"
* Only the text segment of the VxWorks image ('vxWorks.res_rom') may run out
* of flash memory. The data segment of the image must reside in standard RAM.
* .IP "-"
* No part of the flash device may be erased while the VxWorks image is running
* from flash memory.
* .LP
* Because TrueFFS garbage collection triggers an erase, this last restriction
* means that you cannot run a VxWorks boot image out of a flash device that
* must also support a writable file system (although a read-only file system
* is OK).
*
* This last restriction arises from the way in which flash devices are
* constructed. The current physical construction of flash memory devices does
* not allow access to the device while an erase is in
* progress anywhere on the flash device. As a result, if TrueFFS tries to
* erase a portion of the flash device, the entire device becomes inaccessible
* to all other users.  If that other user happens to be the VxWorks image
* looking for its next instruction, the VxWorks image crashes.
*
*RETURNS: OK or ERROR
*/

STATUS tffsBootImagePut
    (
    int driveNo,		/* TFFS drive number */
    int offset,			/* offset in the flash chip/card */
    char * filename		/* binary format of the bootimage */
    )
    {
    PhysicalInfo info;
    UINT unitNo;
    UINT unitSize;
    UINT addr;
    char * pBuf;
    int fd;
    int ix;

    if (tffsSocket[driveNo] == NULL)
	return (ERROR);

    if (tffsRawio (driveNo, TFFS_GET_PHYSICAL_INFO, (int)&info, 0, 0) != OK)
        {
        printErr ("Unable to get physical info from drive\n");
	return (ERROR);
	}

    if ((pBuf = (char *)malloc (info.unitSize)) == NULL)
	{
       	printErr ("Can't allocate %d bytes of memory\n", info.unitSize);
	return( ERROR ) ;
	}

    if ((fd = open (filename, O_RDONLY, 0644)) == ERROR)
        {
        printErr ("Can't open \"%s\"\n", filename);
	free( pBuf ) ;
        return (ERROR);
	}

    addr = offset;
    unitNo = offset / info.unitSize;
    unitSize = info.unitSize - (offset % info.unitSize);

    /* If the offset is not Erase Unit aligned we need to save the the
     * contents of the region begining at the start of this erase unit
     * and ending at the specified address so we can restore it after
     * we erase the Erase Unit
     */
    if (tffsRawio (driveNo, TFFS_PHYSICAL_READ, unitNo * info.unitSize,
	           offset % info.unitSize, (int)pBuf) != OK)
        {
	printErr ("Failed attempting to save Erase Unit %d\n", unitNo);
	close (fd);
	free( pBuf ) ;
	return (ERROR);
	}

    if (tffsRawio (driveNo, TFFS_PHYSICAL_ERASE, unitNo, 1, 0) != OK)
	{
	printErr ("Failed attempting to erase Erase Unit %d\n", unitNo);
	close (fd);
	free( pBuf ) ;
	return (ERROR);
	}

    if (tffsRawio (driveNo, TFFS_PHYSICAL_WRITE, unitNo * info.unitSize,
	           offset % info.unitSize, (int)pBuf) != OK)
        {
	printErr ("Failed attempting to restore Erase Unit %d\n", unitNo);
	close (fd);
	free( pBuf ) ;
	return (ERROR);
	}

    while (unitSize)
	{
        if ((ix = read (fd, pBuf, unitSize)) == ERROR)
            {
            printErr ("Error reading inputfile: 0x%x\n", errno);
	    free( pBuf ) ;
            return (ERROR);
            }

        if (ix == 0)
	    break;

        if ((addr + ix) > info.mediaSize)
	    {
	    printErr ("Error : Attempting to write beyond Flash boundary\n");
	    close (fd);
	    free( pBuf ) ;
	    return (ERROR);
	    }

        if (tffsRawio (driveNo, TFFS_PHYSICAL_WRITE, addr, ix, (int)pBuf) != OK)
	    {
	    printErr ("Physical write failed at address 0x%x\n", addr);
	    close (fd);
	    free( pBuf ) ;
	    return (ERROR);
	    }

	addr += ix;
	unitSize -= ix;
	if (unitSize == 0)
	    {
            unitSize = info.unitSize;
            unitNo++;
            if (tffsRawio (driveNo, TFFS_PHYSICAL_ERASE, unitNo, 1, 0) != OK)
	        {
		printErr ("Failed attempting to erase Erase Unit %d\n", unitNo);
	        close (fd);
	        free( pBuf ) ;
	        return (ERROR);
	        }
	    }
	}

    close (fd);
    free( pBuf ) ;
    return (OK);
    }

#endif	/* INCLUDE_TFFS_BOOT_IMAGE */
