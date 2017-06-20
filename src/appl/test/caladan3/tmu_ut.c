/* $Id: tmu_ut.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/dm.h>
#include <soc/sbx/caladan3/tmu/hash.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/sbx/caladan3/lrp.h>
#include <appl/test/caladan3/c3hppc_lrp.h>
#include <soc/sbx/sbx_drv.h>


typedef enum tmu_ut_test_id_e {
    DM_ALLOC_TEST_ID =0,
    DM_TEST_ID =1,
    DM_UCODE_TEST_ID =2,
    EML_TEST_ID =3,
    MAX_TEST_ID
} tmu_ut_test_id_e_t; 

typedef struct _dm_table_param_s {
    int table_size;
    int entry_size_bits;
    int tableid;
} dm_table_param_t; 

dm_table_param_t dm_tables[] = {{262144, SOC_SBX_TMU_DM_494b_SIZE, 0}};

static int tmu_ut_test_result;

typedef struct _hash_table_param_s {
    soc_sbx_tmu_hash_param_t param;
    soc_sbx_tmu_hash_handle_t handle;
} hash_table_param_t; 

hash_table_param_t hash_test_param[2];

int c3_ut_tmu_hash_alloc_test(int unit)
{
    int rv=SOC_E_NONE;
    soc_sbx_tmu_hash_param_t *param;

    sal_memset(&hash_test_param[0],0,sizeof(hash_table_param_t));

    param = &hash_test_param[0].param;
    param->capacity = 1000000;
    param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    param->chain_length = 6;
    param->num_hash_table_entries = 4194304;
    param->num_chain_table_entries = 262144;

    rv =  soc_sbx_caladan3_tmu_hash_table_alloc(unit, param, &hash_test_param[0].handle);
    
    return rv;
}

int c3_ut_tmu_hash_run_unit_test(int unit)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x11223344, 0x55667788};
    uint32 chain_key[] = {0xaabbccdd, 0xeeff0101};
    uint32 value[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef};
    uint32 read_value[] = {0, 0, 0, 0};

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);

    if (SOC_SUCCESS(rv)) {
        /* verify added value */
        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
        if (SOC_SUCCESS(rv)) {
            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                if (value[index] != read_value[index]) {
                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                            read_value[index], value[index], index);
                    return SOC_E_FAIL;
                }
            }

            /* try to reinsert same key expect errors */
            rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
            if (SOC_SUCCESS(rv)) {
                cli_out("ERROR: Allowed duplicate key insertion !!!!\n");
                rv = SOC_E_FAIL;
            } else {
                rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle,
                                                         &chain_key[0], &value[0]);     
                if (SOC_SUCCESS(rv)) {
                    rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle,
                                                             &chain_key[0], &read_value[0]);
                    if (SOC_SUCCESS(rv)) {
                        for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                            if (value[index] != read_value[index]) {
                                cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                        read_value[index], value[index], index);
                                return SOC_E_FAIL;
                            }
                        }
                        
                        rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle,
                                                                 &chain_key[0], &value[0]);
                        if (SOC_SUCCESS(rv)) {
                            cli_out("ERROR: Allowed duplicate key insertion !!!!\n");
                            rv = SOC_E_FAIL;
                        } else {
                            rv = SOC_E_NONE;
                        }
                    }
                }           
            }

            /* delete the bucket node & see if chain becomes the new bucket node */
            rv = soc_sbx_caladan3_tmu_hash_entry_delete(unit, hash_test_param[0].handle, &key[0]);
            if (SOC_SUCCESS(rv)) {
                rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
                if (SOC_SUCCESS(rv)) {
                    cli_out("ERROR: bucket key not yet deleted !!!!\n");
                    rv = SOC_E_FAIL;
                } else {
                    sal_memset(&read_value[0], 0, sizeof(read_value));
                    rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle,
                                                             &chain_key[0], &read_value[0]);
                    if (SOC_SUCCESS(rv)) {
                        for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                            if (value[index] != read_value[index]) {
                                cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                        read_value[index], value[index], index);
                                return SOC_E_FAIL;
                            }
                        }
                    }
                }
            }
        }
    }

    return rv;
}

int c3_ut_tmu_hash_cleanup_test(int unit)
{
    return soc_sbx_caladan3_tmu_hash_table_free(unit, hash_test_param[0].handle);
}

int c3_ut_tmu_dm_alloc_unit_test(int unit)
{
    int rv, index;
    dm_table_param_t noise_dm_tables[] = {{256*1024, 256, 0}, /* tbl 1 */
                                          {8*1024, 128, 0}, /* tbl 2 */
                                          {64*1024, 128, 0}, /* tbl 3 */
                                          {512*1024, 256, 0}, /* tbl 4 */
                                          {4*1024, 128, 0}, /* tbl 5 */
                                          {32*1024, 128, 0} /* tbl 6 */};

    /* (1) allocate single table tbl1 */
    rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[0].table_size, 
                                             noise_dm_tables[0].entry_size_bits,
                                             &noise_dm_tables[0].tableid);    
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to allocate DM table1: %d\n",unit,rv);
        rv = SOC_E_FAIL;
    }

    /* (2) allocate second table tbl2*/
    if (SOC_SUCCESS(rv)) {
    rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[1].table_size, 
                                             noise_dm_tables[1].entry_size_bits,
                                             &noise_dm_tables[1].tableid);    
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to allocate DM table2: %d\n",unit,rv);
        rv = SOC_E_FAIL;
    }
    }

    /* (3) free tbl1 & allocate tbl1 again to see it takes best fit */
    if (SOC_SUCCESS(rv)) {
    soc_sbx_caladan3_tmu_dm_table_free(unit, noise_dm_tables[0].tableid);
    rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[0].table_size, 
                                             noise_dm_tables[0].entry_size_bits,
                                             &noise_dm_tables[0].tableid);    
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to allocate DM table0: %d\n",unit,rv);
        rv = SOC_E_FAIL;
    }
    }

    /* (4) allocate tbl3, tbl4, tbl5, tbl6, free tbl3, tbl5, allocate tbl3 again */
    if (SOC_SUCCESS(rv)) {
        for (index=2; index < COUNTOF(noise_dm_tables); index++) {
            rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[index].table_size, 
                                                     noise_dm_tables[index].entry_size_bits,
                                                     &noise_dm_tables[index].tableid);    
            if (SOC_FAILURE(rv)) {
                cli_out("%d Failed to allocate DM table%d: %d\n",unit,index,rv);
                rv = SOC_E_FAIL;
            }
        }

        soc_sbx_caladan3_tmu_dm_table_free(unit, noise_dm_tables[2].tableid);
        soc_sbx_caladan3_tmu_dm_table_free(unit, noise_dm_tables[4].tableid);

        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[2].table_size, 
                                                     noise_dm_tables[2].entry_size_bits,
                                                     &noise_dm_tables[2].tableid);
            if (SOC_FAILURE(rv)) {
                cli_out("%d Failed to allocate DM table2: %d\n",unit,rv);
                rv = SOC_E_FAIL;
            }
        }

        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, noise_dm_tables[4].table_size, 
                                                     noise_dm_tables[4].entry_size_bits,
                                                     &noise_dm_tables[4].tableid);    
            if (SOC_FAILURE(rv)) {
                cli_out("%d Failed to allocate DM table4: %d\n",unit,rv);
                rv = SOC_E_FAIL;
            }
        }
    }

    soc_sbx_caladan3_tmu_table_map_dump(unit, -1, -1);

    /* (5) free all */
    for (index=0; index < COUNTOF(noise_dm_tables); index++) {
        soc_sbx_caladan3_tmu_dm_table_free(unit, noise_dm_tables[index].tableid);
    }
    
    return rv;
}


static
int c3_tmu_dm_verify_access(int unit, int testindex) 
{
    int rv=SOC_E_NONE, index;
    uint32 *write_buf, *read_buf, mask;
    uint32 pattern[] = {0x00112233, 0x44556677,
                        0x8899aabb, 0xccddeeff};
    int num_words;

    num_words = BITS2WORDS(dm_tables[testindex].entry_size_bits);
    write_buf = (uint32*) sal_alloc(num_words * sizeof(uint32), "tmu-write-buf");
    read_buf = (uint32*) sal_alloc(num_words * sizeof(uint32), "tmu-read-buf");

    /* Set & read back memory */
    sal_memset(&write_buf[0], 0, num_words);
    sal_memset(&read_buf[0], 0, num_words);

    cli_out("Entry Value Writtern: ");
    for (index=0; index < num_words; index++) {
        write_buf[index] = pattern[index % COUNTOF(pattern)];
        cli_out("%x ", write_buf[index]);
    }
    cli_out("\n");

    rv = soc_sbx_caladan3_tmu_dm_set(unit, dm_tables[testindex].tableid, 0, 
                                     &write_buf[0], num_words);
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to set DM table ID [%d] Entry[%d]: %d\n",
                unit, dm_tables[testindex].tableid, 0, rv);
    } else {
        /* read & verify */
        rv = soc_sbx_caladan3_tmu_dm_get(unit, dm_tables[testindex].tableid, 0, 
                                         &read_buf[0], num_words);
        if (SOC_FAILURE(rv)) {
            cli_out("%d Failed to get DM table ID [%d] Entry[%d]: %d\n",
                    unit, dm_tables[testindex].tableid, 0, rv);
        } else {
            cli_out("Entry Value READ: ");
            for (index=0; index < num_words; index++) {
                cli_out("%x ", read_buf[index]);
            }
            cli_out("\n");

            /* mask out ECC bits on read/write buffer before comparision */
            mask = (1 << SOC_SBX_CALADAN3_TMU_DM_ECC_BITS) - 1;
            mask <<= (31 - SOC_SBX_CALADAN3_TMU_DM_ECC_BITS);
            write_buf[num_words-1] &= ~mask;
            read_buf[num_words-1] &= ~mask;
            
            if(sal_memcmp(&write_buf[0], &read_buf[0], num_words) != 0) {
                cli_out("%d Read & write entries for DM table ID [%d] Entry[%d]: Mismatch!!!\n",
                        unit, dm_tables[testindex].tableid, 0);
                rv = SOC_E_FAIL;
            }
        }
    }
    
    sal_free(read_buf);
    sal_free(write_buf);
    return rv;
}

int
c3_ut_tmu_dm_test_init(int unit)
{
    int rv=SOC_E_NONE;
    
    rv = soc_sbx_caladan3_tmu_dm_table_alloc(unit, dm_tables[0].table_size, 
                                             dm_tables[0].entry_size_bits,
                                             &dm_tables[0].tableid);    
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to allocate DM table: %d\n",unit,rv);
        return SOC_E_FAIL;
    } 

    return rv;
}

int
c3_ut_tmu_dm_test_run(int unit)
{
    int rv=SOC_E_NONE;

    rv = c3_ut_tmu_dm_test_init(unit);
    if (SOC_SUCCESS(rv)) {
        rv = c3_tmu_dm_verify_access(unit, 0);
        if (SOC_FAILURE(rv)) {
            cli_out(" Test Failed %d \n", rv);
        }
    }
    
    return rv;
}

int
c3_ut_tmu_dm_test_cleanup(int unit)
{
    return soc_sbx_caladan3_tmu_dm_table_free(unit, dm_tables[0].tableid);
}

/*****
* ucode verification test 
******/
#define _DM_LRP_PORT_NUM_ (4)

int tmu_dm_ut_setup_ucode_lookup(int unit)
{
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    lrb_dm_segment_table_entry_t entry;
    uint32 rval=0, field=0;
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_dm_ut.txt");
   
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, LRB_DM_SEGMENT_TABLEm,
                                      MEM_BLOCK_ANY,
                                     dm_tables[0].tableid, &entry));

    /* DM for now uses Port 4 which corresponds to DM0 */
    field = 3; /* 3 = 512 bit result from c15-c0 */
    soc_mem_field_set(unit, LRB_DM_SEGMENT_TABLEm, &entry.entry_data[0], DM0f, &field); 

    SOC_IF_ERROR_RETURN(soc_mem_write(unit, LRB_DM_SEGMENT_TABLEm,
                                     MEM_BLOCK_ANY,
                                     dm_tables[0].tableid, &entry));

    return SOC_E_NONE;
}

int tmu_dm_ut_verify_ucode_lookup(int unit, int entryidx)
{
    int index;
    unsigned int dm_pyld[BITS2WORDS(SOC_SBX_TMU_DM_494b_SIZE)];

#define _DBG_VERBOSE

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    if (entryidx < 0 || entryidx >= dm_tables[0].table_size) return SOC_E_PARAM;

#ifdef _DBG_VERBOSE
    /* write shared register */
    cli_out("\n$ DM entry Index: %d \n ", entryidx);
#endif

    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0, entryidx)); 

    /* start microcode execution */
    { uint64 uuVal = COMPILER_64_INIT(0,100);
      SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, uuVal));
    }

    cli_out("Check DM Index = %d \n",
            c3hppc_lrp_read_shared_register(unit,16*0+3));

    
    for (index=0; index < 16; index++) {
        dm_pyld[index] =  c3hppc_lrp_read_shared_register(unit, 
                                                          (16*(index%4)) + (index/4) + 1);
#ifdef _DBG_VERBOSE
        cli_out("C%d reg = 0x%x \n", 15-index, dm_pyld[index]);
#endif
    }


    for (index=0; index < 4; index++) {
        dm_pyld[index] =  c3hppc_lrp_read_shared_register(unit,16*index+1);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", dm_pyld[index]);
#endif
    }

    for (index=0; index < 4; index++) {
        dm_pyld[index+4] =  c3hppc_lrp_read_shared_register(unit,16*index+2);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", dm_pyld[index+4]);
#endif
    }

    for (index=0; index < 4; index++) {
        dm_pyld[index+4] =  c3hppc_lrp_read_shared_register(unit,16*index+3);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", dm_pyld[index+8]);
#endif
    }

    for (index=0; index < 4; index++) {
        dm_pyld[index+4] =  c3hppc_lrp_read_shared_register(unit,16*index+4);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", dm_pyld[index+12]);
#endif
    }


#ifdef _DBG_VERBOSE
    /* verify.. */
    cli_out("Sanity Values = 0x%x \n",  
            c3hppc_lrp_read_shared_register(unit,16*3+5));
#endif
   
    return SOC_E_NONE;
}

int c3_ut_tmu_dm_ucode_unit_test_run(int unit)
{
    int rv=SOC_E_NONE;
    uint32 *write_buf, *read_buf, mask;
    uint32 pattern[] = {0,
                        0x11111111,
                        0x22222222,
                        0x33333333,
                        0x44444444,
                        0x55555555,
                        0x66666666,
                        0x77777777,
                        0x88888888,
                        0x99999999,
                        0xaaaaaaaa,
                        0xbbbbbbbb,
                        0xcccccccc,
                        0xdddddddd,
                        0xeeeeeeee,
                        0xffffffff};
    int num_words, testindex=0, index, entryidx=11;

    num_words = BITS2WORDS(dm_tables[testindex].entry_size_bits);
    write_buf = (uint32*) sal_alloc(num_words * sizeof(uint32), "tmu-write-buf");
    read_buf = (uint32*) sal_alloc(num_words * sizeof(uint32), "tmu-read-buf");

    /* Set & read back memory */
    sal_memset(&write_buf[0], 0, num_words);
    sal_memset(&read_buf[0], 0, num_words);

    cli_out("Entry Value Writtern: ");
    for (index=0; index < num_words; index++) {
        write_buf[index] = pattern[index % COUNTOF(pattern)];
        cli_out("%x ", write_buf[index]);
    }
    cli_out("\n");

    rv = soc_sbx_caladan3_tmu_dm_set(unit, dm_tables[testindex].tableid, entryidx, 
                                     &write_buf[0], num_words);
    if (SOC_FAILURE(rv)) {
        cli_out("%d Failed to set DM table ID [%d] Entry[%d]: %d\n",
                unit, dm_tables[testindex].tableid, entryidx, rv);
    } else {
        /* read & verify */
        rv = soc_sbx_caladan3_tmu_dm_get(unit, dm_tables[testindex].tableid, entryidx, 
                                         &read_buf[0], num_words);
        if (SOC_FAILURE(rv)) {
            cli_out("%d Failed to get DM table ID [%d] Entry[%d]: %d\n",
                    unit, dm_tables[testindex].tableid, entryidx, rv);
        } else {
            cli_out("Entry Value READ: ");
            for (index=0; index < num_words; index++) {
                cli_out("%x ", read_buf[index]);
            }
            cli_out("\n");

            /* mask out ECC bits on read/write buffer before comparision */
            mask = (1 << SOC_SBX_CALADAN3_TMU_DM_ECC_BITS) - 1;
            mask <<= (31 - SOC_SBX_CALADAN3_TMU_DM_ECC_BITS);
            write_buf[num_words-1] &= ~mask;
            read_buf[num_words-1] &= ~mask;
            
            if(sal_memcmp(&write_buf[0], &read_buf[0], num_words) != 0) {
                cli_out("%d Read & write entries for DM table ID [%d] Entry[%d]: Mismatch!!!\n",
                        unit, dm_tables[testindex].tableid, entryidx);
                rv = SOC_E_FAIL;
            }
        }
    }

    if (SOC_SUCCESS(rv)) {
        tmu_dm_ut_setup_ucode_lookup(unit);
        tmu_dm_ut_verify_ucode_lookup(unit, entryidx);
    }

    sal_free(read_buf);
    sal_free(write_buf);
    return rv;
}



/******************/
/*** Test runner **/
/******************/

int c3_ut_tmu_test_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit;
    
    rv = soc_sbx_caladan3_tmu_driver_init(pc3swTestInfo->unit);
    if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
        cli_out("C3 %d TMU driver init failed\n",pc3swTestInfo->unit);
        return rv;
    } else {
        rv = SOC_E_NONE;
    }
    
    unit = pc3swTestInfo->unit;

    switch (pc3swTestInfo->testid) {
    case DM_TEST_ID: 
    case DM_UCODE_TEST_ID:
        rv = c3_ut_tmu_dm_test_init(unit);
        break;

    case EML_TEST_ID:
        rv = c3_ut_tmu_hash_alloc_test(unit);
        break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}

int
c3_ut_tmu_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit;
    tmu_ut_test_result = -1;

    unit = pc3swTestInfo->unit;

    switch (pc3swTestInfo->testid) {
    case DM_ALLOC_TEST_ID: 
        rv = c3_ut_tmu_dm_alloc_unit_test(unit);
        break;

    case DM_TEST_ID: 
        rv = c3_ut_tmu_dm_test_run(unit);
        break;

    case DM_UCODE_TEST_ID:
        rv = c3_ut_tmu_dm_ucode_unit_test_run(unit);
        break;

    case EML_TEST_ID:
        rv = c3_ut_tmu_hash_run_unit_test(unit);
      break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    if (SOC_FAILURE(rv)) {
        return rv;
    } 

    tmu_ut_test_result = 0;
    return 0;
}

int
c3_ut_tmu_test_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit;

    if (tmu_ut_test_result < 0) {
        /* dont cleanup, helps debugging */
        return -1;
    }

    unit = pc3swTestInfo->unit;

    switch (pc3swTestInfo->testid) {

    case DM_TEST_ID: 
    case DM_UCODE_TEST_ID:        
        rv = c3_ut_tmu_dm_test_cleanup(unit);
        break;

    case EML_TEST_ID:
        rv = c3_ut_tmu_hash_cleanup_test(unit);
        break;

    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
