/*
 * $Id: ramon_fe1600_drv.c,v 1.72 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 DRV
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

#include <shared/bsl.h>
#include <shared/shr_template.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/defs.h>
#include <soc/mem.h>

#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/fe1600/fe1600_fabric_topology.h>
#include <soc/dnxf/fe1600/fe1600_drv.h>
#include <soc/dnxf/fe1600/fe1600_port.h>
#include <soc/dnxf/fe1600/fe1600_config_defs.h>
#include <soc/dnxf/fe1600/fe1600_config_imp_defs.h>
#include <shared/bitop.h>
#include <soc/register.h>
#include <soc/dnxc/legacy/dnxc_mbist.h>

extern char *_build_release;

#if defined(BCM_88790_A0)

/*Defines*/
#define SOC_RAMON_FE1600_GRACEFUL_SHUT_DOWN_DISABLE_DELAY 2000000 /*2 seconds*/

/*
 * PVT monitor
 */
#define _SOC_RAMON_FE1600_PVT_MON_NOF                             (4)
#define _SOC_RAMON_FE1600_PVT_FACTOR                              (5660)
#define _SOC_RAMON_FE1600_PVT_BASE                                (4283900)

/* 
 *MAC Comma burst
 */
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE1          (11)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE1            (2)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE2          (11)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE2            (1)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE3          (12)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE3            (1)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_REPEATER     (_SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE2)
#define _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_REPEATER       (_SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE2)


static
soc_reg_t ramon_fe1600_interrupt_monitor_mem_reg[] = {
    CCS_ECC_ERR_MONITOR_MEM_MASKr,
    DCH_ECC_ERR_MONITOR_MEM_MASKr,
    DCL_ECC_ERR_MONITOR_MEM_MASKr,
    DCMA_ECC_ERR_MONITOR_MEM_MASK_Ar,
    DCMB_ECC_ERR_MONITOR_MEM_MASK_Ar,
    ECI_ECC_ERR_MONITOR_MEM_MASKr,
    BRDC_FMACH_ECC_1B_ERR_MONITOR_MEM_MASKr,
    BRDC_FMACH_ECC_2B_ERR_MONITOR_MEM_MASKr,
    BRDC_FMACL_ECC_1B_ERR_MONITOR_MEM_MASKr,
    BRDC_FMACL_ECC_2B_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_2B_ERR_MONITOR_MEM_MASKr,
    RTP_ECC_ERR_2B_MONITOR_MEM_MASKr,
    RTP_ECC_ERR_1B_MONITOR_MEM_MASKr,
    RTP_PAR_ERR_MEM_MASKr,
    INVALIDr
};

static
soc_reg_t ramon_fe1600_interrupts_mask_registers[] = {
    CCS_INTERRUPT_MASK_REGISTERr,
    CCS_CAPTURE_FILTER_MASK_0r,
    CCS_CAPTURE_FILTER_MASK_1r,
    DCH_INTERRUPT_MASK_REGISTERr,
    DCH_INTERRUPT_MASK_REGISTER_1r,
    DCH_INTERRUPT_MASK_REGISTER_2r,
    DCH_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_0r,
    DCH_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_1r,
    DCH_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_2r,
    DCH_ERROR_FILTER_MASKr,
    DCH_ERROR_FILTER_MASK_ENr,
    DCL_INTERRUPT_MASK_REGISTERr,
    DCMA_INTERRUPT_MASK_REGISTERr,
    DCMB_INTERRUPT_MASK_REGISTERr,
    ECI_INTERRUPT_MASK_REGISTERr,
    ECI_MAC_INTERRUPT_MASK_REGISTERr,
    ECI_ECI_INTERNAL_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTER_1r,
    FMAC_INTERRUPT_MASK_REGISTER_2r,
    FMAC_INTERRUPT_MASK_REGISTER_3r,
    FMAC_INTERRUPT_MASK_REGISTER_4r,
    FMAC_INTERRUPT_MASK_REGISTER_5r,
    FMAC_INTERRUPT_MASK_REGISTER_6r,
    FMAC_INTERRUPT_MASK_REGISTER_7r,
    FMAC_INTERRUPT_MASK_REGISTER_8r,
    FMAC_INTERRUPT_MASK_REGISTER_9r,
    FSRD_INTERRUPT_MASK_REGISTERr,
    FSRD_QUAD_INTERRUPT_MASK_REGISTERr,
    FSRD_WC_UC_MEM_MASK_BITMAPr,
    OCCG_INTERRUPT_MASK_REGISTERr,
    RTP_INTERRUPT_MASK_REGISTERr,
    RTP_DRHP_INTERRUPT_MASK_REGISTERr,
    RTP_DRHS_INTERRUPT_MASK_REGISTERr,
    RTP_CRH_INTERRUPT_MASK_REGISTERr,
    RTP_GENERAL_INTERRUPT_MASK_REGISTERr,
    RTP_ECC_1B_ERR_INTERRUPT_MASK_REGISTERr,
    RTP_ECC_2B_ERR_INTERRUPT_MASK_REGISTERr,
    RTP_PAR_ERR_INTERRUPT_MASK_REGISTERr,
    INVALIDr
};

int
soc_dnxf_clean_rtp_table_array(int unit, soc_mem_t mem, soc_reg_above_64_val_t data)
{
    uint32 reg_val32;
    unsigned i, elem_num;
    uint32 current_address, element_skip;        /* current address in the array, and its skip */
    uint64  val64;
    DNXC_INIT_FUNC_DEFS;
    
    if (SOC_MEM_IS_ARRAY_SAFE(unit,mem))
    {
        elem_num = SOC_MEM_NUMELS(unit, mem);
        element_skip = SOC_MEM_ELEM_SKIP(unit, mem);
    }
    else
    {
        elem_num = 1;
        element_skip = 0;
    }
    current_address = SOC_MEM_INFO(unit, mem).base;
    
    for (i = 0; i < elem_num; ++i) { /* loop over all the elements of the array */
        DNXC_IF_ERR_EXIT(WRITE_RTP_INDIRECT_COMMAND_WR_DATAr(unit, data));
        COMPILER_64_ZERO(val64);
        DNXC_IF_ERR_EXIT(WRITE_RTP_INDIRECT_COMMAND_DATA_INCREMENTr(unit, val64));
    
        reg_val32 = 0;
        soc_reg_field_set(unit, RTP_INDIRECT_COMMAND_ADDRESSr, &reg_val32, INDIRECT_COMMAND_ADDRf, current_address);

        soc_reg_field_set(unit, RTP_INDIRECT_COMMAND_ADDRESSr, &reg_val32, INDIRECT_COMMAND_TYPEf, 0 /*write command*/);
        DNXC_IF_ERR_EXIT(WRITE_RTP_INDIRECT_COMMAND_ADDRESSr(unit, reg_val32));
    
        reg_val32 = 0;
        soc_reg_field_set(unit, RTP_INDIRECT_COMMANDr, &reg_val32, INDIRECT_COMMAND_COUNTf, SOC_MEM_INFO(unit, mem).index_max+1);
        soc_reg_field_set(unit, RTP_INDIRECT_COMMANDr, &reg_val32, INDIRECT_COMMAND_TRIGGERf, 1);
        DNXC_IF_ERR_EXIT(WRITE_RTP_INDIRECT_COMMANDr(unit, reg_val32));

        current_address += element_skip;
    }

exit:
    DNXC_FUNC_RETURN;
}

#define RTP_INDIRECT_COUNT (8*1024)

STATIC int
soc_ramon_fe1600_reset_tables(int unit)
{
    soc_reg_above_64_val_t data, field;
    int total_links, active_links;
    uint32 score, entry;
    uint32 bmp[4];
    uint32 totsf_val, slsct_val, score_slsct, links_count, sctinc_val, sctinc;
    int link;
	soc_field_t scrub_en;
	uint64 reg_val64;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    scrub_en = soc_reg64_field32_get(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, reg_val64, SCT_SCRUB_ENABLEf); 
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, 0);                       
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

    SOC_REG_ABOVE_64_CLEAR(data);

    /*MCLBT*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MCLBTPm, data));
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MCLBTSm, data));

    /*RCGLBT*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_RCGLBTm, data));
    /*TOTSF*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_TOTSFm, data));
    /*SLSCT*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_SLSCTm, data));
    /*SCTINC*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_SCTINCm, data));

    /*Should be cleared to value of {10d30,10d30,10d30,10d30}*/
    SOC_REG_ABOVE_64_CLEAR(field);
    soc_mem_field_set(unit, RTP_MEM_800000m, data, ITEM_1f, field);
    
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MEM_800000m, data));
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MEM_900000m, data));

    totsf_val = 0;
    links_count = 1;
    soc_mem_field_set(unit, RTP_TOTSFm, &totsf_val, LINK_NUMf, (uint32*)&links_count);

    slsct_val = 0;
    score_slsct = 0;
    soc_mem_field_set(unit, RTP_SLSCTm, &slsct_val, LINK_NUMf, &score_slsct);

    sctinc_val = 0;
    sctinc = 0;
    soc_mem_field_set(unit, RTP_SCTINCm, &sctinc_val, LINK_NUMf, &sctinc);

    for(link = 0 ; link < SOC_DNXF_DEFS_GET(unit, nof_links) ; link++) {
        /*build bitmap*/
        bmp[0] = bmp[1] = bmp[2] = bmp[3] = 0;
        SHR_BITSET(bmp,link);

        DNXC_IF_ERR_EXIT(WRITE_RTP_RCGLBTm(unit, MEM_BLOCK_ALL, link, bmp));
        DNXC_IF_ERR_EXIT(WRITE_RTP_TOTSFm(unit, MEM_BLOCK_ALL, link, &totsf_val));
        DNXC_IF_ERR_EXIT(WRITE_RTP_SLSCTm(unit, MEM_BLOCK_ALL, link, &slsct_val));
        DNXC_IF_ERR_EXIT(WRITE_RTP_SCTINCm(unit, MEM_BLOCK_ALL, link, &sctinc_val));
    }

    /*Multiplier*/
    for(total_links = 1 ; total_links <= 64 ; total_links++) {
        for(active_links = 1 ; active_links <= total_links ; active_links++) {
            score = (SOC_RAMON_FE1600_MAX_LINK_SCORE * active_links) / total_links;
            if ((SOC_RAMON_FE1600_MAX_LINK_SCORE * active_links) % total_links != 0) {
                score = score + 1;
            }
            if(active_links < 33) {
                entry=(total_links-1)*32+active_links-1;
            } else {
                entry = (64-total_links)*32+64-active_links; 
            }
             DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_TBm(unit, MEM_BLOCK_ALL, entry, &score));
        }
    }

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, scrub_en);                       
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_fe1600_reset_cmic_regs(int unit) 
{
    int lcpll, i;
    int dividend, divisor, mdio_delay;
    uint32 rval;
    uint32 lcpll_in, lcpll_out, control_1, control_3;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*LCPLL*/
    for(lcpll=0 ; lcpll<SOC_RAMON_FE1600_NOF_LCPLL ; lcpll++) {

#ifdef BCM_88754_A0
        if (SOC_IS_BCM88754_A0(unit))
        {
            lcpll_in = 1;
            lcpll_out = 1;
        } else 
#endif /*BCM_88754_A0*/
        {
            lcpll_in = soc_property_suffix_num_get(unit, lcpll, spn_SERDES_FABRIC_CLK_FREQ, "in", 1);
            lcpll_out = soc_property_suffix_num_get(unit, lcpll, spn_SERDES_FABRIC_CLK_FREQ, "out", 1);
        }

        /*config device*/
        switch(lcpll_out) {
            case soc_dnxc_init_serdes_ref_clock_125:
                control_1 = SOC_RAMON_FE1600_LCPLL_CONTROL1_125_VAL;
                break;
            case soc_dnxc_init_serdes_ref_clock_156_25:
                control_1 = SOC_RAMON_FE1600_LCPLL_CONTROL1_156_25_VAL;
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("lcpll_out: %d is out-of-ranget (use 0=125MHz, 1=156.25MHz"), lcpll_out));
        }

        switch(lcpll_in) {
            case soc_dnxc_init_serdes_ref_clock_125:
                control_3 = SOC_RAMON_FE1600_LCPLL_CONTROL3_125_VAL;
                break;
            case soc_dnxc_init_serdes_ref_clock_156_25:
                control_3 = SOC_RAMON_FE1600_LCPLL_CONTROL3_156_25_VAL;
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("lcpll_out: %d is out-of-ranget (use 0=125MHz, 1=156.25MHz"), lcpll_out));
        }

        switch(lcpll) {
             case 0:
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS0_PLL_CONTROL_1r(unit, control_1));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS0_PLL_CONTROL_2r(unit, 0x0064c000));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS0_PLL_CONTROL_3r(unit, control_3));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS0_PLL_CONTROL_4r(unit, 0x15c00000));
                break;
             case 1:
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS1_PLL_CONTROL_1r(unit, control_1));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS1_PLL_CONTROL_2r(unit, 0x0064c000));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS1_PLL_CONTROL_3r(unit, control_3));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS1_PLL_CONTROL_4r(unit, 0x15c00000));
                break;
             case 2:
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS2_PLL_CONTROL_1r(unit, control_1));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS2_PLL_CONTROL_2r(unit, 0x0064c000));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS2_PLL_CONTROL_3r(unit, control_3));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS2_PLL_CONTROL_4r(unit, 0x15c00000));
                break;
             case 3:
            default:
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS3_PLL_CONTROL_1r(unit, control_1));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS3_PLL_CONTROL_2r(unit, 0x0064c000));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS3_PLL_CONTROL_3r(unit, control_3));
                DNXC_IF_ERR_EXIT(WRITE_CMIC_XGXS3_PLL_CONTROL_4r(unit, 0x15c00000));
                break;
        }
    }

    DNXC_IF_ERR_EXIT(WRITE_CMIC_MISC_CONTROLr(unit, 0x00000f00)); 

    /*Reset*/
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SOFT_RESET_REGr(unit, 0x0));
    sal_usleep(20);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SOFT_RESET_REGr(unit, 0xffffffff));


   /* SBUS ring map:
     * Ring 0: ECI (0), OCCG (1)
     * Ring 1: DCH0 (2), DCH1 (3), DCH2 (4), CMICE (5), DCH3 (6), 
     *         DCL0 (7), DCL1 (8), DCL2 (9), DCL3 (10)
     * Ring 2: DCMA0 (11), DCMB0 (12), DCMA1 (13), DCMB1 (14), DCMC (15), 
     *         CCS0 (16), CCS1 (17), RTP (18), MESH_TOPOLOGY (19)
     * Ring 3: MAC0 (20), MAC1 (21), MAC2 (22), MAC3 (23), MAC4 (24), MAC5 (25), 
     *         MAC6 (26), MAC7 (27), MAC8 (28), MAC9 (29), MAC10 (30), MAC11 (31),
     *         MAC12 (32), MAC13 (33), MAC14 (34), MAC15 (35), BRDC_MACL(61)
     * Ring 4: MAC16 (36), MAC17 (37), MAC18 (37), MAC19 (39), MAC20 (40), MAC21 (41), 
     *         MAC22 (42), MAC23 (43), MAC24 (44), MAC25 (45), MAC26 (46), MAC27 (47),
     *         MAC28 (48), MAC29 (49), MAC30 (50), MAC31 (51), BRDC_MACH(62)
     * Ring 5: FSRD0 (52), FSRD1 (53), FSRD2 (54), FSRD3 (55), FSRD4 (56),
     *         FSRD5 (57), FSRD6 (58), FSRD7 (59), BRDC_FSRD(60)
     * Ring 6: OTPC (63)
     * Ring 7: -
     */
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_0r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_1r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_1r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_2r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_2r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_3r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_3r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_4r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_4r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_5r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_5r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_6r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_6r).rst_val_lo));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_7r(unit, SOC_REG_INFO(unit, CMIC_SBUS_RING_MAP_7r).rst_val_lo));

     /*Init broadcast 61*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 0,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 1,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 2,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 3,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 4,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 5,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 6,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 7,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 8,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 9,  61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 10, 61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 11, 61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 12, 61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 13, 61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 14, 61));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 15, 61));
    
    /*Init broadcast 62*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 16, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 17, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 18, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 19, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 20, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 21, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 22, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 23, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 24, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 25, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 26, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 27, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 28, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 29, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 30, 62));
    DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, 31, 62));

    /* Init broadcast 60 - FSRD */
#ifdef BCM_88754_A0
    if (!SOC_IS_BCM88754_A0(unit))
    {
#endif
        for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd) ; i++) { 
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SBUS_BROADCAST_IDr(unit, i, 60));
        }
#ifdef BCM_88754_A0
    }
#endif
    /* Mdio - internal*/
    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, 1);
    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, 24);
    rval = 0;
    DNXC_IF_ERR_EXIT(READ_CMIC_RATE_ADJUST_INT_MDIOr(unit, &rval));
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVIDENDf, dividend);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, rval));

    /*Mdio -Delay*/
    rval = 0;
    DNXC_IF_ERR_EXIT(READ_CMIC_CONFIGr(unit, &rval));
    mdio_delay = 0x7; /*setting max delay*/
    soc_reg_field_set(unit, CMIC_CONFIGr, &rval, MDIO_OUT_DELAYf, mdio_delay);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CONFIGr(unit, rval));

    /*ramon_fe1600 utils temp*/ 
    rval = 0x1D0003;
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CORE_PLL3_CTRL_STATUS_REGISTER_3r(unit, rval));
    /* The sleep should be 2 clocks of 5k = total 10milisec
     * To be on the safe side will put 20milisec sleep.
     */
    sal_usleep(20000);
    rval = 0x41D0003;
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CORE_PLL3_CTRL_STATUS_REGISTER_3r(unit, rval));
    
    /*Linkscan*/
   DNXC_IF_ERR_EXIT(soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_LINK_STAT_MSG_CLR));
   DNXC_IF_ERR_EXIT(soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_LINK_SCAN_EN_CLR));

exit:
    DNXC_FUNC_RETURN;
}

/*Set registers according to SOC properties*/
STATIC int
soc_ramon_fe1600_set_operation_mode(int unit) {

    uint32 reg_val32;
    uint64 reg_val64, val64;
    int i;
    uint32 repeater_mode_get;
    uint32 multistage_config;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*ECI*/
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_1r(unit, &reg_val32));
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, REP_MODEf, SOC_DNXF_IS_REPEATER(unit) ? 1 : 0);

    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, FE_13_MODEf, SOC_DNXF_IS_FE13(unit)  ? 1 : 0);
    multistage_config = (!SOC_DNXF_IS_MULTISTAGE(unit)) ? 0 :                                                                                        /*for single stage set to 0*/
                        (SOC_DNXF_IS_FE2(unit) && SOC_IS_FE1600_B0_AND_ABOVE(unit) && SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system) ? 0 : 1;     /*for multistage FE2 iff B0 and VSC128_in system set to 0*/ 
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, MULTI_STAGEf, multistage_config);
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, VSC_128_MODEf, SOC_DNXF_CONFIG(unit).system_is_fe600_in_system  ? 1 : 0);
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, PETRA_SYSTEMf, SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system  ? 1 : 0);
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, DUAL_PIPE_ENf, SOC_DNXF_CONFIG(unit).is_dual_mode  ? 1 : 0);
    if(SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system) {
        soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, FIELD_9_10f, 1);
    } else if ((SOC_DNXF_CONFIG(unit).system_is_dual_mode_in_system)) {
        soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, FIELD_9_10f, 2);
    } else {
        soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, FIELD_9_10f, 0);
    }
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, M_24L_MODEf, SOC_DNXF_CONFIG(unit).fabric_optimize_patial_links  ? 1 : 0);
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, PETRA_ENHANCE_ENf, (SOC_DNXF_CONFIG(unit).fabric_merge_cells)  ? 1 : 0);
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg_val32, PETRA_TDM_FRAG_NUMf, SOC_DNXF_CONFIG(unit).fabric_TDM_fragment);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_1r(unit, reg_val32));

    /*check if repeater mode is cfg correctly -  devices for repeater only use (this field is burned)*/
    /*check it at inner Block*/
    DNXC_IF_ERR_EXIT(READ_DCH_REG_005Ar(unit, REG_PORT_ANY, &reg_val32));
    repeater_mode_get = soc_reg_field_get(unit, DCH_REG_005Ar, reg_val32, FIELD_3_3f) ? 1 : 0;
    if (SOC_DNXF_IS_REPEATER(unit) != repeater_mode_get  && !SAL_BOOT_PLISIM) {
        DNXC_EXIT_WITH_ERR(SOC_E_INIT, (_BSL_DNXC_MSG("FABRIC_DEVICE_MODE!=REPEATER -  device for repeater only use")));
    }

    /*RTP*/
    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    COMPILER_64_SET(val64, 0,(soc_dnxf_load_balancing_mode_normal == SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode) ? 1 : 0);
    soc_reg64_field_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, DISABLE_RCG_LOAD_BALANCINGf, 
                            val64);                              
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));
    
    DNXC_IF_ERR_EXIT(READ_RTP_MULTICAST_MODE_SELECTIONr(unit, &reg_val32));
    soc_reg_field_set(unit, RTP_MULTICAST_MODE_SELECTIONr, &reg_val32, MC_INDIRECT_LIST_OF_FAPS_MODEf, 
                        soc_dnxf_multicast_mode_indirect == SOC_DNXF_CONFIG(unit).fabric_multicast_mode ? 1 : 0);


    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTICAST_MODE_SELECTIONr(unit, reg_val32));
    
    /*DCH*/
    reg_val32 = 0xf;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 2, 0x5b, 1, &reg_val32));
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 3, 0x5b, 1, &reg_val32));
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 4, 0x5b, 1, &reg_val32));
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 6, 0x5b, 1, &reg_val32));

    for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_instances_dch) ; i++) {    


        DNXC_IF_ERR_EXIT(READ_DCH_REG_0061r(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_REG_0061r, &reg_val32, PETRA_PIPE_BMP_ENf, 1);
        soc_reg_field_set(unit, DCH_REG_0061r, &reg_val32, PETRA_UNI_PRIf, SOC_DNXF_CONFIG(unit).vcs128_unicast_priority);
        soc_reg_field_set(unit, DCH_REG_0061r, &reg_val32, PETRA_PIPE_TDM_PRIf,  (SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe ? DNXF_TDM_PRIORITY_OVER_PRIMARY_PIPE : DNXF_TDM_PRIORITY_OVER_SECONDARY_PIPE));
        DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0061r(unit, i, reg_val32));
    }

    /*DCL*/
    for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_instances_dcl) ; i++) {   
        DNXC_IF_ERR_EXIT(READ_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, &reg_val32, PETRA_PIPE_TDM_PRIf, (SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe ? DNXF_TDM_PRIORITY_OVER_PRIMARY_PIPE : DNXF_TDM_PRIORITY_OVER_SECONDARY_PIPE));
        DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, i, reg_val32));
    }


exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_fe1600_set_fmac_config(int unit) {

    uint32 reg_val32;
    int i;
    int link, blk, inner_link;
    soc_dnxf_fabric_link_device_mode_t link_mode;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*FMAC Leaky bucket configuration*/
    DNXC_IF_ERR_EXIT(READ_FMAC_LEAKY_BUCKET_CONTROL_REGISTERr(unit, 0, &reg_val32));
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_FILL_RATEf, SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_LINK_UP_THf, 0x20);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_LINK_DN_THf, 0x10);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, SIG_DET_BKT_RST_ENAf, 0x1);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, ALIGN_LCK_BKT_RST_ENAf, 0x1);
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_LEAKY_BUCKET_CONTROL_REGISTERr(unit, reg_val32));
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_LEAKY_BUCKET_CONTROL_REGISTERr(unit, reg_val32));

        /*Comma burst configuration*/
    if (SOC_DNXF_IS_FE13(unit))
    {
       PBMP_SFI_ITER(unit, link)
       {
           DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_link_device_mode_get,(unit, link, 0/*tx*/, &link_mode)));
           DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_link_to_block_mapping, (unit, link, &blk, &inner_link, SOC_BLK_FMAC)));

           DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk, inner_link, &reg_val32));
           soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_BYTE_MODEf, 0x1);
           if (link_mode == soc_dnxf_fabric_link_device_mode_multi_stage_fe1)
           {
               soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE1);
               soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE1);
           } else {
               soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE3);
               soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE3);
           }
           DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk, inner_link, reg_val32));
       }
    } else {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 0, 0, &reg_val32));
        soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_BYTE_MODEf, 0x1);
        if (SOC_DNXF_IS_REPEATER(unit)) {
            soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_REPEATER);
            soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_REPEATER);
        } else {
            soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_PERIOD_FE2);
            soc_reg_field_set(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, _SOC_RAMON_FE1600_DRV_COMMA_BURST_SIZE_FE2);
        }
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 0, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 1, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 2, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 3, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 0, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 1, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 2, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FMAL_COMMA_BURST_CONFIGURATIONr(unit, 3, reg_val32));
    }
    
    if(SOC_DNXF_IS_FE13(unit)) {
        /*Set upper 64 links*/
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_REG_0065r(unit,0xf));
    }

    /*enable llfc by default*/
    reg_val32 = 0;
    soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val32, LNK_LVL_FC_RX_ENf, 0xf);
    soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val32, LNK_LVL_FC_TX_ENf, 0xf);
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, reg_val32));
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, reg_val32));

    /*enable RX_LOS_SYNC interrupt*/
    for(i=0; i<SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; i++) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FPS_CONFIGURATION_RX_SYNCr(unit, REG_PORT_ANY, i, &reg_val32));
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val32 ,FPS_N_RX_SYNC_FORCE_LCK_ENf, 0);
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val32 ,FPS_N_RX_SYNC_FORCE_SLP_ENf, 0);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FPS_CONFIGURATION_RX_SYNCr(unit, i, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FPS_CONFIGURATION_RX_SYNCr(unit, i, reg_val32));
    }
exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_fe1600_set_fsrd_config(int unit) {

    uint32 hv_disable, reg_val;
    int blk_ins, quad, global_quad;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

#ifdef BCM_88754_A0
    /*Not required for BCM88754_A0*/
    if (SOC_IS_BCM88754_A0(unit))
    {
        SOC_EXIT;
    }
#endif
    for(blk_ins=0 ; blk_ins<SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd) ; blk_ins++) { 

        for(quad=0 ; quad<SOC_RAMON_FE1600_NOF_QUADS_IN_FSRD ; quad++)
        {
            /*if quad is disabled - continue*/
            if(SOC_IS_FE1600_B0_AND_ABOVE(unit) && SOC_PBMP_MEMBER(SOC_CONTROL(unit)->info.sfi.disabled_bitmap, blk_ins*SOC_RAMON_FE1600_NOF_LINKS_IN_FSRD + quad*SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD)) {
                continue;
            }
            DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, &reg_val));
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_POWER_DOWNf, 0);
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_IDDQf, 0);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
            sal_usleep(20);

            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_HWf, 1);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
            sal_usleep(20);

            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_MDIO_REGSf, 1);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
            sal_usleep(20);

            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_PLLf, 1);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
            sal_usleep(20);

            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_FIFOf, 1);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
            sal_usleep(20);
         
            global_quad = blk_ins*SOC_RAMON_FE1600_NOF_QUADS_IN_FSRD + quad;
            hv_disable = soc_property_suffix_num_get(unit, global_quad, spn_SRD_TX_DRV_HV_DISABLE, "quad", 0);
            LOG_DEBUG(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "%s[FSRD%d, quad %d]=%d \n"),spn_SRD_TX_DRV_HV_DISABLE,blk_ins,quad,hv_disable));

            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_TX_DRV_HV_DISABLEf, hv_disable);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad, reg_val));
     
        }
    }


exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_fe1600_set_ccs_config(int unit) {
    
    int blk_ins;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    for(blk_ins=0 ; blk_ins<SOC_RAMON_FE1600_NOF_INSTANCES_CCS ; blk_ins++) {
        DNXC_IF_ERR_EXIT(READ_CCS_REG_0124r(unit, blk_ins, &reg_val));       
        soc_reg_field_set(unit, CCS_REG_0124r, &reg_val, FIELD_0_6f, 0x34);
        soc_reg_field_set(unit, CCS_REG_0124r, &reg_val, FIELD_8_14f, 0x31);
        DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0124r(unit, blk_ins, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_set_mesh_topology_config(int unit) {

    uint32 reg_val32, intg, frac; 
    uint64 frac64;
    soc_reg_t mesh_topology_reg_0107;
#ifdef BCM_88790_SUPPORT
    soc_reg_above_64_val_t reg_val_above_64, field_val_above_64;
    uint32 field_zeros =0;
    int gt_size = -1;
#endif
    DNXC_INIT_FUNC_DEFS;
    /*Should be replaced by alias feature*/
#ifdef BCM_88790_SUPPORT
    if (SOC_IS_RAMON(unit))
    {
        mesh_topology_reg_0107 = MESH_TOPOLOGY_MESH_TOPOLOGY_REG_0107r;
    } else
#endif
    {
        mesh_topology_reg_0107  = MESH_TOPOLOGY_REG_0107r;
    }

    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val32));
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_5f, 0);
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, FIELD_27_27f, 1);
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val32));

    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGY_2r(unit, &reg_val32));
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGY_2r, &reg_val32, FIELD_4_17f, 2049);    
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGY_2r(unit, reg_val32));

    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_INITr(unit, &reg_val32));
    if(SOC_DNXF_IS_FE13(unit)) {
        soc_reg_field_set(unit, MESH_TOPOLOGY_INITr, &reg_val32, INITf, 10);
        soc_reg_field_set(unit, MESH_TOPOLOGY_INITr, &reg_val32, CONFIG_1f, 2);
        soc_reg_field_set(unit, MESH_TOPOLOGY_INITr, &reg_val32, CONFIG_2f, 0x0); 
    } else {
        soc_reg_field_set(unit, MESH_TOPOLOGY_INITr, &reg_val32, CONFIG_2f, 1);
    }
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_INITr(unit, reg_val32));

    intg =  SOC_DNXF_CONFIG(unit).system_ref_core_clock / SOC_DNXF_CONFIG(unit).core_clock_speed;
    COMPILER_64_SET(frac64, 0, SOC_DNXF_CONFIG(unit).system_ref_core_clock);
    COMPILER_64_SUB_32(frac64, SOC_DNXF_CONFIG(unit).core_clock_speed*intg);
    COMPILER_64_SHL(frac64, 19);
    DNXC_IF_ERR_EXIT(soc_dnxf_compiler_64_div_32(frac64, SOC_DNXF_CONFIG(unit).core_clock_speed, &frac));
    

    soc_reg32_get(unit, mesh_topology_reg_0107, REG_PORT_ANY, 0, &reg_val32);
#ifdef BCM_88790_SUPPORT
    if (SOC_IS_RAMON(unit))
    {
        soc_reg_field_set(unit, mesh_topology_reg_0107, &reg_val32, REG_107_CONFIG_1f, frac);
        soc_reg_field_set(unit, mesh_topology_reg_0107, &reg_val32, REG_107_CONFIG_2f, intg);
    }
    else 
#endif 
    {
        soc_reg_field_set(unit, mesh_topology_reg_0107, &reg_val32, FIELD_0_18f, frac);
        soc_reg_field_set(unit, mesh_topology_reg_0107, &reg_val32, FIELD_20_23f, intg);
    }
    soc_reg32_set(unit, mesh_topology_reg_0107, REG_PORT_ANY, 0, reg_val32);

    


#ifdef BCM_88790_SUPPORT
    if (!SOC_IS_FE1600(unit))
    {
        
        
        gt_size = soc_property_suffix_num_get(unit,0,spn_CUSTOM_FEATURE, "mesh_topology_size", -1);

        DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val32));
        if (gt_size == -1) {
            if (SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system) {
                soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_2f, 1);
            } else if (SOC_DNXF_CONFIG(unit).system_contains_multiple_pipe_device) {
                soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_2f, 2);
            } else {
                soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_2f, 0);
            }
        } else {
            soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_2f, gt_size);
        }
        DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val32));

        DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_REG_011Br(unit, &reg_val32));
        soc_reg_field_set(unit, MESH_TOPOLOGY_REG_011Br, &reg_val32, REG_11B_CONFIG_0f, 5); /*nof cells*/
         
        
        soc_reg_field_set(unit, MESH_TOPOLOGY_REG_011Br, &reg_val32, REG_11B_CONFIG_1f, 0xc);
        

        soc_reg_field_set(unit, MESH_TOPOLOGY_REG_011Br, &reg_val32, REG_11B_CONFIG_2f, SOC_DNXF_CONFIG(unit).mesh_topology_fast ? 1 : 0);


        DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_REG_011Br(unit, reg_val32));



        if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit))
        {
            SOC_REG_ABOVE_64_CLEAR(reg_val_above_64);
            SOC_REG_ABOVE_64_CLEAR(field_val_above_64);
            DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGY_REG_0110r(unit, reg_val_above_64));
            soc_reg_above_64_field_get(unit, MESH_TOPOLOGY_MESH_TOPOLOGY_REG_0110r, reg_val_above_64, REG_110_CONFIG_0f, field_val_above_64);
            /* currently 0-71 bits are on, need to clear bits 68-71, 32-35 */
            SOC_REG_ABOVE_64_RANGE_COPY(field_val_above_64, 32, &field_zeros, 0, 4);
            SOC_REG_ABOVE_64_RANGE_COPY(field_val_above_64, 68, &field_zeros, 0, 4);
            soc_reg_above_64_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGY_REG_0110r, reg_val_above_64, REG_110_CONFIG_0f, field_val_above_64);
            DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGY_REG_0110r(unit, reg_val_above_64));
        }


    }
#endif /*BCM_88790_SUPPORT*/

    sal_usleep(20);

    /* Enable back */
    DNXC_IF_ERR_EXIT(READ_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, &reg_val32));
    soc_reg_field_set(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, &reg_val32, RESERVED_5f, 1);
    DNXC_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_MESH_TOPOLOGYr(unit, reg_val32));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_fe1600_set_rtp_config(int unit) {

    uint32 reg_val32; 
    uint64 reg_val64, val64;
    soc_reg_above_64_val_t reg_val_above_64;
    uint32 core_clock_speed;
    uint32 rtpwp;
    uint32 wp_at_core_clock_steps;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    core_clock_speed = SOC_DNXF_CONFIG(unit).core_clock_speed;

     /*RTPWP CALC*/
    wp_at_core_clock_steps = ((SOC_RAMON_FE1600_RTP_REACHABILTY_WATCHDOG_RATE / 1000)/*micro sec*/ * core_clock_speed /*KHz*/) / 1000 /*convert micro sec to mili */;
    rtpwp = wp_at_core_clock_steps/4096;
    rtpwp = (rtpwp * 4096 < wp_at_core_clock_steps) ? rtpwp+1 : rtpwp; /*ceiling*/

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr_REG32(unit, &reg_val32));
    soc_reg_field_set(unit, RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr, &reg_val32, RTPWPf, rtpwp);
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr_REG32(unit, reg_val32));

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    if(soc_dnxf_load_balancing_mode_destination_unreachable == SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode) {
        COMPILER_64_SET(val64,0,0);
        soc_reg64_field_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, val64);
    } else {
        COMPILER_64_SET(val64,0,1);
        soc_reg64_field_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, val64);
    }
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, UPDATE_BASE_INDEXf, 0x3f); /*max base index - 2k FAP-IDs*/
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

    /*link load load-balancing*/
    DNXC_IF_ERR_EXIT(READ_RTP_DRH_LOAD_BALANCING_CONFIGr(unit, &reg_val32));
    soc_reg_field_set(unit, RTP_DRH_LOAD_BALANCING_CONFIGr, &reg_val32, LOAD_BALANCE_LEVELS_IGNOREf, 0);
    DNXC_IF_ERR_EXIT(WRITE_RTP_DRH_LOAD_BALANCING_CONFIGr(unit, reg_val32));

    if (SOC_DNXF_IS_FE13(unit)) {
        DNXC_IF_ERR_EXIT(READ_RTP_REG_0069r(unit, &reg_val32)); 
        soc_reg_field_set(unit, RTP_REG_0069r, &reg_val32, FIELD_10_10f, 1);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REG_0069r(unit, reg_val32)); 
    }



    SOC_REG_ABOVE_64_ALLONES(reg_val_above_64);
    DNXC_IF_ERR_EXIT(WRITE_RTP_LINK_BUNDLE_BITMAPr(unit, 0, reg_val_above_64));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * DCH default thresholds
 */
#define SOC_RAMON_FE1600_DCH_LLFC_TH_SINGLE_PIPE_DEF                  (100)
#define SOC_RAMON_FE1600_DCH_LLFC_TH_DUAL_PIPE_DEF                    (40)
#define SOC_RAMON_FE1600_DCH_IFM_ALL_FULL_TH_SINGLE_PIPE_DEF          (120)
#define SOC_RAMON_FE1600_DCH_IFM_ALL_FULL_TH_DUAL_PIPE_DEF            (60)
#define SOC_RAMON_FE1600_DCH_LOW_PRI_CELL_DROP_TH_SINGLE_PIPE_DEF     (64)
#define SOC_RAMON_FE1600_DCH_LOW_PRI_CELL_DROP_TH_DUAL_PIPE_DEF       (32)
#define SOC_RAMON_FE1600_DCH_FIELD_31_31_DEF                          (1)
#define SOC_RAMON_FE1600_DCH_REG_0091_FIELD_4_24_FE3_VSC256_DEF       (0x1200)
#define SOC_RAMON_FE1600_DCH_REG_0091_FIELD_4_24_FE1_FE2_VSC256_DEF   (0x400)

#define SOC_RAMON_FE1600_DCH_GCI_TH_DEF                               (129)

STATIC int
soc_ramon_fe1600_set_dch_config(int unit) {
    uint32 reg32_val;
    int port;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*
     * Link level flow control thresholds
     */

    for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) { 
        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Pr, &reg32_val, LNK_LVL_FC_TH_0_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCH_LLFC_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCH_LLFC_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Pr, &reg32_val, LNK_LVL_FC_TH_1_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCH_LLFC_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCH_LLFC_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Pr, &reg32_val, IFM_ALL_FULL_TH_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCH_IFM_ALL_FULL_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCH_IFM_ALL_FULL_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Pr, &reg32_val, LOW_PRI_CELL_DROP_TH_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCH_LOW_PRI_CELL_DROP_TH_DUAL_PIPE_DEF: SOC_RAMON_FE1600_DCH_LOW_PRI_CELL_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Pr, &reg32_val, FIELD_31_31f, SOC_RAMON_FE1600_DCH_FIELD_31_31_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_Pr(unit, port, reg32_val));
    }

    for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) {
        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Sr, &reg32_val, LNK_LVL_FC_TH_0_Sf, SOC_RAMON_FE1600_DCH_LLFC_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Sr, &reg32_val, LNK_LVL_FC_TH_1_Sf, SOC_RAMON_FE1600_DCH_LLFC_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Sr, &reg32_val, IFM_ALL_FULL_TH_Sf, SOC_RAMON_FE1600_DCH_IFM_ALL_FULL_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Sr, &reg32_val, LOW_PRI_CELL_DROP_TH_Sf, SOC_RAMON_FE1600_DCH_LOW_PRI_CELL_DROP_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_Sr, &reg32_val, FIELD_31_31f, SOC_RAMON_FE1600_DCH_FIELD_31_31_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_Sr(unit, port, reg32_val));
    }

    /*ALUWP Config*/
    for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) {
        DNXC_IF_ERR_EXIT(READ_DCH_REG_0061r(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_REG_0061r, &reg32_val, FIELD_0_7f, 0Xfd);
        DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0061r(unit, port, reg32_val));
    }

    /*
     * GCI thresholds
     */
    for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) { 
        DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, &reg32_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, &reg32_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, &reg32_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        /*disable*/
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, &reg32_val, DCH_LOCAL_GCI_EN_Pf, 0);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, &reg32_val, DCH_LOCAL_MCI_EN_Pf, 0);
        DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, &reg32_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, &reg32_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, &reg32_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        /*disable*/
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, &reg32_val, DCH_LOCAL_GCI_EN_Sf, 0);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, &reg32_val, DCH_LOCAL_MCI_EN_Sf, 0);
        DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, &reg32_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, &reg32_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, &reg32_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Pf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, &reg32_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, &reg32_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, &reg32_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Sf, SOC_RAMON_FE1600_DCH_GCI_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr(unit, port, reg32_val));
    }

    /*Internal thresholds*/
    if (SOC_DNXF_IS_REPEATER(unit)) {
        for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) {    
            DNXC_IF_ERR_EXIT(READ_DCH_REG_0091r(unit, port, &reg32_val));
            soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_0_0f, 0X0); /*disable internal threshold*/
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0091r(unit, port, reg32_val));
        }

    } else if (SOC_DNXF_IS_FE2(unit)) {
        for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) {
            DNXC_IF_ERR_EXIT(READ_DCH_REG_0091r(unit, port, &reg32_val));
            soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_0_0f, 0X1); /*enable internal threshold*/
            soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_4_22f, SOC_RAMON_FE1600_DCH_REG_0091_FIELD_4_24_FE1_FE2_VSC256_DEF);
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0091r(unit, port, reg32_val));
        }
    } else if (SOC_DNXF_IS_FE13(unit)) {
        /*FE1*/
        for (port=0 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch)/2; port++) { /*FE1 threshold*/
            DNXC_IF_ERR_EXIT(READ_DCH_REG_0091r(unit, port, &reg32_val));
            soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_0_0f, 0X1); /*enable internal threshold*/
            soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_4_22f, SOC_RAMON_FE1600_DCH_REG_0091_FIELD_4_24_FE1_FE2_VSC256_DEF);
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0091r(unit, port, reg32_val));
        }
        /*FE3*/
        if (SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system)
        {
            for (port=SOC_DNXF_DEFS_GET(unit, nof_instances_dch)/2 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) { /*FE3 threshold*/
                DNXC_IF_ERR_EXIT(READ_DCH_REG_0091r(unit, port, &reg32_val));
                soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_0_0f, 0X0); /*disable internal threshold*/
                DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0091r(unit, port, reg32_val));
            }
        } else {
            for (port=SOC_DNXF_DEFS_GET(unit, nof_instances_dch)/2 ; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); port++) { /*FE3 threshold*/
                DNXC_IF_ERR_EXIT(READ_DCH_REG_0091r(unit, port, &reg32_val));
                soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_0_0f, 0X1); /*enable internal threshold*/
                soc_reg_field_set(unit, DCH_REG_0091r, &reg32_val, FIELD_4_22f, SOC_RAMON_FE1600_DCH_REG_0091_FIELD_4_24_FE3_VSC256_DEF);
                DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0091r(unit, port, reg32_val));
            }
        }
    }

       
exit:
    DNXC_FUNC_RETURN;
}

/*
 * DCM default thresholds
 */
#define SOC_RAMON_FE1600_DCM_0_DROP_TH_SINGLE_PIPE_DEF                    (280)
#define SOC_RAMON_FE1600_DCM_1_DROP_TH_SINGLE_PIPE_DEF                    (290)
#define SOC_RAMON_FE1600_DCM_2_DROP_TH_SINGLE_PIPE_DEF                    (300)
#define SOC_RAMON_FE1600_DCM_3_DROP_TH_SINGLE_PIPE_DEF                    (360)

#define SOC_RAMON_FE1600_DCM_0_DROP_TH_DUAL_PIPE_DEF                      (100)
#define SOC_RAMON_FE1600_DCM_1_DROP_TH_DUAL_PIPE_DEF                      (110)
#define SOC_RAMON_FE1600_DCM_2_DROP_TH_DUAL_PIPE_DEF                      (120)
#define SOC_RAMON_FE1600_DCM_3_DROP_TH_DUAL_PIPE_DEF                      (180)

#define SOC_RAMON_FE1600_DCM_ALM_FULL_DUAL_PIPE_DEF                       (120)
#define SOC_RAMON_FE1600_DCM_ALM_FULL_SINGLE_PIPE_DEF                     (320)
#define SOC_RAMON_FE1600_DCM_FULL_TH_DUAL_PIPE_DEF                        (180)
#define SOC_RAMON_FE1600_DCM_FULL_TH_SINGLE_PIPE_DEF                      (360)

#define SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF                         (128)
#define SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF                       (256)


STATIC int
soc_ramon_fe1600_set_dcm_config(int unit) {
    uint64 reg64_val;
    uint32 reg32_val;
    int port;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*DCMA configuration*/
    /*
     * Priority drop default thresholds
     */
    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr(unit, port, &reg64_val));
        soc_reg64_field32_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUA_P_0_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_0_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_0_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUA_P_1_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_1_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_1_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUA_P_2_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_2_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_2_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUA_P_3_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_3_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_3_DROP_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr(unit, port, reg64_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr(unit, port, &reg64_val));
        soc_reg64_field32_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMA_P_0_DROP_THf, SOC_RAMON_FE1600_DCM_0_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMA_P_1_DROP_THf, SOC_RAMON_FE1600_DCM_1_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMA_P_2_DROP_THf, SOC_RAMON_FE1600_DCM_2_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMA_P_3_DROP_THf, SOC_RAMON_FE1600_DCM_3_DROP_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr(unit, port, reg64_val));
    }

    /*
     * Full/Almost full default thresholds
     */
    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMU_ALM_FULL_0r(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMA_DCMU_ALM_FULL_0r, &reg32_val, DCMUA_ALM_FULLf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_ALM_FULL_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_ALM_FULL_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMU_ALM_FULL_0r, &reg32_val, DCMU_FULL_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_FULL_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_FULL_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMU_ALM_FULL_0r(unit, port, reg32_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMM_ALM_FULL_0r(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMA_DCMM_ALM_FULL_0r, &reg32_val, DCMMA_ALM_FULLf, SOC_RAMON_FE1600_DCM_ALM_FULL_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMM_ALM_FULL_0r, &reg32_val, DCMM_FULL_THf, SOC_RAMON_FE1600_DCM_FULL_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMM_ALM_FULL_0r(unit, port, reg32_val));
    }

    /*
     * GCI default thresholds
     */

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMUA_GCI_THr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMA_DCMUA_GCI_THr, &reg32_val, GCI_TH_LOWf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMUA_GCI_THr, &reg32_val, GCI_TH_MEDf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMUA_GCI_THr, &reg32_val, GCI_TH_HIGHf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMUA_GCI_THr(unit, port, reg32_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMA; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMMA_GCI_THr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMA_DCMMA_GCI_THr, &reg32_val, GCI_TH_LOWf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMMA_GCI_THr, &reg32_val, GCI_TH_MEDf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMA_DCMMA_GCI_THr, &reg32_val, GCI_TH_HIGHf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMMA_GCI_THr(unit, port, reg32_val));
    }


    /*DCMB configuration*/
    /*
     * Priority drop default thresholds
     */
    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr(unit, port, &reg64_val));
        soc_reg64_field32_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUB_P_0_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_0_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_0_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUB_P_1_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_1_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_1_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUB_P_2_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_2_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_2_DROP_TH_SINGLE_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMUB_P_3_DROP_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_3_DROP_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_3_DROP_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr(unit, port, reg64_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr(unit, port, &reg64_val));
        soc_reg64_field32_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMB_P_0_DROP_THf, SOC_RAMON_FE1600_DCM_0_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMB_P_1_DROP_THf, SOC_RAMON_FE1600_DCM_1_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMB_P_2_DROP_THf, SOC_RAMON_FE1600_DCM_2_DROP_TH_DUAL_PIPE_DEF);
        soc_reg64_field32_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, &reg64_val, DCMMB_P_3_DROP_THf, SOC_RAMON_FE1600_DCM_3_DROP_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr(unit, port, reg64_val));
    }

    /*
     * Full/Almost full default thresholds
     */
    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMUBLM_FULL_0r(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMB_DCMUBLM_FULL_0r, &reg32_val, DCMUB_ALM_FULLf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_ALM_FULL_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_ALM_FULL_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMUBLM_FULL_0r, &reg32_val, DCMU_FULL_THf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_FULL_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_FULL_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMUBLM_FULL_0r(unit, port, reg32_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMMBLM_FULL_0r(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMB_DCMMBLM_FULL_0r, &reg32_val, DCMMB_ALM_FULLf, SOC_RAMON_FE1600_DCM_ALM_FULL_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMMBLM_FULL_0r, &reg32_val, DCMM_FULL_THf, SOC_RAMON_FE1600_DCM_FULL_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMMBLM_FULL_0r(unit, port, reg32_val));
    }

    /*
     * GCI default thresholds
     */

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMUB_GCI_THr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMB_DCMUB_GCI_THr, &reg32_val, GCI_TH_LOWf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMUB_GCI_THr, &reg32_val, GCI_TH_MEDf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMUB_GCI_THr, &reg32_val, GCI_TH_HIGHf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCM_GCI_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMUB_GCI_THr(unit, port, reg32_val));
    }

    for (port=0 ; port < SOC_RAMON_FE1600_NOF_INSTANCES_DCMB; port++) {
        DNXC_IF_ERR_EXIT(READ_DCMB_DCMMB_GCI_THr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCMB_DCMMB_GCI_THr, &reg32_val, GCI_TH_LOWf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMMB_GCI_THr, &reg32_val, GCI_TH_MEDf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCMB_DCMMB_GCI_THr, &reg32_val, GCI_TH_HIGHf, SOC_RAMON_FE1600_DCM_GCI_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMMB_GCI_THr(unit, port, reg32_val));
    }

     

exit:
    DNXC_FUNC_RETURN;
}

/*
 * DCL thresholds  
 */

#define SOC_RAMON_FE1600_DCL_U_LLFC_TH_DEF                                    (322)
#define SOC_RAMON_FE1600_DCL_M_LLFC_TH_DEF                                    (194)

#define SOC_RAMON_FE1600_DCL_P_ALM_FULL_TH_DEF                                (322)
#define SOC_RAMON_FE1600_DCL_S_ALM_FULL_TH_DEF                                (194)

#define SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_SINGLE_PIPE_TH_DEF                  (180)
#define SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_SINGLE_PIPE_TH_DEF                  (220)
#define SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_SINGLE_PIPE_TH_DEF                 (260)
#define SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_DUAL_PIPE_TH_DEF                    (80)
#define SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_DUAL_PIPE_TH_DEF                    (100)
#define SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_DUAL_PIPE_TH_DEF                   (120)
#define SOC_RAMON_FE1600_DCL_GCI_LOW_TH_S_TH_DEF                              (100)
#define SOC_RAMON_FE1600_DCL_GCI_MED_TH_S_TH_DEF                              (120)
#define SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_S_TH_DEF                             (140)

#define SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_SINGLE_PIPE_DEF                      (200)
#define SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_DUAL_PIPE_DEF                        (110)

#define SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_SINGLE_PIPE_DEF                     (320)
#define SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_P_DUAL_PIPE_DEF                     (128)
#define SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_S_DUAL_PIPE_DEF                     (192)

STATIC int
soc_ramon_fe1600_set_dcl_config(int unit) {
    uint32 reg32_val;
    uint64 reg64_val, val64;
    int port;
    DNXC_INIT_FUNC_DEFS;

    SOC_RAMON_FE1600_ONLY(unit);

    /* 
     *LLFC thresholds
     */
    for (port=0; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dcl); port++) {
        DNXC_IF_ERR_EXIT(READ_DCL_DCL_LLFC_THr(unit, port, &reg64_val));
        COMPILER_64_SET(val64,0, SOC_RAMON_FE1600_DCL_U_LLFC_TH_DEF);
        soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg64_val, DCLU_LLFC_TH_TYPE_0f, val64);
        soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg64_val, DCLU_LLFC_TH_TYPE_1f, val64);
        COMPILER_64_SET(val64,0, SOC_RAMON_FE1600_DCL_M_LLFC_TH_DEF);
        soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg64_val, DCLM_LLFC_TH_TYPE_0f, val64);
        soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg64_val, DCLM_LLFC_TH_TYPE_1f, val64);
        DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_LLFC_THr(unit, port, reg64_val));
    }

    /*
     *Almost full threholds
     */

    for (port=0; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dcl); port++) {
        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_ALM_FULL_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_0_ALM_FULL_Pr, &reg32_val, TYPE_0_ALM_FULL_Pf, SOC_RAMON_FE1600_DCL_P_ALM_FULL_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_ALM_FULL_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_ALM_FULL_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_1_ALM_FULL_Pr, &reg32_val, TYPE_1_ALM_FULL_Pf, SOC_RAMON_FE1600_DCL_P_ALM_FULL_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_ALM_FULL_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_ALM_FULL_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_0_ALM_FULL_Sr, &reg32_val, TYPE_0_ALM_FULL_Sf, SOC_RAMON_FE1600_DCL_S_ALM_FULL_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_ALM_FULL_Sr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_ALM_FULL_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_1_ALM_FULL_Sr, &reg32_val, TYPE_1_ALM_FULL_Sf, SOC_RAMON_FE1600_DCL_S_ALM_FULL_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_ALM_FULL_Sr(unit, port, reg32_val));
    }

    /*
     * GCI thresholds
     */

    for (port=0; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dcl); port++) { 
        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg32_val, GCI_TH_LOW_0_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_SINGLE_PIPE_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg32_val, GCI_TH_MED_0_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_SINGLE_PIPE_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg32_val, GCI_TH_HIGH_0_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_SINGLE_PIPE_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_GCI_TH_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg32_val, GCI_TH_LOW_1_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_LOW_TH_P_SINGLE_PIPE_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg32_val, GCI_TH_MED_1_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_MED_TH_P_SINGLE_PIPE_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg32_val, GCI_TH_HIGH_1_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_DUAL_PIPE_TH_DEF : SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_P_SINGLE_PIPE_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_GCI_TH_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg32_val, GCI_TH_LOW_0_Sf, SOC_RAMON_FE1600_DCL_GCI_LOW_TH_S_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg32_val, GCI_TH_MED_0_Sf, SOC_RAMON_FE1600_DCL_GCI_MED_TH_S_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg32_val, GCI_TH_HIGH_0_Sf,SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_S_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_GCI_TH_Sr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg32_val, GCI_TH_LOW_1_Sf, SOC_RAMON_FE1600_DCL_GCI_LOW_TH_S_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg32_val, GCI_TH_MED_1_Sf, SOC_RAMON_FE1600_DCL_GCI_MED_TH_S_TH_DEF);
        soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg32_val, GCI_TH_HIGH_1_Sf,SOC_RAMON_FE1600_DCL_GCI_HIGH_TH_S_TH_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_GCI_TH_Sr(unit, port, reg32_val));
    }

    /*
     * RCI thresholds  
     */
    for (port=0; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dcl); port++) {
        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Pr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Pr, &reg32_val, RCI_TH_HIGH_0_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_SINGLE_PIPE_DEF);
        soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Pr, &reg32_val, RCI_TH_HIGH_1_Pf, SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_SINGLE_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_01_RCI_TH_Pr(unit, port, reg32_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Sr(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Sr, &reg32_val, RCI_TH_HIGH_0_Sf, SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_DUAL_PIPE_DEF);
        soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Sr, &reg32_val, RCI_TH_HIGH_1_Sf, SOC_RAMON_FE1600_DCL_RCI_HIGH_TH_DUAL_PIPE_DEF);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_01_RCI_TH_Sr(unit, port, reg32_val));
    }

    /*
     * Priority drop threholds
     */
    for (port=0; port < SOC_DNXF_DEFS_GET(unit, nof_instances_dcl); port++) {
        COMPILER_64_SET(val64,0,SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_P_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_SINGLE_PIPE_DEF);

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PPr(unit, port, &reg64_val));
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg64_val, TYPE_0_DRP_P_0_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg64_val, TYPE_0_DRP_P_1_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg64_val, TYPE_0_DRP_P_2_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg64_val, TYPE_0_DRP_P_3_Pf, val64);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_DRP_PPr(unit, port, reg64_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PPr(unit, port, &reg64_val));
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg64_val, TYPE_1_DRP_P_0_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg64_val, TYPE_1_DRP_P_1_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg64_val, TYPE_1_DRP_P_2_Pf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg64_val, TYPE_1_DRP_P_3_Pf, val64);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_DRP_PPr(unit, port, reg64_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PSr(unit, port, &reg64_val));
        COMPILER_64_SET(val64,0,SOC_DNXF_CONFIG(unit).is_dual_mode ? SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_S_DUAL_PIPE_DEF : SOC_RAMON_FE1600_DCL_DROP_PRIO_TH_SINGLE_PIPE_DEF);

        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg64_val, TYPE_0_DRP_P_0_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg64_val, TYPE_0_DRP_P_1_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg64_val, TYPE_0_DRP_P_2_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg64_val, TYPE_0_DRP_P_3_Sf, val64);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_DRP_PSr(unit, port, reg64_val));

        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PSr(unit, port, &reg64_val));

        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg64_val, TYPE_1_DRP_P_0_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg64_val, TYPE_1_DRP_P_1_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg64_val, TYPE_1_DRP_P_2_Sf, val64);
        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg64_val, TYPE_1_DRP_P_3_Sf, val64);
        DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_DRP_PSr(unit, port, reg64_val));


        DNXC_IF_ERR_EXIT(READ_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, port, &reg32_val));
        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, &reg32_val, LOW_PR_DROP_EN_Pf, 0xf);
        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, &reg32_val, LOW_PR_DROP_EN_Sf, 0xf);
        DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, port, reg32_val));
    }


exit:
    DNXC_FUNC_RETURN;
}


/*
 * Returns TRUE if the memory is dynamic
 */
int soc_ramon_fe1600_tbl_is_dynamic(int unit, soc_mem_t mem) {

    switch (mem) {        
    case RTP_DLLUPm:
    case RTP_DLLUSm:
    case RTP_MEM_1100000m:
    case RTP_MEM_800000m:
    case RTP_MEM_900000m:
        return TRUE;
    case RTP_DUCTPm:
    case RTP_DUCTSm:
    case RTP_CUCTm:
    case RTP_RMHMTm:
        if(!SOC_DNXF_IS_REPEATER(unit)) {
            return TRUE;
        } else {
            return FALSE;
        }
    case RTP_SLSCTm:
        if (soc_dnxf_load_balancing_mode_destination_unreachable != SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode) {
            return TRUE;
        } else {
            return FALSE;
        }

    default:
        return FALSE;
    }
}

/*
 * ramon_fe1600 interrupt initialization
 */
STATIC int
soc_ramon_fe1600_interrupt_disable(int unit)
{
    uint32 inst_idx;
    soc_block_types_t  block;
    int blk;
    int instance;
    soc_reg_above_64_val_t above_64;
    int rc;

    DNXC_INIT_FUNC_DEFS;

    /* mask all interrupts */
    SOC_REG_ABOVE_64_CLEAR(above_64);
    for(inst_idx=0; ramon_fe1600_interrupts_mask_registers[inst_idx] != INVALIDr; ++inst_idx) {
        if (SOC_REG_PTR(unit, ramon_fe1600_interrupts_mask_registers[inst_idx])== NULL)
        {
            continue;
        }
        block = SOC_REG_INFO(unit, ramon_fe1600_interrupts_mask_registers[inst_idx]).block;
        SOC_BLOCKS_ITER(unit, blk, block) {
            instance = SOC_BLOCK_NUMBER(unit, blk);
            rc = soc_reg_above_64_set(unit, ramon_fe1600_interrupts_mask_registers[inst_idx], instance, 0, above_64);
            DNXC_IF_ERR_EXIT(rc);
        }
    }

    /* Enable ECI interrupt masking (since there is double masking towards the CMIC) */
    rc = WRITE_ECI_INTERRUPT_MASK_REGISTERr(unit, 0xffffffff);
    DNXC_IF_ERR_EXIT(rc);
    rc = WRITE_ECI_MAC_INTERRUPT_MASK_REGISTERr(unit, 0xffffffff);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * ramon_fe1600 SER initialization
 */
int
soc_ramon_fe1600_ser_init(int unit)
{
    uint32 inst_idx;
    soc_block_types_t  block;
    int blk;
    int instance;
    soc_reg_above_64_val_t above_64;
    int rc;

    DNXC_INIT_FUNC_DEFS;

    /* unmask SER monitor registers 64*/
    SOC_REG_ABOVE_64_ALLONES(above_64);
    for(inst_idx=0; ramon_fe1600_interrupt_monitor_mem_reg[inst_idx] != INVALIDr; inst_idx++) {
        block = SOC_REG_INFO(unit, ramon_fe1600_interrupt_monitor_mem_reg[inst_idx]).block;
        SOC_BLOCKS_ITER(unit, blk, block) {
            instance = SOC_BLOCK_NUMBER(unit, blk);
            rc = soc_reg_above_64_set(unit, ramon_fe1600_interrupt_monitor_mem_reg[inst_idx], instance, 0, above_64);
            DNXC_IF_ERR_EXIT(rc);
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_reset_device(int unit)
{
    uint64 reg_val64;
    uint32 reg_val32, i;
    soc_reg_above_64_val_t reg_val_above_64;
    int rc;
    uint32 ser_test_iters = soc_property_suffix_num_get_only_suffix(unit, -1, spn_BIST_ENABLE, "set_test_iters_num", 0);
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
    
    rc =soc_ramon_fe1600_reset_cmic_regs(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*Soft reset*/
    COMPILER_64_SET(reg_val64, 0x7, 0xFFFFFFFF);
    DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_RESETr(unit, reg_val64));
    sal_sleep(1);
    COMPILER_64_SET(reg_val64, 0x0, 0x0);
    DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_RESETr(unit,reg_val64));
    reg_val32 = 0x1ff;
    DNXC_IF_ERR_EXIT(WRITE_ECI_SB_RSTN_AND_POWER_DOWNr(unit,reg_val32));
    sal_usleep(20);

    rc = soc_ramon_fe1600_set_operation_mode(unit);
    DNXC_IF_ERR_EXIT(rc);
    if (ser_test_iters) { /* Perform SER testing if configured to do so */
        uint32 time_to_wait = soc_property_suffix_num_get_only_suffix(unit, -1, spn_BIST_ENABLE, "ser_test_delay_sec", 0);
        if (time_to_wait == 0) { /* if both sec and ms are specified, sec is used */
            time_to_wait = soc_property_suffix_num_get_only_suffix(unit, -1, spn_BIST_ENABLE, "ser_test_delay_us", 0);
            if (time_to_wait == 0) {
                time_to_wait = 3600 | DNXC_MBIST_TEST_LONG_WAIT_DELAY_IS_SEC; /* default of one hour if not specified */
            }
        } else {
            time_to_wait |= DNXC_MBIST_TEST_LONG_WAIT_DELAY_IS_SEC;
        }
        DNXC_IF_ERR_EXIT(soc_bist_ramon_fe1600_ser_test(unit, 1, ser_test_iters, time_to_wait));
    }

    /* perform MBIST if configured to do so */
    if (SOC_DNXF_CONFIG(unit).run_mbist) {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_mbist(unit, SOC_DNXF_CONFIG(unit).run_mbist - 1));

        /*Rerun Soft reset*/
        COMPILER_64_SET(reg_val64, 0x7, 0xFFFFFFFF);
        DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_RESETr(unit, reg_val64));
        sal_sleep(1);
        COMPILER_64_SET(reg_val64, 0x0, 0x0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_RESETr(unit,reg_val64));
        reg_val32 = 0x1ff;
        DNXC_IF_ERR_EXIT(WRITE_ECI_SB_RSTN_AND_POWER_DOWNr(unit,reg_val32));
        sal_usleep(20);

        rc = soc_ramon_fe1600_set_operation_mode(unit);
        DNXC_IF_ERR_EXIT(rc);
    }
    
    /* Init broadcast 60 - FSRD */
#ifdef BCM_88754_A0
    if (!SOC_IS_BCM88754_A0(unit))
    {
#endif
        for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd) ; i++) { 
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SBUS_BROADCAST_IDr(unit, i, 60));
        }
#ifdef BCM_88754_A0
    }
#endif

    rc = soc_ramon_fe1600_reset_tables(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_fmac_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_fsrd_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_ccs_config(unit);
    DNXC_IF_ERR_EXIT(rc); 
    rc = soc_ramon_fe1600_set_mesh_topology_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_rtp_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_dch_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_dcm_config(unit);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_ramon_fe1600_set_dcl_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*additional configurations*/
    for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_instances_dch) ; i++) {

        DNXC_IF_ERR_EXIT(READ_DCH_REG_0060r(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_REG_0060r, &reg_val32, FIELD_21_24f, 0x4);
        DNXC_IF_ERR_EXIT(WRITE_DCH_REG_0060r(unit, i, reg_val32));

        SOC_REG_ABOVE_64_CLEAR(reg_val_above_64);
        reg_val_above_64[3] = 0x00180000;
        DNXC_IF_ERR_EXIT(WRITE_DCH_ERROR_FILTERr(unit, i, reg_val_above_64));

        SOC_REG_ABOVE_64_ALLONES(reg_val_above_64);
        reg_val_above_64[3] = 0xffe7ffff;
        DNXC_IF_ERR_EXIT(WRITE_DCH_ERROR_FILTER_MASKr(unit, i, reg_val_above_64));

        DNXC_IF_ERR_EXIT(WRITE_DCH_ERROR_FILTER_MASK_ENr(unit, i, 1));
    }

    /*Set default low priority drop select*/
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_multicast_low_prio_drop_select_priority_set,(unit, soc_dnxf_fabric_priority_0));
    DNXC_IF_ERR_EXIT(rc);

    /*Enable Fe1600_B0 bug fixes*/
#ifdef BCM_88790_A0
    if (SOC_IS_FE1600_B0_AND_ABOVE(unit)) {
        rc = soc_ramon_fe1600_bo_hw_bug_fixes_enable(unit, 1);
        DNXC_IF_ERR_EXIT(rc);
    }
#endif

    /*Reduced Fe1600 Support*/
    if (SOC_IS_FE1600_REDUCED(unit)) {
        rc = soc_ramon_fe1600_reduced_support_set(unit);
        DNXC_IF_ERR_EXIT(rc);
    }

    rc = soc_ramon_fe1600_interrupt_init(unit);
    DNXC_IF_ERR_EXIT(rc);

    rc = soc_ramon_fe1600_interrupt_disable(unit);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_drv_soft_init
 * Purpose:
 *      Run blocks soft init
 * Parameters:
 *      unit  -                     (IN)     Unit number.
 *      soft_reset_mode_flags -     (IN)     ignored
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_XXX      Error occurred  
 */
int
soc_ramon_fe1600_drv_soft_init(int unit, uint32 soft_reset_mode_flags)
{
    uint64 reg_val64;
    uint32 reg32_val;
    int blk, i;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*avoid half cells*/
    for (blk = 0; blk < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); blk++) {  
        for(i=0; i<SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; i++) {
            DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, i, &reg32_val));
            soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg32_val ,FIELD_8_8f,  1);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, i, reg32_val));        
        }
    }

    /*soft reset*/
    COMPILER_64_SET(reg_val64, 0x7, 0xFFFFFFFF);
    DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_INITr(unit, reg_val64));
    
    /*release soft reset*/
    COMPILER_64_SET(reg_val64, 0x0, 0x0);
    DNXC_IF_ERR_EXIT(WRITE_ECI_FE_1600_SOFT_INITr(unit,reg_val64));

    for (blk = 0; blk < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); blk++) { 
        for(i=0; i<SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; i++) {
            DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, i, &reg32_val));
            soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg32_val ,FIELD_8_8f,  0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, i, reg32_val));        
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


int
soc_ramon_fe1600_TDM_fragment_validate(int unit, uint32 tdm_frag) 
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(SOC_FAILURE(soc_reg_field_validate(unit, ECI_GLOBAL_1r, PETRA_TDM_FRAG_NUMf, tdm_frag))) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("TDM_FRAGMENT: %d is out-of-ranget"), tdm_frag));
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
soc_ramon_fe1600_nof_block_instances(int unit, soc_block_types_t block_types, int *nof_block_instances) 
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    DNXC_NULL_CHECK(nof_block_instances);
    DNXC_NULL_CHECK(block_types);

    switch(block_types[0]) {
        case SOC_BLK_FMAC:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_mac); 
            break;
        case SOC_BLK_FSRD:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd);
            break;
        case SOC_BLK_DCH:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_dch);
            break;
        case SOC_BLK_CCS:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_CCS;
            break;
        case SOC_BLK_DCL:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl);
            break;
        case SOC_BLK_RTP:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_RTP;
            break;
        case SOC_BLK_OCCG:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_OCCG;
            break;
        case SOC_BLK_ECI:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_ECI;
            break;
        case SOC_BLK_DCMA:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_DCMA;
            break;
        case SOC_BLK_DCMB:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_DCMB;
            break;
        case SOC_BLK_DCMC:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_DCMC;
            break;
        case SOC_BLK_CMIC:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_CMIC;
            break;
        case SOC_BLK_MESH_TOPOLOGY:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_MESH_TOPOLOGY;
            break;
        case SOC_BLK_OTPC:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_OTPC;
            break;
        case SOC_BLK_BRDC_FMACH:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_brdc_fmach); 
            break;
        case SOC_BLK_BRDC_FMACL:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_BRDC_FMACL;
            break;
        case SOC_BLK_BRDC_FSRD:
            *nof_block_instances = SOC_RAMON_FE1600_NOF_INSTANCES_BRDC_FSRD;
            break;

        default:
            *nof_block_instances = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_temperature_monitor_get(int unit, int temperature_max, soc_switch_temperature_monitor_t *temperature_array, int *temperature_count)
{
    int i, rv;
    uint32 reg32_val;
    int curr;
    soc_reg_t temp_reg[] = {CMIC_THERMAL_MON_RESULT_0r, CMIC_THERMAL_MON_RESULT_1r, CMIC_THERMAL_MON_RESULT_2r, CMIC_THERMAL_MON_RESULT_3r};
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    
    
    if (temperature_max < _SOC_RAMON_FE1600_PVT_MON_NOF)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Array size should be equal or bigger from %d.\n"), _SOC_RAMON_FE1600_PVT_MON_NOF));
    }

    for (i = 0; i < _SOC_RAMON_FE1600_PVT_MON_NOF; i++)
    {
        rv = soc_pci_getreg(unit, soc_reg_addr(unit, temp_reg[i], REG_PORT_ANY, 0), &reg32_val);
        DNXC_IF_ERR_EXIT(rv);

        curr = soc_reg_field_get(unit, temp_reg[i], reg32_val, TEMP_DATAf);
        /*curr [0.1 C] = 428.39 - curr * 5.66*/
        temperature_array[i].curr =  (_SOC_RAMON_FE1600_PVT_BASE - curr * _SOC_RAMON_FE1600_PVT_FACTOR) / 1000;
        temperature_array[i].peak = -1;
    }

    *temperature_count = _SOC_RAMON_FE1600_PVT_MON_NOF;
        
exit:
    DNXC_FUNC_RETURN;
}

/*
 * AVS read
 */

STATIC int 
soc_ramon_fe1600_read_otp_mem_word(int unit, int addr, uint32* rdata)
{

    uint32 data[20];

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    data[0] = addr;
    /* Otpc CPUAddress Register*/
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));

    data[0] = 0x00a80001;
    /* This register is used to program the OTP commands through the CPU interface */
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

    data[0] = 0x00e00000;
    /* This register is used to program the OTP commands through the CPU interface.*/
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
   
    sal_usleep(20000);
    /* The CPU status register provides all the necessary information indicating the success or failure of the executed command. */   
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));
    /* This register is used to capture 32bit data corresponding to the Address field in the OTPC_CNTRL Register. */
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x00080210, 1, rdata)); 

    data[0] = 0x00a80000;
    /* This register is used to program the OTP commands through the CPU interface */
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));  

exit:
    DNXC_FUNC_RETURN;
}


STATIC int
soc_ramon_fe1600_otp_prog_en(int unit)
{

    uint32 data[20];

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    data[0] = 0x00000001;
    /* Otpc Mode Register, indicatethat the OTP is programmable by CPUinterface. */
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080200, 1, data));

    data[0] = 0x00000000;
    /* set Otpc CPUAddress Register to 0*/
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));

    data[0] = 0x00280001;
    /* This register is used to program the OTP commands through the CPU interface. */
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

    data[0] = 0x00e00000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x00080210, 1, data));

    data[0] = 0x00280000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));
  
    data[0] = 0x0000000f;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));

    data[0] = 0x00000000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x0008022c, 1, data));

    data[0] = 0x00280003;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

    data[0] = 0x00e00000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));

    data[0] = 0x00280002;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));
  
    data[0] = 0x00000004;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));

    data[0] = 0x00000000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x0008022c, 1, data));

    data[0] = 0x00280003;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

    data[0] = 0x00e00000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));

    data[0] = 0x00280002;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));
  
    data[0] = 0x00000008;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));

    data[0] = 0x00000000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x0008022c, 1, data));

    data[0] = 0x00280003;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

    data[0] = 0x00e00000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));

    data[0] = 0x00280002;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));
    
    data[0] = 0x0000000d;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080228, 1, data));
    data[0] = 0x00000000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x0008022c, 1, data));
    data[0] = 0x00280003;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));
    data[0] = 0x00e00000;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080204, 1, data));
    DNXC_IF_ERR_EXIT(soc_direct_reg_get(unit, 0x3f, 0x0008020c, 1, data));
    data[0] = 0x00280002;
    DNXC_IF_ERR_EXIT(soc_direct_reg_set(unit, 0x3f, 0x00080208, 1, data));

exit:
    DNXC_FUNC_RETURN;
}


int
soc_ramon_fe1600_avs_value_get(int unit, uint32* avs_val)
{
    uint32 rdata;
    int    raddr;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_otp_prog_en(unit));  
    
    /* read avs status from otp (bits 330-332, entry 10, bits 10-12). */
    raddr = 10 * 0x20;  
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_read_otp_mem_word(unit, raddr, &rdata));

    *avs_val=0;
    SHR_BITCOPY_RANGE(avs_val, 0, &rdata, 10, 3);

exit:
    DNXC_FUNC_RETURN;

}

#ifdef BCM_88790_A0
int
soc_ramon_fe1600_bo_hw_bug_fixes_enable(int unit, int enable)
{
    uint32 reg32_val;
    int i;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    for(i=0; i< SOC_DNXF_DEFS_GET(unit, nof_instances_dch); i++) {
        DNXC_IF_ERR_EXIT(READ_DCH_FE_1600_B_0_ENABLERSr(unit, i, &reg32_val));

        /*Enable RAMON_FE1600 B0 bug fix of fragment number translation in multistage systems with petraa*/
        soc_reg_field_set(unit, DCH_FE_1600_B_0_ENABLERSr, &reg32_val ,PETRA_A_SYS_FIX_ENABLEf, enable ? 1 : 0);

        /*Bug fix of multistage systems in mixed dual pipe where TDM traffic is going through primary pipe as well as secondary.*/
        if (SOC_DNXF_IS_MULTISTAGE_FE2(unit) || (SOC_DNXF_IS_FE13(unit) && i>=2/*DCH2 or DCH3*/)) {
            soc_reg_field_set(unit, DCH_FE_1600_B_0_ENABLERSr, &reg32_val ,PETRA_TDM_OVER_PRIMARY_ENf, enable && SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe ? 1 : 0);
        }

        DNXC_IF_ERR_EXIT(WRITE_DCH_FE_1600_B_0_ENABLERSr(unit, i, reg32_val));
    }

    /*Enable Mac-Tx pump when leaky bucket is down*/
    for(i=0; i<SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; i++) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, REG_PORT_ANY, i, &reg32_val));
        soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg32_val ,FMAL_N_TX_PUMP_WHEN_LB_DNf, enable ? 1 : 0);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACL_FMAL_GENERAL_CONFIGURATIONr(unit, i, reg32_val));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMACH_FMAL_GENERAL_CONFIGURATIONr(unit, i, reg32_val));
    }

exit:
    DNXC_FUNC_RETURN;
}
#endif /*BCM_88790_A0*/

int
soc_ramon_fe1600_reduced_support_set(int unit)
{
    uint32 reg32_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Power Down DCQ*/
    DNXC_IF_ERR_EXIT(READ_ECI_SB_RSTN_AND_POWER_DOWNr(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_DCQ_2f, 1);
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_DCQ_3f, 1);
    DNXC_IF_ERR_EXIT(WRITE_ECI_SB_RSTN_AND_POWER_DOWNr(unit, reg32_val));

    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_1r(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_GLOBAL_1r, &reg32_val, DCQ_DOWNf, 0xC); /*DCQ2 and DCQ3*/
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_1r(unit, reg32_val));

    /*Power Down FMAC*/
    DNXC_IF_ERR_EXIT(READ_ECI_SB_RSTN_AND_POWER_DOWNr(unit, &reg32_val));
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_FMAC_4f, 1);
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_FMAC_5f, 1);
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_FMAC_6f, 1);
    soc_reg_field_set(unit, ECI_SB_RSTN_AND_POWER_DOWNr, &reg32_val, POWER_DOWN_FMAC_7f, 1);
    DNXC_IF_ERR_EXIT(WRITE_ECI_SB_RSTN_AND_POWER_DOWNr(unit, reg32_val));

    /*Power Down LCPLL*/
    DNXC_IF_ERR_EXIT(READ_CMIC_MISC_CONTROLr(unit, &reg32_val));
    soc_reg_field_set(unit, CMIC_MISC_CONTROLr, &reg32_val, XGXS2_PLL_PWRDWNf, 1);
    soc_reg_field_set(unit, CMIC_MISC_CONTROLr, &reg32_val, XGXS3_PLL_PWRDWNf, 1);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MISC_CONTROLr(unit, reg32_val));

    /*Set macro values*/
    SOC_DNXF_DEFS_SET(unit, nof_links, SOC_RAMON_FE1600_REDUCED_NOF_LINKS);
    SOC_DNXF_DEFS_SET(unit, nof_instances_mac, SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_MAC);
    SOC_DNXF_DEFS_SET(unit, nof_instances_mac_fsrd, SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_MAC_FSRD);
    SOC_DNXF_DEFS_SET(unit, nof_instances_dch, SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_DCH);
    SOC_DNXF_DEFS_SET(unit, nof_instances_dcl, SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_DCL);
    SOC_DNXF_DEFS_SET(unit, nof_instances_brdc_fmach, SOC_RAMON_FE1600_REDUCED_BLK_NOF_INSTANCES_BRDC_FMACH);

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_soc_properties_validate(int unit)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*fabric_device_mode*/
    if (SOC_IS_FE1600_REDUCED(unit)) {
        /*Multistage is not supported*/
        if (SOC_DNXF_CONFIG(unit).fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe2 ||
            SOC_DNXF_CONFIG(unit).fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe13)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("RAMON_FE1600_REDUCED do not support multistage ")));
        }
    }

    switch (SOC_DNXF_CONFIG(unit).fabric_device_mode)
    {
        case soc_dnxf_fabric_device_mode_single_stage_fe2:
        case soc_dnxf_fabric_device_mode_multi_stage_fe2:
        case soc_dnxf_fabric_device_mode_multi_stage_fe13:
        case soc_dnxf_fabric_device_mode_repeater:
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("RAMON_FE1600 does not support this device mode")));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_graceful_shutdown_set(int unit, soc_pbmp_t active_links, int shutdown, soc_pbmp_t unisolated_links, int isolate_device) 
{
    int rv;
    soc_port_t port;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if (SOC_DNXF_IS_FE13(unit)) 
    {
        /*FE13*/
        rv =  MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_fe13_graceful_shutdown_set, (unit, active_links, unisolated_links, shutdown));
        DNXC_IF_ERR_EXIT(rv);
    } 
    else 
    {
        /*FE2*/
        if (shutdown) {
            /*stop sending reachability cells*/
            rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_reachability_mask_set,(unit, active_links,soc_dnxc_isolation_status_isolated));
            DNXC_IF_ERR_EXIT(rv);

            sal_usleep(1000000); /*sleep 1 second*/

            /*RX reset*/
            SOC_PBMP_ITER(active_links, port)
            {
                rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_rx_enable_set, (unit, port, 0, 0));
                DNXC_IF_ERR_EXIT(rv);
            }
            sal_usleep(20000); /*sleep 20 milisec*/

            /*TX reset*/
            SOC_PBMP_ITER(active_links, port)
            {
                rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_tx_enable_set, (unit, port, 0));
                DNXC_IF_ERR_EXIT(rv);
            }
            sal_usleep(50000); /*sleep 50 mili sec*/

            if (!isolate_device)
            {
                /*Total isolate*/
                rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_topology_isolate_set, (unit, soc_dnxc_isolation_status_isolated)); 
                DNXC_IF_ERR_EXIT(rv);   
            }
        } else {

            /*Remove Total isolate*/
            if (!isolate_device)
            {
                rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_isolate_set,(unit, soc_dnxc_isolation_status_active)); 
                DNXC_IF_ERR_EXIT(rv);   
            }

            /*TX reset*/
            SOC_PBMP_ITER(active_links, port)
            {
                rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_tx_enable_set, (unit, port, 1));
                DNXC_IF_ERR_EXIT(rv);
            }

            /*RX reset*/
            SOC_PBMP_ITER(active_links, port)
            {
                rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_rx_enable_set, (unit, port, 0, 1));
                DNXC_IF_ERR_EXIT(rv);
            }

            sal_usleep(SOC_RAMON_FE1600_GRACEFUL_SHUT_DOWN_DISABLE_DELAY); 

            /*start sending reachability cells*/
            rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_reachability_mask_set,(unit, unisolated_links,soc_dnxc_isolation_status_active));
            DNXC_IF_ERR_EXIT(rv);

        }
    }

exit:
    DNXC_FUNC_RETURN;
}




int
soc_ramon_fe1600_drv_fe13_graceful_shutdown_set(int unit, soc_pbmp_t active_links, soc_pbmp_t unisolated_links, int shutdown) 
{
    int rv;
    soc_port_t port;
    DNXC_INIT_FUNC_DEFS;

    SOC_RAMON_FE1600_ONLY(unit);

    if (shutdown)
    {
        rv = soc_ramon_fe1600_drv_fe13_isolate_set(unit, unisolated_links, 1);
        DNXC_IF_ERR_EXIT(rv);

        /*RX reset*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_ramon_fe1600_port_control_rx_enable_set(unit, port, 0, 0);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(20000); /*sleep 20 milisec*/

        /*TX reset*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_ramon_fe1600_port_control_tx_enable_set(unit, port, 0);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(50000); /*sleep 50 mili sec*/

    } else { /*power up*/

        /*TX enable*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_ramon_fe1600_port_control_tx_enable_set(unit, port, 1);
            DNXC_IF_ERR_EXIT(rv);
        }

        /*RX enable*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_ramon_fe1600_port_control_rx_enable_set(unit, port, 0, 1);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(1000000); /*sleep 1 second*/

        rv = soc_ramon_fe1600_drv_fe13_isolate_set(unit, unisolated_links, 0);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_fe13_links_bitmap_get(int unit, soc_reg_above_64_val_t *all_links_bitmap, soc_reg_above_64_val_t *fap_links_bitmap, soc_reg_above_64_val_t *fe2_links_bitmap)
{
    soc_port_t port;
    int rv;
    soc_dnxf_fabric_link_device_mode_t link_device_mode;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(*all_links_bitmap);
    SOC_REG_ABOVE_64_CLEAR(*fap_links_bitmap);
    SOC_REG_ABOVE_64_CLEAR(*fe2_links_bitmap);
    for (port = 0; port < SOC_DNXF_DEFS_GET(unit, nof_links); port++)
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_link_device_mode_get, (unit, port, 1, &link_device_mode));
        DNXC_IF_ERR_EXIT(rv);

        SHR_BITSET(*all_links_bitmap, port);
        if (link_device_mode == soc_dnxf_fabric_link_device_mode_multi_stage_fe1)
        {
            /*link connected to FAP*/
            SHR_BITSET(*fap_links_bitmap, port);
        } else {
            /*link connected to FE2*/
            SHR_BITSET(*fe2_links_bitmap, port);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_fe13_isolate_set(int unit, soc_pbmp_t unisolated_links_pbmp, int isolate) 
{
    int rv;
    int ccs_index;
    soc_reg_above_64_val_t reg_above_64_val, all_links_bitmap, fap_links_bitmap, fe2_links_bitmap, unisolated_links;
    uint32 reg32;
    soc_port_t port;
    DNXC_INIT_FUNC_DEFS;

    SOC_RAMON_FE1600_ONLY(unit);

    rv = soc_ramon_fe1600_drv_fe13_links_bitmap_get(unit, &all_links_bitmap, &fap_links_bitmap, &fe2_links_bitmap);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(unisolated_links);
    SOC_PBMP_ITER(unisolated_links_pbmp, port)
    {
        SHR_BITSET(unisolated_links, port);
    }

    if (isolate)
    {
        /*Stop RTP update*/
        rv = READ_RTP_REG_0063r(unit, &reg32);
        DNXC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, RTP_REG_0063r, &reg32, FIELD_0_7f, 0x0);
        rv = WRITE_RTP_REG_0063r(unit, reg32);
        DNXC_IF_ERR_EXIT(rv);

        /* 
         *Isolate Links to FAP
         */
        DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));
        SOC_REG_ABOVE_64_AND(reg_above_64_val, fe2_links_bitmap);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));

        sal_usleep(30000); /*30 milisec*/

        /*CCS cells drop*/
        for (ccs_index = 0; ccs_index < SOC_DNXF_DEFS_GET(unit, nof_instances_ccs); ccs_index++)
        {
            DNXC_IF_ERR_EXIT(READ_CCS_REG_0062r(unit, ccs_index, reg_above_64_val));
            SOC_REG_ABOVE_64_AND(reg_above_64_val, fe2_links_bitmap);
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0062r(unit, ccs_index, reg_above_64_val));
        }       
        
        sal_usleep(30000); /*30 milisec*/

        /* 
         *Isolate Links to FE2
         */
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));

        sal_usleep(30000); /*30 milisec*/

        /*CCS cells drop*/
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
        for (ccs_index = 0; ccs_index < SOC_DNXF_DEFS_GET(unit, nof_instances_ccs); ccs_index++)
        {
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0062r(unit, ccs_index, reg_above_64_val));
        }       
        
        sal_usleep(30000); /*30 milisec*/

        /*Start RTP update*/
        rv = READ_RTP_REG_0063r(unit, &reg32);
        DNXC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, RTP_REG_0063r, &reg32, FIELD_0_7f, 0x1);
        rv = WRITE_RTP_REG_0063r(unit, reg32);
        DNXC_IF_ERR_EXIT(rv);

        sal_usleep(1000000); /*sleep 1 second*/

        
    } else { /*unisolate*/

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_fabric_topology_mesh_topology_reset(unit));

        sal_usleep(30000); /*30 milisec*/

        /*Unisolate links to FE2*/
        SOC_REG_ABOVE_64_COPY(reg_above_64_val, fe2_links_bitmap);
        for (ccs_index = 0; ccs_index < SOC_DNXF_DEFS_GET(unit, nof_instances_ccs); ccs_index++)
        {
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0062r(unit, ccs_index, reg_above_64_val));
        } 
       
        sal_usleep(30000); /*30 milisec*/

        SOC_REG_ABOVE_64_COPY(reg_above_64_val, fe2_links_bitmap);
        SOC_REG_ABOVE_64_AND(reg_above_64_val, unisolated_links);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));

        sal_usleep(30000); /*30 milisec*/

        /*Unisolate links to FAP*/
        SOC_REG_ABOVE_64_COPY(reg_above_64_val, all_links_bitmap);
        for (ccs_index = 0; ccs_index < SOC_DNXF_DEFS_GET(unit, nof_instances_ccs); ccs_index++)
        {
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0062r(unit, ccs_index, reg_above_64_val));
        }
        
        sal_usleep(30000); /*30 milisec*/

        SOC_REG_ABOVE_64_COPY(reg_above_64_val, all_links_bitmap);
        SOC_REG_ABOVE_64_AND(reg_above_64_val, unisolated_links);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));
    }

exit:
    DNXC_FUNC_RETURN;
}


/* reg  ECI_REG_0053r 
               [31:20] - SDK version of the last regular init; [19:19] - Did WB take place? 
               (=0x0,0x1) ; [18:18] - Did ISSU take place? (=0x0,0x1); 
               * Note that bits [15:0] are in use and should not be overridden. 
*/
int
soc_ramon_fe1600_drv_sw_ver_set(int unit)
{
    int         ver_val[3] = {0,0,0};
    uint32      regval, i, prev_regval;
    char        *ver;
    char        *cur_number_ptr;
    int rc;
    DNXC_INIT_FUNC_DEFS;
   
    
    regval = 0;
    ver = _build_release;

    cur_number_ptr = sal_strchr(ver, '-');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Invalid Build Release")));
    }
    ++cur_number_ptr;
    ver_val[0] = _shr_ctoi (cur_number_ptr);
    cur_number_ptr = sal_strchr(cur_number_ptr, '.');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Invalid Build Release")));
    }
    ++cur_number_ptr;
    ver_val[1] = _shr_ctoi (cur_number_ptr);
    cur_number_ptr = sal_strchr(cur_number_ptr, '.');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Invalid Build Release")));
    }
    ++cur_number_ptr;
    ver_val[2] = _shr_ctoi (cur_number_ptr);

   
    DNXC_IF_ERR_EXIT(READ_ECI_REG_0053r(unit, &prev_regval));

    for (i=0; i<3; i++) {
       regval = (regval | (0xf & ver_val[i]))<<4;
   }
    
   if (SOC_WARM_BOOT(unit)) {
       regval = regval | 0x8;
   }
   regval = (regval << 16 ) | ( prev_regval & 0xffff);

   SOC_DNXF_ALLOW_WARMBOOT_WRITE(WRITE_ECI_REG_0053r(unit, regval), rc);
   DNXC_IF_ERR_EXIT(rc);

exit:
  DNXC_FUNC_RETURN;;
}
 
#ifdef BCM_88754_A0
/*
 * BCM88754 includes Async fifo interface. 
 * This function configures async fifo using this interface.  
 */
#define SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_TIMEOUT                 (1000000) /*1 second*/
#define SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_MIN_POLLS               (100)     /*Number of times to check without a sleep*/
#define SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_MASK     (0x1FFFFFF)
#define SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_OFFSET   (3)

int
soc_ramon_fe1600_drv_bcm88754_async_fifo_set(int unit, int fmac_index, int lane, soc_field_t field, uint32 data) 
{
    uint64 reg64_val;
    uint32 reg32_val;
    uint32 read_valid;
    soc_timeout_t to;
    uint32 async_fifo_config_read;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Lock is not required - the bcm layer includes per device lock*/

    /* 
     *Reterive an updated value 
     */

    /*Configure read command*/
    DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, fmac_index, 0 /*not an array register*/, &reg64_val));
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMAND_LANEf, lane);
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMANDf, 0x1); /*read command*/
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_READ_COMMANDf, 0x0); /*read lane SIF configuration*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, fmac_index, 0 /*not an array register*/, reg64_val));

    /*Trigger*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_ACCESS_TRIGGERr(unit, fmac_index, 0x1));
    sal_usleep(10); /*10 micro second*/

    /*ACK polling*/
    if (SAL_BOOT_PLISIM) {
        read_valid = 1; /*Avoid polling on simulation*/
    } else {
        read_valid = 0;
    }

    soc_timeout_init(&to, SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_TIMEOUT , SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_MIN_POLLS);
    reg32_val = 0;
    while (read_valid == 0)
    {
        if (soc_timeout_check(&to)) {
            DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("FMAC SIF timeout")));
        }
        DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_ACCESS_READ_DATAr(unit, fmac_index, &reg32_val));
        read_valid = soc_reg_field_get(unit, FMAC_ASYNC_FIFO_ACCESS_READ_DATAr, reg32_val, FMAL_SIF_CPU_COMMAND_DATA_VALIDf);
    }

    /*Copy value*/
    /*FMAC_ASYNC_FIFO_ACCESS_READ_DATA bit 0-24 represents regsiter FMAC_ASYNC_FIFO_CONFIGURATION bits 3-27*/
    async_fifo_config_read = (reg32_val & SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_MASK) << SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_OFFSET;
    COMPILER_64_SET(reg64_val, 0x0, async_fifo_config_read);

    /*Configure SIF write command and data*/
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMAND_LANEf, lane);
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_READ_COMMANDf, 0x0);  /*Sif configuration*/
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMANDf, 0x0); /*write command*/
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, field, data);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, fmac_index, 0 /*not an array register*/, reg64_val));

    /*Trigger*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_ACCESS_TRIGGERr(unit, fmac_index, 0x1));

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_bcm88754_async_fifo_get(int unit, int fmac_index, int lane, int field, uint32 *data) 
{
    uint64 reg64_val;
    uint32 reg32_val;
    uint32 read_valid;
    soc_timeout_t to;
    uint32 async_fifo_config_read;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Lock is not required - the bcm layer includes per device lock*/

    /* 
     *Reterive an updated value
     */

    /*Configure read command*/
    DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, fmac_index, 0 /*not an array register*/, &reg64_val));
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMAND_LANEf, lane);
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_COMMANDf, 0x1); /*read command*/
    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_SIF_CPU_READ_COMMANDf, 0x0); /*read lane SIF configuration*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, fmac_index, 0 /*not an array register*/, reg64_val));

    /*Trigger*/
    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_ACCESS_TRIGGERr(unit, fmac_index, 0x1));
    sal_usleep(10); /*10 micro second*/

    /*ACK polling*/
    if (SAL_BOOT_PLISIM) {
        read_valid = 1; /*Avoid polling on simulation*/
    } else {
        read_valid = 0;
    }

    soc_timeout_init(&to, SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_TIMEOUT , SOC_RAMON_FE1600_DRV_BCM88754_SIF_POLLING_MIN_POLLS);
    reg32_val = 0;
    while (read_valid == 0)
    {
        if (soc_timeout_check(&to)) {
            DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("FMAC SIF timeout")));
        }
        DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_ACCESS_READ_DATAr(unit, fmac_index, &reg32_val));
        read_valid = soc_reg_field_get(unit, FMAC_ASYNC_FIFO_ACCESS_READ_DATAr, reg32_val, FMAL_SIF_CPU_COMMAND_DATA_VALIDf);
    }

    /*Copy value*/
    /*FMAC_ASYNC_FIFO_ACCESS_READ_DATA bit 0-24 represents regsiter FMAC_ASYNC_FIFO_CONFIGURATION bits 3-27*/
    async_fifo_config_read = (reg32_val & SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_MASK) << SOC_RAMON_FE1600_DRV_BCM88754_SIF_ASYNC_FIFO_CONFIG_READ_OFFSET;
    COMPILER_64_SET(reg64_val, 0x0, async_fifo_config_read);
    *data = soc_reg64_field32_get(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, reg64_val, field);

exit:
    DNXC_FUNC_RETURN;
}
#endif /*BCM_88754_A0*/


/*
 * Memory bist test
 */
int
soc_ramon_fe1600_drv_mbist(int unit, int skip_errors)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    SOC_DNXF_DRV_INIT_LOG(unit, "Memory Bist");
    rc = soc_bist_all_ramon_fe1600(unit, skip_errors);
    DNXC_IF_ERR_EXIT(rc);
        
exit:
    DNXC_FUNC_RETURN;   
}

int
soc_ramon_fe1600_drv_link_to_block_mapping(int unit, int link, int* block_id,int* inner_link, int block_type)
{   
    int nof_links_in_blk, asymmetric_quad = 0;

    DNXC_INIT_FUNC_DEFS;

    if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit) && (block_type == SOC_BLK_DCH || block_type == SOC_BLK_DCM))
    {
        MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_asymmetrical_quad_get, (unit, link, &asymmetric_quad));
        if (asymmetric_quad == 0 || asymmetric_quad == 1)
        {
            *block_id = asymmetric_quad; /* first changed quad is in block 0, second is in block 1*/
            *inner_link = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq) + link % SOC_DNXF_DEFS_GET(unit, nof_links_in_quad); /* 36 + inner quad link */
            SOC_EXIT;
        }
    }
    switch (block_type)
    {
       case SOC_BLK_DCH:
       case SOC_BLK_DCM:

           nof_links_in_blk = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq);

           *block_id=INT_DEVIDE(link,nof_links_in_blk);
           *inner_link=link % nof_links_in_blk;
           break;

       case SOC_BLK_DCL:
           nof_links_in_blk = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcl);

           *block_id=INT_DEVIDE(link,nof_links_in_blk);
           *inner_link=link % nof_links_in_blk;
           break;

       case SOC_BLK_FMAC:
           nof_links_in_blk = SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);

           *block_id=INT_DEVIDE(link,nof_links_in_blk);
           *inner_link=link % nof_links_in_blk;
           break;


       default:
           DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("block (%d) - link to block mapiing is not supported"), block_type));
           break;
    }


exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_fe1600_drv_block_pbmp_get(int unit, int block_type, int blk_instance, soc_pbmp_t *pbmp)
{   
    int first_link = 0,range = 0;

    DNXC_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*pbmp);

    switch (block_type)
    {
       case SOC_BLK_DCH:
       case SOC_BLK_DCM:
           first_link = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq) * blk_instance;
           range = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq);
           
           break;
       default:
           DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("block (%d) - block pbmp is not supported"), block_type));
           break;
    }

    SOC_PBMP_PORTS_RANGE_ADD(*pbmp, first_link, range);
    SOC_PBMP_AND(*pbmp, PBMP_SFI_ALL(unit));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_drv_blocks_reset
 * Purpose:
 *      RAMON_FE1600 reset blocks
 * Parameters:
 *      unit                          - (IN)  Unit number.
 *      force_blocks_reset_value      - (IN)  if 0 - reset all blocks. otherwise use  block_bitmap
 *      block_bitmap                  - (IN)  which blocks to reset.
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_fe1600_drv_blocks_reset(int unit, int force_blocks_reset_value, soc_reg_above_64_val_t *block_bitmap) 
{
    uint64 reg_val64;
    uint32 reg_val32;

    DNXC_INIT_FUNC_DEFS;

    if (force_blocks_reset_value)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR("block reset force is not supported")));
    } else {
         
        DNXC_IF_ERR_RETURN(WRITE_CMIC_SOFT_RESET_REGr(unit, 0x0));
        sal_usleep(20);
        DNXC_IF_ERR_RETURN(WRITE_CMIC_SOFT_RESET_REGr(unit, 0xffffffff));
        sal_usleep(20);
        COMPILER_64_SET(reg_val64, 0x7, 0xFFFFFFFF);
        DNXC_IF_ERR_RETURN(WRITE_ECI_FE_1600_SOFT_RESETr(unit, reg_val64));
        sal_sleep(1);
        COMPILER_64_SET(reg_val64, 0x0, 0x0);
        DNXC_IF_ERR_RETURN(WRITE_ECI_FE_1600_SOFT_RESETr(unit,reg_val64));
        reg_val32 = 0x1ff;
        DNXC_IF_ERR_RETURN(WRITE_ECI_SB_RSTN_AND_POWER_DOWNr(unit,reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;  
}

/*
 * Function:
 *      soc_ramon_fe1600_drv_test_reg_filter
 * Purpose:
 *      Special registers should not be tested
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      reg                         - (IN)  relevant reg
 *      is_filtered                 - (OUT) if 1 - do not test this reg
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_fe1600_drv_test_reg_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch(reg) 
    {
        case ECI_POWER_UP_MACHINEr:
        case CMIC_SOFT_RESET_REGr:
        case CMIC_SOFT_RESET_REG_2r:
        case ECI_FE_1600_SOFT_RESETr:
        case ECI_FE_1600_SOFT_INITr:
        case ECI_SB_RSTN_AND_POWER_DOWNr:
        case FMAC_ASYNC_FIFO_CONFIGURATIONr:
        /*In these registers the read value 
          will always be different than write value by design*/
        case ECI_INDIRECT_COMMANDr:
        case FSRD_INDIRECT_COMMANDr:
        case RTP_INDIRECT_COMMANDr:
                *is_filtered = 1;
        default:
            break;
    }

#ifdef BCM_88754_A0
    if (SOC_IS_BCM88754_A0(unit))
    {
        switch(reg)
        {
            case ECI_REG_0120r:
                *is_filtered = 1;
            default:
                break;
        }
    }
#endif /*BCM_88754_A0*/

    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_drv_test_reg_default_val_filter
 * Purpose:
 *      Special registers should not be tested
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      reg                         - (IN)  relevant reg
 *      is_filtered                 - (OUT) if 1 - do not test this reg
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_fe1600_drv_test_reg_default_val_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    /*Don't test initialization registers 
      and registers which aren't consistent
      between blocks of the same type*/
    switch(reg) {
        case FMAC_ASYNC_FIFO_CONFIGURATIONr:
        case CCS_REG_0058r:
        case DCL_REG_005Cr:
        case FMAC_SBUS_LAST_IN_CHAINr:
        case FSRD_REG_0058r:
        case ECI_FE_1600_SOFT_RESETr:
        case ECI_FE_1600_SOFT_INITr:
        case ECI_SB_RSTN_AND_POWER_DOWNr:
        case FMAC_SBUS_BROADCAST_IDr:
        case CCS_REG_0080r:
        /*skip the following register - 
          expecting a write to eci gloabl register in Fe1600_A0 in order to get reset value
          in contrast to Fe1600_B0*/
        case FMAC_REG_005Ar:
        case DCH_REG_005Ar:
        case DCMA_REG_005Ar:
        case DCMB_REG_005Ar:
        case DCMC_REG_005Ar:
        case DCL_REG_005Ar:
        case RTP_REG_005Ar:
        case MESH_TOPOLOGY_GLOBAL_REGr:
            *is_filtered = 1; /* Skip these registers */
        default:
            break;
    }

    DNXC_FUNC_RETURN; 
}


/*
 * Function:
 *      soc_ramon_fe1600_drv_test_mem_filter
 * Purpose:
 *      Special memories (dynamic) should not be tested
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      mem                         - (IN)  relevant reg
 *      is_filtered                 - (OUT) if 1 - do not test this mem
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_fe1600_drv_test_mem_filter(int unit, soc_mem_t mem, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch(mem) 
    {
        case RTP_RMHMTm:
        case RTP_CUCTm:
        case RTP_DUCTPm:
        case RTP_DUCTSm:
        case RTP_MEM_800000m:
        case RTP_MEM_900000m:
            *is_filtered = 1;
        default:
            break;
    }

    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_drv_test_brdc_blk_filter
 * Purpose:
 *      Special registers should not be tested in broadcast block test
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      reg                         - (IN)  relevant reg
 *      is_filtered                 - (OUT) if 1 - do not test this reg
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_fe1600_drv_test_brdc_blk_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch(reg) 
    {   
        /*FMACH*/
        case BRDC_FMACL_ASYNC_FIFO_CONFIGURATIONr:
        case BRDC_FMACL_SBUS_BROADCAST_IDr:
        case BRDC_FMACL_SBUS_LAST_IN_CHAINr:
        case BRDC_FMACL_GTIMER_TRIGGERr:
        /*FMACL*/
        case BRDC_FMACH_ASYNC_FIFO_CONFIGURATIONr:
        case BRDC_FMACH_SBUS_BROADCAST_IDr:
        case BRDC_FMACH_SBUS_LAST_IN_CHAINr:
        case BRDC_FMACH_GTIMER_TRIGGERr:
        /*FSRD*/
        case BRDC_FSRD_SBUS_BROADCAST_IDr:
        case BRDC_FSRD_REG_0054r:
        case BRDC_FSRD_REG_0058r:
        case BRDC_FSRD_REG_01E9r:
        case BRDC_FSRD_REG_01EBr:
        case BRDC_FSRD_REG_01ECr:
        case BRDC_FSRD_REG_01EDr:
        case BRDC_FSRD_REG_01EFr:
        case BRDC_FSRD_REG_01F1r:
        case BRDC_FSRD_REG_01F3r:
        case BRDC_FSRD_GTIMER_TRIGGERr:
        case BRDC_FSRD_INDIRECT_COMMANDr:
                *is_filtered = 1;
                break;
        default:
            break;
    }

    DNXC_FUNC_RETURN; 
}
/*
 * Function:
 *      soc_ramon_fe1600_drv_test_brdc_blk_info_get
 * Purpose:
 *      Returns necessary info on device broadcast blocks
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      max_size                    - (IN)  max number of broadcast blocks
 *      brdc_info                   - (OUT) structure which holds the required info about each broadcast block
 *      actual_size                 - (OUT) number of broadcast blocks
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int 
soc_ramon_fe1600_drv_test_brdc_blk_info_get(int unit, int max_size, soc_reg_brdc_block_info_t *brdc_info, int *actual_size)
{
    int instance;
    int i;
    DNXC_INIT_FUNC_DEFS;
    *actual_size = 0;

    /*FMACL*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_FMACL;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 2; instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*FMACH*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_FMACH;
        for (i = 0, instance = SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 2; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*FSRD*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_FSRD;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FSRD_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

exit:
    DNXC_FUNC_RETURN; 
}

int
soc_ramon_fe1600_drv_block_valid_get(int unit, int blktype, int blkid, char *valid)
{
    int nof_block_instances;
    DNXC_INIT_FUNC_DEFS;

    if (SOC_IS_FE1600_REDUCED(unit)) 
    {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_nof_block_instances(unit, &blktype, &nof_block_instances));
        if (blkid >= nof_block_instances)
        {
            *valid = 0;
        } else {
            *valid = 1;
        }
    } else {

        *valid = 1;
    }

exit:
    DNXC_FUNC_RETURN; 
}

#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME

