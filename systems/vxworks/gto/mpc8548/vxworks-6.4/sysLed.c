/* sysLed.c - Wind River SBC8548 User LED driver */

/* $Id: sysLed.c,v 1.4 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,13feb06,kds  cleaned up code
01a,11aug05,kds  adapted for wrSbc8548 from wrSbc834x (rev 01a)
*/

/*
DESCRIPTION
This module contains the LED driver.

INCLUDE FILES: sysLed.h
*/

/* includes */
#include "sysLed.h"

#ifdef INCLUDE_SYSLED

/* locals */

void sysLedOn()
{
    UINT reg;
    
    reg = *M85XX_GPOUTDR(CCSBAR);
    reg |= (1 << 7);
    *M85XX_GPOUTDR(CCSBAR) = reg;   
}

void sysLedOff()
{
    UINT reg;
    
    reg = *M85XX_GPOUTDR(CCSBAR);
    reg &= ~(1 << 7);
    *M85XX_GPOUTDR(CCSBAR) = reg;   
}
#endif /* INCLUDE_SYSLED */
