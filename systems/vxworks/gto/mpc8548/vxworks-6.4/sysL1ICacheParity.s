/* Copyright 1984-2004 Wind River Systems, Inc. */

/* $Id: sysL1ICacheParity.s,v 1.2 2011/07/21 16:14:13 yshtil Exp $
modification history
--------------------
01a,10jun04,dtr  L1 Errata Fix created.
*/
	FUNC_EXPORT(sysIParityHandler)
	FUNC_EXPORT(sysIParityHandlerEnd)
	DATA_IMPORT(instrParityCount)
	_WRS_TEXT_SEG_START

#define DETECT_EXCHDL_ADRS(ivor)  \
        mfspr   p0, IVPR;         \
        mfspr   p1, ivor;         \
        or      p1, p1, p0;       \
        mfspr   p0, MCSRR0;       \
        cmpw    p0, p1;           \
        beq     faultDetected;

/*********************************************************************
 *
 * sysIParityHandler - This routine is call for a machine check. 
 * This routine will invalidate the instruction cache for the address 
 * in MCSRRO. If only instruction parity error then it will return from
 * machine check else it will go to standard machine check handler.
 */ 
FUNC_BEGIN(sysIParityHandler)
        /* Save registers used */
     
        mtspr   SPRG4_W ,p0
        mtspr   SPRG5_W ,p1
        mfcr    p0
        mtspr   SPRG6_W ,p0
            
        /* check for ICPERR */
        mfspr   p1, MCSR
        andis.  p1, p1, 0x4000
        beq     ppcE500Mch_norm

        /* check if mcsrr0 is pointing to 1st instr of exception handler */
        DETECT_EXCHDL_ADRS(IVOR0)
        DETECT_EXCHDL_ADRS(IVOR1)
        DETECT_EXCHDL_ADRS(IVOR2)
        DETECT_EXCHDL_ADRS(IVOR3)
        DETECT_EXCHDL_ADRS(IVOR4)
        DETECT_EXCHDL_ADRS(IVOR5)
        DETECT_EXCHDL_ADRS(IVOR6)
        DETECT_EXCHDL_ADRS(IVOR8)
        DETECT_EXCHDL_ADRS(IVOR10)
        DETECT_EXCHDL_ADRS(IVOR11)
        DETECT_EXCHDL_ADRS(IVOR12)
        DETECT_EXCHDL_ADRS(IVOR13)
        DETECT_EXCHDL_ADRS(IVOR14)
        DETECT_EXCHDL_ADRS(IVOR15)
        DETECT_EXCHDL_ADRS(IVOR32)
        DETECT_EXCHDL_ADRS(IVOR33)
        DETECT_EXCHDL_ADRS(IVOR34)
        DETECT_EXCHDL_ADRS(IVOR35)

        /* p0 here has mcsrr0 value, round to cache line boundary */
        rlwinm  p0, p0, 0, 0, 31 - CACHE_ALIGN_SHIFT 
        /* invalidate instruction cache */
        icbi    r0, p0
        isync
#ifdef INCLUDE_SHOW_ROUTINES
        /* Add 1 to instrParityCount to measure no of parity errors */ 
        lis     p0, HIADJ(instrParityCount)
        addi    p0, p0, LO(instrParityCount)
        lwz     p1, 0(p0)
        addi    p1, p1, 1
        stw     p1, 0(p0)
#endif
        /* return after invalidate */
        mfspr   p0, SPRG6_R
        mtcr    p0
        mfspr   p0, SPRG4_R
        mfspr   p1, SPRG5_R
        isync     
        rfmci /*.long   0x4c00004c*/

ppcE500Mch_norm:
        mfspr   p0, SPRG6_R
        mtcr    p0
        mfspr   p0, SPRG4_R
        mfspr   p1, SPRG5_R
        ba      0x200       /* _EXC_OFF_MACH */

faultDetected:
        /* rebooting, no need to save regs */
        bl      chipErrataCpu29Print
        li      p0, BOOT_NORMAL
        b       sysToMonitor      /* reset */
FUNC_END(sysIParityHandler)
FUNC_LABEL(sysIParityHandlerEnd)
	
/* Branch to above handler copied to _EXC_OFF_END */
FUNC_BEGIN(jumpIParity)
	ba     sysIParityHandler
FUNC_END(jumpIParity)	
