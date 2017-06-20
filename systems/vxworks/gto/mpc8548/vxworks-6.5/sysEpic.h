/* sysEpic.h - Embedded Programmable Interrupt Controller (EPIC) driver */

/* $Id: sysEpic.h,v 1.3 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* Copyright 1996, 1998 Motorola, Inc. */

/*
modification history
--------------------
01b,27jan06,dtr  Tidy up - coding conventions.
01a,06may05,dtr  Modified for 8548 from cds85xx/01h
*/

#ifndef	__INCsysEpich
#define	__INCsysEpich

#ifdef __cplusplus
extern "C" {
#endif

/*  interrupt handler description  */
    
typedef struct intHandlerDesc     
    {
    VOIDFUNCPTR			vec;	/* interrupt vector */
    int				arg;	/* interrupt handler argument */
    struct  intHandlerDesc *	next;	/* pointer to the next handler */
    } INT_HANDLER_DESC;

/*   limit values  */

/* bits 31(MSB)...0(LSB) enable only 1 bit at bit x */
#define EPIC_BIT(x)          (1 << (x))

/* bits 31(MSB)...0(LSB) enable from bit x to bit y where y>x */
#define EPIC_BITS_M2N(x, y)  (( 1 << ((y)-(x)+1) ) - 1) << (x)

/* bits 31(MSB)...0(LSB) from bit x towards LSB enable y bits */          
#define EPIC_BITS(x, y)      EPIC_BITS_M2N(x-y+1, x)

#define  INTERRUPT_TABLESIZE   256
    
#define EPIC_CCSROFF  		0x40000			/* EUMBBAR of EPIC  */

/* default is LEVEL sensitive, ACTIVE_HIGH polarity */

#ifndef EPIC_EX_DFT_SENSE
#   define EPIC_EX_DFT_SENSE	EPIC_SENSE_LVL
#endif  /* EPIC_EX_DFT_SENSE */

#ifndef EPIC_EX_DFT_POLAR
#   define EPIC_EX_DFT_POLAR	EPIC_INT_ACT_HIGH
#endif  /* EPIC_EX_DFT_POLAR */

#ifndef EPIC_IN_DFT_POLAR
#   define EPIC_IN_DFT_POLAR	EPIC_INT_ACT_HIGH
#endif  /* EPIC_IN_DFT_POLAR */

/*   Private Access Registers  */
#define EPIC_IPI_DPATCH_REG0	(EPIC_CCSROFF + 0x00040)/* IPI0 dispatch */
#define EPIC_IPI_DPATCH_REG1	(EPIC_CCSROFF + 0x00050)/* IPI1 dispatch */
#define EPIC_IPI_DPATCH_REG2	(EPIC_CCSROFF + 0x00060)/* IPI2 dispatch */
#define EPIC_IPI_DPATCH_REG3	(EPIC_CCSROFF + 0x00070)/* IPI3 dispatch */
#define EPIC_CTASK_PRI_REG	(EPIC_CCSROFF + 0x00080)/* Cur Task Prio */
#define EPIC_WHO_AM_I_REG	(EPIC_CCSROFF + 0x00090)/* Who am I */
#define EPIC_INT_ACK_REG	(EPIC_CCSROFF + 0x000a0)/* Int ack */
#define EPIC_EOI_REG		(EPIC_CCSROFF + 0x000b0)/* End of Int */

/*   Global and Timer  */
#define EPIC_FEATURES_REG	(EPIC_CCSROFF + 0x01000)/* Feature reporting */
#define EPIC_GLOBAL_REG		(EPIC_CCSROFF + 0x01020)/* Global config.  */
#define EPIC_VENDOR_ID_REG	(EPIC_CCSROFF + 0x01080)/* Vendor id */
#define EPIC_PROC_INIT_REG	(EPIC_CCSROFF + 0x01090)/* Processor init. */
#define EPIC_IPI_0_VEC_REG	(EPIC_CCSROFF + 0x010a0)/* IPI0 vect/prio */
#define EPIC_IPI_1_VEC_REG	(EPIC_CCSROFF + 0x010b0)/* IPI1 vect/prio */
#define EPIC_IPI_2_VEC_REG	(EPIC_CCSROFF + 0x010c0)/* IPI2 vect/prio */
#define EPIC_IPI_3_VEC_REG	(EPIC_CCSROFF + 0x010d0)/* IPI3 vect/prio */
#define EPIC_SPUR_VEC_REG	(EPIC_CCSROFF + 0x010e0)/* Spurious vector */
#define EPIC_TM_FREQ_REG	(EPIC_CCSROFF + 0x010f0)/* Timer Frequency */

#define EPIC_TM0_CUR_COUNT_REG	(EPIC_CCSROFF + 0x01100)/* Gbl TM0 Cur. Count*/
#define EPIC_TM0_BASE_COUNT_REG	(EPIC_CCSROFF + 0x01110)/* Gbl TM0 Base Count*/
#define EPIC_TM0_VEC_REG	(EPIC_CCSROFF + 0x01120)/* Gbl TM0 Vector Pri*/
#define EPIC_TM0_DES_REG	(EPIC_CCSROFF + 0x01130)/* Gbl TM0 Dest. */

#define EPIC_TM1_CUR_COUNT_REG	(EPIC_CCSROFF + 0x01140)/* Gbl TM1 Cur. Count*/
#define EPIC_TM1_BASE_COUNT_REG	(EPIC_CCSROFF + 0x01150)/* Gbl TM1 Base Count*/
#define EPIC_TM1_VEC_REG	(EPIC_CCSROFF + 0x01160)/* Gbl TM1 Vector Pri*/
#define EPIC_TM1_DES_REG	(EPIC_CCSROFF + 0x01170)/* Gbl TM1 Dest. */

#define EPIC_TM2_CUR_COUNT_REG	(EPIC_CCSROFF + 0x01180)/* Gbl TM2 Cur. Count*/
#define EPIC_TM2_BASE_COUNT_REG	(EPIC_CCSROFF + 0x01190)/* Gbl TM2 Base Count*/
#define EPIC_TM2_VEC_REG	(EPIC_CCSROFF + 0x011a0)/* Gbl TM2 Vector Pri*/
#define EPIC_TM2_DES_REG	(EPIC_CCSROFF + 0x011b0)/* Gbl TM2 Dest */

#define EPIC_TM3_CUR_COUNT_REG	(EPIC_CCSROFF + 0x011c0)/* Gbl TM3 Cur. Count*/
#define EPIC_TM3_BASE_COUNT_REG	(EPIC_CCSROFF + 0x011d0)/* Gbl TM3 Base Count*/
#define EPIC_TM3_VEC_REG	(EPIC_CCSROFF + 0x011e0)/* Gbl TM3 Vector Pri*/
#define EPIC_TM3_DES_REG	(EPIC_CCSROFF + 0x011f0)/* Gbl TM3 Dest. */

#define EPIC_TM_CTRL		(EPIC_CCSROFF + 0x01300)/* Timer Control */
#define EPIC_IRQ_SUMM_REG0	(EPIC_CCSROFF + 0x01310)/* IRQ_OUT Summary 0 */
#define EPIC_IRQ_SUMM_REG1	(EPIC_CCSROFF + 0x01320)/* IRQ_OUT Summary 1 */
#define EPIC_CRIT_SUMM_REG0	(EPIC_CCSROFF + 0x01330)/* Crit Int Summary 0 */
#define EPIC_CRIT_SUMM_REG1	(EPIC_CCSROFF + 0x01340)/* Crit Int Summary 1 */
#define EPIC_PERFMON_0_MSK_REG0	(EPIC_CCSROFF + 0x01350)/* PerfMon 0 Mask 0 */
#define EPIC_PERFMON_0_MSK_REG1	(EPIC_CCSROFF + 0x01360)/* PerfMon 0 Mask 1 */
#define EPIC_PERFMON_1_MSK_REG0	(EPIC_CCSROFF + 0x01370)/* PerfMon 1 Mask 0 */
#define EPIC_PERFMON_1_MSK_REG1	(EPIC_CCSROFF + 0x01380)/* PerfMon 1 Mask 1 */
#define EPIC_PERFMON_2_MSK_REG0	(EPIC_CCSROFF + 0x01390)/* PerfMon 2 Mask 0 */
#define EPIC_PERFMON_2_MSK_REG1	(EPIC_CCSROFF + 0x013a0)/* PerfMon 2 Mask 1 */
#define EPIC_PERFMON_3_MSK_REG0	(EPIC_CCSROFF + 0x013b0)/* PerfMon 3 Mask 0 */
#define EPIC_PERFMON_3_MSK_REG1	(EPIC_CCSROFF + 0x013c0)/* PerfMon 3 Mask 1 */
#define EPIC_MSG_REG0		(EPIC_CCSROFF + 0x01400)/* Message 0 */
#define EPIC_MSG_REG1		(EPIC_CCSROFF + 0x01410)/* Message 1 */
#define EPIC_MSG_REG2		(EPIC_CCSROFF + 0x01420)/* Message 2 */
#define EPIC_MSG_REG3		(EPIC_CCSROFF + 0x01430)/* Message 3 */
#define EPIC_MSG_EN_REG		(EPIC_CCSROFF + 0x01500)/* Message Enable */
#define EPIC_MSG_STATE_REG	(EPIC_CCSROFF + 0x01510)/* Message Status */

/*   Interrupt Source Config  */
#define EPIC_EX_INT0_VEC_REG	(EPIC_CCSROFF + 0x10000)/* Ext IRQ0 vect/prio*/
#define EPIC_EX_INT0_DES_REG	(EPIC_CCSROFF + 0x10010)/* Ext IRQ0 Dest */
#define EPIC_EX_INT1_VEC_REG	(EPIC_CCSROFF + 0x10020)/* Ext IRQ1 vect/prio*/
#define EPIC_EX_INT1_DES_REG	(EPIC_CCSROFF + 0x10030)/* Ext IRQ1 Dest */
#define EPIC_EX_INT2_VEC_REG	(EPIC_CCSROFF + 0x10040)/* Ext IRQ2 vect/prio*/
#define EPIC_EX_INT2_DES_REG	(EPIC_CCSROFF + 0x10050)/* Ext IRQ2 Dest */
#define EPIC_EX_INT3_VEC_REG	(EPIC_CCSROFF + 0x10060)/* Ext IRQ3 vect/prio*/
#define EPIC_EX_INT3_DES_REG	(EPIC_CCSROFF + 0x10070)/* Ext IRQ3 Dest */
#define EPIC_EX_INT4_VEC_REG	(EPIC_CCSROFF + 0x10080)/* Ext IRQ4 vect/prio*/
#define EPIC_EX_INT4_DES_REG	(EPIC_CCSROFF + 0x10090)/* Ext IRQ4 Dest */
#define EPIC_EX_INT5_VEC_REG	(EPIC_CCSROFF + 0x100a0)/* Ext IRQ5 vect/prio*/
#define EPIC_EX_INT5_DES_REG	(EPIC_CCSROFF + 0x100b0)/* Ext IRQ5 Dest */
#define EPIC_EX_INT6_VEC_REG	(EPIC_CCSROFF + 0x100c0)/* Ext IRQ6 vect/prio*/
#define EPIC_EX_INT6_DES_REG	(EPIC_CCSROFF + 0x100d0)/* Ext IRQ6 Dest */
#define EPIC_EX_INT7_VEC_REG	(EPIC_CCSROFF + 0x100e0)/* Ext IRQ7 vect/prio*/
#define EPIC_EX_INT7_DES_REG	(EPIC_CCSROFF + 0x100f0)/* Ext IRQ7 Dest */
#define EPIC_EX_INT8_VEC_REG	(EPIC_CCSROFF + 0x10100)/* Ext IRQ8 vect/prio*/
#define EPIC_EX_INT8_DES_REG	(EPIC_CCSROFF + 0x10110)/* Ext IRQ8 Dest */
#define EPIC_EX_INT9_VEC_REG	(EPIC_CCSROFF + 0x10120)/* Ext IRQ9 vect/prio*/
#define EPIC_EX_INT9_DES_REG	(EPIC_CCSROFF + 0x10130)/* Ext IRQ9 Dest */
#define EPIC_EX_INT10_VEC_REG	(EPIC_CCSROFF + 0x10140)/* Ext IRQ10 vect/pri*/
#define EPIC_EX_INT10_DES_REG	(EPIC_CCSROFF + 0x10150)/* Ext IRQ10 Dest */
#define EPIC_EX_INT11_VEC_REG	(EPIC_CCSROFF + 0x10160)/* Ext IRQ11 vect/pri*/
#define EPIC_EX_INT11_DES_REG	(EPIC_CCSROFF + 0x10170)/* Ext IRQ11 Dest */

#define EPIC_IN_INT0_VEC_REG	(EPIC_CCSROFF + 0x10200)/* Int IRQ0 vect/prio*/
#define EPIC_IN_INT0_DES_REG	(EPIC_CCSROFF + 0x10210)/* Int IRQ0 Dest */
#define EPIC_IN_INT1_VEC_REG	(EPIC_CCSROFF + 0x10220)/* Int IRQ1 vect/prio*/
#define EPIC_IN_INT1_DES_REG	(EPIC_CCSROFF + 0x10230)/* Int IRQ1 Dest */
#define EPIC_IN_INT2_VEC_REG	(EPIC_CCSROFF + 0x10240)/* Int IRQ2 vect/prio*/
#define EPIC_IN_INT2_DES_REG	(EPIC_CCSROFF + 0x10250)/* Int IRQ2 Dest */
#define EPIC_IN_INT3_VEC_REG	(EPIC_CCSROFF + 0x10260)/* Int IRQ3 vect/prio*/
#define EPIC_IN_INT3_DES_REG	(EPIC_CCSROFF + 0x10270)/* Int IRQ3 Dest */
#define EPIC_IN_INT4_VEC_REG	(EPIC_CCSROFF + 0x10280)/* Int IRQ4 vect/prio*/
#define EPIC_IN_INT4_DES_REG	(EPIC_CCSROFF + 0x10290)/* Int IRQ4 Dest */
#define EPIC_IN_INT5_VEC_REG	(EPIC_CCSROFF + 0x102a0)/* Int IRQ5 vect/prio*/
#define EPIC_IN_INT5_DES_REG	(EPIC_CCSROFF + 0x102b0)/* Int IRQ5 Dest */
#define EPIC_IN_INT6_VEC_REG	(EPIC_CCSROFF + 0x102c0)/* Int IRQ6 vect/prio*/
#define EPIC_IN_INT6_DES_REG	(EPIC_CCSROFF + 0x102d0)/* Int IRQ6 Dest */
#define EPIC_IN_INT7_VEC_REG	(EPIC_CCSROFF + 0x102e0)/* Int IRQ7 vect/prio*/
#define EPIC_IN_INT7_DES_REG	(EPIC_CCSROFF + 0x102f0)/* Int IRQ7 Dest */
#define EPIC_IN_INT8_VEC_REG	(EPIC_CCSROFF + 0x10300)/* Int IRQ8 vect/prio*/
#define EPIC_IN_INT8_DES_REG	(EPIC_CCSROFF + 0x10310)/* Int IRQ8 Dest */
#define EPIC_IN_INT9_VEC_REG	(EPIC_CCSROFF + 0x10320)/* Int IRQ9 vect/prio*/
#define EPIC_IN_INT9_DES_REG	(EPIC_CCSROFF + 0x10330)/* Int IRQ9 Dest */
#define EPIC_IN_INT10_VEC_REG	(EPIC_CCSROFF + 0x10340)/* Int IRQ10 vect/pri*/
#define EPIC_IN_INT10_DES_REG	(EPIC_CCSROFF + 0x10350)/* Int IRQ10 Dest */
#define EPIC_IN_INT11_VEC_REG	(EPIC_CCSROFF + 0x10360)/* Int IRQ11 vect/pri*/
#define EPIC_IN_INT11_DES_REG	(EPIC_CCSROFF + 0x10370)/* Int IRQ11 Dest */
#define EPIC_IN_INT12_VEC_REG	(EPIC_CCSROFF + 0x10380)/* Int IRQ12 vect/pri*/
#define EPIC_IN_INT12_DES_REG	(EPIC_CCSROFF + 0x10390)/* Int IRQ12 Dest */
#define EPIC_IN_INT13_VEC_REG	(EPIC_CCSROFF + 0x103a0)/* Int IRQ13 vect/pri*/
#define EPIC_IN_INT13_DES_REG	(EPIC_CCSROFF + 0x103b0)/* Int IRQ13 Dest */
#define EPIC_IN_INT14_VEC_REG	(EPIC_CCSROFF + 0x103c0)/* Int IRQ14 vect/pri*/
#define EPIC_IN_INT14_DES_REG	(EPIC_CCSROFF + 0x103d0)/* Int IRQ14 Dest */
#define EPIC_IN_INT15_VEC_REG	(EPIC_CCSROFF + 0x103e0)/* Int IRQ15 vect/pri*/
#define EPIC_IN_INT15_DES_REG	(EPIC_CCSROFF + 0x103f0)/* Int IRQ15 Dest */
#define EPIC_IN_INT16_VEC_REG	(EPIC_CCSROFF + 0x10400)/* Int IRQ16 vect/pri*/
#define EPIC_IN_INT16_DES_REG	(EPIC_CCSROFF + 0x10410)/* Int IRQ16 Dest */
#define EPIC_IN_INT17_VEC_REG	(EPIC_CCSROFF + 0x10420)/* Int IRQ17 vect/pri*/
#define EPIC_IN_INT17_DES_REG	(EPIC_CCSROFF + 0x10430)/* Int IRQ17 Dest */
#define EPIC_IN_INT18_VEC_REG	(EPIC_CCSROFF + 0x10440)/* Int IRQ18 vect/pri*/
#define EPIC_IN_INT18_DES_REG	(EPIC_CCSROFF + 0x10450)/* Int IRQ18 Dest */
#define EPIC_IN_INT19_VEC_REG	(EPIC_CCSROFF + 0x10460)/* Int IRQ19 vect/pri*/
#define EPIC_IN_INT19_DES_REG	(EPIC_CCSROFF + 0x10470)/* Int IRQ19 Dest */
#define EPIC_IN_INT20_VEC_REG	(EPIC_CCSROFF + 0x10480)/* Int IRQ20 vect/pri*/
#define EPIC_IN_INT20_DES_REG	(EPIC_CCSROFF + 0x10490)/* Int IRQ20 Dest */
#define EPIC_IN_INT21_VEC_REG	(EPIC_CCSROFF + 0x104a0)/* Int IRQ21 vect/pri*/
#define EPIC_IN_INT21_DES_REG	(EPIC_CCSROFF + 0x104b0)/* Int IRQ21 Dest */
#define EPIC_IN_INT22_VEC_REG	(EPIC_CCSROFF + 0x104c0)/* Int IRQ22 vect/pri*/
#define EPIC_IN_INT22_DES_REG	(EPIC_CCSROFF + 0x104d0)/* Int IRQ22 Dest */
#define EPIC_IN_INT23_VEC_REG	(EPIC_CCSROFF + 0x104e0)/* Int IRQ23 vect/pri*/
#define EPIC_IN_INT23_DES_REG	(EPIC_CCSROFF + 0x104f0)/* Int IRQ23 Dest */
#define EPIC_IN_INT24_VEC_REG	(EPIC_CCSROFF + 0x10500)/* Int IRQ24 vect/pri*/
#define EPIC_IN_INT24_DES_REG	(EPIC_CCSROFF + 0x10510)/* Int IRQ24 Dest */
#define EPIC_IN_INT25_VEC_REG	(EPIC_CCSROFF + 0x10520)/* Int IRQ25 vect/pri*/
#define EPIC_IN_INT25_DES_REG	(EPIC_CCSROFF + 0x10530)/* Int IRQ25 Dest */
#define EPIC_IN_INT26_VEC_REG	(EPIC_CCSROFF + 0x10540)/* Int IRQ26 vect/pri*/
#define EPIC_IN_INT26_DES_REG	(EPIC_CCSROFF + 0x10550)/* Int IRQ26 Dest */
#define EPIC_IN_INT27_VEC_REG	(EPIC_CCSROFF + 0x10560)/* Int IRQ27 vect/pri*/
#define EPIC_IN_INT27_DES_REG	(EPIC_CCSROFF + 0x10570)/* Int IRQ27 Dest */
#define EPIC_IN_INT28_VEC_REG	(EPIC_CCSROFF + 0x10580)/* Int IRQ28 vect/pri*/
#define EPIC_IN_INT28_DES_REG	(EPIC_CCSROFF + 0x10590)/* Int IRQ28 Dest */
#define EPIC_IN_INT29_VEC_REG	(EPIC_CCSROFF + 0x105a0)/* Int IRQ29 vect/pri*/
#define EPIC_IN_INT29_DES_REG	(EPIC_CCSROFF + 0x105b0)/* Int IRQ29 Dest */
#define EPIC_IN_INT30_VEC_REG	(EPIC_CCSROFF + 0x105c0)/* Int IRQ30 vect/pri*/
#define EPIC_IN_INT30_DES_REG	(EPIC_CCSROFF + 0x105d0)/* Int IRQ30 Dest */
#define EPIC_IN_INT31_VEC_REG	(EPIC_CCSROFF + 0x105e0)/* Int IRQ31 vect/pri*/
#define EPIC_IN_INT31_DES_REG	(EPIC_CCSROFF + 0x105f0)/* Int IRQ31 Dest */
#define EPIC_IN_INT32_VEC_REG	(EPIC_CCSROFF + 0x10600)/* Int IRQ32 vect/pri*/
#define EPIC_IN_INT32_DES_REG	(EPIC_CCSROFF + 0x10610)/* Int IRQ32 Dest */
#define EPIC_IN_INT33_VEC_REG	(EPIC_CCSROFF + 0x10620)/* Int IRQ33 vect/pri*/
#define EPIC_IN_INT33_DES_REG	(EPIC_CCSROFF + 0x10630)/* Int IRQ33 Dest */
#define EPIC_IN_INT34_VEC_REG	(EPIC_CCSROFF + 0x10640)/* Int IRQ34 vect/pri*/
#define EPIC_IN_INT34_DES_REG	(EPIC_CCSROFF + 0x10650)/* Int IRQ34 Dest */
#define EPIC_IN_INT35_VEC_REG	(EPIC_CCSROFF + 0x10660)/* Int IRQ35 vect/pri*/
#define EPIC_IN_INT35_DES_REG	(EPIC_CCSROFF + 0x10670)/* Int IRQ35 Dest */
#define EPIC_IN_INT36_VEC_REG	(EPIC_CCSROFF + 0x10680)/* Int IRQ36 vect/pri*/
#define EPIC_IN_INT36_DES_REG	(EPIC_CCSROFF + 0x10690)/* Int IRQ36 Dest */
#define EPIC_IN_INT37_VEC_REG	(EPIC_CCSROFF + 0x106a0)/* Int IRQ37 vect/pri*/
#define EPIC_IN_INT37_DES_REG	(EPIC_CCSROFF + 0x106b0)/* Int IRQ37 Dest */
#define EPIC_IN_INT38_VEC_REG	(EPIC_CCSROFF + 0x106c0)/* Int IRQ38 vect/pri*/
#define EPIC_IN_INT38_DES_REG	(EPIC_CCSROFF + 0x106d0)/* Int IRQ38 Dest */
#define EPIC_IN_INT39_VEC_REG	(EPIC_CCSROFF + 0x106e0)/* Int IRQ39 vect/pri*/
#define EPIC_IN_INT39_DES_REG	(EPIC_CCSROFF + 0x106f0)/* Int IRQ39 Dest */
#define EPIC_IN_INT40_VEC_REG	(EPIC_CCSROFF + 0x10700)/* Int IRQ40 vect/pri*/
#define EPIC_IN_INT40_DES_REG	(EPIC_CCSROFF + 0x10710)/* Int IRQ40 Dest */
#define EPIC_IN_INT41_VEC_REG	(EPIC_CCSROFF + 0x10720)/* Int IRQ41 vect/pri*/
#define EPIC_IN_INT41_DES_REG	(EPIC_CCSROFF + 0x10730)/* Int IRQ41 Dest */
#define EPIC_IN_INT42_VEC_REG	(EPIC_CCSROFF + 0x10740)/* Int IRQ42 vect/pri*/
#define EPIC_IN_INT42_DES_REG	(EPIC_CCSROFF + 0x10750)/* Int IRQ42 Dest */
#define EPIC_IN_INT43_VEC_REG	(EPIC_CCSROFF + 0x10760)/* Int IRQ43 vect/pri*/
#define EPIC_IN_INT43_DES_REG	(EPIC_CCSROFF + 0x10770)/* Int IRQ43 Dest */
#define EPIC_IN_INT44_VEC_REG	(EPIC_CCSROFF + 0x10780)/* Int IRQ44 vect/pri*/
#define EPIC_IN_INT44_DES_REG	(EPIC_CCSROFF + 0x10790)/* Int IRQ44 Dest */
#define EPIC_IN_INT45_VEC_REG	(EPIC_CCSROFF + 0x107a0)/* Int IRQ45 vect/pri*/
#define EPIC_IN_INT45_DES_REG	(EPIC_CCSROFF + 0x107b0)/* Int IRQ45 Dest */
#define EPIC_IN_INT46_VEC_REG	(EPIC_CCSROFF + 0x107c0)/* Int IRQ46 vect/pri*/
#define EPIC_IN_INT46_DES_REG	(EPIC_CCSROFF + 0x107d0)/* Int IRQ46 Dest */
#define EPIC_IN_INT47_VEC_REG	(EPIC_CCSROFF + 0x107e0)/* Int IRQ47 vect/pri*/
#define EPIC_IN_INT47_DES_REG	(EPIC_CCSROFF + 0x107f0)/* Int IRQ48 Dest */

#define EPIC_MSG_INT0_VEC_REG	(EPIC_CCSROFF + 0x11600)/* MSG INT0 vect/prio*/
#define EPIC_MSG_INT0_DES_REG	(EPIC_CCSROFF + 0x11610)/* MSG INT0 Dest  */
#define EPIC_MSG_INT1_VEC_REG	(EPIC_CCSROFF + 0x11620)/* MSG INT1 vect/prio*/
#define EPIC_MSG_INT1_DES_REG	(EPIC_CCSROFF + 0x11630)/* MSG INT1 Dest  */
#define EPIC_MSG_INT2_VEC_REG	(EPIC_CCSROFF + 0x11640)/* MSG INT2 vect/prio*/
#define EPIC_MSG_INT2_DES_REG	(EPIC_CCSROFF + 0x11650)/* MSG INT2 Dest  */
#define EPIC_MSG_INT3_VEC_REG	(EPIC_CCSROFF + 0x11660)/* MSG INT3 vect/prio*/
#define EPIC_MSG_INT3_DES_REG	(EPIC_CCSROFF + 0x11670)/* MSG INT3 Dest  */

#define EPIC_P0_IPI_DPATCH_REG0	(EPIC_CCSROFF + 0x20040)/* P0 IPI0 dispatch */
#define EPIC_P0_IPI_DPATCH_REG1	(EPIC_CCSROFF + 0x20050)/* P0 IPI1 dispatch */
#define EPIC_P0_IPI_DPATCH_REG2	(EPIC_CCSROFF + 0x20060)/* P0 IPI2 dispatch */
#define EPIC_P0_IPI_DPATCH_REG3	(EPIC_CCSROFF + 0x20070)/* P0 IPI3 dispatch */
#define EPIC_P0_CTASK_PRI_REG	(EPIC_CCSROFF + 0x20080)/* P0 Cur Task Prio */
#define EPIC_P0_WHO_AM_I_REG	(EPIC_CCSROFF + 0x20090)/* P0 Who am I */
#define EPIC_P0_INT_ACK_REG	(EPIC_CCSROFF + 0x200a0)/* P0 Int Ack */
#define EPIC_P0_EOI_REG		(EPIC_CCSROFF + 0x200b0)/* P0 End of Int */


#define EPIC_EX_VEC_REG_INTERVAL	0x20	/* ex vector regs distance */
#define EPIC_IN_VEC_REG_INTERVAL	0x20	/* in vector regs distance */
#define EPIC_GT_VEC_REG_INTERVAL	0x40	/* tm vector regs distance */
#define EPIC_MSG_VEC_REG_INTERVAL	0x20	/* msg vector regs distance */
#define EPIC_IPI_VEC_REG_INTERVAL	0x10	/* ipi vector regs distance */

#define EPIC_EX_DEST_REG_VECREGOFF	0x10	/* EIDR offset from vec reg */
#define EPIC_IN_DEST_REG_VECREGOFF	0x10	/* IIDR offset from vec reg */
#define EPIC_MSG_DEST_REG_VECREGOFF	0x10	/* MIDR offset from vec reg */

#define EPIC_EX_VEC_REG(irq)     (EPIC_EX_INT0_VEC_REG + \
                                 ((irq) * EPIC_EX_VEC_REG_INTERVAL))

#define EPIC_IN_VEC_REG(irq)     (EPIC_IN_INT0_VEC_REG + \
                                 ((irq) * EPIC_IN_VEC_REG_INTERVAL))

#define EPIC_GT_VEC_REG(irq)     (EPIC_TM0_VEC_REG + \
                                 ((irq) * EPIC_GT_VEC_REG_INTERVAL))

#define EPIC_MSG_VEC_REG(irq)    (EPIC_MSG_INT0_VEC_REG + \
                                 ((irq) * EPIC_MSG_VEC_REG_INTERVAL))

#define EPIC_IPI_VEC_REG(irq)    (EPIC_IPI_0_VEC_REG + \
                                 ((irq) * EPIC_IPI_VEC_REG_INTERVAL))

#define EPIC_EX_DEST_REG(irq)    (EPIC_EX_VEC_REG(irq) + \
                                  EPIC_EX_DEST_REG_VECREGOFF)
#define EPIC_IN_DEST_REG(irq)    (EPIC_IN_VEC_REG(irq) + \
                                  EPIC_IN_DEST_REG_VECREGOFF)
#define EPIC_MSG_DEST_REG(irq)   (EPIC_MSG_VEC_REG(irq) + \
                                  EPIC_MSG_DEST_REG_VECREGOFF)

/* GCR register */

#define EPIC_GCR_RESET		EPIC_BIT(31)
#define EPIC_GCR_MODE_MIXED	EPIC_BIT(29)

/* IPI Vector/Priority registers */

#define EPIC_IPIVPR_INTR_MSK        EPIC_BIT(31)
#define EPIC_IPIVPR_INTR_ACTIVE     EPIC_BIT(30)
#define EPIC_IPIVPR_PRIORITY_MSK    (EPIC_BIT(19) | EPIC_BIT(18) | EPIC_BIT(17) | EPIC_BIT(16))
#define EPIC_IPIVPR_PRIORITY(p)     (((p) << 16) & EPIC_IPIVPR_PRIORITY_MSK)
#define EPIC_IPIVPR_VECTOR_MSK      (0xffff)
#define EPIC_IPIVPR_VECTOR(vec)     ((vec) & EPIC_IPIVPR_VECTOR_MSK)

/* Global Timer Vector/Priority registers */

#define EPIC_GTVPR_INTR_MSK        EPIC_BIT(31)
#define EPIC_GTVPR_INTR_ACTIVE     EPIC_BIT(30)
#define EPIC_GTVPR_PRIORITY_MSK    (EPIC_BIT(19) | EPIC_BIT(18) | EPIC_BIT(17) | EPIC_BIT(16))
#define EPIC_GTVPR_PRIORITY(p)     (((p) << 16) & EPIC_GTVPR_PRIORITY_MSK)
#define EPIC_GTVPR_VECTOR_MSK      (0xffff)
#define EPIC_GTVPR_VECTOR(vec)     ((vec) & EPIC_GTVPR_VECTOR_MSK)

/* Summary registers */

#define EPIC_IRQSR0_MSG_INT_MSK 0xf000
#define EPIC_IRQSR0_MSG_INT(n)  (EPIC_BIT(15-(n)) & EPIC_IRQSR0_MSG_INT_MSK)
#define EPIC_IRQSR0_EX_INT_MSK  0xfff
#define EPIC_IRQSR0_EX_INT(n)   (EPIC_BIT(11-(n)) & EPIC_IRQSR0_EX_INT_MSK)
#define EPIC_IRQSR1_IN_INT(n)   EPIC_BIT(31-(n))
#define EPIC_CISR0_MSG_INT_MSK  0xf000
#define EPIC_CISR0_MSG_INT(n)   (EPIC_BIT(15-(n)) & EPIC_CISR0_MSG_INT_MSK)
#define EPIC_CISR0_EX_INT_MSK   0xfff
#define EPIC_CISR0_EX_INT(n)    (EPIC_BIT(11-(n)) & EPIC_CISR0_EX_INT_MSK)
#define EPIC_CISR1_IN_INT(n)    EPIC_BIT(31-(n))

/* Message registers */

#define EPIC_MER_EN_MSK         0xf
#define EPIC_MER_EN(n)          (EPIC_BIT(n) & EPIC_MER_EN_MSK)
#define EPIC_MSR_ST_MSK         0xf
#define EPIC_MSR_ST(n)          (EPIC_BIT(n) & EPIC_MER_ST_MSK)

/* EIVPR registers */

#define EPIC_EIVPR_INTR_MSK         EPIC_BIT(31)
#define EPIC_EIVPR_INTR_ACTIVE      EPIC_BIT(30)
#define EPIC_EIVPR_INTR_POLARITY    EPIC_BIT(23)
#define EPIC_EIVPR_INTR_SENSE       EPIC_BIT(22)
#define EPIC_EIVPR_POLARITY(p)      ((p) << 23)
#define EPIC_EIVPR_SENS(s)          ((s) << 22)
#define EPIC_EIVPR_PRIORITY_MSK	    (EPIC_BIT(19) | EPIC_BIT(18) | EPIC_BIT(17) | EPIC_BIT(16))
#define EPIC_EIVPR_PRIORITY(p) 	    (((p) << 16) & EPIC_EIVPR_PRIORITY_MSK)
#define EPIC_EIVPR_VECTOR_MSK       (0xffff)
#define EPIC_EIVPR_VECTOR(vec) 	    ((vec) & EPIC_EIVPR_VECTOR_MSK)
#define EPIC_INT_ACT_LOW            0
#define EPIC_INT_ACT_HIGH           1
#define EPIC_INT_EDG_NEG            0
#define EPIC_INT_EDG_POS            1
#define EPIC_SENSE_LVL              1
#define EPIC_SENSE_EDG              0

/* EIDR registers */

#define EPIC_EIDR_EX_PIN        EPIC_BIT(31)
#define EPIC_EIDR_CRIT_INT      EPIC_BIT(30)

/* Options for *VPR and *IDR registers */

#define EPIC_OPT_EN_MSK             EPIC_EIVPR_INTR_MSK
#define EPIC_OPT_EN_Y               0x00000000
#define EPIC_OPT_EN_N               0x10000000
#define EPIC_OPT_POLAR_MSK          EPIC_EIVPR_INTR_POLARITY
#define EPIC_OPT_POLAR_ACT_LOW      0x00000000
#define EPIC_OPT_POLAR_ACT_HIGH     0x00800000
#define EPIC_OPT_POLAR_EDG_NEG      0x00000000
#define EPIC_OPT_POLAR_EDG_POS      0x00800000
#define EPIC_OPT_SENSE_MSK          EPIC_EIVPR_INTR_SENSE
#define EPIC_OPT_SENSE_EDG          0x00000000
#define EPIC_OPT_SENSE_LVL          0x00400000
#define EPIC_OPT_PRI_MSK            EPIC_EIVPR_PRIORITY_MSK
#define EPIC_OPT_PRI_VALUE(p)       EPIC_EIVPR_PRIORITY(p)
#define EPIC_OPT_EXPIN_MSK          (EPIC_EIDR_EX_PIN >> 16)
#define EPIC_OPT_EXPIN_OFF          (0x00000000 >> 16)
#define EPIC_OPT_EXPIN_ON           (0x80000000 >> 16)
#define EPIC_OPT_CRIT_MSK           (EPIC_EIDR_CRIT_INT >> 16)
#define EPIC_OPT_CRIT_OFF           (0x00000000 >> 16)
#define EPIC_OPT_CRIT_ON            (0x40000000 >> 16)

/* IIVPR registers */

#define EPIC_IIVPR_INTR_MSK         EPIC_BIT(31)
#define EPIC_IIVPR_INTR_ACTIVE      EPIC_BIT(30)
#define EPIC_IIVPR_INTR_POLARITY    EPIC_BIT(23)
#define EPIC_IIVPR_POLARITY(p)      ((p) << 23)
#define EPIC_IIVPR_PRIORITY_MSK     (EPIC_BIT(19) | EPIC_BIT(18) | EPIC_BIT(17) | EPIC_BIT(16))
#define EPIC_IIVPR_PRIORITY(p)      (((p) << 16) & EPIC_IIVPR_PRIORITY_MSK)
#define EPIC_IIVPR_VECTOR_MSK       (0xffff)
#define EPIC_IIVPR_VECTOR(vec)      ((vec) & EPIC_IIVPR_VECTOR_MSK)

/* IIDR registers */

#define EPIC_IIDR_EX_PIN        EPIC_BIT(31)
#define EPIC_IIDR_CRIT_INT      EPIC_BIT(30)

/* MIVPR registers */

#define EPIC_MIVPR_INTR_MSK         EPIC_BIT(31)
#define EPIC_MIVPR_INTR_ACTIVE      EPIC_BIT(30)
#define EPIC_MIVPR_PRIORITY_MSK     (EPIC_BIT(19) | EPIC_BIT(18) | EPIC_BIT(17) | EPIC_BIT(16))
#define EPIC_MIVPR_PRIORITY(p)      (((p) << 16) & EPIC_MIVPR_PRIORITY_MSK)
#define EPIC_MIVPR_VECTOR_MSK       (0xffff)
#define EPIC_MIVPR_VECTOR(vec)      ((vec) & EPIC_MIVPR_VECTOR_MSK)

/* MIDR registers */

#define EPIC_MIDR_EX_PIN        EPIC_BIT(31)
#define EPIC_MIDR_CRIT_INT      EPIC_BIT(30)

/* IPIDR registers */

#define EPIC_IPIDR_P0           EPIC_BIT(0)

/* CTPR register */

#define EPIC_CTPR_TASKPRI_MSK   (EPIC_BIT(3) | EPIC_BIT(2) | EPIC_BIT(1) | EPIC_BIT(0))
#define EPIC_CTPR_TASKPRI(p)    ((p) & EPIC_CTPR_TASKPRI_MSK)

/* WHOAMI register */

#define EPIC_WHOAMI_ID_MSK      (EPIC_BIT(4) | EPIC_BIT(3) | EPIC_BIT(2) | EPIC_BIT(1) | EPIC_BIT(0))
#define EPIC_WHOAMI_ID(n)       ((n) & EPIC_WHOAMI_ID_MSK)


#define EPIC_INTER_IN_SERVICE	 2

#define EPIC_IN_INTERRUPT 	20	/* internal type */
#define EPIC_EX_INTERRUPT 	21	/* external type */
#define EPIC_INV_INTER_SOURCE 	22	/* invalid interrupt source */
#define EPIC_GT_INTERRUPT 	23	/* global timer type */
#define EPIC_MSG_INTERRUPT 	24	/* message type */
#define EPIC_IPI_INTERRUPT 	25	/* inter-processor type */
#define EPIC_VEC_HAS_NO_IDR     26	/* vector has no IDR reg */
#define EPIC_VEC_OPTION_NA      27	/* option(s) not avail for this vec */
#define EPIC_VEC_OPTION_INV     28	/* option mask is invalid */

#define EPIC_MAX_EXT_IRQS	12
#define EPIC_MAX_IN_IRQS	48
#define EPIC_MAX_GT_IRQS	4
#define EPIC_MAX_MSG_IRQS	4
#define EPIC_MAX_IPI_IRQS	4

#define EPIC_VEC_EXT_IRQ0       0
#define EPIC_VEC_IN_IRQ0        (EPIC_VEC_EXT_IRQ0 + EPIC_MAX_EXT_IRQS)
#define EPIC_VEC_GT_IRQ0        (EPIC_VEC_IN_IRQ0 + EPIC_MAX_IN_IRQS)
#define EPIC_VEC_MSG_IRQ0       (EPIC_VEC_GT_IRQ0 + EPIC_MAX_GT_IRQS)
#define EPIC_VEC_IPI_IRQ0       (EPIC_VEC_MSG_IRQ0 + EPIC_MAX_MSG_IRQS)
#define EPIC_VEC_CTRL_EXT       (EPIC_VEC_IPI_IRQ0 + EPIC_MAX_IPI_IRQS)
#define EPIC_MAX_ALL_IRQS       EPIC_VEC_CTRL_EXT

#define EPIC_PRIORITY_MIN	0    /* minimum level of priority */
#define EPIC_PRIORITY_MAX	15   /* maximum level of priority */
#define EPIC_PRIORITY_DEFAULT	3
#define EPIC_INV_PRIO_ERROR	((ULONG)(-1))


#define EPIC_L2CACHE_INT_NUM        0
#define EPIC_L2CACHE_INT_VEC        (EPIC_L2CACHE_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_ECM_INT_NUM            1
#define EPIC_ECM_INT_VEC            (EPIC_ECM_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DDR_INT_NUM            2
#define EPIC_DDR_INT_VEC            (EPIC_DDR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_LBC_INT_NUM            3
#define EPIC_LBC_INT_VEC            (EPIC_LBC_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DMA0_INT_NUM           4
#define EPIC_DMA0_INT_VEC           (EPIC_DMA0_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DMA1_INT_NUM           5
#define EPIC_DMA1_INT_VEC           (EPIC_DMA1_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DMA2_INT_NUM           6
#define EPIC_DMA2_INT_VEC           (EPIC_DMA2_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DMA3_INT_NUM           7
#define EPIC_DMA3_INT_VEC           (EPIC_DMA3_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_PCI1_INT_NUM           8
#define EPIC_PCI1_INT_VEC           (EPIC_PCI1_INT_NUM + EPIC_VEC_IN_IRQ0)

#define EPIC_PCI2_INT_NUM           9
#define EPIC_PCI2_INT_VEC           (EPIC_PCI2_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_PCI_EXPRESS_INT_NUM    10
#define EPIC_PCI_EXPRESS_INT_VEC    (EPIC_PCI_EXPRESS_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES11_INT_NUM          11
#define EPIC_RES11_INT_VEC          (EPIC_RES11_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DUART2_INT_NUM         26
#define EPIC_DUART2_INT_VEC         (EPIC_DUART2_INT_NUM + EPIC_VEC_IN_IRQ0)

#define EPIC_TSEC1TX_INT_NUM        13
#define EPIC_TSEC1TX_INT_VEC        (EPIC_TSEC1TX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC1RX_INT_NUM        14
#define EPIC_TSEC1RX_INT_VEC        (EPIC_TSEC1RX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC3TX_INT_NUM        15
#define EPIC_TSEC3TX_INT_VEC        (EPIC_TSEC3TX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC3RX_INT_NUM        16
#define EPIC_TSEC3RX_INT_VEC        (EPIC_TSEC3RX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC3ERR_INT_NUM       17
#define EPIC_TSEC3ERR_INT_VEC       (EPIC_TSEC3ERR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC1ERR_INT_NUM       18
#define EPIC_TSEC1ERR_INT_VEC       (EPIC_TSEC1ERR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC2TX_INT_NUM        19
#define EPIC_TSEC2TX_INT_VEC        (EPIC_TSEC2TX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC2RX_INT_NUM        20
#define EPIC_TSEC2RX_INT_VEC        (EPIC_TSEC2RX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC4TX_INT_NUM        21
#define EPIC_TSEC4TX_INT_VEC        (EPIC_TSEC4TX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC4RX_INT_NUM        22
#define EPIC_TSEC4RX_INT_VEC        (EPIC_TSEC4RX_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC4ERR_INT_NUM       23
#define EPIC_TSEC4ERR_INT_VEC       (EPIC_TSEC4ERR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_TSEC2ERR_INT_NUM       24
#define EPIC_TSEC2ERR_INT_VEC       (EPIC_TSEC2ERR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES25_INT_NUM          25
#define EPIC_RES25_INT_VEC          (EPIC_RES25_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_DUART_INT_NUM          26
#define EPIC_DUART_INT_VEC          (EPIC_DUART_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_I2C_INT_NUM            27
#define EPIC_I2C_INT_VEC            (EPIC_I2C_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_PERFMON_INT_NUM        28
#define EPIC_PERFMON_INT_VEC        (EPIC_PERFMON_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SEC_INT_NUM            29
#define EPIC_SEC_INT_VEC            (EPIC_SEC_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES30_INT_NUM          30
#define EPIC_RES30_INT_VEC          (EPIC_RES30_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES31_INT_NUM          31
#define EPIC_RES31_INT_VEC          (EPIC_RES31_INT_NUM + EPIC_VEC_IN_IRQ0)

#define EPIC_SR_ERR_INT_NUM      32
#define EPIC_SR_ERR_INT_VEC      (EPIC_SR_ERR_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_OUT_DB1_INT_NUM  33
#define EPIC_SR_OUT_DB1_INT_VEC  (EPIC_SR_OUT_DB1_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_IN_DB1_INT_NUM   34
#define EPIC_SR_IN_DB1_INT_VEC   (EPIC_SR_IN_DB1_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES35_INT_NUM          35
#define EPIC_RES35_INT_VEC          (EPIC_RES35_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES36_INT_NUM          36
#define EPIC_RES36_INT_VEC          (EPIC_RES36_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_OUT_MU1_INT_NUM  37
#define EPIC_SR_OUT_MU1_INT_VEC  (EPIC_SR_OUT_MU1_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_IN_MU1_INT_NUM   38
#define EPIC_SR_IN_MU1_INT_VEC   (EPIC_SR_IN_MU1_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_OUT_MU2_INT_NUM  39
#define EPIC_SR_OUT_MU2_INT_VEC  (EPIC_SR_OUT_MU2_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_SR_IN_MU2_INT_NUM   40
#define EPIC_SR_IN_MU2_INT_VEC   (EPIC_SR_IN_MU2_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES41_INT_NUM          41
#define EPIC_RES41_INT_VEC          (EPIC_RES41_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES42_INT_NUM          42
#define EPIC_RES42_INT_VEC          (EPIC_RES42_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES43_INT_NUM          43
#define EPIC_RES43_INT_VEC          (EPIC_RES43_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES44_INT_NUM          44
#define EPIC_RES44_INT_VEC          (EPIC_RES44_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES45_INT_NUM          45
#define EPIC_RES45_INT_VEC          (EPIC_RES45_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES46_INT_NUM          46
#define EPIC_RES46_INT_VEC          (EPIC_RES46_INT_NUM + EPIC_VEC_IN_IRQ0)
#define EPIC_RES47_INT_NUM          47
#define EPIC_RES47_INT_VEC          (EPIC_RES47_INT_NUM + EPIC_VEC_IN_IRQ0)

IMPORT STATUS	sysEpicIntConnect (VOIDFUNCPTR * vector, VOIDFUNCPTR routine,
                                   int parameter);
IMPORT int	sysEpicIntEnable (int vector);
IMPORT int	sysEpicIntDisable (int vector);
IMPORT void   	sysEpicInit (void);
IMPORT STATUS 	sysEpicIntrInit (void);
IMPORT int	sysEpicVecOptionsSet (int vector, UINT32 mask, UINT32 options);
IMPORT UINT32	sysEpicVecOptionsGet (int vector);
#ifdef INCLUDE_EPIC_CRT_INTR
IMPORT UINT32	epicCisr0Get (void);
IMPORT UINT32	epicCisr1Get (void);
IMPORT int	sysEpicCrtIntSet (int vector);
IMPORT int	sysEpicCrtIntUnset (int vector);
IMPORT int	sysEpicCrtIntGet (int vector);
#endif  /* INCLUDE_EPIC_CRT_INTR */
IMPORT int  	epicCurTaskPrioSet (int prioNum);
IMPORT int    	epicIntAck (void);
IMPORT void   	epicEOI (void);
IMPORT ULONG	epicGetVecRegAdrs (int vector);
IMPORT ULONG	epicGetDestRegAdrs (int vector);
IMPORT STATUS 	epicIntSourceSet (ULONG srcAddr, int polarity,
                                  int sense, int priority, int vector);
IMPORT STATUS 	epicIntSourceGet (ULONG srcAddr, int * pEnableMask,
                                  int * pPolarity, int * pSense,
                                  int * pPriority, int * pVector);
IMPORT UINT32	sysEpicRegRead (ULONG regNum);
IMPORT void	sysEpicRegWrite (ULONG regNum, UINT32 regVal);

#ifdef __cplusplus
}
#endif

#endif	/* __INCsysEpich */

