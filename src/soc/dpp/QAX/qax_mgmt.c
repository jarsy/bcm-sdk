/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_mgmt.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/QAX/qax_mgmt.h>
#if 0
#include <soc/dpp/JER/jer_defs.h>
#include <soc/dpp/JER/jer_fabric.h>
#endif /* 0 */

#define QAX_MGMT_NOF_PROCESSOR_IDS 17

/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 qax_mgmt_system_fap_id_set(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32  sys_fap_id
  )
{
    uint32 reg32;
    SOCDNX_INIT_FUNC_DEFS;
    if (sys_fap_id >= ARAD_NOF_FAPS_IN_SYSTEM) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("FAP ID %u is illegal, must be under %u."),
          sys_fap_id, ARAD_NOF_FAPS_IN_SYSTEM));
    }
    /* configure the FAP ID, and TDM source FAP ID */
    SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32));
    soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32, DEV_IDf, sys_fap_id);
    SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32));

    if (SOC_DPP_CONFIG(unit)->tdm.is_bypass &&
        SOC_DPP_CONFIG(unit)->arad->init.fabric.is_128_in_system &&
        SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system &&
        !SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr,
          REG_PORT_ANY, 0, TDM_SOURCE_FAP_IDf,
          sys_fap_id + SOC_DPP_CONFIG(unit)->arad->tdm_source_fap_id_offset));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* Get the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32
  qax_mgmt_system_fap_id_get(
    SOC_SAND_IN  int       unit,
    SOC_SAND_OUT uint32    *sys_fap_id
  )
{
  uint32       fld_val;

  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(sys_fap_id);


  SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_GLOBAL_GENERAL_CFG_2r, REG_PORT_ANY, 0, DEV_IDf, &fld_val));
  *sys_fap_id = fld_val;

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* set all the bits controlling the revision fixes (chicken bits) in the device.
*********************************************************************/
uint32 qax_mgmt_revision_fixes (int unit)
{
    uint64 reg64_val;
    uint32 reg32_val;
    soc_reg_above_64_val_t reg_above_64_val;
    int array_index, core_index = 0;

    SOCDNX_INIT_FUNC_DEFS;
    if (!SOC_IS_QUX(unit)) {
    /*
     * CFC_SCH_OOB_RX_CFG
     */
    SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, CFC_SCH_OOB_RX_CFGr, REG_PORT_ANY,  0, &reg64_val));
    /* CFC-SCH OOB CRC is calculated wrong */
    soc_reg64_field32_set(unit, CFC_SCH_OOB_RX_CFGr, &reg64_val, SCH_OOB_CRC_CFGf, 0x7);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, CFC_SCH_OOB_RX_CFGr, REG_PORT_ANY,  0,  reg64_val));
    }
    /*
     * CRPS_CRPS_GENERAL_CFG
     */
    for (array_index = 0; array_index < QAX_MGMT_NOF_PROCESSOR_IDS; ++array_index) {
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, CRPS_CRPS_GENERAL_CFGr, REG_PORT_ANY, array_index, &reg32_val));
        /* QPTS - "active counters" counter isn't accurate when using "Qsize"    */ 
        soc_reg_field_set(unit, CRPS_CRPS_GENERAL_CFGr, &reg32_val, CRPS_N_ACT_CNT_VALIDATE_ENf, 0x1);
        /* QPTS - Ovth memory bypass bug    */
        soc_reg_field_set(unit, CRPS_CRPS_GENERAL_CFGr, &reg32_val, CRPS_N_OVTH_MEM_RFRSH_BPASS_ENf, 0x1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, CRPS_CRPS_GENERAL_CFGr, REG_PORT_ANY, array_index,  reg32_val));
    }

    /*
     * PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATION
     */
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr, REG_PORT_ANY,  0, reg_above_64_val));
    /* If set, the limit on the number of entries in the MACT is according to FID,  else the limit is according to lif. - Default MACT limit per FID */
    soc_reg_field_set(unit, PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr, reg_above_64_val, LARGE_EM_CFG_LIMIT_MODE_FIDf, 0x1);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, PPDB_B_LARGE_EM_COUNTER_DB_CNTR_PTR_CONFIGURATIONr, REG_PORT_ANY,  0,  reg_above_64_val));

    /*
     * EGQ_CFG_BUG_FIX_CHICKEN_BITS_REG_1
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EGQ_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, core_index,  0, &reg32_val));
    /* Bug Desc: EGQ  ERPP/ETPP  EPP can wrongly determine the EEI as ISID for IP routed packets. (1 - bug fix disabled, the logic is simillar to B0 implementation) */
    soc_reg_field_set(unit, EGQ_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, &reg32_val, CFG_BUG_FIX_18_DISABLEf, 0x0);
    /* Bug Desc: IHB  bounce back filter for unicast packets (1 - bug fix disabled, the logic is simillar to B0 implementation) Prevent bounce back filtering for UC packets. Fixes Plus-EBF1 */
    soc_reg_field_set(unit, EGQ_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, &reg32_val, CFG_BUG_FIX_98_DISABLEf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EGQ_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, core_index,  0,  reg32_val));

    /*
     * EGQ_EGRESS_SHAPER_ENABLE_SETTINGS
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, core_index,  0, &reg32_val));
    /* If set, then the value of each credit corresponding to the Q-Pair shapers calnedar is 1/128 bytes else 1/1256 bytes. */
    soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg32_val, QPAIR_SPR_RESOLUTIONf, 0x0);
    /* If set, then the value of each credit corresponding to the TCG shapers calnedar is 1/128 bytes else 1/1256 bytes. */
    soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg32_val, TCG_SPR_RESOLUTIONf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, core_index,  0,  reg32_val));

    /*PetraB FTMH and ARAD PPH mode*/
    if ((TRUE == SOC_DPP_CONFIG(unit)->arad->init.pp_enable) && (TRUE == SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system) &&
        soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "petrab_in_tm_mode", 0) )
    {
        /*
         * EGQ_PP_CONFIG
         */
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EGQ_PP_CONFIGr, core_index,  0, &reg32_val));
        /* Bug Desc: IHB  bounce back filter for unicast packets (1 - bug fix disabled, the logic is simillar to B0 implementation) Prevent bounce back filtering for UC packets. Fixes Plus-EBF1 */
        soc_reg_field_set(unit, EGQ_PP_CONFIGr, &reg32_val, FIELD_13_13f, 0x1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EGQ_PP_CONFIGr, core_index,  0,  reg32_val));
    }

    /*
     * EGQ_QPAIR_SPR_DIS
     */
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, EGQ_QPAIR_SPR_DISr, core_index,  0, reg_above_64_val));
    /* If set then the corresponding Q-Pair shaper is disabled (gets auto credit and no need to insert it to the calendar). */
    soc_reg_field_set(unit, EGQ_QPAIR_SPR_DISr, reg_above_64_val, QPAIR_SPR_DISf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, EGQ_QPAIR_SPR_DISr, core_index,  0,  reg_above_64_val));

    /*
     * EGQ_TCG_SPR_DIS
     */
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, EGQ_TCG_SPR_DISr, core_index,  0, reg_above_64_val));
    /* If set then the corresponding Q-Pair shaper is disabled (gets auto credit and no need to insert it to the calendar). */
    soc_reg_field_set(unit, EGQ_TCG_SPR_DISr, reg_above_64_val, TCG_SPR_DISf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, EGQ_TCG_SPR_DISr, core_index,  0,  reg_above_64_val));

    /*
     * EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, core_index,  0, &reg32_val));
    /* EGQ - ERPP/ETPP - EPP can wrongly determine the EEI as ISID for IP routed packets. (1 - bug fix disabled, the logic is simillar to B0 implementation)  */
    soc_reg_field_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, &reg32_val, CFG_BUG_FIX_18_DISABLEf, 0x0);
    /* 0- Use AC EEDB Entry as Data Entry is enabled, 1- Feature is disabled, Enable PON 3tag fix */
    soc_reg_field_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, &reg32_val, CFG_PLUS_F_24_DISABLEf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_1r, core_index,  0,  reg32_val));

    /*
     * EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, core_index,  0, &reg32_val));
    if ((SOC_DPP_CONFIG(unit)->pp.next_hop_mac_extension_enable)) 
    {
        /* for compatibilty with ARAD, Disable hardware computation of Host-Index for DA. 
         * Instead, do it the Arad way: PMF will add pph learn extension (system header ), egress program editor will stamp the DA  
         * if this soc property is disabled, then use hardware computation using the chicken bit
         */
        soc_reg_field_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, &reg32_val, CFG_USE_HOST_INDEX_FOR_DA_CALC_ENABLEf, SOC_IS_NEXT_HOP_MAC_EXT_ARAD_COMPATIBLE(unit) ? 0x0 : 0x1);
    }

    /* 0- MPLS pipe model fix is disabled, 1- MPLS pipe model fix is enabled, Enable MPLS Pipe fix */ 
    soc_reg_field_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, &reg32_val, CFG_MPLS_PIPE_FIX_ENABLEf, 0x1);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EPNI_CFG_BUG_FIX_CHICKEN_BITS_REG_2r, core_index,  0,  reg32_val));

    /*
     * EPNI_CFG_DC_OVERLAY
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EPNI_CFG_DC_OVERLAYr, core_index,  0, &reg32_val));
    /* VxLAN: native support, 0x1 - Enables construction of VxLAN-UDP headers (in encapsulation block), and using Model-4 EEDB (in prp block), when the (EEDBoutLif.EncapMode == Eth_IP) 0x0 - DC Overlays Disabled */
    soc_reg_field_set(unit, EPNI_CFG_DC_OVERLAYr, &reg32_val, CFG_EN_VXLAN_ENCAPSULATIONf, 0x1);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EPNI_CFG_DC_OVERLAYr, core_index,  0,  reg32_val));

    /*
     * EPNI_CFG_ENABLE_FILTERING_PER_FWD_CODE
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EPNI_CFG_ENABLE_FILTERING_PER_FWD_CODEr, core_index,  0, &reg32_val));
    /* 0 - Disable ETPP filters, 1 - Enable ETPP filters, Disable ETPP filtering for the Snoop/Mirroring forwarding code (code 4'hB). Fixes Plus-EBF5 */
    soc_reg_field_set(unit, EPNI_CFG_ENABLE_FILTERING_PER_FWD_CODEr, &reg32_val, CFG_ENABLE_FILTERING_PER_FWD_CODEf, 0xf7ff);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EPNI_CFG_ENABLE_FILTERING_PER_FWD_CODEr, core_index,  0,  reg32_val));

    /*
     * EPNI_PP_CONFIG
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, EPNI_PP_CONFIGr, core_index,  0, &reg32_val));
    /*PetraB FTMH and ARAD PPH mode*/
    if ((TRUE == SOC_DPP_CONFIG(unit)->arad->init.pp_enable) && (TRUE == SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system) &&
        soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "petrab_in_tm_mode", 0) )
    {
        /* EPNI_REGFILE.PetrabWithAradPpMode = 1 */
        soc_reg_field_set(unit, EPNI_PP_CONFIGr, &reg32_val, PETRAB_WITH_ARAD_PP_MODEf, 0x1);
    }
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, EPNI_PP_CONFIGr, core_index,  0,  reg32_val));

    /*
     * IHB_LBP_GENERAL_CONFIG_0
     */
    SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, IHB_LBP_GENERAL_CONFIG_0r, core_index,  0, &reg64_val));
    /*PetraB FTMH and ARAD PPH mode*/
    if ((TRUE == SOC_DPP_CONFIG(unit)->arad->init.pp_enable) && (TRUE == SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system) &&
        soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "petrab_in_tm_mode", 0) )
    {
        /* if set than the Arad will issue a FTMH heade in Petra format and PPH header in Arad format. The PPH will have pph-eep-ext and oam-ts-ext if needed */
        soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &reg64_val, PETRA_FTMH_WITH_ARAD_PPH_MODEf, 0x1);
    }
    /* If set, in_lif_profile is added to FHEI Header instead of the reserved bits - Enable passing inlif profile in FHEI. */
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &reg64_val, ENABLE_FHEI_WITH_IN_LIF_PROFILEf, 0x1);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, IHB_LBP_GENERAL_CONFIG_0r, core_index,  0,  reg64_val));

    /*
     * IHP_VTT_GENERAL_CONFIGS_1
     */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, IHP_VTT_GENERAL_CONFIGS_1r, core_index,  0, &reg32_val));
    /* If set, and forwarding header is mpls while Terminated-TTL is not Valid, cos marking will be updated according to mpls header. */
    soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, &reg32_val, ENABLE_COS_MARKING_UPGRADESf, 0x1);
    /* If unset,  an inner compatible multicast identification will be executed - IGMP feature: enable compatible-mc after tunnel-termination and upgrade second stage parsing for IGMP */
    soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, &reg32_val, DISABLE_INNER_COMPATIBLE_MCf, 0x0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, core_index,  0,  reg32_val));

    /*
     * EPNI_CHICKEN_BIT_VECTOR
     */
    /* Set PHP checken bit for QAX-B0/QUX*/
    if (SOC_IS_QAX_B0(unit) || SOC_IS_QUX(unit)) {
        /* Bit  0 - Mask PHP BOS:'0' - fix is disabled   '1' - Fix is enable
         * Bit  1 - UDP Checksum Update:'0' - fix is enabled '1' - Fix is disabled
         * Bit  2 -InLIF Profile F2B:'0' - fix is disabled   '1' - Fix is enable
         * Bits 3 - Reserved.
         */
        reg32_val = 0;
        soc_reg_field_set(unit, EPNI_CHICKEN_BIT_VECTORr, &reg32_val, CHICKEN_BIT_VECTORf, 5);
        SOCDNX_IF_ERR_EXIT(WRITE_EPNI_CHICKEN_BIT_VECTORr(unit, reg32_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

