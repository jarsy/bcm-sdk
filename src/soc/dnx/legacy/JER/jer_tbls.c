/*
 * $Id: drv.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
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
#include <soc/dnx/legacy/multicast_imp.h>

/* SOC DNX JER2_JER includes */
#include <soc/dnx/legacy/JER/jer_tbls.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>

/* SOC DNX Arad includes */ 
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>



/* 
 * Defines
 */

#define JER2_JER_MBMP_SET_EXCLUDED(_mem)      SHR_BITSET(jer2_jer_excluded_mems_bmap_p, _mem)
#define JER2_JER_MBMP_IS_EXCLUDED(_mem)       SHR_BITGET( jer2_jer_excluded_mems_bmap_p, _mem)

/**
 *   explanation to the content of the following skip list
 *    1. ILKN* are not realy memories
 *    2.  EPNI*  is defines only not real memory 
 *    3.  KAPS_TCAM  need special initialization done in kap
 *        module sw
 *    4.  SER* not touched by asic so no need to init
 *    5. SCH_SCHEDULER_INIT  is write only memory
 */

static soc_mem_t jer2_jer_tbls_88375_excluded_mem_list[] = {
    EDB_EEDB_BANKm,
    EDB_EEDB_TOP_BANKm,
    IRR_MCDBm,
    IRR_QUEUE_IS_OCB_COMMITTEDm,
    EDB_LINK_LAYER_OR_ARP_NEW_FORMATm,
    EDB_PROTECTION_ENTRYm,
    EDB_TRILL_FORMAT_FULL_ENTRYm,
    EDB_TRILL_FORMAT_HALF_ENTRYm,
    ILKN_PMH_PORT_0_CPU_ACCESSm,
    ILKN_PMH_PORT_1_CPU_ACCESSm,
    ILKN_PML_PORT_0_CPU_ACCESSm,
    ILKN_PML_PORT_1_CPU_ACCESSm,
    SCH_SCHEDULER_INITm,
    SER_ACC_TYPE_MAPm,
    SER_MEMORYm,
    SER_RESULT_0m,
    SER_RESULT_1m,
    SER_RESULT_DATA_0m,
    SER_RESULT_DATA_1m,
    SER_RESULT_EXPECTED_0m,
    SER_RESULT_EXPECTED_1m,
    EGQ_VSI_MEMBERSHIPm,
    KAPS_TCMm,
    EDB_MAP_OUTLIF_TO_DSPm, /* will be initialized to a none zero value */
    BRDC_FSRD_FSRD_WL_EXT_MEMm,
    FSRD_FSRD_WL_EXT_MEMm,
    XLPORT_WC_UCMEM_DATAm,
    CLPORT_WC_UCMEM_DATAm,

    /* Has to be last memory in array */
    INVALIDm
};

static soc_mem_t *jer2_jer_tbls_88690_excluded_mem_list = jer2_jer_tbls_88375_excluded_mem_list;



/* 
 * Tables Init Functions
 */ 
uint32 soc_jer2_jer_sch_tbls_init(int unit)
{
    uint32 table_entry[128] = {0};
    int core;

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_tbls_to_zero", !(
#ifdef PLISIM
        SAL_BOOT_PLISIM || /*not pcid and not emulation*/
#endif
        SOC_DNX_CONFIG(unit)->emulation_system))) {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_FC_MAP_FCMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_CH_NIF_RATES_CONFIGURATION_CNRCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_FLOW_SUB_FLOW_FSFm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
    }

    /* all relevant bits initialized to DNX_TMC_MAX_FAP_ID */
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, DNX_TMC_MAX_FAP_ID);
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, 0);

    /* trigger SCH_SCHEDULER_INIT to reset dynamic SCH tables */
    soc_mem_field32_set(unit, SCH_SCHEDULER_INITm, table_entry, SCH_INITf, 0x1);
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_INITm(unit, SCH_BLOCK(unit, core), 0x0, table_entry));
    }
    soc_mem_field32_set(unit, SCH_SCHEDULER_INITm, table_entry, SCH_INITf, 0x0);

    soc_mem_field32_set(unit, SCH_PS_8P_RATES_PSRm, table_entry, PS_8P_RATES_PSRf, 128);
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_PS_8P_RATES_PSRm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry));
    soc_mem_field32_set(unit, SCH_PS_8P_RATES_PSRm, table_entry, PS_8P_RATES_PSRf, 0);

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_0m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_1m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_2m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_3m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_4m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_5m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_6m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_7m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_8m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_9m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_10m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_11m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_12m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_13m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_14m, SCH_BLOCK(unit, core), table_entry));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR_CAL_15m, SCH_BLOCK(unit, core), table_entry));
    }

    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_0_WEIGHTf, 0x1);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_1_WEIGHTf, 0x2);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_2_WEIGHTf, 0x4);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_3_WEIGHTf, 0x8);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_4_WEIGHTf, 0x10);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_5_WEIGHTf, 0x20);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_6_WEIGHTf, 0x40);
    soc_mem_field32_set(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, table_entry, WFQ_PG_7_WEIGHTf, 0x80);
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, SCH_BLOCK(unit, core), table_entry));
    }

exit:
    DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_irr_tbls_init(int unit)
{
    uint32 m[2]= {0};
    DNXC_INIT_FUNC_DEFS;
    /*If there isn't dram in the system then define all queues as OCB-only, else define all queues as Dram-mixed.*/
    
    if (/*SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.nof_drams*/ 0) {
        soc_mem_field32_set(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, m, COMMITMENTf, 0x0); 
    } else {
        soc_mem_field32_set(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, m, COMMITMENTf, 0xffffffff); 
    }
    
    if (SOC_DNX_CONFIG(unit)->emulation_system) { /* partial init for emulation: one core and part of the queues */
        DNXC_IF_ERR_EXIT(dnxc_fill_partial_table_with_entry(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, 0, 0, MEM_BLOCK_ALL, 0, 10000, m));
    } else {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, MEM_BLOCK_ALL, m));
    }

    /* Destination Table */
    sal_memset(m, 0, sizeof(m)); /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, m, QUEUE_NUMBERf, 0x1ffff); /* mark as disabled entry */
    soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, m, TC_PROFILEf, 0); /* JER2_ARAD_IPQ_TC_PROFILE_DFLT is 0 */
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IRR_DESTINATION_TABLEm, MEM_BLOCK_ANY, m)); /* fill table with the entry */
    sal_memset(m, 0, sizeof(m));

exit:
    DNXC_FUNC_RETURN;
}

uint32  soc_jer2_jer_idr_tbls_init (int unit){
	uint32 table_entry = 0;
	DNXC_INIT_FUNC_DEFS;

	
	DNXC_IF_ERR_EXIT(dnxc_fill_partial_table_with_entry(unit, IDR_ETHERNET_METER_CONFIGm,0,1, MEM_BLOCK_ANY,0,1279, &table_entry));
    /* ingress Qs buffer type selection */
    table_entry = 1;
	DNXC_IF_ERR_EXIT(dnxc_fill_partial_table_with_entry(unit, IDR_DRAM_BUFFER_TYPEm, 0, 0, MEM_BLOCK_ANY, 48, 63, &table_entry));

exit:
    DNXC_FUNC_RETURN;

}

uint32 soc_jer2_jer_ire_tbls_init(int unit)
{
    DNXC_INIT_FUNC_DEFS;

/*exit:*/
    DNXC_FUNC_RETURN;
}

uint32 soc_jer2_jer_iqm_tbls_init( int unit)
{
    uint32 table_default[SOC_MAX_MEM_WORDS] = {0};
    uint32 table_pcp_default[SOC_MAX_MEM_WORDS] = {0xaaaaaaaa};
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /* Initialize IQM_PACKING_MODE_TABLE*/
    /* Each line in the table configures fabric-pcp mode of 16 devices*/
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_pcp_enable) {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQM_PACK_MODEm, IQM_BLOCK(unit, SOC_CORE_ALL), table_pcp_default)) ;
    } else {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQM_PACK_MODEm, IQM_BLOCK(unit, SOC_CORE_ALL), table_default)) ;
    }
    soc_mem_field32_set(unit, IQM_PQWQm, table_default, PQ_WEIGHTf, 2);
    soc_mem_field32_set(unit, IQM_PQWQm, table_default, PQ_AVRG_ENf, 1); /* Needed to enable WRED for the rate class */
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQM_PQWQm, IQM_BLOCK(unit, SOC_CORE_ALL),  table_default)); /* fill table with the entry */
    sal_memset(table_default, 0, sizeof(table_default));

exit:
    DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_iqmt_tbls_init(int unit)
{
    uint32 table_entry = 0;
    soc_reg_above_64_val_t table_default;
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);


    /* IQMT_ING_RPT_PCM is dynamic, so we need to take dynamic access */
    DNXC_IF_ERR_EXIT(WRITE_IQMT_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

    /* Initialize IQMT_ING_RPT_PCM */
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_ING_RPT_PCMm, IQMT_BLOCK(unit), &table_entry));

    /* Initialize IQMT_PDM_X */
    if (!SAL_BOOT_PLISIM && SOC_MEM_IS_VALID(unit, IQMT_PDM_0m)) {
        SOC_REG_ABOVE_64_CLEAR(table_default);
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_0m, IQMT_BLOCK(unit), table_default));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_1m, IQMT_BLOCK(unit), table_default));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_2m, IQMT_BLOCK(unit), table_default));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_3m, IQMT_BLOCK(unit), table_default));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_4m, IQMT_BLOCK(unit), table_default));
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IQMT_PDM_5m, IQMT_BLOCK(unit), table_default));
    }

    DNXC_IF_ERR_EXIT(WRITE_IQMT_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 0));

exit:
    DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_ips_tbls_init(int unit)
{
    uint32 table_entry[2] = {JER2_ARAD_IPQ_INVALID_FLOW_QUARTET};
    uint8 silent = 0;

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /* Init common Arad tables */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_mgmt_ips_tbls_init(unit, silent));

     /*
     *  all devices is set to FMS enabled, EFMS is in bypass mode by default
     */
    soc_mem_field32_set(unit, IPS_FMSBYPm, table_entry, FMS_BYP_BMPf, 0x0000); /*1 bit for each device, 16 devices in row */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, IPS_FMSBYPm, IPS_BLOCK(unit, SOC_CORE_ALL), table_entry));
    soc_mem_field32_set(unit, IPS_FMSBYPm, table_entry, FMS_BYP_BMPf, 0);

exit:
    DNXC_FUNC_RETURN;
}


uint32 soc_jer2_jer_fcr_tbls_init(int unit)
{
    uint32 table_entry[1] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    /*
     *  all devices is set to EFMS bypass enabled, EFMS is in bypass mode by default
     */
    soc_mem_field32_set(unit, FCR_EFMS_SOURCE_PIPEm, table_entry, DATAf, 0xff); /*1 bit for each device, 8 devices in row */
    DNXC_SAND_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, FCR_EFMS_SOURCE_PIPEm, FCR_BLOCK(unit), table_entry));
    soc_mem_field32_set(unit, FCR_EFMS_SOURCE_PIPEm, table_entry, DATAf, 0);

exit:
    DNXC_FUNC_RETURN;
}



/*********************************************************************
* NAME:
*     soc_jer2_jer_ipt_tbls_init
* TYPE:
*   PROC
* DATE:
*   Dec 14 2015
* FUNCTION:
*     Init IPT priority tables (IPT_PRIORITY_BITS_MAPPING_2_FDT and
*     IPT_TDM_BIT_MAPPING_2_FDT) according to tc and tdm bits
* INPUT:
*  DNX_SAND_IN  int                                unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   None
*********************************************************************/

soc_error_t
  soc_jer2_jer_ipt_tbls_init(
      DNX_SAND_IN  int                 unit
    )
{
    int i;

    uint32 fabric_priority;

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /*0 - 3 - fabric priorities*/
    for(i=0;i<SOC_JER2_JER_FABRIC_PRIORITY_NDX_NOF ; i++) {
        uint32 is_tdm, tc;
        /*params according to i*/
        is_tdm = (i & SOC_JER2_JER_FABRIC_PRIORITY_NDX_IS_TDM_MASK) >> SOC_JER2_JER_FABRIC_PRIORITY_NDX_IS_TDM_OFFSET;
        tc = (i & SOC_JER2_JER_FABRIC_PRIORITY_NDX_TC_MASK) >> SOC_JER2_JER_FABRIC_PRIORITY_NDX_TC_OFFSET;

        if(is_tdm) {
            fabric_priority = 3;
        } else {/*according to tc*/
            /*tc=0, 1, 2 ==> prio=0*/
            /*tc=3, 4, 5 ==> prio=1*/
            /*tc=6, 7    ==> prio=2*/
            fabric_priority = tc/3;
        }

        DNXC_IF_ERR_EXIT(WRITE_IPT_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &fabric_priority));
        DNXC_IF_ERR_EXIT(WRITE_IPT_TDM_BIT_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &is_tdm));
    }

    exit:
      DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_fdt_tbls_init(int unit)
{
    uint32 table_default[SOC_MAX_MEM_WORDS] = {0};
    uint32 pcp_config_data = 2;
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    if ((SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_EGR_CORE_MESH_MODE) == 0 && /* If the table is used for mesh MC, it must be cleared */
         SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_pcp_enable) {
        /* fabric-pcp is configured in bits 6:7 of each line */
        SHR_BITCOPY_RANGE(table_default, 6, &pcp_config_data, 0, SOC_JER2_JER_FABRIC_PCP_LENGTH);
    } else {
        
    }

    /* Initialize FDT_IPT_MESH_MC table */
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, FDT_IPT_MESH_MCm, MEM_BLOCK_ALL, table_default));

exit:
    DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_egq_tbls_init(int unit)
{
    soc_reg_above_64_val_t data;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_PCTm, data, CGM_PORT_PROFILEf, JER2_ARAD_EGR_PORT_THRESH_TYPE_15);
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, EGQ_PCTm, MEM_BLOCK_ALL, data));

    /* Set the VSI-Membership table to all 1's */
    SOC_REG_ABOVE_64_ALLONES(data);
    DNXC_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, EGQ_VSI_MEMBERSHIPm, MEM_BLOCK_ALL, data));

    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, EDB_MAP_OUTLIF_TO_DSPm, MEM_BLOCK_ALL, data)); /* map all CUDs to a none valid port */

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_DSP_PTR_MAPm, data, OUT_TM_PORTf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
    DNXC_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, EGQ_DSP_PTR_MAPm, MEM_BLOCK_ALL, data));

    SOC_REG_ABOVE_64_CLEAR(data);
    soc_mem_field32_set(unit, EGQ_PER_IFC_CFGm, data, OTM_PORT_NUMBERf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
    DNXC_IF_ERR_EXIT(jer2_arad_fill_table_with_entry(unit, EGQ_PER_IFC_CFGm, MEM_BLOCK_ALL, data));

exit:
    DNXC_FUNC_RETURN;
}



uint32 soc_jer2_jer_oamp_tbls_init(int unit)
{
    soc_reg_above_64_val_t table_default;
    uint32 write_val = 1;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(table_default);


    /* initialize DMM_TRIGER (sic) table*/
    DNXC_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit , write_val));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_DM_TRIGERm, MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_RX_FLOW_PARAMSm , MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_RX_FLOW_STATSm , MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_TXm , MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_TX_EVC_PARAMS_ENTRY_1m , MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_TX_EVC_PARAMS_ENTRY_2m , MEM_BLOCK_ALL, table_default));
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, OAMP_SAT_TX_GEN_PARAMSm , MEM_BLOCK_ALL, table_default));

    write_val = 0;
    DNXC_IF_ERR_EXIT(WRITE_OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit , write_val));

    

exit:
    DNXC_FUNC_RETURN;
}


/*
* CLP external memory is connected to interfaces on PLL2.
* When PLL1 is enabled init all blocks.
*/
uint32 soc_jer2_jer_clp_tbls_init(int unit)
{
    uint32 reset_value[DNXC_MAX_U32S_IN_MEM_ENTRY] = {0};
    soc_jer2_jer_pll_config_t        *pll;

    DNXC_INIT_FUNC_DEFS;

    pll = &SOC_DNX_CONFIG(unit)->jer2_jer->pll;

   if (!(pll->ref_clk_pmh_in == soc_dnxc_init_serdes_ref_clock_disable || pll->ref_clk_pmh_out == soc_dnxc_init_serdes_ref_clock_disable)) {
        DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, CLPORT_WC_UCMEM_DATAm, MEM_BLOCK_ALL, reset_value));

    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer2_jer_excluded_tbls_list_set
 * Purpose:
 *      sets the excluded memory list with the relevant memories
 * Parameters:
 *      unit    - Device Number 
 * Returns:
 *      SOC_E_XXX 
 * Note:
 *      to insert a memory to excluded list write the memory's name in the relevant exclude list above
 */
int soc_jer2_jer_excluded_tbls_list_set(int unit) 
{
    SHR_BITDCL *jer2_jer_excluded_mems_bmap_p = NULL;
    int mem_iter = 0;
    soc_mem_t* excluded_list;

    DNXC_INIT_FUNC_DEFS;

    /* get relevant exclude mems bmap of wanted device and define jer2_jer_excluded_mems_bmap_p for MACROs */
    jer2_jer_excluded_mems_bmap_p = SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap;

    /* get relevant exclude mems list of wanted device type */
    if (SOC_IS_QMX(unit)) {
        excluded_list = jer2_jer_tbls_88375_excluded_mem_list;
    } else if(SOC_IS_JERICHO(unit)){ 
        excluded_list = jer2_jer_tbls_88690_excluded_mem_list;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Unknown Device Type\n")));
    }

    /* set exclude bmap to zero */
    sal_memset(jer2_jer_excluded_mems_bmap_p, 0, SHR_BITALLOCSIZE(NUM_SOC_MEM));

    /* iterate over exclude list to set bmap */
    mem_iter = 0;
    while (excluded_list[mem_iter] != INVALIDm) {
        JER2_JER_MBMP_SET_EXCLUDED(excluded_list[mem_iter]);
        ++mem_iter;
    }

exit:
    DNXC_FUNC_RETURN;     
}

/*
 * Function:
 *      soc_jer2_jer_static_tbls_reset
 * Purpose:
 *      iterates over all memories and resets the static ones
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jer_static_tbls_reset (int unit)
{
    SHR_BITDCL *jer2_jer_excluded_mems_bmap_p = NULL;
    int mem_iter = 0;
    uint32 reset_value[128] = {0};

    DNXC_INIT_FUNC_DEFS;


    
    /* set excluded mem list */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_excluded_tbls_list_set(unit));

    /* define jer2_jer_excluded_mems_bmap_p for MACROs */
    jer2_jer_excluded_mems_bmap_p = SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap;

    /* iterate over all mems */
    for (mem_iter = 0; mem_iter < NUM_SOC_MEM; mem_iter++) 
    {
        if (SOC_MEM_IS_VALID(unit, mem_iter) && /* Memory must be valid for the device */
            /* memory must be static (not dynamic) and not read-only */
            (soc_mem_flags(unit, mem_iter) & (SOC_MEM_FLAG_SIGNAL | SOC_MEM_FLAG_READONLY)) == 0 &&

            /* memory must not be an alias, to avoid multiple resets of the same memory */
            (mem_iter == SOC_MEM_ALIAS_MAP(unit, mem_iter) || !SOC_MEM_IS_VALID(unit, SOC_MEM_ALIAS_MAP(unit, mem_iter))) &&
            !JER2_JER_MBMP_IS_EXCLUDED(mem_iter)) { /* if mem is in excluded list */

            /* reset memory - set all values to 0 */
            LOG_VERBOSE(BSL_LS_SOC_INIT,(BSL_META_U(unit,"Reseting static memory # %d - %s\n"),mem_iter, SOC_MEM_NAME(unit, mem_iter)));

            
            DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, mem_iter, MEM_BLOCK_ALL, reset_value));
        }
    }

    /* Init tables which are wrongly marked as dynamic */

    /* Allow writes to FDA_FDA_MCm, and stay with this configuration for later writes to this memory */
    DNXC_IF_ERR_EXIT(WRITE_FDA_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, FDA_FDA_MCm, MEM_BLOCK_ALL, reset_value));

    
    exit:
        DNXC_FUNC_RETURN;     
}

static 
int
soc_jer2_jer_enable_dynamic_access(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    uint32 value = *(uint32 *)data;
    int blk;
    int inst=0;

    DNXC_INIT_FUNC_DEFS;
    if (!SOC_REG_IS_VALID(unit,ainfo->reg) || 
        sal_strstr(SOC_REG_NAME(unit,ainfo->reg),"BRDC")!=NULL  ||
        !SOC_REG_FIELD_VALID(unit,ainfo->reg,ENABLE_DYNAMIC_MEMORY_ACCESSf)) {
        SOC_EXIT;
    }
    SOC_BLOCK_ITER(unit, blk, SOC_REG_FIRST_BLK_TYPE(SOC_REG_INFO(unit, ainfo->reg).block)) {
        DNXC_IF_ERR_EXIT(soc_reg_field32_modify(unit, ainfo->reg, inst,ENABLE_DYNAMIC_MEMORY_ACCESSf, value));
        inst++;
    }



    exit:
        DNXC_FUNC_RETURN;     
}

static
 int
soc_jer2_jer_mem_reset(int unit, soc_mem_t mem, void* reset_value)
{
    DNXC_INIT_FUNC_DEFS;
    if (!soc_mem_is_signal(unit,mem) || soc_mem_is_readonly(unit,mem)) {
        SOC_EXIT;
    }
    switch (mem) {
 
            /**
             * not realy memories by Ariel S.
             */
        case IQM_PAKCET_DESCRIPTOR_MEMORY_A_DYNAMICm:
        case IQM_PAKCET_DESCRIPTOR_MEMORY_B_DYNAMICm:
        case IQM_PAKCET_DESCRIPTOR_MEMORY_ECC_DYNAMICm:
        /**
         * not realy memories by Daniel M.
         */
        case MMU_DRAM_ADDRESS_SPACEm:
        case OCB_OCB_ADDRESS_SPACEm:



        SOC_EXIT;


    }
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, mem, MEM_BLOCK_ALL, reset_value)); 

    exit:
        DNXC_FUNC_RETURN;     
}

int
soc_jer2_jer_dynamic_tbls_reset (int unit)
{
    soc_reg_above_64_val_t reg_value;
    uint32 value=1;
    uint32 reset_value[128] = {0};
    DNXC_INIT_FUNC_DEFS;

    /* first enable dynamic access/disable for all blocks*/
    DNXC_IF_ERR_EXIT(soc_reg_iterate(unit, soc_jer2_jer_enable_dynamic_access, &value)) ;
    DNXC_IF_ERR_EXIT(soc_mem_iterate(unit,soc_jer2_jer_mem_reset, reset_value));
    /* and then  disable dynamic access  for all blocks*/
    value=0;
    DNXC_IF_ERR_EXIT(soc_reg_iterate(unit, soc_jer2_jer_enable_dynamic_access, &value)) ;

    /**
     * we make here blocks sw reset since  we zeroing all the 
     * dynamic memory before to prevet ser and now we want that the 
     * hw initialized dynamic memories with real values 
     */

    SOC_REG_ABOVE_64_ALLONES(reg_value);  
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_value));
    SOC_REG_ABOVE_64_CLEAR(reg_value);
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_value));

     /*  reset again the IRE*/
    soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_value, BLOCKS_SOFT_INIT_1f, 1); 
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_value));
    SOC_REG_ABOVE_64_CLEAR(reg_value);
    DNXC_IF_ERR_EXIT(WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_value));


    exit:
        DNXC_FUNC_RETURN;     
}
/*
 * Read table SCH_DEVICE_RATE_MEMORY_DRM from block SCH,
 * doesn't take semaphore.
 * See also:
 *   jer2_jer_sch_device_rate_entry_core_get_unsafe(), jer2_arad_sch_drm_tbl_get_unsafe()
 *   jer2_arad_sch_device_rate_entry_get_unsafe()
 */
uint32
  jer2_jer_sch_drm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_DRM_TBL_DATA* SCH_drm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE];
  soc_mem_t mem ;
  soc_field_t field ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DRM_TBL_GET_UNSAFE);
  err = dnx_sand_os_memset(&(data[0]),0x0,sizeof(data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  err = dnx_sand_os_memset(SCH_drm_tbl_data,0x0,sizeof(JER2_ARAD_SCH_DRM_TBL_DATA));
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);
  /*
   * Note that this table is only for multi-core devices. Not for Arad.
   * field = (soc_field_t)DEVICE_RATEf ;
   */
  mem = (soc_mem_t)SCH_DEVICE_RATE_MEMORY_DRMm ;
  field = (soc_field_t)DEVICE_RATEf ;
  err = soc_mem_read(unit,mem,SCH_BLOCK(unit, core),entry_offset,data) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  SCH_drm_tbl_data->device_rate = soc_mem_field32_get(unit,mem,data,field);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_sch_drm_tbl_get_unsafe()",0,0);
}
/*
 * Write table SCH_DEVICE_RATE_MEMORY_DRM from block SCH,
 * doesn't take semaphore.
 * See also:
 *   jer2_jer_sch_device_rate_entry_core_set_unsafe(), jer2_arad_sch_drm_tbl_set_unsafe()
 */
uint32
  jer2_jer_sch_drm_tbl_set_unsafe(
    DNX_SAND_IN   int                   unit,
    DNX_SAND_IN   int                   core,
    DNX_SAND_IN   uint32                entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_DRM_TBL_DATA *SCH_drm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE];
  soc_mem_t mem ;
  soc_field_t field ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DRM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]),0x0,sizeof(data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  /*
   * Note that this table is only for multi-core devices. Not for Arad.
   * field = (soc_field_t)DEVICE_RATEf ;
   */
  mem = (soc_mem_t)SCH_DEVICE_RATE_MEMORY_DRMm ;
  field = (soc_field_t)DEVICE_RATEf ;
  /*
   * Use SCH0 although, apparently, any of the two may be used.
   */
  soc_mem_field32_set(unit,mem,data,field,SCH_drm_tbl_data->device_rate);
  err = soc_mem_write(unit,mem,SCH_BLOCK(unit, core),entry_offset,data) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_jer_sch_drm_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_jer_sch_shds_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHDS_TBL_GET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_SHDS_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_SHDS_OFFSET;
  }
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_shds_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_SHDS_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  SCH_shds_tbl_data->peak_rate_man_even   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    PEAK_RATE_MAN_EVENf            );
  SCH_shds_tbl_data->peak_rate_exp_even   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    PEAK_RATE_EXP_EVENf            );
  SCH_shds_tbl_data->max_burst_even   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    MAX_BURST_EVENf            );
  SCH_shds_tbl_data->slow_rate2_sel_even   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    SLOW_RATE_2_SEL_EVENf            );
  SCH_shds_tbl_data->peak_rate_man_odd   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    PEAK_RATE_MAN_ODDf            );
  SCH_shds_tbl_data->peak_rate_exp_odd   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    PEAK_RATE_EXP_ODDf            );
  SCH_shds_tbl_data->max_burst_odd   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    MAX_BURST_ODDf            );
  SCH_shds_tbl_data->slow_rate2_sel_odd   = soc_mem_field32_get(
    unit,
    SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data,
    SLOW_RATE_2_SEL_ODDf            );
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shds_tbl_get_unsafe()",0,0);
}
/*
 * Write indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_jer_sch_shds_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  )
{
  uint32
    err;
  uint32
    data_shds[JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
  uint32 orig_val; /*for SCH_ENABLE_DYNAMIC_MEMORY_ACCESS change*/
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHDS_TBL_SET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_SHDS_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_SHDS_OFFSET;
  }
  err = dnx_sand_os_memset(
          &(data_shds[0]),
          0x0,
          sizeof(data_shds)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,PEAK_RATE_MAN_EVENf,
    SCH_shds_tbl_data->peak_rate_man_even );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,PEAK_RATE_EXP_EVENf,
    SCH_shds_tbl_data->peak_rate_exp_even );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,MAX_BURST_EVENf,
    SCH_shds_tbl_data->max_burst_even );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,SLOW_RATE_2_SEL_EVENf,
    SCH_shds_tbl_data->slow_rate2_sel_even );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,PEAK_RATE_MAN_ODDf,
    SCH_shds_tbl_data->peak_rate_man_odd );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,PEAK_RATE_EXP_ODDf,
    SCH_shds_tbl_data->peak_rate_exp_odd );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,MAX_BURST_ODDf,
    SCH_shds_tbl_data->max_burst_odd );
  soc_mem_field32_set(
    unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
    data_shds,SLOW_RATE_2_SEL_ODDf,
    SCH_shds_tbl_data->slow_rate2_sel_odd );
  {
    uint32  val32 ;
    uint32 slow_max_bucket_width ;
    soc_field_info_t *soc_field_info_p ;
    /*
     * maximal allowed value for 'max_burst' field
     */
    uint32 max_val_max_burst ;
    /*
     * Bits in field
     */
    uint16 max_burst_field_len ;
    /*
     * Space to hold data to be loaded into TMC table
     */
    uint32
      data_jer2_tmc[JER2_JERICHO_SCH_TOKEN_MEMORY_CONTROLLER_TMC_TBL_ENTRY_SIZE] ;

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(err,10,exit,JER2_ARAD_REG_ACCESS_ERR,READ_SCH_SHAPER_CONFIGURATION_REGISTER_1r(unit, core, &val32)) ;
    slow_max_bucket_width = soc_reg_field_get(unit, SCH_SHAPER_CONFIGURATION_REGISTER_1r, val32, SLOW_MAX_BUCKET_WIDTHf) ;
    /*
     * Verify: The SLOW_MAX_BUCKET_WIDTH field (on SCH_SHAPER_CONFIGURATION_REGISTER_1)
     * may only have values from 0 to 3.
     * Redundant but safer.
     */
    DNX_SAND_ERR_IF_ABOVE_MAX(slow_max_bucket_width, 3, JER2_ARAD_FLD_OUT_OF_RANGE, 20, exit) ;
    /*
     * We assume that the widths of MAX_BURST_EVEN and MAX_BURST_ODD are the same.
     */
    soc_field_info_p = soc_mem_fieldinfo_get(unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,MAX_BURST_EVENf) ;
    DNX_SAND_CHECK_NULL_PTR(soc_field_info_p, 30, exit) ;
    max_burst_field_len = soc_field_info_p->len ;
    max_val_max_burst = (1 << max_burst_field_len) - 1 ;
    /*
     * For slow_max_bucket_width = 2,3 the maximum allowed val for MAX_BURST_EVEN/MAX_BURST_ODD is 255,127
     * respectively. 
     * For slow_max_bucket_width = 0,1 the maximum allowed val for MAX_BURST_EVEN/MAX_BURST_ODD is 511
     */
    if (slow_max_bucket_width >= 2)
    {
      max_val_max_burst = (1 << (max_burst_field_len - slow_max_bucket_width + 1)) - 1 ;
    }
    /*
     * Start by verifying range for MAX_BURST_EVEN/MAX_BURST_ODD.
     *
     * Load fields TOKEN_COUNT,SLOW_STATUS on:
     * SCH_TOKEN_MEMORY_CONTROLLER_TMC (even flow)
     * SCH_TOKEN_MEMORY_CONTROLLER_TMC_MSB (odd flow)
     * TOKEN_COUNT gets the value of MAX_BURST_EVEN/MAX_BURST_ODD as set above.
     * Note that SLOW_STATUS is ALWAYS set to '1' 
     */

    if (SCH_shds_tbl_data->max_burst_update_even)
    {
      DNX_SAND_ERR_IF_ABOVE_MAX(SCH_shds_tbl_data->max_burst_even, max_val_max_burst,JER2_ARAD_FLD_OUT_OF_RANGE,40,exit) ;
    }
    if (SCH_shds_tbl_data->max_burst_update_odd)
    {
      DNX_SAND_ERR_IF_ABOVE_MAX(SCH_shds_tbl_data->max_burst_odd, max_val_max_burst,JER2_ARAD_FLD_OUT_OF_RANGE,60,exit) ;
    }

    if (SCH_shds_tbl_data->max_burst_update_even)
    {
      err = soc_reg32_get(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, &orig_val);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 48, exit);
      err = soc_reg32_set(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, 1);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 49, exit);
      err =
        dnx_sand_os_memset(&(data_jer2_tmc[0]),0x0,sizeof(data_jer2_tmc));
      soc_mem_field32_set(
        unit,SCH_TOKEN_MEMORY_CONTROLLER_TMCm,
        data_jer2_tmc,TOKEN_COUNTf,SCH_shds_tbl_data->max_burst_even );
      soc_mem_field32_set(
        unit,SCH_TOKEN_MEMORY_CONTROLLER_TMCm,
        data_jer2_tmc,SLOW_STATUSf,1 );
      err =
        soc_mem_write(
          unit,SCH_TOKEN_MEMORY_CONTROLLER_TMCm,
          SCH_BLOCK(unit,core),entry_offset,data_jer2_tmc);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 50, exit);
      /* Restore dynamic memory access */
      err = soc_reg32_set(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, orig_val);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 51, exit);
    }
    if (SCH_shds_tbl_data->max_burst_update_odd)
    {
       err = soc_reg32_get(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, &orig_val);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 68, exit);
      err = soc_reg32_set(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, 1);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 69, exit);
      err =
        dnx_sand_os_memset(&(data_jer2_tmc[0]),0x0,sizeof(data_jer2_tmc));
      soc_mem_field32_set(
        unit,SCH_TOKEN_MEMORY_CONTROLLER_TMC_MSBm,
        data_jer2_tmc,TOKEN_COUNTf,
        SCH_shds_tbl_data->max_burst_odd );
      soc_mem_field32_set(
        unit,SCH_TOKEN_MEMORY_CONTROLLER_TMC_MSBm,
        data_jer2_tmc,SLOW_STATUSf,1 );
      err =
        soc_mem_write(
          unit,SCH_TOKEN_MEMORY_CONTROLLER_TMC_MSBm,
          SCH_BLOCK(unit,core),entry_offset,data_jer2_tmc);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 70, exit);
      /* Restore dynamic memory access */
      err = soc_reg32_set(unit, SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr, REG_PORT_ANY, 0, orig_val);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 71, exit);
    }
  }
  err = soc_mem_write(
          unit,SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          SCH_BLOCK(unit,core),entry_offset,
          data_shds);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 80, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shds_tbl_set_unsafe()",0,0);
}

/*
 * Fill the whole table with the given entry, uses fast DMA filling when run on real hardware.
 * Marks the table not to later be filled by soc_jer2_jer_tbls_zero.
 */

int jer2_jer_fill_and_mark_memory(
    const int       unit,
    const soc_mem_t mem,        /* memory/table to fill */
    const void      *entry_data /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  )
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, mem, MEM_BLOCK_ALL, entry_data));
    /* mark the memory not to be later zeroed */
    SHR_BITSET(SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap, mem);

exit:
    DNXC_FUNC_RETURN;
}



/* Jericho+ functions */

/* Static memoried which should be skipped when zeroing all static memories */
static soc_mem_t jer2_jerplus_tbls_excluded_mem_list[] = {
    IRR_MCDBm,
    ILKN_PMH_PORT_0_CPU_ACCESSm, /* These are not real memories */
    ILKN_PMH_PORT_1_CPU_ACCESSm,
    ILKN_PML_PORT_0_CPU_ACCESSm,
    ILKN_PML_PORT_1_CPU_ACCESSm,
    SCH_SCHEDULER_INITm,
    OAMP_FLEX_VER_MASK_TEMPm, 
#ifndef JER2_JERPLUS_REDSOLVE_LATER
    OAMP_FLOW_STAT_ACCUM_ENTRY_34m,
#endif /* JER2_JERPLUS_REDSOLVE_LATER */
    SER_ACC_TYPE_MAPm,
    SER_MEMORYm,
    SER_RESULT_DATA_1m,
    SER_RESULT_DATA_0m,
    SER_RESULT_EXPECTED_1m,
    SER_RESULT_EXPECTED_0m,
    SER_RESULT_1m,
    SER_RESULT_0m,
    
    IHB_FIFO_DSP_1m,
    IHB_FIFO_DSP_2m,
    IHP_FIFO_8_TO_41m,
    /*
     * not realy memories by Ariel S.
     */
    IQM_PAKCET_DESCRIPTOR_MEMORY_A_DYNAMICm,
    IQM_PAKCET_DESCRIPTOR_MEMORY_B_DYNAMICm,
    IQM_PAKCET_DESCRIPTOR_MEMORY_ECC_DYNAMICm,
    /*
     * not realy memories by Daniel M.
     */
    MMU_DRAM_ADDRESS_SPACEm,
    OCB_OCB_ADDRESS_SPACEm,

    /* Failed only on part of JerichoP devices. need to be debugged by ASIC */
    CLPORT_WC_UCMEM_DATAm,
    XLPORT_WC_UCMEM_DATAm,
    /* Has to be last memory in array */
    INVALIDm
};

/* registers to enable Jericho dynamic memory writes */
soc_reg_t jer2_jer_dynamic_mem_enable_regs[] = {
    BRDC_FSRD_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CFC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EDB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FCR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FDA_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FDT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IDR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PMH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PML_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPST_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IQMT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MMU_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MTRPS_EM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OCB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_A_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    PPDB_B_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    RTP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_BBS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    INVALIDr
};

/* Registers to disable Jericho dynamic memory writes (for blocks we don't wish to allow writes to dynamic memories). */
soc_reg_t jer2_jer_dynamic_mem_disable_regs[] = {
    BRDC_FSRD_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CFC_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FCR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    FDT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IDR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IHP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PMH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    ILKN_PML_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPST_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IPT_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    IRR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MMU_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    MTRPS_EM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OAMP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OCB_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    RTP_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    SCH_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    KAPS_BBS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
    INVALIDr
};

/*
 * Functions
 */

static uint32 soc_jer2_jerplus_irr_tbls_init(int unit)
{
    uint32 m[2]= {0};
    DNXC_INIT_FUNC_DEFS;
    /*If there isn't dram in the system then define all queues as OCB-only, else define all queues as Dram-mixed.*/
    
    if (/*!SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.nof_drams*/ 0) {
        soc_mem_field32_set(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, m, COMMITMENTf, 0xffffffff); 
        if (SOC_DNX_CONFIG(unit)->emulation_system) { /* partial init for emulation: one core and part of the queues */
            DNXC_IF_ERR_EXIT(dnxc_fill_partial_table_with_entry(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, 0, 0, MEM_BLOCK_ALL, 0, 10000, m));
            SHR_BITSET(SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap, IRR_QUEUE_IS_OCB_COMMITTEDm);
        } else {
            DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IRR_QUEUE_IS_OCB_COMMITTEDm, m));
        }
    }

    /* Destination Table */
    m[0] = 0; /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, m, QUEUE_NUMBERf, 0x1ffff); /* mark as disabled entry */
    soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, m, TC_PROFILEf, 0); /* JER2_ARAD_IPQ_TC_PROFILE_DFLT is 0 */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IRR_DESTINATION_TABLEm, m)); /* fill table with the entry */

exit:
    DNXC_FUNC_RETURN;
}


static int soc_jer2_jerplus_ips_tbls_init(int unit)
{
    uint32 table_entry[2] = {JER2_ARAD_IPQ_INVALID_FLOW_QUARTET};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    /*
     * Mark whole table as 'invalid'. See jer2_arad_interrupt_handles_corrective_action_ips_qdesc().
     */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IPS_FLWIDm, table_entry));

     /*
     *  all devices is set to FMS enabled, EFMS is in bypass mode by default
     */
    soc_mem_field32_set(unit, IPS_FMSBYPm, table_entry, FMS_BYP_BMPf, 0x0000); /*1 bit for each device, 16 devices in row */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IPS_FMSBYPm, table_entry));

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
* soc_jer2_jerplus_ipt_tbls_init: Init IPT priority tables (IPT_PRIORITY_BITS_MAPPING_2_FDT
*   and IPT_TDM_BIT_MAPPING_2_FDT) according to tc and tdm bits
*********************************************************************/
static int soc_jer2_jerplus_ipt_tbls_init(int unit)
{
    int i;
    uint32 fabric_priority, is_tdm, tc;
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /*0 - 3 - fabric priorities*/
    for (i = 0; i < SOC_JER2_JER_FABRIC_PRIORITY_NDX_NOF; i++) {
        /*params according to i*/
        is_tdm = (i & SOC_JER2_JER_FABRIC_PRIORITY_NDX_IS_TDM_MASK) >> SOC_JER2_JER_FABRIC_PRIORITY_NDX_IS_TDM_OFFSET;
        tc = (i & SOC_JER2_JER_FABRIC_PRIORITY_NDX_TC_MASK) >> SOC_JER2_JER_FABRIC_PRIORITY_NDX_TC_OFFSET;

        if (is_tdm) {
            fabric_priority = 3;
        } else { /* according to tc */
            /*tc=0, 1, 2 ==> prio=0*/
            /*tc=3, 4, 5 ==> prio=1*/
            /*tc=6, 7    ==> prio=2*/
            fabric_priority = tc / 3;
        }

        DNXC_IF_ERR_EXIT(WRITE_IPT_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &fabric_priority));
        DNXC_IF_ERR_EXIT(WRITE_IPT_TDM_BIT_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &is_tdm));
    }
    SHR_BITSET(SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap, IPT_PRIORITY_BITS_MAPPING_2_FDTm);
    SHR_BITSET(SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap, IPT_TDM_BIT_MAPPING_2_FDTm);

exit:
    DNXC_FUNC_RETURN;
}

/*used also for JER2_QAX*/
int soc_jer2_jerplus_fdt_tbls_init(int unit)
{
    uint32 table_default[5] = {0};
    uint32 pcp_config_data = 2;
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /* Initialize FDT_IPT_MESH_MC table */
    if ((SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_EGR_CORE_MESH_MODE) == 0 && /* If the table is used for mesh MC, it must be cleared */
         SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_pcp_enable) {
        /* fabric-pcp is configured in bits 6:7 of each line */
        SHR_BITCOPY_RANGE(table_default, 6, &pcp_config_data, 0, SOC_JER2_JER_FABRIC_PCP_LENGTH);
        DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, FDT_IPT_MESH_MCm, table_default));
    }

exit:
    DNXC_FUNC_RETURN;
}

static int soc_jer2_jerplus_fcr_tbls_init(int unit)
{
    uint32 table_entry[1] = {0};

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);
    /*
     *  all devices is set to EFMS bypass enabled, EFMS is in bypass mode by default
     */
    soc_mem_field32_set(unit, FCR_EFMS_SOURCE_PIPEm, table_entry, DATAf, 0xff); /*1 bit for each device, 8 devices in row */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, FCR_EFMS_SOURCE_PIPEm, table_entry));

exit:
    DNXC_FUNC_RETURN;
}



static int soc_jer2_jerplus_iqm_tbls_init(int unit)
{
    uint32 table_default[1] = {0};
    uint32 table_pcp_default[2] = {0xaaaaaaaa};
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    DNXC_IF_ERR_EXIT(WRITE_IQMT_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, 1));

    /* Initialize IQM_PACKING_MODE_TABLE*/
    /* Each line in the table configures fabric-pcp mode of 16 devices*/
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_pcp_enable) {
        DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IQM_PACK_MODEm, table_pcp_default)) ;
    }
    soc_mem_field32_set(unit, IQM_PQWQm, table_default, PQ_WEIGHTf, 2);
    soc_mem_field32_set(unit, IQM_PQWQm, table_default, PQ_AVRG_ENf, 1); /* Needed to enable WRED for the rate class */
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, IQM_PQWQm, table_default)); /* fill table with the entry */
    sal_memset(table_default, 0, sizeof(table_default));

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * Tables Init Functions
 */ 
static int soc_jer2_jerplus_sch_tbls_init(int unit)
{
    uint32 table_entry[1] = {0};
    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    /* all relevant bits initialized to DNX_TMC_MAX_FAP_ID */
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, DNX_TMC_MAX_FAP_ID);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry));
    soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING_FFMm, table_entry, DEVICE_NUMBERf, 0);

    soc_mem_field32_set(unit, SCH_PS_8P_RATES_PSRm, table_entry, PS_8P_RATES_PSRf, 128);
    DNXC_IF_ERR_EXIT(jer2_jer_fill_and_mark_memory(unit, SCH_PS_8P_RATES_PSRm, table_entry));

exit:
    DNXC_FUNC_RETURN;
}

static int soc_jer2_jerplus_egq_tbls_init(int unit)
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

/* enable/disable dynamic memory writes using the givein dynamic memory write control registers */
int soc_jer2_jer_control_dynamic_mem_writes(
    DNX_SAND_IN int unit,
    DNX_SAND_IN soc_reg_t *regs, /* control registers to write to, terminated by INVALIDr */
    DNX_SAND_IN uint32 val) /* value (0/1) to write to the registers */
{
    const soc_reg_t *r = regs;
    DNXC_INIT_FUNC_DEFS;

    for (; *r != INVALIDr; ++r) {
	    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, *r, REG_PORT_ANY, 0, val));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * initialize memory zeroing exclusion to the given array.
 */
void soc_jer2_jer_tbls_zero_init(int unit, soc_mem_t *mem_exclude_list)
{
    SHR_BITDCL *jer2_jerplus_excluded_mems_bmap_p = SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap;
    soc_mem_t* excluded_list = mem_exclude_list; /* excluded memory list iterator */

    /* Set exclusion bitmap to zero. Bits of manually filled memories will be later set to 1. */
    sal_memset(jer2_jerplus_excluded_mems_bmap_p, 0, SHR_BITALLOCSIZE(NUM_SOC_MEM));

    /* Add excluded memories to the exclusion bitmap */
    for (; *excluded_list != INVALIDm; ++excluded_list)  { /* iterate on the excluded memories */
        SHR_BITSET(jer2_jerplus_excluded_mems_bmap_p, *excluded_list); /* set the bits of excluded memories in the bitmap */
    }
}

/*
 * initialize all tables to zero, except for exception array, and marked not to be zeroed.
 */
int soc_jer2_jer_tbls_zero(int unit)
{
    /* define bitmap for memory exclusions */
    SHR_BITDCL *jer2_jerplus_excluded_mems_bmap_p = SOC_DNX_CONFIG(unit)->jer2_jer->excluded_mems.excluded_mems_bmap;
    int mem_iter = 0;
    uint32 entry0[128] = {0};
    DNXC_INIT_FUNC_DEFS;
        
    /* Zero tables if not running in emulation/simulation */
    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_tbls_to_zero", !(
#ifdef PLISIM
            SAL_BOOT_PLISIM || /*not pcid and not emulation*/
#endif
            SOC_DNX_CONFIG(unit)->emulation_system))) {

        /* Zero all the memories that need it. Memories should not read only, not aliases, not filled earlier... */
        for (mem_iter = 0; mem_iter < NUM_SOC_MEM; mem_iter++) { /* iterate over all memories */
            if (SOC_MEM_IS_VALID(unit, mem_iter) && /* Memory must be valid for the device */
                (soc_mem_flags(unit, mem_iter) & SOC_MEM_FLAG_READONLY) == 0 && /* should not be read-only (dynamic memories are zeroed) */
    
                /* memory must not be an alias, to avoid multiple resets of the same memory */
                (mem_iter == SOC_MEM_ALIAS_MAP(unit, mem_iter) || !SOC_MEM_IS_VALID(unit, SOC_MEM_ALIAS_MAP(unit, mem_iter))) &&
                !SHR_BITGET(jer2_jerplus_excluded_mems_bmap_p, mem_iter)) { /* if the mem is not in excluded bitmap */
    
                /* reset memory - set all values to 0 */
                LOG_VERBOSE(BSL_LS_SOC_INIT,(BSL_META_U(unit,"Reseting static memory # %d - %s\n"),mem_iter, SOC_MEM_NAME(unit, mem_iter)));
    
                DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, mem_iter, MEM_BLOCK_ALL, entry0));
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer2_jerplus_tbls_init
 * Purpose:
 *      initialize all tables relevant for Jericho+.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer2_jerplus_tbls_init(int unit)
{
    /* define bitmap for memory exclusions */
    uint32 sch_init_val = 1;
    DNXC_INIT_FUNC_DEFS;

    /* initialize memory zeroing exclusion to the given array. */
    soc_jer2_jer_tbls_zero_init(unit, jer2_jerplus_tbls_excluded_mem_list);
        
    DNXC_IF_ERR_EXIT(jer2_arad_tbl_access_init_unsafe(unit)); /* init jer2_arad_fill_table_with_variable_values_by_caching() */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_control_dynamic_mem_writes(unit, jer2_jer_dynamic_mem_enable_regs, 1)); /* enable dynamic memory writes */

    /* Init tables having none zero values */

    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_sch_tbls_init(unit)); /* SCH block */
    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_irr_tbls_init(unit)); /* IRR block */

    /* IRE block */
    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_iqm_tbls_init(unit)); /* CGM block */

    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_ips_tbls_init(unit)); /* IPS block */
    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_fcr_tbls_init(unit)); /* FCR block */
    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_ipt_tbls_init(unit)); /* IPT block */

    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_fdt_tbls_init(unit)); /* FDT block */

    DNXC_IF_ERR_EXIT(soc_jer2_jerplus_egq_tbls_init(unit)); /* EGQ block */

    DNXC_IF_ERR_EXIT(soc_jer2_jer_idr_tbls_init(unit));
    
    if (SOC_DNX_CONFIG(unit)->emulation_system <= 1) { /* partial init for emulation: one core and part of the queues */
        DNXC_IF_ERR_EXIT(dnx_mult_rplct_tbl_entry_unoccupied_set_all(unit));
    }
    DNXC_IF_ERR_EXIT(dnx_mcds_multicast_init2(unit));

    /* initialize all tables to zero, except for exception array, and marked not to be zeroed. */
    DNXC_IF_ERR_EXIT(soc_jer2_jer_tbls_zero(unit));
    DNXC_IF_ERR_EXIT(soc_jer2_jer_control_dynamic_mem_writes(unit, jer2_jer_dynamic_mem_disable_regs, 0)); /* disable dynamic memory writes */

    /* trigger SCH_SCHEDULER_INIT to reset dynamic SCH tables, needs to be after the dynamic table zeroing */
    DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_INITm(unit, SCH_BLOCK(unit, SOC_CORE_ALL), 0, &sch_init_val));

exit:
    DNXC_FUNC_RETURN;
}
