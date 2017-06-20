/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer_ofp_rates.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif


/*
 * Includes 
 */
#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <sal/compiler.h>

#include <soc/mcm/memregs.h>

#include <soc/dpp/drv.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/mbcm.h>

#include <soc/dpp/JER/jer_ports.h>
#include <soc/dpp/JER/jer_ofp_rates.h>
#include <soc/dpp/JER/jer_egr_queuing.h>
#include <soc/dpp/QAX/qax_link_bonding.h>

#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/arad_ofp_rates.h>
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>

#define SOC_JER_OFP_RATES_NOF_CALS_IN_DUAL_MODE 2
/*
 * Static functions 
 */

STATIC int
soc_jer_ofp_rates_interface_internal_rate_set(int unit, int core, uint32 egr_if_id /* offset of the calendar or interface id*/, uint32 internal_rate, int is_cal_shaper)
{
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;

    /* read */
    SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), egr_if_id, &data));

    if (is_cal_shaper) { /* cal shaper */
        /* update calendar rate */
        if (internal_rate > SOC_DPP_DEFS_GET(unit, cal_internal_rate_max)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("calendar rate for interface is too big \n")));
        }
        soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data, CH_SPR_RATE_Af, internal_rate);
        soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data, CH_SPR_RATE_Bf, internal_rate);
    } else { /* interface shaper */
        /* update interface rate */
        if (internal_rate > ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_IF_RATE) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("interface rate for interface is too big \n")));
        }           
        soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data, CH_IF_SHAPER_RATEf, internal_rate);
    }

    /* write */
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), egr_if_id, &data));

exit:
    SOCDNX_FUNC_RETURN
}

int
soc_jer_ofp_rates_interface_internal_rate_get(
    SOC_SAND_IN   int                   unit, 
    SOC_SAND_IN   int                   core, 
    SOC_SAND_IN   uint32                egr_if_id, 
    SOC_SAND_OUT  uint32                *internal_rate)
{
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;

    /* get internal interface rate */
    SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), egr_if_id, &data));    
    *internal_rate = soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_get(unit, &data, CH_IF_SHAPER_RATEf);

exit:
    SOCDNX_FUNC_RETURN
}


/*
 * Implementation
 */


int
soc_jer_ofp_rates_init(SOC_SAND_IN int unit)
{
    int                    core, cal_id, idx, ps_id, tcg_id, res;
    soc_reg_above_64_val_t data64;
    uint32                 fld_val, data;
    uint32 init_max_burst = ARAD_OFP_RATES_BURST_DEAULT;
    ARAD_SW_DB_DEV_RATE tcg_rate;
    ARAD_SW_DB_DEV_RATE queue_rate;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {

        SOCDNX_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &data));

        /*Init max burst entries to max value */
        SOCDNX_SAND_IF_ERR_EXIT(arad_fill_table_with_entry(unit, EGQ_PMCm, EGQ_BLOCK(unit, core), &init_max_burst));

        SOCDNX_SAND_IF_ERR_EXIT(arad_fill_table_with_entry(unit, EGQ_TCG_PMCm, EGQ_BLOCK(unit, core), &init_max_burst));

        SOCDNX_SAND_IF_ERR_EXIT(arad_fill_table_with_entry(unit, EGQ_QP_PMCm, EGQ_BLOCK(unit, core), &init_max_burst));

        /* update sw db with default burst size */
        for (idx = 0; idx < ARAD_EGR_NOF_BASE_Q_PAIRS; idx++)
        {
            /* port burst */
            res = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.rates.egq_bursts.set(unit, core, idx, init_max_burst);
            SOCDNX_IF_ERR_EXIT(res);

            /* port priority burst */
            res = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.port_priority_cal.queue_rate.get(unit, core, idx, &queue_rate);
            SOCDNX_IF_ERR_EXIT(res);
            queue_rate.egq_bursts = init_max_burst;
            res = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.port_priority_cal.queue_rate.set(unit, core, idx, &queue_rate);
            SOCDNX_IF_ERR_EXIT(res);
        }

        for (ps_id = 0; ps_id < ARAD_EGR_NOF_PS; ps_id++)
        {
            for (tcg_id = 0; tcg_id < ARAD_NOF_TCGS; tcg_id++)
            {
                /* tcg burst */
                res = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.tcg_cal.tcg_rate.get(unit, core, ps_id, tcg_id, &tcg_rate);
                SOCDNX_IF_ERR_EXIT(res);
                tcg_rate.egq_bursts = init_max_burst;
                res = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.tcg_cal.tcg_rate.set(unit, core, ps_id, tcg_id, &tcg_rate);
                SOCDNX_IF_ERR_EXIT(res);
            }
        }

        /* By default Enable Interface, OTM, QPair, TCG shapers */
        fld_val = 0x1;
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, INTERFACE_SPR_ENAf, fld_val);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, OTM_SPR_ENAf, fld_val);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, QPAIR_SPR_ENAf, fld_val);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, TCG_SPR_ENAf, fld_val);

        /* Enable resolution decrease for TCG and QPair shapers */
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, QPAIR_SPR_RESOLUTIONf, 1);
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, TCG_SPR_RESOLUTIONf, 1);

        /* empty queues are ignored (field name is confusing, 0 means ignore) */
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, IGNORE_QEMPTYf, 0);

        /* set default packet size in bytes for packet mode shaping */
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, SHAPER_PACKET_RATE_CONSTf, ARAD_OFP_RATES_DEFAULT_PACKET_SIZE);

        SOCDNX_SAND_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, data));

        SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.egq_tcg_qpair_shaper_enable.set(unit,TRUE));

        /* iterate over all calendars */
        for (cal_id = 0; cal_id < SOC_DPP_DEFS_GET(unit, nof_channelized_calendars); cal_id++) {
            /* read */
            SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), cal_id, &data64));

            /* set interface rate to max*/    
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_IF_SHAPER_RATEf, ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_IF_RATE);
            /* set interface burst to max */
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_IF_SHAPER_MAX_BURSTf, ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_IF_BURST);
            /* set calendar rate to max*/    
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_RATE_Af, SOC_DPP_DEFS_GET(unit, cal_internal_rate_max));
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_RATE_Bf, SOC_DPP_DEFS_GET(unit, cal_internal_rate_max));
            /* set calendar burst to max */
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_MAX_BURST_Af, ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_CAL_BURST);
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_MAX_BURST_Bf, ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_CAL_BURST);
            /* set calendar length to 0*/
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_CAL_LEN_Af, 0x0);
            soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data64, CH_SPR_CAL_LEN_Bf, 0x0);

            /* write */
            SOCDNX_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), cal_id, &data64));
        }
    }

exit:
    SOCDNX_FUNC_RETURN
}

int
soc_jer_ofp_rates_egq_interface_shaper_set(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  int                          core,
    SOC_SAND_IN  uint32                       tm_port,
    SOC_SAND_IN  SOC_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    SOC_SAND_IN  uint32                       if_shaper_rate
  )
{
    uint32       egr_if_id, internal_rate, low_priority_cal, high_priority_cal, cal_id;
    int          is_single_cal_mode = 0;
    int          is_channelized;
    soc_port_t   port;   

    SOCDNX_INIT_FUNC_DEFS;

    /* get egress interface shaper id */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_get(unit,port, &is_channelized));
    if (is_channelized) {
        SOCDNX_IF_ERR_EXIT(soc_jer_egr_port2egress_offset(unit, core, tm_port, &egr_if_id));
    } else {
        egr_if_id = SOC_DPP_DEFS_GET(unit, non_channelized_cal_id);
    }

    /* translate Kbps rate to internal rate (clocks) */
    SOCDNX_IF_ERR_EXIT(arad_ofp_rates_egq_shaper_rate_to_internal(unit, ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB, if_shaper_rate, &internal_rate));

    /*  there are two differnet inteface shapers for channelized interfaces:
     *   1. Interface shaper - responsible for port priotization (SP prioity)
     *   2. Calendar shaper - responsible for fairness among the scheduled ports 
     */
     
    /* 1. set interface shaper rate (prioritization) */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_interface_internal_rate_set(unit, core, egr_if_id, internal_rate, FALSE /*is_cal_shaper*/));

    /* 2. set calendars shapers rate (fairness) */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_single_cal_mode_get(unit, port, &is_single_cal_mode)); 
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_get(unit, port, &low_priority_cal));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_get(unit, port, &high_priority_cal));

    if (is_single_cal_mode) { /* single calendar mode  - NO fairness!!!! */
        /* calendar shaper is set to max (becomes irrelvant, only inteface shaper is actually working) */
        cal_id = INVALID_CALENDAR;
        if (high_priority_cal != INVALID_CALENDAR) {
            cal_id = high_priority_cal;
        } else {
            if (low_priority_cal != INVALID_CALENDAR) {
                cal_id = low_priority_cal;
            }
        }

        if (cal_id != INVALID_CALENDAR) {
            SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_interface_internal_rate_set(unit, core, cal_id, 
                                                                             SOC_DPP_DEFS_GET(unit, cal_internal_rate_max), TRUE /*is_cal_shaper*/));
        }
    } else { /* dual calendars mode - Fairness achieved */
        /* need to check first if the calendar is valid although working in dual calendar mode
           since if all ports are at the same priority then only one calendar is actually allocated */
        if (low_priority_cal != INVALID_CALENDAR) {
            SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_interface_internal_rate_set(unit, core, low_priority_cal, internal_rate, TRUE /*is_cal_shaper*/));
        }

        if (high_priority_cal != INVALID_CALENDAR) {
            SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_interface_internal_rate_set(unit, core, high_priority_cal, internal_rate, TRUE /*is_cal_shaper*/));
        }
    }

exit:
    SOCDNX_FUNC_RETURN
}


int
soc_jer_ofp_rates_egq_interface_shaper_get(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  int                       core,
    SOC_SAND_IN  uint32                    tm_port,
    SOC_SAND_OUT uint32                    *if_shaper_rate
  )
{
    uint32 egr_if_id, internal_rate;
    soc_port_t port;
    int is_channelized;

    SOCDNX_INIT_FUNC_DEFS;

    /* get egress interface shaper id */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_get(unit,port, &is_channelized));
    if (is_channelized) {
        SOCDNX_IF_ERR_EXIT(soc_jer_egr_port2egress_offset(unit, core, tm_port, &egr_if_id));
    } else {
        egr_if_id = SOC_DPP_DEFS_GET(unit, non_channelized_cal_id);
    }

    /* get interface shaper internal rate */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_interface_internal_rate_get(unit, core, egr_if_id, &internal_rate));

    /* translate from internal rate (clocks) to Kbps */
    SOCDNX_IF_ERR_EXIT(arad_ofp_rates_egq_shaper_rate_from_internal(unit, ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB, internal_rate, if_shaper_rate));

exit:
    SOCDNX_FUNC_RETURN
}


int
soc_jer_ofp_rates_egq_single_port_rate_sw_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core,
    SOC_SAND_IN  uint32                tm_port,
    SOC_SAND_IN  uint32                rate
    )
{
    uint32     base_q_pair, base_q_pair_i, low_priority_cal, high_priority_cal, old_cal, new_cal, tm_port_i, num_required_slots,
               num_required_slots_h, num_required_slots_l, rate_i;
    int        is_channelized, is_single_cal_mode, is_high_priority_port, is_high_priority_port_i, core_i, i;
    soc_port_t port, port_i;
    soc_pbmp_t ports_bm;
    SOC_SAND_OCC_BM_PTR modified_cals_occ;
    soc_port_if_t   interface_type;
    soc_error_t rv;
#ifdef BCM_LB_SUPPORT
    soc_pbmp_t lb_ports;
    uint32     lb_port_num = 0;
    uint32     egress_offset = 0;
#endif

    SOCDNX_INIT_FUNC_DEFS

    /* calendar allocation */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_get(unit, port, &is_channelized));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));

    /* in case of ERP port do nothing*/
    if (interface_type == SOC_PORT_IF_ERP) { /* do nothing */
        SOC_EXIT;
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.modified_channelized_cals_occ.get(unit, core, &modified_cals_occ);
    SOCDNX_IF_ERR_EXIT(rv);


    if (is_channelized) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_get(unit, port, &high_priority_cal));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_get(unit, port, &low_priority_cal));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit,mbcm_dpp_egr_queuing_is_high_priority_port_get,(unit, core, tm_port, &is_high_priority_port)));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_single_cal_mode_get(unit, port, &is_single_cal_mode));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &ports_bm));        

#ifdef BCM_LB_SUPPORT
        if (SOC_IS_QAX(unit) && SOC_DPP_CONFIG(unit)->qax->link_bonding_enable) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_port2egress_offset, (unit, core, tm_port, &egress_offset)));
            if (egress_offset == SOC_QAX_EGR_IF_LBG_RESERVE) {
                SOCDNX_IF_ERR_EXIT(qax_lb_ports_on_reserve_intf_get(unit, &lb_ports, &lb_port_num));
                SOC_PBMP_ASSIGN(ports_bm, lb_ports);
            }
        }
#endif

        if (is_single_cal_mode) {
            /* check if calendar is already allocated */
            if (high_priority_cal == INVALID_CALENDAR && low_priority_cal == INVALID_CALENDAR) {
                /* allocate new calendar */
                old_cal = INVALID_CALENDAR;                           
            } else {
                if (high_priority_cal != INVALID_CALENDAR && low_priority_cal != INVALID_CALENDAR && high_priority_cal != low_priority_cal) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("port %d is in single cal mode but has 2 calendars\n"), port));
                }
                /* calendar already allocated  */
                old_cal = (high_priority_cal != INVALID_CALENDAR) ? high_priority_cal : low_priority_cal;
            }

            /* calculate num of required slots */
            num_required_slots = 0;
            SOC_PBMP_ITER(ports_bm, port_i) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port_i, &core_i));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, tm_port_i,  &base_q_pair_i));
                SOCDNX_SAND_IF_ERR_EXIT(arad_sw_db_egq_port_rate_get(unit, core_i, base_q_pair_i, &rate_i));
                /* if port rate is not 0 add it to slots requirement */
                if ( ((tm_port_i != tm_port) && (rate_i != 0)) || ((tm_port_i == tm_port) && (rate != 0)) ) {
                    num_required_slots++;
                }
            }

            SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_calendar_allocate(unit, core, tm_port, num_required_slots, old_cal, &new_cal));

            /* mark calendar as modified */
            if (new_cal != INVALID_CALENDAR) {
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, modified_cals_occ, new_cal, TRUE));
            }

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_set(unit, port, new_cal));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_set(unit, port, new_cal));

        } else { /* dual calendar mode */

            /* calculate num of required slots */
            num_required_slots = num_required_slots_h = num_required_slots_l = 0;
            SOC_PBMP_ITER(ports_bm, port_i) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port_i, &core_i));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, tm_port_i,  &base_q_pair_i));
                SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit,mbcm_dpp_egr_queuing_is_high_priority_port_get,(unit, core_i, tm_port_i, &is_high_priority_port_i)));
                SOCDNX_SAND_IF_ERR_EXIT(arad_sw_db_egq_port_rate_get(unit, core_i, base_q_pair_i, &rate_i));
                /* if port rate is not 0 add it to slots requirement */
                if ( ((tm_port_i != tm_port) && (rate_i != 0)) || ((tm_port_i == tm_port) && (rate != 0)) ) {
                    if (is_high_priority_port_i) {
                        num_required_slots_h++;
                    } else {
                        num_required_slots_l++;
                    }
                }
            }

            /* handle both calendars */
            for (i = 0; i < SOC_JER_OFP_RATES_NOF_CALS_IN_DUAL_MODE; i++) {

                /* first handle the other priority calendar (it might be released or switched to smaller calendar) */
                is_high_priority_port = !is_high_priority_port; 
                old_cal = is_high_priority_port ? high_priority_cal : low_priority_cal;
                num_required_slots = is_high_priority_port ? num_required_slots_h : num_required_slots_l;

                SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_calendar_allocate(unit, core, tm_port, num_required_slots, old_cal, &new_cal));

                if (is_high_priority_port) {
                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_set(unit, port, new_cal));
                } else {
                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_set(unit, port, new_cal));
                }
                /* mark calendar as modified */
                if (new_cal != INVALID_CALENDAR) {
                    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, modified_cals_occ, new_cal, TRUE));
                }
            }
        }
    } else { /* non channelized interface - static calendar */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_set(unit, port, SOC_DPP_DEFS_GET(unit, non_channelized_cal_id)));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_set(unit, port, SOC_DPP_DEFS_GET(unit, non_channelized_cal_id)));
        /* mark calendar as modified */
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, modified_cals_occ, SOC_DPP_DEFS_GET(unit, non_channelized_cal_id), TRUE));
    }

    /* Getting the port's base_q_pair */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

    /* Setting rate to sw */
    SOCDNX_SAND_IF_ERR_EXIT(arad_sw_db_egq_port_rate_set(unit, core, base_q_pair, rate));

    /* Mark port as valid */
    SOCDNX_SAND_IF_ERR_EXIT(arad_sw_db_is_port_valid_set(unit, core, base_q_pair, TRUE));

exit:
    SOCDNX_FUNC_RETURN
}

int 
soc_jer_ofp_rates_calendar_allocate(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  int             core,
    SOC_SAND_IN  uint32          tm_port,
    SOC_SAND_IN  uint32          num_required_slots,
    SOC_SAND_IN  uint32          old_cal,
    SOC_SAND_OUT uint32          *new_cal)
{
    SOC_SAND_OCC_BM_PTR cals_occ;
    uint32              num_of_big_calendars, num_of_calendars, small_cal_size;
    uint8               found;
    soc_port_t          port;
    soc_error_t         rv;
    uint32 num_required_slots_final; /* needed inorder to overwrite the original num_required_slots value in case only big calendar is needed*/
    soc_port_if_t       interface;
    SOCDNX_INIT_FUNC_DEFS

    *new_cal = INVALID_CALENDAR;

    num_of_calendars = SOC_DPP_DEFS_GET(unit, nof_channelized_calendars);
    num_of_big_calendars = SOC_DPP_DEFS_GET(unit, nof_big_channelized_calendars);
    small_cal_size = SOC_DPP_DEFS_GET(unit, small_channelized_cal_size);

    rv = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.channelized_cals_occ.get(unit, core, &cals_occ);
    SOCDNX_IF_ERR_EXIT(rv);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit,port,&interface));

    /* in case ILKN interface requires big calendar */
    if((interface == _SHR_PORT_IF_ILKN) && SOC_DPP_CONFIG(unit)->jer->tm.is_ilkn_big_cal && (num_required_slots>0))
    {
        num_required_slots_final = SOC_DPP_DEFS_GET(unit, big_channelized_cal_size);
    } 
    else
    {
        num_required_slots_final = num_required_slots;
    }

    if (old_cal == INVALID_CALENDAR) { /* allocate new calendar */
        if (num_required_slots_final != 0) {
            found = FALSE;
            if (num_required_slots_final <= small_cal_size) { /* allocate samll calendar */
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_get_next_in_range(unit, cals_occ, num_of_big_calendars, (num_of_calendars-2), TRUE, new_cal, &found));  
            } 

            if (!found) { /* try to allocate big calendar since all small calendars are occupied */
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_get_next_in_range(unit, cals_occ, 0, (num_of_big_calendars -1), TRUE, new_cal, &found));
                if (!found) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("No calendars are left for port %d \n"), port));
                }
            }

            /* mark new calendar as occupied */
            SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, cals_occ, *new_cal, TRUE));
        }

    } else { /* calendar was already allocated, check if need to switch to new calendar */

        if (num_required_slots_final == 0) {
                SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_calendar_deallocate(unit, core, old_cal));
        } else { /* check if need to allocate new calendar */
            found = FALSE;
            if (num_required_slots_final <= small_cal_size) { /* suits for small channelized cal */
                if (old_cal < num_of_big_calendars) { /* no need for such a big calendar, can switch to smaller one */
                    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_get_next_in_range(unit, cals_occ, num_of_big_calendars, (num_of_calendars-2), TRUE, new_cal, &found));
                    if (!found) {
                        /* Old cal can be used */
                        *new_cal = old_cal;
                    }
                } else {
                     *new_cal = old_cal;
                }
            } else { /* num_required_slots_final > small_channelized_cal_size */
                if (old_cal >=  num_of_big_calendars) { /* need to switch to bigger calendar */
                    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_get_next_in_range(unit, cals_occ, 0, (num_of_big_calendars -1), TRUE, new_cal, &found));
                    if (!found) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("No calendars are left for port %d \n"), port));
                    }
                } else {
                    /* Old cal can be used */
                    *new_cal = old_cal;
                }
            }

            if (found) { /* if found then calendar switch is required */
                /* deallocate old calendar */
                SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_calendar_deallocate(unit, core, old_cal));
                /* mark new calendar as occupied */
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, cals_occ, *new_cal, TRUE));
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN
}

int
soc_jer_ofp_rates_calendar_deallocate(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  int             core,
    SOC_SAND_IN  uint32          cal_id)
{
    SOC_SAND_OCC_BM_PTR cals_occ;
    soc_error_t         rv;

    SOCDNX_INIT_FUNC_DEFS

    rv = sw_state_access[unit].dpp.soc.arad.tm.arad_egr_ports.channelized_cals_occ.get(unit, core, &cals_occ);
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_SAND_IF_ERR_EXIT(soc_sand_occ_bm_occup_status_set(unit, cals_occ, cal_id, FALSE));

exit:
    SOCDNX_FUNC_RETURN
}


int
soc_jer_ofp_rates_port2chan_cal_get(
    SOC_SAND_IN  int        unit, 
    SOC_SAND_IN  int        core, 
    SOC_SAND_IN  uint32     tm_port, 
    SOC_SAND_OUT uint32     *calendar)
{
    soc_port_t port;
    int        is_high_priority_port;

    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port)); 
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit,mbcm_dpp_egr_queuing_is_high_priority_port_get,(unit, core, tm_port, &is_high_priority_port)));

    if (is_high_priority_port) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_get(unit, port, calendar));
    } else {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_get(unit, port, calendar));
    }

exit:
    SOCDNX_FUNC_RETURN
}


int
soc_jer_ofp_rates_retrieve_egress_shaper_reg_field_names(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  ARAD_OFP_RATES_CAL_INFO                  *cal_info,
    SOC_SAND_IN  ARAD_OFP_RATES_CAL_SET                   cal2set,    
    SOC_SAND_IN  ARAD_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE   field_type,
    SOC_SAND_OUT soc_mem_t                                *memory_name,
    SOC_SAND_OUT soc_field_t                              *field_name
  )
{
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_NULL_CHECK(memory_name);
    SOCDNX_NULL_CHECK(field_name);
    SOCDNX_NULL_CHECK(cal_info);

    if (cal_info->cal_type == ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB || 
        cal_info->cal_type == ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY ||
        cal_info->cal_type == ARAD_OFP_RATES_EGQ_CAL_TCG)
    {
        *memory_name = EGQ_EGRESS_SHAPER_CONFIGURATIONm;
        switch (field_type)
        {
        case SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
            *field_name = (cal2set == ARAD_OFP_RATES_CAL_SET_A) ? CH_SPR_RATE_Af:CH_SPR_RATE_Bf;
            break;
        case SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
            *field_name = (cal2set == ARAD_OFP_RATES_CAL_SET_A) ? CH_SPR_MAX_BURST_Af:CH_SPR_MAX_BURST_Bf;      
            break;
        case SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
            *field_name = (cal2set == ARAD_OFP_RATES_CAL_SET_A) ? CH_SPR_CAL_LEN_Af:CH_SPR_CAL_LEN_Bf;      
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG_STR( "Illegal enum")));
        }
    } else {
      SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG_STR( "Illegal enum")));
    }

exit:
    SOCDNX_FUNC_RETURN
}


int 
soc_jer_ofp_rates_egress_shaper_mem_field_read (
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  int                                      core,
    SOC_SAND_IN  ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    SOC_SAND_IN  soc_mem_t                                mem_name,
    SOC_SAND_IN  soc_field_t                              field_name,
    SOC_SAND_OUT uint32                                   *data
    )
{
    soc_reg_above_64_val_t data_above_64;
    uint32 offset = 0;

    SOCDNX_INIT_FUNC_DEFS

    switch (cal_info->cal_type) {
        case SOC_TMC_OFP_RATES_EGQ_CAL_CHAN_ARB:
            offset = cal_info->chan_arb_id;
            break;
        case SOC_TMC_OFP_RATES_EGQ_CAL_TCG:
            offset = JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_TCG;
            break;
        case SOC_TMC_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
            offset = JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_QUEUE_PAIR;
            break;
        default:
            break;
    }

    /* read */
    SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), offset, &data_above_64));
    *data = soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_get(unit, &data_above_64, field_name);

exit:
    SOCDNX_FUNC_RETURN
}

int 
soc_jer_ofp_rates_egress_shaper_mem_field_write (
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  int                                      core,
    SOC_SAND_IN  ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    SOC_SAND_IN  soc_mem_t                                mem_name,
    SOC_SAND_IN  soc_field_t                              field_name,
    SOC_SAND_OUT uint32                                   data
    )
{
    soc_reg_above_64_val_t data_above_64;
    uint32 offset = 0;

    SOCDNX_INIT_FUNC_DEFS

    switch (cal_info->cal_type) {
        case SOC_TMC_OFP_RATES_EGQ_CAL_CHAN_ARB:
            offset = cal_info->chan_arb_id;
            break;
        case SOC_TMC_OFP_RATES_EGQ_CAL_TCG:
            offset = JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_TCG;
            break;
        case SOC_TMC_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
            offset = JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_QUEUE_PAIR;
            break;
        default:
            break;
    }

    /* read */
    SOCDNX_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), offset, &data_above_64));
    /* modify */
    soc_EGQ_EGRESS_SHAPER_CONFIGURATIONm_field32_set(unit, &data_above_64, field_name, data);
    /* write */
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_CONFIGURATIONm(unit, EGQ_BLOCK(unit, core), offset, &data_above_64));

exit:
    SOCDNX_FUNC_RETURN
}

soc_mem_t
  soc_jer_ofp_rates_egq_scm_chan_arb_id2scm_id(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 chan_arb_id
  )
{  
  soc_mem_t
    egq_scm_name;
  /*
   *  Go to the correct table
   */
  switch (chan_arb_id)
  {
      case 0:
        egq_scm_name = EGQ_CH_SCM_0m;
        break;
      case 1:
        egq_scm_name = EGQ_CH_SCM_1m;
        break;
      case 2:
        egq_scm_name = EGQ_CH_SCM_2m;
        break;
      case 3:
        egq_scm_name = EGQ_CH_SCM_3m;
        break;
      case 4:
        egq_scm_name = EGQ_CH_SCM_4m;
        break;
      case 5:
        egq_scm_name = EGQ_CH_SCM_5m;
        break;
      case 6:
        egq_scm_name = EGQ_CH_SCM_6m;
        break;
      case 7:
        egq_scm_name = EGQ_CH_SCM_7m;
        break;
      case 8:
        egq_scm_name = EGQ_CH_SCM_8m;
        break;
      case 9:
        egq_scm_name = EGQ_CH_SCM_9m;
        break;
      case 10:
        egq_scm_name = EGQ_CH_SCM_10m;
        break;
      case 11:
        egq_scm_name = EGQ_CH_SCM_11m;
        break;
      case 12:
        egq_scm_name = EGQ_CH_SCM_12m;
        break;
      case 13:
        egq_scm_name = EGQ_CH_SCM_13m;
        break;
      case 14:
        egq_scm_name = EGQ_CH_SCM_14m;
        break;
      case 15:
        egq_scm_name = EGQ_CH_SCM_15m;
        break;
      case 16:
        egq_scm_name = EGQ_CH_SCM_16m;
        break;
      case 17:
        egq_scm_name = EGQ_CH_SCM_17m;
        break;
      case 18:
        egq_scm_name = EGQ_CH_SCM_18m;
        break;
      case 19:
        egq_scm_name = EGQ_CH_SCM_19m;
        break;
      case 20:
        egq_scm_name = EGQ_CH_SCM_20m;
        break;
      case 21:
        egq_scm_name = EGQ_CH_SCM_21m;
        break;
      case 22:
        egq_scm_name = EGQ_CH_SCM_22m;
        break;
      case 23:
        egq_scm_name = EGQ_CH_SCM_23m;
        break;
      case 24:
        egq_scm_name = EGQ_CH_SCM_24m;
        break;
      case 25:
        egq_scm_name = EGQ_CH_SCM_25m;
        break;
      case 26:
        egq_scm_name = EGQ_CH_SCM_26m;
        break;
      case 27:
        egq_scm_name = EGQ_CH_SCM_27m;
        break;
      case 28:
        egq_scm_name = EGQ_CH_SCM_28m;
        break;
      case 29:
        egq_scm_name = EGQ_CH_SCM_29m;
        break;
      case 30:
        egq_scm_name = EGQ_CH_SCM_30m;
        break;
      case 31:
        egq_scm_name = EGQ_CH_SCM_31m;
        break;
      case JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_NON_CHN:
        egq_scm_name = EGQ_NONCH_SCMm;
        break;
      default:
        egq_scm_name = EGQ_CH_0_SCMm;
  }

  return egq_scm_name;
}

int
soc_jer_ofp_rates_egress_shaper_cal_write (
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  int                                      core,
    SOC_SAND_IN  SOC_TMC_OFP_RATES_CAL_INFO              *cal_info,
    SOC_SAND_IN  SOC_TMC_OFP_RATES_CAL_SET                cal2set,    
    SOC_SAND_IN  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    SOC_SAND_IN  uint32                                   data
    )
{
    soc_field_t field_name;
    soc_mem_t memory_name;

    SOCDNX_INIT_FUNC_DEFS

    /* retrieve memory and field names */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_retrieve_egress_shaper_reg_field_names(
                                                                unit,
                                                                cal_info,
                                                                cal2set,    
                                                                field_type,
                                                                &memory_name,
                                                                &field_name));

    /* write */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_egress_shaper_mem_field_write (
                                                              unit,
                                                              core,
                                                              cal_info,   
                                                              memory_name,
                                                              field_name,
                                                              data));
exit:
    SOCDNX_FUNC_RETURN
}

int
soc_jer_ofp_rates_egress_shaper_cal_read (
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  int                                      core,
    SOC_SAND_IN  SOC_TMC_OFP_RATES_CAL_INFO              *cal_info,
    SOC_SAND_IN  SOC_TMC_OFP_RATES_CAL_SET                cal2set,    
    SOC_SAND_IN  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    SOC_SAND_OUT uint32                                   *data
    )
{
    soc_field_t field_name;
    soc_mem_t memory_name;

    SOCDNX_INIT_FUNC_DEFS

    /* retrieve memory and field names */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_retrieve_egress_shaper_reg_field_names(
                                                                unit,
                                                                cal_info,
                                                                cal2set,    
                                                                field_type,
                                                                &memory_name,
                                                                &field_name));

    /* write */
    SOCDNX_IF_ERR_EXIT(soc_jer_ofp_rates_egress_shaper_mem_field_read (
                                                              unit,
                                                              core,
                                                              cal_info,   
                                                              memory_name,
                                                              field_name,
                                                              data));
exit:
    SOCDNX_FUNC_RETURN
}

#undef _ERR_MSG_MODULE_NAME

