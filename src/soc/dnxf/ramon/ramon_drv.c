/*
 * $Id: ramon_drv.c,v 1.1.2.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON DRV
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <soc/mem.h>

#include <soc/dnxc/legacy/dnxc_cmic.h>
#include <soc/dnxc/legacy/dnxc_iproc.h>
#include <soc/dnxc/legacy/dnxc_ser_correction.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/mbcm.h>

#include <soc/dnxf/ramon/ramon_drv.h>
#include <soc/dnxf/ramon/ramon_defs.h>
#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxf/ramon/ramon_stack.h>
#include <soc/dnxf/ramon/ramon_port.h>
#include <soc/dnxf/ramon/ramon_fabric_flow_control.h>
#include <soc/dnxf/ramon/ramon_fabric_links.h>
#include <soc/dnxf/ramon/ramon_fabric_topology.h>

#include <soc/dnxf/fe1600/fe1600_config_imp_defs.h>
#include <soc/dnxf/fe1600/fe1600_drv.h>

extern char *_build_release;

/* 
 * SBUS rings mapping
 */
#define _SOC_RAMON_DRV_SBUS_RING_MAP_0_VAL                 (0x02222227)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_1_VAL                 (0x33333222)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_2_VAL                 (0X55333333)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_3_VAL                 (0x65555555)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_4_VAL                 (0x66666666)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_5_VAL                 (0x55555555)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_6_VAL                 (0x66666665)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_7_VAL                 (0x44444466)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_8_VAL                 (0x10444444)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_9_VAL                 (0x22040065)
#define _SOC_RAMON_DRV_SBUS_RING_MAP_10_VAL                (0x00000033)

#define _SOC_RAMON_DRV_BRDC_FMAC_AC_ID                     (72)
#define _SOC_RAMON_DRV_BRDC_FMAC_BD_ID                     (73)
#define _SOC_RAMON_DRV_BRDC_DCH                            (78)
#define _SOC_RAMON_DRV_BRDC_DCM                            (80)
#define _SOC_RAMON_DRV_BRDC_DCL                            (79)
#define _SOC_RAMON_DRV_BRDC_CCS                            (81)
#define _SOC_RAMON_DRV_BRDC_FSRD                           (76)

/*
 * PVT monitor
 */
#define _SOC_RAMON_ECI_PVT_MON_CONTROL_REG_POWERDOWN_BIT   (32)
#define _SOC_RAMON_ECI_PVT_MON_CONTROL_REG_RESET_BIT       (33)
#define _SOC_RAMON_PVT_MON_NOF                             (4)
#define _SOC_RAMON_PVT_FACTOR                              (49103)
#define _SOC_RAMON_PVT_BASE                                (41205000)

/* WFQ config */
#define _SOC_RAMON_WFQ_PIPES_PRIORITY_INIT_VALUE           (0x7)

/*FE13 asymmetrical*/
#define _SOC_RAMON_DRV_FE13_ASYM_FAP_FIRST_LINK_QUARTER_0_PART_2       (104)
#define _SOC_RAMON_DRV_FE13_ASYM_FAP_FIRST_LINK_QUARTER_1_PART_2       (140)
#define _SOC_RAMON_DRV_FE13_ASYM_FAP_RANGE_PART_2                      (4)
#define _SOC_RAMON_DRV_FE13_ASYM_FE2_RANGE                             (32)

STATIC int
soc_ramon_reset_cmic_iproc_regs(int unit) 
{
    DNXC_INIT_FUNC_DEFS;

    /*Configure PAXB, enabling the access of CMIC*/
    DNXC_IF_ERR_EXIT(soc_dnxc_iproc_config_paxb(unit));
    sal_usleep(10*1000); /*wait 10 mili sec*/
    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_device_hard_reset(unit, SOC_DNXC_RESET_ACTION_INOUT_RESET));
    DNXC_IF_ERR_EXIT(soc_dnxc_iproc_config_paxb(unit));

    /* Config Endianess */
    soc_endian_config(unit);
    soc_pci_ep_config(unit,0);

    /*SBUS rings map configuration*/
    DNXC_IF_ERR_EXIT(soc_ramon_drv_rings_map_set(unit));
    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_sbus_timeout_set(unit, SOC_DNXF_CONFIG(unit).core_clock_speed /*KHz*/, SOC_CONTROL(unit)->schanTimeout));

    /* Clear SCHAN_ERR */
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC0_SCHAN_ERRr(unit, 0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC1_SCHAN_ERRr(unit, 0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC2_SCHAN_ERRr(unit, 0));

    /*MDIO configuration*/
    DNXC_IF_ERR_EXIT(soc_ramon_drv_mdio_config_set(unit));
    
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Get the AVS - Adjustable Voltage Scaling value of the FE3600
*********************************************************************/
int soc_ramon_avs_value_get(
            int       unit,
            uint32*      avs_val)
{
    uint32
        reg_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(avs_val);

    *avs_val = 0;

    DNXC_IF_ERR_EXIT(READ_ECI_REG_01BBr(unit, &reg_val));
    *avs_val = soc_reg_field_get(unit, ECI_REG_01BBr, reg_val, FIELD_24_26f);

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_drv_blocks_reset
 * Purpose:
 *      RAMON reset blocks
 * Parameters:
 *      unit                          - (IN)  Unit number.
 *      force_blocks_reset_value      - (IN)  if 0 - reset all blocks. otherwise use  block_bitmap
 *      block_bitmap                  - (IN)  which blocks to reset.
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
int
soc_ramon_drv_blocks_reset(int unit, int force_blocks_reset_value, soc_reg_above_64_val_t *block_bitmap) 
{
    soc_reg_above_64_val_t reg_above_64;
    DNXC_INIT_FUNC_DEFS;

    if (force_blocks_reset_value)
    {
        SOC_REG_ABOVE_64_COPY(reg_above_64, *block_bitmap);
    } else {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64);
    }
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64));
    sal_sleep(1); 
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_RESETr(unit, reg_above_64));
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SBUS_RESETr(unit, reg_above_64));
    

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_drv_fabric_device_mode_set(int unit, soc_dnxf_fabric_device_mode_t fabric_device_mode)
{
    uint32 repeater_mode_get, repeater_mode_set;
    uint32 reg32_val;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_FE_CFG_1r(unit, &reg32_val));

    /*Repeater*/
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, REP_MODEf, 
                      (fabric_device_mode == soc_dnxf_fabric_device_mode_repeater) ? 1 : 0);
    /*Active Repea  ter*/
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, REP_MODE_FC_ENf, 0); /*Disable: not supported*/
        
    /*FE13*/
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, FE_13_MODEf, 
                      (fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe13 || 
                       fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe13_asymmetric) ? 1 : 0);
    /*Asymmetrical FE13*/
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, FAP_80_LINKSf, SOC_DNXF_IS_FE13_ASYMMETRIC(unit)? 1:0); 

    /*Multistage*/
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, MULTISTAGE_MODEf, 
                      (fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe2 ||
                       fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe13 ||
                       fabric_device_mode == soc_dnxf_fabric_device_mode_multi_stage_fe13_asymmetric) ? 1 : 0);
    

    

    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_FE_CFG_1r(unit, reg32_val));

    /*Repeater - devices for repeater only use check*/
    DNXC_IF_ERR_EXIT(READ_DCH_GLOBAL_GENERAL_FE_CFG_1r(unit, REG_PORT_ANY, &reg32_val));
    repeater_mode_get = soc_reg_field_get(unit, DCH_GLOBAL_GENERAL_FE_CFG_1r, reg32_val, REP_MODEf);
    repeater_mode_set = (fabric_device_mode == soc_dnxf_fabric_device_mode_repeater) ? 1 : 0;
    if (repeater_mode_set  != repeater_mode_get && !SAL_BOOT_PLISIM) {
        DNXC_EXIT_WITH_ERR(SOC_E_INIT, (_BSL_DNXC_MSG("FABRIC_DEVICE_MODE!=REPEATER -  device for repeater only use")));
    }

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_drv_fabric_load_balancing_set(int unit, soc_dnxf_load_balancing_mode_t load_balancing)
{
    uint64 reg_val64;
    uint32 load_balancing_val;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));

    load_balancing_val = soc_dnxf_load_balancing_mode_normal == load_balancing ? 1 : 0;
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, DISABLE_RCG_LOAD_BALANCINGf, 
                            load_balancing_val);

    load_balancing_val = (soc_dnxf_load_balancing_mode_destination_unreachable == load_balancing) ? 0 : 1;
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, load_balancing_val);
                            
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

    /*link load load-balancing*/
    DNXC_IF_ERR_EXIT(READ_RTP_DRH_LOAD_BALANCING_LEVEL_CONFIGr(unit, &reg_val64));
    soc_reg64_field32_set(unit, RTP_DRH_LOAD_BALANCING_LEVEL_CONFIGr, &reg_val64, LOAD_BALANCE_LEVELS_IGNOREf, 0);
    DNXC_IF_ERR_EXIT(WRITE_RTP_DRH_LOAD_BALANCING_LEVEL_CONFIGr(unit, reg_val64));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_drv_fabric_multicast_config_set(int unit, soc_dnxf_multicast_mode_t mc_mode, int fe_mc_priority_map_enable)
{
    int i;
    uint32 reg_val32;
    uint32 multicast_mode_field_val;
    soc_dnxf_multicast_table_mode_t multicast_mode;
    uint64 reg_val64;
    DNXC_INIT_FUNC_DEFS;


    for (i=0 ; i < SOC_DNXF_DEFS_GET(unit, nof_instances_dch) ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCH_PRIORITY_TRANSLATIONr(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_PRIORITY_TRANSLATIONr, &reg_val32, LOW_PRE_MUL_SETf, SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable ? 1 : 0);
        soc_reg_field_set(unit, DCH_PRIORITY_TRANSLATIONr, &reg_val32, MID_PRE_MUL_SETf, SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable ? 1 : 0);
        DNXC_IF_ERR_EXIT(WRITE_DCH_PRIORITY_TRANSLATIONr(unit, i, reg_val32));

    }
    if (SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable)
    {
        COMPILER_64_ZERO(reg_val64);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_DCH_USE_MC_CELL_PRIO_BMPr(unit, reg_val64));
    }
    

    /*MC mode: direct / indirect*/
    DNXC_IF_ERR_EXIT(READ_RTP_MULTICAST_MODE_SELECTIONr(unit, &reg_val32));
    soc_reg_field_set(unit, RTP_MULTICAST_MODE_SELECTIONr, &reg_val32, MC_INDIRECT_LIST_OF_FAPS_MODEf, 
                        soc_dnxf_multicast_mode_indirect == mc_mode ? 1 : 0);
    
    /* configure according to multicast table mode */
    DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_multicast_mode_get, (unit, &multicast_mode)));
    switch (multicast_mode)
    {
        case soc_dnxf_multicast_table_mode_64k:
            multicast_mode_field_val = 0; /* 000 */
            break;
        case soc_dnxf_multicast_table_mode_128k:
            multicast_mode_field_val = 1; /* 001 */
            break;
        case soc_dnxf_multicast_table_mode_128k_half:
            multicast_mode_field_val = 4; /* 100 */
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("wrong mc_table_mode value %d"),SOC_DNXF_CONFIG(unit).fe_mc_id_range));
    }
    soc_reg_field_set(unit, RTP_MULTICAST_MODE_SELECTIONr, &reg_val32, MC_TABLE_MODEf, multicast_mode_field_val);

    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTICAST_MODE_SELECTIONr(unit, reg_val32));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_drv_fabric_local_routing_set(int unit, int local_routing_enable)
{
    int i;
	uint32 reg_val32;
	uint64 reg_val64;

	DNXC_INIT_FUNC_DEFS;

	DNXC_IF_ERR_EXIT(READ_RTP_DRH_LOAD_BALANCING_GENERAL_CONFIGr(unit, &reg_val64));
	soc_reg64_field32_set(unit, RTP_DRH_LOAD_BALANCING_GENERAL_CONFIGr, &reg_val64, ENABLE_LOCAL_FE_1_ROUTINGf, local_routing_enable == 1 ? 0xf : 0);
    soc_reg64_field32_set(unit, RTP_DRH_LOAD_BALANCING_GENERAL_CONFIGr, &reg_val64, ENABLE_CTRL_LOCAL_FE_1_ROUTINGf, 0x1); /*Always allow local routing for contorl cells*/
	DNXC_IF_ERR_EXIT(WRITE_RTP_DRH_LOAD_BALANCING_GENERAL_CONFIGr(unit, reg_val64));

	for (i=0 ; i < SOC_DNXF_DEFS_GET(unit, nof_instances_dcm) ; i++ )
	{
		DNXC_IF_ERR_EXIT(READ_DCM_DCM_ENABLERS_REGISTERr(unit, i, &reg_val32));
		soc_reg_field_set(unit, DCM_DCM_ENABLERS_REGISTERr, &reg_val32, LOCAL_ROUT_ENABLEf, local_routing_enable == 1 ? 1:0);
		DNXC_IF_ERR_EXIT(WRITE_DCM_DCM_ENABLERS_REGISTERr(unit, i, reg_val32));
	}

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
_soc_ramon_drv_fabric_cell_priority_config_set(int unit, uint32 min_tdm_priority)
{
    uint32 reg32_val;
    uint32 tdm_bmp;
    int dch_instance;
    DNXC_INIT_FUNC_DEFS;

    /*TDM priority*/
    for (dch_instance = 0; dch_instance < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); dch_instance++)
    {
        tdm_bmp = 0;
        DNXC_IF_ERR_EXIT(READ_DCH_TDM_PRIORITYr(unit, dch_instance, &reg32_val));
        if (min_tdm_priority == SOC_DNXF_FABRIC_TDM_PRIORITY_NONE) 
        {
            /*Disable tdm priority*/
            soc_reg_field_set(unit, DCH_TDM_PRIORITYr, &reg32_val, TDM_PRI_BMP_ENf, 0x0); 
            soc_reg_field_set(unit, DCH_TDM_PRIORITYr, &reg32_val, TDM_PRI_BMPf, tdm_bmp);
        } else {
            /*Mark tdm priorties*/
            SHR_BITSET_RANGE(&tdm_bmp, min_tdm_priority, soc_dnxf_fabric_priority_nof - min_tdm_priority);
            soc_reg_field_set(unit, DCH_TDM_PRIORITYr, &reg32_val, TDM_PRI_BMP_ENf, 0x1); 
            soc_reg_field_set(unit, DCH_TDM_PRIORITYr, &reg32_val, TDM_PRI_BMPf, tdm_bmp);
        }
        DNXC_IF_ERR_EXIT(WRITE_DCH_TDM_PRIORITYr(unit, dch_instance, reg32_val));
    }


exit:
    DNXC_FUNC_RETURN;
}

#define _SOC_RAMON_MAX_LINKS_IN_DCH    (40)

STATIC int
_soc_ramon_drv_fabric_pipes_config_set(int unit, soc_dnxc_fabric_pipe_map_t fabric_pipe_mapping)
{
    uint32 reg32_val;
    int rv, link;
    int dch_instance;
    int fmac_instance, inner_link;
    uint64 reg64_val;
    DNXC_INIT_FUNC_DEFS;

    /* 
     *Set number of pipes 
     */

    /*ECI*/
    rv = READ_ECI_GLOBAL_GENERAL_FE_CFG_1r(unit,&reg32_val);
    DNXC_IF_ERR_EXIT(rv);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, PIPE_1_ENf, fabric_pipe_mapping.nof_pipes > 1);
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_FE_CFG_1r, &reg32_val, PIPE_2_ENf, fabric_pipe_mapping.nof_pipes > 2);
    rv = WRITE_ECI_GLOBAL_GENERAL_FE_CFG_1r(unit,reg32_val);
    DNXC_IF_ERR_EXIT(rv);

    /*FMAC*/
    PBMP_SFI_ITER(unit, link)
    {
        fmac_instance = link / SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
        inner_link = link % SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
        rv  = READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, fmac_instance, inner_link , &reg32_val);
        DNXC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg32_val, FMAL_N_PARALLEL_DATA_PATHf, fabric_pipe_mapping.nof_pipes - 1);
        rv  = WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, fmac_instance, inner_link , reg32_val);
        DNXC_IF_ERR_EXIT(rv);
    }

    /*DCH*/
    COMPILER_64_SET(reg64_val, 0, 0);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes  == 2)
    {
        COMPILER_64_MASK_CREATE(reg64_val, _SOC_RAMON_MAX_LINKS_IN_DCH, 0);
    }
    rv = WRITE_BRDC_DCH_TWO_PIPES_BMPr(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);

    COMPILER_64_SET(reg64_val, 0, 0);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes  == 3)
    {
        COMPILER_64_MASK_CREATE(reg64_val, _SOC_RAMON_MAX_LINKS_IN_DCH, 0);
    }
    rv = WRITE_BRDC_DCH_THREE_PIPES_BMPr(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);

    /*DCL*/
    COMPILER_64_SET(reg64_val, 0, 0);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes  == 2)
    {
        COMPILER_64_MASK_CREATE(reg64_val, SOC_DNXF_DEFS_GET(unit, nof_links_in_dcl), 0);
    }
    rv = WRITE_BRDC_DCL_TWO_PIPES_BMPr(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);

    COMPILER_64_SET(reg64_val, 0, 0);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes  == 3)
    {
        COMPILER_64_MASK_CREATE(reg64_val, SOC_DNXF_DEFS_GET(unit, nof_links_in_dcl), 0);
    }
    rv = WRITE_BRDC_DCL_THREE_PIPES_BMPr(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);

    /* 
     * Pipe Mapping
     */
    /*DCH*/
    for (dch_instance = 0; dch_instance < SOC_DNXF_DEFS_GET(unit, nof_instances_dch); dch_instance++)
    {
        rv = READ_DCH_PIPES_SEPARATION_REGISTERr(unit, dch_instance, &reg32_val);
        DNXC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, UC_PRI_0_PIPEf, fabric_pipe_mapping.config_uc[0]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, UC_PRI_1_PIPEf, fabric_pipe_mapping.config_uc[1]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, UC_PRI_2_PIPEf, fabric_pipe_mapping.config_uc[2]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, UC_PRI_3_PIPEf, fabric_pipe_mapping.config_uc[3]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, MC_PRI_0_PIPEf, fabric_pipe_mapping.config_mc[0]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, MC_PRI_1_PIPEf, fabric_pipe_mapping.config_mc[1]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, MC_PRI_2_PIPEf, fabric_pipe_mapping.config_mc[2]);
        soc_reg_field_set(unit, DCH_PIPES_SEPARATION_REGISTERr, &reg32_val, MC_PRI_3_PIPEf, fabric_pipe_mapping.config_mc[3]);
        rv = WRITE_DCH_PIPES_SEPARATION_REGISTERr(unit, dch_instance, reg32_val);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_set_operation_mode(int unit)
{
    int rv;
    DNXC_INIT_FUNC_DEFS;

    /* 
     * Device Mode Configuration
     */
     rv = _soc_ramon_drv_fabric_device_mode_set(unit, SOC_DNXF_CONFIG(unit).fabric_device_mode);
     DNXC_IF_ERR_EXIT(rv);
    

     /*
     * Fabric pipes configuration
     */
     rv = _soc_ramon_drv_fabric_cell_priority_config_set(unit, SOC_DNXF_CONFIG(unit).fabric_tdm_priority_min);
     DNXC_IF_ERR_EXIT(rv);

    /*
     * Fabric pipes configuration
     */
     rv = _soc_ramon_drv_fabric_pipes_config_set(unit, SOC_DNXF_FABRIC_PIPES_CONFIG(unit));
     DNXC_IF_ERR_EXIT(rv);


     /*
      * Multicast mode configuration
      */
     rv = _soc_ramon_drv_fabric_multicast_config_set(unit,  SOC_DNXF_CONFIG(unit).fabric_multicast_mode, SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable); 
     DNXC_IF_ERR_EXIT(rv);

     /* 
      * Load Balancing
      */
     rv = _soc_ramon_drv_fabric_load_balancing_set(unit, SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode);
     DNXC_IF_ERR_EXIT(rv);

	 if (SOC_DNXF_IS_FE13(unit))
	 {
		 rv = _soc_ramon_drv_fabric_local_routing_set(unit, SOC_DNXF_CONFIG(unit).fabric_local_routing_enable);
		 DNXC_IF_ERR_EXIT(rv);
	 }


exit:
    DNXC_FUNC_RETURN;
}

#define _SOC_RAMON_DRV_MULTIPLIER_TABLE_MAX_LINKS                  (72)

#define _SOC_RAMON_DRV_MULTIPLIER_T1_LENGTH                        (64)
#define _SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH                         (32)
#define _SOC_RAMON_DRV_MULTIPLIER_T1_MAX_TOTAL_LINKS               (_SOC_RAMON_DRV_MULTIPLIER_T1_LENGTH)
#define _SOC_RAMON_DRV_MULTIPLIER_T1_MAX_ACTIVE_LINKS              (_SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH)

#define _SOC_RAMON_DRV_MULTIPLIER_T1_AND_T2_SIZE                   (_SOC_RAMON_DRV_MULTIPLIER_T1_LENGTH * _SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH)

#define _SOC_RAMON_DRV_MULTIPLIER_T2_LENGTH                        (32)
#define _SOC_RAMON_DRV_MULTIPLIER_T2_WIDTH                         (32)
#define _SOC_RAMON_DRV_MULTIPLIER_T2_MIN_TOTAL_LINKS               (33)
#define _SOC_RAMON_DRV_MULTIPLIER_T2_MIN_ACTIVE_LINKS              (_SOC_RAMON_DRV_MULTIPLIER_T1_MAX_ACTIVE_LINKS + 1)

#define _SOC_RAMON_DRV_MULTIPLIER_T3_LENGTH                        (8)
#define _SOC_RAMON_DRV_MULTIPLIER_T3_WIDTH                         (64)
#define _SOC_RAMON_DRV_MULTIPLIER_T3_MIN_TOTAL_LINKS               (_SOC_RAMON_DRV_MULTIPLIER_T1_MAX_TOTAL_LINKS + 1)
#define _SOC_RAMON_DRV_MULTIPLIER_T3_MAX_ACTIVE_LINKS              (64)
#define _SOC_RAMON_DRV_MULTIPLIER_T3_SIZE                          (_SOC_RAMON_DRV_MULTIPLIER_T3_LENGTH * _SOC_RAMON_DRV_MULTIPLIER_T3_WIDTH)

#define _SOC_RAMON_DRV_MULTIPLIER_T4_LENGTH                        (8)
#define _SOC_RAMON_DRV_MULTIPLIER_T4_WIDTH                         (8)
#define _SOC_RAMON_DRV_MULTIPLIER_T4_MIN_TOTAL_LINKS               (_SOC_RAMON_DRV_MULTIPLIER_T1_MAX_TOTAL_LINKS + 1)
#define _SOC_RAMON_DRV_MULTIPLIER_T4_MIN_ACTIVE_LINKS              (_SOC_RAMON_DRV_MULTIPLIER_T3_MAX_ACTIVE_LINKS + 1)

#define _SOC_RAMON_MULTIPLIER_TABLE_DUMP 0


extern int soc_dnxf_clean_rtp_table_array(int unit, soc_mem_t mem, soc_reg_above_64_val_t data);


STATIC int
soc_ramon_reset_tables(int unit)
{
    soc_reg_above_64_val_t data;
    uint32 total_links, active_links, score[1], entry, ecc[1], mem_row_bit_width;
    uint32 bmp[5];
    uint32 totsf_val, slsct_val, score_slsct, links_count, sctinc_val, sctinc;
    soc_field_t scrub_en;
    uint64 reg_val64;
    int link, array_index;
    uint32 table_entry[5] = {0};
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    scrub_en = soc_reg64_field32_get(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, reg_val64, SCT_SCRUB_ENABLEf); 
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, 0);                       
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

    SOC_REG_ABOVE_64_CLEAR(data);
    /*MCLBTP*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MCLBTPm, data));
    /*MNOLP*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_MNOLPm, data));
    /*RCGLBT*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_RCGLBTm, data));
    /*TOTSF*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_TOTSFm, data));
    /*SLSCT*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_SLSCTm, data));
    /*SCTINC*/
    DNXC_IF_ERR_EXIT(soc_dnxf_clean_rtp_table_array(unit, RTP_SCTINCm, data));

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
        bmp[0] = bmp[1] = bmp[2] = bmp[3] = bmp[4] = 0;
        SHR_BITSET(bmp,link);

        DNXC_IF_ERR_EXIT(WRITE_RTP_RCGLBTm(unit, MEM_BLOCK_ALL, link, bmp));
        DNXC_IF_ERR_EXIT(WRITE_RTP_TOTSFm(unit, MEM_BLOCK_ALL, link, &totsf_val));
        DNXC_IF_ERR_EXIT(WRITE_RTP_SLSCTm(unit, MEM_BLOCK_ALL, link, &slsct_val));
        DNXC_IF_ERR_EXIT(WRITE_RTP_SCTINCm(unit, MEM_BLOCK_ALL, link, &sctinc_val));
        for (array_index = 0; array_index < SOC_DNXF_DEFS_GET(unit, nof_rtp_mclbtp_instances); array_index++) {
            DNXC_IF_ERR_EXIT(WRITE_RTP_MNOLPm(unit, array_index, MEM_BLOCK_ALL, link, table_entry));
            DNXC_IF_ERR_EXIT(WRITE_RTP_MCSFFPm(unit, array_index, MEM_BLOCK_ALL, link, table_entry));
            DNXC_IF_ERR_EXIT(WRITE_RTP_MCLBTPm(unit, array_index, MEM_BLOCK_ALL, link, table_entry));
        }
    }

    for(link = 0 ; link <= soc_mem_index_max(unit, RTP_FFLBPm); link++) {
        for (array_index = 0; array_index < SOC_MEM_NUMELS(unit,RTP_FFLBPm); array_index++) {
            DNXC_IF_ERR_EXIT(WRITE_RTP_FFLBPm(unit, array_index, MEM_BLOCK_ALL, link, table_entry));
        }
    }

    /* 
     * 72X72 Triangle only table which folded to 64X32 + the reset
     *      +
     *      |\
     *      | \
     *      |  \
     *      |   \
     *      |   |\
     *      |   | \
     *      |   |  \
     *      |T1 |T2 \
     *      +---+----+
     *      |   T3   |\ <- T4
     *      +---+------+
     *  
     *  
     */

    /*
    * Original calc
    * Folded address = 
    *     (num_of_active_links >  64) && (total_links_to_same_fap > 64) ?   
    *        2048+(72%64)*64 + ((total_links_to_same_fap-1)%8)*8 + ((num_of_active_links-1)%8) :
    *      (num_of_active_links <=  64) && (total_links_to_same_fap > 64) ?   
    *        (2048+((total_links_to_same_fap-1)%8)*64+ (num_of_active_links -1) :    
    *    (num_of_active_links < 33) ? 
    *                 (((total_links_to_same_fap - 1) * 32) + (num_of_active_links -1)); 
    *     ( ( (64- total_links_to_same_fap )*32) + (64 - num_of_active_links) )  
    */
#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
    LOG_CLI((BSL_META_U(unit,
                        "MUL TABLE\n\n")));
#endif

#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
    LOG_CLI((BSL_META_U(unit,
                        "%02d || "), 0));
#endif
    for(active_links = 1 ; active_links <= 72 ; active_links++) {
#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
        LOG_CLI((BSL_META_U(unit,
                            "%04d | "), active_links));
#endif
    }
#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
    LOG_CLI((BSL_META_U(unit,
                        "\n\n")));
#endif
    for(total_links = 1 ; total_links <= _SOC_RAMON_DRV_MULTIPLIER_TABLE_MAX_LINKS ; total_links++) {

#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
        LOG_CLI((BSL_META_U(unit,
                            "%02d || "), total_links));
#endif
        for(active_links = 1 ; active_links <= total_links ; active_links++) {
            *score = (SOC_DNXF_DRV_MULTIPLIER_MAX_LINK_SCORE * active_links) / total_links;
            if ((SOC_DNXF_DRV_MULTIPLIER_MAX_LINK_SCORE * active_links) % total_links != 0) {
                (*score)++;
            }
            if (total_links >= _SOC_RAMON_DRV_MULTIPLIER_T4_MIN_TOTAL_LINKS && active_links >= _SOC_RAMON_DRV_MULTIPLIER_T4_MIN_ACTIVE_LINKS) 
            {
                /*T4 entries*/
                entry = _SOC_RAMON_DRV_MULTIPLIER_T1_AND_T2_SIZE + _SOC_RAMON_DRV_MULTIPLIER_T3_SIZE
                    + ((total_links - 1) % _SOC_RAMON_DRV_MULTIPLIER_T4_LENGTH)*_SOC_RAMON_DRV_MULTIPLIER_T4_WIDTH
                    + (active_links - 1) % _SOC_RAMON_DRV_MULTIPLIER_T4_WIDTH;
                        
            } else if (total_links >= _SOC_RAMON_DRV_MULTIPLIER_T3_MIN_TOTAL_LINKS && active_links <= _SOC_RAMON_DRV_MULTIPLIER_T3_MAX_ACTIVE_LINKS)
            {
                /*T3 enteries*/
                entry = _SOC_RAMON_DRV_MULTIPLIER_T1_AND_T2_SIZE 
                    + ((total_links - 1) % _SOC_RAMON_DRV_MULTIPLIER_T3_LENGTH)*_SOC_RAMON_DRV_MULTIPLIER_T3_WIDTH
                    + active_links - 1;
            } else if (active_links <= _SOC_RAMON_DRV_MULTIPLIER_T1_MAX_ACTIVE_LINKS)
            {
                /*T1 enteries*/
                entry = (total_links - 1)*_SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH + active_links - 1;
            } else {
                /*T2 enteries*/
                entry = (_SOC_RAMON_DRV_MULTIPLIER_T1_LENGTH - total_links)* _SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH
                    + _SOC_RAMON_DRV_MULTIPLIER_T1_WIDTH  + _SOC_RAMON_DRV_MULTIPLIER_T2_WIDTH - active_links;
            }
#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
            LOG_CLI((BSL_META_U(unit,
                                "%04d | "), entry));
#endif
            mem_row_bit_width = soc_mem_entry_bits(unit, RTP_MULTI_TBm) - soc_mem_field_length(unit, RTP_MULTI_TBm, ECCf);
            DNXC_IF_ERR_EXIT(calc_ecc(unit, mem_row_bit_width, score, ecc));
            soc_mem_field32_set(unit, RTP_MULTI_TBm, score, ECCf, *ecc);
            DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_TBm(unit, MEM_BLOCK_ALL, entry, score));
        }
#if _SOC_RAMON_MULTIPLIER_TABLE_DUMP
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
#endif
    }
    
    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
    soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, scrub_en);                       
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_set_fmac_config(int unit)
{
    uint32 reg_val32, field[1];
    int i;
    int link, blk, inner_link;
    soc_dnxf_fabric_link_device_mode_t link_mode;
    int nof_links_in_mac;
    DNXC_INIT_FUNC_DEFS;

    nof_links_in_mac = SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);

    /*FMAC Leaky bucket configuration*/
    DNXC_IF_ERR_EXIT(READ_FMAC_LEAKY_BUCKET_CONTROL_REGISTERr(unit, 0, &reg_val32));
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_FILL_RATEf, SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_LINK_UP_THf, 0x20);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, BKT_LINK_DN_THf, 0x10);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, SIG_DET_BKT_RST_ENAf, 0x1);
    soc_reg_field_set(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr, &reg_val32, ALIGN_LCK_BKT_RST_ENAf, 0x1);
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_LEAKY_BUCKET_CONTROL_REGISTERr(unit, reg_val32));
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_LEAKY_BUCKET_CONTROL_REGISTERr(unit, reg_val32));


    /*Comma burst configuration*/
    if (SOC_DNXF_IS_FE13(unit))
    {
       PBMP_SFI_ITER(unit, link)
       {
           DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_link_device_mode_get,(unit, link, 0/*tx*/, &link_mode)));
           DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_link_to_block_mapping, (unit, link, &blk, &inner_link, SOC_BLK_FMAC)));

           DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, blk, inner_link, &reg_val32));
           soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_BYTE_MODEf, 0x1);
           if (link_mode == soc_dnxf_fabric_link_device_mode_multi_stage_fe1)
           {
               soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, SOC_RAMON_PORT_COMMA_BURST_PERIOD_FE1);
               soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, SOC_RAMON_PORT_COMMA_BURST_SIZE_FE1);
           } else {
               soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, SOC_RAMON_PORT_COMMA_BURST_PERIOD_FE3);
               soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, SOC_RAMON_PORT_COMMA_BURST_SIZE_FE3);
           }
           DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, blk, inner_link, reg_val32));
       }
    } else {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 0, 0, &reg_val32));
        soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_BYTE_MODEf, 0x1);
        if (SOC_DNXF_IS_REPEATER(unit)) {
            soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, SOC_RAMON_PORT_COMMA_BURST_PERIOD_REPEATER);
            soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, SOC_RAMON_PORT_COMMA_BURST_SIZE_REPEATER);
        } else {
            soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_TX_PERIODf, SOC_RAMON_PORT_COMMA_BURST_PERIOD_FE2);
            soc_reg_field_set(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr, &reg_val32, FMAL_N_CM_BRST_SIZEf, SOC_RAMON_PORT_COMMA_BURST_SIZE_FE2);
        }

        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 0, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 1, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 2, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 3, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 0, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 1, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 2, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FMAL_TX_COMMA_BURST_CONFIGURATIONr(unit, 3, reg_val32));
       
    }
    
    if(SOC_DNXF_IS_FE13(unit)) {

        PBMP_SFI_ITER(unit, link)
        {

            DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_link_device_mode_get,(unit, link, 0/*tx*/, &link_mode)));
            if (link_mode == soc_dnxf_fabric_link_device_mode_multi_stage_fe1)
            {
                DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_link_to_block_mapping, (unit, link, &blk, &inner_link, SOC_BLK_FMAC)));

                DNXC_IF_ERR_EXIT(READ_FMAC_LINK_TOPO_MODE_REG_0r(unit, blk, &reg_val32));
                *field = soc_reg_field_get(unit, FMAC_LINK_TOPO_MODE_REG_0r, reg_val32, LINK_TOPO_MODE_0f);
                SHR_BITSET(field, inner_link);
                soc_reg_field_set(unit, FMAC_LINK_TOPO_MODE_REG_0r, &reg_val32, LINK_TOPO_MODE_0f, *field);

                *field = soc_reg_field_get(unit, FMAC_LINK_TOPO_MODE_REG_0r, reg_val32, LINK_TOPO_MODE_1f);
                SHR_BITCLR(field, inner_link);
                soc_reg_field_set(unit, FMAC_LINK_TOPO_MODE_REG_0r, &reg_val32, LINK_TOPO_MODE_1f, *field);

                DNXC_IF_ERR_EXIT(WRITE_FMAC_LINK_TOPO_MODE_REG_0r(unit, blk, reg_val32));
            }
        }
    } else if (SOC_DNXF_IS_REPEATER(unit))
    {
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_LINK_TOPO_MODE_REG_0r(unit,0));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_LINK_TOPO_MODE_REG_0r(unit,0));
    }

    /*Enable llfc by default*/
    reg_val32 = 0;
    soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val32, LNK_LVL_FC_RX_ENf, 0xf);
    soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val32, LNK_LVL_FC_TX_ENf, 0xf);
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, reg_val32));
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, reg_val32));

    /*Enable RX_LOS_SYNC interrupt*/
    for(i=0; i<nof_links_in_mac; i++) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FPS_CONFIGURATION_RX_SYNCr(unit, REG_PORT_ANY, i, &reg_val32));
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val32 ,FPS_N_RX_SYNC_FORCE_LCK_ENf, 0);
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val32 ,FPS_N_RX_SYNC_FORCE_SLP_ENf, 0);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FPS_CONFIGURATION_RX_SYNCr(unit, i, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FPS_CONFIGURATION_RX_SYNCr(unit, i, reg_val32));
    }

    /*Enable Mac-Tx pump when leaky bucket is down*/
    for(i=0; i<nof_links_in_mac; i++) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_TX_GENERAL_CONFIGURATIONr_REG32(unit, REG_PORT_ANY, i, &reg_val32));
        soc_reg_field_set(unit, FMAC_FMAL_TX_GENERAL_CONFIGURATIONr, &reg_val32 ,FMAL_N_TX_PUMP_WHEN_LB_DNf, 1);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_FMAL_TX_GENERAL_CONFIGURATIONr(unit, i, reg_val32));
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_FMAL_TX_GENERAL_CONFIGURATIONr(unit, i, reg_val32));
    }

    /*Enable interleaving*/
    
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_AC_REG_0102r(unit, 0xf));
    DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_BD_REG_0102r(unit, 0xf));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_set_fsrd_config(int unit)
{
    uint32 reg_val32;
    DNXC_INIT_FUNC_DEFS;
    if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit) || SOC_DNXF_IS_FE2(unit))
    {
        if (SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_26 != -1)
        {
            DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_CTRLr(unit, 8, 2, &reg_val32)); /* 8*3 + 2 */
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val32, SRD_QUAD_N_LCREF_ENf, 0);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, 8, 2, reg_val32));       
        }
        if (SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_35 != -1)
        {
            DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_CTRLr(unit, 11, 2, &reg_val32)); /* 11*3 + 2 */
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val32, SRD_QUAD_N_LCREF_ENf, 0);
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, 11, 2, reg_val32));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_set_ccs_config(int unit)
{
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_FUNC_RETURN;
}


STATIC int
soc_ramon_set_rtp_config(int unit)
{
    uint32 reg_val32;
    uint32 core_clock_speed;
    uint32 rtpwp;
    uint32 wp_at_core_clock_steps;
    DNXC_INIT_FUNC_DEFS;
    
    core_clock_speed = SOC_DNXF_CONFIG(unit).core_clock_speed;

     /*RTPWP CALC*/
    wp_at_core_clock_steps = ((SOC_DNXF_IMP_DEFS_GET(unit, rtp_reachabilty_watchdog_rate)/ 1000)/*micro sec*/ * core_clock_speed /*KHz*/) / 1000 /*convert micro sec to mili */;
    rtpwp = wp_at_core_clock_steps/4096;
    rtpwp = (rtpwp * 4096 < wp_at_core_clock_steps) ? rtpwp+1 : rtpwp; /*ceiling*/

    DNXC_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr_REG32(unit, &reg_val32));
    soc_reg_field_set(unit, RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr, &reg_val32, RTPWPf, rtpwp);
    DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_PROCESSOR_CONFIGURATIONr_REG32(unit, reg_val32));

    if (SOC_DNXF_IS_FE13(unit)){
        DNXC_IF_ERR_EXIT(READ_RTP_ALL_REACHABLE_CFGr(unit, &reg_val32));
        soc_reg_field_set(unit, RTP_ALL_REACHABLE_CFGr, &reg_val32, ALRC_ENABLE_SLOW_LINK_DOWNf, 1);
        DNXC_IF_ERR_EXIT(WRITE_RTP_ALL_REACHABLE_CFGr(unit, reg_val32));
    }


    DNXC_IF_ERR_EXIT(soc_ramon_stk_module_max_fap_set(unit, SOC_RAMON_STK_MAX_MODULE));

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_attach_links_to_default_fifo(int unit)
{
    int i, all_links[SOC_RAMON_NOF_LINKS];

    DNXC_INIT_FUNC_DEFS

    for (i = 0 ; i < SOC_DNXF_DEFS_GET(unit, nof_links); i++)
    {
        all_links[i] = i;
    }

    DNXC_IF_ERR_EXIT(soc_ramon_fabric_links_link_type_set(unit, bcmFabricPipeAll, soc_dnxf_fabric_link_fifo_type_index_0, 1, 1, 1, 1, SOC_DNXF_DEFS_GET(unit, nof_links), all_links));

exit:
    DNXC_FUNC_RETURN;

}

#define SOC_RAMON_DCH_REG_01BE_FIELD_4_24_FE3_DEF       (0x1200)
#define SOC_RAMON_DCH_REG_01BE_FIELD_4_24_FE1_FE2_DEF   (0x400)

STATIC int
soc_ramon_set_dch_config(int unit)
{
    uint64 reg_val64;
    uint32 reg_val32;
    int  i, nof_instances_dch; 
    soc_dnxc_fabric_pipe_t pipes[3];
    soc_dnxf_drv_dch_default_thresholds_t dch_thresholds_default_values;

    DNXC_INIT_FUNC_DEFS;

    nof_instances_dch = SOC_DNXF_DEFS_GET(unit, nof_instances_dch);
    
    for (i=0; i < 3 ; i++)  /* iterate over all possible pipes and create an array  {bcmFabricPipe0,bcmFabricPipe1,bcmFabricPipe2} */ 
    {
        SOC_DNXC_FABRIC_PIPE_INIT(pipes[i]);
        SOC_DNXC_FABRIC_PIPE_SET(&(pipes[i]), i);
    }

    soc_ramon_init_dch_thresholds_config(unit, &dch_thresholds_default_values);

    for (i=0; i < 3 ; i++)  /* iterate over all possible pipes*/
    {
        /* init fifo size threshold*/

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dch_thresholds_default_values.fifo_size[i] ));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dch_thresholds_default_values.fifo_size[i] ));

        /* set bcmFabricLinkRxFifoLLFC threshold */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_llfc_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dch_thresholds_default_values.llfc_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_llfc_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dch_thresholds_default_values.llfc_threshold[i]));

        /* set bcmFabricLinkRxMcLowPrioDrop threshold */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_multicast_low_prio_drop_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dch_thresholds_default_values.mc_low_prio_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_multicast_low_prio_drop_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dch_thresholds_default_values.mc_low_prio_threshold[i]));

        /* set bcmFabricLinkRxFull threshold*/

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dch_thresholds_default_values.full_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_rx_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dch_thresholds_default_values.full_threshold[i]));
    }

    /*ALUWP config*/
    for (i=0 ; i < nof_instances_dch ; i++) {     
        DNXC_IF_ERR_EXIT(READ_DCH_DCH_ENABLERS_REGISTER_2r(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_DCH_ENABLERS_REGISTER_2r, &reg_val32, FIELD_0_7f, 0Xfd);
        DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_ENABLERS_REGISTER_2r(unit, i, reg_val32));
    }


    /* set "WdeifmenP0" bit in LLFC registers */
    for (i=0 ; i < nof_instances_dch ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_P_0r(unit, i, &reg_val64));
        soc_reg64_field32_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_P_0r, &reg_val64, FIELD_36_36f, 1);
        DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_P_0r(unit, i, reg_val64));

        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_P_1r(unit, i, &reg_val64));
        soc_reg64_field32_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_P_1r, &reg_val64, FIELD_36_36f, 1);
        DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_P_1r(unit, i, reg_val64));

        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_P_2r(unit, i, &reg_val64));
        soc_reg64_field32_set(unit, DCH_LINK_LEVEL_FLOW_CONTROL_P_2r, &reg_val64, FIELD_36_36f, 1);
        DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_P_2r(unit, i, reg_val64));
    }
   
    /* set wfq priorities */
    for (i=0 ; i < nof_instances_dch ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCH_PIPES_WEIGHTS_REGISTERr(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_PIPES_WEIGHTS_REGISTERr, &reg_val32, CONFIG_0f, _SOC_RAMON_WFQ_PIPES_PRIORITY_INIT_VALUE);
        DNXC_IF_ERR_EXIT(WRITE_DCH_PIPES_WEIGHTS_REGISTERr(unit, i, reg_val32));
    }

    for (i=0 ; i < nof_instances_dch ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCH_PRIORITY_TRANSLATIONr(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCH_PRIORITY_TRANSLATIONr, &reg_val32, FIELD_4_5f, 3);
        soc_reg_field_set(unit, DCH_PRIORITY_TRANSLATIONr, &reg_val32, FIELD_6_7f, 3);
        soc_reg_field_set(unit, DCH_PRIORITY_TRANSLATIONr, &reg_val32, FIELD_8_9f, 3);
        DNXC_IF_ERR_EXIT(WRITE_DCH_PRIORITY_TRANSLATIONr(unit, i, reg_val32));
    }

    /*Internal thresholds*/
    if (SOC_DNXF_IS_REPEATER(unit)) {
        for (i=0 ; i < nof_instances_dch; i++) {    
            DNXC_IF_ERR_EXIT(READ_DCH_REG_01BEr(unit, i, &reg_val32));
            soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_0_0f, 0X0); /*disable internal threshold*/
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_01BEr(unit, i, reg_val32));
        }

    } else if (SOC_DNXF_IS_FE2(unit)) {
        for (i=0 ; i < nof_instances_dch; i++) {    
            DNXC_IF_ERR_EXIT(READ_DCH_REG_01BEr(unit, i, &reg_val32));
            soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_0_0f, 0X1); /*enable internal threshold*/
            soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_4_22f, SOC_RAMON_DCH_REG_01BE_FIELD_4_24_FE1_FE2_DEF);
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_01BEr(unit, i, reg_val32));
        }
    } else if (SOC_DNXF_IS_FE13(unit)) {
        /*FE1 threshold*/
        for (i=0 ; i < nof_instances_dch/2; i++) {     
            DNXC_IF_ERR_EXIT(READ_DCH_REG_01BEr(unit, i, &reg_val32));
            soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_0_0f, 0X1); /*enable internal threshold*/
            soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_4_22f, SOC_RAMON_DCH_REG_01BE_FIELD_4_24_FE1_FE2_DEF);
            DNXC_IF_ERR_EXIT(WRITE_DCH_REG_01BEr(unit, i, reg_val32));
        }

        /*FE3 threshold*/
        if (SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system)
        {
            for (i=nof_instances_dch/2 ; i < nof_instances_dch; i++) {    
                DNXC_IF_ERR_EXIT(READ_DCH_REG_01BEr(unit, i, &reg_val32));
                soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_0_0f, 0X0); /*disable internal threshold*/
                DNXC_IF_ERR_EXIT(WRITE_DCH_REG_01BEr(unit, i, reg_val32));
            }
        } else {
            for (i=nof_instances_dch/2 ; i < nof_instances_dch; i++) {    
                DNXC_IF_ERR_EXIT(READ_DCH_REG_01BEr(unit, i, &reg_val32));
                soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_0_0f, 0X1); /*enable internal threshold*/
                soc_reg_field_set(unit, DCH_REG_01BEr, &reg_val32, FIELD_4_22f, SOC_RAMON_DCH_REG_01BE_FIELD_4_24_FE3_DEF);
                DNXC_IF_ERR_EXIT(WRITE_DCH_REG_01BEr(unit, i, reg_val32));
            }
        }
    }

           
exit:
    DNXC_FUNC_RETURN;
}


STATIC int
soc_ramon_set_dcm_config(int unit)
{
    int i, nof_instances_dcm; 
    uint32 reg_val32;
    soc_dnxc_fabric_pipe_t pipes[3];
    soc_dnxf_drv_dcm_default_thresholds_t dcm_thresholds_default_values;
    DNXC_INIT_FUNC_DEFS;

    for (i=0; i < 3 ; i++)  /* iterate over all possible pipes and create an array  {bcmFabricPipe0,bcmFabricPipe1,bcmFabricPipe2} */ 
    {
        SOC_DNXC_FABRIC_PIPE_INIT(pipes[i]);
        SOC_DNXC_FABRIC_PIPE_SET(&(pipes[i]), i);
    }

    nof_instances_dcm = SOC_DNXF_DEFS_GET(unit, nof_instances_dcm);

    soc_ramon_init_dcm_thresholds_config(unit, &dcm_thresholds_default_values);


    for (i=0 ; i < SOC_DNXF_MAX_NOF_PIPES; i++) /* iterate over all possible pipes*/
    {
       

        /* init fifo size threshold*/
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.fifo_size[i], 0));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.fifo_size[i], 0));

        /* set bcmFabricLinkMidAlmostFull */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 
                                                                                        -1 /*both almost full thresholds*/, 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.almost_full_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], -1 /*both almost full thresholds*/, 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.almost_full_threshold[i]));

        /* set bcmFabricLinkMidPrio[0-3]Drop*/

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio0Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_0_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio0Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_0_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio1Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_1_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio1Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_1_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio2Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_2_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio2Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_2_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio3Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_3_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio3Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.prio_3_threshold[i]));

        /* set bcmFabricLinkMidGciLvl[1-3]FC */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_low_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_low_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_med_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_med_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_high_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_gci_threshold_set(unit, bcmFabricLinkMidGciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.gci_high_threshold[i]));

        /* set bcmFabricLinkMidRciLvl[1-3]FC */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_low_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_low_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_med_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_med_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_high_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_rci_threshold_set(unit, bcmFabricLinkMidRciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.rci_high_threshold[i]));

        /* set DCM Full thresholds */
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.full_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.full_threshold[i]));
        
        /* disable low priority replications drops in DCM: HW init value of DCM_MC_LOW_PRIO_REP_THD_P_TYPE_0r MC_LOW_PRIO_REP_EN_P_N_TYPE_0f -> 0 */
    }

    /*thresholds for local switch*/
    if (SOC_DNXF_IS_FE13(unit) && SOC_DNXF_CONFIG(unit).fabric_local_routing_enable)
    {
        /*Assign local switch FIFOs to type index 1*/
        /*dcm0-1 fifos 0-1 and dcm2-3 fifos 2-3 are dedicated to local switching*/
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 0 /*dcm_instance*/, 0/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 0 /*dcm_instance*/, 1/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 1 /*dcm_instance*/, 0/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 1 /*dcm_instance*/, 1/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 2 /*dcm_instance*/, 2/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 2 /*dcm_instance*/, 3/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 3 /*dcm_instance*/, 2/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_type_set(unit, 3 /*dcm_instance*/, 3/*fifo_index*/, soc_dnxf_fabric_link_fifo_type_index_1));

        for (i=0 ; i < SOC_DNXF_MAX_NOF_PIPES; i++) /* iterate over all possible pipes*/
        {
            /*Adjust thresholds for local switching*/
            /*fifo size*/
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.local_switch_fifo_size[i], 0));

            /*almost full - just for dcm0-1 fifos 0-1*/
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 
                                                                                            0 /*almost full 0*/, 0 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.local_switch_almost_full_0_threshold[i]));
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 
                                                                                            1 /*almost full 0*/, 0 /*is_fe1*/, 1/*is_fe3*/, dcm_thresholds_default_values.local_switch_almost_full_1_threshold[i]));

             /* set bcmFabricLinkMidPrio[0-3]Drop*/
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio0Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, SOC_RAMON_DCM_MAX_THRESHOLD));
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio1Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, SOC_RAMON_DCM_MAX_THRESHOLD));
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio2Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, SOC_RAMON_DCM_MAX_THRESHOLD));
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_prio_drop_threshold_set(unit, bcmFabricLinkMidPrio3Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, SOC_RAMON_DCM_MAX_THRESHOLD));

            /* set DCM Full thresholds */
            DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_mid_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1 /*is_fe1*/, 1/*is_fe3*/, SOC_RAMON_DCM_MAX_THRESHOLD));
        }
        

    }

    /* set wfq priorities */
    for (i=0 ; i < nof_instances_dcm ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCM_PIPES_WEIGHTS_REGISTERr(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCM_PIPES_WEIGHTS_REGISTERr, &reg_val32, CONFIG_0f, _SOC_RAMON_WFQ_PIPES_PRIORITY_INIT_VALUE);
        DNXC_IF_ERR_EXIT(WRITE_DCM_PIPES_WEIGHTS_REGISTERr(unit, i, reg_val32));
    }
exit:
    DNXC_FUNC_RETURN;
}





STATIC int
soc_ramon_set_dcl_config(int unit)
{
    int i, nof_instances_dcl; 
    uint32 reg_val32;
    soc_dnxc_fabric_pipe_t pipes[3];
    soc_dnxf_drv_dcl_default_thresholds_t dcl_thresholds_default_values;
    DNXC_INIT_FUNC_DEFS;

    nof_instances_dcl = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl);

    for (i=0; i < 3 ; i++)  /* iterate over all possible pipes and create an array  {bcmFabricPipe0,bcmFabricPipe1,bcmFabricPipe2} */ 
    {
        SOC_DNXC_FABRIC_PIPE_INIT(pipes[i]);
        SOC_DNXC_FABRIC_PIPE_SET(&(pipes[i]), i);
    }
    soc_ramon_init_dcl_thresholds_config(unit, &dcl_thresholds_default_values);


    for (i=0; i < 3; i++)
    {
        /* init fifo sizes*/

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.fifo_size[i], 0));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_fifo_size_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.fifo_size[i], 0));


        /* set bcmFabricLinkFE1TxBypassLLFC */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_bypass_llfc_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.llfc_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_bypass_llfc_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.llfc_threshold[i]));

        /* set bcmFabricLinkTxAlmostFull */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.almost_full_threshold[i], 1));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_almost_full_threshold_set(unit, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.almost_full_threshold[i], 1));

        /* set bcmFabricLinkTxGciLvl[1-3]FC */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.gci_low_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.gci_low_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.gci_med_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.gci_med_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.gci_high_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_gci_threshold_set(unit, bcmFabricLinkTxGciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.gci_high_threshold[i]));

        /* set bcmFabricLinkTxRciLvl[1-3]FC */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.rci_low_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl1FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.rci_low_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.rci_med_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl2FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.rci_med_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.rci_high_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_rci_threshold_set(unit, bcmFabricLinkTxRciLvl3FC, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.rci_high_threshold[i]));

        /* set bcmFabricLinkTxPrio[0-3]Drop */

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio0Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.prio_0_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio0Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.prio_0_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio1Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.prio_1_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio1Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.prio_1_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio2Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.prio_2_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio2Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.prio_2_threshold[i]));

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio3Drop, soc_dnxf_fabric_link_fifo_type_index_0, pipes[i], 1, 1, dcl_thresholds_default_values.prio_3_threshold[i]));
        DNXC_IF_ERR_EXIT(soc_ramon_fabric_flow_control_tx_drop_threshold_set(unit, bcmFabricLinkTxPrio3Drop, soc_dnxf_fabric_link_fifo_type_index_1, pipes[i], 1, 1, dcl_thresholds_default_values.prio_3_threshold[i]));
    }

    /* set wfq priorities */
    for (i=0 ; i < nof_instances_dcl ; i++)
    {
        DNXC_IF_ERR_EXIT(READ_DCL_SHAPER_AND_WFQ_CFGr(unit, i, &reg_val32));
        soc_reg_field_set(unit, DCL_SHAPER_AND_WFQ_CFGr, &reg_val32, CONFIG_0f, _SOC_RAMON_WFQ_PIPES_PRIORITY_INIT_VALUE);
        DNXC_IF_ERR_EXIT(WRITE_DCL_SHAPER_AND_WFQ_CFGr(unit, i, reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;
}

STATIC int
soc_ramon_set_eci_config(int unit)
{
    uint32 reg32_val;
    int rv;
    DNXC_INIT_FUNC_DEFS;
    
    

    rv = READ_ECI_GLOBAL_GENERAL_CFG_1r(unit, &reg32_val);
    DNXC_IF_ERR_EXIT(rv);
    if(SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system) {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, FIELD_1_2f, 1);
    } else if (SOC_DNXF_CONFIG(unit).system_contains_multiple_pipe_device) {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, FIELD_1_2f, 2);
    } else {
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_1r, &reg32_val, FIELD_1_2f, 0);
    }
    rv = WRITE_ECI_GLOBAL_GENERAL_CFG_1r(unit, reg32_val);
    DNXC_IF_ERR_EXIT(rv);
    
    /*PVT configuration*/
    DNXC_IF_ERR_EXIT(soc_ramon_drv_pvt_monitor_enable(unit));

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_reset_device(int unit)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    /* cmic regs reset function */
    rc =soc_ramon_reset_cmic_iproc_regs(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* interrupt init*/
    rc = soc_ramon_interrupts_init(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* blocks reset*/
    rc = soc_ramon_drv_blocks_reset(unit, 0 /*full reset*/ , NULL);
    DNXC_IF_ERR_EXIT(rc);

    /*PLL configueation*/
    rc = soc_ramon_drv_pll_config_set(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*
     * Memory bist
     */
    if (SOC_DNXF_CONFIG(unit).run_mbist) {
         rc = soc_ramon_bist_all(unit, SOC_DNXF_CONFIG(unit).run_mbist - 1);
        DNXC_IF_ERR_EXIT(rc);
        
       /*Rerun reset sequance*/
        rc =soc_ramon_drv_blocks_reset(unit, 0 /*full reset*/ , NULL);
        DNXC_IF_ERR_EXIT(rc);
    }

    /*Blocks broadcast*/
    rc = soc_ramon_drv_sbus_broadcast_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*soft reset - into reset*/
    rc = soc_ramon_drv_soft_init(unit, SOC_DNXC_RESET_ACTION_IN_RESET);
    DNXC_IF_ERR_EXIT(rc);

    /* interrupt init*/
    rc = soc_ramon_interrupts_disable(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set operation mode function */
    rc = soc_ramon_set_operation_mode(unit);
    DNXC_IF_ERR_EXIT(rc);
    

    /* reset tables function*/
    rc = soc_ramon_reset_tables(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set fmac config function*/
    rc = soc_ramon_set_fmac_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set fsrd config function */
    rc = soc_ramon_set_fsrd_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set ccs config function */
    rc = soc_ramon_set_ccs_config(unit);
    DNXC_IF_ERR_EXIT(rc); 

    /* set mesh topology config function */
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_set_mesh_topology_config ,(unit));
    DNXC_IF_ERR_EXIT(rc);    

    /* set rtp config function */
    rc = soc_ramon_set_rtp_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* attach links to default fifo type */

    rc = soc_ramon_attach_links_to_default_fifo(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set dch config function */
    rc = soc_ramon_set_dch_config(unit);
    DNXC_IF_ERR_EXIT(rc);    

    /* set dcm config function */
    rc = soc_ramon_set_dcm_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* set dcl config function */
    rc = soc_ramon_set_dcl_config(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*set eci config function*/
    rc = soc_ramon_set_eci_config(unit);
    DNXC_IF_ERR_EXIT(rc);

     /*Set default low priority drop select*/    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_multicast_low_prio_drop_select_priority_set,(unit, soc_dnxf_fabric_priority_0));
    DNXC_IF_ERR_EXIT(rc);

    /*soft reset - out of reset*/
    rc = soc_ramon_drv_soft_init(unit, SOC_DNXC_RESET_ACTION_OUT_RESET);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}


int
soc_ramon_drv_pll_config_set(int unit)
{
    int lcpll;
    uint32 reg32_val;
    soc_reg_above_64_val_t reg_above_64_val;
    int lcpll_in, lcpll_out;
    soc_reg_t pll_config_reg[] = {ECI_MISC_PLL_0_CONFIGr, ECI_MISC_PLL_1_CONFIGr, ECI_MISC_PLL_2_CONFIGr, ECI_MISC_PLL_3_CONFIGr};
    DNXC_INIT_FUNC_DEFS;


    /*Check if PLL configuration is possible*/
    DNXC_IF_ERR_EXIT(READ_ECI_POWERUP_CONFIGr_REG32(unit, &reg32_val));

    if (SHR_BITGET(&reg32_val, SOC_RAMON_PORT_ECI_POWER_UP_CONFIG_STATIC_PLL_BIT) && !SAL_BOOT_PLISIM)
    {
        /*PLL static configuration - the PLL was set at power up*/
        SOC_EXIT;
    }


    for(lcpll=0 ; lcpll<SOC_DNXF_DEFS_GET(unit, nof_lcpll) ; lcpll++) 
    {
        lcpll_in = SOC_DNXF_CONFIG(unit).fabric_port_lcpll_in[lcpll];
        lcpll_out = SOC_DNXF_CONFIG(unit).fabric_port_lcpll_out[lcpll];
        
        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
        switch(lcpll_out) {
            case soc_dnxc_init_serdes_ref_clock_125:
                SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_OUT_125_MHZ_WORD_0, 0);
                break;
            case soc_dnxc_init_serdes_ref_clock_156_25:
                SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_OUT_156_25_MHZ_WORD_0, 0);
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("lcpll_out: %d is out-of-ranget (use 0=125MHz, 1=156.25MHz"), lcpll_out));
                break;
        }

        switch(lcpll_in) {
            case soc_dnxc_init_serdes_ref_clock_125:
                SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_IN_125_MHZ_WORD_1, 1);
                break;
            case soc_dnxc_init_serdes_ref_clock_156_25:
                SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_IN_156_25_MHZ_WORD_1, 1);
                break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("lcpll_out: %d is out-of-ranget (use 0=125MHz, 1=156.25MHz"), lcpll_out));
            break;
        }

        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_2, 2);
        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_3, 3);
        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_4, 4);
        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_5_STAGE1, 5);
        

        /*Stage 1*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, pll_config_reg[lcpll], REG_PORT_ANY, 0, reg_above_64_val));

        /*Stage 2*/
        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_5_STAGE2, 5);
        DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, pll_config_reg[lcpll], REG_PORT_ANY, 0, reg_above_64_val));
        
        /*Stage 3*/
        SOC_REG_ABOVE_64_WORD_SET(reg_above_64_val, SOC_RAMON_PORT_PLL_CONFIG_DEFAULT_WORD_5_STAGE3, 5);
        DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, pll_config_reg[lcpll], REG_PORT_ANY, 0, reg_above_64_val));
    }

exit:
    DNXC_FUNC_RETURN; 
}

int
soc_ramon_drv_mdio_config_set(int unit)
{
    int dividend, divisor;
    int mdio_int_freq, mdio_delay;
    uint32 core_clock_speed_mhz;
    DNXC_INIT_FUNC_DEFS;
    
    /* Mdio - internal*/

    /*Dividend values*/
    if (SOC_DNXF_CONFIG(unit).mdio_int_dividend == -1) 
    {
        /*default value*/
        dividend =  SOC_DNXF_IMP_DEFS_GET(unit, mdio_int_dividend_default);
        SOC_DNXF_CONFIG(unit).mdio_int_dividend = dividend;
    } else {
        dividend = SOC_DNXF_CONFIG(unit).mdio_int_dividend;
    }

    if (SOC_DNXF_CONFIG(unit).mdio_int_divisor == -1) 
    {
        /*Calc default dividend and divisor*/
        mdio_int_freq = SOC_DNXF_IMP_DEFS_GET(unit, mdio_int_freq_default);
        core_clock_speed_mhz = SOC_DNXF_CONFIG(unit).core_clock_speed / 1000;

        divisor = core_clock_speed_mhz * dividend / (2* mdio_int_freq);
        SOC_DNXF_CONFIG(unit).mdio_int_divisor = divisor;
    } else {
        divisor = SOC_DNXF_CONFIG(unit).mdio_int_divisor;
    }
    


    mdio_delay = SOC_DNXF_IMP_DEFS_GET(unit, mdio_int_out_delay_default);

    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_mdio_config(unit,dividend,divisor,mdio_delay));


exit:
    DNXC_FUNC_RETURN; 
}

int
soc_ramon_drv_pvt_monitor_enable(int unit)
{
    uint64 reg64_val;
    soc_reg_t pvt_monitors[] = {ECI_PVT_MON_A_CONTROL_REGr, ECI_PVT_MON_B_CONTROL_REGr, ECI_PVT_MON_C_CONTROL_REGr, ECI_PVT_MON_D_CONTROL_REGr};
    int pvt_index;
    DNXC_INIT_FUNC_DEFS;

    /*Init*/
    COMPILER_64_ZERO(reg64_val);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++)
    {
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /*Powerdown*/
    COMPILER_64_BITSET(reg64_val, _SOC_RAMON_ECI_PVT_MON_CONTROL_REG_POWERDOWN_BIT);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++)
    {
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /*Powerup*/
    COMPILER_64_ZERO(reg64_val);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++)
    {
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

    /*Reset*/
    COMPILER_64_BITSET(reg64_val, _SOC_RAMON_ECI_PVT_MON_CONTROL_REG_RESET_BIT);
    for (pvt_index = 0; pvt_index < (sizeof(pvt_monitors) / sizeof(soc_reg_t)); pvt_index++)
    {
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, pvt_monitors[pvt_index], REG_PORT_ANY, 0, reg64_val));
    }

exit:
    DNXC_FUNC_RETURN; 
}

/*
 * SBUS ring map:
 * Ring 0: OTPC(84), AVS(2)
 * Ring 2: DCH(96-103), CCH(104-111), QRH(88-95), MCT(87), RTP(86), MESH_TOPOLOGY(85), BRDC_DCH(122), BRDC_CCH(123), BRDC_QRH(126)
 * Ring 3: DCML(3-6, 8-11), BRDC_DCML(124)
 * Ring 4: FMAC(12-59), FSRD(60-83), BRDC_MAC(120), BRDC_FSRD(121)
 * Ring 5: LCM(112-119), BRDC_LCM(125)
 * Ring 7: ECI(0), CMIC(127)
 */
int
soc_ramon_drv_rings_map_set(int unit)
{
	DNXC_INIT_FUNC_DEFS;

	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x73333027));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x44443333));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x44444444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x22204444));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x22222222));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x22222222));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x22222222));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x55555555));
	DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x72532244));

	exit:
	DNXC_FUNC_RETURN;
}

int
soc_ramon_drv_sbus_broadcast_config(int unit)
{
    int i;
    int nof_blocks;
    int broadcast_id;
    int disabled;

    DNXC_INIT_FUNC_DEFS;

    /*FMAC broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_mac);
    for (i=0;i< nof_blocks;i++)
    {
        DNXC_IF_ERR_EXIT(soc_ramon_drv_mac_broadcast_id_get(unit,i,&broadcast_id));
        DNXC_IF_ERR_EXIT(WRITE_FMAC_SBUS_BROADCAST_IDr(unit, i,broadcast_id));

    }
    
    /*FSRD broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd);
    for (i=0;i<nof_blocks;i++)
    {
        
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_quad_disabled, (unit, i * SOC_DNXF_DEFS_GET(unit, nof_quads_in_fsrd), &disabled)));
        if (!disabled)
        {
            DNXC_IF_ERR_EXIT(WRITE_FSRD_SBUS_BROADCAST_IDr(unit, i, _SOC_RAMON_DRV_BRDC_FSRD));
        }

    }

    /*DCH broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_dch);
    for (i=0;i<nof_blocks;i++)
    {
        DNXC_IF_ERR_EXIT(WRITE_DCH_SBUS_BROADCAST_IDr(unit, i, _SOC_RAMON_DRV_BRDC_DCH));
    }

    /*DCL broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl);
    for (i=0;i<nof_blocks;i++)
    {
        DNXC_IF_ERR_EXIT(WRITE_DCL_SBUS_BROADCAST_IDr(unit, i, _SOC_RAMON_DRV_BRDC_DCL));
    }

    /*DCM broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_dcm);
    for (i=0;i<nof_blocks;i++)
    {
        DNXC_IF_ERR_EXIT(WRITE_DCM_SBUS_BROADCAST_IDr(unit, i, _SOC_RAMON_DRV_BRDC_DCM)); 

    }

    /*CCS broadcast*/
    nof_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_ccs);
    for (i=0;i<nof_blocks;i++)
    {
        DNXC_IF_ERR_EXIT(WRITE_CCS_SBUS_BROADCAST_IDr(unit, i, _SOC_RAMON_DRV_BRDC_CCS));

    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_drv_mac_broadcast_id_get(int unit, int block_num,int *broadcast_id)
{
    
    int nof_mac_blocks;
    DNXC_INIT_FUNC_DEFS;

    nof_mac_blocks = SOC_DNXF_DEFS_GET(unit, nof_instances_mac);

    if (block_num<0 || block_num >= nof_mac_blocks)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL , (_BSL_DNXC_MSG("Invalid mac block number")));
        
    }

    if (block_num<=8 || (block_num>=18 && block_num<=26)) /* mac_02 */
    {
        *broadcast_id=_SOC_RAMON_DRV_BRDC_FMAC_AC_ID;
    }
    else
    {
        *broadcast_id=_SOC_RAMON_DRV_BRDC_FMAC_BD_ID;
    }




exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_drv_soc_properties_validate(int unit)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_drv_soft_init
 * Purpose:
 *      Run blocks soft init
 * Parameters:
 *      unit  -                     (IN)     Unit number.
 *      soft_reset_mode_flags -     (IN)     SOC_DNXC_RESET_ACTION_IN_RESET, SOC_DNXC_RESET_ACTION_OUT_RESET, SOC_DNXC_RESET_ACTION_INOUT_RESET
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_XXX      Error occurred  
 */

int 
soc_ramon_drv_soft_init(int unit, uint32 soft_reset_mode_flags)
{
    soc_reg_above_64_val_t reg_above_64;
    DNXC_INIT_FUNC_DEFS;

    if (soft_reset_mode_flags == SOC_DNXC_RESET_ACTION_IN_RESET || soft_reset_mode_flags == SOC_DNXC_RESET_ACTION_INOUT_RESET)
    {
        SOC_REG_ABOVE_64_ALLONES(reg_above_64);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64));
    }

    if (soft_reset_mode_flags == SOC_DNXC_RESET_ACTION_OUT_RESET || soft_reset_mode_flags == SOC_DNXC_RESET_ACTION_INOUT_RESET)
    {
        SOC_REG_ABOVE_64_CLEAR(reg_above_64);
        DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_drv_test_reg_filter
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
soc_ramon_drv_test_reg_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch (reg) 
    {

       case MESH_TOPOLOGY_ECC_ERR_1B_INITIATEr:
       case MESH_TOPOLOGY_ECC_ERR_1B_MONITOR_MEM_MASKr:
       case MESH_TOPOLOGY_ECC_ERR_2B_INITIATEr:
       case MESH_TOPOLOGY_ECC_ERR_2B_MONITOR_MEM_MASKr:
       case MESH_TOPOLOGY_GLOBAL_MEM_OPTIONSr:
       case MESH_TOPOLOGY_RESERVED_MTCPr:

       case ECI_BLOCKS_POWER_DOWNr:
       case CMIC_CPS_RESETr:
       case ECI_BLOCKS_SOFT_RESETr:
       case ECI_BLOCKS_SOFT_INITr:
       case ECI_BLOCKS_SBUS_RESETr:
       case FMAC_ASYNC_FIFO_CONFIGURATIONr:
       /*In these registers the read value 
          will always be different than write value by design*/
       case ECI_INDIRECT_COMMANDr:
       case FSRD_INDIRECT_COMMANDr:
       case RTP_INDIRECT_COMMANDr:
       case DCL_INDIRECT_COMMANDr:
       case FMAC_INDIRECT_COMMANDr:
       /*SBUS last in chain*/
       case FMAC_SBUS_LAST_IN_CHAINr:
       case FSRD_SBUS_LAST_IN_CHAINr:
       case DCH_SBUS_LAST_IN_CHAINr:
       case DCM_SBUS_LAST_IN_CHAINr:
       case DCMC_SBUS_LAST_IN_CHAINr:
       case DCL_SBUS_LAST_IN_CHAINr:
       case CCS_SBUS_LAST_IN_CHAINr:
       case RTP_SBUS_LAST_IN_CHAINr:
       case MESH_TOPOLOGY_SBUS_LAST_IN_CHAINr:
       case ECI_SBUS_LAST_IN_CHAINr:
       case OCCG_REG_0087r:
       /*the following registers are filtered temporarily- the length of these registers should be amended according to the block they are in 
       (need to be the longest line of a register that is RW instead of always 640 bit)*/
       case DCL_INDIRECT_COMMAND_WR_DATAr:
       case ECI_INDIRECT_COMMAND_WR_DATAr:
       case FSRD_INDIRECT_COMMAND_WR_DATAr:
       case RTP_INDIRECT_COMMAND_WR_DATAr:
       /*the following registers should not exist in data base and therefore filtered*/
			 		
       /*the following registers are filtered because they relate to Pll*/ 
       case ECI_REG_0172r:
       case ECI_REG_016Cr:
       case ECI_PRM_PLL_CONTROL_STATUSr:
       
           *is_filtered = 1;
       default:
            break;
    }

    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_drv_test_reg_default_val_filter
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
soc_ramon_drv_test_reg_default_val_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    /*Don't test initialization registers 
      and registers which aren't consistent
      between blocks of the same type*/
    switch(reg) {
     
      case MESH_TOPOLOGY_ECC_ERR_1B_INITIATEr:
      case MESH_TOPOLOGY_ECC_ERR_1B_MONITOR_MEM_MASKr:
      case MESH_TOPOLOGY_ECC_ERR_2B_INITIATEr:
      case MESH_TOPOLOGY_ECC_ERR_2B_MONITOR_MEM_MASKr:
      case MESH_TOPOLOGY_GLOBAL_MEM_OPTIONSr:
      case MESH_TOPOLOGY_RESERVED_MTCPr:

       /*reset and power*/
       case ECI_BLOCKS_POWER_DOWNr:
       case CMIC_CPS_RESETr:
       case ECI_BLOCKS_SOFT_RESETr:
       case ECI_BLOCKS_SOFT_INITr:
       case ECI_BLOCKS_SBUS_RESETr:
       
       /*indirect*/
       case ECI_INDIRECT_COMMANDr:
       case FSRD_INDIRECT_COMMANDr:
       case RTP_INDIRECT_COMMANDr:
       case DCL_INDIRECT_COMMANDr:
       case FMAC_INDIRECT_COMMANDr:

       /*async fifo*/
       case FMAC_ASYNC_FIFO_CONFIGURATIONr:

       /*SBUS last in chain*/
       case FMAC_SBUS_LAST_IN_CHAINr:
       case FSRD_SBUS_LAST_IN_CHAINr:
       case DCH_SBUS_LAST_IN_CHAINr:
       case DCM_SBUS_LAST_IN_CHAINr:
       case DCMC_SBUS_LAST_IN_CHAINr:
       case DCL_SBUS_LAST_IN_CHAINr:
       case CCS_SBUS_LAST_IN_CHAINr:
       case RTP_SBUS_LAST_IN_CHAINr:
       case MESH_TOPOLOGY_SBUS_LAST_IN_CHAINr:
       case ECI_SBUS_LAST_IN_CHAINr:
       case OCCG_REG_0087r:

       /*FMAC broadcast ID*/
       case FMAC_SBUS_BROADCAST_IDr:
       	
       /*the following registers are filtered temporarily- the length of these registers should be amended according to the block they are in 
       (need to be the longest line of a register that is RW instead of always 640 bit)*/
       case DCL_INDIRECT_COMMAND_WR_DATAr:
       case ECI_INDIRECT_COMMAND_WR_DATAr:
       case FSRD_INDIRECT_COMMAND_WR_DATAr:
       case RTP_INDIRECT_COMMAND_WR_DATAr:
       /*the following registers should not exist in data base and therefore filtered*/
			 		
       /*the following registers are filtered because they relate to Pll*/ 
       case ECI_REG_0172r:
       case ECI_REG_016Cr:
       case ECI_PRM_PLL_CONTROL_STATUSr:
       	
            *is_filtered = 1; /* Skip these registers */
        default:
            break;
    }

    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_drv_test_mem_filter
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
soc_ramon_drv_test_mem_filter(int unit, soc_mem_t mem, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch(mem) 
    {
        case RTP_RMHMTm:
        case RTP_CUCTm:
        case RTP_DUCTPm:
        case RTP_MEM_1100000m:

        
        case FSRD_FSRD_WL_EXT_MEMm:
        case RTP_MULTI_CAST_TABLE_UPDATEm:
            *is_filtered = 1;
            break;
        default:
            break;
    }

    DNXC_FUNC_RETURN; 
}


/*
   [31:20] - SDK version of the last regular init;
   [19:8] - if ISSU took place then set the SDK version after ISSU;
   [7:4] - Did WB take place?(=0x0,0x1) ;
   [3:0] - Did ISSU take place? (=0x0,0x1) 
*/

int
soc_ramon_drv_sw_ver_set(int unit)
{
    int         ver_val[3] = {0,0,0};
    uint32      regval, i, prev_regval;
    char        *ver;
    char        *cur_number_ptr;
    int         rc;
    int         wb, issu, bit_ndx ;
    uint32      prev_ver_val[3] = {0,0,0};

    DNXC_INIT_FUNC_DEFS;
   
    
    regval = 0;
    ver = _build_release;

    cur_number_ptr = sal_strchr(ver, '-');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Invalid version format.")));

    }
    ++cur_number_ptr;
    ver_val[0] = _shr_ctoi (cur_number_ptr);
    cur_number_ptr = sal_strchr(cur_number_ptr, '.');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Invalid version format.")));

    }
    ++cur_number_ptr;
    ver_val[1] = _shr_ctoi (cur_number_ptr);
    cur_number_ptr = sal_strchr(cur_number_ptr, '.');
    if(cur_number_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Invalid version format.")));
    }
    ++cur_number_ptr;
    ver_val[2] = _shr_ctoi (cur_number_ptr);

    DNXC_IF_ERR_EXIT(READ_ECI_SW_VERSIONr(unit, &prev_regval));
    wb = 0;
    issu = 0;
  /*    [31:20] - SDK version of the last regular init */
    if (SOC_WARM_BOOT(unit)) {
        wb = 1;
        bit_ndx = 28;

        for (i=0; i<3; i++) {
            /* Get last regular init version*/
            prev_ver_val[i]= ( prev_regval >> (bit_ndx - i*4)) & 0xf;

            if (prev_ver_val[i] != ver_val[i]) {
                  issu = 1;
            }
            regval = (regval | (0xf & prev_ver_val[i])) << 4;
        }

    } else {
        for (i=0; i<3; i++) {
            regval = (regval | (0xf & ver_val[i])) << 4;
        }
    }


     /* If issu set current version in 19:8*/
     for (i=0; i<3; i++) {
         if (issu) {
             regval = (regval | (0xf & ver_val[i]));
         }
         regval = regval << 4;
     }

     /* 7:4 is wb */
     regval = (regval | (0xf & wb)) << 4;

     /* 3:0 is issu */
     regval = (regval | (0xf & issu));


     SOC_DNXF_ALLOW_WARMBOOT_WRITE(WRITE_ECI_SW_VERSIONr(unit, regval), rc);
     DNXC_IF_ERR_EXIT(rc);

exit:
     DNXC_FUNC_RETURN;;

}

int
soc_ramon_drv_asymmetrical_quad_get(int unit, int link, int *asymmetrical_quad)
{
    int nof_quad;
    DNXC_INIT_FUNC_DEFS;

    nof_quad = INT_DEVIDE(link, SOC_RAMON_NOF_LINKS_IN_QUAD);

    if (nof_quad == SOC_RAMON_ASYMMETRICAL_FE13_QUAD_0)
    {
        *asymmetrical_quad = 0;
    }
    else if (nof_quad == SOC_RAMON_ASYMMETRICAL_FE13_QUAD_1)
    {
        *asymmetrical_quad = 1;
    }
    else
    {
        *asymmetrical_quad = -1;
    }

    DNXC_FUNC_RETURN;
}

int soc_ramon_drv_reg_access_only_reset(int unit)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    /* cmic regs reset function */
    rc =soc_ramon_reset_cmic_iproc_regs(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* blocks reset*/
    rc = soc_ramon_drv_blocks_reset(unit, 0 /*full reset*/ , NULL);
    DNXC_IF_ERR_EXIT(rc);

    /*Blocks broadcast*/
    rc = soc_ramon_drv_sbus_broadcast_config(unit);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_drv_test_brdc_blk_filter
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
soc_ramon_drv_test_brdc_blk_filter(int unit, soc_reg_t reg, int *is_filtered)
{
    DNXC_INIT_FUNC_DEFS;

    *is_filtered = 0;

    switch(reg) 
    {   
        /*FMAC_AC*/
        case BRDC_FMAC_AC_ASYNC_FIFO_CONFIGURATIONr:
        case BRDC_FMAC_AC_SBUS_BROADCAST_IDr:
        case BRDC_FMAC_AC_SBUS_LAST_IN_CHAINr:
        case BRDC_FMAC_AC_GTIMER_TRIGGERr:
        /*FMAC_BD*/
        case BRDC_FMAC_BD_ASYNC_FIFO_CONFIGURATIONr:
        case BRDC_FMAC_BD_SBUS_BROADCAST_IDr:
        case BRDC_FMAC_BD_SBUS_LAST_IN_CHAINr:
        case BRDC_FMAC_BD_GTIMER_TRIGGERr:
        /*FSRD*/
        case BRDC_FSRD_SBUS_BROADCAST_IDr:
        case BRDC_FSRD_SBUS_LAST_IN_CHAINr:
        case BRDC_FSRD_GTIMER_TRIGGERr:
        case BRDC_FSRD_ERROR_INITIATION_DATAr:
        case BRDC_FSRD_INDIRECT_COMMANDr:
        case BRDC_FSRD_INDIRECT_COMMAND_WR_DATAr:
        case BRDC_FSRD_INDIRECT_FORCE_BUBBLEr:

        case BRDC_FSRD_RESERVED_PCMI_0_Tr:
        case BRDC_FSRD_RESERVED_PCMI_1_Tr:
        case BRDC_FSRD_RESERVED_PCMI_2_Tr:
        case BRDC_FSRD_RESERVED_PCMI_4_Tr:


        /*DCH*/
        case BRDC_DCH_SBUS_BROADCAST_IDr:
        case BRDC_DCH_SBUS_LAST_IN_CHAINr:
        case BRDC_DCH_GTIMER_TRIGGERr:
        case BRDC_DCH_DCH_ERROR_INITIATION_DATAr:
        case BRDC_DCH_ERROR_INITIATION_DATAr:
        case BRDC_DCH_GEN_ERR_MEMr:
        /*the following registers are filtered since they dont have bit 0- 
          the test tries to write 0x1 to this reg*/
        case BRDC_DCH_DCH_ENABLERS_REGISTER_1r:
        case BRDC_DCH_PRIORITY_TRANSLATIONr:
        case BRDC_DCH_RESERVED_PCMI_2_Tr:
        case BRDC_DCH_RESERVED_PCMI_4_Tr:
        /*DCM*/
        case BRDC_DCM_SBUS_BROADCAST_IDr:
        case BRDC_DCM_SBUS_LAST_IN_CHAINr:
        case BRDC_DCM_GTIMER_TRIGGERr:
        case BRDC_DCM_ERROR_INITIATION_DATAr:

        case BRDC_DCM_RESERVED_PCMI_0_Tr:
        case BRDC_DCM_RESERVED_PCMI_1_Tr:
        case BRDC_DCM_RESERVED_PCMI_2_Tr:
        case BRDC_DCM_RESERVED_PCMI_4_Tr:


        /*DCL*/
        case BRDC_DCL_SBUS_BROADCAST_IDr:
        case BRDC_DCL_SBUS_LAST_IN_CHAINr:
        case BRDC_DCL_GTIMER_TRIGGERr:
        case BRDC_DCL_ERROR_INITIATION_DATAr:
        case BRDC_DCL_INDIRECT_COMMANDr:
        case BRDC_DCL_RESERVED_PCMI_0_Tr:
        case BRDC_DCL_RESERVED_PCMI_1_Tr:

        case BRDC_DCL_TRANSMIT_DATA_CELL_TRIGGERr:
        case BRDC_DCL_INDIRECT_FORCE_BUBBLEr:
        /*CCS*/
        case BRDC_CCS_SBUS_BROADCAST_IDr:
        case BRDC_CCS_SBUS_LAST_IN_CHAINr:
        case BRDC_CCS_GTIMER_TRIGGERr:
        case BRDC_CCS_CPU_SOURCE_CELL_TRIGGERr:
        case BRDC_CCS_ERROR_INITIATION_DATAr:
                *is_filtered = 1;
                break;
        default:
            break;
    }



    DNXC_FUNC_RETURN; 
}
/*
 * Function:
 *      soc_ramon_drv_test_brdc_blk_info_get
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
soc_ramon_drv_test_brdc_blk_info_get(int unit, int max_size, soc_reg_brdc_block_info_t *brdc_info, int *actual_size)
{
    int instance;
    int i;
    DNXC_INIT_FUNC_DEFS;
    *actual_size = 0;

    /*FMAC_AC*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_FMAC_AC;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 4; instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        for (instance = SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 2; instance < 3 * SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 4; instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*FMAC_BD*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_FMAC_BD;
        for (i = 0, instance = SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 4; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 2; instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        for (instance = 3 * SOC_DNXF_DEFS_GET(unit, nof_instances_mac) / 4; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = FMAC_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*FSRD*/
    if (!(dnxc_device_block_for_feature(unit,DNXC_FABRIC_12_QUADS_FEATURE)) &&
        !(dnxc_device_block_for_feature(unit,DNXC_FABRIC_24_QUADS_FEATURE)) &&
        !(dnxc_device_block_for_feature(unit,DNXC_FABRIC_18_QUADS_FEATURE))) /*SKUs do not support broadcast fsrd*/
    {
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

    }
    /*DCH*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_DCH;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = DCH_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*DCM*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_DCM;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = DCM_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*DCL*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_DCL;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = DCL_BLOCK(unit, instance); 
        }
        brdc_info[*actual_size].blk_ids[i] = -1;

        (*actual_size)++;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Test does not support all device block ids")));
    }

    /*CCS*/
    if (max_size > *actual_size)
    {
        brdc_info[*actual_size].blk_type = SOC_BLK_BRDC_CCS;
        for (i = 0, instance = 0; instance < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); instance++, i++)
        {
            brdc_info[*actual_size].blk_ids[i] = CCS_BLOCK(unit, instance); 
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
soc_ramon_drv_temperature_monitor_get(int unit, int temperature_max, soc_switch_temperature_monitor_t *temperature_array, int *temperature_count)
{
    int i;
    uint32 reg32_val;
    int peak, curr;
    soc_reg_t temp_reg[] = {ECI_PVT_MON_A_THERMAL_DATAr, ECI_PVT_MON_B_THERMAL_DATAr, ECI_PVT_MON_C_THERMAL_DATAr, ECI_PVT_MON_D_THERMAL_DATAr};
    soc_field_t curr_field[] = {THERMAL_DATA_Af, THERMAL_DATA_Bf, THERMAL_DATA_Cf, THERMAL_DATA_Df};
    soc_field_t peak_field[] = {PEAK_THERMAL_DATA_Af, PEAK_THERMAL_DATA_Bf, PEAK_THERMAL_DATA_Cf, PEAK_THERMAL_DATA_Df};
    
    DNXC_INIT_FUNC_DEFS;

    if (temperature_max < _SOC_RAMON_PVT_MON_NOF)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("Array size should be equal or bigger from %d.\n"), _SOC_RAMON_PVT_MON_NOF));
    }

    for (i = 0; i < _SOC_RAMON_PVT_MON_NOF; i++)
    {
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, temp_reg[i], REG_PORT_ANY, 0, &reg32_val));

        curr = soc_reg_field_get(unit, temp_reg[i], reg32_val, curr_field[i]);
        /*curr [0.1 C] = 4120.5 - curr * 4.9103*/
        temperature_array[i].curr =  (_SOC_RAMON_PVT_BASE - curr * _SOC_RAMON_PVT_FACTOR) / 10000;

        peak = soc_reg_field_get(unit, temp_reg[i], reg32_val, peak_field[i]);
        /*peak [0.1 C] = 4120.5 - peak * 4.9103*/
        temperature_array[i].peak = (_SOC_RAMON_PVT_BASE - peak * _SOC_RAMON_PVT_FACTOR) / 10000;
    }

    *temperature_count = _SOC_RAMON_PVT_MON_NOF;

exit:
    DNXC_FUNC_RETURN; 
}

int
soc_ramon_drv_fe13_graceful_shutdown_set(int unit, soc_pbmp_t active_links, soc_pbmp_t unisolated_links, int shutdown) 
{
    DNXC_INIT_FUNC_DEFS

    DNXC_IF_ERR_EXIT(soc_ramon_drv_graceful_shutdown_set(unit, active_links, shutdown, unisolated_links, 1 /*param not relevant for fe13 isolate*/));

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_drv_graceful_shutdown_set(int unit, soc_pbmp_t active_links, int shutdown, soc_pbmp_t unisolated_links, int isolate_device) 
{
    int rv;
    soc_port_t port;
    DNXC_INIT_FUNC_DEFS;


    if (shutdown)
    {
        if (SOC_DNXF_IS_FE13(unit))
        {
            /*FE13*/
            rv = soc_ramon_drv_fe13_isolate_set(unit, unisolated_links, 1);
            DNXC_IF_ERR_EXIT(rv);
        }
        else
        {
            /*FE2*/
            rv = soc_ramon_fabric_topology_isolate_set(unit, soc_dnxc_isolation_status_isolated); 
            DNXC_IF_ERR_EXIT(rv);
        }

        /*RX reset - mac only*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_dnxc_port_control_rx_enable_set(unit, port, SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE, 0);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(20000); /*sleep 20 milisec*/

        /*Disable link*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_dnxc_port_enable_set(unit, port, 0);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(50000); /*sleep 50 mili sec*/

    } else { /*power up*/

        /*Enable link*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_dnxc_port_enable_set(unit, port, 1);
            DNXC_IF_ERR_EXIT(rv);
        }

        /*RX enable - mac only*/
        SOC_PBMP_ITER(active_links, port)
        {
            rv = soc_dnxc_port_control_rx_enable_set(unit, port, SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE, 1);
            DNXC_IF_ERR_EXIT(rv);
        }
        sal_usleep(1000000); /*sleep 1 second*/

        if (SOC_DNXF_IS_FE13(unit))
        {
            /*FE13*/
            rv = soc_ramon_drv_fe13_isolate_set(unit, unisolated_links, 0);
            DNXC_IF_ERR_EXIT(rv);
        }
        else
        {
            /*FE2*/
            if (!isolate_device) {
                rv = soc_ramon_fabric_topology_isolate_set(unit, soc_dnxc_isolation_status_active); 
                DNXC_IF_ERR_EXIT(rv);
            }
        }

    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_drv_fe13_links_bitmap_get(int unit, soc_reg_above_64_val_t *all_links_bitmap, soc_reg_above_64_val_t *fap_links_bitmap, soc_reg_above_64_val_t *fe2_links_bitmap)
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
soc_ramon_drv_fe13_isolate_set(int unit, soc_pbmp_t unisolated_links_pbmp, int isolate) 
{
    int rv;
    int ccs_index;
    soc_reg_above_64_val_t reg_above_64_val, all_links_bitmap, fap_links_bitmap, fe2_links_bitmap, unisolated_links;
    uint32 reg32, rtp_stop_en;
    soc_port_t port;
    DNXC_INIT_FUNC_DEFS;

    rv = soc_ramon_drv_fe13_links_bitmap_get(unit, &all_links_bitmap, &fap_links_bitmap, &fe2_links_bitmap);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(unisolated_links);
    SOC_PBMP_ITER(unisolated_links_pbmp, port)
    {
        SHR_BITSET(unisolated_links, port);
    }

    if (isolate)
    {
        /*Stop RTP update*/
        rv = READ_RTP_RESERVED_10r(unit, &reg32);
        DNXC_IF_ERR_EXIT(rv);

        /* Store STOP RTP state */
        rtp_stop_en = soc_reg_field_get(unit, RTP_RESERVED_10r, reg32, FIELD_0_7f);

        soc_reg_field_set(unit, RTP_RESERVED_10r, &reg32, FIELD_0_7f, 0x0);
        rv = WRITE_RTP_RESERVED_10r(unit, reg32);
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
            DNXC_IF_ERR_EXIT(READ_CCS_REG_0102r(unit, ccs_index, reg_above_64_val));
            SOC_REG_ABOVE_64_AND(reg_above_64_val, fe2_links_bitmap);
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0102r(unit, ccs_index, reg_above_64_val));
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
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0102r(unit, ccs_index, reg_above_64_val));
        }       
        
        sal_usleep(30000); /*30 milisec*/

        /*Start RTP update*/
        rv = READ_RTP_RESERVED_10r(unit, &reg32);
        DNXC_IF_ERR_EXIT(rv);
        soc_reg_field_set(unit, RTP_RESERVED_10r, &reg32, FIELD_0_7f, rtp_stop_en);
        rv = WRITE_RTP_RESERVED_10r(unit, reg32);
        DNXC_IF_ERR_EXIT(rv);

        sal_usleep(1000000); /*sleep 1 second*/

        
    } else { /*unisolate*/

        DNXC_IF_ERR_EXIT(soc_ramon_fabric_topology_mesh_topology_reset(unit));

        sal_usleep(30000); /*30 milisec*/

        /*Unisolate links to FE2*/
        SOC_REG_ABOVE_64_COPY(reg_above_64_val, fe2_links_bitmap);
        for (ccs_index = 0; ccs_index < SOC_DNXF_DEFS_GET(unit, nof_instances_ccs); ccs_index++)
        {
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0102r(unit, ccs_index, reg_above_64_val));
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
            DNXC_IF_ERR_EXIT(WRITE_CCS_REG_0102r(unit, ccs_index, reg_above_64_val));
        }
        
        sal_usleep(30000); /*30 milisec*/

        SOC_REG_ABOVE_64_COPY(reg_above_64_val, all_links_bitmap);
        SOC_REG_ABOVE_64_AND(reg_above_64_val, unisolated_links);
        DNXC_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reg_above_64_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_ramon_nof_block_instances(int unit, soc_block_types_t block_types, int *nof_block_instances) 
{
    DNXC_INIT_FUNC_DEFS;

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
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_CCS;
            break;
        case SOC_BLK_DCL:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl);
            break;
        case SOC_BLK_RTP:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_RTP;
            break;
        case SOC_BLK_OCCG:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_OCCG;
            break;
        case SOC_BLK_ECI:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_ECI;
            break;
        case SOC_BLK_DCM:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_DCM;
            break;
        case SOC_BLK_DCMC:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_DCMC;
            break;
        case SOC_BLK_CMIC:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_CMIC;
            break;
        case SOC_BLK_MESH_TOPOLOGY:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_MESH_TOPOLOGY;
            break;
        case SOC_BLK_OTPC:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_OTPC;
            break;
        case SOC_BLK_BRDC_FMACH:
            *nof_block_instances = SOC_DNXF_DEFS_GET(unit, nof_instances_brdc_fmach); 
            break;
        case SOC_BLK_BRDC_FMACL:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_BRDC_FMACL;
            break;
        case SOC_BLK_BRDC_FSRD:
            *nof_block_instances = SOC_RAMON_NOF_INSTANCES_BRDC_FSRD;
            break;

        default:
            *nof_block_instances = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Memory bist test
 */
int
soc_ramon_drv_mbist(int unit, int skip_errors)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    SOC_DNXF_DRV_INIT_LOG(unit, "Memory Bist");
    rc = soc_ramon_bist_all(unit, skip_errors);
    DNXC_IF_ERR_EXIT(rc);
        
exit:
    DNXC_FUNC_RETURN;   
}

int
soc_ramon_drv_block_pbmp_get(int unit, int block_type, int blk_instance, soc_pbmp_t *pbmp)
{   
    int first_link = 0,range = 0;
    int first_link_part_2 = 0, range_part_2 = 0;


    DNXC_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*pbmp);

    switch (block_type)
    {
       case SOC_BLK_DCH:
       case SOC_BLK_DCM:
           first_link = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq) * blk_instance;
           if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit))
           {
               if (blk_instance < SOC_DNXF_DEFS_GET(unit, nof_instances_dch) / 2)
               {
                   range = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq);
                   first_link_part_2 = blk_instance ? _SOC_RAMON_DRV_FE13_ASYM_FAP_FIRST_LINK_QUARTER_1_PART_2 : _SOC_RAMON_DRV_FE13_ASYM_FAP_FIRST_LINK_QUARTER_0_PART_2;
                   range_part_2 = _SOC_RAMON_DRV_FE13_ASYM_FAP_RANGE_PART_2;
               } else {
                   range = _SOC_RAMON_DRV_FE13_ASYM_FE2_RANGE;
               }
           } else {
               range = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcq);
           }
           
           break;
       default:
           DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("block (%d) - block pbmp is not supported"), block_type));
           break;
    }

    SOC_PBMP_PORTS_RANGE_ADD(*pbmp, first_link, range);
    SOC_PBMP_PORTS_RANGE_ADD(*pbmp, first_link_part_2, range_part_2);
    SOC_PBMP_AND(*pbmp, PBMP_SFI_ALL(unit));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Returns TRUE if the memory is dynamic
 */
int soc_ramon_tbl_is_dynamic(int unit, soc_mem_t mem) {

    /*
     
    *1.	Those memories are dynamic always:
    *RTP_DLLUP
    *RTP_RESERVED_23
    *
    *2.	Those memories has no CPU access:
    *   RTP_MEM_800000
    *  RTP_MEM_900000
    *
    *3.	The following memories dynamic if device is not repeater:
    *RTP_DUCTP
    *RTP_CUCT
    *RTP_RMHMT
    *
    *4.	Memories below is dynamic if fabric_load_balancing_mode != load_balancing_mode_destination_unreachable
    *RTP_SLSCT
    */

    if (soc_mem_is_readonly(unit, mem) || soc_mem_is_writeonly(unit, mem) || soc_mem_is_signal(unit, mem)) {
        return TRUE;
    }

    switch (mem) {
    case RTP_DUCTPm:
    case RTP_CUCTm:
    case RTP_RMHMTm:
    case RTP_MULTI_CAST_TABLE_UPDATEm:
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

int
soc_ramon_drv_block_valid_get(int unit, int blktype, int blockid, char *valid)
{
    int rv;
    int disabled = 0;
    DNXC_INIT_FUNC_DEFS;

    if (blktype == SOC_BLK_FSRD) {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_quad_disabled, (unit, blockid * SOC_DNXF_DEFS_GET(unit, nof_quads_in_fsrd), &disabled));
        DNXC_IF_ERR_EXIT(rv);
        *valid = ((disabled == 0) ? 1 : 0);
    } else {
        *valid  = 1;
    }

exit:
    DNXC_FUNC_RETURN; 
}


#undef _ERR_MSG_MODULE_NAME
