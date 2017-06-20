/*
 * $Id: ser.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_MEM

#include <shared/bsl.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <soc/mem.h>

#ifndef  BCM_ROBO_SUPPORT
#include <soc/mcm/allenum.h>
#endif

#include <soc/chip.h>
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
#include <soc/dcmn/dcmn_mem.h>
#include <soc/dcmn/dcmn_intr_handler.h>
#endif /* defined(BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) */

#ifdef BCM_PETRA_SUPPORT
#include <bcm_int/dpp/counters.h>
#endif /* BCM_PETRA_SUPPORT */
#include <soc/dpp/ARAD/arad_interrupts.h>

#include "testlist.h"

extern int bcm_common_linkscan_enable_set(int,int);

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)

#define MAX_NOF_ATTAMPTS_TO_GET_A_RW_BIT 100
#define BIT_IN_UINT32 (sizeof(uint32)*8)
#define MAX_SER_RETRIES 10

typedef enum {
    ECC1 = 0,
    ECC2
} error_type;

/* ser_test_params */
typedef struct ser_test_params_s {
    int unit;
    char* mem_name_parse;           /* memory name to parse */
    soc_mem_t mem;                  /* memory to test*/
    char* index_parse;              /* index to parse */
    int index;                      /* index to test */
    char* array_index_parse;        /* array index to parse */
    int array_index;                /* array index to test */
    int copyno;                     /* block number to test */
    int cache_state;                /* use cache state test */
    uint32 interrupt;  /* interrupt to check result */
    error_type error;               /* error type to generate (ECC1, ECC2)*/
    uint8 run_all;                  /* if set we run the tr for each of the chip memories,otherwise for a specific memory as given to mem_name_parse*/
    int start_from;              /* strat from id relevant if run_all set */
    uint32 count;              /* how many memories to run on */
    uint32 total_counter;            /* total number of memories  run the test*/
    uint32 skipped_counter;           /* total number of memories  skipped the test*/
    uint32 unjustified_skipped_counter; /* total number of memories  unjustified skipped the test and need to be check individually*/
    uint32 error_counter;            /* total number of memories  failed the test*/
    int help;                       /* show usage */
}ser_test_params_t;



typedef struct arad_sch_ser_map_s {
    soc_mem_t mem;
    int    interrupt;
    soc_reg_t error_initiate_reg;
    soc_field_t error_initiate_field;
    int nof_errors;  

}arad_sch_ser_map_t;

static arad_sch_ser_map_t arad_sch_ser_map[] = {

    {SCH_HR_SCHEDULER_CONFIGURATION_SHCm, ARAD_INT_SCH_SHCPARERROR,SCH_PAR_ERR_INITIATEr,SHC_PAR_ERROR_INITf,1},
    {SCH_CL_SCHEDULERS_TYPE__SCTm, ARAD_INT_SCH_SCTPARERROR,SCH_PAR_ERR_INITIATEr,SCT_PAR_ERROR_INITf,1},
    {SCH_MEM_04700000m, ARAD_INT_SCH_CSDTPARERROR,SCH_PAR_ERR_INITIATEr,CSDT_PAR_ERROR_INITf,1},
    {SCH_DUAL_SHAPER_MEMORY__DSMm, ARAD_INT_SCH_DSMPARERROR,SCH_PAR_ERR_INITIATEr,DSM_PAR_ERROR_INITf,1},
    {SCH_TOKEN_MEMORY_CONTROLLER__TMCm, ARAD_INT_SCH_TMCPARERROR,SCH_PAR_ERR_INITIATEr,TMC_LSB_PAR_ERROR_INITf,1},
    {SCH_CL_SCHEDULERS_TYPE_SCTm, ARAD_INT_SCH_SCTPARERROR,SCH_PAR_ERR_INITIATEr,SCT_PAR_ERROR_INITf,1},
    {SCH_MEM_03300000m, ARAD_INT_SCH_FLTCLECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLTCL_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03300000m, ARAD_INT_SCH_FLTCLECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLTCL_2B_ECC_ERROR_INITf,2},
    {SCH_PIR_SHAPERS_STATIC_TABEL__PSSTm, ARAD_INT_SCH_PSSTPARERROR,SCH_PAR_ERR_INITIATEr,PSST_PAR_ERROR_INITf,1},
    {SCH_MEM_03000000m, ARAD_INT_SCH_FLHHRECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLHHR_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03000000m, ARAD_INT_SCH_FLHHRECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLHHR_2B_ECC_ERROR_INITf,2},
    {SCH_CIR_SHAPERS_STATIC_TABEL__CSSTm, ARAD_INT_SCH_CSSTPARERROR,SCH_PAR_ERR_INITIATEr,CSST_PAR_ERROR_INITf,1},
    {SCH_MEM_01300000m, ARAD_INT_SCH_FDMDECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FDMD_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_01300000m, ARAD_INT_SCH_FDMDECCERROR,SCH_ECC_ERR_2B_INITIATEr,FDMD_2B_ECC_ERROR_INITf,2},
    {SCH_DEVICE_RATE_MEMORY__DRMm, ARAD_INT_SCH_DRMPARERROR,SCH_PAR_ERR_INITIATEr,DRM_PAR_ERROR_INITf,1},
    {SCH_FLOW_DESCRIPTOR_MEMORY_STATIC__FDMSm, ARAD_INT_SCH_FDMSPARERROR,SCH_PAR_ERR_INITIATEr,FDMS_PAR_ERROR_INITf,1},
    {SCH_FLOW_SUB_FLOW__FSFm, ARAD_INT_SCH_FSFPARERROR,SCH_PAR_ERR_INITIATEr,FSF_PAR_ERROR_INITf,1},
    {SCH_MEM_01700000m, ARAD_INT_SCH_DCDECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,DCD_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_01700000m, ARAD_INT_SCH_DCDECCERROR,SCH_ECC_ERR_2B_INITIATEr,DCD_2B_ECC_ERROR_INITf,2},
    {SCH_MEM_01400000m, ARAD_INT_SCH_SHDDECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,SHDD_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_01400000m, ARAD_INT_SCH_SHDDECCERROR,SCH_ECC_ERR_2B_INITIATEr,SHDD_2B_ECC_ERROR_INITf,2},
    {SCH_MEM_04D00000m, ARAD_INT_SCH_PSDDPARERROR,SCH_PAR_ERR_INITIATEr,PSDD_PAR_ERROR_INITf,1},
    {SCH_MEM_01C00000m, ARAD_INT_SCH_DPNPARERROR,SCH_PAR_ERR_INITIATEr,DPN_PAR_ERROR_INITf,1},
    {SCH_PORT_SCHEDULER_WEIGHTS_PSWm, ARAD_INT_SCH_PSWPARERROR,SCH_PAR_ERR_INITIATEr,PSW_PAR_ERROR_INITf,1},
    {SCH_CL_SCHEDULERS_CONFIGURATION_SCCm, ARAD_INT_SCH_SCCPARERROR,SCH_PAR_ERR_INITIATEr,SCC_PAR_ERROR_INITf,1},
    {SCH_MEM_03400000m, ARAD_INT_SCH_FLHFQECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLHFQ_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03400000m, ARAD_INT_SCH_FLHFQECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLHFQ_2B_ECC_ERROR_INITf,2},
    {SCH_MEM_03100000m, ARAD_INT_SCH_FLTHRECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLTHR_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03100000m, ARAD_INT_SCH_FLTHRECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLTHR_2B_ECC_ERROR_INITf,2},
    {SCH_FLOW_TO_FIP_MAPPING_FFMm, ARAD_INT_SCH_FFMPARERROR,SCH_PAR_ERR_INITIATEr,FFM_PAR_ERROR_INITf,1},
    {SCH_SHARED_DEVICE_RATE_SHARED_DRMm, ARAD_INT_SCH_DRMPARERROR,SCH_PAR_ERR_INITIATEr,DRM_PAR_ERROR_INITf,1},
    {SCH_FLOW_GROUP_MEMORY_FGMm, ARAD_INT_SCH_FGMPARERROR,SCH_PAR_ERR_INITIATEr,FGM_PAR_ERROR_INITf,1},
    {SCH_MEM_03500000m, ARAD_INT_SCH_FLTFQECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLTFQ_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03500000m, ARAD_INT_SCH_FLTFQECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLTFQ_2B_ECC_ERROR_INITf,2},
    {SCH_FLOW_TO_FIP_MAPPING__FFMm, ARAD_INT_SCH_FFMPARERROR,SCH_PAR_ERR_INITIATEr,FFM_PAR_ERROR_INITf,1},
    {SCH_CL_SCHEDULERS_CONFIGURATION__SCCm, ARAD_INT_SCH_SCCPARERROR,SCH_PAR_ERR_INITIATEr,SCC_PAR_ERROR_INITf,1},
    {SCH_PORT_SCHEDULER_WEIGHTS__PSWm, ARAD_INT_SCH_PSWPARERROR,SCH_PAR_ERR_INITIATEr,PSW_PAR_ERROR_INITf,1},
    {SCH_DUAL_SHAPER_MEMORY_DSMm, ARAD_INT_SCH_DSMPARERROR,SCH_PAR_ERR_INITIATEr,DSM_PAR_ERROR_INITf,1},
    {SCH_FLOW_GROUP_MEMORY__FGMm, ARAD_INT_SCH_FGMPARERROR,SCH_PAR_ERR_INITIATEr,FGM_PAR_ERROR_INITf,1},
    {SCH_FLOW_TO_QUEUE_MAPPING__FQMm, ARAD_INT_SCH_FQMPARERROR,SCH_PAR_ERR_INITIATEr,FQM_PAR_ERROR_INITf,1},
    {SCH_FLOW_STATUS_MEMORY__FSMm, ARAD_INT_SCH_FSMECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FSM_PORT_A_1B_ECC_ERROR_INITf,2},
    {SCH_FLOW_STATUS_MEMORY__FSMm, ARAD_INT_SCH_FSMECCERROR,SCH_ECC_ERR_2B_INITIATEr,FSM_PORT_A_2B_ECC_ERROR_INITf,2},
    {SCH_HR_SCHEDULER_CONFIGURATION__SHCm, ARAD_INT_SCH_SHCPARERROR,SCH_PAR_ERR_INITIATEr,SHC_PAR_ERROR_INITf,1},
    {SCH_FLOW_TO_QUEUE_MAPPING_FQMm, ARAD_INT_SCH_FQMPARERROR,SCH_PAR_ERR_INITIATEr,FQM_PAR_ERROR_INITf,1},
    {SCH_MEM_03200000m, ARAD_INT_SCH_FLHCLECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,FLHCL_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_03200000m, ARAD_INT_SCH_FLHCLECCERROR,SCH_ECC_ERR_2B_INITIATEr,FLHCL_2B_ECC_ERROR_INITf,2},
    {SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, ARAD_INT_SCH_PSSTPARERROR,SCH_PAR_ERR_INITIATEr,PSST_PAR_ERROR_INITf,1},
    {SCH_MEM_01600000m, ARAD_INT_SCH_DHDECCERRORFIXED,SCH_ECC_ERR_1B_INITIATEr,DHD_1B_ECC_ERROR_INITf,2},
    {SCH_MEM_01600000m, ARAD_INT_SCH_DHDECCERROR,SCH_ECC_ERR_2B_INITIATEr,DHD_2B_ECC_ERROR_INITf,2},
    {SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm, ARAD_INT_SCH_FDMSPARERROR,SCH_PAR_ERR_INITIATEr,FDMS_PAR_ERROR_INITf,1},
    {SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, ARAD_INT_SCH_CSSTPARERROR,SCH_PAR_ERR_INITIATEr,CSST_PAR_ERROR_INITf,1},
    {SCH_FLOW_SUB_FLOW_FSFm, ARAD_INT_SCH_FSFPARERROR,SCH_PAR_ERR_INITIATEr,FSF_PAR_ERROR_INITf,1},
    {SCH_MEM_04A00000m, ARAD_INT_SCH_PSDTPARERROR,SCH_PAR_ERR_INITIATEr,PSDT_PAR_ERROR_INITf,1},
    {INVALIDm}
};

static ser_test_params_t *ser_parameters[SOC_MAX_NUM_DEVICES];
static dcmn_block_control_info_t ser_test_blocks_control_info[SOC_MAX_NUM_DEVICES][NOF_BCM_BLOCKS];
void update_mem_int(int unit)
{
    ser_test_params_t *ser_test_params = ser_parameters[unit];
    soc_block_type_t block;
    soc_mem_t mem = ser_test_params->mem;

    block = dcmn_blktype_to_index(SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem)));
;
    ser_test_params->interrupt = ser_test_blocks_control_info[unit][block].parity_int;
    if (SOC_MEM_FIELD_VALID(unit,mem,ECCf)) {
         ser_test_params->interrupt = ser_test_params->error ==ECC1  ? ser_test_blocks_control_info[unit][block].ecc1_int : ser_test_blocks_control_info[unit][block].ecc2_int;
    }


}

/*
 * Function:
 *      ser_test_get_block_num_unsafe
 * Purpose:
 *      finds a given memory block number after providing its copy number
 * Parameters:
 *      unit    - Device Number
 *      mem     - Memory to find its block number 
 *      copyno  - block copy number to look for
 * Returns:
 *      on success - block num
 *      on failure - -1
 */
int ser_test_get_block_num_unsafe(int unit, soc_mem_t mem, int copyno)
{
    int block_iter, min_block, max_block;

    /* Define range of blocks to iterate over */
    min_block = SOC_MEM_INFO(unit, mem).minblock;
    max_block = SOC_MEM_INFO(unit, mem).maxblock;

    /* Iterate over range and look for relevant copyno, and return the actual block number */
    for (block_iter = min_block; block_iter <= max_block; ++block_iter) 
    {
        if (SOC_BLOCK_INFO(unit, block_iter).number == copyno) 
        {
            return block_iter ;
        }
    }

    /* relevant copy of blk was not found */
    return -1;
}

char tr153_test_usage[] = 
"TR 153 memory ser test:\n"
" \n"
  "Memory=<value>                  -  Specifies a memory name to run test on - default is \"\"\n" 
  "Index=<value>                   -  Specifies index in memory to run test on - default is min index\n" 
  "ArrayIndex=<value>              -  Specifies array index in memory to run test on - default is min array index\n" 
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
#else
  
  "CopyNo=<file name>              -  Specifies copy number of memory (for memory which exists in several blocks) - default is 0 \n" 
  "CacheState=<1/0>                -  Specifies if need to validate the Ser fix from the cache or that the interrupt appearance is sufficant - default is 0 \n"
  "ErrorType=<1/0>                 -  Specifies if Error type is <0> ECC 1bit or <1> ECC 2bit - if memory is protected by PARITY, this argument is ignored - default is 0\n"
  "RunAll=<1/0>                    -  Specifies if the test run on specific memory<0> Or perform for each memory<1> \n"
  "StartFrom=memName                    -  Specifies mem name to start from  relevant only if RunAll set \n"
  "MemCount=id                    -   Specifies how many memories to run on \n"
  "Help=<1/0>                      -  Specifies if tr 153 help is on and exit or off - default is off\n"
  "\n"
;
#endif



/*
 * Function:
 *      memory_ser_test_init
 * Purpose:
 *      takes care of all of init process for the ser test, get arguments and make a perliminery sanity check for given args
 * Parameters:
 *      unit    - Device Number
 *      a       - count of arguments 
 *      p       - actual arguments
 * Returns:
 *      BCM_E_XXX
 */
extern int memory_ser_test_init(int unit, args_t *a, void **p)
{
    ser_test_params_t *ser_test_params = NULL;
    int num_soc_mem = NUM_SOC_MEM;
    parse_table_t parse_table;
    int rv = BCM_E_INIT;
    dcmn_block_control_info_t config;
    char *start_from;

    /** allocate memory for DB for test parameters **/
    ser_test_params = ser_parameters[unit];
    if (ser_test_params == NULL) 
    {
        ser_test_params = sal_alloc(sizeof(ser_test_params_t), "ser_test");
        if (ser_test_params == NULL) 
        {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s: cannot allocate memory test data\n"), ARG_CMD(a)));
            return -1;
        }
        sal_memset(ser_test_params, 0, sizeof(ser_test_params_t));
        ser_parameters[unit] = ser_test_params;
    }
    ser_test_params->start_from = 0;
    ser_test_params->count = NUM_SOC_MEM;

    /** seting default values **/
    parse_table_init(unit, &parse_table);
    parse_table_add(&parse_table,  "Memory",        PQ_STRING, "",       &(ser_test_params->mem_name_parse),    NULL);
    parse_table_add(&parse_table,  "Index",         PQ_STRING, "min",    &(ser_test_params->index_parse),       NULL);
    parse_table_add(&parse_table,  "ArrayIndex",    PQ_STRING, "min",    &(ser_test_params->array_index_parse), NULL);
    parse_table_add(&parse_table,  "CopyNo",        PQ_INT,    0,        &(ser_test_params->copyno),            NULL);
    parse_table_add(&parse_table,  "CacheState",    PQ_BOOL,   0,        &(ser_test_params->cache_state),       NULL);
    parse_table_add(&parse_table,  "ErrorType",     PQ_INT,    0,        &(ser_test_params->error),             NULL);
    parse_table_add(&parse_table,  "StartFrom",     PQ_STRING, "",       &(start_from),                         NULL);
    parse_table_add(&parse_table,  "MemCount",      PQ_DFL|PQ_INT,  &num_soc_mem,        &(ser_test_params->count),             NULL);
    parse_table_add(&parse_table,  "RunAll",        PQ_INT8,    0,        &(ser_test_params->run_all),           NULL);
    parse_table_add(&parse_table,  "Help",          PQ_INT,    0,        &(ser_test_params->help),              NULL);

    ser_test_params->unit = unit;

    /** print usage and exit **/ 
    if (ser_test_params->help) 
    {
        /* print usage */
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s"), tr153_test_usage));
        goto done;
    }

    config.gmo_reg = SOC_IS_FE1600(unit) ? ECI_GLOBAL_1r : (SOC_IS_ARADPLUS_AND_BELOW(unit) ? ECI_GLOBALFr : ECI_GLOBAL_MEM_OPTIONSr);

    dcmn_collect_blocks_control_info(unit,ser_test_blocks_control_info[unit],&config);
    /** parsing arguments and checking seting needed values, checking validity of given options **/
    if (parse_arg_eq(a, &parse_table) < 0) 
    {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s: Invalid option: %s\n"), ARG_CMD(a), ARG_CUR(a)));
        /* print usage */
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s"), tr153_test_usage));
        goto done;
    }

    /** making sure no extra options were given **/ 
    if (ARG_CNT(a) != 0) 
    {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s: extra options starting with \"%s\"\n"), ARG_CMD(a), ARG_CUR(a)));
        /* print usage */
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"%s"), tr153_test_usage));
        goto done;
    }

    if (!ser_test_params->run_all) {
        /** validating arguments values **/
        /* validate memory name and store mem enumerator in ser_test_params->mem */
        if (parse_memory_name(unit, &(ser_test_params->mem), ser_test_params->mem_name_parse, NULL, NULL) < 0) 
        {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"Memory \"%s\" is invalid\n"), ser_test_params->mem_name_parse));
            goto done;
        }
        if (!SOC_MEM_FIELD_VALID(unit,ser_test_params->mem,ECCf) &&
            !SOC_MEM_FIELD_VALID(unit,ser_test_params->mem,PARITYf)) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"Memory \"%s\" must contain PARITY or ECC fields\n"), ser_test_params->mem_name_parse));
            goto done;
        }

        if (soc_mem_is_readonly(unit, ser_test_params->mem)) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"Memory \"%s\" is readonly register\n"), ser_test_params->mem_name_parse));
            goto done;
        }
        /* validate memory index and store index in ser_test_params->index */
        ser_test_params->index = parse_memory_index(unit, ser_test_params->mem, ser_test_params->index_parse);
        if ( (ser_test_params->index < parse_memory_index(unit, ser_test_params->mem, "min")) || 
             (ser_test_params->index > parse_memory_index(unit, ser_test_params->mem, "max"))    ) 
        {

            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"index %d is invalid for memory \"%s\"\n"), ser_test_params->index, ser_test_params->mem_name_parse));
            goto done;
        }

        /* validate memory array index and store array index in ser_test_params->array_index */
        ser_test_params->array_index = parse_memory_array_index(unit, ser_test_params->mem, ser_test_params->array_index_parse);
        if ( (ser_test_params->array_index < parse_memory_array_index(unit, ser_test_params->mem, "min")) || 
             (ser_test_params->array_index > parse_memory_array_index(unit, ser_test_params->mem, "max"))    ) 
        {

            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"array index %d is invalid for memory \"%s\"\n"), ser_test_params->index, ser_test_params->mem_name_parse));
            goto done;
        }

        /* validate memory copy number */
        if (!SOC_MEM_BLOCK_VALID(unit, ser_test_params->mem, ser_test_params->copyno)) 
        {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"Copy Number %d is invalid for memory \"%s\"\n"), ser_test_params->copyno, ser_test_params->mem_name_parse));
            goto done;
        }


    } else {
        if (sal_strlen(start_from)) {
            if (parse_memory_name(unit, &(ser_test_params->start_from), start_from, NULL, NULL) < 0) 
            {
                LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"Memory \"%s\" is invalid\n"), start_from));
                goto done;
            }
        } else{
            ser_test_params->start_from=0;
        }
    }
    /* validate error type is acceptable */
    if ((ser_test_params->error != ECC1) && (ser_test_params->error != ECC2)) 
    {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"error type number %d is invalid\n"), ser_test_params->error ));
        goto done;
    }
    LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit,"choosing Interrupt number %d \n"), ser_test_params->interrupt ));
    /** Turn off other Threads **/
    bcm_common_linkscan_enable_set(unit,0);
    soc_counter_stop(unit);
#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit)) {
        rv = bcm_dpp_counter_bg_enable_set(unit, FALSE);
        if (BCM_E_NONE == rv)
        {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"counter processor background accesses suspended\n")));
        }
        else
        {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"counter processor background access suspend failed: %d (%s)\n"),rv, _SHR_ERRMSG(rv)));
        }
    }
#endif /* BCM_PETRA_SUPPORT */

    rv = BCM_E_NONE;

done:
    parse_arg_eq_done(&parse_table);
    if (rv != BCM_E_NONE) 
    {
        test_error(unit, "There was a problem with parameters, they were not entered correctly\n");
    }
    return rv; 
}

static  soc_field_t fields[] = {ECCf,PARITYf,NUM_SOC_FIELD};


static int mem_is_ff_or_regarray(int unit, soc_mem_t mem)
{

    /*  flipflops and regarrays doesnt generate ser interrupts*/

    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        switch (mem) {
            case IRE_NIF_PORT_TO_CTXT_BIT_MAPm:
            case IRR_SNOOP_MIRROR_TABLE_1m:
            case IRR_SNOOP_MIRROR_TABLE_0m:
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip on FF Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
                return 1;
        }
    }

    return 0;


}

STATIC
arad_sch_ser_map_t  *get_sch_ser_map_entry(soc_mem_t mem,uint32 nof_errors)
{

    int i;

    for (i=0;arad_sch_ser_map[i].mem!=INVALIDm;i++) {
        if (arad_sch_ser_map[i].mem == mem &&
            arad_sch_ser_map[i].nof_errors==nof_errors) {
            return &(arad_sch_ser_map[i]);
        }
    }
    return NULL;
}


/*
 * Function:
 *      memory_ser_test_run
 * Purpose:
 *      run the test, ser test should simulate ser error and check its handling
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC
int memory_ser_test_run_mem(int unit, args_t *a, void *p)
{
    uint32 mem_buff[SOC_MAX_MEM_WORDS], mask[SOC_MAX_MEM_WORDS], mem_buff_restored_from_cache[SOC_MAX_MEM_WORDS],orig_mem_buff[SOC_MAX_MEM_WORDS];
    uint32 interrupt_count, new_interrupt_count, sleep_for_a_bit = 2000000;
    int rv = BCM_E_NONE, test_failed = 0, entry_length;
    int nof_needed_bits, nof_bits_found = 0, random_bit, attampt_counter, toggle_counter, block_num;
    int bits_found[] = {-1, -1};
    ser_test_params_t *ser_test_params = NULL;
    soc_mem_t mem = 0;
    bcm_switch_event_control_t get_interrupt_count;
    uint8 succeed=0;
    uint32 i;
    soc_block_t block ;
    soc_reg_t   gmo_reg ;
    bcm_switch_event_control_t type;
    uint32 int_mask;
    uint32 copyno=0;
    arad_sch_ser_map_t  *sch_ser_map_entry = NULL;

    SOC_INIT_FUNC_DEFS;

    /** Check Validity of Test Parameters **/ 
    ser_test_params = ser_parameters[unit];
    if (ser_test_params == NULL) {
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Invalid test params\n")));
        test_failed = 1;
        rv = BCM_E_UNAVAIL;
        goto done;
    }


    mem = ser_test_params->mem;
    /** Print Test Parameters - Temp chunk, need to be removed once test is completed **/
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Recieved Arguments are:\n" 
                                                  "Memory name : %s\t"             "Memory num  : %d\t"          "Index       : %d\t"        "Array Index : %d\n"
                                                  "CopyNo      : %d\t"             "Cache State : %d\t"          "Interrupt   : %d\t"        "Error Type  : %d\n"),
                                         SOC_MEM_NAME(unit, ser_test_params->mem), ser_test_params->mem,         ser_test_params->index,     ser_test_params->array_index,
                                                  ser_test_params->copyno,         ser_test_params->cache_state, ser_test_params->interrupt, ser_test_params->error));


    nof_needed_bits = ser_test_params->error == ECC2 && SOC_MEM_FIELD_VALID(unit,mem,ECCf) ? 2 : 1;
    block = dcmn_blktype_to_index(SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem)));
    if (SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))==SOC_BLK_SCH&&
        SOC_IS_ARADPLUS_AND_BELOW(unit)) {
         sch_ser_map_entry = get_sch_ser_map_entry(mem,nof_needed_bits);
        
        if (sch_ser_map_entry==NULL) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s not connect to an interrupt\n"), SOC_MEM_NAME(unit, mem)));
            test_failed = 1;
            rv = BCM_E_PARAM;
            goto done;
        }
        type.event_id = sch_ser_map_entry->interrupt; 
        get_interrupt_count.event_id =type.event_id;
    } else {
        type.event_id = ser_test_params->interrupt; 
        get_interrupt_count.event_id = ser_test_params->interrupt;
    }
    type.action=bcmSwitchEventMask;
    type.index=0;
    bcm_switch_event_control_get(unit,BCM_SWITCH_EVENT_DEVICE_INTERRUPT,type,&int_mask);
    if (int_mask) {

        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Interrupt %d Masked sow skipping the test for Mem:%s\n"), ser_test_params->interrupt,SOC_MEM_NAME(unit, ser_test_params->mem))); 
        test_failed = 1;
        rv = BCM_E_PARAM;
        goto done;
    }

    /** Get Parameters and Preperations **/
    sal_memset(mask, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    sal_memset(mem_buff, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    sal_memset(mem_buff_restored_from_cache, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    copyno = ser_test_params->copyno;

    gmo_reg = ser_test_blocks_control_info[unit][block].gmo_reg;

    if (!SOC_REG_IS_VALID(unit,gmo_reg) || SOC_REG_ARRAY(unit,gmo_reg) || !SOC_REG_FIELD_VALID(unit,gmo_reg,CPU_BYPASS_ECC_PARf)) {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s block doesnt have general configuration register\n"), SOC_MEM_NAME(unit, mem)));
        test_failed = 1;
        rv = BCM_E_PARAM;
        goto done;
    }
    block_num = ser_test_get_block_num_unsafe(unit, mem, ser_test_params->copyno);
    get_interrupt_count.index = ser_test_params->copyno;


    /* start !sch*/

    if (sch_ser_map_entry==NULL) {
        sal_srand(sal_time());

        /** Check Memory is not W/O or R/O **/
        if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_WRITEONLY) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s Write Only memory - Invalid memory\n"), SOC_MEM_NAME(unit, mem)));
            test_failed = 1;
            rv = BCM_E_UNAVAIL;
            goto done;
        }

        if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_READONLY) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s Read Only memory - Invalid memory\n"), SOC_MEM_NAME(unit, mem)));
            test_failed = 1;
            rv = BCM_E_UNAVAIL;
            goto done;
        }

        /** get given interrupt's count **/
        get_interrupt_count.action = bcmSwitchEventStat;

        /** Get Bit/s to Toggle **/
        entry_length = soc_mem_entry_bits(unit, mem);
        if (entry_length == 0) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s entry size is 0, invalid size\n"), SOC_MEM_NAME(unit, mem)));
            test_failed = 1;
            rv = BCM_E_FAIL;
            goto done;
        }
        soc_mem_datamask_rw_get(unit, mem, mask); 
        /* get random bit, check if not r/o or w/o, and save it, repeat untill got all needed bits, or timed out. */
        for ( attampt_counter = 0; (attampt_counter < MAX_NOF_ATTAMPTS_TO_GET_A_RW_BIT) && (nof_needed_bits > nof_bits_found); ++attampt_counter) {
            random_bit = sal_rand() % entry_length ;
            /* check if bit is r/w */
            if ( mask[random_bit / BIT_IN_UINT32] & (1u << random_bit % BIT_IN_UINT32) ) {
                /* Save found Bit */
                bits_found[nof_bits_found++] = random_bit;
                /* mark bit as not r/w - inorder not to get the same one if more then 1 is needed */
                mask[random_bit / BIT_IN_UINT32] ^= (1u << random_bit % BIT_IN_UINT32);
            } else {
                LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "bit %d/%d"               "of Memory %s is not r/w bit and is being skipped, skip count = %d\n"), 
                                                          random_bit, entry_length - 1, SOC_MEM_NAME(unit, mem),                           attampt_counter ));
            }
        }

        /** check if loop timed-out **/
        if (nof_needed_bits != nof_bits_found) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "attampt to get bits to change in Memory %s timed-out\n"), SOC_MEM_NAME(unit, mem)));
            test_failed = 1;
            rv = BCM_E_TIMEOUT;
            goto done;
        }


        /** Change Entry not in cache **/
        /* Read entry */
        bcm_switch_event_control_get(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, get_interrupt_count, &interrupt_count);

        _SOC_IF_ERR_EXIT(soc_mem_read_no_cache(unit, SOC_MEM_NO_FLAGS, mem,
                                               ser_test_params->array_index,
                                               block_num, ser_test_params->index,
                                               mem_buff));

        /* change requiered bits */
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "change bits at pos:")));
        sal_memcpy(orig_mem_buff,mem_buff,sizeof(orig_mem_buff));
        for (toggle_counter = 0; toggle_counter < nof_bits_found; ++toggle_counter) {
            LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "(%d) "), bits_found[toggle_counter]));
            mem_buff[bits_found[toggle_counter]/BIT_IN_UINT32] ^= (1u << bits_found[toggle_counter]%BIT_IN_UINT32);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "\n")));

        /** Bypass PARITY and ECC **/
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, gmo_reg, copyno, CPU_BYPASS_ECC_PARf, 0x1));

        /** Write modified entry **/ 
        _SOC_IF_ERR_EXIT(soc_mem_array_write_extended(unit, SOC_MEM_DONT_USE_CACHE, mem, ser_test_params->array_index, block_num, ser_test_params->index, mem_buff));

        /** Re-activate PARITY and ECC and Read memory --> Should Trigger the Interrupt **/
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, gmo_reg, copyno, CPU_BYPASS_ECC_PARf, 0x0)); 
        _SOC_IF_ERR_EXIT(soc_mem_read_no_cache(unit, SOC_MEM_NO_FLAGS, mem,
                                               ser_test_params->array_index,
                                               block_num, ser_test_params->index,
                                               mem_buff_restored_from_cache));
         sal_usleep(sleep_for_a_bit);


         /* end !sch*/
    } else {
        /**
         *  sequence by ashraf  
         *  ser sequence for creating ser in sch memory
         *  BCM.0>  s SCH_ERROR_INITIATION_DATA 1
         *  BCM.0>  m SCH_PAR_ERR_INITIATE FDMS_PAR_ERROR_INIT=0
         *  BCM.0>  m SCH_PAR_ERR_INITIATE FDMS_PAR_ERROR_INIT=1
         *  BCM.0> d disable_cache SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMS 0 1
         *  
         */

        get_interrupt_count.action = bcmSwitchEventStat;
        bcm_switch_event_control_get(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, get_interrupt_count, &interrupt_count);

        _SOC_IF_ERR_EXIT(soc_mem_read_no_cache(unit, SOC_MEM_NO_FLAGS, mem,
                                               ser_test_params->array_index,
                                               block_num, ser_test_params->index,
                                               mem_buff));

        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, SCH_ERROR_INITIATION_DATAr, REG_PORT_ANY,
                                                 ERR_WRf, 0x1));

        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, sch_ser_map_entry->error_initiate_reg, REG_PORT_ANY,
                                                 sch_ser_map_entry->error_initiate_field, 0x0));

        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, sch_ser_map_entry->error_initiate_reg, REG_PORT_ANY,
                                                 sch_ser_map_entry->error_initiate_field, 0x1));
        _SOC_IF_ERR_EXIT(soc_mem_read_no_cache(unit, SOC_MEM_NO_FLAGS, mem,
                                               ser_test_params->array_index,
                                               block_num, ser_test_params->index,
                                               mem_buff_restored_from_cache));



    }

    /** Check again given interrupt's count **/
    bcm_switch_event_control_get(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, get_interrupt_count, &new_interrupt_count);
    succeed=0;
    for (i=0;i<MAX_SER_RETRIES;i++) {
        if (new_interrupt_count <= interrupt_count) {
            /* if the counter test failed lets try read test maybe the nmemory already fixed*/
                sal_usleep(sleep_for_a_bit);
                cli_out("retry reading counter  %d\n",i);
                bcm_switch_event_control_get(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, get_interrupt_count, &new_interrupt_count);
                continue;
        }
        succeed=1;
        break;

        }    

    if (!succeed) {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Memory %s Interrupt doesnt trigger(counter=%d) check to see if the interrupt number %d supplied  correct\n"), SOC_MEM_NAME(unit, mem), new_interrupt_count, ser_test_params->interrupt));
        test_failed = 1;
        rv = BCM_E_FAIL;
        goto done;
    }
    /**
     * we not perform comparisson in the following situation 
     * because we cant fix the memory 
     */
    if ( 
        soc_mem_is_writeonly(unit, mem) ||
        dcmn_tbl_is_dynamic(unit,mem) ||
        (SOC_MEM_FIELD_VALID(unit,mem,ECCf) && soc_mem_field_length(unit,mem,ECCf)<=1)||
        (SOC_MEM_FIELD_VALID(unit,mem,PARITYf) && soc_mem_field_length(unit,mem,PARITYf)!=1)||
        (!soc_mem_cache_get(unit,mem,SOC_BLOCK_ALL) && (!SOC_MEM_FIELD_VALID(unit,mem,ECCf) || 
        (SOC_MEM_ECC_TYPE(unit,mem) != SOC_MEM_ECC_TYPE_NORMAL && SOC_MEM_ECC_TYPE(unit,mem) != SOC_MEM_ECC_TYPE_WIDE) ||
                                                        SOC_MEM_TYPE(unit,mem) == SOC_MEM_TYPE_XOR)) ||
        !(ser_test_params->error == ECC1  && 
        (SOC_MEM_FIELD_VALID(unit, mem, ECCf)  || 
         (SOC_MEM_FIELD_VALID(unit, mem, PARITYf) && soc_mem_cache_get(unit, mem, copyno))))) {
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip Comparing Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
        goto done;
    }


    /*  we expected from ecc protected memories  or  cached parity memories
     *   to be fix by the interrupt handler
     */
        /* in case of ecc1 test if our correction by shadow or by ecc1 correction succeed. we compare the corrected data with the original data*/
    _SOC_IF_ERR_EXIT(soc_mem_read_no_cache(unit, SOC_MEM_NO_FLAGS, mem,
                                           ser_test_params->array_index,
                                           block_num, ser_test_params->index,
                                           mem_buff_restored_from_cache));
    if (sal_memcmp(mem_buff_restored_from_cache, orig_mem_buff, sizeof(orig_mem_buff))) {
        LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Warning:Memory %s value doesnt corrected to it original value \n"), SOC_MEM_NAME(unit, mem)));
        test_failed = 1;
        rv = BCM_E_FAIL;
    }





done:

    if (test_failed) {
        test_error(unit, "Memory Ser Test Failed!\n");
    }
    return rv;

exit:
    test_error(unit, "Memory Ser Test Failed!\n");
    SOC_FUNC_RETURN;
}





STATIC
int mem_ser_test_cb(int unit, soc_mem_t mem,void *p)
{
    bcm_error_t rv = BCM_E_NONE;
    ser_test_params_t *ser_test_params = ser_parameters[unit];
    uint32 min_index,max_index;
    uint64 index_addition;
    uint64 sal_rand_max = COMPILER_64_INIT(0, SAL_RAND_MAX);
    int instance;
 


    

    /* we skip on memory if it not in reuiered range*/
    if ( 
         mem < ser_test_params->start_from ||
        mem > ser_test_params->start_from  + ser_test_params->count ) {
        ser_test_params->skipped_counter++;       
        return rv;
    }

    /* we skip on memory if it doesnt contain ECC  and is not cacheble*/
    if ( 
        !dcmn_mem_contain_one_of_the_fields(unit,mem,fields) ||
        mem_is_ff_or_regarray(unit,mem) ||
        soc_mem_is_readonly(unit, mem) ||
        sal_strstr(SOC_MEM_NAME(unit,mem), "BRDC_") != NULL  ||
        SOC_MEM_IS_VALID(unit, SOC_MEM_ALIAS_MAP(unit, mem)) ||
        SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))==SOC_BLK_IPROC ||
        SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))==SOC_BLK_CMIC ||
        SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))==SOC_BLK_FF  ) {
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
        ser_test_params->skipped_counter++;       
        return rv;
    }



    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        switch (mem) {
        /** 
         *  block at arad_ser_init
         *  due to false positive interrupts
         */ 
        case IRR_SNOOP_MIRROR_DEST_TABLEm: 
        case IRE_CTXT_MEM_CONTROLm:
        case IRE_NIF_PORT_TO_CTXT_BIT_MAPm:
        case IRR_SNOOP_MIRROR_TABLE_0m:
        case IRR_SNOOP_MIRROR_TABLE_1m:
        case NBI_RLENG_MEMm:
        case NBI_TLENG_MEMm:
        case NBI_RTYPE_MEMm:
        case NBI_TTYPE_MEMm:
        case NBI_RPKTS_MEMm:
        case NBI_TPKTS_MEMm:
 /**
  * IPT - EGQDATA has ECC only on the upper 24 MSB's. make sure 
  * you generate the error in one of those bits : 511:488 
  * Yael Konforty  
  */
        case IPT_EGQDATAm:

      /** 
       * the IPS_QDESC is not accesible by cpu 
       * while the  IPS_QDESC_TABLE divided into 2 parts 
       * one protected by parity and two by ecc 
       * Yael Konforty  
       * */ 
        case IPS_QDESCm:
        case IPS_QDESC_TABLEm:

            /* 
             * the following are alias with ECC instead of parity 
             * should be removed from here after fix at SDK-89304 
             */
        case PPDB_B_LARGE_EM_FID_COUNTER_DBm:
        case PPDB_B_LARGE_EM_FID_PROFILE_DBm:
        case PPDB_B_LARGE_EM_PORT_MINE_TABLE_LAG_PORTm:
        case PPDB_B_LARGE_EM_PORT_MINE_TABLE_PHYSICAL_PORTm:

            /*
             * Regarding mrps,
             * MRPS_MCDA_PRFCFG_SHARING_DIS
             *MRPS_MCDA_PRFCFG_SHARING_EN
             * Are not memories. They are descriptions of 2    
             *possibilities for configuring McdaPrfcfg memory.              
             *Inbal
             */
        case MRPS_MCDA_PRFCFG_SHARING_DISm:
        case MRPS_MCDA_PRFCFG_SHARING_ENm:



            /**
             * cant have ser unless traffic(ariels)
             */
        case IQM_PDMm:
        case IQM_BDBLLm:
        case IQM_PQDMDm:
            /**
             * dont have initiate doesnt work(ariels)
             */
        case IDR_MCDA_PRFCFG_0m:
        case IDR_MCDB_PRFCFG_0m:

            /**
             * dont have cpu write support (ariels)
             */
        case IQM_VSQ_D_MX_OCm:
        case IQM_VSQ_F_MX_OCm:



            /**
             * recommended by ashraf to skip this memories 
             *  SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CAL
             *  divided into regions each region can get different interrupt
             *  
             *  SCH_SCHEDULER_ENABLE_MEMORY_SEM
             *  represent 2 memories
             *  you cant expect which region get the intterupt
             * 
             */
        case SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm:
        case SCH_SCHEDULER_ENABLE_MEMORY_SEMm:




            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
            ser_test_params->skipped_counter++;       
            return rv;
        }
/*     unjustified skip list*/
        switch (mem) {


            /**
             * the list of the below crps mems produce 
             * too many ints 
             * This is expected behavior for CRPS as well. (Inbal)
             */

        case CRPS_CRPS_0_OVTH_MEMm:
                /**
                 * for sered EGQ_QP_CBM or EGQ_TCG_CBM 
                 * we must set EGQ_EGRESS_SHAPER_ENABLE_SETTING to 0 else the 
                 * desing will block the access for this memories
                 */
            case EGQ_QP_CBMm:
            case EGQ_TCG_CBMm:

                /**
                 * for sered IPS_QSZ 
                 * we must set  IPS_CREDIT_WATCHDOG_CONFIGURATION to 0 
                 * else the desing will read infinitly from IPS_QSZ 
                 * and will generate infinitly interrupts 
                 */
            case IPS_QSZm:

                /* make problem in tr 153 RunAll=1,but fix correct at tr 153 small run*/
            case PPDB_A_OEMB_STEP_TABLEm:


            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
            ser_test_params->unjustified_skipped_counter++;       
            return rv;
        }




    } else if (SOC_IS_JERICHO(unit)) {

       /*  jericho and above*/

        /*     unjustified skip list*/
        switch (mem) {
            /** 
             *  list of unjustified skipped memories
             */ 



                /**
                 * the list of the below crps mems produce 
                 * too many ints 
                 * This is expected behavior for CRPS as well. (Inbal)
                 */
            case CRPS_CRPS_13_CNTS_MEMm:
            case CRPS_CRPS_13_OVTH_MEMm:
            case CRPS_CRPS_14_CNTS_MEMm:
            case CRPS_CRPS_14_OVTH_MEMm:
            case CRPS_CRPS_15_CNTS_MEMm:
            case CRPS_CRPS_15_OVTH_MEMm:
            case CRPS_CRPS_16_CNTS_MEMm:
            case CRPS_CRPS_16_OVTH_MEMm:
            case CRPS_CRPS_17_CNTS_MEMm:
            case CRPS_CRPS_17_OVTH_MEMm:
            case CRPS_CRPS_1_CNTS_MEMm:
            case CRPS_CRPS_1_OVTH_MEMm:
            case CRPS_CRPS_2_CNTS_MEMm:
            case CRPS_CRPS_2_OVTH_MEMm:
            case CRPS_CRPS_3_CNTS_MEMm:
            case CRPS_CRPS_3_OVTH_MEMm:
            case CRPS_CRPS_4_CNTS_MEMm:
            case CRPS_CRPS_4_OVTH_MEMm:
            case CRPS_CRPS_5_CNTS_MEMm:
            case CRPS_CRPS_5_OVTH_MEMm:
            case CRPS_CRPS_6_CNTS_MEMm:
            case CRPS_CRPS_6_OVTH_MEMm:
            case CRPS_CRPS_7_CNTS_MEMm:
            case CRPS_CRPS_7_OVTH_MEMm:
            case CRPS_CRPS_8_CNTS_MEMm:
            case CRPS_CRPS_8_OVTH_MEMm:
            case CRPS_CRPS_9_CNTS_MEMm:
            case CRPS_CRPS_9_OVTH_MEMm:
            case CRPS_CRPS_10_OVTH_MEMm:
            case CRPS_CRPS_11_CNTS_MEMm:
            case CRPS_CRPS_11_OVTH_MEMm:
            case CRPS_CRPS_12_CNTS_MEMm:
            case CRPS_CRPS_12_OVTH_MEMm:
            case CRPS_CRPS_0_CNTS_MEMm:
            case CRPS_CRPS_0_OVTH_MEMm:
            case  CRPS_CRPS_10_CNTS_MEMm:


                /**
                 * All these memories behave the same as Arad.
                 * Same reason for not seeing 1b SER reaction.
                 * Ariels
                 */
            case IQM_IQM_READ_PENDING_FIFOm:
            case IQM_PAKCET_DESCRIPTOR_MEMORY_ECC_DYNAMICm:
            case IQMT_BDBLLm:
            case IQMT_PDM_0m:
            case IQMT_PDM_1m:
            case IQMT_PDM_2m:
            case IQMT_PDM_3m:
            case IQMT_PDM_4m:
            case IQMT_PDM_5m:
            case IQM_VSQD_MX_OCm:
            case IQM_VSQE_MX_OCm:
            case IQM_VSQF_MX_OCm:


                /**
                 * this mem produce 
                 * too many ints 
                 * For RTP this is expected  then 
                 * Uri 
                 */
            case RTP_UNICAST_DISTRIBUTION_MEMORYm:

            /*   the ser for this block disabled at init see soc_dcmn_ser_init_cb*/
            case ECI_MBU_MEMm:

                /**
                 * 1b/2b ECC Ignore 
                 */
            case OCB_OCB_ADDRESS_SPACEm:

            /*SKIP QAX Dynamic memories, these momeories are written failure occasionally*/
            case IEP_MCDB_DYNAMICm:
            case IEP_MCDA_DYNAMICm:
            case IMP_MCDB_DYNAMICm:
            case IMP_MCDA_DYNAMICm:

            /*SKIP JER Dynamic memories, these momeories are written failure occasionally*/
            case SCH_CIR_SHAPERS_DYNAMIC_TABEL_CSDTm:
            case SCH_PIR_SHAPERS_DYNAMIC_TABEL_PSDTm:
            case MRPS_MCDA_DYNAMICm:
            case MRPS_MCDB_DYNAMICm:
            case MTRPS_EM_MCDA_DYNAMICm:
            case MTRPS_EM_MCDB_DYNAMICm:

            
            case EDB_EEDB_BANKm:
            case EDB_EEDB_TOP_BANKm:

            /*QAX SKIP memories*/
            case OAMP_FLEX_VER_MASK_TEMPm:
            case ITE_SNP_MIRR_CMD_MAPm:
            case ITE_SNP_MIRR_SYS_HEADER_INFOm:
            case KAPS_BUCKET_MAP_MEMORYm:
            case KAPS_BUCKET_MEMORYm:
            case KAPS_RPB_ADSm:
            case CRPS_CRPS_CNTS_MEMm:
            case CRPS_CRPS_OVTH_MEMm:
            case CRPS_CRPS_PRE_READ_MEMm:
            case ECI_ZIKARON_MBUm:
            case EDB_OUTRIF_TABLEm:
            case EDB_RIF_PROFILE_TABLEm:

                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip unjustified on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
                ser_test_params->unjustified_skipped_counter++;       
                return rv;
        }
    }

    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
       /*  jericho plus only*/

        /*     unjustified skip list*/
        switch (mem) {
            /**
             *  list of unjustified skipped memories
             */
            case EDB_GLEM_STEP_TABLEm:
            case IHB_FEC_PATH_SELECTm:
            case IHP_VTT_PATH_SELECTm:
            case IHB_FIFO_PIPE_40_TO_50m:
            case KAPS_BBS_BUCKET_MEMORYm:
            case SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm:
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip unjustified on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
                ser_test_params->unjustified_skipped_counter++;
                return rv;
        }
    }

    if (SOC_IS_QUX(unit)) {
       /*  Qux */

        switch (mem) {
            
            case IMP_MCDA_PRFCFG_0m:
            case IMP_MCDA_PRFCFG_1m:
            case IMP_MCDA_PRFSELm:
            case IMP_MCDB_PRFCFG_0m:
            case IMP_MCDB_PRFCFG_1m:
            case IMP_MCDB_PRFSELm:
            case IEP_MCDA_PRFCFG_0m:
            case IEP_MCDA_PRFCFG_1m:
            case IEP_MCDA_PRFSELm:

            
            case IPSEC_SPU_WRAPPER_TOP_SPU_INPUT_FIFO_MEM_Hm:
            case IPSEC_SPU_WRAPPER_TOP_SPU_OUTPUT_FIFO_MEM_Hm:
            case OAMP_RMEP_DB_EXTm:

            

            case CGM_VOQ_VSQS_PRMSm:
            case CGM_IPPPMm:
            case IHB_FEC_ENTRYm:
            case IHB_FEC_SUPER_ENTRYm:
            case IHP_LIF_TABLEm:
            case IHP_VSI_LOW_CFG_1m:
            case OAMP_PE_PROGRAMm:
            case OAMP_PE_GEN_MEMm:
            case PPDB_A_OEMA_KEYT_PLDT_Hm:
            case PPDB_A_OEMB_KEYT_PLDT_Hm:



                 LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Skip unjustified on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
                ser_test_params->unjustified_skipped_counter++;
                return rv;
        }
    }

    ser_test_params->total_counter++;
    ser_test_params->mem = mem;
    min_index =  parse_memory_index(unit, mem, "min");
    max_index =  parse_memory_index(unit, mem, "max");
    COMPILER_64_SET(index_addition, 0, max_index - min_index);
    COMPILER_64_UMUL_32(index_addition, sal_rand());
    COMPILER_64_UDIV_64(index_addition, sal_rand_max);
    ser_test_params->index = min_index + COMPILER_64_LO(index_addition);

    min_index = parse_memory_array_index(unit, mem, "min");
    max_index = parse_memory_array_index(unit, mem, "max");
    COMPILER_64_SET(index_addition, 0, max_index - min_index);
    COMPILER_64_UMUL_32(index_addition, sal_rand());
    COMPILER_64_UDIV_64(index_addition, sal_rand_max);
    ser_test_params->array_index = min_index + COMPILER_64_LO(index_addition);

    update_mem_int(unit);

    ser_test_params->copyno = 0;
    SOC_MEM_BLOCK_ITER(unit,mem,instance){

        rv = memory_ser_test_run_mem(unit,NULL,NULL);
        if (rv != BCM_E_NONE) {
            break;
        }
        ser_test_params->copyno++; 
    }
     

    switch (rv) {
    case BCM_E_PARAM:
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"second skip list(param) on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
        ser_test_params->unjustified_skipped_counter++;  
        break;
    case BCM_E_TIMEOUT:
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"second skip list(timeout) on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
        ser_test_params->unjustified_skipped_counter++;  
        break;
    case BCM_E_UNAVAIL:
    case BCM_E_FAIL:        
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,"Found Error  on Mem:%s\n"),SOC_MEM_NAME(unit, mem)));
        ser_test_params->error_counter++; 
    default: 
        break;
    }
    return rv; 

}



/*
 * Function:
 *      memory_ser_test_run
 * Purpose:
 *      run the test, ser test should simulate ser error and check its handling
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int memory_ser_test_run(int unit, args_t *a, void *p)
{

    bcm_error_t rv = BCM_E_NONE;
    ser_test_params_t *ser_test_params = NULL;

    ser_test_params = ser_parameters[unit];

    /** enable dynamic **/
    enable_dynamic_memories_access(unit);

    ser_test_params->total_counter=0;
    ser_test_params->error_counter=0;
    ser_test_params->skipped_counter=0;
    ser_test_params->unjustified_skipped_counter=0;
    sal_srand(sal_time());
    if (!ser_test_params->run_all) {
        int instance;
        soc_mem_t mem =  ser_test_params->mem;
        update_mem_int(unit);
        ser_test_params->copyno = 0;
        SOC_MEM_BLOCK_ITER(unit,mem,instance){

            rv = memory_ser_test_run_mem(unit,NULL,NULL);
            if (rv != BCM_E_NONE) {
                break;
            }
            ser_test_params->copyno++; 
        }

        return rv;
    }
    if (soc_mem_iterate(unit, mem_ser_test_cb, &unit) < 0)
        LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "SER_MEM_TEST: unit %d  failed\n"), unit));
    LOG_INFO(BSL_LS_APPL_TESTS,(BSL_META_U(unit, "Mem run(%d) failed(%d) skipped(%d) skipped counter 2(%d) \n"), 
                                ser_test_params->total_counter,
                                ser_test_params->error_counter,
                                ser_test_params->skipped_counter,
                                ser_test_params->unjustified_skipped_counter
                                ));

    return rv;
}

/*
 * Function:
 *      memory_ser_test_done
 * Purpose:
 *      clean-up after ser test was done
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      BCM_E_XXX
 */
extern int memory_ser_test_done(int unit, void *p)
{
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Ser Tests Done\n")));
    sal_free(ser_parameters[unit]);
    ser_parameters[unit] = NULL;
    return 0;
}

#endif /* BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT*/

#undef _ERR_MSG_MODULE_NAME

