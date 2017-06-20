/*
 * $Id: caladan3.c,v 1.78.8.1.6.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Implementation of Caladan3 SOC Initialization
 *
 * NOTE:
 * SOC driver infrastructure cleanup pending.
 */



#include <shared/bsl.h>

#include <sal/appl/sal.h>
#include <soc/defs.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/cmtypes.h>
#include <soc/linkctrl.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/i2c.h>
#include <soc/ipoll.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/link.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/ci.h>
#include <soc/sbx/caladan3/rce.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/shmoo_ddr40.h>
#include <soc/sbx/caladan3/tmu/wb_db_tmu.h>

/* Enable the macro below for HPP cluster only QT image testing */
/*#define HPP_CLUSTER_ONLY 1*/

#ifndef HPP_CLUSTER_ONLY
#define UCODE_LOAD 1
#define TMU_INIT 1
#define CMU_INIT 1
#define COP_INIT 1
#define ETU_INIT 1
#define CMU_NO_TR 1
#endif

extern int soc_sbx_caladan3_sws_pr_enable(int unit, int blk, int enable);

/* Set this to enable calculating and displaying the time required
 * do do the sws part of fast reconfig.
 */
#define CHECK_RECONFIG_TIME 1

/*
 * Function:
 *      soc_reset_bcm88030_a0
 * Purpose:
 *      Special reset sequencing for BCM88030
 *      Setup SBUS block mapping.
 */
STATIC void
soc_reset_bcm88030_a0(int unit)
{
    uint32 def_reg __attribute__((unused))=0;
    uint16 dev_id;
    uint8  rev_id;
    int    spl=0;
    uint32 rval=0;
    int port;
    uint32 fc_oob_clk_in_mhz=0;
    uint32 mdiv=0;
    uint32 mdiv_remainder=0;

    
    if (SOC_IS_RCPU_ONLY(unit)) {
        /* Skip reset process for rcpu only unit */
        return;
    }

    /*
     * Configure endian mode in case the system just came out of reset.
     */
    soc_endian_config(unit);

    /*
     * Configure bursting in case the system just came out of reset.
     */
    soc_pci_burst_enable(unit);

    /*
     * After setting the reset bit, StrataSwitch PCI registers cannot be
     * accessed for 300 cycles or the CMIC will hang.  This is mostly of
     * concern on the Quickturn simulation but we leave plenty of time.
     */
    soc_cm_get_id(unit, &dev_id, &rev_id);

    /* Suspend IRQ polling while resetting */
    soc_ipoll_pause(unit);

    if (soc_property_get(unit, spn_SOC_SKIP_RESET, 0)) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "soc_init: skipping hard reset\n")));
    } else {
        /* CPS reset problematic on PLI, the amount of time
         * to wait is not constant and may vary from machine
         * to machine. So, don't do it.
         */
        if (!SAL_BOOT_RTLSIM) {
            /* Block interrupts while setting the busy flag */
            spl = sal_splhi();
            SOC_CONTROL(unit)->soc_flags |= SOC_F_BUSY;

            /* Unblock interrupts */
            sal_spl(spl);


            /* CMIC reset */
#ifdef BCM_CMICM_SUPPORT
            if (soc_feature(unit, soc_feature_cmicm)) {
                soc_pci_write(unit, CMIC_CPS_RESET_OFFSET, 1);
            } else
#endif
            {
                soc_pci_write(unit, CMIC_CONFIG,
                              soc_pci_read(unit, CMIC_CONFIG) | CC_RESET_CPS);
            }


            /* delay after CMIC reset */
            if (soc_feature(unit, soc_feature_reset_delay) && !SAL_BOOT_QUICKTURN) {
                sal_usleep(1000000);
            } else {
                if (SAL_BOOT_QUICKTURN) {
                    sal_usleep(10 * MILLISECOND_USEC);
                } else {
                    sal_usleep(1 * MILLISECOND_USEC);
                }
            }
        }

        /* NOTE:
         * Caladan3 doesn't have ARL table, but we wait here anyhow just
         * to be safe. Tune it after everything is stablized.
         */
        if (SAL_BOOT_QUICKTURN) {
            sal_usleep(250 * MILLISECOND_USEC);
        } else {
            sal_usleep(10 * MILLISECOND_USEC);
        }

        /* Restore endian mode since the reset cleared it. */
        soc_endian_config(unit);

	/* Following a CPS Reset, a dummy read will be required to any CMIC
	 * register to flush out the stale data that exists in PCIe core
	 */
	soc_pci_read(unit, CMIC_CONFIG);
	sal_usleep(1000);

        /* Restore bursting */
        soc_pci_burst_enable(unit);

    /* Synchronize cached interrupt mask */
#ifdef BCM_CMICM_SUPPORT
        if (soc_feature(unit, soc_feature_cmicm)) {
            soc_cmicm_intr0_disable(unit, ~0);
            soc_cmicm_intr1_disable(unit, ~0);
        } else
#endif
        {
            soc_intr_disable(unit, ~0);
        }

        /* Block interrupts */
        spl = sal_splhi();
        SOC_CONTROL(unit)->soc_flags &= ~SOC_F_BUSY;
        /* Unblock interrupts */
        sal_spl(spl);

        /* Resume IRQ polling if active */
        soc_ipoll_continue(unit);
    }

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
       def_reg = CMIC_CMCx_SCHAN_MESSAGEn(SOC_PCI_CMC(unit), 0);
    }
#endif

    /* Use 156.25Mhz reference clock for LCPLL */
    

    /*
     * SBUS Block SBUS ID:
     , IL0(8), IL1(9)
     * LR(10), OC(11) 
     * SBUS ring number:
     * ring 0: XL. CL0, XT2, XT1, IL0, CX
     * ring 1: PR0, PT0, QM, PT1, PR1, STGF(top), PB
     * ring 2: PP, LR, STGF(LR), CO0, CO1, OC, STFG(OC), PD, STGF(top)
     * ring 3: STGF(top), ETW, ET
     * ring 4: TMU
     * ring 5: IL1, CL1, CI(0-19), CI, STGF(TM1,0)
     * ring 6: CM
     * ring 7: OT
     */
#ifdef BCM_CMICM_SUPPORT

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "soc_reset_bcm88030_a0 cmicm=%d, bootflag=0x%x SAL_BOOT_PLISIM=0x%x, BOOT_F_BCMSIM=0x%x\n"), (soc_feature(unit, soc_feature_cmicm) != 0), 
                 sal_boot_flags_get(), SAL_BOOT_PLISIM, BOOT_F_BCMSIM));

    if (soc_feature(unit, soc_feature_cmicm) &&
        !(SAL_BOOT_BCMSIM || SAL_BOOT_PLISIM)) {

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_reset_bcm88030_a0 configure SBUS_RING\n")));

        /* CO0(1/2), CO1(2/2), CL0(3/0), CL1(4/5), CM (5/7), CMICM(6), ET (7/3) */
        WRITE_CMIC_SBUS_RING_MAP_0_7r(  unit, 0x55555550);   /* block 7  - 0  */
        WRITE_CMIC_SBUS_RING_MAP_8_15r( unit, 0x55555555);   /* block 15 - 8  */
        WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x30226505);   /* block 23 - 16 */
        WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x17222503);   /* block 31 - 24 */
        WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x41111122);   /* block 39 - 32 */
        WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x44444444);   /* block 47 - 40 */
        WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x24444444);   /* block 55 - 48 */
        WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x00004444);   /* block 63 - 56 */

        WRITE_CMIC_SBUS_TIMEOUTr(unit, 0x7d0);
    }
#endif
    rval = 0;
    WRITE_CX_CONFIGr(unit, rval);
    soc_reg_field_set(unit, CX_CONFIGr, &rval, SOFT_RESET_Nf, 1);
    WRITE_CX_CONFIGr(unit, rval);

    /*
     * Enable plls
     */
    rval = 0;
    WRITE_CX_MAC_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_MAC_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_MAC_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_MAC_PLL_RESETr(unit, rval);

    rval = 0;
    WRITE_CX_WC_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_WC_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_WC_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_WC_PLL_RESETr(unit, rval);

    rval = 0;
    WRITE_CX_BS_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_BS_PLL_RESETr(unit, rval);

    rval = 0;
    WRITE_CX_TS_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_TS_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_TS_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_TS_PLL_RESETr(unit, rval);

    rval = 0;
    WRITE_CX_SWS_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_SWS_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SWS_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_SWS_PLL_RESETr(unit, rval);
    rval = 0;
    WRITE_CX_TMU_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_TMU_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_TMU_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_TMU_PLL_RESETr(unit, rval);
    rval = 0;
    WRITE_CX_HPP_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_HPP_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_HPP_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_HPP_PLL_RESETr(unit, rval);
    rval = 0;
    WRITE_CX_DDR03_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_DDR03_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_DDR03_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_DDR03_PLL_RESETr(unit, rval);
    rval = 0;
    WRITE_CX_SE0_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_SE0_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SE0_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_SE0_PLL_RESETr(unit, rval);
    rval = 0;
    WRITE_CX_SE1_PLL_RESETr(unit, rval);
    soc_reg_field_set(unit, CX_SE1_PLL_RESETr, &rval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SE1_PLL_RESETr, &rval, POST_RESET_Nf, 1);
    WRITE_CX_SE1_PLL_RESETr(unit, rval);

    /*
     * Blocks out of reset
     */
    WRITE_CX_SOFT_RESET_0r(unit, 0);
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PP_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CO0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CO1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, RC_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, OC_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PD_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CL0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CL0_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CL1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CL1_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT0_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT1_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT2_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XT2_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XL_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, XL_PORT_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, IL0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, IL1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, QM_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PT0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PT1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PR0_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PR1_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, PB_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CI_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, CM_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, ET_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    rval = 0;
    WRITE_CX_SOFT_RESET_1r(unit, rval);
    READ_CX_SOFT_RESET_1r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_1r, &rval, TMA_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_1r, &rval, TMB_RESET_Nf, 1);
    soc_reg_field_set(unit, CX_SOFT_RESET_1r, &rval, TIMESTAMP_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_1r(unit, rval);

    rval = 0;
    WRITE_OC_CONFIGr(unit, rval);
    soc_reg_field_set( unit, OC_CONFIGr, &rval, SOFT_RESET_Nf, 1 ); 
    WRITE_OC_CONFIGr(unit, rval);

    rval = 0;
    READ_LRB_SVP_SEM_CONFIGr( unit, 0, &rval );
    soc_reg_field_set( unit, LRB_SVP_SEM_CONFIGr, &rval, INIT_VALUEf, 128 );
    WRITE_LRB_SVP_SEM_CONFIGr( unit, 0, rval );

    rval = 0;
    WRITE_LRA_CONFIG0r(unit, rval);
    soc_reg_field_set( unit, LRA_CONFIG0r, &rval, SOFT_RESET_Nf, 1 ); 
    WRITE_LRA_CONFIG0r(unit, rval);

    rval = 0;
    WRITE_LRB_CONFIG0r(unit, rval);
    soc_reg_field_set( unit, LRB_CONFIG0r, &rval, SOFT_RESET_Nf, 1 ); 
    WRITE_LRB_CONFIG0r(unit, rval);

    /* 
     * These are done here to facilitate TR tests to complete. 
     * The MACs must be initialized only after the SWS is ready 
     * The "spn_LRP_BYPASS" condition is necessary for c3hppc emulation which does not contain the MACs
     */

    if (soc_property_get(unit, spn_LRP_BYPASS, 0) < 3) {

      PBMP_CL_ITER(unit, port)  {
          rval = 0xf;/*XXXTTT*/
          WRITE_PORT_MAC_CONTROLr(unit, port, rval);
          soc_sbx_caladan3_xgxs_reset(unit, port, 0);
          soc_sbx_caladan3_xgxs_reset(unit, port, 1);
          soc_sbx_caladan3_xgxs_reset(unit, port, 2);          
      }

      PBMP_XL_ITER(unit, port)  {
          rval = 0xf;
          WRITE_PORT_MAC_CONTROLr(unit, port, rval);
          soc_sbx_caladan3_xgxs_reset(unit, port, 0);
          soc_sbx_caladan3_xgxs_reset(unit, port, 1);
          soc_sbx_caladan3_xgxs_reset(unit, port, 2);
      }
      PBMP_XT_ITER(unit, port)  {
          rval = 0xf;
          WRITE_PORT_MAC_CONTROLr(unit, port, rval);
          rval = 0xf;
          WRITE_PORT_MAC_CONTROLr(unit, port, rval);
      }

    }

    if (soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        return;
    }

    /* Running TMU 660MHz operation  -- (25 * 132(ndiv)) / 5(mdiv) */
    READ_CX_TMU_PLL_NDIV_INTEGERr(unit, &rval);
    if (dev_id == BCM88039_DEVICE_ID) {
	/* need to run TMU at higher speed (660Mhz) for 88039 */
	soc_reg_field_set(unit, CX_TMU_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 132);
    } else if (dev_id == BCM88034_DEVICE_ID) {
	/* need to run TMU at higher speed (495Mhz) for 88034 */
	soc_reg_field_set(unit, CX_TMU_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 99);
    } else {
	/* need to run TMU at higher speed (600Mhz) for 88038 */
	soc_reg_field_set(unit, CX_TMU_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 120);
    }
    WRITE_CX_TMU_PLL_NDIV_INTEGERr(unit, rval);	

    /* Run SWS at 641M, might want to bump up higher later pending HW investigation */
    READ_CX_SWS_PLL_NDIV_INTEGERr(unit, &rval);
    soc_reg_field_set(unit, CX_SWS_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 0x9A);
    WRITE_CX_SWS_PLL_NDIV_INTEGERr(unit, rval);   

    /* Run LRP at non-default speed, only support 1.1GHZ and default 1GHZ for now
     * LRP 1100MHz operation  -- (25 * 132(ndiv)) / 3(mdiv) 
     */
    READ_CX_HPP_PLL_NDIV_INTEGERr(unit, &rval);
    switch (SOC_SBX_CFG(unit)->uClockSpeedInMHz) {
	case 1100:
	    soc_reg_field_set(unit, CX_HPP_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 132);
	    break;
	case 900:
	    soc_reg_field_set(unit, CX_HPP_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 108);
	    break;
	case 742:
	    soc_reg_field_set(unit, CX_HPP_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 89);
	    break;
	default:
	    soc_reg_field_set(unit, CX_HPP_PLL_NDIV_INTEGERr, &rval, NDIV_INTf, 120);
	    break;	    
    }
    WRITE_CX_HPP_PLL_NDIV_INTEGERr(unit, rval);

    /* Synchronous ethernet default PLL setting is wrong 0x80 instead of 0x50, 
     * change it here. We need to toggle load_en bit after MDIV value change.
     */
    READ_CX_SE0_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE0_PLL_CHANNEL_0r, &rval, MDIVf, 0x50);
    WRITE_CX_SE0_PLL_CHANNEL_0r(unit, rval);   
    
    READ_CX_SE1_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE1_PLL_CHANNEL_0r, &rval, MDIVf, 0x50);
    WRITE_CX_SE1_PLL_CHANNEL_0r(unit, rval);   
    
    READ_CX_SE0_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE0_PLL_CHANNEL_0r, &rval, LOAD_ENf, 1);
    WRITE_CX_SE0_PLL_CHANNEL_0r(unit, rval);   
    
    READ_CX_SE0_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE0_PLL_CHANNEL_0r, &rval, LOAD_ENf, 0);
    WRITE_CX_SE0_PLL_CHANNEL_0r(unit, rval);   

    READ_CX_SE1_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE1_PLL_CHANNEL_0r, &rval, LOAD_ENf, 1);
    WRITE_CX_SE1_PLL_CHANNEL_0r(unit, rval);   
    
    READ_CX_SE1_PLL_CHANNEL_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SE1_PLL_CHANNEL_0r, &rval, LOAD_ENf, 0);
    WRITE_CX_SE1_PLL_CHANNEL_0r(unit, rval);   

    fc_oob_clk_in_mhz = soc_property_get(unit, "fc_oob_clk_in_mhz", 125);

    /* 
     * Equation to calculate fc_clk (divide this value by 2 to get ilkn oob clk)
     *
     * Fvco = 1/pdiv * (ndiv_int + ndiv_frac/2^20) * Fref
     *
     * pdiv=1 ndiv_frac=0 ndiv_int=120 ndiv_frac=0 fref=25MHz
     * Fvco = 1/1 * (120 + 0) * 25MHz = 3000MHz
     *
     * F clkout = Fvco/mdiv
     *
     * mdiv = Fvco/F clkout
     *
     * mdiv = 3000MHz/fc_oob_clk_in_mhz
     */
    mdiv_remainder = 3000%(fc_oob_clk_in_mhz);
    if (mdiv_remainder == 0) {
        mdiv = 3000/(fc_oob_clk_in_mhz);
        READ_CX_TS_PLL_CHANNEL_2r(unit, &rval);
        soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_2r, &rval, MDIVf, mdiv);
        WRITE_CX_TS_PLL_CHANNEL_2r(unit, rval);   
        
        READ_CX_PLL_CTRLr(unit, &rval);
        soc_reg_field_set(unit, CX_PLL_CTRLr, &rval, TS_BOND_OVERRIDEf, 1);
        WRITE_CX_PLL_CTRLr(unit, rval);   
        
        READ_CX_TS_PLL_CHANNEL_2r(unit, &rval);
        soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_2r, &rval, LOAD_ENf, 1);
        WRITE_CX_TS_PLL_CHANNEL_2r(unit, rval);   
        
        READ_CX_TS_PLL_CHANNEL_2r(unit, &rval);
        soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_2r, &rval, LOAD_ENf, 0);
        WRITE_CX_TS_PLL_CHANNEL_2r(unit, rval);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: property fc_oob_clk_in_mhz(%d) value out of range, leaving default value\n"), fc_oob_clk_in_mhz));
    }
}
/*
 * Function:
 *     _soc_sbx_caladan3_cmic_init
 * Purpose:
 *     Caladan3 intializaiton code
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
STATIC int
_soc_sbx_caladan3_cmic_init(int unit, int reset)
{
    soc_control_t       *soc;
    soc_persist_t       *sop;
    uint16              dev_id;
    uint8               rev_id;
    int rv = SOC_E_NONE;
    uint32 divisor = 0, regval = 0;
#ifdef BCM_CMICM_SUPPORT
    int i;
    int numq = 0;
#endif

    if (!SOC_UNIT_VALID(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d not valid\n"), unit));
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d not attached\n"), unit));
        return(SOC_E_UNIT);
    }

    /***********************************************************************/
    /* If the device has already been initialized before, perform some     */
    /* de-initialization to avoid stomping on existing activities.         */
    /***********************************************************************/
    if (SOC_WARM_BOOT(unit)) {
        (void)soc_dma_abort(unit);
    }

    if (soc->soc_flags & SOC_F_INITED) {
        (void)soc_dma_abort(unit);             /* Turns off/clean up DMA */
#if 0 /*enable after counter attach */
        (void)soc_counter_stop(unit);          /* Stop counter collection */
#endif
#ifdef INCLUDE_MEM_SCAN
        (void)soc_mem_scan_stop(unit);         /* Stop memory scanner */
#endif
#ifdef  INCLUDE_I2C
        (void)soc_i2c_detach(unit);            /* Free up I2C driver mem */
#endif
        soc->soc_flags &= ~SOC_F_INITED;
    }

    soc_cm_get_id(unit, &dev_id, &rev_id);
    /* soc_sbx_info_config(unit,dev_id); moved to attach time*/

    soc_dcb_unit_init(unit);

    /***********************************************************************/
    /* Always be sure device has correct endian configuration before       */
    /* touching registers - device may not have been configured yet.       */
    /***********************************************************************/
    if (!SOC_WARM_BOOT(unit)) {
        soc_endian_config(unit);
    }

    /***********************************************************************/
    /* Always enable bursting before doing any more reads or writes        */
    /***********************************************************************/
    if (!SOC_WARM_BOOT(unit)) {
        soc_pci_burst_enable(unit);
    }

    /***********************************************************************/
    /* Begin initialization from a known state (reset).                    */
    /***********************************************************************/
    /* Attach DMA */
    if ((rv = soc_dma_attach(unit, reset)) < 0) {
        return SOC_E_INTERNAL;
    }

    if ((dev_id == BCM88030_DEVICE_ID) &&
        (SOC_SBX_CFG(unit)->uClockSpeedInMHz > 1000)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "on unit %d, bcm88038 device can only support upto 1000MHz LRP clock\n"),
                   unit));
        return SOC_E_PARAM;
    }

    if ((dev_id == BCM88034_DEVICE_ID) &&
        (SOC_SBX_CFG(unit)->uClockSpeedInMHz > 742)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "on unit %d, bcm88034 device can only support upto 742MHz LRP clock\n"),
                   unit));
        return SOC_E_PARAM;
    }

    if ((SOC_SBX_CFG(unit)->uClockSpeedInMHz > 1100) ||
        (SOC_SBX_CFG(unit)->uClockSpeedInMHz < 742)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "on unit %d, LRP clock %d out of range [742, 1100]\n"),
                   unit, SOC_SBX_CFG(unit)->uClockSpeedInMHz));
        return SOC_E_PARAM;
    }

    /*
     * Update saved chip state to reflect values after reset.
     */
    soc->soc_flags &= (SOC_F_RESET | SOC_F_RCPU_ONLY);
    sop->debugMode = 0;
    soc->pciParityDPC = 0;
    soc->pciFatalDPC = 0;

    
    if (reset && !SOC_WARM_BOOT(unit)) {
        soc_reset_bcm88030_a0(unit);
    }

    /***********************************************************************/
    /* Configure CMIC PCI registers correctly for driver operation.        */
    /*                                                                     */
    /* NOTE:  When interrupt driven, the internal SOC registers cannot     */
    /*        be accessed until the CMIC interrupts are enabled.           */
    /***********************************************************************/
    if (!SAL_BOOT_PLISIM && !SOC_WARM_BOOT(unit)) {
        /*
         * Check that PCI memory space is mapped correctly by running a
         * quick diagnostic on the S-Channel message buffer.
         */

        SOC_IF_ERROR_RETURN(soc_pci_test(unit));
    }

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
        
        /* Enable PCI Bus Error and Parity Error Interrupts */
        /* soc_intr_enable(unit, IRQ_PCI_PARITY_ERR | IRQ_PCI_FATAL_ERR); */
        /* S-Channel Error Interrupt */
        /* soc_intr_enable(unit, IRQ_SCHAN_ERR); */
        /* Link status updates */
        soc_cmicm_intr1_enable(unit, IRQ_CMCx_LINK_STAT_MOD);

        for (i = 0, numq = 0; i < SOC_CMCS_NUM(unit); i++) {
            if (i == SOC_PCI_CMC(unit)) {
                NUM_CPU_ARM_COSQ(unit, i) =
                    soc_property_uc_get(unit, 0, spn_NUM_QUEUES,
                                        NUM_CPU_COSQ(unit));
            } else {
                NUM_CPU_ARM_COSQ(unit, i) =
                    soc_property_uc_get(unit, i, spn_NUM_QUEUES, 0);
            }
            /*LOG_CLI((BSL_META_U(unit,
                                  "\nCpu %d NumQ %d"), i, NUM_CPU_ARM_COSQ(unit, i)));*/
            numq += NUM_CPU_ARM_COSQ(unit, i);
        }

        if (numq > NUM_CPU_COSQ(unit)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "_soc_sbx_caladan3_cmic_init: total cpu and arm cosq %04x unexpected\n"),
                      numq));
        }
    } 
#endif /* CMICM Support */

    if (!SOC_WARM_BOOT(unit)) {
        /* no need to config CMIC in warm boot - already running */

        WRITE_CMIC_CMC0_CH1_COS_CTRL_RX_0r(unit, 0xffffffff);
        WRITE_CMIC_CMC0_CH1_COS_CTRL_RX_1r(unit, 0xffffffff);
        
        /* Configure Internal MDIO clock rate 12.5 Mhz */
        READ_CMIC_RATE_ADJUST_INT_MDIOr(unit, &regval);
        divisor = SOC_SBX_CALADAN3_CORE_CLOCK / 25;
        soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &regval, DIVISORf, divisor);
        WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, regval);
        
        /* CMIC External MDIO clock rate to about 2.5Mhz */
        READ_CMIC_RATE_ADJUSTr(unit, &regval);
        divisor = SOC_SBX_CALADAN3_CORE_CLOCK / 5;
        soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &regval, DIVISORf, divisor);
        WRITE_CMIC_RATE_ADJUSTr(unit, regval);
    }


    soc->soc_flags |= SOC_F_INITED;

    if (SOC_IS_RCPU_ONLY(unit)) {
        return SOC_E_NONE;
    }

    return rv;
}


/*
 * Function:
 *     soc_sbx_caladan3_detach
 * Purpose:
 *     Cleanup and free all resources allocated during device specific
 *     initialization routine.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
soc_sbx_caladan3_detach(int unit)
{
    int rv = SOC_E_NONE;
    soc_control_t       *soc;

    soc = SOC_CONTROL(unit);

    if (soc->schan_wb_mutex) {
        sal_mutex_destroy(soc->schan_wb_mutex);
        soc->schan_wb_mutex = NULL;
    }
    return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_isr
 * Purpose:
 *   C3 ISR
 * Parameters:
 *     unit - Device number
 * Returns: None
 */
void
soc_sbx_caladan3_isr(void *_unit)
{
}


/*
 * Function:
 *     soc_sbx_caladan3_port_info_config
 * Purpose:
 *     Configures mapping of microcode port to c3 physical ports.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Microcode port mapping loaded successfully
 * Notes:
 *     If user configuration is invalid not specified, the default
 *     microcode port mapping is loaded.
 */
int
soc_sbx_caladan3_port_info_config(int unit)
{
    if (!SOC_HOTSWAP_TDM) {
        return soc_sbx_caladan3_port_info_load(unit);
    } else {
        return soc_sbx_caladan3_port_info_update(unit);
    }
}


/*
 * Function:
 *     soc_sbx_caladan3_ucode_init
 * Purpose:
 *     Initialize ucode environment
 */
int
soc_sbx_caladan3_ucode_init(int unit)
{
    int rv = SOC_E_NONE;
    
    rv = soc_sbx_caladan3_lr_ucodemgr_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_init: unit %d ucode manager init failed %d \n"), unit, rv));
        return(SOC_E_INIT);
    }
    /*
     * At this point decide how the image should be loaded
     * currently load default method
     */
    rv = soc_sbx_caladan3_ucodemgr_loadimg(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_init: unit %d ucode image download failed %d \n"), unit, rv));
        return(SOC_E_INIT);
    }
    return rv;
}

/*
 * Function:
 *     _soc_sbx_caladan3_ddr_init
 * Purpose:
 *     Caladan3 DDR Interfaces init
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 *     Assumes valid unit, soc control.
 */
int
soc_sbx_caladan3_ddr_init(int unit)
{
#ifdef BCM_DDR3_SUPPORT
    uint32 ddr_config = 0;
    int max_memories;
    int rv = SOC_E_NONE;
    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (dev_id == BCM88034_DEVICE_ID) {
        max_memories = SOC_SBX_CALADAN3_TMU_CI_INSTANCE_NUM/2;
    } else {
        max_memories = SOC_SBX_CALADAN3_TMU_CI_INSTANCE_NUM;
    }

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    if (soc_feature(unit, soc_feature_ddr3)) {
        SOC_DDR3_NUM_COLUMNS(unit) = soc_property_get(unit,spn_EXT_RAM_COLUMNS, 1024);
        SOC_DDR3_NUM_BANKS(unit) = soc_property_get(unit,spn_EXT_RAM_BANKS, 8);
        SOC_DDR3_NUM_MEMORIES(unit) = soc_property_get(unit,spn_EXT_RAM_PRESENT, max_memories);
        SOC_DDR3_NUM_ROWS(unit) = soc_property_get(unit,spn_EXT_RAM_ROWS, 16384);

        /* default to different DDR speed and mem grade based on device id */
        if ((dev_id == BCM88034_DEVICE_ID)) {
              SOC_DDR3_CLOCK_MHZ(unit) = soc_property_get(unit, spn_DDR3_CLOCK_MHZ, DDR_FREQ_800);
              SOC_DDR3_MEM_GRADE(unit) = soc_property_get(unit, spn_DDR3_MEM_GRADE, MEM_GRADE_111111);
        } else if ((dev_id == BCM88039_DEVICE_ID)) {
              SOC_DDR3_CLOCK_MHZ(unit) = soc_property_get(unit, spn_DDR3_CLOCK_MHZ, DDR_FREQ_1066);
              SOC_DDR3_MEM_GRADE(unit) = soc_property_get(unit, spn_DDR3_MEM_GRADE, MEM_GRADE_141414);
        } else {
              SOC_DDR3_CLOCK_MHZ(unit) = soc_property_get(unit, spn_DDR3_CLOCK_MHZ, DDR_FREQ_933);
              SOC_DDR3_MEM_GRADE(unit) = soc_property_get(unit, spn_DDR3_MEM_GRADE, MEM_GRADE_131313);
        }


        if (SOC_DDR3_NUM_MEMORIES(unit) > max_memories) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ddr_init: unit %d number of memories(%d) greater than max(%d)\n"),
                       unit, SOC_DDR3_NUM_MEMORIES(unit), max_memories));
            return SOC_E_UNIT;
        }

        if ((dev_id != BCM88039_DEVICE_ID) &&
            (SOC_DDR3_CLOCK_MHZ(unit) == DDR_FREQ_1066)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ddr_init: unit %d 2133 DDR is only stable on bcm88039 device\n"),
                       unit));
            return SOC_E_PARAM;
        }

        if ((dev_id == BCM88034_DEVICE_ID) &&
            (SOC_DDR3_CLOCK_MHZ(unit) != DDR_FREQ_800)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ddr_init: unit %d bcm88034 device only support 1600 DDR\n"),
                       unit));
            return SOC_E_PARAM;
        }

#ifdef BCM_WARM_BOOT_SUPPORT
        if (!SOC_WARM_BOOT(unit)) {
#endif
            /* don't write to the chip if in warm boot */

            ddr_config = (1 << SOC_DDR3_NUM_MEMORIES(unit)) - 1;
            soc_ddr40_set_shmoo_dram_config(unit, ddr_config);
            rv = soc_ddr40_phy_pll_ctl( unit, 0, SOC_DDR3_CLOCK_MHZ(unit), DDR_PHYTYPE_AND, 0 );
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_soc_sbx_caladan3_ddr_init: on unit %d failed at ddr40_phy_pll_ctl() %d"), unit, rv));
                return rv;
            } else {
                rv = soc_ddr40_ctlr_ctl( unit, 0, DDR_CTLR_T2, 0 );
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_soc_sbx_caladan3_ddr_init: on unit %d failed at ddr40_ctlr_ctl()  %d"), unit, rv));
                    return rv;
                } else {
                    rv = soc_ddr40_phy_calibrate( unit, 0, DDR_PHYTYPE_AND, 0 );
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "_soc_sbx_caladan3_ddr_init: on unit %d failed at ddr40_phy_calibrate() %d"), unit, rv));
                        return rv;
                    }
                }
            }
#ifdef BCM_WARM_BOOT_SUPPORT
        }
#endif
    }
    return rv;
#else /* BCM_DDR3_SUPPORT */
    return SOC_E_UNAVAIL;
#endif /* BCM_DDR3_SUPPORT */
}


/*
 * Function:
 *     soc_sbx_caladan3_driver_init
 * Purpose:
 *     Caladan3 driver intializaiton code
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */

int
soc_sbx_caladan3_driver_init(int unit)
{
    int rv = SOC_E_NONE;
    soc_control_t *soc;

    if (!SOC_UNIT_VALID(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d not valid\n"), unit));
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d not attached\n"), unit));
        return(SOC_E_UNIT);
    }

    rv = soc_sbx_caladan3_ocm_hw_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d OCM hardware init failed %d\n"),
                   unit, rv));
        return (rv);
    }

    rv = soc_sbx_caladan3_ocm_driver_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d" 
                              "OCM driver init failed: %d\n"), unit, rv));
        return SOC_E_UNIT;
    }

#ifndef HPP_CLUSTER_ONLY

    rv = soc_sbx_caladan3_sws_driver_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d " 
                              "SWS driver init failed: %d\n"), unit, rv));
        return SOC_E_UNIT;
    }

    rv = soc_sbx_caladan3_ppe_driver_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ppe_driver_init: unit %d " 
                              "PPE driver init failed: %d\n"), unit, rv));
        return SOC_E_UNIT;
    }


    rv = soc_sbx_caladan3_ped_driver_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d " 
                              "PED driver init failed: %d\n"), unit, rv));
        return SOC_E_UNIT;
    }
#endif /* HPP_CLUSTER_ONLY */

    rv = soc_sbx_caladan3_lr_driver_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d LRP driver init failed %d\n"),
                   unit, rv));
        return (rv);
    }

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_LRP_BYPASS, 0) < 2) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_ci_init(unit));
    }

    SOC_SBX_CONTROL(unit)->ucodetype = soc_sbx_configured_ucode_get(unit);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Caladan3 unit %d ucodetype %d\n"), unit, SOC_SBX_CONTROL(unit)->ucodetype));
    SOC_SBX_CONTROL(unit)->ucode_erh = soc_sbx_configured_ucode_erh_get(unit);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Caladan3 unit %d ucode_erh %d\n"), unit, SOC_SBX_CONTROL(unit)->ucode_erh));

    /*
     * Enable TMU/COP/CMU if LRP_BYPASS indicates so
     */
    if (soc_property_get(unit, spn_LRP_BYPASS, 0) < 2) {

#if TMU_INIT

        rv = soc_sbx_caladan3_tmu_driver_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d TMU driver init failed %d\n"), 
                       unit, rv));
            return (rv);
        }
       
#endif
#ifndef HPP_CLUSTER_ONLY
#if CMU_INIT
#ifdef CMU_NO_TR
        rv = soc_sbx_caladan3_cmu_driver_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d CMU driver init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif
#endif

#if COP_INIT
        rv = soc_sbx_caladan3_cop_driver_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d COP driver init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

#ifdef ETU_INIT
        if (soc_property_get(unit, spn_CALADAN3_ETU_ENABLE, TRUE)) {
            rv = soc_sbx_caladan3_etu_driver_init(unit);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Caladan3 unit %d ETU driver init failed %d\n"),
                           unit, rv));
            return (rv);
            }
        } /* if (enabling Caladan 3 ETU, default to TRUE) */
#endif
#endif /* HPP_CLUSTER_ONLY */
    }

    if (!soc_property_get(unit, spn_LRP_BYPASS, 0)) {

#if UCODE_LOAD
        rv = soc_sbx_caladan3_ucode_init(unit);
        if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d Ucode init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
        /* moved tmu WB init here to ensure other soc layer init complete */
        rv = soc_sbx_tmu_wb_state_init(unit);
        if (SOC_E_NONE != rv) {
            LOG_DEBUG(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                       "Caladan3 unit %d tmu hash init final failed %d\n"),
                       unit, rv));
            return rv;
        }       
#endif

#ifdef BCM_WARM_BOOT_SUPPORT

        /* Once complete driver init and g3p1 init (ucode_init) is complete
         * we have all the segments configured and can now update the cmu warm
         * boot state.
         */
        rv = soc_sbx_caladan3_cmu_init_final(unit);
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d cmu init final failed %d\n"), unit, rv));
            return rv;
        }
        
#endif /* BCM_WARM_BOOT_SUPPORT */    
    

        rv = soc_c3_rce_init(unit);
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d RCE init failed %d\n"), unit, rv));
            return rv;
        }
    }

    return rv;
}


/*
 * Function:
 *     soc_sbx_caladan3_packet_dma_init
 * Purpose:
 *     Caladan3 packet DMA intializaiton code
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit.
 *     Must follow soc_sbx_caladan3_driver_init.
 */
STATIC int
soc_sbx_caladan3_packet_dma_init(int unit)
{
    if (soc_dma_init(unit) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d DMA initialization failed\n"),
                   unit));
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}


int
soc_sbx_caladan3_reconfig(int unit)
{
    int                 rv;
    uint32  regval;
#if CHECK_RECONFIG_TIME
    sal_usecs_t         start_time, end_time;
    float               seconds;
#endif
    int                 xl_port;


    if (SOC_CONTROL(unit)->cl0_reset) {

        /* Resetting the MAC allows packets in */
        READ_CX_SOFT_RESET_0r(unit, &regval);

        soc_reg_field_set(unit, CX_SOFT_RESET_0r, &regval, CL0_RESET_Nf, 0);
        soc_reg_field_set(unit, CX_SOFT_RESET_0r, &regval, CL0_PORT_RESET_Nf, 0);

        WRITE_CX_SOFT_RESET_0r(unit, regval);

        soc_reg_field_set(unit, CX_SOFT_RESET_0r, &regval, CL0_RESET_Nf, 1);
        soc_reg_field_set(unit, CX_SOFT_RESET_0r, &regval, CL0_PORT_RESET_Nf, 1);

        WRITE_CX_SOFT_RESET_0r(unit, regval);

    }

#if CHECK_RECONFIG_TIME
    start_time = sal_time_usecs();
#endif

    if (!SOC_HOTSWAP_TDM) {

        /* Stop traffic from the lines and fabric */
        soc_sbx_caladan3_sws_pr_enable(unit, SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(0), 0);
        soc_sbx_caladan3_sws_pr_enable(unit, SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(1), 0);

        READ_LRA_CONFIG0r(unit, &regval);
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 0);
        WRITE_LRA_CONFIG0r(unit, regval);

    }

    if (!SOC_HOTSWAP_TDM) {
        rv = soc_sbx_caladan3_sws_driver_init(unit);
        if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_driver_init: unit %d "
                                  "SWS driver init failed: %d\n"), unit, rv));
            return rv;
        }
    } else {
        rv = soc_sbx_caladan3_sws_hotswap(unit);
        if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_hotswap: unit %d "
                                  "SWS driver init failed: %d\n"), unit, rv));
            return rv;
        }
    }

    if (!SOC_HOTSWAP_TDM) {

        READ_LRA_CONFIG0r(unit, &regval);
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 1);
        WRITE_LRA_CONFIG0r(unit, regval);

    }

    if (SOC_CONTROL(unit)->cl0_reset &&
        (!SOC_IS_CALADAN3_REVB(unit) || !SOC_HOTSWAP_TDM)) {

        /* Force CMIC port to give credits to the PT0 block */
        regval = 0;
        WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, regval);
        soc_reg_field_set(unit, CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr,
            &regval, RELEASE_ALL_CREDITSf, 1);
        WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, regval);

        /* Reset the XL ports */

        /* XL ports start one after the CMIC port */
        soc_sbx_caladan3_cmic_port_get(unit, &xl_port);
        xl_port++;

        READ_PORT_ENABLE_REGr(unit, xl_port, &regval);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT0f , 0);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT1f , 0);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT2f , 0);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT3f , 0);
        WRITE_PORT_ENABLE_REGr(unit, xl_port, regval);

        READ_PORT_SOFT_RESETr(unit, xl_port, &regval);
        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE0f, 1);
        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE1f, 1);
        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE2f, 1);
        WRITE_PORT_SOFT_RESETr(unit, xl_port, regval);

        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE0f, 0);
        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE1f, 0);
        soc_reg_field_set(unit, PORT_SOFT_RESETr, &regval, XPORT_CORE2f, 0);
        WRITE_PORT_SOFT_RESETr(unit, xl_port, regval);

        READ_PORT_ENABLE_REGr(unit, xl_port, &regval);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT0f , 1);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT1f , 1);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT2f , 1);
        soc_reg_field_set(unit, PORT_ENABLE_REGr, &regval, PORT3f , 1);
        WRITE_PORT_ENABLE_REGr(unit, xl_port, regval);
    }

    rv = soc_sbx_caladan3_ped_driver_init(unit);
    if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_driver_init: unit %d "
                              "PED driver init failed: %d\n"), unit, rv));
        return rv;
    }

    rv = soc_sbx_caladan3_mac_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 MAC hardware init failed\n")));
        return rv;
    }

    rv = soc_sbx_caladan3_link_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 Link init failed\n")));
        return rv;
    }

    if (!SOC_HOTSWAP_TDM) {

        /* Enable traffic from the lines and fabric */
        soc_sbx_caladan3_sws_pr_enable(unit, SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(0), 1);
        soc_sbx_caladan3_sws_pr_enable(unit, SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(1), 1);

    }

#if CHECK_RECONFIG_TIME
    end_time = sal_time_usecs();
    seconds = (float)(end_time - start_time) / 1000000.0;
    LOG_CLI((BSL_META_U(unit,
                        "reconfig tdm: sws init time: %f seconds\n"), seconds));
#endif

    return rv;
}


/*
 * Function:
 *     soc_sbx_caladan3_init
 * Purpose:
 *     Caladan3 intializaiton code
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
soc_sbx_caladan3_init(int unit, soc_sbx_config_t *cfg)

{
    int rv = SOC_E_NONE;
    soc_control_t *soc;

    /*
     * Create mutex.
     */
    soc = SOC_CONTROL(unit);
    if ((soc->schan_wb_mutex = sal_mutex_create("schan_wb_mutex")) == NULL) {
        return SOC_E_MEMORY;
    }


    SOC_SBX_CFG_CALADAN3(unit)->c3_64bit_pc = soc_property_get(unit,"c3_64bit_pc", 0);
    if (!SOC_UNIT_VALID(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d not valid\n"), unit));
        return SOC_E_UNIT;
    }

    if (!cfg) {
        return SOC_E_PARAM;
    }

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_init: unit %d not attached\n"), unit));
        return SOC_E_UNIT;
    }
    
    if (SOC_RECONFIG_TDM) {
        rv = soc_sbx_caladan3_reconfig(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_reconfig failed: %s\n"),
                       soc_errmsg(rv)));
        }
        return rv;
    }

    rv = _soc_sbx_caladan3_cmic_init(unit, cfg->reset_ul);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 CMIC hardware init failed\n")));
        return rv;
    }

    if (soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        return rv;
    }

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_LRP_BYPASS, 0) < 2) {
        rv = soc_sbx_caladan3_ddr_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d DDR init failed %d\n"),
                       unit, rv));
            return (rv);
        }
    }
    if ( soc_property_get(unit, spn_LRP_BYPASS, 0) < 2) {
        rv = soc_sbx_caladan3_driver_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d Caladan3 Driver init failed %d\n"),
                       unit, rv));
            return rv;
        }
    }

    rv = soc_sbx_caladan3_packet_dma_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d Caladan3 Packet DMA init failed %d\n"),
                   unit, rv));
        return rv;
    }

    rv = soc_sbx_caladan3_mac_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 MAC hardware init failed\n")));
        return rv;
    }

    rv = soc_sbx_caladan3_link_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 Link init failed\n")));
        return rv;
    }

    return rv;
}

#ifdef BCM_SBUSDMA_SUPPORT    
/*
 * Function:
 *     soc_sbx_caladan3_sbusdma_cmc_ch_map
 * Purpose:
 *     Caladan3 sbusdma DMA cmc/channel select based on operation and memory
 * Parameters:
 *     (IN)unit    - Device number
 *     (IN)mem     - Memory type
 *                 - for desc DMA, pass in INVALIDm
 *     (IN)op      - Dma type  SOC_SBUSDMA_TYPE_TDMA
 *                             SOC_SBUSDMA_TYPE_SLAM
 *                             SOC_SBUSDMA_TYPE_DESC
 *     (OUT)cmc - Cmicm CMC (0-2)
 *     (OUT)ch  - Cmicm CMC channel 
 * Notes:
 *     when TMB DMA flow control is enabled. this requires soc_feature(soc_feature_cmicm_multi_dma_cmc)
 *     to allow multiple DMA channels used for a particular DMA (such as SLAM dma)
 */
void
soc_sbx_caladan3_sbusdma_cmc_ch_map(int unit, soc_mem_t mem, uint32 op, int *cmc, int *ch)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    /* following is cmc/ch required for certain memory by hardware
     * when flow control is enabled. 
     * so for other memories, we need to move table/slam/desc DMA
     * to cmc2.
     */
    if (op == SOC_SBUSDMA_TYPE_TDMA) {
	*cmc = 2;
	*ch = soc->tdma_ch;
    } else if (op == SOC_SBUSDMA_TYPE_SLAM) {
	switch (mem) {
	    case TMB_UPDATER_CMD_FIFO0m:
		*cmc = 0;
		*ch = 0;
		break;
	    case TMB_UPDATER_CMD_FIFO1m:
		*cmc = 0;
		*ch = 1;
		break;
	    case TMB_UPDATER_FREE_CHAIN_FIFO0m:
		*cmc = 0;
		*ch = 2;
		break;
	    case TMB_UPDATER_FREE_CHAIN_FIFO1m:
		*cmc = 1;
		*ch = 0;
		break;
	    case TMB_UPDATER_FREE_CHAIN_FIFO2m:
		*cmc = 1;
		*ch = 1;
		break;
	    case TMB_UPDATER_FREE_CHAIN_FIFO3m:
		*cmc = 1;
		*ch = 2;
		break;
	    case ETU_CP_FIFOm:
		*cmc = 2;
		*ch = 0;
		break;
	    case INVALIDm:
	    default:
		/* all other memories moves to cmc2, keep the original channel */
		*cmc = 2;
		*ch = soc->tslam_ch;
		break;
	}
    } else {
	*cmc = 2;
	*ch = soc->desc_ch;
    }
}

/*
 * Function:
 *     soc_sbx_caladan3_sbusdma_cmc_ch_type_get
 * Purpose:
 *     get Caladan3 sbusdma DMA cmc/channel DMA type
 * Parameters:
 *     (IN)unit    - Device number
 *     (IN)cmc     - Cmicm CMC (0-2)
 *     (IN)ch      - Cmicm CMC channel 
 *     (OUT)op     - Dma type  SOC_SBUSDMA_TYPE_TDMA
 *                             SOC_SBUSDMA_TYPE_SLAM
 *                             SOC_SBUSDMA_TYPE_DESC
 * Notes:
 *     this requires soc_feature(soc_feature_cmicm_multi_dma_cmc)
 *     to allow multiple DMA channels used for a particular DMA (such as SLAM dma)
 */
int
soc_sbx_caladan3_sbusdma_cmc_ch_type_get(int unit, int cmc, int ch, uint32 *op)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    if ((cmc != 2) || (ch == soc->tslam_ch)) {
	*op = SOC_SBUSDMA_TYPE_SLAM;
    } else if (ch == soc->tdma_ch) {
	*op = SOC_SBUSDMA_TYPE_TDMA;
    } else if (ch == soc->desc_ch) {
	*op = SOC_SBUSDMA_TYPE_DESC;
    } else {
	return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}
#endif /* BCM_SBUSDMA_SUPPORT */

/*
 * Function
 *   soc_sbx_caladan3_ilkn_12p5_ghz_pll_config
 * Purpose
 *   Override the PLL for IL and SWS to push the ilkn operate at 12.5G
 *   The SWS is pushed to 650Mhz
 *   The MAC is retained at 412.5Mhz (shares pll with ilkn)
 *   ILKN is pushed to 721.875Mhz
 *   Warning changing PLLs affect system functionality and may 
 *           lead to severe issues
 */
int
soc_sbx_caladan3_ilkn_12p5_ghz_pll_config(int unit, int enable) 
{
    int rv = SOC_E_NONE;
    uint32 rval = 0;

    if (enable) {
        /* Enable Override */
        SOC_IF_ERROR_RETURN(READ_CX_PLL_CTRLr(unit, &rval));
        soc_reg_field_set(unit, CX_PLL_CTRLr, 
                          &rval, MAC_BOND_OVERRIDEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CX_PLL_CTRLr(unit, rval));

        /* Update NDiv */
        SOC_IF_ERROR_RETURN(
            READ_CX_MAC_PLL_NDIV_INTEGERr(unit, &rval));
        soc_reg_field_set(unit, CX_MAC_PLL_NDIV_INTEGERr, 
                          &rval, NDIV_INTf, 0x73);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_NDIV_INTEGERr(unit, rval));

        SOC_IF_ERROR_RETURN(
            READ_CX_MAC_PLL_NDIV_FRACTIONr(unit, &rval));
        soc_reg_field_set(unit, CX_MAC_PLL_NDIV_FRACTIONr, 
                          &rval, NDIV_FRACf, 0x80000);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_NDIV_FRACTIONr(unit, rval));

        /* Update Channel 1 */
        SOC_IF_ERROR_RETURN(
            READ_CX_MAC_PLL_CHANNEL_1r(unit, &rval));
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                          &rval, MDIVf, 0x7);
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                          &rval, LOAD_ENf, 1);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_CHANNEL_1r(unit, rval));
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                          &rval, LOAD_ENf, 0);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_CHANNEL_1r(unit, rval));

        /* Update Channel 0 */
        SOC_IF_ERROR_RETURN(
            READ_CX_MAC_PLL_CHANNEL_0r(unit, &rval));
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                          &rval, MDIVf, 0x4);
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                          &rval, LOAD_ENf, 1);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_CHANNEL_0r(unit, rval));
        soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                          &rval, LOAD_ENf, 0);
        SOC_IF_ERROR_RETURN(
            WRITE_CX_MAC_PLL_CHANNEL_0r(unit, rval));

        /* update SWS clk */
        SOC_IF_ERROR_RETURN(
            READ_CX_SWS_PLL_NDIV_INTEGERr(unit, &rval));
        if (soc_reg_field_get(unit, CX_SWS_PLL_NDIV_INTEGERr,
                              rval, NDIV_INTf) != 0x9C) {
            soc_reg_field_set(unit, CX_SWS_PLL_NDIV_INTEGERr, 
                              &rval, NDIV_INTf, 0x9C);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_SWS_PLL_NDIV_INTEGERr(unit, rval));   
        }

    } else {

        /* update SWS clk if required */
        SOC_IF_ERROR_RETURN(
            READ_CX_SWS_PLL_NDIV_INTEGERr(unit, &rval));
        if (soc_reg_field_get(unit, CX_SWS_PLL_NDIV_INTEGERr,
                              rval, NDIV_INTf) != 0x9A) {
            soc_reg_field_set(unit, CX_SWS_PLL_NDIV_INTEGERr, 
                              &rval, NDIV_INTf, 0x9A);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_SWS_PLL_NDIV_INTEGERr(unit, rval));   
        }
        SOC_IF_ERROR_RETURN(
            READ_CX_PLL_CTRLr(unit, &rval));
        if (soc_reg_field_get(unit, CX_PLL_CTRLr,
                              rval, MAC_BOND_OVERRIDEf) == 1) {
            /* Override set 
             * we were operating at higher clock, Fall back 
             */
 
            /* Update Ndiv */           
            SOC_IF_ERROR_RETURN(
                READ_CX_MAC_PLL_NDIV_INTEGERr(unit, &rval));
            soc_reg_field_set(unit, CX_MAC_PLL_NDIV_INTEGERr, 
                              &rval, NDIV_INTf, 0x84);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_NDIV_INTEGERr(unit, rval));

            SOC_IF_ERROR_RETURN(
                READ_CX_MAC_PLL_NDIV_FRACTIONr(unit, &rval));
            soc_reg_field_set(unit, CX_MAC_PLL_NDIV_FRACTIONr, 
                              &rval, NDIV_FRACf, 0);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_NDIV_FRACTIONr(unit, rval));

            /* Update Channel 1 */
            SOC_IF_ERROR_RETURN(
                READ_CX_MAC_PLL_CHANNEL_1r(unit, &rval));
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                              &rval, MDIVf, 0x8);
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                              &rval, LOAD_ENf, 1);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_CHANNEL_1r(unit, rval));
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_1r, 
                              &rval, LOAD_ENf, 0);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_CHANNEL_1r(unit, rval));
    
            /* Update Channel 0 */
            SOC_IF_ERROR_RETURN(
                READ_CX_MAC_PLL_CHANNEL_0r(unit, &rval));
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                              &rval, MDIVf, 0x5);
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                              &rval, LOAD_ENf, 1);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_CHANNEL_0r(unit, rval));
            soc_reg_field_set(unit, CX_MAC_PLL_CHANNEL_0r, 
                              &rval, LOAD_ENf, 0);
            SOC_IF_ERROR_RETURN(
                WRITE_CX_MAC_PLL_CHANNEL_0r(unit, rval));

        }
    }
    /* Allow some time */
    sal_usleep(1000);
    SOC_IF_ERROR_RETURN(READ_CX_MAC_PLL_STATUSr(unit, &rval));
    if (soc_reg_field_get(unit, CX_MAC_PLL_STATUSr, rval, LOCKf) == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "ilkn_12p5_ghz_pll_config: MAC PLL lock failed\n")));
        return SOC_E_FAIL;
    }
    SOC_IF_ERROR_RETURN(READ_CX_SWS_PLL_STATUSr(unit, &rval));
    if (soc_reg_field_get(unit, CX_SWS_PLL_STATUSr, rval, LOCKf) == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "ilkn_12p5_ghz_pll_config: SWS PLL lock failed\n")));
        return SOC_E_FAIL;
    }
    
    return rv;
}

#endif /* BCM_CALADAN3_SUPPORT */


