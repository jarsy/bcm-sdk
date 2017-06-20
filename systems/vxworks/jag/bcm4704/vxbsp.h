/*
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
    

*/
/*
 * Copyright(c) 2001 Broadcom Corp.
 * All Rights Reserved.
 * $Id: vxbsp.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 */

#ifndef __INCbcm47xxh
#define	__INCbcm47xxh

#include "vxWorks.h"

#include "hnbutypedefs.h"
#include "sbconfig.h"
#include "bcm4704.h"
#include "sbutils.h"

#ifndef KSEG1ADDR
#define KSEG1ADDR(_a) ((unsigned long)(_a) | 0xA0000000)
#endif

#define DEF_SBINTVEC    0x00000001            /* MIPS_timer + UART */  
#define DEF_SBIPSFLAG   0x043F0201      /* 4=pci, 3=Not used, 2=et1, 1=et0 */

#define BCM47XX_SR      (SR_CU0| INT_LVL_IORQ0 | INT_LVL_IORQ4 | \
                         INT_LVL_IORQ3 | INT_LVL_IORQ1 | \
                         INT_LVL_TIMER | INT_LVL_IORQ2 | SR_IE)

/* interrupt priority */
#define INT_PRIO_MSB            TRUE   /* interrupt priority msb highest */

/* interrupt levels */
#define INT_LVL_TIMER           SR_IBIT8        /* timer (fixed) */
#define INT_LVL_IORQ4           SR_IBIT7        /* IORQ 4 */
#define INT_LVL_IORQ3           SR_IBIT6        /* IORQ 3 */
#define INT_LVL_IORQ2           SR_IBIT5        /* IORQ 2 */
#define INT_LVL_IORQ1           SR_IBIT4        /* IORQ 1 */
#define INT_LVL_IORQ0           SR_IBIT3        /* IORQ 0 */
#define INT_LVL_SW1             SR_IBIT2        /* sw interrupt 1 (fixed) */
#define INT_LVL_SW0             SR_IBIT1        /* sw interrupt 0 (fixed) */

/* interrupt indexes */
#define INT_INDX_TIMER          7       /* timer (fixed) */
#define INT_INDX_IORQ4          6
#define INT_INDX_IORQ3          5       /* IORQ 3 */
#define INT_INDX_IORQ2          4       /* IORQ 2 */
#define INT_INDX_IORQ1          3       /* IORQ 1 */
#define INT_INDX_IORQ0          2       /* IORQ 0 */
#define INT_INDX_SW1            1       /* sw interrupt 1       */
#define INT_INDX_SW0            0       /* sw interrupt 0       */

/* interrupt vectors */

/* shared mips int 0 */
#ifdef INCLUDE_PCMCIA
#define IV_LAST_VEC             82
#define IV_EXT_ALT4_VEC 		81	   
#define IV_EXT_ALT3_VEC 		80
#else
#define IV_LAST_VEC             80
#endif
#define IV_EXT_ALT2_VEC 		79	   
#define IV_EXT_ALT1_VEC 		78	   
#define IV_IORQ0_BIT6_VEC		77	   
#define IV_IORQ0_BIT5_VEC		76	   
#define IV_IORQ0_BIT4_VEC		75	   
#define IV_IORQ0_BIT3_VEC		74	   
#define IV_IORQ0_BIT2_VEC		73	   
#define IV_IORQ0_BIT1_VEC		72	   
#define IV_IORQ0_BIT0_VEC		71	   

#define IV_RTIME_VEC            70      /* timer (fixed)          */
#define IV_IORQ4_VEC            69      
#define IV_IORQ3_VEC            68      
#define IV_IORQ2_VEC            67      
#define IV_IORQ1_VEC            66      
#define IV_IORQ0_VEC            65      

#define INT_VEC_IORQ0           IV_IORQ0_VEC
#define INT_VEC_IORQ1           IV_IORQ1_VEC
#define INT_VEC_IORQ2           IV_IORQ2_VEC
#define INT_VEC_IORQ3           IV_IORQ3_VEC
#define INT_VEC_IORQ4           IV_IORQ4_VEC
#define INT_VEC_IORQ5           IV_RTIME_VEC


extern uint32 get_sb_clock(void); /* routine to return sb clock */

#endif	/* __INCbcm47xxh */
