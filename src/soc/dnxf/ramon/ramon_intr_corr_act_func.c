/*
 * $Id: ramon_intr_corr_act_func.c, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement Correction action functions for ramon interrupts.
 */

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <sal/core/time.h>
#include <shared/bsl.h>

#include <soc/intr.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/dnxf/cmn/mbcm.h>
#include <soc/dnxc/legacy/dnxc_intr_handler.h>
#include <appl/dcmn/interrupts/interrupt_handler.h>
#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_defs.h>

#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxf/ramon/ramon_intr_cb_func.h>
#include <soc/dnxf/ramon/ramon_intr_corr_act_func.h>
#include <soc/dnxc/legacy/dnxc_ser_correction.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

#define RAMON_INTERRUPTS_SCH_FLOW_TO_FIP_MAPPING_FFM_SIZE 16384
#define RAMON_INTERRUPTS_NOF_FLOWS_PER_FFM_ENTRY 8

/*************
 * FUNCTIONS *
 *************/

int
ramon_interrupt_data_collection_for_shadowing(
    int unit,
    int block_instance,
    ramon_interrupt_type en_interrupt,
    char* special_msg,
    dnxc_int_corr_act_type* p_corrective_action,
    dnxc_interrupt_mem_err_info* shadow_correct_info)
{
    int rc;
    uint32 cntf, cnt_overflowf, addrf, addr_validf;
    soc_reg_t cnt_reg = INVALIDr;
    unsigned numels;
    int index;
    soc_mem_t mem;
    char* memory_name;
    uint32 block;
    dnxc_memory_dc_t type;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(special_msg);
    DNXC_NULL_CHECK(p_corrective_action);
    DNXC_NULL_CHECK(shadow_correct_info);
    *p_corrective_action = DNXC_INT_CORR_ACT_NONE;

    switch(en_interrupt) {

/*  SDK-102608
    case RAMON_INT_DCL_ECC_ECC_2B_ERR_INT:

                cnt_reg = DCL_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_CCS_ECC_ECC_2B_ERR_INT:

                cnt_reg = CCS_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_RTP_ECC_ECC_2B_ERR_INT:

                cnt_reg = RTP_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_DCH_ECC_ECC_2B_ERR_INT:

                cnt_reg = DCH_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_ECI_ECC_ECC_2B_ERR_INT:

                cnt_reg = ECI_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_DCM_ECC_ECC_2B_ERR_INT:

                cnt_reg = DCM_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_FSRD_ECC_ECC_2B_ERR_INT:

                cnt_reg = FSRD_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_FMAC_ECC_ECC_2B_ERR_INT:

                cnt_reg = FMAC_ECC_2B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC2B_DC;
                break;
    case RAMON_INT_DCL_ECC_ECC_1B_ERR_INT:

                cnt_reg = DCL_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_CCS_ECC_ECC_1B_ERR_INT:

                cnt_reg = CCS_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_RTP_ECC_ECC_1B_ERR_INT:

                cnt_reg = RTP_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_DCH_ECC_ECC_1B_ERR_INT:

                cnt_reg = DCH_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_ECI_ECC_ECC_1B_ERR_INT:

                cnt_reg = ECI_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_FMAC_ECC_ECC_1B_ERR_INT:

                cnt_reg = FMAC_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_FSRD_ECC_ECC_1B_ERR_INT:

                cnt_reg = FSRD_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_DCM_ECC_ECC_1B_ERR_INT:

                cnt_reg = DCM_ECC_1B_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_ECC1B_DC;
                break;
    case RAMON_INT_CCS_CRP_PARITY_ERR_INT:

                cnt_reg = CCS_CRP_PARITY_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_PARITY_DC;
                break;
    case RAMON_INT_FMAC_ECC_PARITY_ERR_INT:

                cnt_reg = FMAC_PARITY_ERR_CNTr;
                DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));
                type =  DNXC_ECC_PARITY_DC;
                break;
*/
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Unavail action for interrupt %d\n"),en_interrupt));
      }



    rc = dnxc_get_cnt_reg_values(unit, type, cnt_reg,block_instance,&cntf, &cnt_overflowf, &addrf,&addr_validf);
    DNXC_IF_ERR_EXIT(rc);




    if (addr_validf != 0) {

        mem = soc_addr_to_mem_extended(unit, block, 0xff, addrf);

        if(mem!= INVALIDm) {
            memory_name = SOC_MEM_NAME(unit, mem);
        } else {
            memory_name = NULL;
        }

        switch(mem) {
        case INVALIDm:
            sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x memory is not accessible",
                    cntf, cnt_overflowf, addrf);
            break;
        default:

            rc = soc_mem_addr_to_array_element_and_index(unit, mem, addrf, &numels, &index);
            DNXC_IF_ERR_EXIT(rc);

            sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x memory=%s, array element=%d, index=%d",
                        cntf, cnt_overflowf, addrf, memory_name, numels, index);
            /* set corrective action */
            if (mem == RTP_SLSCTm) {
                *p_corrective_action = DNXC_INT_CORR_ACT_RTP_SLSCT;
            } else {
                rc = dnxc_mem_decide_corrective_action(unit, type, mem, block_instance,p_corrective_action, special_msg);
            }

            DNXC_IF_ERR_EXIT(rc);
            shadow_correct_info->array_index = numels;
            shadow_correct_info->copyno = block_instance + SOC_MEM_BLOCK_MIN(unit, mem);
            shadow_correct_info->min_index = index;
            shadow_correct_info->max_index = index;
            shadow_correct_info->mem = mem;
        }

     } else {
         sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x address is not valid",
                     cntf, cnt_overflowf, addrf);
     }

exit:
    DNXC_FUNC_RETURN;
}

int
ramon_interrupt_handles_corrective_action_shutdown_fbr_link(
    int unit,
    int block_instance,
    ramon_interrupt_type interrupt_id,
    char* msg)
{
    uint32 port = block_instance*4 + SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].bit_in_field;
    uint32 rc;
    DNXC_INIT_FUNC_DEFS;


    rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_enable_set, (unit, port, 0));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;

}

/* Corrective Action main function */
int
ramon_interrupt_handles_corrective_action(
    int unit,
    int block_instance,
    ramon_interrupt_type interrupt_id,
    char *msg,
    dnxc_int_corr_act_type corr_act,
    void *param1,
    void *param2)
{
    int rc;
    uint32 flags;

    DNXC_INIT_FUNC_DEFS;

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    rc = soc_interrupt_flags_get(unit, interrupt_id, &flags);
    DNXC_IF_ERR_EXIT(rc);

    /* Corrective action will be performed only if BCM_AND_USR_CB flag set or corrective action override flag not overriden */
    if(((flags & SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB) == 0) && SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE)) {
        corr_act = DNXC_INT_CORR_ACT_NONE;
    }

    switch(corr_act) {
    case DNXC_INT_CORR_ACT_NONE:
        rc = dnxc_interrupt_handles_corrective_action_do_nothing(unit, block_instance, interrupt_id, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;

    case DNXC_INT_CORR_ACT_SOFT_RESET:
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_SOFT_RESET_BLOCKS_FABRIC);
        } else {
            rc = dnxc_interrupt_handles_corrective_action_soft_reset(unit, block_instance, interrupt_id, msg);
            DNXC_IF_ERR_EXIT(rc);
        }
        break;
    case DNXC_INT_CORR_ACT_HARD_RESET:
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_HARD_RESET);
        } else {
            rc = dnxc_interrupt_handles_corrective_action_hard_reset(unit, block_instance, interrupt_id, msg);
            DNXC_IF_ERR_EXIT(rc);
        }
        break;
    case DNXC_INT_CORR_ACT_SHUTDOWN_FBR_LINKS:
        rc = ramon_interrupt_handles_corrective_action_shutdown_fbr_link(unit, block_instance, interrupt_id, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    case DNXC_INT_CORR_ACT_PRINT:
        rc = dnxc_interrupt_handles_corrective_action_print(unit, block_instance, interrupt_id,(char*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    case DNXC_INT_CORR_ACT_SHADOW:
        rc = dnxc_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    case DNXC_INT_CORR_ACT_ECC_1B_FIX:
        rc = dnxc_interrupt_handles_corrective_action_for_ecc_1b(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    case DNXC_INT_CORR_ACT_SHADOW_AND_SOFT_RESET:
        rc = dnxc_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        BCMDNX_IF_ERR_EXIT(rc);
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_SOFT_RESET_BLOCKS_FABRIC);
        } else {
            rc = dnxc_interrupt_handles_corrective_action_soft_reset(unit, block_instance, interrupt_id, msg);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;
    case DNXC_INT_CORR_ACT_RTP_SLSCT:
/*    	SDK-102608
    	rc = dnxc_interrupt_handles_corrective_action_for_rtp_slsct(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
*/
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("UnKnown corrective action")));
  }

exit:
  DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

