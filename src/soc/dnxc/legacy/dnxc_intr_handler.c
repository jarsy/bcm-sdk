/*
 * $Id: soc_dnxc_intr_handler.c, v1 16/06/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement soc interrupt handler.
 */

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>

#include <soc/intr.h>
#include <soc/ipoll.h>

#include <soc/drv.h>
#include <soc/register.h>
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/multicast_imp.h>
#endif

#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_intr_corr_act_func.h>

#include <soc/mcm/allenum.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

/*
 *  Interrupt Data Semaphor time out defines
 *      the time out of the deinit interrupts is long because some call-back.
 */
#define SOC_DNXC_INTR_DATA_SEM_TAKE_TO_CB  10000
#define SOC_DNXC_INTR_DATA_SEM_TAKE_TO_DEINIT 20000000

/*************
 * TYPE DEFS *
 *************/
static dnxc_block_control_info_t dnxc_intr_blocks_control_info[SOC_MAX_NUM_DEVICES][NOF_BCM_BLOCKS];



/*************
 * FUNCTIONS *
 *************/

dnxc_memory_dc_t
dnxc_get_cnt_reg_type(int unit,soc_reg_t cnt_reg)
{
    if (SOC_REG_FIELD_VALID(unit,cnt_reg,PARITY_ERR_ADDR_VALIDf)) {
        return DNXC_ECC_PARITY_DC;
    }

    if (SOC_REG_FIELD_VALID(unit,cnt_reg,ECC_2B_ERR_ADDR_VALIDf)) {
        return DNXC_ECC_ECC2B_DC;
    }

    if (SOC_REG_FIELD_VALID(unit,cnt_reg,ECC_1B_ERR_ADDR_VALIDf)) {
        return DNXC_ECC_ECC1B_DC;
    }


    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_1_ECC_2B_ERR_ADDR_VALIDf)) {
        return DNXC_P_1_ECC_ECC2B_DC;
    }

    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_1_ECC_1B_ERR_ADDR_VALIDf)) {
        return DNXC_P_1_ECC_ECC1B_DC;
    }



    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_2_ECC_2B_ERR_ADDR_VALIDf)) {
        return DNXC_P_2_ECC_ECC2B_DC;
    }

    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_2_ECC_1B_ERR_ADDR_VALIDf)) {
        return DNXC_P_2_ECC_ECC1B_DC;
    }



    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_3_ECC_2B_ERR_ADDR_VALIDf)) {
        return DNXC_P_3_ECC_ECC2B_DC;
    }

    if (SOC_REG_FIELD_VALID(unit,cnt_reg,P_3_ECC_1B_ERR_ADDR_VALIDf)) {
        return DNXC_P_3_ECC_ECC1B_DC;
    }


    return DNXC_INVALID_DC;

}


int
dnxc_get_cnt_reg_values(
                            int unit,
                            dnxc_memory_dc_t type,
                            soc_reg_t cnt_reg,
                            int copyno,
                            uint32 *cntf, 
                            uint32 *cnt_overflowf, 
                            uint32 *addrf, 
                            uint32 *addr_validf
                             )
{
    uint64 counter;
    int rc;
    soc_field_t         
        error_addr_valid_field=INVALIDf, 
        error_addr_field=INVALIDf,error_cnt_overflow_field=INVALIDf, 
        error_cnt_field=INVALIDf;
    DNXC_INIT_FUNC_DEFS;

    switch (type) {
    case DNXC_ECC_PARITY_DC:
        error_addr_valid_field = PARITY_ERR_ADDR_VALIDf;
        error_addr_field = PARITY_ERR_ADDRf;
        error_cnt_overflow_field = PARITY_ERR_CNT_OVERFLOWf;
        error_cnt_field = PARITY_ERR_CNTf;
        break;

    case DNXC_ECC_ECC2B_DC:
        error_addr_valid_field = ECC_2B_ERR_ADDR_VALIDf;
        error_addr_field = ECC_2B_ERR_ADDRf;
        error_cnt_overflow_field = ECC_2B_ERR_CNT_OVERFLOWf;
        error_cnt_field = ECC_2B_ERR_CNTf;
        break;
    case DNXC_ECC_ECC1B_DC:
        error_addr_valid_field = ECC_1B_ERR_ADDR_VALIDf;
        error_addr_field = ECC_1B_ERR_ADDRf;
        error_cnt_overflow_field = ECC_1B_ERR_CNT_OVERFLOWf;
        error_cnt_field = ECC_1B_ERR_CNTf;
        break;

    case DNXC_P_1_ECC_ECC1B_DC:
        error_addr_valid_field = P_1_ECC_1B_ERR_ADDR_VALIDf;
        error_addr_field = P_1_ECC_1B_ERR_ADDRf;
        error_cnt_overflow_field = P_1_ECC_1B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_1_ECC_1B_ERR_CNTf;
        break;
    case DNXC_P_1_ECC_ECC2B_DC:
        error_addr_valid_field = P_1_ECC_2B_ERR_ADDR_VALIDf;
        error_addr_field = P_1_ECC_2B_ERR_ADDRf;
        error_cnt_overflow_field = P_1_ECC_2B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_1_ECC_2B_ERR_CNTf;
        break;

    case DNXC_P_2_ECC_ECC1B_DC:
        error_addr_valid_field = P_2_ECC_1B_ERR_ADDR_VALIDf;
        error_addr_field = P_2_ECC_1B_ERR_ADDRf;
        error_cnt_overflow_field = P_2_ECC_1B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_2_ECC_1B_ERR_CNTf;
        break;
    case DNXC_P_2_ECC_ECC2B_DC:
        error_addr_valid_field = P_2_ECC_2B_ERR_ADDR_VALIDf;
        error_addr_field = P_2_ECC_2B_ERR_ADDRf;
        error_cnt_overflow_field = P_2_ECC_2B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_2_ECC_2B_ERR_CNTf;
        break;


    case DNXC_P_3_ECC_ECC1B_DC:
        error_addr_valid_field = P_3_ECC_1B_ERR_ADDR_VALIDf;
        error_addr_field = P_3_ECC_1B_ERR_ADDRf;
        error_cnt_overflow_field = P_3_ECC_1B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_3_ECC_1B_ERR_CNTf;
        break;
    case DNXC_P_3_ECC_ECC2B_DC:
        error_addr_valid_field = P_3_ECC_2B_ERR_ADDR_VALIDf;
        error_addr_field = P_3_ECC_2B_ERR_ADDRf;
        error_cnt_overflow_field = P_3_ECC_2B_ERR_CNT_OVERFLOWf;
        error_cnt_field = P_3_ECC_2B_ERR_CNTf;
        break;

    default:
        LOG_ERROR(BSL_LS_BCM_INTR,
                  (BSL_META_U(unit,
                              "Unknown type %d"),type));
        rc = SOC_E_NOT_FOUND;
        SOC_EXIT;


    }

    rc = soc_reg_get(unit, cnt_reg, copyno, 0, &counter);
    DNXC_IF_ERR_EXIT(rc);

   /* get address validity bit */
   *addr_validf = soc_reg64_field32_get(unit, cnt_reg, counter, error_addr_valid_field);

   /* get memory address bit */
   *addrf = soc_reg64_field32_get(unit, cnt_reg, counter, error_addr_field);

    /* get counter overflow indication  */
   *cnt_overflowf = soc_reg64_field32_get(unit, cnt_reg, counter, error_cnt_overflow_field);

   /* get counter value of 2 bit error */
   *cntf = soc_reg64_field32_get(unit, cnt_reg, counter, error_cnt_field);
exit:
    DNXC_FUNC_RETURN;


}


int
dnxc_interrupt_memory_cached(int unit, soc_reg_t mem, int block_instance, int* cached_flag)
{
    int copyno;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cached_flag);
    switch (mem) {
        case  IRR_MCDBm:
            *cached_flag=1;
            break;
        default:
            copyno = (block_instance == SOC_BLOCK_ALL) ? SOC_BLOCK_ALL : block_instance + SOC_MEM_BLOCK_MIN(unit, mem);
            *cached_flag = soc_mem_cache_get(unit, mem, copyno);

    }

exit:
    DNXC_FUNC_RETURN;
}



int
dnxc_intr_add_handler(
    int unit,
    int en_inter,
    int occurrences,
    int timeCycle,
    soc_handle_interrupt_func inter_action,
    soc_handle_interrupt_func inter_recurring_action)
{
    DNXC_INIT_FUNC_DEFS;
    dnxc_intr_add_handler_ext(unit,en_inter,occurrences,timeCycle,inter_action,inter_recurring_action,NULL);

    DNXC_FUNC_RETURN;
}


int
dnxc_intr_add_handler_ext(
    int unit,
    int en_inter,
    int occurrences,
    int timeCycle,
    soc_handle_interrupt_func inter_action,
    soc_handle_interrupt_func inter_recurring_action,
    char *msg)
{
    DNXC_INIT_FUNC_DEFS;

    SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_inter].recurring_action_cycle_counting = occurrences;
    SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_inter].recurring_action_cycle_time = timeCycle;
    SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_inter].func_arr = inter_action;
    SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_inter].func_arr_recurring_action = inter_recurring_action;
    SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_inter].msg = msg;

    DNXC_FUNC_RETURN;
}

static int
dnxc_intr_interrupt_recurring_detect(int unit, int block_instance, uint32 en_interrupt) 
{
    int currentTime;
    int cycleTime, *startCountingTime;
    int cycleCount, *counter;
    soc_handle_interrupt_func func_arr_recurring_action;
    DNXC_INIT_FUNC_DEFS;

    currentTime = sal_time(); /*seconds*/

    cycleTime = (SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].recurring_action_cycle_time); 
    startCountingTime = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].recurring_action_time);
    func_arr_recurring_action = (SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].func_arr_recurring_action);

    cycleCount = (SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].recurring_action_cycle_counting);
    counter  = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].recurring_action_counters);

    LOG_VERBOSE(BSL_LS_BCM_INTR,
                (BSL_META_U(unit,
                            "%s: en_interrupt=%d, currentTime=%d, cycleTime=%d, *startCountingTime=%d, cycleCount=%d, *counter=%d.\n"), FUNCTION_NAME(), en_interrupt, currentTime, cycleTime, *startCountingTime, cycleCount, *counter));

    if (((cycleTime) <= 0) || ((cycleCount) <=0) || (func_arr_recurring_action==NULL)) {
        return 0;
    }

    if (1 == cycleCount) {
        return 1;
    }

    if (currentTime - *startCountingTime > cycleTime) {
        /*init recurring counting*/
        *startCountingTime = currentTime;
        *counter = 1;
        return 0;
    }

    (*counter)++;
    if (*counter >= cycleCount) {
        (*counter) = 0;
        return 1;
    }

    DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_print_info_get(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    char* special_msg)
{
    int rc;
    uint32 cntf, cnt_overflowf, addrf, addr_validf;
    soc_reg_t cnt_reg = INVALIDr;
    soc_mem_t mem;
    char* memory_name;
    uint32 block;
    dnxc_memory_dc_t type ;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(special_msg);
    cnt_reg = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].cnt_reg;
    if (!SOC_REG_IS_VALID(unit, cnt_reg)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Unavail action for interrupt %d\n"),en_interrupt));
    }
    type = dnxc_get_cnt_reg_type(unit, cnt_reg);

    if (type==DNXC_INVALID_DC) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Unavail action for interrupt %d\n"),en_interrupt));
    }
    DNXC_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));

    rc = dnxc_get_cnt_reg_values(unit, type, cnt_reg,block_instance,&cntf, &cnt_overflowf, &addrf,&addr_validf);
    DNXC_IF_ERR_EXIT(rc);

    if (addr_validf != 0) {

        mem = soc_addr_to_mem_extended(unit, block, 0xff, addrf);
        if(mem!= INVALIDm) {
            SOC_MEM_ALIAS_TO_ORIG(unit,mem);
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
            sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x memory=%s",
                        cntf, cnt_overflowf, addrf, memory_name);

        }

     } else {
         sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x address is not valid",
                     cntf, cnt_overflowf, addrf);
     }

exit:
    DNXC_FUNC_RETURN;
}

void dnxc_intr_switch_event_cb(
    int unit, 
    soc_switch_event_t event, 
    uint32 arg1, 
    uint32 arg2, 
    uint32 arg3, 
    void *userdata)
{
    int interrupt_id       = arg1;
    int block_instance  = arg2;
    soc_interrupt_db_t* interrupt;
    int nof_interrupts = 0;
    uint32 flags;
    int is_storm_count_period, is_storm_nominal;
    int rc;
    int inter_get;
    char info_msg[240];

    DNXC_INIT_FUNC_DEFS;
    COMPILER_REFERENCE(_rv);

    /*handle only interrupts*/
    if(SOC_SWITCH_EVENT_DEVICE_INTERRUPT != event) {
        return;
    }
    
    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOC_MSG("invalid unit")));
    }
    
    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device"))); 
    }

    /*
     * Take the interrupt Data semaphor
     */ 
    if(SOC_CONTROL(unit)->interrupts_info->interrupt_sem != NULL) {
        if( sal_sem_take(SOC_CONTROL(unit)->interrupts_info->interrupt_sem, SOC_DNXC_INTR_DATA_SEM_TAKE_TO_CB) != 0) {
            LOG_ERROR(BSL_LS_BCM_INTR,
                      (BSL_META_U(unit,
                                  "Failed to take the interrupt data Semaphor")));
                       return;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_INTR,
                  (BSL_META_U(unit,
                              "Interrupt data is uninitialiezed")));
                   return;
    }

    rc = soc_nof_interrupts(unit, &nof_interrupts); 
    DNXC_IF_ERR_EXIT(rc);

    if(interrupt_id >= nof_interrupts || interrupt_id < 0){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid interrupt")));
    }

    interrupt = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id]);

    /*mask*/
    rc = soc_interrupt_disable(unit, block_instance, interrupt);
    DNXC_IF_ERR_EXIT(rc);

    /* update statistics */
    rc = soc_interrupt_stat_cnt_increase(unit, block_instance, interrupt_id);
    DNXC_IF_ERR_EXIT(rc);

    /*storm detection*/
    rc = soc_interrupt_update_storm_detection(unit, block_instance, interrupt);
    DNXC_IF_ERR_EXIT(rc);
    rc = soc_interrupt_is_storm(unit, block_instance, interrupt, &is_storm_count_period, &is_storm_nominal);
    DNXC_IF_ERR_EXIT(rc);
    if (is_storm_count_period || is_storm_nominal) {
        /* Storm detected: keep the interrupt masked and return from CB */
        rc = dnxc_interrupt_print_info(unit, block_instance, interrupt_id, 0, DNXC_INT_CORR_ACT_NONE, "Storm Detected");
        DNXC_IF_ERR_EXIT(rc);
        rc = dnxc_interrupt_handles_corrective_action_hard_reset(unit, block_instance, interrupt_id, "Storm Detected");
        DNXC_IF_ERR_EXIT(rc);
    } else {

        /*interrupt_action*/
        if (dnxc_intr_interrupt_recurring_detect(unit, block_instance, interrupt_id)) {
            /*recurring action*/
            char *info = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].msg;
            rc = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].func_arr_recurring_action(unit, block_instance, interrupt_id, info);
            DNXC_IF_ERR_EXIT(rc);
        } else {
            if (SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].func_arr!=NULL) {
                char *info = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].msg;
                rc = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].func_arr(unit, block_instance, interrupt_id, info); 
                DNXC_IF_ERR_EXIT(rc);
            } else {
                rc = dnxc_interrupt_print_info_get(unit, block_instance, interrupt_id, info_msg);
                DNXC_IF_ERR_EXIT(rc);
                rc = dnxc_interrupt_print_info(unit, block_instance, interrupt_id, 0, DNXC_INT_CORR_ACT_NONE, info_msg);
                DNXC_IF_ERR_EXIT(rc);
            }
        }
    }

    /*check if should unmask interrupt after handled*/
    rc = soc_interrupt_flags_get(unit, interrupt_id, &flags);
    DNXC_IF_ERR_EXIT(rc);
    if (flags & SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE) {
        SOC_EXIT;
    }

    /* check if there is an clear function for the interrupt */
    if (NULL == interrupt->interrupt_clear) {
        if (interrupt->vector_id == 0) {
            LOG_WARN(BSL_LS_BCM_SWITCH,
                     (BSL_META_U(unit,
                                 "Warning: Interrupt not cleared, NULL pointer of interrupt_clear, no vector_id\n")));
        } else {
            LOG_WARN(BSL_LS_BCM_SWITCH,
                     (BSL_META_U(unit,
                                 "Warning: call to interrupt clear for vector pointer, nothing done\n")));
        }
    } else {
        rc = interrupt->interrupt_clear(unit, block_instance, interrupt_id);
        if(rc!= SOC_E_UNAVAIL) {
            DNXC_IF_ERR_EXIT(rc);
        }
    }

    rc = soc_interrupt_get(unit, block_instance, interrupt, &inter_get);
    DNXC_IF_ERR_EXIT(rc);

    if (inter_get == 0x0 || (flags & SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK)) {
         if (!(is_storm_count_period || is_storm_nominal)) {
             rc = soc_interrupt_enable(unit, block_instance, interrupt);
             DNXC_IF_ERR_EXIT(rc);
         }
    } else {
        rc = dnxc_interrupt_print_info(unit, block_instance, interrupt_id, 0, DNXC_INT_CORR_ACT_NONE, "Interrupt clear Failed!!! Masked");
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    if(SOC_UNIT_VALID(unit)) {
        sal_sem_give(SOC_CONTROL(unit)->interrupts_info->interrupt_sem);
    }
    return;
}

int
dnxc_intr_handler_deinit(int unit)
{
    sal_sem_t intr_sem;
    DNXC_INIT_FUNC_DEFS;

    if (SOC_CONTROL(unit)->interrupts_info == NULL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOC_MSG("interrupts wasn't initilized")));
    }

    intr_sem = SOC_CONTROL(unit)->interrupts_info->interrupt_sem;
    if(intr_sem != NULL) {
        if( sal_sem_take(intr_sem, SOC_DNXC_INTR_DATA_SEM_TAKE_TO_DEINIT ) != 0) {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOC_MSG("Failed to take the interrupt data Semaphor")));
        }
    } 

    
    DNXC_LEGACY_FIXME_ASSERT;

    if (intr_sem != NULL) {
        sal_sem_destroy(intr_sem);
    }

exit:     
    DNXC_FUNC_RETURN;
}

int
dnxc_intr_handler_short_init(int unit)
{
    dnxc_block_control_info_t config;

    DNXC_INIT_FUNC_DEFS;

    config.gmo_reg = SOC_IS_ARADPLUS_AND_BELOW(unit) ? ECI_GLOBALFr : INVALIDr;
    dnxc_collect_blocks_control_info(unit,dnxc_intr_blocks_control_info[unit],&config);
    DNXC_FUNC_RETURN;
}

int
dnxc_intr_handler_init(int unit)
{
    sal_sem_t intr_sem;
    dnxc_block_control_info_t config;

    DNXC_INIT_FUNC_DEFS;

    config.gmo_reg = SOC_IS_ARADPLUS_AND_BELOW(unit) ? ECI_GLOBALFr : INVALIDr;

    intr_sem = sal_sem_create("Interrupt appl data Sem", sal_sem_BINARY, 0);
    if(intr_sem == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOC_MSG("Failed to create the Interrupt Data Semaphor")));     
    }

    
    DNXC_LEGACY_FIXME_ASSERT;

    dnxc_collect_blocks_control_info(unit,dnxc_intr_blocks_control_info[unit],&config);

    SOC_CONTROL(unit)->interrupts_info->interrupt_sem = intr_sem;
    if(sal_sem_give(SOC_CONTROL(unit)->interrupts_info->interrupt_sem)) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOC_MSG("Failed to give the Interrupts Data Semaphor")));;
    }

exit:
    if (SOC_FAILURE(_rv)) {
        sal_sem_destroy(intr_sem);
        SOC_CONTROL(unit)->interrupts_info->interrupt_sem = NULL; 
    }
    /* In case of failure _rv can't be SOC_E_NONE, and hence, after the check of SOC_FAILURE(_rv) the allocation will be freed. */
    /* coverity[leaked_storage:FALSE] */
    DNXC_FUNC_RETURN;
}


int
dnxc_set_block_control_regs_info_cb(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    dnxc_block_control_info_main_t *m = (dnxc_block_control_info_main_t *)data;
    dnxc_block_control_info_t *map = m->map;
    soc_reg_t reg=ainfo->reg;
    soc_reg_info_t reg_info = SOC_REG_INFO(unit, reg);   
    soc_block_type_t block = dnxc_blktype_to_index(SOC_REG_FIRST_BLK_TYPE(reg_info.block)) ;

    if (SOC_REG_FIELD_VALID(unit,reg,CPU_BYPASS_ECC_PARf)) {
        map[block].gmo_reg = reg;
        return SOC_E_NONE;
    }
    if (sal_strstr(SOC_REG_NAME(unit,reg),"ECC_ERR_1B_MONITOR_MEM_MASK")) {
        map[block].ecc1_monitor_mem_reg = reg;
    }
    return SOC_E_NONE;


}


void dnxc_collect_blocks_control_info(int unit, dnxc_block_control_info_t *map,dnxc_block_control_info_t *config)
{
    soc_interrupt_db_t *ints_db = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
    int i;
    int  nof_ints;
    dnxc_block_control_info_main_t m;
    soc_block_type_t block;
    int rv = 0;

    m.map = map;
    m.config = config;

    soc_nof_interrupts(unit,&nof_ints);
    for (i=0;i<nof_ints;i++) {
        soc_reg_t reg = ints_db[i].cnt_reg;
        soc_reg_info_t reg_info ;   
        soc_block_type_t block;
        if (reg==INVALIDr || !SOC_REG_IS_VALID(unit,reg)) {
            continue;
        }
        reg_info = SOC_REG_INFO(unit, reg);   
        block = dnxc_blktype_to_index(SOC_REG_FIRST_BLK_TYPE(reg_info.block)) ;

        if (SOC_REG_FIELD_VALID(unit,reg,PARITY_ERR_ADDR_VALIDf)) {
            map[block].parity_int=ints_db[i].id;
        }

        if (SOC_REG_FIELD_VALID(unit,reg,ECC_1B_ERR_ADDR_VALIDf)) {
            map[block].ecc1_int=ints_db[i].id;
        }


        if (SOC_REG_FIELD_VALID(unit,reg,ECC_2B_ERR_ADDR_VALIDf)) {
            map[block].ecc2_int=ints_db[i].id;
        }
        map[block].ecc1_monitor_mem_reg = INVALIDr;
        map[block].gmo_reg = INVALIDr;

    }

    for (i=0;i <SOC_BLK_LAST_DNX;i++) {
        if (i>=SOC_BLK_END_NOT_DNX && i < SOC_BLK_FIRST_DNX) {
            continue;
        }
        block = dnxc_blktype_to_index(i); 
        map[block].gmo_reg = config->gmo_reg;
    }
    rv = soc_reg_iterate(unit, dnxc_set_block_control_regs_info_cb, &m); 

    if (SOC_FAILURE(rv)) {
        cli_out("error in soc_reg_iterate - %d\n",rv);
    }
}




int
dnxc_disable_block_ecc_check(int unit, int interrupt_id, soc_mem_t mem, int copyno, soc_reg_above_64_val_t value, soc_reg_above_64_val_t orig_value)
{
    soc_block_t block ;
    soc_reg_t   ecc1_monitor_mem_reg ;

    DNXC_INIT_FUNC_DEFS;

    block = dnxc_blktype_to_index(SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))) ;
    ecc1_monitor_mem_reg = dnxc_intr_blocks_control_info[unit][block].ecc1_monitor_mem_reg;
    if (!SOC_REG_IS_VALID(unit,ecc1_monitor_mem_reg)) {
        cli_out("cant find ecc1_monitor_mem_reg for mem %s\n",SOC_MEM_NAME(unit,mem));
        SOC_EXIT;
    }

    if (orig_value!=NULL) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, ecc1_monitor_mem_reg,  copyno, 0, orig_value));
    }
    else
    {
        soc_interrupt_db_t *ints_db = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
        uint32  cntf;
        uint32  cnt_overflowf;
        uint32  addrf;
        uint32  addr_validf;

        soc_reg_t cnt_reg = ints_db[interrupt_id].cnt_reg;
        DNXC_IF_ERR_EXIT(dnxc_get_cnt_reg_values(unit, dnxc_get_cnt_reg_type(unit,cnt_reg), cnt_reg,copyno,&cntf, &cnt_overflowf, &addrf,&addr_validf));
        if (addr_validf) {
            cli_out("mem %s   cnt_reg doesnt cleared as expected\n",
                    SOC_MEM_NAME(unit,mem));
        }

    }
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit,ecc1_monitor_mem_reg, copyno, 0,value));




exit:
    DNXC_FUNC_RETURN;
}



int dnxc_blktype_to_index(soc_block_t blktype)
{
    int index = blktype;
    if (blktype>SOC_BLK_END_NOT_DNX) {
       index -=(SOC_BLK_FIRST_DNX - SOC_BLK_END_NOT_DNX) ;
    }
    return index;
}


int
dnxc_get_ser_entry_from_cache(int unit,  soc_mem_t mem, int copyno, int array_index, int index, uint32 *data_entry)
{
    int rc;
    uint8 orig_read_mode = SOC_MEM_FORCE_READ_THROUGH(unit);
    uint8 *vmap;
    int mem_array_vmap_offset = index;

    DNXC_INIT_FUNC_DEFS;

    SOC_MEM_ALIAS_TO_ORIG(unit,mem);

    switch (mem) {
#ifdef BCM_DNX_SUPPORT
        case  IRR_MCDBm:
            rc = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mult_get_entry,(unit,index,data_entry)));
            break;
#endif /* BCM_DNX_SUPPORT */
        default:
            SOC_MEM_FORCE_READ_THROUGH_SET(unit, 0); /* read from cache */

            vmap = SOC_MEM_STATE(unit, mem).vmap[copyno];
            if (SOC_MEM_IS_ARRAY(unit, mem)) {
                mem_array_vmap_offset = SOC_MEM_SIZE(unit,mem) * array_index + index;
            }

            if (!CACHE_VMAP_TST(vmap, mem_array_vmap_offset) || SOC_MEM_TEST_SKIP_CACHE(unit)) {
                LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Cache memory %s at entry %u is not available"),
                           SOC_MEM_NAME(unit, mem), index));
            }

            rc = soc_mem_array_read(unit, mem, array_index, copyno, index, data_entry);
            SOC_MEM_FORCE_READ_THROUGH_SET(unit, orig_read_mode); /* read from cache */

    }


    DNXC_IF_ERR_EXIT(rc);


exit:
    DNXC_FUNC_RETURN;
}





int
dnxc_int_name_to_id(int unit, char *name)
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;

}

int
dnxc_fdt_unreachdest_interrupt_clear(int unit, int block_instance, int interrupt_id)
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;

    LOG_DEBUG(BSL_LS_BCM_INTR, (BSL_META_U(unit,"clear FDT UnreachDest interrupt, block %d, interrupt %d"),
                                block_instance, interrupt_id));

    DNXC_IF_ERR_EXIT(READ_FDT_UNREACHABLE_DESTINATION_DISCARDEDr(unit, &reg_val));
exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME



