#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#ifdef BCM_88675_A0
/* $Id: jer_ingress_packet_queuing.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/JER/jer_ingress_packet_queuing.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/dpp_config_defs.h>
#include <soc/dpp/ARAD/arad_ingress_packet_queuing.h>
#ifdef THESE_INCLUDES_ARE_NOT_CURRENTLY_NEEDED
#include <soc/dpp/cosq.h>
#include <soc/dpp/TMC/tmc_api_egr_queuing.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/ARAD/arad_api_nif.h>
#include <soc/dpp/ARAD/arad_nif.h>
#include <soc/dpp/ARAD/arad_scheduler_end2end.h>

#include <soc/dpp/ARAD/arad_ports.h>

#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/ARAD/arad_mgmt.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/arad_chip_tbls.h>
#include <soc/dpp/ARAD/arad_debug.h>
#include <soc/mem.h>

#include <soc/dpp/SAND/Utils/sand_u64.h>
#endif /* THESE_INCLUDES_ARE_NOT_CURRENTLY_NEEDED */

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/* Max & min values for struct ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO:      */
#define JER_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN      0


/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */




/*********************************************************************
* NAME:
*     arad_ipq_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
jer_ipq_init(
   SOC_SAND_IN  int                 unit
   ) {
    uint32 res = SOC_E_NONE;
    uint32 system_red = ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ? 1 : 0;
    uint32 mcr_limit_uc;
    uint64 reg64;
    uint32 reg32;
    int core;
    soc_dpp_config_arad_t *dpp_arad;

    SOCDNX_INIT_FUNC_DEFS;

    dpp_arad = SOC_DPP_CONFIG(unit)->arad;

    /* Set System red enable by default */
    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        SOCDNX_IF_ERR_EXIT(READ_IPS_IPS_GENERAL_CONFIGURATIONSr(unit, core, &reg64));
        if (SOC_IS_QAX(unit)) {
            soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, DISCARD_ALL_CGM_MSGf, 0);
        } else {
            soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, DISCARD_ALL_IQM_MSGf, 0);
        }
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, DISCARD_ALL_CRDTf, 0);
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, DISABLE_STATUS_MSG_GENf, 0);
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, DIS_DEQ_CMDSf, 0);
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, ENABLE_SYSTEM_REDf, system_red);
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, UPDATE_MAX_QSZ_FROM_LOCALf, 1);
        soc_reg64_field32_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &reg64, SEND_FSM_FOR_BIGGER_MAX_QUEUE_SIZEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_IPS_IPS_GENERAL_CONFIGURATIONSr(unit, core, reg64));
    }

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
        if (!SOC_IS_QAX(unit)) {
            res = soc_reg_above_64_field32_read(unit, IRR_MCR_FIFO_CONFIGr, REG_PORT_ANY, core, MCR_N_LIMIT_UCf, &mcr_limit_uc);
            SOCDNX_SAND_IF_ERR_EXIT(res);
            /*
             * When FC is received from IQM, use separate thresholds for snoop/mirror, and separate counters
             */
            res = soc_reg_above_64_field32_modify(unit, IRR_UNICAST_FIFO_THRESHOLDSr, REG_PORT_ANY, core, MCR_N_UC_FIFO_SNOOP_THRESHOLDf, ARAD_IPQ_UC_FIFO_SNOOP_THRESHOLD(mcr_limit_uc));
            res = soc_reg_above_64_field32_modify(unit, IRR_UNICAST_FIFO_THRESHOLDSr, REG_PORT_ANY, core, MCR_N_UC_FIFO_MIRROR_THRESHOLDf, ARAD_IPQ_UC_FIFO_MIRROR_THRESHOLD(mcr_limit_uc));
            SOCDNX_IF_ERR_EXIT(res);
        }
    }


    /* configure credit deq command to support full rate */
    if (!SOC_IS_QAX(unit)) {
        reg32 = 0;
        if (dpp_arad->init.drc_info.dram_num == 0) { /* ocb only mode */
            soc_reg_field_set(unit, IPS_DEQ_COMMAND_CREDIT_ADJUSTr, &reg32, DEQ_COMMAND_CREDIT_THf, 0x30);
            soc_reg_field_set(unit, IPS_DEQ_COMMAND_CREDIT_ADJUSTr, &reg32, LFSR_BIT_NUMf, 0x3);
            SOCDNX_IF_ERR_EXIT(WRITE_IPS_DEQ_COMMAND_CREDIT_ADJUSTr(unit, SOC_CORE_ALL, reg32));
        } else { /* DRAM mode */
            SOCDNX_IF_ERR_EXIT(READ_IPS_MAX_DEQ_COMMAND_CREDIT_CONFIGr(unit, 0, &reg32));
            soc_reg_field_set(unit, IPS_MAX_DEQ_COMMAND_CREDIT_CONFIGr, &reg32, MAX_DQ_CMD_CRDT_1f, 0x80);
            SOCDNX_IF_ERR_EXIT(WRITE_IPS_MAX_DEQ_COMMAND_CREDIT_CONFIGr(unit, SOC_CORE_ALL, reg32));

            reg32 = 0;
            soc_reg_field_set(unit, IPS_DEQ_COMMAND_CREDIT_ADJUSTr, &reg32, DEQ_COMMAND_CREDIT_THf, 0x70);
            soc_reg_field_set(unit, IPS_DEQ_COMMAND_CREDIT_ADJUSTr, &reg32, LFSR_BIT_NUMf, 0x3);
            SOCDNX_IF_ERR_EXIT(WRITE_IPS_DEQ_COMMAND_CREDIT_ADJUSTr(unit, SOC_CORE_ALL, reg32));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}


#ifdef JER_OVERRIDE_ARAD_IMPLMENTATION

/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer_ipq_explicit_mapping_mode_info_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
    ) {

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /*
     * JER_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN may be changed and be grater then zero.
     */
    /* coverity[unsigned_compare] */
    if ((info->base_queue_id < JER_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN) ||
        (info->base_queue_id >= SOC_DPP_DEFS_GET(unit, nof_queues))) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_SOCDNX_MSG("base queue ID is out of range")));
    }

    LOG_ERROR(BSL_LS_SOC_INIT,
              (BSL_META_U(unit,
                          "Jericho function not implemented yet\n")));
           
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
jer_ipq_explicit_mapping_mode_info_get(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_OUT SOC_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) {

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(info);

    SOC_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(info);
    LOG_ERROR(BSL_LS_SOC_INIT,
              (BSL_META_U(unit,
                          "Jericho function not implemented yet\n")));
           
exit:
    SOCDNX_FUNC_RETURN;
}
#endif

int
jer_ipq_default_invalid_queue_set(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_IN  uint32         queue_id,
   SOC_SAND_IN  int            enable) 
{

    int core_id;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(core, core_id) {
        if (enable) {
            SOCDNX_IF_ERR_EXIT(WRITE_IRR_IQM_INVALID_DESTINATION_QUEUEr(unit, core_id, queue_id));
        } else {
            SOCDNX_IF_ERR_EXIT(WRITE_IRR_IQM_INVALID_DESTINATION_QUEUEr(unit, core_id, ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)));
        }
    }
           
exit:
    SOCDNX_FUNC_RETURN;
}

int
jer_ipq_default_invalid_queue_get(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_OUT uint32         *queue_id,
   SOC_SAND_OUT int            *enable) 
{

    int core_id;

    SOCDNX_INIT_FUNC_DEFS;

    core_id = (core == SOC_CORE_ALL) ? 0 : core;
    SOCDNX_IF_ERR_EXIT(READ_IRR_IQM_INVALID_DESTINATION_QUEUEr(unit, core_id, queue_id));

    if (*queue_id != ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)) {
        *enable = 1;
    } else {
        *enable = 0;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>
#endif /* BCM_88675_A0 */

