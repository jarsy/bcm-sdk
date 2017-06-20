/* m24lv128Eeprom.h - constants for accessing the x1240 serial EEPROM */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: m24lv128Eeprom.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
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
01b,18dec02,agf  reduce size of EEPROM
01a,03oct02,agf  written
*/
 
#ifndef __INCm24lv128Eepromh
#define __INCm24lv128Eepromh
 
#ifdef __cplusplus
extern "C" {
#endif
 
#ifndef _ASMLANGUAGE                                                                    

/*
 * Device constants
 */

/* this can be increased if needed, but will affect boot-up times */

#define M24LV128_EEPROM_SIZE  1024

#endif  /* _ASMLANGUAGE */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __INCm24lv128Eepromh */                                                                                                                                              
