/*
 * $Id: ea.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     ea.h
 * Purpose:
 *
 */
#ifndef _BCM_DIAG_EA_H
#define _BCM_DIAG_EA_H
#include <appl/diag/diag.h>

#define CLI_CMD(_f,_u)  \
        extern cmd_result_t     _f(int, args_t *); \
        extern char             _u[];
extern int bcm_ea_cmd_cnt;
extern cmd_t bcm_ea_cmd_list[];

#if defined(BCM_TK371X_SUPPORT)
CLI_CMD(cmd_ea_clear,		cmd_ea_clear_usage)
CLI_CMD(if_ea_l2, 			if_ea_l2_usage)
CLI_CMD(if_ea_l2mode,		if_ea_l2mode_usage)
CLI_CMD(if_ea_linkscan,		if_ea_linkscan_usage)
CLI_CMD(cmd_ea_mem_list,	cmd_ea_mem_list_usage)
CLI_CMD(cmd_ea_reg_list,	cmd_ea_reg_list_usage)
CLI_CMD(if_ea_mcast,		if_ea_mcast_usage)
CLI_CMD(if_ea_port, 		if_ea_port_usage)
CLI_CMD(if_ea_port_control, if_ea_port_control_usage)
CLI_CMD(if_ea_port_cross_connect, if_ea_port_cross_connect_usage)
CLI_CMD(if_ea_port_rate, 	if_ea_port_rate_usage)
CLI_CMD(if_ea_port_samp_rate, if_ea_port_samp_rate_usage)
CLI_CMD(if_ea_port_stat,	if_ea_port_stat_usage)
CLI_CMD(if_ea_pvlan,		if_ea_pvlan_usage)
CLI_CMD(cmd_ea_cos, 		cmd_ea_cos_usage)
CLI_CMD(if_ea_field_proc,   if_ea_field_proc_usage)
CLI_CMD(cmd_ea_rx_cfg, 		cmd_ea_rx_cfg_usage)
CLI_CMD(cmd_ea_rx_init,		cmd_ea_rx_init_usage)
CLI_CMD(cmd_ea_tx,  		cmd_ea_tx_usage)
CLI_CMD(cmd_ea_tx_count, 	cmd_ea_tx_count_usage)
CLI_CMD(cmd_ea_tx_start,  	cmd_ea_tx_start_usage)
CLI_CMD(cmd_ea_tx_stop,		cmd_ea_tx_stop_usage)
CLI_CMD(cmd_ea_show, 		cmd_ea_show_usage)
CLI_CMD(test_ea_domaen, 	test_ea_domaen_usage)
CLI_CMD(cmd_ea_soc, 		cmd_ea_soc_usage)
CLI_CMD(cmd_ea_switch_control,	cmd_ea_switch_control_usage)
CLI_CMD(if_ea_stg, 			if_ea_stg_usage)
CLI_CMD(if_ea_port_phy_control, if_ea_port_phy_control_usage)
#endif
#endif /* _BCM_DIAG_EA_H */
