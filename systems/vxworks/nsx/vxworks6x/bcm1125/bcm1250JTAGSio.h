/* bcm1250JTAGSio.h - Broadcom BCM1250 JTAG comm driver header */

/* Copyright 2002-2004 Wind River Systems, Inc. */

/* $Id: bcm1250JTAGSio.h,v 1.3 2011/07/21 16:14:48 yshtil Exp $
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
01c,11apr05,kab  fix comments for apiGen (SPR 107842)
01b,05may04,agf  fix compiler warnings
01a,15nov01,agf	 created.
*/

#ifndef __INCbcm1250JTAGSioh
#define __INCbcm1250JTAGSioh

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	_ASMLANGUAGE

#include "sioLib.h"

/* device and channel structures */

typedef struct bcm1250JTAG	/* BCM1250_JTAG_CHAN_CHANNEL */
    {
    /* always goes first */
    SIO_CHAN	sio;		/* driver functions */
  
    /* callbacks */
  
    STATUS	(*getTxChar)();	/* pointer to a xmitr function */
    STATUS	(*putRcvChar)();/* pointer to a recvr function */
    void *	getTxArg;
    void *	putRcvArg;
  
    UINT32	channel;	/* channel number (0) */

    /* channel registers */
    UINT32        base_reg;
  
    /* misc values */
    UINT32	mode;		/* mode (poll or int) */

    int		waiting_input;
    unsigned long long input_buf;
  
    } BCM1250_JTAG_CHAN;



#if defined(__STDC__) || defined(__cplusplus)

/* serial procedures */
IMPORT	void	bcm1250JTAGDevInit	(BCM1250_JTAG_CHAN *);
IMPORT	void	bcm1250JTAGDevInit2	(BCM1250_JTAG_CHAN *);

#else   /* __STDC__ */

IMPORT	void	bcm1250JTAGDevInit	();
IMPORT	void	bcm1250JTAGDevInit2	();

#endif  /* __STDC__ */

#endif	/* _ASMLANGUAGE */

/* channels */
#define JTAG_CHANNEL_A   0
#define JTAG_N_CHANS     1	/* number of serial channels on chip */


/* SIO -- bcm1250Duart serial channel chip -- register definitions */

  /* registers offsets */
#define JTAG_CONS_CONTROL   0x00
#define JTAG_CONS_INPUT     0x20
#define JTAG_CONS_OUTPUT    0x40
#define JTAG_CONS_MAGICNUM  0x50FABEEF12349873ULL


#ifdef __cplusplus
}
#endif

#endif /* __INCbcm1250JTAGSioh */
