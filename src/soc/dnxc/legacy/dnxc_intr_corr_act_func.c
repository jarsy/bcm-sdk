/*
 * $Id: jer2_jer_appl_intr_corr_act_func.c, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement Correction action functions for jer2_jericho interrupts.
 */

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <sal/core/time.h>
#include <shared/bsl.h>
#include <sal/core/dpc.h>


#include <soc/intr.h>
#include <soc/drv.h>
#include <soc/mem.h>

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/dnxc_intr_corr_act_func.h>
#include <soc/dnxc/legacy/dnxc_ser_correction.h>
#include <soc/dnxc/legacy/dnxc_mem.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

#define JER2_ARAD_XOR_MEM_DATA_MAX_SIZE       0x5
#define JER2_ARAD_MBMP_IS_DYNAMIC(_mem)       _SHR_PBMP_MEMBER(jer2_arad_mem_is_dynamic_bmp[_mem/32], _mem%32)
static dnxc_intr_action_t *dnxc_intr_action_info[SOC_MAX_NUM_DEVICES];

typedef struct {
    soc_mem_t mem;
    int sram_banks_bits;
    int entry_used_bits;
    int direct_write_type;
} dnxc_xor_mem_info;

static dnxc_xor_mem_info jer2_jer_xor_mem_info[] = {
    {IRE_CTXT_MAPm, 2, 10, 0},
    {IRR_MCDBm, 5, 13, 0},
    {IRR_DESTINATION_TABLEm, 3, 12, 0},
    {IRR_LAG_TO_LAG_RANGEm, 2, 8, 0},
    {IRR_LAG_MAPPINGm, 3, 11, 0},
    {IRR_STACK_FEC_RESOLVEm, 2, 8, 0},
    {IRR_STACK_TRUNK_RESOLVEm, 2, 10, 0},
    {IHP_VSI_LOW_CFG_1m, 3, 12, 0},
    {IHB_ISEM_KEYT_PLDT_Hm, 2, 11, 0},
    {PPDB_A_FEC_SUPER_ENTRY_BANKm, 2, 11, 1},
    {PPDB_A_OEMA_KEYT_PLDT_Hm, 2, 10, 1},
    {PPDB_A_OEMB_KEYT_PLDT_Hm, 2, 9, 1},
    {IHP_LIF_TABLEm, 2, 12, 1},
    {PPDB_B_LARGE_EM_KEYT_PLDT_Hm, 5, 12, 1},
    {PPDB_B_LARGE_EM_FID_COUNTER_DBm, 3, 12, 1},
    {PPDB_B_LARGE_EM_FID_PROFILE_DBm, 2, 10, 1},
    {EDB_GLEM_KEYT_PLDT_Hm, 2, 12, 0},
    {OAMP_PE_PROGRAMm, 2, 8, 0},
    {OAMP_PE_GEN_MEMm, 2, 12, 0},
    {INVALIDm}
};

static dnxc_xor_mem_info jer2_qax_xor_mem_info[] = {
    {CGM_VOQ_VSQS_PRMSm, 2, 11, 1},
    {CGM_IPPPMm, 2, 6, 1},
    {IHB_FEC_ENTRYm, 3, 12, 1},
    {IHP_LIF_TABLEm, 4, 11, 1},
    {IHP_VSI_LOW_CFG_1m, 2, 12, 1},
    {OAMP_PE_PROGRAMm, 2, 9, 1},
    {OAMP_PE_GEN_MEMm, 2, 12, 1},
    {PPDB_A_OEMA_KEYT_PLDT_Hm, 2, 9, 1},
    {PPDB_A_OEMB_KEYT_PLDT_Hm, 2, 8, 1},
    {INVALIDm}
};

soc_mem_t jer2_jer_em_mem[] = {
    PPDB_B_LARGE_EM_KEYT_PLDT_Hm,
    PPDB_A_OEMA_KEYT_PLDT_Hm,
    PPDB_A_OEMB_KEYT_PLDT_Hm,
    IHB_ISEM_KEYT_PLDT_Hm,
    OAMP_REMOTE_MEP_EXACT_MATCH_KEYT_PLDT_Hm,
    EDB_GLEM_KEYT_PLDT_Hm,
    EDB_ESEM_KEYT_PLDT_Hm,
    NUM_SOC_MEM
};

/*************
 * FUNCTIONS *
 *************/

void dnxc_intr_action_info_set(int unit, dnxc_intr_action_t *dnxc_intr_action_info_set)
{
    dnxc_intr_action_info[unit] = dnxc_intr_action_info_set;
    return;
}

dnxc_intr_action_t *dnxc_intr_action_info_get(int unit)
{
    return dnxc_intr_action_info[unit];
}

static
int
dnxc_mem_is_em(int unit, soc_mem_t mem)
{
    int index;

    if (!SOC_IS_JERICHO(unit)) {
        return 0;
    }

    for (index = 0; jer2_jer_em_mem[index] != NUM_SOC_MEM; index++) {
        if (jer2_jer_em_mem[index] == mem) {
            return 1;
        }
    }

    return 0;
}

static
soc_mem_severe_t
get_mem_severity(int unit, soc_mem_t mem)
{
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        return SOC_MEM_FIELD_VALID(unit,mem,ECCf) ? SOC_MEM_SEVERE_HIGH : SOC_MEM_SEVERE_LOW;
    }

    return SOC_MEM_SEVERITY(unit,mem);
}

static
dnxc_xor_mem_info *dnxc_xor_mem_info_get(int unit, soc_mem_t mem)
{
    int index;
    dnxc_xor_mem_info *xor_mem_info;

    xor_mem_info = SOC_IS_QAX(unit) ? jer2_qax_xor_mem_info : jer2_jer_xor_mem_info;
    for (index = 0; xor_mem_info[index].mem != INVALIDm; index++) {
        if (xor_mem_info[index].mem == mem) {
            return &(xor_mem_info[index]);
        }
    }

    return NULL;
}

int
dnxc_mem_decide_corrective_action(int unit,dnxc_memory_dc_t type,soc_mem_t mem,int copyno, dnxc_int_corr_act_type *action_type, char* special_msg)
{
    /*
*    Each memory is marked at HW PLs by 1 of the following 3 options: low, medium, severe.
*SW will implement soft error correction for memories according the following guidelines.
*Severe (ecc_1b)-if memory static and accessible SW ecc_1b correction routine will take place. Otherwise, ignore.
*Severe (parity/ecc_2b)-soft reset.
*Medium (ecc_1b)-if memory static and accessible SW ecc_1b correction routine will take place. Otherwise, ignore.
*Medium (parity/ecc_2b) - if memory static and accessible and SW shadow available fix with shadow. Otherwise soft reset
*Low (ecc_1b)-if memory static and accessible SW ecc_1b correction routine will take place. Otherwise, ignore.
*Low (parity/ecc_2b) - - if memory static and accessible and SW shadow available fix with shadow. Otherwise ignore.     
    this little bit outdated for more accurate scheme look in 
    confluence.broadcom.com/display/DNXSW/SER 


    */
    soc_mem_severe_t severity = get_mem_severity(unit,mem);
    uint32 dynamic_mem ;
    uint32 mem_is_accessible ;
    int cache_enable;
    int rc;

    DNXC_INIT_FUNC_DEFS;
    dynamic_mem =  dnxc_tbl_is_dynamic(unit,mem);

    mem_is_accessible = !(soc_mem_is_readonly(unit, mem) || soc_mem_is_writeonly(unit, mem) );

    if (SOC_MEM_TYPE(unit,mem) == SOC_MEM_TYPE_XOR) {
        switch (type) {
        case DNXC_ECC_ECC1B_DC:
        case DNXC_P_1_ECC_ECC1B_DC:
        case DNXC_P_2_ECC_ECC1B_DC:
        case DNXC_P_3_ECC_ECC1B_DC:
            *action_type = DNXC_INT_CORR_ACT_XOR_FIX;
            SOC_EXIT;
        default:
            if ((mem == IHP_VSI_LOW_CFG_1m) || (mem == OAMP_PE_PROGRAMm)) {
                *action_type = DNXC_INT_CORR_ACT_XOR_FIX;
                SOC_EXIT;
            }
            break;
        }
    }

    if (dynamic_mem) {
        switch (type) {

        case DNXC_ECC_ECC1B_DC:
        case DNXC_P_1_ECC_ECC1B_DC:
        case DNXC_P_2_ECC_ECC1B_DC:
        case DNXC_P_3_ECC_ECC1B_DC:
            if (SOC_IS_DNXF(unit) || dnxc_mem_is_em(unit, mem)) {
                *action_type = DNXC_INT_CORR_ACT_ECC_1B_FIX;
            } else {
                *action_type = DNXC_INT_CORR_ACT_NONE;
            }
            SOC_EXIT;
        default:
            if (SOC_IS_JERICHO(unit) && dnxc_mem_is_em(unit, mem)) {
                *action_type = DNXC_INT_CORR_ACT_EM_SOFT_RECOVERY;
                SOC_EXIT;
            } else if (severity == SOC_MEM_SEVERE_HIGH) {
                *action_type = DNXC_INT_CORR_ACT_SOFT_RESET;
                SOC_EXIT;
            }
        }
        *action_type = DNXC_INT_CORR_ACT_NONE;
        SOC_EXIT;
    }

    /* static mem*/
    rc = dnxc_interrupt_memory_cached(unit, mem, copyno, &cache_enable);
    if(rc != SOC_E_NONE) {
        sal_sprintf(special_msg, "mem %s failed to check cachebaility \n",
                    SOC_MEM_NAME(unit, mem));
    }
    DNXC_IF_ERR_EXIT(rc);
    if (mem_is_accessible) {
        switch (type) {

        case DNXC_ECC_ECC1B_DC:
        case DNXC_P_1_ECC_ECC1B_DC:
        case DNXC_P_2_ECC_ECC1B_DC:
        case DNXC_P_3_ECC_ECC1B_DC:
            if (cache_enable && SOC_MEM_FIELD_VALID(unit, mem, ECCf)) {
                *action_type =  DNXC_INT_CORR_ACT_SHADOW;
            } else {
                *action_type =  DNXC_INT_CORR_ACT_ECC_1B_FIX;
            }

            SOC_EXIT;
        default:
            if (!cache_enable) {
                sal_sprintf(special_msg, "mem %s is not cached \n",
                            SOC_MEM_NAME(unit, mem));
                *action_type =  DNXC_INT_CORR_ACT_HARD_RESET;
                SOC_EXIT;
            }
            switch (severity) {
            case SOC_MEM_SEVERE_HIGH:
                *action_type =  DNXC_INT_CORR_ACT_SHADOW_AND_SOFT_RESET;
                SOC_EXIT;
            default:
                *action_type = DNXC_INT_CORR_ACT_SHADOW ;
                SOC_EXIT;
            }
    } } else {

            switch (type) {

            case DNXC_ECC_ECC1B_DC:
            case DNXC_P_1_ECC_ECC1B_DC:
            case DNXC_P_2_ECC_ECC1B_DC:
            case DNXC_P_3_ECC_ECC1B_DC:

                *action_type =  DNXC_INT_CORR_ACT_NONE;
                SOC_EXIT;
            default:
                *action_type =  DNXC_INT_CORR_ACT_HARD_RESET;
            }



    }

    exit:
        DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_data_collection_for_shadowing(
    int unit,
    int block_instance,
    uint32 en_interrupt,
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
    dnxc_memory_dc_t type ;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(special_msg);
    DNXC_NULL_CHECK(p_corrective_action);
    DNXC_NULL_CHECK(shadow_correct_info);
    *p_corrective_action = DNXC_INT_CORR_ACT_NONE;
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
            SOC_MEM_ALIAS_TO_ORIG(unit,mem);
            rc = soc_mem_addr_to_array_element_and_index(unit, mem, addrf, &numels, &index);
            DNXC_IF_ERR_EXIT(rc);

            sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x memory=%s, array element=%d, index=%d",
                        cntf, cnt_overflowf, addrf, memory_name, numels, index);
            /* set corrective action */
            if (mem == IPS_QSZm) {
                *p_corrective_action = DNXC_INT_CORR_ACT_IPS_QSZ_CORRECT;
            } else {
                rc = dnxc_mem_decide_corrective_action(unit, type, mem, block_instance,p_corrective_action, special_msg);
                DNXC_IF_ERR_EXIT(rc);
            } 

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
dnxc_interrupt_handles_corrective_action_shadow(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg)
{
    int rc;
    uint32* data_entry=0;
    int entry_dw;
    int current_index;
    soc_mem_t mem;
    int copyno;

    DNXC_INIT_FUNC_DEFS;

    mem = shadow_correct_info_p->mem;

    copyno = (shadow_correct_info_p->copyno == COPYNO_ALL) ? SOC_MEM_BLOCK_ANY(unit, mem) : shadow_correct_info_p->copyno;

    DNXC_NULL_CHECK(shadow_correct_info_p);
 
    if (dnxc_tbl_is_dynamic(unit, mem)) {
        SOC_EXIT;
    }

    if (!soc_mem_cache_get(unit, mem, copyno)   && mem != IRR_MCDBm) {
         DNXC_IF_ERR_EXIT(SOC_E_UNAVAIL);
    }
 
    entry_dw = soc_mem_entry_words(unit, mem);
    data_entry = sal_alloc((entry_dw * 4), "JER2_ARAD_INTERRUPT Shadow data entry allocation");

    if (NULL == data_entry) {
        DNXC_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    
    for (current_index = shadow_correct_info_p->min_index ; current_index <= shadow_correct_info_p->max_index ; current_index++) {

        rc = dnxc_get_ser_entry_from_cache(unit,   mem, copyno, 
                                           shadow_correct_info_p->array_index, current_index, data_entry);
       DNXC_IF_ERR_EXIT(rc);

        rc = soc_mem_array_write(unit, mem, shadow_correct_info_p->array_index, shadow_correct_info_p->copyno, current_index, data_entry);
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    DNXC_FUNC_RETURN;
}

static
int
dnxc_interrupt_corrected_data_entry(
    int unit,
    int block_instance,
    dnxc_interrupt_mem_err_info* ecc_1b_correct_info_p,
    int current_index,
    int cache_enable,
    int xor_flag,
    uint32 *data_entry)
{
    int rc;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(ecc_1b_correct_info_p);
    rc = dnxc_interrupt_memory_cached(unit, ecc_1b_correct_info_p->mem, block_instance, &cache_enable);

    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
    }
    if (cache_enable) {
        rc = dnxc_get_ser_entry_from_cache(unit,   ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->copyno,
                                           ecc_1b_correct_info_p->array_index, current_index, data_entry);
    } else {
        rc = soc_mem_array_read(unit, ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->array_index, ecc_1b_correct_info_p->copyno, current_index, data_entry);
    }
    DNXC_IF_ERR_EXIT(rc);

    if (xor_flag == 0) {
        if (cache_enable&& SOC_MEM_FIELD_VALID(unit, ecc_1b_correct_info_p->mem, ECCf)) {
            LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"correction for  %s will be done by shadow\n"),
                        SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        }
        else
        {
            LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"correction for  %s will be done by ecc 1 bit correction\n"),
                        SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        }
    }

    /* if mem not in cache and memory have ecc (what should be either we not supposed to be here)*/
    if (!cache_enable && SOC_MEM_FIELD_VALID(unit, ecc_1b_correct_info_p->mem, ECCf) ) {
        uint32 mem_row_bit_width = soc_mem_entry_bits(unit, ecc_1b_correct_info_p->mem) - soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf);
        uint32 ecc_field[1];
        uint32 ecc_field_prev;

        *ecc_field = soc_mem_field32_get(unit, ecc_1b_correct_info_p->mem, data_entry, ECCf);
        ecc_field_prev = *ecc_field;
        if (1<<soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf)< mem_row_bit_width) {
            LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Ecc len:%d for memory %s len%d \n"),
                       soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf), SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem), current_index));
            DNXC_IF_ERR_EXIT(SOC_E_INTERNAL);
        }
        DNXC_IF_ERR_EXIT(ecc_correction(unit,ecc_1b_correct_info_p->mem, mem_row_bit_width,xor_flag, data_entry, ecc_field));
        if (ecc_field_prev != *ecc_field) {
            soc_mem_field32_set(unit,ecc_1b_correct_info_p->mem,data_entry, ECCf, *ecc_field);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_handles_corrective_action_for_xor(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* xor_correct_info,
    char* msg)
{
    int rc;
    int sp_index, sp_bank_size, sp_banks_num;
    int current_index, correct_index, free_index, oper_index;
    int offset, free_offset, cache_enable, entry_dw;
    uint32 *data_entry = NULL;
    dnxc_xor_mem_info *xor_mem_info;
    uint32 eci_global_value;
    uint32 xor_correct_value[JER2_ARAD_XOR_MEM_DATA_MAX_SIZE] = {0}, dump_xor_value[JER2_ARAD_XOR_MEM_DATA_MAX_SIZE] = {0};
    soc_reg_above_64_val_t value, orig_value;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(xor_correct_info);

    rc = dnxc_interrupt_memory_cached(unit, xor_correct_info->mem, block_instance, &cache_enable);
    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
        SOC_EXIT;
    }

    entry_dw = soc_mem_entry_words(unit, xor_correct_info->mem);
    data_entry = sal_alloc((entry_dw * 4), "Data entry allocation");
    if (NULL == data_entry) {
        DNXC_IF_ERR_EXIT(SOC_E_MEMORY);
    }

    sal_memset(data_entry, 0, (entry_dw * 4));
    xor_mem_info = dnxc_xor_mem_info_get(unit, xor_correct_info->mem);
    if (xor_mem_info == NULL) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"The memory %s is not XOR memory.\n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
        SOC_EXIT;
    }

    sp_banks_num = 1 << xor_mem_info->sram_banks_bits;
    if (SOC_MEM_IS_ARRAY(unit, xor_correct_info->mem)) {
        sp_bank_size = SOC_MEM_ELEM_SKIP(unit, xor_correct_info->mem) / sp_banks_num;
    } else {
        sp_bank_size = SOC_MEM_SIZE(unit, xor_correct_info->mem) / sp_banks_num;
    }

    if (SOC_MEM_FIELD_VALID(unit, xor_correct_info->mem, ECCf)) {
        SOC_REG_ABOVE_64_CLEAR(value);
        DNXC_IF_ERR_EXIT(dnxc_disable_block_ecc_check(unit,interrupt_id, xor_correct_info->mem,block_instance,value,orig_value));
    }

    if (cache_enable) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"correction for  %s will be done by shadow\n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
    } else if (SOC_MEM_FIELD_VALID(unit, xor_correct_info->mem, ECCf)) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"correction for  %s will be done by ecc 1 bit correction\n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
    } else {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"The table %s can not be recovered.\n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
        SOC_EXIT;
    }

    for (current_index = xor_correct_info->min_index; current_index <= xor_correct_info->max_index; current_index++) {
        offset = current_index & ((1 << xor_mem_info->entry_used_bits) - 1);
        free_offset = offset / 2;

        /* Update all SP banks value */
        for (sp_index = 0; sp_index < sp_banks_num; sp_index++) {
            correct_index = offset + sp_index * sp_bank_size;
            free_index = free_offset + sp_index * sp_bank_size;

            DNXC_IF_ERR_EXIT(dnxc_interrupt_corrected_data_entry(unit, block_instance, xor_correct_info,
                                                              correct_index, cache_enable, 1, data_entry));

            DNXC_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                                                       xor_correct_info->copyno, correct_index, data_entry));

            if (sp_index ==0) {
                sal_memcpy(dump_xor_value, data_entry, entry_dw * sizeof(uint32));
            }

            for (oper_index = 0; oper_index < JER2_ARAD_XOR_MEM_DATA_MAX_SIZE; oper_index++) {
                xor_correct_value[oper_index] ^= data_entry[oper_index];
            }

            /* Calculate free Data */
            if (xor_mem_info->direct_write_type == 0) {
                if (cache_enable) {
                    rc = dnxc_get_ser_entry_from_cache(unit, xor_correct_info->mem, xor_correct_info->copyno,
                                                       xor_correct_info->array_index, free_index, data_entry);
                } else {
                    rc = soc_mem_array_read(unit, xor_correct_info->mem, xor_correct_info->array_index,
                                                            xor_correct_info->copyno, free_index, data_entry);
                }

                DNXC_IF_ERR_EXIT(rc);
                if (sp_index > 0) {
                    for (oper_index = 0; oper_index < JER2_ARAD_XOR_MEM_DATA_MAX_SIZE; oper_index++) {
                        xor_correct_value[oper_index] ^= data_entry[oper_index];
                    }
                }
            }
        }

        /* Update XOR bank value */
        DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_MEM_OPTIONSr(unit, &eci_global_value));

        soc_reg_field_set(unit, ECI_GLOBAL_MEM_OPTIONSr, &eci_global_value, WRITE_TO_XORf, 1);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_MEM_OPTIONSr(unit, eci_global_value));

        rc = soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index, xor_correct_info->copyno,
                                 xor_mem_info->direct_write_type ? offset : free_offset, xor_correct_value);

        soc_reg_field_set(unit, ECI_GLOBAL_MEM_OPTIONSr, &eci_global_value, WRITE_TO_XORf, 0);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_MEM_OPTIONSr(unit, eci_global_value));

        DNXC_IF_ERR_EXIT(rc);

        sal_memset(xor_correct_value, 0, JER2_ARAD_XOR_MEM_DATA_MAX_SIZE * sizeof(uint32));
        DNXC_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                     xor_correct_info->copyno, offset, xor_correct_value));
        DNXC_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                     xor_correct_info->copyno, offset, dump_xor_value));
    }

    if (SOC_MEM_FIELD_VALID(unit, xor_correct_info->mem, ECCf)) {
        DNXC_IF_ERR_EXIT(dnxc_disable_block_ecc_check(unit,interrupt_id,xor_correct_info->mem,block_instance,orig_value,NULL));
    }

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    DNXC_FUNC_RETURN;
}

int 
dnxc_interrupt_handles_corrective_action_for_ecc_1b(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dnxc_interrupt_mem_err_info* ecc_1b_correct_info_p,
    char* msg)
{
    int rc;
    uint32* data_entry=0;
    int entry_dw;
    int current_index;
    int cache_enable;
    soc_reg_above_64_val_t value, orig_value;
    int copyno;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(ecc_1b_correct_info_p);
    copyno = ecc_1b_correct_info_p->copyno - SOC_MEM_BLOCK_MIN(unit, ecc_1b_correct_info_p->mem);
    rc = dnxc_interrupt_memory_cached(unit, ecc_1b_correct_info_p->mem, block_instance, &cache_enable);

    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
     }
                  
   
    if (SOC_IS_DNX(unit) && (dnxc_tbl_is_dynamic(unit, ecc_1b_correct_info_p->mem) && !dnxc_mem_is_em(unit, ecc_1b_correct_info_p->mem))) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Interrupt will not be handled cause %s is dynamic\n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
    }

    entry_dw = soc_mem_entry_words(unit, ecc_1b_correct_info_p->mem);
    data_entry = sal_alloc((entry_dw * 4), "JER2_JER_INTERRUPT ecc 1 data entry allocation");

    if (NULL == data_entry) {
        DNXC_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Before correction of %s \n"),
                SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));

    /* disable ser for block because we need to read the errornous memory*/
    SOC_REG_ABOVE_64_CLEAR(value);
    DNXC_IF_ERR_EXIT(dnxc_disable_block_ecc_check(unit,interrupt_id, ecc_1b_correct_info_p->mem,copyno,value,orig_value));

    for (current_index = ecc_1b_correct_info_p->min_index ; current_index <= ecc_1b_correct_info_p->max_index ; current_index++) {
        rc = dnxc_interrupt_corrected_data_entry(unit, block_instance, ecc_1b_correct_info_p, current_index, cache_enable, 0, data_entry);
        DNXC_IF_ERR_EXIT(rc);

        rc = soc_mem_array_write(unit, ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->array_index, ecc_1b_correct_info_p->copyno, current_index, data_entry);
        DNXC_IF_ERR_EXIT(rc);

    }
    DNXC_IF_ERR_EXIT(dnxc_disable_block_ecc_check(unit,interrupt_id,ecc_1b_correct_info_p->mem,copyno,orig_value,NULL));

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    DNXC_FUNC_RETURN;
}

int dnxc_interrupt_handles_corrective_action_soft_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg)
{
    int rc = 0;
    DNXC_INIT_FUNC_DEFS;

    rc = soc_device_reset(unit, SOC_DNXC_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET, SOC_DNXC_RESET_ACTION_INOUT_RESET);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

int dnxc_interrupt_handles_corrective_action_hard_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg)
{
    DNXC_INIT_FUNC_DEFS;

    LOG_DEBUG(BSL_LS_BCM_INTR,
             (BSL_META_U(unit,
                         "Decision: Hard reset! interrupt id: %d, block instance: %d \n"),interrupt_id, block_instance));

    
    DNXC_LEGACY_FIXME_ASSERT;

    DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_handles_corrective_action_do_nothing (
  int unit,
  int block_instance,
  uint32 interrupt_id,
  char *msg)
{
    DNXC_INIT_FUNC_DEFS;

    /*empty function*/

    DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_print_info(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    int recurring_action,
    dnxc_int_corr_act_type corr_act,
    char *special_msg)
{
    int rc;
    uint32 flags;
    soc_interrupt_db_t* interrupt;

    char cur_special_msg[DNXC_INTERRUPT_SPECIAL_MSG_SIZE];
    char cur_corr_act_msg[DNXC_INTERRUPT_COR_ACT_MSG_SIZE];
    char print_msg[DNXC_INTERRUPT_PRINT_MSG_SIZE];

    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOC_MSG("invalid unit")));
    }

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    interrupt = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt]);

    if (special_msg == NULL) {
        sal_snprintf(cur_special_msg, sizeof(cur_special_msg), "None");
    } else {
        sal_strncpy(cur_special_msg, special_msg, sizeof(cur_special_msg) - 1);
    }

    rc = soc_interrupt_flags_get(unit, en_interrupt, &flags);
    DNXC_IF_ERR_EXIT(rc);

    /* Corrective action will be performed only if BCM_AND_USR_CB flag set or corrective action override flag not overriden */
    if(((flags & SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB) == 0) && SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE)) {
        corr_act = DNXC_INT_CORR_ACT_NONE;
    }

    switch(corr_act) {
        case DNXC_INT_CORR_ACT_HARD_RESET:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Soft Reset");
            break;
        case DNXC_INT_CORR_ACT_NONE:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "None");
            break;
        case DNXC_INT_CORR_ACT_SOFT_RESET:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Soft Reset");
            break;
        case DNXC_INT_CORR_ACT_PRINT:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Print");
            break;
        case DNXC_INT_CORR_ACT_HANDLE_OAMP_EVENT_FIFO:
            sal_sprintf(cur_corr_act_msg, "Handle OAMP Event Fifo");
            break;
        case DNXC_INT_CORR_ACT_HANDLE_OAMP_STAT_EVENT_FIFO:
            sal_sprintf(cur_corr_act_msg, "Handle OAMP Statistics Event Fifo");
            break;
        case DNXC_INT_CORR_ACT_SHADOW:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Shadow");
            break;
        case DNXC_INT_CORR_ACT_SHUTDOWN_FBR_LINKS:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown link");
            break;
        case DNXC_INT_CORR_ACT_CLEAR_CHECK:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Clear Check");
            break;
        case DNXC_INT_CORR_ACT_CONFIG_DRAM:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Config DRAM");
            break;
        case DNXC_INT_CORR_ACT_ECC_1B_FIX:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "ECC 1b Correct");
            break;
        case DNXC_INT_CORR_ACT_EPNI_EM_SOFT_RECOVERY:
        case DNXC_INT_CORR_ACT_IHB_EM_SOFT_RECOVERY:
        case DNXC_INT_CORR_ACT_IHP_EM_SOFT_RECOVERY:
        case DNXC_INT_CORR_ACT_OAMP_EM_SOFT_RECOVERY:
        case DNXC_INT_CORR_ACT_EM_SOFT_RECOVERY:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "EM Soft Recovery");
            break;
        case DNXC_INT_CORR_ACT_FORCE:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Force");
            break;
        case DNXC_INT_CORR_ACT_HANDLE_CRC_DEL_BUF_FIFO:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Handle CRC Delete Buffer FIFO");
            break;
        case DNXC_INT_CORR_ACT_HANDLE_MACT_EVENT_FIFO:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Handle MACT Event FIFO");
            break;
        case DNXC_INT_CORR_ACT_HARD_RESET_WITHOUT_FABRIC:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Hard Reset without Fabric");
            break;
        case DNXC_INT_CORR_ACT_INGRESS_HARD_RESET:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Ingress Hard Reset");
            break;
        case DNXC_INT_CORR_ACT_IPS_QDESC:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "IPS QDESC Clear Unused");
            break;
        case DNXC_INT_CORR_ACT_REPROGRAM_RESOURCE:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Reprogram Resource");
            break;
        case DNXC_INT_CORR_ACT_RTP_LINK_MASK_CHANGE:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "RTP Link Mask Change");
            break;
        case DNXC_INT_CORR_ACT_RX_LOS_HANDLE:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "RX LOS Handle");
            break;
        case DNXC_INT_CORR_ACT_SHADOW_AND_SOFT_RESET:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Shadow and Soft Reset");
            break;
        case DNXC_INT_CORR_ACT_SHUTDOWN_UNREACH_DESTINATION:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown Unreachable Destination");
            break;
        case DNXC_INT_CORR_ACT_TCAM_SHADOW_FROM_SW_DB:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "TCAM Shadow from SW DB");
            break;
        case DNXC_INT_CORR_ACT_RTP_SLSCT:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "RTP SLSCT");
            break;
        case DNXC_INT_CORR_ACT_SHUTDOWN_LINKS:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown links");
            break;
        case DNXC_INT_CORR_ACT_MC_RTP_CORRECT:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "MC RTP Correct");
            break;
        case DNXC_INT_CORR_ACT_UC_RTP_CORRECT:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "UC RTP Correct");
            break;
        case DNXC_INT_CORR_ACT_ALL_REACHABLE_FIX:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "All Rechable fix");
            break;
        case DNXC_INT_CORR_ACT_IPS_QSZ_CORRECT:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "IPS QSZ Correct");
            break;
        case DNXC_INT_CORR_ACT_XOR_FIX:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "XOR Fix");
            break;
        case DNXC_INT_CORR_ACT_DYNAMIC_CALIBRATION:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Dynamic calibration");
            break;
        default:
            sal_snprintf(cur_corr_act_msg, DNXC_INTERRUPT_COR_ACT_MSG_SIZE, "Unknown");
    }

    /* prepare string for print */
#if defined(SOC_NO_NAMES)
    sal_snprintf(print_msg, DNXC_INTERRUPT_PRINT_MSG_SIZE, "id=%d, index=%d, block=%d, unit=%d, recurring_action=%d | %s | %s\n ",
                en_interrupt, interrupt->reg_index, block_instance, unit, recurring_action, cur_special_msg, cur_corr_act_msg);
#else
    sal_snprintf(print_msg, DNXC_INTERRUPT_PRINT_MSG_SIZE,"name=%s, id=%d, index=%d, block=%d, unit=%d, recurring_action=%d | %s | %s\n ",
                interrupt->name, en_interrupt, interrupt->reg_index, block_instance, unit, recurring_action, cur_special_msg, cur_corr_act_msg);
#endif

    /* Print per interrupt mechanism */
    if(SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE)) {
        LOG_ERROR(BSL_LS_SOC_INTR, (BSL_META_U(unit,"%s"), print_msg));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
dnxc_interrupt_handles_corrective_action_print(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char* msg_print,
    char* msg)
{
    soc_interrupt_db_t* interrupt;
    char print_msg[DNXC_INTERRUPT_PRINT_MSG_SIZE];

    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOC_MSG("invalid unit")));
    }

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    interrupt = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id]);

    /* prepare string for print */
#if defined(SOC_NO_NAMES)
    sal_snprintf(print_msg, DNXC_INTERRUPT_PRINT_MSG_SIZE, "id=%d, index=%d, block=%d, unit=%d | %s\n ",
                interrupt_id, interrupt->reg_index, block_instance, unit, msg_print);
#else
    sal_snprintf(print_msg, DNXC_INTERRUPT_PRINT_MSG_SIZE,"name=%s, id=%d, index=%d, block=%d, unit=%d | %s\n ",
                interrupt->name, interrupt_id, interrupt->reg_index, block_instance, unit, msg_print);
#endif

    LOG_ERROR(BSL_LS_BCM_INTR,
              (BSL_META_U(unit,
                          "%s"),
               print_msg));

exit:
    DNXC_FUNC_RETURN;
}

/* Corrective Action main function */
int
dnxc_interrupt_handles_corrective_action(
    int unit,
    int block_instance,
    uint32 interrupt_id,
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

    case DNXC_INT_CORR_ACT_PRINT:
        rc = dnxc_interrupt_handles_corrective_action_print(unit, block_instance, interrupt_id,(char*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    case DNXC_INT_CORR_ACT_SHADOW:
        rc = dnxc_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;

    case DNXC_INT_CORR_ACT_SHADOW_AND_SOFT_RESET:
        rc = dnxc_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_SOFT_RESET_BLOCKS_FABRIC);
        } else {
            rc = dnxc_interrupt_handles_corrective_action_soft_reset(unit, block_instance, interrupt_id, msg);
            DNXC_IF_ERR_EXIT(rc);
        }
        break;

    case DNXC_INT_CORR_ACT_ECC_1B_FIX:
        rc = dnxc_interrupt_handles_corrective_action_for_ecc_1b(unit, block_instance, interrupt_id,(dnxc_interrupt_mem_err_info*)param1, msg);
        DNXC_IF_ERR_EXIT(rc);
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("UnKnown corrective action")));
  }

exit:
  DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

