/* configNet.h - Network configuration header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/* $Id: configNet.h,v 1.2 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
01c,29may02,jmb  get rid of FEI, call BSP wrapper for BCM570x END
                 load function
01b,02may02,jmb  Fix init string for BCM5701
01a,31mar98,cn   written.
*/

#ifndef INCconfigNeth
#define INCconfigNeth

#include "vxWorks.h"
#include "end.h"

/*
 * Broadcom StrataSwitch (TM) SENS device.
 */
#undef INCLUDE_STRATASWITCH_END
#ifdef INCLUDE_STRATASWITCH_END
IMPORT END_OBJ *socend_load(char *is, void*);
#define STRATA_FUNC socend_load
#define STRATA_INIT_STR  "0:0x08000000"
#endif
/*
 * Default SENS device: DEC-21143-XX (onboard ethernet)
 * Int Str= unit:MBAR:MBAR:ivec:ilvl:mem:memsize:usrFlags:offset
 */
#ifdef INCLUDE_END_DEC_21X4X
#define DEC_21X4X_FUNC       dec21x4xEndLoad
/* Onboard */
#define DEC_21X4X_STRING     "0x80000000:0:1:1:-1:-1:0:0x40004000"

/* RAMIX PMC Adapter */
#define DEC_21X4X_STRING1    "0xfd001000:0:2:2:-1:-1:0:0x40004000"
#define DEC_21X4X_STRING2    "0xfd002000:0:2:2:-1:-1:0:0x40004000"
#define DEC_21X4X_STRING3    "0xfd003000:0:2:2:-1:-1:0:0x40004000"
#define DEC_21X4X_STRING4    "0xfd004000:0:2:2:-1:-1:0:0x40004000"

IMPORT END_OBJ* DEC_21X4X_FUNC(char*, void*);
#endif /* INCLUDE_END_DEC21X4X */

#ifdef INCLUDE_END_DEC_21X40
/*
 * SENS Device for DEC_21X40 (21140-AF)
 */
#define	DEC_21X40_FUNC	     	dec21x40EndLoad
#define DEC_21X40_STRING 	"0x80000000:0:3:3:32:32:-1:-1:0x80000000"
IMPORT END_OBJ* DEC_21X40_FUNC(char*, void*);
#endif /* INCLUDE_END_DEC_21X40 */

/*
 * SENS Device for BCM570x 10/100/1000 PCI Ethernet chip
 */
#include "etherMultiLib.h"
#include "end.h"
#include "endLib.h"

IMPORT END_OBJ *sysBcm570xEndLoad(char *initString, void* ap);
#define BCM570X_FUNC sysBcm570xEndLoad

/* Format is unit#, PCI Memory Base, Iline, align */
/* Unit number is prepended in muxDevLoad, don't put it here */

#define BCM570X_INIT_STRING "0x80000000:1:0" 

/* CPCI adapter */
/* #define BCM570X_INIT_STRING "0x80000000:2:0" */



END_TBL_ENTRY endDevTbl [] =
{
  { 0, BCM570X_FUNC, BCM570X_INIT_STRING, 0, NULL, FALSE},
  { 0, END_TBL_END, NULL, 0, NULL, FALSE}
};

#endif /* INCconfigNeth */

