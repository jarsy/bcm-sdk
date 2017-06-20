/*
 * $Id: jer2_jer_appl_intr_corr_act_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement header correction action functions for jer2_jericho interrupts.
 */

#ifndef _DNXC_INTR_CORR_ACT_FUNC_H_
#define _DNXC_INTR_CORR_ACT_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dnxc/legacy/dnxc_intr_handler.h>

/*************
 * DEFINES   *
 *************/
#define DNXC_INTERRUPT_PRINT_MSG_SIZE               512
#define DNXC_INTERRUPT_COR_ACT_MSG_SIZE             36
#define DNXC_INTERRUPT_SPECIAL_MSG_SIZE             1000

/*************
 * ENUMERATIONS *
 *************/
typedef enum {
    DNXC_INT_CORR_ACT_CLEAR_CHECK,
    DNXC_INT_CORR_ACT_CONFIG_DRAM,
    DNXC_INT_CORR_ACT_ECC_1B_FIX,
    DNXC_INT_CORR_ACT_EPNI_EM_SOFT_RECOVERY,
    DNXC_INT_CORR_ACT_FORCE,
    DNXC_INT_CORR_ACT_HANDLE_CRC_DEL_BUF_FIFO,
    DNXC_INT_CORR_ACT_HANDLE_MACT_EVENT_FIFO,
    DNXC_INT_CORR_ACT_HANDLE_OAMP_EVENT_FIFO,
    DNXC_INT_CORR_ACT_HANDLE_OAMP_STAT_EVENT_FIFO,
    DNXC_INT_CORR_ACT_HARD_RESET,
    DNXC_INT_CORR_ACT_HARD_RESET_WITHOUT_FABRIC,
    DNXC_INT_CORR_ACT_IHB_EM_SOFT_RECOVERY,
    DNXC_INT_CORR_ACT_IHP_EM_SOFT_RECOVERY,
    DNXC_INT_CORR_ACT_INGRESS_HARD_RESET,
    DNXC_INT_CORR_ACT_IPS_QDESC,
    DNXC_INT_CORR_ACT_NONE,
    DNXC_INT_CORR_ACT_OAMP_EM_SOFT_RECOVERY,
    DNXC_INT_CORR_ACT_PRINT,
    DNXC_INT_CORR_ACT_REPROGRAM_RESOURCE,
    DNXC_INT_CORR_ACT_RTP_LINK_MASK_CHANGE,
    DNXC_INT_CORR_ACT_RX_LOS_HANDLE,
    DNXC_INT_CORR_ACT_SHADOW,
    DNXC_INT_CORR_ACT_SHADOW_AND_SOFT_RESET,
    DNXC_INT_CORR_ACT_SHUTDOWN_FBR_LINKS,
    DNXC_INT_CORR_ACT_SHUTDOWN_UNREACH_DESTINATION,
    DNXC_INT_CORR_ACT_SOFT_RESET,
    DNXC_INT_CORR_ACT_TCAM_SHADOW_FROM_SW_DB,
    DNXC_INT_CORR_ACT_RTP_SLSCT,
    DNXC_INT_CORR_ACT_SHUTDOWN_LINKS,
    DNXC_INT_CORR_ACT_MC_RTP_CORRECT,
    DNXC_INT_CORR_ACT_UC_RTP_CORRECT,
    DNXC_INT_CORR_ACT_ALL_REACHABLE_FIX,
    DNXC_INT_CORR_ACT_EVENT_READY,
    DNXC_INT_CORR_ACT_IPS_QSZ_CORRECT,
    DNXC_INT_CORR_ACT_EM_SOFT_RECOVERY,
    DNXC_INT_CORR_ACT_XOR_FIX,
    DNXC_INT_CORR_ACT_DYNAMIC_CALIBRATION,
    DNXC_INT_CORR_ACT_MAX
} dnxc_int_corr_act_type;

/*************
 * STRUCTURES *
 *************/
typedef struct {
    soc_mem_t mem;
    unsigned int array_index;
    int copyno;
    int min_index;
    int max_index;
} dnxc_interrupt_mem_err_info;

typedef struct dnxc_intr_action_s
{
    soc_handle_interrupt_func func_arr;
    dnxc_int_corr_act_type corr_action;
} dnxc_intr_action_t;

/*************
 * FUNCTIONS *
 *************/

void dnxc_intr_action_info_set(int unit, dnxc_intr_action_t *dnxc_intr_action_info_set);
dnxc_intr_action_t *dnxc_intr_action_info_get(int unit);
    
int dnxc_mem_decide_corrective_action(int unit,dnxc_memory_dc_t type,soc_mem_t mem,int copyno, dnxc_int_corr_act_type *action_type, char* special_msg);
    

int 
dnxc_interrupt_handles_corrective_action_shadow(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg);

int
dnxc_interrupt_handles_corrective_action_for_xor(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* xor_correct_info,
    char* msg);

int 
dnxc_interrupt_handles_corrective_action_for_ecc_1b(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* ecc_1b_correct_info_p,
    char* msg);
int
dnxc_interrupt_handles_corrective_action_do_nothing (
  int unit,
  int block_instance,
  uint32 interrupt_id,
  char *msg);
    
int dnxc_interrupt_handles_corrective_action_soft_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg);

int dnxc_interrupt_handles_corrective_action_hard_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg);        

int
dnxc_interrupt_print_info(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    int recurring_action,
    dnxc_int_corr_act_type corr_act,
    char *special_msg);

int
dnxc_interrupt_handles_corrective_action_print(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char* msg_print,
    char* msg);

/* 
#ifdef BCM_DNX_SUPPORT
int
dnxc_interrupt_handles_corrective_action_for_ips_qsz(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg);

int
dnxc_interrupt_data_collection_for_tcamprotectionerror(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    JER2_ARAD_TCAM_LOCATION *location,
    dnxc_int_corr_act_type* corrective_action);

int
dnxc_interrupt_handles_corrective_action_tcam_shadow_from_sw_db(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    JER2_ARAD_TCAM_LOCATION* location,
    char* msg);
#endif

#ifdef BCM_DNXF_SUPPORT
int
dnxc_interrupt_handles_corrective_action_for_rtp_slsct(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg);
#endif
*/
int
dnxc_interrupt_data_collection_for_shadowing(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    char* special_msg,
    dnxc_int_corr_act_type* p_corrective_action,
    dnxc_interrupt_mem_err_info* shadow_correct_info);
int
dnxc_interrupt_handles_corrective_action(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg,
    dnxc_int_corr_act_type corr_act,
    void *param1,
    void *param2);
#endif /* _DNXC_INTR_CORR_ACT_FUNC_H_ */
