/*
 *         
 * $Id:$
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *
 */

#ifndef _PM4X25TD_SHARED_H_
#define _PM4X25TD_SHARED_H_

#define PM4X25_LANES_PER_CORE (4)
#define MAX_PORTS_PER_PM4X25 (4)

struct pm4x25_s{
    portmod_pbmp_t phys;
    int first_phy;
    phymod_ref_clk_t ref_clk;
    phymod_polarity_t polarity;
    phymod_lane_map_t lane_map;
    phymod_firmware_load_method_t fw_load_method;
    phymod_firmware_loader_f external_fw_loader;
    phymod_core_access_t int_core_access;
    int nof_phys[PM4X25_LANES_PER_CORE] ; /* internal + external phys for each lane */
    uint8 in_pm12x10;
    uint8 core_num;
    portmod_mac_soft_reset_f portmod_mac_soft_reset;
    portmod_xphy_lane_connection_t lane_conn[MAX_PHYN][PM4X25_LANES_PER_CORE];
    phymod_afe_pll_t afe_pll;
    int warmboot_skip_db_restore;
    int rescal;
};

#define PM_4x25_INFO(pm_info) ((pm_info)->pm_data.pm4x25_db)


#endif /*_PM4X25TD_SHARED_H_*/
