#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_debug.c,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Utils/sand_conv.h>

#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#include <soc/dnx/legacy/ARAD/arad_debug.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/mbcm.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_DBG_AUTOCREDIT_RATE_MIN_TH          (15)
#define JER2_ARAD_DBG_AUTOCREDIT_RATE_MAX_TH          (dnx_sand_power_of_2(19))
#define JER2_ARAD_DBG_AUTOCREDIT_RATE_MIN_VALUE       (1)
#define JER2_ARAD_DBG_AUTOCREDIT_RATE_MAX_VALUE       (15)
#define JER2_ARAD_DBG_AUTOCREDIT_RATE_OFFSET          (3)
#define JER2_ARAD_DEBUG_RST_DOMAIN_MAX                (JER2_ARAD_DBG_NOF_RST_DOMAINS-1)
/*
 *  Polling values for the queue flush
 */

/************************************************************************/
/* Flush Queue register: must be aligned with register database,        */
/* but defined here to minimize access time                             */
/************************************************************************/

/*
 *   Manual Queue Operation : 0x0380 :
 *   ips.manual_queue_operation_reg
 */


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
/*********************************************************************
* NAME:
*     jer2_arad_dbg_rate2autocredit_rate
* FUNCTION:
*   Calculate the AutoCredit rate (four bits) based on the rate in Mbps.
* INPUT:
*   DNX_SAND_IN  uint32   rate -
*     The rate in Mbps.
*   DNX_SAND_OUT uint32  *autocr_rate_bits
*     The four bits computed from the rate, where
*     rate[Mbps]=CreditSize[Bytes/Credit]*8[bits/Bytes]*1000 /
*     (2^(autocr_rate_bits+3)[clocks/credit] * clock_rate[ns/clock])
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   None.
*********************************************************************/
static uint32
jer2_arad_dbg_rate2autocredit_rate(
   DNX_SAND_IN  int  unit,
   DNX_SAND_IN  uint32   rate,
   DNX_SAND_OUT uint32   *autocr_rate_bits
   )
{
    uint32
       res,
       credit_worth,
       device_ticks_per_sec,
       autocr_rate = 0; /* The credit rate in [clocks/credit] */

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_RATE2AUTOCREDIT_RATE);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
       res, 2, exit, JER2_ARAD_GET_ERR_TEXT_001, MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit,&credit_worth)));

    if (rate == 0) /* Disabling the auto-generation */
    {
        *autocr_rate_bits = 0;
    } else
    {
        device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

        res = dnx_sand_kbits_per_sec_to_clocks(
           rate * 1000,
           credit_worth,
           device_ticks_per_sec,
           &autocr_rate
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        if (autocr_rate <= JER2_ARAD_DBG_AUTOCREDIT_RATE_MIN_TH)
        {
            *autocr_rate_bits = JER2_ARAD_DBG_AUTOCREDIT_RATE_MIN_VALUE;
        } else if (autocr_rate >= JER2_ARAD_DBG_AUTOCREDIT_RATE_MAX_TH)
        {
            *autocr_rate_bits = JER2_ARAD_DBG_AUTOCREDIT_RATE_MAX_VALUE;
        } else /* autocr_rate between 15 and 2^19 */
        {
            *autocr_rate_bits = dnx_sand_log2_round_down(autocr_rate) - JER2_ARAD_DBG_AUTOCREDIT_RATE_OFFSET;
        }
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_rate2autocredit_rate()", 0, 0);
}

/*********************************************************************
* NAME:
*     jer2_arad_dbg_autocredit_rate2rate
* FUNCTION:
*   Calculate the AutoCredit rate (four bits) based on the rate in Mbps.
* INPUT:
*   DNX_SAND_IN  uint32  autocr_rate_bits
*     The four bits computed from the rate.
*   DNX_SAND_OUT uint32  *rate -
*     The rate in Mbps, where
*     rate[Mbps]=CreditSize[Bytes/Credit]*8[bits/Bytes]*1000 /
*     (2^(autocr_rate_bits+3)[clocks/credit] * clock_rate[ns/clock])
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   None.
*********************************************************************/
static uint32
jer2_arad_dbg_autocredit_rate2rate(
   DNX_SAND_IN  int  unit,
   DNX_SAND_IN  uint32   autocr_rate_bits,
   DNX_SAND_OUT uint32   *rate
   )
{
    uint32
       res,
       credit_worth,
       device_ticks_per_sec,
       autocr_rate = 0; /* The credit rate in [clocks/credit] */

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_AUTOCREDIT_RATE2RATE);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
       res, 2, exit, JER2_ARAD_GET_ERR_TEXT_001, MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_mgmt_credit_worth_get, (unit,&credit_worth)));

    if (autocr_rate_bits == 0) /* Disabling the auto-generation */
    {
        *rate = 0;
    } else
    {
        device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

        autocr_rate = dnx_sand_power_of_2(autocr_rate_bits + JER2_ARAD_DBG_AUTOCREDIT_RATE_OFFSET);

        res = dnx_sand_clocks_to_kbits_per_sec(
           autocr_rate,
           credit_worth,
           device_ticks_per_sec,
           rate      /* in Kbits/sec */
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        *rate /= 1000;
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_autocredit_rate2rate()", 0, 0);
}



/*********************************************************************
*     Configure the Scheduler AutoCredit parameters.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
jer2_arad_dbg_autocredit_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  DNX_TMC_DBG_AUTOCREDIT_INFO *info,
   DNX_SAND_OUT uint32                  *exact_rate
   )
{
    uint32
       fld_val,
       fld_val2,
       autocr_rate = 0,
       res;



    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_AUTOCREDIT_SET_UNSAFE);

    DNX_SAND_CHECK_NULL_INPUT(info);
    DNX_SAND_CHECK_NULL_INPUT(exact_rate);



    if (info->first_queue > info->last_queue)
    {
        fld_val = 0;
        fld_val2 = JER2_ARAD_MAX_QUEUE_ID(unit);
    } else
    {
        fld_val = info->first_queue;
        fld_val2 = info->last_queue;
    }
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IPS_AUTO_CREDIT_MECHANISM_FIRST_QUEUEr, SOC_CORE_ALL, 0, AUTO_CR_FRST_QUEf,  fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IPS_AUTO_CREDIT_MECHANISM_LAST_QUEUEr, SOC_CORE_ALL, 0, AUTO_CR_LAST_QUEf,  fld_val2));
    /* Computation of the right AutoCredit rate */
    if (info->rate == 0) /* Disabling Autocredit*/
    {
        autocr_rate = 0;
        *exact_rate = 0;
    } else
    {
        res = jer2_arad_dbg_rate2autocredit_rate(
           unit,
           info->rate,
           &autocr_rate
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        res = jer2_arad_dbg_autocredit_rate2rate(
           unit,
           autocr_rate,
           exact_rate
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 55, exit);
    }
    fld_val = autocr_rate;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IPS_AUTO_CREDIT_MECHANISM_RATE_CONFIGURATIONr, SOC_CORE_ALL, 0, AUTO_CR_RATEf,  fld_val));


exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_autocredit_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Configure the Scheduler AutoCredit parameters.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
jer2_arad_dbg_autocredit_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  DNX_TMC_DBG_AUTOCREDIT_INFO *info
   )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_AUTOCREDIT_VERIFY);

    DNX_SAND_CHECK_NULL_INPUT(info);

    DNX_SAND_MAGIC_NUM_VERIFY(info);

    DNX_SAND_ERR_IF_ABOVE_MAX(
       info->first_queue, JER2_ARAD_MAX_QUEUE_ID(unit),
       JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 10, exit
       );

    DNX_SAND_ERR_IF_ABOVE_MAX(
       info->last_queue, JER2_ARAD_MAX_QUEUE_ID(unit),
       JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 20, exit
       );

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_autocredit_verify()", 0, 0);
}

/*********************************************************************
*     Configure the Scheduler AutoCredit parameters.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
jer2_arad_dbg_autocredit_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_OUT DNX_TMC_DBG_AUTOCREDIT_INFO *info
   )
{
    uint32
       fld_val,
       res;



    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_AUTOCREDIT_GET_UNSAFE);

    DNX_SAND_CHECK_NULL_INPUT(info);



    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, IPS_AUTO_CREDIT_MECHANISM_FIRST_QUEUEr, SOC_CORE_ALL, 0, AUTO_CR_FRST_QUEf, &info->first_queue));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, IPS_AUTO_CREDIT_MECHANISM_FIRST_QUEUEr, SOC_CORE_ALL, 0, AUTO_CR_LAST_QUEf, &info->last_queue));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, IPS_AUTO_CREDIT_MECHANISM_RATE_CONFIGURATIONr, SOC_CORE_ALL, 0, AUTO_CR_RATEf, &fld_val));

    if (fld_val == 0) /* Disabling Autocredit*/
    {
        info->rate = 0;
    } else
    {
        res = jer2_arad_dbg_autocredit_rate2rate(
           unit,
           fld_val,
           &(info->rate)
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_autocredit_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Enable/disable the egress shaping.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
jer2_arad_dbg_egress_shaping_enable_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint8                 enable
   )
{
    uint32
       res,
       fld_val,
       reg_val;
    int core;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_SET_UNSAFE);

    fld_val = DNX_SAND_BOOL2NUM(enable);

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core)
    {
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 1, exit, JER2_ARAD_REG_ACCESS_ERR, READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));

        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, OTM_SPR_ENAf, fld_val);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, QPAIR_SPR_ENAf, fld_val);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, TCG_SPR_ENAf, fld_val);

        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 2, exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val));
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_egress_shaping_enable_set_unsafe()", 0, 0);
}

uint32
jer2_arad_dbg_egress_shaping_enable_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_OUT uint8                 *enable
   )
{
    uint32
       res,
       fld_val,
       reg_val;



    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_GET_UNSAFE);

    DNX_SAND_CHECK_NULL_INPUT(enable);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 1, exit, JER2_ARAD_REG_ACCESS_ERR, READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, 0, &reg_val));
    fld_val = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_ENAf);

    *enable = DNX_SAND_NUM2BOOL(fld_val);

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_egress_shaping_enable_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Enable/disable device-level flow control.
*     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
jer2_arad_dbg_flow_control_enable_set_unsafe(
   DNX_SAND_IN  int  unit,
   DNX_SAND_IN  uint8  enable
   )
{
    uint32
       res,
       fld_val;



    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_SET_UNSAFE);

    fld_val = DNX_SAND_BOOL2NUM(enable);
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, CFC_CFC_ENABLERSr, REG_PORT_ANY, 0, CFC_ENf,  fld_val));


exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_flow_control_enable_set_unsafe()", 0, 0);
}

uint32
jer2_arad_dbg_flow_control_enable_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_OUT uint8                 *enable
   )
{
    uint32
       res,
       fld_val;



    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_GET_UNSAFE);

    DNX_SAND_CHECK_NULL_INPUT(enable);



    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, CFC_CFC_ENABLERSr, REG_PORT_ANY, 0, CFC_ENf, &fld_val));
    *enable = DNX_SAND_NUM2BOOL(fld_val);

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_flow_control_enable_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Resets the ingress pass. The following blocks are
*     soft-reset (running soft-init): IPS, IQM, IPT, MMU,
*     DPRC, IRE, IDR, IRR. As part of the reset sequence,
*     traffic is stopped, and re-started (according to the
*     original condition).
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_dbg_ingr_reset_unsafe(
   DNX_SAND_IN  int                 unit
   )
{
    uint32
       res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_INGR_RESET_UNSAFE);

    res = jer2_arad_dbg_dev_reset_unsafe(
       unit,
       JER2_ARAD_DBG_RST_DOMAIN_INGR
       );
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_ingr_reset_unsafe()", 0, 0);
}

/*********************************************************************
 *     Soft-resets the device. As part of the reset sequence,
 *     traffic is stopped, and re-started (according to the
 *     original condition).
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
uint32 jer2_arad_dbg_dev_reset_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_DBG_RST_DOMAIN      rst_domain)
{
    uint32
       autogen_reg_val,
       fmc_scheduler_configs_reg_val_orig,
       fld32_val,
       res = DNX_SAND_OK;
    uint8
       is_traffic_enabled_orig,
       is_ctrl_cells_enabled_orig;
    uint8
       is_ingr,
       is_egr,
       is_fabric = 0;
    uint64
       soft_init_reg_val,
       soft_init_reg_val_orig,
       soft_init_reg_val_orig_wo_clp,
       fld64_val;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_DEV_RESET_UNSAFE);

    /* This lock is added since threads might access the device during soft reset, causing schan timeout */
    SCHAN_LOCK(unit);

    is_ingr   = DNX_SAND_NUM2BOOL((rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC));
    is_egr    = DNX_SAND_NUM2BOOL((rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC));
    is_fabric = DNX_SAND_NUM2BOOL((rst_domain == JER2_ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC) ||
                                  (rst_domain == JER2_ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC));

    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): Start. is_ingr=%d, is_egr=%d, is_fabric=%d\n"), FUNCTION_NAME(), is_ingr, is_egr, is_fabric));

    /************************************************************************/
    /* Disable Traffic                                                      */
    /************************************************************************/
    /*
     *  Store current traffic-enable-state (just in case: if we got here, it is enabled)
    */
    res = jer2_arad_mgmt_enable_traffic_get(unit, &is_traffic_enabled_orig);
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): Disable Traffic. is_traffic_enabled_orig=%d\n"), FUNCTION_NAME(), is_traffic_enabled_orig));

    res = jer2_arad_mgmt_enable_traffic_set(unit, FALSE);
    DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    if (is_fabric == TRUE)
    {
        /*
         *  Store current traffic-enable-state (just in case: if we got here, it is enabled)
         */
        res = jer2_arad_mgmt_all_ctrl_cells_enable_get(unit, &is_ctrl_cells_enabled_orig);
        DNX_SAND_CHECK_FUNC_RESULT(res, 16, exit);

        /* calling unsafe methods is discouraged. in this case we do it because jer2_arad_dbg_dev_reset is always called safely */
        res = jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(unit, FALSE, JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET);
        DNX_SAND_CHECK_FUNC_RESULT(res, 18, exit);
    }

    /************************************************************************/
    /* Validate Data Path is clean - active queue = 0                       */
    /************************************************************************/

    if (is_egr)
    {

        LOG_VERBOSE(BSL_LS_SOC_INIT,
                    (BSL_META_U(unit,
                                "%s(): Validate Data Path is clean- egr.\n"), FUNCTION_NAME()));

        res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, CGM_TOTAL_PD_CNTr, REG_PORT_ANY, 0, TOTAL_PD_CNTf, 0x0);
        if (dnx_sand_update_error_code(res, &ex) != no_err)
        {
            LOG_ERROR(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "%s(): Error Validating Dtat Path is clean: CGM_TOTAL_PD_CNTr, TOTAL_PD_CNTf.\n"), FUNCTION_NAME()));
        }

        res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, CGM_TOTAL_DB_CNTr, REG_PORT_ANY, 0, TOTAL_DB_CNTf, 0x0);
        if (dnx_sand_update_error_code(res, &ex) != no_err)
        {
            LOG_ERROR(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "%s(): Error Validating Dtat Path is clean: CGM_TOTAL_DB_CNTr, TOTAL_DB_CNTf.\n"), FUNCTION_NAME()));
        }
    }

    /************************************************************************/
    /* Read original configuration                                          */
    /************************************************************************/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_ECI_BLOCKS_SOFT_INITr_REG64(unit, &soft_init_reg_val));
    soft_init_reg_val_orig = soft_init_reg_val;
    soft_init_reg_val_orig_wo_clp = soft_init_reg_val;

    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): Read original configuration. soft_init_reg_val_orig=0x%x,0x%x\n"), FUNCTION_NAME(), COMPILER_64_HI(soft_init_reg_val_orig), COMPILER_64_LO(soft_init_reg_val_orig)));
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    sal_memcpy(dram_orig_enabled, SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.is_valid, sizeof(uint8) * SOC_DNX_DEFS_GET(unit, hw_dram_interfaces_max));
#endif 
    /************************************************************************/
    /* IN-RESET                                                             */
    /************************************************************************/
    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): IN-RESET\n"), FUNCTION_NAME()));
    COMPILER_64_SET(fld64_val, 0, 0x1);
    if (is_ingr)
    {

        if (!SOC_IS_ARDON(unit))
        {
            /* caching IHB_OPCODE_MAP_RX/TX memories in order to rewrite them after soft reset */
            if (soc_mem_cache_get(unit, IHB_OPCODE_MAP_RXm, SOC_MEM_BLOCK_MIN(unit, IHB_OPCODE_MAP_RXm)) == FALSE)
            {
                if (soc_mem_cache_set(unit, IHB_OPCODE_MAP_RXm, COPYNO_ALL, TRUE))
                {
                    LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Caching IHB_OPCODE_MAP_RXm for rewrite after soft reset not succeeded\n")));
                }
            }
            if (soc_mem_cache_get(unit, IHB_OPCODE_MAP_TXm, SOC_MEM_BLOCK_MIN(unit, IHB_OPCODE_MAP_TXm)) == FALSE)
            {
                if (soc_mem_cache_set(unit, IHB_OPCODE_MAP_TXm, COPYNO_ALL, TRUE))
                {
                    LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Caching IHB_OPCODE_MAP_TXm for rewrite after soft reset not succeeded\n")));
                }
            }
            if (SOC_IS_ARADPLUS(unit))
            {
                /* caching OAMP_PE_PROG_TCAM memory in order to rewrite it after soft reset */
                if (soc_mem_cache_get(unit, OAMP_PE_PROG_TCAMm, SOC_MEM_BLOCK_MIN(unit, OAMP_PE_PROG_TCAMm)) == FALSE)
                {
                    if (soc_mem_cache_set(unit, OAMP_PE_PROG_TCAMm, COPYNO_ALL, TRUE))
                    {
                        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Caching OAMP_PE_PROG_TCAMm for rewrite after soft reset not succeeded\n")));
                    }
                }
            }
        }

        /* caching IRR_SMOOTH_DIVISION memory in order to rewrite it after soft reset */
        if (soc_mem_cache_get(unit, IRR_SMOOTH_DIVISIONm, SOC_MEM_BLOCK_MIN(unit, IRR_SMOOTH_DIVISIONm)) == FALSE)
        {
            if (soc_mem_cache_set(unit, IRR_SMOOTH_DIVISIONm, COPYNO_ALL, TRUE))
            {
                LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Caching IRR_SMOOTH_DIVISIONm for rewrite after soft reset not succeeded\n")));
            }
        }

        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_IPS_FMC_SCHEDULER_CONFIGSr(unit, &fmc_scheduler_configs_reg_val_orig));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  25,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IPS_FMC_SCHEDULER_CONFIGSr(unit,  0x04000000));
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        res = jer2_arad_kbp_recover_rx_shut_down(unit, elk->kbp_mdio_id[0]);
        DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);
#endif
#endif 
        /*
         *  Soft-init: put in-reset IPS, IQM, IPT, MMU, DPRC, IRE, IDR, IRR, FDT, PDM
         */

        /* PDM sould do hard reset */
        COMPILER_64_TO_32_LO(fld32_val, fld64_val);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IQM_IQM_ENABLERSr, SOC_CORE_ALL, 0, PDM_INIT_ENf,  fld32_val));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  34,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_RESETr, REG_PORT_ANY, 0, PDM_RESETf,  fld64_val));

        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IPS_INITf, fld64_val, soft_init_reg_val, 36, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IQM_INITf, fld64_val, soft_init_reg_val, 38, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IPT_INITf, fld64_val, soft_init_reg_val, 40, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, MMU_INITf, fld64_val, soft_init_reg_val, 42, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, OCB_INITf, fld64_val, soft_init_reg_val, 44, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IRE_INITf, fld64_val, soft_init_reg_val, 46, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IDR_INITf, fld64_val, soft_init_reg_val, 48, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IRR_INITf, fld64_val, soft_init_reg_val, 50, exit);
        if (is_fabric == 0x1)
        {
            JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, FDT_INITf, fld64_val, soft_init_reg_val, 52, exit);
            JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, FCT_INITf, fld64_val, soft_init_reg_val, 54, exit);
        }
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, OAMP_INITf, fld64_val, soft_init_reg_val, 56, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IHP_INITf, fld64_val, soft_init_reg_val, 58, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IHB_INITf, fld64_val, soft_init_reg_val, 60, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CFC_INITf, fld64_val, soft_init_reg_val, 62, exit);

        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val));
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        res = jer2_arad_mgmt_dram_init_drc_soft_init(unit, dram_orig_enabled, 0x1);
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
#endif 
        /* Reseting CMICM TXI credits */
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x40));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_CMIC_TXBUF_IPINTF_INTERFACE_CREDITSr(unit, 0x0));

    } /* is_ingr */

    if (is_egr)
    {

        /* Close FCR */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1003,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_FL_STSf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1004,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_CRD_FCRf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1005,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_SRf,  0x1));

        /* Rest scheduler (in-out) */
        res = jer2_arad_dbg_sch_reset_unsafe(unit);
        DNX_SAND_CHECK_FUNC_RESULT(res, 1070, exit);

        COMPILER_64_SET(fld64_val, 0, 0x1);

        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, EGQ_INITf, fld64_val, soft_init_reg_val, 1030, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, EPNI_INITf, fld64_val, soft_init_reg_val, 1031, exit);
        if (is_fabric == 0x1)
        {
            JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, FDR_INITf, fld64_val, soft_init_reg_val, 1032, exit);
            JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, FCR_INITf, fld64_val, soft_init_reg_val, 1034, exit);
        }
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, OLP_INITf, fld64_val, soft_init_reg_val, 1034, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CFC_INITf, fld64_val, soft_init_reg_val, 39, exit);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1040,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val));

        /* Resetting CMICM RXI credits */
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITSr(unit, 0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EPNI_INIT_TXI_CONFIGr, SOC_CORE_ALL, 0, INIT_TXI_CMICMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_CMICMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_CMICMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OLPr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OLPf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OAMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OAMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_RCYr, SOC_CORE_ALL, 0, INIT_FQP_TXI_RCYf,  0x1));

    } /* is_egr */

    /* NIF Reset, currently at full reset */
    if (is_ingr && is_egr)
    {

        /* Reseting CLP/CLP before NBI */
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CLP_0_INITf, fld64_val, soft_init_reg_val, 24, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CLP_1_INITf, fld64_val, soft_init_reg_val, 25, exit);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  28,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val));

        /* Reseting NBI */
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, NBI_INITf, fld64_val, soft_init_reg_val, 27, exit); /* Adjusting the bitmap */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, NBI_INITf,  0x1));
    }

    /************************************************************************/
    /* OUT-OF-RESET                                                         */
    /************************************************************************/
    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): OUT-OF-RESET.\n"), FUNCTION_NAME()));
    COMPILER_64_ZERO(fld64_val);
    if (is_ingr)
    {

        /* Take PDM out of hard reset */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_RESETr, REG_PORT_ANY, 0, PDM_RESETf,  fld64_val));

        /* IDR + MMU - out of soft init */
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, IDR_INITf, fld64_val, soft_init_reg_val, 50, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, MMU_INITf, fld64_val, soft_init_reg_val, 52, exit);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val));

        /* after soft init is done,  IDR_CONTEXT_MRU table should be set to (16k-128) since it's
           the biggest packet size acceptable by egress */
        DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_mgmt_set_mru_by_dbuff_size(unit), 120, exit);

        /* 
         *  Reset Auto-gen (Except FbcInternalReuse bit (0x4))
         */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_IDR_STATIC_CONFIGURATIONr(unit, &autogen_reg_val));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  62,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IDR_STATIC_CONFIGURATIONr(unit,  0x4));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  64,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IDR_STATIC_CONFIGURATIONr(unit,  autogen_reg_val));
          
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        /* DPRC - out of soft init*/
        res = jer2_arad_mgmt_dram_init_drc_soft_init(unit, dram_orig_enabled, 0x0);
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
#endif 
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2160,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IPS_FMC_SCHEDULER_CONFIGSr(unit,  fmc_scheduler_configs_reg_val_orig));
    } /* is_ingr */

    /* NIF Reset, currently at full reset */
    if (is_ingr && is_egr)
    {

        COMPILER_64_SET(fld64_val, 0, 0x1);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CLP_0_INITf, fld64_val, soft_init_reg_val_orig_wo_clp, 24, exit);
        JER2_ARAD_FLD_TO_REG64(ECI_BLOCKS_SOFT_INITr, CLP_1_INITf, fld64_val, soft_init_reg_val_orig_wo_clp, 25, exit);
    }

    /************************************************************************/
    /* OUT-OF-RESET, Revert to original (Soft-init per-block map)           */
    /************************************************************************/
    /* soft_init_reg_val_orig is probably 0x0 - take all other blocks out of soft init */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val_orig_wo_clp));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_ECI_BLOCKS_SOFT_INITr_REG64(unit, soft_init_reg_val_orig));

    /* reconfigure wiped tables with cache value */
    if (is_ingr)
    {
        if (!SOC_IS_ARDON(unit))
        {
            jer2_arad_update_table_with_cache(unit, IHB_OPCODE_MAP_RXm);
            jer2_arad_update_table_with_cache(unit, IHB_OPCODE_MAP_TXm);
            if (SOC_IS_ARADPLUS(unit))
            {
                jer2_arad_update_table_with_cache(unit, OAMP_PE_PROG_TCAMm);
            }
        }

        jer2_arad_update_table_with_cache(unit, IRR_SMOOTH_DIVISIONm);
    }
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    res = jer2_arad_kbp_recover_rx_enable(unit, elk->kbp_mdio_id[0]);
    DNX_SAND_CHECK_FUNC_RESULT(res, 29, exit);
    /* KBP Recovery */
    if (elk->kbp_recover_enable)
    {
        res = jer2_arad_kbp_recover_run_recovery_sequence(unit, 0, elk->kbp_mdio_id[0], elk->kbp_recover_iter, 1);
    }
    DNX_SAND_CHECK_FUNC_RESULT(res, 32, exit);
#endif
#endif 
    /************************************************************************/
    /* Validate/Poll for out-of-reset/init-done indications                 */
    /************************************************************************/
    sal_usleep(1000);

    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): Validate/Poll for out-of-reset/init-done indications.\n"), FUNCTION_NAME()));
    res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, 0, IPS_INIT_TRIGGERf, 0x0);
    if (dnx_sand_update_error_code(res, &ex) != no_err)
    {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "%s(): Error Validate out-of-reset done indications: IPS_IPS_GENERAL_CONFIGURATIONSr, IPS_INIT_TRIGGERf.\n"), FUNCTION_NAME()));
    }

    res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, REG_PORT_ANY, 0, PDM_INITf, 0x0);
    if (dnx_sand_update_error_code(res, &ex) != no_err)
    {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "%s(): Error Validate out-of-reset done indications: IQM_IQM_INITr, IQC_INITf.\n"), FUNCTION_NAME()));
    }

    res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EGQ_EGQ_BLOCK_INIT_STATUSr, REG_PORT_ANY, 0, EGQ_BLOCK_INITf, 0x0);
    if (dnx_sand_update_error_code(res, &ex) != no_err)
    {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "%s(): Error Validate out-of-reset done indications: EGQ_EGQ_BLOCK_INIT_STATUSr, EGQ_BLOCK_INITf.\n"), FUNCTION_NAME()));
    }

    if (is_egr)
    {

        /* Open FCR */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2003,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_FL_STSf,  0x0));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2004,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_CRD_FCRf,  0x0));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2005,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr, REG_PORT_ANY, 0, DIS_SRf,  0x0));

        /* Resetting CMICM RXI credits */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EPNI_INIT_TXI_CONFIGr, SOC_CORE_ALL, 0, INIT_TXI_CMICMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_CMICMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_CMICMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OLPr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OLPf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_OAMr, SOC_CORE_ALL, 0, INIT_FQP_TXI_OAMf,  0x1));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_INIT_FQP_TXI_RCYr, SOC_CORE_ALL, 0, INIT_FQP_TXI_RCYf,  0x1));

    } /* is_egr */

    /************************************************************************/
    /* Restore Configuration if needed                                      */
    /************************************************************************/
    if (is_fabric == 0x1)
    {
        res = jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(unit, is_ctrl_cells_enabled_orig, JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET);
        DNX_SAND_CHECK_FUNC_RESULT(res, 18, exit);
    }

    /************************************************************************/
    /*  Restore traffic                                                     */
    /************************************************************************/
    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "%s(): Restore traffic.\n"), FUNCTION_NAME()));
    res = jer2_arad_mgmt_enable_traffic_set(unit, is_traffic_enabled_orig);
    DNX_SAND_CHECK_FUNC_RESULT(res, 3000, exit);

    /************************************************************************/
    /*  Clear interrupts                                                    */
    /************************************************************************/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3001,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IQM_INTERRUPT_REGISTERr(unit, SOC_CORE_ALL,  0xffffffff));

exit:
    SCHAN_UNLOCK(unit);
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_dev_reset_unsafe()", rst_domain, 0);
}

uint32 jer2_arad_dbg_dev_reset_verify(
   DNX_SAND_IN  JER2_ARAD_DBG_RST_DOMAIN      rst_domain)
{
    DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_DBG_DEV_RESET_VERIFY);

    DNX_SAND_ERR_IF_ABOVE_MAX(rst_domain, JER2_ARAD_DEBUG_RST_DOMAIN_MAX, JER2_ARAD_DEBUG_RST_DOMAIN_OUT_OF_RANGE_ERR, 10, exit);

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_dev_reset_verify()", rst_domain, 0);
}

/*********************************************************************
 *     Resets the end-to-end scheduler. The reset is performed
 *     by clearing the internal scheduler pipes, and then
 *     performing soft-reset.
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
uint32 jer2_arad_dbg_sch_reset_unsafe(
   DNX_SAND_IN  int unit)
{
    uint32
       mc_conf_0_fld_val,
       mc_conf_1_fld_val,
       ingr_shp_en_fld_val,
       res = DNX_SAND_OK;
    JER2_ARAD_SCH_SCHEDULER_INIT_TBL_DATA
       init_tbl;
    uint32
       tbl_data[JER2_ARAD_SCH_SCHEDULER_INIT_TBL_ENTRY_SIZE] = { 0 };

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DBG_SCH_RESET_UNSAFE);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_DVS_CONFIG_1r, REG_PORT_ANY, 0, FORCE_PAUSEf,  0x1));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, SOC_CORE_ALL, 0, DISABLE_FABRIC_MSGSf,  0x1));

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1006,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r(unit, SOC_CORE_ALL, &mc_conf_0_fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1006,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, &mc_conf_1_fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r, SOC_CORE_ALL, 0, MULTICAST_GFMC_ENABLEf,  0x0));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r, SOC_CORE_ALL, 0, MULTICAST_BFMC_1_ENABLEf,  0x0));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r, SOC_CORE_ALL, 0, MULTICAST_BFMC_2_ENABLEf,  0x0));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1026,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r, SOC_CORE_ALL, 0, MULTICAST_BFMC_3_ENABLEf,  0x0));

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, SOC_CORE_ALL, 0, INGRESS_SHAPING_ENABLEf, &ingr_shp_en_fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  72,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, SOC_CORE_ALL, 0, INGRESS_SHAPING_ENABLEf,  0x0));

    /*
    * Soft reset the scheduler
    */
    init_tbl.schinit = 0x0;

    res = jer2_arad_sch_scheduler_init_tbl_set_unsafe(unit, 0x0, &init_tbl);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    soc_mem_field32_set(unit, SCH_MEM_01F00000m, tbl_data, ITEM_0_0f, 0x1);

    /*
    * Recover original configuration
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_DVS_CONFIG_1r, REG_PORT_ANY, 0, FORCE_PAUSEf,  0x0));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, SOC_CORE_ALL, 0, DISABLE_FABRIC_MSGSf,  0x0));

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2022,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r(unit, SOC_CORE_ALL,  mc_conf_0_fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2022,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL,  mc_conf_1_fld_val));

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  170,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, SCH_INGRESS_SHAPING_PORT_CONFIGURATIONr, SOC_CORE_ALL, 0, INGRESS_SHAPING_ENABLEf,  ingr_shp_en_fld_val));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_dbg_sch_reset_unsafe()", 0, 0);
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */

