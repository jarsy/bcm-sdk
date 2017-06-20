/* $Id: tmu_taps_ut.c,v 1.74.12.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/taps/tcam.h>
#include <soc/sbx/caladan3/tmu/taps/sbucket.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>
#include <soc/sbx/caladan3/tmu/taps/work_queue.h>
#include <soc/error.h>
#include <soc/sbx/caladan3/hpcm.h>
#include <shared/alloc.h>
#include <sal/core/time.h>
#include <soc/sbx/caladan3/lrp.h>
#include <appl/test/caladan3/c3hppc_lrp.h>
#include <appl/test/caladan3/c3hppc_tmu.h>



static int tmu_taps_ut_test_result;
static int tmu_taps_ucode_loaded;
static int tmu_taps_ut_debug = 0;
/* #define _DBG_VERBOSE */


/*********************/
/*     Tests         */
/*********************/
enum TAPS_TEST_IDS
{
    TAPS_TEST_RUN                       = 0,
    TAPS_TCAM_TEST_RUN                  = 1,
    TAPS_DOMINO_TEST_RUN                = 2,
    TAPS_BASIC_TEST_RUN                 = 3,
    TAPS_CAPACITY_TEST_RUN              = 4,
    TAPS_BASIC_MULTITABLE_TEST_RUN      = 5,
    TAPS_RANDOM_CAPACITY_TEST_RUN       = 6,
    TAPS_PARALLEL_TEST_RUN              = 7,
    TAPS_IPV6_TEST_RUN                  = 8,
    TAPS_IPV6_UPDATE_AND_GET_RUN        = 9,
    TAPS_IPV6_RANDOM_ACCESS_RUN         = 10,
    TAPS_MULTIPLE_KEY_TYPE_TEST_RUN     = 11,
    TAPS_BPM_TEST_RUN                   = 12,
    TAPS_IPV4_CUSTOMIZED_TEST_RUN       = 13,
    TAPS_IPV6_CUSTOMIZED_TEST_RUN       = 14,
    TAPS_IPV4_DELETE_TEST_RUN           = 15,
    TAPS_IPV6_DELETE_TEST_RUN           = 16,
    TAPS_IPV4_UPDATE_AND_GET_TEST_RUN   = 17,
    TAPS_IPV4_RANDOM_ACCESS_TEST_RUN    = 18,
    TAPS_IPV4_TRAVERSE_TEST_RUN         = 19,
    TAPS_IPV6_TRAVERSE_TEST_RUN         = 20,
	TAPS_ERROR_HANDLING_TEST_RUN        = 21,
    TAPS_IPV4_BUCKET_REDIST_TEST_RUN    = 22,
    TAPS_IPV6_BUCKET_REDIST_TEST_RUN    = 23
};

enum TAPS_TEST_CASES
{
    TAPS_IPV6_CASE_0=0
};





/*********************/
/** EML unit tests **/
/*********************/
typedef struct _taps_table_param_s {
    taps_init_params_t param;
    taps_handle_t taps;
    taps_arg_t arg;
    unsigned int mem_begin, mem_end, mem_max;
} taps_table_param_t; 

typedef struct _taps_vrf_param_s {
    uint32 vrf;
    uint32 vrf_routes;    
} taps_vrf_param_t; 

typedef struct _taps_route_param_s {
    uint32 keys[BITS2WORDS(TAPS_MAX_KEY_SIZE)];
    uint32 key_length; 
    uint32 payload[_TAPS_PAYLOAD_WORDS_];
    /* use this key will hit the route, if all 0xFFFFFFFF, it means match key not exist
     * for example, have 2 subnets that are 1 bit longer and cover both "0" and "1" case.
     */   
    uint32 match_key[BITS2WORDS(TAPS_MAX_KEY_SIZE)];
} taps_route_param_t; 

typedef struct _taps_bucket_redistribution_clear_s {
    int unit;
    taps_handle_t taps;
    taps_wgroup_handle_t *wgroup;
    taps_tcam_pivot_handle_t tph;
    taps_spivot_handle_t sph;
    taps_route_param_t *routes;
} taps_bucket_redistribution_clear_t; 


#define _MAX_TAPS_PARAM_ (4)
#define V4_RAND() \
    (((sal_rand()&0xFF)<<24)|((sal_rand()&0xFF)<<16)|((sal_rand()&0xFF)<<8)|(sal_rand()&0xFF))

static taps_table_param_t taps_test_param[_MAX_TAPS_PARAM_];
static unsigned int defentry[3][_TAPS_PAYLOAD_WORDS_] = {{0x11223344, 0x55667788, 0, 0},{0x11223344, 0x55667788, 0, 1},
                                                        {0x11223344, 0x55667788, 0, 2}};
static unsigned int _key[TAPS_MAX_KEY_SIZE_WORDS];
static unsigned int _key_pyld[_TAPS_PAYLOAD_WORDS_] = {0xba5eba11, 0x12345678, 0xdeadbeef, 0x7fffff /*ecc-0*/};
static unsigned int _ucode_key[TAPS_MAX_KEY_SIZE_WORDS];
static unsigned int _ucode_pyld[_TAPS_PAYLOAD_WORDS_];
static void tmu_taps_make_ipv6_key(int rpb_pivot, int bb_pivot, int dram_index, uint32 * ipv6_key);
static void tmu_taps_pack_ipv6(uint32 * pfx_cache,uint64 *prefix_cache, int len);
static int taps_ut_setup_ipv6_ucode_lookup(int unit, taps_arg_t *p_t0_arg);
static void tmu_taps_ipv6_ad(int rpb_pivot, int bb_pivot,int dram_pfx, uint32 *p_adata);
int taps_ut_verify_ipv6_ucode_lookup(int unit, taps_arg_t *arg);


#define INVALID_CASE 255
#define _VRF_BASE_ 0x1234
static int vrf_base;
#define _KEY_BASE_ 0x10000000

#define TIME_STAMP_DBG
/*#undef TIME_STAMP_DBG*/
#ifdef TIME_STAMP_DBG 
static sal_usecs_t        start;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    cli_out(\
            "\n %s: Time Stamp: [%u]\n",                         \
            msg, SAL_USECS_SUB(sal_time_usecs(), start));      \
  } while(0);

#define TIME_STAMP_RATE(msg, count)                                       \
  do {                                                              \
    cli_out(\
            "\n %s: Rate: [%u]\n",                         \
            msg, (count) * 1000 / (SAL_USECS_SUB(sal_time_usecs(), start)/1000)); \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#define TIME_STAMP_RATE(msg, count) 
#endif

/* Routes that bypass the drive rlogic and stamp in a canned route table */
static int taps_pop_canned_ipv6(int unit, taps_init_params_t *param, taps_handle_t handle,int routes);


extern taps_container_t *taps_state[SOC_MAX_NUM_DEVICES];

int taps_ut_setup_ipv6_keyploder(int unit, taps_arg_t *arg);


int taps_table_ipv6_init(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_init_params_t *param = &taps_test_param[testInfo->testTapsInst].param;
    uint32 ipv6_defentry[4] = {0x0};
    int slave_idx, slave_unit;

    ipv6_defentry[3]=     0x3fffa;

    cli_out("\nUnified: %d, Search mode: %d\n", 
            testInfo->testTapsInst == 2,testInfo->testSearchModeId);
    taps_set_caching(unit,1);

    /* Creating our own TAPS Table */
    sal_memset(&taps_test_param[testInfo->testTapsInst],0,sizeof(taps_table_param_t));

    /* instance */
    param->instance = testInfo->testTapsInst;

    /* key attribute */
    param->key_attr.type = TAPS_IPV6_KEY_TYPE;
    param->key_attr.length = TAPS_IPV6_KEY_SIZE;
    param->key_attr.vrf_length = TAPS_IPV6_MAX_VRF_SIZE;

    /* segment attribute */
    if (_TAPS_IS_PARALLEL_MODE_(param->instance)) {
        param->seg_attr.seginfo[param->instance ].num_entry = 1024; 
    } else {
        param->seg_attr.seginfo[param->instance ].num_entry = 1024 * 2; 
    }
    param->divide_ratio = 5;
	
    param->tcam_layout = TAPS_TCAM_QUAD_ENTRY;
    param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES;

    for (index=0; index < TAPS_DDR_TABLE_MAX; index++) {
        param->dbucket_attr.flags[index] = 0;
        param->dbucket_attr.table_id[index] = 0;
    }

    param->mode = testInfo->testSearchModeId;
    if (param->mode == TAPS_OFFCHIP_ALL) {
        param->max_capacity_limit = 4096 * 1024;
        param->dbucket_attr.num_dbucket_pfx = testInfo->testNumDramPrf;
    } else {
        param->max_capacity_limit = 256 * 1024;
        param->dbucket_attr.num_dbucket_pfx = 0;
    }
    
    param->defpayload = &ipv6_defentry[0];

    param->host_share_table = 0;
    rv = taps_create(unit, param, &taps_test_param[testInfo->testTapsInst].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create IPv6 TAPS context !!!\n");
    }
    else
    {
    cli_out("Created IPv6 TAPS context 0x%x\n",
            (uint32)taps_test_param[testInfo->testTapsInst].taps);
    }

    param->host_share_table = testInfo->testTapsShare;
    taps_test_param[testInfo->testTapsInst].taps->param.host_share_table = param->host_share_table;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, SOC_IS_SBX_MASTER(unit)?unit:-1, param->host_share_table)) {
        for (slave_idx = 0; slave_idx < SOC_SBX_CONTROL(unit)->num_slaves; slave_idx++) {
            slave_unit = SOC_SBX_CONTROL(unit)->slave_units[slave_idx];
            taps_set_caching(slave_unit,1);
            rv = taps_insert_default_entry_for_slave_unit(slave_unit, taps_test_param[testInfo->testTapsInst].taps);
            if (SOC_FAILURE(rv)) {
                cli_out("!!! Failed to init taps for slave unit !!!\n");
            }
        }
    }

    taps_test_param[testInfo->testTapsInst].arg.taps =
    taps_test_param[testInfo->testTapsInst].taps;

    return rv;
}

int taps_table_mutiple_key_type_init(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_init_params_t *param = &taps_test_param[testInfo->testTapsInst].param;
    int test_case;

    cli_out("\nUnified: %d, Search mode: %d\n", 
            testInfo->testTapsInst == 2,testInfo->testSearchModeId);
    taps_set_caching(unit,1);
    sal_memset(&taps_test_param[testInfo->testTapsInst],0,sizeof(taps_table_param_t));

    /* instance */
    param->instance = testInfo->testTapsInst;
    test_case = testInfo->testCase;

    switch (test_case) {
    case 0:
        param->key_attr.type = TAPS_IPV4_KEY_TYPE;
        param->key_attr.length = TAPS_32BITS_KEY_SIZE;
        param->tcam_layout = TAPS_TCAM_SINGLE_ENTRY;
        break;
    case 1:
        param->key_attr.type = TAPS_IPV6_KEY_TYPE;
        param->key_attr.length = TAPS_64BITS_KEY_SIZE;
        param->tcam_layout = TAPS_TCAM_QUAD_ENTRY;
        break;
    case 2:
        param->key_attr.type = TAPS_IPV6_KEY_TYPE;
        param->key_attr.length = TAPS_96BITS_KEY_SIZE;
        param->tcam_layout = TAPS_TCAM_QUAD_ENTRY;
        break;
    default:
        break;
    }
   
    param->key_attr.vrf_length = 0;

    /* segment attribute */
    param->seg_attr.seginfo[param->instance].num_entry = 0; /* Let the driver allocate*/
    param->divide_ratio = 5;

    for (index=0; index < TAPS_DDR_TABLE_MAX; index++) {
        param->dbucket_attr.flags[index] = 0;
        param->dbucket_attr.table_id[index] = 0;
    }

    param->mode = testInfo->testSearchModeId;
    if (param->mode == TAPS_OFFCHIP_ALL) {
        param->max_capacity_limit = 2048 * 1024;
        param->dbucket_attr.num_dbucket_pfx = testInfo->testNumDramPrf;
    } else {
        param->max_capacity_limit = 288 * 1024;
        param->dbucket_attr.num_dbucket_pfx = 0;
    }
    
    param->defpayload = &defentry[testInfo->testTapsInst][0];
    rv = taps_create(unit, param, &taps_test_param[testInfo->testTapsInst].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context !!!\n");
    }

    taps_test_param[testInfo->testTapsInst].arg.taps =
            taps_test_param[testInfo->testTapsInst].taps;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&taps_test_param[testInfo->testTapsInst].mem_begin, &taps_test_param[testInfo->testTapsInst].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", taps_test_param[testInfo->testTapsInst].mem_begin, taps_test_param[testInfo->testTapsInst].mem_max);
#endif

    tmu_taps_ut_test_result = 0;
    vrf_base = _VRF_BASE_;

    return rv;
}


int taps_table_init(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_init_params_t *param = &taps_test_param[testInfo->testTapsInst].param;
    int slave_idx, slave_unit;

    cli_out("\nUnified: %d, Search mode: %d\n", 
            testInfo->testTapsInst == 2,testInfo->testSearchModeId);
    taps_set_caching(unit,1);
    sal_memset(&taps_test_param[testInfo->testTapsInst],0,sizeof(taps_table_param_t));

    
    /* instance */
    param->instance = testInfo->testTapsInst;

    /* key attribute */
    param->key_attr.type = TAPS_IPV4_KEY_TYPE;
    param->key_attr.length= TAPS_IPV4_KEY_SIZE;
    param->key_attr.vrf_length = TAPS_IPV4_MAX_VRF_SIZE;

    /* segment attribute */
    param->seg_attr.seginfo[param->instance].num_entry = 0; /* Let the driver allocate*/
    param->divide_ratio = 5;
    
    param->tcam_layout = TAPS_TCAM_SINGLE_ENTRY;
    param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES;

    for (index=0; index < TAPS_DDR_TABLE_MAX; index++) {
        param->dbucket_attr.flags[index] = 0;
        param->dbucket_attr.table_id[index] = 0;
    }

    param->mode = testInfo->testSearchModeId;
    if (param->mode == TAPS_OFFCHIP_ALL) {
        param->max_capacity_limit = 2048 * 1024;
        param->dbucket_attr.num_dbucket_pfx = testInfo->testNumDramPrf;
    } else {
        param->max_capacity_limit = 288 * 1024;
        param->dbucket_attr.num_dbucket_pfx = 0;
    }
    param->defpayload = &defentry[testInfo->testTapsInst][0];

    /* Don't need to extend the create precedure to other slave units */
    param->host_share_table = 0;  
    rv = taps_create(unit, param, &taps_test_param[testInfo->testTapsInst].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context !!!\n");
    }
    else {
        cli_out("Created IPv4 TAPS context 0x%x\n",
                (uint32)taps_test_param[testInfo->testTapsInst].taps);
    }

    
    param->host_share_table = testInfo->testTapsShare;
    taps_test_param[testInfo->testTapsInst].taps->param.host_share_table = param->host_share_table;
    if (_IS_MASTER_SHARE_LPM_TABLE(unit, SOC_IS_SBX_MASTER(unit)?unit:-1, param->host_share_table)) {
        for (slave_idx = 0; slave_idx < SOC_SBX_CONTROL(unit)->num_slaves; slave_idx++) {
            slave_unit = SOC_SBX_CONTROL(unit)->slave_units[slave_idx];
            taps_set_caching(slave_unit,1);
            rv = taps_insert_default_entry_for_slave_unit(slave_unit, taps_test_param[testInfo->testTapsInst].taps);
            if (SOC_FAILURE(rv)) {
                cli_out("!!! Failed to init taps for slave unit !!!\n");
            }
        }
    }

    
    taps_test_param[testInfo->testTapsInst].arg.taps =
            taps_test_param[testInfo->testTapsInst].taps;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&taps_test_param[testInfo->testTapsInst].mem_begin, &taps_test_param[testInfo->testTapsInst].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", taps_test_param[testInfo->testTapsInst].mem_begin, taps_test_param[testInfo->testTapsInst].mem_max);
#endif

    tmu_taps_ut_test_result = 0;
    vrf_base = _VRF_BASE_;

    if(testInfo->testVerbFlag)
        taps_tcam_dump(unit, taps_test_param[testInfo->testTapsInst].arg.taps->tcam_hdl,
                testInfo->testVerbFlag);

    return rv;
}

int taps_multi_table_init(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    taps_init_params_t *param = &taps_test_param[0].param;

    rv = soc_sbx_caladan3_tmu_driver_init(unit);
    if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d TMU driver init failed %d\n"), 
                   unit, rv));
        return (rv);
    }

    sal_memset(taps_test_param,0,sizeof(taps_init_params_t)*_MAX_TAPS_PARAM_);

    /* instance */
    param->instance = TAPS_INST_0;

    /* key attribute */
    param->key_attr.type = TAPS_IPV4_KEY_TYPE;
    param->key_attr.length = TAPS_IPV4_KEY_SIZE;
    param->key_attr.vrf_length = TAPS_IPV4_MAX_VRF_SIZE;

    /* segment attribute */
    param->seg_attr.seginfo[param->instance].offset = 0;
    param->seg_attr.seginfo[param->instance].num_entry = 1024*2; /* num tcam entries */

    param->tcam_layout = TAPS_TCAM_SINGLE_ENTRY;
    
    param->sbucket_attr.format = SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES;

    param->dbucket_attr.num_dbucket_pfx = 7;

    for (index=0; index < TAPS_DDR_TABLE_MAX; index++) {
        param->dbucket_attr.flags[index] = 0;
        param->dbucket_attr.table_id[index] = 0;
    }

    param->mode = TAPS_OFFCHIP_ALL;

    param->max_capacity_limit = 256*1024;

    param->defpayload = &defentry[0][0];

    rv = taps_create(unit, param, &taps_test_param[0].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context for table:0 !!!\n");
    }

    /* create second taps instance */
    param = &taps_test_param[1].param;
    sal_memcpy(param,  &taps_test_param[0].param, sizeof(taps_init_params_t));

    param->seg_attr.seginfo[param->instance].offset = 2*1024;
    rv = taps_create(unit, param, &taps_test_param[1].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context for table:1 !!!\n");
    }

    param = &taps_test_param[2].param;
    sal_memcpy(param,  &taps_test_param[0].param, sizeof(taps_init_params_t));
    param->instance = TAPS_INST_1;
    param->seg_attr.seginfo[param->instance].num_entry = 1024*2; /* num tcam entries */

    rv = taps_create(unit, param, &taps_test_param[2].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context for table:2 !!!\n");
    }

    param = &taps_test_param[3].param;
    sal_memcpy(param,  &taps_test_param[2].param, sizeof(taps_init_params_t));
    param->instance = TAPS_INST_1;
    param->seg_attr.seginfo[param->instance].offset = 2*1024;

    rv = taps_create(unit, param, &taps_test_param[3].taps);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS context for table:3 !!!\n");
    }

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&taps_test_param[0].mem_begin, &taps_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", taps_test_param[0].mem_begin, taps_test_param[0].mem_max);
#endif

    tmu_taps_ut_test_result = 0;
    vrf_base = _VRF_BASE_;
    return rv;
}


#define _TAPS_LRP_PRG_NUM_ (4)
#define _TAPS_TMU_PRG_NUM_ (4)


#define _TAPS_LRP_IPV6_PRG_NUM_ (5)
#define _TAPS_TMU_IPV6_PRG_NUM_ (5)

int taps_ut_payload_verify(taps_search_mode_e_t mode, 
                        uint32 *expected_pyld,
                        uint32 *gotten_pyld) 
{
    int rv = SOC_E_NONE;
    uint32 onchip_expected_payload;
    uint32 onchip_gotten_payload;
    if (mode == TAPS_ONCHIP_ALL) {
        onchip_expected_payload = expected_pyld[0] 
                            & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        onchip_gotten_payload = gotten_pyld[0] 
                            & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        if (onchip_expected_payload != onchip_gotten_payload) {
            cli_out("\n!!!! Failed to match payload "
                    "Expected:0x%x "
                    "got:0x%x !!!\n",
                    onchip_expected_payload,
                    onchip_gotten_payload);
            rv = SOC_E_FAIL;
        } 
    } else {
        if (sal_memcmp(gotten_pyld, expected_pyld, 
                    sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_) != 0) {
            cli_out("\n!!!! Failed to match payload "
                    "Expected:0x%x-0x%x-0x%x-0x%x, "
                    "got:0x%x-0x%x-0x%x-0x%x  !!!\n",
                    expected_pyld[0], expected_pyld[1],
                    expected_pyld[2], expected_pyld[3],
                    gotten_pyld[0], gotten_pyld[1], 
                    gotten_pyld[2], gotten_pyld[3]);
            rv = SOC_E_FAIL;
        }
    }
    return rv;
}

int taps_ut_setup_ucode_lookup(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    int slave_idx, slave_unit;
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    taps_init_params_t *param = &arg->taps->param;
    uint32 rval=0;
    uint8 key_zero_v = TRUE;
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    if (param->instance == TAPS_INST_1) {
        /* coverity[secure_coding] */
        sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps1_ut.txt");
        key_zero_v = FALSE;
    } else {
        /* coverity[secure_coding] */
        sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps_ut.txt");
    }

    /* key format:
     * VRF <bit 48-32> , IPV4<31-0> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _TAPS_TMU_PRG_NUM_;
    if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY;
    } else {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_UNIFIED_KEY;
    }
    prog_info.key_info[0].shift[0] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 6;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 0;      
    prog_info.key_info[0].taps_seg = arg->taps->segment;
    prog_info.key_info[0].valid = TRUE;
    if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
        prog_info.key_info[0].tableid = 0;
    } else if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    } else {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    }
    prog_info.key_shift[0] = 0; 

    if (param->instance == TAPS_INST_1) {
        sal_memcpy(&prog_info.key_info[1], &prog_info.key_info[0], sizeof(soc_sbx_caladan3_tmu_key_info_t));
        prog_info.key_info[0].valid = FALSE;
        prog_info.key_info[1].valid = TRUE;        
    }
    
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));
    tmu_taps_ucode_loaded = TRUE;

    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _TAPS_LRP_PRG_NUM_,
                                  _TAPS_TMU_PRG_NUM_, 0,
                                  key_zero_v, !key_zero_v));
            
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info)); 

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_setup_ucode_lookup(slave_unit, arg);
        }
    }

    return rv;
}

/* Set up the microcode for IPv6 lookup*/
int taps_ut_setup_ipv6_ucode_lookup(int unit, taps_arg_t *p_t0_arg)
{
    int rv = SOC_E_NONE;
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    uint32 rval=0;
    int slave_idx, slave_unit;
    
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps_ipv6_ut.txt");


    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));
    tmu_taps_ucode_loaded = TRUE;

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, p_t0_arg->taps->master_unit, p_t0_arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < p_t0_arg->taps->num_slaves; slave_idx++) {
            slave_unit = p_t0_arg->taps->slave_units[slave_idx];
            rv = taps_ut_setup_ipv6_ucode_lookup(slave_unit, p_t0_arg);
            if (SOC_FAILURE(rv)) {
                break;
            }
        }
    }

    return rv;
}

int taps_ut_setup_multiple_key_type_ucode_lookup(int unit, taps_arg_t *arg, int key_size)
{
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    taps_init_params_t *param = &arg->taps->param;
    uint32 rval=0;
    uint8 key_zero_v = TRUE;
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    
    /* key format:
     * VRF <bit 48-32> , IPV4<31-0> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
   
    if (key_size <= TAPS_IPV4_KEY_SIZE) {
        if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
            prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY;
        } else {
            prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_UNIFIED_KEY;
        }
        prog_info.program_num = _TAPS_TMU_PRG_NUM_;
        if (param->instance == TAPS_INST_1) {
            /* coverity[secure_coding] */
            sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps1_ut.txt");
            key_zero_v = FALSE;
        } else {
            /* coverity[secure_coding] */
            sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps_ut.txt");
        }
    } else {
        if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
            prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_SUB_KEY;
        } else {
            prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_UNIFIED_KEY;
        }
        prog_info.program_num = _TAPS_TMU_IPV6_PRG_NUM_;
        /* coverity[secure_coding] */
        sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_taps_ipv6_ut.txt");
    }
    prog_info.key_info[0].shift[0] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = BITS2BYTES(key_size);
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 0;      
    prog_info.key_info[0].taps_seg = arg->taps->segment;
    prog_info.key_info[0].valid = TRUE;
    if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
        prog_info.key_info[0].tableid = 0;
    } else if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    } else {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    }
    prog_info.key_shift[0] = 0; 

    if (param->instance == TAPS_INST_1) {
        sal_memcpy(&prog_info.key_info[1], &prog_info.key_info[0], sizeof(soc_sbx_caladan3_tmu_key_info_t));
        prog_info.key_info[0].valid = FALSE;
        prog_info.key_info[1].valid = TRUE;        
    }
    
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));
    tmu_taps_ucode_loaded = TRUE;

    if (key_size <= TAPS_IPV4_KEY_SIZE) {
        SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _TAPS_LRP_PRG_NUM_,
                                                          _TAPS_TMU_PRG_NUM_, 0,
                                                          key_zero_v, !key_zero_v));
    } else {
        SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _TAPS_LRP_IPV6_PRG_NUM_,
                                                          _TAPS_TMU_IPV6_PRG_NUM_, 0,
                                                          TRUE, FALSE));
    }
            
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info)); 

    return SOC_E_NONE;
}

int taps_ut_setup_ipv6_keyploder(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    int slave_idx, slave_unit;
    
    sal_memset(&prog_info, 0, sizeof(prog_info));


    /* key format:
     * VRF <bit 272--256> , IPV6<127-0> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _TAPS_TMU_IPV6_PRG_NUM_;
    if (_TAPS_IS_PARALLEL_MODE_(arg->taps->param.instance)) {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_SUB_KEY;
    } else {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_UNIFIED_KEY;
    }
    prog_info.key_info[0].shift[0] = 32; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 2;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 16;      
    prog_info.key_info[0].taps_seg = arg->taps->segment;
    prog_info.key_info[0].valid = TRUE;
    if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
        prog_info.key_info[0].tableid = 0;
    } else if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    } else {
        prog_info.key_info[0].tableid = arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    }
    prog_info.key_shift[0] = 16; 

    
    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _TAPS_LRP_IPV6_PRG_NUM_,
                                  _TAPS_TMU_IPV6_PRG_NUM_, 0,
                                  TRUE, FALSE));
            
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info)); 

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv =  taps_ut_setup_ipv6_keyploder(slave_unit, arg);
            if (SOC_FAILURE(rv)) {
                break;
            }
        }
    }

    return rv;
}

int taps_ut_verify_ucode_lookup(int unit, taps_arg_t *arg)
{
    int index;
    static int ucode_eng_started=0;

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    if (tmu_taps_ucode_loaded == 0) return SOC_E_NONE;

    
    if (arg->length == TAPS_32BITS_KEY_SIZE) {
        _ucode_key[0] = arg->key[1];
        _ucode_key[1] = 0;
    } else if (arg->length != TAPS_IPV4_KEY_SIZE){
        _ucode_key[0] = _KEY_BASE_;
        _ucode_key[1] = vrf_base;
    } else if (arg->length == TAPS_IPV4_KEY_SIZE){
        /* ucode key's are different order than host */
        _ucode_key[0] = arg->key[1];
        _ucode_key[1] = arg->key[0];
    }

#ifdef _DBG_VERBOSE
    /* write shared register */
    cli_out("\n$ Ucode Lookup: Sk0: 0x%x 0x%x \n Arg->Length: %d\n", 
            _ucode_key[1], _ucode_key[0], arg->length);
#endif

    /* subkey0, 31-0 p0 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0, _ucode_key[0])); 
    /* subkey0, 63-32 p1 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*1, _ucode_key[1])); 

    /* start microcode execution if not already running*/
    if(!ucode_eng_started)
    { uint64 continuous_epochs = C3HPPC_LRP__CONTINUOUS_EPOCHS;
        SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, continuous_epochs));
        ucode_eng_started=0;
    }
    
    for (index=0; index < 4; index++) {
        _ucode_pyld[index] =  c3hppc_lrp_read_shared_register(unit,16*index+1);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", _ucode_pyld[index]);
#endif
    }
#if 0
    cli_out("Payload shared reg = 0x%x \n", 
            c3hppc_lrp_read_shared_register(unit,16*0+1));
    cli_out("Payload shared reg = 0x%x \n", 
            c3hppc_lrp_read_shared_register(unit,16*1+1));
    cli_out("Payload shared reg = 0x%x \n", 
            c3hppc_lrp_read_shared_register(unit,16*2+1));
    cli_out("Payload shared reg = 0x%x \n", 
            c3hppc_lrp_read_shared_register(unit,16*3+1));
#endif
    return SOC_E_NONE;
}


int taps_ut_verify_ipv6_ucode_lookup(int unit, taps_arg_t *arg)
{
    int rv = SOC_E_NONE;
    int index, i;
    uint32 ipv6_ucode_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];
    uint32 error;
    int offset = 0;

    if (SAL_BOOT_PLISIM) 
    return SOC_E_NONE;
    
    if (tmu_taps_ucode_loaded == 0) 
    return SOC_E_NONE;
 
    for(i=0; i<5; i++) {
    ipv6_ucode_key[i]=arg->key[4-i];
    }

#ifdef _DBG_VERBOSE
    cli_out("\n$ IPV6 Ucode Lookup:  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n Arg->Length: %d\n", 
            ipv6_ucode_key[4],ipv6_ucode_key[3], ipv6_ucode_key[2], ipv6_ucode_key[1],
            ipv6_ucode_key[0], arg->length);
#endif
     
    /* subkey0, 31-0 p0 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0 + offset, ipv6_ucode_key[0])); 
    /* subkey0, 63-32 p1 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*1 + offset, ipv6_ucode_key[1])); 
    /* subkey0, 95-64 p2 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*2 + offset, ipv6_ucode_key[2])); 
    /* subkey0, 127-96 p3 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*3 + offset, ipv6_ucode_key[3])); 


    offset = 1; 
    /* subkey0, 287-256 p0  VRF */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0 + offset, ipv6_ucode_key[4])); 

    /* Run the microcode */
    { 
        uint64 uuVal = COMPILER_64_INIT(0,100);
        SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, uuVal));
    }

    
    for (index=0; index < 4; index++) {
        _ucode_pyld[index] =  c3hppc_lrp_read_shared_register(unit,16*index+2);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", _ucode_pyld[index]);
#endif
    }
    rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                arg->payload,
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        error = c3hppc_lrp_read_shared_register(unit,16*0+3);
        cli_out("\n!!!! Microcode Lookup Failed to match IPV6 Key:"
                "0x%x 0x%x 0x%x 0x%x 0x%x, error 0x%x!!!!!!\n",
                ipv6_ucode_key[4],ipv6_ucode_key[3], ipv6_ucode_key[2],
                ipv6_ucode_key[1],ipv6_ucode_key[0], error);
        rv = SOC_E_FAIL;
    }
    return rv;
}


/* Verify at both sw/hw sides. 
 * Check whether there is a longer key which has the same prefix value as search key
 */
int taps_verify_bpm(int unit, taps_arg_t *arg, taps_route_param_t *route)
{
    unsigned int pyld[] = {0,0,0,0};
    int rv=SOC_E_NONE;
    int slave_idx, slave_unit COMPILER_ATTRIBUTE((unused)), shift_len;
    uint32 bpm_key[TAPS_MAX_KEY_SIZE_WORDS], temp_key[TAPS_MAX_KEY_SIZE_WORDS];
    taps_arg_t temp_arg;
    uint32 bpm_length;

    temp_arg = *arg;
    arg->key = &(route->keys[0]);
    arg->length = route->key_length;
    arg->payload = pyld;
    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)){
        /* Fail to get the prefix, try to find the bpm prefix*/
        rv = taps_get_lpm_route(unit, arg, bpm_key, &bpm_length);
        if (SOC_FAILURE(rv)) {
             cli_out("\n!!!! Failed to get bpm payload !!!\n");
             return rv;
        } else {
           LOG_DEBUG(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "\n route key 0x%x 0x%x ----- BPM Key 0x%x 0x%x, bpm len %d!!!\n"), 
                      route->keys[0], route->keys[1], bpm_key[0], bpm_key[1], bpm_length));
        }
    }

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    arg->key = &(route->match_key[0]);
    arg->length = arg->taps->param.key_attr.length;
    if ((arg->key[0] == 0xFFFFFFFF) && (arg->key[1] == 0xFFFFFFFF)) {
        return SOC_E_NONE;
    }

    /* Find whether there is longer key match current route */
    shift_len =  TAPS_IPV4_KEY_SIZE - route->key_length; 
    while (shift_len > 0) {
        sal_memcpy(&temp_key[0], route->keys, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
        taps_key_shift(TAPS_IPV4_KEY_SIZE, &temp_key[0], route->key_length, 0 - shift_len);
        temp_arg.key = &temp_key[0];
        temp_arg.length = route->key_length + shift_len;
        temp_arg.payload = &pyld[0];
        rv = taps_get_route(unit, &temp_arg);
        if (SOC_SUCCESS(rv)) {
            break;
        }
        shift_len--;
    }

    rv = taps_ut_verify_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x !!!\n", route->keys[0], route->keys[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    } else {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "## verify route 0x%x 0x%x length %d, lookup key 0x%x 0x%x!!!\n"),
                   route->keys[0], route->keys[1], route->key_length,
                   route->match_key[0], route->match_key[1]));
    }

    /* Check bpm payload is the same between sw and hw*/
    rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &pyld[0],
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                route->keys[0], route->keys[1]);
        return SOC_E_FAIL;
    }   

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_verify_ucode_lookup(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to lookup ucode for"
                        " key 0x%x 0x%x !!!\n", route->keys[0], route->keys[1]);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            } else {
                LOG_DEBUG(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "## verify route 0x%x 0x%x length %d, lookup key 0x%x 0x%x!!!\n"),
                           route->keys[0], route->keys[1], route->key_length,
                           route->match_key[0], route->match_key[1]));
            }
            
            /* Check bpm payload is the same between sw and hw*/
            rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                        &pyld[0],
                                        &_ucode_pyld[0]);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                        route->keys[0], route->keys[1]);
                return SOC_E_FAIL;
            } 
        }
    }

    return rv;
}

int taps_verify_ipv6_bpm(int unit, taps_arg_t *arg, taps_route_param_t *route)
{
    unsigned int pyld[] = {0,0,0,0};
    int rv=SOC_E_NONE;
    int slave_idx, slave_unit COMPILER_ATTRIBUTE((unused)), shift_len;
    uint32 bpm_key[TAPS_MAX_KEY_SIZE_WORDS], temp_key[TAPS_MAX_KEY_SIZE_WORDS];
    taps_arg_t temp_arg;
    uint32 bpm_length;

    temp_arg = *arg;
    arg->key = &(route->keys[0]);
    arg->length = route->key_length;
    arg->payload = pyld;
    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        /* Fail to get the prefix, try to find the bpm prefix*/
        rv = taps_get_lpm_route(unit, arg, bpm_key, &bpm_length);
        if (SOC_FAILURE(rv)) {
             cli_out("\n!!!! Failed to get bpm payload !!!\n");
             return rv;
        } else {
           LOG_DEBUG(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "\n route key 0x%x 0x%x 0x%x 0x%x 0x%x ----- "
                                 "BPM Key 0x%x 0x%x 0x%x 0x%x 0x%x, bpm len %d!!!\n"), 
                      route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4],
                      bpm_key[0], bpm_key[1], bpm_key[2], bpm_key[3], bpm_key[4], bpm_length));
        }
    }

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    arg->key = &(route->match_key[0]);
    arg->length = arg->taps->param.key_attr.length;
    if ((arg->key[0] == 0xFFFFFFFF) && (arg->key[1] == 0xFFFFFFFF)) {
        return SOC_E_NONE;
    }
    
    /* Find whether there is longer key match current route */
    shift_len =  TAPS_IPV6_KEY_SIZE - route->key_length; 
    while (shift_len > 0) {
        sal_memcpy(&temp_key[0], route->keys, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
        taps_key_shift(TAPS_IPV6_KEY_SIZE, &temp_key[0], route->key_length, 0 - shift_len);
        temp_arg.key = &temp_key[0];
        temp_arg.length = route->key_length + shift_len;
        temp_arg.payload = &pyld[0];
        rv = taps_get_route(unit, &temp_arg);
        if (SOC_SUCCESS(rv)) {
            break;
        }
        shift_len--;
    }
    
    rv = taps_ut_verify_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x 0x%x 0x%x 0x%x !!!\n", 
                route->keys[0], route->keys[1], route->keys[2], route->keys[3],
                route->keys[4]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    } else {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "## verify route 0x%x 0x%x length %d, lookup key 0x%x 0x%x!!!\n"),
                   route->keys[0], route->keys[1], route->key_length,
                   route->match_key[0], route->match_key[1]));
    }
    
    /* Check bpm payload is the same between sw and hw*/
    rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &pyld[0],
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4]);
        return SOC_E_FAIL;
    }   

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_verify_ipv6_ucode_lookup(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to lookup ucode for"
                        " key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n", route->keys[0], route->keys[1], 
                        route->keys[2], route->keys[3], route->keys[4]);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            } else {
                LOG_DEBUG(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "## verify route 0x%x 0x%x 0x%x 0x%x 0x%x length %d, "
                                      "lookup key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n"),
                           route->keys[0], route->keys[1], route->keys[2], route->keys[3],
                           route->keys[4], route->key_length,
                           route->match_key[0], route->match_key[1], 
                           route->match_key[2], route->match_key[3], 
                           route->match_key[4]));
            }
            
            /* Check bpm payload is the same between sw and hw*/
            rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                        &pyld[0],
                                        &_ucode_pyld[0]);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                        route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4]);
                return SOC_E_FAIL;
            } 
        }
    }

    return rv;
}

int taps_verify_pfx(int unit, taps_arg_t *arg, uint8 fib_verify)
{
    unsigned int pyld[] = {0,0,0,0};
    int rv=SOC_E_NONE;
    int slave_idx, slave_unit;
    
    arg->payload = pyld;
    rv = taps_get_route(unit, arg);

    if (SOC_SUCCESS(rv)) {
        rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &_key_pyld[0],
                                &pyld[0]);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to match Key: 0x%x 0x%x!!!\n",
                    _key[0], _key[1]);
            return SOC_E_FAIL;
        }   
    } else {
        cli_out("\n!!!! Failed to get route 0x%x 0x%x route Eror:%d!!!\n",
                _key[0], _key[1], rv);
        return SOC_E_FAIL;
    }   

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;
    if (!fib_verify) return SOC_E_NONE;

    rv = taps_ut_verify_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x !!!\n", _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    rv = taps_ut_payload_verify(arg->taps->param.mode,
                                &pyld[0],
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                _key[0], _key[1]);
    }   

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_verify_ucode_lookup(slave_unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to lookup ucode for"
                        " UNIT :%d key 0x%x 0x%x !!!\n", slave_unit, _key[0], _key[1]);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }

            rv = taps_ut_payload_verify(arg->taps->param.mode,
                                        &pyld[0],
                                        &_ucode_pyld[0]);

            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!!UNIT :%d Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                        slave_unit, _key[0], _key[1]);
            }
        }
    }
    return rv;
}

int taps_verify_route(int unit, taps_arg_t *arg, taps_route_param_t *route, uint8 fib_verify)
{
    unsigned int pyld[] = {0,0,0,0};
    int rv=SOC_E_NONE;
    int slave_idx, slave_unit;

    /* get route (software lookup) */
    arg->key = &(route->keys[0]);
    arg->length = route->key_length;
    arg->payload = pyld;
    rv = taps_get_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &route->payload[0],
                                &pyld[0]);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to match route: 0x%x 0x%x!!!\n",
                    route->keys[0], route->keys[1]);
            return SOC_E_FAIL;
        }
    } else {
        cli_out("\n!!!! Failed to get route 0x%x 0x%x route Eror:%d!!!\n",
                route->keys[0], route->keys[1], rv);
        return SOC_E_FAIL;
    }   

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;
    if (!fib_verify) return SOC_E_NONE;

    /* verify route, lrp lookup */
    arg->key = &(route->match_key[0]);
    arg->length = arg->taps->param.key_attr.length;
    if ((arg->key[0] == 0xFFFFFFFF) && (arg->key[1] == 0xFFFFFFFF)) {
        /* failed to find a match key for this route, skip for now */
        return SOC_E_NONE;
    }

    rv = taps_ut_verify_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x !!!\n", route->keys[0], route->keys[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    } else {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "## verify route 0x%x 0x%x length %d, lookup key 0x%x 0x%x!!!\n"),
                   route->keys[0], route->keys[1], route->key_length,
                   route->match_key[0], route->match_key[1]));
    }

    rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &route->payload[0],
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                route->keys[0], route->keys[1]);
        return SOC_E_FAIL;
    }

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_verify_ucode_lookup(slave_unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to lookup ucode for"
                        " key 0x%x 0x%x !!!\n", route->keys[0], route->keys[1]);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            } else {
                LOG_DEBUG(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "## verify route 0x%x 0x%x length %d, lookup key 0x%x 0x%x!!!\n"),
                           route->keys[0], route->keys[1], route->key_length,
                           route->match_key[0], route->match_key[1]));
            }

            rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                        &route->payload[0],
                                        &_ucode_pyld[0]);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x!!!\n",
                        route->keys[0], route->keys[1]);
                return SOC_E_FAIL;
            }
        }
    }
    return rv;
}

int taps_verify_ipv6_route(int unit, taps_arg_t *arg, taps_route_param_t *route, uint8 fib_verify)
{
    unsigned int pyld[] = {0,0,0,0};
    int rv=SOC_E_NONE;
    int slave_idx, slave_unit;
    
    /* get route (software lookup) */
    arg->key = &(route->keys[0]);
    arg->length = route->key_length;
    arg->payload = pyld;
    rv = taps_get_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &route->payload[0],
                                &pyld[0]);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to match route:"
                    "0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                    route->keys[0], route->keys[1], route->keys[2], 
                    route->keys[3], route->keys[4]);
            return SOC_E_FAIL;
        }
    } else {
        cli_out("\n!!!! Failed to get route "
                "0x%x 0x%x 0x%x 0x%x 0x%x route Eror:%d!!!\n",
                route->keys[0], route->keys[1], route->keys[2], 
                route->keys[3], route->keys[4], rv);
        return SOC_E_FAIL;
    }   

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;
    if (!fib_verify) return SOC_E_NONE;

    /* verify route, lrp lookup */
    arg->key = &(route->match_key[0]);
    arg->length = arg->taps->param.key_attr.length;
    if ((arg->key[0] == 0xFFFFFFFF) && 
        (arg->key[1] == 0xFFFFFFFF) && 
        (arg->key[2] == 0xFFFFFFFF) && 
        (arg->key[3] == 0xFFFFFFFF) && 
        (arg->key[4] == 0xFFFFFFFF)) {
        /* failed to find a match key for this route, skip for now */
        return SOC_E_NONE;
    }

    rv = taps_ut_verify_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x 0x%x 0x%x 0x%x  match key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4],
                route->match_key[0], route->match_key[1], route->match_key[2], route->match_key[3], route->match_key[4]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    } else {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "## verify route 0x%x 0x%x 0x%x 0x%x 0x%x length %d,"
                              "lookup key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n"),
                   route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4],
                   route->key_length, route->match_key[0], route->match_key[1], route->match_key[2],
                   route->match_key[3], route->match_key[4]));
    }

    rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                &route->payload[0],
                                &_ucode_pyld[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key:"
                "0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                route->keys[0], route->keys[1], route->keys[2], 
                route->keys[3], route->keys[4]);
    }

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < arg->taps->num_slaves; slave_idx++) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_ut_verify_ipv6_ucode_lookup(slave_unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to lookup ucode for"
                        " key 0x%x 0x%x 0x%x 0x%x 0x%x  match key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                        route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4],
                        route->match_key[0], route->match_key[1], route->match_key[2], route->match_key[3], route->match_key[4]);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            } else {
                LOG_DEBUG(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "## verify route 0x%x 0x%x 0x%x 0x%x 0x%x length %d,"
                                      "lookup key 0x%x 0x%x 0x%x 0x%x 0x%x!!!\n"),
                           route->keys[0], route->keys[1], route->keys[2], route->keys[3], route->keys[4],
                           route->key_length, route->match_key[0], route->match_key[1], route->match_key[2],
                           route->match_key[3], route->match_key[4]));
            }
            
            rv = taps_ut_payload_verify(arg->taps->param.mode,          
                                        &route->payload[0],
                                        &_ucode_pyld[0]);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Microcode Lookup Failed to match Key:"
                        "0x%x 0x%x 0x%x 0x%x 0x%x!!!\n",
                        route->keys[0], route->keys[1], route->keys[2], 
                        route->keys[3], route->keys[4]);
            }

        }
    }
    
    return rv;
}

int taps_delete_verify_pfx(int unit, taps_arg_t *arg)
{
    int rv=SOC_E_NONE;
    
    rv = taps_delete_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        rv = taps_get_route(unit, arg);
        if (rv != SOC_E_NOT_FOUND) {
            cli_out("\n!!!! Failed to verify delete route !!!\n");
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        } else {
            rv = SOC_E_NONE;
        }
    } else {
        cli_out("\n!!!! Failed to Delete route 0x%x 0x%x route !!!\n",
                arg->key[0], arg->key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }    
    return rv;
}

int taps_basic_table_test_run(int unit, int param_idx, uint8 fib_verify)
{
    int index, fibcnt[_MAX_TAPS_PARAM_], max, rv;
    taps_arg_t *arg=NULL;

    _key[0] = vrf_base;
    _key[1] = 0;

    if (param_idx > 0 && param_idx >= _MAX_TAPS_PARAM_) return SOC_E_PARAM;

    max = ((param_idx < 0) ? (_MAX_TAPS_PARAM_-1):param_idx);
    
    sal_memset(fibcnt, 0, sizeof(int)*_MAX_TAPS_PARAM_);

    for (index=(param_idx<0)?0:param_idx; index < max; index++) {
        /* insert & get routes from multiple tables */
        arg = &taps_test_param[index].arg;

        arg->taps = taps_test_param[0].taps;
        arg->key = &_key[0];
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt[index]++;
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vpn def route %d !!!\n", index);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }

        rv = taps_verify_pfx(unit, arg, fib_verify);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);
            cli_out("\n!!!! Failed to verify route:%d %d!!!\n", 
                    fibcnt[index], index);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }

        /* insert a route onto the vpn */
        /* 10.0.0.0/30 */
        _key[0] = vrf_base;
        _key[1] = _KEY_BASE_ >> 31; /*_KEY_BASE_;*/
        arg->taps = taps_test_param[0].taps;
        arg->key = _key;
        arg->length = TAPS_IPV4_KEY_SIZE - 31;
        _key_pyld[0] = fibcnt[index]++;
        arg->payload = _key_pyld;

        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }    

        /* get and verify */
        rv = taps_verify_pfx(unit, arg, fib_verify);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);
            cli_out("\n!!!! Failed to verify route:%d %d!!!\n",
                    fibcnt[index], index);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }

        /* delete the inserted prefix */
        rv = taps_delete_verify_pfx(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        } 

        /* delete the vpn route */
        arg->taps = taps_test_param[0].taps;
        arg->key = &_key[0];
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;

        rv = taps_delete_verify_pfx(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        } 
    }

    return SOC_E_NONE;
}




int taps_basic_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE;

    rv = taps_ut_setup_ucode_lookup(unit, &taps_test_param[0].arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    return taps_basic_table_test_run(unit, 0, TRUE);
}

int taps_basic_multi_table_test_run(int unit, void *data)
{
    return taps_basic_table_test_run(unit, -1, FALSE);
}

/*
* IPv4 update/get case
* 1. Insert vrf default routes and loopback routes
* 2. Insert routes with random value&len
* 3. Verify routes
* 4. Update payload
* 5. Verify again
*/
int taps_ipv4_udpate_and_get_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, route_idx, vrf_idx;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    int dump_flags;

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    netmask_start = 16;
    single_vrf = (sal_rand() % 2 == 0);
    vrf_count = sal_rand() % 4096;
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* add 127.0.0.0/8 to match customer test case */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0xF0000000; /* mark this payload as a 127.0.0.0/8 route payload */
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Generate  and Insert routes with random value&length
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes[route_idx].keys[0] = 0;
        } else {
            routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
        }
        
        if (netmask_len == 32) {
            routes[route_idx].keys[1] = ipaddr;
        } else {
            routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
        }
        routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;
            

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
        routes[route_idx].payload[0] = route_idx;     

        /* Try to get this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }

    cli_out("##used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
            arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
            num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
    
    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Verified %d(K) routes after insert \n", route_idx/1024);
        }
    }

    /****************************************************
        *Update routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
            routes[route_idx].payload[0] = (0x10000 + route_idx) 
                        & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        } else {
            routes[route_idx].payload[0] = 0xFF000000 + route_idx;
        }
        
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        
        rv = taps_update_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to update routes key 0x%x 0x%x length %d : %s !!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                    soc_errmsg(rv));
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Updated %d(K) routes\n", route_idx/1024);
        }
    }

    /****************************************************
        *Verify routes again
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Verified %d(K) routes after update\n", route_idx/1024);
        }
    }
           
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

/*
* IPv6 update/get case
* 1. Insert vrf default routes and loopback routes
* 2. Insert routes with random value&len
* 3. Verify routes
* 4. Update payload
* 5. Verify again
*/
int taps_ipv6_udpate_and_get_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, num_routes, route_idx, 
        vrf_idx, vrf_count, netmask_start;
    int netmask_len;
    uint32 ipaddr[4], vrf, prefixSeed;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    int dump_flags;

    netmask_start = 32;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;

    sal_srand(prefixSeed);

    vrf_count = sal_rand() % 4096;

    
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* Set up the IPv6 keyploder to use this taps table */
    rv=taps_ut_setup_ipv6_keyploder(unit, arg);    
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to setup IPv6 keyploder !!!\n");
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);


    /****************************************************
        *Insert vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Insert routes
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV6_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr[0] = V4_RAND();
        ipaddr[1] = V4_RAND();
        ipaddr[2] = V4_RAND();
        ipaddr[3] = V4_RAND();

        /* Vrf value */
        vrf = sal_rand() % vrf_count;

        /* align the keys by shifting */
        routes[route_idx].keys[0] = vrf;
        routes[route_idx].keys[1] = ipaddr[0];
        routes[route_idx].keys[2] = ipaddr[1];
        routes[route_idx].keys[3] = ipaddr[2];
        routes[route_idx].keys[4] = ipaddr[3];
        routes[route_idx].key_length =  TAPS_IPV6_MAX_VRF_SIZE + netmask_len;
            
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].keys,
                            TAPS_IPV6_KEY_SIZE, 
                            TAPS_IPV6_PFX_SIZE - netmask_len);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        rv =  taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    netmask_len - TAPS_IPV6_PFX_SIZE);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        routes[route_idx].payload[0] = route_idx;     

        /* Insert this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x 0x%x 0x%x "
                        "0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                        routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }

    cli_out("##used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
            arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
            num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);

    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }

    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Verified %d(K) routes after insert \n", route_idx/1024);
        }
    }

    
    /****************************************************
        *Update routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        if (arg->taps->param.mode == TAPS_ONCHIP_ALL) {
            routes[route_idx].payload[0] = (0x10000 + route_idx) 
                        & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        } else {
            routes[route_idx].payload[0] = 0xFF000000 + route_idx;
        }
        
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        
        rv = taps_update_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Updated %d(K) routes\n", route_idx/1024);
        }
    }

     /****************************************************
        *Verify routes again
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Verified %d(K) routes after update\n", route_idx/1024);
        }
    }
    
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}


/*
* Fuction : 
*       taps_multiple_key_type_test_run
* Purpose:
*       Test insert/update/get case.
*       
*       This function used to test  whether taps_update_route()
* could work fine. Insert some routes, then update payload of these 
* routes, finally verify the results.
*/

int taps_multiple_key_type_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index=0, fibcnt=0;
    uint32 key_value, key_length;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    int num_routes = testInfo->testNumRoutes;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg; 
    taps_route_param_t *route = NULL;
    int test_case;

    arg->taps = taps_test_param[testInfo->testTapsInst].taps;
    test_case = testInfo->testCase;
    key_length = testInfo->testKeyLength;

   
    rv = taps_ut_setup_multiple_key_type_ucode_lookup(unit, arg, arg->taps->param.key_attr.length);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    route = (taps_route_param_t *)sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route Info");
    sal_memset(route, 0, sizeof(taps_route_param_t) * num_routes);

    fibcnt++;

    key_value = sal_rand() & 0x7FFFFFFF;
    switch (test_case) {
    case 0: 
        if (key_length > TAPS_32BITS_KEY_SIZE) {
            if(route) sal_free(route);
            return SOC_E_PARAM;
        }
        
        for (index = 0; index < num_routes; index++) {
            route[index].keys[0] = 0;
            route[index].keys[1] = key_value++ & TP_MASK(key_length);
            route[index].key_length = key_length;
            route[index].payload[0] = fibcnt;
            sal_memcpy(&route[index].match_key[0], &route[index].keys[0], sizeof(uint32) * BITS2WORDS(TAPS_IPV4_KEY_SIZE));
            /* shift left to form 48 bits key */
            rv = taps_key_shift(arg->taps->param.key_attr.lookup_length, route[index].match_key,
                        route[index].key_length, route[index].key_length-arg->taps->param.key_attr.length);
            if (SOC_FAILURE(rv)) {
               cli_out("\n!!!! Failed to shift key !!!\n");
               tmu_taps_ut_test_result = -1;
               sal_free(route);
               return SOC_E_FAIL;
            }
            fibcnt++;
            if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
                cli_out("!!! generated %d(K) routes\n", fibcnt/1024);
            }
        }
        break;
    case 1:
        if (key_length > TAPS_64BITS_KEY_SIZE) {
            if(route) sal_free(route);
            return SOC_E_PARAM;
        }
        for (index = 0; index < num_routes; index++) {
            route[index].keys[0] = 0;
            route[index].keys[1] = 0;
            route[index].keys[2] = 0;
            
            if (key_length > 32) {
                route[index].keys[3] = key_value & TP_MASK(key_length - 32);
                route[index].keys[4] = key_value++;
            } else {
                route[index].keys[3] = 0;
                route[index].keys[4] = key_value++ & TP_MASK(key_length);
            }
            route[index].key_length = key_length;
            route[index].payload[0] = fibcnt;
            sal_memcpy(&route[index].match_key[0], &route[index].keys[0], sizeof(uint32) * BITS2WORDS(TAPS_IPV6_KEY_SIZE));
            /* shift left to form 48 bits key */
            rv = taps_key_shift(arg->taps->param.key_attr.lookup_length, route[index].match_key,
                        route[index].key_length, route[index].key_length-arg->taps->param.key_attr.length);
            if (SOC_FAILURE(rv)) {
               cli_out("\n!!!! Failed to shift key !!!\n");
               tmu_taps_ut_test_result = -1;
               sal_free(route);
               return SOC_E_FAIL;
            }
            fibcnt++;
            if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
                cli_out("!!! generated %d(K) routes\n", fibcnt/1024);
            }
        }
        break;
    case 2:
        if (key_length > TAPS_96BITS_KEY_SIZE) {
            if(route) sal_free(route);          
            return SOC_E_PARAM;
        }
        for (index = 0; index < num_routes; index++) {
            route[index].keys[0] = 0;
            route[index].keys[1] = 0;
            if (key_length > 64) {
                route[index].keys[2] = key_value & TP_MASK(key_length - 64);
                route[index].keys[3] = key_value;
                route[index].keys[4] = key_value++;
            } else if (key_length > 32) {
                route[index].keys[2] = 0;
                route[index].keys[3] = key_value & TP_MASK(key_length - 32);
                route[index].keys[4] = key_value++;
            } else {
                route[index].keys[2] = 0;
                route[index].keys[3] = 0;
                route[index].keys[4] = key_value++ & TP_MASK(key_length);
            }
            route[index].key_length = testInfo->testKeyLength;
            route[index].payload[0] = fibcnt;
            sal_memcpy(&route[index].match_key[0], &route[index].keys[0], sizeof(uint32) * BITS2WORDS(TAPS_IPV6_KEY_SIZE));
            /* shift left to form 48 bits key */
            rv = taps_key_shift(arg->taps->param.key_attr.lookup_length, route[index].match_key,
                        route[index].key_length, route[index].key_length-arg->taps->param.key_attr.length);
            if (SOC_FAILURE(rv)) {
               cli_out("\n!!!! Failed to shift key !!!\n");
               tmu_taps_ut_test_result = -1;
               sal_free(route);
               return SOC_E_FAIL;
            }
            fibcnt++;
            if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
                cli_out("!!! generated %d(K) routes\n", fibcnt/1024);
            }
        }
        break;
    default:
        break;
    }
    for (index = 0; index < num_routes; index++) {
        arg->key = &route[index].keys[0];
        arg->length = route[index].key_length;
        arg->payload = &route[index].payload[0];
        /* coverity[ stack_use_overflow ] */
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x :%s!!!\n",
                    route[index].keys[0], route[index].keys[1], soc_errmsg(rv));
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if(route) sal_free(route);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        if (route) sal_free(route);
        return SOC_E_FAIL;
    }

    for (index = 0; index < num_routes; index++) {
        arg->key = &route[index].keys[0];
        arg->length = route[index].key_length;
        arg->payload = &route[index].payload[0];
        /* get and verify */
        if (test_case == 0) {
            rv = taps_verify_route(unit, arg, &route[index], TRUE);
        } else {
            rv = taps_verify_ipv6_route(unit, arg, &route[index], TRUE);
        }
        
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to verify route:%d :%s!!!\n", fibcnt,soc_errmsg(rv));
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if(route) sal_free(route);
            return SOC_E_FAIL;
        }
    }

    if(route) sal_free(route);
    return rv;
}


int taps_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index=0, fibcnt=0;
    taps_arg_t *arg = &taps_test_param[0].arg;
    unsigned int bpm_length=0;
    int num_routes = 64 * 1024;

    _key[0] = 0;
    _key[1] = vrf_base;

    /* insert vpn default route */
    arg->taps = taps_test_param[0].taps;
    arg->key = &_key[0];
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    _key_pyld[0] = fibcnt++;
    arg->payload = &_key_pyld[0];

    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vpn def route !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* get and verify */
    rv = taps_verify_pfx(unit, arg, TRUE);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* insert a route onto the vpn */
    /* 10.0.0.1/32 */
    _key[0] = vrf_base;
    _key[1] = _KEY_BASE_ + fibcnt;
    arg->taps = taps_test_param[0].taps;
    arg->key = _key;
    arg->length = TAPS_IPV4_KEY_SIZE;
    _key_pyld[0] = fibcnt++;
    arg->payload = _key_pyld;

    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }   

    /* get and verify */
    rv = taps_verify_pfx(unit, arg, TRUE);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* insert a route onto non existing vpn & check failure */
    /* insert a route onto the vpn */
    /* vrf=1, 10.0.0.1/32 */
    _key[0] = vrf_base + 1;
    _key[1] = 0x10000001;
    arg->taps = taps_test_param[0].taps;
    arg->key = &_key[0];
    arg->length = TAPS_IPV4_KEY_SIZE;
    arg->payload = &_key_pyld[0];

    rv = taps_insert_route(unit, arg);
    if (rv == SOC_E_PARAM) {
        tmu_taps_ut_test_result = 0;
        rv = SOC_E_NONE;
    } else {
        cli_out("\n!!!! Failed on default vrf check !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    } 

    tmu_taps_ut_test_result = 0;

    for (index=0; index <  num_routes; index++) {
        _key[0] = vrf_base;
        _key[1] = _KEY_BASE_+fibcnt;
        arg->taps = taps_test_param[0].taps;
        arg->key = &_key[0];
        arg->length = TAPS_IPV4_KEY_SIZE;
        _key_pyld[0] = fibcnt;
        arg->payload = _key_pyld;

        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            /* get and verify */
            rv = taps_verify_pfx(unit, arg, TRUE);
            if (SOC_FAILURE(rv)) {
                taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }
        } else {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }
        fibcnt++;
    }

    for (index=1; index < 15; index++) {
        _key[0] = vrf_base;
        _key[1] = _KEY_BASE_ >> (32-index); /* 0/10.0.0.0/index*/
        arg->key = _key;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE+index;
        _key_pyld[0] = fibcnt;
        arg->payload = _key_pyld;

        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            /* get and verify */
            rv = taps_verify_pfx(unit, arg, TRUE);
            if (SOC_FAILURE(rv)) {
                taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }
        } else {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }    
        fibcnt++;
    }

    _key[0] = vrf_base;
    _key[1] = _KEY_BASE_;
    arg->length = TAPS_IPV4_KEY_SIZE;
    arg->key = _key;
    index--; /* offset index */

    rv = taps_find_bpm(unit, arg, &bpm_length);
    if (SOC_SUCCESS(rv)) {
        if (bpm_length != TAPS_IPV4_MAX_VRF_SIZE + index) {
            cli_out("\n!!!! Expected BPM len=%d got %d"
                    " for route 0x%x 0x%x route !!!\n",
                    TAPS_IPV4_MAX_VRF_SIZE + index, 
                    bpm_length, _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }
    } else {
        cli_out("\n!!!! Failed to find bpm for route 0x%x 0x%x route !!!\n",
                _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    if (1) {
        taps_tcam_dump(unit, arg->taps->tcam_hdl, 2);
        return rv;
    }

    /* delete the previous bpm route & verify new bpm */
    _key[0] = vrf_base;
    _key[1] = _KEY_BASE_ >> (32-index); /* 0/10.0.0.0/index*/
    arg->length = TAPS_IPV4_MAX_VRF_SIZE+index;
    rv = taps_delete_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        /* get and verify */
        rv = taps_get_route(unit, arg);
        if (rv != SOC_E_NOT_FOUND) {
            cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        } else {
            rv = SOC_E_NONE;
        }
    } else {
        cli_out("\n!!!! Failed to Delete route 0x%x 0x%x route !!!\n",
                _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }    

    _key[0] = vrf_base;
    _key[1] = _KEY_BASE_;
    arg->length = TAPS_IPV4_KEY_SIZE;
    arg->key = _key;
    index--; /* bpm must be the next longer one after the previous bpm was deleted */
    rv = taps_find_bpm(unit, arg, &bpm_length);
    if (SOC_SUCCESS(rv)) {
        if (bpm_length != TAPS_IPV4_MAX_VRF_SIZE + index) {
            cli_out("\n!!!! Expected BPM len=%d got %d"
                    " for route 0x%x 0x%x route !!!\n",
                    TAPS_IPV4_MAX_VRF_SIZE + index, 
                    bpm_length, _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }
    } else {
        cli_out("\n!!!! Failed to find bpm for route 0x%x 0x%x route !!!\n",
                _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

#if 0
    taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);
   
    for (index = 0; index < 14; index++) {
        taps_dbucket_entry_dump(unit, arg->taps, 0, 20, index, 3);
    }
#endif

    return rv;
}


int taps_tcam_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE;
    taps_arg_t *arg = &taps_test_param[0].arg;

    /* run the tcam unit test */
    arg->taps = taps_test_param[0].taps;

    rv = taps_tcam_ut(unit, arg->taps, 1);

    return rv;
}

int taps_test_cleanup(int unit, void *data)
{
    int rv=SOC_E_NONE;
    int i;

    if (tmu_taps_ut_test_result  == 0) /* dont cleanup if failed for debugging */ {
        for (i = 0; i < _MAX_TAPS_PARAM_; i++) {
            if(taps_test_param[i].taps != NULL){
                rv = taps_destroy(unit, taps_test_param[i].taps);
                    taps_test_param[i].taps = NULL;
                    if (SOC_FAILURE(rv)) {
                        cli_out("Failed to Cleanup Table: %d !!! \n", i);
                        return rv;
                    }
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
                    sal_alloc_resource_usage_get(&taps_test_param[i].mem_end, &taps_test_param[i].mem_max);
                    cli_out("\n Mem Begin(%d) End(%d) Max(%d)\n",
                            taps_test_param[0].mem_begin, taps_test_param[i].mem_end, taps_test_param[i].mem_max);
#endif
            }
        }
        soc_sbx_caladan3_tmu_program_free(unit, _TAPS_TMU_PRG_NUM_);
    }

    return rv;
}

int taps_multi_table_test_cleanup(int unit, void *data)
{
    int rv=SOC_E_NONE, index;

    if (tmu_taps_ut_test_result  == 0) /* dont cleanup if failed for debugging */ {
        for (index=0; index < _MAX_TAPS_PARAM_; index++) {
            rv = taps_destroy(unit, taps_test_param[index].taps);
            if (SOC_FAILURE(rv)) {
                cli_out("Failed to Cleanup Table: %d !!! \n", index);
            } else {
                sal_memset(&taps_test_param[index],0,sizeof(taps_init_params_t));
            }
        }
    }

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&taps_test_param[0].mem_end, &taps_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) End(%d) Max(%d)\n",
            taps_test_param[0].mem_begin, taps_test_param[0].mem_end, taps_test_param[0].mem_max);
#endif
    return rv;
}

int taps_domino_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index=0, fibcnt=0;
    taps_arg_t *arg = &taps_test_param[0].arg;

    _key[0] = vrf_base;
    _key[1] = 0;

    /* insert vpn default route */
    arg->taps = taps_test_param[0].taps;
    arg->key = &_key[0];
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    arg->payload = &_key_pyld[0];
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vpn def route !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* get and verify */
    rv = taps_verify_pfx(unit, arg, TRUE);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    tmu_taps_ut_test_result = 0;

    for (index=0; index < 4*1024; index++) {
        _key[0] = vrf_base;
        _key[1] = _KEY_BASE_+fibcnt;
        arg->taps = taps_test_param[0].taps;
        arg->key = &_key[0];
        arg->length = TAPS_IPV4_KEY_SIZE;
        _key_pyld[0] = fibcnt;
        arg->payload = _key_pyld;

        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            /* get and verify */
            rv = taps_verify_pfx(unit, arg, TRUE);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }
        } else {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }    
        fibcnt++;
    }

    arg->taps = taps_test_param[0].taps;
    _key[0] = vrf_base;
    arg->length = TAPS_IPV4_KEY_SIZE;
    arg->payload = _key_pyld;

    for (index=0; index < fibcnt; index++) {
        _key[1] = _KEY_BASE_+index;
        arg->key = _key;
        rv = taps_delete_verify_pfx(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }    
    }

    /* tbd: randomly delete routes */

    /* delete the vrf route */
    _key[1] = 0;
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    rv = taps_delete_verify_pfx(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to delete route 0x%x 0x%x route !!!\n",
                _key[0], _key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }    

    return rv;
}

/* skip verify */
#define _VERIFY_STYLE_NONE (0)
/* verifies the last added route */
#define _VERIFY_STYLE_LAST_ROUTE (1)
/* verifies all routes at given time on fib after each route is added*/ 
#define _VERIFY_STYLE_ITERATIVE  (2)
/* adds all routes, verifies them all at the end on single iteration */
#define _VERIFY_STYLE_BULK       (3)

int taps_capacity_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE, index = 0, fibcnt = 0, vindex = 0, slave_idx = 0, slave_unit;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    int num_routes = 0;
    int style = _VERIFY_STYLE_BULK; /* _VERIFY_STYLE_LAST_ROUTE;  _VERIFY_STYLE_ITERATIVE */
    uint32 key_base = _KEY_BASE_;
    uint32 flags = TAPS_DUMP_TCAM_HW_PIVOT | TAPS_DUMP_TCAM_HW_BKT |
            TAPS_DUMP_SRAM_HW_PIVOT | TAPS_DUMP_SRAM_HW_BKT |
            TAPS_DUMP_DRAM_HW_PFX | TAPS_DUMP_DRAM_HW_BKT;

    /* Get the number of routes to run from input parameters */
    if (testInfo->testNumRoutes > 0) {
    num_routes = testInfo->testNumRoutes;

    }

    if(num_routes == 0)
    {
        cli_out(" Exiting Test with no routes\n");

    return SOC_E_NONE;
    }

    if (num_routes < 100) {
        style = _VERIFY_STYLE_ITERATIVE;
    } else if (num_routes < 1000) {
        style = _VERIFY_STYLE_LAST_ROUTE;
    } else {
        style = _VERIFY_STYLE_BULK;
    }

    cli_out("Test VRF = 0x%x Key-Base = 0x%x Number Routes 0x%x\n", 
            vrf_base, key_base,num_routes);

    _key[0] = 0;
    _key[1] = vrf_base;

    /* insert vpn default route */
    arg->taps = taps_test_param[testInfo->testTapsInst].taps;
    arg->key = &_key[0];
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    _key_pyld[0] = fibcnt++;
    arg->payload = &_key_pyld[0];

    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    /* Avoid race condition with autocache service thread prior to verification */
    rv = taps_flush_cache(unit);
    if (SOC_FAILURE(rv)) {
        cli_out("unit %d taps_flush_cache Flush failed err=%d\n",unit, rv);
        return SOC_E_FAIL;
    }

    if (_IS_MASTER_SHARE_LPM_TABLE(unit, arg->taps->master_unit, arg->taps->param.host_share_table)) {
        while(slave_idx < arg->taps->num_slaves) {
            slave_unit = arg->taps->slave_units[slave_idx];
            rv = taps_flush_cache(unit);
            if (SOC_FAILURE(rv)) {
                cli_out("unit %d taps_flush_cache Flush failed err=%d\n",slave_unit, rv);
                return SOC_E_FAIL;
            }
            ++slave_idx;
        }
    }
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vpn def route !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                    arg->taps->master_unit, 
                                                    arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        return SOC_E_FAIL;
    }

    /* get and verify default vrf route */
    rv = taps_verify_pfx(unit, arg, TRUE);
    if (SOC_SUCCESS(rv)) {
        taps_dump(unit, arg->taps, flags);
        tmu_taps_ut_test_result = 0;
    } else {
        taps_dump(unit, arg->taps, flags);
        cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    LOG_DEBUG(BSL_LS_APPL_COMMON,
              (BSL_META_U(unit,
                          "## \nAdding 0x%x routes in FIB \n"),
               num_routes));

    TIME_STAMP_START
    /* insert a route onto the vpn */
    /* 10.0.0.1/32 */
    tmu_taps_ut_test_result = 0;

    for (index = 0; index < num_routes; index++) {

        _key[0] = vrf_base;
        _key[1] = key_base + fibcnt;
        arg->taps = taps_test_param[testInfo->testTapsInst].taps;
        arg->key = &_key[0];
        arg->length = TAPS_IPV4_KEY_SIZE;
        _key_pyld[0] = fibcnt;
        arg->payload = _key_pyld;

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to insert route 0x%x 0x%x route !!!\n",
                    _key[0], _key[1]);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }
#if 0
        else {
            cli_out("\n... Inserted route:  0x%x 0x%x \n", _key[0], _key[1]);
        }
#endif

        fibcnt++;

        if (_VERIFY_STYLE_LAST_ROUTE == style) {
            /* get and verify */
            rv = taps_verify_pfx(unit, arg, TRUE);
            taps_tcam_dump(unit, arg->taps->tcam_hdl, testInfo->testVerbFlag);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }
        } else if (_VERIFY_STYLE_ITERATIVE == style) {

            for (vindex = 1; vindex < fibcnt; vindex++) {
                _key[0] = vrf_base;
                _key[1] = key_base + vindex;
                arg->taps = taps_test_param[testInfo->testTapsInst].taps;
                arg->key = &_key[0];
                arg->length = TAPS_IPV4_KEY_SIZE;
                _key_pyld[0] = vindex;
                arg->payload = _key_pyld;

                /* get and verify */
                rv = taps_verify_pfx(unit, arg, TRUE);
                if (SOC_FAILURE(rv)) {
                    cli_out("############ DUMP ###############\n");
                    taps_dump(unit, arg->taps,
                            TAPS_DUMP_TCAM_SW_BKT | TAPS_DUMP_TCAM_SW_PIVOT);
                    cli_out("############ DUMP ###############\n");

                    taps_dump(unit, arg->taps, testInfo->testVerbFlag);
                    cli_out(
                            "\n!!!! Failed to verify route:%d Fib-count:%d !!!\n",
                            vindex, fibcnt);
                    tmu_taps_ut_test_result = -1;
                    return SOC_E_FAIL;
                }
            }
        }

        if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Added %d(K) routes to FIB \n"),
                       fibcnt / 1024));
        }
    }


    /* Avoid race condition with autocache service thread prior to verification */
   rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                arg->taps->master_unit,
                                                arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        return SOC_E_FAIL;
    }

    TIME_STAMP("### Total Route Update time");
    TIME_STAMP_RATE("### Average Route Update rate", fibcnt);

    TIME_STAMP_START

    if (_VERIFY_STYLE_BULK == style) {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "## \nVerifying %d(K) routes in FIB \n"),
                   (fibcnt - 1) / 1024));
        for (vindex = 1; vindex < fibcnt; vindex++) {
            _key[0] = vrf_base;
            _key[1] = key_base + vindex;
            arg->taps = taps_test_param[testInfo->testTapsInst].taps;
            arg->key = &_key[0];
            arg->length = TAPS_IPV4_KEY_SIZE;
            _key_pyld[0] = vindex;
            arg->payload = _key_pyld;

            /* get and verify */
            rv = taps_verify_pfx(unit, arg, TRUE);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, testInfo->testVerbFlag);
                cli_out(
                        "\n!!!! Failed to verify route:%d Fib-count:%d !!!\n",
                        vindex, fibcnt);
                tmu_taps_ut_test_result = -1;
                return SOC_E_FAIL;
            }

            if (vindex % 1024 == 0)
                LOG_DEBUG(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "## Verified %d(K) routes in FIB \n"),
                           vindex / 1024));

        }
    }


    cli_out("\n Test Passed \n");

    TIME_STAMP("### Total verification time")
    TIME_STAMP_RATE("### Average Route Verification rate", fibcnt);

    if(testInfo->testVerbFlag)
        taps_dump(unit, arg->taps,testInfo->testVerbFlag );

    return rv;
}

/* 0-99, each represent 1 percent of distribution,
 * number in there represent the key length not including
 * the vrf. For example, for ipv4, value should be 1 to 32
 */
static uint32 _key_length_distribution[100];

/* incremental (by 1) key */
#define _TAPS_KEY_INCREMENTAL (0)
/* random key */
#define _TAPS_KEY_RANDOM      (1)

/* key length distribution
 * if between 1-32, fixed key length specified by the mode
 * if following defines, used prefined length distribution
 */
#define _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN (1)
#define _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MAX (32)
#define _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_IPV6_MAX (128)
#define _TAPS_DISTRIBUTION_MODE_PREDEFINED_1  (-1)
#define _TAPS_DISTRIBUTION_MODE_PREDEFINED_2  (-2)

typedef struct _keylength_encode_s {
    uint32 key_len;         /* IP addr len*/
    uint32 encode_value;    /* Encode the key_len to a value, such as /b00, /b01,/b10 */
    uint32 bits_num;        /* How many bits used to encode this key_len*/
} keylength_encode_t;

/* offset of Distriubution length. The base value is 16 */
#define DIST_LEN_OFFSET(len) ((len) - 16)

/* Distribution length range: 16 -32 */  
#define DIST_LEN_RANGE 17


static keylength_encode_t encode_array[DIST_LEN_RANGE];
static uint32 base_addr[DIST_LEN_RANGE];
static uint32 distlen_counter[DIST_LEN_RANGE];
static uint32 distlen_max_routes[DIST_LEN_RANGE];

int taps_get_random_vrf(taps_vrf_param_t *vrf_info, int vrf_count)
{
    int matched, index;
    int vrf;
    
    /* randomize vrf and make sure it doesn't match existing one */
    while (1) {
        matched = FALSE;
        vrf = sal_rand() & 0x3FF;
        for (index = 0; index < vrf_count; index++) {
            if (vrf_info[index].vrf == vrf) {
                matched = TRUE;
                break;
            }
        }

        if (matched == FALSE) {
            break;
        }
    }
    return vrf;
}

int taps_create_default_route(int unit, int instance, int vrf, int *p_fibcnt)
{
    int rv = SOC_E_NONE, fibcnt = *p_fibcnt;
    taps_arg_t *arg = &taps_test_param[instance].arg;
    _key[0] = 0;
    _key[1] = vrf;

    arg->taps = taps_test_param[instance].taps;
    arg->key = &_key[0];
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    _key_pyld[0] = fibcnt++;
    _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
    
    arg->payload = &_key_pyld[0];
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vpn def route: %s !!!\n",soc_errmsg(rv));
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    *p_fibcnt = fibcnt;
    return rv;
}
    
int taps_get_one_route_key(taps_route_param_t *routes, int vrf,
                        int key_pattern_mode, int distribution_mode)

{
    uint32 low_bits, high_bits,middle_bits;
    uint32 ipaddr_len;
    uint32 ipaddr;
    uint32 cnt_idx;
    int    generate_success = FALSE;

    while(generate_success == FALSE) {
        if ((distribution_mode >= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN) &&
            (distribution_mode <= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MAX)) {
            /* fixed key length distribution */
            ipaddr_len = _key_length_distribution[0];
        } else {
            while(1) {
                ipaddr_len = sal_rand() % 100; /* random number between 0-99 */
                ipaddr_len = _key_length_distribution[ipaddr_len]; /* get true length based on distribution */
                
                if (distlen_counter[DIST_LEN_OFFSET(ipaddr_len)] <= distlen_max_routes[DIST_LEN_OFFSET(ipaddr_len)]) {
                    break;
                }
            }
        }
        
        /* if ipaddr_len == 0, try again */
        if (ipaddr_len == 0) {
            continue;
        }
        
        if (key_pattern_mode == _TAPS_KEY_INCREMENTAL) {
            /* incremental */
            ipaddr = ++base_addr[DIST_LEN_OFFSET(ipaddr_len)];
        } else if (key_pattern_mode == _TAPS_KEY_RANDOM) {
            /* random prefix */
            base_addr[DIST_LEN_OFFSET(ipaddr_len)] += (sal_rand()%4 + 1);
            ipaddr = base_addr[DIST_LEN_OFFSET(ipaddr_len)];
        } else {
            /* not supported */
            cli_out("\n!!!! Unsupported key pattern mode %d !!!\n", key_pattern_mode);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;      
        }
        
        /* align the keys by shifting */
        if (ipaddr_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes->keys[0] = 0;
        } else {
            routes->keys[0] = (vrf >> (32 - ipaddr_len));
        }
        
        if (ipaddr_len == 32) {
            routes->keys[1] = ipaddr;
        } else {
            routes->keys[1] = (ipaddr & ((1 << ipaddr_len)-1)) | \
                ((vrf & ((1<<(32-ipaddr_len))-1)) << ipaddr_len);
        }
        routes->key_length =  TAPS_IPV4_MAX_VRF_SIZE + ipaddr_len;

        /* Find the encode entry for a specific len */
        for (cnt_idx = 0; cnt_idx < DIST_LEN_RANGE; cnt_idx++) {
            if(encode_array[cnt_idx].key_len == DIST_LEN_OFFSET(routes->key_length)){
                break;
            }
        }
        if (cnt_idx == DIST_LEN_RANGE) {
            cli_out("%s : Fail to generate a key\n",FUNCTION_NAME());
            return SOC_E_FAIL;
        }
        /* Get the lower (key_len - bits_num) bits in routes[fibcnt].keys[1]*/
        low_bits = routes->keys[1] & ((1<< (encode_array[cnt_idx].key_len - encode_array[cnt_idx].bits_num)) - 1);

        /* Get the higher (32 - key_len) bits in routes[fibcnt].keys[1] */
        high_bits = routes->keys[1] & ~((1<< encode_array[cnt_idx].key_len) - 1);

        /* Set encode value to instead of (bits_num) bits in routes[fibcnt].keys[1]  */
        middle_bits = (encode_array[cnt_idx].encode_value >> (32 - encode_array[cnt_idx].key_len)) & ((1<< encode_array[cnt_idx].key_len) -1);

        routes->keys[1] = high_bits | middle_bits | low_bits;
        
        distlen_counter[DIST_LEN_OFFSET(ipaddr_len)]++;
        generate_success = TRUE;
    }

    return SOC_E_NONE;
}

/*
* IPv4 mix insert/delete
* 1. Insert vrf default routes and loopback routes
* 2. Insert routes with random value&len
* 3. Verify routes
* 4. Delete routes and verify routes.
* 5. Repeate step 2 - 4 for 10 times.
*/
int taps_ipv4_random_access_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, 
        route_idx, vrf_idx, shuffle_route_idx, access_times, access_num;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t shuffle_route;
    taps_route_param_t *routes = NULL;
    int dump_flags;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    access_times = 0;
    access_num = 1;
    netmask_start = 16;
    
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n", record_mem, record_mem_max);
#endif

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* add 127.0.0.0/8 to match customer test case */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0xF0000000; /* mark this payload as a 127.0.0.0/8 route payload */
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    
    for(access_times = 0; access_times < access_num; access_times++){
        

        route_idx = 0;
        fibcnt = 0;
        single_vrf = (sal_rand() % 2 == 0);
        vrf_count = sal_rand() % 4096;
        sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);
        
        /****************************************************
            *Generate and Insert routes with random value&length
            ****************************************************/
        while (TRUE) {
            if (route_idx >= num_routes) {
                break;
            }
            /* Generate route */
            /* Random IP length between 16-32*/
            netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

            /* IP value */
            ipaddr = V4_RAND();

            /* Vrf value */
            if (single_vrf) {
                vrf = vrf_count;
            } else {
                vrf = sal_rand() % vrf_count;
            }

            /* align the keys by shifting */
            if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
                routes[route_idx].keys[0] = 0;
            } else {
                routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
            }
            
            if (netmask_len == 32) {
                routes[route_idx].keys[1] = ipaddr;
            } else {
                routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                    ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
            }
            routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;
                

            /* Construct match_key */
            sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                    sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
            taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                        routes[route_idx].key_length, 
                        routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
            routes[route_idx].payload[0] = route_idx;     

            /* Try to get this route */
            arg->length = routes[route_idx].key_length;
            arg->key = &routes[route_idx].keys[0];
            arg->payload= &routes[route_idx].payload[0];

            rv = taps_insert_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                if (rv == SOC_E_EXISTS) {
                    /* Exist route, then try to get another one */
                    continue;
                } else {
                    cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                            soc_errmsg(rv));
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }

            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
            }
            route_idx++;
        }
        cli_out("##%d time access used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
                access_times, arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
                num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
        /****************************************************
            *Verify routes
            ****************************************************/
        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            /* Should find out the bpm route */
            rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! Verified %d(K) routes after insert \n", route_idx/1024);
            }
        }

        /* To delete with random sequence, we need to shuffle routes */
        /* shuffle routes to create random route insert pattern */
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            shuffle_route_idx = sal_rand() % num_routes;
            if (shuffle_route_idx != route_idx) {
                /* swap the key/length of 2 routes */
                sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
            }
        }

       /****************************************************
            *Delete routes
            ****************************************************/
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            arg->length = routes[route_idx].key_length;
            arg->key = &routes[route_idx].keys[0];
            arg->payload= &routes[route_idx].payload[0];
            rv = taps_delete_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to Delete routes "
                        "key 0x%x 0x%x length %d !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            } else {
                /* Get routes, should return not found */
                rv = taps_get_route(unit, arg);
                if (rv != SOC_E_NOT_FOUND) {
                    cli_out("\n!!!! Failed to Delete routes "
                            "key 0x%x 0x%x length %d : return %d!!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], 
                            routes[route_idx].key_length, rv);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                } 
                rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                    arg->taps->master_unit, 
                                                    arg->taps->param.host_share_table));
                if (SOC_FAILURE(rv)) {
                    cli_out("taps_flush_cache Flush failed err=%d\n", rv);
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
                /* Should find out the bpm route */
                rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
                if (SOC_FAILURE(rv)) {
                    taps_dump(unit, arg->taps, dump_flags);
                    cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! Deleted %d(K) routes\n", route_idx/1024);
            }
        }
    }
    
    /****************************************************
        *Delete loopback route and vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        /* Delete 127.0.0.0/8 */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
   
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL; 
    }
#endif    
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

/*
* IPv4 traverse test
* 1. Insert vrf default routes and loopback routes
* 2. Insert routes with random value&len, netmask is between 16 to 32
* 3. Traverse and try to get each route to see if there is someone can't be found.
*/
int taps_ipv4_traverse_route_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, 
        route_idx, vrf_idx, traverse_num;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS], next_key[TAPS_MAX_KEY_SIZE_WORDS],
        key_length, next_key_length;
    int dump_flags;

    sal_memset(key, 0, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
    sal_memset(next_key, 0, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;
    traverse_num = 0;

    sal_srand(prefixSeed);

    netmask_start = 16;
    single_vrf = sal_rand() % 2;
    vrf_count = sal_rand() % 4096;
   
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Generate  and Insert routes with random value&length
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }
        /* align the keys by shifting */
        if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes[route_idx].keys[0] = 0;
        } else {
            routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
        }
        
        if (netmask_len == 32) {
            routes[route_idx].keys[1] = ipaddr;
        } else {
            routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
        }
        routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;
            

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
        routes[route_idx].payload[0] = route_idx;     

        /* Try to get this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }

    cli_out("##used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
            arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
            num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
    
    /****************************************************
        *Traverse and verify each routes
        ****************************************************/

    /* Get first key */
    rv = taps_iterator_first(unit, arg, &key[0], &key_length);
    if (SOC_FAILURE(rv)) {
        cli_out("Fail in taps_iterator_first \n");
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return rv;
    }
    /* Shift key from g3p1 format to driver format */
    rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, key,
              TAPS_IPV4_KEY_SIZE, TAPS_IPV4_KEY_SIZE - key_length);
    if (SOC_FAILURE(rv)) { 
        cli_out("\n!!!! Failed to shift first route 0x%x 0x%x length %d!!!\n",
                key[0], key[1], key_length);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return rv;
    }
    /* Verify first key */
    arg->key = &key[0];
    arg->length = key_length;
    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        taps_dump(unit, arg->taps, dump_flags);
        cli_out("\n!!!! Failed to get route 0x%x 0x%x length %d!!!\n",
                routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    traverse_num++;
    /* Loop to get and verify all the routes*/
    do {
        arg->key = &key[0];
        arg->length = key_length;
        rv = taps_iterator_next(unit, arg, &next_key[0], &next_key_length);
        if (SOC_SUCCESS(rv)) {
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, next_key,
              TAPS_IPV4_KEY_SIZE,TAPS_IPV4_KEY_SIZE - next_key_length);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to shift next route 0x%x 0x%x length %d!!!\n",
                        next_key[0], next_key[1], next_key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return rv;
            }
            /* Get the next key */
            arg->key = &next_key[0];
            arg->length = next_key_length;
            rv = taps_get_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to get route 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            sal_memcpy(key, next_key, TAPS_MAX_KEY_SIZE_WORDS * sizeof(uint32));
            key_length = next_key_length;
            traverse_num++;
        } else if (rv == SOC_E_NOT_FOUND) {
            /* No more key */
        } else {
            /* Error happen */
            cli_out("Fail in taps_iterator_next \n");
            break;
        }
    } while (rv != SOC_E_NOT_FOUND);

    if (traverse_num != num_routes + 4096) {
        rv = SOC_E_FAIL;
        cli_out("\n##ERROR! Miss to traverse %d routes \n", num_routes + 4096 - traverse_num);
    } else {
        rv = SOC_E_NONE;
        cli_out("\n##Success to traverse all %d routes \n", traverse_num);
    }
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

/*
* IPv6 traverse test
* 1. Insert vrf default routes
* 2. Insert routes with random value&len, netmask is between 32 to 128
* 3. Traverse and try to get each route to see if there is someone can't be found.
*/
int taps_ipv6_traverse_route_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, num_routes, route_idx, 
        vrf_idx, traverse_num, single_vrf, vrf_count, netmask_start;
    int netmask_len;
    uint32 ipaddr[4], vrf, prefixSeed;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS], next_key[TAPS_MAX_KEY_SIZE_WORDS],
        key_length, next_key_length;
    int dump_flags;

    traverse_num = 0;
    netmask_start = 32;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;
    sal_memset(key, 0, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
    sal_memset(next_key, 0, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;

    sal_srand(prefixSeed);
    single_vrf = sal_rand() % 2;
    vrf_count = sal_rand() % 4096;

    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);


    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Insert routes
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV6_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr[0] = V4_RAND();
        ipaddr[1] = V4_RAND();
        ipaddr[2] = V4_RAND();
        ipaddr[3] = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        routes[route_idx].keys[0] = vrf;
        routes[route_idx].keys[1] = ipaddr[0];
        routes[route_idx].keys[2] = ipaddr[1];
        routes[route_idx].keys[3] = ipaddr[2];
        routes[route_idx].keys[4] = ipaddr[3];
        routes[route_idx].key_length =  TAPS_IPV6_MAX_VRF_SIZE + netmask_len;
            
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].keys,
                            TAPS_IPV6_KEY_SIZE, 
                            TAPS_IPV6_PFX_SIZE - netmask_len);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        rv =  taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    netmask_len - TAPS_IPV6_PFX_SIZE);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        routes[route_idx].payload[0] = route_idx;     

        /* Insert this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x 0x%x 0x%x "
                        "0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                        routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }


    /****************************************************
        *Traverse and verify each routes
        ****************************************************/

    /* Get first key */
    rv = taps_iterator_first(unit, arg, &key[0], &key_length);
    if (SOC_FAILURE(rv)) {
        cli_out("Fail in taps_iterator_first \n");
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return rv;
    }
    /* Shift key from g3p1 format to driver format */
    rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, key,
              TAPS_IPV6_KEY_SIZE,TAPS_IPV6_KEY_SIZE - key_length);
    if (SOC_FAILURE(rv)) {
        cli_out("Fail to shift first key\n");
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return rv;
    }
    /* Verify first key */
    arg->key = &key[0];
    arg->length = key_length;
    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        taps_dump(unit, arg->taps, dump_flags);
        cli_out("\n!!!! Failed to get routes key 0x%x 0x%x 0x%x 0x%x "
                "0x%x length %d : %s !!!\n", 
                routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                soc_errmsg(rv));
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    traverse_num++;
    /* Loop to get and verify all the routes*/
    do {
        arg->key = &key[0];
        arg->length = key_length;
        rv = taps_iterator_next(unit, arg, &next_key[0], &next_key_length);
        if (SOC_SUCCESS(rv)) {
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, next_key,
              TAPS_IPV6_KEY_SIZE,TAPS_IPV6_KEY_SIZE - next_key_length);
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to shift next key\n");
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return rv;
            }
            /* Get the next key */
            rv = taps_get_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to get routes key 0x%x 0x%x 0x%x 0x%x "
                        "0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                        routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                        soc_errmsg(rv));
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            sal_memcpy(key, next_key, TAPS_MAX_KEY_SIZE_WORDS * sizeof(uint32));
            key_length = next_key_length;
            traverse_num++;
        } else if (rv == SOC_E_NOT_FOUND) {
            /* No more key */
        } else {
            /* Error happen */
            cli_out("Fail in taps_iterator_next \n");
            break;
        }
    } while (rv != SOC_E_NOT_FOUND);

    if (traverse_num != num_routes + 4096) {
        rv = SOC_E_FAIL;
        cli_out("\n##ERROR! Miss to traverse %d routes \n", num_routes + 4096 - traverse_num);
    } else {
        rv = SOC_E_NONE;
        cli_out("\n##Success to traverse all %d routes \n", traverse_num);
    }
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

int _taps_ut_dbucket_redistribution_clear_all(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE, route_idx;
    uint8 isbpm;
    taps_bucket_redistribution_clear_t *cbdata = (taps_bucket_redistribution_clear_t *)user_data;
    route_idx = *((int *)dph->cookie);
    
    rv = taps_dbucket_delete_prefix(cbdata->unit, 
                                 cbdata->taps, cbdata->wgroup,
                                 cbdata->sph->dbh, dph);
    if (SOC_FAILURE(rv)) {
        cli_out("Failed to delete dprefix %d\n", rv);
        return rv;
    }
    rv = taps_sbucket_propagate_prefix(cbdata->unit, cbdata->taps, 
                           cbdata->tph->sbucket, cbdata->wgroup,
                           cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                           _BRR_INVALID_CPE_, FALSE, &isbpm);
    if (SOC_FAILURE(rv)) {
        cli_out("Failed to sbucket propagate %d\n", rv);
        return rv;
    }
    rv = taps_tcam_propagate_prefix(cbdata->unit, cbdata->taps->tcam_hdl, cbdata->wgroup,
                             cbdata->tph, FALSE, -1,
                             cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                             &isbpm);
    if (SOC_FAILURE(rv)) {
        cli_out("Failed to tcam propagate %d\n", rv);
        return rv;
    }
    cbdata->routes[route_idx].key_length = 0;
    return SOC_E_NONE;
}
int _taps_ut_dbucket_redistribution_clear(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    uint8 isbpm;
    int trie0_count, trie1_count;
    int route_idx;
    taps_bucket_redistribution_clear_t *cbdata = (taps_bucket_redistribution_clear_t *)user_data;

    SHR_BITCOUNT_RANGE(cbdata->sph->dbh->pfx_bmap0, trie0_count, 0, cbdata->taps->param.dbucket_attr.num_dbucket_pfx);
    SHR_BITCOUNT_RANGE(cbdata->sph->dbh->pfx_bmap1, trie1_count, 0, cbdata->taps->param.dbucket_attr.num_dbucket_pfx);

    if (dph->length > cbdata->taps->param.key_attr.vrf_length
        && (trie0_count + trie1_count > 
        TAPS_DBUCKET_DIST_THRESHOLD(cbdata->taps->param.dbucket_attr.num_dbucket_pfx*2))) {
        route_idx = *((int *)dph->cookie);
        
        rv = taps_dbucket_delete_prefix(cbdata->unit, 
                                 cbdata->taps, cbdata->wgroup,
                                 cbdata->sph->dbh, dph);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to delete dprefix %d\n", rv);
            return rv;
        }
        rv = taps_sbucket_propagate_prefix(cbdata->unit, cbdata->taps, 
                           cbdata->tph->sbucket, cbdata->wgroup,
                           cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                           _BRR_INVALID_CPE_, FALSE, &isbpm);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to sbucket propagate %d\n", rv);
            return rv;
        }
        rv = taps_tcam_propagate_prefix(cbdata->unit, cbdata->taps->tcam_hdl, cbdata->wgroup,
                                 cbdata->tph, FALSE, -1,
                                 cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                                 &isbpm);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to tcam propagate %d\n", rv);
            return rv;
        }
        cbdata->routes[route_idx].key_length = 0;
    }
    return rv;
}

int _taps_ut_spivot_redistribution_clear_cb_for_offchip(taps_spivot_handle_t sph, void *user_data)
{
    int rv = SOC_E_NONE;
    int spivot_count, del_pivot_id;
    taps_work_type_e_t work_type;
    taps_bucket_redistribution_clear_t *cbdata = (taps_bucket_redistribution_clear_t *)user_data;
    SHR_BITCOUNT_RANGE(sph->sbh->pivot_bmap, spivot_count, 0, sph->sbh->prefix_number);
    cbdata->sph = sph;
    if (sph != sph->sbh->wsph 
        && sph->length != cbdata->taps->param.key_attr.vrf_length
        && spivot_count > TAPS_SBUCKET_DIST_THRESHOLD(sph->sbh->prefix_number)) {
        /* Remove the sph and the dbucket */
        rv = taps_dbucket_destroy_traverse(cbdata->unit, 
                               cbdata->taps, cbdata->wgroup,
                               sph->dbh, cbdata, 
                               _taps_ut_dbucket_redistribution_clear_all);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to clear dbucket %d\n", rv);
            return rv;
        }
        rv = taps_dbucket_destroy(cbdata->unit, cbdata->taps, cbdata->wgroup[cbdata->unit], sph->dbh);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to destroy wgroup %d\n", rv);
            return rv;
        }
        del_pivot_id = sph->index;
        rv = taps_sbucket_delete_pivot(cbdata->unit, cbdata->taps, cbdata->wgroup, sph->sbh, sph);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to delete spivot %d\n", rv);
            return rv;
        } 

        rv = taps_sbucket_pivot_id_free(cbdata->unit, cbdata->taps, sph->sbh, del_pivot_id);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to free spivot id %d\n", rv);
            return rv;
        }
    } else {
        rv = taps_dbucket_destroy_traverse(cbdata->unit, 
                               cbdata->taps, cbdata->wgroup,
                               sph->dbh, cbdata, 
                               _taps_ut_dbucket_redistribution_clear);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to clear dbucket %d\n", rv);
            return rv;
        }
    }
    work_type = TAPS_DBUCKET_WORK;
    rv = taps_work_commit(cbdata->unit, cbdata->wgroup[cbdata->unit], &work_type, 1, _TAPS_BULK_COMMIT);
    work_type = TAPS_SBUCKET_WORK;
    rv = taps_work_commit(cbdata->unit, cbdata->wgroup[cbdata->unit], &work_type, 1, _TAPS_BULK_COMMIT);
    work_type = TAPS_TCAM_WORK;
    rv = taps_work_commit(cbdata->unit, cbdata->wgroup[cbdata->unit], &work_type, 1, _TAPS_BULK_COMMIT);
    return rv;
}

int _taps_ut_spivot_redistribution_clear_cb_for_onchip(taps_spivot_handle_t sph, void *user_data)
{
    int rv = SOC_E_NONE, route_idx;
    int spivot_count, del_pivot_id;
    uint8 isbpm;
    taps_work_type_e_t work_type;
    taps_bucket_redistribution_clear_t *cbdata = (taps_bucket_redistribution_clear_t *)user_data;
    SHR_BITCOUNT_RANGE(sph->sbh->pivot_bmap, spivot_count, 0, sph->sbh->prefix_number);
    if (sph->length > cbdata->taps->param.key_attr.vrf_length
        && spivot_count > TAPS_SBUCKET_DIST_THRESHOLD(sph->sbh->prefix_number)) {
        route_idx = *((int *)sph->cookie);
        
        del_pivot_id = sph->index;
    
        rv = taps_sbucket_delete_pivot(cbdata->unit, cbdata->taps, cbdata->wgroup, sph->sbh, sph);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to delete spivot %d\n", rv);
        } 

        rv = taps_sbucket_pivot_id_free(cbdata->unit, cbdata->taps, sph->sbh, del_pivot_id);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to free spivot id %d\n", rv);
        }
        if (cbdata->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        /* For mode one, we need to do propagation in tcam */
            rv = taps_tcam_propagate_prefix(cbdata->unit, cbdata->taps->tcam_hdl, cbdata->wgroup,
                     cbdata->tph, FALSE, -1,
                     cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                     &isbpm);
        } else {
            /* For mode zero, just update the asso data */
            rv = taps_tcam_propagate_prefix_for_modezero(cbdata->unit, cbdata->taps->tcam_hdl, 
                     cbdata->wgroup, cbdata->tph, FALSE,
                     cbdata->routes[route_idx].keys, cbdata->routes[route_idx].key_length,
                     cbdata->routes[route_idx].payload);
        }
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to do tcam propagation %d\n", rv);
        }
        work_type = TAPS_SBUCKET_WORK;
        rv = taps_work_commit(cbdata->unit, cbdata->wgroup[cbdata->unit], &work_type, 1, _TAPS_BULK_COMMIT);
        work_type = TAPS_TCAM_WORK;
        rv = taps_work_commit(cbdata->unit, cbdata->wgroup[cbdata->unit], &work_type, 1, _TAPS_BULK_COMMIT);

        cbdata->routes[route_idx].key_length = 0;
    }

    return rv;
}

int _taps_ut_sbucket_redistribution_clear(taps_tcam_pivot_handle_t tph, void *user_data)
{
    taps_bucket_redistribution_clear_t *cbdata = (taps_bucket_redistribution_clear_t *)user_data;
    cbdata->tph = tph;
    if (cbdata->taps->param.mode == TAPS_OFFCHIP_ALL) {
        taps_sbucket_destroy_traverse(cbdata->unit, cbdata->taps, cbdata->wgroup,
                      tph->sbucket, cbdata, _taps_ut_spivot_redistribution_clear_cb_for_offchip);
    } else {
        taps_sbucket_destroy_traverse(cbdata->unit, cbdata->taps, cbdata->wgroup,
                      tph->sbucket, cbdata, _taps_ut_spivot_redistribution_clear_cb_for_onchip);
    }
    return SOC_E_NONE;
}
/* Clear the sbucket/dbucket to less than 5% fill to satisfy the condition of bucket redistribution */
int tasp_bucket_redistribution_clear(int unit, taps_handle_t taps,
                                            taps_route_param_t *routes)
{
    int rv;
    taps_wgroup_handle_t wgroup[SOC_MAX_NUM_DEVICES];
    taps_bucket_redistribution_clear_t cbdata;
    /* create work group */
    rv = taps_work_group_create_for_all_devices(unit, taps, taps->wqueue, 
                                _TAPS_DEFAULT_WGROUP_,
                                taps->param.host_share_table,
                                wgroup);
    if (SOC_FAILURE(rv)) {
        cli_out("Failed to create wgroup %d\n", rv);
        return rv;
    }    

    cbdata.unit = unit;
    cbdata.taps = taps;
    cbdata.wgroup = &wgroup[0];
    cbdata.routes = routes;
    taps_tcam_traverse(unit, taps, taps->tcam_hdl, &cbdata, _taps_ut_sbucket_redistribution_clear);
    
    rv = taps_work_group_destroy_for_all_devices(unit, taps, 
                                wgroup, taps->param.host_share_table);
    if (SOC_FAILURE(rv)) {
        cli_out("Failed to destory wgroup %d\n", rv);
    }
    return rv;
}

int taps_ipv4_bucket_redist_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, 
        route_idx, vrf_idx, valid_num;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    int *cookie_idx = NULL;
    int dump_flags;
    int use_count;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    netmask_start = 16;
    
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    cookie_idx = sal_alloc(sizeof(int) * num_routes, "cookie info");

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n", record_mem, record_mem_max);
#endif

    single_vrf = 1;
    vrf_count = 1;
    vrf = 0;

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < vrf_count; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */            
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
    }

    route_idx = 0;
    fibcnt = 0;
    
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);
    
    /****************************************************
        *Generate and Insert routes with random value&length
        ****************************************************/
    while (TRUE) {
        if (route_idx >= num_routes) {
            rv = tasp_bucket_redistribution_clear(unit, arg->taps, routes); 
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to clear taps for redistribution %d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            use_count = arg->taps->tcam_hdl->use_count;
            rv = taps_host_bucket_redist(unit, arg->taps);
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to redist bucket %d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            cli_out("\n Removed %d tcam entry during bucket redistribution\n", 
                    use_count - arg->taps->tcam_hdl->use_count);
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = 0;
        } else {
            /* coverity[dead_error_line] */
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes[route_idx].keys[0] = 0;
        } else {
            routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
        }
        
        if (netmask_len == 32) {
            routes[route_idx].keys[1] = ipaddr;
        } else {
            routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
        }
        routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
        routes[route_idx].payload[0] = route_idx;     

        /* Try to get this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        cookie_idx[route_idx] = route_idx;
        arg->cookie = (void *)&cookie_idx[route_idx];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
        }

        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }
    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        if (cookie_idx) sal_free(cookie_idx);
        return SOC_E_FAIL;
    }
    valid_num = 0;
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        if (routes[route_idx].key_length == 0) {
            continue;
        }
        rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
        valid_num++;
    }

    cli_out("!!! Verified %d routes after insert \n", valid_num);
    valid_num = 0;
   /****************************************************
        *Delete routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        if (routes[route_idx].key_length == 0) {
            continue;
        }
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Delete routes "
                    "key 0x%x 0x%x length %d !!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        } else {
            /* Get routes, should return not found */
            rv = taps_get_route(unit, arg);
            if (rv != SOC_E_NOT_FOUND) {
                cli_out("\n!!!! Failed to Delete routes "
                        "key 0x%x 0x%x length %d : return %d!!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], 
                        routes[route_idx].key_length, rv);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            } 
            rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                arg->taps->master_unit, 
                                                arg->taps->param.host_share_table));
            if (SOC_FAILURE(rv)) {
                cli_out("taps_flush_cache Flush failed err=%d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            /* Should find out the bpm route */
            rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
        }
        valid_num++;
    }
    cli_out("!!! Deleted %d routes \n", valid_num);
    
    /****************************************************
        *Delete loopback route and vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < vrf_count; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
    }

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        if (routes) sal_free(routes);
        if (cookie_idx) sal_free(cookie_idx);
        return SOC_E_FAIL; 
    }
#endif    

    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    if (cookie_idx) sal_free(cookie_idx);
    return rv;
}

int taps_ipv6_bucket_redist_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, 
      route_idx, vrf_idx, shuffle_route_idx, access_times, access_num COMPILER_ATTRIBUTE((unused)), valid_num;
    int netmask_len;
    uint32 ipaddr[4], vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t shuffle_route;
    taps_route_param_t *routes = NULL;
    int dump_flags;
    int use_count;
    int *cookie_idx = NULL;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    access_times = 0;
    access_num = 1;
    netmask_start = 32;
    
    single_vrf = 1;
    vrf_count = 1;
    vrf = 0;
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* Set up the IPv6 keyploder to use this taps table */
    rv=taps_ut_setup_ipv6_keyploder(unit, arg);    
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to setup IPv6 keyploder !!!\n");
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    cookie_idx = sal_alloc(sizeof(int) * num_routes, "cookie info");
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n", record_mem, record_mem_max);
#endif

    /****************************************************
        *Insert vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < vrf_count; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
    }

    route_idx = 0;
    fibcnt = 0;
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);
    
    /****************************************************
        *Generate and Insert routes with random value&length
        ****************************************************/
    while (TRUE) {
        if (route_idx >= num_routes) {
            rv = tasp_bucket_redistribution_clear(unit, arg->taps, routes); 
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to clear taps for redistribution %d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            use_count = arg->taps->tcam_hdl->use_count;
            rv = taps_host_bucket_redist(unit, arg->taps);
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to redist bucket %d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            cli_out("\n Removed %d tcam entry during bucket redistribution\n",
                    use_count - arg->taps->tcam_hdl->use_count);
            num_routes = route_idx;
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV6_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr[0] = V4_RAND();
        ipaddr[1] = V4_RAND();
        ipaddr[2] = V4_RAND();
        ipaddr[3] = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = 0;
        } else {
            /* coverity[dead_error_line] */
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        routes[route_idx].keys[0] = vrf;
        routes[route_idx].keys[1] = ipaddr[0];
        routes[route_idx].keys[2] = ipaddr[1];
        routes[route_idx].keys[3] = ipaddr[2];
        routes[route_idx].keys[4] = ipaddr[3];
        routes[route_idx].key_length =  TAPS_IPV6_MAX_VRF_SIZE + netmask_len;
            
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].keys,
                            TAPS_IPV6_KEY_SIZE, 
                            TAPS_IPV6_PFX_SIZE - netmask_len);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        rv =  taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    netmask_len - TAPS_IPV6_PFX_SIZE);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
        routes[route_idx].payload[0] = route_idx;     

        /* Insert this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        cookie_idx[route_idx] = route_idx;
        arg->cookie = (void *)&cookie_idx[route_idx];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x 0x%x 0x%x "
                        "0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                        routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }
    cli_out("##%d time access used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
            access_times, arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
            num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        if (cookie_idx) sal_free(cookie_idx);
        return SOC_E_FAIL;
    }
    valid_num = 0;
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        if (routes[route_idx].key_length == 0) {
            continue;
        }
        
        /* Should find out the bpm route */
        rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
        valid_num++;
    }
    cli_out("!!! Verified %d routes after insert \n", valid_num);

    /* To delete with random sequence, we need to shuffle routes */
    /* shuffle routes to create random route insert pattern */
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        shuffle_route_idx = sal_rand() % num_routes;
        if (shuffle_route_idx != route_idx) {
            /* swap the key/length of 2 routes */
            sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
        }
    }
    valid_num = 0;
   /****************************************************
        *Delete routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        if (routes[route_idx].key_length == 0) {
            continue;
        }
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Delete routes 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        } else {
            /* Get routes, should return not found */
            rv = taps_get_route(unit, arg);
            if (rv != SOC_E_NOT_FOUND) {
                cli_out("\n!!!! Failed to get routes 0x%x 0x%x 0x%x 0x%x 0x%x "
                        "length %d. return %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1],
                        routes[route_idx].keys[2], routes[route_idx].keys[3],
                        routes[route_idx].keys[4], routes[route_idx].key_length, rv);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            } 
            
            rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                arg->taps->master_unit, 
                                                arg->taps->param.host_share_table));
            if (SOC_FAILURE(rv)) {
                cli_out("taps_flush_cache Flush failed err=%d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
            /* Should find out the bpm route */
            rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x "
                        "0x%x 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1], 
                        routes[route_idx].keys[2], routes[route_idx].keys[3], 
                        routes[route_idx].keys[4], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                if (cookie_idx) sal_free(cookie_idx);
                return SOC_E_FAIL;
            }
        }
        valid_num++;
    }
    cli_out("!!! Deleted %d routes\n", valid_num);
    /****************************************************
        *Delete vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < vrf_count; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            if (cookie_idx) sal_free(cookie_idx);
            return SOC_E_FAIL;
        }
    }
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        if (cookie_idx) sal_free(cookie_idx);
        return SOC_E_FAIL; 
    }
#endif   

    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    if (cookie_idx) sal_free(cookie_idx);
    return rv;
}

int taps_bpm_test_run(int unit, void *data)
{   
    int rv;
    int index;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    int vrf=0x1234;
    taps_route_param_t route;

    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    arg->key = &_key[0];
    _key[0] = 0;
    _key[1] = vrf;
    arg->length = TAPS_IPV4_MAX_VRF_SIZE;
    _key_pyld[0] = 0; /* mark this payload as a vrf route payload */
    arg->payload = &_key_pyld[0];
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    _key[0] = 0;
    _key[1] = (vrf << 16) + 0x1010;
    _key_pyld[0] = 0xfff;
    arg->length = 32;
    arg->payload = &_key_pyld[0];
    rv = taps_insert_route(unit, arg);
    if (SOC_SUCCESS(rv)) {
        tmu_taps_ut_test_result = 0;
    } else {
        cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    for (index=0; index<testInfo->testNumRoutes; index++,index++) {
        _key[0] = vrf;
        _key[1] = 0x10100000+index;
        _key_pyld[0] = index+100;
        arg->length = TAPS_IPV4_KEY_SIZE;
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;
        }
    }
    for (index=0; index<testInfo->testNumRoutes*2; index++) {
        _key[0] = vrf;
        _key[1] = 0x10100000+index;
        _key_pyld[0] = index+100;
        arg->length = TAPS_IPV4_KEY_SIZE;
        arg->payload = &_key_pyld[0];
        
        route.keys[0] = _key[0];
        route.keys[1] = _key[1];
        /* Set the key_length to  TAPS_IPV4_KEY_SIZE */
        route.key_length = TAPS_IPV4_KEY_SIZE;
        
        sal_memcpy(&route.match_key[0], &route.keys[0], 
                sizeof(uint32) * TAPS_IPV4_KEY_SIZE_WORDS);
        rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, route.match_key,
                    route.key_length, route.key_length-TAPS_IPV4_KEY_SIZE);
        if (SOC_FAILURE(rv)) {    
            cli_out("\n!!!! Failed to shift key !!!\n");
            return rv;  
        }
        rv = taps_verify_bpm(unit, arg, &route);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to verify bpm !!!\n");
            return SOC_E_FAIL;
        }
    }
    return rv;
}

int taps_random_capacity_test_run(int unit, void *data)
{
    int num_routes;                                  /* number of routes */
    int style = _VERIFY_STYLE_BULK;                  /* ucode verify style */
    uint32 prefixSeed;
    int distribution_mode = 32;                      /* key length distribution mode, see above */
    int key_pattern_mode = _TAPS_KEY_INCREMENTAL;    /* key pattern mode, see above */
    int single_vrf = TRUE;                           /* if TRUE, all prefixes on single vrf, otherwise random vrf */
    int shuffle = FALSE;                             /* if TRUE, shuffle the IP address randomly */
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    /* dump flags 
    uint32 flags = TAPS_DUMP_TCAM_SW_BKT | TAPS_DUMP_TCAM_SW_PIVOT | \
                   TAPS_DUMP_SRAM_SW_BKT | TAPS_DUMP_SRAM_SW_PIVOT | \
                   TAPS_DUMP_DRAM_SW_BKT | TAPS_DUMP_DRAM_SW_PFX;                   
    */
    uint32 flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;

    int rv=SOC_E_NONE, index=0, fibcnt=0, vindex=0;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    uint32 vrf_routes, ipaddr=0, ipaddr_len;
    uint32 low_bits, middle_bits, high_bits;
        
    int vrf, vrf_count = 0, vrf_fibbase = 0, matched = FALSE;
    taps_route_param_t *routes = NULL;
    taps_route_param_t tmp_route;
    taps_vrf_param_t *vrf_info = NULL;
    int test_case, key_length;
    int encode_set_bit = 0;
    int value_bit = 0;
    int base_encode_bits_num = 2;
    int cnt_idx = 0;

    prefixSeed = testInfo->nTestSeed;
    test_case = testInfo->testCase;
    key_length = testInfo->testKeyLength;    
    num_routes = testInfo->testNumRoutes;

    sal_memset(encode_array, 0, sizeof(keylength_encode_t) * DIST_LEN_RANGE);
    
    if (num_routes < 100) {
        style = _VERIFY_STYLE_ITERATIVE;
    } else if (num_routes < 1000) {
        style = _VERIFY_STYLE_LAST_ROUTE;
    } else {
        style = _VERIFY_STYLE_BULK;
    }

    switch (test_case) {
    case 0:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, insert in order
         */
        distribution_mode = 32;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = FALSE;
        break;
    case 1:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, shuffle for random
         * insertion order
         */
        distribution_mode = 32;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = TRUE;
        break;
    case 2:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * if the key length is so short that all combination of
         * the key pattern would be less than the specified num_routes
         * then we only create 1<<(key_length%32) routes.
         * old failed case 1:
         *   prefixSeed = 0x32bf5353;
         */
        distribution_mode = key_length%33;
        if (distribution_mode < TAPS_IPV4_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV4_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        if (((key_length%33)<28) && (num_routes >= (1<<(key_length%33)))) {
            /* key length 28 could have 128M routes in 1 vrf */
            num_routes = (1<<(key_length%33))-1;
        }
        shuffle = FALSE;
        break;
    case 3:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order
         * if the key length is so short that all combination of
         * the key pattern would be less than the specified num_routes
         * then we only create 1<<(key_length%33) routes.
         * old failed case 1:
         *   prefixSeed = 0xa6913f9e;
         * old failed case 2:
         *   prefixSeed = 0xb27c455c;
         */
        distribution_mode = key_length%33;
        if (distribution_mode < TAPS_IPV4_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV4_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        if (((key_length%33)<28) && (num_routes >= (1<<(key_length%33)))) {
            /* key length 28 could have 128M routes in 1 vrf */
            /* key_length checked cannot be greater than 27 */
            /* coverity[large_shift] */
            num_routes = (1<<(key_length%33))-1;
        }
        shuffle = TRUE;
        break;
    case 4:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, insert in order
         * each vrf has random number of routes.
         */
        distribution_mode = 32;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = FALSE;
        break;
    case 5:
        /* multiple vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, shuffle for random
         * insertion order
         * each vrf has random number of routes.
         */
        distribution_mode = 32;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;
        break;
    case 6:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = key_length%33;
        if (distribution_mode < TAPS_IPV4_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV4_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = FALSE;        
        break;
    case 7:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = key_length%33;
        if (distribution_mode < TAPS_IPV4_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV4_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    case 8:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_1;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = TRUE;     
        break;
    case 9:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_1;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    case 10:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_2;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    default:
        break;
    } 

    /* setup length distribution table */
    for (index = 0; index < 100; index++) {
        if ((distribution_mode >= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN) &&
            (distribution_mode <= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MAX)) {
            /* fixed key length distribution */
            _key_length_distribution[index] = distribution_mode;
        } else if (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1) {
            /* predefined mode 1
             * 16 -> 5%, 17 -> 2%, 18 -> 3%, 19 -> 7%, 20 -> 8%, 
             * 21 -> 8%, 22 -> 9%, 23 -> 7%, 24 -> 50%, 32 -> 1%
             */
            if (index < 5) {
                _key_length_distribution[index] = 16;
            } else if (index < 5+2) {
                _key_length_distribution[index] = 17;
            } else if (index < 5+2+3) {
                _key_length_distribution[index] = 18;
            } else if (index < 5+2+3+7) {
                _key_length_distribution[index] = 19;
            } else if (index < 5+2+3+7+8) {
                _key_length_distribution[index] = 20;
            } else if (index < 5+2+3+7+8+8) {
                _key_length_distribution[index] = 21;       
            } else if (index < 5+2+3+7+8+8+9) {
                _key_length_distribution[index] = 22;
            } else if (index < 5+2+3+7+8+8+9+7) {
                _key_length_distribution[index] = 23;
            } else if (index < 5+2+3+7+8+8+9+7+50) {
                _key_length_distribution[index] = 24;
            } else {
                _key_length_distribution[index] = 32;
            }
        } else if (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2) {
            /* predefined mode 2
             * 16-32 evenly distributed.
             * 16-17 5% each, 20-32 6% each 
             */
            if (index < 10) {
                _key_length_distribution[index] = 16+(index/5);
            } else {
                _key_length_distribution[index] = 16+2+((index-10)/6);
            }
        } else {
            cli_out("\n!!!! Unsupported key length distribution mode %d !!!\n", distribution_mode);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;      
        }
    }

    /* Encode each distribute len in dist_array 
    *   Nodes : Put length '24' prio to length '19', since '24' would occupy 50% of capacity 
    *               in _TAPS_DISTRIBUTION_MODE_PREDEFINED_1 mode. 
    *               So to support maxinum 2M capacity, it requires at least 20 bits to support 
    *          2^20=1M routes. So only (24-20=4) bits could be used for encoding.
    *
    *   Key_len Encode_value    Bits_num
    *   16      0x80000000      2
    *   17      0xc0000000      2
    *   18      0x40000000      3
    *   24      0x60000000      3
    *   19      0x20000000      4
    *   20      0x30000000      4
    *   21      0x10000000      5
    *   22      0x18000000      5
    *   23      0x08000000      6
    *   25      0x0c000000      6
    *   26      0x04000000      7
    *   27      0x06000000      7
    *   28      0x02000000      8
    *   29      0x03000000      8
    *   30      0x01000000      9
    *   31      0x01800000      9
    *   32      0x00800000      10
    */
    {
        int dist_array[DIST_LEN_RANGE] = {16,17,18,24,19,20,21,22,23,25,26,27,28,29,30,31,32};
        for (index = 0; index < DIST_LEN_RANGE; index++) {
            encode_set_bit = 31 - index/2;
            value_bit = encode_set_bit - 1;
            encode_array[index].key_len = dist_array[index];
            encode_array[index].encode_value = 0;
            encode_array[index].bits_num = base_encode_bits_num + index/2;
            _BITSET(encode_array[index].encode_value, encode_set_bit);
            if (index % 2 != 0) {
                _BITSET(encode_array[index].encode_value, value_bit);
            }
        }
    }

    /* Sort by key_len, change '24' back to the correct position*/
    for (index = 0; index < DIST_LEN_RANGE - 1; index++) {
        keylength_encode_t tmp_encode_data;
        if (encode_array[index].key_len > encode_array[index + 1].key_len) {
            tmp_encode_data = encode_array[index + 1];
            encode_array[index + 1] = encode_array[index];
            encode_array[index] = tmp_encode_data;
        }
        /* Calculate the max routes for each key_len after encoding*/
        distlen_max_routes[index] = 1 << (encode_array[index].key_len - encode_array[index].bits_num); 
        if (index == DIST_LEN_RANGE - 2) {
            /* last member */
            distlen_max_routes[DIST_LEN_RANGE-1] = 1 << (encode_array[DIST_LEN_RANGE-1].key_len - encode_array[DIST_LEN_RANGE-1].bits_num); 
        }
    }
    cli_out("## Test SEED = 0x%x \n", prefixSeed);
    sal_srand(prefixSeed);

    /* setup lookup ucode */
    arg->taps = taps_test_param[testInfo->testTapsInst].taps;
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* fill in random routes */
    routes = sal_alloc(sizeof(taps_route_param_t)*num_routes, "Route info");
    vrf_info = sal_alloc(sizeof(taps_vrf_param_t)*num_routes, "vrf info");
    while (1) {
        if (fibcnt == num_routes) {
            break;
        } else if (fibcnt > num_routes) {
            /* should never happen, catch it here anyhow */
            assert(0);
        }
        sal_memset(distlen_counter, 0, sizeof(uint32) * DIST_LEN_RANGE);
        sal_memset(base_addr, 0, sizeof(uint32) * DIST_LEN_RANGE);
        
        if (single_vrf) {
            vrf_routes = num_routes;
        } else {
            /* pick number of routes in the vrf */
            vrf_routes = sal_rand() % (num_routes - fibcnt);
            if (vrf_routes > (num_routes - fibcnt)/4) {
                /* max a quarter of what's left */
                vrf_routes = (num_routes - fibcnt)/4;
            }
            
            if (vrf_routes > 4096) {
                /* max 8K routes per vrf, this is to speed up the route duplicate check */
                vrf_routes = 4096;
            }
            
            if (vrf_routes < 100) {
                /* min 100 routes per vrf */
                vrf_routes = 100;
            }
            
            if (vrf_routes > (num_routes - fibcnt)) {
                /* allocate all remaining routes to the last vrf */
                vrf_routes = num_routes - fibcnt;
            }
        }
        vrf_info[vrf_count].vrf_routes = vrf_routes;
        /* randomize vrf and make sure it doesn't match existing one */
        while (1) {
            matched = FALSE;
            vrf = sal_rand() & 0x3FF;
            for (index = 0; index < vrf_count; index++) {
                if (vrf_info[index].vrf == vrf) {
                    matched = TRUE;
                    break;
                }
            }
            
            if (matched == FALSE) {
                break;
            }
        }
        vrf_info[vrf_count].vrf = vrf;
        
        /* randomize ipaddr and ipaddr length */
        cli_out("Test VRF %d = 0x%x num routes = 0x%x\n",
                vrf_count, vrf_info[vrf_count].vrf, vrf_info[vrf_count].vrf_routes);
        
        vrf_fibbase = fibcnt;
        
        for (index = 0; index < vrf_info[vrf_count].vrf_routes; index++) {
            if ((distribution_mode >= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN) &&
                (distribution_mode <= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MAX)) {
                /* fixed key length distribution */
                ipaddr_len = _key_length_distribution[0];
            } else {
                while(1) {
                    ipaddr_len = sal_rand() % 100; /* random number between 0-99 */
                    ipaddr_len = _key_length_distribution[ipaddr_len]; /* get true length based on distribution */
                    
                    if (distlen_counter[DIST_LEN_OFFSET(ipaddr_len)] <= distlen_max_routes[DIST_LEN_OFFSET(ipaddr_len)]) {
                        break;
                    }
                }
            }
            
            /* if ipaddr_len == 0, try again */
            if (ipaddr_len == 0) {
                index--;
                continue;
            }
            
            if (key_pattern_mode == _TAPS_KEY_INCREMENTAL) {
                /* incremental */
                if (index == 0) {
                    /* random initial value */
                    ipaddr = sal_rand() & 0x7FFFFFFF;
                    
                } else {
                    /* increment by 1 */
                    if ((distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1) ||
                        (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2)) {
                        ipaddr = ++base_addr[DIST_LEN_OFFSET(ipaddr_len)];
                    } else {
                        ipaddr++;
                    }
                }
            } else if (key_pattern_mode == _TAPS_KEY_RANDOM) {
                /* random prefix */
                if (index == 0) {
                    /* random initial value */
                    ipaddr = sal_rand() & 0x7FFFFFFF;
                } else {
                    /* random increments */
                    if ((distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1) ||
                        (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2)) {
                        base_addr[DIST_LEN_OFFSET(ipaddr_len)] += sal_rand()%4 + 1;
                        ipaddr = base_addr[DIST_LEN_OFFSET(ipaddr_len)];
                    } else {
                        ipaddr += (sal_rand()%4 + 1);
                    }
                }
            } else {
                /* not supported */
                cli_out("\n!!!! Unsupported key pattern mode %d !!!\n", key_pattern_mode);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;      
            }
            
            if (index == 0) {
                if ((distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1) ||
                    (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2)) {
                    for (cnt_idx = 0; cnt_idx < DIST_LEN_RANGE; cnt_idx++) {
                        /* Use the same base IP for difference len*/
                        base_addr[cnt_idx] = ipaddr;
                    }
                }
            }
            
            if (fibcnt >= num_routes) {
                break;
            }
            
            /* align the keys by shifting */
            if (ipaddr_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
                routes[fibcnt].keys[0] = 0;
            } else {
                routes[fibcnt].keys[0] = (vrf >> (32 - ipaddr_len));
            }
            
            if (ipaddr_len == 32) {
                routes[fibcnt].keys[1] = ipaddr;
            } else {
                routes[fibcnt].keys[1] = (ipaddr & ((1 << ipaddr_len)-1)) | \
                    ((vrf & ((1<<(32-ipaddr_len))-1)) << ipaddr_len);
            }
            routes[fibcnt].key_length =  TAPS_IPV4_MAX_VRF_SIZE + ipaddr_len;
            
            if (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1 
                || distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2) {
                /* Find the encode entry for a specific len */
                for (cnt_idx = 0; cnt_idx < DIST_LEN_RANGE; cnt_idx++) {
                    if(encode_array[cnt_idx].key_len == DIST_LEN_OFFSET(routes[fibcnt].key_length)){
                        break;
                    }
                }
                assert(cnt_idx < DIST_LEN_RANGE);
                /* Get the lower (key_len - bits_num) bits in routes[fibcnt].keys[1]*/
                low_bits = routes[fibcnt].keys[1] & ((1<< (encode_array[cnt_idx].key_len - encode_array[cnt_idx].bits_num)) - 1);
                
                /* Get the higher (32 - key_len) bits in routes[fibcnt].keys[1] */
                high_bits = routes[fibcnt].keys[1] & ~((1<< encode_array[cnt_idx].key_len) - 1);
                
                /* Set encode value to instead of (bits_num) bits in routes[fibcnt].keys[1]  */
                middle_bits = (encode_array[cnt_idx].encode_value >> (32 - encode_array[cnt_idx].key_len)) & ((1<< encode_array[cnt_idx].key_len) -1);
                
                routes[fibcnt].keys[1] = high_bits | middle_bits | low_bits;
            }    
            if (key_pattern_mode != _TAPS_KEY_INCREMENTAL) {
                /* make sure the ipaddr and ipaddr_len doesn't match to existing one in the vrf */
                matched = FALSE;
                for (vindex = 0; vindex < index; vindex++) {
                    if ( (routes[fibcnt].key_length == routes[vindex+vrf_fibbase].key_length) &&
                         (routes[fibcnt].keys[0] == routes[vindex+vrf_fibbase].keys[0]) &&
                         (routes[fibcnt].keys[1] == routes[vindex+vrf_fibbase].keys[1])) {
                        matched = TRUE;
                        break;
                    }
                }
                if (matched) {
                    /* if matched to an existing one, try again */
                    index--;
                    continue;
                }
            }
            distlen_counter[DIST_LEN_OFFSET(ipaddr_len)]++;
            fibcnt++;
            if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
                cli_out("!!! generated %d(K) routes\n", fibcnt/1024);
            }
        }
        /* for each route find what prefix should we use for ucode verification lookup */
        for (index = vrf_fibbase; index < fibcnt; index++) {
            
            sal_memcpy(&routes[index].match_key[0], &routes[index].keys[0],
                       sizeof(uint32)*BITS2WORDS(TAPS_IPV4_KEY_SIZE));
            
            /* shift left to form 48 bits key */
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[index].match_key,
                                routes[index].key_length, routes[index].key_length-TAPS_IPV4_KEY_SIZE);
            
            if (SOC_FAILURE(rv)) {
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
            
            if (key_pattern_mode == _TAPS_KEY_INCREMENTAL) {
                if (index == vrf_fibbase) {
                    cli_out("Test VRF %d = 0x%x key length %d key base 0x%x %08x\n",
                            vrf_count, vrf_info[vrf_count].vrf, routes[index].key_length,
                            routes[index].keys[0], routes[index].keys[1]);
                }       
            } else {
                cli_out("Test VRF %d = 0x%x route %d key length %d key 0x%x %08x matched key 0x%x %08x\n",
                        vrf_count, vrf_info[vrf_count].vrf, index, routes[index].key_length,
                        routes[index].keys[0], routes[index].keys[1],
                        routes[index].match_key[0], routes[index].match_key[1]);
            }
            
            if ((index >= 1024) && ((index % (num_routes/10)) == 0)) {
                cli_out("!!! generated lookup key for %d(K) routes\n", index/1024);
            }
        }
        vrf_count++;
    }
    
    if (shuffle) {
        /* shuffle routes to create random route insert pattern */
        for (index = 0; index < num_routes; index++) {
            vindex = sal_rand() % num_routes;
            if (index != vindex) {
                /* swap the key/length of 2 routes */
                sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
            }
        }
    }
    
    /* insert all vrf default routes */
    fibcnt = 0;
    for (vrf = 0; vrf < vrf_count; vrf++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_info[vrf].vrf;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_info[vrf].vrf);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        vrf_base = vrf_info[vrf].vrf;
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            taps_dump(unit, arg->taps, flags);
            cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
        
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "Vrf %d 0.0.0.0/0\n"),
                     _key[1]&0xFFFF));
        
        /* add 127.0.0.0/8 to match customer test case */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = ((vrf_info[vrf].vrf)<<8)+0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE+8;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0xF0000000; /* mark this payload as a 127.0.0.0/8 route payload */
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_info[vrf].vrf);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
    }
    
    
    TIME_STAMP_START;
    /* insert routes onto the vrf */
    tmu_taps_ut_test_result = 0;

    for (index=0; index < num_routes; index++) {
        /* this will build the payload for the route */
        _key_pyld[0] = fibcnt;
        sal_memcpy(&(routes[index].payload[0]), _key_pyld, sizeof(uint32)*_TAPS_PAYLOAD_WORDS_);
        
        /* build route insert argments */
        arg->taps = taps_test_param[testInfo->testTapsInst].taps;
        arg->key = &routes[index].keys[0];
        arg->length = routes[index].key_length;
        arg->payload = routes[index].payload;

        /* insert route */
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to insert %dth route 0x%x 0x%x, len: %d route: %s !!!\n",
                    fibcnt, routes[index].keys[0], routes[index].keys[1], routes[index].key_length, soc_errmsg(rv));
            if (rv == SOC_E_FULL) {
                /* if it's full, we keep verify all routes inserted so far */
                cli_out("\n!!!! Verify %d routes inserted so far !!!\n", fibcnt);
                num_routes = index;
                break;
            } else {
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        }
        
        fibcnt++;
        
        /* verify route */
        if (_VERIFY_STYLE_LAST_ROUTE == style) {
            /* get and verify the route just inserted */
            rv = taps_verify_route(unit, arg, &(routes[index]), TRUE);
            if (SOC_FAILURE(rv)) {
                /*taps_tcam_dump(unit, arg->taps->tcam_hdl, 3);*/
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        } else if (_VERIFY_STYLE_ITERATIVE == style) {
            /* get and verify all routes inserted before */
            for (vindex=0; vindex < index; vindex++) {
                /* get and verify */
                rv = taps_verify_route(unit, arg, &(routes[vindex]), TRUE);
                if (SOC_FAILURE(rv)) {
                    cli_out("############ DUMP ###############\n");
                    taps_dump(unit, arg->taps, TAPS_DUMP_TCAM_SW_BKT | TAPS_DUMP_TCAM_SW_PIVOT);
                    cli_out("############ DUMP ###############\n");

                    taps_dump(unit, arg->taps, flags);
                    cli_out("\n!!!! Failed to verify route:%d Fib-count:%d !!!\n", 
                            vindex, fibcnt);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    /* coverity[check_after_deref] */
                    if (vrf_info) sal_free(vrf_info);
                    return SOC_E_FAIL;
                }
            }
        }

        if ((routes[index].match_key[0] == 0xFFFFFFFF) && (routes[index].match_key[1] == 0xFFFFFFFF)) {
            sal_memcpy(&routes[index].match_key[0], &routes[index].keys[0],
                       sizeof(uint32)*BITS2WORDS(TAPS_IPV4_KEY_SIZE));
            
            /* shift left to form 48 bits key */
            rv = taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[index].match_key,
                                routes[index].key_length, routes[index].key_length-TAPS_IPV4_KEY_SIZE);
            
            ipaddr = routes[index].match_key[1] & (((1<<(routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE))-1) << (TAPS_IPV4_KEY_SIZE-routes[index].key_length));
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "Vrf %d %d.%d.%d.%d/%d\n"),
                         routes[index].match_key[0]&0xFFFF,
                         ipaddr>>24, (ipaddr>>16)&0xFF, (ipaddr>>8)&0xFF, ipaddr&0xFF,
                         routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
            
            routes[index].match_key[0] = 0xFFFFFFFF;
            routes[index].match_key[1] = 0xFFFFFFFF;
        } else {
            ipaddr = routes[index].match_key[1] & (((1<<(routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE))-1) << (TAPS_IPV4_KEY_SIZE-routes[index].key_length));
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "Vrf %d %d.%d.%d.%d/%d\n"),
                         routes[index].match_key[0]&0xFFFF,
                         ipaddr>>24, (ipaddr>>16)&0xFF, (ipaddr>>8)&0xFF, ipaddr&0xFF,
                         routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
        }
        
        if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Added %d(K) routes to FIB \n"),
                       fibcnt/1024));
        }
    }

    cli_out("## Total Added %d routes to FIB \n", fibcnt);

    TIME_STAMP("### Total Route Update time");
    TIME_STAMP_RATE("### Average Route Update rate", fibcnt);

    TIME_STAMP_START;

    /* Flushout any remaining route commands prior to check of routes */
    taps_flush_cache(unit);
  
    cli_out("## used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
            arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
            num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
    
    if (_VERIFY_STYLE_BULK == style) {
        /* get and verify all routes inserted */
        for (vindex=1; vindex < num_routes; vindex++) {
            /* get and verify */
            rv = taps_verify_route(unit, arg, &(routes[vindex]), TRUE);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, flags);
                cli_out("\n!!!! Failed to verify route:%d, 0x%x, 0x%x, len :%d Fib-count:%d !!!\n", 
                        vindex,routes[vindex].keys[0], routes[vindex].keys[1], routes[vindex].key_length,fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        }
    }
    TIME_STAMP("### Total verification time");
    TIME_STAMP_RATE("### Average Route Verification rate", num_routes);    


    if(testInfo->testVerbFlag)
        taps_dump(unit, arg->taps, testInfo->testVerbFlag);

    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    /* coverity[check_after_deref] */
    if (vrf_info) sal_free(vrf_info);
    return rv;
}

/*
* IPv6 mix insert/delete
* 1. Insert vrf default routes 
* 2. Insert routes with random value&len
* 3. Verify routes
* 4. Delete routes and get routes.
* 5. Repeate step 2 - 4 for 10 times.
*/
int taps_ipv6_random_access_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf COMPILER_ATTRIBUTE((unused)), vrf_count, netmask_start, num_routes, 
        route_idx, vrf_idx, shuffle_route_idx, access_times, access_num;
    int netmask_len;
    uint32 ipaddr[4], vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t shuffle_route;
    taps_route_param_t *routes = NULL;
    int dump_flags;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    access_times = 0;
    access_num = 1;
    netmask_start = 32;
    
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* Set up the IPv6 keyploder to use this taps table */
    rv=taps_ut_setup_ipv6_keyploder(unit, arg);    
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to setup IPv6 keyploder !!!\n");
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n", record_mem, record_mem_max);
#endif

    /****************************************************
        *Insert vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    for(access_times = 0; access_times < access_num; access_times++){
        route_idx = 0;
        fibcnt = 0;
        single_vrf = (sal_rand() % 2 == 0);
        vrf_count = sal_rand() % 4096;
        sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);
        
        /****************************************************
            *Generate and Insert routes with random value&length
            ****************************************************/
        while (TRUE) {
            if (route_idx >= num_routes) {
                break;
            }
            /* Generate route */
            /* Random IP length between 16-32*/
            netmask_len = netmask_start + sal_rand() % (TAPS_IPV6_PFX_SIZE - netmask_start + 1);

            /* IP value */
            ipaddr[0] = V4_RAND();
            ipaddr[1] = V4_RAND();
            ipaddr[2] = V4_RAND();
            ipaddr[3] = V4_RAND();

            /* Vrf value */
            vrf = sal_rand() % vrf_count;

            /* align the keys by shifting */
            routes[route_idx].keys[0] = vrf;
            routes[route_idx].keys[1] = ipaddr[0];
            routes[route_idx].keys[2] = ipaddr[1];
            routes[route_idx].keys[3] = ipaddr[2];
            routes[route_idx].keys[4] = ipaddr[3];
            routes[route_idx].key_length =  TAPS_IPV6_MAX_VRF_SIZE + netmask_len;
                
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].keys,
                                TAPS_IPV6_KEY_SIZE, 
                                TAPS_IPV6_PFX_SIZE - netmask_len);
            if (SOC_FAILURE(rv)) {
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }

            /* Construct match_key */
            sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                    sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
            rv =  taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].match_key,
                        routes[route_idx].key_length, 
                        netmask_len - TAPS_IPV6_PFX_SIZE);
            if (SOC_FAILURE(rv)) {
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            routes[route_idx].payload[0] = route_idx;     

            /* Insert this route */
            arg->length = routes[route_idx].key_length;
            arg->key = &routes[route_idx].keys[0];
            arg->payload= &routes[route_idx].payload[0];

            rv = taps_insert_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                if (rv == SOC_E_EXISTS) {
                    /* Exist route, then try to get another one */
                    continue;
                } else {
                    cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x 0x%x 0x%x "
                            "0x%x length %d : %s !!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                            routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                            soc_errmsg(rv));
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
            }
            route_idx++;
        }
        cli_out("##%d time access used %d tcam entries out of %d total tcam entries, expected capacity %d \n",
                access_times, arg->taps->tcam_hdl->use_count, arg->taps->tcam_hdl->size, 
                num_routes * arg->taps->tcam_hdl->size / arg->taps->tcam_hdl->use_count);
        /****************************************************
            *Verify routes
            ****************************************************/
        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            /* Should find out the bpm route */
            rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1],
                        routes[route_idx].keys[2], routes[route_idx].keys[3],
                        routes[route_idx].keys[4], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! Verified %d(K) routes after insert \n", route_idx/1024);
            }
        }

        /* To delete with random sequence, we need to shuffle routes */
        /* shuffle routes to create random route insert pattern */
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            shuffle_route_idx = sal_rand() % num_routes;
            if (shuffle_route_idx != route_idx) {
                /* swap the key/length of 2 routes */
                sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
            }
        }

       /****************************************************
            *Delete routes
            ****************************************************/
        for (route_idx = 0; route_idx < num_routes; route_idx++) {
            arg->length = routes[route_idx].key_length;
            arg->key = &routes[route_idx].keys[0];
            arg->payload= &routes[route_idx].payload[0];
            rv = taps_delete_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to Delete routes 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1],
                        routes[route_idx].keys[2], routes[route_idx].keys[3],
                        routes[route_idx].keys[4], routes[route_idx].key_length);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            } else {
                /* Get routes, should return not found */
                rv = taps_get_route(unit, arg);
                if (rv != SOC_E_NOT_FOUND) {
                    cli_out("\n!!!! Failed to get routes 0x%x 0x%x 0x%x 0x%x 0x%x "
                            "length %d. return %d!!!\n",
                            routes[route_idx].keys[0], routes[route_idx].keys[1],
                            routes[route_idx].keys[2], routes[route_idx].keys[3],
                            routes[route_idx].keys[4], routes[route_idx].key_length, rv);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                } 
                
                rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                    arg->taps->master_unit, 
                                                    arg->taps->param.host_share_table));
                if (SOC_FAILURE(rv)) {
                    cli_out("taps_flush_cache Flush failed err=%d\n", rv);
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
                /* Should find out the bpm route */
                rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
                if (SOC_FAILURE(rv)) {
                    taps_dump(unit, arg->taps, dump_flags);
                    cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x "
                            "0x%x 0x%x 0x%x length %d!!!\n",
                            routes[route_idx].keys[0], routes[route_idx].keys[1], 
                            routes[route_idx].keys[2], routes[route_idx].keys[3], 
                            routes[route_idx].keys[4], routes[route_idx].key_length);
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! Deleted %d(K) routes\n", route_idx/1024);
            }
        }
    }

    /****************************************************
        *Delete vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL; 
    }
#endif   

    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}


int taps_ipv6_test_run(int unit, void *data)
{
    int num_routes;                                  /* number of routes */
    int style = _VERIFY_STYLE_BULK;                  /* ucode verify style */
    uint32 prefixSeed;
    int distribution_mode = 32;                      /* key length distribution mode, see above */
    int key_pattern_mode = _TAPS_KEY_INCREMENTAL;    /* key pattern mode, see above */
    int single_vrf = TRUE;                           /* if TRUE, all prefixes on single vrf, otherwise random vrf */
    int shuffle = FALSE;                             /* if TRUE, shuffle the IP address randomly */
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    int rv=SOC_E_NONE, index=0, fibcnt=0, vindex=0;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_init_params_t *param = &taps_test_param[testInfo->testTapsInst].param;  
    uint32 vrf_routes, ipaddr[BITS2WORDS(TAPS_IPV6_PFX_SIZE)], ipaddr_len;
    int vrf, vrf_count = 0, vrf_fibbase = 0, matched = FALSE, num_tries;
    taps_route_param_t *routes = NULL;
    taps_route_param_t tmp_route;
    taps_vrf_param_t *vrf_info = NULL;
    int test_case, key_length;
    uint32 flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;

    prefixSeed = testInfo->nTestSeed;
    test_case = testInfo->testCase;    
    key_length = testInfo->testKeyLength;    
    num_routes = testInfo->testNumRoutes;

    if (num_routes < 100) {
        style = _VERIFY_STYLE_ITERATIVE;
    } else if (num_routes < 1000) {
        style = _VERIFY_STYLE_LAST_ROUTE;
    } else {
        style = _VERIFY_STYLE_BULK;
    }

    

    ipaddr[0] = ipaddr[1] = ipaddr[2] = ipaddr[3] = 0; 
    switch (test_case) {
    case 0:
        /* hardcode version */
        /* Call the routine to set up the canned routes */         
        taps_pop_canned_ipv6(unit, param, taps_test_param[testInfo->testTapsInst].taps,testInfo->testNumRoutes);
        
        /* Dump the table if requested */
        if(testInfo->testVerbFlag)
            taps_dump(unit, arg->taps,testInfo->testVerbFlag ); 
        
        return rv;
    case 1:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 128, insert in order
         */
        distribution_mode = 128;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = FALSE;
        break;
    case 2:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 128, shuffle for random
         * insertion order
         */
        distribution_mode = 128;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = TRUE;
        break;
    case 3:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * if the key length is so short that all combination of
         * the key pattern would be less than the specified num_routes
         * then we only create 1<<(key_length%32) routes.
         */
        distribution_mode = key_length%129;
        if (distribution_mode < TAPS_IPV6_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV6_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        if (((key_length%129)<28) && (num_routes >= (1<<(key_length%129)))) {
            /* key length 114 could have 128M routes in 1 vrf */
            num_routes = (1<<(key_length%129))-1;
        }
        shuffle = FALSE;
        break;
    case 4:
        /* single vrf, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order
         * if the key length is so short that all combination of
         * the key pattern would be less than the specified num_routes
         * then we only create 1<<(key_length%33) routes.
         */
        distribution_mode = key_length%129;
        if (distribution_mode < TAPS_IPV6_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV6_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        if (((key_length%129)<28) && (num_routes >= (1<<(key_length%129)))) {
            /* key length 114 could have 128M routes in 1 vrf */
            /* key_length checked above cannot be more than 27 */
            /* coverity[large_shift] */
            num_routes = (1<<(key_length%129))-1;
        }
        shuffle = TRUE;
        break;
    case 5:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, insert in order
         * each vrf has random number of routes.
         */
        distribution_mode = 128;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = FALSE;
        break;
    case 6:
        /* multiple vrf, random vrf value, random ip starting address,
         * incremental ip address, fix length 32, shuffle for random
         * insertion order
         * each vrf has random number of routes.
         */
        distribution_mode = 128;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;
        break;
    case 7:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = key_length%129;
        if (distribution_mode < TAPS_IPV6_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV6_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = FALSE;        
        break;
    case 8:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = key_length%129;
        if (distribution_mode < TAPS_IPV6_MAX_VRF_SIZE) {
            distribution_mode = TAPS_IPV6_MAX_VRF_SIZE;
        }
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    case 9:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, insert in order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_1;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = TRUE;  
        shuffle = TRUE;     
        break;
    case 10:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_1;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    case 11:
        /* multiple vrfs, random vrf value, random ip starting address,
         * incremental ip address, specified length, shuffle for random
         * insertion order.
         * each vrf has random number of routes, but will have a minimum
         * of 100 routes and maximum of 8K or a quarter of all routes.
         */
        distribution_mode = _TAPS_DISTRIBUTION_MODE_PREDEFINED_2;
        key_pattern_mode = _TAPS_KEY_INCREMENTAL;
        single_vrf = FALSE; 
        shuffle = TRUE;     
        break;
    case 12:
        
        break;
    default:
        break;
    } 

    /* setup length distribution table */
    for (index = 0; index < 100; index++) {
        if ((distribution_mode >= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN) &&
            (distribution_mode <= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_IPV6_MAX)) {
            /* fixed key length distribution */
            _key_length_distribution[index] = distribution_mode;
        } else if (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_1) {
            /* predefined mode 1. 99% 64 bits prefixes, 1% 128 bits prefixes */
            if (index == 50) {
                _key_length_distribution[index] = 128;
            } else {
                _key_length_distribution[index] = 64;
            }
        } else if (distribution_mode == _TAPS_DISTRIBUTION_MODE_PREDEFINED_2) {
            /* predefined mode 2
             * 29-128 evenly distributed.
             * 1% each
             */
            _key_length_distribution[index] = index+29;
        } else {
            cli_out("\n!!!! Unsupported key length distribution mode %d !!!\n", distribution_mode);
            tmu_taps_ut_test_result = -1;
            return SOC_E_FAIL;      
        }
    }

    cli_out("## Test SEED = 0x%x \n", prefixSeed);
    sal_srand(prefixSeed);

    /* setup lookup ucode */
    arg->taps = taps_test_param[testInfo->testTapsInst].taps;
    rv = taps_ut_setup_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* Set up the IPv6 keyploder to use this taps table */
    rv=taps_ut_setup_ipv6_keyploder(unit, arg);    
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to setup IPv6 keyploder !!!\n");
    }

    /* fill in random routes */
    routes = sal_alloc(sizeof(taps_route_param_t)*num_routes, "Route info");
    vrf_info = sal_alloc(sizeof(taps_vrf_param_t)*num_routes, "vrf info");
    while (1) {
        if (fibcnt == num_routes) {
            break;
        } else if (fibcnt > num_routes) {
            /* should never happen, catch it here anyhow */
            assert(0);
        }
        
        if (single_vrf) {
            vrf_routes = num_routes;
        } else {
            /* pick number of routes in the vrf */
            vrf_routes = sal_rand() % (num_routes - fibcnt);
            if (vrf_routes > (num_routes - fibcnt)/4) {
                /* max a quarter of what's left */
                vrf_routes = (num_routes - fibcnt)/4;
            }
            
            if (vrf_routes > 4096) {
                /* max 8K routes per vrf, this is to speed up the route duplicate check */
                vrf_routes = 4096;
            }
            
            if (vrf_routes < 100) {
                /* min 100 routes per vrf */
                vrf_routes = 100;
            }
            
            if (vrf_routes > (num_routes - fibcnt)) {
                /* allocate all remaining routes to the last vrf */
                vrf_routes = num_routes - fibcnt;
            }
        }
        vrf_info[vrf_count].vrf_routes = vrf_routes;
        
        /* randomize vrf and make sure it doesn't match existing one */
        while (1) {
            matched = FALSE;
            if (tmu_taps_ut_debug) {
                vrf = 0;
            } else {
                vrf = sal_rand() & 0x3FF;
            }
            for (index = 0; index < vrf_count; index++) {
                if (vrf_info[index].vrf == vrf) {
                    matched = TRUE;
                    break;
                }
            }
            
            if (matched == FALSE) {
                break;
            }
        }
        vrf_info[vrf_count].vrf = vrf;
        
        /* randomize ipaddr and ipaddr length */
        cli_out("Test VRF %d = 0x%x num routes = 0x%x\n",
                vrf_count, vrf_info[vrf_count].vrf, vrf_info[vrf_count].vrf_routes);
        
        vrf_fibbase = fibcnt;
        for (index = 0; index < vrf_info[vrf_count].vrf_routes; index++) {
            if (key_pattern_mode == _TAPS_KEY_INCREMENTAL) {
                /* incremental */
                if (index == 0) {
                    /* random initial value */
                    ipaddr[0] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[1] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[2] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[3] = sal_rand() & 0xFFFFFFFF;
                } else {
                    /* increment by 1 */
                    ipaddr[0]++;
                }
            } else if (key_pattern_mode == _TAPS_KEY_RANDOM) {
                /* random prefix */
                if (index == 0) {
                    /* random initial value */
                    ipaddr[0] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[1] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[2] = sal_rand() & 0xFFFFFFFF;
                    ipaddr[3] = sal_rand() & 0xFFFFFFFF;
                } else {
                    /* random increments */
                    ipaddr[0] += (sal_rand()%4 + 1);
                }
            } else {
                /* not supported */
                cli_out("\n!!!! Unsupported key pattern mode %d !!!\n", key_pattern_mode);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;      
            }
            
            ipaddr_len = sal_rand() % 100; /* random number between 0-99 */
            ipaddr_len = _key_length_distribution[ipaddr_len]; /* get true length based on distribution */
            
            /* if ipaddr_len == 0, try again */
            if (ipaddr_len == 0) {
                index--;
                continue;
            }
            
            if (fibcnt >= num_routes) {
                break;
            }
            
            /* align the keys by shifting */
            routes[fibcnt].keys[0] = 0;
            routes[fibcnt].keys[1] = ipaddr[3];
            routes[fibcnt].keys[2] = ipaddr[2];
            routes[fibcnt].keys[3] = ipaddr[1];
            routes[fibcnt].keys[4] = ipaddr[0];
            
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[fibcnt].keys,
                                ipaddr_len, ipaddr_len-TAPS_IPV6_PFX_SIZE);
            if (SOC_FAILURE(rv)) {
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
            
            routes[fibcnt].keys[0] = vrf;
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[fibcnt].keys,
                                TAPS_IPV6_KEY_SIZE, TAPS_IPV6_PFX_SIZE-ipaddr_len);
            if (SOC_FAILURE(rv)) {
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
            routes[fibcnt].key_length =  TAPS_IPV4_MAX_VRF_SIZE + ipaddr_len;
            
            if (key_pattern_mode != _TAPS_KEY_INCREMENTAL) {
                /* make sure the ipaddr and ipaddr_len doesn't match to existing one in the vrf */
                matched = FALSE;
                for (vindex = 0; vindex < index; vindex++) {
                    if ( (routes[fibcnt].key_length == routes[vindex+vrf_fibbase].key_length) &&
                         (routes[fibcnt].keys[4] == routes[vindex+vrf_fibbase].keys[4]) &&
                         (routes[fibcnt].keys[3] == routes[vindex+vrf_fibbase].keys[3]) &&
                         (routes[fibcnt].keys[2] == routes[vindex+vrf_fibbase].keys[2]) &&
                         (routes[fibcnt].keys[1] == routes[vindex+vrf_fibbase].keys[1]) &&
                         (routes[fibcnt].keys[0] == routes[vindex+vrf_fibbase].keys[0])) {
                        matched = TRUE;
                        break;
                    }
                }
                if (matched) {
                    /* if matched to an existing one, try again */
                    index--;
                    continue;
                }
            }
            
            fibcnt++;
            if ((fibcnt >= 1024) && ((fibcnt % (num_routes/10)) == 0)) {
                cli_out("!!! generated %d(K) routes\n", fibcnt/1024);
            }
        }
        
        /* for each route find what prefix should we use for ucode verification lookup */
        for (index = vrf_fibbase; index < fibcnt; index++) {
            if ((distribution_mode >= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_MIN) &&
                (distribution_mode <= _TAPS_DISTRIBUTION_MODE_FIXED_LENGTH_IPV6_MAX)) {
                /* fixed length key */
                sal_memcpy(&routes[index].match_key[0], &routes[index].keys[0],
                           sizeof(uint32)*BITS2WORDS(TAPS_IPV6_KEY_SIZE));
                
                /* shift left to form 144 bits key */
                rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[index].match_key,
                                    routes[index].key_length, routes[index].key_length-TAPS_IPV6_KEY_SIZE);
                if (SOC_FAILURE(rv)) {
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    /* coverity[check_after_deref] */
                    if (vrf_info) sal_free(vrf_info);
                    return SOC_E_FAIL;
                }
            } else {
                /* we are having a mixed length routes, we need to find 
                 * a key that will hit this route.
                 */
                sal_memcpy(&routes[index].match_key[0], &routes[index].keys[0],
                           sizeof(uint32)*BITS2WORDS(TAPS_IPV6_KEY_SIZE));
                
                /* shift left to form 144 bits key */
                rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[index].match_key,
                                    routes[index].key_length, routes[index].key_length-TAPS_IPV6_KEY_SIZE);
                
                if (SOC_FAILURE(rv)) {
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    /* coverity[check_after_deref] */
                    if (vrf_info) sal_free(vrf_info);
                    return SOC_E_FAIL;
                }
                
                if (routes[index].key_length != TAPS_IPV6_KEY_SIZE) {
                    /* we randomly pick a key and see if it will match to any of 
                     * other routes longer and matching the route. if it matches,
                     * we need to pick another one till we are successful.
                     * Do it this way since the longest route length (other than 48 bits)
                     * for predefined1 mode is 16+24=40 bits. So we have at least 
                     * 128 value to pick from and a small chance of collision.
                     */
                    num_tries = 20;
                    
                    /* mask off the don't care bits using shifts */
                    rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[index].match_key,
                                        TAPS_IPV6_KEY_SIZE, TAPS_IPV6_KEY_SIZE - routes[index].key_length);         
                    if (SOC_FAILURE(rv)) {
                        /* coverity[check_after_deref] */
                        if (routes) sal_free(routes);
                        /* coverity[check_after_deref] */
                        if (vrf_info) sal_free(vrf_info);
                        return SOC_E_FAIL;
                    }           
                    
                    rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[index].match_key,
                                        routes[index].key_length, routes[index].key_length - TAPS_IPV6_KEY_SIZE);           
                    if (SOC_FAILURE(rv)) {
                        /* coverity[check_after_deref] */
                        if (routes) sal_free(routes);
                        /* coverity[check_after_deref] */
                        if (vrf_info) sal_free(vrf_info);
                        return SOC_E_FAIL;
                    }           
                    
                    while(num_tries > 0) {
                        /* put in random bits */
                        if ((TAPS_IPV6_KEY_SIZE - routes[index].key_length) >= 32) {
                            routes[index].match_key[4] = sal_rand();
                        } else {
                            routes[index].match_key[4] |= (sal_rand() & ((1<<(TAPS_IPV6_KEY_SIZE-routes[index].key_length))-1));
                        }
                        
                        /* make sure there is no existing route match to this key */
                        matched = FALSE;
                        for (vindex = vrf_fibbase; vindex < fibcnt; vindex++) {             
                            if (routes[index].key_length >= routes[vindex].key_length) {
                                continue;
                            }
                            if (taps_key_match(TAPS_IPV6_KEY_SIZE, routes[index].match_key, TAPS_IPV6_KEY_SIZE,
                                               routes[vindex].keys, routes[vindex].key_length)) {
                                matched = TRUE;             
                                num_tries--;
                                break;
                            }
                        }
                        if (matched == FALSE) {
                            LOG_DEBUG(BSL_LS_APPL_COMMON,
                                      (BSL_META_U(unit,
                                                  "Found a lookup key 0x%x %08x %08x %08x %08x for VRF %d = 0x%x route %d key length %d key 0x%x %08x %08x %08x %08x\n"),
                                       routes[index].match_key[0], routes[index].match_key[1], routes[index].match_key[2], routes[index].match_key[3],
                                       routes[index].match_key[4], vrf_count, vrf_info[vrf_count].vrf, index, routes[index].key_length,
                                       routes[index].keys[0], routes[index].keys[1], routes[index].keys[2], routes[index].keys[3], routes[index].keys[4]));
                            break;
                        }
                    }
                    
                    if (num_tries == 0) {
                        LOG_DEBUG(BSL_LS_APPL_COMMON,
                                  (BSL_META_U(unit,
                                              "Failed to find a lookup key for VRF %d = 0x%x route %d key length %d key 0x%x %08x %08x %08x%08x "
                                              "skipping the lookup verification for this route\n"),
                                   vrf_count, vrf_info[vrf_count].vrf, index, routes[index].key_length,
                                   routes[index].keys[0], routes[index].keys[1], routes[index].keys[2], routes[index].keys[3], routes[index].keys[4]));
                        /* put a special pattern here */
                        routes[index].match_key[0]=0xFFFFFFFF;
                        routes[index].match_key[1]=0xFFFFFFFF;
                        routes[index].match_key[2]=0xFFFFFFFF;
                        routes[index].match_key[3]=0xFFFFFFFF;
                        routes[index].match_key[4]=0xFFFFFFFF;
                    }
                }
            }
            
            if (key_pattern_mode == _TAPS_KEY_INCREMENTAL) {
                if (index == vrf_fibbase) {
                    cli_out("Test VRF %d = 0x%x key length %d key base 0x%x %08x %08x %08x %08x\n",
                            vrf_count, vrf_info[vrf_count].vrf, routes[index].key_length,
                            routes[index].keys[0], routes[index].keys[1], routes[index].keys[2], routes[index].keys[3], routes[index].keys[4]);
                }       
            } else {
                cli_out("Test VRF %d = 0x%x route %d key length %d key 0x%x %08x %08x %08x %08x matched key 0x%x %08x %08x %08x %08x\n",
                        vrf_count, vrf_info[vrf_count].vrf, index, routes[index].key_length,
                        routes[index].keys[0], routes[index].keys[1], routes[index].keys[2], routes[index].keys[3], routes[index].keys[4],
                        routes[index].match_key[0], routes[index].match_key[1], routes[index].match_key[2], routes[index].match_key[3], routes[index].match_key[4]);
            }
            
            if ((index >= 1024) && ((index % (fibcnt/10)) == 0)) {
                cli_out("!!! generated lookup key for %d(K) routes\n", index/1024);
            }
        }
        vrf_count++;
    }
    
    if (shuffle) {
        /* shuffle routes to create random route insert pattern */
        for (index = 0; index < num_routes; index++) {
            vindex = sal_rand() % num_routes;
            if (index != vindex) {
                /* swap the key/length of 2 routes */
                sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                
                sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
            }
        }
    }
    
    /* insert all vrf default routes */
    fibcnt = 0;
    for (vrf = 0; vrf < vrf_count; vrf++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_info[vrf].vrf;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_info[vrf].vrf);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route (no fibverify for now) */
        vrf_base = vrf_info[vrf].vrf;
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            taps_dump(unit, arg->taps, flags);
            cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
        
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "Vrf %d 0.0.0.0/0\n"),
                     _key[1]&0xFFFF));
    }

    TIME_STAMP_START;
    /* insert routes onto the vrf */
    tmu_taps_ut_test_result = 0;

    for (index=0; index < num_routes; index++) {
        /* this will build the payload for the route */
        _key_pyld[0] = fibcnt;
        sal_memcpy(&(routes[index].payload[0]), _key_pyld, sizeof(uint32)*_TAPS_PAYLOAD_WORDS_);
        
        /* build route insert argments */
        arg->taps = taps_test_param[testInfo->testTapsInst].taps;
        arg->key = &routes[index].keys[0];
        arg->length = routes[index].key_length;
        arg->payload = routes[index].payload;
        
        /* insert route */
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to insert %dth route 0x%x 0x%x route !!!\n",
                    fibcnt, routes[index].keys[0], routes[index].keys[1]);
            if (rv == SOC_E_FULL) {
                /* if it's full, we keep verify all routes inserted so far */
                cli_out("\n!!!! Verify %d routes inserted so far !!!\n", fibcnt);
                num_routes = index;
                break;
            } else {
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        } else {
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                         arg->key[0]&0xFFFF,
                         arg->key[1], arg->key[2], arg->key[3], arg->key[4], 
                         arg->length-TAPS_IPV4_MAX_VRF_SIZE));
        }
        
        fibcnt++;

        /* verify route */
        if (_VERIFY_STYLE_LAST_ROUTE == style) {
            /* get and verify the route just inserted */
            rv = taps_verify_ipv6_route(unit, arg, &(routes[index]), TRUE);
            if (SOC_FAILURE(rv)) {
                cli_out("############ DUMP ###############\n");
                taps_dump(unit, arg->taps, flags);
                cli_out("############ DUMP ###############\n");
                
                cli_out("\n!!!! Failed to verify route:%d !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        } else if (_VERIFY_STYLE_ITERATIVE == style) {
            /* get and verify all routes inserted before */
            for (vindex=0; vindex < index; vindex++) {
                /* get and verify */
                rv = taps_verify_ipv6_route(unit, arg, &(routes[vindex]), TRUE);
                if (SOC_FAILURE(rv)) {
                    cli_out("############ DUMP ###############\n");
                    taps_dump(unit, arg->taps, flags);
                    cli_out("############ DUMP ###############\n");
                    
                    taps_dump(unit, arg->taps, flags);
                    cli_out("\n!!!! Failed to verify route:%d Fib-count:%d !!!\n", 
                            vindex, fibcnt);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    /* coverity[check_after_deref] */
                    if (vrf_info) sal_free(vrf_info);
                    return SOC_E_FAIL;
                }
            }
        }
        
        if ((routes[index].match_key[0] == 0xFFFFFFFF) &&
            (routes[index].match_key[1] == 0xFFFFFFFF) &&
            (routes[index].match_key[2] == 0xFFFFFFFF) &&
            (routes[index].match_key[3] == 0xFFFFFFFF) &&
            (routes[index].match_key[4] == 0xFFFFFFFF)) {
            sal_memcpy(&routes[index].match_key[0], &routes[index].keys[0],
                       sizeof(uint32)*BITS2WORDS(TAPS_IPV6_KEY_SIZE));
            
            /* shift left to form 144 bits key */
            rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[index].match_key,
                                routes[index].key_length, routes[index].key_length-TAPS_IPV6_KEY_SIZE);
            
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                       routes[index].match_key[0]&0xFFFF,
                       routes[index].match_key[1], routes[index].match_key[2], routes[index].match_key[3], routes[index].match_key[4], 
                       routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
            
            routes[index].match_key[0] = 0xFFFFFFFF;
            routes[index].match_key[1] = 0xFFFFFFFF;
            routes[index].match_key[2] = 0xFFFFFFFF;
            routes[index].match_key[3] = 0xFFFFFFFF;
            routes[index].match_key[4] = 0xFFFFFFFF;
        } else {
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                       routes[index].match_key[0]&0xFFFF,
                       routes[index].match_key[1], routes[index].match_key[2],
                       routes[index].match_key[3], routes[index].match_key[4], 
                       routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
        }
        
        if (fibcnt % 1024 == 0) {
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Added %d(K) routes to FIB \n"),
                       fibcnt/1024));
        }
    }
    
    cli_out("## Total Added %d routes to FIB \n", fibcnt);
    
    TIME_STAMP("### Total Route Update time");
    TIME_STAMP_RATE("### Average Route Update rate", fibcnt);

    TIME_STAMP_START;

    if (_VERIFY_STYLE_BULK == style) {
        /* get and verify all routes inserted */
        for (vindex=0; vindex < num_routes; vindex++) {
            /* get and verify */
            rv = taps_verify_ipv6_route(unit, arg, &(routes[vindex]), TRUE);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, flags);
                cli_out("\n!!!! Failed to verify route:%d Fib-count:%d !!!\n", 
                        vindex, fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                /* coverity[check_after_deref] */
                if (vrf_info) sal_free(vrf_info);
                return SOC_E_FAIL;
            }
        }
    }


    /* do lookup on keys that does not match to any inserted keys
     * we should return default vrf or shorter routes. We can do that by
     * delete the existing routes and use those routes to look up
     */
    for (index=0; index < num_routes; index+=8) {
        arg->taps = taps_test_param[testInfo->testTapsInst].taps;
        arg->key = &routes[index].keys[0];
        arg->length = routes[index].key_length;

        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete %dth route !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return rv;
        }          
    }
    
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                            arg->taps->master_unit, 
                                            arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        /* coverity[check_after_deref] */
        if (vrf_info) sal_free(vrf_info);
        return SOC_E_FAIL;
    }

    for (index=0; index < num_routes; index+=8) {
        /* verify route, lrp lookup */
        arg->key = &(routes[index].match_key[0]);
        arg->length = arg->taps->param.key_attr.length;
        if ((arg->key[0] == 0xFFFFFFFF) && 
            (arg->key[1] == 0xFFFFFFFF) && 
            (arg->key[2] == 0xFFFFFFFF) && 
            (arg->key[3] == 0xFFFFFFFF) && 
            (arg->key[4] == 0xFFFFFFFF)) {
            /* this route doesn't have match key, skip */
            continue;
        }

        rv = taps_verify_ipv6_bpm(unit, arg, &routes[index]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[index].keys[0], routes[index].keys[1],
                    routes[index].keys[2], routes[index].keys[3],
                    routes[index].keys[4], routes[index].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            /* coverity[check_after_deref] */
            if (vrf_info) sal_free(vrf_info);
            return SOC_E_FAIL;
        }
    }

    if(testInfo->testVerbFlag)
        taps_dump(unit, arg->taps, testInfo->testVerbFlag);

    TIME_STAMP("### Total verification time");
    TIME_STAMP_RATE("### Average Route Verification rate", fibcnt);
    
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    /* coverity[check_after_deref] */
    if (vrf_info) sal_free(vrf_info);
    return rv;
}

/* gets the payload information from fib, issues ucode lookup
 * to verify if the fib payload is same as looked up from ucode */
int test_taps_get_route(int unit, 
                        uint32 *key, 
                        uint32 *payload,
                        uint32 length)
{
    taps_table_param_t *tbl_param = &taps_test_param[0];
    taps_arg_t *arg = &tbl_param->arg;
    int rv=SOC_E_NONE;

    if (!key || !payload) {
        cli_out("\n!!!! Bad parameter !!!\n");
        return SOC_E_PARAM;
    }

    if (tbl_param->taps == NULL) {
        cli_out("\n!!!! No TAPS instance available to test !!!\n");
        return SOC_E_PARAM;
    }

    /* for now only /32 is tested */
    if (length != TAPS_IPV4_KEY_SIZE) {
        cli_out("\n for now only /32 is tested !! \n");
        return SOC_E_PARAM;
    }

    sal_memset(payload, 0, sizeof(uint32) * 4);

    arg->taps = taps_test_param[0].taps;
    arg->key = key;
    arg->length = length;
    arg->payload = payload;

    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to get route 0x%x 0x%x route !!!\n",
                key[0], key[1]);
        return SOC_E_FAIL;
    }   

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    rv = taps_ut_verify_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x !!!\n", key[0], key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    if (sal_memcmp(payload, _ucode_pyld, 
                   sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_) != 0) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x: CPE: 0x%x\n "
                "Expected:0x%x-0x%x-0x%x-0x%x\n"
                "Ucode Payload:0x%x-0x%x-0x%x-0x%x  !!!\n",
                key[0], key[1], (unsigned int)_TAPS_KEY_2_CPE_31B(key[1], 31),
                payload[0], payload[1], payload[2], payload[3],
                _ucode_pyld[0], _ucode_pyld[1], _ucode_pyld[2], _ucode_pyld[3]);
        return SOC_E_FAIL;
    }

    return rv;
}

/* adds a given route to fib - Note update not yet supported 
 * issues ucode lookup to verify if the fib payload is same as looked up from ucode */
int test_taps_add_route(int unit, 
                        uint32 *key, 
                        uint32 *payload,
                        uint32 length)
{
    taps_table_param_t *tbl_param = &taps_test_param[0];
    taps_arg_t *arg = &tbl_param->arg;
    int rv=SOC_E_NONE;

    if (!key || !payload) {
        cli_out("\n!!!! Bad parameter !!!\n");
        return SOC_E_PARAM;
    }

    if (tbl_param->taps == NULL) {
        cli_out("\n!!!! No TAPS instance available to test !!!\n");
        return SOC_E_PARAM;
    }

    if (!tmu_taps_ucode_loaded) {
        cli_out("\n!!!! No Ucode loaded to test loading...");
        rv = taps_ut_setup_ucode_lookup(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Load ucode !!!\n");
            return SOC_E_FAIL;
        }
        tmu_taps_ucode_loaded = TRUE;
    }

    /* for now only /32 is tested */
    if (length != TAPS_IPV4_KEY_SIZE) {
        cli_out("\n for now only /32 is tested !! \n");
        return SOC_E_PARAM;
    }

    arg->taps = taps_test_param[0].taps;
    arg->key = key;
    arg->length = length;
    arg->payload = payload;
    /* coverity[ stack_use_overflow ] */
    rv = taps_insert_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to set route 0x%x 0x%x route !!!\n",
                key[0], key[1]);
        return SOC_E_FAIL;
    }

    rv = taps_get_route(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to get route 0x%x 0x%x route !!!\n",
                key[0], key[1]);
        return SOC_E_FAIL;
    }   

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    rv = taps_ut_verify_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to lookup ucode for"
                " key 0x%x 0x%x !!!\n", key[0], key[1]);
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    if (sal_memcmp(payload, _ucode_pyld, 
                   sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_) != 0) {
        cli_out("\n!!!! Microcode Lookup Failed to match Key: 0x%x 0x%x "
                "Expected:0x%x-0x%x-0x%x-0x%x\n"
                "Ucode Payload:0x%x-0x%x-0x%x-0x%x  !!!\n",
                key[0], key[1],
                payload[0], payload[1], payload[2], payload[3],
                _ucode_pyld[0], _ucode_pyld[1], _ucode_pyld[2], _ucode_pyld[3]);
        return SOC_E_FAIL;
    }

    return rv;
}



int taps_table_parallel_init(int unit, void *data)
{
    int rv = SOC_E_NONE;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;

    /* Initialize two tables. One for each taps instance and identical otherwise */
    testInfo->testTapsInst=0;

    rv=taps_table_init(unit,data);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS Instance 0 !!!\n");
    }

    testInfo->testTapsInst=1;
    rv = taps_table_init(unit,data);
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to create TAPS Instance 1 !!!\n");
    }

    return rv;

}


/* Set up the microcode for parallel operation of the two TAPS Instances */
int taps_ut_setup_par_ucode_lookup(int unit, taps_arg_t *p_t0_arg, taps_arg_t *p_t1_arg)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_program_info_t prog_info;
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    uint32 rval=0;
    int slave_idx, slave_unit;
    
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_tapsp_ut.txt");

    /* key format:
     * VRF <bit 48-32> , IPV4<31-0> */

    /* VRI + Destination IP address */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _TAPS_TMU_PRG_NUM_;

    /* VRF + Destination IP Address */
    prog_info.key_info[0].valid = TRUE;
    if (_TAPS_IS_PARALLEL_MODE_(p_t0_arg->taps->param.instance)) {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY;
    } else {
        prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_UNIFIED_KEY;
    }
    
    if (p_t0_arg->taps->param.mode == TAPS_ONCHIP_ALL) {
        prog_info.key_info[0].tableid = 0;
    } else if (p_t0_arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        prog_info.key_info[0].tableid = p_t0_arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    } else {
    prog_info.key_info[0].tableid = p_t0_arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    }
    prog_info.key_info[0].taps_seg = p_t0_arg->taps->segment;
    prog_info.key_info[0].shift[0] = 8; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 2;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 4;
    prog_info.key_shift[0] = 4;


    /* VRF + Source IP Address */
    prog_info.key_info[1].valid = TRUE;
    if (_TAPS_IS_PARALLEL_MODE_(p_t1_arg->taps->param.instance)) {
        prog_info.key_info[1].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY;
    } else {
        prog_info.key_info[1].lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_UNIFIED_KEY;
    }
   if (p_t1_arg->taps->param.mode == TAPS_ONCHIP_ALL) {
        prog_info.key_info[1].tableid = 0;
    } else if (p_t1_arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        prog_info.key_info[1].tableid = p_t1_arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    } else {
    prog_info.key_info[1].tableid = p_t1_arg->taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    }
    prog_info.key_info[1].taps_seg = p_t1_arg->taps->segment;
    prog_info.key_info[1].shift[0] = 8; /* bytes */
    prog_info.key_info[1].bytes_to_mask[0] = 2;
    prog_info.key_info[1].shift[1] = 4; /* bytes */
    prog_info.key_info[1].bytes_to_mask[1] = 4;
    prog_info.key_shift[1] = 4;



    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));
    tmu_taps_ucode_loaded = TRUE;

    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _TAPS_LRP_PRG_NUM_,
                                  _TAPS_TMU_PRG_NUM_, 0,
                                  TRUE, FALSE));

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info));

    if (_IS_MASTER_SHARE_LPM_TABLE(unit,  p_t0_arg->taps->master_unit, p_t0_arg->taps->param.host_share_table)
        && _IS_MASTER_SHARE_LPM_TABLE(unit,  p_t1_arg->taps->master_unit, p_t1_arg->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx <  p_t0_arg->taps->num_slaves; slave_idx++) {
            slave_unit =  p_t0_arg->taps->slave_units[slave_idx];
            rv =  taps_ut_setup_par_ucode_lookup(slave_unit, p_t0_arg, p_t1_arg);
            if (SOC_FAILURE(rv)) {
                break;
            }
        }
    }
    
    return rv;
}



int taps_ut_verify_par_ucode_lookup(int unit, taps_arg_t *p_taps_arg[], unsigned int p_payload[2][_TAPS_PAYLOAD_WORDS_])
{
    int i, j;
    int rv = SOC_E_NONE;
    int cmp_size = sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_;
    unsigned int ucode_key[3]={0}; /* Will contain a 16 bit VRF along with 32 bit DA and 32 bit SA */
    unsigned int ucode_payload[2][4];
    /* Check that the VRF is identical for the two entries. We will be testing
     * an entry of the form | 16 bits VRF|32 bits Dest IP|32 bits Source IP |    */

#if 0
    for (i = 0; i <2; i++) {
        cli_out(
                "\n$ Ucode Parallel  Lookup: taps:%d: 0x%x 0x%x  key length: %d\n",
                i, p_taps_arg[i]->key[0], p_taps_arg[i]->key[1],
                p_taps_arg[i]->length);
    }
#endif


    /* Build up the ucode key.
     * Need ucode key to be VRF(16) | SA(32) | DA(32) */

    /* Shift the values over based on the size
     * The microcode picks up values as
     * key[0] = bits31_0
     * key[1] = bits63_32
     * key[2] = bits95_64
     * */
    /* For now handle two cases */
    if(p_taps_arg[0]->length == TAPS_IPV4_MAX_VRF_SIZE)
    {
        ucode_key[0] = 0x0;
        ucode_key[1] = 0x0;
        ucode_key[2] = p_taps_arg[0]->key[1]; /* The upper 16 bits */
    }
    else if(p_taps_arg[0]->length == TAPS_IPV4_KEY_SIZE)
    {
        ucode_key[0] = p_taps_arg[0]->key[1]; /* DA */
        ucode_key[1] = p_taps_arg[1]->key[1]; /* SA */
        ucode_key[2] = p_taps_arg[0]->key[0]; /* The upper 16 bits */
    }
#if 0
    cli_out("Using key: 0x%x %x %x\n", ucode_key[2],
            ucode_key[1], ucode_key[0]);
#endif


    /* Load in the data using shared registers */
    SOC_IF_ERROR_RETURN(
            c3hppc_lrp_write_shared_register(unit, 16*0, ucode_key[0]));
    SOC_IF_ERROR_RETURN(
            c3hppc_lrp_write_shared_register(unit, 16*1, ucode_key[1]));
    SOC_IF_ERROR_RETURN(
            c3hppc_lrp_write_shared_register(unit, 16*2, ucode_key[2]));

    /* Run the microcode */
    { uint64 uuVal = COMPILER_64_INIT(0,100);
      SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, uuVal));
    }

    /* Load results back from Lookup */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            ucode_payload[i][j] =
                    c3hppc_lrp_read_shared_register(unit, 16 * j + 1 + i);
#if 0
            cli_out("Payload for TAPS %d = 0x%x \n", i,
                    ucode_payload[i][j]);
#endif

        }
        /* do the compare */
        rv = sal_memcmp(&p_payload[i][0], &ucode_payload[i][0], cmp_size);
        if (!SOC_SUCCESS(rv)) {
            cli_out(
                    "\n!!!! Microcode Lookup Taps:%dFailed to match Key: 0x%x 0x%x 0x%x\n"
                    "Expected:0x%x-0x%x-0x%x-0x%x\n"
                    "Ucode Payload:0x%x-0x%x-0x%x-0x%x  !!!\n", i,
                    ucode_key[2], ucode_key[1], ucode_key[0], p_payload[i][0],
                    p_payload[i][1], p_payload[i][2], p_payload[i][3],
                    ucode_payload[i][0], ucode_payload[i][1],
                    ucode_payload[i][2], ucode_payload[i][3]);
            return SOC_E_FAIL;
        }

    }
        return SOC_E_NONE;
}



int taps_verify_par_pfx(int unit, taps_arg_t *p_taps_arg[],  unsigned int p_payload[2][_TAPS_PAYLOAD_WORDS_],  uint8 fib_verify)
{
    unsigned int pyld[] = {0,0,0,0};
    int i;
    int rv=SOC_E_NONE;

    /* Verify routes are in the host tables */
    for(i=0; i <2; i++)
    {
        p_taps_arg[i]->payload = pyld;
        rv = taps_get_route(unit, p_taps_arg[i]);

        /* Restore the pointer */
        p_taps_arg[i]->payload = &p_payload[i][0];

        if (SOC_SUCCESS(rv)) {
            rv = taps_ut_payload_verify(p_taps_arg[i]->taps->param.mode,
                                &p_payload[i][0], &pyld[0]);
            
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to match Key: 0x%x 0x%x!!!\n",
                        _key[0], _key[1]);
                return SOC_E_FAIL;
            }
        } else {
            cli_out(
                    "\n!!!! Failed to get route 0x%x 0x%x route Eror:%d!!!\n",
                    p_taps_arg[i]->key[0] , p_taps_arg[i]->key[1] , rv);
            return SOC_E_FAIL;
        }
    }
    if (SAL_BOOT_PLISIM) return SOC_E_NONE;
    if (!fib_verify) return SOC_E_NONE;

    rv = taps_ut_verify_par_ucode_lookup(unit, p_taps_arg, p_payload);

    return rv;
}


int taps_parallel_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int num_routes;
    uint32 vrf_base= _VRF_BASE_;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;

    int i;
    int fibcnt = 0;
    taps_arg_t *p_taps_arg[2];
    int taps_i; /* Counter for taps instances */

    unsigned int key[2][BITS2WORDS(TAPS_IPV4_KEY_SIZE)] = {{0}};
    unsigned int key_p_payload[2][_TAPS_PAYLOAD_WORDS_] = { {0xba5eba11, 0x87654321, 0xdeadbeef, 0x7fffff},
                                                            {0xba5eba11, 0x12345678, 0xdeadbeef, 0x7fffff}};

    uint32 flags = TAPS_DUMP_TCAM_HW_PIVOT | TAPS_DUMP_TCAM_HW_BKT |
                TAPS_DUMP_SRAM_HW_PIVOT | TAPS_DUMP_SRAM_HW_BKT |
                TAPS_DUMP_DRAM_HW_PFX | TAPS_DUMP_DRAM_HW_BKT;

    /* Get the number of routes to run from input parameters */
    num_routes = testInfo->testNumRoutes;

    if (num_routes == 0) {
        cli_out(" Exiting Test with no routes\n");
        return SOC_E_NONE;
    }

    rv = taps_ut_setup_par_ucode_lookup(unit, &taps_test_param[0].arg,
            &taps_test_param[1].arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        return SOC_E_FAIL;
    }

    /* Insert routes into each TAPS Instance */
    cli_out(
            "Parallel TAPS Instances Test: VRF = 0x%x Number Routes 0x%x\n",
            vrf_base, num_routes);

    /* Insert VRF default routes for two taps instances (da and sa tables) */
    for (i = 0; i <2; i++) {
        key[i][0] = 0x0;
        key[i][1] = vrf_base;

        taps_test_param[i].arg.key = key[i];
        taps_test_param[i].arg.length = TAPS_IPV4_MAX_VRF_SIZE;
        key_p_payload[i][0] = fibcnt++;
        taps_test_param[i].arg.payload = &key_p_payload[i][0];
        /* coverity[ stack_use_overflow ] */
        rv = taps_insert_route(unit, &taps_test_param[i].arg);
        if (!SOC_SUCCESS(rv)) {
            cli_out(
                    "\n!!!! Failed to insert vpn def route TAPS Instance %d!!!\n",
                    i);
            return SOC_E_FAIL;
        }
        p_taps_arg[i] = &taps_test_param[i].arg;
    }

    /* Get and verify default vrf route */
    rv = taps_verify_par_pfx(unit, p_taps_arg, key_p_payload, TRUE);

    if (!SOC_SUCCESS(rv)) {
        cli_out("\n!!!! Failed to verify VRF default routes!!!\n");
        taps_dump(unit, p_taps_arg[0]->taps, flags);
        taps_dump(unit, p_taps_arg[1]->taps, flags);

        return SOC_E_FAIL;
    }
    /* Add routes to the two instances (as we would for SA/DA parallel lookups */
    key[0][0] = vrf_base;  /* VRF  */
    key[1][0] = vrf_base;  /* VRF */

    key[0][1] = 0x0a000000;  /* 10.0.0.0 start for DA */
    key[1][1] = 0xC0000000;  /* 192.0.0.0 start for SA */

    taps_test_param[0].arg.length = TAPS_IPV4_KEY_SIZE;
    taps_test_param[1].arg.length = TAPS_IPV4_KEY_SIZE;

    cli_out("## \nAdding 0x%x routes to each TAPS \n",num_routes);

    for (i = 0; i < num_routes; i++) {
        for(taps_i = 0; taps_i < 2;taps_i++)
        {
            key[taps_i][1]++; /*Bump up IP address */
            key_p_payload[taps_i][0]++; /* bump up the payload */

            /* Insert the route */
            rv = taps_insert_route(unit, &taps_test_param[taps_i].arg);
            if (!SOC_SUCCESS(rv)) {
                cli_out("\n%s:Failed to insert route TAPS Instance %d\n",
                        FUNCTION_NAME(), taps_i);
                taps_dump(unit, p_taps_arg[0]->taps, flags);
                taps_dump(unit, p_taps_arg[1]->taps, flags);
            }

        }

        if ((i+1) % 1024 == 0) {
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Added %d(K) routes to Each FIB \n"),
                       (i+1) / 1024));
        }
    }

    /* The actual number of routes we inserted successfully */

    num_routes = i;

    /* Do the verification */
    cli_out("## \nVerifying %d(K) routes in each FIB \n", num_routes/ 1024);

    key[0][1] = 0x0a000000;  /* 10.0.0.0 start for DA */
    key[1][1] = 0xC0000000;  /* 192.0.0.0 start for SA */

    key_p_payload[0][0] = 0; /* Set the payloads to match insertions */
    key_p_payload[1][0] = 1;

    for (i = 0; i < num_routes; i++) {

        key[0][1]++; /* Bump up the DA */
        key[1][1]++; /* Bump up the SA */

        /* Set the payloads to check */
        key_p_payload[0][0]++; /* Bump up payload for DA */
        key_p_payload[1][0]++; /* Bump up payload for SA */


        /* Get and verify default vrf route */
        rv = taps_verify_par_pfx(unit, p_taps_arg, key_p_payload, TRUE);

        if (!SOC_SUCCESS(rv)) {
            cli_out("\n!!!! Failed to verify Routes!!!\n");
            if(testInfo->testVerbFlag)
            {
                taps_dump(unit, p_taps_arg[0]->taps, flags);
                taps_dump(unit, p_taps_arg[1]->taps, flags);
            }
            return SOC_E_FAIL;
        }

        if ((i+1) % 1024 == 0)
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Verified %d(K) routes in Each FIB \n"),
                       (i+1) / 1024));

    }

        return SOC_E_NONE;

}


int taps_ipv6_test_cleanup(int unit, void *data)
{
    int rv = SOC_E_NONE;


    return rv;
}



int taps_par_test_cleanup(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int i;

    for (i = 0; i < 2; i++) {
        rv = taps_destroy(unit, taps_test_param[i].taps);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to Cleanup Table: %d !!! \n", i);
            return rv;
        }

        soc_sbx_caladan3_tmu_program_free(unit, _TAPS_TMU_PRG_NUM_);

    }
    return rv;
}

int taps_ipv4_customized_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;

    return rv;
}

/* add incremental routes, increment by 1 */
static int taps_ipv6_add_routes(taps_route_param_t *route, uint32 *key_base, uint32 key_length, int num_keys, uint32 payload_cookie) {
    uint32 route_index;
    int rv = SOC_E_NONE;
    for (route_index = 0; route_index < num_keys; route_index++) {
        route[route_index].key_length = key_length + TAPS_IPV6_MAX_VRF_SIZE;
        route[route_index].keys[4] = key_base[4]+route_index;
        if (route[route_index].keys[4] < key_base[3]) {
            /* carry */
            route[route_index].keys[3] = key_base[3] + 1;
        } else {
            route[route_index].keys[3] = key_base[3];
        }
        route[route_index].keys[2] = key_base[2];
        route[route_index].keys[1] = key_base[1];
        route[route_index].keys[0] = key_base[0];

        route[route_index].payload[0] = 0;
        route[route_index].payload[1] = 0;
        route[route_index].payload[2] = payload_cookie;
        route[route_index].payload[3] = route_index;
    }          
    return rv;
}

typedef struct _taps_ipv6_key_info_s {
    uint32 keys[TAPS_IPV6_KEY_SIZE_WORDS];
    uint32 key_len; 
    uint32 num_keys;
} taps_ipv6_key_info_t; 


/* all vrf 0, incremental, mixed length */
#define _V6KEY_INFO0_NUM_KEYGROUPS (15)
taps_ipv6_key_info_t v6key_info0[_V6KEY_INFO0_NUM_KEYGROUPS] = \
{ {{0, (0x2000<<16)+0, 0,         0,         0},               114, 25},
  {{0, (0x2000<<16)+0, 0,         0,         0x7<<16},         112, 25},
  {{0, (0x2000<<16)+0, 0,         0,         0x20<<16},        110, 25},
  {{0, (0x2000<<16)+0, 0,         0,         0x90<<16},        108, 50},
  {{0, (0x2000<<16)+0, 0,         0,         0x3c0<<16},       106, 50},
  {{0, (0x2000<<16)+0, 0,         0,         0x1100<<16},      104, 75},
  {{0, (0x2000<<16)+0, 0,         1,         0},               96,  100},
  {{0, (0x2000<<16)+0, 0,         0x100,     0},               88,  150},
  {{0, (0x2000<<16)+0, 0,         1<<16,     0},               80,  250},
  {{0, (0x2000<<16)+0, 0,         0x100<<16, 0},               72,  750},
  {{0, (0x2000<<16)+0, 3,         0,         0},               64,  2500},
  {{0, (0x2000<<16)+0, 0xa00,     0,         0},               56,  500},
  {{0, (0x2000<<16)+0, 2<<16,     0,         0},               48,  250},
  {{0, (0x2000<<16)+0, 0x100<<16, 0,         0},               40,  150},
  {{0, (0x2000<<16)+1, 0,         0,         0},               32,  100},
};
    
int taps_ipv6_customized_test0(int unit, c3sw_test_info_t *testInfo, uint32 prefixSeed) {
    int vrf, loop, num_loops, num_routes, index, vindex, shuffle = TRUE;
    int key_group, max_routes;
    uint32 ipv6_key[TAPS_IPV6_KEY_SIZE_WORDS];
    int fibcnt = 0;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    taps_route_param_t tmp_route;
    int rv = SOC_E_NONE;
    
    num_loops = 1000000;
    max_routes = 10000;
    cli_out("## Test SEED = 0x%x \n", prefixSeed);
    sal_srand(prefixSeed);

    routes = sal_alloc(sizeof(taps_route_param_t)*max_routes, "Route info");
    if (routes == NULL) {
        return SOC_E_MEMORY;
    }

    /* insert all vrf defaults */
    for (vrf = 0; vrf < 4096; vrf++) {
        arg->key = &ipv6_key[0];
        ipv6_key[0] = 0;
        ipv6_key[1] = 0;
        ipv6_key[2] = 0;
        ipv6_key[3] = 0;
        ipv6_key[4] = vrf;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    cli_out("\n Inserted all default vrfs!!!\n");

    /* create routes */
    num_routes = 0;
    /* shift the keys based on key length */
    for (key_group = 0; key_group < _V6KEY_INFO0_NUM_KEYGROUPS; key_group++) {
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, v6key_info0[key_group].keys, TAPS_IPV6_KEY_SIZE,
                            TAPS_IPV6_KEY_SIZE - v6key_info0[key_group].key_len - TAPS_IPV6_MAX_VRF_SIZE);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to shift key base for key group %d!!!\n", key_group);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            tmu_taps_ut_test_result = -1;
            return rv;
        }
    }

    /* add all keys into routes */
    for (key_group = 0; key_group < _V6KEY_INFO0_NUM_KEYGROUPS; key_group++) {
        rv = taps_ipv6_add_routes(&routes[num_routes], &v6key_info0[key_group].keys[0],
                                  v6key_info0[key_group].key_len, v6key_info0[key_group].num_keys, key_group);

        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to add key group %d!!!\n", key_group);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            tmu_taps_ut_test_result = -1;
            return rv;
        }
        num_routes += v6key_info0[key_group].num_keys;
    }

    for (index = 0; index < num_routes; index++) {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                   routes[index].keys[0]&0xFFFF,
                   routes[index].keys[1], routes[index].keys[2],
                   routes[index].keys[3], routes[index].keys[4], 
                   routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
    }

    /* loop, random insert/delete */
    for (loop = 0; loop < num_loops; loop++) {
        cli_out("\n Loop %d!!!\n", loop);

        /* shuffle routes */
        if (shuffle) {
            /* shuffle routes to create random route insert pattern */
            for (index = 0; index < num_routes; index++) {
                vindex = sal_rand() % num_routes;
                if (index != vindex) {
                    /* swap the key/length of 2 routes */
                    sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
                }
            }
        }
        
        /* insert routes */
        for (index = 0; index < num_routes; index++) {
            if (index == 0) {
                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "first inserted Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                             routes[index].keys[0]&0xFFFF,
                             routes[index].keys[1], routes[index].keys[2],
                             routes[index].keys[3], routes[index].keys[4], 
                             routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
                
            }
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                       routes[index].keys[0]&0xFFFF,
                       routes[index].keys[1], routes[index].keys[2],
                       routes[index].keys[3], routes[index].keys[4], 
                       routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));

            arg->taps = taps_test_param[testInfo->testTapsInst].taps;
            arg->key = &routes[index].keys[0];
            arg->length = routes[index].key_length;
            arg->payload = routes[index].payload;

            /* insert route */
            rv = taps_insert_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to insert %dth route %x!!!\n", index, rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                tmu_taps_ut_test_result = -1;
                return rv;
            }            

        }
        
        cli_out("\n Loop %d inserted all routes!!!\n", loop);

        /* shuffle routes */
        if (shuffle) {
            /* shuffle routes to create random route delete pattern */
            for (index = 0; index < num_routes; index++) {
                vindex = sal_rand() % num_routes;
                if (index != vindex) {
                    /* swap the key/length of 2 routes */
                    sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
                }
            }
        }
        
        /* delete routes */
        for (index = 0; index < num_routes; index++) {
            if (index == 0) {
                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "first deleted Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                             routes[index].keys[0]&0xFFFF,
                             routes[index].keys[1], routes[index].keys[2],
                             routes[index].keys[3], routes[index].keys[4], 
                             routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
                
            }
            arg->taps = taps_test_param[testInfo->testTapsInst].taps;
            arg->key = &routes[index].keys[0];
            arg->length = routes[index].key_length;

            /* insert route */
            rv = taps_delete_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to delete %dth route !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return rv;
            }                        
        }
        
        cli_out("\n Loop %d deleted all routes!!!\n", loop);
    }

    /* make sure all default vrfs are still there */

    tmu_taps_ut_test_result = 0;
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

#define _V6KEY_INFO1_NUM_KEYGROUPS (15)
taps_ipv6_key_info_t v6key_info1[_V6KEY_INFO1_NUM_KEYGROUPS] = \
{ {{0, (0x2000<<16)+0, 0,         0,         0},               114, 250},
  {{0, (0x2000<<16)+0, 0,         0,         0x7<<16},         112, 250},
  {{0, (0x2000<<16)+0, 0,         0,         0x20<<16},        110, 250},
  {{0, (0x2000<<16)+0, 0,         0,         0x90<<16},        108, 500},
  {{0, (0x2000<<16)+0, 0,         0,         0x3c0<<16},       106, 500},
  {{0, (0x2000<<16)+0, 0,         0,         0x1100<<16},      104, 750},
  {{0, (0x2000<<16)+0, 0,         1,         0},               96,  1000},
  {{0, (0x2000<<16)+0, 0,         0x100,     0},               88,  1500},
  {{0, (0x2000<<16)+0, 0,         1<<16,     0},               80,  2500},
  {{0, (0x2000<<16)+0, 0,         0x100<<16, 0},               72,  7500},
  {{0, (0x2000<<16)+0, 3,         0,         0},               64,  25000},
  {{0, (0x2000<<16)+0, 0xa00,     0,         0},               56,  5000},
  {{0, (0x2000<<16)+0, 2<<16,     0,         0},               48,  2500},
  {{0, (0x2000<<16)+0, 0x100<<16, 0,         0},               40,  1500},
  {{0, (0x2000<<16)+1, 0,         0,         0},               32,  1000},
};
    
int taps_ipv6_customized_test1(int unit, c3sw_test_info_t *testInfo, uint32 prefixSeed) {
    int vrf, loop, num_loops, num_routes, index, vindex, shuffle = TRUE;
    int key_group, max_routes;
    uint32 ipv6_key[TAPS_IPV6_KEY_SIZE_WORDS];
    int fibcnt = 0;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    taps_route_param_t tmp_route;
    int rv = SOC_E_NONE;
    
    num_loops = 1000000;
    max_routes = 100000;
    cli_out("## Test SEED = 0x%x \n", prefixSeed);
    sal_srand(prefixSeed);

    routes = sal_alloc(sizeof(taps_route_param_t)*max_routes, "Route info");
    if (routes == NULL) {
        return SOC_E_MEMORY;
    }

    /* insert all vrf defaults */
    for (vrf = 0; vrf < 1; vrf++) {
        arg->key = &ipv6_key[0];
        ipv6_key[0] = 0;
        ipv6_key[1] = 0;
        ipv6_key[2] = 0;
        ipv6_key[3] = 0;
        ipv6_key[4] = vrf;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    cli_out("\n Inserted all default vrfs!!!\n");

    /* create routes */
    num_routes = 0;
    /* shift the keys based on key length */
    for (key_group = 0; key_group < _V6KEY_INFO1_NUM_KEYGROUPS; key_group++) {
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, v6key_info1[key_group].keys, TAPS_IPV6_KEY_SIZE,
                            TAPS_IPV6_KEY_SIZE - v6key_info1[key_group].key_len - TAPS_IPV6_MAX_VRF_SIZE);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to shift key base for key group %d!!!\n", key_group);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            tmu_taps_ut_test_result = -1;
            return rv;
        }
    }

    /* add all keys into routes */
    for (key_group = 0; key_group < _V6KEY_INFO1_NUM_KEYGROUPS; key_group++) {
        rv = taps_ipv6_add_routes(&routes[num_routes], &v6key_info1[key_group].keys[0],
                                  v6key_info1[key_group].key_len, v6key_info1[key_group].num_keys, key_group);

        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to add key group %d!!!\n", key_group);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            tmu_taps_ut_test_result = -1;
            return rv;
        }
        num_routes += v6key_info1[key_group].num_keys;
    }

    for (index = 0; index < num_routes; index++) {
        LOG_DEBUG(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                   routes[index].keys[0]&0xFFFF,
                   routes[index].keys[1], routes[index].keys[2],
                   routes[index].keys[3], routes[index].keys[4], 
                   routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
    }

    /* loop, random insert/delete */
    for (loop = 0; loop < num_loops; loop++) {
        cli_out("\n Loop %d!!!\n", loop);

        /* shuffle routes */
        if (shuffle) {
            /* shuffle routes to create random route insert pattern */
            for (index = 0; index < num_routes; index++) {
                vindex = sal_rand() % num_routes;
                if (index != vindex) {
                    /* swap the key/length of 2 routes */
                    sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
                }
            }
        }
        
        /* insert routes */
        for (index = 0; index < num_routes; index++) {
            if (index == 0) {
                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "first inserted Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                             routes[index].keys[0]&0xFFFF,
                             routes[index].keys[1], routes[index].keys[2],
                             routes[index].keys[3], routes[index].keys[4], 
                             routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
                
            }
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                       routes[index].keys[0]&0xFFFF,
                       routes[index].keys[1], routes[index].keys[2],
                       routes[index].keys[3], routes[index].keys[4], 
                       routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));

            arg->taps = taps_test_param[testInfo->testTapsInst].taps;
            arg->key = &routes[index].keys[0];
            arg->length = routes[index].key_length;
            arg->payload = routes[index].payload;

            /* insert route */
            rv = taps_insert_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to insert %dth route %x!!!\n", index, rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                tmu_taps_ut_test_result = -1;
                return rv;
            }            

        }
        
        cli_out("\n Loop %d inserted all routes!!!\n", loop);

        /* shuffle routes */
        if (shuffle) {
            /* shuffle routes to create random route delete pattern */
            for (index = 0; index < num_routes; index++) {
                vindex = sal_rand() % num_routes;
                if (index != vindex) {
                    /* swap the key/length of 2 routes */
                    sal_memcpy(&tmp_route, &routes[index], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[index], &routes[vindex], sizeof(taps_route_param_t));
                    
                    sal_memcpy(&routes[vindex], &tmp_route, sizeof(taps_route_param_t));
                }
            }
        }
        
        /* delete routes */
        for (index = 0; index < num_routes; index++) {
            if (index == 0) {
                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "first deleted Vrf %d %8x.%8x.%8x.%8x/%d\n"),
                             routes[index].keys[0]&0xFFFF,
                             routes[index].keys[1], routes[index].keys[2],
                             routes[index].keys[3], routes[index].keys[4], 
                             routes[index].key_length-TAPS_IPV4_MAX_VRF_SIZE));
                
            }
            arg->taps = taps_test_param[testInfo->testTapsInst].taps;
            arg->key = &routes[index].keys[0];
            arg->length = routes[index].key_length;

            /* insert route */
            rv = taps_delete_route(unit, arg);
            if (SOC_FAILURE(rv)) {
                cli_out("\n!!!! Failed to delete %dth route !!!\n", fibcnt);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return rv;
            }                        
        }
        
        cli_out("\n Loop %d deleted all routes!!!\n", loop);
    }

    /* make sure all default vrfs are still there */

    tmu_taps_ut_test_result = 0;
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

int taps_ipv6_customized_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    int test_case, prefixSeed;

    test_case = testInfo->testCase;    
    prefixSeed = testInfo->nTestSeed;

    switch (test_case) {
        case 0:
            rv = taps_ipv6_customized_test0(unit, testInfo, prefixSeed);
            break;
        case 1:
            rv = taps_ipv6_customized_test1(unit, testInfo, prefixSeed);
            break;
        default:
            break;
    }

    return rv;
}

/*********************************************
* Delete test case
* 1. Insert 4096 vrf default routes
* 2. Insert 127.0.0.1 for each vrf
* 3. Insert random routes.
* 4. Shuffle all of the routes
* 5. Delete routes with random sequence
* 6. Verify each route whether it could get the bpm 
* 7. Optional: If enable route_flapping, then repeat insert/deleter all routes for 10 times.
*                   We will check memory leak, update rate at here
***********************************************/
int taps_ipv4_delete_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, netmask_dist, 
        netmask_dist_num, num_routes, route_idx, shuffle_route_idx, 
        vrf_idx, route_flapping, route_flapping_times, route_flapping_num;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    taps_route_param_t shuffle_route;
    int dump_flags;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);
    
    /* Always do route flapping */
    route_flapping = TRUE;

    /* Get random netmask start len.
        * Currently netmask would start from 8 or 16
        */

    netmask_dist_num = 2;
    
    netmask_dist = sal_rand() % netmask_dist_num;
    switch (netmask_dist) {
        case 0:
            netmask_start = 8;
            break;
        case 1:
            netmask_start = 16;
            break;  
        default:
            netmask_start = 8;
            break;
    }

    /*Get random vrf_count and vrf */
    single_vrf = (sal_rand() % 2 == 0);
    vrf_count = sal_rand() % 4096; 

    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }
    
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);

    /* Memory leak check */
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n",
            record_mem, record_mem_max);
#endif


    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* add 127.0.0.0/8 to match customer test case */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0xF0000000; /* mark this payload as a 127.0.0.0/8 route payload */
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Insert routes
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes[route_idx].keys[0] = 0;
        } else {
            routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
        }
        
        if (netmask_len == 32) {
            routes[route_idx].keys[1] = ipaddr;
        } else {
            routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
        }
        routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;
            

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
        routes[route_idx].payload[0] = route_idx;     

        /* Insert this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }

    /* This test case if for delete, so we don't do verify after insert */

    /****************************************************
        *Shuffle routes
        ****************************************************/

    /* To delete with random sequence, we need to shuffle routes */
    /* shuffle routes to create random route insert pattern */
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        shuffle_route_idx = sal_rand() % num_routes;
        if (shuffle_route_idx != route_idx) {
            /* swap the key/length of 2 routes */
            sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
        }
    }

    /****************************************************
        *Delete routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Delete routes "
                    "key 0x%x 0x%x length %d !!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        /* Get this route, should return SOC_E_NOT_FOUND */
        rv = taps_get_route(unit, arg);
        if (rv != SOC_E_NOT_FOUND) {
            cli_out("\n!!!! Failed to verify route: key 0x%x 0x%x length %d !!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        /* Should find out the bpm route */
        rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Deleted and verified %d(K) routes after insert \n", route_idx/1024);
        }
    }

    /* Route flapping test, will insert/delete all routes for 10 times */
    if (route_flapping) {
        route_flapping_num = 1;
        for (route_flapping_times = 0; route_flapping_times < route_flapping_num; 
            route_flapping_times++) {
            /* Insert routes */
            TIME_STAMP_START
            for (route_idx = 0; route_idx < num_routes; route_idx++) {
                arg->length = routes[route_idx].key_length;
                arg->key = &routes[route_idx].keys[0];
                arg->payload= &routes[route_idx].payload[0];
                rv = taps_insert_route(unit, arg);
                if (SOC_FAILURE(rv)) {
                    cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                            "for vrf %d !!!\n", 
                            vrf_idx);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
            TIME_STAMP("### Total Route Update time");
            TIME_STAMP_RATE("### Average Route Update rate", num_routes);
            /* Verify?? */
            /* Delete routes */
            for (route_idx = 0; route_idx < num_routes; route_idx++) {
                arg->length = routes[route_idx].key_length;
                arg->key = &routes[route_idx].keys[0];
                arg->payload= &routes[route_idx].payload[0];
                rv = taps_delete_route(unit, arg);
                if (SOC_FAILURE(rv)) {
                    cli_out("\n!!!! Failed to Delete routes "
                            "key 0x%x 0x%x length %d !!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
        }
    }

    /****************************************************
        *Delete loopback route and vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        /* Delete 127.0.0.0/8 */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
   
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL; 
    }
#endif  
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

/*********************************************
* Delete test case
* 1. Insert 4096 vrf default routes
* 2. Insert 127.0.0.1 for each vrf
* 3. Insert random routes.
* 4. Shuffle all of the routes
* 5. Delete routes with random sequence
* 6. Verify each route whether it could get the bpm 
* 7. Optional: If enable route_flapping, then repeat insert/deleter all routes for 10 times.
*                   We will check memory leak, update rate at here
***********************************************/
int taps_ipv6_delete_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, netmask_dist, 
        netmask_dist_num, num_routes, route_idx, shuffle_route_idx, 
        vrf_idx, route_flapping, route_flapping_times, route_flapping_num;
    int netmask_len;
    uint32 ipaddr[4], vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    taps_route_param_t shuffle_route;
    int dump_flags;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif
    
    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    /* Always do route flapping */
    route_flapping = TRUE;

    /* Get random netmask start len.
        * Currently netmask would start from 8, 16, 32, 64 or 96
        */

    netmask_dist_num = 5;

    netmask_dist = sal_rand() % netmask_dist_num;
    switch (netmask_dist) {
    case 0:
        netmask_start = 8;
        break;
    case 1:
        netmask_start = 16;
        break; 
    case 2:
        netmask_start = 32;
        break; 
    case 3:
        netmask_start = 64;
        break; 
    case 4:
        netmask_start = 96;
        break; 
    default:
        netmask_start = 8;
        break;
    }

    /*Get random vrf_count and vrf */
    single_vrf = (sal_rand() % 2 == 0);
    vrf_count = sal_rand() % 4096; 


    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ipv6_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    /* Set up the IPv6 keyploder to use this taps table */
    rv=taps_ut_setup_ipv6_keyploder(unit, arg);    
    if (SOC_FAILURE(rv)) {
        cli_out("!!! Failed to setup IPv6 keyploder !!!\n");
    }

    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);

    /* Memory leak check */
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n",
            record_mem, record_mem_max);
#endif

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }

    /****************************************************
        *Insert routes
        ****************************************************/
    route_idx = 0;
    fibcnt = 0;
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV6_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr[0] = V4_RAND();
        ipaddr[1] = V4_RAND();
        ipaddr[2] = V4_RAND();
        ipaddr[3] = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }
        /* align the keys by shifting */
        routes[route_idx].keys[0] = vrf;
        routes[route_idx].keys[1] = ipaddr[0];
        routes[route_idx].keys[2] = ipaddr[1];
        routes[route_idx].keys[3] = ipaddr[2];
        routes[route_idx].keys[4] = ipaddr[3];
        routes[route_idx].key_length =  TAPS_IPV6_MAX_VRF_SIZE + netmask_len;
            
        rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].keys,
                            TAPS_IPV6_KEY_SIZE, 
                            TAPS_IPV6_PFX_SIZE - netmask_len);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        rv =  taps_key_shift(TAPS_IPV6_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    netmask_len - TAPS_IPV6_PFX_SIZE);
        if (SOC_FAILURE(rv)) {
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        routes[route_idx].payload[0] = route_idx;     

        /* Insert this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];

        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x 0x%x 0x%x "
                        "0x%x length %d : %s !!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                        routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                        soc_errmsg(rv));
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
        }
        route_idx++;
    }

    /* This test case if for delete, so we don't do verify after insert */

    /****************************************************
        *Shuffle routes
        ****************************************************/
    /* To delete with random sequence, we need to shuffle routes */
    /* shuffle routes to create random route insert pattern */
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        shuffle_route_idx = sal_rand() % num_routes;
        if (shuffle_route_idx != route_idx) {
            /* swap the key/length of 2 routes */
            sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
        }
    }

    /****************************************************
        *Delete routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Delete routes key 0x%x 0x%x 0x%x 0x%x "
                    "0x%x length %d : %s !!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                    routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                    soc_errmsg(rv));
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        /* Get this route, should return SOC_E_NOT_FOUND */
        rv = taps_get_route(unit, arg);
        if (rv != SOC_E_NOT_FOUND) {
             cli_out("\n!!!! Failed to verify routes key 0x%x 0x%x 0x%x 0x%x "
                     "0x%x length %d : %s !!!\n", 
                     routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].keys[2],
                     routes[route_idx].keys[3], routes[route_idx].keys[4], routes[route_idx].key_length,
                     soc_errmsg(rv));
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        /* Should find out the bpm route */
        rv = taps_verify_ipv6_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x 0x%x 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1],
                    routes[route_idx].keys[2], routes[route_idx].keys[3],
                    routes[route_idx].keys[4], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Deleted and verified %d(K) routes after insert \n", route_idx/1024);
        }
    }
        /* Route flapping test, will insert/delete all routes for 10 times */
    if (route_flapping) {
        route_flapping_num = 1;
        for (route_flapping_times = 0; route_flapping_times < route_flapping_num; 
            route_flapping_times++) {
            /* Insert routes */

            TIME_STAMP_START
            for (route_idx = 0; route_idx < num_routes; route_idx++) {
                arg->length = routes[route_idx].key_length;
                arg->key = &routes[route_idx].keys[0];
                arg->payload= &routes[route_idx].payload[0];
                rv = taps_insert_route(unit, arg);
                if (SOC_FAILURE(rv)) {
                    cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                            "for vrf %d !!!\n", 
                            vrf_idx);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
            TIME_STAMP("### Total Route Update time");
            TIME_STAMP_RATE("### Average Route Update rate", num_routes);
            /* Verify?? */
            /* Delete routes */
            for (route_idx = 0; route_idx < num_routes; route_idx++) {
                arg->length = routes[route_idx].key_length;
                arg->key = &routes[route_idx].keys[0];
                arg->payload= &routes[route_idx].payload[0];
                rv = taps_delete_route(unit, arg);
                if (SOC_FAILURE(rv)) {
                    cli_out("\n!!!! Failed to Delete routes "
                            "key 0x%x 0x%x length %d !!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                }
            }
        }
    }

    /****************************************************
        *Delete vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = 0;
        _key[2] = 0;
        _key[3] = 0;
        _key[4] = vrf_idx;
        arg->length = TAPS_IPV6_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL; 
    }
#endif
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}

int watch_point[100];
int taps_error_handling_test_run(int unit, void *data)
{
    int rv = SOC_E_NONE;
    int fibcnt = 0, prefixSeed, single_vrf, vrf_count, netmask_start, num_routes, base_route_num, 
        route_idx, vrf_idx, watch_point_index = 0, watch_point_max, verify_eh = FALSE, 
        watch_point_set = TRUE, index;
    int netmask_len;
    uint32 ipaddr, vrf;
    c3sw_test_info_t *testInfo = (c3sw_test_info_t *)data;
    taps_arg_t *arg = &taps_test_param[testInfo->testTapsInst].arg;
    taps_route_param_t *routes = NULL;
    int dump_flags;
    int skip_wp_array[INS_ST_WP_MAX], skip_wp_idx = 0, skip_wp_cnt = 0;
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int cur_mem, cur_mem_max, record_mem, record_mem_max;
#endif

    dump_flags = TAPS_DUMP_TCAM_ALL | TAPS_DUMP_SRAM_ALL | TAPS_DUMP_DRAM_ALL;
    num_routes = testInfo->testNumRoutes;
    prefixSeed = testInfo->nTestSeed;

    sal_srand(prefixSeed);

    netmask_start = 16;
    
    /****************************************************
        *Setup ucode
        ****************************************************/
    rv = taps_ut_setup_ucode_lookup(unit, arg);
    if (SOC_FAILURE(rv)) {
        cli_out("\n!!!! Failed to setup ucode !!!\n");
        tmu_taps_ut_test_result = -1;
        return SOC_E_FAIL;
    }

    sal_memset(watch_point, 0, sizeof(int) * 100);
    routes = sal_alloc(sizeof(taps_route_param_t) * num_routes, "Route info");
    
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&record_mem, &record_mem_max);
    cli_out("\nBefore insert Mem Begin(%d) Max(%d) \n", record_mem, record_mem_max);
#endif

    /****************************************************
        *Insert vrf default route and loopback route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0x80000000; /* mark this payload as a vrf route payload */
        arg->payload = &_key_pyld[0];
        /* coverity[ stack_use_overflow ] */        
        rv = taps_insert_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to insert vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }

        rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                            arg->taps->master_unit, 
                                                            arg->taps->param.host_share_table));
        if (SOC_FAILURE(rv)) {
            cli_out("taps_flush_cache Flush failed err=%d\n", rv);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* get and verify default vrf route */
        rv = taps_verify_pfx(unit, arg, FALSE);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify vrf routes:%d !!!\n", fibcnt);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        
        /* add 127.0.0.0/8 to match customer test case */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        _key_pyld[0] = fibcnt++;
        _key_pyld[0] |= 0xF0000000; /* mark this payload as a 127.0.0.0/8 route payload */
        arg->payload = &_key_pyld[0];
        
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Insert 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
    
    route_idx = 0;
    fibcnt = 0;
    single_vrf = (sal_rand() % 2 == 0);
    vrf_count = sal_rand() % 4096;
    sal_memset(routes, 0, sizeof(taps_route_param_t) * num_routes);

    if (arg->taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        skip_wp_array[skip_wp_cnt++] = INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_1;
        skip_wp_array[skip_wp_cnt++] = INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_2;
    }

    base_route_num = num_routes/2;
    watch_point_index = testInfo->testCase;
    if (arg->taps->param.mode == TAPS_OFFCHIP_ALL) {
        watch_point_max = (int)INS_ST_WP_MAX;
    } else {
        watch_point_max = (int)INS_ST_ONCHIP_WP_MAX;
    }
    
    /****************************************************
        *Generate and Insert routes with random value&length
        ****************************************************/
    while (TRUE) {
        if (route_idx >= num_routes) {
            break;
        } else if (route_idx >= base_route_num) {
            if (watch_point_index < watch_point_max && !verify_eh) {
                for (skip_wp_idx = 0; skip_wp_idx < skip_wp_cnt; skip_wp_idx++) {
                    if (skip_wp_array[skip_wp_idx] == watch_point_index) {
                        watch_point_set = FALSE;
                        break;
                    }
                }
                if (watch_point_set) {
                    watch_point[watch_point_index] = TRUE; 
                    verify_eh = TRUE;
                    cli_out("\n###Look for taps error at watch point %d ###\n", watch_point_index);
                } else {
                    watch_point_index++;
                    watch_point_set = TRUE;
                }
            }
        }
        /* Generate route */
        /* Random IP length between 16-32*/
        netmask_len = netmask_start + sal_rand() % (TAPS_IPV4_PFX_SIZE - netmask_start + 1);

        /* IP value */
        ipaddr = V4_RAND();

        /* Vrf value */
        if (single_vrf) {
            vrf = vrf_count;
        } else {
            vrf = sal_rand() % vrf_count;
        }

        /* align the keys by shifting */
        if (netmask_len <= (32 - TAPS_IPV4_MAX_VRF_SIZE)) {
            routes[route_idx].keys[0] = 0;
        } else {
            routes[route_idx].keys[0] = (vrf >> (32 - netmask_len));
        }
        
        if (netmask_len == 32) {
            routes[route_idx].keys[1] = ipaddr;
        } else {
            routes[route_idx].keys[1] = (ipaddr & ((1 << netmask_len)-1)) | \
                ((vrf & ((1<<(32-netmask_len))-1)) << netmask_len);
        }
        routes[route_idx].key_length =  TAPS_IPV4_MAX_VRF_SIZE + netmask_len;
            

        /* Construct match_key */
        sal_memcpy(routes[route_idx].match_key, routes[route_idx].keys, 
                sizeof(uint32) * BITS2WORDS(TAPS_MAX_KEY_SIZE));
        taps_key_shift(TAPS_IPV4_KEY_SIZE, routes[route_idx].match_key,
                    routes[route_idx].key_length, 
                    routes[route_idx].key_length - TAPS_IPV4_KEY_SIZE);
        routes[route_idx].payload[0] = route_idx;     

        /* Try to get this route */
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_insert_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            if (rv == SOC_E_EXISTS) {
                /* Exist route, then try to get another one */
                continue;
            } else {
                if (!verify_eh) {
                    cli_out("\n!!!! Failed to Insert routes key 0x%x 0x%x length %d : %s !!!\n", 
                            routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length,
                            soc_errmsg(rv));
                    tmu_taps_ut_test_result = -1;
                    /* coverity[check_after_deref] */
                    if (routes) sal_free(routes);
                    return SOC_E_FAIL;
                } else {
                    taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                            arg->taps->master_unit, 
                                            arg->taps->param.host_share_table));
                    for (index = 0; index < route_idx; index++) {
                        /* Should find out the bpm route */
                        rv = taps_verify_bpm(unit, arg, &routes[index]);
                        if (SOC_FAILURE(rv)) {
                            taps_dump(unit, arg->taps, dump_flags);
                            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                                    routes[index].keys[0], routes[index].keys[1], routes[index].key_length);
                            /* coverity[check_after_deref] */
                            if (routes) sal_free(routes);
                            return SOC_E_FAIL;
                        }
                        if (index == route_idx - 1) {
                            cli_out("###Verified over %d routes for Watch point %d ###\n", route_idx, watch_point_index);
                        }
                    }
                    watch_point[watch_point_index] = FALSE; 
                    verify_eh = FALSE;
                    watch_point_index++;
                }
            }
        } else {
            if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
                cli_out("!!! generated and inserted %d(K) routes\n", route_idx/1024);
            }
            route_idx++;
        }
    }
    /****************************************************
        *Verify routes
        ****************************************************/
    rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                        arg->taps->master_unit, 
                                                        arg->taps->param.host_share_table));
    if (SOC_FAILURE(rv)) {
        cli_out("taps_flush_cache Flush failed err=%d\n", rv);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL;
    }
    
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        /* Should find out the bpm route */
        arg->key = &(routes[route_idx].keys[0]);
        arg->length = routes[route_idx].key_length;
        rv = taps_get_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to get bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
        if (SOC_FAILURE(rv)) {
            taps_dump(unit, arg->taps, dump_flags);
            cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Verified %d(K) routes after insert \n", route_idx/1024);
        }
    }
#if 0
    /* To delete with random sequence, we need to shuffle routes */
    /* shuffle routes to create random route insert pattern */
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        shuffle_route_idx = sal_rand() % num_routes;
        if (shuffle_route_idx != route_idx) {
            /* swap the key/length of 2 routes */
            sal_memcpy(&shuffle_route, &routes[route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[route_idx], &routes[shuffle_route_idx], sizeof(taps_route_param_t));
            
            sal_memcpy(&routes[shuffle_route_idx], &shuffle_route, sizeof(taps_route_param_t));
        }
    }
#endif
   /****************************************************
        *Delete routes
        ****************************************************/
    for (route_idx = 0; route_idx < num_routes; route_idx++) {
        arg->length = routes[route_idx].key_length;
        arg->key = &routes[route_idx].keys[0];
        arg->payload= &routes[route_idx].payload[0];
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to Delete routes "
                    "key 0x%x 0x%x length %d!!!\n", 
                    routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        } else {
#if 0
            /* Get routes, should return not found */
            rv = taps_get_route(unit, arg);
            if (rv != SOC_E_NOT_FOUND) {
                cli_out("\n!!!! Failed to Delete routes "
                        "key 0x%x 0x%x length %d : return %d!!!\n", 
                        routes[route_idx].keys[0], routes[route_idx].keys[1], 
                        routes[route_idx].key_length, rv);
                tmu_taps_ut_test_result = -1;
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            } 
            rv = taps_host_flush_cache(unit, _IS_MASTER_SHARE_LPM_TABLE(unit, 
                                                arg->taps->master_unit, 
                                                arg->taps->param.host_share_table));
            if (SOC_FAILURE(rv)) {
                cli_out("taps_flush_cache Flush failed err=%d\n", rv);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
            /* Should find out the bpm route */
            rv = taps_verify_bpm(unit, arg, &routes[route_idx]);
            if (SOC_FAILURE(rv)) {
                taps_dump(unit, arg->taps, dump_flags);
                cli_out("\n!!!! Failed to verify bpm for key 0x%x 0x%x length %d!!!\n",
                        routes[route_idx].keys[0], routes[route_idx].keys[1], routes[route_idx].key_length);
                /* coverity[check_after_deref] */
                if (routes) sal_free(routes);
                return SOC_E_FAIL;
            }
#endif
        }
        if ((route_idx >= 1024) && ((route_idx % (num_routes/10)) == 0)) {
            cli_out("!!! Deleted %d(K) routes\n", route_idx/1024);
        }
    }
    
    /****************************************************
        *Delete loopback route and vrf default route
        ****************************************************/
    for (vrf_idx = 0; vrf_idx < 4096; vrf_idx++) {
        /* Delete 127.0.0.0/8 */
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = (vrf_idx << 8) + 0x7F;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE + 8;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_FAILURE(rv)) {
            cli_out("\n!!!! Failed to delete 127.0.0.8 "
                    "for vrf %d !!!\n", 
                    vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
   
        arg->key = &_key[0];
        _key[0] = 0;
        _key[1] = vrf_idx;
        arg->length = TAPS_IPV4_MAX_VRF_SIZE;
        
        rv = taps_delete_route(unit, arg);
        if (SOC_SUCCESS(rv)) {
            tmu_taps_ut_test_result = 0;
        } else {
            cli_out("\n!!!! Failed to delete vrf def route for vrf %d!!!\n", vrf_idx);
            tmu_taps_ut_test_result = -1;
            /* coverity[check_after_deref] */
            if (routes) sal_free(routes);
            return SOC_E_FAIL;
        }
    }
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&cur_mem, &cur_mem_max);
    cli_out("\nAfter Delete Mem Begin(%d) Max(%d) \n", cur_mem, cur_mem_max);
    if (cur_mem != record_mem) {
        cli_out("\n!!!Memory leak %d!!!\n", cur_mem - record_mem);
        /* coverity[check_after_deref] */
        if (routes) sal_free(routes);
        return SOC_E_FAIL; 
    }
#endif    
    /* coverity[check_after_deref] */
    if (routes) sal_free(routes);
    return rv;
}



/* test database */
test_call_back_t taps_test_cback_array[] = {
    {TAPS_TEST_RUN, taps_table_init, taps_test_run, taps_test_cleanup},
    {TAPS_TCAM_TEST_RUN, taps_table_init, taps_tcam_test_run, taps_test_cleanup},
    {TAPS_DOMINO_TEST_RUN, taps_table_init, taps_domino_test_run, taps_test_cleanup},
    {TAPS_BASIC_TEST_RUN, taps_table_init, taps_basic_test_run, taps_test_cleanup},
    {TAPS_CAPACITY_TEST_RUN, taps_table_init, taps_capacity_test_run, taps_test_cleanup},
    {TAPS_BASIC_MULTITABLE_TEST_RUN, taps_multi_table_init, taps_basic_multi_table_test_run, taps_multi_table_test_cleanup},
    {TAPS_RANDOM_CAPACITY_TEST_RUN, taps_table_init, taps_random_capacity_test_run, taps_test_cleanup},
    {TAPS_PARALLEL_TEST_RUN, taps_table_parallel_init, taps_parallel_test_run, taps_par_test_cleanup},
    {TAPS_IPV6_TEST_RUN, taps_table_ipv6_init, taps_ipv6_test_run, taps_test_cleanup},
    {TAPS_IPV6_UPDATE_AND_GET_RUN, taps_table_ipv6_init, taps_ipv6_udpate_and_get_run, taps_test_cleanup},
    {TAPS_IPV6_RANDOM_ACCESS_RUN, taps_table_ipv6_init, taps_ipv6_random_access_run, taps_test_cleanup},
    {TAPS_MULTIPLE_KEY_TYPE_TEST_RUN, taps_table_mutiple_key_type_init, taps_multiple_key_type_test_run, taps_test_cleanup},
    {TAPS_BPM_TEST_RUN, taps_table_init, taps_bpm_test_run, taps_test_cleanup},
    {TAPS_IPV4_CUSTOMIZED_TEST_RUN, taps_table_init, taps_ipv4_customized_test_run, taps_test_cleanup},
    {TAPS_IPV6_CUSTOMIZED_TEST_RUN, taps_table_ipv6_init, taps_ipv6_customized_test_run, taps_test_cleanup},
    {TAPS_IPV4_DELETE_TEST_RUN, taps_table_init, taps_ipv4_delete_test_run, taps_test_cleanup},
    {TAPS_IPV6_DELETE_TEST_RUN, taps_table_ipv6_init, taps_ipv6_delete_test_run, taps_test_cleanup},
    {TAPS_IPV4_UPDATE_AND_GET_TEST_RUN, taps_table_init, taps_ipv4_udpate_and_get_test_run, taps_test_cleanup},
    {TAPS_IPV4_RANDOM_ACCESS_TEST_RUN, taps_table_init, taps_ipv4_random_access_test_run, taps_test_cleanup},
    {TAPS_IPV4_TRAVERSE_TEST_RUN, taps_table_init, taps_ipv4_traverse_route_test_run, taps_test_cleanup},
    {TAPS_IPV6_TRAVERSE_TEST_RUN, taps_table_ipv6_init, taps_ipv6_traverse_route_test_run, taps_test_cleanup},
	{TAPS_ERROR_HANDLING_TEST_RUN, taps_table_init, taps_error_handling_test_run, taps_test_cleanup},
    {TAPS_IPV4_BUCKET_REDIST_TEST_RUN, taps_table_init, taps_ipv4_bucket_redist_test_run, taps_test_cleanup},
    {TAPS_IPV6_BUCKET_REDIST_TEST_RUN, taps_table_ipv6_init, taps_ipv6_bucket_redist_test_run, taps_test_cleanup}
};

#define TMU_TAPS_IPV6_LPM_PROGRAM 5

/*********************/
/*     Test Runner   */
/*********************/
int c3_ut_tmu_taps_test_init(c3sw_test_info_t *testinfo, void *userdata)
{
  int rv=SOC_E_NONE;
  int index;
  int slave_idx, slave_unit;
  dq_p_t elem;
  taps_handle_t taps;
    
  if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(taps_test_cback_array)) return SOC_E_PARAM;
    
    
  /* free all the programs */
  for (index=0; index<SOC_SBX_CALADAN3_TMU_MAX_PROGRAM; index++) {
    rv = soc_sbx_caladan3_tmu_program_free(testinfo->unit, index);
    if (SOC_FAILURE(rv) && (rv != SOC_E_PARAM)) {
      /* failed */
      return rv;
    }
  }
  
  /* destroy all existing taps if there are any */
  if (!DQ_EMPTY(&taps_state[testinfo->unit]->taps_object_list)){
    DQ_TRAVERSE(&taps_state[testinfo->unit]->taps_object_list, elem) {
      taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
      rv = taps_destroy(testinfo->unit, taps);
    } DQ_TRAVERSE_END(&taps_state[testinfo->unit]->taps_object_list, elem);
  }

  if(_IS_MASTER_SHARE_LPM_TABLE(testinfo->unit, 
    SOC_IS_SBX_MASTER(testinfo->unit)?testinfo->unit:-1, testinfo->testTapsShare)) {
    for (slave_idx = 0; slave_idx < SOC_SBX_CONTROL(testinfo->unit)->num_slaves; slave_idx++) {
        slave_unit = SOC_SBX_CONTROL(testinfo->unit)->slave_units[slave_idx];
        for (index=0; index<SOC_SBX_CALADAN3_TMU_MAX_PROGRAM; index++) {
            rv = soc_sbx_caladan3_tmu_program_free(slave_unit, index);
            if (SOC_FAILURE(rv) && (rv != SOC_E_PARAM)) {
              /* failed */
              return rv;
            }
        }  
        /* destroy all existing taps if there are any */
        if (!DQ_EMPTY(&taps_state[slave_unit]->taps_object_list)){
            DQ_TRAVERSE(&taps_state[slave_unit]->taps_object_list, elem) {
            taps = DQ_ELEMENT_GET(taps_handle_t, elem, taps_list_node);
            rv = taps_destroy(slave_unit, taps);
        } DQ_TRAVERSE_END(&taps_state[slave_unit]->taps_object_list, elem);
        }
    }
  }    
      
  if (taps_test_cback_array[testinfo->testid].init)
    rv = taps_test_cback_array[testinfo->testid].init(testinfo->unit, (void*)testinfo);

  return rv;
}

int
c3_ut_tmu_taps_test_run(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(taps_test_cback_array)) return SOC_E_PARAM;


    /* Set up to count how many TMU commands we used */
    tmu_cmd_count_clear();

    if (taps_test_cback_array[testinfo->testid].run)
      rv = taps_test_cback_array[testinfo->testid].run(testinfo->unit, (void *)testinfo);

    /* Dump out how many commands we sent */
    tmu_cmd_count_print();
    return rv;
}

int
c3_ut_tmu_taps_test_done(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(taps_test_cback_array)) return SOC_E_PARAM;

    if (taps_test_cback_array[testinfo->testid].clean)
        rv = taps_test_cback_array[testinfo->testid].clean(testinfo->unit, userdata);

    return rv;
}


#define DBUCKET_TAPS_BUCKET_LOOKUP  (16)
#define DBUCKET_TAPS_PAYLOAD_LOOKUP (18)



/* crc40gsm = x40 + x26 + x23 + x17 + x3 + 1 */
static uint64 tmu_taps_psig_hash_calc( uint32 *puPrefixData, uint32 uHashAdjust ) {
  uint32  uAdjustedValue, uPrefixData;
  uint64  b;
  uint64  b40;
  uint64  crc40;
  uint64  crc40_bit39;
  uint64  uuTmp;
  uint32  auAdjustedPrefix[4];
  int     nWord, nShift;

  COMPILER_64_ZERO(crc40);

  for ( nWord = 0; nWord < 4; nWord++ ) {
    auAdjustedPrefix[nWord] = 0;
    uPrefixData = puPrefixData[nWord];
    for ( nShift = 0; nShift < 32; nShift += 8 ) {
      uAdjustedValue = (((uPrefixData >> nShift) & 0xff) + uHashAdjust) & 0xff;
      if ( nWord == 3 && nShift == 24 ) uAdjustedValue &= 0x7f;
      auAdjustedPrefix[nWord] |= uAdjustedValue << nShift;
    }
  }

  for ( nWord = 3; nWord >= 0; --nWord ) {
    for ( nShift = ((nWord == 3) ? 30 : 31); nShift >= 0; --nShift ) {
      COMPILER_64_SET(b,0,((auAdjustedPrefix[nWord] >> nShift) & 1));
      COMPILER_64_ZERO(b40);
      COMPILER_64_ZERO(crc40_bit39);
      if (COMPILER_64_BITTEST(crc40, 39))
        COMPILER_64_SET(crc40_bit39, 0, 1);
      uuTmp = b;
      COMPILER_64_XOR(uuTmp, crc40_bit39); /* <<  0 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 3);           /* <<  3 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 14);          /* << 17 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 6 );          /* << 23 */
      COMPILER_64_OR(b40, uuTmp);
      COMPILER_64_SHL(uuTmp, 3 );          /* << 26 */
      COMPILER_64_OR(b40, uuTmp);

      COMPILER_64_SHL(crc40,1);
      COMPILER_64_SET(uuTmp, 0x000000ff,0xffffffff);
      COMPILER_64_AND(crc40, uuTmp);
      COMPILER_64_XOR(crc40, b40);
    }
  }


  return crc40;
}

/*#define  TAPS_DEBUG_CANNED_ROUTES */

#define TAPS_CANNED_IPV6_RPB_NUMBER               1024  /*Use 1024 Max 1K */
#define TAPS_CANNED_IPV6_BB_NUMBER                  32 /* 32 max */
#define TAPS_DRAM_IPV6_PFX_PER_BUCKET               10 /* Use 10 - Max 80 (a pair each of 40) */


#define TMU_TAPS_RPB_BITS                           16
#define TMU_TAPS_BB_BITS                             5


static uint32 associated_ipv6_data[2 * TAPS_CANNED_IPV6_BB_NUMBER * TAPS_DRAM_IPV6_PFX_PER_BUCKET][8];

void tmu_dump_hardware(int unit,  taps_handle_t handle,int pivot_cnt,int bb_cnt,int dbucket_cnt)
{
  int i,j,k;

  for(i=0;i<pivot_cnt;i++)
    {
      taps_tcam_entry_dump(0,handle->tcam_hdl->tapsi,handle->segment,i,0);
      for(j=0;j<bb_cnt;j++){
    taps_sbucket_entry_dump(unit,handle->tcam_hdl->tapsi,handle->segment,i,j,12,0x7fff);
    for(k=0;k<dbucket_cnt;k++)
      taps_dbucket_entry_dump(unit,handle,0,j,k,0x7fff, FALSE);
      }
    }
    
}

/* Populates a TAPS with IPV6 Entries */  
int taps_pop_canned_ipv6(int unit, taps_init_params_t *param, taps_handle_t handle, int routes)
{
  int rpb_pivot_cnt;
  int bb_pivot_cnt;
  int dram_pfx_cnt;
  int entry_table_id;
  int entry_size;
  uint64 prefix_cache[2 * TAPS_DRAM_IPV6_PFX_PER_BUCKET];
  uint32 *pfx_cache[TAPS_CANNED_IPV6_BB_NUMBER];
  uint32 ipv6_key[4];
  unsigned int *bb_key[TAPS_CANNED_IPV6_BB_NUMBER];
  int i,npair;

  int route_cnt=0;


  int rv=SOC_E_NONE;
  soc_sbx_caladan3_tmu_cmd_t *tmu_write;
  taps_wgroup_handle_t wgroup;

  for (i = 0; i < TAPS_CANNED_IPV6_BB_NUMBER; i++) {
    pfx_cache[i] = sal_alloc(sizeof(uint32) * 32, "pfx_cache"); /* pfx_cache pointer array wide is 32*/
    bb_key[i] = sal_alloc(sizeof(uint32) * 5, "bb_key"); /* bb_key pointer array wide is 5*/
  }

  /* Loop over the root pivot blocks creating a command to load each */
  for ( rpb_pivot_cnt = 0; rpb_pivot_cnt < TAPS_CANNED_IPV6_RPB_NUMBER; rpb_pivot_cnt++) {

    /* Create a workgroup for all the commands associated with this pivot */
    rv = taps_work_group_create(unit, handle->wqueue[unit], 
                  _TAPS_DEFAULT_WGROUP_,
                  &wgroup);


    /* Create the command for the RPB*/
    rv = tmu_cmd_alloc(unit, &tmu_write);
    if (SOC_FAILURE(rv)) {
      cli_out("%s: Failed to create TMU Command\n",FUNCTION_NAME());
      goto end;
    }
    /* Load up the command */


    tmu_write->opcode = SOC_SBX_TMU_CMD_TAPS;
    tmu_write->cmd.taps.instance =  handle->hwinstance;
    tmu_write->cmd.taps.max_key_size  = TAPS_IPV6_KEY_SIZE;
    tmu_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    tmu_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    tmu_write->cmd.taps.subcmd.rpb_write.segment = handle->segment;
    tmu_write->cmd.taps.subcmd.rpb_write.offset = rpb_pivot_cnt;
    tmu_write->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    tmu_write->cmd.taps.subcmd.rpb_write.bpm_length = 0; 
    tmu_write->cmd.taps.subcmd.rpb_write.kshift = 0; 
    tmu_write->cmd.taps.subcmd.rpb_write.bucket = rpb_pivot_cnt;
    tmu_write->cmd.taps.subcmd.rpb_write.best_match = 0; 
    tmu_write->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    tmu_write->cmd.taps.subcmd.rpb_write.length = 16; 
    tmu_write->cmd.taps.subcmd.rpb_write.key[4] = rpb_pivot_cnt; 


    /* Enqueue the command for the root pivot block */
    rv = taps_work_enqueue(unit, wgroup, TAPS_TCAM_WORK, &(tmu_write->wq_list_elem));
    if (SOC_FAILURE(rv)) {
      cli_out("%s: Failed to enqueue command for RPB\n",FUNCTION_NAME());
      goto end;
    }

    

#ifdef  TAPS_DEBUG_CANNED_ROUTES
          cli_out("%s: Sending TMU Command for RPB\n",FUNCTION_NAME());
      tmu_cmd_printf(unit, tmu_write);    
#endif 

    /* Now load up the sram buckets associated with this pivot */
    for(bb_pivot_cnt=0; bb_pivot_cnt < TAPS_CANNED_IPV6_BB_NUMBER; bb_pivot_cnt++)
      {


    /* Create the command for this SRAM bucket */
    rv = tmu_cmd_alloc(unit, &tmu_write);
    if (SOC_FAILURE(rv)) {
      cli_out("%s: Failed to create TMU Command for BB\n",FUNCTION_NAME());
      goto end;
    }


    /* Set the key for this BB */
    bb_key[bb_pivot_cnt][0]=bb_key[bb_pivot_cnt][1]=bb_key[bb_pivot_cnt][2]=bb_key[bb_pivot_cnt][3]=0;
    bb_key[bb_pivot_cnt][4] =  (bb_pivot_cnt + 1) | (rpb_pivot_cnt <<  TMU_TAPS_BB_BITS );


    /* Load up the command */   
    tmu_write->opcode = SOC_SBX_TMU_CMD_TAPS;
        tmu_write->cmd.taps.instance = handle->hwinstance;
    tmu_write->cmd.taps.max_key_size  = TAPS_IPV6_KEY_SIZE;
        tmu_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BB;
        tmu_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE;
        tmu_write->cmd.taps.subcmd.bb_write.segment = handle->segment;
        tmu_write->cmd.taps.subcmd.bb_write.offset = rpb_pivot_cnt;
        tmu_write->cmd.taps.subcmd.bb_write.prefix_id = bb_pivot_cnt;
        tmu_write->cmd.taps.subcmd.bb_write.kshift = 0;
        tmu_write->cmd.taps.subcmd.bb_write.align_right = FALSE;
    sal_memcpy(tmu_write->cmd.taps.subcmd.bb_write.key,
            bb_key[bb_pivot_cnt],
            sizeof(uint32) * TAPS_IPV6_KEY_SIZE_WORDS);
    tmu_write->cmd.taps.subcmd.bb_write.length = TMU_TAPS_BB_BITS + TMU_TAPS_RPB_BITS ;
        tmu_write->cmd.taps.subcmd.bb_write.format = handle->param.sbucket_attr.format;


    /* Add it to the work group */
    rv = taps_work_enqueue(unit, wgroup, TAPS_TCAM_WORK, &(tmu_write->wq_list_elem));    
    if (SOC_FAILURE(rv)) {
      cli_out("%s: Failed to enqueue command for BB\n",FUNCTION_NAME());
      goto end;
    }

#ifdef  TAPS_DEBUG_CANNED_ROUTES
    cli_out("%s: Sending TMU Command for BB\n",FUNCTION_NAME());
    tmu_cmd_printf(unit, tmu_write);      

#endif 


    /* Do  associated dram prefixes */
    for(dram_pfx_cnt=0; dram_pfx_cnt < 2* TAPS_DRAM_IPV6_PFX_PER_BUCKET; dram_pfx_cnt++)
      {

        if(dram_pfx_cnt == 0)
          { 


        /* Populate the command */
        /* Load the data */
        for(i=0;i< 2 * TAPS_DRAM_IPV6_PFX_PER_BUCKET; i++){

          /* make the key so we can do the hash over it. */
          tmu_taps_make_ipv6_key(rpb_pivot_cnt, bb_pivot_cnt, i, ipv6_key);
          /* Hash the key*/
          prefix_cache[i] = tmu_taps_psig_hash_calc(ipv6_key,0); 

        }

        /*  Pack the psigs */
        tmu_taps_pack_ipv6(pfx_cache[bb_pivot_cnt],prefix_cache,127); 

        entry_table_id = handle->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
        entry_size = 512; /* Hardcoded for a bucket size of 10 */

        for(npair=0; npair < 2; npair++)
          {
            /* Create the command for this DRAM bucket */
            rv = tmu_cmd_alloc(unit, &tmu_write);
            if (SOC_FAILURE(rv)) {
                cli_out("%s: Failed to create TMU Command for DRAM Bucket Prefix\n",FUNCTION_NAME());
                goto end;
            }


            tmu_write->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
            tmu_write->cmd.xlwrite.table = entry_table_id;
            tmu_write->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
            tmu_write->cmd.xlwrite.entry_num = 2 *  bb_pivot_cnt + npair;
            tmu_write->cmd.xlwrite.offset = 0;
            tmu_write->cmd.xlwrite.size = (entry_size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
            tmu_write->cmd.xlwrite.value_size = entry_size;
	    for (i=0; i< BITS2WORDS(tmu_write->cmd.xlwrite.value_size); i++) {
		tmu_write->cmd.xlwrite.value_data[i] = pfx_cache[bb_pivot_cnt][npair*16+i];
	    }


#ifdef  TAPS_DEBUG_CANNED_ROUTES
            cli_out("%s: Sending TMU Command for DRAM\n",FUNCTION_NAME());
            tmu_cmd_printf(unit, tmu_write);      
#endif 

            /* Add it to the work group */
            rv = taps_work_enqueue(unit, wgroup, TAPS_TCAM_WORK, &(tmu_write->wq_list_elem));    
            if (SOC_FAILURE(rv)) {
                cli_out("%s: Failed to enqueue command for DRAM\n",FUNCTION_NAME());
                goto end;
            }
          }
          }
        /* Create the command for the associated data */
        rv = tmu_cmd_alloc(unit, &tmu_write);
        if (SOC_FAILURE(rv)) {
            cli_out("%s: Failed to create TMU Command for DRAM Associated Data\n",FUNCTION_NAME());
            goto end;
        }
          
        entry_table_id = handle->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
        entry_size = _tmu_dbase[unit]->table_cfg.table_attr[entry_table_id].entry_size_bits;
        
        /* calculate the associated data for this route */
        tmu_taps_ipv6_ad(rpb_pivot_cnt,bb_pivot_cnt,dram_pfx_cnt,
                 &associated_ipv6_data[2* bb_pivot_cnt * TAPS_DRAM_IPV6_PFX_PER_BUCKET + dram_pfx_cnt][0]);
        

        tmu_write->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
        tmu_write->cmd.xlwrite.table = entry_table_id;
        tmu_write->cmd.xlwrite.lookup =  SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
        tmu_write->cmd.xlwrite.entry_num = 2* bb_pivot_cnt * TAPS_DRAM_IPV6_PFX_PER_BUCKET + dram_pfx_cnt;
        tmu_write->cmd.xlwrite.offset = 0;
        tmu_write->cmd.xlwrite.size = (entry_size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
        tmu_write->cmd.xlwrite.value_size = entry_size;
	for (i=0; i< BITS2WORDS(tmu_write->cmd.xlwrite.value_size); i++) {
	    tmu_write->cmd.xlwrite.value_data[i] = associated_ipv6_data[2* bb_pivot_cnt * TAPS_DRAM_IPV6_PFX_PER_BUCKET + dram_pfx_cnt][i];
	}

        /* Add it to the work group */
        rv = taps_work_enqueue(unit, wgroup, TAPS_TCAM_WORK, &(tmu_write->wq_list_elem));    
        if (SOC_FAILURE(rv)) {
            cli_out("%s: Failed to enqueue command for Associated Data\n",FUNCTION_NAME());
            goto end;
        }

#ifdef  TAPS_DEBUG_CANNED_ROUTES
        tmu_taps_make_ipv6_key(rpb_pivot_cnt, bb_pivot_cnt, dram_pfx_cnt, ipv6_key);
        cli_out("%s: Route: 0x%04x 00x%04x 0x%04x x%04x \n DRAM Associated Data\n"
                ,FUNCTION_NAME(),ipv6_key[3],ipv6_key[2],ipv6_key[1],ipv6_key[0]);
        tmu_cmd_printf(unit, tmu_write);
#endif 

        

        route_cnt++;   
        if(route_cnt % 1024 == 0)
            LOG_DEBUG(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "## Added %d(K) IPV6 routes to FIB\n"),
                       route_cnt / 1024));

        if(route_cnt == routes)
          {
        /* Commit the word for this slice of the route tree */
        rv = taps_tcam_commit(unit, wgroup);
    
        /* Destroy the workgroup */
        taps_work_group_destroy(unit, wgroup);

#ifdef  TAPS_DEBUG_CANNED_ROUTES
        tmu_dump_hardware(unit,  handle, rpb_pivot_cnt+1,bb_pivot_cnt+1,dram_pfx_cnt+1);
#endif
        goto end;
          }
      }
      }
    /* Commit the word for this slice of the route tree */
     rv = taps_tcam_commit(unit, wgroup);
    
    /* Destroy the workgroup */
    taps_work_group_destroy(unit, wgroup);




  }


#ifdef  TAPS_DEBUG_CANNED_ROUTES
  tmu_dump_hardware(unit,  handle, TAPS_CANNED_IPV6_RPB_NUMBER,TAPS_CANNED_IPV6_BB_NUMBER,2*TAPS_DRAM_IPV6_PFX_PER_BUCKET);
        
#endif

    goto end;

end:
    for (i = 0; i < TAPS_CANNED_IPV6_BB_NUMBER; i++) {
        sal_free(pfx_cache[i]);
        sal_free(bb_key[i]);
    }
    return rv;
}

/* helper routine for loading canned IPv6 route table
   Assumes two blocks of 10 prefixes */

void tmu_taps_pack_ipv6(uint32 * pfx_block,uint64 *hashed_prefix,int len )
{
  int i,j;
  uint64 * p_handy = (uint64 *) pfx_block;
  uint64 p_sig[20];

  /* Page 195 in the TMU document */
  
  /* construct the psigs from the hashed prefixes */
  for(i = 0; i < 20; i++)
    {
      uint64 uuTmp;
      uuTmp = hashed_prefix[i];
      COMPILER_64_SHL(uuTmp, 8);
      COMPILER_64_SET(p_sig[i], 0, (len << 1) | 0x1);
      COMPILER_64_OR(p_sig[i], uuTmp);
    }
  

  /* Now load up the pfx_block(s) We do a pair each time */
  for(i=0,j=0;i<=8;i += 8,j+=10)
    { uint64 uuMask1 = COMPILER_64_INIT(0x0000FFFF, 0xFFFFFFFF);
      uint64 uuMask2 = COMPILER_64_INIT(0xFFFF0000, 0x00000000);
      uint64 uuMask3 = COMPILER_64_INIT(0x00000000, 0xFFFFFFFF);
      uint64 uuMask4 = COMPILER_64_INIT(0xFFFFFFFF, 0x00000000);
      uint64 uuMask5 = COMPILER_64_INIT(0x00000000, 0x0000FFFF);
      uint64 uuMask6 = COMPILER_64_INIT(0xFFFFFFFF, 0xFFFF0000);
      uint64 uuTmp1, uuTmp2;
      
      /* p_handy[0 + i] */
      uuTmp1 = p_sig[0 + j];
      uuTmp2 = p_sig[1 + j];
      COMPILER_64_AND(uuTmp1, uuMask1);
      COMPILER_64_SHL(uuTmp2, 48);
      COMPILER_64_AND(uuTmp2, uuMask2);
      p_handy[0 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[0 + i], uuTmp2);

      /* p_handy[1 + i] */
      uuTmp1 = p_sig[1 + j];
      uuTmp2 = p_sig[2 + j];
      COMPILER_64_SHR(uuTmp1, 16);
      COMPILER_64_AND(uuTmp1, uuMask3);
      COMPILER_64_SHL(uuTmp2, 32);
      COMPILER_64_AND(uuTmp2, uuMask4);
      p_handy[1 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[1 + i], uuTmp2);

      /* p_handy[2 + i] */
      uuTmp1 = p_sig[2 + j];
      uuTmp2 = p_sig[3 + j];
      COMPILER_64_SHR(uuTmp1, 32);
      COMPILER_64_AND(uuTmp1, uuMask5);
      COMPILER_64_SHL(uuTmp2, 16);
      COMPILER_64_AND(uuTmp2, uuMask6);
      p_handy[2 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[2 + i], uuTmp2);

      /* p_handy[3 + i] */
      p_handy[3 + i] = p_sig[4 + j];
      COMPILER_64_AND(p_handy[3 + i], uuMask1);
      
      /* p_handy[4 + i] */
      uuTmp1 = p_sig[5 + j];
      uuTmp2 = p_sig[6 + j];
      COMPILER_64_AND(uuTmp1, uuMask1);
      COMPILER_64_SHL(uuTmp2, 48);
      COMPILER_64_AND(uuTmp2, uuMask2);
      p_handy[0 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[0 + i], uuTmp2);

      /* p_handy[5 + i] */
      uuTmp1 = p_sig[6 + j];
      uuTmp2 = p_sig[7 + j];
      COMPILER_64_SHR(uuTmp1, 16);
      COMPILER_64_AND(uuTmp1, uuMask3);
      COMPILER_64_SHL(uuTmp2, 32);
      COMPILER_64_AND(uuTmp2, uuMask4);
      p_handy[5 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[5 + i], uuTmp2);

      /* p_handy[6 + i] */
      uuTmp1 = p_sig[7 + j];
      uuTmp2 = p_sig[8 + j];
      COMPILER_64_SHR(uuTmp1, 32);
      COMPILER_64_AND(uuTmp1, uuMask5);
      COMPILER_64_SHL(uuTmp2, 16);
      COMPILER_64_AND(uuTmp2, uuMask6);
      p_handy[6 + i] = uuTmp1;
      COMPILER_64_OR(p_handy[6 + i], uuTmp2);

      /* p_handy[7 + i] */
      p_handy[7 + i] = p_sig[9 + j];
      COMPILER_64_AND(p_handy[7 + i], uuMask1);
      
      
    }


  for(i=0;i<16;i++) {
    uint32 
      uTmp1 = COMPILER_64_LO(p_handy[i]), 
      uTmp2 = COMPILER_64_HI(p_handy[i]);
    
    COMPILER_64_SET(p_handy[i], uTmp1, uTmp2);
  }

}




/* support function for canned ipv6 route table creation.
   Given a root pivot, a bucket block and a dram bucket prefix id constructs a 128 bit key */

void tmu_taps_make_ipv6_key(int rpb_pivot, int bb_pivot, int dram_index, uint32 * ipv6_key)
{


  /* First flavour */
  /* Of form 
     0000:0001:0000:0001:0000:0000:0000:0001
     0000:0001:0000:0001:0000:0000:0000:0002
     0000:0001:0000:0001:0000:0000:0000:0003 
     ...
     0000:0001:0000:0001:0000:0000:0000:0013

     0000:0001:0000:0002:0000:0000:0000:0001
     0000:0001:0000:0002:0000:0000:0000:0002
     0000:0001:0000:0002:0000:0000:0000:0003 
     ...
     0000:0001:0000:0002:0000:0000:0000:0013

     .....

     0000:0002:0000:0002:0000:0000:0000:0001
     0000:0002:0000:0002:0000:0000:0000:0002
     0000:0002:0000:0002:0000:0000:0000:0003 
     ...
     0000:0002:0000:0002:0000:0000:0000:0013

     ...
   */

    if (tmu_taps_ut_debug) {
    ipv6_key[0] = ipv6_key[1] = ipv6_key[2] = ipv6_key[3] = 0;
    ipv6_key[0] = dram_index;
    } else {
    ipv6_key[3] = ((bb_pivot+1) << (32 -  TMU_TAPS_BB_BITS))  |   ((dram_index > 9 ? 1:0) << (32 -  TMU_TAPS_BB_BITS -1));               /* bits 127-95 */
    ipv6_key[2] = 0;                             /* bits 95 -64 */
    ipv6_key[1] = 0;                             /* bits 63 -32  (bit 63 is the bit associated with bucket pairs*/
    ipv6_key[0] = dram_index%10 + 1;                /* bits 31  -0 */
    }
}


/* Creates the associated data for an ipv6 entry */
void tmu_taps_ipv6_ad(int rpb_pivot, int bb_pivot,int dram_pfx, uint32 *p_adata)
{

  uint32 ipv6[4];

  /* Format is first the ipv6 key then the 119 bit entry */


  /* calculate our canned IPv6 address */
  tmu_taps_make_ipv6_key(rpb_pivot, bb_pivot, dram_pfx, ipv6);


  /* Need the CPE encoded version. */
  
  
  p_adata[0] = ipv6[0] << 1 | 1; 
  p_adata[1] = (ipv6[1] << 1) | (p_adata[0] >> 31);    
  p_adata[2] = (ipv6[2] << 1) | (p_adata[1] >> 31); 
  p_adata[3] = (ipv6[3] << 1) |  (p_adata[2] >> 31); 

  /* Now for the routing. We will loop over 4 fibs based on the dram prefix index */
  p_adata[4] = (0x3FFFA) + dram_pfx % 4;
  p_adata[5] = 0;
  p_adata[6] = 0;
  p_adata[7] = 0;

}





#endif /* #ifdef BCM_CALADAN3_SUPPORT */
