/* x1240RtcEeprom.c - X1240 real-time clock & eeprom driver */

/* Copyright 2004 Wind River Systems, Inc. */

/* $Id: x1240RtcEeprom.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
**********************************************************************
*/

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01f,08nov04,mdo  Documentation fixes for apigen
01e,03oct02,agf  changes for shared sentosa support
01d,20jun02,pgh  Change path to bcm1250Lib.h.
01c,13jun02,pgh  Fix SPR 76014 and SPR 76016.
01b,26mar02,tlc  Clean up compiler warnings.
01a,08jan01,agf  written
*/

/*
DESCRIPTION

This module implements the X1240 real-time clock and eeprom access routines
for the BCM91250A Swarm and BCM91250E Sentosa boards

INCLUDE FILES:
*/

/* includes */

#include "vxWorks.h"

#include "bcm1250.h"
#include "bcm1250Lib.h"
#include "x1240RtcEeprom.h"
#if defined(BCM1250_SWARM)
#include "swarm.h"
#elif defined(BCM1250_SENTOSA)
#include "sentosa.h"
#endif


/* defines */


/* externals */


/* forward declarations */

void smbusInit (int );
int smbusWaitReady (int );
int smbusReadByte (int, int, int );
int smbusWriteByte (int, int, int, int );


/* globals */


/* locals */


/**********************************************************************
* x1240RtcEepromOpen - Open EEPROM device
*
* This routine opens the EEPROM device.  For the X1240, we do a quick
* test read to be sure the device is out there.
*
* RETURNS: OK or ERROR
*
* ERRNO
*/

STATUS x1240RtcEepromOpen
    (
    int smbus_channel
    )
    {
    int     b;
    int     retries;

    /*
     * Try to read byte 0 from the device.  If it does not
     * respond, fail the open.  Give it a few tries (up to
     * 300ms) in case the X1240 is busy wiggling its
     * RESET line.
     */

    smbusInit (smbus_channel);

    retries = 3;
    while (retries > 0)
        {
        retries--;
        b = smbusReadByte (smbus_channel,
                            X1241_ARRAY_ADDRESS,
                            0);
        if (b != ERROR)
            break;              /* read is ok */
        }

    if (retries == 0)
       return  ERROR;

    /*
     * See if the watchdog is enabled.  If it is, turn it off.
     */

    b = smbusReadByte (smbus_channel,
                        X1241_CCR_ADDRESS,
                        X1241REG_BL);

    if (b != (X1241REG_BL_WD1 | X1241REG_BL_WD0))
        {
        (void) smbusWriteByte (smbus_channel,
                         X1241_CCR_ADDRESS,
                         X1241REG_SR,
                         X1241REG_SR_WEL);

        (void) smbusWriteByte (smbus_channel,
                         X1241_CCR_ADDRESS,
                         X1241REG_SR,
                         X1241REG_SR_WEL | X1241REG_SR_RWEL);

        (void) smbusWriteByte (smbus_channel,
                         X1241_CCR_ADDRESS,
                         X1241REG_BL,
                         (X1241REG_BL_WD1 | X1241REG_BL_WD0));

        (void) smbusWriteByte (smbus_channel,
                         X1241_CCR_ADDRESS,
                         X1241REG_SR,
                         0);
        }

    return OK;
    }


/**********************************************************************
* x1240RtcEepromRead - read from the EEPROM
*
* This routine reads the specified number of bytes from the EEPROM.
*
* RETURNS: OK if the EEPROM was read, ERROR if an error occurred
*
* ERRNO
*/

int x1240RtcEepromRead
    (
    int             smbus_channel,
    unsigned int    buf_offset,
    unsigned char * buf_ptr,
    int             buf_length,
    int *           buf_retlen
    )
    {
    unsigned char * bptr;
    int             blen;
    int             idx;
    int             b = 0;

    bptr = buf_ptr;
    blen = buf_length;

    if ((buf_offset + blen) > X1241_EEPROM_SIZE)
        return -1;

    idx = (int) buf_offset;

    while (blen > 0)
        {
        b = smbusReadByte (smbus_channel,
                            X1241_ARRAY_ADDRESS,
                            idx);
        if (b == ERROR)
            break;

        *bptr++ = (unsigned char) b;
        blen--;
        idx++;
        }

    *buf_retlen = bptr - buf_ptr;
    return (b == ERROR) ? ERROR : OK;
    }


/*********************************************************************
* x1240RtcEepromWrite - write to the EEPROM
*
* This routine writes the specified number of bytes to the EEPROM.
*
* RETURNS: OK if bytes were written, ERROR if an error occurred
*
* ERRNO
*/

int x1240RtcEepromWrite
    (
    int             smbus_channel,
    unsigned int    buf_offset,
    unsigned char * buf_ptr,
    int             buf_length,
    int *           buf_retlen
    )
    {
    unsigned char * bptr;
    int             blen;
    int             idx;
    int             b = 0;

    bptr = buf_ptr;
    blen = buf_length;

    if ((buf_offset + blen) > X1241_EEPROM_SIZE)
        return -1;

    idx = (int) buf_offset;

    b = smbusWriteByte (smbus_channel,
                         X1241_CCR_ADDRESS,
                         X1241REG_SR,
                         X1241REG_SR_WEL);
    if (b == ERROR)
        return ERROR;

    while (blen > 0)
        {
        b = *bptr++;
        b = smbusWriteByte (smbus_channel,
                             X1241_ARRAY_ADDRESS,
                             idx,
                             b);
        if (b == ERROR)
            break;

        blen--;
        idx++;
        }

    (void) smbusWriteByte (smbus_channel,
                     X1241_CCR_ADDRESS,
                     X1241REG_SR,
                     0);

    *buf_retlen = bptr - buf_ptr;
    return (b == ERROR) ? ERROR : OK;
    }


/**********************************************************************
* x1240RtcEepromClose - close the EEPROM device
*
* This routine closes the EEPROM device.
*
* RETURNS: OK, always
*
* ERRNO
*/

STATUS x1240RtcEepromClose (void)
    {
    return OK;
    }
