/* sysGpio.c - system-dependent GPIO library */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,04jun15,dnb  Initial version.
*/

/*
DESCRIPTION
This module provides BSP functionality to support accessing 
GPIO on the iProc Platform

*/

/* this file is included by sysLib.c */

/* defines */
#define ChipcommonA_GPIOInput   0x18000060
#define ChipcommonA_GPIOOut	0x18000064
#define ChipcommonA_GPIOOutEn	0x18000068


/*******************************************************************************
*
* sysGpioOutEn - set gpio output enable for ChipCommonA GPIO
*
* 
* RETURNS: Previous setting of output enable
*/

UINT32 sysGpioOutEn
    (
    UINT32 index,
    BOOL   enable
    )
    {
    UINT32	prev;

    UINT32	mask = 1 << index;
    UINT32	*p = (UINT32 *)(ChipcommonA_GPIOOutEn);

    prev = *p;

    if(enable)
	{
	*p = prev | mask;
	}
    else
	{
	*p = prev & ~mask;
	}

    return prev;
    }

/*******************************************************************************
*
* sysGpioOut - set gpio output for ChipCommonA GPIO
*
* 
* RETURNS: Previous value of output
*/

UINT32 sysGpioOut
    (
    UINT32 index,
    BOOL   enable
    )
    {
    UINT32	prev;

    UINT32	mask = 1 << index;
    UINT32	*p = (UINT32 *)(ChipcommonA_GPIOOut);

    prev = *p;

    if(enable)
	{
	*p = prev | mask;
	}
    else
	{
	*p = prev & ~mask;
	}

    return prev;
    }

/*******************************************************************************
*
* sysGpioInput - read gpio value for ChipCommonA GPIO
*
* 
* RETURNS: current value of input
*/

UINT32 sysGpioIn
    (
    UINT32 index
    )
    {
    UINT32      val;
    UINT32	mask = 1 << index;
    UINT32	*p = (UINT32 *)(ChipcommonA_GPIOInput);

    val = (*p & mask);

    return val ? 1 : 0;

    }

