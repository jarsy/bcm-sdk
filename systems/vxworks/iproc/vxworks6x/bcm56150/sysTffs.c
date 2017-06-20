/* f2xFlash-sysTffs.c - f2xFlash TrueFFS socket driver template */

/*
 * Copyright (c) 2001-2002,2007,2011-2012 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01e,26mar12,tjf updated copyright
01d,09aug11,tjf changed example code in sysTffsInit() to match f2xFlashMem docs
01c,18jan07,tjf updated copyright
01b,19may02,tjf added initialization of vol.window.baseAddress in f2xRegister()
01a,29jan01,tjf writted by Ted Friedl, Wind River PS, Madison WI
*/

/*
DESCRIPTION
This library provides board-independent TrueFFS socket-level routines
necessary to create one or more TrueFFS devices from an f2xFlashMem
flash group (represented by a F2X_GID).  The description of this library
is devoid of the typical "boiler plate" found in the socket driver
examples.  Instead, the focus is on quickly getting TrueFFS running on
flash employing the f2xFlash code suite.

The following is a description of the recommended steps to make one
or more f2xFlash/TrueFFS devices operational:

STEP 0: INSTALL TFFS

STEP 1: GET THE f2xFlash FILES
Copy the files f2xFlashMem.c, f2xFlashMem.h, f2xFlashMtd.c and
f2xFlash-sysTffs.c into the project directory if using the
Tornado Project facility, or the target directory if using make.

STEP 2: INCLUDE f2xFlashMem.o IN THE KERNEL
Add f2xFlashMem.o to the VxWorks kernel either through the Tornado
Project facility or by adding f2xFlashMem.o to the MACH_EXTRA line
of the BSP Makefile.

STEP 3: CREATE VALID FLASH GROUP(S)
The next step is to create f2xFlashMem flash groups that will
later correspond to TrueFFS devices.  At this point, it is often
easiest to experiment by using a script executed from the shell
instead of putting the group creation code into the kernel
initialization sequence.  When finished, be sure the
f2xGroupBlockSize() of each group is not zero (see documentation
for f2xGroupBlockSize() in f2xFlashMem.c).  This is almost always
natural for flash.

STEP 4: TEST THE FLASH GROUPS(S)
TrueFFS needs a solid foundation to build on. Before creating a
TrueFFS device from a flash group, be sure to test the group at the
f2xFlashMem-level to be sure it was created properly.

STEP 5: INCLUDE TFFS IN THE KERNEL
In the routine sysTffsInit() (below), add the group creation code
and call f2xRegister() for each group.  Add the definitions of
INCLUDE_TFFS and INCLUDE_DOSFS to config.h, and add f2xFlashMtd.o
and f2xFlash-sysTffs.o to the kernel (as in step 2).

Note that the first group registered with f2xRegister() is
considered TrueFFS device number 0, the second is device number 1
and so on.

STEP 6: ERASE AND FORMAT THE TFFS DEVICE(S)
Before mounting a TFFS device, it must be erased and formatted.
Do this with the following commands:
.CS
    -> tffsRawio <device number>, 3, 0, <number of blocks>
.CE
and
.CS
    -> tffsDevFormat <device number>, 0
.CE
The number of blocks can be determined by dividing f2xGroupSize()
by f2xGroupBlockSize() for the correspondinng group.

STEP 7: MOUNT THE TFFS DEVICE(S)
To use a TrueFFS device, it must be mounted.  Mounting is 
accomplished with the following command:
.CS
    -> usrTffsConfig <device number>, 0, <mount point>
.CE
here's a real-life example:
.CS
    -> usrTffsConfig 0, 0, "/tffs0"
.CE

SPECIAL CONSIDERATIONS
The f2xFlash philosophy differs from TrueFFS in that it assumes
the flash memory is embedded (it is not removable), always writable,
and the type/geometry of the flash is determined at compile-time or
during kernel initialization (before groups are created). The
f2xFlash philosophy also differs in that an f2xFlash group, and
therefore the f2xFlash MTD, may control only a fraction of a flash
chip or array.

SPECIAL CONSIDERATIONS: MIXING MTDs
Other MTDs may be used with the f2xFlash MTD.  Because 
f2xFlashMtdIdentify() claims all sockets as its own, the identify
routines of any added MTDs must be respresented in the mtdTable[]
(see target/src/drv/tffs/tffsConfig.c) *before* the element
representing f2xFlashMtdIdentify().  Of course, additional socket
driver code is required to support other MTDs.

SPECIAL CONSIDERATIONS: CREATING A BOOT AREA
This code was written to support TrueFFS on 100% of a f2xFlash
group.  If it is desirable to create a "fallow area" at the
beginning of the group (e.g., for boot code), the user is invited
to write a sysTffsFormat() routine similar to those found in the
TrueFFS socket driver examples.
*/

/* includes */

#include "vxWorks.h"
#include "tffs/flsocket.h"
#include "tffs/flflash.h"
#include "tffs/tffsDrv.h"
#include "f2xFlashMem.h"
#include "config.h"

/* defines */

#define	INCLUDE_TL_FTL                        /* FTL translation layer */

#define INCLUDE_MTD_I28F008_BAJA              /* hijack Baja MTD existence */
#define i28f008BajaIdentify f2xFlashMtdIdentify

/* locals */

/* foward declarations */

FLStatus f2xFlashMtdIdentify (FLFlash vol);

/* included source */

#include "tffs/tffsConfig.c"

/*******************************************************************************
*
* f2xCardDetected - stub
*
* RETURNS: TRUE
*/

LOCAL FLBoolean f2xCardDetected
    (
    FLSocket vol
    )
    {
    return (TRUE);
    }

/*******************************************************************************
*
* f2xVccOn - stub
*
* RETURNS: N/A
*/

LOCAL void f2xVccOn
    (
    FLSocket vol
    )
    {
    }

/*******************************************************************************
*
* f2xVccOff - stub
*
* RETURNS: N/A
*/

LOCAL void f2xVccOff
    (
    FLSocket vol
    )
    {
    }

#ifdef SOCKET_12_VOLTS

/*******************************************************************************
*
* f2xVppOn - stub
*
* RETURNS: flOK.
*/

LOCAL FLStatus f2xVppOn
    (
    FLSocket vol
    )
    {
    return (flOK);
    }

/*******************************************************************************
*
* f2xVppOff - stub
*
* RETURNS: N/A
*/

LOCAL void f2xVppOff
    (
    FLSocket vol
    )
    {
    }

#endif	/* SOCKET_12_VOLTS */

/*******************************************************************************
*
* f2xInitSocket - stub
*
* RETURNS: flOK.
*/

LOCAL FLStatus f2xInitSocket
    (
    FLSocket vol
    )
    {
    return (flOK);
    }

/*******************************************************************************
*
* f2xSetWindow - stub
*
* RETURNS: N/A
*/

LOCAL void f2xSetWindow
    (
    FLSocket vol
    )
    {
    }

/*******************************************************************************
*
* f2xSetMappingContext - stub
*
* RETURNS: N/A
*/

LOCAL void f2xSetMappingContext
    (
    FLSocket vol,
    unsigned page
    )
    {
    }

/*******************************************************************************
*
* f2xGetAndClearCardChangeIndicator - stub
*
* RETURNS: FALSE.
*/

LOCAL FLBoolean f2xGetAndClearCardChangeIndicator
    (
    FLSocket vol
    )
    {
    return (FALSE);
    }

/*******************************************************************************
*
* f2xWriteProtected - stub
*
* RETURNS: FALSE.
*/

LOCAL FLBoolean f2xWriteProtected
    (
    FLSocket vol
    )
    {
    return (FALSE);
    }

/*******************************************************************************
*
* flFitInSocketWindow - stub
*
* RETURNS: chipSize
*
* NOMANUAL
*/

long int flFitInSocketWindow
    (
    long int chipSize,
    int      interleaving,
    long int windowSize
    )
    {
    return (chipSize);
    }

/*******************************************************************************
*
* flDelayLoop - stub
*
* RETURNS: N/A
*
* NOMANUAL
*/

void flDelayLoop
    (
    int  cycles
    )
    {
    while (--cycles);
    }

/*******************************************************************************
*
* f2xRegister - install routines for f2xFlash socket
*
* This routine installs necessary functions for the f2xFlash socket.
*
* RETURNS: N/A
*/

LOCAL void f2xRegister
    (
    F2X_GID gid    /* group id */
    )
    {
    FLSocket vol = flSocketOf (noOfDrives);

    tffsSocket[noOfDrives] = "F2X";

    vol.serialNo = (unsigned)gid;
    vol.window.baseAddress = (unsigned int)f2xGroupMap (gid, 0) >> 12;
    vol.cardDetected = f2xCardDetected;
    vol.VccOn = f2xVccOn;
    vol.VccOff = f2xVccOff;
#ifdef SOCKET_12_VOLTS
    vol.VppOn = f2xVppOn;
    vol.VppOff = f2xVppOff;
#endif
    vol.initSocket = f2xInitSocket;
    vol.setWindow = f2xSetWindow;
    vol.setMappingContext = f2xSetMappingContext;
    vol.getAndClearCardChangeIndicator = f2xGetAndClearCardChangeIndicator;
    vol.writeProtected = f2xWriteProtected;

    noOfDrives++;
    }

/*******************************************************************************
*
* sysTffsInit - board level initialization for TFFS
*
* This routine calls the socket registration routines for the socket component
* drivers that will be used with this BSP. The order of registration signifies
* the logical drive number given to the drive associated with the socket.
*
* RETURNS: N/A
*/

LOCAL void sysTffsInit (void)
    {
    /* --- groups should be created in sysHwInit2() and referenced here --- */

    IMPORT F2X_GID sysTffsGid;

    /* --- calls to f2xRegister() go here --- */

    f2xRegister(sysTffsGid);
    }
