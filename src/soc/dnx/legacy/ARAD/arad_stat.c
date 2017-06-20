/*
 * $Id: jer2_arad_stat.c,v 1.14 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC JER2_ARAD FABRIC STAT
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_STAT

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>
#include <shared/bitop.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>
#include <soc/dnx/legacy/ARAD/arad_defs.h>
#include <soc/dnx/legacy/ARAD/arad_stat.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <bcm/stat.h>


int 
soc_jer2_arad_mac_controlled_counter_get(int unit, int counter_id, int port, uint64* val){

    int blk_idx, lane_idx ,link,rv, port_offset;
    uint32 control_reg;
    uint32 first_mac_counter;
    uint64 mask;
    int length, counter_locked=0;
    uint32 offset = 0, phy_port;
    int ilkn_over_fabric_port = 0;
    DNXC_INIT_FUNC_DEFS;

    if (IS_IL_PORT(unit,port)) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY 
        ilkn_over_fabric_port = SOC_DNX_IS_ILKN_PORT_OVER_FABRIC(unit, offset);
#endif 
    } else {
        offset = 0;
        ilkn_over_fabric_port = 0;
    }

    if(IS_SFI_PORT(unit,port) || ilkn_over_fabric_port) {
       if (ilkn_over_fabric_port) {
           port_offset = SOC_DNX_DEFS_GET(unit, first_fabric_link_id);
           DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port , &phy_port));
           link = phy_port - SOC_DNX_FIRST_FABRIC_PHY_PORT(unit);
       } else {
            port_offset = SOC_DNX_DEFS_GET(unit, first_fabric_link_id); /*offset for QMX*/
            link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port - port_offset);
       }

       blk_idx = link/SOC_JER2_ARAD_NOF_LINKS_IN_MAC;
       lane_idx = link % SOC_JER2_ARAD_NOF_LINKS_IN_MAC;

        first_mac_counter = SOC_DNX_DEFS_GET(unit, mac_counter_first);

        DNXC_IF_ERR_EXIT(MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_stat_counter_length_get, (unit, counter_id, &length)));

        counter_id = counter_id-first_mac_counter;
        
        COUNTER_LOCK(unit);
        counter_locked = 1;

        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr(unit, blk_idx, &control_reg));
        soc_reg_field_set(unit, FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr, &control_reg, LANE_SELECTf, lane_idx);
        soc_reg_field_set(unit, FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr, &control_reg, COUNTER_SELECTf, counter_id);
        SOC_DNX_ALLOW_WARMBOOT_WRITE(WRITE_FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr(unit, blk_idx, control_reg), rv);
        DNXC_IF_ERR_EXIT(rv);
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_STATISTICS_OUTPUTr(unit, blk_idx, val));

        counter_locked = 0;
        COUNTER_UNLOCK(unit);

        COMPILER_64_MASK_CREATE(mask, length, 0);
        COMPILER_64_AND((*val), mask);

    } else {
        COMPILER_64_SET(*val, 0, 0);
    }
    

exit:
    if (counter_locked) {
        COUNTER_UNLOCK(unit);
    }
    DNXC_FUNC_RETURN;
}


soc_controlled_counter_t soc_jer2_arad_controlled_counter[] = {
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_TX_CONTROL_CELLS_COUNTER,
        "TX Control cells",
        "TX Control cells",
        _SOC_CONTROLLED_COUNTER_FLAG_TX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_CELL_COUNTER,
        "TX Data cell",
        "TX Data cell",
        _SOC_CONTROLLED_COUNTER_FLAG_TX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_BYTE_COUNTER,
        "TX Data byte",
        "TX Data byte",
        _SOC_CONTROLLED_COUNTER_FLAG_TX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_CRC_ERRORS_COUNTER,
        "RX CRC errors",
        "RX CRC errors",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_CORRECTABLE_ERROR, /*SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_CRC_ERROR , SOC_JER2_ARAD_MAC_COUNTERS_RX_8B_10B_DISPARITY_ERRORS*/
        "RX (L)FEC correctable \\ BEC crc \\ 8b/10b disparity",
        "RX correctable",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_CONTROL_CELLS_COUNTER,
        "RX Control cells",
        "RX Control cells",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_CELL_COUNTER,
        "RX Data cell",
        "RX Data cell",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_BYTE_COUNTER,
        "RX Data byte",
        "RX Data byte",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_DROPPED_RETRANSMITTED_CONTROL,
        "RX dropped retransmitted control",
        "RX drop retransmit",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_TX_BEC_RETRANSMIT,
        "TX BEC retransmit",
        "TX BEC retransmit",
        _SOC_CONTROLLED_COUNTER_FLAG_TX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_RETRANSMIT,
        "RX BEC retransmit",
        "RX BEC retransmit",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_TX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS,
        "TX Asyn fifo rate at units of 40 bits",
        "TX Asyn fifo rate",
        _SOC_CONTROLLED_COUNTER_FLAG_NOT_PRINTABLE | _SOC_CONTROLLED_COUNTER_FLAG_TX | _SOC_CONTROLLED_COUNTER_FLAG_LOW | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS,
        "RX Asyn fifo rate at units of 40 bits",
        "RX Asyn fifo rate",
        _SOC_CONTROLLED_COUNTER_FLAG_NOT_PRINTABLE | _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_LOW | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_UNCORRECTABLE_ERRORS, /*SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_RX_FAULT, SOC_JER2_ARAD_MAC_COUNTERS_RX_8B_10B_CODE_ERRORS*/
        "RX (L)FEC uncorrectable \\ BEC fault \\ 8b/10b code errors",
        "RX uncorrectable",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_LLFC_PRIMARY, 
        "RX LLFC Primary",
        "RX LLFC Primary",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        soc_jer2_arad_mac_controlled_counter_get,
        SOC_JER2_ARAD_MAC_COUNTERS_RX_LLFC_SECONDARY, 
        "RX LLFC Secondary",
        "RX LLFC Secondary",
        _SOC_CONTROLLED_COUNTER_FLAG_RX | _SOC_CONTROLLED_COUNTER_FLAG_HIGH | _SOC_CONTROLLED_COUNTER_FLAG_MAC,
        COUNTER_IDX_NOT_COLLECTED
    },
    {
        NULL,
        -1,
        "",
        "",
        COUNTER_IDX_NOT_COLLECTED
    }
};

/*
 * Function:
 *      soc_jer2_arad_fabric_stat_init
 * Purpose:
 *      Init JER2_ARAD fabric stat
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      SOC_E_XXX         Operation failed
 */
soc_error_t 
soc_jer2_arad_fabric_stat_init(int unit)
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_WARM_BOOT(unit)) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr(unit, 0, &reg_val));
        soc_reg_field_set(unit, FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr, &reg_val, DATA_COUNTER_MODEf, 0);
        soc_reg_field_set(unit, FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr, &reg_val, DATA_BYTE_COUNTER_HEADERf, 1);
        soc_reg_field_set(unit, FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr, &reg_val, COUNTER_CLEAR_ON_READf, 1);
        /*write to broadcast instead of updating each instance individually*/
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_FMAL_STATISTICS_OUTPUT_CONTROLr(unit, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer2_arad_mapping_stat_get
 * Purpose:
 *      given counter type return counter id
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      type  - (IN)     SNMP statistics type defined in bcm_stat_val_t
 *      cnt_type - (OUT) Array of identifiers of counters. Loaded by this procedure.
 *      num_cntrs_p - (OUT) Number of counters actually loaded into 'cnt_type'
 *                       array.
 *      num_cntrs_in - (IN) Maximal number of counters caller is willing to
 *                       accept.
 * Returns:
 *      SOC_E_NONE        Success  
 *      SOC_E_UNAVAIL     Counter not supported for the port
 *      SOC_E_XXX         Operation failed
 */

soc_error_t
soc_jer2_arad_mapping_stat_get(int unit, soc_port_t port, uint32 *cnt_type, int *num_cntrs_p, bcm_stat_val_t type, int num_cntrs_in)
{
    int pcs;
    DNXC_INIT_FUNC_DEFS;

    *num_cntrs_p = num_cntrs_in;

    if (*num_cntrs_p <= 0) {
        DNXC_EXIT_WITH_ERR(
            SOC_E_PARAM,(_BSL_DNXC_MSG("*num_cntrs_p (%d) is zero or negative"), *num_cntrs_p)); 
    }
    /*
     * At this point, *num_cntrs_p is at least '1'.
     */

    *num_cntrs_p = 1 ;
    switch(type) {
        case snmpBcmTxControlCells:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_TX_CONTROL_CELLS_COUNTER;
            break;
        case snmpBcmTxDataCells:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_CELL_COUNTER;
            break;
        case snmpBcmTxDataBytes:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_BYTE_COUNTER;
            break;
        case snmpBcmRxCrcErrors:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_CRC_ERRORS_COUNTER;
            break;
        case snmpBcmRxFecCorrectable:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 
            pcs = -1;
            if(soc_dnxc_port_pcs_8_9_legacy_fec != pcs && soc_dnxc_port_pcs_64_66_fec != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("port: %d, counter %d supported only for FEC ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_CORRECTABLE_ERROR;
            break;
        case snmpBcmRxBecCrcErrors:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 

            pcs = -1;
            if(soc_dnxc_port_pcs_64_66_bec != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("port: %d, counter %d supported only for BEC ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_CRC_ERROR;
            break;
        case snmpBcmRxDisparityErrors:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 

            pcs = -1;
            if(soc_dnxc_port_pcs_8_10 != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("port: %d, counter %d supported only for 8b/10b ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_8B_10B_DISPARITY_ERRORS;
            break;
        case snmpBcmRxControlCells:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_CONTROL_CELLS_COUNTER;
            break;
        case snmpBcmRxDataCells:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_CELL_COUNTER;
            break;
        case snmpBcmRxDataBytes:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_BYTE_COUNTER;
            break;
        case snmpBcmRxDroppedRetransmittedControl:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_DROPPED_RETRANSMITTED_CONTROL;
            break;
        case snmpBcmTxBecRetransmit:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_TX_BEC_RETRANSMIT;
            break;
        case snmpBcmRxBecRetransmit:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_RETRANSMIT;
            break;
        case snmpBcmTxAsynFifoRate:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_TX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS;
            break;
        case snmpBcmRxAsynFifoRate:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS;
            break;
        case snmpBcmRxFecUncorrectable:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 

            pcs = -1;
            if(soc_dnxc_port_pcs_8_9_legacy_fec != pcs && soc_dnxc_port_pcs_64_66_fec != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("port: %d, counter %d supported only for FEC ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_UNCORRECTABLE_ERRORS;
            break;
        case snmpBcmRxBecRxFault:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 

            pcs = -1;
            if(soc_dnxc_port_pcs_64_66_bec != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("port: %d, counter %d supported only for BEC ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_RX_FAULT;
            break;
        case snmpBcmRxCodeErrors:
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            DNX_IF_ERR_EXIT(bcm_dnx_port_control_get(unit, port, bcmPortControlPCS, &pcs));
#endif 

            pcs = -1;
            if(soc_dnxc_port_pcs_8_10 != pcs) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("port: %d, counter %d supported only for 8b/10b ports"),port, type)); 
            }
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_8B_10B_CODE_ERRORS;
            break;
        case snmpBcmRxLlfcPrimary:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_LLFC_PRIMARY;
            break;
        case snmpBcmRxLlfcSecondary:
            cnt_type[0] = SOC_JER2_ARAD_MAC_COUNTERS_RX_LLFC_SECONDARY;
            break;
        default:
            *num_cntrs_p = 0 ;
            DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("stat type %d isn't supported"), type)); 
        }
        
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_jer2_arad_stat_controlled_counter_enable_get(int unit, soc_port_t port, int index, int *enable, int *printable) {
    soc_control_t	*soc;
    DNXC_INIT_FUNC_DEFS;

    *enable = 1;
    *printable = 1;
    soc = SOC_CONTROL(unit);

    if (BCM_PBMP_MEMBER(PBMP_SFI_ALL(unit), port)) {
        if (!(soc->controlled_counters[index].flags & _SOC_CONTROLLED_COUNTER_FLAG_MAC)) {
            *enable = 0;
            SOC_EXIT;
        }
    } else {
        /*NIF*/
        if (!(soc->controlled_counters[index].flags & _SOC_CONTROLLED_COUNTER_FLAG_NIF)) {
            *enable = 0;
        }
    }
    if(soc->controlled_counters[index].flags & _SOC_CONTROLLED_COUNTER_FLAG_NOT_PRINTABLE) {
        *printable = 0;
    }

    
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_jer2_arad_stat_counter_length_get(int unit, int counter_id, int *length)
{
    DNXC_INIT_FUNC_DEFS;

    switch (counter_id)
    {
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_DROPPED_RETRANSMITTED_CONTROL:
        case SOC_JER2_ARAD_MAC_COUNTERS_TX_BEC_RETRANSMIT:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_BEC_RETRANSMIT:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_UNCORRECTABLE_ERRORS:
            *length = 16;
            break;

        case SOC_JER2_ARAD_MAC_COUNTERS_RX_CRC_ERRORS_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_LFEC_FEC_CORRECTABLE_ERROR:
        case SOC_JER2_ARAD_MAC_COUNTERS_TX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_ASYN_FIFO_RATE_AT_UNITS_OF_40_BITS:
            *length = 32;
            break;

        case SOC_JER2_ARAD_MAC_COUNTERS_TX_CONTROL_CELLS_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_CELL_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_TX_DATA_BYTE_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_CONTROL_CELLS_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_CELL_COUNTER:
        case SOC_JER2_ARAD_MAC_COUNTERS_RX_DATA_BYTE_COUNTER:
            *length = 48;
            break;

        default:
            *length = 0;
            break;
    }

	DNXC_FUNC_RETURN;
}


soc_error_t
soc_jer2_arad_stat_path_info_get(int unit, soc_dnx_stat_path_info_t *info)
{
    uint32 reg_val, count32;
    uint64 reg64_val, count64;
    soc_reg_above_64_val_t reg_val_above_64; 
    int core, found;

    DNXC_INIT_FUNC_DEFS;

    info->ingress_core = -1;
    info->egress_core = -1;
    info->drop = soc_dnx_stat_path_drop_stage_none;
    /*ingress core*/
    if (SOC_IS_JERICHO(unit) && !SOC_IS_QAX(unit))
    {
        found = 0;
        SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
            /*Ingress core*/
            DNXC_IF_ERR_EXIT(READ_IRR_IQM_INTERFACE_COUNTERr(unit, core, reg_val_above_64));
            count32 =  soc_reg_above_64_field32_get(unit, IRR_IQM_INTERFACE_COUNTERr, reg_val_above_64, IQM_N_PC_COUNTERf);
            if (count32)
            {   
                info->ingress_core = core;
                found = 1;
                break;
            }
        }
    }
    else
    {
        info->ingress_core = 0;
        found = 1;
    }


    if (found)
    {

          
        /*Check if dropped in ingress TM*/
        found = 0;
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            DNXC_IF_ERR_EXIT(READ_FDT_TRANSMITTED_DATA_CELLS_COUNTERr(unit, &reg_val));
            found += (reg_val ? 1 : 0);
        }        
        if (SOC_REG_IS_VALID(unit, EGQ_IPT_PACKET_COUNTERr))
        {
            DNXC_IF_ERR_EXIT(READ_EGQ_IPT_PACKET_COUNTERr(unit, &reg64_val));
            if (!COMPILER_64_IS_ZERO(reg64_val))
            {   
                found = 1;
            }
        } 
#ifdef BCM_JER2_JERICHO_SUPPORT
        else
        {
            if (!soc_feature(unit, soc_feature_no_fabric)) {
                SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
                    DNXC_IF_ERR_EXIT(READ_FDA_EGQ_CELLS_IN_CNT_IPTr(unit, core, &reg_val));
                    found += (reg_val ? 1 : 0);
                }
            }
        }
#endif /*BCM_JER2_JERICHO_SUPPORT*/
        if (found)
        {

            
            /*egress core*/
            found = 0;
            SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
                DNXC_IF_ERR_EXIT(READ_EPNI_EPE_PACKET_COUNTERr(unit, core, &reg64_val));
                count64 = soc_reg64_field_get(unit, EPNI_EPE_PACKET_COUNTERr, reg64_val, EPE_PACKET_COUNTERf);
                if (!COMPILER_64_IS_ZERO(count64))
                {   
                    info->egress_core = core;
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                info->drop = soc_dnx_stat_path_drop_stage_egress_tm; /*packets wasn't transmitted from egress tm to egress pp*/
            }
        }
        else
        {
            info->drop = soc_dnx_stat_path_drop_stage_ingress_tm; /* packets wasn't transmitted to fabric */
        }
    }
    else
    {
        info->drop = soc_dnx_stat_path_drop_stage_ingress_no_packet; /* packets wasn't entered to ingress*/
    }
    

exit:
    DNXC_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME

