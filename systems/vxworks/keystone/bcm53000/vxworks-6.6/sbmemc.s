#include "aimemc.h"

/* Convenient macro for writing registers (use t0 for base) */
#define ddr_write(offset, value)   \
    li      a0, (value);               \
    sw      a0, offset(t0);


/*  *********************************************************************
    *  BOARD_DRAMINFO
    *  
    *  Return the address of the DRAM information table
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      v0 - DRAM info table, return 0 to use default table
    ********************************************************************* */
	.ent	board_draminfo
board_draminfo:
	/*	
	 * XXX - in the future, v0 could return the address of the
	 * PROM SDRAM table.
	 */

	move	v0,zero		# auto configure
	j	ra
	.end    board_draminfo

/* $Id: sbmemc.s,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * Register usage within this file: 
 *
 *		top	ncdlsearch	test_mem	Xdr_do_init	sb_core_reset
 *	v0:	retval			retval
 *	v1:	corerev	-		-		corerev		-
 *	a0:	coreptr	coreptr		-		coreptr		coreptr
 *	a1:		x		x		x		sdr/ddr flag
 *	a2:				x
 *	a3:				x
 *	t0:	-	-				config
 *	t1:	-	-				mode
 *	t2:	-	wr/strm		off		wr/strm
 *	t3:	-	rd/strd				rd/strd
 *	t4:	-	g/clkd				g/clkd
 *	t5:				x
 *	t6:	retaddr	-				-		-
 *	t7:	-	-				retaddr		-
 *	s0:		pass_count			-		-
 *	s1:		wrsum/clkdsum			-		-
 *	s2:		rdsum/pass_countmax		-		-
 *	s3:		gsum/strmmax			-		-
 *	s4:		wrlim/stdmmax			-		-
 *	s5:		rdlim/clkdmax			-		-
 *	s6:		glim/clkdlim			-		-
 *	s7:		dll				-		-
 *	t8:	-	-				x		tmp
 *	t9:	-	-				x		retaddr
 *	k0:	trace	trace		trace		-		-
 *	k1:	trace	trace		trace		-		-
 *	gp:
 *	sp:
 *	s8:	-	step				-		-
 *	ra:
 */

#define _K1BASE 		(0xa0000000)	/* kernel unmapped uncached */

#define _PHYS_TO_K1(pa)	(_K1BASE | (pa))

#define _KSEG1			0xa0000000
#define _KSEG1ADDR(a)		(((a) & 0x1fffffff) | _KSEG1)

#define SI_ENUM_BASE 0x18000000
#define SB_ENUM_BASE 0x18000000
#define CC_CHIPID 0
#define AI_DDRPHY_BASE 0x1800f000
#define AI_MEMC_SLAVE_BASE      0x18106000
#define AI_MEMC_BASE            0x18006000
#define	AI_RESETCTRL		0x800

/* (required) Row bits: 0=11bits, 1=12bits, ..., 5=16bits */
#define CFG_DDR_ROW_BITS        2   /* 13 bits */

/* (required) Column bits: 1=9bits, 2=10bits, 3=11bits */
#define CFG_DDR_COLUMN_BITS     2   /* 10 bits */

/* (required) Bank bits: 0=2bits, 3=3bits */
#define CFG_DDR_BANK_BITS       0  

/* (required) Memory width: 0=16bits, 1=32bits, 2=64bits */
#define CFG_DDR_MEM_WIDTH       1

/* (required) CAS Latency (NOTE: could be affected by PLL clock) */
#define CFG_DDR_CAS_LATENCY     5

/* (required) t_wr (picoseconds) */
#define CFG_DDR_T_WR            15000


/*  *********************************************************************
    *  BOARD_DRAMINIT
    *  
    *  This routine gets DRAM running and activate memory.
    *  
    *  Input parameters: 
    *      a0 - points to configuration table returned by board_draminfo
    *           or 0 to use an automatic table (currently ignored)
    *      
    *  Return value:
    *      v0 - total memory installed
    *      
    *  Registers used:
    *      can use all registers.
    ********************************************************************* */

       	.ent	sysMemInit
        .set    noreorder
sysMemInit:	

        move    t7,ra

        /* Figure out if we have an SB or AI chip */
        li      s2,_PHYS_TO_K1(SI_ENUM_BASE)         # s2 = SI_ENUM_BASE
        li      s3,_K1BASE                       # s3 = KSEG1
        li      t0,0xf0000000
        lw      t1,CC_CHIPID(s2)
        and     t1,t0,t1
        srl     s7,t1,28                # s7 = ChipType (0 for SB, = 1 for AI)

        /* Check if we booted from flash, compute reloc for text addresses */
        bal     1f
        nop

1:	li      t0,0x1fffffff
        and     t0,t0,ra
        li      t1,0x1fc00000
        blt     t0,t1,2f
        move    s5,zero
        la      t0,1b
        sub     s5,ra,t0                        # s5: Relocation factor

        /* Call appropriate draminit for chip type */
2:    
        la      t2,ai_draminit
3:      add     t2,t2,s5
        jalr    t2
        nop

        /* Size memory if needed (Need to reinit TRACE after sb_draminit) */
        beqz    v0,szmem
        nop

        li      a0,-1                           # -1 means no controller
        bne     v0,a0,4f
        nop
        move    v0,zero

4:      jr      t7
        nop

szmem:
        li      s3,K1BASE                       # s3 = KSEG1
        li      t2,0xaa55beef
        sw      zero,0x3c(s3)
        li      v0,(1 << 20)
        li      v1,(128 << 20)

5:      or      t0,v0,s3
        sw      t2,0x3c(t0)
        lw      t1,0x3c(t0)    # Read back to ensure completion
        lw      t1,0x3c(s3) 
        beq     t1,t2,6f
        nop

        sll     v0,v0,1
        bne     v0,v1,5b
        nop
        /* Didn't find an alias, must be 128MB */

6:      srl	v0,20			/* return in megabytes */
        jr      t7
        nop

        .set    reorder
      	.end	sysMemInit
/*        END(board_draminit)*/


/*  *********************************************************************
    *  AI_DRAMINIT
    *  
    *  AI version of DDR / memory controller initialization
    *
    *  This routine deals with DDR23_PHY and PL341 MEMC.
    *  
    ********************************************************************* */
 	.ent	ai_draminit
	.set	mips32
	.set	noreorder
ai_draminit:

	move	t6,ra

        /* 
         * ddr23_phy_init
         */

        li      t0, _KSEG1ADDR(AI_DDRPHY_BASE)
        
        /* Recalibrate dn/pd PVT compensation */
        ddr_write(DDR23PHY_ZQ_PVT_COMP_CTL, 0x04000000);
        li      t2, 0x10000000;
wait_sample_done:   /* Wait until sample_done == 1 */
        lw      t1, DDR23PHY_ZQ_PVT_COMP_CTL(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_sample_done;
        nop
        
        /* setup PLL */
        ddr_write(DDR23PHY_PLL_CONFIG,      0x8000000c);
        ddr_write(DDR23PHY_PLL_PRE_DIVIDER, 
                0x00000011 + (PLL_NDIV_INT_VAL << 8));
        ddr_write(DDR23PHY_PLL_DIVIDER,     0x02000000);
        ddr_write(DDR23PHY_PLL_CONFIG,      0x80000008);
        
        /* Wait for PLL locked */
        li      t2, 1;
wait_pll_lock:   /* Wait until lock == 1 */
        lw      t1, DDR23PHY_PLL_STATUS(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_pll_lock;
        nop


        /* De-assert PLL reset */
        ddr_write(DDR23PHY_PLL_CONFIG,      0x80000000);
        ddr_write(DDR23PHY_PLL_CONFIG,      0x00000000);
    
        /* 
         * Calibrate VDL
         */
        
        /* calib_once + calib_fast (for all BL) */
        ddr_write(DDR23PHY_BL3_VDL_CALIBRATE, 0x00000003);
        ddr_write(DDR23PHY_BL2_VDL_CALIBRATE, 0x00000003);
        ddr_write(DDR23PHY_BL1_VDL_CALIBRATE, 0x00000003);
        ddr_write(DDR23PHY_BL0_VDL_CALIBRATE, 0x00000003);
    
        li      t2, 0x0000003;
        li      t0, _KSEG1ADDR(AI_DDRPHY_BASE)
wait_vdl_lock:   /* Wait until calib_idle == 1 and locked for all BL */

        lw      t1, DDR23PHY_BL3_VDL_STATUS(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_vdl_lock;
        nop
        lw      t1, DDR23PHY_BL2_VDL_STATUS(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_vdl_lock;
        nop
        lw      t1, DDR23PHY_BL1_VDL_STATUS(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_vdl_lock;
        nop
        lw      t1, DDR23PHY_BL0_VDL_STATUS(t0)
        and     t1, t1, t2;
        beq     t1, zero, wait_vdl_lock;
        nop
    
        /* VDL override */
        lw      t1, DDR23PHY_BL0_VDL_STATUS(t0)
        srl     t1, t1, 8
        andi    t2, t1, 0x3f                        /* VDL step size */
        li      t1, 1
        sll     t1, 16
        or      t2, t2, t1                          /* ovr_en */
        li      t1, 1
        sll     t1, 20
        or      t2, t2, t1                          /* ovr_force */
        sw      t2, DDR23PHY_STATIC_VDL_OVERRIDE(t0)


        /*
         * Memory controller PL341 initialization
         */ 
        
        /* De-assert core reset */
        li      t0, _KSEG1ADDR(AI_MEMC_SLAVE_BASE);
        ddr_write(AI_RESETCTRL, 0x00000000)
    
        li      t0, _KSEG1ADDR(AI_MEMC_BASE);
        
#ifdef CFG_DDR_REFRESH_PRD
        /* refresh_prd */
        ddr_write(PL341_refresh_prd, MEMCYCLES(CFG_DDR_REFRESH_PRD));
#endif
        
#ifdef CFG_DDR_CAS_LATENCY
        /* cas_latency */
        ddr_write(PL341_cas_latency, (CFG_DDR_CAS_LATENCY << 1));
#endif
    
#ifdef CFG_DDR_WRITE_LATENCY
        /* write_latency */
        ddr_write(PL341_write_latency, CFG_DDR_WRITE_LATENCY);
#endif
    
#ifdef CFG_DDR_T_MRD
        /* MRD time [6:0] */
        ddr_write(PL341_t_mrd, MEMCYCLES(CFG_DDR_T_MRD));
#endif
    
#ifdef CFG_DDR_T_RAS
        /* ras time [4:0] */
        ddr_write(PL341_t_ras, MEMCYCLES(CFG_DDR_T_RAS));
#endif
    
#ifdef CFG_DDR_T_RC
        /* t_rc [4:0] */
        ddr_write(PL341_t_rc, MEMCYCLES(CFG_DDR_T_RC));
#endif
    
#ifdef CFG_DDR_T_RCD
        /* t_rcd [4:0] */
        ddr_write(PL341_t_rcd, MEMCYCLES(CFG_DDR_T_RCD));
#endif
    
#ifdef CFG_DDR_T_RFC
        /* t_rfc [6:0]  schedule_rfc[14:8] */
        ddr_write(PL341_t_rfc,
            ((MEMCYCLES(CFG_DDR_T_RFC) - 3) << 8) | MEMCYCLES(CFG_DDR_T_RFC));
#endif
    
#ifdef CFG_DDR_T_RP
        /* t_rp [3:0] */
        ddr_write(PL341_t_rp, 
            ((MEMCYCLES(CFG_DDR_T_RP) - 3) << 8) | MEMCYCLES(CFG_DDR_T_RP));
#endif
    
#ifdef CFG_DDR_T_RRD
        /* t_rrd [2:0] */
        ddr_write(PL341_t_rrd, MEMCYCLES(CFG_DDR_T_RRD));
#endif
    
#ifdef CFG_DDR_T_WR
        /* t_wr */
        ddr_write(PL341_t_wr, MEMCYCLES(CFG_DDR_T_WR));
#endif
    
#ifdef CFG_DDR_T_WTR
        /* t_wtr */
        ddr_write(PL341_t_wtr, MEMCYCLES(CFG_DDR_T_WTR));
#endif
    
#ifdef CFG_DDR_T_XP
        /* t_xp[7:0] */
        ddr_write(PL341_t_xp, MEMCYCLES(CFG_DDR_T_XP));
#endif
    
#ifdef CFG_DDR_T_XSR
        /* t_xsr[7:0] */
        ddr_write(PL341_t_xsr, MEMCYCLES(CFG_DDR_T_XSR));
#endif
    
#ifdef CFG_DDR_T_ESR
        /* t_esr[7:0] */
        ddr_write(PL341_t_esr, MEMCYCLES(CFG_DDR_T_ESR));
#endif
    
#ifdef CFG_DDR_T_FAW
        /* t_faw */
        ddr_write(PL341_t_faw, 
            ((MEMCYCLES(CFG_DDR_T_FAW) - 3) << 8) | MEMCYCLES(CFG_DDR_T_FAW));
#endif
    
        /* memory_cfg register */
        ddr_write(PL341_memory_cfg, 
            (2 << 15) | (CFG_DDR_ROW_BITS << 3) | CFG_DDR_COLUMN_BITS
            );
    
        /* memory_cfg2 register */
        ddr_write(PL341_memory_cfg2, 
            (CFG_DDR_MEM_WIDTH << 6) | (CFG_DDR_BANK_BITS << 4)
            );
    
        /* chip0 configuration */
        ddr_write(PL341_chip_0_cfg, 0x00000000);

        /* user_config0 */
        ddr_write(PL341_user_config0, 0x00000003);


        /*
         * DDR2 chip initialization
         */

        /* Issue NOP */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_NOP);
    
        /* issue precharge all */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_PRECHARGE_ALL);
    
        /* Set EMR2 */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(2)); 
        
        /* Set EMR3 */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(3)); 
        
        /* DLL Enable */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(1) | 
            MCHIP_EMR1_DLL_DISABLE(0)
            );
    
        /* DLL Reset */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(0) |
            MCHIP_MR_WRITE_RECOVERY(MEMCYCLES(CFG_DDR_T_WR)) |
            MCHIP_MR_DLL_RESET(1) |
            MCHIP_MR_CAS_LATENCY(CFG_DDR_CAS_LATENCY) |
            MCHIP_MR_BURST_LENGTH
            );
    
        /* issue precharge all */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_PRECHARGE_ALL);
    
        /* auto-refresh 2 times */
        ddr_write(PL341_direct_cmd, MCHIP_CMD_AUTO_REFRESH);
        ddr_write(PL341_direct_cmd, MCHIP_CMD_AUTO_REFRESH);
        
        /* DLL Reset=0 (Un-Reset DLL) */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(0) |
            MCHIP_MR_WRITE_RECOVERY(MEMCYCLES(CFG_DDR_T_WR)) |
            MCHIP_MR_DLL_RESET(0) | 
            MCHIP_MR_CAS_LATENCY(CFG_DDR_CAS_LATENCY) | 
            MCHIP_MR_BURST_LENGTH
            );
        
        /* DLL Enable & RTT 75ohm */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(1) | 
            MCHIP_EMR1_DLL_DISABLE(0) | MCHIP_EMR1_RTT_75_OHM |
            MCHIP_EMR1_OCD_CALI_EXIT
            );
    
        /* OCD Defaults */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(1) | 
            MCHIP_EMR1_DLL_DISABLE(0) | MCHIP_EMR1_RTT_75_OHM |
            MCHIP_EMR1_OCD_CALI_DEFAULT
            );
    
        /* OCD Exit */
        ddr_write(PL341_direct_cmd, 
            MCHIP_CMD_MODE_REG | MCHIP_MODEREG_SEL(1) | 
            MCHIP_EMR1_DLL_DISABLE(0) | MCHIP_EMR1_RTT_75_OHM |
            MCHIP_EMR1_OCD_CALI_EXIT
            );
    
        /* set MEMC to GO */
        ddr_write(PL341_memc_cmd, 0);
        nop
        nop
    
        jr      t6
        nop
    
        .set    reorder
      	.end	ai_draminit
