/*
 * $Id: qax_sram.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*
 * Includes
 */
#include <shared/bsl.h>

/* SAL includes */
#include <sal/appl/sal.h>

/* SOC includes */
#include <soc/error.h>
#include <soc/dcmn/error.h>
#include <soc/dcmn/dcmn_mem.h>

/* SOC DPP includes */
#include <soc/dpp/QAX/qax_sram.h>
#include <soc/dpp/QAX/qax_link_bonding.h>

int soc_qax_sram_conf_set(int unit)
{

    uint32 mem_entry[SOC_MAX_MEM_WORDS] = {0};
    uint32 reg32_val;
    uint64 reg64_val;

    SOCDNX_INIT_FUNC_DEFS;

    /* SPB_STATIC_CONFIGURATIONr */
    COMPILER_64_ZERO(reg64_val);
    SOCDNX_IF_ERR_EXIT( READ_SPB_STATIC_CONFIGURATIONr(unit, &reg64_val));
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, DDC_2_NUM_OF_BUFF_THf, 4);
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, DEL_NUM_OF_BUFF_AF_THf, 0x7d0);
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, DEL_NUM_OF_PKT_AF_THf, 0x64);
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, PTC_PDQ_RDY_TH_WORDf, 8);
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, STOP_IRE_WHEN_FBC_EMPTYf, 1);
    soc_reg64_field32_set(unit, SPB_STATIC_CONFIGURATIONr, &reg64_val, STOP_IRE_THf, 5);
    SOCDNX_IF_ERR_EXIT( WRITE_SPB_STATIC_CONFIGURATIONr(unit, reg64_val));

    /* SPB_CONTEXT_MRUm */
    sal_memset(mem_entry, 0, SOC_MAX_MEM_WORDS);
    soc_mem_field32_set(unit, SPB_CONTEXT_MRUm, mem_entry, MAX_SIZEf, 0x2fff);
    soc_mem_field32_set(unit, SPB_CONTEXT_MRUm, mem_entry, MAX_ORG_SIZEf, 0x2fff);
    soc_mem_field32_set(unit, SPB_CONTEXT_MRUm, mem_entry, MIN_ORG_SIZEf, 0x20);
    soc_mem_field32_set(unit, SPB_CONTEXT_MRUm, mem_entry, MIN_SIZEf, 0x20);
    SOCDNX_IF_ERR_EXIT( dcmn_fill_table_with_entry( unit, SPB_CONTEXT_MRUm, MEM_BLOCK_ALL, mem_entry));

    /* SPB_DYNAMIC_CONFIGURATIONr */
    SOCDNX_IF_ERR_EXIT( READ_SPB_DYNAMIC_CONFIGURATIONr(unit,&reg32_val));
    soc_reg_field_set(unit, SPB_DYNAMIC_CONFIGURATIONr, &reg32_val, SOC_IS_QUX(unit)?MAX_BUFFERS_THRESHOLDf:FIELD_1_6f, 0x2e);
    soc_reg_field_set(unit, SPB_DYNAMIC_CONFIGURATIONr, &reg32_val, FBC_INITf, 1);
    SOCDNX_IF_ERR_EXIT( WRITE_SPB_DYNAMIC_CONFIGURATIONr(unit,reg32_val));
    SOCDNX_IF_ERR_EXIT( soc_reg_field32_modify(unit, SPB_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, FBC_INITf, 0));

    /* Init SRAM for LBG */
#ifdef BCM_QAX_SUPPORT
    if (SOC_IS_QAX(unit) && soc_property_get(unit, spn_LINK_BONDING_ENABLE, 0)) {
        SOCDNX_IF_ERR_EXIT(qax_lb_ing_init(unit));
    }
#endif

exit:
    SOCDNX_FUNC_RETURN;
}
