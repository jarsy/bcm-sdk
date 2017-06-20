/* $Id: sysL2Backcache.h,v 1.2 2011/07/21 16:14:08 yshtil Exp $
 *****************************************************************
 *  File - l2cache.h
 *  This file defines registers and values needed in defining 
 *  l2 cache functions.
 *  10Sep98,My	Created
 */

#ifndef _INCsysL2Backh
#define _INCsysL2Backh

#include "vxWorks.h"

#define	L2CR_REG   	          1017	/* l2CR register number */
#define HID0_REG		  1008
#define PVR_REG			   287  /* pvr register number */
#define ARTHUR			     8  /* Upper bit 16 bit value of 740/750 */




#define WRITE_ADDR_U 		0x0060	/* upper 16 bits of write address */
#define L2_SIZE_1M_U 		0x0010	/* upper 16 bitts of 1 Meg */
#define L2_ADR_INCR 		0x100	/* address increament value */
#define L2_SIZE_1M 		0x1000  /* 1 MG (0x100000) / 0x100 = 0x1000 */
#define L2_SIZE_HM 		0x800   /* 512K counts  */ 
#define L2_SIZE_QM 		0x400   /* 256K(0x40000) / L2_ADR_INCR = 0x40  */

/*
 * Defining values for L2CR register: 
 *  -  L2 cache enable (1) / disable (0)  (bit 0)
 *  -  cache size (bits 2-3; 3: 1 MB, 2: 512 KB, 1: 256 KB)
 *  -  1.5 clock ratio (bits 4-6)
 *  -  Pinpelined (register-register) synchronous burst RAM (bits 7-8)
 *  -  L2 Data only (bit 9)
 *  -  Test mode on (1) or off (0) (bit 13)
 *
 */

#define L2CR_1M_TST_U  		0x3526  /* 1MB Data, Global Inv., Test mode on */
#define L2CR_1M_TST 		0x35260000 
#define L2CR_1M_U 		0x3502  /* 1MB Data, Test mode off   */
#define L2CR_HM_U 		0x2502  /* 512KB Data, Test mode of  */
#define L2CR_QM_U 		0x1502  /* 512KB Data, Test mode of  */

#define L2CR_DISABLE_MASK_U 	0x7fff	/* Disable L2 - upper 16 bits  */
#define L2CR_GLOBAL_INV_U 	0x0020  /* Global invalidate - upper 16 bits */
#define L2CR_EN_U               0x8000  /* Set L2CR enable bit */

#define L1_DCACHE_ENABLE	0x4400
#define L1_DCACHE_INV_MASK	0xfbff
#define L1_DCACHE_DISABLE	0xbfff		

#ifndef _ASMLANGUAGE

  extern void  sysL1DcacheEnable(void);
  extern void  sysL2BackEnable(ULONG);
  extern ULONG sysL2BackAutoSize(void);
  extern void  sysL1DcacheDisable(void);
  extern void  sysL2BackDisable(void);
  extern void  sysL2BackGlobalInv(void);
  extern void  sysL2CRWrite(ULONG);
  extern ULONG sysPVRReadBoot();
  extern ULONG sysPVRReadSys();
  extern ULONG sysL2CRRead();

#endif

#endif

