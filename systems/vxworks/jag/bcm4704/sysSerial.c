/*
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
    

*/
/*
 * Copyright(c) 2001 Broadcom Corp.
 * All Rights Reserved.
 * $Id: sysSerial.c,v 1.6 2006/01/12 22:57:03 sanjayg Exp $
 */
/* sysSerial.c - bcm47xx BSP serial device initialization */

/* Copyright 1984-1999 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01b,12nov99,hsm   cleaned
01a,28jul99,hsm   created.
*/

#include "vxWorks.h"
#include "arch/mips/ivMips.h"
#include "intLib.h"
#include "config.h"
#include "sysLib.h"
#include "drv/sio/ns16552Sio.h"
#include "vxbsp.h"
#include "bcmutils.h"
#include "bcm4704.h"

/* device initialization structure */

typedef struct
    {
    USHORT vector;                      /* Interrupt vector */
    ULONG  baseAdrs;                    /* Register base address */
    USHORT regSpace;                    /* Address Interval */
    USHORT intLevel;                    /* Interrupt level */
    } NS16550_CHAN_PARAS;

/* Local data structures */

static NS16550_CHAN  ns16550Chan[N_UART_CHANNELS];
static volatile int dummyx;

/*
*  The channel select address line (addr3) for the external DUART on the MBZ
*  board is the opposite polarity from a normal address line, so COM1 is at
*  a higher address than COM2.  
*/
static NS16550_CHAN_PARAS devParas[] = 
   {
#if 1
   {COM1_INT_VEC, COM1_BASE_ADR, COM1_ADR_INTERVAL, COM1_INT_LVL},
   {COM2_INT_VEC, COM2_BASE_ADR, COM2_ADR_INTERVAL, COM2_INT_LVL}
#else
   {INT_VEC_IORQ0, COM1_BASE_ADR, COM1_ADR_INTERVAL, INT_VEC_IORQ0},
   {INT_VEC_IORQ0, COM2_BASE_ADR, COM2_ADR_INTERVAL, INT_VEC_IORQ0}
#endif   
   };


static unsigned int xtal_freq = COM1_FREQ;

#define UART_REG(reg,chan) \
                (devParas[chan].baseAdrs + reg * devParas[chan].regSpace)

/******************************************************************************
*
* sysSerialChannelPrimaryCheck - Check console UART channel used and update 
*   devParas table if required.
*
* RETURNS: N/A
*/
void
sysSerialChannelPrimaryCheck()
{
    NS16550_CHAN_PARAS devParasTemp;

    if ((SYS_REVID_GET() != BOARD_ID_LM_P48) &&
        (SYS_REVID_GET() != BOARD_ID_LM_FB_P48) &&
        (SYS_REVID_GET() != BOARD_ID_LM_P6_CX4V2)) {
        return;
    }

    /* Check for alternate console port jumper */
    if (SYS_ALTCONS_GET() & 1) {
        return;
    }

    devParasTemp = devParas[0];
    devParas[0] = devParas[1];
    devParas[1] = devParasTemp;
}

/******************************************************************************
*
* sysSerialHwInit - initialize the BSP serial devices to a quiesent state
*
* This routine initializes the BSP serial device descriptors and puts the
* devices in a quiesent state.  It is called from sysHwInit() with
* interrupts locked.
*
* RETURNS: N/A
*/

void sysSerialHwInit (void)
    {
    int i;
    sysSerialChannelPrimaryCheck();

    for (i = 0; i < N_UART_CHANNELS; i++)
        {        
        ns16550Chan[i].regs             = (UINT8 *)devParas[i].baseAdrs;
        ns16550Chan[i].level            = devParas[i].vector;

        ns16550Chan[i].ier              = 0;
        ns16550Chan[i].lcr              = 0;
        ns16550Chan[i].pad1             = 0;

        ns16550Chan[i].channelMode      = 0;
        ns16550Chan[i].regDelta         = devParas[i].regSpace;
        ns16550Chan[i].baudRate         = CONSOLE_BAUD_RATE;

        ns16550Chan[i].xtal	        = xtal_freq;

        ns16550DevInit (&ns16550Chan[i]);
        }

    }

/******************************************************************************
*
* sysSerialInt - BSP specific serial device ISR
*
*/
void sysSerialInt(int unit)
{
    ns16550Int (&ns16550Chan[unit]);
}

/******************************************************************************
*
* sysSerialHwInit2 - connect BSP serial device interrupts
*
* This routine connects the BSP serial device interrupts.  It is called from
* sysHwInit2().  Serial device interrupts could not be connected in
* sysSerialHwInit() because the kernel memory allocator was not initialized
* at that point, and intConnect() calls malloc().
*
* RETURNS: N/A
*/

void sysSerialHwInit2 (void)
{
    int i;
    for (i = 0; i < N_UART_CHANNELS; i++)
       {
         if(i==0) 
            intConnect (INUM_TO_IVEC (IV_EXT_ALT1_VEC), sysSerialInt, 0);  
         if(i==1)
            intConnect (INUM_TO_IVEC (IV_EXT_ALT2_VEC), sysSerialInt, 1);
       }
    /* XXX: Todo, use CHIPC IRQ */
}

/******************************************************************************
*
* sysSerialHwReset - reset the serial controllers
*
* Shutdown all controllers capable of generating interrupts, especially
* controllers using DMA to transfer channel command blocks.  Most Bug
* monitors presume that a hardware reset precedes entry to the monitor
* code.
*
* RETURNS: N/A.
*/

VOID sysSerialHwReset (void)
    {
    int i;

    for (i = 0; i < N_UART_CHANNELS; i++)
        ns16550DevInit (&ns16550Chan[i]);
    }

/******************************************************************************
*
* sysSerialChanGet - get the SIO_CHAN device associated with a serial channel
*
* This routine gets the SIO_CHAN device associated with a specified serial
* channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/

SIO_CHAN * sysSerialChanGet
    (
    int channel         /* serial channel */
    )
    {
    if (channel < 0 || channel >= NELEMENTS(ns16550Chan))
        return (SIO_CHAN *)ERROR;
    
    return ((SIO_CHAN *) &ns16550Chan[channel]);
    }

/******************************************************************************
 *
 * Serial debugging print routines
 *
 * The following routines are for debugging, especially useful in early
 * boot ROM and ISR code.
 *
 * sysSerialPutc
 * sysSerialGetc
 * sysSerialPrintString
 * sysSerialPrintHex
 *
 * The sysSerialPrintString and sysSerialPrintHex routines should
 * basically ALWAYS work.  They re-initialize the UART and turn off
 * interrupts every time.
 */

#define SS_CHAN         0
#ifdef  MIPSEL
#define ADDR_XOR_3 0
#else
#define ADDR_XOR_3 3
#endif

UCHAR sysSerialInByte(int addr )
{
    dummyx = (*(volatile unsigned char*)0xb8000000);
    return *(volatile unsigned char *) (addr^ADDR_XOR_3);
}

void sysSerialOutByte (int addr,UCHAR c)
{
    unsigned char x;
    dummyx = (*(volatile unsigned char*)0xb8000000);
    *(volatile unsigned char *) (addr^ADDR_XOR_3) = c;

    /* PR13509 - CHIPC hang on UART register write:
     * The workaround is that MIPS must read any SB location that isn't
     * in the UART after any UART write. The location read doesn't even
     * have to be in chipcommon. It's believed that this bug does not
     * affect any other cores accessing UART.
     */

     x = (*(volatile unsigned char*)0xb8000000);
}


void sysSerialDelay(void)
{
    volatile int i;
    for (i = 0; i < 0x10000; i++)
        ;
}

void sysSerialPutc(int c)
{
    int i = 10000;
    while (!(sysSerialInByte(UART_REG(LSR, SS_CHAN)) & 0x40) && i--)
        ;
    sysSerialOutByte(UART_REG(THR, SS_CHAN), c);
}

int sysSerialGetc(void)
{
    while (!(sysSerialInByte(UART_REG(LSR, SS_CHAN)) & 0x01))
        ;
    return sysSerialInByte(UART_REG(RBR, SS_CHAN));
}

void sysSerialUARTPollInit
   (
   int channel
   )
{
/* #define UART_CLOCK 1850000*/
#define UART_CLOCK 1851851
#define BASE_BAUD  9600
#define DIV_LO     ((UART_CLOCK/16)/BASE_BAUD)
#define DIV_HI     (DIV_LO>>8)	
   
   sysSerialOutByte (UART_REG (LCR, channel), 0x80);
   sysSerialOutByte (UART_REG (DLL, channel), (unsigned char)DIV_LO);
   sysSerialOutByte (UART_REG (DLM, channel), (unsigned char)DIV_HI);
   sysSerialOutByte (UART_REG (LCR, channel), 0x03);
   sysSerialOutByte (UART_REG (MCR, channel), 0xb);
}

void sysSerialPrintString(char *s)
{
    int c, il;

    il = intLock();
    /* sysSerialUARTPollInit (SS_CHAN);*/

    while ((c = *s++) != 0) {
        if (c == '\n')
            sysSerialPutc('\r');
        sysSerialPutc(c);
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}

void sysSerialPrintHex(UINT32 value, int cr)
{
    const char hex[] = "0123456789abcdef";
    int i, il;
    il = intLock();
    /* sysSerialUARTPollInit (SS_CHAN);*/
    for (i = 0; i < 8; i++)
        sysSerialPutc(hex[value >> (28 - i * 4) & 0xf]);
    if (cr) {
        sysSerialPutc('\r');
        sysSerialPutc('\n');
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}
