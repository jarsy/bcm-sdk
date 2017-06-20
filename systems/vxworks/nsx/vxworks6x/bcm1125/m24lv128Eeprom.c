/* m24lv128Eeprom.c - Microchip eeprom driver */

/* Copyright 2004 Wind River Systems, Inc. */

/* $Id: m24lv128Eeprom.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
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
01b,15nov04,mdo  Documentation fixes for apigen
01a,03oct02,agf  written
*/

/*
DESCRIPTION

This module implements the Microchip serial eeprom access routines
for the BCM91250A Swarm and BCM91250E Sentosa boards

INCLUDE FILES
*/

/* includes */

#include "vxWorks.h"

#include "bcm1250.h"
#include "bcm1250Lib.h"
#include "m24lv128Eeprom.h"
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
* m24lv128EepromOpen - open the EEPROM device
*
* This routine opens the m24lv128 EEPROM.  For the X1240, we do a 
* quick test read to be sure the device is out there.
*
* RETURNS: OK, else ERROR
*
* ERRNO
*/

STATUS m24lv128EepromOpen
    (
    int smbus_channel
    )
    {
    int     b;

    smbusInit (smbus_channel);

    b = smbusReadByte (smbus_channel,
                       M24LV128_SMBUS_DEV,
                       0);

    return ((b == ERROR) ? ERROR : OK);
    }


/**********************************************************************
* m24lv128EepromRead - read bytes from the EEPROM
*
* This routine reads bytes from the EEPROM of a specified length.
*
* RETURNS: OK if the EEPROM was read, ERROR if an error occurred
*
* ERRNO
*/

int m24lv128EepromRead
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

    if ((buf_offset + blen) > M24LV128_EEPROM_SIZE)
        return ERROR;

    idx = (int) buf_offset;

    while (blen > 0)
        {
        b = smbusReadByte (smbus_channel,
                           M24LV128_SMBUS_DEV,
                           idx);
        if (b == ERROR)
            break;

        *bptr++ = (unsigned char) b;
        blen--;
        idx++;
        }

    *buf_retlen = bptr - buf_ptr;
    return ((b == ERROR) ? ERROR : OK);
    }


/*********************************************************************
* m24lv128EepromWrite - write bytes to the EEPROM
*
* This routine writes bytes to the EEPROM of a specified length.
*
* RETURNS: OK if bytes were written, ERROR if an error occurred
*
* ERRNO
*/

int m24lv128EepromWrite
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

    if ((buf_offset + blen) > M24LV128_EEPROM_SIZE)
        return ERROR;

    idx = (int) buf_offset;

    while (blen > 0)
        {
        b = *bptr++;
        b = smbusWriteByte (smbus_channel,
                            M24LV128_SMBUS_DEV,
                            idx,
                            b);
        if (b == ERROR)
            break;

        blen--;
        idx++;
        }

    *buf_retlen = bptr - buf_ptr;
    return ((b == ERROR) ? ERROR : OK);
    }


/**********************************************************************
* m24lv128EepromClose - close the EEPROM device
*
* This routine closes the EEPROM device.
*
* RETURNS: 0 if ok
*
* ERRNO
*/

STATUS m24lv128EepromClose ()
    {
    return OK;
    }
