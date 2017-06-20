/* x1240RtcEeprom.h - constants for accessing the x1240 serial EEPROM */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: x1240RtcEeprom.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
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
01b,03oct02,agf  changes for shared sentosa support
01a,08jan01,agf  written
*/
 
#ifndef __INCx1240RtcEepromh
#define __INCx1240RtcEepromh
 
#ifdef __cplusplus
extern "C" {
#endif
 
#ifndef _ASMLANGUAGE                                                                    

/*
 * Device constants
 */

#define X1241_EEPROM_SIZE  2048

/*
 * Register bits
 */

#define X1241REG_SR_BAT     0x80    /* currently on battery power */
#define X1241REG_SR_RWEL    0x04    /* r/w latch is enabled, can write RTC */
#define X1241REG_SR_WEL     0x02    /* r/w latch is unlocked, can enable r/w now */
#define X1241REG_SR_RTCF    0x01    /* clock failed */
#define X1241REG_BL_BP2     0x80    /* block protect 2 */
#define X1241REG_BL_BP1     0x40    /* block protect 1 */
#define X1241REG_BL_BP0     0x20    /* block protect 0 */
#define X1241REG_BL_WD1     0x10
#define X1241REG_BL_WD0     0x08
#define X1241REG_HR_MIL     0x80    /* military time format */

/*
 * Register numbers
 */

#define X1241REG_BL     0x10    /* block protect bits */
#define X1241REG_INT    0x11    /*  */
#define X1241REG_SC     0x30    /* Seconds */
#define X1241REG_MN     0x31    /* Minutes */
#define X1241REG_HR     0x32    /* Hours */
#define X1241REG_DT     0x33    /* Day of month */
#define X1241REG_MO     0x34    /* Month */
#define X1241REG_YR     0x35    /* Year */
#define X1241REG_DW     0x36    /* Day of Week */
#define X1241REG_Y2K    0x37    /* Year 2K */
#define X1241REG_SR     0x3F    /* Status register */

#define X1241_CCR_ADDRESS   0x6F
#define X1241_ARRAY_ADDRESS 0x57

#endif  /* _ASMLANGUAGE */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __INCx1240RtcEepromh */                                                                                                                                              
