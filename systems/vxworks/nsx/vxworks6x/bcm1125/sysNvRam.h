/* sysNvRam.h - CFE interface to the nvRAM parameters */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: sysNvRam.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
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
01a,03oct02,agf  written
*/
 
#ifndef __INCsysNvRamh
#define __INCsysNvRamh
 
#ifdef __cplusplus
extern "C" {
#endif
 
#ifndef _ASMLANGUAGE                                                                    

/*
 * TLV types.  These codes are used in the "type-length-value"
 * encoding of the items stored in the NVRAM device (flash or EEPROM)
 *
 * The layout of the flash/nvram is as follows:
 *
 * <type> <length> <data ...> <type> <length> <data ...> <type_end>
 *
 * The type code of "ENV_TLV_TYPE_END" marks the end of the list.
 * The "length" field marks the length of the data section, not
 * including the type and length fields.
 *
 * Environment variables are stored as follows:
 *
 * <type_env> <length> <flags> <name> = <value>
 *
 * If bit 0 (low bit) is set, the length is an 8-bit value.
 * If bit 0 (low bit) is clear, the length is a 16-bit value
 *
 * Bit 7 set indicates "user" TLVs.  In this case, bit 0 still
 * indicates the size of the length field.
 *
 * Flags are from the constants below:
 *
 */
 
#define ENV_LENGTH_16BITS       0x00    /* for low bit */
#define ENV_LENGTH_8BITS        0x01
 
#define ENV_TYPE_USER           0x80
 
#define ENV_CODE_SYS(n,l) (((n)<<1)|(l))
#define ENV_CODE_USER(n,l) ((((n)<<1)|(l)) | ENV_TYPE_USER)
 
/*
 * The actual TLV types we support
 */
 
#define ENV_TLV_TYPE_END        0x00
#define ENV_TLV_TYPE_ENV        ENV_CODE_SYS(0,ENV_LENGTH_8BITS)                

 
/*
 * Environment variable flags
 */
 
#define ENV_FLG_NORMAL          0x00    /* normal read/write */
#define ENV_FLG_BUILTIN         0x01    /* builtin - not stored in flash */
#define ENV_FLG_READONLY        0x02    /* read-only - cannot be changed */     

#endif  /* _ASMLANGUAGE */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __INCsysNvRamh */                                                                                                                                              
