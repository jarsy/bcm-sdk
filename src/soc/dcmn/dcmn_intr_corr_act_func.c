/*
 * $Id: jer_appl_intr_corr_act_func.c, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement Correction action functions for jericho interrupts.
 */

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <sal/core/time.h>
#include <shared/bsl.h>
#include <sal/core/dpc.h>
#include <appl/dcmn/interrupts/interrupt_handler.h>
#ifdef BCM_DPP_SUPPORT
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/mbcm.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/arad_api_dram.h> 
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp.h>
#endif

#ifdef BCM_DFE_SUPPORT
#include <soc/dfe/cmn/dfe_drv.h>
#include <appl/dcmn/rx_los/rx_los.h>
#endif

#include <soc/intr.h>
#include <soc/drv.h>
#include <soc/mem.h>

#include <soc/dcmn/dcmn_defs.h>
#include <soc/dcmn/dcmn_intr_corr_act_func.h>
#include <soc/dcmn/dcmn_ser_correction.h>
#include <soc/dcmn/dcmn_mem.h>
#ifdef BCM_DPP_SUPPORT
#include <soc/dcmn/dcmn_pp_em_ser.h>
#endif

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

#define ARAD_XOR_MEM_DATA_MAX_SIZE       0x5
#define ARAD_MBMP_IS_DYNAMIC(_mem)       _SHR_PBMP_MEMBER(arad_mem_is_dynamic_bmp[_mem/32], _mem%32)
static dcmn_intr_action_t *dcmn_intr_action_info[SOC_MAX_NUM_DEVICES];

typedef struct {
    soc_mem_t mem;
    int sram_banks_bits;
    int entry_used_bits;
    int direct_write_type;
} dcmn_xor_mem_info;

static dcmn_xor_mem_info jer_xor_mem_info[] = {
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

static dcmn_xor_mem_info qax_xor_mem_info[] = {
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

soc_mem_t jer_em_mem[] = {
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

void dcmn_intr_action_info_set(int unit, dcmn_intr_action_t *dcmn_intr_action_info_set)
{
    dcmn_intr_action_info[unit] = dcmn_intr_action_info_set;
    return;
}

dcmn_intr_action_t *dcmn_intr_action_info_get(int unit)
{
    return dcmn_intr_action_info[unit];
}

STATIC
int
dcmn_mem_is_em(int unit, soc_mem_t mem)
{
    int index;

    if (!SOC_IS_JERICHO(unit)) {
        return 0;
    }

    for (index = 0; jer_em_mem[index] != NUM_SOC_MEM; index++) {
        if (jer_em_mem[index] == mem) {
            return 1;
        }
    }

    return 0;
}

STATIC
soc_mem_severe_t
get_mem_severity(int unit, soc_mem_t mem)
{
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        return SOC_MEM_FIELD_VALID(unit,mem,ECCf) ? SOC_MEM_SEVERE_HIGH : SOC_MEM_SEVERE_LOW;
    }

    return SOC_MEM_SEVERITY(unit,mem);
}

STATIC
dcmn_xor_mem_info *dcmn_xor_mem_info_get(int unit, soc_mem_t mem)
{
    int index;
    dcmn_xor_mem_info *xor_mem_info;

    xor_mem_info = SOC_IS_QAX(unit) ? qax_xor_mem_info : jer_xor_mem_info;
    for (index = 0; xor_mem_info[index].mem != INVALIDm; index++) {
        if (xor_mem_info[index].mem == mem) {
            return &(xor_mem_info[index]);
        }
    }

    return NULL;
}

int
dcmn_mem_decide_corrective_action(int unit,dcmn_memory_dc_t type,soc_mem_t mem,int copyno, dcmn_int_corr_act_type *action_type, char* special_msg)
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

    SOCDNX_INIT_FUNC_DEFS;
    dynamic_mem =  dcmn_tbl_is_dynamic(unit,mem);

    mem_is_accessible = !(soc_mem_is_readonly(unit, mem) || soc_mem_is_writeonly(unit, mem) );

    if (SOC_MEM_TYPE(unit,mem) == SOC_MEM_TYPE_XOR) {
        switch (type) {
        case DCMN_ECC_ECC1B_DC:
        case DCMN_P_1_ECC_ECC1B_DC:
        case DCMN_P_2_ECC_ECC1B_DC:
        case DCMN_P_3_ECC_ECC1B_DC:
            if (dcmn_mem_is_em(unit, mem)) {
                *action_type = DCMN_INT_CORR_ACT_EM_ECC_1B_FIX;
            } else {
                *action_type = DCMN_INT_CORR_ACT_XOR_FIX;
            }
            SOC_EXIT;
        default:
            if ((mem == IHP_VSI_LOW_CFG_1m) || (mem == OAMP_PE_PROGRAMm)) {
                *action_type = DCMN_INT_CORR_ACT_XOR_FIX;
                SOC_EXIT;
            }
            break;
        }
    }

    if (dynamic_mem) {
        switch (type) {

        case DCMN_ECC_ECC1B_DC:
        case DCMN_P_1_ECC_ECC1B_DC:
        case DCMN_P_2_ECC_ECC1B_DC:
        case DCMN_P_3_ECC_ECC1B_DC:
            if (SOC_IS_DFE(unit)) {
                *action_type = DCMN_INT_CORR_ACT_ECC_1B_FIX;
            } else if (dcmn_mem_is_em(unit, mem)) {
                *action_type = DCMN_INT_CORR_ACT_EM_ECC_1B_FIX;
            } else {
                *action_type = DCMN_INT_CORR_ACT_NONE;
            }
            SOC_EXIT;
        default:
            if (SOC_IS_JERICHO(unit) && dcmn_mem_is_em(unit, mem)) {
                *action_type = DCMN_INT_CORR_ACT_EM_SOFT_RECOVERY;
                SOC_EXIT;
            } else if (severity == SOC_MEM_SEVERE_HIGH) {
                *action_type = DCMN_INT_CORR_ACT_SOFT_RESET;
                SOC_EXIT;
            }
        }
        *action_type = DCMN_INT_CORR_ACT_NONE;
        SOC_EXIT;
    }

    /* static mem*/
    rc = interrupt_memory_cached(unit, mem, copyno, &cache_enable);
    if(rc != SOC_E_NONE) {
        sal_sprintf(special_msg, "mem %s failed to check cachebaility \n",
                    SOC_MEM_NAME(unit, mem));
    }
    SOCDNX_IF_ERR_EXIT(rc);
    if (mem_is_accessible) {
        switch (type) {

        case DCMN_ECC_ECC1B_DC:
        case DCMN_P_1_ECC_ECC1B_DC:
        case DCMN_P_2_ECC_ECC1B_DC:
        case DCMN_P_3_ECC_ECC1B_DC:
            if (cache_enable && SOC_MEM_FIELD_VALID(unit, mem, ECCf)) {
                *action_type =  DCMN_INT_CORR_ACT_SHADOW;
            } else {
                *action_type =  DCMN_INT_CORR_ACT_ECC_1B_FIX;
            }

            SOC_EXIT;
        default:
            if (!cache_enable) {
                sal_sprintf(special_msg, "mem %s is not cached \n",
                            SOC_MEM_NAME(unit, mem));
                *action_type =  DCMN_INT_CORR_ACT_HARD_RESET;
                SOC_EXIT;
            }
            switch (severity) {
            case SOC_MEM_SEVERE_HIGH:
                *action_type =  DCMN_INT_CORR_ACT_SHADOW_AND_SOFT_RESET;
                SOC_EXIT;
            default:
                *action_type = DCMN_INT_CORR_ACT_SHADOW ;
                SOC_EXIT;
            }
    } } else {

            switch (type) {

            case DCMN_ECC_ECC1B_DC:
            case DCMN_P_1_ECC_ECC1B_DC:
            case DCMN_P_2_ECC_ECC1B_DC:
            case DCMN_P_3_ECC_ECC1B_DC:

                *action_type =  DCMN_INT_CORR_ACT_NONE;
                SOC_EXIT;
            default:
                *action_type =  DCMN_INT_CORR_ACT_HARD_RESET;
            }



    }

    exit:
        SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_data_collection_for_shadowing(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    char* special_msg,
    dcmn_int_corr_act_type* p_corrective_action,
    dcmn_interrupt_mem_err_info* shadow_correct_info)
{
    int rc;
    uint32 cntf, cnt_overflowf, addrf, addr_validf;
    soc_reg_t cnt_reg = INVALIDr;
    unsigned numels;
    int index;
    soc_mem_t mem;
    char* memory_name;
    uint32 block;
    dcmn_memory_dc_t type ;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(special_msg);
    SOCDNX_NULL_CHECK(p_corrective_action);
    SOCDNX_NULL_CHECK(shadow_correct_info);
    *p_corrective_action = DCMN_INT_CORR_ACT_NONE;
    cnt_reg = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt].cnt_reg;
    if (!SOC_REG_IS_VALID(unit, cnt_reg)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Unavail action for interrupt %d\n"),en_interrupt));
    }
    type = get_cnt_reg_type(unit, cnt_reg);

    if (type==DCMN_INVALID_DC) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Unavail action for interrupt %d\n"),en_interrupt));
    }
    SOCDNX_IF_ERR_EXIT(soc_get_reg_first_block_id(unit,cnt_reg,&block));

    rc = dcmn_get_cnt_reg_values(unit, type, cnt_reg,block_instance,&cntf, &cnt_overflowf, &addrf,&addr_validf);
    SOCDNX_IF_ERR_EXIT(rc);

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
            SOCDNX_IF_ERR_EXIT(rc);

            sal_sprintf(special_msg, "nof_occurences=%04u, cnt_overflowf=0x%01x, memory address=0x%08x memory=%s, array element=%d, index=%d",
                        cntf, cnt_overflowf, addrf, memory_name, numels, index);
            /* set corrective action */
            if (mem == IPS_QSZm) {
                *p_corrective_action = DCMN_INT_CORR_ACT_IPS_QSZ_CORRECT;
            } else {
                rc = dcmn_mem_decide_corrective_action(unit, type, mem, block_instance,p_corrective_action, special_msg);
                SOCDNX_IF_ERR_EXIT(rc);
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
    SOCDNX_FUNC_RETURN;
}
STATIC soc_error_t calc_par(
        int             unit,
        const uint32    len,
        const uint32    width,
        const uint32   *data,
        uint32         *parity)
{
    uint32 i, j;
    uint32 pat[SOC_MAX_MEM_WORDS]; 

    if((len == 0) || (width == 0) || (len < width) || (data == NULL) || (parity == NULL)) {
        return 0;
    }

    sal_memset(parity, 0, (width+31)/32 * sizeof(uint32));
    sal_memset(pat, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));

    for(i = 0; i < len; i++) {
        if( data[i/32] & ( 1 << (i%32))) {
            pat[(i % width) / 32] |= 1 << ((i % width) % 32);
        }
        if(((i%width) == (width-1)) || ((i+1) == len)) {
            for(j = 0; j <= (i % width)/ 32; j++) {
                parity[j] ^= pat[j];
                pat[j] = 0;
            }
        }
    }
    return 0;
}

int 
dcmn_interrupt_handles_corrective_action_shadow(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg)
{
    int rc;
    uint32* data_entry=0;
    int entry_dw;
    int current_index;
    soc_mem_t mem;
    int copyno;
    uint32 mem_row_bit_width;
    uint32 ecc_field[SOC_MAX_MEM_WORDS];

    SOCDNX_INIT_FUNC_DEFS;

    mem = shadow_correct_info_p->mem;

    copyno = (shadow_correct_info_p->copyno == COPYNO_ALL) ? SOC_MEM_BLOCK_ANY(unit, mem) : shadow_correct_info_p->copyno;

    SOCDNX_NULL_CHECK(shadow_correct_info_p);
 
    if (dcmn_tbl_is_dynamic(unit, mem)) {
        SOC_EXIT;
    }

    if (!soc_mem_cache_get(unit, mem, copyno)   && mem != IRR_MCDBm) {
         SOCDNX_IF_ERR_EXIT(SOC_E_UNAVAIL);
    }
 
    entry_dw = soc_mem_entry_words(unit, mem);
    data_entry = sal_alloc((entry_dw * 4), "ARAD_INTERRUPT Shadow data entry allocation");

    if (NULL == data_entry) {
        SOCDNX_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    
    for (current_index = shadow_correct_info_p->min_index ; current_index <= shadow_correct_info_p->max_index ; current_index++) {

        rc = dcmn_get_ser_entry_from_cache(unit,   mem, copyno, 
                                           shadow_correct_info_p->array_index, current_index, data_entry);
       SOCDNX_IF_ERR_EXIT(rc);

        /* update cache ECC/PARITY fields */
        if (SOC_MEM_FIELD_VALID(unit, mem, PARITYf)) {
            mem_row_bit_width = soc_mem_entry_bits(unit, mem) - soc_mem_field_length(unit, mem, PARITYf);
            calc_par(unit, mem_row_bit_width, soc_mem_field_length(unit, mem, PARITYf), data_entry, ecc_field);
            soc_mem_field_width_fit_set(unit, mem, data_entry, PARITYf, ecc_field);
        } else if (SOC_MEM_FIELD_VALID(unit, mem, ECCf)) {
            mem_row_bit_width = soc_mem_entry_bits(unit, mem) - soc_mem_field_length(unit, mem, ECCf);
            calc_ecc(unit, mem_row_bit_width, data_entry, ecc_field);
            soc_mem_field_width_fit_set(unit, mem, data_entry, ECCf, ecc_field);
        }

        rc = soc_mem_array_write(unit, mem, shadow_correct_info_p->array_index, shadow_correct_info_p->copyno, current_index, data_entry);
        SOCDNX_IF_ERR_EXIT(rc);
    }

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    SOCDNX_FUNC_RETURN;
}

STATIC
int
dcmn_interrupt_corrected_data_entry(
    int unit,
    int block_instance,
    dcmn_interrupt_mem_err_info* ecc_1b_correct_info_p,
    int current_index,
    int cache_enable,
    int xor_flag,
    uint32 *data_entry)
{
    int rc;
    uint32 ecc_field[SOC_MAX_MEM_WORDS];
    uint32 mem_row_bit_width;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(ecc_1b_correct_info_p);
    rc = interrupt_memory_cached(unit, ecc_1b_correct_info_p->mem, block_instance, &cache_enable);

    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
    }
    if (cache_enable) {
        rc = dcmn_get_ser_entry_from_cache(unit,   ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->copyno,
                                           ecc_1b_correct_info_p->array_index, current_index, data_entry);
        /* update cache ECC field */
        if ( SOC_MEM_FIELD_VALID(unit, ecc_1b_correct_info_p->mem, ECCf)) {
            mem_row_bit_width = soc_mem_entry_bits(unit, ecc_1b_correct_info_p->mem) - soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf);
            calc_ecc(unit, mem_row_bit_width, data_entry, ecc_field);
            soc_mem_field_width_fit_set(unit, ecc_1b_correct_info_p->mem, data_entry, ECCf, ecc_field);
        }
    } else {
        rc = soc_mem_array_read(unit, ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->array_index, ecc_1b_correct_info_p->copyno, current_index, data_entry);
    }
    SOCDNX_IF_ERR_EXIT(rc);

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
        uint32 ecc_field_prev;

        mem_row_bit_width = soc_mem_entry_bits(unit, ecc_1b_correct_info_p->mem) - soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf);
        *ecc_field = soc_mem_field32_get(unit, ecc_1b_correct_info_p->mem, data_entry, ECCf);
        ecc_field_prev = *ecc_field;
        if (1<<soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf)< mem_row_bit_width) {
            LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Ecc len:%d for memory %s len%d \n"),
                       soc_mem_field_length(unit, ecc_1b_correct_info_p->mem, ECCf), SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem), current_index));
            SOCDNX_IF_ERR_EXIT(SOC_E_INTERNAL);
        }
        SOCDNX_IF_ERR_EXIT(ecc_correction(unit,ecc_1b_correct_info_p->mem, mem_row_bit_width,xor_flag, data_entry, ecc_field));
        if (ecc_field_prev != *ecc_field) {
            soc_mem_field32_set(unit,ecc_1b_correct_info_p->mem,data_entry, ECCf, *ecc_field);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_handles_corrective_action_for_xor(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* xor_correct_info,
    char* msg)
{
    int rc;
    int sp_index, sp_bank_size, sp_banks_num;
    int current_index, correct_index, free_index, oper_index;
    int offset, free_offset, cache_enable, entry_dw;
    uint32 *data_entry = NULL;
    dcmn_xor_mem_info *xor_mem_info;
    uint32 eci_global_value;
    uint32 xor_correct_value[ARAD_XOR_MEM_DATA_MAX_SIZE] = {0}, dump_xor_value[ARAD_XOR_MEM_DATA_MAX_SIZE] = {0};
    uint32 free_value[ARAD_XOR_MEM_DATA_MAX_SIZE] = {0};
    soc_reg_above_64_val_t value, orig_value;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(xor_correct_info);

    rc = interrupt_memory_cached(unit, xor_correct_info->mem, block_instance, &cache_enable);
    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, xor_correct_info->mem)));
        SOC_EXIT;
    }

    entry_dw = soc_mem_entry_words(unit, xor_correct_info->mem);
    data_entry = sal_alloc((entry_dw * 4), "Data entry allocation");
    if (NULL == data_entry) {
        SOCDNX_IF_ERR_EXIT(SOC_E_MEMORY);
    }

    sal_memset(data_entry, 0, (entry_dw * 4));
    xor_mem_info = dcmn_xor_mem_info_get(unit, xor_correct_info->mem);
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
        SOCDNX_IF_ERR_EXIT(dcmn_disable_block_ecc_check(unit,interrupt_id, xor_correct_info->mem,block_instance,value,orig_value));
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

            SOCDNX_IF_ERR_EXIT(dcmn_interrupt_corrected_data_entry(unit, block_instance, xor_correct_info,
                                                              correct_index, cache_enable, 1, data_entry));

            SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                                                       xor_correct_info->copyno, correct_index, data_entry));

            if (sp_index ==0) {
                sal_memcpy(dump_xor_value, data_entry, entry_dw * sizeof(uint32));
            }

            for (oper_index = 0; oper_index < entry_dw; oper_index++) {
                xor_correct_value[oper_index] ^= data_entry[oper_index];
            }

            /* Calculate free Data */
            if (xor_mem_info->direct_write_type == 0) {
                if (cache_enable) {
                    rc = dcmn_get_ser_entry_from_cache(unit, xor_correct_info->mem, xor_correct_info->copyno,
                                                       xor_correct_info->array_index, free_index, data_entry);
                } else {
                    rc = soc_mem_array_read(unit, xor_correct_info->mem, xor_correct_info->array_index,
                                                            xor_correct_info->copyno, free_index, data_entry);
                }

                if (sp_index ==0) {
                    sal_memcpy(free_value, data_entry, entry_dw * sizeof(uint32));
                }

                SOCDNX_IF_ERR_EXIT(rc);
                if (sp_index > 0) {
                    for (oper_index = 0; oper_index < entry_dw; oper_index++) {
                        xor_correct_value[oper_index] ^= data_entry[oper_index];
                    }
                }
            }
        }

        /* Update XOR bank value */
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_MEM_OPTIONSr(unit, &eci_global_value));

        soc_reg_field_set(unit, ECI_GLOBAL_MEM_OPTIONSr, &eci_global_value, WRITE_TO_XORf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_MEM_OPTIONSr(unit, eci_global_value));

        rc = soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index, xor_correct_info->copyno,
                                 xor_mem_info->direct_write_type ? offset : free_offset, xor_correct_value);

        soc_reg_field_set(unit, ECI_GLOBAL_MEM_OPTIONSr, &eci_global_value, WRITE_TO_XORf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_MEM_OPTIONSr(unit, eci_global_value));

        SOCDNX_IF_ERR_EXIT(rc);

        sal_memset(xor_correct_value, 0, ARAD_XOR_MEM_DATA_MAX_SIZE * sizeof(uint32));
        SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                     xor_correct_info->copyno, offset, xor_correct_value));
        SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                     xor_correct_info->copyno, offset, dump_xor_value));

        if (xor_mem_info->direct_write_type == 0) {
            SOCDNX_IF_ERR_EXIT(soc_mem_array_write(unit, xor_correct_info->mem, xor_correct_info->array_index,
                     xor_correct_info->copyno, free_offset, free_value));
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, xor_correct_info->mem, ECCf)) {
        SOCDNX_IF_ERR_EXIT(dcmn_disable_block_ecc_check(unit,interrupt_id,xor_correct_info->mem,block_instance,orig_value,NULL));
    }

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    SOCDNX_FUNC_RETURN;
}

int 
dcmn_interrupt_handles_corrective_action_for_ecc_1b(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* ecc_1b_correct_info_p,
    char* msg)
{
    int rc;
    uint32* data_entry=0;
    int entry_dw;
    int current_index;
    int cache_enable;
    soc_reg_above_64_val_t value, orig_value;
    int copyno;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(ecc_1b_correct_info_p);
    copyno = ecc_1b_correct_info_p->copyno - SOC_MEM_BLOCK_MIN(unit, ecc_1b_correct_info_p->mem);
    rc = interrupt_memory_cached(unit, ecc_1b_correct_info_p->mem, block_instance, &cache_enable);

    if(rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Couldnt decide cache state for %s \n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
     }
                  
   
    if (SOC_IS_DPP(unit) && (dcmn_tbl_is_dynamic(unit, ecc_1b_correct_info_p->mem) && !dcmn_mem_is_em(unit, ecc_1b_correct_info_p->mem))) {
        LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Interrupt will not be handled cause %s is dynamic\n"),
                    SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));
        SOC_EXIT;
    }

    entry_dw = soc_mem_entry_words(unit, ecc_1b_correct_info_p->mem);
    data_entry = sal_alloc((entry_dw * 4), "JER_INTERRUPT ecc 1 data entry allocation");

    if (NULL == data_entry) {
        SOCDNX_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    LOG_ERROR(BSL_LS_BCM_INTR, (BSL_META_U(unit,"Before correction of %s \n"),
                SOC_MEM_NAME(unit, ecc_1b_correct_info_p->mem)));

    /* disable ser for block because we need to read the errornous memory*/
    SOC_REG_ABOVE_64_CLEAR(value);
    SOCDNX_IF_ERR_EXIT(dcmn_disable_block_ecc_check(unit,interrupt_id, ecc_1b_correct_info_p->mem,copyno,value,orig_value));

    for (current_index = ecc_1b_correct_info_p->min_index ; current_index <= ecc_1b_correct_info_p->max_index ; current_index++) {
        rc = dcmn_interrupt_corrected_data_entry(unit, block_instance, ecc_1b_correct_info_p, current_index, cache_enable, 0, data_entry);
        SOCDNX_IF_ERR_EXIT(rc);

        rc = soc_mem_array_write(unit, ecc_1b_correct_info_p->mem, ecc_1b_correct_info_p->array_index, ecc_1b_correct_info_p->copyno, current_index, data_entry);
        SOCDNX_IF_ERR_EXIT(rc);

    }
    SOCDNX_IF_ERR_EXIT(dcmn_disable_block_ecc_check(unit,interrupt_id,ecc_1b_correct_info_p->mem,copyno,orig_value,NULL));

exit:
    if(data_entry != NULL) {
        sal_free(data_entry);
    }
    SOCDNX_FUNC_RETURN;
}

int dcmn_interrupt_handles_corrective_action_soft_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg)
{
    int rc = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rc = soc_device_reset(unit, SOC_DCMN_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET, SOC_DCMN_RESET_ACTION_INOUT_RESET);
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    SOCDNX_FUNC_RETURN;
}

int dcmn_interrupt_handles_corrective_action_hard_reset(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg)
{
    int rc = 0;
#if defined(BCM_DFE_SUPPORT) && defined(INCLUDE_INTR)
    bcm_pbmp_t rx_los_pbmp;
    int rx_los_enable;
#endif

    SOCDNX_INIT_FUNC_DEFS;

    LOG_ERROR(BSL_LS_BCM_INTR,
             (BSL_META_U(unit,
                         "Decision: Hard reset is required! interrupt id: %d, block instance: %d \n"),interrupt_id, block_instance));

#ifdef BCM_DPP_SUPPORT
    if (SOC_IS_DPP(unit)) {
        rc = soc_device_reset(unit, SOC_DPP_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET, SOC_DPP_RESET_ACTION_INOUT_RESET);
        SOCDNX_IF_ERR_EXIT(rc);
    }
#endif

#ifdef BCM_DNX_SUPPORT
    if (SOC_IS_DNX(unit)) {
        rc = soc_device_reset(unit, SOC_DCMN_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET, SOC_DCMN_RESET_ACTION_INOUT_RESET);
        SOCDNX_IF_ERR_EXIT(rc);
    }
#endif

#if defined(BCM_DFE_SUPPORT) && defined(INCLUDE_INTR)
    if (SOC_IS_DFE(unit)) {
        if (SOC_IS_FE1600(unit)) {
            /*RX LOS App disable - required in order to run soft reset without traffic loss*/
            rc = rx_los_enable_get(unit, &rx_los_pbmp, &rx_los_enable);
            SOCDNX_IF_ERR_EXIT(rc);

            if (rx_los_enable) {
                rc = rx_los_unit_detach(unit);
                SOCDNX_IF_ERR_EXIT(rc);
            }
        }

        rc = soc_init(unit);
        SOCDNX_IF_ERR_EXIT(rc);

        if (SOC_IS_FE1600(unit)) {
            /*RX LOS App enable - without resetting RX LOS - required in order to run soft reset without traffic loss*/
            if (rx_los_enable) {
                rc = rx_los_unit_attach(unit, rx_los_pbmp, 1);
                SOCDNX_IF_ERR_EXIT(rc);
            }
        }
    }
#endif

exit:
    SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_handles_corrective_action_do_nothing (
  int unit,
  int block_instance,
  uint32 interrupt_id,
  char *msg)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*empty function*/

    SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_print_info(
    int unit,
    int block_instance,
    uint32 en_interrupt,
    int recurring_action,
    dcmn_int_corr_act_type corr_act,
    char *special_msg)
{
    int rc;
    uint32 flags;
    soc_interrupt_db_t* interrupt;

    char cur_special_msg[DCMN_INTERRUPT_SPECIAL_MSG_SIZE];
    char cur_corr_act_msg[DCMN_INTERRUPT_COR_ACT_MSG_SIZE];
    char print_msg[DCMN_INTERRUPT_PRINT_MSG_SIZE];

    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOC_MSG("invalid unit")));
    }

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    interrupt = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[en_interrupt]);

    if (special_msg == NULL) {
        sal_snprintf(cur_special_msg, sizeof(cur_special_msg), "None");
    } else {
        sal_strncpy(cur_special_msg, special_msg, sizeof(cur_special_msg) - 1);
    }

    rc = soc_interrupt_flags_get(unit, en_interrupt, &flags);
    SOCDNX_IF_ERR_EXIT(rc);

    /* Corrective action will be performed only if BCM_AND_USR_CB flag set or corrective action override flag not overriden */
    if(((flags & SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB) == 0) && SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE)) {
        corr_act = DCMN_INT_CORR_ACT_NONE;
    }

    switch(corr_act) {
        case DCMN_INT_CORR_ACT_HARD_RESET:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Hard Reset is Required");
            break;
        case DCMN_INT_CORR_ACT_NONE:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "None");
            break;
        case DCMN_INT_CORR_ACT_SOFT_RESET:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Soft Reset");
            break;
        case DCMN_INT_CORR_ACT_PRINT:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Print");
            break;
        case DCMN_INT_CORR_ACT_HANDLE_OAMP_EVENT_FIFO:
            sal_sprintf(cur_corr_act_msg, "Handle OAMP Event Fifo");
            break;
        case DCMN_INT_CORR_ACT_HANDLE_OAMP_STAT_EVENT_FIFO:
            sal_sprintf(cur_corr_act_msg, "Handle OAMP Statistics Event Fifo");
            break;
        case DCMN_INT_CORR_ACT_SHADOW:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Shadow");
            break;
        case DCMN_INT_CORR_ACT_SHUTDOWN_FBR_LINKS:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown link");
            break;
        case DCMN_INT_CORR_ACT_CLEAR_CHECK:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Clear Check");
            break;
        case DCMN_INT_CORR_ACT_CONFIG_DRAM:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Config DRAM");
            break;
        case DCMN_INT_CORR_ACT_ECC_1B_FIX:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "ECC 1b Correct");
            break;
        case DCMN_INT_CORR_ACT_EM_ECC_1B_FIX:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "EM ECC 1b Correct");
            break;
        case DCMN_INT_CORR_ACT_EPNI_EM_SOFT_RECOVERY:
        case DCMN_INT_CORR_ACT_IHB_EM_SOFT_RECOVERY:
        case DCMN_INT_CORR_ACT_IHP_EM_SOFT_RECOVERY:
        case DCMN_INT_CORR_ACT_OAMP_EM_SOFT_RECOVERY:
        case DCMN_INT_CORR_ACT_EM_SOFT_RECOVERY:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "EM Soft Recovery");
            break;
        case DCMN_INT_CORR_ACT_FORCE:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Force");
            break;
        case DCMN_INT_CORR_ACT_HANDLE_CRC_DEL_BUF_FIFO:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Handle CRC Delete Buffer FIFO");
            break;
        case DCMN_INT_CORR_ACT_HANDLE_MACT_EVENT_FIFO:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Handle MACT Event FIFO");
            break;
        case DCMN_INT_CORR_ACT_HARD_RESET_WITHOUT_FABRIC:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Hard Reset without Fabric");
            break;
        case DCMN_INT_CORR_ACT_INGRESS_HARD_RESET:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Ingress Hard Reset");
            break;
        case DCMN_INT_CORR_ACT_IPS_QDESC:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "IPS QDESC Clear Unused");
            break;
        case DCMN_INT_CORR_ACT_REPROGRAM_RESOURCE:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Reprogram Resource");
            break;
        case DCMN_INT_CORR_ACT_RTP_LINK_MASK_CHANGE:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "RTP Link Mask Change");
            break;
        case DCMN_INT_CORR_ACT_RX_LOS_HANDLE:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "RX LOS Handle");
            break;
        case DCMN_INT_CORR_ACT_SHADOW_AND_SOFT_RESET:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Shadow and Soft Reset");
            break;
        case DCMN_INT_CORR_ACT_SHUTDOWN_UNREACH_DESTINATION:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown Unreachable Destination");
            break;
        case DCMN_INT_CORR_ACT_TCAM_SHADOW_FROM_SW_DB:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "TCAM Shadow from SW DB");
            break;
        case DCMN_INT_CORR_ACT_RTP_SLSCT:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "RTP SLSCT");
            break;
        case DCMN_INT_CORR_ACT_SHUTDOWN_LINKS:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Shutdown links");
            break;
        case DCMN_INT_CORR_ACT_MC_RTP_CORRECT:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "MC RTP Correct");
            break;
        case DCMN_INT_CORR_ACT_UC_RTP_CORRECT:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "UC RTP Correct");
            break;
        case DCMN_INT_CORR_ACT_ALL_REACHABLE_FIX:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "All Rechable fix");
            break;
        case DCMN_INT_CORR_ACT_IPS_QSZ_CORRECT:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "IPS QSZ Correct");
            break;
        case DCMN_INT_CORR_ACT_XOR_FIX:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "XOR Fix");
            break;
        case DCMN_INT_CORR_ACT_DYNAMIC_CALIBRATION:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Dynamic calibration");
            break;
        default:
            sal_snprintf(cur_corr_act_msg, DCMN_INTERRUPT_COR_ACT_MSG_SIZE, "Unknown");
    }

    /* prepare string for print */
#if defined(SOC_NO_NAMES)
    sal_snprintf(print_msg, DCMN_INTERRUPT_PRINT_MSG_SIZE, "id=%d, index=%d, block=%d, unit=%d, recurring_action=%d | %s | %s\n ",
                en_interrupt, interrupt->reg_index, block_instance, unit, recurring_action, cur_special_msg, cur_corr_act_msg);
#else
    sal_snprintf(print_msg, DCMN_INTERRUPT_PRINT_MSG_SIZE,"name=%s, id=%d, index=%d, block=%d, unit=%d, recurring_action=%d | %s | %s\n ",
                interrupt->name, en_interrupt, interrupt->reg_index, block_instance, unit, recurring_action, cur_special_msg, cur_corr_act_msg);
#endif

    /* Print per interrupt mechanism */
    if(SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE)) {
        LOG_ERROR(BSL_LS_SOC_INTR, (BSL_META_U(unit,"%s"), print_msg));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_handles_corrective_action_print(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char* msg_print,
    char* msg)
{
    soc_interrupt_db_t* interrupt;
    char print_msg[DCMN_INTERRUPT_PRINT_MSG_SIZE];

    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_SOC_MSG("invalid unit")));
    }

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    interrupt = &(SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id]);

    /* prepare string for print */
#if defined(SOC_NO_NAMES)
    sal_snprintf(print_msg, DCMN_INTERRUPT_PRINT_MSG_SIZE, "id=%d, index=%d, block=%d, unit=%d | %s\n ",
                interrupt_id, interrupt->reg_index, block_instance, unit, msg_print);
#else
    sal_snprintf(print_msg, DCMN_INTERRUPT_PRINT_MSG_SIZE,"name=%s, id=%d, index=%d, block=%d, unit=%d | %s\n ",
                interrupt->name, interrupt_id, interrupt->reg_index, block_instance, unit, msg_print);
#endif

    LOG_ERROR(BSL_LS_BCM_INTR,
              (BSL_META_U(unit,
                          "%s"),
               print_msg));

exit:
    SOCDNX_FUNC_RETURN;
}

#ifdef BCM_DPP_SUPPORT
int
dcmn_interrupt_handles_corrective_action_for_ips_qsz(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg)
{
    uint32 ips_qsz_value = 0, dynamic_val;
    int current_index;
    int rc;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(shadow_correct_info_p);

    SOCDNX_IF_ERR_EXIT(READ_IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, &dynamic_val));

    for (current_index = shadow_correct_info_p->min_index ; current_index <= shadow_correct_info_p->max_index ; current_index++) {
        SOCDNX_IF_ERR_EXIT(WRITE_IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
        rc = soc_mem_array_write(unit, shadow_correct_info_p->mem, shadow_correct_info_p->array_index, shadow_correct_info_p->copyno, current_index, &ips_qsz_value);
        SOCDNX_IF_ERR_EXIT(WRITE_IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, dynamic_val));
        SOCDNX_IF_ERR_EXIT(rc);
    }

exit:
    SOCDNX_FUNC_RETURN;

}

int
dcmn_interrupt_data_collection_for_tcamprotectionerror(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    ARAD_TCAM_LOCATION *location,
    dcmn_int_corr_act_type* corrective_action)
{

    int rc;
    uint32 fld_val_bank, fld_val_entry, reg_value;
    ARAD_TCAM_BANK_ENTRY_SIZE entry_size;
    soc_reg_t tcam_protection_reg;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(location);
    SOCDNX_NULL_CHECK(corrective_action);

    tcam_protection_reg = SOC_IS_JERICHO(unit) ? PPDB_A_TCAM_PROTECTION_ERRORr : IHB_TCAM_PROTECTION_ERRORr;
    /*
     * Get the address (bank and entry) in which an error occured
     */
    rc = soc_reg32_get(unit, tcam_protection_reg, block_instance, 0, &reg_value);
    SOCDNX_IF_ERR_EXIT(rc);

    fld_val_bank = soc_reg_field_get(unit, tcam_protection_reg, reg_value, TCAM_PROTECTION_ERROR_TCAM_BANKf);
    fld_val_entry = soc_reg_field_get(unit, tcam_protection_reg, reg_value, TCAM_PROTECTION_ERROR_TCAM_ENTRY_ADDf);

    /* first get bank entry size to adjust error address */
    rc = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, fld_val_bank, &entry_size);
    SOCDNX_IF_ERR_EXIT(rc);

    /* if entry size is 320-bit then take the even bank, even if the error occured in odd bank */
    location->bank_id = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? (fld_val_bank - (fld_val_bank % 2)) : fld_val_bank;

    /* The tcam-entry-parity table contains 2 entries for each line
     * in the tcam bank, even entries for key and odd entries for mask.
     * if entry size in 80b then location should remain the same
     * otherwize, tha error address read from HW should be divided by 2
     */
    location->entry = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? fld_val_entry : (fld_val_entry / 2);

    *corrective_action = DCMN_INT_CORR_ACT_TCAM_SHADOW_FROM_SW_DB;

exit:
    SOCDNX_FUNC_RETURN;
}

int
dcmn_interrupt_handles_corrective_action_tcam_shadow_from_sw_db(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    ARAD_TCAM_LOCATION* location,
    char* msg)
{
    uint32 rc,
        entry_ndx,
        nof_entries,
        global_location_id;
    uint8
        bank_valid = FALSE,
        found = FALSE;

    SOC_SAND_OCC_BM_PTR
        occ_bm;
    ARAD_TCAM_GLOBAL_LOCATION
        global_location;
    ARAD_TCAM_BANK_OWNER
        bank_owner;
    ARAD_TCAM_BANK_ENTRY_SIZE
        entry_size;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(location);

    rc = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, location->bank_id, &bank_valid);
    SOCDNX_IF_ERR_EXIT(rc);

    if (bank_valid)
    {
        uint8 in_use ;
        /* Validate this entry is in Occupation Bitmap */
            rc = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(unit, location->bank_id, 0, &occ_bm);
        SOCDNX_IF_ERR_EXIT(rc);
        soc_sand_occ_is_bitmap_active(unit,occ_bm,&in_use) ;
        if (!in_use) {
            /* error if occ bitmap for valid bank doesn't exist */
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOC_MSG("No occ bitmap for valid bank")));
        }
    }
    else {
        /* bank is empty, shouldn't be read */
        goto exit;
    }

    /* get bank entry size (80 bits or 160/320) */
    rc = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, location->bank_id, &entry_size);
    SOCDNX_IF_ERR_EXIT(rc);

    /* data is restored differently according to bank owner */
    rc = arad_tcam_bank_owner_get_unsafe(unit,location->bank_id, &bank_owner);
    SOCDNX_SAND_IF_ERR_EXIT(rc);

    /* bank entry size will determine if the correction should be done once or twice */
    nof_entries = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? 2 : 1;

    /* if entry is 80bits then the following is done for both entries in the same line */
    for (entry_ndx = 0; entry_ndx < nof_entries; entry_ndx++)
    {
        if (entry_ndx == 1)
        {
            /* if there is a second run of this loop, then next entry should be the neighbor */
            location->entry = ((location->entry % 2) == 0) ? (location->entry + 1) : (location->entry - 1);
        }

        /* see if specific entry is in occupation bitmap */
        rc = soc_sand_occ_bm_is_occupied(unit, occ_bm, location->entry, &found);
        SOCDNX_SAND_IF_ERR_EXIT(rc);

        if (found)
        {
            /* convert the memory address to location in TCAM (entry) */
            rc = arad_tcam_global_location_encode(unit, location, &global_location_id);
            SOCDNX_SAND_IF_ERR_EXIT(rc);

            /* get PPD entry ID from location in TCAM */
            rc = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.get(unit, global_location_id, &global_location);
            SOCDNX_IF_ERR_EXIT(rc);
        }

        /* handle according to bank owner */
        switch (bank_owner)
        {
        case ARAD_TCAM_BANK_OWNER_PMF_0:
        case ARAD_TCAM_BANK_OWNER_PMF_1:
        case ARAD_TCAM_BANK_OWNER_EGRESS_ACL:

            /* handle errors for PMF banks */
            rc = arad_pp_fp_rewrite_entry(unit, found, &global_location, location);
            SOCDNX_SAND_IF_ERR_EXIT(rc);

            break;

        case ARAD_TCAM_BANK_OWNER_FLP_TCAM:

            /* handle errors for FLP banks */
            rc = arad_pp_frwrd_ip_tcam_rewrite_entry(unit, found, &global_location, location);
            SOCDNX_SAND_IF_ERR_EXIT(rc);

            break;

        case ARAD_TCAM_BANK_OWNER_FLP_TRAPS:
            /* Currently not handled, SW db is necessary */
            break;

        case ARAD_TCAM_BANK_OWNER_VT:
        case ARAD_TCAM_BANK_OWNER_TT:

            /* handle errors for VT/TT banks */
            rc = arad_pp_isem_access_tcam_rewrite_entry(unit, found, &global_location, location);
            SOCDNX_SAND_IF_ERR_EXIT(rc);

            break;

        default:
            break;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}
#endif

#ifdef BCM_DFE_SUPPORT
int
dcmn_interrupt_handles_corrective_action_for_rtp_slsct(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg)
{
    uint32 slsct_val = 0, score_slsct;
    soc_field_t scrub_en;
    uint64 reg_val64;
    int current_index;
    int rc;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(shadow_correct_info_p);

    if(SOC_DFE_CONFIG(unit).fabric_load_balancing_mode == soc_dfe_load_balancing_mode_normal)
    {
        /* Table is unused */
        /* Write 0 to the entry */
        for (current_index = shadow_correct_info_p->min_index ; current_index <= shadow_correct_info_p->max_index ; current_index++) {
            rc = soc_mem_array_write(unit, shadow_correct_info_p->mem, shadow_correct_info_p->array_index, shadow_correct_info_p->copyno, current_index, &slsct_val);
            SOCDNX_IF_ERR_EXIT(rc);
        }
    }
    else if(SOC_DFE_CONFIG(unit).fabric_load_balancing_mode == soc_dfe_load_balancing_mode_balanced_input)
    {
        /* Table is used */
        /* reset the scrubber */
        SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
        scrub_en = soc_reg64_field32_get(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, reg_val64, SCT_SCRUB_ENABLEf);
        soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

        SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, &reg_val64));
        soc_reg64_field32_set(unit, RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr, &reg_val64, SCT_SCRUB_ENABLEf, scrub_en);
        SOCDNX_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_MESSAGE_GENERATOR_CONFIGURATIONr_REG64(unit, reg_val64));

    }
    else if(SOC_DFE_CONFIG(unit).fabric_load_balancing_mode == soc_dfe_load_balancing_mode_destination_unreachable)
    {
        /* Table is used */
        /* write max value to the entry */
        score_slsct = SOC_DFE_DRV_MULTIPLIER_MAX_LINK_SCORE;
        soc_mem_field_set(unit, RTP_SLSCTm, &slsct_val, LINK_NUMf, &score_slsct);

        for (current_index = shadow_correct_info_p->min_index ; current_index <= shadow_correct_info_p->max_index ; current_index++) {
            rc = soc_mem_array_write(unit, shadow_correct_info_p->mem, shadow_correct_info_p->array_index, shadow_correct_info_p->copyno, current_index, &slsct_val);
            SOCDNX_IF_ERR_EXIT(rc);
        }
    }
    else
    {
        /* Unexpected load balancing mode */
        SOCDNX_IF_ERR_EXIT(SOC_E_UNAVAIL);
    }

exit:
    SOCDNX_FUNC_RETURN;
}
#endif


#ifdef BCM_DPP_SUPPORT
int
dcmn_interrupt_handles_corrective_action_handle_oamp_event_fifo(int unit,
                                                                int block_instance,
                                                                uint32 en_arad_interrupt,
                                                                char* msg)
{

    sal_dpc(soc_ppd_oam_dma_event_handler, INT_TO_PTR(unit), INT_TO_PTR(SOC_PPC_OAM_DMA_EVENT_TYPE_EVENT),0,0,0 );
    return SOC_E_NONE;
}

int
dcmn_interrupt_handles_corrective_action_handle_oamp_stat_event_fifo(int unit,
                                                                     int block_instance,
                                                                     uint32 en_arad_interrupt,
                                                                     char* msg)
{

    sal_dpc(soc_ppd_oam_dma_event_handler, INT_TO_PTR(unit), INT_TO_PTR(SOC_PPC_OAM_DMA_EVENT_TYPE_STAT_EVENT),0,0,0 );
    return SOC_E_NONE;
}

int
dcmn_interrupt_handles_corrective_action_shutdown_fbr_link(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char* msg)
{
   uint32 port = block_instance*4 + SOC_CONTROL(unit)->interrupts_info->interrupt_db_info[interrupt_id].bit_in_field;
    uint32 rc;
    SOCDNX_INIT_FUNC_DEFS;


    rc = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_port_enable_set, (unit, port, 0,0));
    SOCDNX_IF_ERR_EXIT(rc);

exit:


    SOCDNX_FUNC_RETURN;

}

int
dcmn_interrupt_handles_corrective_action_em_ser(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* shadow_correct_info_p,
    char* msg)
{
    int rc;
    soc_mem_t mem;
    int copyno;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(shadow_correct_info_p);

    mem = shadow_correct_info_p->mem;
    copyno = (shadow_correct_info_p->copyno == COPYNO_ALL) ? SOC_MEM_BLOCK_ANY(unit, mem) : shadow_correct_info_p->copyno;

    rc = dcmn_pp_em_ser(unit, mem, shadow_correct_info_p->array_index, copyno, shadow_correct_info_p->min_index);
    SOCDNX_IF_ERR_EXIT(rc);

exit:
    SOCDNX_FUNC_RETURN;
}
#endif

/* Corrective Action main function */
int
dcmn_interrupt_handles_corrective_action(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    char *msg,
    dcmn_int_corr_act_type corr_act,
    void *param1,
    void *param2)
{
    int rc;
    uint32 flags;

    SOCDNX_INIT_FUNC_DEFS;

    if(!SOC_INTR_IS_SUPPORTED(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    rc = soc_interrupt_flags_get(unit, interrupt_id, &flags);
    SOCDNX_IF_ERR_EXIT(rc);

    /* Corrective action will be performed only if BCM_AND_USR_CB flag set or corrective action override flag not overriden */
    if(((flags & SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB) == 0) && SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE)) {
        corr_act = DCMN_INT_CORR_ACT_NONE;
    }

    switch(corr_act) {
    case DCMN_INT_CORR_ACT_NONE:
        rc = dcmn_interrupt_handles_corrective_action_do_nothing(unit, block_instance, interrupt_id, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;

    case DCMN_INT_CORR_ACT_SOFT_RESET:
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_SOFT_RESET_BLOCKS_FABRIC);
        } else {
            rc = dcmn_interrupt_handles_corrective_action_soft_reset(unit, block_instance, interrupt_id, msg);
            SOCDNX_IF_ERR_EXIT(rc);
        }
        break;

    case DCMN_INT_CORR_ACT_HARD_RESET:
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_HARD_RESET);
        } else {
            rc = dcmn_interrupt_handles_corrective_action_hard_reset(unit, block_instance, interrupt_id, msg);
            SOCDNX_IF_ERR_EXIT(rc);
        }
        break;

    case DCMN_INT_CORR_ACT_PRINT:
        rc = dcmn_interrupt_handles_corrective_action_print(unit, block_instance, interrupt_id,(char*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_SHADOW:
        rc = dcmn_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;

    case DCMN_INT_CORR_ACT_SHADOW_AND_SOFT_RESET:
        rc = dcmn_interrupt_handles_corrective_action_shadow(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        if (soc_property_suffix_num_get(unit,-1, spn_CUSTOM_FEATURE, "ser_reset_cb_en", 0)) {
            soc_event_generate(unit, SOC_SWITCH_EVENT_DEVICE_INTERRUPT, interrupt_id, block_instance, ASIC_SOFT_RESET_BLOCKS_FABRIC);
        } else {
            rc = dcmn_interrupt_handles_corrective_action_soft_reset(unit, block_instance, interrupt_id, msg);
            SOCDNX_IF_ERR_EXIT(rc);
        }
        break;

    case DCMN_INT_CORR_ACT_ECC_1B_FIX:
        rc = dcmn_interrupt_handles_corrective_action_for_ecc_1b(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
#ifdef BCM_DPP_SUPPORT
    case DCMN_INT_CORR_ACT_HANDLE_OAMP_EVENT_FIFO:
        rc = dcmn_interrupt_handles_corrective_action_handle_oamp_event_fifo(unit, block_instance, interrupt_id, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_HANDLE_OAMP_STAT_EVENT_FIFO:
        rc = dcmn_interrupt_handles_corrective_action_handle_oamp_stat_event_fifo(unit, block_instance, interrupt_id, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_SHUTDOWN_FBR_LINKS:
        rc = dcmn_interrupt_handles_corrective_action_shutdown_fbr_link(unit, block_instance, interrupt_id, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_IPS_QSZ_CORRECT:
        rc = dcmn_interrupt_handles_corrective_action_for_ips_qsz(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_XOR_FIX:
        rc = dcmn_interrupt_handles_corrective_action_for_xor(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_TCAM_SHADOW_FROM_SW_DB:
        rc = dcmn_interrupt_handles_corrective_action_tcam_shadow_from_sw_db(unit, block_instance, interrupt_id,(ARAD_TCAM_LOCATION*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_EM_SOFT_RECOVERY:
        rc = dcmn_interrupt_handles_corrective_action_em_ser(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
    case DCMN_INT_CORR_ACT_EM_ECC_1B_FIX:
        rc = dcmn_interrupt_handles_corrective_action_for_em_ecc_1b(unit, block_instance, interrupt_id,(dcmn_interrupt_mem_err_info*)param1, msg);
        SOCDNX_IF_ERR_EXIT(rc);
        break;
#endif
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOC_MSG("UnKnown corrective action")));
  }

exit:
  SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

