/*
 * $Id: port_sw_db.c,v 1.43 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC PORT SW DB
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#define SW_DB_PRINT_COLUMN_LENGTH (24)
#define SW_DB_PRINT_MAX_COLUMNS_IN_LINE (6)


#include <soc/dcmn/error.h>
#include <soc/types.h>
#include <soc/defs.h>
#include <soc/cm.h>
#include <soc/portmode.h>
#include <soc/drv.h>

#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/mbcm.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/NIF/ports_manager.h>
#include <soc/dpp/ARAD/arad_egr_queuing.h> /* Doron TBD remove*/
#include <soc/dpp/TMC/tmc_api_link_bonding.h>
#include <soc/dpp/QAX/qax_link_bonding.h>
#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/dcmn/dcmn_crash_recovery.h>
#endif

/*****************************/
/****    Containers       ****/
/*****************************/
soc_port_unit_info_t        ports_unit_info[SOC_MAX_NUM_DEVICES];
soc_port_core_info_t        core_info[SOC_MAX_NUM_DEVICES][SOC_DPP_DEFS_MAX(NOF_CORES)];

/*****************************/
/****       Macros        ****/
/*****************************/
#define PORT_UNIT_INFO  (ports_unit_info[unit])
#define PORT_CORE_INFO(_port_core)  (core_info[unit][_port_core])

#define PORT_INFO_INDEX(interface, phy) ((interface == SOC_PORT_IF_QSGMII) && ((phy-1)%4 == 0) ? 1 : 0)

/*****************************/
/**** Static  functions   ****/
/*****************************/
STATIC soc_error_t soc_port_sw_db_master_channel_set(int unit, soc_port_t port, soc_port_t master_port);
STATIC soc_error_t soc_port_sw_db_phy_ports_set(int unit, soc_port_t port, soc_pbmp_t phy_ports);
STATIC soc_error_t port_sw_db_packet_tdm_check(int unit, SOC_TMC_PORT_HEADER_TYPE header_type);

/*****************************/
/****  SW DB  functions   ****/
/*****************************/

soc_error_t
soc_port_sw_db_init(int unit) 
{
    soc_port_t port;
    int blk, bindex, i;
    int first_phy_port;
    soc_error_t rv;
    soc_phy_port_sw_db_t phy_port;
    soc_logical_port_sw_db_t logical_port;
#ifdef BCM_LB_SUPPORT
    soc_modem_t modem_i;
    uint64 val_64 = COMPILER_64_INIT(0, 0);
#endif
    SOCDNX_INIT_FUNC_DEFS;

    if(!SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(arad_sw_db_sw_state_alloc(unit));

#ifdef BCM_LB_SUPPORT
        if (SOC_IS_QAX(unit)) {
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.qax.tm.lb_info.lb_egr_if_bmp_used.alloc_bitmap(unit, SOC_TMC_LB_NOF_LBG*2));
            for (modem_i = 0; modem_i < SOC_TMC_LB_NOF_MODEM; modem_i++) {
                SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.qax.tm.lb_info.lb_ing_modem_fragment_cnt.set(unit, modem_i, val_64));
                SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.qax.tm.lb_info.lb_ing_modem_byte_cnt.set(unit, modem_i, val_64));
            }
        }
#endif
    }

    sal_memset(&phy_port, 0x0, sizeof(phy_port));
    sal_memset(&logical_port, 0x0, sizeof(logical_port));
    for(port = 0; port < SOC_MAX_NUM_PORTS; ++port) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.set(unit, port, &logical_port);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.set(unit, port, &phy_port);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    SOC_PBMP_CLEAR(PORT_UNIT_INFO.quads_out_of_reset);
    SOC_PBMP_CLEAR(PORT_UNIT_INFO.all_phy_pbmp);

    for (port = 0 ;; port++) {
        blk = SOC_PORT_IDX_INFO(unit ,port, 0).blk;
        bindex = SOC_PORT_IDX_INFO(unit ,port, 0).bindex;
        if (blk < 0 && bindex < 0) {            /* end of list */
            break;
        }
        if (blk < 0) {                          /* empty slot */
            continue;
        }
        SOC_PBMP_PORT_ADD(PORT_UNIT_INFO.all_phy_pbmp, port);

    }

    /* Init all ports */
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        SOC_INFO(unit).port_p2l_mapping[port] = -1;
    }
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        SOC_INFO(unit).port_l2p_mapping[port] = -1;
        SOC_INFO(unit).port_speed_max[port] = -1;
        SOC_INFO(unit).port_num_lanes[port] = 1;
        SOC_INFO(unit).port_type[port] = SOC_BLK_NONE;
    }

    SOC_INFO(unit).rcy_port_count = 0;
    SOC_INFO(unit).rcy_port_start = -1;

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.set(unit, port, ARAD_EGR_INVALID_BASE_Q_PAIR);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.set(unit, port, -1);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.set(unit, port, -1);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.set(unit, port, INVALID_EGR_INTERFACE);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.protocol_offset.set(unit, port, 0);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.high_pirority_cal.set(unit, first_phy_port, INVALID_CALENDAR);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.low_pirority_cal.set(unit, first_phy_port, INVALID_CALENDAR);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_single_cal_mode.set(unit, first_phy_port, FALSE);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.set(unit, port, SOC_TMC_PORT_HEADER_TYPE_NONE);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_in.set(unit, port, SOC_TMC_PORT_HEADER_TYPE_NONE);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    /* We are not going to change macros to avoid such cases */
    /* coverity[same_on_both_sides] */
    for(i=0 ; i<SOC_DPP_DEFS_MAX(NOF_CORES) ; i++) {
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            core_info[unit][i].tm_to_local_port_map[port] = SOC_MAX_NUM_PORTS;
            core_info[unit][i].pp_to_local_port_map[port] = SOC_MAX_NUM_PORTS;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_destroy(int unit) 
{
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(PORT_UNIT_INFO.quads_out_of_reset);
    SOC_PBMP_CLEAR(PORT_UNIT_INFO.all_phy_pbmp);

    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_pm_interface_properties_set
 * Purpose:
 *      Per interface properties add
 * Parameters:
 *      unit                - (IN) Unit number.
 *      port                - (IN) Port Number;
 *      all_phy_pbmp        - (IN) ALL phy ports of the device (include such that aren't in use)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_port_sw_db_interface_properties_set(int unit, soc_port_t port) 
{
    char *bname;
    uint32 lane;
    soc_error_t rv;
    soc_port_if_t interface_type;
    int first_phy_port;
    int nof_lanes_per_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    int pml1_base_lane = SOC_DPP_DEFS_GET(unit, pml1_base_lane); 
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, &interface_type);
    SOCDNX_IF_ERR_EXIT(rv);
    switch (interface_type) {
        case SOC_PORT_IF_SGMII:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).ge.num;
            PORT_SW_DB_PORT_ADD(ge, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "ge";
            break;
        case SOC_PORT_IF_DNX_XAUI:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).xe.num;
            PORT_SW_DB_PORT_ADD(xe, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 10000;
            bname = "xe";
            break;
        case SOC_PORT_IF_XFI:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).xe.num;
            PORT_SW_DB_PORT_ADD(xe, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 10000;
            if(SOC_IS_JERICHO(unit) && !SOC_IS_QUX(unit)){
                SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy_port, &lane)));
                if(lane <= nof_lanes_per_nbi || (SOC_IS_JERICHO_PLUS_ONLY(unit) && lane > pml1_base_lane)){
                    SOC_INFO(unit).port_speed_max[port] = 25000;
                }
            }
            bname = "xe";
            break;
        case SOC_PORT_IF_XLAUI:
        case SOC_PORT_IF_XLAUI2:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).xl.num;
            PORT_SW_DB_PORT_ADD(xl, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = (IS_HG_PORT(unit, port)) ? 42000 : 40000;
            if(SOC_IS_JERICHO(unit)){
                if(SOC_INFO(unit).port_l2p_mapping[port] <= 24){
                    SOC_INFO(unit).port_speed_max[port] = 50000;
                }
            }
            bname = "xl";
            break;
        case SOC_PORT_IF_RXAUI:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).xe.num;
            PORT_SW_DB_PORT_ADD(xe, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 10000;
            bname = "xe";
            break;
        case SOC_PORT_IF_ILKN:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).il.num;
            PORT_SW_DB_PORT_ADD(il, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 12500;
            bname = "il";
            break;
        case SOC_PORT_IF_CAUI:
        case SOC_PORT_IF_CAUI4:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).ce.num;
            PORT_SW_DB_PORT_ADD(ce, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            if (SOC_INFO(unit).port_num_lanes[port] == 12) {
                SOC_INFO(unit).port_speed_max[port] = (IS_HG_PORT(unit, port)) ? 127000 : 120000;
            } else {
                SOC_INFO(unit).port_speed_max[port] = (IS_HG_PORT(unit, port)) ? 106000 : 100000;
            }
            bname = "ce";
            break;
        case SOC_PORT_IF_QSGMII:
            SOC_INFO(unit).port_offset[port] = SOC_INFO(unit).qsgmii.num;
            PORT_SW_DB_PORT_ADD(qsgmii, port);
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "qsgmii";
            break;
        case SOC_PORT_IF_CPU:
            SOC_PBMP_PORT_ADD((SOC_INFO(unit).cmic_bitmap), port);
            PORT_SW_DB_PORT_ADD(all, port);
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "cpu";
            break;
        case SOC_PORT_IF_TM_INTERNAL_PKT:
            PORT_SW_DB_PORT_ADD(port, port);
            PORT_SW_DB_PORT_ADD(all, port);
            /*Un-mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).all.disabled_bitmap, port);
            SOC_INFO(unit).port_speed_max[port] = 1200000;
            bname = "tm_internal_pkt";
            break;
        case SOC_PORT_IF_RCY:
            PORT_SW_DB_PORT_ADD(rcy, port);
            PORT_SW_DB_PORT_ADD(all, port);
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "rcy";
            SOC_INFO(unit).rcy_port_count++;
            SOC_INFO(unit).rcy_port_start = (SOC_INFO(unit).rcy_port_start < port && SOC_INFO(unit).rcy_port_start >= 0 ? SOC_INFO(unit).rcy_port_start : port);
            break;
        case SOC_PORT_IF_OLP:
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "olp";
            break;
        case SOC_PORT_IF_OAMP:
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "oamp";
            break;
        case SOC_PORT_IF_ERP:
            SOC_INFO(unit).port_speed_max[port] = 1000;
            bname = "erp";
            break;
        case SOC_PORT_IF_SAT:
            SOC_INFO(unit).port_speed_max[port] = 40000;
            bname = "sat";
            PORT_SW_DB_PORT_ADD(all, port);
            break;
        case SOC_PORT_IF_IPSEC:
            SOC_INFO(unit).port_speed_max[port] = 20000;
            bname = "ipsec";
            PORT_SW_DB_PORT_ADD(all, port);
            break;
        case SOC_PORT_IF_NOCXN:
            SOC_INFO(unit).port_speed_max[port] = 0;
            bname = "nocxn";
            break;
        case SOC_PORT_IF_LBG:
            PORT_SW_DB_PORT_ADD(lbgport, port);
            PORT_SW_DB_PORT_ADD(all, port);
            SOC_INFO(unit).port_speed_max[port] = 10000;
            bname = "lbg";
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Interface %d isn't supported"), interface_type));
            break;
    }

    sal_snprintf(SOC_INFO(unit).port_name[port], sizeof(SOC_INFO(unit).port_name[port]), "%s%d", bname, port);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_pm_port_add_validate
 * Purpose:
 *      Validate port before adding
 * Parameters:
 *      unit                - (IN) Unit number.
 *      port                - (IN) Port Number.
 *      channel             - (IN) Channel number.
 *      interface           - (IN) Interface Type.
 *      phy_ports           - (IN) Phy ports in use by this port.
 * Returns:
 *      SOC_E_xxx
 */

STATIC soc_error_t
soc_port_sw_db_port_validate(int unit, soc_port_t port, uint32 channel, soc_port_if_t interface, soc_pbmp_t phy_ports) 
{
    uint32 port_i, is_valid, num_of_lanes, channel_o;
    soc_pbmp_t ports_bm, phy_ports_o, and_bm;
    soc_port_if_t interface_o;
    SOCDNX_INIT_FUNC_DEFS;

    /*check port not already defined*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("Port %d already exists"),port));
    }

    /*1. check interface type is supported 
      2. check num_of_lanes according to interface type*/
    SOC_PBMP_COUNT(phy_ports, num_of_lanes);
    switch (interface) {
        case SOC_PORT_IF_XFI:
        case SOC_PORT_IF_QSGMII:
        case SOC_PORT_IF_SGMII:
        case SOC_PORT_IF_CPU:
            if (1 != num_of_lanes) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be 1"), port));
            }
            break;
        case SOC_PORT_IF_RXAUI:
        case SOC_PORT_IF_XLAUI2:
            if (2 != num_of_lanes) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be 2"), port));
            }
            break;
        case SOC_PORT_IF_DNX_XAUI:
        case SOC_PORT_IF_XLAUI:
            if (4 != num_of_lanes) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be 4"), port));
            }
            break;
        case SOC_PORT_IF_CAUI:
            if (SOC_IS_JERICHO(unit)) {
                if(4 != num_of_lanes) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be 4"), port));
                }
            } else {
                if (10 != num_of_lanes && 12 != num_of_lanes) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be 10 or 12"), port));
                }
            }
            break;
        case SOC_PORT_IF_ILKN:
            if (1 > num_of_lanes) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d number of lanes must be at least 1"), port));
            }
            break;
        case SOC_PORT_IF_RCY:
        case SOC_PORT_IF_ERP:
        case SOC_PORT_IF_OLP:
        case SOC_PORT_IF_OAMP:
        case SOC_PORT_IF_SAT:
        case SOC_PORT_IF_IPSEC:
        case SOC_PORT_IF_TM_INTERNAL_PKT:
        case SOC_PORT_IF_LBG:
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Interface %d isn't supported"), interface));
            break;
    }

    /*validate other interface on the same phy lanes*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, 0, &ports_bm));
    SOC_PBMP_ITER(ports_bm, port_i) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port_i, &interface_o));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port_i, &phy_ports_o));
        SOC_PBMP_ASSIGN(and_bm, phy_ports_o);
        SOC_PBMP_AND(and_bm, phy_ports);

        /*if two ports has overlap phy ports*/
        if (SOC_PBMP_NOT_NULL(and_bm)) {
            if (interface == interface_o) {
                /*if same interface physical lanes must have full agreement or no overlap at all*/
                if ((interface != SOC_PORT_IF_CPU) && SOC_PBMP_NEQ(phy_ports, phy_ports_o)) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d and port %d has overlap phy ports"),port,port_i));
                } else {
                    /*same interface over same lanes - make sure channel is OK*/
                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_channel_get(unit, port_i, &channel_o));
                    if (channel == channel_o) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d and port %d has the same channel"),port,port_i));
                    }
                }
            } else {
                if (interface == SOC_PORT_IF_LBG) {
                    /* 
                     * Do nothing
                     *   1) It's OK that LBG port overlap phy ports of non-LBG port.
                     *   2) It's not allowed that non-LBG port overlap phy ports of LBG port
                     * 
                     *  LBG port occupy an entry in IRE_PORT_TO_BASE_ADDRESS_MAPm to connect LBG port to IRE. 
                     *  For non-LBG traffic, one phy port has one entry in IRE_PORT_TO_BASE_ADDRESS_MAPm.
                     */                    
                }
                else {
                    /*if not same interface overlap isn't allowed*/
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d and port %d has same interface overlap"),port,port_i));
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_port_validate_and_add(int unit, int core, soc_port_t port,  uint32 channel, uint32 flags, soc_port_if_t interface, soc_pbmp_t phy_ports) 
{
    SOCDNX_INIT_FUNC_DEFS;

    /*validate port info*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_validate(unit, port, channel, interface, phy_ports));

    /*add port to DB*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_add(unit,  core, port,channel, flags, interface, phy_ports));

exit:
    SOCDNX_FUNC_RETURN;
}


STATIC soc_error_t
soc_port_sw_db_bmp_remove(int unit, soc_ptype_t *bmp, soc_port_t port) 
{
    soc_pbmp_t temp;
    soc_port_t port_i;
    SOCDNX_INIT_FUNC_DEFS;

    /*save temp bitmap*/
    SOC_PBMP_ASSIGN(temp, bmp->bitmap);
    SOC_PBMP_PORT_REMOVE(temp, port);

    /*clear structure*/
    bmp->max = -1;
    bmp->min = -1;
    for (port_i = 0; port_i < bmp->num; port_i++) {
        bmp->port[port_i] = 0;
    }
    bmp->num = 0;
    SOC_PBMP_CLEAR(bmp->bitmap);

    /*rebuild structure*/
    SOC_PBMP_ITER(temp, port_i) {
        bmp->port[bmp->num] = port_i;
        bmp->num++;
        if ((bmp->min < 0) || (bmp->min > port_i)) {
            bmp->min = port_i;
        }
        if (port_i > bmp->max) {
            bmp->max = port_i;
        }
        SOC_PBMP_PORT_ADD(bmp->bitmap, port_i);
    }

    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_pm_interface_properties_remove
 * Purpose:
 *      Per interface properties remove
 * Parameters:
 *      unit                - (IN) Unit number.
 *      port                - (IN) Port Number;
 *      all_phy_pbmp        - (IN) ALL phy ports of the device (include such that aren't in use)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_port_sw_db_interface_properties_remove(int unit, soc_port_t port) 
{
    char *bname;
    soc_error_t rv;
    soc_port_if_t interface_type;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, &interface_type);
    SOCDNX_IF_ERR_EXIT(rv);
    switch(interface_type) {   
        case SOC_PORT_IF_SGMII:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).ge), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;
        case SOC_PORT_IF_DNX_XAUI:
        case SOC_PORT_IF_XFI:
        case SOC_PORT_IF_RXAUI:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).xe), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;   
        case SOC_PORT_IF_XLAUI:
        case SOC_PORT_IF_XLAUI2:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).xl), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
        break;
        case SOC_PORT_IF_ILKN:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).il), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;
        case SOC_PORT_IF_CAUI:
        case SOC_PORT_IF_CAUI4:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).ce), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;
        case SOC_PORT_IF_QSGMII:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).qsgmii), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;
        case SOC_PORT_IF_CPU:
            SOC_PBMP_PORT_REMOVE((SOC_INFO(unit).cmic_bitmap), port);  
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port)); 
            break;
        case SOC_PORT_IF_TM_INTERNAL_PKT:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).port), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            /*Mark the port as disabled port - dynamic ports feature*/
            SOC_PBMP_PORT_ADD(SOC_INFO(unit).all.disabled_bitmap, port);
            break;
        case SOC_PORT_IF_RCY:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).rcy), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            break;
        case SOC_PORT_IF_LBG:
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).lbgport), port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).all), port));
            break;
        case SOC_PORT_IF_SAT:
            
        case SOC_PORT_IF_OLP:
        case SOC_PORT_IF_OAMP:
        case SOC_PORT_IF_ERP:
        case SOC_PORT_IF_NOCXN:
        case SOC_PORT_IF_IPSEC:
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Interface %d isn't supported"), interface_type));
            break;
    }

    SOC_INFO(unit).port_speed_max[port] = -1;

    bname = "?";
    sal_snprintf(SOC_INFO(unit).port_name[port], sizeof(SOC_INFO(unit).port_name[port]), "%s%d", bname, port);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_next_master_get(int unit, soc_port_t port, soc_port_t *next_master) 
{
    soc_port_t port_i;
    int valid;
    int port_phy;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    (*next_master) = -1;

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; port_i++) {
        /*search for port mapped to same interface*/
        if (port_i != port) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port_i, &valid);
            SOCDNX_IF_ERR_EXIT(rv);
            if(valid) {
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port_i, &first_phy_port);
                SOCDNX_IF_ERR_EXIT(rv);
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &port_phy);
                SOCDNX_IF_ERR_EXIT(rv);

                if(first_phy_port == port_phy) {
                    (*next_master) = port_i;
                    break;
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_port_remove(int unit, soc_port_t port) 
{
    soc_port_t master_port, new_master, port_i;
    uint32 is_master_channel, is_valid;
    uint8 is_virtual;
    int blk;
    SOC_TMC_PORT_HEADER_TYPE header_type_in;
    int first_phy_port;
    soc_error_t rv;
    soc_port_if_t interface_type;
    int port_core;
    uint32 channels_count;
    uint32 tm_port;
    uint32 pp_port;
    soc_port_t new_port_for_blk = 0;
    int found_new_port;
    
    SOCDNX_INIT_FUNC_DEFS;

    
#ifdef CRASH_RECOVERY_SUPPORT
    soc_dcmn_cr_suppress(unit, dcmn_cr_no_support_port_core_info);
#endif /* CRASH_RECOVERY_SUPPORT */

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_in.get(unit, port, &header_type_in);
    SOCDNX_IF_ERR_EXIT(rv);
    if (header_type_in == BCM_SWITCH_PORT_HEADER_TYPE_ETH) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).ether), port));
    } else if (header_type_in == BCM_SWITCH_PORT_HEADER_TYPE_STACKING) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).st), port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.pbmp_is_null(unit, first_phy_port, &is_virtual));
    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.master_port.get(unit, first_phy_port, &master_port);
    SOCDNX_IF_ERR_EXIT(rv);
    is_master_channel = (master_port == port ? 1 : 0);

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.get(unit, port, &port_core);
    SOCDNX_IF_ERR_EXIT(rv);
    
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.get(unit, port, &tm_port);
    SOCDNX_IF_ERR_EXIT(rv);

    if (tm_port != _SOC_DPP_PORT_INVALID) {
        PORT_CORE_INFO(port_core).tm_to_local_port_map[tm_port] = SOC_MAX_NUM_PORTS;
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.get(unit, port, &pp_port);
    SOCDNX_IF_ERR_EXIT(rv);
    if (pp_port != _SOC_DPP_PORT_INVALID) {
        PORT_CORE_INFO(port_core).pp_to_local_port_map[pp_port] = SOC_MAX_NUM_PORTS;
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.set(unit, port, ARAD_EGR_INVALID_BASE_Q_PAIR);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.set(unit, port, _SOC_DPP_PORT_INVALID);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.set(unit, port, _SOC_DPP_PORT_INVALID);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.set(unit, port, INVALID_EGR_INTERFACE);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.protocol_offset.set(unit, port, 0);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.set(unit, port, 0);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.priority_mode.set(unit, port, 0);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.set(unit, port, SOC_TMC_PORT_HEADER_TYPE_NONE);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_in.set(unit, port, SOC_TMC_PORT_HEADER_TYPE_NONE);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.set(unit, port, 0);
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_set(unit, port, 0));

    /*decrease number of ports*/
    SOC_INFO(unit).port_num--;

    /*specific interface properties*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_properties_remove(unit, port));

    /*need new master*/
    if (is_master_channel) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_next_master_get(unit, port, &new_master)); 

        for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; port_i++) {
            if (SOC_INFO(unit).port_p2l_mapping[port_i] == port) {
                SOC_INFO(unit).port_p2l_mapping[port_i] = new_master;
            }
        }
    } else {
        new_master = master_port;
    }

    /*clear logical-to-physical*/
    SOC_INFO(unit).port_l2p_mapping[port] = -1;

    /*clear to block bitmap*/
    if (!is_virtual) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, &interface_type);
        SOCDNX_IF_ERR_EXIT(rv);

        if (interface_type == SOC_PORT_IF_CPU) {
            blk = SOC_PORT_IDX_INFO(unit, 0, PORT_INFO_INDEX(interface_type, 0)).blk;
        } else {
            blk = SOC_PORT_IDX_INFO(unit, first_phy_port, PORT_INFO_INDEX(interface_type, first_phy_port)).blk;
        }
        SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).block_bitmap[blk], port);

        /*remove port block type*/
        SOC_INFO(unit).port_type[port] = SOC_BLK_NONE;

        /* if block_port point to our block in to find another port which is attached to this block*/
        if (SOC_INFO(unit).block_port[blk] == port) {
            found_new_port = 0;
            SOC_PBMP_ITER(SOC_INFO(unit).block_bitmap[blk], new_port_for_blk) {
                SOC_INFO(unit).block_port[blk] = new_port_for_blk;
                found_new_port = 1;
                break;
            }

            if (!found_new_port) {
                SOC_INFO(unit).block_port[blk]  = -1;
            }
        }

        /*remove CPU info*/
        if (SOC_PORT_IF_CPU == interface_type && is_master_channel) {
            SOC_INFO(unit).cmic_port = (new_master >= 0 ? new_master : 0);
            SOC_INFO(unit).cmic_block = (new_master >= 0 ? SOC_INFO(unit).cmic_block : -1);
        }
    } else {
        /*if virtual the phy_port isn't real and should be removed from all_phy bitmap*/
        SOC_PBMP_PORT_REMOVE(PORT_UNIT_INFO.all_phy_pbmp, first_phy_port);
    }

    /*num of lanes*/
    SOC_INFO(unit).port_num_lanes[port] = 1;

    /*if only channel added to interface*/
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.get(unit, first_phy_port, &channels_count));
    if (1 == channels_count) {
        soc_pbmp_t pbmp_clear;
        /*init interface properties*/
        SOC_PBMP_CLEAR(pbmp_clear);
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_set(unit, port, pbmp_clear));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_set(unit, port, SOC_PORT_IF_NOCXN));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_set(unit, port, 0));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_initialized_set(unit, port, 0));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_runt_pad_set(unit, port, 0));

        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.high_pirority_cal.set(unit, first_phy_port, 0);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.low_pirority_cal.set(unit, first_phy_port, 0);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_single_cal_mode.set(unit, first_phy_port, FALSE);
        SOCDNX_IF_ERR_EXIT(rv);
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_master_channel_set(unit, port, 0));

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_set(unit, port, 0));
    } else {
        if (is_master_channel) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_master_channel_set(unit, port, new_master));
        }
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.set(unit, port, 0);
    SOCDNX_IF_ERR_EXIT(rv);
    --channels_count;
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.set(unit, first_phy_port, channels_count));
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.set(unit, port, 0));
    
exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_runt_pad_set(int unit, soc_port_t port, uint32 runt_pad) 
{
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.runt_pad.set(unit, first_phy_port, runt_pad);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_runt_pad_get(int unit, soc_port_t port, uint32 *runt_pad) 
{
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.runt_pad.get(unit, first_phy_port, runt_pad);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_first_phy_port_get(int unit, soc_port_t port, uint32 *phy_port /*one based*/) 
{
    uint32 found = 0, is_valid;
    soc_pbmp_t phy_ports;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.get(unit, first_phy_port, &phy_ports));
    SOC_PBMP_ITER(phy_ports, *phy_port) {
        found = 1;
        break;
    }

    if (!found) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Port %d has no physical lanes"),port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_core_get(int unit, soc_port_t port, int *core  /*zero based*/) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.get(unit, port, core);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_in_block_index_get(int unit, soc_port_t port, uint32 *in_block_index /*zero based*/) 
{
    uint32 phy_port, is_valid;
    soc_port_if_t interface;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface));

    if (interface == SOC_PORT_IF_CPU) {
        *in_block_index = SOC_PORT_IDX_INFO(unit, 0, PORT_INFO_INDEX(interface, 0)).bindex;
    } else {
        *in_block_index = SOC_PORT_IDX_INFO(unit, phy_port, PORT_INFO_INDEX(interface, phy_port)).bindex;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_num_lanes_get(int unit, soc_port_t port, uint32 *count) 
{
    uint32 is_valid;
    int count_lcl;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.pbmp_count(unit, first_phy_port, &count_lcl);
    SOCDNX_IF_ERR_EXIT(rv);
    *count = count_lcl;

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_interface_type_get(int unit, soc_port_t port, soc_port_if_t *interface_type) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, interface_type);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Check if the given port is a "virtual recycle port" which is really a dynamically allocated reassembly context.
 * returns TRUE or FALSE. On any error, just returns FALSE.
 */
int
soc_port_sw_db_interface_is_virt_rcy_port(int unit, soc_port_t port, int *is_virt_rcy_port) 
{
    uint32 is_valid;
    soc_error_t rv;
    soc_port_if_t interface_type;
    int flags;
    int first_phy_port;
    
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, &interface_type);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = soc_port_sw_db_is_valid_port_get(unit, port, &is_valid);
    if ((rv == SOC_E_NONE) && is_valid && (interface_type == _SHR_PORT_IF_RCY)) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
        SOCDNX_IF_ERR_EXIT(rv);
        *is_virt_rcy_port =  (flags & SOC_PORT_FLAGS_VIRTUAL_RCY_INTERFACE) ? TRUE : FALSE;
    } else {
        *is_virt_rcy_port = FALSE;
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_interface_type_set(int unit, soc_port_t port, soc_port_if_t interface_type) 
{
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    if (port < 0 || port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.set(unit, first_phy_port, interface_type);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_speed_set(int unit, soc_port_t port, int speed) 
{
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.speed.set(unit, first_phy_port, speed);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_speed_get(int unit, soc_port_t port, int *speed) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if(!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.speed.get(unit, first_phy_port, speed);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_flag_add(int unit, soc_port_t port, uint32 flag) 
{
    uint32 is_valid;
    int flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;


    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
    SOCDNX_IF_ERR_EXIT(rv);
    flags |= flag;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.set(unit, port, flags);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_flag_remove(int unit, soc_port_t port, uint32 flag) 
{
    uint32 is_valid;
    int flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
    SOCDNX_IF_ERR_EXIT(rv);
    flags &= ~flag;
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.set(unit, port, flags);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_flags_get(int unit, soc_port_t port, uint32 *flags) 
{
    uint32 is_valid;
    int lcl_flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &lcl_flags);
    SOCDNX_IF_ERR_EXIT(rv);
    *flags = lcl_flags;

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_encap_mode_set(int unit, soc_port_t port, soc_encap_mode_t encap_mode) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.encap_mode.set(unit, port, encap_mode);
    SOCDNX_IF_ERR_EXIT(rv);

    /*we save this data in the hg in soc contol also - legacy*/
    if (encap_mode == SOC_ENCAP_HIGIG2) {
        PORT_SW_DB_PORT_ADD(hg, port);
        SOC_PBMP_PORT_ADD(SOC_HG2_PBM(unit), port);
    } else {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_bmp_remove(unit, &(SOC_INFO(unit).hg), port));
        SOC_PBMP_PORT_REMOVE(SOC_HG2_PBM(unit), port);
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_encap_mode_get(int unit, soc_port_t port, soc_encap_mode_t *encap_mode) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.encap_mode.get(unit, port, encap_mode);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_is_hg_set(int unit, soc_port_t port, uint32 is_hg) 
{
    uint32 is_valid;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_encap_mode_set(unit, port, (is_hg) ? SOC_ENCAP_HIGIG2 : SOC_ENCAP_IEEE));

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_is_hg_get(int unit, soc_port_t port, uint32 *is_hg) 
{
    uint32 is_valid;
    soc_encap_mode_t encap_mode;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_encap_mode_get(unit, port, &encap_mode));
    *is_hg = (encap_mode == SOC_ENCAP_HIGIG2);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_initialized_set(int unit, soc_port_t port, uint32 is_initialized) 
{
    uint32 is_valid;
    soc_error_t rv;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid && is_initialized) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.initialized.set(unit, first_phy_port, is_initialized);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_is_initialized_get(int unit, soc_port_t port, uint32 *is_initialized) 
{
    soc_error_t rv;
    uint32 is_valid;
    int is_initialized_lcl;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.initialized.get(unit, first_phy_port, &is_initialized_lcl);
    *is_initialized = is_initialized_lcl ? 1 : 0;
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_is_single_cal_mode_set(int unit, soc_port_t port, int is_single_mode) 
{
    uint32      is_valid;
    soc_error_t rv;
    int first_phy_port;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_single_cal_mode.set(unit, first_phy_port, is_single_mode);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_is_single_cal_mode_get(int unit, soc_port_t port, int *is_single_mode) 
{
    soc_error_t rv;
    uint32 is_valid;
    int first_phy_port;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_single_cal_mode.get(unit, first_phy_port, is_single_mode);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_high_priority_cal_set(int unit, soc_port_t port, uint32 cal_id) 
{
    soc_error_t rv;
    int first_phy_port;

    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.high_pirority_cal.set(unit, first_phy_port, cal_id);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_high_priority_cal_get(int unit, soc_port_t port, uint32 *cal_id) 
{
    soc_error_t rv;
    uint32 is_valid;
    int first_phy_port;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.high_pirority_cal.get(unit, first_phy_port, cal_id);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_low_priority_cal_set(int unit, soc_port_t port, uint32 cal_id) 
{
    int first_phy_port;
    soc_error_t rv;

    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.low_pirority_cal.set(unit, first_phy_port, cal_id);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_low_priority_cal_get(int unit, soc_port_t port, uint32 *cal_id) 
{
    soc_error_t rv;
    uint32 is_valid;
    int first_phy_port;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.low_pirority_cal.get(unit, first_phy_port, cal_id);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_serdes_quads_out_of_reset_set(int unit, soc_pbmp_t quads) 
{
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_ASSIGN(PORT_UNIT_INFO.quads_out_of_reset, quads);

    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_serdes_quads_out_of_reset_get(int unit, soc_pbmp_t *quads) 
{

    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_ASSIGN(*quads, PORT_UNIT_INFO.quads_out_of_reset);

    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_master_channel_get(int unit, soc_port_t port, soc_port_t *master_port) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.master_port.get(unit, first_phy_port, master_port);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t 
soc_port_sw_db_is_master_get(int unit, soc_port_t port, uint32* is_master)
{
    soc_port_t master_port;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_master_channel_get(unit, port, &master_port));
    *is_master = (port == master_port);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t 
soc_port_sw_db_interface_rate_get(int unit, soc_port_t port, uint32* rate)
{
    uint32 num_of_lanes;
    int speed;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_get(unit, port, &speed));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    
    if(SOC_PORT_IF_ILKN == interface_type) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_of_lanes));
        *rate = num_of_lanes*speed;
    } else {
        *rate = speed;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_error_t
port_sw_db_packet_tdm_check(int unit, SOC_TMC_PORT_HEADER_TYPE header_type)
{

    soc_error_t rv;
    soc_dpp_config_tdm_t *tdm = &(SOC_DPP_CONFIG(unit)->tdm);

    SOCDNX_INIT_FUNC_DEFS;

    rv = (!tdm->is_packet && header_type == SOC_TMC_PORT_HEADER_TYPE_TDM_RAW) ? SOC_E_CONFIG : SOC_E_NONE ;
    if (rv==SOC_E_CONFIG) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG,(_BSL_SOCDNX_MSG("Trying to configure port for packet tdm while packet tdm disabled for all device")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_error_t
soc_port_sw_db_master_channel_set(int unit, soc_port_t port, soc_port_t master_port) 
{
    soc_error_t rv;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.master_port.set(unit, first_phy_port, master_port);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_is_valid_port_get(int unit, soc_port_t port, uint32 *is_valid) 
{
    int valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    if (port < 0 || port >= SOC_MAX_NUM_PORTS) {
        *is_valid = 0;
    } else {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port, &valid);
        SOCDNX_IF_ERR_EXIT(rv);
        *is_valid = valid;
    }

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_is_valid_port_set(int unit, soc_port_t port, uint32 valid) 
{
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    if (port < 0 || port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    } else {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.set(unit, port, valid);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_phy_ports_get(int unit, soc_port_t port, soc_pbmp_t *phy_ports) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.get(unit, first_phy_port, phy_ports);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_error_t
soc_port_sw_db_phy_ports_set(int unit, soc_port_t port, soc_pbmp_t phy_ports) 
{
    soc_error_t rv;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.set(unit, first_phy_port, phy_ports);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_valid_ports_get(int unit, uint32 required_flag, soc_pbmp_t *ports_bm) 
{
    soc_port_t port;
    uint32 is_valid;
    int flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*ports_bm);

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid)) ;
        if (is_valid) {
            /*if no required flags - pass all*/
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
            SOCDNX_IF_ERR_EXIT(rv);
            if (!required_flag || (flags & required_flag)) {
                SOC_PBMP_PORT_ADD(*ports_bm, port);
            }
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_valid_ports_core_get(int unit, int core, uint32 required_flag, soc_pbmp_t *ports_bm) 
{
    soc_port_t port;
    uint32 is_valid;
    int port_core;
    int flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*ports_bm);

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid)) ;
        if (is_valid) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &port_core)) ;
            if (port_core == core) {
                /*if no required flags - pass all*/
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
                SOCDNX_IF_ERR_EXIT(rv);
                if (!required_flag || (flags & required_flag)) {
                    SOC_PBMP_PORT_ADD(*ports_bm, port);
                }
            }
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*ports bitmap get - include invalid ports*/
soc_error_t
soc_port_sw_db_ports_get(int unit, uint32 required_flag, soc_pbmp_t *ports_bm) 
{
    soc_port_t port;
    int flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*ports_bm);

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        /*if no required flags - pass all*/
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, port, &flags);
        SOCDNX_IF_ERR_EXIT(rv);
        if (!required_flag || (flags & required_flag)) {
            SOC_PBMP_PORT_ADD(*ports_bm, port);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_protocol_offset_set(int unit, soc_port_t port, uint32 flags, uint32 offset) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.protocol_offset.set(unit, port, offset);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_protocol_offset_get(int unit, soc_port_t port, uint32 flags, uint32 *offset) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.protocol_offset.get(unit, port, offset);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_num_of_channels_get(int unit, soc_port_t port, uint32 *num_of_channels) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.get(unit, first_phy_port, num_of_channels);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_max_channel_num_get(int unit, soc_port_t port, uint32 *max_channel) 
{
    soc_port_t port_i;
    uint32 is_valid;
    int channel;
    int valid;
    int first_phy_port;
    int cur_first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.get(unit, port, &channel);
    SOCDNX_IF_ERR_EXIT(rv);
    *max_channel = channel;

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; port_i++) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port_i, &valid);
        SOCDNX_IF_ERR_EXIT(rv);
        if (valid) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port_i, &cur_first_phy_port);
            SOCDNX_IF_ERR_EXIT(rv);
            if (first_phy_port == cur_first_phy_port) {
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.get(unit, port_i, &channel);
                SOCDNX_IF_ERR_EXIT(rv);
                if (*max_channel < channel) {
                    *max_channel = channel;
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_ports_to_same_interface_get(int unit, soc_port_t port, soc_pbmp_t *ports) 
{
    soc_port_t port_i;
    uint32 is_valid;
    int valid;
    int first_phy_port;
    int cur_first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    SOC_PBMP_CLEAR(*ports);

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; port_i++) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port_i, &valid);
        SOCDNX_IF_ERR_EXIT(rv);
        if (valid) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port_i, &cur_first_phy_port);
            SOCDNX_IF_ERR_EXIT(rv);
            if (first_phy_port == cur_first_phy_port) {
                SOC_PBMP_PORT_ADD(*ports, port_i);
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_port_from_interface_type_get(int unit, soc_port_if_t interface_type, int first_phy_port, soc_port_t *port_to_return) 
{
    soc_port_t port;
    soc_port_if_t port_phy_interface_type;
    int valid;
    int port_phy;
    soc_error_t rv;

    SOCDNX_INIT_FUNC_DEFS;

    *port_to_return = SOC_MAX_NUM_PORTS;

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port, &valid);
        SOCDNX_IF_ERR_EXIT(rv);
        if (valid) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &port_phy);
            SOCDNX_IF_ERR_EXIT(rv);
            if (first_phy_port == port_phy) {
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, port_phy, &port_phy_interface_type);
                SOCDNX_IF_ERR_EXIT(rv);
                if(interface_type == port_phy_interface_type) {
                    *port_to_return = port;
                    break;
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_port_with_channel_get(int unit, soc_port_t port, uint32 channel, soc_port_t *port_match_channel) 
{
    soc_pbmp_t same_interface_ports;
    soc_port_t port_i;
    uint32 is_valid, channel_i = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    is_valid = 0;
    *port_match_channel = SOC_MAX_NUM_PORTS;
    SOC_PBMP_CLEAR(same_interface_ports);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &same_interface_ports));

    SOC_PBMP_ITER(same_interface_ports, port_i) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_channel_get(unit, port_i, &channel_i));
        if (channel_i == channel) {
            *port_match_channel = port_i;
            break;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_port_ptc_get(int unit, soc_port_t port, soc_port_t *ptc)
{
    soc_pbmp_t same_interface_ports;
    soc_port_t port_i;
    uint32 is_valid, channel_i = 0;
    int core;
    uint32 tm_port;
    
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    is_valid = 0;
    *ptc = SOC_MAX_NUM_PORTS;
    SOC_PBMP_CLEAR(same_interface_ports);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &same_interface_ports));

    SOC_PBMP_ITER(same_interface_ports, port_i) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_channel_get(unit, port_i, &channel_i));
        if (channel_i == 0) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core));
            *ptc = tm_port;
            break;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}



soc_error_t
soc_port_sw_db_channel_get(int unit, soc_port_t port, uint32 *channel) 
{
    uint32 is_valid;
    int channel_lcl;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.get(unit, port, &channel_lcl);
    SOCDNX_IF_ERR_EXIT(rv);
    
    *channel = channel_lcl;

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_latch_down_get(int unit, soc_port_t port, int *latch_down) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Port %d is invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.latch_down.get(unit, first_phy_port, latch_down);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_latch_down_set(int unit, soc_port_t port, int latch_down) 
{
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.latch_down.set(unit, first_phy_port, latch_down);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_local_to_tm_port_set(int unit, soc_port_t port, uint32 tm_port) 
{
    uint32 is_valid;
    int port_core;
    uint32 port_info_tm_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    
#ifdef CRASH_RECOVERY_SUPPORT
    soc_dcmn_cr_suppress(unit, dcmn_cr_no_support_port_core_info);
#endif /* CRASH_RECOVERY_SUPPORT */

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.get(unit, port, &port_core);
    SOCDNX_IF_ERR_EXIT(rv);
    if ((tm_port != _SOC_DPP_PORT_INVALID) && (PORT_CORE_INFO(port_core).tm_to_local_port_map[tm_port] != SOC_MAX_NUM_PORTS)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("Port %d: tm_port(%d) already in use"),port, tm_port));
    }

    if((tm_port != _SOC_DPP_PORT_INVALID)) {
        PORT_CORE_INFO(port_core).tm_to_local_port_map[tm_port] = port;
    } else {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.get(unit, port, &port_info_tm_port);
        SOCDNX_IF_ERR_EXIT(rv);
        PORT_CORE_INFO(port_core).tm_to_local_port_map[port_info_tm_port] = SOC_MAX_NUM_PORTS;
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.set(unit, port, tm_port);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_port_sw_db_local_to_pp_port_set(int unit, soc_port_t port, uint32 pp_port) 
{
    uint32 is_valid;
    int port_core;
    uint32 port_info_pp_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.get(unit, port, &port_core);
    SOCDNX_IF_ERR_EXIT(rv);

    
    if ((pp_port != _SOC_DPP_PORT_INVALID) && (pp_port > SOC_DPP_CONFIG(unit)->pdm_extension.max_pp_port) && (SOC_DPP_CONFIG(unit)->arad->init.dram.pdm_mode == SOC_TMC_INIT_PDM_MODE_REDUCED)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("pp_port(%d) is out of maximum PP-port %d"),pp_port, SOC_DPP_CONFIG(unit)->pdm_extension.max_pp_port));
    }
    if ((pp_port != _SOC_DPP_PORT_INVALID) && (PORT_CORE_INFO(port_core).pp_to_local_port_map[pp_port] != SOC_MAX_NUM_PORTS)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("pp_port(%d) already in use"),pp_port));
    }

    if (pp_port != _SOC_DPP_PORT_INVALID) {
        PORT_CORE_INFO(port_core).pp_to_local_port_map[pp_port] = port;
    } else {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.get(unit, port, &port_info_pp_port);
        SOCDNX_IF_ERR_EXIT(rv);
        PORT_CORE_INFO(port_core).pp_to_local_port_map[port_info_pp_port] = SOC_MAX_NUM_PORTS;
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.set(unit, port, pp_port);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_local_to_tm_port_get(int unit, soc_port_t port, uint32* tm_port, int* core) 
{
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;
    /*
     * Note that soc_port_sw_db_core_get() also verifies 'port' is valid
     */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, core)) ;
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.get(unit, port, tm_port);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_local_to_pp_port_get(int unit, soc_port_t port, uint32* pp_port, int* core)
{
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;
    /*
     * Note that soc_port_sw_db_core_get() also verifies 'port' is valid.
     */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, core)) ;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_port.get(unit, port, pp_port);
    SOCDNX_IF_ERR_EXIT(rv);
    if (*pp_port == _SOC_DPP_PORT_INVALID) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("PP Port for port %d is invalid"),port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_is_channelized_port_set(int unit, soc_port_t port, int is_channelized) 
{
    soc_error_t rv;
    int first_phy_port;
    SOCDNX_INIT_FUNC_DEFS;

    if (port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid port %d"), port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_channelized.set(unit, first_phy_port, is_channelized);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_is_channelized_port_get(int unit, soc_port_t port, int *is_channelized) 
{
    uint32 is_valid;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d invalid"),port));
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.is_channelized.get(unit, first_phy_port, is_channelized);
    SOCDNX_IF_ERR_EXIT(rv); 

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_tm_to_local_port_get(int unit, int core, uint32 tm_port, soc_port_t *port) {

    SOCDNX_INIT_FUNC_DEFS;

    if(core == MEM_BLOCK_ALL || core == SOC_CORE_ALL) {
        core = 0;
    }
    if(core < 0 || core >= SOC_DPP_DEFS_GET(unit, nof_cores)){
        return SOC_E_PARAM;
    }

    if (tm_port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(_SHR_E_PORT, (_BSL_SOCDNX_MSG("Invalid tm_port %d"), tm_port));
    }

    *port = core_info[unit][core].tm_to_local_port_map[tm_port];

    if(*port == SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("tm_port %d invalid mapping"), tm_port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_pp_to_local_port_get(int unit, int core, uint32 pp_port, soc_port_t *port) 
{
    SOCDNX_INIT_FUNC_DEFS;

    if(core == MEM_BLOCK_ALL || core == SOC_CORE_ALL) {
        core = 0;
    }
    if(core < 0 || core >= SOC_DPP_DEFS_GET(unit, nof_cores)){
        return SOC_E_PARAM;
    }

    if (pp_port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid pp_port %d"), pp_port));
    }

    *port = core_info[unit][core].pp_to_local_port_map[pp_port];

    if(*port == SOC_MAX_NUM_PORTS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("pp_port %d invalid mapping"), pp_port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_pp_is_valid_get(int unit, int core, uint32 pp_port,  uint32 *is_valid)
{
    soc_port_t port;
    SOCDNX_INIT_FUNC_DEFS;

    if(core == MEM_BLOCK_ALL || core == SOC_CORE_ALL) {
        core = 0;
    }
    if(core < 0 || core >= SOC_DPP_DEFS_GET(unit, nof_cores)){
        return SOC_E_PARAM;
    }

    if (pp_port >= SOC_MAX_NUM_PORTS) {
        (*is_valid) = 0;
        SOC_EXIT;
    }

    port = core_info[unit][core].pp_to_local_port_map[pp_port];

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, is_valid));

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_local_to_out_port_priority_set(int unit, soc_port_t port, uint32 nof_priorities) 
{
    uint32 is_valid;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.priority_mode.set(unit, port, nof_priorities);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_port_sw_db_local_to_out_port_priority_get(int unit, soc_port_t port, uint32* nof_priorities) 
{
    uint32 is_valid;
    int priority_mode;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d invalid"),port));
    }

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.priority_mode.get(unit, port, &priority_mode);
    SOCDNX_IF_ERR_EXIT(rv);
    *nof_priorities = priority_mode;

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_port_sw_db_tm_port_to_out_port_priority_get(int unit, int core, uint32 tm_port, uint32* nof_priorities)
{
    soc_port_t port;
    int priority_mode;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.priority_mode.get(unit, port, &priority_mode);
    SOCDNX_IF_ERR_EXIT(rv);
    *nof_priorities = priority_mode;

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_port_sw_db_pp_port_to_out_port_priority_get(int unit, int core, uint32 pp_port, uint32* nof_priorities)
{
    soc_port_t port;
    int priority_mode;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_pp_to_local_port_get(unit, core, pp_port, &port));

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.priority_mode.get(unit, port, &priority_mode);
    SOCDNX_IF_ERR_EXIT(rv);
    *nof_priorities = priority_mode;

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_pp_port_to_base_q_pair_get(int unit,int core, uint32 pp_port,  uint32* base_q_pair)
{
    soc_port_t port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_pp_to_local_port_get(unit, core, pp_port, &port));

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.get(unit, port, base_q_pair);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_tm_port_to_base_q_pair_get(int unit, int core, uint32 tm_port, uint32* base_q_pair)
{
    soc_port_t port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.get(unit, port, base_q_pair);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*Header info*/
soc_error_t
soc_port_sw_db_hdr_type_out_set(int unit, soc_port_t port, SOC_TMC_PORT_HEADER_TYPE header_type_out)
{
    soc_error_t rv;
    soc_port_if_t interface;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(port_sw_db_packet_tdm_check(unit, header_type_out));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface));

    /* update port bitmaps */
    if(SOC_PORT_IF_OLP != interface && SOC_PORT_IF_OAMP != interface) {
        /* TDM port*/
        if ((header_type_out == SOC_TMC_PORT_HEADER_TYPE_TDM)
            || (header_type_out == SOC_TMC_PORT_HEADER_TYPE_TDM_RAW)) {
          PORT_SW_DB_PORT_ADD(tdm, port);
        }
    }
    /* update db */
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.set(unit,
                                   port, header_type_out);
    SOCDNX_IF_ERR_EXIT(rv);
exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_hdr_type_in_set(int unit, soc_port_t port, SOC_TMC_PORT_HEADER_TYPE header_type_in)
{
    soc_error_t rv;
    soc_port_if_t interface;
    uint32 flags;

 
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(port_sw_db_packet_tdm_check(unit, header_type_in));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));

    /* update port bitmaps */
    if(SOC_PORT_IF_OLP != interface && SOC_PORT_IF_OAMP != interface && !(SOC_PORT_IS_ELK_INTERFACE(flags))) {
        if ((header_type_in == SOC_TMC_PORT_HEADER_TYPE_ETH) 
            || (header_type_in == SOC_TMC_PORT_HEADER_TYPE_MPLS_RAW)
            || (header_type_in == SOC_TMC_PORT_HEADER_TYPE_INJECTED_PP)
            || (header_type_in == SOC_TMC_PORT_HEADER_TYPE_INJECTED_2_PP)
            || (header_type_in == SOC_TMC_PORT_HEADER_TYPE_XGS_MAC_EXT)){
          PORT_SW_DB_PORT_ADD(ether, port);
        }  
        if (header_type_in == SOC_TMC_PORT_HEADER_TYPE_STACKING) {
          PORT_SW_DB_PORT_ADD(st, port);
        }
    }
   
    /* update db */   
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_in.set(unit,
                                   port, header_type_in);
    SOCDNX_IF_ERR_EXIT(rv);
exit:
    SOCDNX_FUNC_RETURN;
}

/*PP port info*/
soc_error_t
soc_port_sw_db_pp_port_flags_add(int unit, soc_port_t port, uint32 flag)
{
    uint32 pp_flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_flags.get(unit, port, &pp_flags);
    SOCDNX_IF_ERR_EXIT(rv);

    pp_flags |= flag;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_flags.set(unit, port, pp_flags);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t 
soc_port_sw_db_pp_port_flags_rmv(int unit, soc_port_t port, uint32 flag)
{
    uint32 pp_flags;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_flags.get(unit, port, &pp_flags);
    SOCDNX_IF_ERR_EXIT(rv);

    pp_flags &= ~flag;

    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_flags.set(unit, port, pp_flags);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_free_tm_ports_get(int unit, int core, soc_pbmp_t* tm_ports)
{
    soc_port_t port;
    soc_pbmp_t ports_bm;
    uint32 tm_port;
    soc_error_t rv;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*tm_ports);
    SOC_PBMP_PORTS_RANGE_ADD(*tm_ports, 0 , SOC_DPP_DEFS_GET(unit, nof_logical_ports));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_core_get(unit, core, 0, &ports_bm));

    /* remove all tm_ports used by valid ports */
    SOC_PBMP_ITER(ports_bm, port) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.tm_port.get(unit, port, &tm_port);
        /*Some ports don't have to have a TM port and can be used only with local ports, for example KBP ports*/
        if (tm_port == 0xFFFFFFFF) {
            continue;
        }
        SOCDNX_IF_ERR_EXIT(rv);
        SOC_PBMP_PORT_REMOVE(*tm_ports, tm_port);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*****************************/
/****    SW DB Print      ****/
/*****************************/
typedef struct port_sw_db_print_line_s {
    int unit;
    int columns_in_line;
    int port_columns[SW_DB_PRINT_MAX_COLUMNS_IN_LINE];
    int ports[SW_DB_PRINT_MAX_COLUMNS_IN_LINE];
    int num_of_ports;
}port_sw_db_print_line_t;

typedef struct soc_phy_to_logical_ports_str {
    char ports[SOC_MAX_NUM_PORTS][SW_DB_PRINT_COLUMN_LENGTH];
} soc_phy_to_logical_ports_str_t;

/* print port_sw_db funcs*/

STATIC soc_error_t
soc_port_sw_db_create_centralized_string(int unit, char* destination, char* source, int window_size)
{
    int len, i;
    SOCDNX_INIT_FUNC_DEFS;
    
    sal_strncat(destination, "|", 1);
    len = sal_strlen(source);
    len = window_size - len;
    for (i = 0; i < (len/2); i++) {
        sal_strncat(destination, " ", 1);
    }
    sal_strncat(destination, source, len);
    for (i = 0; i < (len/2); i++) {
        sal_strncat(destination, " ", 1);
    }
    if ((len%2)) {
        sal_strncat(destination, " ", 1);
    }

    SOCDNX_FUNC_RETURN    
}




/**
 * get str of the ranges of physical ports whos first physical 
 * port is first_phy_port 
 * @param unit 
 * @param first_phy_port
 * @param ports_str - the output e.g: "1-12 24-28" (1 base port numbers)
 * 
 */
STATIC soc_error_t
_soc_port_sw_db_ports_str_get(int unit, int first_phy_port, char *ports_str)
{
    int port_i;
    int range_begin = -1;
    int last_added = -1;
    char str[SW_DB_PRINT_COLUMN_LENGTH];
    soc_pbmp_t phy_ports;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.phy_ports.get(unit, first_phy_port, &phy_ports));
    PBMP_ITER(phy_ports, port_i){
        /*first time handle*/
        if(last_added == -1){
            last_added = port_i;
            range_begin = port_i;
            sal_sprintf(ports_str, "%d", last_added);
            continue;
        }
        /*continue of range*/
        if((last_added == port_i - 1)){
            last_added = port_i;
            continue;
        }
        /*avoid overflow*/
        if(sal_strlen(ports_str) > 100){
            return SOC_E_FAIL;
        }
        /*the range that ended was not 1 item range*/
        if(last_added != range_begin){
            sal_sprintf(str, " - %d", last_added);
            sal_strncat(ports_str, str, sal_strlen(str));
        }
        range_begin = port_i;
        last_added = port_i;
        sal_sprintf(str, ", %d", last_added);
        sal_strncat(ports_str, str, sal_strlen(str));
    }
    /*close the last range*/
    if(last_added != range_begin){
        sal_sprintf(str, " - %d", last_added);
        sal_strncat(ports_str, str, sal_strlen(str));
    }
    /*handle no ports in list e.g: OLP ERP*/
    if(last_added == -1){
        sal_sprintf(str, "%d", first_phy_port);
        sal_sprintf(ports_str, str);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/**
 * get the flags of the logical port of the master channel that 
 * its first phyical port is  first_phy_port and 
 * 
 * @param unit 
 * @param first_phy_port 
 * @param str 
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t
_soc_port_sw_db_flags_str_get(int unit, int first_phy_port, char *str)
{
    int port_flags;
    soc_port_t master_port;
    soc_error_t rv;

    SOCDNX_INIT_FUNC_DEFS;

    str[0] = 0;
    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.master_port.get(unit, first_phy_port, &master_port);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.get(unit, master_port, &port_flags);
    SOCDNX_IF_ERR_EXIT(rv);
    if (port_flags & SOC_PORT_FLAGS_STAT_INTERFACE) {
        sal_strncat(str, "STAT", 149);
    }
    if (port_flags & SOC_PORT_FLAGS_NETWORK_INTERFACE) {
        sal_strncat(str, " NIF", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_PON_INTERFACE) {
        sal_strncat(str, " PON", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_VIRTUAL_RCY_INTERFACE) {
        sal_strncat(str, " VIRT RCY", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_ELK) {
        sal_strncat(str, " ELK", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_XGS_MAC_EXT) {
        sal_strncat(str, " XGS MAC EXT", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_FIBER) {
        sal_strncat(str, " FIBER", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_SCRAMBLER) {
        sal_strncat(str, " SCRAMBLER", 149 - sal_strlen(str));
    }
    if (port_flags & SOC_PORT_FLAGS_LB_MODEM) {
        sal_strncat(str, " MODEM", 149 - sal_strlen(str));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/**
 * get the first col of the port_db table
 * 
 * @param line_number - the line number in the table
 * @param str - the output
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t
_soc_port_sw_db_first_col_str_get(int line_number, char *str)
{
    switch(line_number){
    case 0:
    case 2:  /*fall through*/
    case 5:  /*fall through*/
    case 11: /*fall through*/
        sal_strncpy(str, "                      ", 150);
        break;
    case 1:
        sal_strncpy(str, "       Physical Port: ", 150);
        break;
    case 3:
        sal_strncpy(str, "           Interface: ", 150);
        break;
    case 4:
        sal_strncpy(str, "               Speed: ", 150);
        break;
    case 6:
        sal_strncpy(str, "LogicalPort, Channel: ", 150);
        break;
    case 7:
        sal_strncpy(str, "         Initialized: ", 150);
        break;
    case 8:
        sal_strncpy(str, "               Flags: ", 150);
        break;
    case 9:
        sal_strncpy(str, "          Latch Down: ", 150);
        break;
    case 10:
        sal_strncpy(str, "                Core: ", 150);
        break;
    }
    return SOC_E_NONE;
}


STATIC soc_error_t
_soc_port_sw_db_logical_ports_str_get(
   int unit,
   int port,
   int entry_id,
   soc_phy_to_logical_ports_str_t *logical_ports,
   int logical_port_count[SOC_MAX_NUM_PORTS],
   char *str)
{
    if (entry_id >= logical_port_count[port]){
       sal_strncpy(str, " ", 150);
    }
    else{
        sal_strncpy(str, logical_ports[port].ports[entry_id], 150);
    }
    return SOC_E_NONE;
}

/**
 * print a line of the port_db table
 * 
 * @param unit 
 * @param table_line - struct with information about the ports 
 *                     to print in the current line of the table
 * @param logical_ports - physical to logical ports 
 *                      mapping(array that in the
 *                      logical_ports[phy_port] -  is array of
 *                      strings: "logical_port , channel"
 * @param logical_port_count  - logical_port_count[phy_port] = 
 *                            the number of strings of
 *                            logical_ports for phy_port.
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t
_soc_port_sw_db_line_print(
   int unit,
   port_sw_db_print_line_t *table_line,
   soc_phy_to_logical_ports_str_t *logical_ports,
    int logical_port_count[SOC_MAX_NUM_PORTS])
{
    char* interface_mode[] = SOC_PORT_IF_NAMES_INITIALIZER;
    int max_logical_ports = 0;
    int is_init;
    char line[200];
    char str[200];
    char temp_str[10];
    int i,j,k, logical_ports_count, logical_port_core, phy_port_core;
    int port;
    soc_port_if_t interface_type;
    int speed;
    int latch_down;
    soc_error_t rv;

    SOCDNX_INIT_FUNC_DEFS;
    for(i = 0 ; i < table_line->num_of_ports; i ++){
        if(logical_port_count[table_line->ports[i]] > max_logical_ports){
            max_logical_ports = logical_port_count[table_line->ports[i]];
        }
    }
    logical_ports_count = 0;
    for(j = 0; j < 12; ){
        _soc_port_sw_db_first_col_str_get(j, line);
        if(logical_ports_count != 0){
            sal_strncpy(line, "                      ", sizeof(line));
        }
        for(i = 0 ; i < table_line->num_of_ports; i ++){
            switch(j){
            case 0:
            case 11: /* fall through*/
                break;
            case 1:
                rv = _soc_port_sw_db_ports_str_get(unit, table_line->ports[i], str);
                SOCDNX_IF_ERR_EXIT(rv);
                break;
            case 2:
            case 5: /*fall through*/
                sal_strncpy(str, " ", sizeof(str));
                break;
            case 3:
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, table_line->ports[i], &interface_type);
                SOCDNX_IF_ERR_EXIT(rv);
                sal_strncpy(str, interface_mode[interface_type], sizeof(str));
                break;
            case 4:
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.speed.get(unit, table_line->ports[i], &speed);
                SOCDNX_IF_ERR_EXIT(rv);
                sal_itoa(str, speed, 10, 0, 0);
                break;
            case 6:
                _soc_port_sw_db_logical_ports_str_get(unit, table_line->ports[i], logical_ports_count, logical_ports, logical_port_count, str);
                break;
            case 7:
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.initialized.get(unit, table_line->ports[i], &is_init);
                SOCDNX_IF_ERR_EXIT(rv);
                if (!is_init){
                    sal_strncpy(str, "Uninitialized", sizeof(str));
                } 
                break;
            case 8:
                _soc_port_sw_db_flags_str_get(unit, table_line->ports[i], str);
                break;
            case 9:
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.latch_down.get(unit, table_line->ports[i], &latch_down);
                SOCDNX_IF_ERR_EXIT(rv);
                if (latch_down) {
                    sal_strncpy(str, "LatchWasDown", sizeof(str));
                }
                break;
            case 10:
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.core.get(unit, table_line->ports[i], &phy_port_core);
                SOCDNX_IF_ERR_EXIT(rv);
                sal_itoa(str, phy_port_core, 10, 0, 0);

                if (SOC_IS_JERICHO(unit)) {
                    /* For CPU logical ports, each logical port may have its own core which
                       does not have to be equal to the CPU 'physical port' */
                    rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, table_line->ports[i], &interface_type);
                    SOCDNX_IF_ERR_EXIT(rv);
                    if (SOC_PORT_IF_CPU == interface_type) {
                        for (port = 0; port < SOC_MAX_NUM_PORTS; ++port) {
                            uint32 is_valid;
                            soc_port_if_t if_type;
                            rv = soc_port_sw_db_is_valid_port_get(unit, port, &is_valid);
                            SOCDNX_IF_ERR_EXIT(rv);
                            if (is_valid) {
                                rv = soc_port_sw_db_interface_type_get(unit, port, &if_type);
                                SOCDNX_IF_ERR_EXIT(rv);
                                if (SOC_PORT_IF_CPU == if_type) {
                                    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.get(unit, port, &logical_port_core);
                                    SOCDNX_IF_ERR_EXIT(rv);
                                    if (logical_port_core != phy_port_core) {
                                        sal_sprintf(temp_str, ", %d", logical_port_core);
                                        sal_strncat(str, temp_str, sizeof(str) - sal_strlen(str) - sal_strlen(temp_str) - 1);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                break;
            }
            if(j == 0 || (j == 11) ){
                for(k = 0 ; k < table_line->port_columns[i]; k++){
                    sal_strncat(line, "-------------------------", sizeof(line) - sal_strlen(line) - 1);
                }
            }
            else{
                rv = soc_port_sw_db_create_centralized_string(unit, line, str, ((SW_DB_PRINT_COLUMN_LENGTH+2)*table_line->port_columns[i]-(table_line->port_columns[i]+1)));
                SOCDNX_IF_ERR_EXIT(rv);
            }
            str[0] = 0;            
        }
        if(j == 0 || (j == 11) ){
            LOG_CLI((BSL_META_U(unit,
                                "%s-\n"), line));
        }
        else{
            LOG_CLI((BSL_META_U(unit,
                                "%s|\n"), line));
        }
        if (j == 6){
            logical_ports_count++;
            if(logical_ports_count >= max_logical_ports){
                j++;
                logical_ports_count = 0;
            }
        }
        else{
            j++;
        }
        line[0] = 0;
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n\n")));
exit:
    SOCDNX_FUNC_RETURN;
}

/**
 * add port to the table line structure 
 *  
 * @param table_line 
 * @param port 
 * @param nof_columns 
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t
_soc_port_sw_db_line_port_add(port_sw_db_print_line_t *table_line, int port, int nof_columns){
    table_line->ports[table_line->num_of_ports] = port;
    table_line->port_columns[table_line->num_of_ports] = nof_columns;
    table_line->columns_in_line += nof_columns;
    table_line->num_of_ports++;
    return SOC_E_NONE;
}


/**
 * add port to the table. if the the add of the port cause 
 * overflow (exceeds from  SW_DB_PRINT_MAX_COLUMNS_IN_LINE) we 
 * print the current line and add port to a new line 
 * 
 * @param unit 
 * @param port - first physical port
 * @param table_line - struct with information about the ports 
 *                     to print in the current line of the table
 * @param logical_ports - physical to logical ports 
 *                      mapping(array that in the
 *                      logical_ports[phy_port] -  is array of
 *                      strings: "logical_port , channel"
 * @param logical_port_count  - logical_port_count[phy_port] = 
 *                            the number of strings of
 *                            logical_ports for phy_port.
 * 
 * @return STATIC soc_error_t 
 */
STATIC soc_error_t
_soc_port_sw_db_table_entry_add(int unit, int port, port_sw_db_print_line_t *table_line,
    soc_phy_to_logical_ports_str_t *logical_ports,
    int logical_port_count[SOC_MAX_NUM_PORTS])
{
    int nof_columns;
    SOCDNX_INIT_FUNC_DEFS;

     nof_columns = 1;

    if(table_line->columns_in_line + nof_columns > SW_DB_PRINT_MAX_COLUMNS_IN_LINE){
        SOCDNX_IF_ERR_EXIT(_soc_port_sw_db_line_print(unit, table_line, logical_ports, logical_port_count ));
        sal_memset(table_line , 0 , sizeof(table_line[0]));
    }
    _soc_port_sw_db_line_port_add(table_line, port, nof_columns);
exit:
    SOCDNX_FUNC_RETURN;
}



soc_error_t 
soc_port_sw_db_print(int unit, uint32 flags)
{
    int port;
    int  i, j; 
    char port_str[SW_DB_PRINT_COLUMN_LENGTH*2];
    char *logical_ports_str[SOC_MAX_NUM_PORTS];
    soc_phy_to_logical_ports_str_t *logical_ports = NULL;
    int logical_port_count[SOC_MAX_NUM_PORTS];
    port_sw_db_print_line_t line_struct;
    int is_init;
    int channel;
    int first_phy_port;
    soc_error_t rv;
    SOCDNX_INIT_FUNC_DEFS;
    
       
    sal_memset(&line_struct, 0 , sizeof(line_struct));
    /* prepare logical ports and channels*/
    for (i = 0; i < SOC_MAX_NUM_PORTS; i++) {
        logical_port_count[i] = 0;
        logical_ports_str[i] = sal_alloc((SW_DB_PRINT_COLUMN_LENGTH+2)*SOC_MAX_NUM_PORTS, "soc_port_sw_db_print.logical_ports_str");
        if(!logical_ports_str[i]){
            LOG_CLI((BSL_META_U(unit,
                                "Memory allocation failure\n")));
            for(j = 0; j < i ; ++j) {
                sal_free(logical_ports_str[j]);
            }
            return SOC_E_MEMORY;
        }
        logical_ports_str[i][0] = 0;
        if (i > 0) {
            sal_strncat(logical_ports_str[i],"                      ", (SW_DB_PRINT_COLUMN_LENGTH+2)*SOC_MAX_NUM_PORTS - sal_strlen(logical_ports_str[i]) - 1);
        }
    }

    logical_ports = sal_alloc(sizeof(soc_phy_to_logical_ports_str_t) * SOC_MAX_NUM_PORTS, "soc_port_sw_db_print.logical_ports");
    if(!logical_ports) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_SOCDNX_MSG("Memory allocation failure")));
    }
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        uint32 is_valid;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid)) ;
        if (is_valid) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.get(unit, port, &channel);
            SOCDNX_IF_ERR_EXIT(rv);
            sal_snprintf(port_str,SW_DB_PRINT_COLUMN_LENGTH, "%d , %d", port, channel);
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port, &first_phy_port);
            SOCDNX_IF_ERR_EXIT(rv);
            sal_strncpy(logical_ports[first_phy_port].ports[logical_port_count[first_phy_port]], port_str, SW_DB_PRINT_COLUMN_LENGTH);
            logical_port_count[first_phy_port]++;
            port_str[0] = 0;
        } 
    }
    
    
    for (port = 0; port < SOC_MAX_NUM_PORTS; port ++) {
        SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.initialized.get(unit, port, &is_init));
        if (!is_init) {
            continue;
        }
        SOCDNX_IF_ERR_EXIT(_soc_port_sw_db_table_entry_add(unit, port, &line_struct, logical_ports, logical_port_count));
    }
    /*print last line*/
    SOCDNX_IF_ERR_EXIT(_soc_port_sw_db_line_print(unit, &line_struct, logical_ports, logical_port_count ));


exit:
    for(j = 0; j < SOC_MAX_NUM_PORTS ; ++j) {
        sal_free(logical_ports_str[j]);
    }
    
    if(logical_ports) {
        sal_free(logical_ports);
    }

    SOCDNX_FUNC_RETURN;
}

/*****************************/
/****  Snapshot Logic     ****/
/*****************************/

/*port sw data base snapshot support*/
/*Required for warmboot support - for any new information which added to the data base - support should be added*/
int soc_port_sw_db_snapshot_valid[SOC_MAX_NUM_DEVICES];
soc_phy_port_sw_db_t phy_ports_info_snapshot[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS];
soc_logical_port_sw_db_t logical_ports_info_snapshot[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS];

#define PORT_PHY_INFO_SS (phy_ports_info_snapshot[unit][logical_ports_info_snapshot[unit][port].first_phy_port])
#define PORT_INFO_SS (logical_ports_info_snapshot[unit][port])

/*
 * Function:
 *      soc_port_sw_db_snapshot_take
 * Purpose:
 *      Take a snapshot of port sw data base.
 *      Restore according to this snap shot can be done by using soc_port_sw_db_snapshot_restore.
 *      Every new information that added to port sw data base should be added to the snapshot support
 * Parameters:
 *      unit                - (IN) Unit number.
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_port_sw_db_snapshot_take(int unit) {
    soc_error_t rv;
    soc_port_t port;
    SOCDNX_INIT_FUNC_DEFS;

    for(port = 0; port < SOC_MAX_NUM_PORTS; ++port) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.get(unit, port, &logical_ports_info_snapshot[unit][port]);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.get(unit, port, &phy_ports_info_snapshot[unit][port]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:    
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_port_sw_db_snapshot_restore
 * Purpose:
 *      Restore port sw data base.
 *      Restore according to this snap shot that was taken soc_port_sw_db_snapshot_take.
 *      Assumes soc_port_sw_db_snapshot_take already called
 *      Every new information that added to port sw data base should be added to the snapshot support
 * Parameters:
 *      unit                - (IN) Unit number.
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_port_sw_db_snapshot_restore(int unit) {
    soc_port_t port;
    SOCDNX_INIT_FUNC_DEFS;

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        uint32 is_valid;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_get(unit, port, &is_valid)) ;
        if (is_valid) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_remove(unit, port));
        }
    }

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        if (PORT_INFO_SS.valid) {

            /*Add valid ports info*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_add(unit, 
                                                        PORT_INFO_SS.core,
                                                        port,
                                                        PORT_INFO_SS.channel,
                                                        PORT_INFO_SS.flags,
                                                        PORT_PHY_INFO_SS.interface_type,
                                                        PORT_PHY_INFO_SS.phy_ports));

            /*mark port as initialized*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_initialized_set(unit, port, PORT_PHY_INFO_SS.initialized));

            /*speed set*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_set(unit, port, PORT_PHY_INFO_SS.speed));

            /*latch down support*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_latch_down_set(unit, port, PORT_PHY_INFO_SS.latch_down));

            /*runt pad*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_runt_pad_set(unit, port, PORT_PHY_INFO_SS.runt_pad));

            /*is chnnelized*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_set(unit, port, PORT_PHY_INFO_SS.is_channelized));

            /* cal mode */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_single_cal_mode_set(unit, port, PORT_PHY_INFO_SS.is_single_cal_mode));

            /* high priority cal */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_set(unit, port, PORT_PHY_INFO_SS.high_pirority_cal));

            /* low priority cal */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_set(unit, port, PORT_PHY_INFO_SS.low_pirority_cal));
            
            /* master port */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_master_channel_set(unit, port, PORT_PHY_INFO_SS.master_port));

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_set(unit, port, PORT_INFO_SS.tm_port));                  
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_pp_port_set(unit, port, PORT_INFO_SS.pp_port));                  
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_out_port_priority_set(unit, port, PORT_INFO_SS.priority_mode));
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.multicast_offset.set(unit, port, PORT_INFO_SS.multicast_offset));
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.pp_flags.set(unit,port, PORT_INFO_SS.pp_flags));                                         
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.fc_type.set(unit, port, PORT_INFO_SS.fc_type));                              
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.mirror_profile.set(unit, port, PORT_INFO_SS.mirror_profile));                                  
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.is_tm_src_syst_port_ext_present.set(unit, port, PORT_INFO_SS.is_tm_src_syst_port_ext_present));            
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.is_stag_enabled.set(unit, port, PORT_INFO_SS.is_stag_enabled));                                
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.is_snoop_enabled.set(unit, port, PORT_INFO_SS.is_snoop_enabled));                              
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.is_tm_ing_shaping_enabled.set(unit, port, PORT_INFO_SS.is_tm_ing_shaping_enabled)); 
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.outlif_ext_en.set(unit, port, PORT_INFO_SS.outlif_ext_en));
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.src_ext_en.set(unit, port, PORT_INFO_SS.src_ext_en));                              
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.dst_ext_en.set(unit, port, PORT_INFO_SS.dst_ext_en)); 
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_hdr_type_out_set(unit, port, PORT_INFO_SS.header_type_out));                                                                      
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_hdr_type_in_set(unit, port, PORT_INFO_SS.header_type_in));                                           
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_header_size.set(unit, port, PORT_INFO_SS.first_header_size));
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.peer_tm_domain.set(unit, port, PORT_INFO_SS.peer_tm_domain));                                                  
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.shaper_mode.set(unit, port, PORT_INFO_SS.shaper_mode));                                                                                    
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.set(unit, port, PORT_INFO_SS.base_q_pair));        
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.set(unit, port, PORT_INFO_SS.egr_interface));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_set(unit, port, 0, PORT_INFO_SS.protocol_offset));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_encap_mode_set(unit, port, PORT_INFO_SS.encap_mode));                                                             
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_port_sw_db_port_add(int unit,  int core_of_port, soc_port_t port,uint32 channel, uint32 flags, soc_port_if_t interface, soc_pbmp_t phy_ports) 
{
    soc_port_t first_phy_port, phy_port, port_i;
    int core_of_first_phy_port ;
    uint32 num_of_lanes, found;
    uint32 is_virtual; /*virtual port - isn't attached with real interface*/
    int blk, is_master_channel = 0;
    soc_block_type_t blktype;
    int core;
    soc_error_t rv;
    soc_port_if_t interface_type;
    uint32 channels_count;
    int valid;
    int cur_first_phy_port;
#ifdef BCM_LB_SUPPORT
    soc_pbmp_t modem_pbmp;
    soc_port_t modem_port = 0;
#endif
    SOCDNX_INIT_FUNC_DEFS;

    core_of_first_phy_port = core_of_port ;
    if (SOC_PBMP_NOT_NULL(phy_ports)) {
        is_virtual = 0;
        SOC_PBMP_ITER(phy_ports, first_phy_port) {break;}
    } else {
        /*virtual port*/
        is_virtual = 1;
        found = 0;
        /*1. attach to port with same interface if avaliable*/
        for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; port_i++) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.valid.get(unit, port_i, &valid);
            SOCDNX_IF_ERR_EXIT(rv);
            if (valid) {
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.get(unit, port_i, &cur_first_phy_port);
                SOCDNX_IF_ERR_EXIT(rv);
                first_phy_port = cur_first_phy_port;
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.core.get(unit, first_phy_port, &core);
                SOCDNX_IF_ERR_EXIT(rv);
                rv = sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.interface_type.get(unit, first_phy_port, &interface_type);
                SOCDNX_IF_ERR_EXIT(rv);
                if (interface == interface_type 
                    && core_of_first_phy_port == core ) {
                    /*found port with same interface*/
                    found = 1;
                    break;
                }
            }
        }
        
        /*2. alllocate virtual phy port*/
        if (!found) {
            if(!SOC_WARM_BOOT(unit)) {
                for (first_phy_port = 0; first_phy_port < SOC_MAX_NUM_PORTS; first_phy_port++) {
                    /*make sure that: 1. the port not previously allocated 2. not phy port (can be virtual)*/
                    if ((-1 == SOC_INFO(unit).port_p2l_mapping[first_phy_port])
                        && (!SOC_PBMP_MEMBER(PORT_UNIT_INFO.all_phy_pbmp, first_phy_port))) {
                        SOC_PBMP_PORT_ADD(PORT_UNIT_INFO.all_phy_pbmp, first_phy_port);
                        found = 1;
                        break;
                    }
                }
            } else {
                /* Take the specific phy_port from the restoration data */
                first_phy_port = PORT_INFO_SS.first_phy_port;
                /* Do the same checks */
                if ((-1 == SOC_INFO(unit).port_p2l_mapping[first_phy_port])
                    && (!SOC_PBMP_MEMBER(PORT_UNIT_INFO.all_phy_pbmp, first_phy_port))) {
                    SOC_PBMP_PORT_ADD(PORT_UNIT_INFO.all_phy_pbmp, first_phy_port);
                    found = 1;
                } else {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Can\'t use the original first_phy_port in warmboot initialization")));
                }
            }
        }
        if (!found) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("All virtual ports already allocated")));
        }
    }

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.first_phy_port.set(unit, port, first_phy_port));
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.channel.set(unit, port, channel));
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.flags.set(unit, port, flags));

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.get(unit, first_phy_port, &channels_count));
    ++channels_count;

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.set(unit, first_phy_port, channels_count));
    if((channels_count > 1) || (interface == SOC_PORT_IF_ILKN)) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_set(unit, port, 1)); 
    } else {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_set(unit, port, 0));
    }

    if (1 == channels_count) {
            /* if this is the first channel added to interface*/
            /*init interface properties*/
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.core.set(unit, first_phy_port, core_of_first_phy_port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_set(unit, port, phy_ports));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_set(unit, port, interface));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_initialized_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_master_channel_set(unit, port, port));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_runt_pad_set(unit, port, 0));
            is_master_channel = 1;
            /* set low and high calendars as invalid */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_high_priority_cal_set(unit, port, INVALID_CALENDAR));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_low_priority_cal_set(unit, port, INVALID_CALENDAR));
    } else {
        /*
         * Enter if there are already channels (logical port) on this interface.
         * In that case, all logical ports on these physical ports (represented
         * by 'first_phy_port') must have the same core. However, CPU logical
         * ports are exceptions and may have, each, its own core value.
         */
        if (SOC_PORT_IF_CPU != interface && SOC_PORT_IF_RCY != interface) {
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.core.get(unit, first_phy_port, &core_of_first_phy_port));
            if (core_of_first_phy_port != core_of_port) {
                SOCDNX_EXIT_WITH_ERR(
                    SOC_E_PARAM,
                    (_BSL_SOCDNX_MSG("Logical port %d: All logical ports on same first_phy_port (%d) must have the same core"), port, first_phy_port)) ;
            }
        }
    }
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.core.set(unit, port, core_of_port);
    SOCDNX_IF_ERR_EXIT(rv);

    /*num of lanes*/
    SOC_PBMP_COUNT(phy_ports, num_of_lanes);
    SOC_INFO(unit).port_num_lanes[port] = num_of_lanes;

    /*add to block bitmap*/
    if (SOC_PORT_IF_TM_INTERNAL_PKT == interface) {
        SOC_INFO(unit).port_type[port] = SOC_BLK_ECI;
    } else if (!is_virtual) {

        if (interface == SOC_PORT_IF_CPU) {
            blk = SOC_PORT_IDX_INFO(unit, 0, PORT_INFO_INDEX(interface, 0)).blk;
        } else {
            blk = SOC_PORT_IDX_INFO(unit, first_phy_port, PORT_INFO_INDEX(interface, first_phy_port)).blk;
        }
        if (blk == -1)
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d interface %d first_phy_port %d is not supported\n"), port, interface, first_phy_port));
        }
        SOC_PBMP_PORT_ADD(SOC_INFO(unit).block_bitmap[blk], port);


        /*Set port block type*/
        blktype = SOC_BLOCK_INFO(unit, blk).type;
        SOC_INFO(unit).port_type[port] = blktype;
        if (is_master_channel && blktype != SOC_BLK_FSRD && blktype != SOC_BLK_FMAC)
        {
                SOC_INFO(unit).block_port[blk] = port;
        }

        /*set CPU info*/
        if (SOC_PORT_IF_CPU == interface && is_master_channel) {
            SOC_INFO(unit).cmic_port = port;
            SOC_INFO(unit).cmic_block = blk;
        }
    } else {
        if (SOC_PORT_IF_NOCXN == interface) {
            SOC_INFO(unit).port_type[port] = SOC_BLK_NONE;
        } else {
            SOC_INFO(unit).port_type[port] = SOC_BLK_ECI;
        }
    }

    /*set logical-to-physical*/
    SOC_INFO(unit).port_l2p_mapping[port] = first_phy_port;

    /*set physical-to-logical*/
    if (is_master_channel) {
        SOC_PBMP_ITER(phy_ports, phy_port) {
            SOC_INFO(unit).port_p2l_mapping[phy_port] = port;

            /*for caui always set all 12 lanes*/
            if ((SOC_PORT_IF_CAUI == interface) && (10 == num_of_lanes)) {
                if (phy_port + 2 >= SOC_MAX_NUM_PORTS) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Trying to acsess to port mapping with invalid index %d"), phy_port + 2));
                }
                SOC_INFO(unit).port_p2l_mapping[phy_port + 1] = port;
                SOC_INFO(unit).port_p2l_mapping[phy_port + 2] = port;
            }
        }
    }

    if (flags != SOC_PORT_FLAGS_VIRTUAL_RCY_INTERFACE) {
        /*specific interface properties*/
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_properties_set(unit, port));
    }

    /*increase number of ports*/
    SOC_INFO(unit).port_num++; 

    /* mark port as valid */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_valid_port_set(unit, port, 1));

    /* set header type to NONE */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_hdr_type_out_set(unit,port,SOC_TMC_PORT_HEADER_TYPE_NONE));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_hdr_type_in_set(unit,port,SOC_TMC_PORT_HEADER_TYPE_NONE));

#ifdef BCM_LB_SUPPORT
    if (SOC_IS_QAX(unit) && SOC_DPP_CONFIG(unit)->qax->link_bonding_enable) 
    {
        if (SOC_PORT_IS_LB_MODEM(flags)) {
            /* update channel count, Modem port is not a real port */
            channels_count--;
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.phy_ports_info.channels_count.set(unit, first_phy_port, channels_count));
        }
        else {
            /* update physical-to-logical */
            SOCDNX_IF_ERR_EXIT(qax_lb_modem_ports_on_same_interface_get(unit, port, &modem_pbmp)); 

            SOC_PBMP_ITER(modem_pbmp, modem_port) 
            {
                SOC_PBMP_ITER(phy_ports, phy_port) {
                    SOC_INFO(unit).port_p2l_mapping[phy_port] = modem_port;

                    /*for caui always set all 12 lanes*/
                    if ((SOC_PORT_IF_CAUI == interface) && (10 == num_of_lanes)) {
                        if (phy_port + 2 >= SOC_MAX_NUM_PORTS) {
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Trying to acsess to port mapping with invalid index %d"), phy_port + 2));
                        }
                        SOC_INFO(unit).port_p2l_mapping[phy_port + 1] = modem_port;
                        SOC_INFO(unit).port_p2l_mapping[phy_port + 2] = modem_port;
                    }
                }
                break;
            }
        }
    }
#endif

exit:
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME


