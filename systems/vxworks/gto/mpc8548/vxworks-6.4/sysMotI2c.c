/* sysMotI2c.c - I2C Driver Source Module */

/* $Id: sysMotI2c.c,v 1.5 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2000, 2002, 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* Copyright 1996-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01e,27jan06,dtr  Tidy up.
01d,10nov05,mdo  Documentation fixes for apigen
01c,21may02,gtf  modified for Raytheon NetFires bsp.
01b,10mar00,rhk  changed over to use sysCalcBusSpd, removed 100MHz speed.
01a,28feb00,rhk  created from version 01d, MV2100 BSP.
*/

/*
DESCRIPTION

This file contains generic functions to read/write an I2C device.
Currently the file only supports the MPC8548 I2C interface.
However, additional I2C bus controllers can be easily be added
as required. This dirver doesn't support mutiple threads/tasks and
therfore the user should control access to driver via mutex if that
is reqd.

INCLUDE FILES: sysMotI2c.h
*/


/* includes */

#include <vxWorks.h>		/* vxWorks generics */
#include "config.h"
#include <ioLib.h>			/* input/output generics */
#include <blkIo.h>			/* block input/output specifics */
#include <semLib.h>			/* semaphore operations */
#include <cacheLib.h>			/* cache control */
#include <intLib.h>			/* interrupt control */
#include <semLib.h>

#include "sysMotI2c.h"		/* driver specifics */
#include "sysMpc85xxI2c.h"	/* Mpc85xx I2C Driver Header Module */
#include <stdio.h>
#include <logLib.h>
#include <stdlib.h>

/* defines */
#undef I2C_DRIVER_DEBUG

/* externals */

#ifdef I2C_DRIVER_TESTS
IMPORT int printf();		/* formatted print */
#endif

IMPORT int  sysClkRateGet();	/* system clock rate */
IMPORT int  rawFsDevInit();	/* raw file system device init */
IMPORT void sysMpc85xxMsDelay (UINT mSeconds);

/* locals */

LOCAL UINT8 i2cInByte (UINT32);
LOCAL void i2cOutByte (UINT32,UINT8);


/* Driver/controller routines table for the Mpc85xx device. */
i2cDrvRoutines_t i2cDrvRoutinesTableMpc85xx =
{
	(int(*)())i2cCycleMpc85xxStart,
	(int(*)())i2cCycleMpc85xxStop,
	(int(*)())i2cCycleMpc85xxRead,
	(int(*)())i2cCycleMpc85xxWrite,
	(int(*)())i2cCycleMpc85xxAckIn,
	(int(*)())i2cCycleMpc85xxAckOut,
	(int(*)())i2cCycleMpc85xxKnownState,
	(void(*)())sysMpc85xxMsDelay
};

/* driver/controller routines table, indexed by the "I2C_DRV_TYPE". */

i2cDrvRoutines_t *i2cDrvRoutinesTables[] =
{
	(i2cDrvRoutines_t *)&i2cDrvRoutinesTableMpc85xx	/* index 0 */
};

I2C_DRV_CTRL i2C1DrvCtrl ;
I2C_DRV_CTRL i2C2DrvCtrl ;
I2C_DRV_CTRL * pI2cDrvCtrl[2] = { NULL, NULL } ;


/**************************************************************************
*
* i2cIoctl - perform I2C read/write operations
*
* The purpose of this function is to perform i2cIn/OutByte operations
* with synchronization.
*
* RETURNS: data written or data read
*
* ERRNO: N/A
*/

UINT8 i2cIoctl
    (
    UINT32 ioctlflg,  /* input/output control flag */
		      /* 0, write */
		      /* 1, read */
		      /* 2, read/modify/write (ORing) */
		      /* 3, read/modify/write (ANDing) */
		      /* 4, read/modify/write (AND/ORing) */
    UINT32  address,	  /* address of device register to be operated upon */
    UINT8   bdata1,	  /* data item 1 for read/write operation */
    UINT8   bdata2	  /* data item 2 for read/write operation */
    )
    {
    UINT8 u8temp;

#ifdef I2C_DRIVER_DEBUG
    logMsg("i2cIoctl: adrs - 0x%x.\n", address,2,3,4,5,6);
#endif

    sysMpc85xxMsDelay(1);

    if ( ioctlflg == I2C_IOCTL_WR )	/* write */
	{
 	i2cOutByte(address, bdata1);
   	}
    else if ( ioctlflg == I2C_IOCTL_RD ) /* read */
	{
	bdata1 = i2cInByte(address);
	}
    else if ( ioctlflg == I2C_IOCTL_RMW_OR ) /* ORing */
	{
	u8temp = i2cInByte(address);
	u8temp |= bdata1;
	sysMpc85xxMsDelay(1);

	i2cOutByte(address, u8temp);
	}
    else if ( ioctlflg == I2C_IOCTL_RMW_AND ) /* ANDing */
	{
	u8temp = i2cInByte(address);
	u8temp &= bdata1;
	sysMpc85xxMsDelay(1);

	i2cOutByte(address, u8temp);
	}
    else if ( ioctlflg == I2C_IOCTL_RMW_AND_OR ) /* AND/ORing */
	{
	u8temp = i2cInByte(address);
	u8temp &= bdata1;
	u8temp |= bdata2;
	sysMpc85xxMsDelay(1);

	i2cOutByte(address, u8temp);
	}

    sysMpc85xxMsDelay(1);

    return(bdata1);
    }


/***************************************************************************
*
* i2cDrvInit - initialize I2C device controller
*
* This function initializes the I2C device controller device for operation.
* This function should only be executed once during system initialization
* time.
*
* NOTE: no printf or logMsg statements should be called during this routine
* since it is called in sysHwInit(). If output is desired a polled or debug
* dump routine should be used.
*
* RETURNS: OK
*
* ERRNO: N/A
*/

STATUS i2cDrvInit
    (
    int unit,
    int i2cControllerType		/* I2C controller type */
    )
    {
    /*
     * Check for unknown controller type, and initialize I2C controller
     * for operation (if needed).  Note: a switch statement does not work here if
     * executing from ROM due to branch history table creation.
     */

    if (pI2cDrvCtrl[unit] == NULL)
        {
        if (unit == 0)
            {
            pI2cDrvCtrl[unit] = &i2C1DrvCtrl ;
            pI2cDrvCtrl[unit]->baseAdrs = M85XX_I2C1_BASE ;
            }
        else if (unit == 1)
            {
            pI2cDrvCtrl[unit] = &i2C2DrvCtrl ;
            pI2cDrvCtrl[unit]->baseAdrs = M85XX_I2C2_BASE ;
            }

        pI2cDrvCtrl[unit]->baseAdrs += CCSBAR ;
        }


    /* disable the I2C module, set the device to Master Mode */
    i2cIoctl(I2C_IOCTL_RMW_AND_OR,
	     (UINT32)(pI2cDrvCtrl[unit]->baseAdrs + MPC85XX_I2C_CONTROL_REG),
	     ((UINT8)~MPC85XX_I2C_CONTROL_REG_MEN),
	     MPC85XX_I2C_CONTROL_REG_MSTA);

    /* initialize and enable the I2C interface */

    /*
     * I2C1 - clock = csb_clk / 3. = 111MHz. -> need to get to 100KHz. -> div by 1024.
     *                gives 100+KHz.
     * I2C2 - clock = csb_clk. = SYS_CLK_FREQ. -> div by 3072. gives 100+KHz.
     * The freq div register needs to be set such that the I2C bus is 100KHz...
     */
    {
    UINT8 divider ;

    divider = 0x31; /* 3072 */

    i2cIoctl(I2C_IOCTL_RMW_AND_OR,
	     (UINT32)(pI2cDrvCtrl[unit]->baseAdrs + MPC85XX_I2C_FREQ_DIV_REG),
	     ((UINT8)~MPC85XX_I2C_FREQ_DIV_REG_MASK),
	     divider);

    /* 0x20 - div by 160 (~300KHz), 0x24 - div by 320 (~150KHz)*/

    /* set the slave address */
    i2cIoctl(I2C_IOCTL_RMW_AND_OR,
	     (UINT32)(pI2cDrvCtrl[unit]->baseAdrs+MPC85XX_I2C_ADR_REG),
	     ((UINT8)~MPC85XX_I2C_ADDRESS_REG_MASK), 1);

    /* enable the interface */
    i2cIoctl(I2C_IOCTL_RMW_OR,
	     (UINT32)(pI2cDrvCtrl[unit]->baseAdrs+MPC85XX_I2C_CONTROL_REG),
	     MPC85XX_I2C_CONTROL_REG_MEN, 0);

    /*
     * set the device to slave mode.  This is required for
     * clearing a BUS BUSY lockup condition.
     */

    i2cIoctl(I2C_IOCTL_RMW_AND,
	     (UINT32)(pI2cDrvCtrl[unit]->baseAdrs+MPC85XX_I2C_CONTROL_REG),
	     ((UINT8)~MPC85XX_I2C_CONTROL_REG_MSTA), 0);
    }

    return(OK);
    }

/******************************************************************************
*
* i2cRead - read blocks from I2C
*
* This function's purpose is to read the specified number of
* blocks from the specified device.
*
* RETURNS: OK, or ERROR on a bad request
*
* ERRNO: N/A
*/

int i2cRead
    (
    int          unit,
    UINT32       deviceAddress,	/* Device's I2C bus address */
    int          deviceType,
    unsigned int startBlk,	/* starting block to read or register to read */
    unsigned int numBlks,	/* number of blocks to read or single/double byte register */
    char *       pBuf				/* pointer to buffer to receive data */
    )
    {
    int localStatus;		/* local status variable */
    i2cCmdPckt_t i2cCmdPacket;	/* command packet */

    /* Check to see if the driver's been installed */

    if(pI2cDrvCtrl[unit]->baseAdrs == 0)
        {
	logMsg("I2C driver for unit %d not initialized.\n", unit,2,3,4,5,6);
        return ERROR ;
        }


    /* Check for a bad request. */

    if ( !numBlks )
	{
	return(ERROR);
	}

    /* Build command packet. */

    i2cCmdPacket.command = I2C_READOP;
    i2cCmdPacket.status = 0;
    i2cCmdPacket.memoryAddress = (unsigned int)pBuf;
    i2cCmdPacket.blockNumber = startBlk;
    i2cCmdPacket.nBlocks = numBlks;
    i2cCmdPacket.eCount = numBlks;
    i2cCmdPacket.aCount = 0;
    i2cCmdPacket.deviceType = deviceType ;

    localStatus = i2cDoOp(unit, deviceAddress, (i2cCmdPckt_t *)&i2cCmdPacket);

#ifdef I2C_DRIVER_DEBUG
    logMsg("command          =%08X\r\n", i2cCmdPacket.command,2,3,4,5,6);
    logMsg("status           =%08X\r\n", i2cCmdPacket.status,2,3,4,5,6);
    logMsg("memory address   =%08X\r\n", i2cCmdPacket.memoryAddress,2,3,4,5,6);
    logMsg("block number     =%08X\r\n", i2cCmdPacket.blockNumber,2,3,4,5,6);
    logMsg("number of blocks =%08X\r\n", i2cCmdPacket.nBlocks,2,3,4,5,6);
    logMsg("expected count   =%08X\r\n", i2cCmdPacket.eCount,2,3,4,5,6);
    logMsg("actual count     =%08X\r\n", i2cCmdPacket.aCount,2,3,4,5,6);
#endif

    /* Return the appropriate status. */

    if ( i2cCmdPacket.status != 0 )
	{
	logMsg("i2cCmdPacket.status - 0x%x\n",i2cCmdPacket.status,2,3,4,5,6);
	localStatus = ERROR ;
	}
    else
	localStatus	= OK ;

    return(localStatus);
    }

/***************************************************************************
*
* i2cWrite - read blocks from I2C
*
* This function purpose is to write the specified number of
* blocks to the specified device.
*
* RETURNS: OK, or ERROR
*
* ERRNO: N/A
*/

int i2cWrite
    (
    int          unit,
    UINT32       deviceAddress,	/* Device's I2C bus address */
    int          deviceType,
    unsigned int startBlk,	/* starting block to write */
    unsigned int numBlks,	/* number of blocks to write */
    char *       pBuf			/* pointer to buffer of send data */
    )
    {
    int localStatus;		/* local status variable */

    i2cCmdPckt_t i2cCmdPacket;	/* command packet */

    /* Check to see if the driver's been installed */

    if(pI2cDrvCtrl[unit]->baseAdrs == 0)
        {
	logMsg("I2C driver for unit %d not initialized.\n", unit,2,3,4,5,6);
        return ERROR ;
        }


    /* Check for a NOP request. */

    if ( !numBlks )
	{
	return(ERROR);
	}

    /* Build command packet. */

    i2cCmdPacket.command = I2C_WRITOP;
    i2cCmdPacket.status = 0;
    i2cCmdPacket.memoryAddress = (unsigned int)pBuf;
    i2cCmdPacket.blockNumber = startBlk;
    i2cCmdPacket.nBlocks = numBlks;
    i2cCmdPacket.eCount = numBlks;
    i2cCmdPacket.aCount = 0;
    i2cCmdPacket.deviceType = deviceType ;

    /* Take ownership, call driver, release ownership. */

    localStatus = i2cDoOp(unit, deviceAddress, (i2cCmdPckt_t *)&i2cCmdPacket);

#ifdef I2C_DRIVER_DEBUG
    logMsg("command          =%08X\r\n", i2cCmdPacket.command,2,3,4,5,6);
    logMsg("status           =%08X\r\n", i2cCmdPacket.status,2,3,4,5,6);
    logMsg("memory address   =%08X\r\n", i2cCmdPacket.memoryAddress,2,3,4,5,6);
    logMsg("block number     =%08X\r\n", i2cCmdPacket.blockNumber,2,3,4,5,6);
    logMsg("number of blocks =%08X\r\n", i2cCmdPacket.nBlocks,2,3,4,5,6);
    logMsg("expected count   =%08X\r\n", i2cCmdPacket.eCount,2,3,4,5,6);
    logMsg("actual count     =%08X\r\n", i2cCmdPacket.aCount,2,3,4,5,6);
#endif

    /* Return the appropriate status. */

    if ( i2cCmdPacket.status != 0 )
	{
	logMsg("i2cCmdPacket.status - 0x%x\n",i2cCmdPacket.status,2,3,4,5,6);
	localStatus = ERROR ;
	}
    else
	localStatus	= OK ;

    return localStatus ;
    }

/*************************************************************************
*
* i2cDoOpPCA9555 - execute commands to the PCA9555 I2C device
*
* This function executes the command operations specified by the command
* packet.
*
* Writing read contents of register, mask bit positions and OR in data
* and write out. Writes only occur to output ports and config regs.
* Reading only from input ports and config regs.
*
* RETURNS: OK, or ERROR
*
* ERRNO: N/A
*/

int i2cDoOpPCA9555
    (
    int             unit,
    UINT32          deviceAddress,    /* device I2C bus address */
    i2cCmdPckt_t *  pI2cCmdPacket     /* pointer to command packet */
    )
    {
    i2cDrvRoutines_t *pRoutine;	/* low level routines table pointer */
    int byteCount;		/* byte counter */
    int statusVariable;		/* local status variable */
    unsigned char *pWriteData;	/* pointer to write data buffer */

    /* Initialize pointer to driver routines table. */
    pRoutine = i2cDrvRoutinesTables[I2C_DRV_TYPE];

    /*
     * read operation,
     *
     */
    if ( pI2cCmdPacket->command == I2C_READOP )
	{
	/* Read the specified number of bytes from the EEPROM. */
	/* NOTE: random read has a dummy write first. */
	statusVariable = 0;

	if ( I2C_KNOWN_STATE(unit) )
	    {
            pI2cCmdPacket->status = I2C_ERROR_KNOWN_STATE;
            return ERROR ;
	    }
        if ( I2C_CYCLE_START(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START ;
            return ERROR ;
            }
        /* device address - write, R/W bit = 0  */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
            }

        if ( I2C_CYCLE_ACKIN(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN ;
            return ERROR ;
            }
        /* command byte (register #) */
        if ( I2C_CYCLE_WRITE( unit, (pI2cCmdPacket->blockNumber&0x7) ) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
            }
        if ( I2C_CYCLE_ACKIN(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
            }
        if ( I2C_CYCLE_START(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START;

            return ERROR ;

            }
        /* device address - read, R/W bit = 1 */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)|0x1) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;

            return ERROR ;
            }
        if ( I2C_CYCLE_ACKIN(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
            }

        /* nBlocks should be a power of 2. */


	/* generate ack on next to last byte; generate stop on last byte */

	for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
	    {
            if (byteCount == (pI2cCmdPacket->nBlocks-2))
                {
                /* Send an ACK on next to last transfer */
                if ( I2C_CYCLE_READ(unit,(unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,1) )
                    {
                    pI2cCmdPacket->status = I2C_ERROR_CYCLE_READ;
                    return ERROR ;
                    }
                }
            else
                {
                if (byteCount == (pI2cCmdPacket->nBlocks-1))
                    {
                    /* send a STOP on last transfer */
                    if ( I2C_CYCLE_STOP(unit) )
                        {
                        pI2cCmdPacket->status = I2C_ERROR_CYCLE_STOP;
                        return ERROR ;
                        }
                    }
                /* No acks on all other transfers */
                if ( I2C_CYCLE_READ(unit,(unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,0) )
                    {
                    pI2cCmdPacket->status = I2C_ERROR_CYCLE_READ;
                    return ERROR ;
                    }
                }

	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;
#ifdef I2C_DRIVER_DEBUG
            logMsg("byteCount - 0x%x.\n", pI2cCmdPacket->aCount,2,3,4,5,6);
#endif
	    }
	/*
	 * update the caller's command packet with status of
	 * the operation
	 */
	pI2cCmdPacket->status = statusVariable;
	}

    /*
     * write operation
     *
     *
     */
    else if ( pI2cCmdPacket->command == I2C_WRITOP )
	{
	/* Initialize pointer to caller's write data buffer. */
	pWriteData = (unsigned char *)pI2cCmdPacket->memoryAddress;

	/* Write the specified number of bytes from the EEPROM. */
	statusVariable = 0;

	if ( I2C_KNOWN_STATE(unit) )
	    {
            pI2cCmdPacket->status = I2C_ERROR_KNOWN_STATE;
            return ERROR ;
	    }
        if ( I2C_CYCLE_START(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START;
            return ERROR ;
            }
        /* device address - write, R/W bit = 0  */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
            }
        if ( I2C_CYCLE_ACKIN(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
           return ERROR ;
            }
        /* command byte (register #) */
        if ( I2C_CYCLE_WRITE( unit, (pI2cCmdPacket->blockNumber&0x7) ) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
            }
        if ( I2C_CYCLE_ACKIN(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
            }

        /* nBlocks should be a power of 2 */
	for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
	    {
	    /* write data */

	    if ( I2C_CYCLE_WRITE(unit, pWriteData[byteCount]) )
		{
                pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
                return ERROR ;
		}
            if ( I2C_CYCLE_ACKIN(unit) )
                {
                pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
                return ERROR ;
                }

	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;
#ifdef I2C_DRIVER_DEBUG
            logMsg("byteCount - 0x%x.\n", pI2cCmdPacket->aCount,2,3,4,5,6);
#endif
	    }
        if ( I2C_CYCLE_STOP(unit) )
            {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_STOP;
            return ERROR ;
            }
	}
    else
	{
	I2C_KNOWN_STATE(unit);
	return ERROR ;
	}

    /* Leave the I2C bus in a known state. */
    I2C_KNOWN_STATE(unit);

    /*
     * update the caller's command packet with status of
     * the operation
     */
    pI2cCmdPacket->status = statusVariable;

    return OK ;
    }



/******************************************************************************
*
* i2cDoOpAT24C256 - execute commands to the AT24C256 I2C device
*
* This function executes the command operations specified by the command
* packet.
*
* RETURNS: OK, or ERROR
*
* NOTE: This driver only uses random byte reads and writes.
* It can be upgraded to use page mode which is faster.
*
* ERRNO: N/A
*/

int i2cDoOpAT24C256
    (
    int    unit,
    UINT32 deviceAddress,		/* device I2C bus address */
    i2cCmdPckt_t *pI2cCmdPacket		/* pointer to command packet */
    )
    {
    i2cDrvRoutines_t *pRoutine;	/* low level routines table pointer */
    int byteCount;		/* byte counter */
    int statusVariable;		/* local status variable */
    unsigned char *pWriteData;	/* pointer to write data buffer */

    /* Initialize pointer to driver routines table. */
    pRoutine = i2cDrvRoutinesTables[I2C_DRV_TYPE];

    /*
     * read operation (EEPROM type devices), for each byte
     * perform the random read operation
     */
    if ( pI2cCmdPacket->command == I2C_READOP )
	{
	/* Read the specified number of bytes from the EEPROM. */
	/* NOTE: random read has a dummy write first. */
	statusVariable = 0;

	if ( I2C_KNOWN_STATE(unit) )
	    {
	    statusVariable = I2C_ERROR_KNOWN_STATE;
	    byteCount = pI2cCmdPacket->nBlocks;
	    }
	for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
	    {
	    if ( I2C_CYCLE_START(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_START;
		break;
		}
	    /* device address - write */
	    if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    /* 1st word address */
	    if ( I2C_CYCLE_WRITE(unit, (((pI2cCmdPacket->blockNumber+byteCount)>>8)&0x7f)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    /* 2nd word address... */
	    if ( I2C_CYCLE_WRITE(unit,((pI2cCmdPacket->blockNumber+byteCount)&0xff)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    if ( I2C_CYCLE_START(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_START;
		break;
		}
	    /* device address - read */
	    if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)|0x1) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    if ( I2C_CYCLE_READ(unit,(unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,0) )
		{
		statusVariable = I2C_ERROR_CYCLE_READ;
		break;
		}
	    if ( I2C_CYCLE_STOP(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_STOP;
		break;
		}

	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;
	    }

	/*
	 * update the caller's command packet with status of
	 * the operation
	 */
	pI2cCmdPacket->status = statusVariable;
	}

    /*
     * write operation (EEPROM type devices), for each byte
     * perform the byte write operation, a delay must be
     * exercised following each byte write
     */
    else if ( pI2cCmdPacket->command == I2C_WRITOP )
	{
	/* Initialize pointer to caller's write data buffer. */
	pWriteData = (unsigned char *)pI2cCmdPacket->memoryAddress;

	/* Write the specified number of bytes from the EEPROM. */
	statusVariable = 0;

	if ( I2C_KNOWN_STATE(unit) )
	    {
	    statusVariable = I2C_ERROR_KNOWN_STATE;
	    byteCount = pI2cCmdPacket->nBlocks;
	    }
	for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
	    {
	    if ( I2C_CYCLE_START(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_START;
		break;
		}
	    /* device address */
	    if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress )) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    /* 1st word address */
	    if ( I2C_CYCLE_WRITE(unit,(((pI2cCmdPacket->blockNumber+byteCount)>>8)&0x7f)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    /* 2nd word address... */
	    if ( I2C_CYCLE_WRITE(unit,((pI2cCmdPacket->blockNumber+byteCount)&0xff)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    /* write data */
	    if ( I2C_CYCLE_WRITE(unit,*(pWriteData + byteCount)) )
		{
		statusVariable = I2C_ERROR_CYCLE_WRITE;
		break;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		break;
		}
	    if ( I2C_CYCLE_STOP(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_STOP;
		break;
		}

	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;

	    /*
	     * delay for at least 10ms to allow EEPROM to complete
	     * the write cycle (internal operation)
	     */
	    I2C_DELAY(10);
	    }
	}
    else
	{
	I2C_KNOWN_STATE(unit);
	return ERROR ;
	}

    /* Leave the I2C bus in a known state. */
    I2C_KNOWN_STATE(unit);

    /*
     * update the caller's command packet with status of
     * the operation
     */
    pI2cCmdPacket->status = statusVariable;

    return OK ;
    }





/******************************************************************************
*
* i2cDoOpAD7417 - execute commands to the AD7417 I2C device
*
* This function executes the command operations specified by the command
* packet.
*
* RETURNS: OK, ERROR if bad request
*
* ERRNO: N/A
*/

int i2cDoOpAD7417
    (
    int    unit,
    UINT32 deviceAddress,		/* device I2C bus address */
    i2cCmdPckt_t *pI2cCmdPacket		/* pointer to command packet */
    )
    {
    i2cDrvRoutines_t *pRoutine;	/* low level routines table pointer */
    int byteCount;		/* byte counter */
    int statusVariable;		/* local status variable */
    unsigned char *pWriteData;	/* pointer to write data buffer */

    /*
     * Modes:
     *  0) writing to address pointer register for subsequent read
     *  1) single byte read
     *  2) double byte read (Toti, Thyst, ADC registers)
     *  3) writing to address pointer register then single byte of data
     *  4) writing to address pointer register then 2 bytes of data (Toti, Thyst, ADC registers)
     *
     *  2 1 0
     *  0 0 0 temperature value (ro) - 2 bytes
     *  0 0 1 config register (r/w) - 1 byte
     *  0 1 0 Thyst (r/w) - 2 bytes
     *  0 1 1 Toti (r/w) - 2 bytes
     *  1 0 0 ADC (ro) - 2 byes
     *  1 0 1 Config2 (r/w, msb only) - 1 byte
     *
     *  NOTE: pI2cCmdPacket->nBlocks is used to figure out if it is a single or double byte
     *  read or write.
     */

    if ( (pI2cCmdPacket->blockNumber < 0) || (pI2cCmdPacket->blockNumber > 5) )
	{
	printf("Invalid register number %d\n", pI2cCmdPacket->blockNumber);
	return ERROR ;
	}
    switch ( pI2cCmdPacket->blockNumber )
	{
	case 0:
	case 2:
	case 3:
	case 4:
	    if ( pI2cCmdPacket->nBlocks != 2 )
		{
		printf("Invalid register size %d, should be 2.\n", pI2cCmdPacket->nBlocks);
		return ERROR ;
		}
	    break;
	case 1:
	case 5:
	    if ( pI2cCmdPacket->nBlocks != 1 )
		{
		printf("Invalid register size %d, should be 1.\n", pI2cCmdPacket->nBlocks);
		return ERROR ;
		}
	    break;
	}

    /* Initialize pointer to driver routines table. */
    pRoutine = i2cDrvRoutinesTables[I2C_DRV_TYPE];

    /* single byte read/write */
    if ( pI2cCmdPacket->command == I2C_READOP )
	{
	statusVariable = 0;
	if ( I2C_KNOWN_STATE(unit) )
	    {
	    statusVariable = I2C_ERROR_KNOWN_STATE;
	    byteCount = pI2cCmdPacket->nBlocks;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_START(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_START;
	    goto errorEnd ;
	    }
	/* device address */
	if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }
	/* address register pointer */
	if ( I2C_CYCLE_WRITE(unit,pI2cCmdPacket->blockNumber) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_STOP(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_STOP;
	    goto errorEnd ;
	    }

	sysMpc85xxMsDelay (1);

	if ( I2C_CYCLE_START(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_START;
	    goto errorEnd ;
	    }
	/* device address */
	if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)|0x1) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_READ(unit,(unsigned char *)pI2cCmdPacket->memoryAddress+0,0) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_READ;
	    goto errorEnd ;
	    }
	/* Increment the actual count of the command packet. */
	pI2cCmdPacket->aCount += 1;
	if ( pI2cCmdPacket->nBlocks == 2 )
	    {
	    if ( I2C_CYCLE_READ(unit,(unsigned char *)pI2cCmdPacket->memoryAddress+1,0) )
		{
		statusVariable = I2C_ERROR_CYCLE_READ;
		goto errorEnd ;
		}
	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;
	    }
	if ( I2C_CYCLE_STOP(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_STOP;
	    goto errorEnd ;
	    }

	/*
	 * update the caller's command packet with status of
	 * the operation
	 */
	pI2cCmdPacket->status = statusVariable;
	}
    else if ( pI2cCmdPacket->command == I2C_WRITOP )
	{
	/* Initialize pointer to caller's write data buffer. */
	pWriteData = (unsigned char *)pI2cCmdPacket->memoryAddress;

	statusVariable = 0;
	if ( I2C_KNOWN_STATE(unit) )
	    {
	    statusVariable = I2C_ERROR_KNOWN_STATE;
	    byteCount = pI2cCmdPacket->nBlocks;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_START(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_START;
	    goto errorEnd ;
	    }
	/* device address */
	if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }
	/* address register pointer */
	if ( I2C_CYCLE_WRITE(unit,pI2cCmdPacket->blockNumber) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }


	if ( I2C_CYCLE_WRITE(unit,*(pWriteData+0)) )
	    {
	    statusVariable = I2C_ERROR_CYCLE_WRITE ;
	    goto errorEnd ;
	    }
	if ( I2C_CYCLE_ACKIN(unit))
	    {
	    statusVariable = I2C_ERROR_CYCLE_ACKIN;
	    goto errorEnd ;
	    }
	/* Increment the actual count of the command packet. */
	pI2cCmdPacket->aCount += 1;
	if ( pI2cCmdPacket->nBlocks == 2 )
	    {
	    if ( I2C_CYCLE_WRITE(unit,*(pWriteData+1)) )
		{
		statusVariable = I2C_ERROR_CYCLE_READ;
		goto errorEnd ;
		}
	    if ( I2C_CYCLE_ACKIN(unit) )
		{
		statusVariable = I2C_ERROR_CYCLE_ACKIN;
		goto errorEnd ;
		}
	    /* Increment the actual count of the command packet. */
	    pI2cCmdPacket->aCount += 1;
	    }
	if ( I2C_CYCLE_STOP(unit))
	    {
	    statusVariable = I2C_ERROR_CYCLE_STOP;
	    goto errorEnd ;
	    }

	/*
	 * update the caller's command packet with status of
	 * the operation
	 */
	pI2cCmdPacket->status = statusVariable;
	}
    else
	{
	I2C_KNOWN_STATE(unit);
	return ERROR ;
	}

    errorEnd:

    /* Leave the I2C bus in a known state. */
    I2C_KNOWN_STATE(unit);

    /*
     * update the caller's command packet with status of
     * the operation
     */
    pI2cCmdPacket->status = statusVariable;
    return OK ;
    }


int i2cDoOpM41T48
    (
    int             unit,
    UINT32          deviceAddress,    /* device I2C bus address */
    i2cCmdPckt_t *  pI2cCmdPacket     /* pointer to command packet */
    )
{
    i2cDrvRoutines_t *pRoutine;	/* low level routines table pointer */
    int byteCount;		/* byte counter */
    int statusVariable;		/* local status variable */
    unsigned char *pWriteData;	/* pointer to write data buffer */

    /* Initialize pointer to driver routines table. */
    pRoutine = i2cDrvRoutinesTables[I2C_DRV_TYPE];

    /*
     * read operation,
     *
     */
    if ( pI2cCmdPacket->command == I2C_READOP )
    {
        /* Read the specified number of bytes from the EEPROM. */
      	/* NOTE: random read has a dummy write first. */
        statusVariable = 0;

        if ( I2C_KNOWN_STATE(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_KNOWN_STATE;
            return ERROR ;
        }
        if ( I2C_CYCLE_START(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START ;
            return ERROR ;
        }
        /* device address - write, R/W bit = 0  */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }

        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN ;
            return ERROR ;
        }
        /* command byte (register #) */
        if ( I2C_CYCLE_WRITE( unit, (pI2cCmdPacket->blockNumber & 0x1f) ) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }
        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
        }
        if ( I2C_CYCLE_START(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START;
            return ERROR ;

        }
        /* device address - read, R/W bit = 1 */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)|0x1) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }
        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
        }

       /* generate ack on next to last byte; generate stop on last byte */
       for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
       {
            if (byteCount == (pI2cCmdPacket->nBlocks-1))
            {       
                /* No ACK on last byte */
                if ( I2C_CYCLE_READ(unit,
                   (unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,0) )
                {
                    pI2cCmdPacket->status = I2C_ERROR_CYCLE_READ;
                    return ERROR ;
                }
                /* send a STOP on last transfer */
                if ( I2C_CYCLE_STOP(unit) )
                {
                    pI2cCmdPacket->status = I2C_ERROR_CYCLE_STOP;
                    return ERROR ;
                }
            } else {
                /* Send an ACK on next to last transfer */
                if ( I2C_CYCLE_READ(unit,
                   (unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,1) )
                {
                    pI2cCmdPacket->status = I2C_ERROR_CYCLE_READ;
                    return ERROR ;
                }
            }
            /* Increment the actual count of the command packet. */
            pI2cCmdPacket->aCount += 1;
#ifdef I2C_DRIVER_DEBUG
            logMsg("byteCount - 0x%x.\n", pI2cCmdPacket->aCount,2,3,4,5,6);
#endif
       }
      /*
       * update the caller's command packet with status of
       * the operation
       */
       pI2cCmdPacket->status = statusVariable;
    }

    /*
     * write operation
     *
     *
     */
    else if ( pI2cCmdPacket->command == I2C_WRITOP )
    {
        /* Initialize pointer to caller's write data buffer. */
        pWriteData = (unsigned char *)pI2cCmdPacket->memoryAddress;

        /* Write the specified number of bytes from the EEPROM. */
        statusVariable = 0;

        if ( I2C_KNOWN_STATE(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_KNOWN_STATE;
            return ERROR ;
        }
        if ( I2C_CYCLE_START(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START;
            return ERROR ;
        }
        /* device address - write, R/W bit = 0  */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress)) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }
        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
        }
        /* command byte (register #) */
        if ( I2C_CYCLE_WRITE( unit, (pI2cCmdPacket->blockNumber&0x1f) ) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }
        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
            return ERROR ;
        }

        /* nBlocks should be a power of 2 */
        for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
        {
            /* write data */
            if ( I2C_CYCLE_WRITE(unit, pWriteData[byteCount]) )
            {
                pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
                return ERROR ;
            }
            if ( I2C_CYCLE_ACKIN(unit) )
            {
                pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN;
                return ERROR ;
            }

            /* Increment the actual count of the command packet. */
            pI2cCmdPacket->aCount += 1;
#ifdef I2C_DRIVER_DEBUG
            logMsg("byteCount - 0x%x.\n", pI2cCmdPacket->aCount,2,3,4,5,6);
#endif
        }
        if ( I2C_CYCLE_STOP(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_STOP;
            return ERROR ;
        }
    } else {
        I2C_KNOWN_STATE(unit);
        return ERROR ;
    }

    /* Leave the I2C bus in a known state. */
    I2C_KNOWN_STATE(unit);

    /*
     * update the caller's command packet with status of
     * the operation
     */
    pI2cCmdPacket->status = statusVariable;

    return OK ;
}

int i2cDoOpPCF8574
    (
    int             unit,
    UINT32          deviceAddress,    /* device I2C bus address */
    i2cCmdPckt_t *  pI2cCmdPacket     /* pointer to command packet */
    )
{
    i2cDrvRoutines_t *pRoutine;	/* low level routines table pointer */
    int byteCount;		/* byte counter */
    int statusVariable;		/* local status variable */

    /* Initialize pointer to driver routines table. */
    pRoutine = i2cDrvRoutinesTables[I2C_DRV_TYPE];

    /*
     * read operation,
     *
     */
    if ( pI2cCmdPacket->command == I2C_READOP )
    {
        /* Read the specified number of bytes from the EEPROM. */
        /* NOTE: random read has a dummy write first. */
        statusVariable = 0;

        if ( I2C_KNOWN_STATE(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_KNOWN_STATE;
            return ERROR ;
        }
        if ( I2C_CYCLE_START(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_START ;
            return ERROR ;
        }
        /* device address - write, R/W bit = 0  */
        if ( I2C_CYCLE_WRITE(unit,i2cAddressMunge(deviceAddress) | 0x41) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_WRITE;
            return ERROR ;
        }

        if ( I2C_CYCLE_ACKIN(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_ACKIN ;
            return ERROR ;
        }

        /* generate ack on next to last byte; generate stop on last byte */
        for ( byteCount = 0; byteCount < pI2cCmdPacket->nBlocks; byteCount++ )
        {
           /* Acks on all other transfers */
            if ( I2C_CYCLE_READ(unit,
               (unsigned char *)pI2cCmdPacket->memoryAddress+byteCount,1) )
            {
                pI2cCmdPacket->status = I2C_ERROR_CYCLE_READ;
                return ERROR ;
            }
 
            /* Increment the actual count of the command packet. */
            pI2cCmdPacket->aCount += 1;
#ifdef I2C_DRIVER_DEBUG
            logMsg("byteCount - 0x%x.\n", pI2cCmdPacket->aCount,2,3,4,5,6);
#endif
        }
        /* send a STOP on last transfer */
        if ( I2C_CYCLE_STOP(unit) )
        {
            pI2cCmdPacket->status = I2C_ERROR_CYCLE_STOP;
            return ERROR ;
        }
 
        /*
         * update the caller's command packet with status of
         * the operation
         */
        pI2cCmdPacket->status = statusVariable;
    } else {
        I2C_KNOWN_STATE(unit);
        return ERROR ;
    }

    /* Leave the I2C bus in a known state. */
    I2C_KNOWN_STATE(unit);

    /*
     * update the caller's command packet with status of
     * the operation
     */
    pI2cCmdPacket->status = statusVariable;


    return OK;
}

/******************************************************************************
*
* i2cDoOp - execute I2C do operation
*
* This function executes the operation as specified
* by the passed command packet.  Currently, the only device types
* that are recognized are the ATMEL AT24C256, Analog Devices AD7417ARU
* and PCA9555.
*
* RETURNS: OK, or ERROR
*
* ERRNO: N/A
*/

int i2cDoOp
    (
    int             unit,
    UINT32          deviceAddress,		/* device I2C bus address */
    i2cCmdPckt_t *  pI2cCmdPacket		/* pointer to command packet */
    )
    {
#ifdef I2C_DRIVER_DEBUG
    logMsg("i2cDoOp: deviceAddress - 0x%x, pI2cCmdPacket - 0x%x\n",
	   deviceAddress, pI2cCmdPacket,3,4,5,6);
#endif
    /* Command interface to stop.  This is for previous error exits. */
    I2C_CYCLE_STOP(unit);

    switch(pI2cCmdPacket->deviceType)
        {

        case I2C_DEVICE_TYPE_EEPROM_24LC128:
        case I2C_DEVICE_TYPE_EEPROM_AT24C256:
	    if ( i2cDoOpAT24C256(unit, deviceAddress, pI2cCmdPacket) != OK )
		return ERROR ;
            break;
        case I2C_DEVICE_TYPE_TEMP_SENSOR_AD7417:
	    if ( i2cDoOpAD7417(unit, deviceAddress, pI2cCmdPacket) != OK )
		return ERROR ;
            break;
        case I2C_DEVICE_TYPE_IOPORT_PCA9555:
	    if ( i2cDoOpPCA9555(unit, deviceAddress, pI2cCmdPacket) != OK )
		return ERROR ;
            break;
        case I2C_DEVICE_TYPE_RTC_M41T48:
	    if ( i2cDoOpM41T48(unit, deviceAddress, pI2cCmdPacket) != OK )
		return ERROR ;
            break;
        case I2C_DEVICE_TYPE_IOPORT_PCF8574:
	    if ( i2cDoOpPCF8574(unit, deviceAddress, pI2cCmdPacket) != OK )
		return ERROR ;
            break;
        default :
            return ERROR ;

	}

    return(OK);
    }


/******************************************************************************
*
* i2cAddressMunge - initialize the i2c device driver
*
* This function's purpose is to munge/modify the I2C device address
* based upon the byte offset into the device.  This only applies to
* EEPROM type devices.  Dependent upon the size of the device, A0-A2
* address lines are utilized as 256 byte size page offsets.
*
* RETURNS: munged address
*
* ERRNO: N/A
*/

unsigned int i2cAddressMunge
    (
    unsigned int deviceAddress
    )
    {
    return((deviceAddress<<1)&0xfe) ;
    }

/* AT24C256 -> 512 pages of 64 bytes each, 15 bits reqd for addressing */
/* 64 bytes -> address masked with 0xffc0 */
/* lsb of device address is for 0/1 read/write */


/*****************************************************************************
*
* i2cInByte - reads a byte from I2C Regs
*
* This function reads a byte from an I2C register.
*
* RETURNS: value of byte read
*
* ERRNO: N/A
*/

LOCAL UINT8 i2cInByte
    (
    UINT32 dataPtr
    )
    {
    UINT8 *ptr = (UINT8*)dataPtr ;
    return(*ptr);
    }


/*****************************************************************************
*
* i2cOutByte - writes a byte to an I2C register
*
* This function writes a byte to an I2C register.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void i2cOutByte
    (
    UINT32 dataPtr,
    UINT8 data
    )
    {
    UINT8 * ptr = (UINT8*)dataPtr ;
    *ptr = data ;
    }

