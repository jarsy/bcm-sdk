/* sysDuart.c -  Wind River SBC8548 Duart device initialization */

/* $Id: sysDuart.c,v 1.2 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,30jan06,kds  Modified from cds8548/sysDuart.c/01a
*/

/*
DESCRIPTION
This file contains board-specific routines for Wind River SBC8548
Duart device initialization. 

INCLUDE FILES:
*/

#include <vxWorks.h>
#include <config.h>
#include <intLib.h>
#include <iv.h>
#include <sysLib.h>
#include <config.h>

#include <sysEpic.h>
#include <sysDuart.h>		        /* MPC8241/8245 duart driver */
#include <drv/sio/ns16552Sio.h>		/* ns16552Sio driver */

/* device description structs */

typedef struct
    {
    USHORT vector;			/* Interrupt vector */
    ULONG  baseAdrs;			/* Register base address */
    USHORT regSpace;			/* Address Interval */
    USHORT intLevel;			/* Interrupt level */
    } NS16552_CHAN_PARAS;

/* external variable */

IMPORT int epicIntTrace;                /* trace epic internal interrupts */

/* global variable */

NS16550_CHAN  ns16550Chan[N_DUART_CHANNELS];

char  *eumbbar_base;                    /* eumbbar base address */

/* static variables */

/* 
 * Both DUART channels are supported on the SBC8548 board. 
 * COM1 is selected by default. To select COM2, swap the COM1_ADR
 * and COM2_ADR constants below.
 */

LOCAL NS16552_CHAN_PARAS devDuartParas[] = 
    {
    {EPIC_DUART_INT_VEC, COM1_ADR, DUART_REG_ADDR_INTERVAL, EPIC_DUART_INT_NUM},
    {EPIC_DUART2_INT_VEC, COM2_ADR, DUART_REG_ADDR_INTERVAL, EPIC_DUART2_INT_NUM}
    };  

/*
 * Array of pointers to all MPC854x serial channels configured.
 * See sysDuartChanGet(). It is this array that maps channel pointers
 * to standard device names.  The first entry will become "/tyCo/2",
 * the second "/tyCo/3".
 */

SIO_CHAN * sysSerialSioChans [N_SIO_CHANNELS] =
    {
    (SIO_CHAN *)&ns16550Chan[0].pDrvFuncs,
    (SIO_CHAN *)&ns16550Chan[1].pDrvFuncs
    };

/* definitions */

#define DUART_REG(reg, chan) \
        *(volatile UINT8 *)(devDuartParas[chan].baseAdrs + reg * devDuartParas[chan].regSpace)


/******************************************************************************
*
* sysDuartHwInit - initialize duart MPC854x devices to a quiescent state
*
* This routine initializes the MPC854x Duart device descriptors and puts
* the devices in a quiescent state.  It is called from sysHwInit() with
* interrupts locked.   Polled mode serial operations are possible, but
* not interrupt mode operations which are enabled by sysDuartHwInit2().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit()
*/ 

void sysDuartHwInit (void)
    {
    int i;

    eumbbar_base = (char *)CCSBAR;

	for (i = 0; i < N_DUART_CHANNELS; i++)
		{
		ns16550Chan[i].regs        = (UINT8 *)devDuartParas[i].baseAdrs;
		ns16550Chan[i].level	   = devDuartParas[i].intLevel;
		ns16550Chan[i].channelMode = SIO_MODE_INT;
		ns16550Chan[i].regDelta    = devDuartParas[i].regSpace;
		ns16550Chan[i].baudRate	   = DUART_BAUD;
		ns16550Chan[i].xtal	   = sysClkFreqGet();

		ns16550DevInit (&ns16550Chan[i]);
		}

	if (ns16550Chan[0].channelMode == SIO_MODE_INT)
		{
       		eumbbar_base[UDCR2] = 0x01;  /* set duart mode */
       		eumbbar_base[ULCR2] = 0x80;  /* open DLAB */
       		eumbbar_base[UAFR2] = 0x00;
       		eumbbar_base[UDMB2] = 0x0a;  /* MSB 9600bps @400Mhz */
		eumbbar_base[UDLB2] = 0xaa;  /* LSB */
		eumbbar_base[ULCR2] = 0x03;  /* clear DLAB, no-parity, 1stop bit, 8bit data */
		eumbbar_base[UMCR2] = 0x02;  /* disable loopback mode */
		eumbbar_base[UIER2] = 0x03;  /* Tx empty, Rx interrupt enable */
		}

    } /* sysDuartHwInit () */


/******************************************************************************
*
* sysDuartHwInit2 - connect MPC854x duart device interrupts
*
* This routine connects the MPC854x duart device interrupts. It is called
* from sysHwInit2().  
*
* Serial device interrupts cannot be connected in sysDuartHwInit() because
* the kernel memory allocator is not initialized at that point and
* intConnect() calls malloc().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit2()
*/ 

void sysSerialHwInit2 (void)
    {
    int i;

    /* connect serial interrupts */
    for (i = 0; i < N_DUART_CHANNELS; i++)
        {
        (void) sysEpicIntConnect ((VOIDFUNCPTR *)((int)devDuartParas[i].vector),
                           (VOIDFUNCPTR)ns16550Int, (int)&ns16550Chan[i] );

        sysEpicIntEnable (devDuartParas[i].vector); 
        }

    } /* sysDuartHwInit2 () */


/******************************************************************************
*
* sysDuartChanGet - get the SIO_CHAN device associated with MPC854x
*
* This routine returns a pointer to the SIO_CHAN device associated
* with a specified serial channel on MPC854x.  It is called by usrRoot() to
* obtain pointers when creating the system serial devices, `/tyCo/x'.  It
* is also used by the WDB agent to locate its serial channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/

SIO_CHAN * sysSerialChanGet
    (
    int channel         /* serial channel */
    )
    {
    if ( (channel < 0) ||
         (channel >= (int)NELEMENTS(sysSerialSioChans)) )
	return (SIO_CHAN *) ERROR;

    return sysSerialSioChans[channel];
    }



