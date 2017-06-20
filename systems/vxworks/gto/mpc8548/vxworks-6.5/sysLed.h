/* sysLed.h - system-dependent LED driver Header File */

/* $Id: sysLed.h,v 1.2 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,11aug05,kds  adapted for wrSbc8548 from wrSbc834x (rev 01a)
*/


#ifndef  __INCsysLedh
#define  __INCsysLedh

#ifdef __cplusplus
extern "C" {
#endif

/* Defines */
/* LED Character display */
#define LED_REG_BASE      ALPHA_LED_BASE_ADRS 
#define LED_REG(x)        (*(volatile UINT8 *) (LED_REG_BASE + ((x) << 3)))

/* Prototypes */

#ifndef _ASMLANGUAGE

#if defined(__STDC__) || defined(__cplusplus)

IMPORT void sysLedOn(void);
IMPORT void sysLedOff(void);
#else

IMPORT void sysLedOn();
IMPORT void sysLedOff();
#endif  /* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif   /* __INCsysLedh  */

