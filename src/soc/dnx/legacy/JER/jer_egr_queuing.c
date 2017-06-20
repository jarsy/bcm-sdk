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

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/types.h>
#include <soc/register.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>

#define SOC_JER2_JER_EGR_MAX_ALPHA_VALUE (7)
#define SOC_JER2_JER_EGR_MIN_ALPHA_VALUE (-7)

typedef struct soc_jer2_jer_egr_if_s {
    uint32 egr_if_in_use[SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES)];
} soc_jer2_jer_egr_if_t;

static soc_jer2_jer_egr_if_t jer2_jer_egr_if[SOC_MAX_NUM_DEVICES][SOC_DNX_DEFS_MAX(NOF_CORES)];

int 
soc_jer2_jer_egr_interface_init(int unit) 
{
    soc_pbmp_t ports_bm;
    int core;
    uint32 egr_if;
    soc_port_t port;
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        for(egr_if=0 ; egr_if<SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES) ; egr_if++) {
            jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] = 0;
        }
    }

    if (SOC_WARM_BOOT(unit)) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &ports_bm));

        SOC_PBMP_ITER(ports_bm, port) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
            DNXC_IF_ERR_EXIT(rv);

            if(egr_if == INVALID_EGR_INTERFACE) {
                continue;
            }

            DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
            jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] = jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] + 1;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
soc_jer2_jer_egr_interface_alloc(int unit, soc_port_t port) 
{
    int core, is_channelized, rate_100g_or_above, is_tdm_queuing = 0;
    uint32 rate_mps, egr_if = 0, other_egr_if = 0;
    soc_port_if_t interface_type;
    soc_port_t master_port;
    int start, end, i;
    uint32     flags;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_master_channel_get(unit, port, &master_port));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags));
    ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_jericho.tm.is_tdm_queuing_on.pbmp_member(unit, port, (uint8*)&is_tdm_queuing));

    if((master_port == port)) { /* First port in interface */
         /* Allocate egress interface */
         switch(interface_type) {
            case SOC_PORT_IF_CPU:
                egr_if = SOC_JER2_JER_EGR_IF_CPU;
                break;

            case SOC_PORT_IF_OLP:
                egr_if = SOC_JER2_JER_EGR_IF_OLP;
                break;

            case SOC_PORT_IF_OAMP:
                egr_if = SOC_JER2_JER_EGR_IF_OAMP;
                break;

            case SOC_PORT_IF_RCY:
                egr_if = SOC_JER2_JER_EGR_IF_RCY;
                break;

            case SOC_PORT_IF_SAT:
                egr_if = SOC_JER2_JER_EGR_IF_SAT;
                break;

            case SOC_PORT_IF_IPSEC:
                egr_if = SOC_JER2_JER_EGR_IF_IPSEC;
                break;


            case SOC_PORT_IF_ILKN:
                egr_if = INVALID_EGR_INTERFACE;
                /* ILKN dedicated mode requiries 2 egress interfaces per ILKN - validate both are free */
                other_egr_if = 0;
                if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) {
                    if (IS_TDM_PORT(unit, port) || is_tdm_queuing) {
                        if ((jer2_jer_egr_if[unit][core].egr_if_in_use[0] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[1] == 0)){
                            egr_if = 0;
                            other_egr_if = 1;
                        } else if ((jer2_jer_egr_if[unit][core].egr_if_in_use[16] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[17] == 0)) {
                            egr_if = 16;
                            other_egr_if = 17;
                        } else if (((jer2_jer_egr_if[unit][core].egr_if_in_use[2] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[3] == 0)) && SOC_IS_QAX(unit)) {
                            egr_if = 2;
                            other_egr_if = 3;
                        } else if (((jer2_jer_egr_if[unit][core].egr_if_in_use[4] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[5] == 0)) && SOC_IS_QAX(unit)) {
                            egr_if = 4;
                            other_egr_if = 5;
                        } else {
                            DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Failed to allocate egr interface, 2 interfaces are needed in ILKN dedicated mode")));
                        }
                    } else {
                        if ((jer2_jer_egr_if[unit][core].egr_if_in_use[0] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[1] == 0)){
                            egr_if = 1;
                            other_egr_if = 0;
                        } else if ((jer2_jer_egr_if[unit][core].egr_if_in_use[16] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[17] == 0)) {
                            egr_if = 17;
                            other_egr_if = 16;
                        } else if (((jer2_jer_egr_if[unit][core].egr_if_in_use[2] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[3] == 0)) && SOC_IS_QAX(unit)) {
                            egr_if = 3;
                            other_egr_if = 2;
                        } else if (((jer2_jer_egr_if[unit][core].egr_if_in_use[4] == 0) && (jer2_jer_egr_if[unit][core].egr_if_in_use[5] == 0)) && SOC_IS_QAX(unit)) {
                            egr_if = 5;
                            other_egr_if = 4;
                        } else {
                            DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Failed to allocate egr interface,  2 interfaces are needed in ILKN dedicated mode")));
                        }
                    }

                    if (jer2_jer_egr_if[unit][core].egr_if_in_use[other_egr_if] != 0) {
                        DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Failed to allocate egr interface, 2 interfaces are needed in ILKN dedicated mode")));
                    }
                }
             break;

             default:
                egr_if = INVALID_EGR_INTERFACE;
                break;
        }

        if(egr_if == INVALID_EGR_INTERFACE) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_channelized_port_get(unit, port, &is_channelized));
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_rate_get(unit, port, &rate_mps));
            rate_100g_or_above = (rate_mps >= 100000 ? 1 : 0);

            /* First search in non-channalized\low-speed range*/
            if(!rate_100g_or_above && !is_channelized) {
                start = SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces) + SOC_DNX_IMP_DEFS_GET(unit, nof_special_non_channelized_interfaces);
                end = SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);

                for(i=start ; i<end ; i++) {
                	  if ((SOC_JER2_JER_EGR_IF_CPU == i) || (SOC_JER2_JER_EGR_IF_OLP == i) ||
                        (SOC_JER2_JER_EGR_IF_OAMP == i) || (SOC_JER2_JER_EGR_IF_RCY == i) ||
                        (SOC_JER2_JER_EGR_IF_SAT == i) || (SOC_JER2_JER_EGR_IF_IPSEC == i)) {
                        continue;
                    }
                    if(jer2_jer_egr_if[unit][core].egr_if_in_use[i] == 0) {
                        egr_if = i;
                        break;
                    }
                }
            }

            /* if not found search in channalized\high speed range*/
            if(egr_if == INVALID_EGR_INTERFACE) {
                start = 0;
                end = SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces) - SOC_DNX_IMP_DEFS_GET(unit, nof_special_channelized_interfaces);

                for(i=start ; i<end ; i++) {
                    /* reserve these egr_if id's for ILKN port */
                    if ((ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) && (i == 0 || i == 1 || i == 16 || i== 17)) {
                        continue;
                    }
                    if ((ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) && SOC_IS_QAX(unit) && (i == 2 || i == 3 || i == 4 || i== 5)) {
                        continue;
                    }
                    if(jer2_jer_egr_if[unit][core].egr_if_in_use[i] == 0) {
                        egr_if = i;
                        break;
                    }
                }
            }

            if(egr_if == INVALID_EGR_INTERFACE) {
                DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Failed to allocate egr interface for port %d"), port));
            }
        }
    } else {
        /* channalized interface - get egr_if of master port */
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, master_port, &egr_if);
        DNXC_IF_ERR_EXIT(rv);
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON && interface_type == SOC_PORT_IF_ILKN) {
            DNX_SAND_SET_BIT(egr_if, 0x0, 0); /*ILKN tdm interleave egr interface*/
            if (!(IS_TDM_PORT(unit, port) || is_tdm_queuing))
            {
                egr_if +=1; /*ILKN not tdm interleave egr interface*/
            }

        }
    }

    if (interface_type != SOC_PORT_IF_LBG) {
        jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] = jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] + 1;
    }
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.set(unit, port, egr_if);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

int 
soc_jer2_jer_egr_interface_free(int unit, soc_port_t port) 
{
    uint32 egr_if;
    int core;
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
    DNXC_IF_ERR_EXIT(rv);

    if(egr_if != INVALID_EGR_INTERFACE) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
        jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] = jer2_jer_egr_if[unit][core].egr_if_in_use[egr_if] - 1;
    }

exit:
    DNXC_FUNC_RETURN;
}


int
  soc_jer2_jer_egr_port2egress_offset(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_OUT uint32              *egr_if
  )
{
    soc_port_t
        port;
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, port, egr_if);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

int
    soc_jer2_jer_egr_is_channelized(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_OUT uint32                *is_channalzied)
{
    uint32 reg_val[1], egr_if;
    DNXC_INIT_FUNC_DEFS;

    /*Get channel arbiter*/
    DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_port2egress_offset(unit, core, tm_port, &egr_if));

    /*Only first 32 interfaces might be channelized*/
    if(egr_if < 32 /*Doron TBD - replace with defines tool*/) {
        DNXC_IF_ERR_EXIT(READ_EGQ_IFC_IS_CHANNELIZEDr(unit, core, reg_val));
        if(SHR_BITGET(reg_val, egr_if)) {
            *is_channalzied = 1;
        } else {
            *is_channalzied = 0;
        }
    } else {
        *is_channalzied = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_jer_egr_nrdy_th_profile_data_set(
    DNX_SAND_IN int unit, 
    DNX_SAND_IN int core,
    DNX_SAND_IN uint32 profile,
    DNX_SAND_IN uint32 profile_data)
{
    uint32 data;
    DNXC_INIT_FUNC_DEFS;

    data = 0;
    
    if (profile <= 2) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_0_2r(unit, core, &data));
        if (profile == 0) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_0_2r, &data, NRDY_TH_0f, profile_data);
        } else if (profile == 1) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_0_2r, &data, NRDY_TH_1f, profile_data);
        } else {
            soc_reg_field_set(unit, EGQ_NRDY_TH_0_2r, &data, NRDY_TH_2f, profile_data);
        }
        DNXC_IF_ERR_EXIT(WRITE_EGQ_NRDY_TH_0_2r(unit, core, data));
    }

    else if (profile >= 3 && profile <= 5) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_3_5r(unit, core, &data));
        if (profile == 3) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_3_5r, &data, NRDY_TH_3f, profile_data);
        } else if (profile == 4) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_3_5r, &data, NRDY_TH_4f, profile_data);
        } else {
            soc_reg_field_set(unit, EGQ_NRDY_TH_3_5r, &data, NRDY_TH_5f, profile_data);
        }
        DNXC_IF_ERR_EXIT(WRITE_EGQ_NRDY_TH_3_5r(unit, core, data));
    }

    else if (profile >= 6 && profile <= 8) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_6_8r(unit, core, &data));
        if (profile == 6) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_6_8r, &data, NRDY_TH_6f, profile_data);
        } else if (profile == 7) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_6_8r, &data, NRDY_TH_7f, profile_data);
        } else {
            soc_reg_field_set(unit, EGQ_NRDY_TH_6_8r, &data, NRDY_TH_8f, profile_data);
        }
        DNXC_IF_ERR_EXIT(WRITE_EGQ_NRDY_TH_6_8r(unit, core, data));
    }

    else if (profile >= 9 && profile <= 11) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_9_11r(unit, core, &data));
        if (profile == 9) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_9_11r, &data, NRDY_TH_9f, profile_data);
        } else if (profile == 10) {
            soc_reg_field_set(unit, EGQ_NRDY_TH_9_11r, &data, NRDY_TH_10f, profile_data);
        } else {
            soc_reg_field_set(unit, EGQ_NRDY_TH_9_11r, &data, NRDY_TH_11f, profile_data);
        }
        DNXC_IF_ERR_EXIT(WRITE_EGQ_NRDY_TH_9_11r(unit, core, data));
    }

    else {
        DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Invalid profile id %d "), profile));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_jer_egr_nrdy_th_profile_data_get(
    DNX_SAND_IN int unit, 
    DNX_SAND_IN int core,
    DNX_SAND_IN uint32 profile,
    DNX_SAND_OUT uint32 *profile_data)
{
    uint32 data;
    DNXC_INIT_FUNC_DEFS;

    data = 0;
    
    if (profile <= 2) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_0_2r(unit, core, &data));
        if (profile == 0) {
            *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_0_2r, data, NRDY_TH_0f);
        } else if (profile == 1) {
            *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_0_2r, data, NRDY_TH_1f);
        } else {
            *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_0_2r, data, NRDY_TH_2f);
        }
    }

    else if (profile >= 3 && profile <= 5) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_3_5r(unit, core, &data));
        if (profile == 3) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_3_5r, data, NRDY_TH_3f);
        } else if (profile == 4) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_3_5r, data, NRDY_TH_4f);
        } else {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_3_5r, data, NRDY_TH_5f);
        }
    }

    else if (profile >= 6 && profile <= 8) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_6_8r(unit, core, &data));
        if (profile == 6) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_6_8r, data, NRDY_TH_6f);
        } else if (profile == 7) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_6_8r, data, NRDY_TH_7f);
        } else {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_6_8r, data, NRDY_TH_8f);
        }
    }

    else if (profile >= 9 && profile <= 11) {
        DNXC_IF_ERR_EXIT(READ_EGQ_NRDY_TH_9_11r(unit, core, &data));
        if (profile == 9) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_9_11r, data, NRDY_TH_9f);
        } else if (profile == 10) {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_9_11r, data, NRDY_TH_10f);
        } else {
             *profile_data = soc_reg_field_get(unit, EGQ_NRDY_TH_9_11r, data, NRDY_TH_11f);
        }
    }

    else {
        DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("Invalid profile id %d "), profile));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_jer_egr_tm_init(int unit)
{   
    soc_pbmp_t pbmp;
    soc_port_t port_i;
    uint32 tm_port, base_q_pair, priority_mode, is_master, tdm_egress_priority, tdm_egress_dp;
    uint32 reg_val, fld_val, flags;
    DNX_TMC_EGR_PORT_SHAPER_MODE shaper_mode;
    int core, is_tdm_queuing = 0;
    soc_port_if_t interface_type;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_reg_above_64_val_t reg_val_above_64, fld_val_above_64;
    soc_error_t rv;
    DNX_TMC_EGR_OFP_SCH_INFO ofp_sch_info;

    DNXC_INIT_FUNC_DEFS;

    /* Regs init */
    reg_val = 0;
    soc_reg_field_set(unit, EGQ_PQP_AND_FQP_CALENDER_SETTINGr, &reg_val, PQP_CALENDER_SWITCH_ENf, 1);
    soc_reg_field_set(unit, EGQ_PQP_AND_FQP_CALENDER_SETTINGr, &reg_val, FQP_CALENDER_SWITCH_ENf, 1);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PQP_AND_FQP_CALENDER_SETTINGr(unit, SOC_CORE_ALL, reg_val));

    DNXC_IF_ERR_EXIT(jer2_arad_egr_queuing_init(unit, SOC_DNX_CONFIG(unit)->jer2_arad->init.eg_cgm_scheme));

    /* In case ERP port is enabled, search for unoccupied NIF interface */
    /*DNXC_IF_ERR_EXIT(jer2_arad_ports_init_interfaces_erp_setting(unit, &(SOC_DNX_CONFIG(unit)->jer2_arad->init.ports));*/

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));

    SOC_PBMP_ITER(pbmp, port_i) {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags));
        if (DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags)) {
            continue;
        }
       
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core));
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair);
        DNXC_IF_ERR_EXIT(rv);
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_out_port_priority_get(unit, port_i, &priority_mode));
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.shaper_mode.get(unit, port_i, &shaper_mode);
        DNXC_IF_ERR_EXIT(rv);
        

        /* Map local port to base_q_pair */
        DNXC_IF_ERR_EXIT(jer2_arad_egr_dsp_pp_to_base_q_pair_set(unit, core,  tm_port, base_q_pair));

        /* Set port priorities */
        DNXC_IF_ERR_EXIT(jer2_arad_egr_dsp_pp_priorities_mode_set(unit,core, tm_port, priority_mode));

        /* set port shaper mode*/
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_dsp_pp_shaper_mode_set_unsafe(unit, core, tm_port, shaper_mode));

        /* Map port to interface */
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        DNXC_IF_ERR_EXIT(soc_jer2_jer_port_to_interface_map_set(unit, port_i, 0));
#endif 

        /*Fast port enable*/
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, port_i, &is_master));
        if (is_master) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port_i, &interface_type));
            if (interface_type == SOC_PORT_IF_ILKN) { 
                DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_fast_port_set(unit, port_i, 1, 1));
            } else if (interface_type == SOC_PORT_IF_CAUI) {
                DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_fast_port_set(unit, port_i, 0, 1));
            } else {
                DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_fast_port_set(unit, port_i, 0, 0));
            }
        }
        /* set all TDM ports to high priority for egress scheduler (calendars) */
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_jericho.tm.is_tdm_queuing_on.pbmp_member(unit, port_i, (uint8*)&is_tdm_queuing));
        if (IS_TDM_PORT(unit, port_i) || is_tdm_queuing) {
            DNX_TMC_EGR_OFP_SCH_INFO_clear(&ofp_sch_info);
            DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_ofp_scheduling_get(unit, core, tm_port, &ofp_sch_info));
            ofp_sch_info.nif_priority = DNX_TMC_EGR_OFP_INTERFACE_PRIO_HIGH;
            DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_ofp_scheduling_set(unit, core, tm_port, &ofp_sch_info));
        }
    }

    /* per core Init */
    ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
    tdm_egress_priority = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_priority;
    tdm_egress_dp = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_dp;

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {

        /* TDM SP mode configuration */
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) {
            /*Enable ilkn_tdm_dedicated_queueing*/
            fld_val = 0x1;
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, core, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, core, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
            if (SOC_IS_QAX(unit)) {
                DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_ILAKEN_2_INTERLEAVE_ENf,  fld_val));
                DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, core, 0, TDM_ILAKEN_2_INTERLEAVE_ENf,  fld_val));
                DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_ILAKEN_3_INTERLEAVE_ENf,  fld_val));
                DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, core, 0, TDM_ILAKEN_3_INTERLEAVE_ENf,  fld_val));
            }

            /* enable TDM traffic in EGQ */
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, EGRESS_TDM_MODEf,  fld_val));
        }

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_PKT_TCf, tdm_egress_priority));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, core, 0, TDM_PKT_DPf, tdm_egress_dp));
    }

    /* By default set disable fragmentation for all interfaces */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        SOC_REG_ABOVE_64_CLEAR(reg_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITSET_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, NIF_NO_FRAG_Lf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,NIF_NO_FRAG_Lf,fld_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITSET_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, NIF_NO_FRAG_Hf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,NIF_NO_FRAG_Hf,fld_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITCLR_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, CPU_NO_FRAGf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,CPU_NO_FRAGf,fld_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITCLR_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, OLP_NO_FRAGf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,OLP_NO_FRAGf,fld_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITCLR_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, OAM_NO_FRAGf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,OAM_NO_FRAGf,fld_val_above_64);

        SOC_REG_ABOVE_64_CLEAR(fld_val_above_64);
        SHR_BITCLR_RANGE(fld_val_above_64, 0, soc_reg_field_length(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, RCY_NO_FRAGf));
        soc_reg_above_64_field_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_val_above_64,RCY_NO_FRAGf,fld_val_above_64);

        DNXC_IF_ERR_EXIT(WRITE_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_val_above_64));
    }


exit:
    DNXC_FUNC_RETURN;
}

/* returns TRUE if all numbers are equal*/
static
int is_all_equal(uint32 *numbers, uint32 nof_numbers){
    uint32 i, comparator = numbers[0];
    for (i = 1; i < nof_numbers; i++) {
        if (numbers[i] != comparator) {
            return FALSE;
        }
    }
    return TRUE;
}

/* returns the index of the smallest number (1st occurence)*/
static
void get_smallest_idx(uint32 *numbers, uint32 nof_numbers, uint32 *smallest_idx){
    int i;
    *smallest_idx = 0;
    for (i = 1; i < nof_numbers; i++) {
        if (numbers[i] < numbers[*smallest_idx]) {
            *smallest_idx = i;
        }
    }
}

/* Returns the index of the last non-zero element in the arr */
static
uint32 get_last_value(uint32 *arr, uint32 arr_length){
    uint32 i, last_idx = 0;

    for(i = 0; i < arr_length; i++){
        if(arr[i] != 0){
            last_idx = i;
        }
    }

    return last_idx;
}

static
int soc_jer2_jer_egr_lcm_above_sum_get(int unit, uint32 *numbers, uint32 nof_numbers, uint32 sum, uint32 *lcm){
    uint32 curr_iter[SOC_PBMP_PORT_MAX] = {0}, current_lcm = 0, smallest_idx = 0;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(numbers);
    sal_memcpy(curr_iter, numbers, nof_numbers);

    while (current_lcm < sum) {
        if(is_all_equal(curr_iter, nof_numbers)){
            if (current_lcm != curr_iter[0]) {
                current_lcm = curr_iter[0]; 
                continue;
            }
        }
        get_smallest_idx(curr_iter, nof_numbers, &smallest_idx);
        curr_iter[smallest_idx] += numbers[smallest_idx];
    }

    *lcm = current_lcm;
exit:
    DNXC_FUNC_RETURN;    
}



/* rate in mbps , lanes as number of instances this port will have on calendar */
static
int soc_jer2_jer_egr_q_nif_rate_to_lanes(int unit, uint32 rate, uint32 *lanes)
{
	DNXC_INIT_FUNC_DEFS;
	DNXC_NULL_CHECK(lanes);
	
    if (rate == 0) 
    {   
       *lanes = 0;
    }

    /* present rate as numer of lanes  1/2/4/8/16 */
    if(rate > 0 && rate <= 10000){
        *lanes = 1;
    } else if (rate > 10000 && rate <= 20000) {
        *lanes = 2;
    } else if (rate > 20000 && rate <= 100000) {
        *lanes = 4;
    } else if (rate > 100000 && rate <= 140000) {
        *lanes = 8;
    } else if (rate > 140000) {
       *lanes = 16;
    }   

exit:
    DNXC_FUNC_RETURN; 
}


#define SOC_JER2_JER_CAL_LANES_OTHERS   4 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_OLP      1 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_OAMP     1 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_CPU      1 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_RCY      1 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_SAT      4 /*Lanes*/
#define SOC_JER2_JER_CAL_LANES_IPSEC    4 /*Lanes*/

#define SOC_JER2_JER_FQP_CAL_IF_OTHERS_GIVE_BW_IF_NEEDED   28
#define SOC_JER2_JER_FQP_CAL_IF_OTHERS_GIVE                29
#define SOC_JER2_JER_FQP_CAL_IF_1_GIG_NIFS                 30
#define SOC_JER2_JER_FQP_CAL_IF_INVALID                    31

#define SOC_JER2_JER_PQP_CAL_IF_1_GIG_NIFS                 129
#define SOC_JER2_JER_PQP_CAL_IF_INVALID                    128

#define SOC_JER2_JER_CAL_ILLEGAL_NIF                       0xfff

#define SOC_JER2_JER_MAX_CAL_LENGTH                        256

int
  soc_jer2_jer_egr_q_nif_cal_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN soc_jer2_jer_egr_cal_type_t cal_to_update
  )
{
    soc_port_t port;
    uint32 egr_if, flags, is_master, other_pqp_to_fill;
    uint32 if_rate_mbps, total_lanes_sum = 0, lane_rate;
    soc_pbmp_t port_bm;
    uint32  calendar_length, nof_padding_if;
    int i, j = 0, nof_active_if = 0;
    int is_special_int = 0, max_ports_nbih;
    soc_field_t cal_set_field;
    soc_mem_t cal_mem;
    uint32 reg_val, current_cal, next_cal, next_cal_offset;
    uint32 lanes[SOC_PBMP_PORT_MAX] = {0}, lcm_temp[SOC_PBMP_PORT_MAX] = {0};
    uint32 lanes_temp_others = 0, lanes_temp_rate_others = 0;
    uint32 lanes_temp_cpu = 1, lanes_temp_olp = 1;/*default value for lowest rate*/
    uint32 lanes_temp_oamp = 1, lanes_temp_rcy = 1, lanes_temp_sat = 1, lanes_temp_ipsec = 1;
    uint32 lanes_temp_lb = 1;
    JER2_ARAD_OFP_RATES_CAL_SCH *calendar = NULL;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_port_if_t interface_type;
    uint32 *temp_buff = NULL;
    uint32 lanes_temp = 0, num_of_lanes, first_lane, shifted_lane = 0;
    uint32 if_1g_idx = (cal_to_update == socJerCalTypeFQP ? SOC_JER2_JER_FQP_CAL_IF_1_GIG_NIFS : SOC_JER2_JER_PQP_CAL_IF_1_GIG_NIFS);
    uint32 dummy_start = 0;
    uint32 tdm_egr_if_mask = 0xfffe;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;

    max_ports_nbih = SOC_DNX_DEFS_GET(unit, nof_ports_nbih);

    DNXC_ALLOC(calendar, JER2_ARAD_OFP_RATES_CAL_SCH, 1,"calendar egr_q_nif_cal");
    DNXC_ALLOC(temp_buff, uint32, SOC_JER2_JER_MAX_CAL_LENGTH, "temp_buff");

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &port_bm));

    SOC_PBMP_ITER(port_bm, port) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags));
        is_special_int = 0;

        if (DNX_PORT_IS_STAT_INTERFACE(flags)) {
            /* Statistics interface is not required for Egress */
            continue;
        }
        if (DNX_PORT_IS_ELK_INTERFACE(flags)){
            continue;
        }
        
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, port, &is_master));
        if(!is_master) {
            continue;
        }
        
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
        DNXC_IF_ERR_EXIT(rv);
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_rate_get(unit, port, &if_rate_mbps));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));

        switch (interface_type) {
            case SOC_PORT_IF_NOCXN:
            {
                continue;
            }
            case SOC_PORT_IF_ERP:
            {
                continue;
            }
            case SOC_PORT_IF_CPU:
            case SOC_PORT_IF_OLP:
            case SOC_PORT_IF_OAMP:
            case SOC_PORT_IF_RCY:
            case SOC_PORT_IF_SAT:
            case SOC_PORT_IF_IPSEC:
            {
                is_special_int = 1;
                break;
            }
            case SOC_PORT_IF_LBG:
            default:
            {
                break;
            }
        }

        /*TDM SP MODE CONFIGURATION*/
        ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;;
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) {
            if (interface_type == SOC_PORT_IF_ILKN) {
               egr_if = egr_if&tdm_egr_if_mask;
            }
        }
        
        if (!(is_special_int)) 
        {
            /* get number of lanes */
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_num_lanes_get(unit, port, &num_of_lanes));
            /* get first lane */
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port, &first_lane));
            /* get shifted lane id to check if it is falcon or eagle*/
            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_qsgmii_offsets_remove, (unit, first_lane, &shifted_lane));
            --shifted_lane;
            if(unit > BCM_MAX_NUM_UNITS) {
                DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_UNIT);
            }
            DNX_SAND_IF_ERR_EXIT(rv);
            lane_rate = if_rate_mbps / num_of_lanes;

            if (shifted_lane < max_ports_nbih && lane_rate > 20000) { /* falcon*/
                lanes_temp = 2 * num_of_lanes;
            } else {
                lanes_temp = num_of_lanes;
            }
        }

        switch (interface_type) {
            case SOC_PORT_IF_ILKN:
            {
                lanes[egr_if] = lanes_temp;
                lanes[egr_if] +=  (lanes[egr_if] % 4 == 0 ? 0 : 4 - (lanes[egr_if] % 4));
                total_lanes_sum += lanes[egr_if];
                break;
            }
            case SOC_PORT_IF_SGMII:
            case SOC_PORT_IF_QSGMII:
            {
                lanes[if_1g_idx] += lanes_temp;
                break;
            }
            case SOC_PORT_IF_CPU:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_cpu);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_OLP:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_olp);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_OAMP:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_oamp);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_RCY:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_rcy);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_SAT:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_sat);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_IPSEC:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps,&lanes_temp_ipsec);
                lanes_temp_rate_others += if_rate_mbps;
                break;
            }
            case SOC_PORT_IF_LBG:
            {
                soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, if_rate_mbps, &lanes_temp_lb);
                total_lanes_sum += lanes_temp_lb; 
                lanes[egr_if] = lanes_temp_lb;
                if (cal_to_update == socJerCalTypeFQP) {
                    total_lanes_sum += lanes_temp_lb; 
                    lanes[egr_if+1] = lanes_temp_lb;
                }
                break;
            }
            default:
            {
                total_lanes_sum += lanes_temp; 
                lanes[egr_if] = lanes_temp;
                break;
            }
        }
    }

    /* round up 1G lanes to 1/2/4/8/16 */
    if(lanes[if_1g_idx] > 0 && lanes[if_1g_idx] < 10){
        lanes[if_1g_idx] = 10;
    } else if (lanes[if_1g_idx] > 10 && lanes[if_1g_idx] < 20) {
        lanes[if_1g_idx] = 20;
    } else if (lanes[if_1g_idx] > 20 && lanes[if_1g_idx] < 40) {
        lanes[if_1g_idx] = 40;
    } else if (lanes[if_1g_idx] > 40 && lanes[if_1g_idx] < 80) {
        lanes[if_1g_idx] = 80;
    } else if (lanes[if_1g_idx] > 80 && lanes[if_1g_idx] < 160) {
        lanes[if_1g_idx] = 160;
    }
    lanes[if_1g_idx] /= 10;
    total_lanes_sum += lanes[if_1g_idx];


    /* handle special interfaces */
    if(cal_to_update == socJerCalTypeFQP) { /*FQP*/
        soc_jer2_jer_egr_q_nif_rate_to_lanes(unit, lanes_temp_rate_others, &lanes_temp_others);
        lanes_temp_others = (lanes_temp_others > 4 ? lanes_temp_others : 4);
        total_lanes_sum += lanes_temp_others;
        lanes[SOC_JER2_JER_FQP_CAL_IF_OTHERS_GIVE_BW_IF_NEEDED] = lanes_temp_others;
    } else /*PQP*/ {

        total_lanes_sum += lanes_temp_cpu;
        lanes[SOC_JER2_JER_EGR_IF_CPU] = lanes_temp_cpu;

        total_lanes_sum += lanes_temp_olp;
        lanes[SOC_JER2_JER_EGR_IF_OLP] = lanes_temp_olp;

        total_lanes_sum += lanes_temp_oamp;
        lanes[SOC_JER2_JER_EGR_IF_OAMP] = lanes_temp_oamp;

        total_lanes_sum += lanes_temp_rcy;
        lanes[SOC_JER2_JER_EGR_IF_RCY] = lanes_temp_rcy;

        if (SOC_IS_QAX(unit)) {
            total_lanes_sum += lanes_temp_sat;
            lanes[SOC_JER2_JER_EGR_IF_SAT] = lanes_temp_sat;

            total_lanes_sum += lanes_temp_ipsec;
            lanes[SOC_JER2_JER_EGR_IF_IPSEC] = lanes_temp_ipsec;
        }
    }

    /* get LCM (lowest common multiple) for the calendar length*/
    for (i = 0; i < SOC_PBMP_PORT_MAX; i++) {
        if (lanes[i] != 0) {
            nof_active_if++;
        }
    }
    for (i = 0, j = 0; i < SOC_PBMP_PORT_MAX; i++) {
        if (lanes[i] != 0) {
            lcm_temp[j++] = lanes[i];
        }
    }
    DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_lcm_above_sum_get(unit, lcm_temp, nof_active_if, total_lanes_sum, &calendar_length)); 

    /* calculate how many dummy interfaces are required and pad accordingly*/
    nof_padding_if = calendar_length - total_lanes_sum;

    dummy_start = get_last_value(lanes, SOC_PBMP_PORT_MAX) + 1;

    for (i = 0; i < nof_padding_if; i++) {
        lanes[dummy_start + i] = 1;
    }

    /*
    * Given the optimal calendar length and the
    * corresponding weight (in slots) of each port,
    * build a calendar that will avoid burstiness
    * behavior as much as possible.
    */
    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_fixed_len_cal_build(unit, lanes, dummy_start + nof_padding_if + 1, calendar_length, SOC_JER2_JER_MAX_CAL_LENGTH, 1, calendar->slots));

    /* Replace dummies with special IFs */
    other_pqp_to_fill = SOC_JER2_JER_EGR_IF_MIN;
    for (i = 0; i < calendar_length; i++)
    {
        if (calendar->slots[i] >= dummy_start) {
            if (cal_to_update == socJerCalTypeFQP) { /*FQP*/
                calendar->slots[i] = SOC_JER2_JER_FQP_CAL_IF_OTHERS_GIVE_BW_IF_NEEDED;
            } else { /*PQP, replace each time with different special interface*/
                calendar->slots[i] = other_pqp_to_fill;
                other_pqp_to_fill = (other_pqp_to_fill == SOC_JER2_JER_EGR_IF_MAX) ?
                    SOC_JER2_JER_EGR_IF_MIN : other_pqp_to_fill+1;
            }
        }
    }
    /* set rest of celendatr entries to not valid */
    for (i=calendar_length; i<SOC_JER2_JER_MAX_CAL_LENGTH; i++) {
        if (cal_to_update == socJerCalTypePQP) {
            calendar->slots[i] = SOC_JER2_JER_PQP_CAL_IF_INVALID; 
        } else {
            calendar->slots[i] = SOC_JER2_JER_FQP_CAL_IF_INVALID;
        }
    }

    /* configure calendar */
    if (cal_to_update == socJerCalTypeFQP) { /* PQP*/
        cal_set_field = FQP_CALENDER_SET_SELf;
        cal_mem = EGQ_FQP_NIF_PORT_MUXm;
    } else /*PQP*/ {
        cal_set_field = PQP_CALENDER_SET_SELf;
        cal_mem = EGQ_PQP_NIF_PORT_MUXm;
    }

    DNXC_IF_ERR_EXIT(READ_EGQ_PQP_AND_FQP_CALENDER_SETTINGr(unit, core, &reg_val));
    current_cal = soc_reg_field_get(unit, EGQ_PQP_AND_FQP_CALENDER_SETTINGr, reg_val, cal_set_field);
    next_cal = current_cal ? 0 : 1;
    next_cal_offset = next_cal ? SOC_JER2_JER_MAX_CAL_LENGTH : 0;

    for (i=0; i<SOC_JER2_JER_MAX_CAL_LENGTH; i++) {
        DNXC_IF_ERR_EXIT(soc_mem_write(unit, cal_mem, EGQ_BLOCK(unit, core), i+next_cal_offset, &(calendar->slots[i])));
    }

    /* switch calendar A<->B sets */
    soc_reg_field_set(unit, EGQ_PQP_AND_FQP_CALENDER_SETTINGr, &reg_val, cal_set_field, next_cal);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PQP_AND_FQP_CALENDER_SETTINGr(unit, core, reg_val));

exit:
    SOC_FREE(temp_buff);
    SOC_FREE(calendar);
    DNXC_FUNC_RETURN;
}

int
  soc_jer2_jer_egr_q_fqp_scheduler_config(
    DNX_SAND_IN int unit
  )
{
    soc_pbmp_t port_bm;
    uint32 flags, is_master, if_rate_mbps, egr_if, field_val;
    soc_port_t port;
    int core, is_tdm_queuing = 0;
    soc_reg_above_64_val_t data;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_port_if_t interface_type;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;
  
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        /*Bandwidth normal requests by others*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_RCY_REQ_ENf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_OAM_REQ_ENf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_OLP_REQ_ENf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_DEL_REQ_ENf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_CPU_REQ_ENf,  0x1));

        /*Bandwidth Higher requests by others*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_RCY_HIGHER_ENf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_OAM_HIGHER_ENf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_OLP_HIGHER_ENf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_DEL_HIGHER_ENf,  0x0));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_FQP_SCHEDULER_CONFIGURATIONr, core, 0, CFG_CPU_HIGHER_ENf,  0x0));
    }

    /*Disable higher requests on account of 100g ports or higher*/
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, DNX_PORT_FLAGS_NETWORK_INTERFACE, &port_bm));

    SOC_PBMP_ITER(port_bm, port) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags));

        if (DNX_PORT_IS_STAT_INTERFACE(flags)) {
            /* Statistics interface is not required for Egress */
            continue;
        }
        if (DNX_PORT_IS_ELK_INTERFACE(flags)){
            continue;
        }
        if (DNX_PORT_IS_LB_MODEM(flags)){
            continue;
        }
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, port, &is_master));
        if(!is_master) {
            continue;
        }

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_rate_get(unit, port, &if_rate_mbps));
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
        DNXC_IF_ERR_EXIT(rv);
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_jericho.tm.is_tdm_queuing_on.pbmp_member(unit, port, (uint8*)&is_tdm_queuing));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));

        /* set interface from higer request bitmap*/
        DNXC_IF_ERR_EXIT(READ_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit ,core), egr_if, data));

        field_val = (if_rate_mbps >= JER2_ARAD_EGR_QUEUEING_HIGER_REQ_MAX_RATE_MBPS) ? 0 : 1; 
        soc_mem_field_set(unit, EGQ_PER_IFC_CFGm, data, HIGHER_REQ_EN_PER_MALf, &field_val);

        DNXC_IF_ERR_EXIT(WRITE_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit ,core), egr_if, data));


        /* handle ILKN dedicated mode */
        ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
        /* we configured the master port egr_if, now we configure the other egr_if for ILKN dedicated mode */
        if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON && interface_type == SOC_PORT_IF_ILKN) {
            if (IS_TDM_PORT(unit, port) || is_tdm_queuing) {
                egr_if++;
            } else {
                egr_if--;
            }

            /* set interface from higer request bitmap*/
            DNXC_IF_ERR_EXIT(READ_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit ,core), egr_if, data));

            field_val = (if_rate_mbps >= JER2_ARAD_EGR_QUEUEING_HIGER_REQ_MAX_RATE_MBPS) ? 0 : 1; 
            soc_mem_field_set(unit, EGQ_PER_IFC_CFGm, data, HIGHER_REQ_EN_PER_MALf, &field_val);

            DNXC_IF_ERR_EXIT(WRITE_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit ,core), egr_if, data));
        }
    }
exit:
    DNXC_FUNC_RETURN;

}

int
  soc_jer2_jer_egr_q_nif_cal_set_all(
    DNX_SAND_IN int unit
  )
{
    int core;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_fqp_scheduler_config(unit));
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_nif_cal_set(unit, core, socJerCalTypeFQP));
        DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_q_nif_cal_set(unit, core, socJerCalTypePQP));
    }

exit:
    DNXC_FUNC_RETURN;
}


int 
soc_jer2_jer_egr_q_fast_port_set(
        DNX_SAND_IN  int unit, 
        DNX_SAND_IN  soc_port_t port, 
        DNX_SAND_IN  int is_ilkn, 
        DNX_SAND_IN  int turn_on
        )
{
    uint32 egress_offset, data, fld_val, tm_port;
    int core, is_tdm_queuing = 0;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core)); 

    DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_port2egress_offset(unit, core, tm_port, &egress_offset));

    /* read */
    DNXC_IF_ERR_EXIT(READ_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit, core), egress_offset, &data));

    /* modify Fast port enable */
    fld_val = (turn_on ? 1 : 0);
    soc_EGQ_PER_IFC_CFGm_field32_set(unit, &data, FAST_PORT_ENf, fld_val);

    /* write */
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit, core), egress_offset, &data));

    /* handle ILKN dedicated mode */
    ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_jericho.tm.is_tdm_queuing_on.pbmp_member(unit, port, (uint8*)&is_tdm_queuing));
    if (is_ilkn && ilkn_tdm_dedicated_queuing) {
        if (IS_TDM_PORT(unit, port) || is_tdm_queuing) {
            egress_offset++;
        } else {
            egress_offset--;
        }

        /* read */
        DNXC_IF_ERR_EXIT(READ_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit, core), egress_offset, &data));

        /* modify Fast port enable */
        soc_EGQ_PER_IFC_CFGm_field32_set(unit, &data, FAST_PORT_ENf, fld_val);

        /* write */
        DNXC_IF_ERR_EXIT(WRITE_EGQ_PER_IFC_CFGm(unit, EGQ_BLOCK(unit, core), egress_offset, &data));
    }

exit:
    DNXC_FUNC_RETURN; 
}

static
uint32 _soc_jer2_jer_convert_alpha_to_value(int unit, int alpha_value, uint32* field_val)
{
    DNXC_INIT_FUNC_DEFS;
    if (alpha_value > SOC_JER2_JER_EGR_MAX_ALPHA_VALUE || alpha_value < SOC_JER2_JER_EGR_MIN_ALPHA_VALUE)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid alpha value %d "), alpha_value));
    }

    else if (alpha_value == 0)
    {
        *field_val = 0;
    }
    else if (alpha_value > 0)
    {
        *field_val = alpha_value;
    }
    else
    {
        *field_val = -1*alpha_value;
        *field_val |= 0x8; /* MSB must be 1 to indicate negative number */
    }

exit:
    DNXC_FUNC_RETURN
}

uint32 _soc_jer2_jer_convert_field_val_to_alpha(int unit, uint32 field_val, int* alpha_value)
{
    uint32 abs_value = 0;
    DNXC_INIT_FUNC_DEFS;
    abs_value = field_val & 0x7; /* 3 LS bits*/
    *alpha_value = abs_value;
    if (field_val & 0x8)
    {
        *alpha_value = *alpha_value * (-1);
    }

    DNXC_FUNC_RETURN;
}

uint32
  soc_jer2_jer_egr_queuing_sp_tc_drop_set_unsafe(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_IN    int   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;
    uint32 alpha_field_val;

    DNXC_INIT_FUNC_DEFS;
    if (threshold_type == soc_dnx_cosq_threshold_data_buffers)
    {
        /* Legacy, write max=min, alpha = 0*/
        DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_queuing_sp_tc_drop_set_unsafe(unit, core, tc, soc_dnx_cosq_threshold_data_buffers_max, threshold_value, drop_type));
        /* write the same value to min */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_queuing_sp_tc_drop_set_unsafe(unit, core, tc, soc_dnx_cosq_threshold_data_buffers_min, threshold_value, drop_type));
        /* write 0 to alpha */
        DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_queuing_sp_tc_drop_set_unsafe(unit, core, tc, soc_dnx_cosq_thereshold_data_buffers_alpha, 0, drop_type));
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sp_tc_drop_set_unsafe(unit, core, tc, threshold_type, threshold_value, drop_type));
    }
  
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers_max)
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sp_tc_drop_set_unsafe(unit, core, tc, soc_dnx_cosq_threshold_data_buffers, threshold_value, drop_type));
    }
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers_min)
    {
        DNXC_IF_ERR_EXIT(READ_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr(unit, core, reg));
        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MINIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MINIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MINIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MINIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MINIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MINIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MINIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MINIMUMLIMITf, threshold_value);
                    break;
            }
        }
        else /* SP 1*/ 
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MINIMUMLIMITf, threshold_value);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MINIMUMLIMITf, threshold_value);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MINIMUMLIMITf, threshold_value);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MINIMUMLIMITf, threshold_value);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MINIMUMLIMITf, threshold_value);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MINIMUMLIMITf, threshold_value);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MINIMUMLIMITf, threshold_value);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MINIMUMLIMITf, threshold_value);
                    break;              
            }
        }
        DNXC_IF_ERR_EXIT(WRITE_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr(unit, core, reg));
    }
    else if (threshold_type == soc_dnx_cosq_thereshold_data_buffers_alpha)
    {
        DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, threshold_value, &alpha_field_val));
        DNXC_IF_ERR_EXIT(READ_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr(unit, core, reg));

        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_0_ALPHAf, alpha_field_val);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_1_ALPHAf, alpha_field_val);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_2_ALPHAf, alpha_field_val);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_3_ALPHAf, alpha_field_val);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_4_ALPHAf, alpha_field_val);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_5_ALPHAf, alpha_field_val);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_6_ALPHAf, alpha_field_val);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_7_ALPHAf, alpha_field_val);
                    break;
            }
        }
        else /* SP 1*/
        {
            switch (tc)
            {
                case 0:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_0_ALPHAf, alpha_field_val);
                    break;
                case 1:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_1_ALPHAf, alpha_field_val);
                    break;
                case 2:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_2_ALPHAf, alpha_field_val);
                    break;
                case 3:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_3_ALPHAf, alpha_field_val);
                    break;
                case 4:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_4_ALPHAf, alpha_field_val);
                    break;
                case 5:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_5_ALPHAf, alpha_field_val);
                    break;
                case 6:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_6_ALPHAf, alpha_field_val);
                    break;
                case 7:
                    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_7_ALPHAf, alpha_field_val);
                    break;
            }
        }
        DNXC_IF_ERR_EXIT(WRITE_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr(unit, core, reg));
    }

exit:
    DNXC_FUNC_RETURN;
}


uint32
  soc_jer2_jer_egr_queuing_sp_tc_drop_get_unsafe(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    tc,
    DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type,
    DNX_SAND_OUT   int*   threshold_value,
    DNX_SAND_IN    soc_dnx_cosq_threshold_global_type_t drop_type
  )
{
    soc_reg_above_64_val_t reg;
    uint32 field_val = 0;

    DNXC_INIT_FUNC_DEFS;
    if (threshold_type == soc_dnx_cosq_threshold_data_buffers)
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sp_tc_drop_get_unsafe(unit, core, tc, threshold_type, threshold_value, drop_type));  
    }
    else if (threshold_type == soc_dnx_cosq_threshold_packet_descriptors || threshold_type == soc_dnx_cosq_threshold_available_data_buffers)
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sp_tc_drop_get_unsafe(unit, core, tc, threshold_type, threshold_value, drop_type));
    }
  
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers_max)
    {
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sp_tc_drop_get_unsafe(unit, core, tc, soc_dnx_cosq_threshold_data_buffers, threshold_value, drop_type));
    }
    else if (threshold_type == soc_dnx_cosq_threshold_data_buffers_min)
    {
        DNXC_IF_ERR_EXIT(READ_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr(unit, core, reg));
        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MINIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MINIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MINIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MINIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MINIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MINIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MINIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MINIMUMLIMITf);
                    break;
            }
        }
        else /* SP 1*/ 
        {
            switch (tc)
            {
                case 0:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MINIMUMLIMITf);
                    break;
                case 1:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MINIMUMLIMITf);
                    break;
                case 2:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MINIMUMLIMITf);
                    break;
                case 3:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MINIMUMLIMITf);
                    break;
                case 4:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MINIMUMLIMITf);
                    break;
                case 5:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MINIMUMLIMITf);
                    break;
                case 6:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MINIMUMLIMITf);
                    break;
                case 7:
                    *threshold_value = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MINIMUMLIMITf);
                    break;              
            }
        }
    }
    else if (threshold_type == soc_dnx_cosq_thereshold_data_buffers_alpha)
    {
        DNXC_IF_ERR_EXIT(READ_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr(unit, core, reg));

        if (drop_type == soc_dnx_cosq_threshold_global_type_service_pool_0)
        {
            switch (tc)
            {
                case 0:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_0_ALPHAf);
                    break;
                case 1:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_1_ALPHAf);
                    break;
                case 2:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_2_ALPHAf);
                    break;
                case 3:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_3_ALPHAf);
                    break;
                case 4:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_4_ALPHAf);
                    break;
                case 5:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_5_ALPHAf);
                    break;
                case 6:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_6_ALPHAf);
                    break;
                case 7:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_0_TC_7_ALPHAf);
                    break;
            }
        }
        else /* SP 1*/
        {
            switch (tc)
            {
                case 0:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_0_ALPHAf);
                    break;
                case 1:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_1_ALPHAf);
                    break;
                case 2:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_2_ALPHAf);
                    break;
                case 3:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_3_ALPHAf);
                    break;
                case 4:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_4_ALPHAf);
                    break;
                case 5:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_5_ALPHAf);
                    break;
                case 6:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_6_ALPHAf);
                    break;
                case 7:
                    field_val = soc_reg_above_64_field32_get(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_ALPHAr, reg, DB_SERVICE_POOL_1_TC_7_ALPHAf);
                    break;
            }
        }
        DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, threshold_value));
    }

exit:
    DNXC_FUNC_RETURN;
}


uint32 
  soc_jer2_jer_egr_queuing_sch_unsch_drop_set_unsafe(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    threshold_type,
    DNX_SAND_IN    DNX_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  )
{
    soc_reg_above_64_val_t mem;
    uint32 field_32;

    DNXC_INIT_FUNC_DEFS;

    /* Write to jer2_jericho-relevant only fields */
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    field_32 = dev_thresh->thresh_type[threshold_type].mc_shared.descriptors_min;
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_MC_PD_SHARED_MIN_THf, &field_32);

    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, dev_thresh->thresh_type[threshold_type].mc_shared.descriptors_alpha, &field_32));
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_MC_PD_SHARED_ALPHAf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_sch_unsch_drop_set_unsafe(unit, core, threshold_type, dev_thresh));
    
exit:
    DNXC_FUNC_RETURN;
}


/* this function only returns mc shared pds min value and alpha which are unsed by Jericho and above*/
uint32 
  soc_jer2_jer_egr_queuing_sch_unsch_drop_get_unsafe(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    int    core,
    DNX_SAND_IN    int    threshold_type,
    DNX_SAND_OUT   DNX_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  )
{
    soc_reg_above_64_val_t mem;
    uint32 field_32;

    DNXC_INIT_FUNC_DEFS;

    /* Write to jer2_jericho-relevant only fields */
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_MC_PD_SHARED_MIN_THf, &field_32);
    dev_thresh->thresh_type[threshold_type].mc_shared.descriptors_min = field_32;

    soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_MC_PD_SHARED_ALPHAf, &field_32);

    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_32, (int *)&dev_thresh->thresh_type[threshold_type].mc_shared.descriptors_alpha));
    
exit:
    DNXC_FUNC_RETURN;
}

uint32
jer2_jer_egr_threshold_types_verify(
   DNX_SAND_IN    int   unit,
   DNX_SAND_IN    soc_dnx_cosq_threshold_type_t threshold_type)
{
    DNXC_INIT_FUNC_DEFS;

    switch (threshold_type)
    {
        
        case soc_dnx_cosq_threshold_bytes:
        case soc_dnx_cosq_threshold_packet_descriptors:
        case soc_dnx_cosq_threshold_packets:
        case soc_dnx_cosq_threshold_data_buffers:
        case soc_dnx_cosq_threshold_available_packet_descriptors:
        case soc_dnx_cosq_threshold_available_data_buffers:
        case soc_dnx_cosq_threshold_buffer_descriptor_buffers:
        case soc_dnx_cosq_threshold_buffer_descriptors:
        case soc_dnx_cosq_threshold_dbuffs:
        case soc_dnx_cosq_threshold_full_dbuffs:
        case soc_dnx_cosq_threshold_mini_dbuffs:
        case soc_dnx_cosq_threshold_dynamic_weight:
        case soc_dnx_cosq_threshold_packet_descriptors_min:
        case soc_dnx_cosq_threshold_packet_descriptors_max:
        case soc_dnx_cosq_threshold_packet_descriptors_alpha:
        case soc_dnx_cosq_threshold_data_buffers_min:
        case soc_dnx_cosq_threshold_data_buffers_max:
        case soc_dnx_cosq_thereshold_data_buffers_alpha:
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Invalid threshold type %d "), threshold_type));
    }

exit:
    DNXC_FUNC_RETURN;
}

static
  uint32
    soc_jer2_jer_egr_unsched_drop_q_pair_thresh_set_unsafe(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32                egress_tc,
    DNX_SAND_IN  uint32                threshold_type,
    DNX_SAND_IN  uint32                drop_precedence,
    DNX_SAND_IN  JER2_ARAD_EGR_THRESH_INFO     *thresh_info
  )
{
  uint32
    offset;
  soc_reg_above_64_val_t data;
  uint32 field_val;
  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(thresh_info);

  if(thresh_info->packet_descriptors > SOC_DNX_DEFS_GET(unit, egq_qdct_pd_max_val))
  {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("packet descriptors is above max value")));
  }
  /* QDCT offset */
  offset = JER2_ARAD_EGQ_QDCT_TABLE_KEY_ENTRY(threshold_type,egress_tc);
  /* Write PD thresholdS - min and alpha only */
  DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  soc_EGQ_QDCT_TABLEm_field32_set(unit, data, QUEUE_MC_PD_MIN_TH_DP_0f + drop_precedence, thresh_info->packet_descriptors_min);

  DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, thresh_info->packet_descriptors_alpha, &field_val));
  soc_EGQ_QDCT_TABLEm_field32_set(unit, data, QUEUE_MC_PD_ALPHA_DP_0f + drop_precedence, field_val);
  DNXC_IF_ERR_EXIT(WRITE_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
exit:
  DNXC_FUNC_RETURN;
}



uint32
  soc_jer2_jer_egr_unsched_drop_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  int                 profile,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_DROP_THRESH     *thresh,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *exact_thresh
  )
{
    uint32 egress_tc;
    JER2_ARAD_EGR_THRESH_INFO thresh_info;

    DNXC_INIT_FUNC_DEFS;

    egress_tc = prio_ndx;

    /* Use jer2_arad function to write values common to jer2_arad and jer2_jericho */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_unsched_drop_set_unsafe(unit, core, profile ,prio_ndx, dp_ndx, thresh, exact_thresh));

    /* Write jer2_jericho-relevant only values */
   
    jer2_arad_JER2_ARAD_EGR_THRESH_INFO_clear(&thresh_info);
    thresh_info.packet_descriptors_min = thresh->queue_pkts_consumed_min[profile];
    thresh_info.packet_descriptors_alpha = thresh->queue_pkts_consumed_alpha[profile];
    DNXC_IF_ERR_EXIT(soc_jer2_jer_egr_unsched_drop_q_pair_thresh_set_unsafe(
               unit,
               core,
               egress_tc,
               profile,
               dp_ndx,
               &thresh_info
           ));
       

    exact_thresh->queue_pkts_consumed_min[profile] = thresh_info.packet_descriptors_min;
    exact_thresh->queue_pkts_consumed_alpha[profile] = thresh_info.packet_descriptors_alpha;
  

exit:
    DNXC_FUNC_RETURN;
}

uint32
  soc_jer2_jer_egr_sched_port_fc_thresh_set_unsafe(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  int                threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH *thresh
  )
{
    soc_reg_above_64_val_t mem;
    uint32 field_32;
    DNXC_INIT_FUNC_DEFS;

    /* Write to jer2_jericho-relevant only fields  - PDCT */
    DNXC_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    field_32 = thresh->packet_descriptors_min;
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_UC_PD_MIN_FC_THf, &field_32);
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, thresh->packet_descriptors_alpha, &field_32));
    soc_EGQ_PDCT_TABLEm_field_set(unit, mem, PORT_UC_PD_ALPHAf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    /* PQST */
    DNXC_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    field_32 = thresh->data_buffers_min;
    soc_EGQ_PQST_TABLEm_field_set(unit, mem, PORT_UC_DB_MIN_FC_THf, &field_32);
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, thresh->data_buffers_alpha, &field_32));
    soc_EGQ_PQST_TABLEm_field_set(unit, mem, PORT_UC_DB_ALPHAf, &field_32);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), threshold_type, mem));
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_sched_port_fc_thresh_set_unsafe(unit, core, threshold_type, thresh));

exit:
    DNXC_FUNC_RETURN;
}

uint32
  soc_jer2_jer_egr_sched_q_fc_thresh_set_unsafe(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  int       core,
    DNX_SAND_IN  int       prio,
    DNX_SAND_IN  int threshold_type,
    DNX_SAND_IN  DNX_TMC_EGR_FC_OFP_THRESH  *thresh
  )
{
    soc_reg_above_64_val_t mem;
    uint32 offset;
    uint32 field_val;
    JER2_ARAD_EGR_THRESH_INFO thresh_info;
    DNXC_INIT_FUNC_DEFS;

    offset = JER2_ARAD_EGQ_QDCT_TABLE_KEY_ENTRY(threshold_type, prio);

    /* Write PD threshold - min and alpha only */
    DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, mem));
    soc_EGQ_QDCT_TABLEm_field32_set(unit, mem, QUEUE_UC_PD_MIN_FC_THf, thresh->packet_descriptors_min);
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, thresh->packet_descriptors_alpha, &field_val));
    soc_EGQ_QDCT_TABLEm_field32_set(unit, mem, QUEUE_UC_PD_ALPHAf, field_val);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, mem));

    /* Write DB thresholds*/
    DNXC_IF_ERR_EXIT(READ_EGQ_QQST_TABLEm(unit, EGQ_BLOCK(unit, core), offset, mem));
    soc_EGQ_QQST_TABLEm_field32_set(unit, mem, QUEUE_UC_DB_MIN_FC_THf, thresh->data_buffers_min);
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, thresh->data_buffers_alpha, &field_val));
    soc_EGQ_QQST_TABLEm_field32_set(unit, mem, QUEUE_UC_DB_ALPHAf, field_val);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_QQST_TABLEm(unit, EGQ_BLOCK(unit, core), offset, mem));
    /* CALL THE JER2_ARAD FUNCTION */
    thresh_info.dbuff = thresh->data_buffers;/* queue_words_consumed parameter used for Data buffers in Arad */
    thresh_info.packet_descriptors = thresh->packet_descriptors;

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_ofp_fc_q_pair_thresh_set_unsafe(
               unit,
               core,
               prio,
               threshold_type,
               &thresh_info));

exit:
    DNXC_FUNC_RETURN;

}

uint32	
  soc_jer2_jer_egr_queuing_if_fc_uc_set_unsafe(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_IN JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  )
{
    soc_reg_above_64_val_t reg_pd_min, reg_db_min;
    uint32 reg_pd_alpha, reg_db_alpha, field_val1, field_val2;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, core, reg_pd_min));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, core, &reg_pd_alpha));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, core, reg_db_min));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, core, &reg_db_alpha));

    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, info->size256_th_alpha, &field_val1));
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_alpha_to_value(unit, info->pd_th_alpha, &field_val2));
	
    switch(uc_if_profile_ndx)
    {
        case 0:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_0f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_0f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_0f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_0f, field_val1);
            break;
        case 1:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_1f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_1f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_1f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_1f, field_val1);
            break;
        case 2:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_2f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_2f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_2f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_2f, field_val1);
            break;
        case 3:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_3f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_3f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_3f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_3f, field_val1);
            break;
        case 4:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_4f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_4f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_4f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_4f, field_val1);
            break;
        case 5:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_5f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_5f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_5f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_5f, field_val1);
            break;
        case 6:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_6f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_6f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_6f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_6f, field_val1);
            break;
        case 7:
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_7f, info->pd_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, &reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_7f, field_val2);
            soc_reg_above_64_field32_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_7f, info->size256_th_min);
            soc_reg_field_set(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, &reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_7f, field_val1);
            break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("invalid profile")));
    }

    DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, core, reg_pd_min));
    DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, core, reg_pd_alpha));
    DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, core, reg_db_min));
    DNXC_IF_ERR_EXIT(WRITE_CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, core, reg_db_alpha));

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_if_fc_uc_set_unsafe(unit, core, uc_if_profile_ndx, info));

exit:
    DNXC_FUNC_RETURN;
}


int	
  soc_jer2_jer_egr_queuing_if_fc_uc_get_unsafe(
	DNX_SAND_IN	int	unit,
    DNX_SAND_IN int core,
	DNX_SAND_IN	uint32	uc_if_profile_ndx,
	DNX_SAND_OUT JER2_ARAD_EGR_QUEUING_IF_UC_FC	*info
  )
{
    soc_reg_above_64_val_t reg_pd_min, reg_db_min;
    uint32 reg_pd_alpha, reg_db_alpha, field_val1 = 0, field_val2 = 0;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr(unit, core, reg_pd_min));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr(unit, core, &reg_pd_alpha));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr(unit, core, reg_db_min));
    DNXC_IF_ERR_EXIT(READ_CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr(unit, core, &reg_db_alpha));

	DNXC_SAND_IF_ERR_EXIT(jer2_arad_egr_queuing_if_fc_uc_get_unsafe(unit, core, uc_if_profile_ndx, info));
	
    switch(uc_if_profile_ndx)
    {
        case 0:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_0f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_0f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_0f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_0f);
            break;
        case 1:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_1f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_1f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_1f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_1f);
            break;
        case 2:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_2f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_2f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_2f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_2f);
            break;
        case 3:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_3f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_3f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_3f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_3f);
            break;
        case 4:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_4f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_4f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_4f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_4f);
            break;
        case 5:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_5f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_5f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_5f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_5f);
            break;
        case 6:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_6f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_6f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_6f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_6f);
            break;
        case 7:
            info->pd_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_MIN_THr, reg_pd_min, CGM_UC_PD_INTERFACE_FC_MIN_TH_7f);
            field_val2 = soc_reg_field_get(unit, CGM_CGM_UC_PD_INTERFACE_FC_ALPHAr, reg_pd_alpha, CGM_UC_PD_INTERFACE_FC_ALPHA_7f);
            info->size256_th_min = soc_reg_above_64_field32_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_MIN_THr, reg_db_min, CGM_UC_SIZE_256_INTERFACE_FC_MIN_TH_7f);
            field_val1 = soc_reg_field_get(unit, CGM_CGM_UC_SIZE_256_INTERFACE_FC_ALPHAr, reg_db_alpha, CGM_UC_SIZE_256_INTERFACE_FC_ALPHA_7f);
            break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("invalid profile")));
    }

    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val1, (int *)(&info->size256_th_alpha)));
    DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val2, (int *)(&info->pd_th_alpha)));
	
exit:
    DNXC_FUNC_RETURN;
}


int soc_jer2_jer_egr_queuing_init_thresholds(int unit, int port_rate, int nof_priorities, int nof_channels, DNX_TMC_EGR_QUEUING_CGM_INIT_THRESHOLDS* cgm_init_thresholds)
{
    DNXC_INIT_FUNC_DEFS;

    cgm_init_thresholds->mc_reserved_pds  = 40; /* Default value - just in case */

    if(port_rate <= 2500) 
    { /* 1G */
        cgm_init_thresholds->threshold_port = 128;
        cgm_init_thresholds->threshold_queue = 128;
        cgm_init_thresholds->port_mc_drop_pds = 135;
        cgm_init_thresholds->port_mc_drop_dbs = 1350;
        cgm_init_thresholds->mc_reserved_pds = 40;
    } 
    else if(port_rate <= 12500) /* 10G */
    { 
        cgm_init_thresholds->threshold_port = 167;
        cgm_init_thresholds->threshold_queue = 84;
        cgm_init_thresholds->port_mc_drop_pds = 722;
        cgm_init_thresholds->port_mc_drop_dbs = 7220;
        switch(nof_priorities)
        {
            case 1:
                break;
            case 2:
                cgm_init_thresholds->mc_reserved_pds = 160;
                break;
            case 8:
                cgm_init_thresholds->mc_reserved_pds = 40;
                break;
            default:
                break;      
        }
    }
    else if (port_rate <= 30000) /* 25G */
    {
        cgm_init_thresholds->threshold_port = 500;
        cgm_init_thresholds->threshold_queue = 250;
        cgm_init_thresholds->port_mc_drop_pds = 2167;
        cgm_init_thresholds->port_mc_drop_dbs = 21670;
        switch (nof_priorities)
        {
            case 1:
                cgm_init_thresholds->threshold_queue = 500;
                break;
            case 2:
                cgm_init_thresholds->mc_reserved_pds = 400;
                break;
            case 8:
                cgm_init_thresholds->mc_reserved_pds  = 100;
                break;
            default:
                break;
        }
    } 
    else if(port_rate <= 50000) /* 40G/50G */
    { 
        cgm_init_thresholds->threshold_port = 500;
        cgm_init_thresholds->threshold_queue = 250;
        cgm_init_thresholds->port_mc_drop_pds = 2167;
        cgm_init_thresholds->port_mc_drop_dbs = 21670;
        switch(nof_priorities)
        {
            case 1:
                cgm_init_thresholds->threshold_queue = 500;
                break;
            case 2:
                cgm_init_thresholds->mc_reserved_pds = 640;
                break;
            case 8:
                cgm_init_thresholds->mc_reserved_pds = 160;
                break;
            default:
                break;
        }
    } 
    else if(port_rate <= 127000) /* 100G , 120G */
    { 
        cgm_init_thresholds->threshold_port = 2000;
        cgm_init_thresholds->threshold_queue = 1000;
        cgm_init_thresholds->port_mc_drop_pds = 8667;
        cgm_init_thresholds->port_mc_drop_dbs = 86670;
        switch(nof_priorities)
        {
            case 1:
                cgm_init_thresholds->threshold_queue = 2000;
                break;
            case 2:
                cgm_init_thresholds->mc_reserved_pds = 1600;
                break;
            case 8:
                cgm_init_thresholds->mc_reserved_pds = 400;
                break;
            default:
                break;
        }
    } 
    else  /* 300G */
    { 
        cgm_init_thresholds->threshold_port = 3000;
        cgm_init_thresholds->threshold_queue = 1500;
        cgm_init_thresholds->port_mc_drop_pds = 13000;
        cgm_init_thresholds->port_mc_drop_dbs = 130000;
        switch(nof_priorities)
        {
            case 1:
                cgm_init_thresholds->threshold_queue = 3000;
                break;
            case 2:
                cgm_init_thresholds->mc_reserved_pds = 1600;
                break;
            case 8:
                cgm_init_thresholds->mc_reserved_pds = 1200;
                break;
            default:
                break;
        }
    }

    cgm_init_thresholds->mc_reserved_pds = cgm_init_thresholds->mc_reserved_pds / nof_channels;
    cgm_init_thresholds->drop_pds_th = 6000;
    cgm_init_thresholds->drop_dbs_th = 6000;
    cgm_init_thresholds->drop_pds_th_tc = 4000;

    DNXC_FUNC_RETURN;
}


static
  uint32
    soc_jer2_jer_egr_ofp_fc_q_pair_thresh_get_unsafe(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_IN  int                 core,
      DNX_SAND_IN  uint32                 egress_tc,
      DNX_SAND_IN  uint32                 threshold_type,
      DNX_SAND_OUT JER2_ARAD_EGR_THRESH_INFO      *thresh_info
    )
{
  uint32
    offset;
  soc_reg_above_64_val_t data;
  uint32 res, field_val;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);


  DNX_SAND_CHECK_NULL_INPUT(thresh_info);

  SOC_REG_ABOVE_64_CLEAR(data);
  /* QDCT offset */
  offset = JER2_ARAD_EGQ_QDCT_TABLE_KEY_ENTRY(threshold_type,egress_tc);
  
  /* Write PD threshold */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1930, exit, READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->packet_descriptors = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_UC_PD_MAX_FC_THf);
  thresh_info->packet_descriptors_min = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_UC_PD_MIN_FC_THf);
  field_val = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_UC_PD_ALPHAf);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1940, exit, _soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, &(thresh_info->packet_descriptors_alpha)));
  /* READ Dbuff threshold */  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1950, exit, READ_EGQ_QQST_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->dbuff = soc_EGQ_QQST_TABLEm_field32_get(unit, data, QUEUE_UC_DB_MAX_FC_THf);
  thresh_info->dbuff_min = soc_EGQ_QQST_TABLEm_field32_get(unit, data, QUEUE_UC_DB_MIN_FC_THf);
  field_val = soc_EGQ_QQST_TABLEm_field32_get(unit, data, QUEUE_UC_DB_ALPHAf);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1960, exit, _soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, &(thresh_info->dbuff_alpha)));
 
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_egr_ofp_fc_q_pair_thresh_get_unsafe()",egress_tc,threshold_type); 
}


int
  jer2_jer_egr_ofp_fc_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  JER2_ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_FC_OFP_THRESH   *thresh
  )
{
    int res;
    uint32 field_val;
    JER2_ARAD_EGR_THRESH_INFO thresh_info;
    soc_reg_above_64_val_t mem;

    DNXC_INIT_FUNC_DEFS;

    if(prio_ndx == JER2_ARAD_EGR_Q_PRIO_ALL)
    {
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_PDCT_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
        soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_UC_PD_MIN_FC_THf, &thresh->packet_descriptors_min);
        soc_EGQ_PDCT_TABLEm_field_get(unit, mem, PORT_UC_PD_ALPHAf, &field_val);
        DNXC_SAND_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, &(thresh->packet_descriptors_alpha)));

        /* EGQ_PQST_TABLEm */
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_PQST_TABLEm(unit, EGQ_BLOCK(unit, core), ofp_type_ndx, mem));
        soc_EGQ_PQST_TABLEm_field_get(unit, mem, PORT_UC_DB_MIN_FC_THf, &thresh->data_buffers_min);
        soc_EGQ_PQST_TABLEm_field_get(unit, mem, PORT_UC_DB_ALPHAf, &field_val);
        DNXC_SAND_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, &(thresh->data_buffers_alpha)));
    }
    else
    {
        res = soc_jer2_jer_egr_ofp_fc_q_pair_thresh_get_unsafe(unit, core, prio_ndx, ofp_type_ndx, &thresh_info);
        DNXC_SAND_IF_ERR_EXIT(res);
        thresh->words = thresh_info.dbuff;
        thresh->data_buffers_min = thresh_info.dbuff_min;
        thresh->data_buffers_alpha = thresh_info.dbuff_alpha;

        thresh->packet_descriptors = thresh_info.packet_descriptors;
        thresh->packet_descriptors_min = thresh_info.packet_descriptors_min;
        thresh->packet_descriptors_alpha = thresh_info.packet_descriptors_alpha;
    }

    res = jer2_arad_egr_ofp_fc_get_unsafe(unit, core, prio_ndx, ofp_type_ndx, thresh);
    DNXC_SAND_IF_ERR_EXIT(res);


exit:
    DNXC_FUNC_RETURN;
}


static
  uint32
    jer2_jer_egr_unsched_drop_q_pair_thresh_get_unsafe(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32                egress_tc,
    DNX_SAND_IN  uint32                threshold_type,
    DNX_SAND_IN  uint32                drop_precedence,
    DNX_SAND_OUT JER2_ARAD_EGR_THRESH_INFO     *thresh_info
  )
{
  soc_reg_above_64_val_t data;
  uint32 offset, field_val;
  
  DNXC_INIT_FUNC_DEFS;
  
  /* QDCT offset */
  offset = JER2_ARAD_EGQ_QDCT_TABLE_KEY_ENTRY(threshold_type,egress_tc);
  
  /* READ PD threshold */
  DNXC_IF_ERR_EXIT(READ_EGQ_QDCT_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->packet_descriptors = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_MC_PD_MAX_TH_DP_0f + drop_precedence);
  thresh_info->packet_descriptors_min = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_MC_PD_MIN_TH_DP_0f + drop_precedence);
  field_val = soc_EGQ_QDCT_TABLEm_field32_get(unit, data, QUEUE_MC_PD_ALPHA_DP_0f + drop_precedence);
  DNXC_IF_ERR_EXIT(_soc_jer2_jer_convert_field_val_to_alpha(unit, field_val, &(thresh_info->packet_descriptors_alpha)));
  
  /* READ Dbuff threshold */  
  DNXC_IF_ERR_EXIT(READ_EGQ_QQST_TABLEm(unit, EGQ_BLOCK(unit, core), offset, data));
  thresh_info->dbuff = soc_EGQ_QQST_TABLEm_field32_get(unit, data, QUEUE_MC_DB_DP_0_THf + drop_precedence);

exit:
  DNXC_FUNC_RETURN;
}

uint32
  soc_jer2_jer_egr_unsched_drop_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_EGR_Q_PRIO          prio_ndx,
    DNX_SAND_IN  uint32                 dp_ndx,
    DNX_SAND_OUT JER2_ARAD_EGR_DROP_THRESH     *thresh
  )
{

  JER2_ARAD_EGR_THRESH_INFO
    thresh_info;
  uint32
    egress_tc;
  JER2_ARAD_EGR_PORT_THRESH_TYPE
    thresh_type;


  DNXC_INIT_FUNC_DEFS;

  egress_tc = prio_ndx;

  for (thresh_type = JER2_ARAD_EGR_PORT_THRESH_TYPE_0; thresh_type < JER2_ARAD_EGR_PORT_NOF_THRESH_TYPES; thresh_type++)
  {     
     jer2_arad_JER2_ARAD_EGR_THRESH_INFO_clear(&thresh_info);

     DNXC_IF_ERR_EXIT(jer2_jer_egr_unsched_drop_q_pair_thresh_get_unsafe(
             unit,
             core,
             egress_tc,
             thresh_type,
             dp_ndx,
             &thresh_info));

     thresh->queue_words_consumed[thresh_type] = thresh_info.dbuff;/* queue_words_consumed parameter used for Data buffers in Arad */
     thresh->queue_pkts_consumed[thresh_type] = thresh_info.packet_descriptors;
     thresh->queue_pkts_consumed_min[thresh_type] = thresh_info.packet_descriptors_min;
     thresh->queue_pkts_consumed_alpha[thresh_type] = thresh_info.packet_descriptors_alpha;
  }

exit:
  DNXC_FUNC_RETURN;
}

uint32 soc_jer2_jer_sp_tc_init(int unit, int core, JER2_ARAD_EGR_QUEUING_DEV_TH    *info)
{
    soc_reg_above_64_val_t reg;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr(unit, core, reg));
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_0_MINIMUMLIMITf, info->pool_tc[0][0].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_1_MINIMUMLIMITf, info->pool_tc[0][1].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_2_MINIMUMLIMITf, info->pool_tc[0][2].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_3_MINIMUMLIMITf, info->pool_tc[0][3].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_4_MINIMUMLIMITf, info->pool_tc[0][4].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_5_MINIMUMLIMITf, info->pool_tc[0][5].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_6_MINIMUMLIMITf, info->pool_tc[0][6].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_0_TC_7_MINIMUMLIMITf, info->pool_tc[0][7].mc.buffers);

    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_0_MINIMUMLIMITf, info->pool_tc[1][0].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_1_MINIMUMLIMITf, info->pool_tc[1][1].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_2_MINIMUMLIMITf, info->pool_tc[1][2].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_3_MINIMUMLIMITf, info->pool_tc[1][3].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_4_MINIMUMLIMITf, info->pool_tc[1][4].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_5_MINIMUMLIMITf, info->pool_tc[1][5].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_6_MINIMUMLIMITf, info->pool_tc[1][6].mc.buffers);
    soc_reg_above_64_field32_set(unit, CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr, reg, DB_SERVICE_POOL_1_TC_7_MINIMUMLIMITf, info->pool_tc[1][7].mc.buffers);
    DNXC_IF_ERR_EXIT(WRITE_CGM_CONGESTION_MANAGEMENT_DB_SERVICE_POOL_MINIMUM_THRESHOLDSr(unit, core, reg));

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

