/*
 * $Id: jer2_qax_tbls.c Exp $
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
#include <shared/bitop.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm_int/common/debug.h>

/* SAL includes */
#include <sal/appl/sal.h>

/* SOC includes */
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/iproc.h>
#include <soc/mem.h>
#include <soc/mcm/memregs.h>

/* SOC DNX includes */
#include <soc/dnx/legacy/drv.h>
#include <soc/dnxc/legacy/dnxc_mem.h>
#include <soc/dnx/legacy/QAX/qax_multicast_imp.h>

/* SOC DNX JER2_JER includes */
#include <soc/dnx/legacy/JER/jer_tbls.h>
#include <soc/dnx/legacy/QAX/qax_tbls.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>
#include <soc/dnx/legacy/QAX/qax_fabric.h>

/* SOC DNX Arad includes */ 
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>


/* 
 * Defines
 */


/* Static memoried which should be skipped when zeroing all static memories */
static soc_mem_t jer2_qax_tbls_excluded_mem_list[] = {
    TAR_MCDBm,
    ILKN_PMH_PORT_0_CPU_ACCESSm, /* These are not real memories */
    ILKN_PMH_PORT_1_CPU_ACCESSm,
    ILKN_PML_PORT_0_CPU_ACCESSm,
    ILKN_PML_PORT_1_CPU_ACCESSm,
    SCH_SCHEDULER_INITm, /* not a real memory, handled separately */
    OAMP_FLEX_VER_MASK_TEMPm, 
    OAMP_FLOW_STAT_ACCUM_ENTRY_4m,
    SER_ACC_TYPE_MAPm,
    SER_MEMORYm,
    SER_RESULT_DATA_1m,
    SER_RESULT_DATA_0m,
    SER_RESULT_EXPECTED_1m,
    SER_RESULT_EXPECTED_0m,
    SER_RESULT_1m,
    SER_RESULT_0m,

    MMU_DRAM_ADDRESS_SPACEm, /* not a real memory */
    DQM_BDB_LINK_LISTm, /* link list is initialized by hw */
    /* Has to be last memory in array */
    INVALIDm
};

/* registers to enable JER2_QAX dynamic memory writes */
soc_reg_t jer2_qax_dynamic_mem_enable_regs[] = {
    CFC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CGM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    DDP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    DQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FCR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FDT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IEP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PMH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PML_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPSEC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPSEC_SPU_WRAPPER_TOP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ITE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MMU_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    NBIH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    NBIL_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PEM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PTS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    RTP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SPB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    TAR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    TXQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    INVALIDr
};

soc_reg_t qux_dynamic_mem_enable_regs[] = {
    CFC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CGM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    DDP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    DQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IEP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPSEC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPSEC_SPU_WRAPPER_TOP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ITE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MMU_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    NIF_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PEM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PTS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SPB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    TAR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    TXQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    INVALIDr
};

/* Registers to disable JER2_QAX dynamic memory writes (for blocks we don't wish to allow writes to dynamic memories). */
soc_reg_t *jer2_qax_dynamic_mem_disable_regs = jer2_qax_dynamic_mem_enable_regs;
soc_reg_t *qux_dynamic_mem_disable_regs = qux_dynamic_mem_enable_regs;

/*
 * Functions
 */


/*
 * Function:
 *      soc_jer2_qax_tbls_init
 * Purpose:
 *      initialize all tables relevant for Jericho.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_qax_tbls_init(int unit)
{
    /* define bitmap for memory exclusions */
    uint32 sch_init_val = 1;
    DNXC_INIT_FUNC_DEFS;

    /* initialize memory zeroing exclusion to the given array. */
    soc_jer2_jer_tbls_zero_init(unit, jer2_qax_tbls_excluded_mem_list);
        
    DNXC_IF_ERR_EXIT(jer2_arad_tbl_access_init_unsafe(unit)); /* init jer2_arad_fill_table_with_variable_values_by_caching() */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_control_dynamic_mem_writes(unit, SOC_IS_QUX(unit) ? qux_dynamic_mem_enable_regs : jer2_qax_dynamic_mem_enable_regs, 1)); /* enable dynamic memory writes */
    
    /* Init tables having none zero values */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_sch_tbls_init(unit)); /* SCH block */
    DNXC_IF_ERR_EXIT(soc_jer2_qax_tar_tbls_init(unit)); /* TAR block */

    /* IRE block */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_ihp_tbls_init(unit)); /* IHP block */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_cgm_tbls_init(unit)); /* CGM block */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_ips_tbls_init(unit)); /* IPS block */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_pts_tbls_init(unit)); /*PTS block*/

    if (!SOC_IS_QUX(unit)) {
    DNXC_IF_ERR_EXIT(soc_jer2_qax_fcr_tbls_init(unit)); /* FCR block */
    }

    DNXC_IF_ERR_EXIT(soc_jer2_qax_txq_tbls_init(unit)); /* TXQ block */
    /* ITE block */

    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_fdt_tbls_init(unit)); /* FDT block */

    DNXC_IF_ERR_EXIT(soc_jer2_qax_egq_tbls_init(unit)); /* EGQ block */


    /* MRPS+MRPS_EM block (IMP+IEP) */
    DNXC_IF_ERR_EXIT(soc_jer2_qax_imp_tbls_init(unit));
	DNXC_IF_ERR_EXIT(soc_jer2_qax_iep_tbls_init(unit));

    /* PPDB block */

    DNXC_IF_ERR_EXIT(jer2_qax_mult_rplct_tbl_entry_unoccupied_set_all(unit));
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_multicast_init2(unit));

    /* initialize all tables to zero, except for exception array, and marked not to be zeroed.  */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_tbls_zero(unit));

    DNXC_IF_ERR_EXIT(soc_jer2_jer_control_dynamic_mem_writes(unit, SOC_IS_QUX(unit) ? qux_dynamic_mem_disable_regs : jer2_qax_dynamic_mem_disable_regs, 0)); /* disable dynamic memory writes */

    /* trigger SCH_SCHEDULER_INIT to reset dynamic SCH tables, needs to be after the dynamic table zeroing */
    DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_INITm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), 0, &sch_init_val));

exit:
    DNXC_FUNC_RETURN;
}


int soc_jer2_qax_ihp_tbls_init(int unit)
{    
    uint32 entry[20] = {0};

    DNXC_INIT_FUNC_DEFS;

    /* default FID = VSI */
    soc_mem_field32_set(unit, IHP_VSI_LOW_CFG_2m, entry, FID_CLASSf, 7);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IHP_VSI_LOW_CFG_2m, entry));    
	
exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_ips_tbls_init(int unit)
{
    uint32 entry[20] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    /*
     * Mark whole table as 'invalid'. See jer2_arad_interrupt_handles_corrective_action_ips_qdesc().
     */
    entry[1] = JER2_ARAD_IPQ_INVALID_FLOW_QUARTET;
    DNXC_SAND_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IPS_FLWIDm, entry));
    entry[1] = 0;

    /* set to max (127) CAM full thresholds */
    soc_mem_field32_set(unit, IPS_CFMEMm, entry, DRAM_CAM_FULL_THRESHOLDf, 127);
    soc_mem_field32_set(unit, IPS_CFMEMm, entry, FABRIC_CAM_FULL_THRESHOLDf, 127);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IPS_CFMEMm, entry));

    /* All devices are set to FMS enabled, EFMS is in bypass mode by default.
       Zero IPS_FMSBYPm which has 1 bit for each device, 16 devices in row */

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_pts_tbls_init(int unit)
{
    uint32 txq_per_wfq_default[SOC_MAX_MEM_WORDS] = {0};
    uint32 pts_per_wfq_default[SOC_MAX_MEM_WORDS] = {0};
    uint32 pts_per_wfq_mesh_default[SOC_MAX_MEM_WORDS] = {0};
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    soc_mem_field32_set(unit, TXQ_PER_WFQ_CFGm, txq_per_wfq_default, WFQ_WEIGHT_0f, 0x1);
    soc_mem_field32_set(unit, TXQ_PER_WFQ_CFGm, txq_per_wfq_default, WFQ_WEIGHT_1f, 0x1);
    soc_mem_field32_set(unit, TXQ_PER_WFQ_CFGm, txq_per_wfq_default, WFQ_MASK_RD_SIZEf, 0x0);
    soc_mem_field32_set(unit, TXQ_PER_WFQ_CFGm, txq_per_wfq_default, DEFAULT_PKT_SIZEf, 0x40);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, TXQ_PER_WFQ_CFGm, txq_per_wfq_default));

    soc_mem_field32_set(unit, PTS_PER_WFQ_CFGm, pts_per_wfq_default, WFQ_WEIGHT_0f, 0x1);
    soc_mem_field32_set(unit, PTS_PER_WFQ_CFGm, pts_per_wfq_default, WFQ_WEIGHT_1f, 0x1);
    soc_mem_field32_set(unit, PTS_PER_WFQ_CFGm, pts_per_wfq_default, WFQ_MASK_RD_SIZEf, 0x0);
    soc_mem_field32_set(unit, PTS_PER_WFQ_CFGm, pts_per_wfq_default, DEFAULT_PKT_SIZEf, 0x40);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, PTS_PER_WFQ_CFGm, pts_per_wfq_default));

    soc_mem_field32_set(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default, WFQ_WEIGHT_0f, 0x1);
    soc_mem_field32_set(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default, WFQ_WEIGHT_1f, 0x1);
    soc_mem_field32_set(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default, WFQ_WEIGHT_2f, 0x1);
    soc_mem_field32_set(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default, WFQ_MASK_RD_SIZEf, 0x0);
    soc_mem_field32_set(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default, DEFAULT_PKT_SIZEf, 0x40);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, PTS_PER_WFQ_MESH_CFGm, pts_per_wfq_mesh_default));

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_fcr_tbls_init(int unit)
{
    uint32 table_entry[1] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    /*
     *  all devices is set to EFMS bypass enabled, EFMS is in bypass mode by default
     */
    soc_mem_field32_set(unit, FCR_EFMS_SOURCE_PIPEm, table_entry, DATAf, 0xff); /*1 bit for each device, 8 devices in row */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, FCR_EFMS_SOURCE_PIPEm, table_entry));
    soc_mem_field32_set(unit, FCR_EFMS_SOURCE_PIPEm, table_entry, DATAf, 0);

exit:
    DNXC_FUNC_RETURN;
}



int soc_jer2_qax_cgm_tbls_init(int unit)
{
    uint32 entry[20] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /* set SRAM only mode */
    soc_mem_field32_set(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, entry, SRAM_DRAM_ONLY_MODEf, -1 );
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, entry));
    sal_memset(entry, 0, sizeof(entry));

    /* Init the Eth mtr table*/
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, CGM_ETH_MTR_PTR_MAPm, MEM_BLOCK_ANY, entry));

    /* disable CNI by default */
    soc_mem_field32_set(unit, CGM_CNI_PRMSm, entry, MAX_SIZE_THf, 0xfff);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, CGM_CNI_PRMSm, entry));
    sal_memset(entry, 0, sizeof(entry));

    /* 
       initialize shared occupied reject thresholds not to discard by default
       reset value of these fields is 0 -- would discard everything 
    */
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, WORDS_SET_THf, 0x1fff);
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, WORDS_CLR_THf, 0x1fff);
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, SRAM_WORDS_SET_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, SRAM_WORDS_CLR_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, SRAM_PDS_SET_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry, SRAM_PDS_CLR_THf, 0xfff);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, CGM_VOQ_SHRD_OC_RJCT_THm, entry));
    sal_memset(entry, 0, sizeof(entry));
	
	/* Set VSQ shared pools to max */
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, WORDS_SET_THf, 0x1fff);
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, WORDS_CLR_THf, 0x1fff);
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, SRAM_BUFFERS_SET_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, SRAM_BUFFERS_CLR_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, SRAM_PDS_SET_THf, 0xfff);
    soc_mem_field32_set(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry, SRAM_PDS_CLR_THf, 0xfff);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, CGM_VSQ_SHRD_OC_RJCT_THm, entry));
    sal_memset(entry, 0, sizeof(entry));

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * Tables Init Functions
 */ 
int soc_jer2_qax_sch_tbls_init(int unit)
{
    uint32 table_entry[4] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    if (!SOC_IS_QUX(unit)) {
    /* all relevant bits initialized to DNX_TMC_MAX_FAP_ID */
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, DNX_TMC_MAX_FAP_ID);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry));
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, 0);
    }
    soc_mem_field32_set(unit, SCH_PS_8P_RATES_PSRm, table_entry, PS_8P_RATES_PSRf, 128);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, SCH_PS_8P_RATES_PSRm, table_entry));

    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_0_WEIGHTf, 0x1);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_1_WEIGHTf, 0x2);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_2_WEIGHTf, 0x4);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_3_WEIGHTf, 0x8);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_4_WEIGHTf, 0x10);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_5_WEIGHTf, 0x20);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_6_WEIGHTf, 0x40);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_7_WEIGHTf, 0x80);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry));

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_egq_tbls_init(int unit)
{
    soc_reg_above_64_val_t data;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_PCTm, data, CGM_PORT_PROFILEf, JER2_ARAD_EGR_PORT_THRESH_TYPE_15);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_PCTm, data));

    /* Set the VSI-Membership table to all 1's */
    SOC_REG_ABOVE_64_ALLONES(data);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_VSI_MEMBERSHIPm, data));


    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_DSP_PTR_MAPm, data, OUT_TM_PORTf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_DSP_PTR_MAPm, data));

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_PER_IFC_CFGm, data, OTM_PORT_NUMBERf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_PER_IFC_CFGm, data));

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_PP_PPCTm, data, MTUf, 0x3fff);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_PP_PPCTm, data));


    if (SOC_DNX_CONFIG(unit)->emulation_system) {
        /*
         * Tables reset script used for emulation is not effective on this
         * table for some reason. This is a small table so this is not too
         * heavy an operation for emulation.
         */
        SOC_REG_ABOVE_64_CLEAR(data);
        DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, EGQ_IVEC_TABLEm, data));
    }

exit:
    DNXC_FUNC_RETURN;
}



int soc_jer2_qax_imp_tbls_init(int unit)
{
	uint32 table_entry[4] = {0};
	DNXC_INIT_FUNC_DEFS;

	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_PRFCFG_0m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_PRFCFG_0m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_PRFCFG_1m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_PRFCFG_1m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_PRFSELm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_PRFSELm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_HEADER_APPEND_SIZE_PTR_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_HEADER_APPEND_SIZE_PTR_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_IN_PP_PORT_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_IN_PP_PORT_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IMP_MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAPm, MEM_BLOCK_ANY, table_entry));

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_iep_tbls_init(int unit)
{
	uint32 table_entry[4] = {0};
	DNXC_INIT_FUNC_DEFS;

	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_PRFCFG_0m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_PRFCFG_0m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_PRFCFG_1m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_PRFCFG_1m, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_PRFSELm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_PRFSELm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_HEADER_APPEND_SIZE_PTR_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_HEADER_APPEND_SIZE_PTR_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_IN_PP_PORT_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_IN_PP_PORT_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDA_IN_PP_PORT_MAP_AND_MTR_PRF_MAPm, MEM_BLOCK_ANY, table_entry));
	DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IEP_MCDB_IN_PP_PORT_MAP_AND_MTR_PRF_MAPm, MEM_BLOCK_ANY, table_entry));

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_qax_tar_tbls_init(int unit)
{
    uint32 table_entry[128] = {0};

    DNXC_INIT_FUNC_DEFS;

    /* destination table */
    table_entry[0] = 0; /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, TAR_DESTINATION_TABLEm, table_entry, QUEUE_NUMBERf, JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit)); /* mark as disabled entry */
    soc_mem_field32_set(unit, TAR_DESTINATION_TABLEm, table_entry, TC_PROFILEf, 0); /* JER2_ARAD_IPQ_TC_PROFILE_DFLT is 0 */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, TAR_DESTINATION_TABLEm, table_entry)); /* fill table with the entry */
    sal_memset(table_entry, 0, sizeof(table_entry));

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_jer2_qax_txq_tbls_init
* TYPE:
*   PROC
* DATE:
*   Dec 14 2015
* FUNCTION:
*     Init TXQ priority tables TXQ_PRIORITY_BITS_MAPPING_2_FDT according to tc bits
* INPUT:
*  DNX_SAND_IN  int                                unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   None
*********************************************************************/

soc_error_t
  soc_jer2_qax_txq_tbls_init(
      DNX_SAND_IN  int                 unit
    )
{
    int i;

    uint32 fabric_priority, tc, fabric_mc_hp_lp_boundary = -1;
    soc_dnxc_fabric_pipe_map_type_t map_type;
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    map_type = SOC_DNX_FABRIC_PIPES_CONFIG(unit).mapping_type;

    /*In case of "uc / hp mc / lp mc" configuration - get the fabric priority boundary for moving from lp to hp*/
    if (map_type == soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc){
        for(i=0;i<SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES-1 ; i++) {
           if (SOC_DNX_FABRIC_PIPES_CONFIG(unit).config_mc[i] != SOC_DNX_FABRIC_PIPES_CONFIG(unit).config_mc[i+1]){
               fabric_mc_hp_lp_boundary = i;
               break;
           }
       }
       if(fabric_mc_hp_lp_boundary == -1){
           DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("mode is fabric_pipe_map_triple_uc_hp_mc_lp_mc but all mc priorities are the same \n")));
       }
    }


    /*0 - 3 - fabric priorities*/
    for(i=0;i<SOC_JER2_QAX_FABRIC_PRIORITY_NDX_NOF ; i++) {
        if (map_type == soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc)
        {
            /*fabric_priority must be fully aligned with hp_bit since dtq uses hp_bit to decide queue id while FDR uses fabric_priority to decide pipe id.
             *  Therefore, in "uc / hp mc / lp mc" configuration =>
             *     LP bit ==> priority 0 (= lowest priority of lp mc)
             *     HP bit ==> lowest priority of hp mc (= fabric_mc_hp_lp_boundary+1) */
            fabric_priority = (i & SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_HP_MASK) >> SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_HP_OFFSET ? (fabric_mc_hp_lp_boundary+1) : 0;
        }else{
            /*params according to i*/
            /*tc=0, 1, 2 ==> prio=0*/
            /*tc=3, 4, 5 ==> prio=1*/
            /*tc=6, 7    ==> prio=2*/
            tc = (i & SOC_JER2_QAX_FABRIC_PRIORITY_NDX_TC_MASK) >> SOC_JER2_QAX_FABRIC_PRIORITY_NDX_TC_OFFSET;
            fabric_priority = tc/3;
        }
        DNXC_IF_ERR_EXIT(WRITE_TXQ_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &fabric_priority));
    }

    /* mark the memory not to be later zeroed */
    SHR_BITSET(SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap, TXQ_PRIORITY_BITS_MAPPING_2_FDTm);

    exit:
      DNXC_FUNC_RETURN;
}

