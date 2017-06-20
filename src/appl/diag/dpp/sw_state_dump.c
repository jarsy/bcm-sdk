/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * 
 * 
 * 
 */


#define REMOVE_UNTIL_COMPILE 0
#define SW_DUMP_DISPLAY_NON_RETRIEVED_STATE 0

#include <shared/bsl.h>

#include <shared/swstate/sw_state_defs.h> 
#include <soc/defs.h>
#include <sal/appl/io.h> 
#include <bcm/error.h>
#include <bcm/debug.h>

#if !(defined __KERNEL__)
#include <appl/diag/dpp/sw_state_dump.h>

#include <sal/appl/editline/editline.h>

#include <appl/diag/dpp/bits_bytes_macros.h> 
/* #include <appl/diag/dpp/bsp_drv_error_defs.h>                */
/* #include <appl/diag/dpp/bsp_drv_flash28f.h>                  */
/* #include <appl/diag/dpp/bsp_info_access.h>                   */
/* #include <appl/diag/dpp/dune_chips.h>                        */
/* #include <appl/diag/dpp/gsa_framework.h>                     */
/* #include <appl/diag/dpp/ref_design_petra_starter.h>          */
/* #include <appl/diag/dpp/ref_design_starter.h>                */
/* #include <appl/diag/dpp/ref_sys.h>                           */
/* #include <appl/diag/dpp/tasks_info.h>                        */ 
/* #include <appl/diag/dpp/ui_defx.h>                           */
/* #include <appl/diag/dpp/utils_arad_card.h>                   */
/* #include <appl/diag/dpp/utils_char_queue.h>                  */
/* #include <appl/diag/dpp/utils_defi.h>                        */
/* #include <appl/diag/dpp/utils_defx.h>                        */
/* #include <appl/diag/dpp/utils_dffs_cpu_mez.h>                */
/* #include <appl/diag/dpp/utils_dune_fpga_download.h>          */
/* #include <appl/diag/dpp/utils_error_defs.h>                  */
/* #include <appl/diag/dpp/utils_front_end_host_card.h>         */
/* #include <appl/diag/dpp/utils_host_board.h>                  */
/* #include <appl/diag/dpp/utils_i2c_mem.h>                     */
/* #include <appl/diag/dpp/utils_line_PTG.h>                    */
/* #include <appl/diag/dpp/utils_line_gfa_bi.h>                 */
/* #include <appl/diag/dpp/utils_line_gfa_petra.h>              */
/* #include <appl/diag/dpp/utils_memory.h>                      */
/* #include <appl/diag/dpp/utils_nvram_configuration.h>         */
/* #include <appl/diag/dpp/utils_pure_defi.h>                   */
/* #include <appl/diag/dpp/utils_string.h>                      */

#include <shared/swstate/sw_state_defs.h> 

#include <soc/dpp/ARAD/arad_api_ingress_scheduler.h> 
#include <soc/dpp/ARAD/arad_scheduler_elements.h> 

#include <appl/dpp/interrupts/interrupt_handler.h> 
#include <appl/dpp/interrupts/interrupt_handler_cb_func.h> 
#include <appl/dpp/interrupts/interrupt_handler_corr_act_func.h> 

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oam.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_port.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ptp.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_rif.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_trap_mgmt.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_vsi.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_oamp_pe.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_array_memory_allocator.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ce_instruction.h> 
/* #include <soc/dpp/ARAD/ARAD_PP/arad_pp_chip_regs.h> */
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_ac.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap_access.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_filter.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_mirror.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_qos.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_vlan_edit.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_esem_access.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_fem.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_bmact.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fcf.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fec.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ilm.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4_lpm_mngr.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4_test.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv6.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact_mgmt.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_trill.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_init.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_l3_src_bind.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lag.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_cos.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_ing_vlan_edit.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_table.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_cos.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_filter.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_mirror.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_parse.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_sa_auth.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_trap.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_vid_assign.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mgmt.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mpls_term.h> 
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mymac.h> 
#include <soc/dpp/ARAD/arad_action_cmd.h> 
#include <soc/dpp/ARAD/arad_api_action_cmd.h> 
#include <soc/dpp/ARAD/arad_api_cnm.h> 
#include <soc/dpp/ARAD/arad_api_cnt.h> 
#include <soc/dpp/ARAD/arad_api_debug.h> 
#include <soc/dpp/ARAD/arad_api_diagnostics.h> 
#include <soc/dpp/ARAD/arad_api_dram.h> 
#include <soc/dpp/ARAD/arad_api_egr_queuing.h> 
#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h> 
#include <soc/dpp/ARAD/arad_api_fabric.h> 
#include <soc/dpp/ARAD/arad_api_flow_control.h> 
#include <soc/dpp/ARAD/arad_api_framework.h> 
#include <soc/dpp/ARAD/arad_api_general.h> 
#include <soc/dpp/ARAD/arad_api_ingress_packet_queuing.h> 
#include <soc/dpp/ARAD/arad_api_ingress_traffic_mgmt.h> 
#include <soc/dpp/ARAD/arad_api_mgmt.h> 
#include <soc/dpp/ARAD/arad_api_multicast_fabric.h> 
#include <soc/dpp/ARAD/arad_api_nif.h> 
#include <soc/dpp/ARAD/arad_api_ofp_rates.h> 
#include <soc/dpp/ARAD/arad_api_ports.h> 
#include <soc/dpp/ARAD/arad_api_stat_if.h> 
#include <soc/dpp/ARAD/arad_api_tdm.h> 
#include <soc/dpp/ARAD/arad_cell.h> 
#include <soc/dpp/ARAD/arad_chip_defines.h> 
#include <soc/dpp/ARAD/arad_chip_regs.h> 
#include <soc/dpp/ARAD/arad_chip_tbls.h> 
#include <soc/dpp/ARAD/arad_cnm.h> 
#include <soc/dpp/ARAD/arad_cnt.h> 
#include <soc/dpp/ARAD/arad_debug.h> 
#include <soc/dpp/ARAD/arad_defs.h> 
#include <soc/dpp/ARAD/arad_diagnostics.h> 
#include <soc/dpp/ARAD/arad_dram.h> 
#include <soc/dpp/ARAD/arad_egr_prog_editor.h> 
#include <soc/dpp/ARAD/arad_egr_queuing.h> 
#include <soc/dpp/ARAD/arad_fabric.h> 
#include <soc/dpp/ARAD/arad_fabric_cell.h> 
#include <soc/dpp/ARAD/arad_flow_control.h> 
#include <soc/dpp/ARAD/arad_framework.h> 
#include <soc/dpp/ARAD/arad_general.h> 
#include <soc/dpp/ARAD/arad_header_parsing_utils.h> 
#include <soc/dpp/ARAD/arad_ingress_packet_queuing.h> 
#include <soc/dpp/ARAD/arad_ingress_scheduler.h> 
#include <soc/dpp/ARAD/arad_ingress_traffic_mgmt.h> 
#include <soc/dpp/ARAD/arad_init.h> 
#include <soc/dpp/ARAD/arad_interrupts.h> 
/* #include <soc/dpp/ARAD/arad_kbp.h>       */
/* #include <soc/dpp/ARAD/arad_kbp_rop.h>   */
/* #include <soc/dpp/ARAD/arad_kbp_xpt.h>   */
#include <soc/dpp/ARAD/arad_link.h> 
#include <soc/dpp/ARAD/arad_mgmt.h> 
#include <soc/dpp/ARAD/arad_multicast_fabric.h> 
#include <soc/dpp/ARAD/arad_nif.h> 
#include <soc/dpp/ARAD/arad_ofp_rates.h> 
#include <soc/dpp/ARAD/arad_parser.h> 
#include <soc/dpp/ARAD/arad_pmf_low_level.h> 
#include <soc/dpp/ARAD/arad_pmf_low_level_ce.h> 
#include <soc/dpp/ARAD/arad_pmf_low_level_db.h> 
#include <soc/dpp/ARAD/arad_pmf_low_level_fem_tag.h> 
#include <soc/dpp/ARAD/arad_pmf_low_level_pgm.h> 
#include <soc/dpp/ARAD/arad_pmf_prog_select.h> 
#include <soc/dpp/ARAD/arad_ports.h> 
#include <soc/dpp/ARAD/arad_reg_access.h> 
#include <soc/dpp/ARAD/arad_scheduler_device.h> 
#include <soc/dpp/ARAD/arad_scheduler_element_converts.h> 
#include <soc/dpp/ARAD/arad_scheduler_end2end.h> 
#include <soc/dpp/ARAD/arad_scheduler_flow_converts.h> 
#include <soc/dpp/ARAD/arad_scheduler_flows.h> 
#include <soc/dpp/ARAD/arad_scheduler_ports.h> 
#include <soc/dpp/ARAD/arad_serdes.h> 
#include <soc/dpp/ARAD/arad_stat.h> 
#include <soc/dpp/ARAD/arad_stat_if.h> 
#include <soc/dpp/ARAD/arad_sw_db.h> 
#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h> 
#include <soc/dpp/ARAD/arad_tbl_access.h> 
#include <soc/dpp/ARAD/arad_tcam.h> 
#include <soc/dpp/ARAD/arad_tcam_key.h> 
#include <soc/dpp/ARAD/arad_tcam_mgmt.h> 
#include <soc/dpp/ARAD/arad_tdm.h> 
#include <soc/dpp/cosq.h> 
#include <soc/dpp/debug.h> 
#include <soc/dpp/dpp_defs.h> 
#include <soc/dpp/dpp_fabric_cell.h> 
#include <soc/dpp/drv.h> 
#include <soc/dpp/fabric.h> 
#include <soc/dpp/headers.h> 
#include <soc/dpp/mbcm.h> 
#include <soc/dpp/pkt.h> 
#include <soc/dpp/port_map.h>
#include <soc/dpp/soc_dpp_i2c.h> 
/* #include <soc/dpp/wb_db_hash_tbl.h>  */
#include <soc/dpp/ARAD/NIF/common_drv.h> 
#include <soc/dpp/ARAD/NIF/ilkn_drv.h> 
#include <soc/dpp/port_sw_db.h> 
#include <soc/dpp/ARAD/NIF/ports_manager.h> 
/* #include <soc/dpp/PCP/pcp_api_diagnostics.h>            */
/* #include <soc/dpp/PCP/pcp_api_framework.h>              */
/* #include <soc/dpp/PCP/pcp_api_frwrd_ilm.h>              */
/* #include <soc/dpp/PCP/pcp_api_frwrd_ipv4.h>             */
/* #include <soc/dpp/PCP/pcp_api_frwrd_mact.h>             */
/* #include <soc/dpp/PCP/pcp_api_frwrd_mact_mgmt.h>        */
/* #include <soc/dpp/PCP/pcp_api_mgmt.h>                   */
/* #include <soc/dpp/PCP/pcp_api_statistics.h>             */
/* #include <soc/dpp/PCP/pcp_api_tbl_access.h>             */
/* #include <soc/dpp/PCP/pcp_array_memory_allocator.h>     */
/* #include <soc/dpp/PCP/pcp_chip_defines.h>               */
/* #include <soc/dpp/PCP/pcp_chip_regs.h>                  */
/* #include <soc/dpp/PCP/pcp_chip_tbls.h>                  */
/* #include <soc/dpp/PCP/pcp_diagnostics.h>                */
/* #include <soc/dpp/PCP/pcp_framework.h>                  */
/* #include <soc/dpp/PCP/pcp_frwrd_ilm.h>                  */
/* #include <soc/dpp/PCP/pcp_frwrd_ipv4.h>                 */
/* #include <soc/dpp/PCP/pcp_frwrd_ipv4_lpm_mngr.h>        */
/* #include <soc/dpp/PCP/pcp_frwrd_ipv4_test.h>            */
/* #include <soc/dpp/PCP/pcp_frwrd_mact.h>                 */
/* #include <soc/dpp/PCP/pcp_frwrd_mact_mgmt.h>            */
/* #include <soc/dpp/PCP/pcp_general.h>                    */
/* #include <soc/dpp/PCP/pcp_init.h>                       */
/* #include <soc/dpp/PCP/pcp_lem_access.h>                 */
/* #include <soc/dpp/PCP/pcp_mgmt.h>                       */
/* #include <soc/dpp/PCP/pcp_oam_api_bfd.h>                */
/* #include <soc/dpp/PCP/pcp_oam_api_eth.h>                */
/* #include <soc/dpp/PCP/pcp_oam_api_general.h>            */
/* #include <soc/dpp/PCP/pcp_oam_api_mpls.h>               */
/* #include <soc/dpp/PCP/pcp_oam_bfd.h>                    */
/* #include <soc/dpp/PCP/pcp_oam_eth.h>                    */
/* #include <soc/dpp/PCP/pcp_oam_general.h>                */
/* #include <soc/dpp/PCP/pcp_oam_mpls.h>                   */
/* #include <soc/dpp/PCP/pcp_reg_access.h>                 */
/* #include <soc/dpp/PCP/pcp_statistics.h>                 */
/* #include <soc/dpp/PCP/pcp_sw_db.h>                      */
/* #include <soc/dpp/PCP/pcp_tbl_access.h>                 */
#include <soc/dpp/PPC/ppc_api_acl.h> 
#include <soc/dpp/PPC/ppc_api_counting.h> 
#include <soc/dpp/PPC/ppc_api_diag.h> 
#include <soc/dpp/PPC/ppc_api_eg_ac.h> 
#include <soc/dpp/PPC/ppc_api_eg_encap.h> 
#include <soc/dpp/PPC/ppc_api_eg_filter.h> 
#include <soc/dpp/PPC/ppc_api_eg_mirror.h> 
#include <soc/dpp/PPC/ppc_api_eg_qos.h> 
#include <soc/dpp/PPC/ppc_api_eg_vlan_edit.h> 
#include <soc/dpp/PPC/ppc_api_fp.h> 
#include <soc/dpp/PPC/ppc_api_fp_egr.h> 
#include <soc/dpp/PPC/ppc_api_fp_fem.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_bmact.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_fcf.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_fec.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_ilm.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_ipv6.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_mact.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_mact_mgmt.h> 
#include <soc/dpp/PPC/ppc_api_frwrd_trill.h> 
#include <soc/dpp/PPC/ppc_api_general.h> 
#include <soc/dpp/PPC/ppc_api_l3_src_bind.h> 
#include <soc/dpp/PPC/ppc_api_lag.h> 
#include <soc/dpp/PPC/ppc_api_lif.h> 
#include <soc/dpp/PPC/ppc_api_lif_cos.h> 
#include <soc/dpp/PPC/ppc_api_lif_ing_vlan_edit.h> 
#include <soc/dpp/PPC/ppc_api_lif_table.h> 
#include <soc/dpp/PPC/ppc_api_llp_cos.h> 
#include <soc/dpp/PPC/ppc_api_llp_filter.h> 
#include <soc/dpp/PPC/ppc_api_llp_mirror.h> 
#include <soc/dpp/PPC/ppc_api_llp_parse.h> 
#include <soc/dpp/PPC/ppc_api_llp_sa_auth.h> 
#include <soc/dpp/PPC/ppc_api_llp_trap.h> 
#include <soc/dpp/PPC/ppc_api_llp_vid_assign.h> 
#include <soc/dpp/PPC/ppc_api_metering.h> 
#include <soc/dpp/PPC/ppc_api_mpls_term.h> 
#include <soc/dpp/PPC/ppc_api_mymac.h> 
#include <soc/dpp/PPC/ppc_api_oam.h> 
#include <soc/dpp/PPC/ppc_api_port.h> 
#include <soc/dpp/PPC/ppc_api_ptp.h> 
#include <soc/dpp/PPC/ppc_api_rif.h> 
#include <soc/dpp/PPC/ppc_api_trap_mgmt.h> 
#include <soc/dpp/PPC/ppc_api_vsi.h> 
#include <soc/dpp/PPD/ppd_api_acl.h> 
#include <soc/dpp/PPD/ppd_api_diag.h> 
#include <soc/dpp/PPD/ppd_api_eg_ac.h> 
#include <soc/dpp/PPD/ppd_api_eg_encap.h> 
#include <soc/dpp/PPD/ppd_api_eg_filter.h> 
#include <soc/dpp/PPD/ppd_api_eg_mirror.h> 
#include <soc/dpp/PPD/ppd_api_eg_qos.h> 
#include <soc/dpp/PPD/ppd_api_eg_vlan_edit.h> 
#include <soc/dpp/PPD/ppd_api_fp.h> 
#include <soc/dpp/PPD/ppd_api_framework.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_bmact.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_fcf.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_fec.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_ilm.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_ipv4.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_ipv6.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_mact.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_mact_mgmt.h> 
#include <soc/dpp/PPD/ppd_api_frwrd_trill.h> 
#include <soc/dpp/PPD/ppd_api_general.h> 
#include <soc/dpp/PPD/ppd_api_l3_src_bind.h> 
#include <soc/dpp/PPD/ppd_api_lag.h> 
#include <soc/dpp/PPD/ppd_api_lif.h> 
#include <soc/dpp/PPD/ppd_api_lif_cos.h> 
#include <soc/dpp/PPD/ppd_api_lif_ing_vlan_edit.h> 
#include <soc/dpp/PPD/ppd_api_lif_table.h> 
#include <soc/dpp/PPD/ppd_api_llp_cos.h> 
#include <soc/dpp/PPD/ppd_api_llp_filter.h> 
#include <soc/dpp/PPD/ppd_api_llp_mirror.h> 
#include <soc/dpp/PPD/ppd_api_llp_parse.h> 
#include <soc/dpp/PPD/ppd_api_llp_sa_auth.h> 
#include <soc/dpp/PPD/ppd_api_llp_trap.h> 
#include <soc/dpp/PPD/ppd_api_llp_vid_assign.h> 
#include <soc/dpp/PPD/ppd_api_metering.h> 
#include <soc/dpp/PPD/ppd_api_mpls_term.h> 
#include <soc/dpp/PPD/ppd_api_mymac.h> 
#include <soc/dpp/PPD/ppd_api_oam.h> 
#include <soc/dpp/PPD/ppd_api_port.h> 
#include <soc/dpp/PPD/ppd_api_ptp.h> 
#include <soc/dpp/PPD/ppd_api_rif.h> 
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h> 
#include <soc/dpp/PPD/ppd_api_vsi.h> 
#include <soc/dpp/SAND/Management/sand_api_ssr.h> 
#include <soc/dpp/SAND/Management/sand_callback_handles.h> 
#include <soc/dpp/SAND/Management/sand_chip_consts.h> 
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h> 
#include <soc/dpp/SAND/Management/sand_device_management.h> 
#include <soc/dpp/SAND/Management/sand_error_code.h> 
#include <soc/dpp/SAND/Management/sand_general_macros.h> 
#include <soc/dpp/SAND/Management/sand_general_params.h> 
#include <soc/dpp/SAND/Management/sand_low_level.h> 
#include <soc/dpp/SAND/Management/sand_module_management.h> 
#include <soc/dpp/SAND/Management/sand_ssr.h> 
#include <soc/dpp/SAND/SAND_FM/sand_cell.h> 
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h> 
#include <soc/dpp/SAND/SAND_FM/sand_indirect_access.h> 
#include <soc/dpp/SAND/SAND_FM/sand_link.h> 
#include <soc/dpp/SAND/SAND_FM/sand_mem_access.h> 
#include <soc/dpp/SAND/SAND_FM/sand_mem_access_callback.h> 
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h> 
#include <soc/dpp/SAND/SAND_FM/sand_trigger.h> 
#include <soc/dpp/SAND/SAND_FM/sand_user_callback.h> 
#include <soc/dpp/SAND/Utils/sand_64cnt.h> 
#include <soc/dpp/SAND/Utils/sand_array_memory_allocator.h> 
#include <soc/dpp/SAND/Utils/sand_code_hamming.h> 
#include <soc/dpp/SAND/Utils/sand_conv.h> 
#include <soc/dpp/SAND/Utils/sand_delta_list.h> 
#include <soc/dpp/SAND/Utils/sand_exact_match.h> 
#include <soc/dpp/SAND/Utils/sand_exact_match_hash.h> 
#include <soc/dpp/SAND/Utils/sand_footer.h> 
#include <soc/dpp/SAND/Utils/sand_framework.h> 
#include <soc/dpp/SAND/Utils/sand_group_member_list.h> 
#include <soc/dpp/SAND/Utils/sand_hashtable.h> 
#include <soc/dpp/SAND/Utils/sand_header.h> 
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h> 
#include <soc/dpp/SAND/Utils/sand_multi_set.h> 
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h> 
#include <soc/dpp/SAND/Utils/sand_os_interface.h> 
#include <soc/dpp/SAND/Utils/sand_pat_tree.h> 
#include <soc/dpp/SAND/Utils/sand_pp_mac.h> 
#include <soc/dpp/SAND/Utils/sand_rand.h> 
#include <soc/dpp/SAND/Utils/sand_sorted_list.h> 
#include <soc/dpp/SAND/Utils/sand_tcm.h> 
#include <soc/dpp/SAND/Utils/sand_trace.h> 
#include <soc/dpp/SAND/Utils/sand_u64.h> 
#include <soc/dpp/SAND/Utils/sand_workload_status.h> 
#include <soc/dpp/TMC/tmc_api_action_cmd.h> 
#include <soc/dpp/TMC/tmc_api_callback.h> 
#include <soc/dpp/TMC/tmc_api_cell.h> 
#include <soc/dpp/TMC/tmc_api_cnm.h> 
#include <soc/dpp/TMC/tmc_api_cnt.h> 
#include <soc/dpp/TMC/tmc_api_debug.h> 
#include <soc/dpp/TMC/tmc_api_default_section.h> 
#include <soc/dpp/TMC/tmc_api_diagnostics.h> 
#include <soc/dpp/TMC/tmc_api_dram.h> 
#include <soc/dpp/TMC/tmc_api_egr_acl.h> 
#include <soc/dpp/TMC/tmc_api_egr_queuing.h> 
#include <soc/dpp/TMC/tmc_api_end2end_scheduler.h> 
#include <soc/dpp/TMC/tmc_api_fabric.h> 
#include <soc/dpp/TMC/tmc_api_flow_control.h> 
#include <soc/dpp/TMC/tmc_api_framework.h> 
#include <soc/dpp/TMC/tmc_api_general.h> 
#include <soc/dpp/TMC/tmc_api_header_parsing_utils.h> 
#include <soc/dpp/TMC/tmc_api_ingress_packet_queuing.h> 
#include <soc/dpp/TMC/tmc_api_ingress_scheduler.h> 
#include <soc/dpp/TMC/tmc_api_ingress_traffic_mgmt.h> 
#include <soc/dpp/TMC/tmc_api_mgmt.h> 
#include <soc/dpp/TMC/tmc_api_multicast_egress.h> 
#include <soc/dpp/TMC/tmc_api_multicast_fabric.h> 
#include <soc/dpp/TMC/tmc_api_multicast_ingress.h> 
#include <soc/dpp/TMC/tmc_api_ofp_rates.h> 
#include <soc/dpp/TMC/tmc_api_packet.h> 
#include <soc/dpp/TMC/tmc_api_pmf_low_level_ce.h> 
#include <soc/dpp/TMC/tmc_api_pmf_low_level_db.h> 
#include <soc/dpp/TMC/tmc_api_pmf_low_level_fem_tag.h> 
#include <soc/dpp/TMC/tmc_api_pmf_low_level_pgm.h> 
#include <soc/dpp/TMC/tmc_api_ports.h> 
#include <soc/dpp/TMC/tmc_api_reg_access.h> 
#include <soc/dpp/TMC/tmc_api_ssr.h> 
#include <soc/dpp/TMC/tmc_api_stack.h> 
#include <soc/dpp/TMC/tmc_api_stat_if.h> 
#include <soc/dpp/TMC/tmc_api_statistics.h> 
#include <soc/dpp/TMC/tmc_api_tcam.h> 
#include <soc/dpp/TMC/tmc_api_tcam_key.h> 
#include <soc/dpp/TMC/tmc_api_tdm.h> 
/* #include <appl/dfe/interrupts/interrupt_handler.h> 
#include <appl/dfe/interrupts/interrupt_handler_cb_func.h> 
#include <appl/dfe/interrupts/interrupt_handler_corr_act_func.h> 
#include <appl/diag/dfe/utils_fe1600_card.h> 
#include <soc/dfe/cmn/dfe_defs.h> 
#include <soc/dfe/cmn/dfe_drv.h> 
#include <soc/dfe/cmn/dfe_fabric.h> 
#include <soc/dfe/cmn/dfe_fabric_cell.h> 
#include <soc/dfe/cmn/dfe_fabric_cell_snake_test.h> 
#include <soc/dfe/cmn/dfe_fabric_source_routed_cell.h> 
#include <soc/dfe/cmn/dfe_interrupt.h> 
#include <soc/dfe/cmn/dfe_multicast.h> 
#include <soc/dfe/cmn/dfe_port.h> 
#include <soc/dfe/cmn/dfe_stack.h> 
#include <soc/dfe/cmn/dfe_stat.h> 
#include <soc/dfe/cmn/mbcm.h> 
#include <soc/dfe/cmn/dfe_diag.h> 
#include <soc/dfe/fe1600/fe1600_defs.h> 
#include <soc/dfe/fe1600/fe1600_drv.h> 
#include <soc/dfe/fe1600/fe1600_fabric_cell.h> 
#include <soc/dfe/fe1600/fe1600_fabric_cell_snake_test.h> 
#include <soc/dfe/fe1600/fe1600_fabric_flow_control.h> 
#include <soc/dfe/fe1600/fe1600_fabric_links.h> 
#include <soc/dfe/fe1600/fe1600_fabric_multicast.h> 
#include <soc/dfe/fe1600/fe1600_fabric_status.h> 
#include <soc/dfe/fe1600/fe1600_fabric_topology.h> 
#include <soc/dfe/fe1600/fe1600_interrupts.h> 
#include <soc/dfe/fe1600/fe1600_link.h> 
#include <soc/dfe/fe1600/fe1600_multicast.h> 
#include <soc/dfe/fe1600/fe1600_port.h> 
#include <soc/dfe/fe1600/fe1600_stack.h> 
#include <soc/dfe/fe1600/fe1600_stat.h> 
#include <soc/dfe/fe1600/fe1600_diag.h> */
#include <appl/dcmn/interrupts/interrupt_handler.h> 
#include <appl/dcmn/rx_los/rx_los.h> 
#include <appl/dcmn/rx_los/rx_los_db.h> 
#include <appl/diag/dcmn/bsp_cards_consts.h> 
#include <appl/diag/dcmn/cmdlist.h> 
#include <appl/diag/dcmn/counter.h> 
#include <appl/diag/dcmn/dcmn_patch_database.h> 
#include <appl/diag/dcmn/diag.h> 
#include <appl/diag/dcmn/gport.h> 
#include <appl/diag/dcmn/init.h> 
#include <appl/diag/dcmn/rate_calc.h> 
#include <appl/diag/dcmn/soc.h> 
#include <appl/diag/dcmn/utils_board_general.h> 
#include <appl/diag/dcmn/fabric.h> 
#include <soc/dcmn/dcmn_captured_buffer.h> 
#include <soc/dcmn/dcmn_cells_buffer.h> 
#include <soc/dcmn/dcmn_defs.h> 
#include <soc/dcmn/dcmn_error.h> 
#include <soc/dcmn/dcmn_fabric_cell.h> 
#include <soc/dcmn/dcmn_wb.h> 
#include <bcm_int/dpp/alloc_mngr.h>
#include <bcm_int/dpp/alloc_mngr_shr.h>
#include <bcm_int/dpp/allocator.h>
#include <bcm_int/dpp/bfd.h>
#include <bcm_int/dpp/cosq.h>
#include <bcm_int/dpp/counters.h>
#include <bcm_int/dpp/dpp_eyescan.h>
#include <bcm_int/dpp/error.h>
#include <bcm_int/dpp/fabric.h>
#include <bcm_int/dpp/failover.h>
#include <bcm_int/dpp/field.h>
#include <bcm_int/dpp/field_int.h>
#include <bcm_int/dpp/gport_mgmt.h>
#include <bcm_int/dpp/ipmc.h>
#include <bcm_int/dpp/l2.h>
#include <bcm_int/dpp/l3.h>
#include <bcm_int/dpp/link.h>
#include <bcm_int/dpp/mim.h>
#include <bcm_int/dpp/mirror.h>
#include <bcm_int/dpp/mpls.h>
#include <bcm_int/dpp/multicast.h>
#include <bcm_int/dpp/oam.h>
#include <bcm_int/dpp/policer.h>
#include <bcm_int/dpp/port.h>
#include <bcm_int/dpp/qos.h>
#include <bcm_int/dpp/rate.h>
#include <bcm_int/dpp/rx.h>
#include <bcm_int/dpp/stack.h>
#include <bcm_int/dpp/stat.h>
#include <bcm_int/dpp/stg.h>
#include <bcm_int/dpp/sw_db.h>
#include <bcm_int/dpp/switch.h>
#include <bcm_int/dpp/trill.h>
#include <bcm_int/dpp/trunk.h>
#include <bcm_int/dpp/tunnel.h>
#include <bcm_int/dpp/utils.h>
#include <bcm_int/dpp/vlan.h>
#include <bcm_int/dpp/vswitch.h>
#include <bcm_int/dpp/gport_mgmt_sw_db.h>
#include <soc/drvtypes.h>
#include <../src/appl/cint/cint_types.h> 
#include <bcm/time.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/shared/llm_appl.h>
#endif

extern shr_sw_state_t *sw_state[BCM_MAX_NUM_UNITS];
extern  ARAD_EGR_PROG_EDITOR_PROGRAM_INFO programs[SOC_MAX_NUM_DEVICES][ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS];
extern  int last_program_id[SOC_MAX_NUM_DEVICES];
extern  int last_program_pointer[SOC_MAX_NUM_DEVICES];


#define SW_STATE_DUMP_STRING_LENGTH_MAX 512

#define SW_STATE_DUMP_SAFE_STRCPY(dest, src) \
do { \
    if (sal_strlen(src) > (sizeof(dest)*8)) { \
        cli_out("   ERROR: Failed in strncopy\n"); \
        return; \
    } \
    sal_strncpy(dest, src, sal_strlen(src)+1);  \
} while (0)

#define SW_STATE_DUMP_SAFE_STRCAT(dest, src) \
do { \
    if (sal_strlen(dest)+sal_strlen(src) > (sizeof(dest)*8)) { \
        cli_out("   ERROR: Failed in sal_strncat\n"); \
        return; \
    } \
    sal_strncat(dest, src, sal_strlen(src)+1);  \
} while (0)

FILE *output_file;

void printArray(const int sizes[], int sizeIndex, int nbrSizes, char* variableName, void** arrayVariable, void (*print_cb)(void*, char*), int pointerNbr, int member_size);
void CHAR_SW_DUMP_PRINT(void* element, char* variableName); 
void DOUBLE_SW_DUMP_PRINT(void* element, char* variableName); 
void FLOAT_SW_DUMP_PRINT(void* element, char* variableName); 
void INT_SW_DUMP_PRINT(void* element, char* variableName); 
void LONG_SW_DUMP_PRINT(void* element, char* variableName); 
void SHORT_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_CHAR_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_INT_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_SHORT_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_LONG_SW_DUMP_PRINT(void* element, char* variableName); 
void UNSIGNED_LONG_LONG_SW_DUMP_PRINT(void* element, char* variableName); 
void SIGNED_CHAR_SW_DUMP_PRINT(void* element, char* variableName); 
void SIGNED_SHORT_SW_DUMP_PRINT(void* element, char* variableName); 
void SIGNED_INT_SW_DUMP_PRINT(void* element, char* variableName); 
void SIGNED_SW_DUMP_PRINT(void* element, char* variableName); 
void SIGNED_LONG_SW_DUMP_PRINT(void* element, char* variableName); 
void uint8_SW_DUMP_PRINT(void* element, char* variableName); 
void uint16_SW_DUMP_PRINT(void* element, char* variableName); 
void uint32_SW_DUMP_PRINT(void* element, char* variableName); 
void uint64_SW_DUMP_PRINT(void* element, char* variableName); 
void int8_SW_DUMP_PRINT(void* element, char* variableName); 
void int16_SW_DUMP_PRINT(void* element, char* variableName); 
void int32_SW_DUMP_PRINT(void* element, char* variableName); 
void int64_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_presel_set_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_datafield_bits_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_qual_set_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_action_set_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_NODE_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_T_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_INTERRUPTS_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DATABASE_STAGE_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_framesync_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_framesync_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_IPV4_SUBNET_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_CHIP_DEFINITIONS_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_atomic_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_parameter_desc_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_function_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_struct_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_enum_map_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_enum_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_constants_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_function_pointer_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_custom_iterator_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_custom_macro_t_SW_DUMP_PRINT(void* element, char* variableName); 
void cint_data_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_pool_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void shr_res_allocator_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_template_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void shr_template_manage_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_cosq_res_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_cosq_pool_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_cosq_pool_entry_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_cosq_pool_ref_t_SW_DUMP_PRINT(void* element, char* variableName); 
void endpoint_list_t_SW_DUMP_PRINT(void* element, char* variableName); 
void endpoint_list_member_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_BFD_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_gport_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_voq_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_connector_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_se_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_list_hd_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_hdlist_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_cal_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_rx_cal_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_cosq_tx_cal_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_fabric_vsq_category_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_device_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_type_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_map_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_chain_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_presel_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_device_stage_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_stage_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_device_st_mapping_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_PREDEFINED_ACL_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_device_group_mode_bits_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_group_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_entry_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DATABASE_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_device_range_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_qualify_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_range_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_range_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_dq_idx_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_qset_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_field_aset_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_res_unit_desc_t_SW_DUMP_PRINT(void *element, char *variableName); 
void _shr_res_type_desc_t_SW_DUMP_PRINT(void *element, char *variableName); 
void _shr_res_pool_desc_t_SW_DUMP_PRINT(void *element, char *variableName); 
void _bcm_dpp_field_group_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_entry_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_entry_common_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_qual_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_tc_p_act_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_tc_b_act_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_entry_dir_ext_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_de_act_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_QUAL_VAL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_U64_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DIR_EXTR_ACTION_VAL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DIR_EXTR_ACTION_LOC_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_field_profile_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_scache_handle_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_arad_field_device_qual_info_layer_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_lif_match_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_lif_type_e_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_port_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_petra_tpid_profile_t_SW_DUMP_PRINT(void* element, char* variableName); 

void bcm_dpp_qos_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_l3_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_l3_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_policer_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_policer_group_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_policer_group_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_stg_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_stg_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_vlan_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_vswitch_bookkeeping_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_petra_mirror_unit_data_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_petra_mirror_data_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PB_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_ACTION_CMD_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_DEST_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_DEST_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_trill_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_AC_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_petra_trill_port_list_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_MAC_ADDRESS_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_LLP_SA_AUTH_MAC_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_VLAN_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_SYS_PORT_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_SYS_PORT_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_KEY_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_DECISION_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_DECISION_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_DECISION_TYPE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_EEI_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_EEI_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_EEI_VAL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_TRILL_DEST_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_MPLS_COMMAND_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_MPLS_LABEL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_MPLS_TUNNEL_MODEL_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_PKT_FRWRD_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_PKT_FRWRD_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_ISID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OUTLIF_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OUTLIF_ENCODE_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_ACTION_PROFILE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_CODE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_petra_l2_freeze_t_SW_DUMP_PRINT(void* element, char* variableName); 
void l2_data_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_petra_l2_cb_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_mac_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_l2_addr_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_trunk_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_cos_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_fabric_distribution_t_SW_DUMP_PRINT(void* element, char* variableName); 
void llm_appl_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_mpls_bookkeeping_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_ILM_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_MPLS_EXP_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_PORT_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_RIF_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OAM_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OAM_UPMEP_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OAM_MIRROR_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_OAM_CPU_TRAP_CODE_TO_MIRROR_PROFILE_MAP_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_bfd_event_types_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_bfd_event_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_bfd_endpoint_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_oam_event_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_oam_group_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_oam_endpoint_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_oam_group_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_oam_group_fault_alarm_defect_priority_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_port_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_port_map_type_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_petra_port_tpid_info_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_TC_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_DP_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_ETHERNET_DA_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_l3_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName); 
void fec_to_ecmps_t_SW_DUMP_PRINT(void* element, char* variableName); 
void ecmp_size_t_SW_DUMP_PRINT(void* element, char* variableName); 
void fec_copy_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_mirror_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_switch_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName); 
void dos_attack_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void multi_device_sync_flags_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_time_interface_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_time_if_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_time_capture_t_SW_DUMP_PRINT(void* element, char* variableName); 
void trunk_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void trunk_init_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_pkt_blk_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_color_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_multicast_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_pkt_stk_forward_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_if_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_rx_reasons_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_rx_reasons_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_rx_reason_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_vlan_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void sal_time_t_SW_DUMP_PRINT(void* element, char* variableName); 
void sbusdma_desc_handle_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_vlan_unit_state_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_vlan_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void fid_ref_count_t_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_LIF_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_res_arad_pool_egress_encap_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_res_arad_pool_cosq_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_res_pool_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _dpp_template_pool_t_SW_DUMP_PRINT(void* element, char* variableName); 
void counters_cntl_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_trunk_private_t_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ISEM_ACCESS_PROGRAM_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_reg_above_64_val_t_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_DEVICE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_MGMT_IPV4_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_MGMT_P2P_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PP_ETHER_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_LLP_FILTER_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_MULTI_SET_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_HASH_TABLE_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_HASH_TABLE_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_HASH_TABLE_T_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_LLP_VID_ASSIGN_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PON_DOUBLE_LOOKUP_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_IPV4_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_IPV4_LPM_MNGR_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_IPV4_LPM_MNGR_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_IPV4_LPM_PXX_MODEL_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ARR_MEM_ALLOCATOR_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ARR_MEM_ALLOCATOR_PTR_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ARR_MEM_ALLOCATOR_T_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_MEMBER_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_GROUP_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_GROUP_MEM_LL_T_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_NODE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_PAT_TREE_KEY_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_IPV4_LPM_MNGR_T_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_IPV4_LPM_FREE_LIST_ITEM_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_ILM_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_FWD_MACT_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_MACT_LEARNING_MODE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_LIF_COS_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_LIF_TABLE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_FEC_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FRWRD_FEC_GLBL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_DIAG_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_DIAG_MODE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_LAG_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_HASH_MASKS_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_VRRP_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_HEADER_DATA_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MAP_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_INDIRECT_MODULE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_INDIRECT_MODULE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_INDIRECT_MODULE_BITS_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_INDIRECT_TABLES_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_INDIRECT_MEMORY_MAP_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_RET_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_DELTA_LIST_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_DELTA_LIST_NODE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_DELTA_LIST_STATISTICS_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_DEVICE_DESC_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_DEVICE_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_LL_TIMER_FUNCTION_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_RAND_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_DEVICE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_OP_MODE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_MGMT_TDM_MODE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_MGMT_TDM_MODE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_LAGS_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TDM_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_INTERFACE_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_INTERFACE_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_DEV_EGR_PORT_PRIORITY_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_DEV_RATE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_DEV_EGR_TCG_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_DEV_EGR_RATE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_OFP_RATES_EGQ_CHAN_ARB_ID_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_EGR_PROG_TM_PORT_PROFILE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_MULTICAST_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_FREE_ENTRIES_BLOCKS_REGION_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_FREE_ENTRIES_BLOCK_SIZE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_FREE_ENTRIES_BLOCK_LIST_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_CELL_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_DRAM_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_SORTED_LIST_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_OCC_BM_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_BANK_ENTRY_SIZE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_TCAM_BANK_ENTRY_SIZE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_ACTION_SIZE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_TCAM_ACTION_SIZE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_DB_PRIO_MODE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_SMALL_BANKS_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_PREFIX_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_SORTED_LIST_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_SAND_SORTED_LIST_T_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_TCAM_MGMT_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_TCAM_ACCESS_PROFILE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_TCAM_BANK_OWNER_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_TCAM_BANK_OWNER_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_TCAM_MANAGED_BANK_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_VSI_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_SRC_BIND_IPV6_STATIC_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_FES_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_FEM_ENTRY_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_RESOURCE_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_DB_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_PSL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_PSL_LEVEL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_PSL_LINE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_SEL_GROUP_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_SEL_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_PSL_TYPE_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_FP_DATABASE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_FP_ENTRY_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_PPC_PMF_PFG_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_QUAL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_HEADER_QUAL_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_IRPP_INFO_FIELD_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_SUB_HEADER_SW_DUMP_PRINT(void* element, char* variableName); 
void SOC_TMC_PMF_CE_SUB_HEADER_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_IRPP_QUALIFIER_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_CNT_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_INTERRUPT_DATA_SW_DUMP_PRINT(void* element, char* variableName); 
void arad_interrupt_type_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SYSPORT_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_SW_DB_TM_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_vlan_translate_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _bcm_dpp_vlan_translate_tag_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_vlan_tpid_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_egress_encap_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_ingress_lif_t_SW_DUMP_PRINT(void* element, char* variableName); 
void bcm_dpp_am_sync_lif_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_dpp_port_map_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_dpp_port_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_driver_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_chip_types_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_reg_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_block_types_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_regtype_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_field_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_field_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_reg_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_reg_above_64_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_reg_array_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_mem_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_mem_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_mem_array_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_block_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_cmap_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_ctr_ref_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_inband_control_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_inband_control_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_global_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_global_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_framesync_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_syncout_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_syncout_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_syncout_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_phy_timesync_mpls_label_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_control_phy_timesync_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_control_phy_timesync_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_vm_entry_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_vm_qual_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_mcrep_control_flag_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_field_qualify_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_field_action_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_fp_tcam_checksum_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_policer_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_policer_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_field_stat_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_wred_config_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_wred_control_t_SW_DUMP_PRINT(void* element, char* variableName); 
void drv_wred_counter_t_SW_DUMP_PRINT(void* element, char* variableName); 
void ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_logical_port_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_phy_port_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_if_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_if_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_encap_mode_t_SW_DUMP_PRINT(void* element, char* variableName); 
void _shr_port_encap_t_SW_DUMP_PRINT(void* element, char* variableName); 
void soc_port_unit_info_t_SW_DUMP_PRINT(void* element, char* variableName); 
void printArray(const int sizes[], int sizeIndex, int nbrSizes, char* variableName, void** arrayVariable, void (*print_cb)(void*, char*), int pointerNbr, int member_size) { 
   char printNameBuffer[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   char arrayIndexCharacters[50]; 
   int i;  
   void** p = arrayVariable;
   void* q;
   int nof_members, elt_size;
   nof_members = 1;

   /* Compute in case of multi-dimension how many members to jump in this dimension */
   for (i= sizeIndex + 1; i<nbrSizes; i++) {
       nof_members = nof_members * sizes[i];
   }
   elt_size = member_size * nof_members;

   for (i= 0; i<sizes[sizeIndex]; i++) {
       if (i != 0) {
          if (pointerNbr == 0) {
             p += (elt_size / sizeof(void *));
          }
          else {
             /* Array of pointers - go to next element */
             p += 1;
          }
       }
       switch (pointerNbr) {
       case 0:
          q = (void*) &(*p);
           break;
       case 1:
          q = *p;
          break;
       case 2:
          q = (void*) *((void**) *p);
           break;
       case 3:
       default:
          q = (void*) **((void***) *p);
           break;
       }
       if (q == NULL) {
           continue;
       }
       if (SHR_BITNULL_RANGE(q, 0, elt_size*8)){
          continue;
       }
       SW_STATE_DUMP_SAFE_STRCPY(printNameBuffer, variableName);  
       SW_STATE_DUMP_SAFE_STRCAT(printNameBuffer, "[");  
       sal_snprintf(arrayIndexCharacters, 50, "%d", i);  
       SW_STATE_DUMP_SAFE_STRCAT(printNameBuffer, arrayIndexCharacters);  
       SW_STATE_DUMP_SAFE_STRCAT(printNameBuffer, "]");  
       /* array is multidimensional */ 
       if (sizeIndex < nbrSizes -1) { 
          printArray(sizes, sizeIndex + 1, nbrSizes, printNameBuffer, (void**) p, print_cb, pointerNbr, member_size); 
       } 
        /* array is one-dimensional, print its elements */ 
       else { 
          print_cb(q, printNameBuffer); 
       } 
   }  
}  
void CHAR_SW_DUMP_PRINT(void* element, char* variableName) { 
   char elt = *((char*) element); 
   sal_fprintf(output_file, "%s: %c\n", variableName, elt); 
} 
void DOUBLE_SW_DUMP_PRINT(void* element, char* variableName) { 
   double elt = *((double*) element); 
   sal_fprintf(output_file, "%s: %G\n", variableName, elt); 
} 
void FLOAT_SW_DUMP_PRINT(void* element, char* variableName) { 
   float elt = *((float*) element); 
   sal_fprintf(output_file, "%s: %G\n", variableName, elt); 
} 
void INT_SW_DUMP_PRINT(void* element, char* variableName) { 
   int elt = *((int*) element); 
   sal_fprintf(output_file, "%s: %d\n", variableName, elt); 
} 
void LONG_SW_DUMP_PRINT(void* element, char* variableName) { 
   long elt = *((long*) element); 
   sal_fprintf(output_file, "%s: %li\n", variableName, elt); 
} 
void SHORT_SW_DUMP_PRINT(void* element, char* variableName) { 
   short elt = *((short*) element); 
   sal_fprintf(output_file, "%s: %hi\n", variableName, elt); 
} 
void UNSIGNED_CHAR_SW_DUMP_PRINT(void* element, char* variableName) { 
   unsigned char elt = *((unsigned char*) element); 
   sal_fprintf(output_file, "%s: %u\n", variableName, elt); 
} 
void UNSIGNED_INT_SW_DUMP_PRINT(void* element, char* variableName) { 
   unsigned int elt = *((unsigned int*) element); 
   sal_fprintf(output_file, "%s: %u\n", variableName, elt); 
} 
void UNSIGNED_SW_DUMP_PRINT(void* element, char* variableName) { 
   unsigned elt = *((unsigned*) element); 
   sal_fprintf(output_file, "%s: %u\n", variableName, elt); 
} 
void UNSIGNED_SHORT_SW_DUMP_PRINT(void* element, char* variableName) { 
   unsigned short elt = *((unsigned short*) element); 
   sal_fprintf(output_file, "%s: %u\n", variableName, elt); 
} 
void UNSIGNED_LONG_SW_DUMP_PRINT(void* element, char* variableName) { 
   unsigned long elt = *((unsigned long*) element); 
   sal_fprintf(output_file, "%s: %lu\n", variableName, elt); 
} 
void UNSIGNED_LONG_LONG_SW_DUMP_PRINT(void* element, char* variableName) { 
   uint64 elt = *((uint64*) element); 
   sal_fprintf(output_file, "%s: %08x%08x\n", variableName, COMPILER_64_HI(elt), COMPILER_64_LO(elt));
} 
void SIGNED_CHAR_SW_DUMP_PRINT(void* element, char* variableName) { 
   signed char elt = *((signed char*) element); 
   sal_fprintf(output_file, "%s: %c\n", variableName, elt); 
} 
void SIGNED_SHORT_SW_DUMP_PRINT(void* element, char* variableName) { 
   signed short elt = *((signed short*) element); 
   sal_fprintf(output_file, "%s: %hi\n", variableName, elt); 
} 
void SIGNED_INT_SW_DUMP_PRINT(void* element, char* variableName) { 
   signed int elt = *((signed int*) element); 
   sal_fprintf(output_file, "%s: %d\n", variableName, elt); 
} 
void SIGNED_SW_DUMP_PRINT(void* element, char* variableName) { 
   signed elt = *((signed*) element); 
   sal_fprintf(output_file, "%s: %d\n", variableName, elt); 
} 
void SIGNED_LONG_SW_DUMP_PRINT(void* element, char* variableName) { 
   signed long elt = *((signed long*) element); 
   sal_fprintf(output_file, "%s: %li\n", variableName, elt); 
} 
void uint8_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_CHAR_SW_DUMP_PRINT(element, variableName); 
} 
void uint16_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_SHORT_SW_DUMP_PRINT(element, variableName); 
} 
void uint32_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_INT_SW_DUMP_PRINT(element, variableName); 
} 
void uint64_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_LONG_SW_DUMP_PRINT(element, variableName); 
} 
void int8_SW_DUMP_PRINT(void* element, char* variableName){ 
   SIGNED_CHAR_SW_DUMP_PRINT(element, variableName); 
} 
void int16_SW_DUMP_PRINT(void* element, char* variableName){ 
   SIGNED_SHORT_SW_DUMP_PRINT(element, variableName); 
} 
void int32_SW_DUMP_PRINT(void* element, char* variableName){ 
   SIGNED_INT_SW_DUMP_PRINT(element, variableName); 
} 
void int64_SW_DUMP_PRINT(void* element, char* variableName){ 
   SIGNED_LONG_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_field_presel_set_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_field_presel_set_t *elt = ((bcm_field_presel_set_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".w"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( BCM_FIELD_PRESEL_SEL_MAX ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->w, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void _bcm_dpp_field_datafield_bits_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_qual_set_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_action_set_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PAT_TREE_NODE_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PAT_TREE_NODE_KEY *elt = ((SOC_SAND_PAT_TREE_NODE_KEY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".val"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->val), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prefix"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->prefix), variableNameWithPath); 
} 
void SOC_SAND_PAT_TREE_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PAT_TREE_T *elt = ((SOC_SAND_PAT_TREE_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".root"); 
   SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT((void*) &(elt->root), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tree_memory"); 
   /* check the pointer is not null */ 
   if (elt->tree_memory) { 
      SOC_SAND_PAT_TREE_NODE_SW_DUMP_PRINT((void*) &(*elt->tree_memory), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cache_enabled"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->cache_enabled), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tree_memory_cache"); 
   /* check the pointer is not null */ 
   if (elt->tree_memory_cache) { 
      SOC_SAND_PAT_TREE_NODE_SW_DUMP_PRINT((void*) &(*elt->tree_memory_cache), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".root_cache"); 
   SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT((void*) &(elt->root_cache), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cache_change_head"); 
   SOC_SAND_PAT_TREE_NODE_KEY_SW_DUMP_PRINT((void*) &(elt->cache_change_head), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".current_node_place"); 
   SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT((void*) &(elt->current_node_place), variableNameWithPath); 
} 
void ARAD_INTERRUPTS_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_INTERRUPTS *elt = ((ARAD_INTERRUPTS*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".interrupt_data"); 
   arraySizes_SW_DUMP[0]=ARAD_INT_LAST; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->interrupt_data, ARAD_INTERRUPT_DATA_SW_DUMP_PRINT, 0, sizeof(ARAD_INTERRUPT_DATA)); 
} 
void _shr_port_phy_timesync_framesync_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _shr_port_phy_timesync_framesync_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _shr_port_phy_timesync_framesync_t *elt = ((_shr_port_phy_timesync_framesync_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode"); 
   _shr_port_phy_timesync_framesync_mode_t_SW_DUMP_PRINT((void*) &(elt->mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".length_threshold"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->length_threshold), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".event_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->event_offset), variableNameWithPath); 
} 
void _shr_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_port_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PP_IPV4_SUBNET_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PP_IPV4_SUBNET *elt = ((SOC_SAND_PP_IPV4_SUBNET*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ip_address"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ip_address), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prefix_len"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->prefix_len), variableNameWithPath); 
} 
void ARAD_CHIP_DEFINITIONS_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_CHIP_DEFINITIONS *elt = ((ARAD_CHIP_DEFINITIONS*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ticks_per_sec"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ticks_per_sec), variableNameWithPath); 
} 
void cint_atomic_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_atomic_type_t *elt = ((cint_atomic_type_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void cint_parameter_desc_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   cint_parameter_desc_t *elt = ((cint_parameter_desc_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*basetype"); 
   /* check the pointer is not null */ 
   if (elt->basetype) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->basetype), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pcount"); 
   INT_SW_DUMP_PRINT((void*) &(elt->pcount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".array"); 
   INT_SW_DUMP_PRINT((void*) &(elt->array), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".num_dimensions"); 
   INT_SW_DUMP_PRINT((void*) &(elt->num_dimensions), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dimensions"); 
   arraySizes_SW_DUMP[0]=CINT_CONFIG_ARRAY_DIMENSION_LIMIT; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->dimensions, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
} 
void cint_function_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_function_t *elt = ((cint_function_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*params"); 
   /* check the pointer is not null */ 
   if (elt->params) { 
      cint_parameter_desc_t_SW_DUMP_PRINT((void*) &(*elt->params), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_struct_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_struct_type_t *elt = ((cint_struct_type_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*struct_members"); 
   /* check the pointer is not null */ 
   if (elt->struct_members) { 
      cint_parameter_desc_t_SW_DUMP_PRINT((void*) &(*elt->struct_members), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_enum_map_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_enum_map_t *elt = ((cint_enum_map_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".value"); 
   INT_SW_DUMP_PRINT((void*) &(elt->value), variableNameWithPath); 
} 
void cint_enum_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_enum_type_t *elt = ((cint_enum_type_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*enum_map"); 
   /* check the pointer is not null */ 
   if (elt->enum_map) { 
      cint_enum_map_t_SW_DUMP_PRINT((void*) &(*elt->enum_map), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_constants_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   cint_enum_map_t_SW_DUMP_PRINT(element, variableName); 
} 
void cint_function_pointer_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_function_pointer_t *elt = ((cint_function_pointer_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*params"); 
   /* check the pointer is not null */ 
   if (elt->params) { 
      cint_parameter_desc_t_SW_DUMP_PRINT((void*) &(*elt->params), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_custom_iterator_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_custom_iterator_t *elt = ((cint_custom_iterator_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_custom_macro_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_custom_macro_t *elt = ((cint_custom_macro_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*name"); 
   /* check the pointer is not null */ 
   if (elt->name) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->name), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void cint_data_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   cint_data_t *elt = ((cint_data_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*atomics"); 
   /* check the pointer is not null */ 
   if (elt->atomics) { 
      cint_atomic_type_t_SW_DUMP_PRINT((void*) &(*elt->atomics), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*functions"); 
   /* check the pointer is not null */ 
   if (elt->functions) { 
      cint_function_t_SW_DUMP_PRINT((void*) &(*elt->functions), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*structures"); 
   /* check the pointer is not null */ 
   if (elt->structures) { 
      cint_struct_type_t_SW_DUMP_PRINT((void*) &(*elt->structures), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*enums"); 
   /* check the pointer is not null */ 
   if (elt->enums) { 
      cint_enum_type_t_SW_DUMP_PRINT((void*) &(*elt->enums), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*typedefs"); 
   /* check the pointer is not null */ 
   if (elt->typedefs) { 
      cint_parameter_desc_t_SW_DUMP_PRINT((void*) &(*elt->typedefs), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*constants"); 
   /* check the pointer is not null */ 
   if (elt->constants) { 
      cint_constants_t_SW_DUMP_PRINT((void*) &(*elt->constants), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fpointers"); 
   /* check the pointer is not null */ 
   if (elt->fpointers) { 
      cint_function_pointer_t_SW_DUMP_PRINT((void*) &(*elt->fpointers), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*iterators"); 
   /* check the pointer is not null */ 
   if (elt->iterators) { 
      cint_custom_iterator_t_SW_DUMP_PRINT((void*) &(*elt->iterators), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*macros"); 
   /* check the pointer is not null */ 
   if (elt->macros) { 
      cint_custom_macro_t_SW_DUMP_PRINT((void*) &(*elt->macros), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void bcm_dpp_am_pool_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_am_pool_info_t *elt = ((bcm_dpp_am_pool_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pool_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->pool_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   INT_SW_DUMP_PRINT((void*) &(elt->res_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".res_id"); 
   shr_res_allocator_t_SW_DUMP_PRINT((void*) &(elt->res_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".start"); 
   INT_SW_DUMP_PRINT((void*) &(elt->start), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_elements_per_allocation"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_elements_per_allocation), variableNameWithPath); 
} 
void shr_res_allocator_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_dpp_am_template_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_am_template_info_t *elt = ((bcm_dpp_am_template_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pool_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->pool_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   INT_SW_DUMP_PRINT((void*) &(elt->template_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".manager"); 
   shr_template_manage_t_SW_DUMP_PRINT((void*) &(elt->manager), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".start"); 
   INT_SW_DUMP_PRINT((void*) &(elt->start), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_entities"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_entities), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".data_size"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->data_size), variableNameWithPath); 
} 
void shr_template_manage_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_dpp_am_cosq_pool_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_dpp_am_cosq_pool_info_t *elt = ((bcm_dpp_am_cosq_pool_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_entries"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_entries), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid_entries"); 
   INT_SW_DUMP_PRINT((void*) &(elt->valid_entries), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*entries"); 
   arraySizes_SW_DUMP[0]=elt->max_entries; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->entries, bcm_dpp_am_cosq_pool_entry_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_am_cosq_pool_entry_t)); 
} 
void bcm_dpp_am_cosq_pool_entry_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_dpp_am_cosq_pool_entry_t *elt = ((bcm_dpp_am_cosq_pool_entry_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_valid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->is_valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_dynamic"); 
   INT_SW_DUMP_PRINT((void*) &(elt->is_dynamic), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pool_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->pool_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->type_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   INT_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".p_pool_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->p_pool_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".res"); 
   arraySizes_SW_DUMP[0]=SHR_BITALLOCSIZE ( DPP_DEVICE_COSQ_MAX_REGION_VAL + 1 ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->res, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".start"); 
   INT_SW_DUMP_PRINT((void*) &(elt->start), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
} 
void bcm_dpp_am_cosq_pool_ref_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_dpp_am_cosq_pool_ref_t *elt = ((bcm_dpp_am_cosq_pool_ref_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_entries"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_entries), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid_entries"); 
   INT_SW_DUMP_PRINT((void*) &(elt->valid_entries), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*entries"); 
   arraySizes_SW_DUMP[0]=elt->max_entries; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->entries, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
} 

void SOC_PPC_BFD_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_dpp_cosq_pfc_rx_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
}
void bcm_gport_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_dpp_cosq_voq_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_voq_config_t *elt = ((bcm_dpp_cosq_voq_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".num_cos"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->num_cos), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ref_cnt"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ref_cnt), variableNameWithPath); 
} 
void bcm_dpp_cosq_connector_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_connector_config_t *elt = ((bcm_dpp_cosq_connector_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".num_cos"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->num_cos), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".src_modid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->src_modid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ref_cnt"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ref_cnt), variableNameWithPath); 
} 
void bcm_dpp_cosq_se_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_se_config_t *elt = ((bcm_dpp_cosq_se_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ref_cnt"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ref_cnt), variableNameWithPath); 
} 
void bcm_dpp_cosq_hdlist_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_dpp_cosq_cal_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_cal_t *elt = ((bcm_dpp_cosq_cal_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx"); 
   bcm_dpp_cosq_rx_cal_t_SW_DUMP_PRINT((void*) &(elt->rx), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx"); 
   bcm_dpp_cosq_tx_cal_t_SW_DUMP_PRINT((void*) &(elt->tx), variableNameWithPath); 
} 
void bcm_dpp_cosq_rx_cal_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_rx_cal_t *elt = ((bcm_dpp_cosq_rx_cal_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".intf"); 
   INT_SW_DUMP_PRINT((void*) &(elt->intf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cal_mode_ndx"); 
   INT_SW_DUMP_PRINT((void*) &(elt->cal_mode_ndx), variableNameWithPath); 
} 
void bcm_dpp_cosq_tx_cal_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_cosq_tx_cal_t *elt = ((bcm_dpp_cosq_tx_cal_t*) element);

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".intf"); 
   INT_SW_DUMP_PRINT((void*) &(elt->intf), variableNameWithPath); 

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cal_mode_ndx"); 
   INT_SW_DUMP_PRINT((void*) &(elt->cal_mode_ndx), variableNameWithPath); 
} 
void bcm_fabric_vsq_category_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_field_info_OLD_t *elt = ((bcm_dpp_field_info_OLD_t*) element);

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".unit"); 
   INT_SW_DUMP_PRINT((void*) &(elt->unit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
/*
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".totalSize"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->totalSize), variableNameWithPath); 
*/

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".unitHandle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->unitHandle), variableNameWithPath); 

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*devInfo"); 
   /* check the pointer is not null */ 
   if (elt->devInfo) { 
      _bcm_dpp_field_device_info_t_SW_DUMP_PRINT((void*) &(*elt->devInfo), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryTcLimit"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryTcLimit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryExtTcLimit"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryExtTcLimit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groupLimit"); 
   _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT((void*) &(elt->groupLimit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryDeLimit"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryDeLimit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dqLimit"); 
   _bcm_dpp_field_dq_idx_t_SW_DUMP_PRINT((void*) &(elt->dqLimit), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rangeQualTypes"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->rangeQualTypes), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryIntTcCount"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryIntTcCount), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryIntTcFree"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryIntTcFree), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryDeFree"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryDeFree), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
*/
/*
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryExtTcCount"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryExtTcCount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
*/
/*
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryExtTcFree"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryExtTcFree), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
*/
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stageD"); 
   /* check the pointer is not null */ 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*groupD"); 
   /* check the pointer is not null */ 
   /*
   if (elt->groupD) { 
      _bcm_dpp_field_group_t_SW_DUMP_PRINT((void*) &(*elt->groupD), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   */ 

/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".unitFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->unitFlags), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dataFieldInUse"); 
   _bcm_dpp_field_datafield_bits_SW_DUMP_PRINT((void*) &(elt->dataFieldInUse), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dataFieldOfsBit"); 
   _bcm_dpp_field_datafield_bits_SW_DUMP_PRINT((void*) &(elt->dataFieldOfsBit), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dataFieldLenBit"); 
   _bcm_dpp_field_datafield_bits_SW_DUMP_PRINT((void*) &(elt->dataFieldLenBit), variableNameWithPath); 
*/
/*
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dataFieldRefs"); 
   arraySizes_SW_DUMP[0]=_BCM_DPP_NOF_FIELD_DATAS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->dataFieldRefs, UNSIGNED_INT_SW_DUMP_PRINT, 0, sizeof(unsigned int)); 
*/
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".wb"); 
} 
void _bcm_dpp_field_device_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_device_info_t *elt = ((_bcm_dpp_field_device_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stages"); 
   _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT((void*) &(elt->stages), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".types"); 
   _bcm_dpp_field_type_idx_t_SW_DUMP_PRINT((void*) &(elt->types), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mappings"); 
   _bcm_dpp_field_map_idx_t_SW_DUMP_PRINT((void*) &(elt->mappings), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualChain"); 
   _bcm_dpp_field_chain_idx_t_SW_DUMP_PRINT((void*) &(elt->qualChain), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".actChain"); 
   _bcm_dpp_field_chain_idx_t_SW_DUMP_PRINT((void*) &(elt->actChain), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".presels"); 
   _bcm_dpp_field_presel_idx_t_SW_DUMP_PRINT((void*) &(elt->presels), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cascadeKeyLimit"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->cascadeKeyLimit), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stage"); 
   /* check the pointer is not null */ 
   if (elt->stage) { 
      _bcm_dpp_field_device_stage_info_t_SW_DUMP_PRINT((void*) &(*elt->stage), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stMapInfo"); 
   /* check the pointer is not null */ 
   if (elt->stMapInfo) { 
      _bcm_dpp_field_device_st_mapping_t_SW_DUMP_PRINT((void*) &(*elt->stMapInfo), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ranges"); 
   /* check the pointer is not null */ 
   if (elt->ranges) { 
      _bcm_dpp_field_device_range_info_t_SW_DUMP_PRINT((void*) &(*elt->ranges), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*qualMap"); 
   /* check the pointer is not null */ 
   if (elt->qualMap) { 
      int32_SW_DUMP_PRINT((void*) &(*elt->qualMap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*actMap"); 
   /* check the pointer is not null */ 
   if (elt->actMap) { 
      int32_SW_DUMP_PRINT((void*) &(*elt->actMap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**typeNames"); 
   /* check the pointer is not null */ 
   if (elt->typeNames) { 
      CHAR_SW_DUMP_PRINT((void*) &(**elt->typeNames), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_type_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_map_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_chain_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_presel_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint16_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_device_stage_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
/*   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX];
   _bcm_dpp_field_device_stage_info_t *elt = ((_bcm_dpp_field_device_stage_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stageName"); 
    check the pointer is not null
   if (elt->stageName) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->stageName), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmStage"); 
   bcm_field_stage_t_SW_DUMP_PRINT((void*) &(elt->bcmStage), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwStageId"); 
   SOC_PPC_FP_DATABASE_STAGE_SW_DUMP_PRINT((void*) &(elt->hwStageId), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stageFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->stageFlags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxGroups"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->maxGroups), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxEntriesInternalTcam"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->maxEntriesInternalTcam), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxEntriesExternalTcam"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->maxEntriesExternalTcam), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxEntriesDe"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->maxEntriesDe), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxProgQuals"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->maxProgQuals), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryMaxQuals"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->entryMaxQuals), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryMaxActs"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->entryMaxActs), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryDeMaxQuals"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->entryDeMaxQuals), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sharesGroups"); 
   INT_SW_DUMP_PRINT((void*) &(elt->sharesGroups), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sharesEntriesTc"); 
   INT_SW_DUMP_PRINT((void*) &(elt->sharesEntriesTc), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sharesEntriesDe"); 
   INT_SW_DUMP_PRINT((void*) &(elt->sharesEntriesDe), variableNameWithPath); */
} 
void bcm_field_stage_t_SW_DUMP_PRINT(void* element, char* variableName){ 
/*   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); */
} 
void SOC_PPC_FP_DATABASE_STAGE_SW_DUMP_PRINT(void* element, char* variableName){ 
/*   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); */
} 
void _bcm_dpp_field_device_st_mapping_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_device_st_mapping_t *elt = ((_bcm_dpp_field_device_st_mapping_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stmStage"); 
   _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT((void*) &(elt->stmStage), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stmType"); 
   _bcm_dpp_field_type_idx_t_SW_DUMP_PRINT((void*) &(elt->stmType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".predefKey"); 
   SOC_PPC_FP_PREDEFINED_ACL_KEY_SW_DUMP_PRINT((void*) &(elt->predefKey), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stmFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->stmFlags), variableNameWithPath); 
} 
void SOC_PPC_FP_PREDEFINED_ACL_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_device_group_mode_bits_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_device_group_mode_bits_t *elt = ((_bcm_dpp_field_device_group_mode_bits_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage"); 
   _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT((void*) &(elt->stage), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode"); 
   bcm_field_group_mode_t_SW_DUMP_PRINT((void*) &(elt->mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".length"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->length), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualLength"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->qualLength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryCount"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->entryCount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryType"); 
   _bcm_dpp_field_entry_type_t_SW_DUMP_PRINT((void*) &(elt->entryType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwType"); 
   SOC_PPC_FP_DATABASE_TYPE_SW_DUMP_PRINT((void*) &(elt->hwType), variableNameWithPath); 
} 
void bcm_field_group_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_entry_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FP_DATABASE_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_device_range_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_device_range_info_t *elt = ((_bcm_dpp_field_device_range_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualifier"); 
   bcm_field_qualify_t_SW_DUMP_PRINT((void*) &(elt->qualifier), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rangeFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rangeFlags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rangeBase"); 
   bcm_field_range_t_SW_DUMP_PRINT((void*) &(elt->rangeBase), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   _bcm_dpp_field_range_idx_t_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
} 
void bcm_field_qualify_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_field_range_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_range_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_field_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_field_dq_idx_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_field_qset_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_field_qset_t *elt = ((bcm_field_qset_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".w"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( BCM_FIELD_QUALIFY_MAX ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->w, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".udf_map"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( BCM_FIELD_USER_NUM_UDFS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->udf_map, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void bcm_field_aset_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_field_aset_t *elt = ((bcm_field_aset_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".w"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( bcmFieldActionCount ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->w, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void _shr_res_unit_desc_t_SW_DUMP_PRINT(void *element, char *variableName){ 
    char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
    int arraySizes_SW_DUMP[4]; 
    _shr_res_unit_desc_t *elt = ((_shr_res_unit_desc_t*) element); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resTypeCount");
    uint16_SW_DUMP_PRINT((void*)&(elt->resTypeCount), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resPoolCount");
    uint16_SW_DUMP_PRINT((void*)&(elt->resPoolCount), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".res");
    arraySizes_SW_DUMP[0]=elt->resTypeCount; 
    arraySizes_SW_DUMP[1]=1; 
    printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->res, _shr_res_type_desc_t_SW_DUMP_PRINT, 1, sizeof(_shr_res_type_desc_t)); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pool");
    arraySizes_SW_DUMP[0]=elt->resPoolCount; 
    arraySizes_SW_DUMP[1]=1; 
    printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->pool, _shr_res_pool_desc_t_SW_DUMP_PRINT, 1, sizeof(_shr_res_pool_desc_t)); 
}
void _shr_res_type_desc_t_SW_DUMP_PRINT(void *element, char *variableName){ 
    char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
    int arraySizes_SW_DUMP[4]; 
    _shr_res_type_desc_t *elt = ((_shr_res_type_desc_t*) element); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resPoolId");
    INT_SW_DUMP_PRINT((void*)&(elt->resPoolId), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resElemSize");
    INT_SW_DUMP_PRINT((void*)&(elt->resElemSize), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".refCount");
    INT_SW_DUMP_PRINT((void*)&(elt->refCount), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".name");
    arraySizes_SW_DUMP[0]=1; 
    printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->name, CHAR_SW_DUMP_PRINT, 0, sizeof(char)); 
}
void _shr_res_pool_desc_t_SW_DUMP_PRINT(void *element, char *variableName){ 
    char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
    int arraySizes_SW_DUMP[4]; 
    _shr_res_pool_desc_t *elt = ((_shr_res_pool_desc_t*) element); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resManagerType");
    shr_res_allocator_t_SW_DUMP_PRINT((void*)&(elt->resManagerType), variableNameWithPath);
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".low");
    INT_SW_DUMP_PRINT((void*)&(elt->low), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count");
    INT_SW_DUMP_PRINT((void*)&(elt->count), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".refCount");
    INT_SW_DUMP_PRINT((void*)&(elt->refCount), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".inuse");
    INT_SW_DUMP_PRINT((void*)&(elt->inuse), variableNameWithPath); 
    SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".name");
    arraySizes_SW_DUMP[0]=1; 
    printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->name, CHAR_SW_DUMP_PRINT, 0, sizeof(char)); 
}
void _bcm_dpp_field_group_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_group_t *elt = ((_bcm_dpp_field_group_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qset"); 
   bcm_field_qset_t_SW_DUMP_PRINT((void*) &(elt->qset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".aset"); 
   bcm_field_aset_t_SW_DUMP_PRINT((void*) &(elt->aset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".preselSet"); 
   bcm_field_presel_set_t_SW_DUMP_PRINT((void*) &(elt->preselSet), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".preselHw"); 
   bcm_field_presel_set_t_SW_DUMP_PRINT((void*) &(elt->preselHw), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pqset"); 
   _bcm_dpp_field_qual_set_t_SW_DUMP_PRINT((void*) &(elt->pqset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".paset"); 
   _bcm_dpp_field_action_set_t_SW_DUMP_PRINT((void*) &(elt->paset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groupTypes"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->groupTypes), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryHead"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryHead), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryTail"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryTail), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryCount"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryCount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groupNext"); 
   _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT((void*) &(elt->groupNext), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groupPrev"); 
   _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT((void*) &(elt->groupPrev), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cascadePair"); 
   _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT((void*) &(elt->cascadePair), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".priority"); 
   INT_SW_DUMP_PRINT((void*) &(elt->priority), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage"); 
   _bcm_dpp_field_stage_idx_t_SW_DUMP_PRINT((void*) &(elt->stage), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".predefKey"); 
   SOC_PPC_FP_PREDEFINED_ACL_KEY_SW_DUMP_PRINT((void*) &(elt->predefKey), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groupFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->groupFlags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".grpMode"); 
   bcm_field_group_mode_t_SW_DUMP_PRINT((void*) &(elt->grpMode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwHandle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hwHandle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".oaset"); 
   bcm_field_aset_t_SW_DUMP_PRINT((void*) &(elt->oaset), variableNameWithPath); 
} 
void _bcm_dpp_field_entry_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _bcm_dpp_field_entry_t *elt = ((_bcm_dpp_field_entry_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryCmn"); 
   _bcm_dpp_field_entry_common_t_SW_DUMP_PRINT((void*) &(elt->entryCmn), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcActP"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->tcActP, _bcm_dpp_field_tc_p_act_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_field_tc_p_act_t)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egrTrapProfile"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->egrTrapProfile), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcActB"); 
   arraySizes_SW_DUMP[0]=_BCM_DPP_NOF_BCM_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->tcActB, _bcm_dpp_field_tc_b_act_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_field_tc_b_act_t)); 
} 
void _bcm_dpp_field_entry_common_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _bcm_dpp_field_entry_common_t *elt = ((_bcm_dpp_field_entry_common_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryNext"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryNext), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryPrev"); 
   _bcm_dpp_field_ent_idx_t_SW_DUMP_PRINT((void*) &(elt->entryPrev), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryGroup"); 
   _bcm_dpp_field_grp_idx_t_SW_DUMP_PRINT((void*) &(elt->entryGroup), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwPriority"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hwPriority), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryQual"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->entryQual, _bcm_dpp_field_qual_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_field_qual_t)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->entryFlags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryPriority"); 
   INT_SW_DUMP_PRINT((void*) &(elt->entryPriority), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwHandle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hwHandle), variableNameWithPath); 
} 
void _bcm_dpp_field_qual_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_qual_t *elt = ((_bcm_dpp_field_qual_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualType"); 
   bcm_field_qualify_t_SW_DUMP_PRINT((void*) &(elt->qualType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwType"); 
   SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT((void*) &(elt->hwType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualData"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->qualData), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qualMask"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->qualMask), variableNameWithPath); 
} 
void _bcm_dpp_field_tc_p_act_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_tc_p_act_t *elt = ((_bcm_dpp_field_tc_p_act_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwType"); 
   SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT((void*) &(elt->hwType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwParam"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hwParam), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwFlags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hwFlags), variableNameWithPath); 
} 
void SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _bcm_dpp_field_tc_b_act_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_tc_b_act_t *elt = ((_bcm_dpp_field_tc_b_act_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmType"); 
   bcm_field_action_t_SW_DUMP_PRINT((void*) &(elt->bcmType), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmParam0"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->bcmParam0), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmParam1"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->bcmParam1), variableNameWithPath); 
} 
void _bcm_dpp_field_entry_dir_ext_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _bcm_dpp_field_entry_dir_ext_t *elt = ((_bcm_dpp_field_entry_dir_ext_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entryCmn"); 
   _bcm_dpp_field_entry_common_t_SW_DUMP_PRINT((void*) &(elt->entryCmn), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".deAct"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->deAct, _bcm_dpp_field_de_act_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_field_de_act_t)); 
} 
void _bcm_dpp_field_de_act_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_field_de_act_t *elt = ((_bcm_dpp_field_de_act_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hwParam"); 
   SOC_PPC_FP_DIR_EXTR_ACTION_VAL_SW_DUMP_PRINT((void*) &(elt->hwParam), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmType"); 
   bcm_field_action_t_SW_DUMP_PRINT((void*) &(elt->bcmType), variableNameWithPath); 
} 
void SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *elt = ((SOC_PPC_FP_DIR_EXTR_ENTRY_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_vals"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->qual_vals, SOC_PPC_FP_QUAL_VAL_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_QUAL_VAL)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".actions"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->actions, SOC_PPC_FP_DIR_EXTR_ACTION_VAL_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_DIR_EXTR_ACTION_VAL)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".priority"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->priority), variableNameWithPath); 
} 
void SOC_PPC_FP_QUAL_VAL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FP_QUAL_VAL *elt = ((SOC_PPC_FP_QUAL_VAL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".val"); 
   SOC_SAND_U64_SW_DUMP_PRINT((void*) &(elt->val), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_valid"); 
   SOC_SAND_U64_SW_DUMP_PRINT((void*) &(elt->is_valid), variableNameWithPath); 
} 
void SOC_SAND_U64_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_U64 *elt = ((SOC_SAND_U64*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".arr"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_U64_NOF_UINT32S; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->arr, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void SOC_PPC_FP_DIR_EXTR_ACTION_VAL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_FP_DIR_EXTR_ACTION_VAL *elt = ((SOC_PPC_FP_DIR_EXTR_ACTION_VAL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fld_ext"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->fld_ext, SOC_PPC_FP_DIR_EXTR_ACTION_LOC_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_DIR_EXTR_ACTION_LOC)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_fields"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_fields), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".base_val"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->base_val), variableNameWithPath); 
} 
void SOC_PPC_FP_DIR_EXTR_ACTION_LOC_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FP_DIR_EXTR_ACTION_LOC *elt = ((SOC_PPC_FP_DIR_EXTR_ACTION_LOC*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fld_lsb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->fld_lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cst_val"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->cst_val), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_bits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_bits), variableNameWithPath); 
} 
void _bcm_dpp_field_profile_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_scache_handle_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_arad_field_device_qual_info_layer_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_arad_field_device_qual_info_layer_t *elt = ((_bcm_arad_field_device_qual_info_layer_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcmQual"); 
   int32_SW_DUMP_PRINT((void*) &(elt->bcmQual), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bits"); 
   int32_SW_DUMP_PRINT((void*) &(elt->bits), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lshift"); 
   int32_SW_DUMP_PRINT((void*) &(elt->lshift), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   int32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void _bcm_dpp_lif_match_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_sw_resources_inlif_t *elt = ((_bcm_dpp_sw_resources_inlif_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lif_type"); 
   _bcm_lif_type_e_SW_DUMP_PRINT((void*) &(elt->lif_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".criteria"); 
   INT_SW_DUMP_PRINT((void*) &(elt->criteria), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".port"); 
   bcm_port_t_SW_DUMP_PRINT((void*) &(elt->port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".match1"); 
   INT_SW_DUMP_PRINT((void*) &(elt->match1), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".match2"); 
   INT_SW_DUMP_PRINT((void*) &(elt->match2), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".match_tunnel"); 
   INT_SW_DUMP_PRINT((void*) &(elt->match_tunnel), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".match_ethertype"); 
   INT_SW_DUMP_PRINT((void*) &(elt->match_ethertype), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".gport_id"); 
   bcm_gport_t_SW_DUMP_PRINT((void*) &(elt->gport_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key1"); 
   INT_SW_DUMP_PRINT((void*) &(elt->key1), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".peer_gport"); 
   bcm_gport_t_SW_DUMP_PRINT((void*) &(elt->peer_gport), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".learn_gport_id"); 
   bcm_gport_t_SW_DUMP_PRINT((void*) &(elt->learn_gport_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid_profile_type"); 
   _bcm_petra_tpid_profile_t_SW_DUMP_PRINT((void*) &(elt->tpid_profile_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vsi"); 
   INT_SW_DUMP_PRINT((void*) &(elt->vsi), variableNameWithPath); 
} 
void _bcm_lif_type_e_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_port_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_petra_tpid_profile_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
 
void bcm_dpp_qos_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_qos_state_t *elt = ((bcm_dpp_qos_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   INT_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ing_lif_cos_mpls_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->ing_lif_cos_mpls_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->ing_lif_cos_mpls_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }   
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ing_pcp_vlan_stag_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->ing_pcp_vlan_stag_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->ing_pcp_vlan_stag_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_remark_mpls_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->egr_remark_mpls_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_remark_mpls_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_mpls_php_ipv6_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->egr_mpls_php_ipv6_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_mpls_php_ipv6_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_pcp_vlan_stag_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->egr_pcp_vlan_stag_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_pcp_vlan_stag_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_l2_i_tag_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->egr_l2_i_tag_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_l2_i_tag_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_dscp_exp_marking_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->egr_dscp_exp_marking_bitmap) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_dscp_exp_marking_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   }   
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ing_cos_opcode_flags"); 
   /* check the pointer is not null */ 
   if (elt->ing_cos_opcode_flags) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->ing_cos_opcode_flags), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_pcp_vlan_dscp_exp_profile_id"); 
   /* check the pointer is not null */ 
   if (elt->egr_pcp_vlan_dscp_exp_profile_id) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_pcp_vlan_dscp_exp_profile_id), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_pcp_vlan_dscp_exp_enable"); 
   /* check the pointer is not null */ 
   if (elt->egr_pcp_vlan_dscp_exp_enable) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_pcp_vlan_dscp_exp_enable), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egr_remark_encap_enable"); 
   /* check the pointer is not null */ 
   if (elt->egr_remark_encap_enable) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egr_remark_encap_enable), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*dp_max"); 
   /* check the pointer is not null */ 
   if (elt->dp_max) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->dp_max), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*opcode_bmp"); 
   /* check the pointer is not null */ 
   if (elt->opcode_bmp) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->opcode_bmp), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void bcm_dpp_l3_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_l3_state_t *elt = ((bcm_dpp_l3_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   INT_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".urpf_mode"); 
   INT_SW_DUMP_PRINT((void*) &(elt->urpf_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcm_dpp_tunnel_bk_info"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*bcm_tunnel_intf_to_eep"); 
   /* check the pointer is not null */ 
   if (elt->bcm_tunnel_intf_to_eep) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->bcm_tunnel_intf_to_eep), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void bcm_dpp_l3_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_l3_info_t *elt = ((bcm_dpp_l3_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".used_intf"); 
   INT_SW_DUMP_PRINT((void*) &(elt->used_intf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".used_vrf"); 
   INT_SW_DUMP_PRINT((void*) &(elt->used_vrf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".used_host"); 
   INT_SW_DUMP_PRINT((void*) &(elt->used_host), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".used_route"); 
   INT_SW_DUMP_PRINT((void*) &(elt->used_route), variableNameWithPath); 
} 
void _dpp_policer_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _dpp_policer_state_t *elt = ((_dpp_policer_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".policer_group"); 
   arraySizes_SW_DUMP[0]=_DPP_POLICER_NOF_POLICERS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->policer_group, _dpp_policer_group_info_t_SW_DUMP_PRINT, 0, sizeof(_dpp_policer_group_info_t)); 
} 
void _dpp_policer_group_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _dpp_policer_group_info_t *elt = ((_dpp_policer_group_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode"); 
   bcm_policer_group_mode_t_SW_DUMP_PRINT((void*) &(elt->mode), variableNameWithPath); 
} 
void bcm_policer_group_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_stg_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_stg_info_t *elt = ((bcm_stg_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   INT_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stg_min"); 
   bcm_stg_t_SW_DUMP_PRINT((void*) &(elt->stg_min), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stg_max"); 
   bcm_stg_t_SW_DUMP_PRINT((void*) &(elt->stg_max), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stg_defl"); 
   bcm_stg_t_SW_DUMP_PRINT((void*) &(elt->stg_defl), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stg_bitmap"); 
   /* check the pointer is not null */ 
   if (elt->stg_bitmap) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->stg_bitmap), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stg_enable"); 
   /* check the pointer is not null */ 
   if (elt->stg_enable) { 
      bcm_pbmp_t_SW_DUMP_PRINT((void*) &(*elt->stg_enable), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stg_state_h"); 
   /* check the pointer is not null */ 
   if (elt->stg_state_h) { 
      bcm_pbmp_t_SW_DUMP_PRINT((void*) &(*elt->stg_state_h), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*stg_state_l"); 
   /* check the pointer is not null */ 
   if (elt->stg_state_l) { 
      bcm_pbmp_t_SW_DUMP_PRINT((void*) &(*elt->stg_state_l), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stg_count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->stg_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*vlan_first"); 
   /* check the pointer is not null */ 
   if (elt->vlan_first) { 
      bcm_vlan_t_SW_DUMP_PRINT((void*) &(*elt->vlan_first), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*vlan_next"); 
   /* check the pointer is not null */ 
   if (elt->vlan_next) { 
      bcm_vlan_t_SW_DUMP_PRINT((void*) &(*elt->vlan_next), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void bcm_stg_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_pbmp_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _shr_pbmp_t *elt = ((_shr_pbmp_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pbits"); 
   arraySizes_SW_DUMP[0]=_SHR_PBMP_WORD_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->pbits, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void bcm_vlan_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint16_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_vswitch_bookkeeping_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_vswitch_bookkeeping_t *elt = ((_bcm_dpp_vswitch_bookkeeping_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*vsi_usage"); 
   /* check the pointer is not null */ 
   if (elt->vsi_usage) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->vsi_usage), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void _bcm_petra_mirror_unit_data_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_petra_mirror_unit_data_t *elt = ((_bcm_petra_mirror_unit_data_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ingressCount"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->ingressCount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ingress"); 
   /* check the pointer is not null */ 
   if (elt->ingress) { 
      _bcm_petra_mirror_data_t_SW_DUMP_PRINT((void*) &(*elt->ingress), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void _bcm_petra_mirror_data_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_petra_mirror_data_t *elt = ((_bcm_petra_mirror_data_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".refCount"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->refCount), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mirrorInfo"); 
   SOC_PB_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT((void*) &(elt->mirrorInfo), variableNameWithPath); 
} 
void SOC_PB_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_ACTION_CMD_MIRROR_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *elt = ((SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cmd"); 
   SOC_TMC_ACTION_CMD_SW_DUMP_PRINT((void*) &(elt->cmd), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mirror_prob"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prob), variableNameWithPath); 
} 
void SOC_TMC_ACTION_CMD_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_TMC_ACTION_CMD *elt = ((SOC_TMC_ACTION_CMD*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dest_id"); 
   SOC_TMC_DEST_INFO_SW_DUMP_PRINT((void*) &(elt->dest_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tc"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->tc), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dp"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->dp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_ptr_low"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->meter_ptr_low), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_ptr_up"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->meter_ptr_up), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_dp"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->meter_dp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".counter_ptr_1"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->counter_ptr_1), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".counter_ptr_2"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->counter_ptr_2), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_ing_mc"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_ing_mc), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".outlif"); 
   SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT((void*) &(elt->outlif), variableNameWithPath); 
} 
void SOC_TMC_DEST_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_TMC_DEST_INFO *elt = ((SOC_TMC_DEST_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_TMC_DEST_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->id), variableNameWithPath); 
} 
void SOC_TMC_DEST_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_TMC_ACTION_CMD_OVERRIDE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_TMC_ACTION_CMD_OVERRIDE *elt = ((SOC_TMC_ACTION_CMD_OVERRIDE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".value"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->value), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".enable"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->enable), variableNameWithPath); 
} 
void bcm_dpp_trill_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_trill_state_t *elt = ((bcm_dpp_trill_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   INT_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mask_set"); 
   INT_SW_DUMP_PRINT((void*) &(elt->mask_set), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trill_out_ac"); 
   SOC_PPC_AC_ID_SW_DUMP_PRINT((void*) &(elt->trill_out_ac), variableNameWithPath);
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".last_used_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->last_used_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
    
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*trill_ports"); 
   /* check the pointer is not null */ 
   if (elt->trill_ports) { 
      bcm_petra_trill_port_list_t_SW_DUMP_PRINT((void*) &(*elt->trill_ports), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_PPC_AC_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_petra_trill_port_list_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_petra_trill_port_list_t *elt = ((bcm_petra_trill_port_list_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".allocated_cnt"); 
   INT_SW_DUMP_PRINT((void*) &(elt->allocated_cnt), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".port_cnt"); 
   INT_SW_DUMP_PRINT((void*) &(elt->port_cnt), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ports"); 
   /* check the pointer is not null */ 
   if (elt->ports) { 
      bcm_gport_t_SW_DUMP_PRINT((void*) &(*elt->ports), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_SAND_PP_MAC_ADDRESS_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_PP_MAC_ADDRESS *elt = ((SOC_SAND_PP_MAC_ADDRESS*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".address"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_PP_MAC_ADDRESS_NOF_U8; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->address, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
} 
void SOC_PPC_LLP_SA_AUTH_MAC_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_LLP_SA_AUTH_MAC_INFO *elt = ((SOC_PPC_LLP_SA_AUTH_MAC_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tagged_only"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tagged_only), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".expect_tag_vid"); 
   SOC_SAND_PP_VLAN_ID_SW_DUMP_PRINT((void*) &(elt->expect_tag_vid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".expect_system_port"); 
   SOC_SAND_PP_SYS_PORT_ID_SW_DUMP_PRINT((void*) &(elt->expect_system_port), variableNameWithPath); 
} 
void SOC_SAND_PP_VLAN_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PP_SYS_PORT_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PP_SYS_PORT_ID *elt = ((SOC_SAND_PP_SYS_PORT_ID*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sys_port_type"); 
   SOC_SAND_PP_SYS_PORT_TYPE_SW_DUMP_PRINT((void*) &(elt->sys_port_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sys_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sys_id), variableNameWithPath); 
} 
void SOC_SAND_PP_SYS_PORT_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_KEY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_type"); 
   SOC_PPC_FRWRD_MACT_KEY_TYPE_SW_DUMP_PRINT((void*) &(elt->key_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_val"); 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_SW_DUMP_PRINT((void*) &(elt->key_val), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_MACT_KEY_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_KEY_VAL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mac"); 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_SW_DUMP_PRINT((void*) &(elt->mac), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ipv4_mc"); 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_SW_DUMP_PRINT((void*) &(elt->ipv4_mc), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_KEY_MAC_ADDR*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mac"); 
   SOC_SAND_PP_MAC_ADDRESS_SW_DUMP_PRINT((void*) &(elt->mac), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fid"); 
   SOC_PPC_FID_SW_DUMP_PRINT((void*) &(elt->fid), variableNameWithPath); 
} 
void SOC_PPC_FID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_KEY_IPV4_MC*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dip"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->dip), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fid"); 
   SOC_PPC_FID_SW_DUMP_PRINT((void*) &(elt->fid), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_VALUE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_VALUE *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_VALUE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_info"); 
   SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_SW_DUMP_PRINT((void*) &(elt->frwrd_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".aging_info"); 
   SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_SW_DUMP_PRINT((void*) &(elt->aging_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".accessed"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->accessed), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->group), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_FRWRD_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".forward_decision"); 
   SOC_PPC_FRWRD_DECISION_INFO_SW_DUMP_PRINT((void*) &(elt->forward_decision), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".drop_when_sa_is_known"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->drop_when_sa_is_known), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_DECISION_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
} 
void SOC_PPC_FRWRD_DECISION_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FRWRD_DECISION_TYPE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_DECISION_TYPE_INFO *elt = ((SOC_PPC_FRWRD_DECISION_TYPE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".eei"); 
   SOC_PPC_EEI_SW_DUMP_PRINT((void*) &(elt->eei), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".outlif"); 
   SOC_PPC_OUTLIF_SW_DUMP_PRINT((void*) &(elt->outlif), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trap_info"); 
   SOC_PPC_TRAP_INFO_SW_DUMP_PRINT((void*) &(elt->trap_info), variableNameWithPath); 
} 
void SOC_PPC_EEI_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_EEI *elt = ((SOC_PPC_EEI*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_PPC_EEI_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".val"); 
   SOC_PPC_EEI_VAL_SW_DUMP_PRINT((void*) &(elt->val), variableNameWithPath); 
} 
void SOC_PPC_EEI_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_EEI_VAL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_EEI_VAL *elt = ((SOC_PPC_EEI_VAL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trill_dest"); 
   SOC_SAND_PP_TRILL_DEST_SW_DUMP_PRINT((void*) &(elt->trill_dest), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mpls_command"); 
   SOC_PPC_MPLS_COMMAND_SW_DUMP_PRINT((void*) &(elt->mpls_command), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".isid"); 
   SOC_SAND_PP_ISID_SW_DUMP_PRINT((void*) &(elt->isid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".raw"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->raw), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".outlif"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->outlif), variableNameWithPath); 
} 
void SOC_SAND_PP_TRILL_DEST_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PP_TRILL_DEST *elt = ((SOC_SAND_PP_TRILL_DEST*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_multicast"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_multicast), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dest_nick"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->dest_nick), variableNameWithPath); 
} 
void SOC_PPC_MPLS_COMMAND_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_MPLS_COMMAND *elt = ((SOC_PPC_MPLS_COMMAND*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".command"); 
   SOC_PPC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT((void*) &(elt->command), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".label"); 
   SOC_SAND_PP_MPLS_LABEL_SW_DUMP_PRINT((void*) &(elt->label), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".push_profile"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->push_profile), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid_profile"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tpid_profile), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".model"); 
   SOC_SAND_PP_MPLS_TUNNEL_MODEL_SW_DUMP_PRINT((void*) &(elt->model), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".has_cw"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->has_cw), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pop_next_header"); 
   SOC_PPC_PKT_FRWRD_TYPE_SW_DUMP_PRINT((void*) &(elt->pop_next_header), variableNameWithPath); 
} 
void SOC_PPC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_MPLS_COMMAND_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_SAND_PP_MPLS_LABEL_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PP_MPLS_TUNNEL_MODEL_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_PKT_FRWRD_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_PKT_FRWRD_TYPE_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_PKT_FRWRD_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_SAND_PP_ISID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_OUTLIF_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_OUTLIF *elt = ((SOC_PPC_OUTLIF*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   SOC_PPC_OUTLIF_ENCODE_TYPE_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".val"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->val), variableNameWithPath); 
} 
void SOC_PPC_OUTLIF_ENCODE_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_TRAP_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_INFO *elt = ((SOC_PPC_TRAP_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".action_profile"); 
   SOC_PPC_ACTION_PROFILE_SW_DUMP_PRINT((void*) &(elt->action_profile), variableNameWithPath); 
} 
void SOC_PPC_ACTION_PROFILE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_ACTION_PROFILE *elt = ((SOC_PPC_ACTION_PROFILE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trap_code"); 
   SOC_PPC_TRAP_CODE_SW_DUMP_PRINT((void*) &(elt->trap_code), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_action_strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->frwrd_action_strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".snoop_action_strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->snoop_action_strength), variableNameWithPath); 
} 
void SOC_PPC_TRAP_CODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO *elt = ((SOC_PPC_FRWRD_MACT_ENTRY_AGING_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_dynamic"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_dynamic), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".age_status"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->age_status), variableNameWithPath); 
} 
void _bcm_petra_l2_freeze_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_petra_l2_freeze_t *elt = ((_bcm_petra_l2_freeze_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frozen"); 
   INT_SW_DUMP_PRINT((void*) &(elt->frozen), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".save_age_ena"); 
   INT_SW_DUMP_PRINT((void*) &(elt->save_age_ena), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".save_learn_ena"); 
   INT_SW_DUMP_PRINT((void*) &(elt->save_learn_ena), variableNameWithPath); 
} 
void l2_data_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   l2_data_t *elt = ((l2_data_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".l2_initialized");
   INT_SW_DUMP_PRINT((void*) &(elt->l2_initialized), variableNameWithPath);
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcm_petra_l2_freeze_state");
   _bcm_petra_l2_freeze_t_SW_DUMP_PRINT((void*) &(elt->bcm_petra_l2_freeze_state), variableNameWithPath);
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mymac_msb_is_set"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->mymac_msb_is_set), variableNameWithPath); 
} 
void _bcm_petra_l2_cb_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_petra_l2_cb_t *elt = ((_bcm_petra_l2_cb_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
} 
void bcm_mac_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_l2_addr_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_l2_addr_t *elt = ((bcm_l2_addr_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mac"); 
   bcm_mac_t_SW_DUMP_PRINT((void*) &(elt->mac), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vid"); 
   bcm_vlan_t_SW_DUMP_PRINT((void*) &(elt->vid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".port"); 
   INT_SW_DUMP_PRINT((void*) &(elt->port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".modid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->modid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tgid"); 
   bcm_trunk_t_SW_DUMP_PRINT((void*) &(elt->tgid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cos_dst"); 
   bcm_cos_t_SW_DUMP_PRINT((void*) &(elt->cos_dst), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cos_src"); 
   bcm_cos_t_SW_DUMP_PRINT((void*) &(elt->cos_src), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".l2mc_index"); 
   INT_SW_DUMP_PRINT((void*) &(elt->l2mc_group), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".block_bitmap"); 
   bcm_pbmp_t_SW_DUMP_PRINT((void*) &(elt->block_bitmap), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".auth"); 
   INT_SW_DUMP_PRINT((void*) &(elt->auth), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group"); 
   INT_SW_DUMP_PRINT((void*) &(elt->group), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".distribution_class"); 
   bcm_fabric_distribution_t_SW_DUMP_PRINT((void*) &(elt->distribution_class), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".encap_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->encap_id), variableNameWithPath); 
} 
void bcm_trunk_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_cos_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_fabric_distribution_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void llm_appl_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   llm_appl_info_t *elt = ((llm_appl_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_channel"); 
   INT_SW_DUMP_PRINT((void*) &(elt->rx_channel), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".uc_num"); 
   INT_SW_DUMP_PRINT((void*) &(elt->uc_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dma_buffer_len"); 
   INT_SW_DUMP_PRINT((void*) &(elt->dma_buffer_len), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*dma_buffer"); 
   /* check the pointer is not null */ 
   if (elt->dma_buffer) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->dma_buffer), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*dmabuf_reply"); 
   /* check the pointer is not null */ 
   if (elt->dmabuf_reply) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->dmabuf_reply), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".event_thread_kill"); 
   INT_SW_DUMP_PRINT((void*) &(elt->event_thread_kill), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".event_enable"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->event_enable), variableNameWithPath); 
} 
void _bcm_dpp_mpls_bookkeeping_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_mpls_bookkeeping_t *elt = ((_bcm_dpp_mpls_bookkeeping_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egress_tunnel_id"); 
   /* check the pointer is not null */ 
   if (elt->egress_tunnel_id) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->egress_tunnel_id), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_PPC_FRWRD_ILM_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
} 
void SOC_SAND_PP_MPLS_EXP_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_PORT_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_RIF_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_OAM_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_OAM_UPMEP_TRAP_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_OAM_MIRROR_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_OAM_CPU_TRAP_CODE_TO_MIRROR_PROFILE_MAP_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_bfd_event_types_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_bfd_event_types_t *elt = ((bcm_bfd_event_types_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".w"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( bcmBFDEventCount ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->w, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void bcm_bfd_event_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_bfd_endpoint_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_oam_event_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_oam_group_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_oam_endpoint_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_oam_group_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_oam_group_info_t *elt = ((bcm_oam_group_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".id"); 
   bcm_oam_group_t_SW_DUMP_PRINT((void*) &(elt->id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".name"); 
   arraySizes_SW_DUMP[0]=BCM_OAM_GROUP_NAME_LENGTH; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->name, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".faults"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->faults), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".persistent_faults"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->persistent_faults), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".clear_persistent_faults"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->clear_persistent_faults), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lowest_alarm_priority"); 
   bcm_oam_group_fault_alarm_defect_priority_t_SW_DUMP_PRINT((void*) &(elt->lowest_alarm_priority), variableNameWithPath); 
} 
void bcm_oam_group_fault_alarm_defect_priority_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_pbmp_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_pbmp_t_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_dpp_port_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_dpp_port_config_t *elt = ((bcm_dpp_port_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bcm_petra_port_init_arrays_done"); 
   INT_SW_DUMP_PRINT((void*) &(elt->bcm_petra_port_init_arrays_done), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".port_pp_initialized"); 
   INT_SW_DUMP_PRINT((void*) &(elt->port_pp_initialized), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".map_tbl_use"); 
   arraySizes_SW_DUMP[0]=_bcm_dpp_port_map_type_count; 
   arraySizes_SW_DUMP[1]=_BCM_DPP_PORT_MAP_MAX_NOF_TBLS; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->map_tbl_use, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trap_to_flag"); 
   arraySizes_SW_DUMP[0]=_BCM_PETRA_PORT_LEARN_NOF_TRAPS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->trap_to_flag, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*bcm_port_tpid_info"); 
   /* check the pointer is not null */ 
   if (elt->bcm_port_tpid_info) { 
      bcm_petra_port_tpid_info_SW_DUMP_PRINT((void*) &(*elt->bcm_port_tpid_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void _bcm_dpp_port_map_type_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_petra_port_tpid_info_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_petra_port_tpid_info *elt = ((bcm_petra_port_tpid_info*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid"); 
   arraySizes_SW_DUMP[0]=_BCM_PETRA_NOF_TPIDS_PER_PORT; 
   arraySizes_SW_DUMP[1]=_BCM_PORT_NOF_TPID_PROFILES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->tpid, uint16_SW_DUMP_PRINT, 0, sizeof(uint16)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid_count"); 
   arraySizes_SW_DUMP[0]=_BCM_PETRA_NOF_TPIDS_PER_PORT; 
   arraySizes_SW_DUMP[1]=_BCM_PORT_NOF_TPID_PROFILES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->tpid_count, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
} 
void SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO *elt = ((SOC_PPC_TRAP_FRWRD_ACTION_PROFILE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bitmap_mask"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->bitmap_mask), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dest_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO_SW_DUMP_PRINT((void*) &(elt->dest_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cos_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO_SW_DUMP_PRINT((void*) &(elt->cos_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO_SW_DUMP_PRINT((void*) &(elt->count_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO_SW_DUMP_PRINT((void*) &(elt->meter_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".policing_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO_SW_DUMP_PRINT((void*) &(elt->policing_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".processing_info"); 
   SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO_SW_DUMP_PRINT((void*) &(elt->processing_info), variableNameWithPath); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_DEST_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_oam_dest"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_oam_dest), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_dest"); 
   SOC_PPC_FRWRD_DECISION_INFO_SW_DUMP_PRINT((void*) &(elt->frwrd_dest), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".add_vsi"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->add_vsi), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vsi_shift"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->vsi_shift), variableNameWithPath); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_COS_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tc"); 
   SOC_SAND_PP_TC_SW_DUMP_PRINT((void*) &(elt->tc), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dp"); 
   SOC_SAND_PP_DP_SW_DUMP_PRINT((void*) &(elt->dp), variableNameWithPath); 
} 
void SOC_SAND_PP_TC_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PP_DP_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_COUNT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".counter_select"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->counter_select), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".counter_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->counter_id), variableNameWithPath); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_METER_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_select"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->meter_select), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->meter_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".meter_command"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->meter_command), variableNameWithPath); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_POLICE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ethernet_police_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ethernet_police_id), variableNameWithPath); 
} 
void SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO *elt = ((SOC_PPC_TRAP_ACTION_PROFILE_PROCESS_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".enable_learning"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->enable_learning), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_trap"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_trap), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_control"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_control), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_offset_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->frwrd_offset_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_type"); 
   SOC_TMC_PKT_FRWRD_TYPE_SW_DUMP_PRINT((void*) &(elt->frwrd_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".frwrd_offset_bytes_fix"); 
   int32_SW_DUMP_PRINT((void*) &(elt->frwrd_offset_bytes_fix), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".da_type"); 
   SOC_SAND_PP_ETHERNET_DA_TYPE_SW_DUMP_PRINT((void*) &(elt->da_type), variableNameWithPath); 
} 
void SOC_SAND_PP_ETHERNET_DA_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO *elt = ((SOC_PPC_TRAP_SNOOP_ACTION_PROFILE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".snoop_cmnd"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->snoop_cmnd), variableNameWithPath); 
} 
void bcm_dpp_l3_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_l3_sw_db_t *elt = ((bcm_dpp_l3_sw_db_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fec_to_ecmp"); 
   /* check the pointer is not null */ 
   if (elt->fec_to_ecmp) { 
      fec_to_ecmps_t_SW_DUMP_PRINT((void*) &(*elt->fec_to_ecmp), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ecmp_size"); 
   /* check the pointer is not null */ 
   if (elt->ecmp_size) { 
      ecmp_size_t_SW_DUMP_PRINT((void*) &(*elt->ecmp_size), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ecmp_successive"); 
   /* check the pointer is not null */ 
   if (elt->ecmp_successive) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->ecmp_successive), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ecmp_protected"); 
   /* check the pointer is not null */ 
   if (elt->ecmp_protected) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->ecmp_protected), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fec_copy_info"); 
   /* check the pointer is not null */ 
   if (elt->fec_copy_info) { 
      fec_copy_info_t_SW_DUMP_PRINT((void*) &(*elt->fec_copy_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void fec_to_ecmps_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   fec_to_ecmps_t *elt = ((fec_to_ecmps_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ecmps"); 
   /* check the pointer is not null */ 
   if (elt->ecmps) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->ecmps), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*corresponding_fecs"); 
   /* check the pointer is not null */ 
   if (elt->corresponding_fecs) { 
      INT_SW_DUMP_PRINT((void*) &(*elt->corresponding_fecs), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_ecmps"); 
   INT_SW_DUMP_PRINT((void*) &(elt->nof_ecmps), variableNameWithPath); 
} 
void ecmp_size_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ecmp_size_t *elt = ((ecmp_size_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cur_size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->cur_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_size), variableNameWithPath); 
} 
void fec_copy_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   fec_copy_info_t *elt = ((fec_copy_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ecmp"); 
   INT_SW_DUMP_PRINT((void*) &(elt->ecmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".real_fec"); 
   INT_SW_DUMP_PRINT((void*) &(elt->real_fec), variableNameWithPath); 
} 
void bcm_dpp_mirror_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   bcm_dpp_mirror_sw_db_t *elt = ((bcm_dpp_mirror_sw_db_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mirror_mode"); 
   INT_SW_DUMP_PRINT((void*) &(elt->mirror_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egress_port_profile"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_MAX_NOF_LOCAL_PORTS_ARAD; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->egress_port_profile, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
} 
void bcm_dpp_switch_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_switch_sw_db_t *elt = ((bcm_dpp_switch_sw_db_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dos_attack"); 
   dos_attack_info_t_SW_DUMP_PRINT((void*) &(elt->dos_attack), variableNameWithPath); 
} 
void dos_attack_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   dos_attack_info_t *elt = ((dos_attack_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tocpu"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tocpu), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sipequaldip"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->sipequaldip), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mintcphdrsize"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->mintcphdrsize), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".v4firstfrag"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->v4firstfrag), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpflags"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpflags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".l4port"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->l4port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpfrag"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpfrag), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".icmp"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->icmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".icmppktoversize"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->icmppktoversize), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".macsaequalmacda"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->macsaequalmacda), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".icmpv6pingsize"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->icmpv6pingsize), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".icmpfragments"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->icmpfragments), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpoffset"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpoffset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".udpportsequal"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->udpportsequal), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpportsequal"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpportsequal), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpflagssf"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpflagssf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpflagsfup"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpflagsfup), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcphdrpartial"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcphdrpartial), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pingflood"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->pingflood), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".synflood"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->synflood), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpsmurf"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpsmurf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcpxmas"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tcpxmas), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".l3header"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->l3header), variableNameWithPath); 
} 
void multi_device_sync_flags_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   multi_device_sync_flags_t *elt = ((multi_device_sync_flags_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".eep"); 
   INT_SW_DUMP_PRINT((void*) &(elt->eep), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".inlif"); 
   INT_SW_DUMP_PRINT((void*) &(elt->inlif), variableNameWithPath); 
} 
void bcm_time_interface_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_time_interface_t *elt = ((bcm_time_interface_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".id"); 
   bcm_time_if_t_SW_DUMP_PRINT((void*) &(elt->id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".drift"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->drift), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".offset"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".accuracy"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->accuracy), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".heartbeat_hz"); 
   INT_SW_DUMP_PRINT((void*) &(elt->heartbeat_hz), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".clk_resolution"); 
   INT_SW_DUMP_PRINT((void*) &(elt->clk_resolution), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bitclock_hz"); 
   INT_SW_DUMP_PRINT((void*) &(elt->bitclock_hz), variableNameWithPath); 
} 
void bcm_time_if_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_time_spec_t *elt = ((bcm_time_spec_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".isnegative"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->isnegative), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".seconds"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->seconds), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nanoseconds"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nanoseconds), variableNameWithPath); 
} 
void bcm_time_capture_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_time_capture_t *elt = ((bcm_time_capture_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".free"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->free), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".syntonous"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->syntonous), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".synchronous"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->synchronous), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".received"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->received), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".received_accuracy"); 
   bcm_time_spec_t_SW_DUMP_PRINT((void*) &(elt->received_accuracy), variableNameWithPath); 
} 
void trunk_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   trunk_state_t *elt = ((trunk_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init_state"); 
   trunk_init_state_t_SW_DUMP_PRINT((void*) &(elt->init_state), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ngroups"); 
   INT_SW_DUMP_PRINT((void*) &(elt->ngroups), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stk_ngroups"); 
   INT_SW_DUMP_PRINT((void*) &(elt->stk_ngroups), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nports"); 
   INT_SW_DUMP_PRINT((void*) &(elt->nports), variableNameWithPath); 
   arraySizes_SW_DUMP[0]= elt->ngroups + elt->stk_ngroups; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->t_info, bcm_dpp_trunk_private_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_trunk_private_t)); 
} 
void trunk_init_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_pkt_blk_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_pkt_blk_t *elt = ((bcm_pkt_blk_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*data"); 
   /* check the pointer is not null */ 
   if (elt->data) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->data), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".len"); 
   INT_SW_DUMP_PRINT((void*) &(elt->len), variableNameWithPath); 
} 
void bcm_color_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_multicast_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_pkt_stk_forward_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_if_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void bcm_rx_reasons_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_rx_reasons_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_rx_reasons_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _shr_rx_reasons_t *elt = ((_shr_rx_reasons_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pbits"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( _SHR_RX_REASON_COUNT ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->pbits, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void _shr_rx_reason_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_vlan_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void sal_time_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_LONG_SW_DUMP_PRINT(element, variableName); 
} 
void sbusdma_desc_handle_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void _bcm_dpp_vlan_unit_state_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _bcm_dpp_vlan_unit_state_t *elt = ((_bcm_dpp_vlan_unit_state_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vlan_info"); 
   bcm_dpp_vlan_info_t_SW_DUMP_PRINT((void*) &(elt->vlan_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fid_ref_count"); 
   arraySizes_SW_DUMP[0]=DPP_NOF_SHARED_FIDS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->fid_ref_count, fid_ref_count_t_SW_DUMP_PRINT, 0, sizeof(fid_ref_count_t)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".wb"); 
} 
void bcm_dpp_vlan_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_vlan_info_t *elt = ((bcm_dpp_vlan_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   INT_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".defl"); 
   bcm_vlan_t_SW_DUMP_PRINT((void*) &(elt->defl), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".defl_pbmp"); 
   bcm_pbmp_t_SW_DUMP_PRINT((void*) &(elt->defl_pbmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".defl_ubmp"); 
   bcm_pbmp_t_SW_DUMP_PRINT((void*) &(elt->defl_ubmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->count), variableNameWithPath); 
} 
void fid_ref_count_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   fid_ref_count_t *elt = ((fid_ref_count_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ref_count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->ref_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->fid), variableNameWithPath); 
} 
void SOC_PPC_LIF_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void _dpp_res_arad_pool_egress_encap_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _dpp_res_arad_pool_cosq_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _dpp_res_pool_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _dpp_template_pool_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 


void bcm_dpp_trunk_private_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_trunk_private_t *elt = ((bcm_dpp_trunk_private_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trunk_id"); 
   bcm_trunk_t_SW_DUMP_PRINT((void*) &(elt->trunk_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".in_use"); 
   INT_SW_DUMP_PRINT((void*) &(elt->in_use), variableNameWithPath); 
} 
void ARAD_PP_ISEM_ACCESS_PROGRAM_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_ISEM_ACCESS_PROGRAM_INFO *elt = ((ARAD_PP_ISEM_ACCESS_PROGRAM_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prog_used"); 
   INT_SW_DUMP_PRINT((void*) &(elt->prog_used), variableNameWithPath); 
} 
void soc_reg_above_64_val_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PP_SW_DB_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_SW_DB *elt = ((ARAD_PP_SW_DB*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*device"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->device, ARAD_PP_SW_DB_DEVICE_SW_DUMP_PRINT, 1, sizeof(ARAD_PP_SW_DB_DEVICE*)); 
} 
void ARAD_PP_SW_DB_DEVICE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_SW_DB_DEVICE *elt = ((ARAD_PP_SW_DB_DEVICE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*oper_mode"); 
   /* check the pointer is not null */ 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*llp_filter"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*llp_vid_assign"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*pon_double_lookup"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ipv4_info"); 
   /* check the pointer is not null */ 
   if (elt->ipv4_info) { 
      ARAD_PP_SW_DB_IPV4_INFO_SW_DUMP_PRINT((void*) &(*elt->ipv4_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ilm_info"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fwd_mact"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*lif_cos"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*lif_table"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fec"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*diag"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*lag"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*vrrp_info"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*header_data"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*rif_to_lif_group_map"); 
   if (elt->rif_to_lif_group_map) { 
      ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MAP_SW_DUMP_PRINT((void*) &(*elt->rif_to_lif_group_map), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void ARAD_PP_MGMT_IPV4_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_MGMT_IPV4_INFO *elt = ((ARAD_PP_MGMT_IPV4_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pvlan_enable"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->pvlan_enable), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_vrfs"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_vrfs), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_routes_in_vrf"); 
   arraySizes_SW_DUMP[0]=SOC_DPP_DEFS_MAX(NOF_VRFS); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->max_routes_in_vrf, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bits_in_phase"); 
   arraySizes_SW_DUMP[0]=ARAD_PP_MGMT_IPV4_LPM_BANKS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->bits_in_phase, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bits_in_phase_valid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->bits_in_phase_valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void ARAD_PP_MGMT_P2P_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_MGMT_P2P_INFO *elt = ((ARAD_PP_MGMT_P2P_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mim_vsi"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->mim_vsi), variableNameWithPath); 
} 
void SOC_SAND_PP_ETHER_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint16_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PP_SW_DB_LLP_FILTER_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_LLP_FILTER_t *elt = ((ARAD_PP_LLP_FILTER_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ref_count"); 
   ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT_SW_DUMP_PRINT((void*) &(elt->ref_count), variableNameWithPath); 
} 
void ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT *elt = ((ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_REF_COUNT*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".arr"); 
   arraySizes_SW_DUMP[0]=ARAD_PP_LLP_FILTER_DESIGNATED_VLAN_TABLE_SIZE; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->arr, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void SOC_SAND_MULTI_SET_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_MULTI_SET_INIT_INFO *elt = ((SOC_SAND_MULTI_SET_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prime_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prime_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_members"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_members), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".member_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->member_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_duplications"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->max_duplications), variableNameWithPath); 
} 

void SOC_SAND_HASH_TABLE_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_HASH_TABLE_INIT_INFO *elt = ((SOC_SAND_HASH_TABLE_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prime_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prime_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".table_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->table_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".table_width"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->table_width), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->key_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".data_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->data_size), variableNameWithPath); 
} 
void SOC_SAND_HASH_TABLE_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint8_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_HASH_TABLE_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_HASH_TABLE_T *elt = ((SOC_SAND_HASH_TABLE_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*lists_head"); 
   /* check the pointer is not null */ 
   if (elt->lists_head) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->lists_head), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*keys"); 
   /* check the pointer is not null */ 
   if (elt->keys) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->keys), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*next"); 
   /* check the pointer is not null */ 
   if (elt->next) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->next), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ptr_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ptr_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".null_ptr"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->null_ptr), variableNameWithPath); 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tmp_buf"); 
   /* check the pointer is not null */ 
   if (elt->tmp_buf) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->tmp_buf), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tmp_buf2"); 
   /* check the pointer is not null */ 
} 
void ARAD_PP_SW_DB_LLP_VID_ASSIGN_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_LLP_VID_ASSIGN_t *elt = ((ARAD_LLP_VID_ASSIGN_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vid_sa_based_enable"); 
   arraySizes_SW_DUMP[0]=ARAD_PP_SW_DB_PP_PORTS_NOF_U32; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->vid_sa_based_enable, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PON_DOUBLE_LOOKUP_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PON_DOUBLE_LOOKUP *elt = ((ARAD_PON_DOUBLE_LOOKUP*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pon_double_lookup_enable"); 
   arraySizes_SW_DUMP[0]=ARAD_PP_SW_DB_PP_PORTS_NOF_U32; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->pon_double_lookup_enable, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PP_SW_DB_IPV4_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_SW_DB_IPV4_INFO *elt = ((ARAD_PP_SW_DB_IPV4_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lpm_mngr"); 
   ARAD_PP_IPV4_LPM_MNGR_INFO_SW_DUMP_PRINT((void*) &(elt->lpm_mngr), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*cm_buf_array"); 
   /* check the pointer is not null */ 
   if (elt->cm_buf_array) { 
     uint32_SW_DUMP_PRINT((void*) &(*elt->cm_buf_array), variableNameWithPath); 
   } else { 
      sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cm_buf_entry_words"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->cm_buf_entry_words), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*free_list"); 
   /* check the pointer is not null */ 
   if (elt->free_list) { 
     ARAD_PP_IPV4_LPM_FREE_LIST_ITEM_INFO_SW_DUMP_PRINT((void*) &(*elt->free_list), variableNameWithPath); 
   } else { 
      sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".free_list_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->free_list_size), variableNameWithPath); 
} 
void ARAD_PP_IPV4_LPM_MNGR_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_IPV4_LPM_MNGR_INFO *elt = ((ARAD_PP_IPV4_LPM_MNGR_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init_info"); 
   ARAD_PP_IPV4_LPM_MNGR_INIT_INFO_SW_DUMP_PRINT((void*) &(elt->init_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".data_info"); 
   ARAD_PP_IPV4_LPM_MNGR_T_SW_DUMP_PRINT((void*) &(elt->data_info), variableNameWithPath); 
} 
void ARAD_PP_IPV4_LPM_MNGR_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_IPV4_LPM_MNGR_INIT_INFO *elt = ((ARAD_PP_IPV4_LPM_MNGR_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prime_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prime_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_vrf_bits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_vrf_bits), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_banks"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_banks), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*nof_bits_per_bank"); 
   /* check the pointer is not null */ 
   if (elt->nof_bits_per_bank) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->nof_bits_per_bank), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*bank_to_mem"); 
   /* check the pointer is not null */ 
   if (elt->bank_to_mem) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->bank_to_mem), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*mem_to_bank"); 
   /* check the pointer is not null */ 
   if (elt->mem_to_bank) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->mem_to_bank), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_mems"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_mems), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*nof_rows_per_mem"); 
   /* check the pointer is not null */ 
   if (elt->nof_rows_per_mem) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->nof_rows_per_mem), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pxx_model"); 
   ARAD_PP_IPV4_LPM_PXX_MODEL_SW_DUMP_PRINT((void*) &(elt->pxx_model), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*mem_allocators"); 
   /* check the pointer is not null */ 
   if (elt->mem_allocators) { 
      ARAD_PP_ARR_MEM_ALLOCATOR_INFO_SW_DUMP_PRINT((void*) &(*elt->mem_allocators), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**rev_ptrs"); 
   /* check the pointer is not null */ 
   if (elt->rev_ptrs) { 
      SOC_SAND_GROUP_MEM_LL_INFO_SW_DUMP_PRINT((void*) &(**elt->rev_ptrs), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*lpms"); 
   /* check the pointer is not null */ 
   if (elt->lpms) { 
      SOC_SAND_PAT_TREE_INFO_SW_DUMP_PRINT((void*) &(*elt->lpms), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_lpms"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_lpms), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_nof_entries_for_hw_lpm"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->max_nof_entries_for_hw_lpm), variableNameWithPath); 
} 
void ARAD_PP_IPV4_LPM_PXX_MODEL_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_PP_ARR_MEM_ALLOCATOR_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_ARR_MEM_ALLOCATOR_INFO *elt = ((ARAD_PP_ARR_MEM_ALLOCATOR_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".instance_prim_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->instance_prim_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".instance_sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->instance_sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_entries"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_entries), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entry_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->entry_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".support_caching"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->support_caching), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".support_defragment"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->support_defragment), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_block_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->max_block_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".wb_var_index"); 
   int32_SW_DUMP_PRINT((void*) &(elt->wb_var_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".arr_mem_allocator_data"); 
   ARAD_PP_ARR_MEM_ALLOCATOR_T_SW_DUMP_PRINT((void*) &(elt->arr_mem_allocator_data), variableNameWithPath); 
} 
void ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PP_ARR_MEM_ALLOCATOR_PTR_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PP_ARR_MEM_ALLOCATOR_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_ARR_MEM_ALLOCATOR_T *elt = ((ARAD_PP_ARR_MEM_ALLOCATOR_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*array"); 
   /* check the pointer is not null */ 
   if (elt->array) { 
      ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY_SW_DUMP_PRINT((void*) &(*elt->array), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".free_list"); 
   ARAD_PP_ARR_MEM_ALLOCATOR_PTR_SW_DUMP_PRINT((void*) &(elt->free_list), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*mem_shadow"); 
   /* check the pointer is not null */ 
   if (elt->mem_shadow) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->mem_shadow), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cache_enabled"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->cache_enabled), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*array_cache"); 
   /* check the pointer is not null */ 
   if (elt->array_cache) { 
      ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY_SW_DUMP_PRINT((void*) &(*elt->array_cache), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".free_list_cache"); 
   ARAD_PP_ARR_MEM_ALLOCATOR_PTR_SW_DUMP_PRINT((void*) &(elt->free_list_cache), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_updates"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_updates), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*update_indexes"); 
   /* check the pointer is not null */ 
   if (elt->update_indexes) { 
      ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY_SW_DUMP_PRINT((void*) &(*elt->update_indexes), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*mem_shadow_cache"); 
   /* check the pointer is not null */ 
   if (elt->mem_shadow_cache) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->mem_shadow_cache), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_SAND_GROUP_MEM_LL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_GROUP_MEM_LL_INFO *elt = ((SOC_SAND_GROUP_MEM_LL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".auto_remove"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->auto_remove), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_groups"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_groups), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".instance_prim_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->instance_prim_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".instance_sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->instance_sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_elements"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_elements), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".support_caching"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->support_caching), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".wb_var_index"); 
   int32_SW_DUMP_PRINT((void*) &(elt->wb_var_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group_members_data"); 
   SOC_SAND_GROUP_MEM_LL_T_SW_DUMP_PRINT((void*) &(elt->group_members_data), variableNameWithPath); 
} 
void SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY *elt = ((SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".first_member"); 
   SOC_SAND_GROUP_MEM_LL_MEMBER_ID_SW_DUMP_PRINT((void*) &(elt->first_member), variableNameWithPath); 
} 
void SOC_SAND_GROUP_MEM_LL_MEMBER_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY *elt = ((SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".next_member"); 
   SOC_SAND_GROUP_MEM_LL_MEMBER_ID_SW_DUMP_PRINT((void*) &(elt->next_member), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prev_member"); 
   SOC_SAND_GROUP_MEM_LL_MEMBER_ID_SW_DUMP_PRINT((void*) &(elt->prev_member), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group_id"); 
   SOC_SAND_GROUP_MEM_LL_GROUP_ID_SW_DUMP_PRINT((void*) &(elt->group_id), variableNameWithPath); 
} 
void SOC_SAND_GROUP_MEM_LL_GROUP_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_GROUP_MEM_LL_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_GROUP_MEM_LL_T *elt = ((SOC_SAND_GROUP_MEM_LL_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*groups"); 
   /* check the pointer is not null */ 
   if (elt->groups) { 
      SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SW_DUMP_PRINT((void*) &(*elt->groups), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*members"); 
   /* check the pointer is not null */ 
   if (elt->members) { 
      SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SW_DUMP_PRINT((void*) &(*elt->members), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cache_enabled"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->cache_enabled), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*groups_cache"); 
   /* check the pointer is not null */ 
   if (elt->groups_cache) { 
      SOC_SAND_GROUP_MEM_LL_GROUP_ENTRY_SW_DUMP_PRINT((void*) &(*elt->groups_cache), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*members_cache"); 
   /* check the pointer is not null */ 
   if (elt->members_cache) { 
      SOC_SAND_GROUP_MEM_LL_MEMBER_ENTRY_SW_DUMP_PRINT((void*) &(*elt->members_cache), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_SAND_PAT_TREE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_PAT_TREE_INFO *elt = ((SOC_SAND_PAT_TREE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tree_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tree_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prime_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prime_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".support_cache"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->support_cache), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".wb_var_index"); 
   int32_SW_DUMP_PRINT((void*) &(elt->wb_var_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pat_tree_data"); 
   SOC_SAND_PAT_TREE_T_SW_DUMP_PRINT((void*) &(elt->pat_tree_data), variableNameWithPath); 
} 
void SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_PAT_TREE_NODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_PAT_TREE_NODE *elt = ((SOC_SAND_PAT_TREE_NODE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".child"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_PAT_TREE_NOF_NODE_CHILD; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->child, SOC_SAND_PAT_TREE_NODE_PLACE_SW_DUMP_PRINT, 0, sizeof(SOC_SAND_PAT_TREE_NODE_PLACE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key"); 
   SOC_SAND_PAT_TREE_KEY_SW_DUMP_PRINT((void*) &(elt->key), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".data"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->data), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prefix"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->prefix), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_prefix"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_prefix), variableNameWithPath); 
} 
void SOC_SAND_PAT_TREE_KEY_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint32_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PP_IPV4_LPM_MNGR_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_IPV4_LPM_MNGR_T *elt = ((ARAD_PP_IPV4_LPM_MNGR_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*bit_depth_per_bank"); 
   /* check the pointer is not null */ 
   if (elt->bit_depth_per_bank) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->bit_depth_per_bank), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_PPC_FRWRD_IPV4_HOST_TABLE_RESOURCE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_PP_IPV4_LPM_FREE_LIST_ITEM_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_IPV4_LPM_FREE_LIST_ITEM_INFO *elt = ((ARAD_PP_IPV4_LPM_FREE_LIST_ITEM_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bank_id"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->bank_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".address"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->address), variableNameWithPath); 
} 
void ARAD_PP_ILM_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_ILM_INFO *elt = ((ARAD_PP_ILM_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mask_inrif"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->mask_inrif), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mask_port"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->mask_port), variableNameWithPath); 
} 
void ARAD_PP_FWD_MACT_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_FWD_MACT *elt = ((ARAD_PP_FWD_MACT*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".learning_mode"); 
   SOC_PPC_FRWRD_MACT_LEARNING_MODE_SW_DUMP_PRINT((void*) &(elt->learning_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_petra_a_compatible"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_petra_a_compatible), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flush_db_data_arr"); 
   arraySizes_SW_DUMP[0]=8; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->flush_db_data_arr, ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR_SW_DUMP_PRINT, 0, sizeof(ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flush_entry_use"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flush_entry_use), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".traverse_mode"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->traverse_mode), variableNameWithPath); 
} 
void SOC_PPC_FRWRD_MACT_LEARNING_MODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR *elt = ((ARAD_PP_FRWRD_MACT_FLUSH_DB_DATA_ARR*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flush_db_data"); 
   arraySizes_SW_DUMP[0]=7; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->flush_db_data, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PP_SW_DB_LIF_COS_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_SW_DB_LIF_COS *elt = ((ARAD_PP_SW_DB_LIF_COS*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".map_from_tc_dp"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->map_from_tc_dp), variableNameWithPath); 
} 
void ARAD_PP_SW_DB_LIF_TABLE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_SW_DB_LIF_TABLE *elt = ((ARAD_PP_SW_DB_LIF_TABLE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lif_use"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( SOC_DPP_DEFS_MAX(NOF_LOCAL_LIFS) * ARAD_PP_SW_DB_TYPE_BITS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->lif_use, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lif_sub_use"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( SOC_DPP_DEFS_MAX(NOF_LOCAL_LIFS) * ARAD_PP_SW_DB_TYPE_BITS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->lif_sub_use, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PP_FEC_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_FEC *elt = ((ARAD_PP_FEC*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fec_entry_type"); 
   arraySizes_SW_DUMP[0]=SOC_DPP_DEFS_MAX(NOF_FECS); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->fec_entry_type, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flp_progs_mapping"); 
   arraySizes_SW_DUMP[0]=SOC_DPP_DEFS_MAX(NOF_FLP_PROGRAMS); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->flp_progs_mapping, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lem_prefix_mapping"); 
   arraySizes_SW_DUMP[0]=(1 << SOC_DPP_DEFS_MAX(NOF_LEM_PREFIXES)); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->lem_prefix_mapping, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
} 
void SOC_PPC_FRWRD_FEC_GLBL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_FRWRD_FEC_GLBL_INFO *elt = ((SOC_PPC_FRWRD_FEC_GLBL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ecmp_sizes"); 
   arraySizes_SW_DUMP[0]=SOC_DPP_DEFS_MAX(ECMP_MAX_SIZE); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->ecmp_sizes, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ecmp_sizes_nof_entries"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ecmp_sizes_nof_entries), variableNameWithPath); 
} 
void ARAD_PP_SW_DB_DIAG_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_SW_DB_DIAG *elt = ((ARAD_PP_SW_DB_DIAG*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trap_dest"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_NOF_TRAP_CODES * 4; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->trap_dest, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".already_saved"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->already_saved), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode_info"); 
   SOC_PPC_DIAG_MODE_INFO_SW_DUMP_PRINT((void*) &(elt->mode_info), variableNameWithPath); 
} 
void SOC_PPC_DIAG_MODE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_PPC_DIAG_MODE_INFO *elt = ((SOC_PPC_DIAG_MODE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flavor"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flavor), variableNameWithPath); 
} 
void ARAD_PP_LAG_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_LAG *elt = ((ARAD_PP_LAG*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lb_key_is_symtrc"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->lb_key_is_symtrc), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".masks"); 
   SOC_PPC_HASH_MASKS_SW_DUMP_PRINT((void*) &(elt->masks), variableNameWithPath); 
} 
void SOC_PPC_HASH_MASKS_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_PP_SW_DB_VRRP_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PP_SW_DB_VRRP_INFO *elt = ((ARAD_PP_SW_DB_VRRP_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vrrp_mac_use_bit_map"); 
   arraySizes_SW_DUMP[0]=256; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->vrrp_mac_use_bit_map, uint16_SW_DUMP_PRINT, 0, sizeof(uint16)); 
} 
void ARAD_PP_HEADER_DATA_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_HEADER_DATA *elt = ((ARAD_PP_HEADER_DATA*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".system_headers_mode"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->system_headers_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ftmh_stacking_ext_enable"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->ftmh_stacking_ext_enable), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ftmh_lb_key_ext_en"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->ftmh_lb_key_ext_en), variableNameWithPath); 
} 
void ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MAP_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MAP *elt = ((ARAD_PP_SW_DB_RIF_TO_LIF_GROUP_MAP*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group_info"); 
   SOC_SAND_GROUP_MEM_LL_INFO_SW_DUMP_PRINT((void*) &(elt->group_info), variableNameWithPath); 
} 
void SOC_SAND_INDIRECT_MODULE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_INDIRECT_MODULE *elt = ((SOC_SAND_INDIRECT_MODULE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*info_arr"); 
   /* check the pointer is not null */ 
   if (elt->info_arr) { 
      SOC_SAND_INDIRECT_MODULE_INFO_SW_DUMP_PRINT((void*) &(*elt->info_arr), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tables_info"); 
   /* check the pointer is not null */ 
   if (elt->tables_info) { 
      SOC_SAND_INDIRECT_TABLES_INFO_SW_DUMP_PRINT((void*) &(*elt->tables_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*memory_map_arr"); 
   /* check the pointer is not null */ 
   if (elt->memory_map_arr) { 
      SOC_SAND_INDIRECT_MEMORY_MAP_SW_DUMP_PRINT((void*) &(*elt->memory_map_arr), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".info_arr_max_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->info_arr_max_index), variableNameWithPath); 
} 
void SOC_SAND_INDIRECT_MODULE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_INDIRECT_MODULE_INFO *elt = ((SOC_SAND_INDIRECT_MODULE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".module_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->module_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".read_result_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->read_result_offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".word_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->word_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".access_trigger"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->access_trigger), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".access_address"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->access_address), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".write_buffer_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->write_buffer_offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".module_bits"); 
   SOC_SAND_INDIRECT_MODULE_BITS_SW_DUMP_PRINT((void*) &(elt->module_bits), variableNameWithPath); 
} 
void SOC_SAND_INDIRECT_MODULE_BITS_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_SAND_INDIRECT_TABLES_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_INDIRECT_TABLES_INFO *elt = ((SOC_SAND_INDIRECT_TABLES_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tables_prefix"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tables_prefix), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tables_prefix_nof_bits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tables_prefix_nof_bits), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".word_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->word_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".read_result_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->read_result_offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".write_buffer_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->write_buffer_offset), variableNameWithPath); 
} 
void SOC_SAND_INDIRECT_MEMORY_MAP_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_INDIRECT_MEMORY_MAP *elt = ((SOC_SAND_INDIRECT_MEMORY_MAP*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
} 
void SOC_SAND_RET_SW_DUMP_PRINT(void* element, char* variableName){ 
   UNSIGNED_SHORT_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_SAND_DELTA_LIST_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_DELTA_LIST *elt = ((SOC_SAND_DELTA_LIST*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*head"); 
   /* check the pointer is not null */ 
   if (elt->head) { 
      SOC_SAND_DELTA_LIST_NODE_SW_DUMP_PRINT((void*) &(*elt->head), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".head_time"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->head_time), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stats"); 
   SOC_SAND_DELTA_LIST_STATISTICS_SW_DUMP_PRINT((void*) &(elt->stats), variableNameWithPath); 
} 
void SOC_SAND_DELTA_LIST_NODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_DELTA_LIST_NODE *elt = ((SOC_SAND_DELTA_LIST_NODE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".num"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*next"); 
   /* check the pointer is not null */ 
   if (elt->next) { 
      SOC_SAND_DELTA_LIST_NODE_SW_DUMP_PRINT((void*) &(*elt->next), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void SOC_SAND_DELTA_LIST_STATISTICS_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_DELTA_LIST_STATISTICS *elt = ((SOC_SAND_DELTA_LIST_STATISTICS*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".current_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->current_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".no_of_pops"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->no_of_pops), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".no_of_removes"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->no_of_removes), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".no_of_inserts"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->no_of_inserts), variableNameWithPath); 
} 
void SOC_SAND_DEVICE_DESC_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_DEVICE_DESC *elt = ((SOC_SAND_DEVICE_DESC*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid_word"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->valid_word), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*base_addr"); 
   /* check the pointer is not null */ 
   if (elt->base_addr) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->base_addr), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mem_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->mem_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".interrupt_bitstream"); 
   arraySizes_SW_DUMP[0]=SIZE_OF_BITSTRAEM_IN_UINT32S; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->interrupt_bitstream, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".device_is_between_isr_to_tcm"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->device_is_between_isr_to_tcm), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".device_interrupt_mask_counter"); 
   INT_SW_DUMP_PRINT((void*) &(elt->device_interrupt_mask_counter), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".logic_chip_type"); 
   SOC_SAND_DEVICE_TYPE_SW_DUMP_PRINT((void*) &(elt->logic_chip_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".chip_type"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->chip_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dbg_ver"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->dbg_ver), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".chip_ver"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->chip_ver), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".device_at_init"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->device_at_init), variableNameWithPath); 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".magic"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->magic), variableNameWithPath); 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
} 
void SOC_SAND_DEVICE_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_SAND_LL_TIMER_FUNCTION_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_LL_TIMER_FUNCTION *elt = ((SOC_SAND_LL_TIMER_FUNCTION*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".name"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_LL_TIMER_MAX_NOF_CHARS_IN_TIMER_NAME; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->name, CHAR_SW_DUMP_PRINT, 0, sizeof(char)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_hits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_hits), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".active"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->active), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".start_timer"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->start_timer), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".total_time"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->total_time), variableNameWithPath); 
} 
void SOC_SAND_RAND_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_SAND_RAND *elt = ((SOC_SAND_RAND*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".state"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_RAND_N; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->state, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".left"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->left), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".initf"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->initf), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*next"); 
   /* check the pointer is not null */ 
   if (elt->next) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->next), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".seed"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->seed), variableNameWithPath); 
} 
void ARAD_SW_DB_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB *elt = ((ARAD_SW_DB*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*arad_device_sw_db"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->arad_device_sw_db, ARAD_SW_DB_DEVICE_SW_DUMP_PRINT, 1, sizeof(ARAD_SW_DB_DEVICE*)); 
} 
void ARAD_SW_DB_DEVICE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".op_mode"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lag"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tdm"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".q_type_map"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cell"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dram"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcam_mgmt"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vsi"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pmf"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cnt"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".interrupts"); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*modport2sysport"); 
} 
void ARAD_SW_DB_OP_MODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_SW_DB_OP_MODE *elt = ((ARAD_SW_DB_OP_MODE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_petrab_in_system"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_petrab_in_system), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tdm_mode"); 
   ARAD_MGMT_TDM_MODE_SW_DUMP_PRINT((void*) &(elt->tdm_mode), variableNameWithPath); 
} 
void ARAD_MGMT_TDM_MODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_MGMT_TDM_MODE_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_MGMT_TDM_MODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_LAGS_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_LAGS_INFO *elt = ((ARAD_LAGS_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".in_use"); 
   arraySizes_SW_DUMP[0]=ARAD_NOF_LAG_GROUPS_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->in_use, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".local_to_reassembly_context"); 
   arraySizes_SW_DUMP[0]=ARAD_NOF_LOCAL_PORTS_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->local_to_reassembly_context, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_TDM_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_TDM *elt = ((ARAD_TDM*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".context_map"); 
   arraySizes_SW_DUMP[0]=ARAD_NOF_TDM_CONTEXT_MAP; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->context_map, ARAD_INTERFACE_ID_SW_DUMP_PRINT, 0, sizeof(ARAD_INTERFACE_ID)); 
} 
void ARAD_INTERFACE_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_INTERFACE_ID_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_INTERFACE_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_SW_DB_DEV_EGR_PORT_PRIORITY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_DEV_EGR_PORT_PRIORITY *elt = ((ARAD_SW_DB_DEV_EGR_PORT_PRIORITY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".queue_rate"); 
   arraySizes_SW_DUMP[0]=ARAD_EGR_NOF_Q_PAIRS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->queue_rate, ARAD_SW_DB_DEV_RATE_SW_DUMP_PRINT, 0, sizeof(ARAD_SW_DB_DEV_RATE)); 
} 
void ARAD_SW_DB_DEV_RATE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_SW_DB_DEV_RATE *elt = ((ARAD_SW_DB_DEV_RATE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egq_rates"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->egq_rates), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egq_bursts"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->egq_bursts), variableNameWithPath); 
} 
void ARAD_SW_DB_DEV_EGR_TCG_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_DEV_EGR_TCG *elt = ((ARAD_SW_DB_DEV_EGR_TCG*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcg_rate"); 
   arraySizes_SW_DUMP[0]=ARAD_EGR_NOF_PS; 
   arraySizes_SW_DUMP[1]=ARAD_NOF_TCGS; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->tcg_rate, ARAD_SW_DB_DEV_RATE_SW_DUMP_PRINT, 0, sizeof(ARAD_SW_DB_DEV_RATE)); 
} 
void ARAD_SW_DB_DEV_EGR_RATE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_SW_DB_DEV_EGR_RATE *elt = ((ARAD_SW_DB_DEV_EGR_RATE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sch_rates"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sch_rates), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egq_rates"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->egq_rates), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egq_bursts"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->egq_bursts), variableNameWithPath); 
} 
void ARAD_OFP_RATES_EGQ_CHAN_ARB_ID_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_EGR_PROG_TM_PORT_PROFILE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_MULTICAST_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_MULTICAST *elt = ((ARAD_MULTICAST*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egress_groups_open_data"); 
   /* check the pointer is not null */ 
   if (elt->egress_groups_open_data) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->egress_groups_open_data), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void ARAD_CELL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_CELL *elt = ((ARAD_CELL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".current_cell_ident"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->current_cell_ident), variableNameWithPath); 
} 
void ARAD_DRAM_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_DRAM *elt = ((ARAD_DRAM*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".dram_deleted_buff_list"); 
   arraySizes_SW_DUMP[0]=ARAD_DRAM_MAX_BUFFERS_IN_ERROR_CNTR; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->dram_deleted_buff_list, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void SOC_SAND_SORTED_LIST_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_SORTED_LIST_INIT_INFO *elt = ((SOC_SAND_SORTED_LIST_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prime_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prime_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sec_handle"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sec_handle), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".list_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->list_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->key_size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".data_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->data_size), variableNameWithPath); 
} 

void SOC_SAND_OCC_BM_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_OCC_BM_INIT_INFO *elt = ((SOC_SAND_OCC_BM_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init_val"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->init_val), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".support_cache"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->support_cache), variableNameWithPath); 
} 
void ARAD_TCAM_BANK_ENTRY_SIZE_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_TCAM_BANK_ENTRY_SIZE_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_TCAM_BANK_ENTRY_SIZE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_TCAM_ACTION_SIZE_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_TCAM_ACTION_SIZE_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_TCAM_ACTION_SIZE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_TCAM_DB_PRIO_MODE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_TCAM_SMALL_BANKS_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_TCAM_PREFIX_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_TCAM_PREFIX *elt = ((ARAD_TCAM_PREFIX*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->bits), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".length"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->length), variableNameWithPath); 
} 
void SOC_SAND_SORTED_LIST_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_SORTED_LIST_INFO *elt = ((SOC_SAND_SORTED_LIST_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init_info"); 
   SOC_SAND_SORTED_LIST_INIT_INFO_SW_DUMP_PRINT((void*) &(elt->init_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".list_data"); 
   SOC_SAND_SORTED_LIST_T_SW_DUMP_PRINT((void*) &(elt->list_data), variableNameWithPath); 
} 
void SOC_SAND_SORTED_LIST_T_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   SOC_SAND_SORTED_LIST_T *elt = ((SOC_SAND_SORTED_LIST_T*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*keys"); 
   /* check the pointer is not null */ 
   if (elt->keys) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->keys), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*next"); 
   /* check the pointer is not null */ 
   if (elt->next) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->next), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*prev"); 
   /* check the pointer is not null */ 
   if (elt->prev) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->prev), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*data"); 
   /* check the pointer is not null */ 
   if (elt->data) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->data), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ptr_size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ptr_size), variableNameWithPath); 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tmp_data"); 
   /* check the pointer is not null */ 
   if (elt->tmp_data) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->tmp_data), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
#endif /* #if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*tmp_key"); 
   /* check the pointer is not null */ 
   if (elt->tmp_key) { 
      uint8_SW_DUMP_PRINT((void*) &(*elt->tmp_key), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
#endif /* #if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".null_ptr"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->null_ptr), variableNameWithPath); 
} 
void ARAD_SW_DB_TCAM_MGMT_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_TCAM_MGMT *elt = ((ARAD_SW_DB_TCAM_MGMT*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".profiles"); 
   arraySizes_SW_DUMP[0]=ARAD_TCAM_NOF_ACCESS_PROFILE_IDS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->profiles, ARAD_SW_DB_TCAM_ACCESS_PROFILE_SW_DUMP_PRINT, 0, sizeof(ARAD_SW_DB_TCAM_ACCESS_PROFILE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".banks"); 
   arraySizes_SW_DUMP[0]=SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->banks, ARAD_SW_DB_TCAM_MANAGED_BANK_SW_DUMP_PRINT, 0, sizeof(ARAD_SW_DB_TCAM_MANAGED_BANK)); 
} 
void ARAD_SW_DB_TCAM_ACCESS_PROFILE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_SW_DB_TCAM_ACCESS_PROFILE *elt = ((ARAD_SW_DB_TCAM_ACCESS_PROFILE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".min_banks"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->min_banks), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bank_owner"); 
   ARAD_TCAM_BANK_OWNER_SW_DUMP_PRINT((void*) &(elt->bank_owner), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".user_data"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->user_data), variableNameWithPath); 
} 
void ARAD_TCAM_BANK_OWNER_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_TCAM_BANK_OWNER_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_TCAM_BANK_OWNER_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_SW_DB_TCAM_MANAGED_BANK_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_TCAM_MANAGED_BANK *elt = ((ARAD_SW_DB_TCAM_MANAGED_BANK*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prefix_db"); 
   arraySizes_SW_DUMP[0]=ARAD_TCAM_NOF_PREFIXES; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->prefix_db, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_dbs"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_dbs), variableNameWithPath); 
} 
void ARAD_SW_DB_VSI_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_VSI *elt = ((ARAD_SW_DB_VSI*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vsi_to_isid"); 
   arraySizes_SW_DUMP[0]=32 * 1024; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->vsi_to_isid, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PMF_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF *elt = ((ARAD_PMF*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pgm_ce"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_PMF_NOF_CYCLES; 
   arraySizes_SW_DUMP[2]=ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS; 
   printArray(arraySizes_SW_DUMP, 0, 3, variableNameWithPath, (void**) elt->pgm_ce, ARAD_PMF_CE_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_CE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pgm_fes"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_PMF_NOF_FES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->pgm_fes, ARAD_PMF_FES_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_FES)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fem_entry"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_LOW_LEVEL_NOF_FEMS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->fem_entry, SOC_PPC_FP_FEM_ENTRY_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_FEM_ENTRY)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rsources"); 
   ARAD_PMF_RESOURCE_SW_DUMP_PRINT((void*) &(elt->rsources), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pgm_db_pmb"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_BIT_TO_U32 ( ARAD_PMF_NOF_DBS ); 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->pgm_db_pmb, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_info"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_DBS; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->db_info, ARAD_PMF_DB_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_DB_INFO)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".psl_info"); 
   ARAD_PMF_PSL_INFO_SW_DUMP_PRINT((void*) &(elt->psl_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".fp_info"); 
} 
void ARAD_PMF_CE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_CE *elt = ((ARAD_PMF_CE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_used"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_used), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_id"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->db_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lsb"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".msb"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->msb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_msb"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_msb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_second_key"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_second_key), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_lsb"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->qual_lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_type"); 
   SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT((void*) &(elt->qual_type), variableNameWithPath); 
} 
void ARAD_PMF_FES_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_FES *elt = ((ARAD_PMF_FES*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_used"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_used), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_id"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->db_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".action_type"); 
   SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT((void*) &(elt->action_type), variableNameWithPath); 
} 
void SOC_PPC_FP_FEM_ENTRY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_FP_FEM_ENTRY *elt = ((SOC_PPC_FP_FEM_ENTRY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_for_entry"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_for_entry), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->db_strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->db_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entry_strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->entry_strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entry_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->entry_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".action_type"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->action_type, SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_ACTION_TYPE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_base_positive"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->is_base_positive, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
} 
void ARAD_PMF_RESOURCE_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_RESOURCE *elt = ((ARAD_PMF_RESOURCE*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ce"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_PMF_NOF_CYCLES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->ce, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_PMF_NOF_CYCLES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->key, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".progs"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( ARAD_PMF_NOF_PROGS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->progs, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PMF_DB_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_DB_INFO *elt = ((ARAD_PMF_DB_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".progs"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( ARAD_PMF_NOF_PROGS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->progs, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prog_used_cycle_bmp"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prog_used_cycle_bmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".used_key"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_PROGS; 
   arraySizes_SW_DUMP[1]=ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->used_key, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prio"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prio), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_320b"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->is_320b), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".alloc_place"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->alloc_place), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_bits_zone"); 
   arraySizes_SW_DUMP[0]=ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX; 
   arraySizes_SW_DUMP[1]=2; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->nof_bits_zone, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cascaded_key"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->cascaded_key), variableNameWithPath); 
} 
void ARAD_PMF_PSL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_PSL_INFO *elt = ((ARAD_PMF_PSL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pfgs_db_pmb"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_PFGS_ARAD; 
   arraySizes_SW_DUMP[1]=ARAD_BIT_TO_U32 ( ARAD_PMF_NOF_DBS ); 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->pfgs_db_pmb, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".default_db_pmb"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( ARAD_PMF_NOF_DBS ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->default_db_pmb, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pfgs_tm_bmp"); 
   arraySizes_SW_DUMP[0]=ARAD_BIT_TO_U32 ( SOC_PPC_FP_NOF_PFGS_ARAD ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->pfgs_tm_bmp, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".levels_info"); 
   arraySizes_SW_DUMP[0]=2; 
   arraySizes_SW_DUMP[1]=ARAD_PMF_NOF_LEVELS_MAX_ALL_STAGES; 
   printArray(arraySizes_SW_DUMP, 0, 2, variableNameWithPath, (void**) elt->levels_info, ARAD_PMF_PSL_LEVEL_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_PSL_LEVEL_INFO)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init_info"); 
   ARAD_PMF_SEL_INIT_INFO_SW_DUMP_PRINT((void*) &(elt->init_info), variableNameWithPath); 
} 
void ARAD_PMF_PSL_LEVEL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_PSL_LEVEL_INFO *elt = ((ARAD_PMF_PSL_LEVEL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".first_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->first_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".last_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->last_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lines"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_NOF_LINES_IN_LEVEL_MAX_ALL_STAGES; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->lines, ARAD_PMF_PSL_LINE_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_PSL_LINE_INFO)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_lines"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_lines), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_new_lines"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_new_lines), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_removed_lines"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_removed_lines), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".level_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->level_index), variableNameWithPath); 
} 
void ARAD_PMF_PSL_LINE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_PSL_LINE_INFO *elt = ((ARAD_PMF_PSL_LINE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".group"); 
   ARAD_PMF_SEL_GROUP_SW_DUMP_PRINT((void*) &(elt->group), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".groups"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_GROUP_LEN; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->groups, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prog_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prog_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void ARAD_PMF_SEL_GROUP_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_SEL_GROUP *elt = ((ARAD_PMF_SEL_GROUP*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mask"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_SEL_LINE_LEN; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->mask, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".value"); 
   arraySizes_SW_DUMP[0]=ARAD_PMF_SEL_LINE_LEN; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->value, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PMF_SEL_INIT_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_SEL_INIT_INFO *elt = ((ARAD_PMF_SEL_INIT_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_reserved_lines"); 
   arraySizes_SW_DUMP[0]=ARAD_NOF_PMF_PSL_TYPES; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->nof_reserved_lines, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pmf_pgm_default"); 
   arraySizes_SW_DUMP[0]=ARAD_NOF_PMF_PSL_TYPES; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->pmf_pgm_default, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_PMF_PSL_TYPE_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void SOC_PPC_FP_DATABASE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_FP_DATABASE_INFO *elt = ((SOC_PPC_FP_DATABASE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".db_type"); 
   SOC_PPC_FP_DATABASE_TYPE_SW_DUMP_PRINT((void*) &(elt->db_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".supported_pfgs"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->supported_pfgs), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".supported_pfgs_arad"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_PFGS_IN_LONGS_ARAD; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->supported_pfgs_arad, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_types"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->qual_types, SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_QUAL_TYPE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".action_types"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->action_types, SOC_PPC_FP_ACTION_TYPE_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_ACTION_TYPE)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".strength"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->strength), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cascaded_coupled_db_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->cascaded_coupled_db_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void ARAD_FP_ENTRY_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_FP_ENTRY *elt = ((ARAD_FP_ENTRY*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nof_db_entries"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nof_db_entries), variableNameWithPath); 
} 
void SOC_PPC_PMF_PFG_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   SOC_PPC_PMF_PFG_INFO *elt = ((SOC_PPC_PMF_PFG_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".soc_sand_magic_num"); 
   CHAR_SW_DUMP_PRINT((void*) &(elt->soc_sand_magic_num), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hdr_format_bmp"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hdr_format_bmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vlan_tag_structure_bmp"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->vlan_tag_structure_bmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pp_ports_bmp"); 
   SOC_SAND_U64_SW_DUMP_PRINT((void*) &(elt->pp_ports_bmp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_array_qualifier"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_array_qualifier), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_vals"); 
   arraySizes_SW_DUMP[0]=SOC_PPC_FP_NOF_QUALS_PER_PFG_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->qual_vals, SOC_PPC_FP_QUAL_VAL_SW_DUMP_PRINT, 0, sizeof(SOC_PPC_FP_QUAL_VAL)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage"); 
   SOC_PPC_FP_DATABASE_STAGE_SW_DUMP_PRINT((void*) &(elt->stage), variableNameWithPath); 
} 
void ARAD_PMF_CE_QUAL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_PMF_CE_QUAL_INFO *elt = ((ARAD_PMF_CE_QUAL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_header_qual"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_header_qual), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".header_qual_info"); 
   ARAD_PMF_CE_HEADER_QUAL_INFO_SW_DUMP_PRINT((void*) &(elt->header_qual_info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".irpp_qual_info"); 
   arraySizes_SW_DUMP[0]=2; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->irpp_qual_info, ARAD_PMF_CE_IRPP_QUALIFIER_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PMF_CE_IRPP_QUALIFIER_INFO)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage"); 
   SOC_PPC_FP_DATABASE_STAGE_SW_DUMP_PRINT((void*) &(elt->stage), variableNameWithPath); 
} 
void ARAD_PMF_CE_HEADER_QUAL_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_CE_HEADER_QUAL_INFO *elt = ((ARAD_PMF_CE_HEADER_QUAL_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_type"); 
   ARAD_PMF_IRPP_INFO_FIELD_SW_DUMP_PRINT((void*) &(elt->qual_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".msb"); 
   int32_SW_DUMP_PRINT((void*) &(elt->msb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lsb"); 
   int32_SW_DUMP_PRINT((void*) &(elt->lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".header_ndx_0"); 
   ARAD_PMF_CE_SUB_HEADER_SW_DUMP_PRINT((void*) &(elt->header_ndx_0), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".header_ndx_1"); 
   ARAD_PMF_CE_SUB_HEADER_SW_DUMP_PRINT((void*) &(elt->header_ndx_1), variableNameWithPath); 
} 
void ARAD_PMF_IRPP_INFO_FIELD_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_PPC_FP_QUAL_TYPE_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_PMF_CE_SUB_HEADER_SW_DUMP_PRINT(void* element, char* variableName){ 
   SOC_TMC_PMF_CE_SUB_HEADER_SW_DUMP_PRINT(element, variableName); 
} 
void SOC_TMC_PMF_CE_SUB_HEADER_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_PMF_CE_IRPP_QUALIFIER_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_CE_IRPP_QUALIFIER_INFO *elt = ((ARAD_PMF_CE_IRPP_QUALIFIER_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".info"); 
   ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES_SW_DUMP_PRINT((void*) &(elt->info), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".signal"); 
   ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL_SW_DUMP_PRINT((void*) &(elt->signal), variableNameWithPath); 
} 
void ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES *elt = ((ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".irpp_field"); 
   ARAD_PMF_IRPP_INFO_FIELD_SW_DUMP_PRINT((void*) &(elt->irpp_field), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_lsb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->is_lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_msb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->is_msb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".buffer_lsb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->buffer_lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".qual_nof_bits"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->qual_nof_bits), variableNameWithPath); 
} 
void ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL *elt = ((ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".irpp_field"); 
   ARAD_PMF_IRPP_INFO_FIELD_SW_DUMP_PRINT((void*) &(elt->irpp_field), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".msb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->msb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".lsb"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->lsb), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".base0"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->base0), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".base1"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->base1), variableNameWithPath); 
} 
void ARAD_SW_DB_CNT_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   ARAD_SW_DB_CNT *elt = ((ARAD_SW_DB_CNT*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*host_buff"); 
   arraySizes_SW_DUMP[0]=SOC_TMC_CNT_NOF_PROCESSOR_IDS_ARAD; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->host_buff, uint32_SW_DUMP_PRINT, 1, sizeof(uint32*)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".buff_line_ndx"); 
   arraySizes_SW_DUMP[0]=SOC_TMC_CNT_NOF_PROCESSOR_IDS_ARAD; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->buff_line_ndx, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void ARAD_INTERRUPT_DATA_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_INTERRUPT_DATA *elt = ((ARAD_INTERRUPT_DATA*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".storm_timed_count"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->storm_timed_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".storm_timed_period"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->storm_timed_period), variableNameWithPath); 
} 
void arad_interrupt_type_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void ARAD_SYSPORT_SW_DUMP_PRINT(void* element, char* variableName){ 
   uint16_SW_DUMP_PRINT(element, variableName); 
} 
void ARAD_SW_DB_TM_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_SW_DB_TM *elt = ((ARAD_SW_DB_TM*) element); 
   int arraySizes_SW_DUMP[4]; 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".is_power_saving_called"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->is_power_saving_called), variableNameWithPath); 

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".queue_to_rate_class_mapping.is_simple_mode"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->queue_to_rate_class_mapping.is_simple_mode), variableNameWithPath); 

   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".queue_to_rate_class_mapping.ref_count");
   arraySizes_SW_DUMP[0]=SOC_TMC_ITM_NOF_RATE_CLASSES;
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->queue_to_rate_class_mapping.ref_count, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
} 
void _bcm_dpp_vlan_translate_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_vlan_translate_action_t *elt = ((_bcm_dpp_vlan_translate_action_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".outer"); 
   _bcm_dpp_vlan_translate_tag_action_t_SW_DUMP_PRINT((void*) &(elt->outer), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".inner"); 
   _bcm_dpp_vlan_translate_tag_action_t_SW_DUMP_PRINT((void*) &(elt->inner), variableNameWithPath); 
} 
void _bcm_dpp_vlan_translate_tag_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _bcm_dpp_vlan_translate_tag_action_t *elt = ((_bcm_dpp_vlan_translate_tag_action_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vid_action"); 
   bcm_vlan_action_t_SW_DUMP_PRINT((void*) &(elt->vid_action), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pcp_action"); 
   bcm_vlan_action_t_SW_DUMP_PRINT((void*) &(elt->pcp_action), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid_action"); 
   bcm_vlan_tpid_action_t_SW_DUMP_PRINT((void*) &(elt->tpid_action), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tpid_val"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->tpid_val), variableNameWithPath); 
} 
void bcm_vlan_tpid_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void bcm_dpp_am_egress_encap_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_am_egress_encap_t *elt = ((bcm_dpp_am_egress_encap_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*egress_encap_banks"); 
   /* check the pointer is not null */ 
   if (elt->egress_encap_banks) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->egress_encap_banks), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".egress_encap_count"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->egress_encap_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
} 
void bcm_dpp_am_ingress_lif_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_am_ingress_lif_t *elt = ((bcm_dpp_am_ingress_lif_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*ingress_lif_banks"); 
   /* check the pointer is not null */ 
   if (elt->ingress_lif_banks) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->ingress_lif_banks), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ingress_lif_count"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->ingress_lif_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
} 
void bcm_dpp_am_sync_lif_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   bcm_dpp_am_sync_lif_t *elt = ((bcm_dpp_am_sync_lif_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*sync_lif_banks"); 
   /* check the pointer is not null */ 
   if (elt->sync_lif_banks) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->sync_lif_banks), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".init"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->init), variableNameWithPath); 
} 
void soc_dpp_port_map_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_dpp_port_map_t *elt = ((soc_dpp_port_map_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".channel"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->channel), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nif_id"); 
   SOC_TMC_INTERFACE_ID_SW_DUMP_PRINT((void*) &(elt->nif_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tm_port"); 
   INT_SW_DUMP_PRINT((void*) &(elt->tm_port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pp_port"); 
   INT_SW_DUMP_PRINT((void*) &(elt->pp_port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".if_rate_mbps"); 
   INT_SW_DUMP_PRINT((void*) &(elt->if_rate_mbps), variableNameWithPath); 
} 
void soc_dpp_port_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_dpp_port_t *elt = ((soc_dpp_port_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".in_use"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->in_use), variableNameWithPath); 
} 
void soc_driver_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_driver_t *elt = ((soc_driver_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   soc_chip_types_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*chip_string"); 
   /* check the pointer is not null */ 
   if (elt->chip_string) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->chip_string), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*origin"); 
   /* check the pointer is not null */ 
   if (elt->origin) { 
      CHAR_SW_DUMP_PRINT((void*) &(*elt->origin), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pci_vendor"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->pci_vendor), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pci_device"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->pci_device), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pci_revision"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->pci_revision), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".num_cos"); 
   INT_SW_DUMP_PRINT((void*) &(elt->num_cos), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**reg_info"); 
   /* check the pointer is not null */ 
   if (elt->reg_info) { 
      soc_reg_info_t_SW_DUMP_PRINT((void*) &(**elt->reg_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**reg_above_64_info"); 
   /* check the pointer is not null */ 
   if (elt->reg_above_64_info) { 
      soc_reg_above_64_info_t_SW_DUMP_PRINT((void*) &(**elt->reg_above_64_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**reg_array_info"); 
   /* check the pointer is not null */ 
   if (elt->reg_array_info) { 
      soc_reg_array_info_t_SW_DUMP_PRINT((void*) &(**elt->reg_array_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**mem_info"); 
   /* check the pointer is not null */ 
   if (elt->mem_info) { 
      soc_mem_info_t_SW_DUMP_PRINT((void*) &(**elt->mem_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**mem_aggr"); 
   /* check the pointer is not null */ 
   if (elt->mem_aggr) { 
      soc_mem_t_SW_DUMP_PRINT((void*) &(**elt->mem_aggr), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**mem_array_info"); 
   /* check the pointer is not null */ 
   if (elt->mem_array_info) { 
      soc_mem_array_info_t_SW_DUMP_PRINT((void*) &(**elt->mem_array_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*block_info"); 
   /* check the pointer is not null */ 
   if (elt->block_info) { 
      soc_block_info_t_SW_DUMP_PRINT((void*) &(*elt->block_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*port_info"); 
   /* check the pointer is not null */ 
   if (elt->port_info) { 
      soc_port_info_t_SW_DUMP_PRINT((void*) &(*elt->port_info), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*counter_maps"); 
   /* check the pointer is not null */ 
   if (elt->counter_maps) { 
      soc_cmap_t_SW_DUMP_PRINT((void*) &(*elt->counter_maps), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".port_num_blktype"); 
   INT_SW_DUMP_PRINT((void*) &(elt->port_num_blktype), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cmicd_base"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->cmicd_base), variableNameWithPath); 
} 
void soc_chip_types_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_reg_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_reg_info_t *elt = ((soc_reg_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".block"); 
   soc_block_types_t_SW_DUMP_PRINT((void*) &(elt->block), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".regtype"); 
   soc_regtype_t_SW_DUMP_PRINT((void*) &(elt->regtype), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".numels"); 
   INT_SW_DUMP_PRINT((void*) &(elt->numels), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nFields"); 
   INT_SW_DUMP_PRINT((void*) &(elt->nFields), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fields"); 
   /* check the pointer is not null */ 
   if (elt->fields) { 
      soc_field_info_t_SW_DUMP_PRINT((void*) &(*elt->fields), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rst_val_lo"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rst_val_lo), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rst_val_hi"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rst_val_hi), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rst_mask_lo"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rst_mask_lo), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rst_mask_hi"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rst_mask_hi), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ctr_idx"); 
   INT_SW_DUMP_PRINT((void*) &(elt->ctr_idx), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".numelportlist_idx"); 
   INT_SW_DUMP_PRINT((void*) &(elt->numelportlist_idx), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".snoop_flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->snoop_flags), variableNameWithPath); 
} 
void soc_block_types_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void soc_regtype_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_field_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_field_info_t *elt = ((soc_field_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".field"); 
   soc_field_t_SW_DUMP_PRINT((void*) &(elt->field), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".len"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->len), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bp"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->bp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void soc_field_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void soc_reg_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void soc_reg_above_64_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_reg_above_64_info_t *elt = ((soc_reg_above_64_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*reset"); 
   /* check the pointer is not null */ 
   if (elt->reset) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->reset), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*mask"); 
   /* check the pointer is not null */ 
   if (elt->mask) { 
      uint32_SW_DUMP_PRINT((void*) &(*elt->mask), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
} 
void soc_reg_array_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_reg_array_info_t *elt = ((soc_reg_array_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".element_skip"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->element_skip), variableNameWithPath); 
} 
void soc_mem_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_mem_info_t *elt = ((soc_mem_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".index_min"); 
   INT_SW_DUMP_PRINT((void*) &(elt->index_min), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".index_max"); 
   INT_SW_DUMP_PRINT((void*) &(elt->index_max), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".minblock"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->minblock), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".maxblock"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->maxblock), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".blocks"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->blocks), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".blocks_hi"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->blocks_hi), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".base"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->base), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".gran"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->gran), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bytes"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->bytes), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nFields"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->nFields), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*fields"); 
   /* check the pointer is not null */ 
   if (elt->fields) { 
      soc_field_info_t_SW_DUMP_PRINT((void*) &(*elt->fields), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".**views"); 
   /* check the pointer is not null */ 
   if (elt->views) { 
      CHAR_SW_DUMP_PRINT((void*) &(**elt->views), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".snoop_flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->snoop_flags), variableNameWithPath); 
} 
void soc_mem_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   INT_SW_DUMP_PRINT(element, variableName); 
} 
void soc_mem_array_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_mem_array_info_t *elt = ((soc_mem_array_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".numels"); 
   UNSIGNED_INT_SW_DUMP_PRINT((void*) &(elt->numels), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".element_skip"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->element_skip), variableNameWithPath); 
} 
void soc_block_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_block_info_t *elt = ((soc_block_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".type"); 
   INT_SW_DUMP_PRINT((void*) &(elt->type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".number"); 
   INT_SW_DUMP_PRINT((void*) &(elt->number), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".schan"); 
   INT_SW_DUMP_PRINT((void*) &(elt->schan), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cmic"); 
   INT_SW_DUMP_PRINT((void*) &(elt->cmic), variableNameWithPath); 
} 
void soc_port_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_port_info_t *elt = ((soc_port_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".blk"); 
   INT_SW_DUMP_PRINT((void*) &(elt->blk), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".bindex"); 
   INT_SW_DUMP_PRINT((void*) &(elt->bindex), variableNameWithPath); 
} 
void soc_cmap_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_cmap_t *elt = ((soc_cmap_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".*cmap_base"); 
   /* check the pointer is not null */ 
   if (elt->cmap_base) { 
      soc_ctr_ref_t_SW_DUMP_PRINT((void*) &(*elt->cmap_base), variableNameWithPath); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", variableNameWithPath); 
   } 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".cmap_size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->cmap_size), variableNameWithPath); 
} 
void soc_ctr_ref_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_ctr_ref_t *elt = ((soc_ctr_ref_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".reg"); 
   soc_reg_t_SW_DUMP_PRINT((void*) &(elt->reg), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".index"); 
   INT_SW_DUMP_PRINT((void*) &(elt->index), variableNameWithPath); 
} 
void soc_port_phy_timesync_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_port_phy_timesync_config_t *elt = ((soc_port_phy_timesync_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".capabilities"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->capabilities), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".validity_mask"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->validity_mask), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".itpid"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->itpid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".otpid"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->otpid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".otpid2"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->otpid2), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".inband_control"); 
   soc_port_phy_timesync_inband_control_t_SW_DUMP_PRINT((void*) &(elt->inband_control), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".gmode"); 
   soc_port_phy_timesync_global_mode_t_SW_DUMP_PRINT((void*) &(elt->gmode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".framesync"); 
   soc_port_phy_timesync_framesync_t_SW_DUMP_PRINT((void*) &(elt->framesync), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".syncout"); 
   soc_port_phy_timesync_syncout_t_SW_DUMP_PRINT((void*) &(elt->syncout), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ts_divider"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->ts_divider), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".original_timecode"); 
   soc_time_spec_t_SW_DUMP_PRINT((void*) &(elt->original_timecode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx_timestamp_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tx_timestamp_offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_timestamp_offset"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rx_timestamp_offset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_link_delay"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->rx_link_delay), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx_sync_mode"); 
   soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT((void*) &(elt->tx_sync_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx_delay_request_mode"); 
   soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT((void*) &(elt->tx_delay_request_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx_pdelay_request_mode"); 
   soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT((void*) &(elt->tx_pdelay_request_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tx_pdelay_response_mode"); 
   soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT((void*) &(elt->tx_pdelay_response_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_sync_mode"); 
   soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT((void*) &(elt->rx_sync_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_delay_request_mode"); 
   soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT((void*) &(elt->rx_delay_request_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_pdelay_request_mode"); 
   soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT((void*) &(elt->rx_pdelay_request_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rx_pdelay_response_mode"); 
   soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT((void*) &(elt->rx_pdelay_response_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mpls_control"); 
   soc_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT((void*) &(elt->mpls_control), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sync_freq"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sync_freq), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_k1"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_k1), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_k2"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_k2), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_k3"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_k3), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_loop_filter"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_loop_filter), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_ref_phase"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_ref_phase), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_1588_dpll_ref_phase_delta"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->phy_1588_dpll_ref_phase_delta), variableNameWithPath); 
} 
void soc_port_phy_timesync_inband_control_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_inband_control_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_phy_timesync_inband_control_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _shr_port_phy_timesync_inband_control_t *elt = ((_shr_port_phy_timesync_inband_control_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".resv0_id"); 
   INT_SW_DUMP_PRINT((void*) &(elt->resv0_id), variableNameWithPath); 
} 
void soc_port_phy_timesync_global_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_global_mode_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_phy_timesync_global_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_port_phy_timesync_framesync_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_framesync_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_phy_timesync_syncout_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _shr_port_phy_timesync_syncout_t *elt = ((_shr_port_phy_timesync_syncout_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode"); 
   _shr_port_phy_timesync_syncout_mode_t_SW_DUMP_PRINT((void*) &(elt->mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pulse_1_length"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->pulse_1_length), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pulse_2_length"); 
   uint16_SW_DUMP_PRINT((void*) &(elt->pulse_2_length), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".interval"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->interval), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".syncout_ts"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->syncout_ts), variableNameWithPath); 
} 
void _shr_port_phy_timesync_syncout_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_port_phy_timesync_syncout_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_syncout_t_SW_DUMP_PRINT(element, variableName); 
} 
void soc_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_time_spec_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_time_spec_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _shr_time_spec_t *elt = ((_shr_time_spec_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".isnegative"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->isnegative), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".seconds"); 
   uint64_SW_DUMP_PRINT((void*) &(elt->seconds), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".nanoseconds"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->nanoseconds), variableNameWithPath); 
} 
void soc_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_phy_timesync_event_message_egress_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_event_message_ingress_mode_t_SW_DUMP_PRINT(element, variableName); 
} 
void soc_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_phy_timesync_mpls_control_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   _shr_port_phy_timesync_mpls_control_t *elt = ((_shr_port_phy_timesync_mpls_control_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".special_label"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->special_label), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".labels"); 
   arraySizes_SW_DUMP[0]=10; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->labels, _shr_port_phy_timesync_mpls_label_t_SW_DUMP_PRINT, 0, sizeof(_shr_port_phy_timesync_mpls_label_t)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".size"); 
   INT_SW_DUMP_PRINT((void*) &(elt->size), variableNameWithPath); 
} 
void _shr_port_phy_timesync_mpls_label_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   _shr_port_phy_timesync_mpls_label_t *elt = ((_shr_port_phy_timesync_mpls_label_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".value"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->value), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mask"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->mask), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
} 
void soc_port_control_phy_timesync_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_control_phy_timesync_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_control_phy_timesync_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_vm_entry_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   int arraySizes_SW_DUMP[4]; 
   drv_vm_entry_t *elt = ((drv_vm_entry_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".prio"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->prio), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_data"); 
   arraySizes_SW_DUMP[0]=2; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->key_data, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".key_mask"); 
   arraySizes_SW_DUMP[0]=2; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->key_mask, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".act_data"); 
   arraySizes_SW_DUMP[0]=2; 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->act_data, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".w"); 
   arraySizes_SW_DUMP[0]=_SHR_BITDCLSIZE ( DRV_VM_QUAL_COUNT ); 
   printArray(arraySizes_SW_DUMP, 0, 1, variableNameWithPath, (void**) elt->w, uint32_SW_DUMP_PRINT, 0, sizeof(SHR_BITDCL)); 
} 
void drv_vm_qual_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_mcrep_control_flag_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_field_qualify_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_field_action_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_fp_tcam_checksum_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   drv_fp_tcam_checksum_t *elt = ((drv_fp_tcam_checksum_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tcam_error"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->tcam_error), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage_ingress_addr"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->stage_ingress_addr), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage_lookup_addr"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->stage_lookup_addr), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stage_egress_addr"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->stage_egress_addr), variableNameWithPath); 
} 
void drv_policer_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   drv_policer_config_t *elt = ((drv_policer_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".mode"); 
   drv_policer_mode_t_SW_DUMP_PRINT((void*) &(elt->mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ckbits_sec"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ckbits_sec), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_ckbits_sec"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->max_ckbits_sec), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ckbits_burst"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->ckbits_burst), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pkbits_sec"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->pkbits_sec), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_pkbits_sec"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->max_pkbits_sec), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".pkbits_burst"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->pkbits_burst), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".kbits_current"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->kbits_current), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".action_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->action_id), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".sharing_mode"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->sharing_mode), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".entropy_id"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->entropy_id), variableNameWithPath); 
} 
void drv_policer_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_field_stat_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_wred_config_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   drv_wred_config_t *elt = ((drv_wred_config_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".drop_prob"); 
   INT_SW_DUMP_PRINT((void*) &(elt->drop_prob), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".max_threshold"); 
   INT_SW_DUMP_PRINT((void*) &(elt->max_threshold), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".min_threshold"); 
   INT_SW_DUMP_PRINT((void*) &(elt->min_threshold), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".gain"); 
   INT_SW_DUMP_PRINT((void*) &(elt->gain), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".hw_index"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->hw_index), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".refer_count"); 
   INT_SW_DUMP_PRINT((void*) &(elt->refer_count), variableNameWithPath); 
} 
void drv_wred_control_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void drv_wred_counter_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 

 
void ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO *elt = ((ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vxlan"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->vxlan), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".erspan"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->erspan), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".rspan"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->rspan), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".xgs_tm"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->xgs_tm), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".xgs_pp"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->xgs_pp), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".epon"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->epon), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".ipv6_tunnel"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->ipv6_tunnel), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".oam_loopback"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->oam_loopback), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".oam_ts_ccm"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->oam_ts_ccm), variableNameWithPath);
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tdm"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tdm), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tdm_stamp_cud"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tdm_stamp_cud), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".tdm_pmm"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->tdm_pmm), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".stacking"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->stacking), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".two_hop_scheduling"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->two_hop_scheduling), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".l2_remote_cpu"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->l2_remote_cpu), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".otmh_cud"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->otmh_cud), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".trill"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->trill), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".vmac_enable"); 
   uint8_SW_DUMP_PRINT((void*) &(elt->vmac_enable), variableNameWithPath); 
} 
void soc_logical_port_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_logical_port_sw_db_t *elt = ((soc_logical_port_sw_db_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".valid"); 
   INT_SW_DUMP_PRINT((void*) &(elt->valid), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".first_phy_port"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->first_phy_port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".channel"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->channel), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".flags"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->flags), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".encap_mode"); 
   soc_encap_mode_t_SW_DUMP_PRINT((void*) &(elt->encap_mode), variableNameWithPath); 
} 

void soc_phy_port_sw_db_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_phy_port_sw_db_t *elt = ((soc_phy_port_sw_db_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".phy_ports"); 
   soc_pbmp_t_SW_DUMP_PRINT((void*) &(elt->phy_ports), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".interface_type"); 
   soc_port_if_t_SW_DUMP_PRINT((void*) &(elt->interface_type), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".speed"); 
   INT_SW_DUMP_PRINT((void*) &(elt->speed), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".master_port"); 
   soc_port_t_SW_DUMP_PRINT((void*) &(elt->master_port), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".channels_count"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->channels_count), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".latch_down"); 
   INT_SW_DUMP_PRINT((void*) &(elt->latch_down), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".runt_pad"); 
   uint32_SW_DUMP_PRINT((void*) &(elt->runt_pad), variableNameWithPath); 
} 
void soc_port_if_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   _shr_port_if_t_SW_DUMP_PRINT(element, variableName); 
} 
void _shr_port_if_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_encap_mode_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void _shr_port_encap_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   sal_fprintf(output_file, "%s: %d\n", variableName, *((int*) element)); 
} 
void soc_port_unit_info_t_SW_DUMP_PRINT(void* element, char* variableName){ 
   char variableNameWithPath[SW_STATE_DUMP_STRING_LENGTH_MAX]; 
   soc_port_unit_info_t *elt = ((soc_port_unit_info_t*) element); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".quads_out_of_reset"); 
   soc_pbmp_t_SW_DUMP_PRINT((void*) &(elt->quads_out_of_reset), variableNameWithPath); 
   SW_STATE_DUMP_SAFE_STRCPY(variableNameWithPath, variableName);     
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   SW_STATE_DUMP_SAFE_STRCAT(variableNameWithPath, ".all_phy_pbmp"); 
   soc_pbmp_t_SW_DUMP_PRINT((void*) &(elt->all_phy_pbmp), variableNameWithPath); 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
} 
void sw_state_dump(char *file_name) { 
    int arraySizes_SW_DUMP[4];

    if ((output_file = sal_fopen(file_name, "w")) == 0) {
        cli_out("Error opening sw dump file %s\n", file_name);
    }
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_counter_thread_is_running", (void**) _bcm_counter_thread_is_running, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   printArray(arraySizes_SW_DUMP, 0, 1, "dpp_saved_counter_1", (void**) dpp_saved_counter_1, uint64_SW_DUMP_PRINT, 1, sizeof(uint64*)); 
   arraySizes_SW_DUMP[0]=BCM_LOCAL_UNITS_MAX; 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_dpp_field_unit_info", (void**) _bcm_dpp_field_unit_info, _bcm_dpp_field_info_t_SW_DUMP_PRINT, 1, sizeof(bcm_dpp_field_info_OLD_t*));
   _bcm_arad_field_device_qual_info_layer_t_SW_DUMP_PRINT((void*) &(_bcm_arad_field_qual_info), "_bcm_arad_field_qual_info"); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_dpp_init_finished_ok", (void**) _bcm_dpp_init_finished_ok, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_dpp_mpls_bk_info", (void**) _bcm_dpp_mpls_bk_info, _bcm_dpp_mpls_bookkeeping_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_mpls_bookkeeping_t)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_mpls_traverse_ilm_keys", (void**) _bcm_mpls_traverse_ilm_keys, SOC_PPC_FRWRD_ILM_KEY_SW_DUMP_PRINT, 1, sizeof(SOC_PPC_FRWRD_ILM_KEY*)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_bcm_mpls_traverse_ilm_vals", (void**) _bcm_mpls_traverse_ilm_vals, SOC_PPC_FRWRD_DECISION_INFO_SW_DUMP_PRINT, 1, sizeof(SOC_PPC_FRWRD_DECISION_INFO*)); 
   INT_SW_DUMP_PRINT((void*) &(_cell_id_curr[0]), "_cell_id_curr"); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_l3_sw_db", (void**) _l3_sw_db, bcm_dpp_l3_sw_db_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_l3_sw_db_t)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_mirror_sw_db", (void**) _mirror_sw_db, bcm_dpp_mirror_sw_db_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_mirror_sw_db_t)); 
   bcm_dpp_switch_sw_db_t_SW_DUMP_PRINT((void*) &(_switch_sw_db[0]), "_switch_sw_db"); 
   arraySizes_SW_DUMP[0]=FRAGMENT_SIZE; 
   arraySizes_SW_DUMP[1]=3; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_soc_port_tx_handles", (void**) _soc_port_tx_handles, sbusdma_desc_handle_t_SW_DUMP_PRINT, 0, sizeof(sbusdma_desc_handle_t)); 
   uint32_SW_DUMP_PRINT((void*) &(_soc_tx_pending[0]), "_soc_tx_pending"); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_g_unitResDesc", (void**) _g_unitResDesc, _shr_res_unit_desc_t_SW_DUMP_PRINT, 1, sizeof(_shr_res_unit_desc_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=PROG_TT_NOF_PROGS;
   printArray(arraySizes_SW_DUMP, 0, 2, "tt_programs", (void**) tt_programs, ARAD_PP_ISEM_ACCESS_PROGRAM_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PP_ISEM_ACCESS_PROGRAM_INFO)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=PROG_VT_NOF_PROGS;
   printArray(arraySizes_SW_DUMP, 0, 2, "vt_programs", (void**) vt_programs, ARAD_PP_ISEM_ACCESS_PROGRAM_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_PP_ISEM_ACCESS_PROGRAM_INFO)); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Arad_pp_lem_actual_stamp", (void**) Arad_pp_lem_actual_stamp, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "last_program_id", (void**) last_program_id, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "last_program_pointer", (void**) last_program_pointer, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "programs_usage", (void**) programs_usage, ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO)); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_indirect_module_arr", (void**) Soc_indirect_module_arr, SOC_SAND_INDIRECT_MODULE_SW_DUMP_PRINT, 0, sizeof(SOC_SAND_INDIRECT_MODULE)); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_sand_nof_repetitions", (void**) Soc_sand_nof_repetitions, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_interrupt_mask_address_arr", (void**) Soc_interrupt_mask_address_arr, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_mem_check_read_write_protect), "Soc_sand_mem_check_read_write_protect"); 
   /* check the pointer is not null */ 
   if (Soc_sand_handles_list) { 
      SOC_SAND_DELTA_LIST_SW_DUMP_PRINT((void*) &(*Soc_sand_handles_list), "Soc_sand_handles_list"); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", "Soc_sand_handles_list"); 
   } 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_array_size), "Soc_sand_array_size"); 
   /* check the pointer is not null */ 
   if (Soc_sand_chip_descriptors) { 
      SOC_SAND_DEVICE_DESC_SW_DUMP_PRINT((void*) &(*Soc_sand_chip_descriptors), "Soc_sand_chip_descriptors"); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", "Soc_sand_chip_descriptors"); 
   } 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_up_counter), "Soc_sand_up_counter"); 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
   uint8_SW_DUMP_PRINT((void*) &(Soc_register_with_id), "Soc_register_with_id"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_driver_is_started), "Soc_sand_driver_is_started"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_max_num_devices), "Soc_sand_max_num_devices"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_min_time_between_tcm_activation), "Soc_sand_min_time_between_tcm_activation"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_system_ticks_in_ms), "Soc_sand_system_ticks_in_ms"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_tcm_mockup_interrupts), "Soc_sand_tcm_mockup_interrupts"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_big_endian), "Soc_sand_big_endian"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_big_endian_was_checked), "Soc_sand_big_endian_was_checked"); 
   uint8_SW_DUMP_PRINT((void*) &(Soc_sand_ll_is_any_active), "Soc_sand_ll_is_any_active"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_LL_TIMER_MAX_NOF_TIMERS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_sand_ll_timer_cnt", (void**) Soc_sand_ll_timer_cnt, SOC_SAND_LL_TIMER_FUNCTION_SW_DUMP_PRINT, 0, sizeof(SOC_SAND_LL_TIMER_FUNCTION)); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_ll_timer_total), "Soc_sand_ll_timer_total"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_asic_style), "Soc_sand_physical_print_asic_style"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_unit_or_base_address), "Soc_sand_physical_print_unit_or_base_address"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_first_reg), "Soc_sand_physical_print_first_reg"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_indirect_write), "Soc_sand_physical_print_indirect_write"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_last_reg), "Soc_sand_physical_print_last_reg"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_part_of_indirect_read), "Soc_sand_physical_print_part_of_indirect_read"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_part_of_indirect_write), "Soc_sand_physical_print_part_of_indirect_write"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_part_of_read_modify_write), "Soc_sand_physical_print_part_of_read_modify_write"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_print_when_writing), "Soc_sand_physical_print_when_writing"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_physical_write_enable), "Soc_sand_physical_write_enable"); 
   INT_SW_DUMP_PRINT((void*) &(Soc_sand_start_module_shut_down_mutex), "Soc_sand_start_module_shut_down_mutex"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_errors_msg_queue_flagged), "Soc_sand_errors_msg_queue_flagged"); 
   /* check the pointer is not null */ 
   if (Soc_sand_supplied_error_buffer) { 
      CHAR_SW_DUMP_PRINT((void*) &(*Soc_sand_supplied_error_buffer), "Soc_sand_supplied_error_buffer"); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", "Soc_sand_supplied_error_buffer"); 
   } 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_supplied_error_handler_is_on), "Soc_sand_supplied_error_handler_is_on"); 
#if REMOVE_UNTIL_COMPILE /* Removed until compiled */   
   SOC_SAND_RAND_SW_DUMP_PRINT((void*) &(Soc_sand_os_rand), "Soc_sand_os_rand"); 
   /* check the pointer is not null */ 
   if (Soc_sand_callback_delta_list) { 
      SOC_SAND_DELTA_LIST_SW_DUMP_PRINT((void*) &(*Soc_sand_callback_delta_list), "Soc_sand_callback_delta_list"); 
   } else { 
       sal_fprintf(output_file, "%s: null \n ", "Soc_sand_callback_delta_list"); 
   } 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_tcm_enable_polling_flag), "Soc_sand_tcm_enable_polling_flag"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_tcm_request_to_die_flag), "Soc_sand_tcm_request_to_die_flag"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_tcm_task_priority), "Soc_sand_tcm_task_priority"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_tcm_task_stack_size), "Soc_sand_tcm_task_stack_size"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_trace_table_index), "Soc_sand_trace_table_index"); 
   uint32_SW_DUMP_PRINT((void*) &(Soc_sand_trace_table_turn_over), "Soc_sand_trace_table_turn_over"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_sand_workload_status_percent", (void**) Soc_sand_workload_status_percent, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   printArray(arraySizes_SW_DUMP, 0, 1, "Soc_sand_workload_status_total", (void**) Soc_sand_workload_status_total, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   uint8_SW_DUMP_PRINT((void*) &(Arad_sw_db_initialized), "Arad_sw_db_initialized"); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_TMC_IF_NOF_NIFS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_bcm_dpp_egr_interface_multicast_thresh_map", (void**) _bcm_dpp_egr_interface_multicast_thresh_map, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_TMC_NOF_FAP_PORTS_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_bcm_dpp_egr_thresh_map", (void**) _bcm_dpp_egr_thresh_map, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_PPC_MAX_NOF_EGRESS_VLAN_EDIT_ACTION_IDS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_bcm_dpp_vlan_edit_eg_action", (void**) _bcm_dpp_vlan_edit_eg_action, _bcm_dpp_vlan_translate_action_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_vlan_translate_action_t)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_PPC_MAX_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_bcm_dpp_vlan_edit_eg_action_mapping", (void**) _bcm_dpp_vlan_edit_eg_action_mapping, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_PPC_NOF_INGRESS_VLAN_EDIT_ACTION_IDS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_bcm_dpp_vlan_edit_ing_action", (void**) _bcm_dpp_vlan_edit_ing_action, _bcm_dpp_vlan_translate_action_t_SW_DUMP_PRINT, 0, sizeof(_bcm_dpp_vlan_translate_action_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_am_egress_encap", (void**) _dpp_am_egress_encap, bcm_dpp_am_egress_encap_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_am_egress_encap_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_am_ingress_lif", (void**) _dpp_am_ingress_lif, bcm_dpp_am_ingress_lif_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_am_ingress_lif_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_am_sync_lif", (void**) _dpp_am_sync_lif, bcm_dpp_am_sync_lif_t_SW_DUMP_PRINT, 0, sizeof(bcm_dpp_am_sync_lif_t)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_bfd_trap_code_oamp_bfd_cc_mpls_tp", (void**) _dpp_arad_pp_bfd_trap_code_oamp_bfd_cc_mpls_tp, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_bfd_trap_code_oamp_bfd_ipv4", (void**) _dpp_arad_pp_bfd_trap_code_oamp_bfd_ipv4, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_bfd_trap_code_oamp_bfd_mpls", (void**) _dpp_arad_pp_bfd_trap_code_oamp_bfd_mpls, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_bfd_trap_code_oamp_bfd_pwe", (void**) _dpp_arad_pp_bfd_trap_code_oamp_bfd_pwe, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_bfd_trap_code_trap_to_cpu", (void**) _dpp_arad_pp_bfd_trap_code_trap_to_cpu, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_mirror_profile_err_level", (void**) _dpp_arad_pp_oam_mirror_profile_err_level, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_mirror_profile_err_passive", (void**) _dpp_arad_pp_oam_mirror_profile_err_passive, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_mirror_profile_recycle", (void**) _dpp_arad_pp_oam_mirror_profile_recycle, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_mirror_profile_snoop_to_cpu", (void**) _dpp_arad_pp_oam_mirror_profile_snoop_to_cpu, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_drop", (void**) _dpp_arad_pp_oam_trap_code_drop, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_frwrd", (void**) _dpp_arad_pp_oam_trap_code_frwrd, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_recycle", (void**) _dpp_arad_pp_oam_trap_code_recycle, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_snoop", (void**) _dpp_arad_pp_oam_trap_code_snoop, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_trap_to_cpu_level", (void**) _dpp_arad_pp_oam_trap_code_trap_to_cpu_level, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_oam_trap_code_trap_to_cpu_passive", (void**) _dpp_arad_pp_oam_trap_code_trap_to_cpu_passive, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_tcam_bfdcc_cv_last_entry_id", (void**) _dpp_arad_pp_tcam_bfdcc_cv_last_entry_id, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_tcam_mim_last_entry_id", (void**) _dpp_arad_pp_tcam_mim_last_entry_id, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_arad_pp_tcam_mpls_last_entry_id", (void**) _dpp_arad_pp_tcam_mpls_last_entry_id, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_ipv4_multi_hop_acc_ref_counter", (void**) _dpp_bfd_ipv4_multi_hop_acc_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_ipv4_udp_sport_ref_counter", (void**) _dpp_bfd_ipv4_udp_sport_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_is_init", (void**) _dpp_bfd_is_init, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_mpls_tp_cc_ref_counter", (void**) _dpp_bfd_mpls_tp_cc_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_mpls_udp_sport_ref_counter", (void**) _dpp_bfd_mpls_udp_sport_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_bfd_pdu_ref_counter", (void**) _dpp_bfd_pdu_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_oam_acc_ref_counter", (void**) _dpp_oam_acc_ref_counter, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dpp_oam_is_init", (void**) _dpp_oam_is_init, uint8_SW_DUMP_PRINT, 0, sizeof(uint8));
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   printArray(arraySizes_SW_DUMP, 0, 1, "bcm_dpp_fec_remote_lif", (void**) bcm_dpp_fec_remote_lif, uint32_SW_DUMP_PRINT, 1, sizeof(uint32*)); 
   arraySizes_SW_DUMP[0]=BCM_MAX_NUM_UNITS; 
   arraySizes_SW_DUMP[1]=SOC_TMC_NOF_FAP_PORTS_MAX; 
   printArray(arraySizes_SW_DUMP, 0, 2, "bcm_dpp_port_tpids_count", (void**) bcm_dpp_port_tpids_count, uint8_SW_DUMP_PRINT, 0, sizeof(uint8)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dflt_tm_pp_port_map", (void**) _dflt_tm_pp_port_map, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_dflt_tm_pp_port_map", (void**) _dflt_tm_pp_port_map, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_DPP_PORT_RANGE_NUM_ENTRIES; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_internal_port_map", (void**) _internal_port_map, soc_dpp_port_map_t_SW_DUMP_PRINT, 0, sizeof(soc_dpp_port_map_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_DPP_PORT_RANGE_NUM_ENTRIES; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_internal_port_map", (void**) _internal_port_map, soc_dpp_port_map_t_SW_DUMP_PRINT, 0, sizeof(soc_dpp_port_map_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "_modid_set_called", (void**) _modid_set_called, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=_SOC_DPP_MAX_LOCAL_PORTS_ALL_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_port_map", (void**) _port_map, soc_dpp_port_map_t_SW_DUMP_PRINT, 0, sizeof(soc_dpp_port_map_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=_SOC_DPP_MAX_LOCAL_PORTS_ALL_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 2, "_port_map", (void**) _port_map, soc_dpp_port_map_t_SW_DUMP_PRINT, 0, sizeof(soc_dpp_port_map_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "soc_dpp_units", (void**) soc_dpp_units, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "soc_dpp_units", (void**) soc_dpp_units, uint32_SW_DUMP_PRINT, 0, sizeof(uint32)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "soc_dpp_pp_ports", (void**) soc_dpp_pp_ports, soc_dpp_port_t_SW_DUMP_PRINT, 1, sizeof(soc_dpp_port_t*)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "soc_dpp_tm_ports", (void**) soc_dpp_tm_ports, soc_dpp_port_t_SW_DUMP_PRINT, 1, sizeof(soc_dpp_port_t*)); 
   INT_SW_DUMP_PRINT((void*) &(current_hash_tbl_idx), "current_hash_tbl_idx"); 
   INT_SW_DUMP_PRINT((void*) &(current_saved_idx), "current_saved_idx"); 
   INT_SW_DUMP_PRINT((void*) &(nof_active_htb), "nof_active_htb"); 
   arraySizes_SW_DUMP[0]=SOC_SAND_MAX_DEVICE; 
   arraySizes_SW_DUMP[1]=SOC_DPP_WB_HASH_TBLS_NUM; 
   uint8_SW_DUMP_PRINT((void*) &(Arad_proc_desc_initialized), "Arad_proc_desc_initialized"); 
   uint8_SW_DUMP_PRINT((void*) &(Arad_chip_defines_init), "Arad_chip_defines_init"); 
   ARAD_CHIP_DEFINITIONS_SW_DUMP_PRINT((void*) &(Arad_chip_definitions), "Arad_chip_definitions"); 
   uint32_SW_DUMP_PRINT((void*) &(_mc_swdb_nof_asserts), "_mc_swdb_nof_asserts"); 
   cint_data_t_SW_DUMP_PRINT((void*) &(arad_mcdb_cint_data), "arad_mcdb_cint_data"); 
   INT_SW_DUMP_PRINT((void*) &(arad_swdb_asserts_enabled), "arad_swdb_asserts_enabled"); 
#endif /* REMOVE_UNTIL_COMPILE */   
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "programs_usage", (void**) programs_usage, ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO_SW_DUMP_PRINT, 0, sizeof(ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INFO)); 
   
   uint8_SW_DUMP_PRINT((void*) &(Arad_sw_db_initialized), "Arad_sw_db_initialized"); 
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_MAX_NUM_PORTS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "logical_ports_info_snapshot", (void**) logical_ports_info_snapshot, soc_logical_port_sw_db_t_SW_DUMP_PRINT, 0, sizeof(soc_logical_port_sw_db_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_MAX_NUM_PORTS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "logical_ports_info_snapshot", (void**) logical_ports_info_snapshot, soc_logical_port_sw_db_t_SW_DUMP_PRINT, 0, sizeof(soc_logical_port_sw_db_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_MAX_NUM_PORTS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "phy_ports_info_snapshot", (void**) phy_ports_info_snapshot, soc_phy_port_sw_db_t_SW_DUMP_PRINT, 0, sizeof(soc_phy_port_sw_db_t)); 
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   arraySizes_SW_DUMP[1]=SOC_MAX_NUM_PORTS; 
   printArray(arraySizes_SW_DUMP, 0, 2, "phy_ports_info_snapshot", (void**) phy_ports_info_snapshot, soc_phy_port_sw_db_t_SW_DUMP_PRINT, 0, sizeof(soc_phy_port_sw_db_t)); 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "ports_unit_info", (void**) ports_unit_info, soc_port_unit_info_t_SW_DUMP_PRINT, 0, sizeof(soc_port_unit_info_t));
#if SW_DUMP_DISPLAY_NON_RETRIEVED_STATE
   arraySizes_SW_DUMP[0]=SOC_MAX_NUM_DEVICES; 
   printArray(arraySizes_SW_DUMP, 0, 1, "soc_port_sw_db_snapshot_valid", (void**) soc_port_sw_db_snapshot_valid, INT_SW_DUMP_PRINT, 0, sizeof(int)); 
#endif /* SW_DUMP_DISPLAY_NON_RETRIEVED_STATE */

   sal_fclose(output_file);

}
#else 

void sw_state_dump(char *file_name) { 
    cli_out("sw_state_dump.c: sw state dump not supported \n"); 
}

#endif /* !(defined __KERNEL__) */

