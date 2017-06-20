/*
 * $Id: port_sw_db.h,v 1.20 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC NIF PORT SW DB H
 */
 
#ifndef _SOC_DNX_NIF_PORT_SW_DB_H_
#define _SOC_DNX_NIF_PORT_SW_DB_H_

#include <soc/error.h>
#include <soc/types.h>
#include <soc/ll.h>
#include <soc/portmode.h>
#include <soc/dnx/legacy/ARAD/arad_api_egr_queuing.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

#define LANES_IN_QUAD 4
#define INVALID_EGR_INTERFACE 0xFFFFFFFF
#define INVALID_CALENDAR      0xff

/*add port to pbmp*/
#define PORT_SW_DB_PORT_ADD(ptype, nport) \
            assert(SOC_INFO(unit).ptype.num < SOC_MAX_NUM_PORTS); \
            if (!SOC_PBMP_MEMBER(SOC_INFO(unit).ptype.bitmap, nport)) { /* add port to type bitmap only if not already a memeber */ \
                SOC_INFO(unit).ptype.port[SOC_INFO(unit).ptype.num++] = nport;\
                if ( (SOC_INFO(unit).ptype.min < 0) || (SOC_INFO(unit).ptype.min > nport) ) {     \
                    SOC_INFO(unit).ptype.min = nport; \
                } \
                if (nport > SOC_INFO(unit).ptype.max) { \
                    SOC_INFO(unit).ptype.max = nport; \
                } \
                SOC_PBMP_PORT_ADD(SOC_INFO(unit).ptype.bitmap, nport); \
            }


/*****************************/
/****   Data Structures   ****/
/*****************************/

/* Per physical port information */
typedef struct dnx_phy_port_sw_db_s{
    int initialized;
    soc_pbmp_t phy_ports; 
    soc_port_if_t interface_type; 
    int speed; 
    soc_port_t master_port;
    uint32 channels_count;
    int is_channelized; /* might be set to also when channels_count == 1*/
    int latch_down;     /* indicates whether link latch has been down since last
                           call to bcm_port_link_state_get */ 
    uint32 runt_pad;
    int core;           /* This is the core of the group of logical ports which
                           are assigned to this physical port. See dnx_logical_port_sw_db_t.
                           Each of the logical ports also carries its corresponding core
                           and they must all be equal to the core specified on this physical
                           port (marked as 'first_phy_port'). Note that CPU logical ports are
                           an exception: Each logical port may have its own core which does
                           not have to be equal to the CPU 'physical port' */
    int is_single_cal_mode;
    uint32 high_pirority_cal;
    uint32 low_pirority_cal;
} dnx_phy_port_sw_db_t;

/* Per logical port information */
typedef struct dnx_logical_port_sw_db_s{

    /* NIF config */
    int valid;
    int first_phy_port;
    int channel;
    uint32 protocol_offset; 
    int flags;
    int core;           /* This is the core of this logical. See 'core' element
                           on 'dnx_phy_port_sw_db_t' */
    soc_encap_mode_t encap_mode;
        
    /* cosq */
    uint32 tm_port;
    uint32 pp_port;
    int priority_mode;
    uint32 base_q_pair;
    uint32 multicast_offset;
    uint32 egr_interface;

    /*OTMH info*/
    DNX_TMC_PORTS_FTMH_EXT_OUTLIF outlif_ext_en;
    int src_ext_en;
    int dst_ext_en;
    uint32 first_header_size;
    DNX_TMC_PORT_HEADER_TYPE header_type_out;
    DNX_TMC_PORT_HEADER_TYPE header_type_in;

    /*PP port info*/
    uint32 pp_flags;
    DNX_TMC_PORTS_FC_TYPE fc_type;
    uint32 mirror_profile;
    int is_tm_src_syst_port_ext_present;    
    int is_stag_enabled;
    int is_snoop_enabled;
    int is_tm_ing_shaping_enabled;
    DNX_TMC_EGR_PORT_SHAPER_MODE shaper_mode;

    /* Stacking */
    uint32 peer_tm_domain;
    uint32 timestamp_and_ssp;  /*Which program to use ,timestamp or SSP or both*/

} dnx_logical_port_sw_db_t;

typedef struct dnx_port_unit_info_s {
    soc_pbmp_t quads_out_of_reset;
    soc_pbmp_t all_phy_pbmp;
} dnx_port_unit_info_t;

typedef struct dnx_port_core_info_S {
    int tm_to_local_port_map[SOC_MAX_NUM_PORTS];
    int pp_to_local_port_map[SOC_MAX_NUM_PORTS];
} dnx_port_core_info_t;

/*mngmt functions*/
soc_error_t dnx_port_sw_db_init(int unit);
soc_error_t dnx_port_sw_db_destroy(int unit);
soc_error_t dnx_port_sw_db_runt_pad_set(int unit, soc_port_t port, uint32 runt_pad);
soc_error_t dnx_port_sw_db_port_add(int unit, int core, soc_port_t port, uint32 channel, uint32 flags, soc_port_if_t interface, soc_pbmp_t phy_ports);
soc_error_t dnx_port_sw_db_port_remove(int unit, soc_port_t port);
soc_error_t dnx_port_sw_db_next_master_get(int unit, soc_port_t port, soc_port_t *next_master);
soc_error_t dnx_port_sw_db_initialized_set(int unit, soc_port_t port, uint32 is_initialized);
soc_error_t dnx_port_sw_db_speed_set(int unit, soc_port_t port, int speed);
soc_error_t dnx_port_sw_db_encap_mode_set(int unit, soc_port_t port, soc_encap_mode_t encap_mode);
soc_error_t dnx_port_sw_db_is_hg_set(int unit, soc_port_t port, uint32 is_hg);
soc_error_t dnx_port_sw_db_flag_add(int unit, soc_port_t port, uint32 flag);
soc_error_t dnx_port_sw_db_flag_remove(int unit, soc_port_t port, uint32 flag);
soc_error_t dnx_port_sw_db_serdes_quads_out_of_reset_set(int unit, soc_pbmp_t quads);
soc_error_t dnx_port_sw_db_print(int unit, uint32 flags);
soc_error_t dnx_port_sw_db_latch_down_set(int unit, soc_port_t port, int latch_down);
soc_error_t dnx_port_sw_db_interface_properties_remove(int unit, soc_port_t port);
soc_error_t dnx_port_sw_db_interface_properties_set(int unit, soc_port_t port);
soc_error_t dnx_port_sw_db_port_validate_and_add(int unit, int core, soc_port_t port, uint32 channel, uint32 flags, soc_port_if_t interface, soc_pbmp_t phy_ports);
soc_error_t dnx_port_sw_db_is_valid_port_set(int unit, soc_port_t port, uint32 valid);
soc_error_t dnx_port_sw_db_is_single_cal_mode_set(int unit, soc_port_t port, int is_single_mode);  
soc_error_t dnx_port_sw_db_high_priority_cal_set(int unit, soc_port_t port, uint32 cal_id);
soc_error_t dnx_port_sw_db_low_priority_cal_set(int unit, soc_port_t port, uint32 cal_id);
 
/*getters*/
soc_error_t dnx_port_sw_db_runt_pad_get(int unit, soc_port_t port, uint32* runt_pad);
soc_error_t dnx_port_sw_db_master_channel_get(int unit, soc_port_t port, soc_port_t* master_port);
soc_error_t dnx_port_sw_db_is_master_get(int unit, soc_port_t port, uint32* is_master);
soc_error_t dnx_port_sw_db_interface_rate_get(int unit, soc_port_t port, uint32* rate);
soc_error_t dnx_port_sw_db_is_valid_port_get(int unit, soc_port_t port, uint32* is_valid);
soc_error_t dnx_port_sw_db_is_initialized_get(int unit, soc_port_t port, uint32* is_initialized);
soc_error_t dnx_port_sw_db_phy_ports_get(int unit, soc_port_t port, soc_pbmp_t* phy_ports);
soc_error_t dnx_port_sw_db_first_phy_port_get(int unit, soc_port_t port, uint32* phy_port /*one based*/);
soc_error_t dnx_port_sw_db_num_lanes_get(int unit, soc_port_t port, uint32* count);
soc_error_t dnx_port_sw_db_interface_type_set(int unit, soc_port_t port, soc_port_if_t interface_type);
soc_error_t dnx_port_sw_db_interface_type_get(int unit, soc_port_t port, soc_port_if_t* interface_type);
soc_error_t dnx_port_sw_db_speed_get(int unit, soc_port_t port, int* speed);
soc_error_t dnx_port_sw_db_encap_mode_get(int unit, soc_port_t port, soc_encap_mode_t *encap_mode);
soc_error_t dnx_port_sw_db_is_hg_get(int unit, soc_port_t port, uint32* is_hg);
soc_error_t dnx_port_sw_db_flags_get(int unit, soc_port_t port, uint32* flags);
soc_error_t dnx_port_sw_db_num_of_channels_get(int unit, soc_port_t port, uint32* num_of_channels);
soc_error_t dnx_port_sw_db_max_channel_num_get(int unit, soc_port_t port, uint32* max_channel);
soc_error_t dnx_port_sw_db_channel_get(int unit, soc_port_t port, uint32* channel);
soc_error_t dnx_port_sw_db_in_block_index_get(int unit, soc_port_t port, uint32* in_block_index /*zero based*/);
soc_error_t dnx_port_sw_db_latch_down_get(int unit, soc_port_t port, int* latch_down);
soc_error_t dnx_port_sw_db_ports_to_same_interface_get(int unit, soc_port_t port, soc_pbmp_t* ports);
soc_error_t dnx_port_sw_db_port_with_channel_get(int unit, soc_port_t port, uint32 channel, soc_port_t* port_match_channel);
soc_error_t dnx_port_sw_db_port_ptc_get(int unit, soc_port_t port, soc_port_t *ptc);
soc_error_t dnx_port_sw_db_port_from_interface_type_get(int unit, soc_port_if_t interface_type, int first_phy_port, soc_port_t *port_to_return);
soc_error_t dnx_port_sw_db_is_single_cal_mode_get(int unit, soc_port_t port, int *is_single_mode);
soc_error_t dnx_port_sw_db_high_priority_cal_get(int unit, soc_port_t port, uint32 *cal_id);
soc_error_t dnx_port_sw_db_low_priority_cal_get(int unit, soc_port_t port, uint32 *cal_id); 

soc_error_t dnx_port_sw_db_valid_ports_get(int unit, uint32 required_flags, soc_pbmp_t* ports_bm);
soc_error_t dnx_port_sw_db_valid_ports_core_get(int unit, int core, uint32 required_flag, soc_pbmp_t *ports_bm); 
soc_error_t dnx_port_sw_db_serdes_quads_out_of_reset_get(int unit, soc_pbmp_t* quads);

/* IMPORTANT NOTE: returned ports_bm include invalid ports!!! use dnx_port_sw_db_valid_ports_get for valid ports only*/
soc_error_t dnx_port_sw_db_ports_get(int unit, uint32 required_flags, soc_pbmp_t* ports_bm);

/* Logical port setters/getters */
soc_error_t dnx_port_sw_db_core_get(int unit, soc_port_t port, int *core  /*zero based*/) ;
soc_error_t dnx_port_sw_db_local_to_tm_port_set(int unit, soc_port_t port, uint32 tm_port);
soc_error_t dnx_port_sw_db_local_to_tm_port_get(int unit, soc_port_t port, uint32* tm_port, int* core);
soc_error_t dnx_port_sw_db_local_to_pp_port_set(int unit, soc_port_t port, uint32 pp_port);
soc_error_t dnx_port_sw_db_local_to_pp_port_get(int unit, soc_port_t port, uint32* pp_port, int* core);
soc_error_t dnx_port_sw_db_tm_to_local_port_get(int unit, int core, uint32 tm_port, soc_port_t *port);
soc_error_t dnx_port_sw_db_pp_to_local_port_get(int unit, int core, uint32 pp_port, soc_port_t *port);
soc_error_t dnx_port_sw_db_pp_is_valid_get(int unit, int core, uint32 pp_port, uint32 *is_valid);
soc_error_t dnx_port_sw_db_is_channelized_port_set(int unit, soc_port_t port, int is_channelized);
soc_error_t dnx_port_sw_db_is_channelized_port_get(int unit, soc_port_t port, int *is_channelized);
soc_error_t dnx_port_sw_db_e2e_if_set(int unit, soc_port_t port, uint32 e2e_if);
soc_error_t dnx_port_sw_db_e2e_if_get(int unit, soc_port_t port, uint32* e2e_if);

soc_error_t dnx_port_sw_db_local_to_out_port_priority_set(int unit, soc_port_t port, uint32 nof_priorities);
soc_error_t dnx_port_sw_db_local_to_out_port_priority_get(int unit, soc_port_t port, uint32* nof_priorities);
soc_error_t dnx_port_sw_db_tm_port_to_out_port_priority_get(int unit, int core, uint32 tm_port, uint32* nof_priorities);
soc_error_t dnx_port_sw_db_pp_port_to_out_port_priority_get(int unit, int core, uint32 pp_port, uint32* nof_priorities);

soc_error_t dnx_port_sw_db_tm_port_to_base_q_pair_get(int unit, int core, uint32 tm_port, uint32* base_q_pair);
soc_error_t dnx_port_sw_db_pp_port_to_base_q_pair_get(int unit, int core, uint32 pp_port, uint32* base_q_pair);

soc_error_t dnx_port_sw_db_free_tm_ports_get(int unit, int core, soc_pbmp_t* tm_ports);

/*Header info*/
soc_error_t dnx_port_sw_db_hdr_type_out_set(int unit, soc_port_t port, DNX_TMC_PORT_HEADER_TYPE header_type_out);
soc_error_t dnx_port_sw_db_hdr_type_in_set(int unit, soc_port_t port, DNX_TMC_PORT_HEADER_TYPE header_type_in);

/*PP port info*/
soc_error_t dnx_port_sw_db_pp_port_flags_add(int unit, soc_port_t port, uint32 flag);
soc_error_t dnx_port_sw_db_pp_port_flags_rmv(int unit, soc_port_t port, uint32 flag);

/* return TRUE or FALSE. On any error, just returns FALSE.  */
soc_error_t dnx_port_sw_db_interface_is_virt_rcy_port(int unit, soc_port_t port, int *is_virt_rcy_port);

/*Port software database snapshot tool*/
soc_error_t dnx_port_sw_db_snapshot_take(int unit);
soc_error_t dnx_port_sw_db_snapshot_restore(int unit);

/*flags*/
#define DNX_PORT_FLAGS_STAT_INTERFACE               0x1
#define DNX_PORT_FLAGS_NETWORK_INTERFACE            0x2
#define DNX_PORT_FLAGS_PON_INTERFACE                0x4
#define DNX_PORT_FLAGS_VIRTUAL_RCY_INTERFACE        0x8
/*#define DNX_PORT_FLAGS_DISABLED                   0x10*/ /* can be used for a new flag */
#define DNX_PORT_FLAGS_ELK                          0x20
#define DNX_PORT_FLAGS_XGS_MAC_EXT                  0x40
/* If set, port supports Initial-VID both for untagged and tagged packets */
#define DNX_PORT_FLAGS_INIT_VID_ONLY                0x80
#define DNX_PORT_FLAGS_FIBER                        0x100
#define DNX_PORT_FLAGS_SCRAMBLER                    0x200
#define DNX_PORT_FLAGS_VENDOR_PP_PORT               0x400
#define DNX_PORT_FLAGS_COE_PORT                     0x800
#define DNX_PORT_FLAGS_COPPER                       0x1000
#define DNX_PORT_FLAGS_SAME_QPAIR                   0x2000
#define DNX_PORT_FLAGS_LB_MODEM                     0x4000

#define SOC_PROTOCOL_OFFSET_FLAGS_FORCE_ILKN        0x1

#define DNX_PORT_IS_STAT_INTERFACE(flags) (flags & DNX_PORT_FLAGS_STAT_INTERFACE)
#define DNX_PORT_IS_NETWORK_INTERFACE(flags) (flags & DNX_PORT_FLAGS_NETWORK_INTERFACE)
#define DNX_PORT_IS_VIRTUAL_RCY_INTERFACE(flags) (flags & DNX_PORT_FLAGS_VIRTUAL_RCY_INTERFACE)
#define DNX_PORT_IS_ELK_INTERFACE(flags) (flags & DNX_PORT_FLAGS_ELK)
#define DNX_PORT_IS_XGS_MAC_EXT_INTERFACE(flags) (flags & DNX_PORT_FLAGS_XGS_MAC_EXT)
#define DNX_PORT_IS_INIT_VID_ONLY_INTERFACE(flags) (flags & DNX_PORT_FLAGS_INIT_VID_ONLY)
#define DNX_PORT_IS_FIBER(flags) (flags & DNX_PORT_FLAGS_FIBER)
#define DNX_PORT_IS_COPPER(flags) (flags & DNX_PORT_FLAGS_COPPER)
#define DNX_PORT_IS_SCRAMBLER(flags) (flags & DNX_PORT_FLAGS_SCRAMBLER)
#define DNX_PORT_IS_VENDOR_PP_PORT(flags) (flags & DNX_PORT_FLAGS_VENDOR_PP_PORT)
#define DNX_PORT_IS_COE_PORT(flags) (flags & DNX_PORT_FLAGS_COE_PORT)
#define DNX_PORT_IS_SAME_QPAIR(flags) (flags & DNX_PORT_FLAGS_SAME_QPAIR)
#define DNX_PORT_IS_LB_MODEM(flags) (flags & DNX_PORT_FLAGS_LB_MODEM)

soc_error_t dnx_port_sw_db_protocol_offset_set(int unit, soc_port_t port, uint32 flags, uint32 offset);
soc_error_t dnx_port_sw_db_protocol_offset_get(int unit, soc_port_t port, uint32 flags, uint32 *offset);


#endif /*_SOC_DNX_NIF_PORT_SW_DB_H_*/


