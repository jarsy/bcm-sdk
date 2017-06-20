/* bcm1250DuartSio.c - BCM1250 serial communications driver */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250DuartSio.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01c,19dec01,agf  fix BaudSet to test range of requested baud
01b,07dec01,agf  apply coding standard fix-ups
01a,15nov01,agf  written. Based on m68681Sio.c driver.
*/

/*
DESCRIPTION
This is the driver for the Broadcom BCM1250 DUART. 
This device includes two universal asynchronous receiver/transmitters, 
and baud rate generators.
This device shares similiarities to the m68681 Duart. However it is 
different enough to justify a seperate device driver. This is because
the bcm1250 DUART has been enhanced to operate more like two
independant UARTs instead of a single DUART. Where one channel
may be used by first processor and the other channel used by the
second processor.  This is possible because, unlike the m68681, 
the bcm1250 Duart asserts a seperate interrupt line 
for each channel. Additional enhancements allow each channel is driven by a 
seperate baud rate generator which is capable of run each channel at up to 
1 Megabaud. In summary, this driver module provides control over each of the 
two serial channels and the baud-rate generators as two independant devices.  

A BCM1250_DUART structure is used to describe the chip. 
The BCM1250_DUART structure is defined in bcm1250DuartSio.h.

Although, the BCM1250 DUART is capable of running in syncronous mode (HDLC), 
only asynchronous serial operation is supported by this driver.

The default serial settings are 8 data bits, 1 stop bit, no parity, 9600
baud, and no control.  These default settings can be overridden
on a channel-by-channel basis by setting the BCM1250_DUART_CHAN options 
and `baudRate' fields to the desired values before calling 
bcm1250DuartDevInit().  See sioLib.h
for option values.  The defaults for the module can be changed by
redefining the macros BCM1250_DUART_DEFAULT_OPTIONS and 
BCM1250_DUART_DEFAULT_BAUD and recompiling this driver.


USAGE
The BSP's sysHwInit() routine typically calls sysSerialHwInit()
which initializes all the hardware addresses in the BCM1250_DUART structure
before calling bcm1250DuartDevInit().  This enables the chip to operate in
polled mode, but not in interrupt mode.  Calling bcm1250DuartDevInit2() from
the sysSerialHwInit2() routine allows interrupts to be enabled and
interrupt-mode operation to be used.

The following example shows the first part of the initialization thorugh
calling bcm1250DuartDevInit():
.CS
#include "drv/sio/bcm1250DuartSio.h"

BCM1250_DUART myDuart;	/@ my device structure @/

#define MY_VEC	(71)	/@ use single vector, #71 @/

sysSerialHwInit()
    {
    /@ initialize the register pointers for portA @/
    myDuart.portA.mr	= BCM1250_DUART_MRA;
    myDuart.portA.sr	= BCM1250_DUART_SRA;
    myDuart.portA.csr	= BCM1250_DUART_CSRA;
    myDuart.portA.cr	= BCM1250_DUART_CRA;
    myDuart.portA.rb	= BCM1250_DUART_RHRA;
    myDuart.portA.tb	= BCM1250_DUART_THRA;

    /@ initialize the register pointers for portB @/
    myDuart.portB.mr = BCM1250_DUART_MRB;
    ...

    /@ initialize the register pointers/data for main duart @/
    myDuart.ivr		= MY_VEC;
    myDuart.ipcr	= BCM1250_DUART_IPCR;
    myDuart.acr		= BCM1250_DUART_ACR;
    myDuart.isr		= BCM1250_DUART_ISR;
    myDuart.imr		= BCM1250_DUART_IMR;
    myDuart.ip		= BCM1250_DUART_IP;
    myDuart.opcr	= BCM1250_DUART_OPCR;
    myDuart.sopbc	= BCM1250_DUART_SOPBC;
    myDuart.ropbc	= BCM1250_DUART_ROPBC;
    myDuart.ctroff	= BCM1250_DUART_CTROFF;
    myDuart.ctron	= BCM1250_DUART_CTRON;
    myDuart.ctlr	= BCM1250_DUART_CTLR;
    myDuart.ctur	= BCM1250_DUART_CTUR;


    bcm1250DuartDevInit (&myDuart);
    }
.CE

The BSP's sysHwInit2() routine typically calls sysSerialHwInit2() which
connects the chips interrupts via intConnect() to the single
interrupt handler bcm1250DuartInt().  After the interrupt service routines are 
connected, the user then calls bcm1250DuartDevInit2() to allow the driver to 
turn on interrupt enable bits, as shown in the following example:

.CS
sysSerialHwInit2 ()
    {
    /@ connect single vector for 68681 @/
    intConnect (INUM_TO_IVEC(MY_VEC), bcm1250DuartInt, (int)&myDuart);

    ...

    /@ allow interrupts to be enabled @/
    bcm1250DuartDevInit2 (&myDuart);
    }
.CE

SPECIAL CONSIDERATIONS:
The CLOCAL hardware option presumes that OP0 and OP1 output bits are wired
to the CTS outputs for channel 0 and channel 1 respectively.  If not wired
correctly, then the user must not select the CLOCAL option.

This driver does not manipulate the output port
or its configuration register in any way.  If the user
selects the CLOCAL option, then the output port bit must be wired correctly
or the hardware flow control will not function correctly.

INCLUDE FILES: drv/sio/bcm1250DuartSio.h
*/

#include "vxWorks.h"
#include "sioLib.h"
#include "intLib.h"
#include "errno.h"
#include "bcm1250IntLib.h"
#include "bcm1250DuartSio.h"
#include "config.h"

#include "bcm1250Lib.h"

#undef PASS2_DUART_WORKAROUND

/* forward static declarations */

LOCAL void   bcm1250DuartInitChannel (BCM1250_DUART_CHAN *);
LOCAL void   bcm1250DuartInitStruct (BCM1250_DUART_CHAN *);
LOCAL STATUS bcm1250DuartModeSet (BCM1250_DUART_CHAN *, UINT);
LOCAL STATUS bcm1250DuartBaudSet (BCM1250_DUART_CHAN *, UINT);
LOCAL STATUS bcm1250DuartOptsSet (BCM1250_DUART_CHAN *, UINT);
LOCAL void   bcm1250DuartTxCoutRelay ( BCM1250_DUART_CHAN * pChan, 
					  BOOL enable );

#if 0 /* not currently used */
LOCAL UINT8 bcm1250DuartAcrGet ( BCM1250_DUART_CHAN * pChan );
#endif 

LOCAL void   bcm1250DuartAcrSet ( BCM1250_DUART_CHAN * pChan, UCHAR chanAcr );
LOCAL void   bcm1250DuartRts ( BCM1250_DUART_CHAN * pChan, 
					  BOOL enable );

LOCAL int    bcm1250DuartIoctl (BCM1250_DUART_CHAN *, int, void *);
LOCAL int    bcm1250DuartTxStartup (BCM1250_DUART_CHAN *);
LOCAL int    bcm1250DuartCallbackInstall (BCM1250_DUART_CHAN *, int, STATUS (*)(), void *);
LOCAL int    bcm1250DuartPollInput (BCM1250_DUART_CHAN *, char *);
LOCAL int    bcm1250DuartPollOutput (BCM1250_DUART_CHAN*, char);

/* driver functions */

LOCAL SIO_DRV_FUNCS bcm1250DuartSioDrvFuncs =
    {
    (int (*)(SIO_CHAN *, int, void *))bcm1250DuartIoctl,
    (int (*)(SIO_CHAN *))bcm1250DuartTxStartup,
    (int (*)())bcm1250DuartCallbackInstall,
    (int (*)(SIO_CHAN *, char*))bcm1250DuartPollInput,
    (int (*)(SIO_CHAN *, char))bcm1250DuartPollOutput
    };

/* typedefs */


/* defines */

#ifndef BCM1250_UART_DEFAULT_BAUD
#   define BCM1250_UART_DEFAULT_BAUD  9600
#endif

#ifndef DUART_DEFAULT_OPTIONS
    /* no handshake, rcvr enabled, 8 data bits, 1 stop bit, no parity */
#   define DUART_DEFAULT_OPTIONS (CLOCAL | CREAD | CS8)
#endif

#define BCM1250_DUART_REF_CLK_RATE 100000000
#define BCM1250_DUART_DIV_COUNTER_MAX 4095

#define MAX_OPTIONS	(0xff)
#define MAX_BAUD	(1000000)
#define MIN_BAUD        (1200)

/******************************************************************************
*
* bcm1250DuartDevInit - intialize a BCM1250_DUART channel
*
* The BSP must already have initialized all the device addresses and
* register pointers in the BCM1250_DUART structure as described in
* `bcm1250DuartSio'. This routine initializes some transmitter and receiver status
* values to be used in the interrupt mask register and then resets the chip
* to a quiescent state.
*
* RETURNS: N/A
*/

void bcm1250DuartDevInit
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    int oldlevel;

    pChan->intEnable		= FALSE;

    if((pChan->channel != BCM1250_DUART_CHANNEL_A ) && 
       (pChan->channel != BCM1250_DUART_CHANNEL_B))
      return;

    pChan->duartBase = (void *)PHYS_TO_K1(A_DUART);

    pChan->chanBase = pChan->duartBase + R_DUART_CHANREG(pChan->channel, 0);
    pChan->chanIMR  = (unsigned long long *)
      (pChan->duartBase + R_DUART_IMRREG(pChan->channel));
    pChan->chanISR  = (unsigned long long *)
      (pChan->duartBase + R_DUART_ISRREG(pChan->channel));

    pChan->intSource = (pChan->channel == BCM1250_DUART_CHANNEL_A) ?
      K_INT_UART_0 : K_INT_UART_1;

    /* clear delta interrupts */
    bcm1250DuartAcrSet (pChan, 0);

    /* don't relay TX clock on output in */
    bcm1250DuartTxCoutRelay (pChan, FALSE );

    /* assert RTS (request to send ) */
    bcm1250DuartRts (pChan, TRUE);

    /* init callbacks and set default options */
    bcm1250DuartInitStruct (pChan);

    oldlevel =  intLock ();

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    /* Clear the interrupt mask register */
    MIPS3_SD(pChan->chanIMR, 0);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    bcm1250DuartInitChannel (pChan);

    intUnlock (oldlevel);
    }

/******************************************************************************
*
* bcm1250DuartTxCoutRelay - Enable or Disable TX CLK on Output Pin  
*
* This routine enables or disables the generation of the TX CLK on
* the corresponding channels output pin (op2/op 3 for Chan A/B respectively)
*
* RETURNS: N/A
*/

void bcm1250DuartTxCoutRelay
    (
     BCM1250_DUART_CHAN * pChan, BOOL enable
    )
    {
    unsigned long long opcr;

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    opcr = MIPS3_LD(pChan->duartBase + R_DUART_OPCR);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    if(enable)
	{
	if(pChan->channel == BCM1250_DUART_CHANNEL_A)
	    opcr |= M_DUART_OPC2_SEL;
	else
	    opcr |= M_DUART_OPC3_SEL;
	}
    else /* disable */
	{
	if(pChan->channel == BCM1250_DUART_CHANNEL_A)
	    opcr &= ~M_DUART_OPC2_SEL;
	else
	    opcr &= ~M_DUART_OPC3_SEL;
	}

    MIPS3_SD(pChan->duartBase + R_DUART_OPCR, opcr);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    }

/******************************************************************************
*
* bcm1250DuartRts - Enable or Disable RTS (request to send)  
*
* This routine enables or disables the generation RTS signal on
* the corresponding channels output pin (op0/op1 for Chan A/B respectively)
*
* RETURNS: N/A
*/

void bcm1250DuartRts
    (
     BCM1250_DUART_CHAN * pChan, BOOL enable
    )
    {
    unsigned long long rts;

    if(enable)
	rts = M_DUART_OUT_PIN_SET(pChan->channel);
    else /* disable */
	rts = M_DUART_OUT_PIN_CLR(pChan->channel);

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD(pChan->duartBase + R_DUART_OUT_PORT, rts);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    }

/******************************************************************************
*
* bcm1250DuartDevInit2 - intialize a BCM1250_DUART channel, part 2
*
* This routine is called as part of sysSerialHwInit2().  It tells
* the driver that interrupt vectors are connected and that it is
* safe to allow interrupts to be enabled.
*
* RETURNS: N/A
*/

void bcm1250DuartDevInit2
    (
    BCM1250_DUART_CHAN * pChan
    )
    {

    /* connect handler */
    bcm1250IntConnect( pChan->intSource, pChan->intVecNum,
			bcm1250DuartInt, (int) pChan);

    /* enable the interrupt by unmasking it in the bcm1250 int ctrl */
    bcm1250IntEnable( pChan->intSource);
      
    /* Allow interrupt mode */
    pChan->intEnable = TRUE;

#if 0
      /* call ModeSet, to startup devices if needed */
      bcm1250DuartModeSet (pChan, SIO_MODE_INT );
#endif
    }

#if 0 /* NOT currently used */
/*******************************************************************************
*
* bcm1250DuartAcrGet - Return logical DUART auxiliary control values for 
* the channel of interest
*
* This routine returns the logical status information of the DUART auxiliary 
* control register * (ACR) for the particular channel of interest.  
*
*
* RETURNS: 
* DUART_ACR_DELTA_CTS_TCLKIN - 	
*    for delta in ip0 (channel A)  or for delta in ip2 (channel B)
* DUART_ACR_DELTA_CIN_RCLKIN -
*    for delta in ip1 (channel A)  or for delta in ip3 (channel B)
*/

LOCAL UINT8 bcm1250DuartAcrGet
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    unsigned long long acrReg;
    UINT8 chanAcr = 0;

    DUART_SHARED_READ (DUART_ACR, acrReg);
    if(pChan->channel == BCM1250_DUART_CHANNEL_A)
	{
	if(acrReg & DUART_ACR_IP0 )
	  chanAcr |= DUART_ACR_DELTA_CTS_TCLKIN;
	if(acrReg & DUART_ACR_IP2 ) 
	  chanAcr |= DUART_ACR_DELTA_CIN_RCLKIN;
	}
    else
	{
	/* channel B */
	if(acrReg & DUART_ACR_IP1 )
	  chanAcr |= DUART_ACR_DELTA_CTS_TCLKIN;
	if(acrReg & DUART_ACR_IP3 )
	  chanAcr |= DUART_ACR_DELTA_CIN_RCLKIN;
	}

    return chanAcr;
    }
#endif


/*******************************************************************************
*
* bcm1250DuartAcrSet - set bits in the DUART auxiliary control register
* based on the desired logical channel settings
*
* This routine sets the bits in the DUART aux control register based on the
* following logical channel setttings:
*
* DUART_CHAN_ACR_DELTA_CTS_TCLKIN - 	
*    for delta in ip0 (channel A)  or for delta in ip2 (channel B)
* DUART_CHAN_ACR_DELTA_CIN_RCLKIN -
*    for delta in ip1 (channel A)  or for delta in ip3 (channel B)
* RETURNS: N/A
*/

LOCAL void bcm1250DuartAcrSet
    (
    BCM1250_DUART_CHAN * pChan,
    UCHAR chanAcr      
    )
    {
    unsigned long long acr;

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    acr = MIPS3_LD(pChan->duartBase + R_DUART_AUX_CTRL);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    if(pChan->channel == BCM1250_DUART_CHANNEL_A)
	{
	/* channel A */
	acr  &= ~( M_DUART_IP0_CHNG_ENA |
		   M_DUART_IP2_CHNG_ENA );
#if 0
	if(chanAcr & DUART_ACR_DELTA_CTS_TCLKIN)
	  acr |= M_DUART_IP0_CHNG_ENA; 
	if(chanAcr & DUART_ACR_DELTA_CIN_RCLKIN)
	  acr |= M_DUART_IP2_CHNG_ENA;
#endif
	}
    else
	{
	/* channel B */
	acr  &= ~( M_DUART_IP1_CHNG_ENA |
		   M_DUART_IP3_CHNG_ENA );
#if 0
	if(chanAcr & DUART_ACR_DELTA_CTS_TCLKIN)
	  acr |= M_DUART_IP1_CHNG_ENA; 
	if(chanAcr & DUART_ACR_DELTA_CIN_RCLKIN)
	  acr |= M_DUART_IP3_CHNG_ENA; 
#endif
	}

    MIPS3_SD(pChan->duartBase + R_DUART_AUX_CTRL, acr);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    }


/******************************************************************************
*
* bcm1250DuartDummyCallback - dummy callback routine.
*
* RETURNS:
* Always returns ERROR
*/

LOCAL STATUS bcm1250DuartDummyCallback (void)
    {
    return (ERROR);
    }

/******************************************************************************
*
* bcm1250DuartTxStartup - start the interrupt transmitter.
*
* This routine enables the transmitters for the specified DUART channel so that
* the DUART channel will interrupt the CPU when TxRDY bit in the status
* register is set.
*
* RETURNS:
* Returns OK on success, or EIO on hardware error.
*/

LOCAL int bcm1250DuartTxStartup
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    char   outChar;
    int    lvl;
    unsigned long long imr, newimr;

    if ((*pChan->getTxChar) (pChan->getTxArg, &outChar) != ERROR)
	{
	lvl = intLock ();
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	MIPS3_SD(pChan->chanBase + R_DUART_TX_HOLD, outChar);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

	/* Enable transmitter-ready interrupts.  */
	imr = MIPS3_LD(pChan->chanIMR);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	newimr = imr | M_DUART_IMR_TX;
	if (imr != newimr)
        { 
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	    MIPS3_SD(pChan->chanIMR, newimr);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
        } 
	intUnlock (lvl);
	}

    return (OK);
    }

/******************************************************************************
*
* bcm1250DuartCallbackInstall - install ISR callbacks to get/put chars.
*
* This driver allows interrupt callbacks, for transmitting characters
* and receiving characters. In general, drivers may support other types
* of callbacks too.
*
* RETURNS:
* Returns OK on success, or ENOSYS for an unsupported callback type.
*/

LOCAL int bcm1250DuartCallbackInstall
    (
    BCM1250_DUART_CHAN *	pChan,
    int		callbackType,
    STATUS	(*callback)(),
    void *      callbackArg
    )
    {
    switch (callbackType)
        {
	case SIO_CALLBACK_GET_TX_CHAR:
	    pChan->getTxChar	= callback;
	    pChan->getTxArg	= callbackArg;
	    return (OK);
	case SIO_CALLBACK_PUT_RCV_CHAR:
	    pChan->putRcvChar	= callback;
	    pChan->putRcvArg	= callbackArg;
	    return (OK);
	default:
	    return (ENOSYS);
	}
    }

/******************************************************************************
*
* bcm1250DuartPollOutput - output a character in polled mode.
*
* This routine polls the status register to see if the TxRDY bit has been set.
* This signals that the transmit holding register is empty and that the
* specified DUART channel is ready for transmission.
*
* RETURNS:
* Returns OK if a character sent, EIO on device error, EAGAIN
* if the output buffer is full.
*/

LOCAL int bcm1250DuartPollOutput
    (
    BCM1250_DUART_CHAN *	pChan,
    char	outChar
    )
    {
    unsigned long long statusReg;

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    statusReg = MIPS3_LD(pChan->chanBase + R_DUART_STATUS);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    /* is the transitter ready to accept a character? */
    if ((statusReg & M_DUART_TX_RDY) == 0x00)
	return (EAGAIN);

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    /* write out the character */
    MIPS3_SD(pChan->chanBase + R_DUART_TX_HOLD, outChar);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    return (OK);
    }

/******************************************************************************
*
* bcm1250DuartPollInput - poll the device for input.
*
* This routine polls the status register to see if the RxRDY bit is set.
* This gets set when the DUART receives a character and signals the
* pressence of a character in the channels receive buffer.
*
* RETURNS:
* Returns OK if a character arrived, EIO on device error, EAGAIN
* if the input buffer if empty.
*/

LOCAL int bcm1250DuartPollInput
    (
    BCM1250_DUART_CHAN *	pChan,
    char *	thisChar
    )
    {
    unsigned long long statusReg;

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    statusReg = MIPS3_LD(pChan->chanBase + R_DUART_STATUS);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    if ((statusReg & M_DUART_RX_RDY) == 0x00)
	return (EAGAIN);

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    *thisChar = MIPS3_LD(pChan->chanBase + R_DUART_RX_HOLD);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    return (OK);
    }

/******************************************************************************
*
* bcm1250DuartModeSet - change channel mode setting
*
* This driver supports both polled and interrupt modes and is capable of
* switching between modes dynamically. If interrupt mode is desired this
* routine enables the channels receiver, transmitter interrupt and the
* received break condition interrupt. If polled mode is desired the xmitrs for
* the specified channel is enabled.
*
* RETURNS:
* Returns a status of OK if the mode was set else ERROR.
*/

LOCAL STATUS bcm1250DuartModeSet
    (
    BCM1250_DUART_CHAN * pChan,
    UINT	newMode
    )
    {
    int oldlevel;
    unsigned long long imr;

    if ((newMode != SIO_MODE_POLL) && (newMode != SIO_MODE_INT))
	return (ERROR);

    oldlevel = intLock ();

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    imr = MIPS3_LD(pChan->chanIMR);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    
    if (newMode == SIO_MODE_INT && pChan->intEnable)
        {
	/* Enable the interrupts for receiver conditions (transmitter will
	 * be enabled when transmit started.
	 */
	imr |= M_DUART_IMR_RX;
        }
    else
        {
	/* Disable transmit and receive interrupts. */
	imr &= ~ (M_DUART_IMR_TX | M_DUART_IMR_RX);
        }

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD(pChan->chanIMR, imr);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD(pChan->chanBase + R_DUART_CMD, M_DUART_TX_EN);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    intUnlock (oldlevel);
    pChan->mode = newMode;

    return (OK);
    }

/******************************************************************************
*
* bcm1250DuartBaudSet - change baud rate for channel
*
* This routine sets the baud rate for the DUART. The interrupts are disabled
* during chip access.
*
* RETURNS:
* Returns a status of OK if the baud rate was set else ERROR.
*/

LOCAL STATUS  bcm1250DuartBaudSet
    (
    BCM1250_DUART_CHAN * pChan,
    UINT	baud
    )
    {
    int oldlevel;
    UINT16 baudcount;

    if ( (baud < MIN_BAUD) ||
         (baud > MAX_BAUD)   )
        return (ERROR);

    /* baud count divisor is only 12 bits wide; limit baudcount at lower rates */
    baudcount =  min ( BCM1250_DUART_DIV_COUNTER_MAX, 
                       (BCM1250_DUART_REF_CLK_RATE/(baud * 20) - 1) );

    oldlevel = intLock ();
   
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD(pChan->chanBase + R_DUART_CLK_SEL, baudcount );
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    intUnlock (oldlevel);
    return (OK);
    }

/*******************************************************************************
*
* bcm1250DuartInitStruct - initializes the channel structure
*
* This routine initializes the specified channels driver functions.
*
* RETURNS: N/A.
*/

LOCAL void bcm1250DuartInitStruct
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    pChan->sio.pDrvFuncs= &bcm1250DuartSioDrvFuncs;

    pChan->getTxChar	= bcm1250DuartDummyCallback;
    pChan->putRcvChar	= bcm1250DuartDummyCallback;
    pChan->mode		= 0; /* undefined */

    if (pChan->options == 0 || pChan->options > MAX_OPTIONS)
	pChan->options = DUART_DEFAULT_OPTIONS;

    if (pChan->baudRate == 0 || pChan->baudRate > MAX_BAUD)
	pChan->baudRate = BCM1250_UART_DEFAULT_BAUD;
    }

/*******************************************************************************
*
* bcm1250DuartInitChannel - initialize a single channel
*
* This routine initializes the specified channel.  It is required
* that the transmitters and receivers have been issued s/w reset commands
* before the mode registers, clock select registers, auxillary clock
* registers & output port configuration registers are changed.
*
* This routine only sets the initial state as part of bcm1250DuartDevInit.  The
* user can change this state through ioct clommands as desired.
*
* RETURNS: N/A.
*/

LOCAL void bcm1250DuartInitChannel
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    /* Reset the transmitters  & receivers  */

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD(pChan->chanBase + R_DUART_CMD, V_DUART_MISC_CMD_RESET_BREAK_INT);

    bcm1250DuartOptsSet (pChan, pChan->options); /* TX is disabled */

    bcm1250DuartBaudSet (pChan, pChan->baudRate);
    }

/*******************************************************************************
*
* bcm1250DuartOptsSet - set the serial options
*
* Set the channel operating mode to that specified.  All sioLib options
* are supported: CLOCAL, CREAD, CSIZE, PARENB, and PARODD.
*
* Note, this routine disables the transmitter.  The calling routine
* may have to re-enable it.
*
* RETURNS:
* Returns OK to indicate success, otherwise ERROR is returned
*/

LOCAL STATUS bcm1250DuartOptsSet
    (
    BCM1250_DUART_CHAN * pChan, /* ptr to channel */
    UINT options	/* new hardware options */
    )
    {
    UINT8 mr1Value = 0;
    UINT8 mr2Value = 0;
    int lvl;

    if (pChan == NULL || options & 0xffffff00)
	return ERROR;

    /* Reset the transmitters  & receivers  */

    switch (options & CSIZE)
	{
	case CS7:
	    mr1Value = V_DUART_BITS_PER_CHAR_7; break;
	default:
	case CS8:
	    mr1Value = V_DUART_BITS_PER_CHAR_8; break;
	}

    if (options & STOPB)
	mr2Value = M_DUART_STOP_BIT_LEN_2;
    else
	mr2Value = M_DUART_STOP_BIT_LEN_1;

    switch (options & (PARENB|PARODD))
	{
	case PARENB|PARODD:
	    mr1Value |= V_DUART_PARITY_MODE_ADD | M_DUART_PARITY_TYPE_ODD;
	    break;
	case PARENB:
	    mr1Value |= V_DUART_PARITY_MODE_ADD | M_DUART_PARITY_TYPE_EVEN;
	    break;
	default:
	case 0:
	    mr1Value |= V_DUART_PARITY_MODE_NONE;
	    break;
	}

    if (!(options & CLOCAL))
	{
	/* clocal disables hardware flow control */
	mr1Value |= M_DUART_RX_RTS_ENA;
	mr2Value |= M_DUART_TX_CTS_ENA;
	}

    lvl = intLock ();

    /* now reset the channel mode registers */

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD (pChan->chanBase + R_DUART_CMD, M_DUART_RX_DIS | M_DUART_TX_DIS);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD (pChan->chanBase + R_DUART_CMD, V_DUART_MISC_CMD_RESET_TX);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD (pChan->chanBase + R_DUART_CMD, V_DUART_MISC_CMD_RESET_RX);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    MIPS3_SD (pChan->chanBase + R_DUART_MODE_REG_1, mr1Value);  /* mode register 1  */
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    MIPS3_SD (pChan->chanBase + R_DUART_MODE_REG_2, mr2Value);  /* mode register 2  */
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    if (options & CREAD) {
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	MIPS3_SD (pChan->chanBase + R_DUART_CMD, M_DUART_RX_EN);
    }
 
    intUnlock (lvl);

    pChan->options = options;

    return OK;
    }

/*******************************************************************************
*
* bcm1250DuartIoctl - special device control
*
* RETURNS:
* Returns OK on success, EIO on device error, ENOSYS on unsupported
* request.
*/

LOCAL int bcm1250DuartIoctl
    (
    BCM1250_DUART_CHAN *	pChan,		/* device to control */
    int		request,		/* request code */
    void *	someArg			/* some argument */
    )
    {
    STATUS result;
    int     arg = (int)someArg;

    switch (request)
	{
	case SIO_BAUD_SET:
	    return (bcm1250DuartBaudSet (pChan, arg) == OK ? OK : EIO);

	case SIO_BAUD_GET:
	    *(int *)arg = pChan->baudRate;
	    return (OK);

	case SIO_MODE_SET:
	    return (bcm1250DuartModeSet (pChan, arg) == OK ? OK : EIO);

	case SIO_MODE_GET:
	    *(int *)arg = pChan->mode;
	    return (OK);

	case SIO_AVAIL_MODES_GET:
	    *(int *)arg = SIO_MODE_INT | SIO_MODE_POLL;
	    return (OK);

	case SIO_HW_OPTS_SET:
	    /* change options, then set mode to restart chip correctly */
	    result = bcm1250DuartOptsSet (pChan, arg);
	    bcm1250DuartModeSet (pChan, pChan->mode);
	    return result;

	case SIO_HW_OPTS_GET:
	    *(int *)arg = pChan->options;
	    return (OK);

	default:
	    return (ENOSYS);
	}
    }


/*******************************************************************************
*
* bcm1250DuartIntWr - handle a transmitter interrupt
*
* This routine handles write interrupts from the DUART. This isr is invoked
* when the TxRDY bit in the interrupt status register has been set. If there
* is no character to transmit the transmitter for the channel is disabled.
*
* RETURNS: N/A
*/

LOCAL void bcm1250DuartIntWr
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    char            outChar;
    unsigned long long imr;

    if ((*pChan->getTxChar) (pChan->getTxArg, &outChar) != ERROR)
	{
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	MIPS3_SD(pChan->chanBase + R_DUART_TX_HOLD, outChar);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	}
    else
        {
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	imr = MIPS3_LD(pChan->chanIMR);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	imr &= ~ M_DUART_IMR_TX;
	MIPS3_SD(pChan->chanIMR, imr);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	}
    }

/*****************************************************************************
*
* bcm1250DuartIntRd - handle a reciever interrupt
*
* This routine handles read interrupts from the DUART.
* The DUART has been programmed to generate read interrupts when the RXRDY
* status bit has been set in the interrupt status register. When a character
* has been  received it is removed from the channels receive buffer.
*
* RETURNS: N/A
*/

LOCAL void bcm1250DuartIntRd
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    unsigned long long inchar;
    unsigned long long statusReg;

#ifdef PASS2_DUART_WORKAROUND
    MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
    statusReg = MIPS3_LD (pChan->chanBase + R_DUART_STATUS);
#ifdef PASS2_DUART_WORKAROUND
    MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

    while ((statusReg & M_DUART_RX_RDY) != 0x00)
        {
	inchar = MIPS3_LD (pChan->chanBase + R_DUART_RX_HOLD);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif

	(*pChan->putRcvChar) (pChan->putRcvArg, (char)inchar);
	if (pChan->mode != SIO_MODE_INT)
	    break;
	statusReg = MIPS3_LD (pChan->chanBase + R_DUART_STATUS);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
        }
    }


/*******************************************************************************
*
* bcm1250DuartInt - handle all DUART interrupts in one vector
*
* This routine handles all interrupts in a single interrupt vector.
* It identifies and services each interrupting source in turn, using
* edge-sensitive interrupt controllers.
*
* RETURNS: N/A
*/

void bcm1250DuartInt
    (
    BCM1250_DUART_CHAN * pChan
    )
    {
    unsigned long long        	intStatus;
    unsigned long long        	imrStatus;

    for ( 
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1),
#endif
             intStatus = MIPS3_LD(pChan->chanISR), 
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1),
#endif
             imrStatus = MIPS3_LD(pChan->chanIMR);

             ((intStatus & imrStatus) & (M_DUART_ISR_TX | M_DUART_ISR_RX)) != 0; 

#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1),
#endif
             intStatus = MIPS3_LD(pChan->chanISR), 
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1),
#endif
             imrStatus = MIPS3_LD(pChan->chanIMR)
#ifdef PASS2_DUART_WORKAROUND
        ,MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1)
#endif
        )
	{
	if ((intStatus & M_DUART_ISR_TX) != 0) {
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	    bcm1250DuartIntWr (pChan);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	    }
	  
	if ((intStatus & M_DUART_ISR_RX) != 0) {
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
	    bcm1250DuartIntRd (pChan);
#ifdef PASS2_DUART_WORKAROUND
        MIPS3_LD (pChan->chanBase + R_DUART_MODE_REG_1);
#endif
            }
	}
    }

