/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_jer_egr_queuing.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/SAND/Utils/sand_occupation_bitmap.h>

#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_device.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_flow_converts.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>

#include <soc/dnx/legacy/JER/jer_sch.h>
#include <soc/dnx/legacy/JER/jer_tbls.h>


#define CAL_0_1_SIZE  1024
#define CAL_2_15_SIZE 256
#define JER2_JER_SCH_HR_PRIORITY_LOW_VAL  (0)
#define JER2_JER_SCH_HR_PRIORITY_HIGH_VAL (15)

/* this value, ASSIGNED/REBOUNDED_CREDIT_WORTH is the divider
   in the port bandwidth equation:
                  QuantaToAdd*CreditSize*8
                 ------------------------
          REBOUNDED_CREDIT_WORTH*AccessPeriod*ClkCycle
*/
#define JER2_JER_SCH_CREDIT_WORTH_DIV_MIN (1)  /*ASSIGNED/REBOUNDED_CREDIT_WORTH min*/
#define JER2_JER_SCH_CREDIT_WORTH_DIV_MAX (0xffff) /*ASSIGNED/REBOUNDED_CREDIT_WORTH max*/

#define JER2_JER_SCH_SHAPERS_QUANTA_TO_ADD_MAX (0x7ff)

/* Declerations of private functinos */
uint32
_jer2_jer_sch_rates_switch_calendars(int unit, int core, int new_calendar_table_num, int new_divider);
uint32
_jer2_jer_sch_rates_calculate_new_calendars(int unit, int core, int max_rate_increased, int *new_rate_configured);
uint32
_jer2_jer_sch_rates_calculate_new_divider(int unit, int core, int hr_calendar_table_num, int max_rate_increased, int old_divider ,uint32 *new_divider);





int
  soc_jer2_jer_sch_init(
    DNX_SAND_IN   int                    unit
    )
{
    soc_pbmp_t pbmp;
    soc_port_t port_i;
    uint32 reg_val, flags, rate_kbps, slow_level, slow_type, mem_val32;
    int core;
    uint32 slow_rate_max_level;
    char* slow_rate_max_level_str;
    int default_slow_rates[8] = {3, 6, 12, 24, 36, 48, 64, 96};
    uint32 is_skip_for_lb = 0;

    DNXC_INIT_FUNC_DEFS;

    /* allocate egq to e2e interface mapping */
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags));
        if (!(DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags) || is_skip_for_lb)) {
            DNXC_IF_ERR_EXIT(soc_jer2_jer_sch_e2e_interface_allocate(unit, port_i));
        }
    }

    /* init e2e registers */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_scheduler_end2end_regs_init(unit));


    /* Set default values for priority propegation */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) 
    {
        /* 0 for low prio, 1 for high prio*/
        DNXC_IF_ERR_EXIT(READ_SCH_HR_PRIORITY_MASKr(unit, core, 0, &reg_val));
        soc_reg_field_set(unit, SCH_HR_PRIORITY_MASKr, &reg_val, HR_PRIORITY_MASK_Nf, JER2_JER_SCH_HR_PRIORITY_LOW_VAL);
        DNXC_IF_ERR_EXIT(WRITE_SCH_HR_PRIORITY_MASKr(unit, core, 0, reg_val));
        
        DNXC_IF_ERR_EXIT(READ_SCH_HR_PRIORITY_MASKr(unit, core, 1, &reg_val));
        soc_reg_field_set(unit, SCH_HR_PRIORITY_MASKr, &reg_val, HR_PRIORITY_MASK_Nf, JER2_JER_SCH_HR_PRIORITY_HIGH_VAL);
        DNXC_IF_ERR_EXIT(WRITE_SCH_HR_PRIORITY_MASKr(unit, core, 1, reg_val)); 

    }

    /* Enable slow factor */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, &reg_val, SLOW_FACTOR_ENABLEf, 0x1);
        /* in JER2_QAX flow ID should be shifted by 64K */
        if (SOC_IS_QAX(unit)) {
            soc_reg_field_set(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, &reg_val, ADD_OFFSET_TO_FLOWf, 0x1);
        }
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core, reg_val));
    }

    slow_rate_max_level_str = soc_property_get_str(unit, spn_SLOW_MAX_RATE_LEVEL);

    if ( (slow_rate_max_level_str == NULL) || (sal_strcmp(slow_rate_max_level_str, "HIGH")==0) )
    {
        slow_rate_max_level = 3; /* 7 */
        SOC_DNX_CONFIG(unit)->jer2_arad->init.max_burst_default_value_bucket_width = (0x7f)*256;
    } 
    else if (sal_strcmp(slow_rate_max_level_str, "NORMAL") == 0)
    {
        slow_rate_max_level = 2; /* 8 */
        SOC_DNX_CONFIG(unit)->jer2_arad->init.max_burst_default_value_bucket_width = (0xff)*256;
    } 
    else if (sal_strcmp(slow_rate_max_level_str, "LOW") == 0)
    {
        slow_rate_max_level = 1; /* 9 */
        SOC_DNX_CONFIG(unit)->jer2_arad->init.max_burst_default_value_bucket_width = (0x1ff)*256;
    }
    else
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Unsupported properties: slow_rate_max_level should be LOW/NORMAL/HIGH")));
    }

    /* Write slow rate max level */
    DNXC_IF_ERR_EXIT(READ_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, &reg_val));
    soc_reg_field_set(unit, SCH_SHAPER_CONFIGURATION_REGISTER_1r, &reg_val, SLOW_MAX_BUCKET_WIDTHf, slow_rate_max_level); 
    soc_reg_field_set(unit, SCH_SHAPER_CONFIGURATION_REGISTER_1r, &reg_val, OVERRIDE_SLOW_RATEf, 1);
    DNXC_IF_ERR_EXIT(WRITE_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, SOC_CORE_ALL, reg_val));
    
    for(slow_level=0 ; slow_level<8 ; slow_level++) {
        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_A_SSAm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), slow_level, &mem_val32));
        soc_mem_field32_set(unit, SCH_SLOW_SCALE_A_SSAm, &mem_val32, MAX_BUCKETf, 0x7);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SLOW_SCALE_A_SSAm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), slow_level, &mem_val32));

        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_B_SSBm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), slow_level, &mem_val32));
        soc_mem_field32_set(unit, SCH_SLOW_SCALE_B_SSBm, &mem_val32, MAX_BUCKETf, 0x7);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SLOW_SCALE_B_SSBm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), slow_level, &mem_val32));
    }
 
    for(slow_level=0 ; slow_level<8 ; slow_level++) {

        rate_kbps = default_slow_rates[slow_level]*1000000;

        for(slow_type=1 ; slow_type<=2 ; slow_type++) {
            DNXC_IF_ERR_EXIT(jer2_jer_sch_slow_max_rates_per_level_set(unit, SOC_CORE_ALL, slow_level, slow_type, rate_kbps));
        }
    }

    /* enable DLM for all groups */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core){
        DNXC_IF_ERR_EXIT(READ_SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr, &reg_val, DLM_ENABLEf, 0x7);
        DNXC_IF_ERR_EXIT(WRITE_SCH_DELETE_MECHANISM_CONFIGURATION_REGISTERr(unit, core, reg_val));
    }

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        DNXC_IF_ERR_EXIT(READ_CFC_RESERVED_SPARE_0r(unit, &reg_val));
        soc_reg_field_set(unit, CFC_RESERVED_SPARE_0r, &reg_val, SCH_OOB_ERR_VALID_CFGf, 0x1);
        DNXC_IF_ERR_EXIT(WRITE_CFC_RESERVED_SPARE_0r(unit, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}
/*********************************************************************
*     This function gets an entry in the device rate table, per core.
*     This function gets an entry in the device rate table.
*     Each entry contains a credit generation rate, for a given
*     pair of fabric congestion (presented by rci_level) and
*     the number of active fabric links.
*     This procedure is only useful for multi-core architectures.
*     The driver reads from the following tables:
*     Device Rate Memory (DRM)
*     For Jericho, this table is SCH_DEVICE_RATE_MEMORY_DRMm
*     Details: in the H file. (search for prototype)
*     Note that validity of input parameters is assumed to have
*     been checked by calling procedures.
* See also:
*   jer2_arad_sch_device_rate_entry_get_unsafe()
*********************************************************************/
uint32
  jer2_jer_sch_device_rate_entry_core_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              rci_level_ndx,
    DNX_SAND_IN  uint32              nof_active_links_ndx,
    DNX_SAND_OUT uint32              *rate
  )
{
  uint32
    interval_in_clock_128_th,
    calc,
    offset,
  credit_worth,
    res;
  JER2_ARAD_SCH_DRM_TBL_DATA
    drm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DEVICE_RATE_ENTRY_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(rate);

  /*
   * Read from DRM table
   */
  offset = (nof_active_links_ndx * SOC_DNX_DEFS_GET(unit, nof_rci_levels)) + rci_level_ndx;
  res = jer2_jer_sch_drm_tbl_get_unsafe(
          unit, core,
          offset,
          &drm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 4, exit);

  interval_in_clock_128_th = drm_tbl_data.device_rate;
  /*
   *  Calculate Device Credit Rate Generation (in Mbits/secs) according to:
   *
   *
   *                       Credit [bits] * Num_of_Mega_clocks_128th_in_sec [(M * clocks)/(128 * sec)]
   *  Rate [Mbits/Sec] =   -----------------------------------------------------------------------
   *                          interval_between_credits_in_clock_128th [clocks/128]
   */
  if (0 == interval_in_clock_128_th)
  {
    *rate = 0;
  }
  else
  {
    /*
     * Get 'credit worth' value using device driver specific procedure.
     */
    res = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mgmt_credit_worth_get,(unit, &credit_worth))) ;
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
    calc = (credit_worth * DNX_SAND_NOF_BITS_IN_CHAR) *
           (jer2_arad_chip_mega_ticks_per_sec_get(unit) * JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_RESOLUTION);
    calc = DNX_SAND_DIV_ROUND(calc, interval_in_clock_128_th);
    *rate = calc;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_sch_device_rate_entry_core_get_unsafe()",0,0);
}
/*********************************************************************
*     This function sets an entry in the device rate table, per core.
*     Each entry sets a credit generation rate, for a given
*     pair of fabric congestion (presented by rci_level) and
*     the number of active fabric links.
*     This procedure is only useful for multi-core architectures.
*     The driver writes to the following tables:
*     Device Rate Memory (DRM)
*     For Jericho, this table is SCH_DEVICE_RATE_MEMORY_DRMm
*     Details: in the H file. (search for prototype)
*     Note that validity of input parameters is assumed to have
*     been checked by calling procedures.
*
* See also:
*   jer2_arad_sch_device_rate_entry_set_unsafe
*********************************************************************/
uint32
  jer2_jer_sch_device_rate_entry_core_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              rci_level_ndx,
    DNX_SAND_IN  uint32              nof_active_links_ndx,
    DNX_SAND_IN  uint32              rate
  )
{
  uint32
    interval_in_clock_128_th,
    calc,
    offset,
    credit_worth,
    res;
  JER2_ARAD_SCH_DRM_TBL_DATA
    drm_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DEVICE_RATE_ENTRY_SET_UNSAFE);
  /*
   * Get 'credit worth' value using device driver specific procedure.
   */
  res = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mgmt_credit_worth_get,(unit, &credit_worth))) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 4, exit);
  /*
   *  Calculate interval between credits (in Clocks/64) according to:
   *
   *
   *                       Credit [bits] * Num_of_Mega_clocks_64th_in_sec [(M * clocks)/(64 * sec)]
   *  Rate [Mbits/Sec] =   -----------------------------------------------------------------------
   *                          interval_between_credits_in_clock_64th [clocks/64]
   */
  if (0 == rate)
  {
    interval_in_clock_128_th = 0;
  }
  else
  {
    calc = (credit_worth * DNX_SAND_NOF_BITS_IN_CHAR) *
           (jer2_arad_chip_mega_ticks_per_sec_get(unit) * JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_RESOLUTION);
    calc = DNX_SAND_DIV_ROUND(calc, rate);
    interval_in_clock_128_th = calc;
  }
  DNX_SAND_LIMIT_FROM_ABOVE(interval_in_clock_128_th, JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_MAX);
  if (interval_in_clock_128_th != 0)
  {
    DNX_SAND_LIMIT_FROM_BELOW(interval_in_clock_128_th, JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_MIN);
  }
  offset = (nof_active_links_ndx * SOC_DNX_DEFS_GET(unit, nof_rci_levels)) + rci_level_ndx;
  drm_tbl_data.device_rate = interval_in_clock_128_th;
  /*
   * Write indirect to DRM table, single entry
   */
  res = 0 ;
  res = jer2_jer_sch_drm_tbl_set_unsafe(
          unit, core,
          offset, &drm_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_jer_sch_device_rate_entry_core_set_unsafe()",0,0);
}


int 
soc_jer2_jer_sch_e2e_interface_allocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port
    )
{
    uint8           found = FALSE;
    uint32          num_of_big_calendars, num_of_channelized_interfaces, num_of_interfaces, e2e_if, tm_port, data_32, egress_offset, base_q_pair,
                    non_channelized_port_offset, is_master;
    int             is_channelized, core, start, end;
    soc_port_if_t   interface_type;
    soc_reg_above_64_val_t tbl_data;
    DNX_SAND_OCC_BM_PTR ifs_occ;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, port, &is_master));

    if (is_master) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_channelized_port_get(unit, port, &is_channelized)); 
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset)));

        num_of_channelized_interfaces = SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces);
        num_of_big_calendars = SOC_DNX_DEFS_GET(unit, nof_big_channelized_calendars);
        num_of_interfaces = SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);

        ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
        end = 1;
        /* handle ILKN dedicated mode */
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON && interface_type == SOC_PORT_IF_ILKN ) 
        {
            DNX_SAND_SET_BIT(egress_offset, 0, 0); /* handle fast ports first */
            end++;
	    }

        for (start = 0; start < end; start++) {
            egress_offset+= start; /* handle ILKN dedicated mode */
            /* The core can't be negative... */
            /* coverity[negative_returns:FALSE] */
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.e2e_interfaces_occ.get(unit, core, &ifs_occ);
            DNXC_IF_ERR_EXIT(rv);
            if (is_channelized) {
                /* Try to allocate big calendars for ILKN interface */
                if (interface_type == SOC_PORT_IF_ILKN) {
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, 0, (num_of_big_calendars-1), TRUE, &e2e_if, &found));
                }

                if (!found) {
                    /* try to allocate all channelized if's ID's*/
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, num_of_big_calendars, (num_of_channelized_interfaces-1), TRUE, &e2e_if, &found));
                }

                if (!found && interface_type != SOC_PORT_IF_ILKN) {
                    /* try to allocate all channelized if's ID's*/
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, 0, (num_of_channelized_interfaces-1), TRUE, &e2e_if, &found));
                }
            } else { /* non channelized interface */

                /* try to allocate non channelized e2e if's ID's*/
                DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, num_of_channelized_interfaces, (num_of_interfaces-1), TRUE, &e2e_if, &found));

                if (!found) {
                    /* try to allocate small channelized e2e if's ID's*/
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, num_of_big_calendars, (num_of_interfaces-1), TRUE, &e2e_if, &found));
                }

                if (!found) {
                    /* try to allocate big channelized e2e if's ID's*/
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_get_next_in_range(unit, ifs_occ, 0, (num_of_interfaces-1), TRUE, &e2e_if, &found));
                }

            }

            if (!found) {
                DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("No e2e interfaces are left for port %d \n"), port));
            } else {
                /* mark e2e interface as used */
                DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, ifs_occ, e2e_if, TRUE));

                /* Set HW SCH offset */
                data_32 = 0;
                SOC_REG_ABOVE_64_CLEAR(tbl_data);

                DNXC_IF_ERR_EXIT(READ_SCH_FC_MAP_FCMm(unit, SCH_BLOCK(unit, core), egress_offset, &data_32));
                soc_SCH_FC_MAP_FCMm_field32_set(unit, &data_32 ,FC_MAP_FCMf, e2e_if);
                DNXC_IF_ERR_EXIT(WRITE_SCH_FC_MAP_FCMm(unit,SCH_BLOCK(unit, core), egress_offset, &data_32));

                if (!JER2_ARAD_SCH_IS_CHNIF_ID(unit, e2e_if)) {
                    /* In case of non channelized port set also Port ID */
                    non_channelized_port_offset = JER2_ARAD_SCH_OFFSET_TO_NON_CHANNELIZED_OFFSET(unit, e2e_if);
                    DNXC_IF_ERR_EXIT(READ_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm(unit,SCH_BLOCK(unit, core),non_channelized_port_offset,tbl_data));
                    soc_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm_field32_set(unit,tbl_data,PORT_IDf,base_q_pair);
                    DNXC_IF_ERR_EXIT(WRITE_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm(unit,SCH_BLOCK(unit, core),non_channelized_port_offset,tbl_data));    
                }
            }

            {
                uint32 array_index, bit_index, sizeof_element ;

                /*
                 * Update bit 'e2e_if' to '0' (open nif, do not force pause)
                 */
                sizeof_element = 32 ;
                array_index = e2e_if / sizeof_element ;
                bit_index = e2e_if % sizeof_element ;
                DNXC_IF_ERR_EXIT(READ_SCH_DVS_NIF_CONFIGr(unit, core, array_index, &data_32)) ;
                data_32 &= DNX_SAND_RBIT(bit_index) ;
                WRITE_SCH_DVS_NIF_CONFIGr(unit, core, array_index, data_32) ;
            }
	    }
    }

exit:
    DNXC_FUNC_RETURN
}


int 
soc_jer2_jer_sch_e2e_interface_deallocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port
    )
{
    uint32          e2e_if, tm_port, data_32, egress_offset, non_channelized_port_offset, is_master;
    int             is_channelized, core, start, end;
    soc_port_if_t   interface_type;
    soc_reg_above_64_val_t tbl_data;
    DNX_SAND_OCC_BM_PTR ifs_occ;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, port, &is_master));

    if (is_master) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_channelized_port_get(unit, port, &is_channelized)); 
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset)));

        ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
        end = 1;
        /* handle ILKN dedicated mode */
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON && interface_type == SOC_PORT_IF_ILKN ) 
        {
            DNX_SAND_SET_BIT(egress_offset, 0, 0); /* handle fast ports first */
            end++;
	    }

        for (start = 0; start < end; start++) {
            egress_offset+=start; /* handle ILKN dedicated mode */
            /* The core can't be negative... */
            /* coverity[negative_returns:FALSE] */
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.e2e_interfaces_occ.get(unit, core, &ifs_occ);
            DNXC_IF_ERR_EXIT(rv);

            data_32 = 0;
            SOC_REG_ABOVE_64_CLEAR(tbl_data);
            DNXC_IF_ERR_EXIT(READ_SCH_FC_MAP_FCMm(unit, SCH_BLOCK(unit, core), egress_offset, &data_32));
            /* get e2e interface*/
            e2e_if = soc_SCH_FC_MAP_FCMm_field32_get(unit, &data_32, FC_MAP_FCMf);

            /* set HW e2e interface to 0 */
            soc_SCH_FC_MAP_FCMm_field32_set(unit, &data_32 ,FC_MAP_FCMf, 0);
            DNXC_IF_ERR_EXIT(WRITE_SCH_FC_MAP_FCMm(unit,SCH_BLOCK(unit, core), egress_offset, &data_32));

            /* mark e2e interface as free */
            DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, ifs_occ, e2e_if, FALSE));

            if (!JER2_ARAD_SCH_IS_CHNIF_ID(unit, e2e_if)) {
                /* In case of non channelized port set also Port ID to 0 */
                non_channelized_port_offset = JER2_ARAD_SCH_OFFSET_TO_NON_CHANNELIZED_OFFSET(unit, e2e_if);
                DNXC_IF_ERR_EXIT(READ_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm(unit,SCH_BLOCK(unit, core),non_channelized_port_offset,tbl_data));
                soc_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm_field32_set(unit,tbl_data,PORT_IDf,0);
                soc_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm_field32_set(unit,tbl_data,PORT_NIF_MAX_CR_RATEf,0);
                DNXC_IF_ERR_EXIT(WRITE_SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm(unit,SCH_BLOCK(unit, core),non_channelized_port_offset,tbl_data));    
            }

            {
                uint32 array_index, bit_index, sizeof_element ;

                /*
                 * update bit 'e2e_if' to '1' (close nif, force pause)
                 */
                sizeof_element = 32 ;
                array_index = e2e_if / sizeof_element ;
                bit_index = e2e_if % sizeof_element ;
                DNXC_IF_ERR_EXIT(READ_SCH_DVS_NIF_CONFIGr(unit, core, array_index, &data_32)) ;
                data_32 |= DNX_SAND_BIT(bit_index) ;
                WRITE_SCH_DVS_NIF_CONFIGr(unit, core, array_index, data_32) ;
            }
        }
    }

exit:
    DNXC_FUNC_RETURN
}

int
  soc_jer2_jer_sch_cal_max_size_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_OUT  uint32*                max_cal_size
   )
{
    uint32 cal_select;
    DNXC_INIT_FUNC_DEFS;

    cal_select = sch_offset / 2;

    if(cal_select == 0 || cal_select == 1) {
        *max_cal_size = CAL_0_1_SIZE;
    } else {
        *max_cal_size = CAL_2_15_SIZE;
    }

    DNXC_FUNC_RETURN;
}

static soc_mem_t cal_memories[] = {
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_0m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_1m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_2m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_3m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_4m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_5m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_6m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_7m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_8m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_9m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_10m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_11m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_12m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_13m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_14m,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_15m
};

int
  soc_jer2_jer_sch_cal_tbl_set(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_IN   uint32                 sch_to_set /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
                  uint32*                slots
  )
{
    uint32
        cal_offset,
        cal_select,
        cal_internal_select,
        cal_size,
        slot,
        entry;
    soc_mem_t
        memory;

    DNXC_INIT_FUNC_DEFS;

    cal_select = sch_offset / 2; /* selects the right dual calendar memory */
    cal_internal_select = sch_offset % 2; /* selects the right calendar within the dual calendar memory */

    if(cal_select == 0 || cal_select == 1) {
        cal_size = CAL_0_1_SIZE;
    } else {
        cal_size = CAL_2_15_SIZE;
    }

    if(slots_count > cal_size) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_DNXC_MSG("Slots count is out of range")));
    }

    /* Each table 0-15 contain 2 active calenders and two passive calendarts */
    cal_offset = cal_internal_select * (2 * cal_size) + sch_to_set * cal_size;

    /* Write memory*/
    memory = cal_memories[cal_select];
    for (slot=0; slot < slots_count; slot++) {
        entry = 0;
        soc_mem_field_set(unit, memory, &entry, PORT_SELf, &(slots[slot]));
        DNXC_IF_ERR_EXIT(soc_mem_write(unit, memory, SCH_BLOCK(unit, core_id), cal_offset+slot, &entry));
    }

exit:
    DNXC_FUNC_RETURN;
}


int
  soc_jer2_jer_sch_cal_tbl_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_IN   uint32                 sch_to_set /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
    DNX_SAND_OUT  uint32*                slots
  )
{
    uint32
        cal_offset,
        cal_select,
        cal_internal_select,
        cal_size,
        slot,
        entry;
    soc_mem_t
        memory;

    DNXC_INIT_FUNC_DEFS;

    cal_select = sch_offset / 2; /* selects the right dual calendar memory */
    cal_internal_select = sch_offset % 2; /* selects the right calendar within the dual calendar memory */

    if(cal_select == 0 || cal_select == 1) {
        cal_size = CAL_0_1_SIZE;
    } else {
        cal_size = CAL_2_15_SIZE;
    }

    if(slots_count > cal_size) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_DNXC_MSG("Slots count is out of range")));
    }

    /* Each table 0-15 contain 2 active calenders and two passive calendarts */
    cal_offset = cal_internal_select * (2 * cal_size) + sch_to_set * cal_size;

    /* Read memory*/
    memory = cal_memories[cal_select];
    for (slot=0; slot < slots_count; slot++) {
        entry = 0;
        DNXC_IF_ERR_EXIT(soc_mem_read(unit, memory, SCH_BLOCK(unit, core_id), cal_offset+slot, &entry));
        soc_mem_field_get(unit, memory, &entry, PORT_SELf, &(slots[slot]));
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_sch_prio_propagation_enable_set(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int enable
    )
{
    uint32 reg_val;
    int core;
    DNXC_INIT_FUNC_DEFS;

    /* for now - write the same value to cores 0/1 */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) 
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, &reg_val, INTERFACE_PRIORITY_PROPAGATION_ENABLEf, enable? 1:0);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, core, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_sch_prio_propagation_enable_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT  int* enable
    )
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;

    /* for now - get value from core 0 */

    DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CONFIGURATION_REGISTERr(unit, 0, &reg_val));
    *enable = soc_reg_field_get(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, reg_val, INTERFACE_PRIORITY_PROPAGATION_ENABLEf);
    
exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_sch_prio_propagation_port_set(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  int cosq,
    DNX_SAND_IN  int is_high_prio
   )
{
    uint32 mem_val, base_q_pair, tm_port;
    int core, offset;
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    offset = base_q_pair + cosq;
    if (offset < 0 || offset > 255)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid offset for port hr %d \n"), offset));
    }

    DNXC_IF_ERR_EXIT(READ_SCH_HR_SCHEDULER_CONFIGURATION_SHCm(unit, SCH_BLOCK(unit, core), offset, &mem_val));
    soc_mem_field32_set(unit, SCH_HR_SCHEDULER_CONFIGURATION_SHCm, &mem_val,HR_PRIORITY_MASK_SELECTf, is_high_prio);
    DNXC_IF_ERR_EXIT(WRITE_SCH_HR_SCHEDULER_CONFIGURATION_SHCm(unit, SCH_BLOCK(unit, core), offset, &mem_val)); 


exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_sch_prio_propagation_port_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  int cosq,
    DNX_SAND_OUT  int *is_high_prio
   )
{
    uint32 mem_val, base_q_pair, field_val, tm_port;
    int offset, core;
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    offset = base_q_pair + cosq;
    if (offset < 0 || offset > 255)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid offset for port hr %d \n"), offset));
    }
       
    DNXC_IF_ERR_EXIT(READ_SCH_HR_SCHEDULER_CONFIGURATION_SHCm(unit, SCH_BLOCK(unit, core), offset, &mem_val));
    soc_mem_field_get(unit, SCH_HR_SCHEDULER_CONFIGURATION_SHCm, &mem_val, HR_PRIORITY_MASK_SELECTf, &field_val);

    *is_high_prio = field_val;
    
exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_jer_sch_slow_max_rates_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_IN  int                 slow_rate_val
  )
{
    int core;
    
    DNXC_INIT_FUNC_DEFS;

    /* Legacy API */
    
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core)
    {
        DNXC_IF_ERR_EXIT(jer2_jer_sch_slow_max_rates_per_level_set(unit, core, 0, slow_rate_type, slow_rate_val));
    }

exit:
    DNXC_FUNC_RETURN; 
   
}

uint32
jer2_jer_sch_slow_max_rates_per_level_set(
   DNX_SAND_IN int unit, 
   DNX_SAND_IN int core, 
   DNX_SAND_IN int level , 
   DNX_SAND_IN int slow_rate_type, 
   DNX_SAND_IN int slow_rate_val)
{
    soc_field_info_t peak_rate_man_fld, peak_rate_exp_fld;
    JER2_ARAD_SCH_SUBFLOW sub_flow;
    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC internal_sub_flow;
    uint32 mem_val32, slow_fld_val;

    DNXC_INIT_FUNC_DEFS;

    jer2_arad_JER2_ARAD_SCH_SUBFLOW_clear(unit, &sub_flow);

    JER2_JER_TBL_SOC_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
    JER2_JER_TBL_SOC_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);

    sub_flow.shaper.max_rate = slow_rate_val;

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_sch_to_internal_subflow_shaper_convert(unit, &sub_flow, &internal_sub_flow, TRUE));

    slow_fld_val = 0;
    slow_fld_val |= JER2_ARAD_FLD_IN_PLACE(internal_sub_flow.peak_rate_exp, peak_rate_exp_fld);
    slow_fld_val |= JER2_ARAD_FLD_IN_PLACE(internal_sub_flow.peak_rate_man, peak_rate_man_fld);

    if (slow_rate_type == 1)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_A_SSAm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
        soc_mem_field_set(unit, SCH_SLOW_SCALE_A_SSAm, &mem_val32, SLOW_RATEf, &slow_fld_val);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SLOW_SCALE_A_SSAm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
    }
    else
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_B_SSBm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
        soc_mem_field_set(unit, SCH_SLOW_SCALE_B_SSBm, &mem_val32, SLOW_RATEf, &slow_fld_val);
        DNXC_IF_ERR_EXIT(WRITE_SCH_SLOW_SCALE_B_SSBm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
    }
    
exit:
    DNXC_FUNC_RETURN;

}




uint32
  jer2_jer_sch_slow_max_rates_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 slow_rate_type,
    DNX_SAND_OUT int      *slow_rate_val
  )
{
   
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_jer_sch_slow_max_rates_per_level_get(unit, 0, 0, slow_rate_type, slow_rate_val));

exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_sch_slow_max_rates_per_level_get(
   DNX_SAND_IN  int   unit,
   DNX_SAND_IN  int   core,
   DNX_SAND_IN  int   level,
   DNX_SAND_IN  int   slow_rate_type,
   DNX_SAND_OUT int*  slow_rate_val)
{
    uint32 slow_fld_val, mem_val32;
    JER2_ARAD_SCH_SUBFLOW sub_flow;
    JER2_ARAD_SCH_INTERNAL_SUB_FLOW_DESC internal_sub_flow;
    soc_field_info_t peak_rate_man_fld, peak_rate_exp_fld;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(slow_rate_val);

    /* These values are accesed but have no influence on the max rates*/
    internal_sub_flow.max_burst = 0;
    internal_sub_flow.slow_rate_index = 0;

    /*
    * The rate register value is interpreted like \{PeakRateExp,
    * PeakRateMan\} in the SHDS table.
    * Get the fields database for the interpretation.
    */
    JER2_JER_TBL_SOC_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_MAN_EVENf, &peak_rate_man_fld);
    JER2_JER_TBL_SOC_REF(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, PEAK_RATE_EXP_EVENf, &peak_rate_exp_fld);

    if (slow_rate_type == 1)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_A_SSAm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
        soc_mem_field_get(unit, SCH_SLOW_SCALE_A_SSAm, &mem_val32, SLOW_RATEf, &slow_fld_val);
    }
    else
    {
        DNXC_IF_ERR_EXIT(READ_SCH_SLOW_SCALE_B_SSBm(unit, SCH_BLOCK(unit, core), level, &mem_val32));
        soc_mem_field_get(unit, SCH_SLOW_SCALE_B_SSBm, &mem_val32, SLOW_RATEf, &slow_fld_val);
    }

    internal_sub_flow.peak_rate_exp = JER2_ARAD_FLD_FROM_PLACE(slow_fld_val, peak_rate_exp_fld);
    internal_sub_flow.peak_rate_man = JER2_ARAD_FLD_FROM_PLACE(slow_fld_val, peak_rate_man_fld);

    /*
    * The slow setting is equivalent to the SHDS setting.
    */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_sch_from_internal_subflow_shaper_convert(unit, &internal_sub_flow, &sub_flow));

    *slow_rate_val = sub_flow.shaper.max_rate;

exit:
    DNXC_FUNC_RETURN;

}

/* given hr number, this function returns the index in the PSST/CSST table in which this hr is configured */
int
_jer2_jer_sch_rates_calendar_index_get(int unit, int core, int offset, int is_hr_calendar ,uint32 *index)
{
    uint32 field_val ,mem_val, i;
    DNXC_INIT_FUNC_DEFS;

    /* if dynamic port 1 to 1 mapping */
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.dynamic_port_enable)
    {
        *index = offset;
    }
    else /* find in which index this hr is configured*/
    {
        int found = 0;
        /* find the index in which this hr is configured */
        for (i = 0 ; i < JER2_ARAD_EGR_NOF_Q_PAIRS ; i++)
        {
            if (is_hr_calendar)
            {
                DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPER_CALENDAR_PSCm(unit, SCH_BLOCK(unit, core), i, &mem_val));
                soc_mem_field_get(unit, SCH_PIR_SHAPER_CALENDAR_PSCm, &mem_val, HR_NUMf, &field_val);
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPER_CALENDAR_CSCm(unit, SCH_BLOCK(unit, core), i, &mem_val));
                soc_mem_field_get(unit, SCH_CIR_SHAPER_CALENDAR_CSCm, &mem_val, PG_NUMf, &field_val);
            }
            if (field_val == offset)
            {
                found = 1;
                *index = i;
                break;
            }
        }
        if (!found)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Unsupported hr\tcg number : %d"), offset));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static
uint32
_jer2_jer_sch_rates_port_get_max_rate(int unit, int core, int is_port_priority, uint32* maximum_rate, int* maximum_multiplicity)
{
    int maximum = 0, multiplicity = 0, i;
    int rate;
    uint8 valid;
    DNXC_INIT_FUNC_DEFS;

    for (i = 0; i < JER2_ARAD_EGR_NOF_Q_PAIRS ; i++)
    {
        if (is_port_priority)
        {
            DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_get(unit, core, i, &rate, &valid));
        }
        else
        {
            DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_port_tcg_rate_get(unit, core, i, &rate, &valid));
        }
        if (valid && (rate >= maximum))
        {
            if (rate > maximum)
            {
                multiplicity = 1;
                maximum = rate;
            } /* equal , increasing multiplicty */
            else
            {
                multiplicity++;
            }
        }
    }
    *maximum_rate = maximum;
    *maximum_multiplicity = multiplicity;

exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_port_priority_hw_set(
   DNX_SAND_IN   int    unit,
   DNX_SAND_IN   int    core)
{
/* This function should be called only during init sequence */
    uint32 mem_val, reg_val, reg_val2,
           access_period, cal_length;
    int    psst_table_location, is_new_rate_configured = 0;



    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_PS_CALENDAR_SELECTr, &reg_val, PS_CALENDAR_SELECTf, 0); /* set default calendar to be 0 */
    DNXC_IF_ERR_EXIT(WRITE_SCH_PS_CALENDAR_SELECTr(unit, core, reg_val));
    
    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
    access_period = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, reg_val, PIR_SHAPERS_CAL_ACCESS_PERIODf);
    cal_length = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, reg_val, PIR_SHAPERS_CAL_LENGTHf);

    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_CONFIGURATION_1r(unit, core, &reg_val2));
    soc_reg_field_set(unit, SCH_PIR_SHAPERS_CONFIGURATION_1r, &reg_val2, PIR_SHAPERS_CAL_ACCESS_PERIOD_1f, access_period);
    soc_reg_field_set(unit, SCH_PIR_SHAPERS_CONFIGURATION_1r, &reg_val2, PIR_SHAPERS_CAL_LENGTH_1f, cal_length);
    DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_CONFIGURATION_1r(unit, core, reg_val2));

    /* config csst second table */
    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
    access_period = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, reg_val, CIR_SHAPERS_CAL_ACCESS_PERIODf);
    cal_length = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, reg_val, CIR_SHAPERS_CAL_LENGTHf);

    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_CONFIGURATION_1r(unit, core, &reg_val2));
    soc_reg_field_set(unit, SCH_CIR_SHAPERS_CONFIGURATION_1r, &reg_val2, CIR_SHAPERS_CAL_ACCESS_PERIOD_1f, access_period);
    soc_reg_field_set(unit, SCH_CIR_SHAPERS_CONFIGURATION_1r, &reg_val2, CIR_SHAPERS_CAL_LENGTH_1f, cal_length);
    DNXC_IF_ERR_EXIT(WRITE_SCH_CIR_SHAPERS_CONFIGURATION_1r(unit, core, reg_val2));

    DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_REBOUNDED_CREDIT_CONFIGURATIONr, &reg_val, REBOUNDED_CREDIT_WORTHf, JER2_JER_SCH_CREDIT_WORTH_DIV_MIN);
    DNXC_IF_ERR_EXIT(WRITE_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, reg_val));
    DNXC_IF_ERR_EXIT(READ_SCH_ASSIGNED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_ASSIGNED_CREDIT_CONFIGURATIONr, &reg_val, ASSIGNED_CREDIT_WORTHf, JER2_JER_SCH_CREDIT_WORTH_DIV_MIN);
    DNXC_IF_ERR_EXIT(WRITE_SCH_ASSIGNED_CREDIT_CONFIGURATIONr(unit, core, reg_val));
    DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_REBOUNDED_CREDIT_CONFIGURATION_1r, &reg_val, REBOUNDED_CREDIT_WORTH_1f, JER2_JER_SCH_CREDIT_WORTH_DIV_MIN);
    DNXC_IF_ERR_EXIT(WRITE_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, reg_val)); 
    DNXC_IF_ERR_EXIT(READ_SCH_ASSIGNED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_ASSIGNED_CREDIT_CONFIGURATION_1r, &reg_val, ASSIGNED_CREDIT_WORTH_1f, JER2_JER_SCH_CREDIT_WORTH_DIV_MIN);
    DNXC_IF_ERR_EXIT(WRITE_SCH_ASSIGNED_CREDIT_CONFIGURATION_1r(unit, core, reg_val));

    /* initalize quantas to 0 */
    for (psst_table_location = 0 ; psst_table_location < 512; psst_table_location++)
    {
        /* write quanta*/
        DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),psst_table_location,&mem_val));
        soc_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm_field32_set(unit,&mem_val,QUANTA_TO_ADDf,0);
        DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit,SCH_BLOCK(unit,core),psst_table_location,&mem_val));
    }


    /* config csst and psst tables and chose new divider using calculate_new_calendars function
       we call jer2_jer_sch_rates_calculate_new_calendars with increasing_maximum_rate = 0
       because we start with the min divider on startup and start incresing it
       until we find the optimal divider for the max rate
    */
     DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calculate_new_calendars(unit, core, /*increasing_maximum_rate*/0, &is_new_rate_configured));
     if (is_new_rate_configured != 1) {
         DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_DNXC_MSG("Could not find valid REBOUNDED_CREDIT_CONFIGURATION")));
     }



exit:
    DNXC_FUNC_RETURN;
}


uint32
jer2_jer_ofp_rates_sch_port_priority_rate_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_IN  uint32             rate
    )
{

    uint32 credit_div, reg_val, nof_ticks, quanta_nof_bits, quanta, offset,
           base_port_tc, hr_calendar_table_num, index, mem_val, max_rate_before_change, max_rate_after_change = 0;
    int old_rate, increasing_maximum_rate = 0 , decreasing_unique_maximum_rate = 0 , is_new_rate_configured = 0, maximum_multiplicity;
    uint8 valid;
    
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_port_get_max_rate(unit, core, 1, &max_rate_before_change, &maximum_multiplicity)); 

    offset = base_port_tc + port_priority_ndx;

    DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_get(unit, core, offset, &old_rate, &valid));

    /* Check if we need to calculate a new calendar */
    /* if we reduce the unqiue maximum rate */
    if ((old_rate == max_rate_before_change) && (rate < old_rate) && (maximum_multiplicity == 1) )
    {
        decreasing_unique_maximum_rate = 1;
    }
    /* if we change the maximum rate */
    else if (rate > max_rate_before_change)
    {
        increasing_maximum_rate = 1;
    }

    /* change sw db */
    DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_set(unit, core, offset, rate, 1));

    /* get the nex max rate */
    if (!increasing_maximum_rate)
    {
        DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_port_get_max_rate(unit, core, 1, &max_rate_after_change, &maximum_multiplicity));
    }

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    /* Generic function: get divider, cal_length, and access period, given which copy we're using */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 1, &credit_div, &nof_ticks, &quanta_nof_bits));

    if (rate == 0)
    {
        quanta = 0;
    }
    else
    {    
      /* 3. calculate quanta */
        DNXC_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, rate, credit_div, nof_ticks, &quanta)); 
        /*DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));*/
    }
    if (((quanta > 2047) && (increasing_maximum_rate)) || decreasing_unique_maximum_rate)
    {
        DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calculate_new_calendars(unit, core, increasing_maximum_rate, &is_new_rate_configured));
    }

    if (!is_new_rate_configured)
    {
        DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 1, &index));

        /* write to the relevant copy of the table */
        DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit,core ), index + hr_calendar_table_num * 256, &mem_val));
        soc_mem_field32_set(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, QUANTA_TO_ADDf, quanta);
        DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit, core), index + hr_calendar_table_num * 256, &mem_val));
    }
    

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_jer_ofp_rates_sch_port_priority_rate_get(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     uint32            priority_ndx,    
    DNX_SAND_OUT    uint32            *rate
  )
{
    uint32 reg_val, hr_calendar_table_num, credit_div, offset, index, mem_val, 
           nof_ticks, base_port_tc, rate_internal, quanta , quanta_nof_bits;

    DNXC_INIT_FUNC_DEFS;
    
    /* get hr calendar number */
    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);
   
    /* get calendar info */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 1, &credit_div, &nof_ticks, &quanta_nof_bits));
  
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));
    
    offset = base_port_tc + priority_ndx;
    /* get index in psst table in which this hr's rate is configured */
    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 1, &index));

    /* Read quanta value from PSST table */
    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit,core), index + 256*hr_calendar_table_num, &mem_val));
    quanta = soc_mem_field32_get(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, QUANTA_TO_ADDf);

    DNXC_IF_ERR_EXIT(jer2_arad_sch_port_qunta_to_rate_kbits_per_sec(
        unit,
        quanta,
        credit_div,
        nof_ticks,
        &rate_internal
      ));
        
    *rate = rate_internal; 

exit:
  DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_port_priority_max_burst_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority, 
    DNX_SAND_IN  uint32             max_burst 
   )
{
    uint32 base_port_tc, offset, index, mem_val, reg_val, hr_calendar_table_num;

    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    offset = base_port_tc + port_priority;
    /* get index in the PSST table*/
    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 1, &index));

    /* write to relevant copy */

    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit,core ), index + hr_calendar_table_num*256, &mem_val));
    soc_mem_field32_set(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, MAX_BURSTf, max_burst);
    DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit, core), index + hr_calendar_table_num*256, &mem_val));

exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_port_priority_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority, 
    DNX_SAND_OUT uint32             *max_burst
   )
{
    uint32 base_port_tc, reg_val, offset, index, mem_val, hr_calendar_table_num;

    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    /* get hr calendar number */
    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    offset = base_port_tc + port_priority;
    /* get index in the PSST table*/
    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 1, &index));

    /* read max burst */

    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit,core ), index + hr_calendar_table_num*256, &mem_val));
    *max_burst = soc_mem_field32_get(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, MAX_BURSTf);

exit:
    DNXC_FUNC_RETURN;
}


uint32
jer2_jer_ofp_rates_sch_tcg_shaper_rate_set(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,
    DNX_SAND_IN     uint32               rate)
{
    uint32  credit_div, reg_val ,nof_ticks, quanta_nof_bits, quanta, base_port_tc, hr_calendar_table_num,
            offset, index, mem_val , decreasing_unique_maximum_rate = 0, increasing_maximum_rate = 0, max_rate_before_change;
    int old_rate, maximum_multiplicity, is_new_rate_configured = 0;
    uint8 valid;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_port_get_max_rate(unit, core, 0, &max_rate_before_change, &maximum_multiplicity)); 

    offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

    DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_port_tcg_rate_get(unit, core, offset, &old_rate, &valid));

    /* Check if we need to calculate a new calendar */
    /* if we reduce the unqiue maximum rate */
    if ((old_rate == max_rate_before_change) && (rate < old_rate) && (maximum_multiplicity == 1) )
    {
        decreasing_unique_maximum_rate = 1;
    }
    /* if we change the maximum rate */
    else if (rate > max_rate_before_change)
    {
        increasing_maximum_rate = 1;
    }

    DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_port_tcg_rate_set(unit, core, offset, rate, 1));

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    /* Generic function: get divider, cal_length, and access period, given which copy we're using */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 0, &credit_div, &nof_ticks, &quanta_nof_bits));

    if (rate == 0)
    {
        quanta = 0;
    }
    else
    {    
      /* 3. calculate quanta */
        DNXC_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, rate, credit_div, nof_ticks, &quanta));  
        /*DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));*/
    }

    if (((quanta > 2047) && (increasing_maximum_rate)) || decreasing_unique_maximum_rate)
    {
        DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calculate_new_calendars(unit, core, increasing_maximum_rate, &is_new_rate_configured));
    }

    if (!is_new_rate_configured)
    {
        DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 0, &index));

        /* write to relevant copy of the calendar */
        DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit,core ), index + 256*hr_calendar_table_num, &mem_val));
        soc_mem_field32_set(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, QUANTA_TO_ADDf, quanta);
        DNXC_IF_ERR_EXIT(WRITE_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit, core), index + 256*hr_calendar_table_num, &mem_val));
    }


exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_tcg_shaper_rate_get(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,
    DNX_SAND_OUT    uint32*               rate)
{
    uint32  reg_val, hr_calendar_table_num, quanta_nof_bits, credit_div, index, 
            nof_ticks, base_port_tc, rate_internal, quanta = 0, offset, mem_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);
   
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 0, &credit_div, &nof_ticks, &quanta_nof_bits));
  
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));
    
    offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 0, &index));

    /* Read quanta value from PSST table */
    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit,core), index + 256*hr_calendar_table_num, &mem_val));
    quanta = soc_mem_field32_get(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, QUANTA_TO_ADDf);

    if (quanta == 0) 
    {
        rate_internal = 0;
    } 
    else 
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_sch_port_qunta_to_rate_kbits_per_sec(
            unit,
            quanta,
            credit_div,
            nof_ticks,
            &rate_internal
        )); 
    }
    *rate = rate_internal; 

exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_tcg_shaper_max_burst_set(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,
    DNX_SAND_IN     uint32               max_burst)
{
    uint32 base_port_tc, offset, tbl_data, index, mem_val, hr_calendar_table_num, reg_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 0, &index));

    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),index + 256*hr_calendar_table_num ,&tbl_data));
    soc_mem_field32_set(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, MAX_BURSTf, max_burst);
    DNXC_IF_ERR_EXIT(WRITE_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),index + 256*hr_calendar_table_num,&tbl_data));

exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_ofp_rates_sch_tcg_shaper_max_burst_get(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            tm_port,
    DNX_SAND_IN     JER2_ARAD_TCG_NDX      tcg_ndx,
    DNX_SAND_OUT    uint32*               max_burst)
{
    uint32 base_port_tc, offset, index, mem_val, reg_val, hr_calendar_table_num;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));

    /* get hr calendar number */
    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    offset = JER2_ARAD_SCH_PORT_TCG_ID_GET(base_port_tc,tcg_ndx);

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calendar_index_get(unit, core, offset, 0, &index));

    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit,SCH_BLOCK(unit,core),index +hr_calendar_table_num*256 ,&mem_val));
    *max_burst = soc_mem_field32_get(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, MAX_BURSTf);

exit:
    DNXC_FUNC_RETURN;
}

uint32
_jer2_jer_sch_rates_switch_calendars(int unit, int core, int new_calendar_table_num, int new_divider)
{
    uint32 mem_val, credit_div, nof_ticks, quanta_nof_bits, reg_val, cal_length_psst, cal_length_csst, hr_num, tcg_index,
           quanta, old_index, new_index, max_burst;
    uint8 valid;
    int rate, i;
    DNXC_INIT_FUNC_DEFS;

    /* config new divider */
    if (new_calendar_table_num == 0)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_REBOUNDED_CREDIT_CONFIGURATIONr, &reg_val, REBOUNDED_CREDIT_WORTHf, new_divider);
        DNXC_IF_ERR_EXIT(WRITE_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, reg_val));

        DNXC_IF_ERR_EXIT(READ_SCH_ASSIGNED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_ASSIGNED_CREDIT_CONFIGURATIONr, &reg_val, ASSIGNED_CREDIT_WORTHf, new_divider);
        DNXC_IF_ERR_EXIT(WRITE_SCH_ASSIGNED_CREDIT_CONFIGURATIONr(unit, core, reg_val));
    }
    else
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_REBOUNDED_CREDIT_CONFIGURATION_1r, &reg_val, REBOUNDED_CREDIT_WORTH_1f, new_divider);
        DNXC_IF_ERR_EXIT(WRITE_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, reg_val));

        DNXC_IF_ERR_EXIT(READ_SCH_ASSIGNED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
        soc_reg_field_set(unit, SCH_ASSIGNED_CREDIT_CONFIGURATION_1r, &reg_val, ASSIGNED_CREDIT_WORTH_1f, new_divider);
        DNXC_IF_ERR_EXIT(WRITE_SCH_ASSIGNED_CREDIT_CONFIGURATION_1r(unit, core, reg_val));
    }

    /* get cal_length, they are equal in both calendar copies */
    DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
    cal_length_psst = soc_reg_field_get(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, reg_val, PIR_SHAPERS_CAL_LENGTHf) + 1;
    DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_CONFIGURATIONr(unit, core, &reg_val));
    cal_length_csst = soc_reg_field_get(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, reg_val, CIR_SHAPERS_CAL_LENGTHf) + 1;
   
    /* config new quantas & old bursts to psst table */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, new_calendar_table_num, 1, &credit_div, &nof_ticks, &quanta_nof_bits));    
    for (i = 0 ; i < cal_length_psst; i++)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPER_CALENDAR_PSCm(unit, SCH_BLOCK(unit, core), i, &mem_val));
        hr_num = soc_mem_field32_get(unit, SCH_PIR_SHAPER_CALENDAR_PSCm, &mem_val, HR_NUMf);

        DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_get(unit, core, hr_num, &rate, &valid));
        if (valid)
        {
            /* calcualte new quanta */
            if (rate == 0)
            {
                quanta = 0;
            }
            else
            {                  
                DNXC_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, rate, credit_div, nof_ticks, &quanta));  
                DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));
            }
            old_index = (new_calendar_table_num == 0) ? i+256:i ;
            new_index = (old_index + 256) % 512;

            /* take burst from old calendar*/
            DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit, core), old_index, &mem_val));
            max_burst = soc_mem_field32_get(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, MAX_BURSTf);
            /* write quanta, burst, to new calendar*/
            DNXC_IF_ERR_EXIT(READ_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit, core), new_index, &mem_val));
            soc_mem_field32_set(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, QUANTA_TO_ADDf, quanta);
            soc_mem_field32_set(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, &mem_val, MAX_BURSTf, max_burst);
            DNXC_IF_ERR_EXIT(WRITE_SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm(unit, SCH_BLOCK(unit, core), new_index, &mem_val));
        }
    }
    /* config new quantas & old bursts to csst table */
    DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, new_calendar_table_num, 0, &credit_div, &nof_ticks, &quanta_nof_bits)); 
    for (i = 0; i < cal_length_csst; i++)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPER_CALENDAR_CSCm(unit, SCH_BLOCK(unit, core), i, &mem_val));
        tcg_index = soc_mem_field32_get(unit, SCH_CIR_SHAPER_CALENDAR_CSCm, &mem_val, PG_NUMf);

        DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_port_tcg_rate_get(unit, core, tcg_index, &rate, &valid));
        if (valid)
        {
            /* calculate new quanta */
            if (rate == 0)
            {
                quanta = 0;
            }
            else
            {                  
                DNXC_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, rate, credit_div, nof_ticks, &quanta));  
                DNX_SAND_LIMIT_FROM_ABOVE(quanta, DNX_SAND_BITS_MASK(quanta_nof_bits-1,0));
            }
            old_index = (new_calendar_table_num == 0) ? i+256:i;
            new_index = (old_index + 256) % 512;

            /* take burst from old copy */
            DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit, core), old_index, &mem_val));
            max_burst = soc_mem_field32_get(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, MAX_BURSTf);

            DNXC_IF_ERR_EXIT(READ_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit, core), new_index, &mem_val));
            soc_mem_field32_set(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, QUANTA_TO_ADDf, quanta);
            soc_mem_field32_set(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, &mem_val, MAX_BURSTf, max_burst);
            DNXC_IF_ERR_EXIT(WRITE_SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm(unit, SCH_BLOCK(unit, core), new_index, &mem_val));
        }
    }

    /* switch calendars */
    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    soc_reg_field_set(unit, SCH_PS_CALENDAR_SELECTr, &reg_val, PS_CALENDAR_SELECTf, new_calendar_table_num);
    DNXC_IF_ERR_EXIT(WRITE_SCH_PS_CALENDAR_SELECTr(unit, core, reg_val));

exit:
    DNXC_FUNC_RETURN;
}

uint32
_jer2_jer_sch_rates_calculate_new_divider(int unit, int core, int hr_calendar_table_num, int max_rate_increased, int old_divider ,uint32 *new_divider)
{
    uint32 nof_ticks, quanta_nof_bits, new_quanta,
           curr_divider, iterate_divider;
    uint32 new_max_rate, new_max_rate_psst, new_max_rate_csst;
    int max_psst_multiplicity, max_csst_multiplicity;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_port_get_max_rate(unit, core, 1, &new_max_rate_psst, &max_psst_multiplicity));
    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_port_get_max_rate(unit, core, 0, &new_max_rate_csst, &max_csst_multiplicity));
    if (new_max_rate_psst == 0 && new_max_rate_csst == 0)
    {
        curr_divider = old_divider; /*keep the same divider, we don't have rates anyway*/
    }
    else
    {
        if ( new_max_rate_psst > new_max_rate_csst )
        {
            new_max_rate = new_max_rate_psst;
            DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 1, &curr_divider, &nof_ticks, &quanta_nof_bits));
        }
        else
        {
            new_max_rate = new_max_rate_csst;
            DNXC_IF_ERR_EXIT(jer2_arad_sch_calendar_info_get(unit, core, hr_calendar_table_num, 0, &curr_divider, &nof_ticks, &quanta_nof_bits));
        }
        curr_divider = old_divider;

        if (max_rate_increased) /* max rate increased - we need to decrease the divider */
        {
            for (curr_divider = old_divider; curr_divider >= JER2_JER_SCH_CREDIT_WORTH_DIV_MIN ; curr_divider--)
            {
                DNXC_SAND_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, new_max_rate, curr_divider, nof_ticks, &new_quanta));
                if(new_quanta <= JER2_JER_SCH_SHAPERS_QUANTA_TO_ADD_MAX)
                {
                    break;
                }
            }
        }
        else /* maximum rate decreased - we can increase the divider in order to get a better resolution */
        {
            /* increase divider to maximum possible, keeping the max rate quanta below 2047 */
            for (iterate_divider = curr_divider + 1; iterate_divider < JER2_JER_SCH_CREDIT_WORTH_DIV_MAX; iterate_divider++)
            {
                DNXC_SAND_IF_ERR_EXIT(jer2_arad_sch_port_rate_kbits_per_sec_to_qaunta(unit, new_max_rate, iterate_divider, nof_ticks, &new_quanta));
                if(new_quanta > JER2_JER_SCH_SHAPERS_QUANTA_TO_ADD_MAX)
                {
                    break;
                }
                curr_divider = iterate_divider;
            }
        }
    }
    *new_divider = curr_divider;

exit:
    DNXC_FUNC_RETURN;
}

uint32
_jer2_jer_sch_rates_calculate_new_calendars(int unit, int core, int max_rate_increased, int *new_rate_configured)
{
    int hr_calendar_table_num;
    uint32 divider, reg_val, new_divider, old_divider;

    DNXC_INIT_FUNC_DEFS;
    
    /* get current working calendar */

    DNXC_IF_ERR_EXIT(READ_SCH_PS_CALENDAR_SELECTr(unit, core, &reg_val));
    hr_calendar_table_num = soc_reg_field_get(unit, SCH_PS_CALENDAR_SELECTr, reg_val, PS_CALENDAR_SELECTf);

    if (hr_calendar_table_num == 0)
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATIONr(unit, core, &reg_val));
        divider = soc_reg_field_get(unit, SCH_REBOUNDED_CREDIT_CONFIGURATIONr, reg_val, REBOUNDED_CREDIT_WORTHf); 
    }
    else
    {
        DNXC_IF_ERR_EXIT(READ_SCH_REBOUNDED_CREDIT_CONFIGURATION_1r(unit, core, &reg_val));
        divider = soc_reg_field_get(unit, SCH_REBOUNDED_CREDIT_CONFIGURATION_1r, reg_val, REBOUNDED_CREDIT_WORTH_1f);
    }
    old_divider = divider;

    DNXC_IF_ERR_EXIT(_jer2_jer_sch_rates_calculate_new_divider(unit, core, hr_calendar_table_num, max_rate_increased, old_divider, &new_divider));
    
    if (old_divider == new_divider) /* no need to change anything */ 
    {
        *new_rate_configured = 0;
        SOC_EXIT;
    }

    /* Switch calendars */
    /* config new divider, and quantas */
    _jer2_jer_sch_rates_switch_calendars(unit, core, (hr_calendar_table_num+1)%2, new_divider);
    *new_rate_configured = 1;

exit:
    DNXC_FUNC_RETURN;

}


#undef _ERR_MSG_MODULE_NAME

