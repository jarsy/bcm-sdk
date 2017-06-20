/* sysMtd.c - MTD driver for flash(s) on wrSbc8548 */

/* $Id: sysMtd.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,30mar07,b_m  written.
*/

/*
DESCRIPTION
This file provides the TFFS MTD driver for the 4 banks of 8-bit Intel V28F128J3
flash on the wrSbc8548 board. The driver handles the low level operations like
erase and program of the flash. It also provides an identify routine to check if
it is compatible with the devices.

The macros DEBUG_PRINT must be defined as 'printf' to provide informative debug
messages if necessary.

The driver is only compatible and tested with wrSbc8548 board. DO NOT use this
driver on any other boards unless the flashes are absolutely the same.

INCLUDE FILES:

SEE ALSO:
*/

#include <stdio.h>
#include "tffs/flflash.h"
#include "tffs/backgrnd.h"

/* defines */

#undef  DEBUG_PRINT

#define INTEL_VENDOR_ID         0x89

#define FLASH_4M_DEV_ID         0x16
#define FLASH_8M_DEV_ID         0x17
#define FLASH_16M_DEV_ID        0x18
#define FLASH_32M_DEV_ID        0x1D

#define VENDOR_ID_INTERLEAVED   0x89898989
#define DEVICE_ID_INTERLEAVED   0x18181818

#define V28F128J3_VENDOR_OFFSET 0
#define V28F128J3_DEVICE_OFFSET 2

#define V28F128J3_FLASH_ID      ((INTEL_VENDOR_ID << 8) | FLASH_16M_DEV_ID)

#define V28F128J3_FLASH_SIZE    0x01000000
#define V28F128J3_FLASH_NUM     4
#define V28F128J3_SECTOR_SIZE   0x20000

#define V28F128J3_CMD_RESET             0xFFFFFFFF
#define V28F128J3_CMD_READ_STATUS       0x70707070
#define V28F128J3_CMD_CLEAR_STATUS      0x50505050
#define V28F128J3_CMD_ERASE_BLOCK       0x20202020
#define V28F128J3_CMD_ERASE_CONFIRM     0xD0D0D0D0
#define V28F128J3_CMD_PROGRAM           0x10101010
#define V28F128J3_CMD_READ_ID           0x90909090

#define V28F128J3_STATUS_READY          0x80808080
#define V28F128J3_STATUS_READY_MASK     0x80808080
#define V28F128J3_STATUS_ERA_ERR_MASK   0x20202020
#define V28F128J3_STATUS_PRG_ERR_MASK   0x10101010
#define V28F128J3_STATUS_ERR_MASK       (V28F128J3_STATUS_ERA_ERR_MASK | V28F128J3_STATUS_PRG_ERR_MASK)

#define V28F128J3_OP_TIMEOUT            5000

#define V28F128J3_RESET(addr)   \
    do  \
    {   \
        *addr = V28F128J3_CMD_RESET;    \
    } while(0);

#define V28F128J3_READ_STATUS(addr) \
    do  \
    {   \
        *addr = V28F128J3_CMD_READ_STATUS;  \
    } while(0);

#define V28F128J3_CLEAR_STATUS(addr)    \
    do  \
    {   \
        *addr = V28F128J3_CMD_CLEAR_STATUS;   \
    } while(0);

#define V28F128J3_ERASE_BLOCK(addr) \
    do  \
    {   \
        *addr = V28F128J3_CMD_ERASE_BLOCK;  \
    } while(0);

#define V28F128J3_ERASE_CONFIRM(addr)   \
    do  \
    {   \
        *addr = V28F128J3_CMD_ERASE_CONFIRM;    \
    } while(0);

#define V28F128J3_PROGRAM(addr, value)  \
    do  \
    {   \
        *addr = V28F128J3_CMD_PROGRAM;    \
        *addr = value;  \
    } while(0);

#define V28F128J3_READ_ID(addr) \
    do  \
    {   \
        *addr = V28F128J3_CMD_READ_ID;  \
    } while(0);


/*******************************************************************************
*
* v28F128J3Program - low level byte programming routine
*
*/

LOCAL FLStatus v28F128J3Program
    (
    UINT32 *    addr,
    UINT32      value
    )
    {
    UINT32  status = V28F128J3_STATUS_READY;
    UINT32  timeout = 0;

    /* set timeout = 5s */
    timeout = flMsecCounter + V28F128J3_OP_TIMEOUT;
    V28F128J3_READ_STATUS(addr);
    do
    {
        status = *addr;
        if (flMsecCounter >= timeout)
            break;
    } while ((status & V28F128J3_STATUS_READY_MASK) != V28F128J3_STATUS_READY);

    if ((status & V28F128J3_STATUS_ERR_MASK) != 0)
        V28F128J3_CLEAR_STATUS(addr);

    V28F128J3_PROGRAM(addr, value);

    /* set timeout = 5s */
    timeout = flMsecCounter + V28F128J3_OP_TIMEOUT;
    do
    {
        status = *addr;
        if (flMsecCounter >= timeout)
        {
            V28F128J3_RESET(addr);
            return flTimedOut;
        }
    } while ((status & V28F128J3_STATUS_READY_MASK) != V28F128J3_STATUS_READY);

    /* check program error bit */
    if (status & V28F128J3_STATUS_PRG_ERR_MASK)
    {
        V28F128J3_RESET(addr);
        return flWriteFault;
    }

    V28F128J3_RESET(addr);
    return flOK;
    }

/*******************************************************************************
*
* v28F128J3Write - write routine for v28F128J3 flash
*
*/

LOCAL FLStatus v28F128J3Write
    (
    FLFlash         vol,
    CardAddress     address,
    const void FAR1 *buffer,
    int             length,
    int             overwrite
    )
    {
    UINT8  *unaligned;
    UINT8  *buf = (UINT8 *)buffer;
    UINT32  left = length;
    UINT32 *aligned;
    UINT32  data, num;
    int     i;

    if (flWriteProtected(vol.socket))
	    return flWriteProtect;

    /* calculate the program addr, make sure it's 32-bit aligned */
    unaligned = (UINT8 *)vol.map (&vol, address, 0);
    num = (UINT32)unaligned & 0x3;
    aligned = (UINT32 *)((UINT32)unaligned - num);

    if (num != 0)
    {
        data = *aligned;

        for (i = num ; i < 4; i++)
        {
            data &= ~(0xFF << ((3 - i) * 8));
            data |= ((*(buf + i - num)) << ((3 - i) * 8));
        }

        if (v28F128J3Program(aligned, data) != flOK)
            return flWriteFault;

        buf  += (4 - num);
        left -= (4 - num);
        aligned++;
    }

    while (left >= 4)
    {
        data = *(UINT32 *)buf;

        if (v28F128J3Program (aligned, data) != flOK)
            return flWriteFault;

        buf  += 4;
        left -= 4;
        aligned++;
    }

    if (left > 0)
    {
        data = *aligned;

        for (i = 0 ; i < left; i++)
        {
            data &= ~(0xFF << ((3 - i) * 8));
            data |= ((*(buf + i)) << ((3 - i) * 8));
        }

        if (v28F128J3Program (aligned, data) != flOK)
            return flWriteFault;
    }

    if (tffscmp((void FAR0 *)unaligned, buffer, length))
    {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("[v28F128J3Write]: data double check error @ 0x%08x ...\n", unaligned);
#endif
        return flWriteFault;
    }

    return flOK;
    }

/*******************************************************************************
*
* v28F128J3Erase - erase routine for v28F128J3 flash
*
*/

LOCAL FLStatus v28F128J3Erase
    (
    FLFlash vol,
    int     firstErasableBlock,
    int     numOfErasableBlocks
    )
    {
    UINT32 * block = NULL;
    UINT32 status = V28F128J3_STATUS_READY;
    UINT32 timeout = 0;
    int i;

    if (flWriteProtected(vol.socket))
	    return flWriteProtect;

    for (i = firstErasableBlock; i < firstErasableBlock + numOfErasableBlocks; i++)
	{
	    block = (UINT32 *)vol.map(&vol, i * vol.erasableBlockSize, 0);

#ifdef DEBUG_PRINT
        DEBUG_PRINT("Erasing block#%03d @ 0x%08x ...\r", i, block);
#endif

    	/* set timeout = 5s */
    	timeout = flMsecCounter + V28F128J3_OP_TIMEOUT;
        V28F128J3_READ_STATUS(block);
        do
        {
            status = *block;
            if (flMsecCounter >= timeout)
                break;
        } while ((status & V28F128J3_STATUS_READY_MASK) != V28F128J3_STATUS_READY);

        if ((status & V28F128J3_STATUS_ERR_MASK) != 0)
            V28F128J3_CLEAR_STATUS(block);

        V28F128J3_ERASE_BLOCK(block);
        V28F128J3_ERASE_CONFIRM(block);

    	/* set timeout = 5s */
    	timeout = flMsecCounter + V28F128J3_OP_TIMEOUT;
        do
        {
            status = *block;
            if (flMsecCounter >= timeout)
            {
                V28F128J3_RESET(block);
                return flTimedOut;
            }
        } while ((status & V28F128J3_STATUS_READY_MASK) != V28F128J3_STATUS_READY);

        /* check erase error bit */
    	if (status & V28F128J3_STATUS_ERA_ERR_MASK)
        {
            V28F128J3_RESET(block);
    		return flWriteFault;
        }
	}

    V28F128J3_RESET(block);
    return flOK;
    }


/*******************************************************************************
*
* v28F128J3Identify - identify routine for v28F128J3 flash
*
*/

FLStatus v28F128J3Identify
    (
    FLFlash vol
    )
    {
    UINT32 * base = (UINT32 *)vol.map(&vol, 0, 0);
    UINT32   vendor, device;

#ifdef DEBUG_PRINT
    DEBUG_PRINT("Entering v28F128J3Identify routine @ base address 0x%08x ...\n", (UINT32)base);
#endif

    /* check the flash id */
    V28F128J3_READ_ID(base);
    vendor = *(base + V28F128J3_VENDOR_OFFSET);
    device = *(base + V28F128J3_DEVICE_OFFSET);
    if ((vendor == VENDOR_ID_INTERLEAVED) && (device == DEVICE_ID_INTERLEAVED))
        vol.type = V28F128J3_FLASH_ID;
    else
    {
        V28F128J3_RESET(base);
        return flUnknownMedia;
    }

    vol.chipSize = V28F128J3_FLASH_SIZE;
    vol.noOfChips = V28F128J3_FLASH_NUM;
    vol.interleaving = V28F128J3_FLASH_NUM;
    vol.erasableBlockSize = V28F128J3_SECTOR_SIZE * vol.interleaving;
    vol.write = v28F128J3Write;
    vol.erase = v28F128J3Erase;

    V28F128J3_RESET(base);
    return flOK;
    }

