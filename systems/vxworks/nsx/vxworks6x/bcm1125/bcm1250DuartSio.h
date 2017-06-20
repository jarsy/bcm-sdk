/* bcm1250DuartSio.h - Broadcom BCM1250 DUART header file */

/* Copyright 2001 Wind River Systems, Inc. */

/* $Id: bcm1250DuartSio.h,v 1.3 2011/07/21 16:14:48 yshtil Exp $
********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

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
01b,11apr05,kab  fix comments for apiGen (SPR 107842)
01a,15nov01,agf   written.
*/

#ifndef __INCbcm1250DuartSioh
#define __INCbcm1250DuartSioh

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	_ASMLANGUAGE

#include "sioLib.h"

/* device and channel structures */


typedef struct bcm1250DuartChan	/* BCM1250_DUART_CHAN */
{
  /* always goes first */
  SIO_CHAN	sio;		/* driver functions */
  
  /* callbacks */
  
  STATUS	(*getTxChar)();	/* pointer to a xmitr function */
  void *	getTxArg;
  STATUS	(*putRcvChar)();/* pointer to a recvr function */
  void *	putRcvArg;

  /* pointers to register regions */

  volatile char	*duartBase;
  int		channel;

  volatile char	*chanBase;
  volatile unsigned long long *chanIMR;
  volatile unsigned long long *chanISR;

  /* misc values */
  
  UINT		baudRate;	/* current baud rate */
  UINT		options;	/* current options, SIO_HW_OPTS_SET */
  
  /* interrupt/polled mode configuration info */
  
  UINT		mode;		/* SIO_MODE_[INT | POLL] */
  int		intVecNum;	/* CPU interrupt vector number */
  int		intSource;	/* BMC1250 interrupt source number */
  BOOL		intEnable;	/* allow interrupt mode flag */
  
} BCM1250_DUART_CHAN;

/* channels */

#define BCM1250_DUART_CHANNEL_A   0
#define BCM1250_DUART_CHANNEL_B   1

#if defined(__STDC__) || defined(__cplusplus)

/* serial procedures */
IMPORT	void	bcm1250DuartDevInit	(BCM1250_DUART_CHAN *);
IMPORT	void	bcm1250DuartDevInit2	(BCM1250_DUART_CHAN *);
IMPORT	void	bcm1250DuartInt	        (BCM1250_DUART_CHAN *);

#else   /* __STDC__ */

IMPORT	void	bcm1250DuartDevInit	();
IMPORT	void	bcm1250DuartDevInit2	();
IMPORT	void	bcm1250DuartInt	();

#endif  /* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCbcm1250DuartSioh */
