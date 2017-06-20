/*
 * $Id: cmdlist.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    cmdlist.h
 * Purpose: Extern declarations for command functions and
 *          their associated usage strings.
 */

#ifndef _DIAG_DCMN_CMDLIST_H
#define _DIAG_DCMN_CMDLIST_H

#include <appl/diag/diag.h>



#define DCL_CMD(_f,_u)  \
    extern cmd_result_t _f(int, args_t *); \
    extern char     _u[];

extern int   bcm_dpp_cmd_cnt;
extern cmd_t bcm_dpp_cmd_list[];

DCL_CMD(cmd_init_dnx,      appl_dcmn_init_usage)

DCL_CMD(cmd_avs,      cmd_avs_usage)

DCL_CMD(cmd_dpp_clear, cmd_dpp_clear_usage)

#if (defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__))
DCL_CMD(cmd_dpp_cpu_i2c, cmd_dpp_cpu_i2c_usage)
#endif


DCL_CMD(cmd_dpp_counter, cmd_dpp_counter_usage)

#if (defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__))
DCL_CMD(cmd_dpp_cpu_regs, cmd_dpp_cpu_regs_usage)
#endif

DCL_CMD(cmd_dpp_ctr_proc, cmd_dpp_ctr_proc_usage)

DCL_CMD(cmd_dpp_dump, cmd_dpp_dump_usage)

DCL_CMD(cmd_dpp_ui, cmd_dpp_ui_usage)

DCL_CMD(cmd_dpp_gport, cmd_dpp_gport_usage)

DCL_CMD(cmd_dpp_reg_get, cmd_dpp_reg_get_usage)

DCL_CMD(cmd_dpp_pem, cmd_dpp_pem_usage)

DCL_CMD(cmd_dpp_gfa_bi, cmd_dpp_gfa_bi_usage)

DCL_CMD(cmd_dpp_fe1600_card, cmd_dpp_fe1600_card_usage)
DCL_CMD(cmd_dpp_fe3200_card, cmd_dpp_fe3200_card_usage)

DCL_CMD(cmd_dpp_arad_card, cmd_dpp_arad_card_usage)
DCL_CMD(cmd_dpp_ardon_card, cmd_dpp_ardon_card_usage)
DCL_CMD(cmd_dpp_qmx_card, cmd_dpp_qmx_card_usage)
DCL_CMD(cmd_dpp_jericho_card, cmd_dpp_jericho_card_usage)

DCL_CMD(cmd_set_device, cmd_set_device_usage)

#ifdef BCM_PETRA_SUPPORT
#if defined(__DUNE_GTO_BCM_CPU__)
DCL_CMD(cmd_dcmn_negev_chassis, cmd_dcmn_negev_chassis_usage)
#endif
#endif

DCL_CMD(cmd_dpp_l2, cmd_dpp_l2_usage)

DCL_CMD(cmd_dpp_l3, cmd_dpp_l3_usage)

#if defined(__DUNE_GTO_BCM_CPU__) && defined(BCM_PETRA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
DCL_CMD(cmd_dpp_llm, cmd_dpp_llm_usage)
#endif /* defined(__DUNE_GTO_BCM_CPU__) && defined(BCM_PETRA_SUPPORT) && defined(BCM_CMICM_SUPPORT) */

DCL_CMD(cmd_dpp_mem_list, cmd_dpp_mem_list_usage)

DCL_CMD(cmd_dpp_reg_list, cmd_dpp_reg_list_usage)

DCL_CMD(cmd_dpp_mem_modify, cmd_dpp_mem_modify_usage)

DCL_CMD(cmd_dpp_reg_modify, cmd_dpp_reg_modify_usage)

DCL_CMD(cmd_dpp_pbmp, cmd_dpp_pbmp_usage)

#if (defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__))
DCL_CMD(cmd_dpp_pcie_reg, cmd_dpp_pcie_reg_usage)
#endif

DCL_CMD(cmd_pcic_access,      cmd_pcic_access_usage)

DCL_CMD(if_dpp_phy, if_dpp_phy_usage)

DCL_CMD(if_dpp_port_stat, if_dpp_port_stat_usage)

DCL_CMD(if_dpp_port, if_dpp_port_usage)

DCL_CMD(cmd_dpp_reg_set, cmd_dpp_reg_set_usage)

DCL_CMD(cmd_dpp_show, cmd_dpp_show_usage)

DCL_CMD(cmd_dpp_intr, cmd_dpp_intr_usage)

DCL_CMD(cmd_dpp_soc, cmd_dpp_soc_usage)

DCL_CMD(cmd_dpp_switch_control, cmd_dpp_switch_control_usage)

#if (defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__)) && defined(BCM_PETRA_SUPPORT)
DCL_CMD(cmd_dpp_time_measure, cmd_dpp_time_measure_usage)
#endif

DCL_CMD(cmd_dpp_tx, cmd_dpp_tx_usage)

DCL_CMD(cmd_dpp_vlan, cmd_dpp_vlan_usage)

DCL_CMD(cmd_dpp_mem_write, cmd_dpp_mem_write_usage)

DCL_CMD(cmd_dpp_diag, cmd_dpp_diag_usage)

#if defined(BCM_PETRA_SUPPORT)
DCL_CMD(cmd_arad_dram_buf, cmd_arad_dram_buf_usage)
DCL_CMD(cmd_arad_dram_mmu_ind_access, cmd_arad_dram_mmu_ind_access_usage)
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
DCL_CMD(cmd_dpp_kbp, cmd_dpp_kbp_usage);
#endif 
#endif /* BCM_PETRA_SUPPORT */

DCL_CMD(cmd_dpp_dma, cmd_dpp_dma_usage)

#ifdef BCM_PETRA_SUPPORT
#ifdef BCM_DDR3_SUPPORT
DCL_CMD(cmd_arad_ddr_phy_regs, cmd_arad_ddr_phy_regs_usage)
DCL_CMD(cmd_arad_ddr_phy_tune, cmd_arad_ddr_phy_tune_usage)
DCL_CMD(cmd_arad_ddr_phy_cdr, cmd_arad_ddr_phy_cdr_usage)
#endif
#endif

#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)
DCL_CMD(cmd_xxreload,   cmd_xxreload_usage)
#endif /*defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)*/

#ifdef BCM_PETRA_SUPPORT
DCL_CMD(cmd_dpp_tdm, cmd_dpp_tdm_usage)

DCL_CMD(cmd_dpp_gtimer, cmd_dpp_gtimer_usage)

DCL_CMD(cmd_dpp_cosq, cmd_dpp_cosq_usage)
DCL_CMD(if_dpp_stg, if_dpp_stg_usage)

DCL_CMD(cmd_dpp_fc, cmd_dpp_fc_usage)
#endif

DCL_CMD(diag_dnx_fabric_diag_pack, diag_dnx_fabric_diag_pack_usage_str)

#if !defined(NO_FILEIO)
DCL_CMD(cmd_dpp_export, cmd_dpp_export_usage_str)
#endif

#ifdef INCLUDE_KNET
DCL_CMD(cmd_dpp_knet, cmd_dpp_knet_usage)
#endif

#ifdef BCM_LB_SUPPORT
DCL_CMD(cmd_dpp_lb, cmd_dpp_lb_usage)
#endif /* BCM_LB_SUPPORT */

#undef  DCL_CMD

#endif  /*  _DIAG_DCMN_CMDLIST_H */
