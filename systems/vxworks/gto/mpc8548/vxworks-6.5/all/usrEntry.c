/* usrEntry.c - user entry point for compressed images */

/* $Id: usrEntry.c,v 1.2 2011/07/21 16:14:20 yshtil Exp $
 * Copyright (c) 1984-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,06sep05,h_k  added gp setting for MIPS. (SPR #111954)
01a,02jun98,ms   written
*/

/*
DESCRIPTION
The routine usrEntry() is the entry point for compressed images.
The decompression stub inflates the main VxWorks or bootrom code into RAM,
and then jumps to it. This is the entry point (i.e., the first address).
It is important not to add any other routines before this one.
*/

#include <vxWorks.h>
#include <usrConfig.h>

/******************************************************************************
*
* usrEntry - entry point for vxWorks_romCompress, vxWorks_rom and bootrom
*            images.
*
* This routine is the entry point after the bootroms and vxWorks romable
* images decompress, if compression is utilized.
* This routine must be the first item of the
* text segment of this file.  With ANSI C, strings will appear in text
* segment so one should avoid entering strings, routines, or anything
* else that might displace this routine from base of the text segment.
*
* It is unwise to add functionality to this routine without due cause.
* We are in the prehistoric period of system initialization.
*
* NOMANUAL
*/ 

void usrEntry (int startType)
    {
#if     (CPU_FAMILY==I960)
    sysInitAlt (startType);             /* jump to the i960 entry point */
#endif

#if (CPU_FAMILY==MIPS)
    WRS_ASM (".extern _gp; la $gp,_gp");
#endif

    usrInit (startType);                /* all others procede below */
    }

