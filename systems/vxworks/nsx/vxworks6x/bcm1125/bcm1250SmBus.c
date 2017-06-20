/* bcm1250SmBus.c - System Management bus driver */

/* $Id: bcm1250SmBus.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * Copyright (c) 2004-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
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
01d,03aug05,dr   added #if (NV_RAM_SIZE!=NONE) for scalability.
01c,28jul05,rlg  spr 104812 modified timing constraints on write eeprom for
                 bootline saves
01b,15nov04,mdo  Documentation fixes for apigen
01a,03oct02,agf  written
*/

/*
DESCRIPTION

This module implements routines for accessing slave devices on the
bcm1250 SM Bus. Not the entire SM Bus protocol is implemented; just
that which is necessary for dealing with serial EEPROM.

INCLUDE FILES
*/

/* includes */

#include <vxWorks.h>
#include "bcm1250.h"
#include "bcm1250Lib.h"
#include "config.h"


/* defines */

#define TIMER_SET(x,v)      x = ((v)*1000000L)
#define TIMER_EXPIRED(x)    ((x) <= 0)
#define POLL(x)             x--

#if (NV_RAM_SIZE != NONE)
/* externals */


/* forward declarations */


/* globals */


/* locals */

/*******************************************************************************
* smbusInit - initialize the specified SMBus channel
*
* This routine initializes the specified SMBus channel.
*
* RETURNS: N/A
*
* ERRNO
*/

void smbusInit
    (
    int chan
    )
    {
    UINT32   reg;

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_FREQ));

    /*
     * XXX: This device can use 400KHz clock (K_SMB_FREQ_400KHZ),
     * XXX: but until there is a SMBus channel-level driver,
     * XXX: this driver shouldn't use it.  There might be slower
     * XXX: devices on the bus, which may be confused by higher
     * XXX: frequencies than they understand.
     */
    MIPS3_SD (reg, K_SMB_FREQ_100KHZ);                  /* 100KHz clock */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_CONTROL));

    MIPS3_SD (reg, 0);      /* not in direct mode, no interrupts, will poll */

    }


/**********************************************************************
* smbusWaitReady - delay till channel is ready
*
* This routine waits until the SMBus channel is ready.  We simply poll
* the busy bit until it clears.
*
* RETURNS: OK, or ERROR
*
* ERRNO
*/
int smbusWaitReady
    (
    int chan
    )
    {
    UINT32    status = 0;
    volatile UINT64    timer;
    volatile UINT32    reg;

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_STATUS));

    /* allow the SMBus channel up to 100ms to finish being busy */

    TIMER_SET (timer, 100L);
    while (!TIMER_EXPIRED (timer))
        {
        POLL (timer);
        status = MIPS3_LD (reg);
        if (!(status & M_SMB_BUSY))
            break;
        }

    if (status & M_SMB_ERROR)
        {
        MIPS3_SD (reg, (status & M_SMB_ERROR));

#if DEBUG_SM_BUS
        printf ("SMBus channel reported an error: %d\n",
                status & M_SMB_ERROR_TYPE ? 1 : 0);
#endif
        return ERROR;
        }

    return OK;
    }


/**********************************************************************
* smbusReadByte - read a byte from the specified channel
*
* This routine reads a byte from the chip.  The 'slaveaddr' parameter 
* determines whether we are reading from the RTC section or the 
* EEPROM section.
*
* RETURNS: value read, or ERROR
*
* ERRNO
*/

int smbusReadByte
    (
    int chan,
    int slaveaddr,
    int devaddr
    )
    {
    volatile UINT32      reg;
    int         val;

    /*
     * Make sure the bus is idle (probably should
     * ignore error here)
     */

    if (smbusWaitReady (chan) == ERROR)
        return ERROR;

    /*
     * Write the device address to the controller. There are two
     * parts, the high part goes in the "CMD" field, and the
     * low part is the data field.
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_CMD));
    MIPS3_SD (reg, ((devaddr >> 8) & 0x3F));

    /*
     * Write the data to the controller
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_DATA));
    MIPS3_SD (reg, ((devaddr & 0xFF) & 0xFF));

    /*
     * Start the command
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_START));
    MIPS3_SD (reg, V_SMB_TT (K_SMB_TT_WR2BYTE) | slaveaddr);

    /*
     * Wait till done
     */
    if (smbusWaitReady (chan) == ERROR)
        return ERROR;

    /*
     * Read the data byte
     */

    MIPS3_SD (reg, V_SMB_TT (K_SMB_TT_RD1BYTE) | slaveaddr);

    if (smbusWaitReady (chan) == ERROR)
        return ERROR;

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_DATA));
    val = MIPS3_LD (reg);

    return (val & 0xFF);
    }


/**********************************************************************
* smbusWriteByte - write a byte from the specified channel
*
* This routine writes a byte to the chip.  The 'slaveaddr' parameter 
* determines whether we are writing to the RTC section or the 
* EEPROM section.
*
* RETURNS: OK, or ERROR
*
* ERRNO
*/

int smbusWriteByte
    (
    int chan,
    int slaveaddr,
    int devaddr,
    int b
    )
    {
    volatile UINT32      reg;
    int         err;
    int         tries;
    int         loop;

    /*
     * Make sure the bus is idle (probably should
     * ignore error here)
     */

    if (smbusWaitReady (chan) == ERROR)
        return ERROR;

    /*
     * Write the device address to the controller. There are two
     * parts, the high part goes in the "CMD" field, and the
     * low part is the data field.
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_CMD));
    MIPS3_SD (reg, ((devaddr >> 8) & 0x3F));

    /*
     * Write the data to the controller
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_DATA));
    MIPS3_SD (reg, ((devaddr & 0xFF) | ((b & 0xFF) << 8)));

    /*
     * Start the command.  Keep pounding on the device until it
     * submits or the timer expires, whichever comes first.  The
     * datasheet says writes can take up to 10ms, so we'll give it
     * 5 'waitready' tries.
     */

    reg = PHYS_TO_K1 (A_SMB_REGISTER (chan, R_SMB_START));
    MIPS3_SD (reg, V_SMB_TT (K_SMB_TT_WR3BYTE) | slaveaddr);

    /*
     * Wait till the SMBus interface is done
     */

    if (smbusWaitReady (chan) < 0)
        return ERROR;

    /*
     * Pound on the device with a current address read
     * to poll for the write complete
     */

    tries = 100;
    err = ERROR;

    while (tries > 0)
        {
        TIMER_SET (loop, 100L);
        while (!TIMER_EXPIRED(loop))
            {
            POLL(loop);
            }

        tries--;

        MIPS3_SD (reg, V_SMB_TT (K_SMB_TT_QUICKCMD) | slaveaddr);

        err = smbusWaitReady (chan);
        if (err == OK)
            break;
        }

    return err;
    }
#endif	/* NV_RAM_SIZE != NONE */
