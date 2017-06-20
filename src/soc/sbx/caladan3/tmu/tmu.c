/*
 * $Id: tmu.c,v 1.69.14.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    tmu.c
 * Purpose: Caladan3 on TMU drivers
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/shmoo_ddr40.h>
#include <soc/phy/ddr40.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/ci.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/dma.h>
#include <soc/sbx/caladan3/tmu/dm.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/hash.h>
#include <soc/sbx/caladan3/tmu/wb_db_tmu.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>


/****************************/
/*** Static Global States ***/
/****************************/
soc_sbx_caladan3_tmu_dbase_t *_tmu_dbase[SOC_MAX_NUM_DEVICES];

/* Convetions:
 * All math are performed on bytes boundary 
 * 1KB = 2 pow 10 bytes
 * 1MB = 2 pow 20 bytes
 * 1GB = 2 pow 30 bytes
 */
#define SOC_SBX_CALADAN3_TMU_DEF_DRAM_SIZE_MBITS (2 * 1024) /* 2 GB */
#define SOC_SBX_CALADAN3_TMU_DEF_NUM_DRAM (16) /* total number of DRAM */
#define SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM (8) /* total banks per DRAM */
#define SOC_SBX_CALADAN3_TMU_DRAM_ROW_SIZE_BYTES (2*1024)
#define SOC_SBX_CALADAN3_TMU_MAX_TABLE_ENTRY_PER_ROW (256)

#define TIME_STAMP_DBG
#undef TIME_STAMP_DBG
#ifdef TIME_STAMP_DBG 
sal_usecs_t        start;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    LOG_CLI((BSL_META("\n %s: Time Stamp: [%u]"), msg, SAL_USECS_SUB(sal_time_usecs(), start))); \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#endif

typedef enum _DDRMemTests {
  DDR_ALL_TESTS = 0,
  DDR_STANDARD_TEST = 1,
  DATA_BUS_WALKING_ONES = 2,
  DATA_BUS_WALKING_ZEROS = 3,
  DDR_DATA_EQ_ADDR = 4,
  DDR_INDIRECT_TEST = 5,
  /* leave as last */
  MEM_TEST_LAST
} DDRMemTests;



/*
 *
 * Function:
 *     tmu_memory_init
 * Purpose:
 *     Initialize tmu memories
 */
STATIC int _tmu_memory_init(int unit)
{
    soc_sbx_caladan3_dram_config_t *dram_cfg;
    uint16              dev_id;
    uint8               rev_id;

    /* initialize sws database */
    if (_tmu_dbase[unit] == NULL) {
        return SOC_E_INIT;
    }

    /* DRAM Properties */
    dram_cfg = &_tmu_dbase[unit]->dram_cfg;

    /* can override with soc parameters */
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (dev_id == BCM88034_DEVICE_ID) {
	dram_cfg->num_dram = SOC_SBX_CALADAN3_TMU_DEF_NUM_DRAM/2;
    } else {
	dram_cfg->num_dram = SOC_SBX_CALADAN3_TMU_DEF_NUM_DRAM;
    }

    dram_cfg->num_dram_banks = SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM;

    dram_cfg->dram_size_mbytes = soc_property_get(unit, spn_EXT_RAM_TOTAL_SIZE,
                                                  SOC_SBX_CALADAN3_TMU_DEF_DRAM_SIZE_MBITS/8);
    if (dram_cfg->dram_size_mbytes % MEM_1GBITS_IN_MBYTES) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d DDR memory part not in multiple of Gbits!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }
    
    dram_cfg->dram_row_size_bytes = SOC_SBX_CALADAN3_TMU_DRAM_ROW_SIZE_BYTES;
    return SOC_E_NONE;
}

STATIC int _tmu_region_map_init(int unit)
{
    int nRegion, nDramBankPair;
    uint32 uRegionbase, uDram, uBank, uDramPool, uInitialDramPool;
    uint32 auRegionTableEntry[2];
    int anDramFields[] = {DRAM0f, DRAM1f, DRAM2f, DRAM3f};
    int anBankFields[] = {BANK0f, BANK1f, BANK2f, BANK3f};
    uint32 *pDmaData, nDmaIndex;
    char *apDramBankRegionbaseVectors[SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION];
    uint32 auAvailableRegionbaseList[SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM * SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION];
    uint32 uAvailableRegionbaseListLength;
    uint32 auDram[SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION], auBank[SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION];
    uint32 uDramBankRegionbaseVectorIndex, uIndex;
    uint32 uDramBank;
    int nNumberOfRegionsPerBank, nNumberOfDramsPerPool;
    int status = SOC_E_NONE;
    soc_sbx_caladan3_region_config_t *region_cfg;
    soc_sbx_caladan3_dram_config_t *dram_cfg;
    uint32 banksize;

    /* initialize sws database */
    if (_tmu_dbase[unit] == NULL) {
        return SOC_E_INIT;
    } 

    region_cfg = &_tmu_dbase[unit]->region_cfg;
    dram_cfg = &_tmu_dbase[unit]->dram_cfg;

    /* number of horizontal = (number of Dram) * (8 bank per Dram) / (4 bank in each region) 
     * number of vertical = (number of region) / number of horizontal
     * banksize = (size of dram) / (8 bank per Dram)
     */
    region_cfg->num_horizontal = dram_cfg->num_dram * 2;
    region_cfg->num_vertical = SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION / region_cfg->num_horizontal;
    banksize = (dram_cfg->dram_size_mbytes * 1024)/dram_cfg->num_dram_banks;

    /* regionsize =  banksize * num-banks-per-region / num-vertical-region 
     * NOTE: this fomula need to be fixed if number of dram is not power of 2.
     */
    region_cfg->region_size_kbytes = (banksize * SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION) /
                                      region_cfg->num_vertical;

    /* rows per region per bank = banksize / (numverticalregion * rowsize) */
    region_cfg->rows_per_region_per_bank = (banksize * 1024)/
                                           (dram_cfg->dram_row_size_bytes * 
                                            region_cfg->num_vertical);

    pDmaData = (uint32 *) soc_cm_salloc( unit, (SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION * sizeof(auRegionTableEntry)), "region map" );
    nDmaIndex = 0;
    
    nNumberOfDramsPerPool = dram_cfg->num_dram / SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; 
    nNumberOfRegionsPerBank = region_cfg->num_vertical;
    
    
    for ( uDramPool = 0; uDramPool < SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; ++uDramPool ) {
	apDramBankRegionbaseVectors[uDramPool] = (char *) sal_alloc( SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION, "vector" );
	sal_memset( apDramBankRegionbaseVectors[uDramPool], 0x01, SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION );
    }
    
    for ( nRegion = 0; nRegion < SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION; ++nRegion ) {
	
	auRegionTableEntry[0] = auRegionTableEntry[1] = 0;
	
	uInitialDramPool = sal_rand() % SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION;
	
	while ( 1 ) {
	    uDramBankRegionbaseVectorIndex = sal_rand() % SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION;
	    if ( apDramBankRegionbaseVectors[uInitialDramPool][uDramBankRegionbaseVectorIndex] ) {
		apDramBankRegionbaseVectors[uInitialDramPool][uDramBankRegionbaseVectorIndex] = 0;
		break;
	    }
	}
	
	uDramBank = uDramBankRegionbaseVectorIndex / nNumberOfRegionsPerBank;
	uRegionbase = uDramBankRegionbaseVectorIndex % nNumberOfRegionsPerBank;
	auBank[uInitialDramPool] = uDramBank % SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM;
	auDram[uInitialDramPool] = (nNumberOfDramsPerPool * uInitialDramPool) + (uDramBank / SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM);
	
	for ( uDramPool = 0; uDramPool < SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; ++uDramPool ) {
	    if ( uDramPool != uInitialDramPool ) {
		
		for ( uDram = 0, uAvailableRegionbaseListLength = 0; uDram < nNumberOfDramsPerPool; ++uDram ) {
		    for ( uBank = 0; uBank < SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM; ++uBank ) {
			uIndex = (uDram * nNumberOfRegionsPerBank * SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM) + 
			    (uBank * nNumberOfRegionsPerBank) + uRegionbase;
			if ( uIndex < SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION && apDramBankRegionbaseVectors[uDramPool][uIndex] ) {
			    auAvailableRegionbaseList[uAvailableRegionbaseListLength++] = (uDram << 4) | uBank;
			}
		    }
		}
		
		if ( uAvailableRegionbaseListLength == 0 ) {
		    return SOC_E_INIT;
		}
		
		uIndex  = sal_rand() % uAvailableRegionbaseListLength; 
		uDram = auAvailableRegionbaseList[uIndex] >> 4;
		auBank[uDramPool] =  auAvailableRegionbaseList[uIndex] & 0xf;
		auDram[uDramPool] = (nNumberOfDramsPerPool * uDramPool) + uDram; 
		uDramBankRegionbaseVectorIndex = (uDram * nNumberOfRegionsPerBank * SOC_SBX_CALADAN3_TMU_BANK_PER_DRAM) +
		    (auBank[uDramPool] * nNumberOfRegionsPerBank) + uRegionbase;
		apDramBankRegionbaseVectors[uDramPool][uDramBankRegionbaseVectorIndex] = 0;
		
	    }
	}
	
	for ( nDramBankPair = 0; nDramBankPair < SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; ++nDramBankPair ) {
	    soc_mem_field_set( unit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
			       auRegionTableEntry, anDramFields[nDramBankPair], &auDram[nDramBankPair] );
	    soc_mem_field_set( unit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
			       auRegionTableEntry, anBankFields[nDramBankPair], &auBank[nDramBankPair] );
	}
	
	uRegionbase *= (((banksize * 1024)/dram_cfg->dram_row_size_bytes)/region_cfg->num_vertical);
	
	soc_mem_field_set( unit, TMB_DISTRIBUTOR_REGION_DEFINITIONm,
			   auRegionTableEntry, REGION_BASEf, &uRegionbase );
	
	pDmaData[nDmaIndex++] = auRegionTableEntry[0];
	pDmaData[nDmaIndex++] = auRegionTableEntry[1];
    }

    status = soc_mem_write_range( unit, TMB_DISTRIBUTOR_REGION_DEFINITIONm, MEM_BLOCK_ANY, 0,
			 (SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION - 1), (void *) pDmaData);
    soc_cm_sfree( unit, pDmaData );
    for ( uDramPool = 0; uDramPool < SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; ++uDramPool ) {
	sal_free( apDramBankRegionbaseVectors[uDramPool] );
    }

    return status;

}
/*
 *
 * Function:
 *     tmu_table_mgr_uninit
 * Purpose:
 *     cleanup
 */
int _tmu_table_mgr_uninit(int unit)
{
    soc_sbx_caladan3_tmu_program_config_t *program_cfg;
    program_cfg = &_tmu_dbase[unit]->program_cfg;
    if (program_cfg->info) {
        sal_free(program_cfg->info);
    }
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     tmu_table_mgr_init
 * Purpose:
 *     Initalize table manager datastructures
 */
STATIC int _tmu_table_mgr_init(int unit)
{
    int status = SOC_E_NONE;
    soc_sbx_caladan3_table_config_t *table_cfg;
    soc_sbx_caladan3_tmu_program_config_t *program_cfg;
    uint16              dev_id;
    uint8               rev_id;
    int                 row_idx;
    /* initialize sws database */
    if (_tmu_dbase[unit] == NULL) {
        return SOC_E_INIT;
    } 

    table_cfg = &_tmu_dbase[unit]->table_cfg;
    program_cfg = &_tmu_dbase[unit]->program_cfg;

    sal_memset(&table_cfg->table_attr[0], 0, 
               sizeof(soc_sbx_caladan3_table_attr_t) * SOC_SBX_CALADAN3_TMU_MAX_TABLE);

    SOC_SBX_C3_BMP_CLEAR(table_cfg->alloc_bmap, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS);
    SOC_SBX_C3_BMP_CLEAR(table_cfg->row_bmap, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS);
    for(row_idx = 0; row_idx < SOC_SBX_CALADAN3_TMU_MAX_BMP; row_idx++) {
        table_cfg->col_used_bits[row_idx] = 0;
        SOC_SBX_C3_BMP_CLEAR(table_cfg->col_bmap[row_idx], SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS);
    }
    
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (dev_id == BCM88034_DEVICE_ID) {
	table_cfg->default_replication = SOC_SBX_CALADAN3_TMU_TABLE_2X_REPLICATION;
    } else {
	table_cfg->default_replication = SOC_SBX_CALADAN3_TMU_TABLE_4X_REPLICATION;
    }

    /* allocate space for program database */
    program_cfg->info = sal_alloc(sizeof(soc_sbx_caladan3_tmu_program_info_t) * 
                                  SOC_SBX_CALADAN3_TMU_MAX_PROGRAM, "tmu-program-dbase");
    if (program_cfg->info) {
        sal_memset(program_cfg->info, 0, sizeof(soc_sbx_caladan3_tmu_program_info_t) * 
                   SOC_SBX_CALADAN3_TMU_MAX_PROGRAM);
        SOC_SBX_C3_BMP_CLEAR(program_cfg->alloc_bmap, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS);
    } else {
        status = SOC_E_MEMORY;
    }

    return status;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_region_map_dump
 * Purpose:
 *     Dump region mapping
 */
void soc_sbx_caladan3_tmu_region_map_dump(int unit, int min, int max)
{
    soc_sbx_caladan3_region_config_t *region_cfg;
    soc_sbx_caladan3_region_attr_t *region_attr;
    int region, index;

    if (min < 0 || max < 0) {
        min = 0;
        max = SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION;
    }

    /* initialize sws database */
    if (_tmu_dbase[unit] == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "!!! No region mapping configured !!!\n")));
    } else {
        region_cfg = &_tmu_dbase[unit]->region_cfg;
        LOG_CLI((BSL_META_U(unit,
                            "\n================================")));
        LOG_CLI((BSL_META_U(unit,
                            "\n TMU Region Map Dump ")));
        LOG_CLI((BSL_META_U(unit,
                            "\n Number of Horizontal Region: %d Vertical: %d Total: %d Size: %d"),
                 region_cfg->num_horizontal, region_cfg->num_vertical,
                 SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION, region_cfg->region_size_kbytes));
        LOG_CLI((BSL_META_U(unit,
                            "\n================================")));
     
        for (region=min; region < max; region++) {

            region_attr = &_tmu_dbase[unit]->region_cfg.region_attr[region];

            LOG_CLI((BSL_META_U(unit,
                                "\n Region %d Mapping: "), region));

            for (index=0; index < SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION; index++) {
                LOG_CLI((BSL_META_U(unit,
                                    "d-%2d:b-%2d "), region_attr->map[index].dram, region_attr->map[index].bank));
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));
    }
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_scrambler_init
 * Purpose:
 *     Bring up TMU hardware
 */
STATIC int _soc_sbx_caladan3_tmu_scrambler_init(int unit, int bypass) 
{
    uint32 regval=0;
    int memidx, index, tableidx, count, subtablesize, status = SOC_E_NONE;
#define SOC_SBX_TMU_NUM_SCRAMBLER_TABLE (9)
#define SOC_SBX_TMU_SCRAMBLER_TABLE_SIZE_BITS (512)
    uint32 randtable[SOC_SBX_TMU_SCRAMBLER_TABLE_SIZE_BITS], randnum; 
    soc_mem_t scrambler_random_mem[3] = {TMB_DISTRIBUTOR_SCRAMBLE_TABLE0m,
					 TMB_DISTRIBUTOR_SCRAMBLE_TABLE1m,
					 TMB_DISTRIBUTOR_SCRAMBLE_TABLE2m};

    /* bypass scrambler for now */
    if (bypass) {
        SOC_IF_ERROR_RETURN(READ_TMB_DISTRIBUTOR_DEBUGr(unit, &regval));
        soc_reg_field_set(unit, TMB_DISTRIBUTOR_DEBUGr, &regval, BYPASS_SCRAMBLERf, 1);
        soc_reg_field_set(unit, TMB_DISTRIBUTOR_DEBUGr, &regval, BYPASS_SCRAMBLER_CRCf, 1);
        soc_reg_field_set(unit, TMB_DISTRIBUTOR_DEBUGr, &regval, CONFIG_RAND_LB_MODEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_TMB_DISTRIBUTOR_DEBUGr(unit, regval));
    } else {
	for (memidx = 0; memidx < 3; memidx++) {
	    subtablesize = SOC_SBX_TMU_SCRAMBLER_TABLE_SIZE_BITS;
	    tableidx=0;
	    
	    for (count=0; count < SOC_SBX_TMU_NUM_SCRAMBLER_TABLE; count++) {
		
		for (index = 0; index < subtablesize; index++) {
		    randtable[index] = 0x80000000 | index;
		}
		
		for (index = 0; index < subtablesize; index++) {
		    while (TRUE) {
			randnum = sal_rand() % subtablesize;
			if (randtable[randnum] & 0x80000000) {
			    randtable[randnum] &= 0x7fffffff;
			    break;
			}
		    }
		    
		    status = soc_mem_write(unit, scrambler_random_mem[memidx], MEM_BLOCK_ANY, tableidx, 
					   (void *) &(randtable[randnum]) );
		    if (SOC_FAILURE(status)) {
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Failed to DMA region mapping data %d \n"),
                                   FUNCTION_NAME(), unit, status));
		    }
		    
		    ++tableidx;
		}
		subtablesize >>= 1;
	    }
	}

	for ( index = 0; index < 64; ++index ) {
	    randnum = sal_rand();
	    WRITE_TMB_GLOBAL_TABLE_SCRAMBLER_CONFIGr(unit, index, randnum);
	}
    }
    
    return status;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_hash_init
 * Purpose:
 *     Hash init
 */
STATIC int _soc_sbx_caladan3_tmu_hash_init(int unit, uint8 bypass) 
{
    uint32 regval=0;

    bypass = (bypass > 0)?TRUE:FALSE;

    /* bypass scrambler for now */
    SOC_IF_ERROR_RETURN(READ_TMA_HASH_BYPASS_DEBUGr(unit, &regval ));
    soc_reg_field_set(unit, TMA_HASH_BYPASS_DEBUGr, &regval, BYPASS_HASHf, bypass);
    SOC_IF_ERROR_RETURN(WRITE_TMA_HASH_BYPASS_DEBUGr(unit, regval));
    SOC_IF_ERROR_RETURN(READ_TMB_HASH_BYPASS_DEBUGr(unit, &regval));
    soc_reg_field_set(unit, TMB_HASH_BYPASS_DEBUGr, &regval, BYPASS_HASHf, bypass);
    SOC_IF_ERROR_RETURN(WRITE_TMB_HASH_BYPASS_DEBUGr(unit, regval));
    return SOC_E_NONE;
}

extern int soc_sbx_caladan3_tmu_ddr_test(int unit, DDRMemTests mode, uint32 pattern );

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hw_init
 * Purpose:
 *     Bring up TMU hardware
 */
int soc_sbx_caladan3_tmu_hw_init(int unit) 
{
    int status = SOC_E_NONE;
    uint32 regval=0, index;
    uint16              dev_id;
    uint8               rev_id;

    if (SAL_BOOT_PLISIM) {
      return status;
    }

    /*
     * Take blocks out of soft reset
     */
    regval = 0;
    SOC_IF_ERROR_RETURN(WRITE_TMA_CONTROLr( unit, regval));
    SOC_IF_ERROR_RETURN(READ_TMA_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, TMA_CONTROLr, &regval, TMA_SOFT_RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TMA_CONTROLr( unit, regval));
    
    regval = 0;
    SOC_IF_ERROR_RETURN(WRITE_TMB_CONTROLr( unit, regval));
    SOC_IF_ERROR_RETURN(READ_TMB_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, TMB_CONTROLr, &regval, TMB_SOFT_RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TMB_CONTROLr(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, TMB_DISTRIBUTOR_CONFIGr, &regval, REGION_MAPPER_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TMB_DISTRIBUTOR_CONFIGr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr(unit, &regval));
    soc_reg_field_set( unit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &regval, TRC_COST_WEIGHTf, 0 );
    soc_reg_field_set( unit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &regval, TFAW_COST_WEIGHTf, 0 );
    soc_reg_field_set( unit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &regval, TREF_COST_WEIGHTf, 0 );
    soc_reg_field_set( unit, TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr, &regval, TWR_COST_WEIGHTf, 0 );
    WRITE_TMB_DISTRIBUTOR_COST_WEIGHTS_CONFIGr( unit, regval );

    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_tmu_scrambler_init(unit, _tmu_dbase[unit]->control_cfg.bypass_scrambler));
    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_tmu_hash_init(unit, _tmu_dbase[unit]->control_cfg.bypass_hash));

    /* Init QE */
    for (index = 0; index < SOC_SBX_CALADAN3_TMU_QE_INSTANCE_NUM; index++) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, TM_QE_CONFIGr, SOC_BLOCK_PORT(unit,index), 0, &regval));
        soc_reg_field_set(unit, TM_QE_CONFIGr, &regval, CACHE_ENABLEf, 1);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, TM_QE_CONFIGr, SOC_BLOCK_PORT(unit,index), 0, regval));
    }
    if ( soc_property_get(unit, spn_BIST_ENABLE, 0) ) {
        soc_sbx_caladan3_tmu_ddr_test( unit, DDR_DATA_EQ_ADDR, 0 );
    }

    if (!SAL_BOOT_PLISIM && TMU_CI_DDR_PATTERN_INIT) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_ci_memory_pattern_init(unit));
    }

    /* reset value for all programs is NOTHING, no action is required by software */
    SOC_IF_ERROR_RETURN(READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval));
    soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO0_AEMPTYf, 1);
    soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO1_AEMPTYf, 1);
    soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO2_AEMPTYf, 1);
    soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO3_AEMPTYf, 1); 
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, regval));

    /* set afull threshold high so free fifo does not flow control */
    SOC_IF_ERROR_RETURN(READ_TMB_UPDATER_FIFO_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, TMB_UPDATER_FIFO_CONFIGr, &regval, FREE_CHAIN_FIFO_AFULL_THRESHf, 1023);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_FIFO_CONFIGr(unit, regval));
    
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_taps_hw_init(unit));
    
    /* can override with soc parameters */
    soc_cm_get_id(unit, &dev_id, &rev_id);

    if ((rev_id == BCM88030_A0_REV_ID) ||
        (rev_id == BCM88030_A1_REV_ID)) {
        /* EML144 is not supported on A0/A1 */
        _tmu_dbase[unit]->control_cfg.eml_144_mode = SOC_SBX_TMU_EML_144BITS_MODE_OFF;       
    }

    if ((rev_id == BCM88030_B0_REV_ID) ||
        (rev_id == BCM88030_B1_REV_ID)) {
        /* For B0/B1, enable interrupts for QE block */
        soc_cmicm_intr4_enable(unit, (0xffff<<(SOC_SBX_CALADAN3_QE15_INTR_POS%32)));
    }
    
    return status;
}
/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_driver_uninit
 * Purpose:
 *     Clean up 
 */
int soc_sbx_caladan3_tmu_driver_uninit(int unit)
{
    int ret = SOC_E_NONE;

    ret = soc_sbx_caladan3_tmu_taps_driver_uninit(unit);
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Taps driver uninit failed\n"), unit));
        return ret;
    }
    ret = soc_sbx_caladan3_tmu_hash_uninit(unit);
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d tmu hash uninit failed\n"), unit));
        return ret;
    }
    ret = tmu_resp_dma_mgr_uninit(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO); 
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d response dma manager uninit failed\n"), unit));
        return ret;
    }
    ret = tmu_cmd_mgr_uninit(unit);
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d cmd manager uninit failed\n"), unit));
        return ret;
    }
    ret = tmu_cmd_dma_mgr_uninit(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO); 
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d cmd dma manager uninit failed\n"), unit));
        return ret;
    }
    ret = _tmu_table_mgr_uninit(unit);
    if (SOC_FAILURE(ret)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d tmu table manager uninit failed\n"), unit));
        return ret;
    }

#ifdef TMU_USE_LOCKS
#ifdef TMU_USE_SPINLOCK
    /* destroy the TMU spin lock */
    if (SOC_FAILURE(pthread_spin_destroy(&_tmu_dbase[unit]->tmu_spin_lock)))
      {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to destroy tmu spinlock\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_RESOURCE;
	
      }
#else
    if (_tmu_dbase[unit]->tmu_mutex) {
        sal_mutex_destroy(_tmu_dbase[unit]->tmu_mutex);
        _tmu_dbase[unit]->tmu_mutex = NULL;
    }
#endif
#endif
#ifdef TMU_DMA_USE_LOCKS
#ifdef TMU_DMA_USE_SPINLOCK
    /* destroy the DMA spin lock */
    if (SOC_FAILURE(pthread_spin_destroy(&_tmu_dbase[unit]->dma_spin_lock)))
      {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to destroy dma spinlock\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_RESOURCE;
	
      }
#else
    if (_tmu_dbase[unit]->dma_mutex) {
        sal_mutex_destroy(_tmu_dbase[unit]->dma_mutex);
        _tmu_dbase[unit]->dma_mutex = NULL;
    }
#endif
#endif

    if (_tmu_dbase[unit] != NULL) {
        sal_free(_tmu_dbase[unit]);
        _tmu_dbase[unit] = NULL;
    }

    return ret;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hw_interrupt_cleanup
 * Purpose:
 *     Clean up interrupts fired during init and mask off 
 */
static void soc_sbx_caladan3_tmu_hw_interrupt_cleanup(int unit) 
{
    int _tmuErrorRegisters[] = { TMA_DIRECT_ERROR0r,
                                TMA_DIRECT_ERROR1r,
                                TMA_DIRECT_ERROR2r,
                                TMA_DIRECT_ERROR3r,
                                TMA_KEYPLODER_ERRORr,
                                TMB_DISTRIBUTOR_SCHEDULER_ERRORr,
                                TMB_DISTRIBUTOR_REFRESH_ERRORr,
                                TMB_DISTRIBUTOR_INTERFACE_FIFO_ERRORr,
                                TMB_DISTRIBUTOR_FUNNEL_FIFO_ERRORr,
                                TMB_UPDATER_ERRORr,
                                TMB_COMPLETION_ERROR0r,
                                TMB_COMPLETION_ERROR1r,
                                TMB_COMPLETION_PARITY_ERRORr };

    int _tmuErrorRegistersCount = COUNTOF(_tmuErrorRegisters);
    int _tmuErrorMaskRegisters[] = { TMA_DIRECT_ERROR0_MASKr,
                                    TMA_DIRECT_ERROR1_MASKr,
                                    TMA_DIRECT_ERROR2_MASKr,
                                    TMA_DIRECT_ERROR3_MASKr,
                                    TMA_KEYPLODER_ERROR_MASKr,
                                    TMB_DISTRIBUTOR_SCHEDULER_ERROR_MASKr,
                                    TMB_DISTRIBUTOR_REFRESH_ERROR_MASKr,
                                    TMB_DISTRIBUTOR_INTERFACE_FIFO_ERROR_MASKr,
                                    TMB_DISTRIBUTOR_FUNNEL_FIFO_ERROR_MASKr,
                                    TMB_UPDATER_ERROR_MASKr,
                                    TMB_COMPLETION_ERROR0_MASKr,
                                    TMB_COMPLETION_ERROR1_MASKr,
                                    TMB_COMPLETION_PARITY_ERROR_MASKr };
    int _tmuErrorMaskRegistersCount = COUNTOF(_tmuErrorMaskRegisters);

    int _tmuCiErrorRegisters[] = { CI_ERRORr };
    int _tmuCiErrorRegistersCount = COUNTOF(_tmuCiErrorRegisters);
    int _tmuCiErrorMaskRegisters[] = { CI_ERROR_MASKr };
    int _tmuCiErrorMaskRegistersCount = COUNTOF(_tmuCiErrorMaskRegisters);
    int _tmuTpErrorRegisters[] = { TP_GLOBAL_EVENTr,
                                  TP_ECC_ERRORr,
                                  TP_TCAM_SCAN_ERRORr,
                                  TP_ECC_STATUS0r,
                                  TP_ECC_STATUS1r };
    int _tmuTpErrorRegistersCount = COUNTOF(_tmuTpErrorRegisters);
    int _tmuTpErrorMaskRegisters[] = { TP_GLOBAL_EVENT_MASKr,
                                      TP_ECC_ERROR_MASKr,
                                      TP_TCAM_SCAN_ERROR_MASKr };
    int _tmuTpErrorMaskRegistersCount = COUNTOF(_tmuTpErrorMaskRegisters);
    
    int _tmuQeErrorRegisters[] = { TM_QE_ERRORr,
                                  TM_QE_FIFO_OVERFLOW_ERRORr,
                                  TM_QE_FIFO_UNDERFLOW_ERRORr,
                                  TM_QE_FIFO_PARITY_ERRORr,
                                  TM_QE_ECC_DEBUGr };
    int _tmuQeErrorRegistersCount = COUNTOF(_tmuQeErrorRegisters);
    int _tmuQeErrorMaskRegisters[] = { TM_QE_ERROR_MASKr,
                                      TM_QE_FIFO_OVERFLOW_ERROR_MASKr,
                                      TM_QE_FIFO_UNDERFLOW_ERROR_MASKr,
                                      TM_QE_FIFO_PARITY_ERROR_MASKr };
    int _tmuQeErrorMaskRegistersCount = COUNTOF(_tmuQeErrorMaskRegisters);

    int index, instance;
    uint32 uRegisterValue;

    for ( index = 0; index < _tmuErrorRegistersCount; ++index ) {
        soc_reg32_set( unit, _tmuErrorRegisters[index], SOC_BLOCK_PORT(unit,0), 0,
                       ((index < _tmuErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
    }
    for ( index = 0; index < _tmuErrorMaskRegistersCount; ++index ) {
        soc_reg32_set( unit, _tmuErrorMaskRegisters[index], SOC_BLOCK_PORT(unit,0), 0, 0xffffffff );
    }
    
    for ( instance = 0; instance < SOC_SBX_CALADAN3_TMU_QE_INSTANCE_NUM; ++instance ) {
        for ( index = 0; index < _tmuQeErrorRegistersCount; ++index ) {
            soc_reg32_set( unit, _tmuQeErrorRegisters[index], SOC_BLOCK_PORT(unit,instance), 0,
                           ((index < _tmuQeErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
        }
        for ( index = 0; index < _tmuQeErrorMaskRegistersCount; ++index ) {
            if (index == 0) {
                soc_reg32_set( unit, _tmuQeErrorMaskRegisters[index], SOC_BLOCK_PORT(unit,instance), 0, 0);
            } else {
                soc_reg32_set( unit, _tmuQeErrorMaskRegisters[index], SOC_BLOCK_PORT(unit,instance), 0, 0xffffffff );
            }
        }
    }
    
    for ( instance = 0; instance < SOC_SBX_CALADAN3_TMU_QE_INSTANCE_NUM; ++instance ) {
        for ( index = 0; index < _tmuCiErrorRegistersCount; ++index ) {
            soc_reg32_set( unit, _tmuCiErrorRegisters[index], SOC_BLOCK_PORT(unit,instance), 0,
                           ((index < _tmuCiErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
        }
        for ( index = 0; index < _tmuCiErrorMaskRegistersCount; ++index ) {
            uRegisterValue = 0;
            if ( _tmuCiErrorMaskRegisters[index] == CI_ERROR_MASKr ) {
                /* These errors needs to be masked due to a clock crossing boundary issue that errantly fires this error. (JIRA CA3-2791) */
                soc_reg_field_set( unit, CI_ERROR_MASKr, &uRegisterValue, WR_CMD_FORMAT_ERR_DISINTf, 1 );
                soc_reg_field_set( unit, CI_ERROR_MASKr, &uRegisterValue, WR_QUEUE_OVF_DISINTf, 1 );
            }
            soc_reg32_set( unit, _tmuCiErrorMaskRegisters[index], SOC_BLOCK_PORT(unit,instance), 0, uRegisterValue );
        }
    }
    
    for ( instance = 0; instance < 2; ++instance ) {
        for ( index = 0; index < _tmuTpErrorRegistersCount; ++index ) {
            soc_reg32_set( unit, _tmuTpErrorRegisters[index], SOC_BLOCK_PORT(unit,instance), 0,
                           ((index < _tmuTpErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
        }
        for ( index = 0; index < _tmuTpErrorMaskRegistersCount; ++index ) {
            soc_reg32_set( unit, _tmuTpErrorMaskRegisters[index], SOC_BLOCK_PORT(unit,instance), 0, 0xffffffff );
        }
    }
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_driver_init
 * Purpose:
 *     Bring up TMU drivers
 */
int soc_sbx_caladan3_tmu_driver_init(int unit) 
{
    int status = SOC_E_NONE;

    /* initialize sws database */
    if (_tmu_dbase[unit] != NULL) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_tmu_driver_uninit(unit));
    }

    _tmu_dbase[unit] = sal_alloc(sizeof(soc_sbx_caladan3_tmu_dbase_t), "tmu-dbase");
    if (_tmu_dbase[unit] == NULL) {
        return SOC_E_MEMORY;
    }

#ifdef TMU_USE_LOCKS
#ifdef TMU_USE_SPINLOCK
    /* create the TMU spin lock */
    if (SOC_FAILURE(pthread_spin_init(&_tmu_dbase[unit]->tmu_spin_lock,PTHREAD_PROCESS_PRIVATE)))
      {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to create tmu spinlock\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_RESOURCE;
	
      }
#else
    _tmu_dbase[unit]->tmu_mutex = sal_mutex_create("TMU MUTEX");
    if (_tmu_dbase[unit]->tmu_mutex == NULL) {
        sal_free(_tmu_dbase[unit]);
        return SOC_E_RESOURCE;
    }
#endif

#endif

#ifdef TMU_DMA_USE_LOCKS
#ifdef TMU_DMA_USE_SPINLOCK
    /* create the DMA spin lock */
    if (SOC_FAILURE(pthread_spin_init(&_tmu_dbase[unit]->dma_spin_lock,PTHREAD_PROCESS_PRIVATE)))
      {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to create dma spinlock\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_RESOURCE;
      }
#else
    _tmu_dbase[unit]->dma_mutex = sal_mutex_create("TMU DMA MUTEX");
    if (_tmu_dbase[unit]->dma_mutex == NULL) {
        sal_free(_tmu_dbase[unit]);
        return SOC_E_RESOURCE;
    }
#endif
#endif

#ifdef TMU_DM_USE_LOCKS
#ifdef TMU_DM_USE_SPINLOCK
    /* create the DM spin lock */
    if (SOC_FAILURE(pthread_spin_init(&_tmu_dbase[unit]->dm_spin_lock,PTHREAD_PROCESS_PRIVATE)))
      {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to create dm spinlock\n"),
                   FUNCTION_NAME(), unit));
	return SOC_E_RESOURCE;
      }
#else
    _tmu_dbase[unit]->dm_mutex = sal_mutex_create("TMU DM MUTEX");
    if (_tmu_dbase[unit]->dm_mutex == NULL) {
        sal_free(_tmu_dbase[unit]);
        return SOC_E_RESOURCE;
    }
#endif
#endif

    
    /* TMU controls */
    _tmu_dbase[unit]->control_cfg.bypass_hash = FALSE;
    _tmu_dbase[unit]->control_cfg.bypass_scrambler = FALSE;
    _tmu_dbase[unit]->control_cfg.eml_144_mode = SOC_SBX_TMU_EML_144BITS_MODE_UNKNOWN;

    status = soc_sbx_caladan3_tmu_hw_init(unit);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Hardware init failed %d \n"),
                   FUNCTION_NAME(), unit, status));
    }

    if (SOC_SUCCESS(status)) {
        status = _tmu_memory_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Memory init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        status = _tmu_region_map_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d DRAM to Region mapping failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        status = _tmu_table_mgr_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Table manager init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        status = tmu_cmd_mgr_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Command Manager Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        /* command dma init */
        status = tmu_cmd_dma_mgr_init(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO); 
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Command DMA Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        /* response dma init */
        status = tmu_resp_dma_mgr_init(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO); 
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Response DMA Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        status = soc_sbx_caladan3_tmu_hash_init(unit, _tmu_dbase[unit]->control_cfg.bypass_hash);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU Hash Database Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    if (SOC_SUCCESS(status)) {
        status = soc_sbx_caladan3_tmu_taps_driver_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TAPS driver Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }
    if (SOC_SUCCESS(status)) {
        status = soc_sbx_caladan3_tmu_dm_driver_init(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU DM driver Init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }
    
    if (!SAL_BOOT_PLISIM && TMU_CI_DDR_PATTERN_INIT && SOC_SUCCESS(status)) {
TIME_STAMP_START
        status = soc_sbx_caladan3_tmu_is_ci_memory_init_done(unit);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d CI DDR init failed %d \n"), 
                       FUNCTION_NAME(), unit, status));
        }
 TIME_STAMP("#### CI-DDR Memory Init Done")
     }

    if (SOC_FAILURE(status)) {
        sal_free(_tmu_dbase[unit]);
        _tmu_dbase[unit] = NULL;
    }

    soc_sbx_caladan3_tmu_hw_interrupt_cleanup(unit);

#ifdef 	BCM_WARM_BOOT_SUPPORT
    /* moved later in init to ensure other soc modules are complete */
    /* status = soc_sbx_tmu_wb_state_init(unit); */
#endif
    
    return status;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_driver_destroy
 * Purpose:
 *     Destroy TMU drivers
 */
int soc_sbx_caladan3_tmu_driver_destroy(int unit) 
{
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_post_cmd
 * Purpose:
 *     Post command to DMA command ring
 */
int soc_sbx_caladan3_tmu_post_cmd(int unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *cmd,
                                  soc_sbx_tmu_cmd_post_flag_e_t flag) 
{
    int status;
    TIME_STAMP_START

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    if (!_SOC_SBX_TMU_VALID_POST_FLAG(flag)) return SOC_E_PARAM;
    if (SOC_SBX_TMU_CMD_POST_FLUSH == flag && cmd) return SOC_E_PARAM;
    if (SOC_SBX_TMU_CMD_POST_FLUSH != flag && !cmd) return SOC_E_PARAM;


    status = tmu_dma_tx(unit, fifoid, cmd, flag);
    if (SOC_SUCCESS(status)) {
        if (cmd) {
            status = tmu_cmd_enqueue(unit, cmd, fifoid, FALSE);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to enqueue command on command manager %d!!!\n"), 
                           FUNCTION_NAME(), unit, status));
                status = SOC_E_INTERNAL;
            }
        }
    } else if (status != SOC_E_FULL && status != SOC_E_EMPTY) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed on TX DMA %d!!!\n"), 
                   FUNCTION_NAME(), unit, status));
    }

    TIME_STAMP("$ soc_sbx_caladan3_tmu_post_cmd") 
    return status;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_master_cache_cmd
 * Purpose:
 *     Cache dma commands for all master & slave, and enter master's queue
 */
int soc_sbx_caladan3_tmu_master_cache_cmd(int unit, int *slave_units, int num_slaves, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *cmd) 
{
    int status;
    TIME_STAMP_START

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    if (!cmd) return SOC_E_PARAM;


    status = tmu_dma_tx_share_cache_cmd(unit, slave_units, num_slaves, fifoid, cmd);
    if (SOC_SUCCESS(status)) {
        if (cmd) {
            status = tmu_cmd_enqueue(unit, cmd, fifoid, FALSE);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to enqueue command on command manager %d!!!\n"), 
                           FUNCTION_NAME(), unit, status));
                status = SOC_E_INTERNAL;
            }
        }
    } else if (status != SOC_E_FULL && status != SOC_E_EMPTY) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed on TX DMA %d!!!\n"), 
                   FUNCTION_NAME(), unit, status));
    }

    TIME_STAMP("$ soc_sbx_caladan3_tmu_post_cmd") 
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_master_flush_cmd
 * Purpose:
 *     Post command to DMA command ring for master & slaves
 */
int soc_sbx_caladan3_tmu_master_flush_cmd(int unit, int *slave_units, int num_slaves, int fifoid) 
{
    int status;

    status = tmu_dma_tx_master_flush_cmd(unit, slave_units, num_slaves, fifoid);
    if (SOC_FAILURE(status)){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed on TX DMA %d!!!\n"), 
                   FUNCTION_NAME(), unit, status));
    }

    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_slave_cache_cmd
 * Purpose:
 *     Get the seqnum of slave unit, and post commands into slave's queue
 */
int soc_sbx_caladan3_tmu_slave_cache_cmd(int unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *cmd) 
{
    int status;
    if (!cmd) return SOC_E_PARAM;   
    status = tmu_cmd_enqueue(unit, cmd, fifoid, TRUE);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue command on command manager %d!!!\n"), 
                   FUNCTION_NAME(), unit, status));
        status = SOC_E_INTERNAL;
    }
    
    return status;
}
STATIC int soc_sbx_caladan3_tmu_validate_resp(int unit, int fifoid,
                                              soc_sbx_caladan3_tmu_cmd_t *cmd,
                                              soc_sbx_caladan3_tmu_cmd_t *resp)
{
    int status = SOC_E_NONE;

    if (!cmd || !resp) {
        return SOC_E_PARAM;
    }

    if (cmd->opcode != resp->opcode) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Request & Response opcode mismatch: "
                              "Expected:%d Response:%d !!!\n"), 
                   FUNCTION_NAME(),unit, cmd->opcode, resp->opcode));
          
        status = SOC_E_FAIL;
    }

    if (SOC_SUCCESS(status) && (cmd->seqnum != resp->seqnum)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Request & Response Sequence number mismatch " 
                              "Expected:%d Response:%d !!!\n"), 
                   FUNCTION_NAME(), unit, cmd->seqnum, resp->seqnum));
        status = SOC_E_FAIL;
    }

    if (SOC_FAILURE(status)) {
        LOG_CLI((BSL_META_U(unit,
                            "# Expected: \n")));
        tmu_cmd_printf(unit, cmd); 
        LOG_CLI((BSL_META_U(unit,
                            "# Response: \n")));
        tmu_cmd_printf(unit, resp);         
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_get_resp
 * Purpose:
 *     Handle response from Response Manager
 */
int soc_sbx_caladan3_tmu_get_resp(int unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *expected_cmd,
                                  void *data_buffer, int data_buffer_len)
{
    int status=SOC_E_NONE, num_words=0, num_taps_response_words=0;
    soc_sbx_caladan3_tmu_cmd_t *cmd;
    soc_sbx_caladan3_tmu_cmd_t resp;

    if (SAL_BOOT_PLISIM || tmu_dma_skip_rx) return status;

    TIME_STAMP_START

    SOC_IF_TMU_UNINIT_RETURN(unit);    

    if (!expected_cmd) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input Arg!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    if ((SOC_SBX_TMU_EXPECT_RESPONSE_DATA(expected_cmd->opcode)) && 
        (!data_buffer || data_buffer_len <= 0)) {   
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input Arg!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM; 
    }


    /* soc_sbx_caladan3_cmd_mgr_dump(unit, fifoid); */

    status = tmu_dma_rx(unit, fifoid, &resp);
    if (SOC_SUCCESS(status)) {

        num_words = SOC_SBX_TMU_CMD_OFFSET_WORDS;
        if (resp.response_trailer) {
            num_words += SOC_SBX_TMU_CMD_OFFSET_WORDS;
            /* LOG_CLI((BSL_META_U(unit,
                                   "\n$$$$$$ Offsetting response words %d\n"), num_words)); */
        }

        /* dequeue, process response, validate it */
        status = tmu_cmd_dequeue(unit, &cmd, fifoid);
        if (SOC_SUCCESS(status)) {
            if (expected_cmd != cmd) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: %s: unit %d Mismatch with current & expected commands \n"), 
                           FUNCTION_NAME(), FILE_LINE_STRING(), unit));
                status = SOC_E_INTERNAL;
            }
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_validate_resp(unit, fifoid, cmd, &resp);
                if (SOC_SUCCESS(status)) {
                    /* pack response to user buffer if applicable */
                    if (resp.response_type == SOC_SBX_TMU_RESPONSE) {
                        /* non-TAPS response */
                        if (!(SOC_SBX_TMU_RESP_SUCCESS(resp.resp_ctrl.resp.errcode))) {
                            status = SOC_E_FAIL;
                            tmu_resp_error_print(unit, &resp);
                        }

                        if (SOC_SUCCESS(status) && SOC_SBX_TMU_EXPECT_RESPONSE_DATA(cmd->opcode)) {

                            num_words += resp.resp_ctrl.resp.size * 2; /* 64 bit size */

                            if (data_buffer_len < resp.resp_ctrl.resp.size*2) {
                                /* no enough space to pack back response data */
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Data buffer provided is too short for response \n"), 
                                           FUNCTION_NAME(), unit));
                                status = SOC_E_PARAM;                        
                            } else {
                                status = tmu_dma_rx_copy_data(unit, fifoid,
                                                              data_buffer, data_buffer_len, 
                                                              resp.resp_ctrl.resp.size);
                            }
                        }
                    } else {
                        /* TAPS response */
                        if (resp.resp_ctrl.taps.err0 || resp.resp_ctrl.taps.err1) {
                            status = SOC_E_FAIL;
                            tmu_resp_error_print(unit, &resp);
                            LOG_CLI((BSL_META_U(unit,
                                                "Command Issued: \n")));
                            tmu_cmd_printf(unit, cmd);     
                        }

                        if (SOC_SUCCESS(status) &&
                            (resp.resp_ctrl.taps.valid0 >0 || resp.resp_ctrl.taps.valid1 > 0)) {
                            num_taps_response_words = (BITS2WORDS(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) *
                                                       ((resp.resp_ctrl.taps.valid0)?1:0 + \
                                                        (resp.resp_ctrl.taps.valid1)?1:0));

                            num_words += num_taps_response_words;

                            if (data_buffer_len < num_taps_response_words) {
                                /* no enough space to pack back response data */
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Data buffer provided is "
                                                      "too short for response \n"), 
                                           FUNCTION_NAME(), unit));
                                status = SOC_E_PARAM;                        
                            } else {
                                status = tmu_dma_rx_copy_data(unit, fifoid,
                                                              data_buffer, data_buffer_len, 
                                                              (num_taps_response_words+1)/2);
                            }
                        }                        
                        
                    }

                    if (SOC_SUCCESS(status)) {
                        status = tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                    } else {
                        tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Response Processing Failed %d !!!\n"), 
                                   FUNCTION_NAME(), unit, status));
                    }

                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Invalid response !!!\n"), 
                               FUNCTION_NAME(), unit));
                    tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                }
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Error dequeuing expected response \n"), 
                       FUNCTION_NAME(), unit));
        }
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "!!! TMU RX Fail: TMU Command Issued: \n")));
        tmu_cmd_printf(unit, expected_cmd);
    }


    TIME_STAMP("$ soc_sbx_caladan3_tmu_get_resp")
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_slave_get_resp
 * Purpose:
 *     Handle response from Response Manager
 */
int soc_sbx_caladan3_tmu_slave_get_resp(int unit, int master_unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *expected_cmd,
                                  void *data_buffer, int data_buffer_len)
{
    int status=SOC_E_NONE, num_words=0, num_taps_response_words=0;
    soc_sbx_caladan3_tmu_cmd_t *cmd;
    soc_sbx_caladan3_tmu_cmd_t resp;

    if (SAL_BOOT_PLISIM || tmu_dma_skip_rx) return status;

    TIME_STAMP_START

    SOC_IF_TMU_UNINIT_RETURN(unit);    

    if (!expected_cmd) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input Arg!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    if ((SOC_SBX_TMU_EXPECT_RESPONSE_DATA(expected_cmd->opcode)) && 
        (!data_buffer || data_buffer_len <= 0)) {   
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input Arg!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM; 
    }


    /* soc_sbx_caladan3_cmd_mgr_dump(unit, fifoid); */

    status = tmu_dma_rx(unit, fifoid, &resp);
    if (SOC_SUCCESS(status)) {

        num_words = SOC_SBX_TMU_CMD_OFFSET_WORDS;
        if (resp.response_trailer) {
            num_words += SOC_SBX_TMU_CMD_OFFSET_WORDS;
            /* LOG_CLI((BSL_META_U(unit,
                                   "\n$$$$$$ Offsetting response words %d\n"), num_words)); */
        }

        /* dequeue, process response, validate it */
        status = tmu_cmd_dequeue(master_unit, &cmd, fifoid);
        if (SOC_SUCCESS(status)) {
            if (expected_cmd != cmd) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: %s: unit %d Mismatch with current & expected commands \n"), 
                           FUNCTION_NAME(), FILE_LINE_STRING(), unit));
                status = SOC_E_INTERNAL;
            }
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_validate_resp(unit, fifoid, cmd, &resp);
                if (SOC_SUCCESS(status)) {
                    /* pack response to user buffer if applicable */
                    if (resp.response_type == SOC_SBX_TMU_RESPONSE) {
                        /* non-TAPS response */
                        if (!(SOC_SBX_TMU_RESP_SUCCESS(resp.resp_ctrl.resp.errcode))) {
                            status = SOC_E_FAIL;
                            tmu_resp_error_print(unit, &resp);
                        }

                        if (SOC_SUCCESS(status) && SOC_SBX_TMU_EXPECT_RESPONSE_DATA(cmd->opcode)) {

                            num_words += resp.resp_ctrl.resp.size * 2; /* 64 bit size */

                            if (data_buffer_len < resp.resp_ctrl.resp.size*2) {
                                /* no enough space to pack back response data */
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Data buffer provided is too short for response \n"), 
                                           FUNCTION_NAME(), unit));
                                status = SOC_E_PARAM;                        
                            } else {
                                status = tmu_dma_rx_copy_data(unit, fifoid,
                                                              data_buffer, data_buffer_len, 
                                                              resp.resp_ctrl.resp.size);
                            }
                        }
                    } else {
                        /* TAPS response */
                        if (resp.resp_ctrl.taps.err0 || resp.resp_ctrl.taps.err1) {
                            status = SOC_E_FAIL;
                            tmu_resp_error_print(unit, &resp);
                            LOG_CLI((BSL_META_U(unit,
                                                "Command Issued: \n")));
                            tmu_cmd_printf(unit, cmd);     
                        }

                        if (SOC_SUCCESS(status) &&
                            (resp.resp_ctrl.taps.valid0 >0 || resp.resp_ctrl.taps.valid1 > 0)) {
                            num_taps_response_words = (BITS2WORDS(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) *
                                                       ((resp.resp_ctrl.taps.valid0)?1:0 + \
                                                        (resp.resp_ctrl.taps.valid1)?1:0));

                            num_words += num_taps_response_words;

                            if (data_buffer_len < num_taps_response_words) {
                                /* no enough space to pack back response data */
                                LOG_ERROR(BSL_LS_SOC_COMMON,
                                          (BSL_META_U(unit,
                                                      "%s: unit %d Data buffer provided is "
                                                      "too short for response \n"), 
                                           FUNCTION_NAME(), unit));
                                status = SOC_E_PARAM;                        
                            } else {
                                status = tmu_dma_rx_copy_data(unit, fifoid,
                                                              data_buffer, data_buffer_len, 
                                                              (num_taps_response_words+1)/2);
                            }
                        }                        
                        
                    }

                    if (SOC_SUCCESS(status)) {
                        status = tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                    } else {
                        tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Response Processing Failed %d !!!\n"), 
                                   FUNCTION_NAME(), unit, status));
                    }

                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Invalid response !!!\n"), 
                               FUNCTION_NAME(), unit));
                    tmu_dma_rx_advance_ring(unit, fifoid, num_words);
                }
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Error dequeuing expected response \n"), 
                       FUNCTION_NAME(), unit));
        }
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "!!! TMU RX Fail: TMU Command Issued: \n")));
        tmu_cmd_printf(unit, expected_cmd);
    }


    TIME_STAMP("$ soc_sbx_caladan3_tmu_get_resp")
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_validate_table_param
 * Purpose:
 *    Validate table parameters
 */
STATIC int _soc_sbx_caladan3_tmu_validate_table_param(int unit, 
                                                      soc_sbx_caladan3_table_attr_t *attr,
                                                      soc_sbx_caladan3_chain_table_attr_t *chain_attr)
{
    if (!attr) {
        return SOC_E_PARAM;
    }

    if (attr->flags && attr->flags != SOC_SBX_TMU_TABLE_FLAG_CHAIN) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Flags not yet supported !!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    if (attr->entry_size_bits % SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Entry size must be unit of %d-bits!!!\n"), 
                   FUNCTION_NAME(), unit, SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS));
        return SOC_E_PARAM;
    }

    if (attr->entry_size_bits < SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MIN ||
        attr->entry_size_bits > SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MAX) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Entry size must be between > %d && < %d bits !!!\n"), 
                   FUNCTION_NAME(), unit, 
                   SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MIN,
                   SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MAX));
        return SOC_E_PARAM;
    }

    if (chain_attr) {
        if (chain_attr->split_mode) return SOC_E_PARAM; /* no split mode supported */
        if (chain_attr->fifo_bitmap > 0xF) return SOC_E_PARAM; /* fifo 0-3 suppoted */
    }

    if (attr->num_entries < SOC_SBX_CALADAN3_TMU_MIN_TABLE_NUM_ENTRIES) {
        attr->num_entries = SOC_SBX_CALADAN3_TMU_MIN_TABLE_NUM_ENTRIES;
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s: unit %d Minimum number of entries per table = %d, resizing it !!!\n"), 
                     FUNCTION_NAME(), unit, SOC_SBX_CALADAN3_TMU_MIN_TABLE_NUM_ENTRIES));
    }

    /* round up? to power of 2 */
    soc_sbx_caladan3_round_power_of_two(unit, (unsigned int*) &attr->num_entries, FALSE);

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_hw_set
 * Purpose:
 *     set table
 */
STATIC int _soc_sbx_caladan3_tmu_table_hw_set(int unit, 
                                              soc_sbx_caladan3_table_attr_t *attr,
                                              soc_sbx_caladan3_chain_table_attr_t *chain_attr)
{
    uint32 regval=0;

    SOC_IF_ERROR_RETURN(READ_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr->id, &regval));
    soc_reg_field_set(unit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, ENTRY_SIZEf,
                      attr->entry_size_bits/SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS);
    soc_reg_field_set(unit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, NUM_ENTRIESf, 
                      soc_sbx_caladan3_msb_bit_pos(attr->num_entries));
    soc_reg_field_set(unit, TMA_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, NEXT_TABLEf, 0);
    SOC_IF_ERROR_RETURN(WRITE_TMA_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr->id, regval));
    
    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr->id, &regval));
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, ENTRY_SIZEf, 
                      attr->entry_size_bits/SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS);
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, NUM_ENTRIESf,  
                      soc_sbx_caladan3_msb_bit_pos(attr->num_entries));
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, NEXT_TABLEf, 0);
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_ENTRY_CONFIGr, &regval, EM_DEFAULTf, _TMU_DEF_HASH_MISS_ENTRY_IDX_);
    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_ENTRY_CONFIGr(unit, attr->id, regval));

    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_LAYOUT_CONFIGr(unit, attr->id, &regval));
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, REPLICATION_FACTORf, attr->replication_factor >> 1);
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, NUM_ROWS_PER_REGIONf,
                      soc_sbx_caladan3_msb_bit_pos(attr->num_row_per_region) + 2 - (attr->replication_factor >> 1));
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, NUM_ENTRIES_PER_ROWf,
                      soc_sbx_caladan3_msb_bit_pos(attr->num_entries_per_row));
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, COLUMN_OFFSETf, attr->column_offset);
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, ROW_OFFSETf, attr->row_offset);
    soc_reg_field_set(unit, TMB_GLOBAL_TABLE_LAYOUT_CONFIGr, &regval, REGION_OFFSETf, attr->region_offset);
    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_LAYOUT_CONFIGr(unit, attr->id, regval));
    
    SOC_IF_ERROR_RETURN(READ_TMA_GLOBAL_TABLE_DEADLINE_CONFIGr( unit, attr->id, &regval));
    soc_reg_field_set( unit, TMA_GLOBAL_TABLE_DEADLINE_CONFIGr, &regval, DEADLINEf, attr->deadline);
    SOC_IF_ERROR_RETURN(WRITE_TMA_GLOBAL_TABLE_DEADLINE_CONFIGr( unit, attr->id, regval));
    
    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_DEADLINE_CONFIGr( unit, attr->id, &regval));
    soc_reg_field_set( unit, TMB_GLOBAL_TABLE_DEADLINE_CONFIGr, &regval, DEADLINEf, attr->deadline);
    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_DEADLINE_CONFIGr( unit, attr->id, regval));
    
    SOC_IF_ERROR_RETURN(READ_TMB_GLOBAL_TABLE_CHAIN_CONFIGr( unit, attr->id, &regval));
    if ((chain_attr) && (!(attr->flags & SOC_SBX_TMU_TABLE_FLAG_CHAIN))) {
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval,
                          UP_CHAIN_LIMITf, chain_attr->length);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval,
                          UP_CHAIN_POOLf, chain_attr->fifo_bitmap);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval,
                          UP_CHAIN_SPLIT_MODEf, chain_attr->split_mode);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval, 
                          UP_CHAIN_HWf, chain_attr->hw_managed);
    } else {
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval, UP_CHAIN_LIMITf, 0);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval, UP_CHAIN_POOLf, 0);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval, UP_CHAIN_SPLIT_MODEf, 0);
        soc_reg_field_set(unit, TMB_GLOBAL_TABLE_CHAIN_CONFIGr, &regval, UP_CHAIN_HWf, 0);
    }
    SOC_IF_ERROR_RETURN(WRITE_TMB_GLOBAL_TABLE_CHAIN_CONFIGr( unit, attr->id, regval));

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_get_table_state
 * Purpose:
 *     get table related state
 */
int soc_sbx_caladan3_tmu_get_table_state(int unit, unsigned int tableid, int *state) 
{
    int status = SOC_E_NONE;
    soc_sbx_caladan3_table_config_t *table_cfg;
    
    if (!state || tableid >=SOC_SBX_CALADAN3_TMU_MAX_TABLE) return SOC_E_PARAM; 

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    TMU_LOCK(unit);

    table_cfg = &_tmu_dbase[unit]->table_cfg;

    if (SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, tableid)) {
        *state = TRUE; /* table is allocated */
    } else {
        *state = FALSE; /* table is free */
    }

    TMU_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_alloc
 * Purpose:
 *     Allocate a table
 */
static int
_soc_sbx_caladan3_tmu_table_alloc(int unit,
                                  soc_sbx_caladan3_table_attr_t *attr,
                                  soc_sbx_caladan3_chain_table_attr_t *chain_attr)
{
    int status = SOC_E_NONE;
    soc_sbx_caladan3_table_config_t *table_cfg;
    soc_sbx_caladan3_table_attr_t *table_attr;
    soc_sbx_caladan3_region_config_t *region_cfg;
    soc_sbx_caladan3_dram_config_t *dram_cfg;
    int curr_row, last_row, best_fit, alloc_row, hole, index, curr_col, alloc_col, free_cols, num_cols;
    int max_entries_per_region;

    if (!attr) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input argument !!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);    

    TMU_LOCK(unit);

    table_cfg = &_tmu_dbase[unit]->table_cfg;
    region_cfg = &_tmu_dbase[unit]->region_cfg;
    dram_cfg = &_tmu_dbase[unit]->dram_cfg;

    /* if replication factor is not specified (0), use the default */
    if (!attr->replication_factor) {
	attr->replication_factor = table_cfg->default_replication;
    }

    if ((attr->flags & SOC_SBX_TMU_TABLE_FLAG_WITH_ID) && 
        (SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, attr->id))) {
        /* realloc or duplicate alloc */
        status = SOC_E_EXISTS;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table Id [%d] in use !!!\n"), 
                   FUNCTION_NAME(), unit, attr->id));
        
    } else {
        status = _soc_sbx_caladan3_tmu_validate_table_param(unit, attr, chain_attr);
        if (SOC_SUCCESS(status)) {
	    /* num_entries_per_row means how many table entries each row (2KB) can hold */
            attr->num_entries_per_row = (dram_cfg->dram_row_size_bytes * 8)/attr->entry_size_bits;
            /* must round down */
            status = soc_sbx_caladan3_round_power_of_two(unit, &attr->num_entries_per_row, TRUE);
            if (SOC_SUCCESS(status)) {
		/* max_entries_per_region means number of table entries each region need to store
		 * on each replication.
		 */
                max_entries_per_region = attr->num_entries / SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION;
		max_entries_per_region = (max_entries_per_region * attr->replication_factor) / SOC_SBX_CALADAN3_TMU_TABLE_4X_REPLICATION;

                /* find number of regions required for given number of entries per row */
                if (max_entries_per_region <= attr->num_entries_per_row) {
                    /* round up to 1 row */
                    attr->num_row_per_region = 1;
                    attr->num_entries_per_row = (max_entries_per_region>=1)?max_entries_per_region:1;
                } else {
                    attr->num_row_per_region = max_entries_per_region/attr->num_entries_per_row;
                }
            }

            assert(SOC_SBX_POWER_OF_TWO(attr->num_entries_per_row));
            assert(SOC_SBX_POWER_OF_TWO(attr->num_row_per_region));
        }

        /* allocate row/col to fit the table */
        alloc_row = -1;
        alloc_col = 0;

        if ((attr->num_row_per_region == 1) &&
            ((attr->num_entries_per_row * attr->entry_size_bits) < (dram_cfg->dram_row_size_bytes * 8))) {
            /* if the table will take less than single row, try to fit in remaining columns of an allocated row */
            num_cols = (attr->num_entries_per_row * attr->entry_size_bits) /  SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS;

            SOC_SBX_C3_BMP_ITER(table_cfg->row_bmap, curr_row, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS) {

                if (curr_row >= region_cfg->rows_per_region_per_bank) {
                    /* pass last valid row */
                    break;
                }

                if (alloc_row >= 0) {
                    /* mem allocated */
                    break;
                }

                /* check if there is enough room left, this way can skip most of rows */
                if ((attr->num_entries_per_row * attr->entry_size_bits) < 
                    (dram_cfg->dram_row_size_bytes * 8 - table_cfg->col_used_bits[curr_row])) {
                    /* enough room left in the row, check if it's continous */
                    for (curr_col = 0, free_cols = 0;
                         curr_col < (dram_cfg->dram_row_size_bytes * 8) / SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS;
                         curr_col++) {
                        if (SOC_SBX_C3_BMP_MEMBER(table_cfg->col_bmap[curr_row], curr_col)) {
                            free_cols = 0;
                        } else {
                            free_cols++;
                        }

                        if (free_cols == num_cols) {
                            /* found num_cols free columns in the row */
                            alloc_row = curr_row;
                            alloc_col = curr_col - (num_cols - 1);

                            /* update number of free columns and free_column bitmap */
                            table_cfg->col_used_bits[curr_row] += (attr->num_entries_per_row * attr->entry_size_bits);
                            for (index = alloc_col;  index <= curr_col; index++) {
                                SOC_SBX_C3_BMP_ADD(table_cfg->col_bmap[curr_row], index);
                            }
                            break;
                        }
                    }
                }

            }
        }

        if (alloc_row < 0) {
            /* try to find consecutive rows to fit in the DM, also try to get the best fit to avoid sparse holes
             * only allocate from the rows that are not being used by any other table
             */
            curr_row = last_row = -1;
            best_fit = -1;
            
            SOC_SBX_C3_BMP_ITER(table_cfg->row_bmap, index, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS) {
                
                if (index >= region_cfg->rows_per_region_per_bank) break;
                
                curr_row = index;

                /* coverity[overrun-local : FALSE] */
                while ((index < region_cfg->rows_per_region_per_bank) && SOC_SBX_C3_BMP_MEMBER(table_cfg->row_bmap,index+1)) {
                    index++;
                }
                
                if (last_row < 0) {
                    hole = curr_row;
                } else {
                    hole = curr_row - last_row - 1;
                }
                
                curr_row = index;
                
                if (hole >= attr->num_row_per_region) {
                    if (best_fit < 0 || hole < best_fit) {
                        best_fit = hole;
                        alloc_row = (last_row < 0) ? 0:last_row + 1;
                    }
                } 
                
                last_row = curr_row;
            }

            if (alloc_row < 0) {
                /* if there is nothing allocated start from row offset 0 */
                if ((curr_row < 0) && 
                    (region_cfg->rows_per_region_per_bank > attr->num_row_per_region)) {
                    alloc_row = 0;
                } else if (curr_row >= 0 && curr_row == last_row) { /* no holes found, look from current to end */
                    /* there is only one allocation which starts on row offset non zero */
                    if ((region_cfg->rows_per_region_per_bank - curr_row - 1) >= attr->num_row_per_region) {
                        alloc_row = curr_row + 1;
                    }
                } 
            }

            if (alloc_row >= 0) {
                /* allocated, update the col_used_bits for each row allocated */
                for (curr_row = alloc_row; curr_row < (alloc_row+attr->num_row_per_region); curr_row++) {
                    table_cfg->col_used_bits[curr_row] = (attr->num_entries_per_row * attr->entry_size_bits);
                    for (curr_col = 0;
                         curr_col < (table_cfg->col_used_bits[curr_row] / SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS);
                         curr_col++) {
                        SOC_SBX_C3_BMP_ADD(table_cfg->col_bmap[curr_row], curr_col);
                    }
                }
            }
        }

        if (alloc_row < 0) {
            status = SOC_E_MEMORY;
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d No memory available to allocate table !!!\n"), 
                       FUNCTION_NAME(), unit));
        } else {
            if (!(attr->flags & SOC_SBX_TMU_TABLE_FLAG_WITH_ID)) {
                attr->id = SOC_SBX_CALADAN3_TMU_MAX_TABLE;
                /* allocate table ID */
                if (attr->flags & SOC_SBX_TMU_TABLE_FLAG_CHAIN) {
                    /* chain table can only be from id 0 - 31 */
                    for (index=0; index < SOC_SBX_CALADAN3_TMU_MAX_CHAIN_TABLE; index++) {
                        if (!SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, index)) {
                            break;
                        }
                    }
                    if (index < SOC_SBX_CALADAN3_TMU_MAX_CHAIN_TABLE) attr->id = index;
                } else {
                    /* top to bottom allocation */
                    for (index=SOC_SBX_CALADAN3_TMU_MAX_TABLE-1; index >= 0; index--) {
                        if (!SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, index)) {
                            break;
                        }
                    }
                    if (index >= 0) attr->id = index;
                }
            }

            if (attr->id < SOC_SBX_CALADAN3_TMU_MAX_TABLE) { /* invalid allocation failed */
                table_attr = &table_cfg->table_attr[attr->id];
                sal_memcpy(table_attr, attr, sizeof(soc_sbx_caladan3_table_attr_t));
                table_attr->row_offset = alloc_row;
                table_attr->column_offset = alloc_col;
                table_attr->region_offset = 0;
                table_attr->deadline = 0;
                /* table_attr->replication_factor = SOC_SBX_CALADAN3_TMU_TABLE_DEF_REPLICATION; */

                status = _soc_sbx_caladan3_tmu_table_hw_set(unit, table_attr, chain_attr);
                if (SOC_SUCCESS(status)) {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "%s: unit %d TMU Table ID [%d] Allocated Successfully: \n"
                                            "Row[%d] Col[%d] Row-per-region[%d] Entries-per-row[%d] replication[%d]\n"),
                                 FUNCTION_NAME(), unit, table_attr->id,
                                 table_attr->row_offset, table_attr->column_offset, 
                                 table_attr->num_row_per_region, table_attr->num_entries_per_row,
                                 table_attr->replication_factor));

                    SOC_SBX_C3_BMP_ADD(table_cfg->alloc_bmap, attr->id);

                    for (index=alloc_row; index < alloc_row + attr->num_row_per_region; index++) {
                        SOC_SBX_C3_BMP_ADD(table_cfg->row_bmap, index);
                    }
                } else {
                    sal_memset(table_attr, 0, sizeof(soc_sbx_caladan3_table_attr_t));
                }
            } else {
                status = SOC_E_RESOURCE;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to allocate TMU Table Id: Error(%d) !!!\n"), 
                           FUNCTION_NAME(), unit, status));
            }
        }
    }


    TMU_UNLOCK(unit);

    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_alloc
 * Purpose:
 *     Allocate a table
 */
int soc_sbx_caladan3_tmu_table_alloc(int unit,
                                      soc_sbx_caladan3_table_attr_t *attr)
{
    return _soc_sbx_caladan3_tmu_table_alloc(unit, attr, NULL);
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_chain_table_alloc
 * Purpose:
 *     Allocate a chain table
 */
int soc_sbx_caladan3_tmu_chain_table_alloc(int unit,
                                           soc_sbx_caladan3_table_attr_t *attr, 
                                           soc_sbx_caladan3_chain_table_attr_t *chain_attr)
{
    return _soc_sbx_caladan3_tmu_table_alloc(unit, attr, chain_attr);
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_free
 * Purpose:
 *     Allocate a table
 */
int soc_sbx_caladan3_tmu_table_free(int unit, int tableid)
{
    int status = SOC_E_NONE, index;
    soc_sbx_caladan3_table_config_t *table_cfg;
    soc_sbx_caladan3_table_attr_t *table_attr;

    if (tableid < 0 || tableid >= SOC_SBX_CALADAN3_TMU_MAX_TABLE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table Id [%d] Invalid !!!\n"), 
                   FUNCTION_NAME(), unit, tableid));
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);    


    TMU_LOCK(unit);

    table_cfg = &_tmu_dbase[unit]->table_cfg;
    if (SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, tableid)) {
        /* clear all table hardware configuration */
        table_attr = &table_cfg->table_attr[tableid];

        SOC_SBX_C3_BMP_REMOVE(table_cfg->alloc_bmap, tableid);

        for (index=table_attr->row_offset; index < table_attr->row_offset + table_attr->num_row_per_region; index++) {
            table_cfg->col_used_bits[index] -= (table_attr->num_entries_per_row * table_attr->entry_size_bits);
            if (table_cfg->col_used_bits[index] == 0) {
                /* only remove when the row is all empty */
                SOC_SBX_C3_BMP_REMOVE(table_cfg->row_bmap, index);
            }
        }

        sal_memset(table_attr, 0, sizeof(soc_sbx_caladan3_table_attr_t));
	table_attr->id = tableid;

        status = _soc_sbx_caladan3_tmu_table_hw_set(unit, table_attr, NULL);

    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table Id [%d] Not Allocated"
                              " trying to free non allocated table !!!\n"), 
                   FUNCTION_NAME(), unit, tableid));
        status = SOC_E_PARAM;
    }

    TMU_UNLOCK(unit);

    return status;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_map_dump
 * Purpose:
 *     Dump TMU table allocation
 */
void soc_sbx_caladan3_tmu_table_map_dump(int unit, int min, int max)
{
    soc_sbx_caladan3_table_config_t *table_cfg;
    soc_sbx_caladan3_table_attr_t *table_attr;
    int index, curr_row, curr_col;
    soc_sbx_caladan3_region_config_t *region_cfg;
    soc_sbx_caladan3_dram_config_t *dram_cfg;

    if (min < 0 || max < 0) {
        min = 0;
        max = SOC_SBX_CALADAN3_TMU_MAX_TABLE;
    }

    /* initialize sws database */
    if (_tmu_dbase[unit] == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "!!! TMU uninitialized !!!\n")));
    } else {
        table_cfg = &_tmu_dbase[unit]->table_cfg;
	region_cfg = &_tmu_dbase[unit]->region_cfg;
        dram_cfg = &_tmu_dbase[unit]->dram_cfg;
        LOG_CLI((BSL_META_U(unit,
                            "\n================================")));
        LOG_CLI((BSL_META_U(unit,
                            "\n TMU Table Map Dump ")));
        SOC_SBX_C3_BMP_COUNT(table_cfg->alloc_bmap, index, SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS);
        LOG_CLI((BSL_META_U(unit,
                            "\n Number of Table Allocated: %d"), index)); 
        LOG_CLI((BSL_META_U(unit,
                            "\n================================")));
     
        for (index=min; index < max; index++) {
            if (SOC_SBX_C3_BMP_MEMBER(table_cfg->alloc_bmap, index)) {
                /* dump allocated table */
                table_attr = &table_cfg->table_attr[index];
                LOG_CLI((BSL_META_U(unit,
                                    "\n# Table ID: %d #"), index));
                LOG_CLI((BSL_META_U(unit,
                                    "\n Num Entries:[%d] Entry Size bits:[%d] Replication:%d"), 
                         table_attr->num_entries, table_attr->entry_size_bits,
                         table_attr->replication_factor));
                LOG_CLI((BSL_META_U(unit,
                                    "\n Row/Region:[%d] Entry/row[%d] Offset:[reg-%d:row-%d:col-%d]"), 
                         table_attr->num_row_per_region,  table_attr->num_entries_per_row,
                         table_attr->region_offset,
                         table_attr->row_offset, table_attr->column_offset));
            } else {
                /*LOG_CLI((BSL_META_U(unit,
                                      "\n-- Table ID: %d - not allocated --"), index));*/
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));
        LOG_CLI((BSL_META_U(unit,
                            "\n Row Allocation Map Dump ")));
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));
        LOG_CLI((BSL_META_U(unit,
                            "\n Rows Range 0-%d \n Unused marked as - , Used marked as * "), region_cfg->rows_per_region_per_bank-1));
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));
        LOG_CLI((BSL_META_U(unit,
                            " Allocated Rows:  00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15\n")));
        for (curr_row = 0; curr_row < region_cfg->rows_per_region_per_bank; curr_row++) {
            if ((curr_row % 0x10) == 0) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n      %2d         "), curr_row/0x10));
            }

            if (SOC_SBX_C3_BMP_MEMBER(table_cfg->row_bmap, curr_row)) {
                LOG_CLI((BSL_META_U(unit,
                                    "  *  ")));
            } else {
                LOG_CLI((BSL_META_U(unit,
                                    "  -  ")));
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));

        LOG_CLI((BSL_META_U(unit,
                            " Row util 1/1000: 00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15\n")));
        for (curr_row = 0; curr_row < region_cfg->rows_per_region_per_bank; curr_row++) {
            if ((curr_row % 0x10) == 0) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n      %2d         "), curr_row/0x10));
            }

            LOG_CLI((BSL_META_U(unit,
                                " %3d "), (table_cfg->col_used_bits[curr_row] * 100)/(dram_cfg->dram_row_size_bytes * 8)));
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));

        LOG_CLI((BSL_META_U(unit,
                            " Row usage bitmap, Most significant bit indicates col 0, total %d columns \n"), 
                 ((dram_cfg->dram_row_size_bytes * 8) / SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS)));

        for (curr_row = 0; curr_row < region_cfg->rows_per_region_per_bank; curr_row++) {
            LOG_CLI((BSL_META_U(unit,
                                "\n row %3d 0b"), curr_row));
            for (curr_col = 0;
                 curr_col < ((dram_cfg->dram_row_size_bytes * 8) / SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS) - 1;
                 curr_col++) {
                if (SOC_SBX_C3_BMP_MEMBER(table_cfg->col_bmap[curr_row], curr_col)) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%1d"),1));
                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "%1d"),0));
                }
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n================================\n")));
    }
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_program_alloc
 * Purpose:
 *     Allocate a TMU program
 */
int soc_sbx_caladan3_tmu_program_alloc(int unit, soc_sbx_caladan3_tmu_program_info_t *info)
{
    int status = SOC_E_NONE, index/*, keyindex*/;
    soc_sbx_caladan3_tmu_program_config_t *program_cfg;
    uint32 regval, field=0;
    uint32 sub_key_reg[] = {TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, TMA_KEYPLODER_SUBKEY1_SHIFT_CONFIGr};
    uint32 lookup_reg[] = {TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, TMA_KEYPLODER_SUBKEY1_LOOKUP_CONFIGr};

    if (!info) {
        return SOC_E_PARAM;
    }

    if (info->flag > 0) {
        if (info->flag != SOC_SBX_TMU_PRG_FLAG_WITH_ID) {
            return SOC_E_PARAM;
        }
        if (info->program_num < 0 || info->program_num >= SOC_SBX_CALADAN3_TMU_MAX_PROGRAM) {
            return SOC_E_PARAM;
        }
    }

    if (info->key_info[0].valid <= 0 && info->key_info[1].valid <= 0) {
        return SOC_E_PARAM;
    }

    /* Sanity check following rules:
     *  NOTE: EML144 mode check is done at table alloc time,
     *        since EML144 and EML176 lookup value is defined to be
     *        same by hardware. Table parameters for EML144 and EML176 are
     *        defined by software and different.
     * on A0/A1:  
     *    (1) If one of subkey is EML176, EML304, EML424, 
     *        the other subkey has to be invalid
     *    (2) Doesn't support EML144
     *    (3) EML Insert/Delete has to be on subkey 0 and
     *        the other subkey has to be invalid
     * on B0/B1:
     *    (1) If one of subkey is EML176, EML304, EML424, 
     *        the other subkey has to be invalid
     *    (2) If EML176 is ever used, EML144 can not be used
     *        IF EML144 is ever used, EML176 can not be used
     *    (3) EML Insert/Delete has to be on subkey 0 and
     *        the other subkey has to be invalid
     */
    for (index=0; index < SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS; index++) {
        if (info->key_info[index].valid == TRUE) {
            if ((info->key_info[index].lookup == SOC_SBX_TMU_LKUP_EML_INSERT_DELETE) ||
                ((info->key_info[index].lookup == SOC_SBX_TMU_LKUP_EML_176) &&
                 (_tmu_dbase[unit]->control_cfg.eml_144_mode != SOC_SBX_TMU_EML_144BITS_MODE_ON)) ||
                (info->key_info[index].lookup == SOC_SBX_TMU_LKUP_EML_304) ||
                (info->key_info[index].lookup == SOC_SBX_TMU_LKUP_EML_424)) {
                if (info->key_info[SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS-index-1].valid == TRUE) {
                    /* only 1 subkey could be valid */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Lookup [%d] type can only support 1 valid subkey !!!\n"),
                               FUNCTION_NAME(), unit, info->key_info[index].lookup));
                    return SOC_E_PARAM;
                }

                if ((info->key_info[index].lookup == SOC_SBX_TMU_LKUP_EML_INSERT_DELETE) &&
                    (index != 0)) {
                    /* insert_delete lookup has to be on subkey 0 */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d EML_INSERT_DELETE Lookup [%d] has to use subkey 0 !!!\n"),
                               FUNCTION_NAME(), unit, info->key_info[index].lookup));
                    return SOC_E_PARAM;
                }
            }
        }
    }
    
    /* dump program info */
#if 0
    LOG_CLI((BSL_META_U(unit,
                        "## TMU program number: %d flags: 0x%x \n"), 
             info->program_num, info->flag));
    for (index=0; index < SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS; index++) {
        LOG_CLI((BSL_META_U(unit,
                            "# KEY Info: \n")));
        LOG_CLI((BSL_META_U(unit,
                            "Lookup: %d table_id: %d valid: %d taps_seg: %d \n"),
                 info->key_info[index].lookup,
                 info->key_info[index].tableid,
                 info->key_info[index].valid,
                 info->key_info[index].taps_seg));
    }
#endif

#if 1
    for (index=0; index < SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS; index++) {
        int keyindex;
        for (keyindex=0; keyindex < SOC_SBX_CALADAN3_TMU_MAX_KEY && info->key_info[index].valid > 0; keyindex++) {
            if (info->key_info[index].shift[keyindex] > SOC_SBX_CALADAN3_TMU_MAX_KEY_BYTE_SHIFT || 
                info->key_info[index].bytes_to_mask[keyindex] > SOC_SBX_CALADAN3_TMU_MAX_KEY_BYTE_SHIFT) {
                     LOG_CLI((BSL_META_U(unit,
                                         "soc_sbx_caladan3_tmu_program_alloc: Clearing uninitialized parameters: \n")));
                     LOG_CLI((BSL_META_U(unit,
                                         " ... Lookup: %d table_id: %d valid: %d taps_seg: %d - index: %d shift: %d bytes_to_mask: %d \n"),
                              info->key_info[index].lookup,
                              info->key_info[index].tableid,
                              info->key_info[index].valid,
                              info->key_info[index].taps_seg,
                              index,
                              info->key_info[index].shift[keyindex],
                              info->key_info[index].bytes_to_mask[keyindex]));
                info->key_info[index].shift[keyindex]=0; info->key_info[index].bytes_to_mask[keyindex]=0;
                /* return SOC_E_PARAM; */
            }
        }
    }
#endif

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    TMU_LOCK(unit);    

    program_cfg = &_tmu_dbase[unit]->program_cfg;

    /* with id allocation */
    if (info->flag) {
        if (SOC_SBX_C3_BMP_MEMBER(program_cfg->alloc_bmap, info->program_num)) {
            status = SOC_E_MEMORY;
        } 
    } else {
        for (index=0; index < SOC_SBX_CALADAN3_TMU_MAX_PROGRAM; index++) {
            if (!SOC_SBX_C3_BMP_MEMBER(program_cfg->alloc_bmap,index)) {
                info->program_num = index;
                break;
            }
        }
    }

    if (SOC_SUCCESS(status)) {
        for (index=0; index < SOC_SBX_CALADAN3_TMU_MAX_KEY; index++) {
            if (info->key_info[index].valid > 0) {
               status = soc_reg32_get(unit, sub_key_reg[index], REG_PORT_ANY, info->program_num, &regval);
                if (SOC_SUCCESS(status)) {
                    soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &regval, 
                                      KEY_MASK_S0f, info->key_info[index].bytes_to_mask[0]);
                    soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &regval, 
                                      KEY_SHIFT_S0f, info->key_info[index].shift[0]);
                    soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &regval, 
                                      KEY_MASK_S1f, info->key_info[index].bytes_to_mask[1]);
                    soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr, &regval, 
                                      KEY_SHIFT_S1f, info->key_info[index].shift[1]);
                    soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr,
                                      &regval, SUB_KEY_SHIFT_S0f, info->key_shift[index]);

                    status = soc_reg32_set(unit, sub_key_reg[index], REG_PORT_ANY,
                                           info->program_num, regval);
                }

                if (SOC_SUCCESS(status)) {
                       status = soc_reg32_get(unit, lookup_reg[index], REG_PORT_ANY, info->program_num, &regval);
                    if (SOC_SUCCESS(status)) {
                        soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &regval, 
                                          LOOKUPf, info->key_info[index].lookup);

                        soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &regval, 
                                          TABLE0f, info->key_info[index].tableid);

                        if (SOC_SBX_TMU_IS_TAPS_LOOKUP(info->key_info[index].lookup)) {
                            field = info->key_info[index].taps_seg;
                        } else {
                            field = 0;
                        }

                        soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &regval, 
                                          TAPS_SEGf, field);

                        status = soc_reg32_set(unit, lookup_reg[index], REG_PORT_ANY,
                                               info->program_num, regval);
                    }
                }
            } else {
                regval = 0;
                soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &regval, 
                                  LOOKUPf,  SOC_SBX_TMU_LKUP_NONE); /* do nothing */
                status = soc_reg32_set(unit, lookup_reg[index], REG_PORT_ANY,
                                           info->program_num, regval);
            }
        }
    }

    if (SOC_SUCCESS(status)) {
        SOC_SBX_C3_BMP_ADD(program_cfg->alloc_bmap, info->program_num);
    }

    TMU_UNLOCK(unit);  
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_program_free
 * Purpose:
 *     Free a TMU program
 */
int soc_sbx_caladan3_tmu_program_free(int unit, int program_num)
{
    int status = SOC_E_NONE, index;
    soc_sbx_caladan3_tmu_program_config_t *program_cfg;
    soc_sbx_caladan3_tmu_program_info_t *info;
    uint32 regval;

    if (program_num < 0 || program_num >= SOC_SBX_CALADAN3_TMU_MAX_PROGRAM) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    TMU_LOCK(unit);    

    program_cfg = &_tmu_dbase[unit]->program_cfg;

    /* with id allocation */
    if (SOC_SBX_C3_BMP_MEMBER(program_cfg->alloc_bmap, program_num)) {

        info = &program_cfg->info[program_num];

        for (index=0; index < SOC_SBX_CALADAN3_TMU_MAX_KEY; index++) {

            regval = 0; /* reset value */

            status = WRITE_TMA_KEYPLODER_SUBKEY0_SHIFT_CONFIGr(unit, program_num, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Program [%d] Key configuration Failed !!!\n"),
                           FUNCTION_NAME(), unit, program_num));
            }

            regval = 0;
            soc_reg_field_set(unit, TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr, &regval, 
                              LOOKUPf, SOC_SBX_TMU_LKUP_NONE);
            status = WRITE_TMA_KEYPLODER_SUBKEY0_LOOKUP_CONFIGr(unit, info->program_num, regval);

            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Program [%d] Lookup configuration Failed !!!\n"),
                           FUNCTION_NAME(), unit, program_num));
            }
        }
        
        SOC_SBX_C3_BMP_REMOVE(program_cfg->alloc_bmap, program_num);

    } else {
        /* program not allocated */
        status = SOC_E_PARAM;
    }

    TMU_UNLOCK(unit);  
    return status;
}

#if 0
/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_table_clear
 * Purpose:
 *     clear number of entries on given table
 */
int soc_sbx_caladan3_tmu_table_clear(int unit, int tableid, uint32 num_entries)
{
    int status = SOC_E_NONE, index, cmd_idx;
#define _XL_MAX_SIZE_ (16 * 64)
#define _MAX_CMDS_ (2)
#define _MAX_WORDS_ (_MAX_CMDS_ * (_XL_MAX_SIZE_/2))
    soc_sbx_caladan3_tmu_cmd_t *cmd[_MAX_CMDS_];    
    soc_sbx_caladan3_table_attr_t *attr=NULL;
    uint32 value[_MAX_WORDS_], *data=NULL, num_cmds=0;

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    TMU_LOCK(unit);    

    /* Currently only Hash root table supported  */
    if (SOC_SBX_C3_BMP_MEMBER(_tmu_dbase[unit]->table_cfg.alloc_bmap, tableid)) {
        attr = &_tmu_dbase[unit]->table_cfg.table_attr[tableid];
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s : unit %d table not initialized \n"), 
                   FUNCTION_NAME(), unit));
        status = SOC_E_PARAM;
    }

    sal_memset(cmd, 0, sizeof(soc_sbx_caladan3_table_attr_t*) * _MAX_CMDS_);

    if (SOC_SUCCESS(status)) {

        sal_memset(&value, 0, COUNTOF(value) * sizeof(uint32));

        switch (attr->lookup) {
        case SOC_SBX_TMU_LKUP_EML_64:
        case SOC_SBX_TMU_LKUP_EML_176:
        case SOC_SBX_TMU_LKUP_EML_304:
        case SOC_SBX_TMU_LKUP_EML_424:
            value[0] = 0xF8000000; /* nl_gt = 0xf */
            value[1] = 0x00000007; /* nl_le = 0xf */
            break;
        case SOC_SBX_TMU_LKUP_EML2ND_64:
        case SOC_SBX_TMU_LKUP_EML2ND_176:
        case SOC_SBX_TMU_LKUP_EML2ND_304:
        case SOC_SBX_TMU_LKUP_EML2ND_424:
            /* use all 0's */
            break;
        default:
            status = SOC_E_PARAM;
            break;
        }

        status = tmu_cmd_alloc(unit, &cmd[0]);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate command %d!!!\n"), 
                       FUNCTION_NAME(), unit, status));
        } else {
            cmd[0]->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
            cmd[0]->cmd.xlwrite.table = tableid;
            cmd[0]->cmd.xlwrite.offset = 0;
            cmd[0]->cmd.xlwrite.size = attr->entry_size_bits / SOC_SBX_TMU_CMD_WORD_SIZE;
            cmd[0]->cmd.xlwrite.value = &value[0];
            cmd[0]->cmd.xlwrite.value_size = attr->entry_size_bits;
            cmd[0]->cmd.xlwrite.lookup = attr->lookup;
            num_cmds++;
        }

        /* maximum size allowed on xl write is 16words or 1k */
        /* break to sub commands if required */
        if (attr->entry_size_bits > _XL_MAX_SIZE_ * _MAX_CMDS_) {
            status = SOC_E_LIMIT;
        } else if (attr->entry_size_bits > _XL_MAX_SIZE_) {

            cmd[0]->cmd.xlwrite.value_size = _XL_MAX_SIZE_;
            cmd[0]->cmd.xlwrite.size = cmd[0]->cmd.xlwrite.value_size/SOC_SBX_TMU_CMD_WORD_SIZE;

            for (index=1; (index <  (attr->entry_size_bits / _XL_MAX_SIZE_) + \
                           ((attr->entry_size_bits % _XL_MAX_SIZE_)?1:0)) && 
                     SOC_SUCCESS(status); index++) {
                status = tmu_cmd_alloc(unit, &cmd[index]);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to allocate command %d!!!\n"), 
                               FUNCTION_NAME(), unit, status));
                } else {
                    sal_memcpy(cmd[index], cmd[0], sizeof(soc_sbx_caladan3_tmu_cmd_t));

                    cmd[index]->cmd.xlwrite.offset = (num_cmds * _XL_MAX_SIZE_)/SOC_SBX_TMU_CMD_WORD_SIZE;
                    cmd[index]->cmd.xlwrite.value_size = (attr->entry_size_bits - (num_cmds * _XL_MAX_SIZE_));
                    if (cmd[index]->cmd.xlwrite.value_size > _XL_MAX_SIZE_) {
                        cmd[index]->cmd.xlwrite.value_size = _XL_MAX_SIZE_;
                    }
                    cmd[index]->cmd.xlwrite.size = cmd[index]->cmd.xlwrite.value_size/SOC_SBX_TMU_CMD_WORD_SIZE;
#if 0
                    cmd[index]->cmd.xlwrite.value = &value[cmd[index-1]->cmd.xlwrite.value_size/32];
#else
                    cmd[index]->cmd.xlwrite.value = &value[0];
#endif
                    num_cmds++;
                }
            }
        }
        
        for (index=0; index < num_entries && SOC_SUCCESS(status); index++) {
            for (cmd_idx=0; cmd_idx < num_cmds; cmd_idx++) {
                cmd[cmd_idx]->cmd.xlwrite.entry_num = index;
                status = soc_sbx_caladan3_tmu_post_cmd(unit, 
                                                       SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                       cmd[cmd_idx],
                                                       SOC_SBX_TMU_CMD_POST_FLAG_NONE);
                if (SOC_SUCCESS(status)) {
                    status = soc_sbx_caladan3_tmu_get_resp(unit,
                                                           SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                           cmd[cmd_idx],
                                                           data, 0);
                    if (SOC_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Invalid response !!!\n"), 
                                   FUNCTION_NAME(), unit));
                    }
                }
            }
        }

        /* free the command to pool */
        if (num_cmds) {
            for (cmd_idx=0; cmd_idx < num_cmds; cmd_idx++) {
                tmu_cmd_free(unit, cmd[cmd_idx]);
            }
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s:%d unit Cleared Table:(%d) Num-Entries(%d)\n"), 
                     FUNCTION_NAME(), unit, tableid, num_entries));
    }

    TMU_UNLOCK(unit);  
    return status;
}
#else
int soc_sbx_caladan3_tmu_table_clear(int unit, int tableid, uint32 num_entries)
{
    int status = SOC_E_NONE, index;
#define _XL_MAX_SIZE_ (16 * 64)
#define _MAX_CMDS_ (2)
#define _MAX_WORDS_ (_MAX_CMDS_ * (_XL_MAX_SIZE_/2))
    soc_sbx_caladan3_tmu_cmd_t *cmd;    
    soc_sbx_caladan3_table_attr_t *attr=NULL;
    uint32 value[_MAX_WORDS_], *data=NULL;

    SOC_IF_TMU_UNINIT_RETURN(unit);    
    TMU_LOCK(unit);    

    /* Currently only Hash root table supported  */
    if (SOC_SBX_C3_BMP_MEMBER(_tmu_dbase[unit]->table_cfg.alloc_bmap, tableid)) {
        attr = &_tmu_dbase[unit]->table_cfg.table_attr[tableid];
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s : unit %d table not initialized \n"), 
                   FUNCTION_NAME(), unit));
        status = SOC_E_PARAM;
    }

    if (SOC_SUCCESS(status)) {

        sal_memset(&value, 0, COUNTOF(value) * sizeof(uint32));

        switch (attr->lookup) {
        case SOC_SBX_TMU_LKUP_EML_64:
        case SOC_SBX_TMU_LKUP_EML_176:
        case SOC_SBX_TMU_LKUP_EML_304:
        case SOC_SBX_TMU_LKUP_EML_424:
            value[0] = 0xF8000000; /* nl_gt = 0xf */
            value[1] = 0x00000007; /* nl_le = 0xf */
            break;
        case SOC_SBX_TMU_LKUP_EML2ND_64:
        case SOC_SBX_TMU_LKUP_EML2ND_176:
        case SOC_SBX_TMU_LKUP_EML2ND_304:
        case SOC_SBX_TMU_LKUP_EML2ND_424:
            /* use all 0's */
            break;
        default:
            status = SOC_E_PARAM;
            break;
        }
        if (SOC_SUCCESS(status)) {
        status = tmu_cmd_alloc(unit, &cmd);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate command %d!!!\n"), 
                       FUNCTION_NAME(), unit, status));
        } else {
            cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
            cmd->cmd.xlwrite.table = tableid;
            cmd->cmd.xlwrite.offset = 0;
            cmd->cmd.xlwrite.size = attr->entry_size_bits / SOC_SBX_TMU_CMD_WORD_SIZE;
            cmd->cmd.xlwrite.value_size = attr->entry_size_bits;
            cmd->cmd.xlwrite.lookup = attr->lookup;
	    for (index=0; index < BITS2WORDS(cmd->cmd.xlwrite.value_size); index++) {
		cmd->cmd.xlwrite.value_data[index] = value[index];
	    }		
        }

        /* maximum size allowed on xl write is 16words or 1k */
        /* break to sub commands if required */
        if (attr->entry_size_bits > _XL_MAX_SIZE_) {
            cmd->cmd.xlwrite.value_size = _XL_MAX_SIZE_;
            cmd->cmd.xlwrite.size = cmd->cmd.xlwrite.value_size/SOC_SBX_TMU_CMD_WORD_SIZE;
        }
        
        for (index=0; index < num_entries && SOC_SUCCESS(status); index++) {
            cmd->cmd.xlwrite.entry_num = index;
            TMU_LOCK(unit);
            status = soc_sbx_caladan3_tmu_post_cmd(unit, 
                                                   SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd,
                                                   SOC_SBX_TMU_CMD_POST_FLAG_NONE);
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_get_resp(unit,
                                                       SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                       cmd,
                                                       data, 0);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Invalid response !!!\n"), 
                               FUNCTION_NAME(), unit));
                }
            }
            TMU_UNLOCK(unit);
        }
    
        /* free the command to pool */
        if (cmd) {
            tmu_cmd_free(unit, cmd);
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s:%d unit Cleared Table:(%d) Num-Entries(%d)\n"), 
                     FUNCTION_NAME(), unit, tableid, num_entries));
        }
    }

    TMU_UNLOCK(unit);  
    return status;
}
#endif

/******** ISR ************/
#define TMU_INT_FIELD_GET(_reg,_regval,_field) do {             \
  if(0 == field) {                                              \
      (field) = soc_reg_field_get(unit, _reg, _regval, _field); \
  }                                                             \
} while(0); 

/*
 *   Function
 *     soc_sbx_caladan3_tmu_isr
 *   Purpose
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) d1-4   : not used
 *   Returns
 *      VOID
 */
void soc_sbx_caladan3_tmu_isr(void *unit_vp,
                              void *d1, void *d2,
                              void *d3, void *d4)
{
    int unit = PTR_TO_INT(unit_vp);
    uint32 regval=0, field=0;
    int status = SOC_E_NONE, index;
    int rv = SOC_E_NONE, other_task_handle = FALSE, handled;

    typedef struct tmu_intr_info_s {
        int reg;
        int mask;
    } tmu_intr_info_t; 

#define TMU_INTR_REG(info) info.reg
#define TMU_INTR_MASK(info) info.mask

    /* interrupts handled */
    tmu_intr_info_t tmu_interrupt[] = {
        /* TMB */
        {TMB_UPDATER_FIFO_PUSH_STATUSr, TMB_UPDATER_FIFO_PUSH_STATUS_MASKr}
    };

    /* TMU interrupts */
    tmu_intr_info_t  tmu_interrupt_unhandled[] = {
        /* ---- TMB INT ----*/
        {TMB_DISTRIBUTOR_SCHEDULER_ERRORr, TMB_DISTRIBUTOR_SCHEDULER_ERROR_MASKr},
        {TMB_DISTRIBUTOR_REFRESH_ERRORr, TMB_DISTRIBUTOR_REFRESH_ERROR_MASKr},
        {TMB_DISTRIBUTOR_INTERFACE_FIFO_ERRORr, TMB_DISTRIBUTOR_INTERFACE_FIFO_ERROR_MASKr},
        {TMB_DISTRIBUTOR_FUNNEL_FIFO_ERRORr, TMB_DISTRIBUTOR_FUNNEL_FIFO_ERROR_MASKr},
        {TMB_DISTRIBUTOR_ECC_ERRORr, TMB_DISTRIBUTOR_ECC_ERROR_MASKr},
        {TMB_UPDATER_FIFO_POP_STATUSr, TMB_UPDATER_FIFO_POP_STATUS_MASKr},
        {TMB_UPDATER_BULK_DELETE_EVENTr, TMB_UPDATER_BULK_DELETE_EVENT_MASKr},
        {TMB_UPDATER_ERRORr, TMB_UPDATER_ERROR_MASKr},
        {TMB_HASH_ECC_ERRORr, TMB_HASH_ECC_ERROR_MASKr},
        {TMB_KEYBUFFER_ECC_ERRORr, TMB_KEYBUFFER_ECC_ERROR_MASKr},
        {TMB_UPDATER_ECC_ERRORr,TMB_UPDATER_ECC_ERROR_MASKr},
        {TMB_COMPLETION_ECC_ERRORr, TMB_COMPLETION_ECC_ERROR_MASKr},
        {TMB_COMPLETION_ERROR0r,TMB_COMPLETION_ERROR0_MASKr}, 
        {TMB_COMPLETION_ERROR1r,TMB_COMPLETION_ERROR1_MASKr},
        {TMB_COMPLETION_PARITY_ERRORr, TMB_COMPLETION_PARITY_ERROR_MASKr},
        {TMB_COMPLETION_TRACE_STATUSr, TMB_COMPLETION_TRACE_STATUS_MASKr},
         /* ---- TM INT ----*/
        {TM_QE_ERRORr, TM_QE_ERROR_MASKr},
        {TM_QE_FIFO_OVERFLOW_ERRORr,TM_QE_FIFO_OVERFLOW_ERROR_MASKr},
        {TM_QE_FIFO_UNDERFLOW_ERRORr, TM_QE_FIFO_UNDERFLOW_ERROR_MASKr},
        {TM_QE_FIFO_PARITY_ERRORr,TM_QE_FIFO_PARITY_ERROR_MASKr},
        {TM_QE_TRACE_STATUSr, TM_QE_TRACE_STATUS_MASKr},
         /* ---- TMA INT ----*/
        {TMA_KEYPLODER_ERRORr, TMA_KEYPLODER_ERROR_MASKr},
        {TMA_HASH_ECC_ERROR0r, TMA_HASH_ECC_ERROR0_MASKr},
        {TMA_HASH_ECC_ERROR1r, TMA_HASH_ECC_ERROR1_MASKr},
        {TMA_DIRECT_ERROR0r, TMA_DIRECT_ERROR0_MASKr},
        {TMA_DIRECT_ERROR1r, TMA_DIRECT_ERROR1_MASKr},
        {TMA_DIRECT_ERROR2r, TMA_DIRECT_ERROR2_MASKr},
        {TMA_DIRECT_ERROR3r, TMA_DIRECT_ERROR3_MASKr},
        {TMA_TRACE_STATUSr, TMA_TRACE_STATUS_MASKr},
        {TMA_PM_ECC_ERRORr, TMA_PM_ECC_ERROR_MASKr}};
    
    
    handled = FALSE;
    for (index=0; index < COUNTOF(tmu_interrupt); index++) {
        status = soc_reg32_get(unit, TMU_INTR_REG(tmu_interrupt[index]), REG_PORT_ANY, 0, &regval);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unit %d: failed(%d) to read interrupt register: 0x%x\n"),
                       FUNCTION_NAME(), unit, status, TMU_INTR_REG(tmu_interrupt[index])));
        } else if (regval > 0) {
            field = 0;
            switch (TMU_INTR_REG(tmu_interrupt[index])) {
            case TMB_UPDATER_FIFO_PUSH_STATUSr:
                {
                    TMU_INT_FIELD_GET(TMB_UPDATER_FIFO_PUSH_STATUSr, regval, FREE_CHAIN_FIFO0_AEMPTYf);
                    TMU_INT_FIELD_GET(TMB_UPDATER_FIFO_PUSH_STATUSr, regval, FREE_CHAIN_FIFO1_AEMPTYf);
                    TMU_INT_FIELD_GET(TMB_UPDATER_FIFO_PUSH_STATUSr, regval, FREE_CHAIN_FIFO2_AEMPTYf);
                    TMU_INT_FIELD_GET(TMB_UPDATER_FIFO_PUSH_STATUSr, regval, FREE_CHAIN_FIFO3_AEMPTYf); 

                    if (field) { /* handler will clear interrupt status */
                        rv = soc_sbx_caladan3_tmu_hash_fifo_feed_trigger(unit);
                        if (SOC_SUCCESS(rv)) {
                            other_task_handle = TRUE;
                        }
                    } else {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s unit %d: unsupported interrupt 0x%8x\n"),
                                   FUNCTION_NAME(), unit, regval));
                        /* clear the status register */
                        soc_reg32_set(unit, TMU_INTR_REG(tmu_interrupt[index]), REG_PORT_ANY, 0, regval);
                        /* unmask this unexpected interrupt */
                        regval = SOC_REG_INFO(unit, TMU_INTR_MASK(tmu_interrupt[index])).rst_val_lo;
                        soc_reg32_set(unit, TMU_INTR_MASK(tmu_interrupt[index]), REG_PORT_ANY, 0, regval);
                    }
                }
                break;
            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: unsupported interrupt 0x%8x\n"),
                           FUNCTION_NAME(), unit, regval));
                /* clear the status register */
                soc_reg32_set(unit, TMU_INTR_REG(tmu_interrupt[index]), REG_PORT_ANY, 0, regval);
                /* unmask this unexpected interrupt */
                regval = SOC_REG_INFO(unit, TMU_INTR_MASK(tmu_interrupt[index])).rst_val_lo;
                soc_reg32_set(unit, TMU_INTR_MASK(tmu_interrupt[index]), REG_PORT_ANY, 0, regval);
                break;
            }
            regval = 0;
            handled = TRUE;
        }
    }

    /* try unhandled interrupts, mask the interrupt source */
    if (!(other_task_handle || handled)) {
        for (index=0; index < COUNTOF(tmu_interrupt_unhandled); index++) {
            status = soc_reg32_get(unit, TMU_INTR_REG(tmu_interrupt_unhandled[index]), REG_PORT_ANY, 0, &regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to read interrupt register: 0x%x\n"),
                           FUNCTION_NAME(), unit, status, TMU_INTR_REG(tmu_interrupt_unhandled[index])));
            } else if (regval > 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: unhandled TMU interrupt 0x%8x, masking off\n"),
                           FUNCTION_NAME(), unit, regval));
                /* clear the status register */
                soc_reg32_set(unit, TMU_INTR_REG(tmu_interrupt_unhandled[index]), REG_PORT_ANY, 0, regval);

                /* mask this unexpected interrupt */
                regval = SOC_REG_INFO(unit, TMU_INTR_MASK(tmu_interrupt_unhandled[index])).rst_val_lo;
                soc_reg32_set(unit, TMU_INTR_MASK(tmu_interrupt_unhandled[index]), REG_PORT_ANY, 0, regval);

                handled = TRUE;
            }
        }
    }

    if (!other_task_handle && handled) {
        /* un-mask the block interrupt if it is not */
        soc_cmicm_intr3_enable(unit, 1<<SOC_SBX_CALADAN3_TMB_INTR_POS);
    }
}

/*
 *   Function
 *     soc_sbx_caladan3_tmu_qe_isr
 *   Purpose
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) d1-4   : not used
 *   Returns
 *      VOID
 */
void soc_sbx_caladan3_tmu_qe_isr(void *unit_vp,
                              void *intr_val, void *d2,
                              void *d3, void *d4)
{
    int unit = PTR_TO_INT(unit_vp);
    int qe_inst = SOC_SBX_CALADAN3_QE0_INTR_POS - PTR_TO_INT(intr_val);
    uint32 table_num = 0;
    uint32 entry_num = 0;
    uint32 regval=0, field=0;
    uint32 key[5];
    int status = SOC_E_NONE, index;
    
    typedef struct tmu_intr_info_s {
        int reg;
        int mask;
    } tmu_intr_info_t; 

#define TMU_INTR_REG(info) info.reg
#define TMU_INTR_MASK(info) info.mask

    /* interrupts handled */
    tmu_intr_info_t tmu_qe_interrupt[] = {
        /* TMB */
        {TM_QE_ERRORr, TM_QE_ERROR_MASKr}
    };

    /* TMU interrupts */
    for (index=0; index < COUNTOF(tmu_qe_interrupt); index++) {
        status = soc_reg32_get(unit, TMU_INTR_REG(tmu_qe_interrupt[index]), SOC_BLOCK_PORT(unit, qe_inst), 0, &regval);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unit %d: failed(%d) to read interrupt register: 0x%x\n"),
                       FUNCTION_NAME(), unit, status, TMU_INTR_REG(tmu_qe_interrupt[index])));
        } else if (regval > 0) {
            field = 0;
            switch (TMU_INTR_REG(tmu_qe_interrupt[index])) {
            case TM_QE_ERRORr:
                {   
                    /* Only support v6_collision interrupt */
                    TMU_INT_FIELD_GET(TM_QE_ERRORr, regval, V6_COLLISIONf);

                    if (field) { /* handler will clear interrupt status */
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_KEY_31_0r, SOC_BLOCK_PORT(unit, qe_inst), 0, &key[4]);
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_KEY_63_32r, SOC_BLOCK_PORT(unit, qe_inst), 0, &key[3]);
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_KEY_95_64r, SOC_BLOCK_PORT(unit, qe_inst), 0, &key[2]);
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_KEY_126_96r, SOC_BLOCK_PORT(unit, qe_inst), 0, &key[1]);
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_TABr, SOC_BLOCK_PORT(unit, qe_inst), 0, &table_num);  
                        soc_reg32_get(unit, TM_QE_V6_COLLISION_ENTRY_NUMr, SOC_BLOCK_PORT(unit, qe_inst), 0, &entry_num);
                        taps_v6_collision_isr(unit, table_num, entry_num, key);
                    } else {
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "%s unit %d: unsupported interrupt 0x%8x in TM_QE_ERROR\n"),
                                     FUNCTION_NAME(), unit, regval));
                        /* unmask this unexpected interrupt */
                        regval = SOC_REG_INFO(unit, TMU_INTR_MASK(tmu_qe_interrupt[index])).rst_val_lo;
                        soc_reg32_set(unit, TMU_INTR_MASK(tmu_qe_interrupt[index]), SOC_BLOCK_PORT(unit, qe_inst), 0, regval);
                    }
                    /* clear the status register */
                    regval = 0xFFFFFFFF;
                    soc_reg32_set(unit, TMU_INTR_REG(tmu_qe_interrupt[index]), SOC_BLOCK_PORT(unit, qe_inst), 0, regval);
                    soc_cmicm_intr4_enable(unit, 1 << ((PTR_TO_INT(intr_val)) % 32));
                }
                break;
            default:
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "%s unit %d: unsupported interrupt 0x%8x\n"),
                             FUNCTION_NAME(), unit, regval));
                /* clear the status register */
                soc_reg32_set(unit, TMU_INTR_REG(tmu_qe_interrupt[index]), SOC_BLOCK_PORT(unit, qe_inst), 0, regval);
                /* unmask this unexpected interrupt */
                regval = SOC_REG_INFO(unit, TMU_INTR_MASK(tmu_qe_interrupt[index])).rst_val_lo;
                soc_reg32_set(unit, TMU_INTR_MASK(tmu_qe_interrupt[index]), SOC_BLOCK_PORT(unit, qe_inst), 0, regval);
                soc_cmicm_intr4_enable(unit, 1 << ((PTR_TO_INT(intr_val)) % 32));
                break;
            }
        }
    }
}

/* run single DDR test 
 * the parameter pattern is only used when mode is DDR_STANDARD_TEST
 */
int soc_sbx_caladan3_tmu_ddr_test(int unit, DDRMemTests mode, uint32 pattern) 
{
    int ci, ci_start=0, ci_end=SOC_MAX_NUM_CI_BLKS;
    uint32 burst_size, start_addr=0, step_size=1, udata=0;

    /* Max out burst size based on device given */
    if (SOC_DDR3_NUM_ROWS(unit) > 32*1024) {
        burst_size = (1 << 25) -1;
    } else if (SOC_DDR3_NUM_ROWS(unit) > 16*1024) {
        burst_size = (1 << 24) -1;
    } else if (SOC_DDR3_NUM_ROWS(unit) > 8*1024) {
        burst_size = (1 << 23) -1;  
    } else {
        burst_size = (1 << 22) -1;
    }

    for (ci = ci_start; ci < ci_end; ci++) {
      /* set test specific attributes */
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit,ci,start_addr));
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STEPr (unit,ci,step_size));
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_BURSTr(unit,ci,burst_size));
      /* only STANDARD_DDR test uses pattern data */
      if (DDR_STANDARD_TEST == mode) {
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA0r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA1r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA2r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA3r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA4r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA5r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA6r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA7r(unit,ci,pattern));
        /* set the alt_data */
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA0r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA1r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA2r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA3r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA4r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA5r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA6r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA7r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&udata));
      }
    }

    udata = 0;
    for (ci = ci_start; ci < ci_end; ci++) {
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0);
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0);
        
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_ITERr(unit, ci, 1));
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &udata));
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, MODEf,(mode-1));
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, RAM_DONEf, 1);      /* W1TC */
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, RAM_TESTf, 0);      /* clear */
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, RAM_TEST_FAILf, 1); /* W1TC */
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, WRITE_ONLYf, 0); 
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, READ_ONLYf,  0); 
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, udata));

        /* set ram_test - to start the test */
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &udata));
        soc_reg_field_set(unit, CI_DDR_TESTr, &udata, RAM_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, udata));
    }

    /* wait long enough for all ci's being tested to be done */
    sal_sleep(2);
    
    /* clean up after test run */
    for (ci = ci_start; ci < ci_end; ci++) {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&udata));
        soc_reg_field_set(unit,CI_DDR_TESTr,&udata,RAM_DONEf,1);      /* W1TC */
        soc_reg_field_set(unit,CI_DDR_TESTr,&udata,RAM_TESTf,0);      /* clear*/
        soc_reg_field_set(unit,CI_DDR_TESTr,&udata,RAM_TEST_FAILf,1); /* W1TC */
        soc_reg_field_set(unit,CI_DDR_TESTr,&udata, WRITE_ONLYf, 0); 
        soc_reg_field_set(unit,CI_DDR_TESTr,&udata, READ_ONLYf, 0); 
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,udata));
    }
    
    return SOC_E_NONE;
}

#endif

