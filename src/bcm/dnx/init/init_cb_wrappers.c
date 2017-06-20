/** \file init_cb_wrappers.c
 * DNX init sequence CB wrapper functions.
 * 
 * The DNX init sequence uses CB functions for init and deinit steps. new functions are written according the required 
 * definitions, old ones however are wrapped and placed in this file. 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_INITSEQDNX

#define ERPP_MAPPING_INLIF_PROFILE_SIZE 256
#define ERPP_MAPPING_OUTLIF_PROFILE_SIZE 16
#define ERPP_FILTER_PER_FWD_CONTEXT_SIZE 64
#define ERPP_FILTER_PER_PORT_SIZE  256
#define FWD_CONTEXT_ETHERNET 6

#include "init_cb_wrappers.h"
#include <soc/dnx/dbal/dbal.h>
#include <bcm_int/dnx/rx/rx.h>
#include <soc/dnx/cmodel/cmodel_reg_access.h>
#include <soc/dnx/dnx_data/dnx_data.h>
#include <soc/dnx/drv.h>
#include <soc/dnx/pemladrv/pemladrv.h>
#include <bcm_int/common/rx.h>
#include <bcm_int/dnx/vlan/vlan.h>
#include <bcm_int/dnx/algo/l3/algo_l3.h>
#include <bcm_int/dnx/l3/l3.h>
#include <shared/dbx/dbx_file.h>
#include <bcm_int/dnx/port/port_pp.h>

/**
 * path to pemla ucode file 
 * (relative to DB path folder)
 */
#  define DB_INIT_DIR_PEMLA_UCODE                         "pemla/u_code_db2pem.txt"

shr_error_e
dnx_init_done_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Notify DNX DATA -  init done
     */
    SHR_IF_ERR_EXIT(dnx_data_mgmt_state_set(unit, DNX_DATA_STATE_F_BCM_INIT_DONE));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_done_deinit(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_NONE, "place holder for actual deinit code\n%s%s%s", EMPTY, EMPTY, EMPTY);
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_info_config_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(soc_dnx_info_config(unit));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_info_config_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_NONE, "place holder for actual deinit code\n%s%s%s", EMPTY, EMPTY, EMPTY);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_feature_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    soc_feature_init(unit);

    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_feature_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_NONE, "place holder for actual deinit code\n%s%s%s", EMPTY, EMPTY, EMPTY);

exit:
    SHR_FUNC_EXIT;
}

#ifdef CMODEL_SERVER_MODE
shr_error_e
dnx_init_cmodel_reg_access_init(
    int unit)
{
    soc_control_t *soc;

    SHR_FUNC_INIT_VARS(unit);

    soc = SOC_CONTROL(unit);

    /*
     * do not enable memscan task, otherwise tr 7 will segmentation fail
     */
    soc->mem_scan_pid = SAL_THREAD_ERROR;
    soc->mem_scan_interval = 0;

    SHR_IF_ERR_EXIT(cmodel_reg_access_init(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_cmodel_reg_access_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(cmodel_reg_access_deinit(unit));

exit:
    SHR_FUNC_EXIT;
}
#endif

shr_error_e
dnx_init_rx_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(_bcm_common_rx_init(unit));
    SHR_IF_ERR_EXIT(bcm_dnx_rx_start(unit, NULL));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_rx_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);
    
    SHR_IF_ERR_EXIT(_SHR_E_NONE);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_l3_algo_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dnx_algo_l3_init(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_l3_algo_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dnx_algo_l3_deinit(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_l3_module_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dnx_l3_module_init(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_l3_module_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(_SHR_E_NONE);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_pp_port_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dnx_pp_port_init(unit));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_pp_port_deinit(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(_SHR_E_NONE);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_hw_overwrite_init(
    int unit)
{
    uint32 data[SOC_REG_ABOVE_64_MAX_SIZE_U32];
    uint64 data64;
    soc_reg_above_64_val_t data_above_64;
    uint32 field_data[SOC_REG_ABOVE_64_MAX_SIZE_U32];
    uint64 field_data64;
    int indx = 0;
    uint32 entry_handle_id = 0;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * LLR configuration
     */
    SHR_IF_ERR_EXIT(READ_IPPE_PINFO_LLRm(unit, SOC_BLOCK_ANY, 1, data));
    field_data[0] = 1;
    soc_mem_field_set(unit, IPPE_PINFO_LLRm, data, INITIAL_VID_ENABLEf, field_data);
    field_data[0] = 0x123;
    soc_mem_field_set(unit, IPPE_PINFO_LLRm, data, DEFAULT_INITIAL_VIDf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPE_PINFO_LLRm(unit, SOC_BLOCK_ALL, 1, data));

    /*
     * VTA Configuration
     */
    SHR_IF_ERR_EXIT(READ_IPPA_VTT_IN_PP_PORT_CONFIGm(unit, 0, SOC_BLOCK_ANY, 1, data));
    field_data[0] = 18;
    soc_mem_field_set(unit, IPPA_VTT_IN_PP_PORT_CONFIGm, data, DEFAULT_PORT_LIFf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPA_VTT_IN_PP_PORT_CONFIGm(unit, 0, SOC_BLOCK_ALL, 1, data));

    /*
     * VTA LIF ARR configuration
     */
    sal_memset(data, 0x0, sizeof(data[0]) * SOC_REG_ABOVE_64_MAX_SIZE_U32);
    SHR_IF_ERR_EXIT(WRITE_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ALL, 41, data));

    /*
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  VLAN_EDIT_VID_2_CONFIG=1"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  VLAN_EDIT_VID_1_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  VLAN_EDIT_PROFILE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  VLAN_EDIT_PCP_DEI_PROFILE_CONFIG=0"
     */
    SHR_IF_ERR_EXIT(READ_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ANY, 41, data));
    field_data[0] = 0x06;
    soc_mem_field_set(unit, IPPA_VTT_FORMAT_CONFIGURATION_TABLEm, data, TYPE_CONFIGf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ALL, 41, data));

    /*
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  TERMINATION_TYPE_CONFIG=0"
     */
    SHR_IF_ERR_EXIT(READ_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ANY, 41, data));
    field_data[0] = 0xd6;
    soc_mem_field_set(unit, IPPA_VTT_FORMAT_CONFIGURATION_TABLEm, data, SYS_INLIF_CONFIGf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ALL, 41, data));

    /*
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  STAT_OBJ_ID_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  STAT_OBJ_CMD_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  SERVICE_TYPE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  QOS_PROFILE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  PROTECTION_PTR_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  PROTECTION_PATH_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  PROPAGATION_PROFILE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  OUT_LIF_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  OAM_LIF_SET_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  NEXT_LAYER_NETWORK_DOMAIN_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  LIF_PROFILE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  LEARN_INFO_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  LEARN_EN_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  FORWARD_DOMAIN_TYPE_CONFIG=0"
     */
    SHR_IF_ERR_EXIT(READ_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ANY, 41, data));
    field_data[0] = 0x392;
    soc_mem_field_set(unit, IPPA_VTT_FORMAT_CONFIGURATION_TABLEm, data, FORWARD_DOMAIN_CONFIGf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPA_VTT_FORMAT_CONFIGURATION_TABLEm(unit, 0, SOC_BLOCK_ALL, 41, data));

    /*
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  FORWARD_DOMAIN_ASSIGNMENT_MODE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  EEI_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  ECN_MAPPING_PROFILE_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  DESTINATION_CONFIG=0"
     * bcm shell "mod IPPA_VTT_FORMAT_CONFIGURATION_TABLE\[0\] 41 1  ACTION_PROFILE_IDX_CONFIG=0"
     * FLPA Configuration
     * Unknown-DA-Profile: addr = {DA-Type(3), In-Lif-DA-Not-Found-Profile, In-PP-Port.DA-Not-Found-Profile} Type: 0-MC, 1-BC, 2-UC,
     * Default Destination = Sys-Port = 0xF1
     */
    SHR_IF_ERR_EXIT(READ_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ANY, 0, data));
    field_data[0] = 0xc00f1;
    soc_mem_field_set(unit, IPPB_UNKNOWN_DA_PROFILEm, data, DESTINATIONf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ALL, 0, data));

    SHR_IF_ERR_EXIT(READ_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ANY, 8, data));
    field_data[0] = 0xc00f1;
    soc_mem_field_set(unit, IPPB_UNKNOWN_DA_PROFILEm, data, DESTINATIONf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ALL, 8, data));

    SHR_IF_ERR_EXIT(READ_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ANY, 16, data));
    field_data[0] = 0xc00f1;
    soc_mem_field_set(unit, IPPB_UNKNOWN_DA_PROFILEm, data, DESTINATIONf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ALL, 16, data));

    SHR_IF_ERR_EXIT(READ_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ANY, 22, data));
    field_data[0] = 0xc00f1;
    soc_mem_field_set(unit, IPPB_UNKNOWN_DA_PROFILEm, data, DESTINATIONf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPB_UNKNOWN_DA_PROFILEm(unit, SOC_BLOCK_ALL, 22, data));

    /*
     * LBP Config
     * Global regs
     */
    SHR_IF_ERR_EXIT(READ_IPPD_FAP_GLOBAL_SYS_HEADER_CFGr(unit, data));
    field_data[0] = 1;
    soc_reg_field_set(unit, IPPD_FAP_GLOBAL_SYS_HEADER_CFGr, data, SYSTEM_HEADERS_MODEf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_IPPD_FAP_GLOBAL_SYS_HEADER_CFGr(unit, data[0]));

    /*
     * LBP regs
     */
    SHR_IF_ERR_EXIT(READ_IPPD_LBP_HEADER_CONFIGr(unit, data));
    field_data[0] = 3;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_RESERVED_BITS_SIZEf, field_data[0]);
    field_data[0] = 22;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_OUT_LIF_SIZEf, field_data[0]);
    field_data[0] = 22;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_IN_LIF_SIZEf, field_data[0]);
    field_data[0] = 8;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_IN_LIF_PROFILE_SIZEf, field_data[0]);
    field_data[0] = 3;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_FWD_LAYER_IDX_SIZEf, field_data[0]);
    field_data[0] = 4;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_FWD_LAYER_ADDITIONAL_INFO_SIZEf, field_data[0]);
    field_data[0] = 18;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_FWD_DOMAIN_SIZEf, field_data[0]);
    field_data[0] = 3;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, PPH_END_OF_PACKET_EDITING_SIZEf, field_data[0]);
    field_data[0] = 0;
    soc_reg_field_set(unit, IPPD_LBP_HEADER_CONFIGr, data, FTMH_TM_DESTINATION_EXT_PRESENTf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_IPPD_LBP_HEADER_CONFIGr(unit, data[0]));

    SHR_IF_ERR_EXIT(READ_IPPD_IS_ETHERNETr(unit, data));
    field_data[0] = 0xfffffff;
    soc_reg_field_set(unit, IPPD_IS_ETHERNETr, data, IS_ETHERNETf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_IPPD_IS_ETHERNETr(unit, data[0]));

    /*
     * LBP Tables
     * Addr: sys_header_profile
     */
    SHR_IF_ERR_EXIT(READ_IPPD_HEADER_PROFILEm(unit, SOC_BLOCK_ANY, 0, data));
    field_data[0] = 1;
    soc_mem_field_set(unit, IPPD_HEADER_PROFILEm, data, BUILD_FTMHf, field_data);
    field_data[0] = 1;
    soc_mem_field_set(unit, IPPD_HEADER_PROFILEm, data, BUILD_PPHf, field_data);
    field_data[0] = 1;
    soc_mem_field_set(unit, IPPD_HEADER_PROFILEm, data, NEVER_ADD_PPH_LEARN_EXTf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPD_HEADER_PROFILEm(unit, SOC_BLOCK_ALL, 0, data));

    /*
     * Addr: cpu_trap_code
     */
    SHR_IF_ERR_EXIT(READ_IPPD_FWD_ACT_PROFILEm(unit, SOC_BLOCK_ANY, 0, data));
    field_data[0] = 1;
    soc_mem_field_set(unit, IPPD_FWD_ACT_PROFILEm, data, FWD_DESTINATION_VALIDf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPPD_FWD_ACT_PROFILEm(unit, SOC_BLOCK_ALL, 0, data));

    /*
     * InLif DB populate
     */
    sal_memset(field_data, 0x0, sizeof(field_data[0]) * SOC_REG_ABOVE_64_MAX_SIZE_U32);
    field_data[0] = 0x00000002;
    field_data[1] = 0x00000000;
    field_data[2] = 0x00010000;
    field_data[3] = 0x2900048C;
    sal_memset(data, 0x0, sizeof(data[0]) * SOC_REG_ABOVE_64_MAX_SIZE_U32);
    soc_mem_field_set(unit, DDHB_MACRO_0_ENTRY_BANKm, data, DATA_0f, field_data);
    SHR_IF_ERR_EXIT(WRITE_DDHB_MACRO_0_ENTRY_BANKm(unit, 0, SOC_BLOCK_ALL, 18, data));

    SHR_IF_ERR_EXIT(READ_CGM_DESTINATION_TABLEm(unit, SOC_BLOCK_ANY, 241, data));
    field_data[0] = 1;
    soc_mem_field_set(unit, CGM_DESTINATION_TABLEm, data, VALIDf, field_data);
    field_data[0] = 1;
    soc_mem_field_set(unit, CGM_DESTINATION_TABLEm, data, TC_PROFILEf, field_data);
    field_data[0] = 256;
    soc_mem_field_set(unit, CGM_DESTINATION_TABLEm, data, QUEUE_NUMBERf, field_data);
    SHR_IF_ERR_EXIT(WRITE_CGM_DESTINATION_TABLEm(unit, SOC_BLOCK_ALL, 241, data));

    /*
     * Yoel
     * write ms cgm0 tbl CGM_TRAFFIC_CLASS_MAPPING_MEM addr 6 value 3'h6
     * key_msb = 2'b00 if destination is sys-port
     */
    SHR_IF_ERR_EXIT(READ_CGM_TRAFFIC_CLASS_MAPPINGm(unit, SOC_BLOCK_ANY, 0, data));
    field_data[0] = 0;
    soc_mem_field_set(unit, CGM_TRAFFIC_CLASS_MAPPINGm, data, TCf, field_data);
    SHR_IF_ERR_EXIT(WRITE_CGM_TRAFFIC_CLASS_MAPPINGm(unit, SOC_BLOCK_ALL, 0, data));

    /*
     * write ms ips0 tbl IPS_QSPM_MEM addr 65 value 15'h100
     */
    SHR_IF_ERR_EXIT(READ_IPS_QSPMm(unit, SOC_BLOCK_ANY, 32, data));
    field_data[0] = 2;
    soc_mem_field_set(unit, IPS_QSPMm, data, SYSTEM_PORTf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPS_QSPMm(unit, SOC_BLOCK_ALL, 32, data));

    SHR_IF_ERR_EXIT(READ_IPS_QSPMm(unit, SOC_BLOCK_ANY, 64, data));
    field_data[0] = 2;
    soc_mem_field_set(unit, IPS_QSPMm, data, SYSTEM_PORTf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPS_QSPMm(unit, SOC_BLOCK_ALL, 64, data));

    SHR_IF_ERR_EXIT(READ_IPS_QSPMm(unit, SOC_BLOCK_ANY, 128, data));
    field_data[0] = 2;
    soc_mem_field_set(unit, IPS_QSPMm, data, SYSTEM_PORTf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPS_QSPMm(unit, SOC_BLOCK_ALL, 128, data));

    SHR_IF_ERR_EXIT(READ_IPS_QSPMm(unit, SOC_BLOCK_ANY, 256, data));
    field_data[0] = 2;
    soc_mem_field_set(unit, IPS_QSPMm, data, SYSTEM_PORTf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPS_QSPMm(unit, SOC_BLOCK_ALL, 256, data));

    /*
     * Set FAF-Dest=21, PP-Port=33
     * write ms ips0 tbl IPS_SPM_MEM addr 256 value 19'h10815
     */
    SHR_IF_ERR_EXIT(READ_IPS_SPMm(unit, SOC_BLOCK_ANY, 2, data));
    field_data[0] = 2;
    soc_mem_field_set(unit, IPS_SPMm, data, PP_DSPf, field_data);
    field_data[0] = 1;
    soc_mem_field_set(unit, IPS_SPMm, data, DST_FAPf, field_data);
    SHR_IF_ERR_EXIT(WRITE_IPS_SPMm(unit, SOC_BLOCK_ALL, 2, data));

    /*
     * bcm shell "mod IPS_SPM 64 1 PP_DSP=2"
     * bcm shell "mod IPS_SPM 64 1 DST_FAP=1"
     * EGRESS Configuration
     * Registers
     */
    SHR_IF_ERR_EXIT(READ_ETPPA_UTILITY_REGSr(unit, data));
    field_data[0] = 0;
    soc_reg_field_set(unit, ETPPA_UTILITY_REGSr, data, BLOCK_IS_ETPPf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_ETPPA_UTILITY_REGSr(unit, data[0]));

    SHR_IF_ERR_EXIT(READ_ETPPA_FAP_GLOBAL_SYS_HEADER_CFGr(unit, data));
    field_data[0] = 0x1;
    soc_reg_field_set(unit, ETPPA_FAP_GLOBAL_SYS_HEADER_CFGr, data, SYSTEM_HEADERS_MODEf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_ETPPA_FAP_GLOBAL_SYS_HEADER_CFGr(unit, data[0]));

    SHR_IF_ERR_EXIT(READ_ETPPA_JERICHO_COMPATIBLE_REGISTERSr(unit, data));
    field_data[0] = 0x5;
    soc_reg_field_set(unit, ETPPA_JERICHO_COMPATIBLE_REGISTERSr, data, JERICHO_FORWARD_CODE_FOR_MPLSf, field_data[0]);
    field_data[0] = 0x1;
    soc_reg_field_set(unit, ETPPA_JERICHO_COMPATIBLE_REGISTERSr, data, JERICHO_FHEI_SIZE_FOR_MPLS_POPf, field_data[0]);
    SHR_IF_ERR_EXIT(WRITE_ETPPA_JERICHO_COMPATIBLE_REGISTERSr(unit, data[0]));

    SHR_IF_ERR_EXIT(READ_ETPPA_SYSTEM_HEADERS_CG_1r(unit, data_above_64));
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LB_KEY_EXT_CG_1_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LB_KEY_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LB_KEY_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LB_KEY_EXT_CG_1_MAPPED_SIZEf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TM_EXT_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TM_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TM_EXT_CG_1_LAYER_IDXf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TM_EXT_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, STACKING_EXT_CG_1_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, STACKING_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, STACKING_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, STACKING_EXT_CG_1_MAPPED_SIZEf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, BIER_EXT_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, BIER_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, BIER_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, BIER_EXT_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, APPLICATION_SPECIFIC_EXT_CG_1_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64,
                                 APPLICATION_SPECIFIC_EXT_CG_1_HEADER_IDXf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64,
                                 APPLICATION_SPECIFIC_EXT_CG_1_LAYER_IDXf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x6);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, APPLICATION_SPECIFIC_EXT_CG_1_SIZEf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0xA);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TSH_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x4);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TSH_CG_1_HEADER_IDXf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TSH_CG_1_LAYER_IDXf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, TSH_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0xC);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, PPH_BASE_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, PPH_BASE_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, PPH_BASE_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0xC);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, PPH_BASE_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, FHEI_EXT_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, FHEI_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, FHEI_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x8);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, FHEI_EXT_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LIF_EXT_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LIF_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LIF_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x9);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LIF_EXT_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x12);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LEARN_EXT_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x4);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LEARN_EXT_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LEARN_EXT_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x13);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, LEARN_EXT_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_BASE_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_BASE_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_BASE_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_BASE_CG_1_SIZEf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0xf);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_DATA_CG_1_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_DATA_CG_1_HEADER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_DATA_CG_1_LAYER_IDXf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x10);
    soc_reg_above_64_field64_set(unit, ETPPA_SYSTEM_HEADERS_CG_1r, data_above_64, UDH_DATA_CG_1_SIZEf, field_data64);
    SHR_IF_ERR_EXIT(WRITE_ETPPA_SYSTEM_HEADERS_CG_1r(unit, data_above_64));

    SHR_IF_ERR_EXIT(READ_ETPPA_PPH_BASE_FIELDS_CG_2r(unit, data_above_64));
    COMPILER_64_SET(field_data64, 0, 0x12);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_LEARN_EXT_PRESENT_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_LEARN_EXT_PRESENT_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x10);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_FHEI_SIZE_CG_2_INNER_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x2);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_FHEI_SIZE_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0xD);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_LIF_EXT_TYPE_CG_2_INNER_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_LIF_EXT_TYPE_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x4F);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_PARSING_START_OFFSET_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x7);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_PARSING_START_OFFSET_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_PARSING_START_TYPE_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x5);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_PARSING_START_TYPE_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x47);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_FWD_LAYER_INDEX_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_FWD_LAYER_INDEX_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x43);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_FORWARDING_LAYER_ADDITIONAL_INFO_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x4);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_FORWARDING_LAYER_ADDITIONAL_INFO_CG_2_WIDTHf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x0);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_FORWARDING_STRENGTH_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_FORWARDING_STRENGTH_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x4B);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_IN_LIF_PROFILE_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x8);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_IN_LIF_PROFILE_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x39);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_FORWARD_DOMAIN_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x12);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_FORWARD_DOMAIN_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x23);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_IN_LIF_CG_2_INNER_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x16);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_IN_LIF_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x1B);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_NWK_QOS_CG_2_INNER_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x8);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_NWK_QOS_CG_2_WIDTHf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x13);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_TTL_CG_2_INNER_OFFSETf,
                                 field_data64);
    COMPILER_64_SET(field_data64, 0, 0x8);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64, PPH_TTL_CG_2_WIDTHf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x53);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_END_OF_PACKET_EDITING_CG_2_INNER_OFFSETf, field_data64);
    COMPILER_64_SET(field_data64, 0, 0x3);
    soc_reg_above_64_field64_set(unit, ETPPA_PPH_BASE_FIELDS_CG_2r, data_above_64,
                                 PPH_END_OF_PACKET_EDITING_CG_2_WIDTHf, field_data64);
    SHR_IF_ERR_EXIT(WRITE_ETPPA_PPH_BASE_FIELDS_CG_2r(unit, data_above_64));

    SHR_IF_ERR_EXIT(READ_ETPPC_CFG_TERMINATION_FWD_CODE_ENr(unit, &data64));
    COMPILER_64_SET(field_data64, 0, 0x1);
    soc_reg64_field_set(unit, ETPPC_CFG_TERMINATION_FWD_CODE_ENr, &data64, CFG_TERMINATION_FWD_CODE_ENf, field_data64);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_CFG_TERMINATION_FWD_CODE_ENr(unit, data64));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 0, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 0, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 1, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 1, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 2, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 2, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 3, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 3, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 4, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 4, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 5, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 5, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 6, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 6, data));

    SHR_IF_ERR_EXIT(READ_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ANY, 7, data));
    field_data[0] = 0x6;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, PCP_DEI_SOURCEf, field_data);
    field_data[0] = 0x3;
    soc_mem_field_set(unit, ETPPC_SIT_PROFILE_MAP_TABLEm, data, TAG_TYPEf, field_data);
    SHR_IF_ERR_EXIT(WRITE_ETPPC_SIT_PROFILE_MAP_TABLEm(unit, SOC_BLOCK_ALL, 7, data));

    /*
     * SAME INTERFACE 
     */

    /*
     * Set default values for outlif profile mapping for same interface filter
     */
    for (indx = 0; indx < ERPP_MAPPING_OUTLIF_PROFILE_SIZE; indx++)
    {
        READ_ERPP_CFG_MAPPING_OUTLIF_PROFILEm(unit, SOC_BLOCK_ANY, indx, data);
        field_data[0] = indx % 4;
        soc_mem_field_set(unit, ERPP_CFG_MAPPING_OUTLIF_PROFILEm, data, OUTLIF_SAME_IFf, field_data);

        /*
         * Enabler for multicast/unicast filter per outlif
         */
        field_data[0] = 1;
        soc_mem_field_set(unit, ERPP_CFG_MAPPING_OUTLIF_PROFILEm, data, ENABLE_MULTICAST_SAME_INTERFACE_FILTERSf,
                          field_data);
        soc_mem_field_set(unit, ERPP_CFG_MAPPING_OUTLIF_PROFILEm, data, OUTLIF_ENABLE_UC_SAME_INTERFACEf, field_data);
        WRITE_ERPP_CFG_MAPPING_OUTLIF_PROFILEm(unit, SOC_BLOCK_ALL, indx, data);

        /*
         * Enabler for same interface filter per outlif profile 
         */
        READ_ERPP_CFG_ENABLE_FILTER_PER_OUTLIF_PROFILEm(unit, SOC_BLOCK_ANY, indx, data);
        field_data[0] = 1;
        soc_mem_field_set(unit, ERPP_CFG_ENABLE_FILTER_PER_OUTLIF_PROFILEm, data,
                          CFG_SAME_INTERFACE_PER_OUTLIF_PROFILEf, field_data);
        WRITE_ERPP_CFG_ENABLE_FILTER_PER_OUTLIF_PROFILEm(unit, SOC_BLOCK_ANY, indx, data);

    }

    /*
     * Set default values for inlif profile mapping for same interface filter
     */
    for (indx = 0; indx < ERPP_MAPPING_INLIF_PROFILE_SIZE; indx++)
    {
        READ_ERPP_CFG_MAPPING_INLIF_PROFILEm(unit, SOC_BLOCK_ANY, indx, data);
        field_data[0] = indx % 4;
        soc_mem_field_set(unit, ERPP_CFG_MAPPING_INLIF_PROFILEm, data, INLIF_SAME_IF_FILTERf, field_data);
        WRITE_ERPP_CFG_MAPPING_INLIF_PROFILEm(unit, SOC_BLOCK_ALL, indx, data);

    }

    /*
     * Set default values for forwarding context mapping for same interface filter
     */
    for (indx = 0; indx < ERPP_FILTER_PER_FWD_CONTEXT_SIZE; indx++)
    {
        READ_ERPP_CFG_MAPPING_FWD_CONTEXTm(unit, SOC_BLOCK_ANY, indx, data);
        /*
         * Map ehternet forwarding context to 0, others to 1 
         */
        if (indx == FWD_CONTEXT_ETHERNET)
        {
            field_data[0] = 0;
        }
        else
        {
            field_data[0] = 1;
        }
        soc_mem_field_set(unit, ERPP_CFG_MAPPING_FWD_CONTEXTm, data, SAME_IF_FILTERf, field_data);
        WRITE_ERPP_CFG_MAPPING_FWD_CONTEXTm(unit, SOC_BLOCK_ALL, indx, data);

        /*
         * Enabler for same interface filter per forwarding context 
         */
        READ_ERPP_CFG_ENABLE_FILTER_PER_FWD_CONTEXTm(unit, SOC_BLOCK_ANY, indx, data);
        /*
         * Enable filtering only for ethernet forwarding context 
         */
        if (indx == FWD_CONTEXT_ETHERNET)
        {
            field_data[0] = 1;
        }
        else
        {
            field_data[0] = 0;
        }
        soc_mem_field_set(unit, ERPP_CFG_ENABLE_FILTER_PER_FWD_CONTEXTm, data, CFG_SAME_INTERFACE_PER_FWD_CONTEXTf,
                          field_data);
        WRITE_ERPP_CFG_ENABLE_FILTER_PER_FWD_CONTEXTm(unit, SOC_BLOCK_ANY, indx, data);
    }

    /*
     * Set default enabling for same interface filtering per port 
     */
    for (indx = 0; indx < ERPP_FILTER_PER_PORT_SIZE; indx++)
    {
        READ_ERPP_CFG_ENABLE_FILTER_PER_PORT_TABLEm(unit, SOC_BLOCK_ANY, indx, data);
        field_data[0] = 1;
        soc_mem_field_set(unit, ERPP_CFG_ENABLE_FILTER_PER_PORT_TABLEm, data, CFG_SAME_INTERFACE_PER_PORTf, field_data);
        WRITE_ERPP_CFG_ENABLE_FILTER_PER_PORT_TABLEm(unit, SOC_BLOCK_ALL, indx, data);

    }

    /*
     * Set default values in SAME_INTERFACE_FILTER table according to jericho mode: 1) Ports-Equal && Out-LIF-is-Port
     * && In-LIF-is-Port 2) Ports-Equal && !Out-LIF-is-Port && !In-LIF-is-Port && LIFs-Equal 
     */

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SAME_INTERFACE_FILTER_TABLE, &entry_handle_id));

    /*
     * 1) Ports-Equal && Out-LIF-is-Port && In-LIF-is-Port, fwd_context is ethernet  (it's mapped to 0).
     */

    /*
     * Setting key fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SYS_PORT_EQUAL, INST_SINGLE, 1);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INLIF_IS_PORT, INST_SINGLE, 1);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTLIF_IS_PORT, INST_SINGLE, 1);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_FWD_CONTEXT, INST_SINGLE, 0);
    /*
     * Setting result to be "enable" 
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER, INST_SINGLE, 1);

    /*
     * Loop through all possible combinations for mapping_by_in_lif_profile, mapping_by_outlif_profile, lifs_equal
     */
    for (indx = 0; indx < 32; indx++)
    {
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_INLIF_PROFILE,
                                   INST_SINGLE, indx & 3);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LIFS_EQUAL, INST_SINGLE, (indx >> 2) & 1);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_OUTLIF_PROFILE,
                                   INST_SINGLE, (indx >> 3) & 3);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    }

    /*
     * 2) Ports-Equal && !Out-LIF-is-Port && !In-LIF-is-Port && LIFs-Equal, fwd_context is ethernet (it's mapped to 0).
     */
    /*
     * Setting key fields 
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SYS_PORT_EQUAL, INST_SINGLE, 1);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INLIF_IS_PORT, INST_SINGLE, 0);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTLIF_IS_PORT, INST_SINGLE, 0);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LIFS_EQUAL, INST_SINGLE, 1);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_FWD_CONTEXT, INST_SINGLE, 0);

    /*
     * Loop through all possible combinations for mapping_by_in_lif_profile, mapping_by_outlif_profile
     */
    for (indx = 0; indx < 16; indx++)
    {
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_INLIF_PROFILE,
                                   INST_SINGLE, indx & 3);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_SAME_INTERFACE_FILTER_BY_OUTLIF_PROFILE,
                                   INST_SINGLE, (indx >> 2) & 3);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));

    }

    /*
     * END OF SAME INTERFACE 
     */

exit:
    dbal_entry_handle_release(unit, entry_handle_id);
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_init_pemla_init(
    int unit)
{
#ifdef CMODEL_SERVER_MODE
    char file_path[RHFILE_MAX_SIZE];
    char file_name[RHFILE_MAX_SIZE];
#endif
    SHR_FUNC_INIT_VARS(unit);

#ifdef CMODEL_SERVER_MODE
    sal_strcpy(file_name, DB_INIT_DIR_PEMLA_UCODE);
    SHR_IF_ERR_EXIT(dbx_file_get_full_path(SOC_CHIP_STRING(unit), file_name, file_path));
    SHR_IF_ERR_EXIT(pemladrv_init(0, file_path));

exit:
#endif
    SHR_FUNC_EXIT;
}
