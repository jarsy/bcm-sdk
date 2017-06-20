/*
 * $Id: cmdlist.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    diag_dnx_cmdlist.h
 * Purpose: Extern declarations for command functions and
 *          their associated usage strings.
 */

#ifndef DIAG_DNXF_CMDLIST_H_INCLUDED
#define DIAG_DNXF_CMDLIST_H_INCLUDED

#include <appl/diag/diag.h>
#include <appl/diag/sand/diag_sand_mem.h>
#include <appl/diag/sand/diag_sand_reg.h>

#define DCL_CMD(_f,_u)  \
    extern cmd_result_t _f(int, args_t *); \
    extern char     _u[];

extern int   bcm_dnxf_cmd_cnt;
extern cmd_t bcm_dnxf_cmd_list[];

DCL_CMD(cmd_init_dnx,      appl_dcmn_init_usage)

DCL_CMD(cmd_avs,      cmd_avs_usage)

DCL_CMD(cmd_dpp_clear, cmd_dpp_clear_usage)

#if defined(__DUNE_WRX_BCM_CPU__)
DCL_CMD(cmd_dpp_cpu_i2c, cmd_dpp_cpu_i2c_usage)
#endif

DCL_CMD(cmd_dpp_counter, cmd_dpp_counter_usage)

#if defined(__DUNE_WRX_BCM_CPU__)
DCL_CMD(cmd_dpp_cpu_regs, cmd_dpp_cpu_regs_usage)
#endif

DCL_CMD(cmd_dpp_ctr_proc, cmd_dpp_ctr_proc_usage)

DCL_CMD(cmd_dpp_ui, cmd_dpp_ui_usage)

DCL_CMD(cmd_dpp_gport, cmd_dpp_gport_usage)

DCL_CMD(cmd_dpp_pem, cmd_dpp_pem_usage)

DCL_CMD(cmd_dpp_gfa_bi, cmd_dpp_gfa_bi_usage)

DCL_CMD(cmd_dpp_pbmp, cmd_dpp_pbmp_usage)

#if defined(__DUNE_WRX_BCM_CPU__)
DCL_CMD(cmd_dpp_pcie_reg, cmd_dpp_pcie_reg_usage)
#endif

DCL_CMD(cmd_pcic_access,      cmd_pcic_access_usage)

DCL_CMD(if_dpp_phy, if_dpp_phy_usage)

DCL_CMD(if_dpp_port_stat, if_dpp_port_stat_usage)

DCL_CMD(if_dpp_port, if_dpp_port_usage)

DCL_CMD(cmd_dpp_show, cmd_dpp_show_usage)

DCL_CMD(cmd_dpp_intr, cmd_dpp_intr_usage)

DCL_CMD(cmd_dpp_soc, cmd_dpp_soc_usage)

DCL_CMD(cmd_dpp_switch_control, cmd_dpp_switch_control_usage)

DCL_CMD(cmd_dpp_tx, cmd_dpp_tx_usage)

DCL_CMD(cmd_dpp_vlan, cmd_dpp_vlan_usage)

DCL_CMD(cmd_dpp_diag, cmd_dpp_diag_usage)

DCL_CMD(cmd_dpp_dma, cmd_dpp_dma_usage)

#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)
DCL_CMD(cmd_xxreload,   cmd_xxreload_usage)
#endif /*defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)*/

DCL_CMD(diag_dnx_fabric_diag_pack, diag_dnx_fabric_diag_pack_usage_str)

#undef  DCL_CMD

#endif  /*  DIAG_DNXF_CMDLIST_H_INCLUDED */
