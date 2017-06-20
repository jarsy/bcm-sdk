/* $Id: c3hppc_tmu_test1.c,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/sbx/sbx_drv.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_TMU_TEST1__STREAM_NUM                                (1)

#define C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM                        (4096)
#define C3HPPC_TMU_TEST1__DM_TABLE_ENTRIES_PER_ROW                  (1)  /* had 128 */

#define C3HPPC_TMU_TEST1__EML_MAX_KEYS                              (16384)
#define C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM                  (4096)
#define C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRIES_PER_ROW            (1)
#define C3HPPC_TMU_TEST1__EML_CHAIN_TABLE_ENTRIES_PER_ROW           (1)
#define C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM       (5)
#define C3HPPC_TMU_TEST1__EML_LRP_KEY_PROGRAM                       (0)
#define C3HPPC_TMU_TEST1__EML_TMU_PROGRAM                           (0)
#define C3HPPC_TMU_TEST1__EML64_UE_LRP_KEY_PROGRAM0                 (2)
#define C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM0                     (2)
#define C3HPPC_TMU_TEST1__EML64_UE_LRP_KEY_PROGRAM1                 (3)
#define C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM1                     (3)

#define C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM                     (4096)
#define C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRIES_PER_ROW               (1)
#define C3HPPC_TMU_TEST1__EMC64_LRP_KEY_PROGRAM                     (1)
#define C3HPPC_TMU_TEST1__EMC64_TMU_PROGRAM                         (1)

#define C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT                           (0)
#define C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT_BASE                      (0)
#define C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR                    (2)
#define C3HPPC_TMU_TEST1__LPMIPV4_MAX_LOOKUP_PREFIXES               (0x10000)
#define C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM                     (256)
#define C3HPPC_TMU_TEST1__LPMIPV4_RPB_PREFIX_SIZE                   (48)
#define C3HPPC_TMU_TEST1__LPMIPV4_BBX_PREFIX_SIZE                   (48)
#define C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM                 (48)
#define C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM           (32)
#define C3HPPC_TMU_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
/* The multiply by 2 is due to the bucket pair concept */
#define C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM            (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * \
                                                                     C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM * 2)
#define C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW      (1)
#define C3HPPC_TMU_TEST1__LPMIPV4_LRP_KEY_PROGRAM                   (4)
#define C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM                       (4)

#define C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT                           (1)
#define C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT_BASE                      (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM / 4)
#define C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM                     (256)
#define C3HPPC_TMU_TEST1__LPMIPV6_RPB_PREFIX_SIZE                   (144)
#define C3HPPC_TMU_TEST1__LPMIPV6_BBX_PREFIX_SIZE                   (128)
#define C3HPPC_TMU_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM                 (72)
#define C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM           (32)
#define C3HPPC_TMU_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
/* The multiply by 2 is due to the bucket pair concept */
#define C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM            (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * \
                                                                     C3HPPC_TMU_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM * 2)
#define C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW      (1)
#define C3HPPC_TMU_TEST1__LPMIPV6_LRP_KEY_PROGRAM                   (5)
#define C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM                       (5)



#define C3HPPC_TMU_TEST1__TSR_ERROR_MASK                      (0x40000000)

#define C3HPPC_TMU_TEST1__NO_ROOT_HITS                        (1) 
#define C3HPPC_TMU_TEST1__EML_INSERT                          (2) 
#define C3HPPC_TMU_TEST1__DO_HASHING                          (3) 
#define C3HPPC_TMU_TEST1__LOGICAL                             (1) 
#define C3HPPC_TMU_TEST1__PHYSICAL                            (2) 

#define C3HPPC_TMU_TEST1__SEQUENTIAL_INCREMENT                (1)
#define C3HPPC_TMU_TEST1__TABLE_CONTENTION                    (1)
#define C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE                  (2)
#define C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING    (4)

#define C3HPPC_TMU_TEST1__KEY_FLOW_SEQUENCE_BASE              (0x40001)  /* Key sequence OCM base address */ 

#define C3HPPC_TMU_TEST1__DRAM_PERFORMANCE                    (1)
#define C3HPPC_TMU_TEST1__DRAM_and_CACHE_PERFORMANCE          (2)

#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static void  setup_emc64_table_contents( int nSetupOptions, uint32 *pRootTableContents, uint32 *pNextTableContents,
                                         int nRootTable, int nNextTable );
static void  setup_emc64_table_entry( uint32 u128Mode, uint32 uEntryKey, uint32 uIndex, uint32 *pTable );
static void  setup_eml64_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                         uint32 *pRootTableContents, uint32 *pChainTableContents );
static void  setup_eml176_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                          uint32 *pRootTableContents, uint32 *pChainTableContents );
static void  setup_eml304_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                          uint32 *pRootTableContents, uint32 *pChainTableContents );
static void  setup_eml424_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                          uint32 *pRootTableContents, uint32 *pChainTableContents );

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;
#endif
static uint32 g_uDmLookUp, g_auEmlFirstLookUp[C3HPPC_TMU_SUBKEY_NUM];

static uint32 g_uPmBucketShift;

static uint8 g_bDoXLreads;
static uint8 g_bDoInitXLreads;
static uint8 g_bPerformanceTest;
static uint8 g_bAllowLateResults;
#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
static uint8 g_bDumpTmuDebugInfo = 0;

static char g_acKeyLookupMissScoreBoard[C3HPPC_TMU_TEST1__EML_MAX_KEYS];
#endif 

int
c3hppc_tmu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  uint32 uResultsTimer;

  g_auEmlFirstLookUp[0] = g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__DO_NOTHING;
  g_uDmLookUp = 0;
  g_uPmBucketShift = 2;
  g_bDoXLreads = 1;
  g_bDoInitXLreads = 0;
  g_bPerformanceTest = 0;
  g_bAllowLateResults = 0;
  
  uResultsTimer = 0;
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uTmuBringUp = 1;

  if ( pc3hppcTestInfo->bPerformanceTest ) {

    g_bPerformanceTest = pc3hppcTestInfo->bPerformanceTest;
    pc3hppcTestInfo->BringUpControl.bTmuCacheEnable = 0;
    uResultsTimer = ( SAL_BOOT_QUICKTURN ) ? 0x10000 : 0x7fffffff;
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
    pc3hppcTestInfo->BringUpControl.bLrpMaximizeActiveContexts = 0;

/*
    if ( pc3hppcTestInfo->bTapsUnified == 1 ) {
      pc3hppcTestInfo->nActiveDmNum = 0;
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_ipv4_unified_perf.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,2);
      g_uPmBucketShift = 4;
      uResultsTimer = 37;
*/

    if ( pc3hppcTestInfo->nDmType == 366 && pc3hppcTestInfo->bEML64Enable && pc3hppcTestInfo->bIPV4Enable ) {

      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_ipv4_perf.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,2);
      g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM366;
      g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML64;
      g_uPmBucketShift = 4;

    } else if ( pc3hppcTestInfo->bIPV4Enable ) {
      pc3hppcTestInfo->nActiveDmNum = 0;
      /*
         This file is used for sequential and random databases to measure both DRAM and Cache impacts.
      */
      if ( pc3hppcTestInfo->bPerformanceTest == C3HPPC_TMU_TEST1__DRAM_and_CACHE_PERFORMANCE ) {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_ipv4_perf.oasm");
        pc3hppcTestInfo->BringUpControl.bTmuCacheEnable = 1;
      } else {
        if ( pc3hppcTestInfo->bTapsUnified == 1 )
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_ipv4_unified_perf.oasm");
        else
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_ipv4_dram_perf.oasm");
      }

      if ( pc3hppcTestInfo->bTapsUnified == 1 ) g_uPmBucketShift = 4;
      else if ( pc3hppcTestInfo->nIPV4BucketPrefixNum <= 7 ) g_uPmBucketShift = 2;
      else if ( pc3hppcTestInfo->nIPV4BucketPrefixNum <  21 ) g_uPmBucketShift = 3;
      else if ( pc3hppcTestInfo->nIPV4BucketPrefixNum <  42 ) g_uPmBucketShift = 4;
      else g_uPmBucketShift = 5;


    } else if ( pc3hppcTestInfo->bIPV6Enable ) {

      pc3hppcTestInfo->nActiveDmNum = 0;
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_ipv6_dram_perf.oasm");

      if ( pc3hppcTestInfo->nIPV6BucketPrefixNum <= 5 ) g_uPmBucketShift = 2;
      else if ( pc3hppcTestInfo->nIPV6BucketPrefixNum <  15 ) g_uPmBucketShift = 3;
      else if ( pc3hppcTestInfo->nIPV6BucketPrefixNum <  30 ) g_uPmBucketShift = 4;
      else g_uPmBucketShift = 5;

    } else if ( pc3hppcTestInfo->bEML64Enable || pc3hppcTestInfo->bEML176Enable ||
                pc3hppcTestInfo->bEML304Enable || pc3hppcTestInfo->bEML424Enable ) {

      if ( pc3hppcTestInfo->nMaxKey >= C3HPPC_TMU_TEST1__EML_MAX_KEYS ) {
        /* EML chain performance tests ... */
        pc3hppcTestInfo->BringUpControl.bTmuCacheEnable = 1;
      }
      pc3hppcTestInfo->nActiveDmNum = 0;
      if ( pc3hppcTestInfo->bEML64Enable ) {

        g_auEmlFirstLookUp[0] = g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__1ST_EML64;
        if ( pc3hppcTestInfo->nMaxKey >= C3HPPC_TMU_TEST1__EML_MAX_KEYS )
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml64_chain_perf.oasm");
        else
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml64_dram_perf.oasm");

      } else if ( pc3hppcTestInfo->bEML176Enable ) {

        g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML176;
        if ( pc3hppcTestInfo->nMaxKey >= C3HPPC_TMU_TEST1__EML_MAX_KEYS )
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml176_chain_perf.oasm");
        else
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml176_dram_perf.oasm");

      } else if ( pc3hppcTestInfo->bEML304Enable ) {

        g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML304;
        if ( pc3hppcTestInfo->nMaxKey >= C3HPPC_TMU_TEST1__EML_MAX_KEYS )
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml304_chain_perf.oasm");
        else
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml304_dram_perf.oasm");

      } else if ( pc3hppcTestInfo->bEML424Enable ) {

        g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML424;
        if ( pc3hppcTestInfo->nMaxKey >= C3HPPC_TMU_TEST1__EML_MAX_KEYS )
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml424_chain_perf.oasm");
        else
          strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_eml424_dram_perf.oasm");

      }
       

    } else {

      switch ( pc3hppcTestInfo->nDmType ) {
        case 119:    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM119;
                     strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_dm119_dram_perf.oasm");
                     break;
        case 247:    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM247;
                     strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_dm494_dram_perf.oasm");
                     break;
        case 366:    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM366;
                     strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_dm494_dram_perf.oasm");
                     break;
        case 494:    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494;
                     strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_dm494_dram_perf.oasm");
                     break;
      }
      pc3hppcTestInfo->nMaxKey = C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM;
      g_uPmBucketShift = 3;

    } 

  } else if ( pc3hppcTestInfo->bEML176Enable && pc3hppcTestInfo->bEML64Enable && pc3hppcTestInfo->bEMC64Enable &&
              pc3hppcTestInfo->bEML144Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML64;
    g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__1ST_EML176;
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_eml144_emc64.oasm");
    uResultsTimer = 100;
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML176Enable && pc3hppcTestInfo->bIPV6Enable && pc3hppcTestInfo->bEMC64Enable &&
              pc3hppcTestInfo->bEML144Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML176;
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml144_ipv6_emc64.oasm");
    uResultsTimer = 100;
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML176Enable && pc3hppcTestInfo->bIPV4Enable && pc3hppcTestInfo->bEMC64Enable &&
              pc3hppcTestInfo->bEML144Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML176;
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml144_ipv4_emc64.oasm");
    uResultsTimer = 100;
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML64Enable && !pc3hppcTestInfo->bEMC64Enable ) {

    g_auEmlFirstLookUp[0] = g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__1ST_EML64;

    if ( pc3hppcTestInfo->nActiveDmNum == 4 && pc3hppcTestInfo->nSwitchCount == 5 ) {

      if ( pc3hppcTestInfo->bEMLInsertEnable ) {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_eml64_switch5_insert.oasm");
      } else {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_eml64_switch5.oasm");
      }
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
      g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM119;
      g_uPmBucketShift = 3;
      pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
      g_bAllowLateResults = (uint8) ( sal_rand() % 2 );
      uResultsTimer = ( g_bAllowLateResults ) ? 128 : 0xffffffff;

    } else if ( pc3hppcTestInfo->nActiveDmNum == 4 && pc3hppcTestInfo->nSwitchCount == 3 ) {

      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_2xeml64.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
      g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM119;

    } else if ( pc3hppcTestInfo->nActiveDmNum == 1 ) {

      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
      g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494;

    }

  } else if ( pc3hppcTestInfo->bIPV4Enable ) {

    if ( pc3hppcTestInfo->bEMC64Enable ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_ipv4_emc64.oasm");
      g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
      pc3hppcTestInfo->nActiveDmNum = 1;
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    }

  } else if ( pc3hppcTestInfo->bIPV6Enable && pc3hppcTestInfo->bEMC64Enable ) {

    if ( pc3hppcTestInfo->nPeriodic ) {
    } else {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_ipv6_emc64.oasm");
    }
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML64Enable && pc3hppcTestInfo->bEMC64Enable ) {

    g_auEmlFirstLookUp[0] = g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__1ST_EML64;

    if ( pc3hppcTestInfo->nSwitchCount == 2 ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_emc64_switch121.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
    } else {
      if ( pc3hppcTestInfo->nPeriodic ) {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_emc64_periodic.oasm");
      } else if ( pc3hppcTestInfo->bEMLInsertEnable ) {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_emc64_insert.oasm");
      } else {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml64_emc64.oasm");
      }
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    }
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML176Enable && pc3hppcTestInfo->bEMC64Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML176;

    if ( pc3hppcTestInfo->bEML144Enable ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml144_emc64.oasm");
      g_auEmlFirstLookUp[1] = C3HPPC_TMU_LOOKUP__1ST_EML176;
    } else {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml176_emc64.oasm");
    }
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;

  } else if ( pc3hppcTestInfo->bEML304Enable && pc3hppcTestInfo->bEMC64Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML304;

    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml304_emc64.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;
    uResultsTimer = 200;

  } else if ( pc3hppcTestInfo->bEML424Enable && pc3hppcTestInfo->bEMC64Enable ) {

    g_auEmlFirstLookUp[0] = C3HPPC_TMU_LOOKUP__1ST_EML424;

    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_eml424_emc64.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    pc3hppcTestInfo->nActiveDmNum = 1;
    uResultsTimer = 400;

  } else if ( pc3hppcTestInfo->nActiveDmNum == 4 ) {

    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM119; 
    if ( pc3hppcTestInfo->nSwitchCount == 5 ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_switch5.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
    } else if ( pc3hppcTestInfo->nSwitchCount == 2 ) {
      if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE ) { 
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_switch2_updater_transitions.oasm");
      } else {
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_switch2.oasm");
      }
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
    } else {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_4xdm_switch1.oasm");
      COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
    }

  } else if ( pc3hppcTestInfo->nActiveDmNum == 1 ) {

    g_uDmLookUp = C3HPPC_TMU_LOOKUP__DM494; 
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "tmu_test1_1xdm_switch1.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
  }




/* For John
pc3hppcTestInfo->nActiveDmNum = 0;
strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "ocm_svp_latency_p0.oasm");
COMPILER_64_ZERO(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition;
*/


  pc3hppcTestInfo->BringUpControl.bTmuEMC128Mode = sal_rand() % 2;
  pc3hppcTestInfo->BringUpControl.uTmuEmlMaxProvisionedKey = 
                                   C3HPPC_MAX( C3HPPC_TMU_TEST1__EML_MAX_KEYS, pc3hppcTestInfo->nMaxKey );

  if ( pc3hppcTestInfo->BringUpControl.bTmuHwEmlChainManagement ) {
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[0].uSegmentLimit = pc3hppcTestInfo->nMaxKey - 1;
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[1].uSegmentLimit = (2 * pc3hppcTestInfo->nMaxKey) - 1;
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;
  


/* For John
uint32 uRegisterValue;
READ_LRB_DEBUGr( pc3hppcTestInfo->nUnit, &uRegisterValue );
soc_reg_field_set( pc3hppcTestInfo->nUnit, LRB_DEBUGr, &uRegisterValue, COP_LATENCYf, 1 );
soc_reg_field_set( pc3hppcTestInfo->nUnit, LRB_DEBUGr, &uRegisterValue, OCM_LATENCYf, 1 );
WRITE_LRB_DEBUGr( pc3hppcTestInfo->nUnit, uRegisterValue );
*/


  c3hppc_tmu_region_map_setup( pc3hppcTestInfo->nUnit, C3HPPC_TMU_REGION_LAYOUT__RANDOM );

  if ( pc3hppcTestInfo->bEMLInsertEnable && uResultsTimer == 0 ) {
    if ( pc3hppcTestInfo->nMaxKey > 8192 ) uResultsTimer = 200;
    if ( pc3hppcTestInfo->nMaxKey > 4096 ) uResultsTimer = 100;
  }
  if ( pc3hppcTestInfo->bIPV6CollisionEnable ) {
    uResultsTimer += 100;
  }
  if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
    uResultsTimer *= 2;
  }
  c3hppc_lrp_set_results_timer( pc3hppcTestInfo->nUnit, uResultsTimer );


  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nCopInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance] );
  }


  c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                              pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                       pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );


  return 0;
}

int
c3hppc_tmu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int  nIndex, nStartIndex, nTimeOut, nTable, nTableIndex, nOcmPort, nTP, nSetupOptions, nKey, nKey1, nSubKey;
  uint32 *pDmTableDmaSratchPad;
  uint32 uReg, uContexts;
  uint32 uColumnOffset, uRegionRowOffset; 
  uint32 uDmEntrySizeIn64b, uDmEntrySizeIn32b, uDmEntriesPerRow;
  uint32 uDynamicUpdateControl, uUpdateCounter, uUpdateNum, u1KblockNum;
  uint32 uLrpDmLookUp;
  uint32 uDeadlineOffset;
  uint32 *pEML_RootTable, *pEML_ChainTable, *pEML_InsertList;
  uint32 uEML_RootEntrySizeIn64b, uEML_RootEntriesPerRow;
  uint32 uEML_ChainEntrySizeIn64b, uEML_ChainEntriesPerRow;
  uint32 uEML_InsertEntrySizeInBytes;
  uint32 uEML_RootTable, uEML_ChainTable, uEML_ChainLimit, uEML_RootTableEntryNum, uEML_FifoChainPoolMask;
  uint8  bEML_MultiTableSetup;
  uint32 *pEMC64RootTable, *pEMC64NextTable;
  uint32 uEMC_EntrySizeIn64b, uEMC_EntriesPerRow;
  uint32 uEMC_RootTable, uEMC_NextTable;
  uint32 *pIPV4BucketTable, *apIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV4BucketTableEntryNum, uIPV4BucketEntrySizeIn64b, uIPV4BucketEntriesPerRow;
  uint32 uIPV4AssocDataEntrySizeIn64b, uIPV4AssocDataEntriesPerRow, uIPV4AssocDataTableEntryNum;
  uint32 auIPV4BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 *pIPV6BucketTable, *apIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV6BucketEntrySizeIn64b, uIPV6BucketEntriesPerRow, uIPV6KeyAndAssocDataTableEntryNum;
  uint32 uIPV6KeyAndAssocDataEntrySizeIn64b, uIPV6KeyAndAssocDataEntriesPerRow;
  uint32 auIPV6BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 *pRPBcommands, *pBBXcommands, *pBRRcommands;
  uint32 auCiMemAccEntryData[8];
  int nUeCi;
  c3hppc_tmu_ci_mem_acc_addr_ut CiMemAccAddr;
  uint32 uUE, uUeEntryAddr;
  uint8 bDmUEseen, bEmlSK0UEseen, bEmlSK1UEseen;
  uint32 *apKeyDeleteList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM], *apKeyReInsertList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 auKeyDeleteListLength[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 uInsertEntrySizeIn32b, uKeySizeIn32b, uKeyFilter, uKeyFilterMask;
  int nCmdFifoSelect;
  int nBulkDelete;
  uint64 uuActionThreshold, uuTotalSearches;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize;
  char *acAvailableKeyList;
  double dMsps;



  nKey1 = 0;
  pDmTableDmaSratchPad = NULL;
  uDmEntrySizeIn64b = 0;
  uDmEntrySizeIn32b = 0;
  uLrpDmLookUp = 0;
  pEML_RootTable = NULL;
  pEML_ChainTable = NULL;
  uEML_ChainTable = 0;
  pEML_InsertList = NULL;
  uEML_InsertEntrySizeInBytes = 0;
  uEML_RootTable = 0;
  uEML_RootTableEntryNum = 0;
  uEML_FifoChainPoolMask = 0;
  bEML_MultiTableSetup = 0;
  pEMC64RootTable = NULL;
  pEMC64NextTable = NULL;
  uEMC_RootTable = 0;
  uIPV4BucketTableEntryNum = 0;
  pIPV4BucketTable = NULL;
  pIPV6BucketTable = NULL;
  pRPBcommands = NULL;
  pBBXcommands = NULL;
  pBRRcommands = NULL;
  nUeCi = 0;
  uUeEntryAddr = 0;
  nBulkDelete = 0;
  
  


  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameter to select the stream of CMGR operations.
   *
   *  15:0   --> Max table entry index used to exercise QE cache with tighter address space
   *  19:16  --> Used for entry address bits 11:0 when running periodic ucode load
   *  20:20  --> No EML root table hit mode
   *  21:21  --> Allow result misses due to key state transitions
   *  22:22  --> EMC 128 mode
   *  23:23  --> Allow late results
   *  47:32  --> Initialized by test to be starting subkey0, used by ucode for incrementing key pattern
   *  63:48  --> Initialized by test to be starting subkey1, used by ucode for incrementing key pattern
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    if ( g_bPerformanceTest ) {
      g_pFlowTable[nIndex].uData[0] = 0xffff;
    } else {
      g_pFlowTable[nIndex].uData[0] = (pc3hppcTestInfo->nMaxKey - 1) & 0xffff;
      g_pFlowTable[nIndex].uData[1] = sal_rand() & g_pFlowTable[nIndex].uData[0];
      g_pFlowTable[nIndex].uData[1] |= (sal_rand() & g_pFlowTable[nIndex].uData[0]) << 16;
    }
    if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_TMU_TEST1__NO_ROOT_HITS ) g_pFlowTable[nIndex].uData[0] |= 0x00100000;
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) {
      g_pFlowTable[nIndex].uData[0] |= 0x00200000;
    }
    if ( pc3hppcTestInfo->BringUpControl.bTmuEMC128Mode ) {
      g_pFlowTable[nIndex].uData[0] |= 0x00400000;
    }
    if ( g_bAllowLateResults ) {
      g_pFlowTable[nIndex].uData[0] |= 0x00800000;
    }
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );


  /****************************************************************************************************************************
   *
   * Performance test setup
   *
   *****************************************************************************************************************************/
  if ( g_bPerformanceTest ) {
    if ( g_bPerformanceTest == C3HPPC_TMU_TEST1__DRAM_and_CACHE_PERFORMANCE ) {
      nDmaBlockSize = pc3hppcTestInfo->nMaxKey;
      pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                    nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                    "ocm dma block");
      acAvailableKeyList = (char *) sal_alloc( C3HPPC_TMU_TEST1__LPMIPV4_MAX_LOOKUP_PREFIXES, "random key buffer");  
      sal_memset( acAvailableKeyList, 0x01, C3HPPC_TMU_TEST1__LPMIPV4_MAX_LOOKUP_PREFIXES );
  
      for ( nIndex = 0, nKey = 0; nIndex < pc3hppcTestInfo->nMaxKey; ++nIndex ) {
        if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_TMU_TEST1__SEQUENTIAL_INCREMENT ) {
          pOcmBlock[nIndex].uData[0] = (uint32) nKey++;
        } else {
          while ( 1 ) {
            nKey = sal_rand() % C3HPPC_TMU_TEST1__LPMIPV4_MAX_LOOKUP_PREFIXES;
            if ( acAvailableKeyList[nKey] ) {
              acAvailableKeyList[nKey] = 0;
              break;
            }
          }
          pOcmBlock[nIndex].uData[0] = (uint32) nKey;
        }

        pOcmBlock[nIndex].uData[1] = pOcmBlock[nIndex].uData[0];
      }
      cli_out("\nInitializing OCM contents with a %s sequence of IPV4 keys ... \n", (pc3hppcTestInfo->nSetupOptions ? "linear" : "random") );
      nOcmPort = c3hppc_ocm_map_lrp2ocm_port(0);
      c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                 C3HPPC_TMU_TEST1__KEY_FLOW_SEQUENCE_BASE, (C3HPPC_TMU_TEST1__KEY_FLOW_SEQUENCE_BASE+nDmaBlockSize-1),
                                 1, pOcmBlock->uData );
      soc_cm_sfree( pc3hppcTestInfo->nUnit, pOcmBlock );
      sal_free( acAvailableKeyList );

    }

    /* Due to the extreme latency this is necessary to prevent DEADLINE ALIASING */
    if ( pc3hppcTestInfo->uReplicationFactor == 1 ) {
      if ( (pc3hppcTestInfo->bIPV4Enable && pc3hppcTestInfo->nIPV4BucketPrefixNum >= 42) ||
           (pc3hppcTestInfo->bIPV6Enable && pc3hppcTestInfo->nIPV6BucketPrefixNum >= 30) ) {

        WRITE_TMA_GLOBAL_ARRIVAL_TIMER_DEBUG1r( pc3hppcTestInfo->nUnit, 2 ); 
        for ( nIndex = 0; nIndex < pc3hppcTestInfo->nNumberOfCIs; ++nIndex ) {
          soc_reg32_get( pc3hppcTestInfo->nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nIndex), 0, &uReg );
          soc_reg_field_set( pc3hppcTestInfo->nUnit, CI_CONFIG3r, &uReg, TCOMP_SHIFTf, 1 );
          soc_reg32_set( pc3hppcTestInfo->nUnit, CI_CONFIG3r, SOC_BLOCK_PORT(pc3hppcTestInfo->nUnit,nIndex), 0, uReg);
        }
      }
    }

    c3hppc_lrp_setup_ring_wheel( pc3hppcTestInfo->nUnit, 2, 0, pc3hppcTestInfo->nMaxKey, 0 ); 
  }

  uRegionRowOffset = 0;

  /****************************************************************************************************************************
   *
   * DM Tables setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->nActiveDmNum ) {

    uDmEntriesPerRow = C3HPPC_TMU_TEST1__DM_TABLE_ENTRIES_PER_ROW;
/* Coverity
*/
    switch ( g_uDmLookUp ) {
      case C3HPPC_TMU_LOOKUP__DM119:    uDmEntrySizeIn64b = 2; uLrpDmLookUp = C3HPPC_LRP_LOOKUP__DM119; break;
      case C3HPPC_TMU_LOOKUP__DM247:    uDmEntrySizeIn64b = 4; uLrpDmLookUp = C3HPPC_LRP_LOOKUP__DM247; break;
      case C3HPPC_TMU_LOOKUP__DM366:    uDmEntrySizeIn64b = 6; uLrpDmLookUp = C3HPPC_LRP_LOOKUP__DM366; break;
      case C3HPPC_TMU_LOOKUP__DM494:    uDmEntrySizeIn64b = 8; uLrpDmLookUp = C3HPPC_LRP_LOOKUP__DM494; break;
    }
    uDmEntrySizeIn32b = 2 * uDmEntrySizeIn64b;

    for ( nTable = 0; nTable < pc3hppcTestInfo->nActiveDmNum; ++nTable ) {
      uDeadlineOffset = 64;
/* Coverity
      uColumnOffset = ( uDmEntriesPerRow == 1 ) ? (uDmEntrySizeIn64b * nTable) : 0;
*/
      uColumnOffset = (uDmEntrySizeIn64b * nTable);
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, nTable, g_uDmLookUp,
                              C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uDmEntriesPerRow, uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0,
                              ( ((nTable+1) == pc3hppcTestInfo->nActiveDmNum) ? &uRegionRowOffset : (uint32 *)NULL ) );
      c3hppc_lrp_setup_dm_segment_table( pc3hppcTestInfo->nUnit, nTable, nTable, uLrpDmLookUp ); 
    }
  
    pDmTableDmaSratchPad = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM * uDmEntrySizeIn64b * sizeof(uint64)),
                                          "dma scratch pad");

    for ( nTable = 0; nTable < pc3hppcTestInfo->nActiveDmNum; ++nTable ) {
      cli_out("\nINFO:  Initializing DM%d Table ...\n", nTable );
      c3hppc_test__setup_dm_table_contents( g_uDmLookUp, nTable, C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM, 0x11100000, pDmTableDmaSratchPad );
      c3hppc_tmu_xl_write( (nTable & 1), nTable, 0, C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM,
                           0, pDmTableDmaSratchPad );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  (nTable & 1), nTable, 0, C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM,
                             0, pDmTableDmaSratchPad );
      }
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 0, C3HPPC_TMU_PM_INTF__DM0, 0, 0, 0 );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 1, C3HPPC_TMU_PM_INTF__DM0, 0, 1, 0 );
    if ( g_bPerformanceTest && pc3hppcTestInfo->nActiveDmNum > 1 ) {
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__DM1, 0, 0, 0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__DM1, 0, 1, 0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__DM2, 0, 0, 0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__DM2, 0, 1, 0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 6, C3HPPC_TMU_PM_INTF__DM3, 0, 0, 0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 7, C3HPPC_TMU_PM_INTF__DM3, 0, 1, 0 );
    }

  } else {
    nTable = 0;
  }


  /****************************************************************************************************************************
   *
   * EML Root and Chain table setup
   *
   *****************************************************************************************************************************/
  if ( g_auEmlFirstLookUp[0] <= C3HPPC_TMU_LOOKUP__1ST_EML424 || g_auEmlFirstLookUp[1] <= C3HPPC_TMU_LOOKUP__1ST_EML424 ) {

    if ( g_auEmlFirstLookUp[0] != g_auEmlFirstLookUp[1] && g_auEmlFirstLookUp[1] != C3HPPC_TMU_LOOKUP__DO_NOTHING ) bEML_MultiTableSetup = 1;

    for ( nSubKey = 0; nSubKey < C3HPPC_TMU_SUBKEY_NUM; ++nSubKey ) {

      if ( nSubKey == 0 || bEML_MultiTableSetup ) { 

        uEML_RootTable = (uint32) nTable;
        uEML_RootTableEntryNum = ( g_bPerformanceTest ) ? C3HPPC_TMU_TEST1__EML_MAX_KEYS :
                                                          C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM;
        uEML_RootEntrySizeIn64b = c3hppc_tmu_get_eml_root_table_entry_size_in_64b( g_auEmlFirstLookUp[nSubKey] );
        uEML_RootEntriesPerRow = C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRIES_PER_ROW;
        uEML_ChainTable = uEML_RootTable + 1;
        nTable = (int) (uEML_ChainTable + 1);
        if ( bEML_MultiTableSetup ) {
          uEML_FifoChainPoolMask = ( nSubKey == 0 ) ? 0x3 : 0xc;
        } else {
          uEML_FifoChainPoolMask = 0xf;
        }
        uEML_ChainLimit = ( pc3hppcTestInfo->bEMLInsertEnable ) ? C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM :
                                                                  C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM;
        uEML_ChainEntrySizeIn64b = uEML_ChainLimit * c3hppc_tmu_get_eml_chain_table_chain_element_entry_size_in_64b( g_auEmlFirstLookUp[nSubKey] );
        uEML_ChainEntriesPerRow = C3HPPC_TMU_TEST1__EML_CHAIN_TABLE_ENTRIES_PER_ROW;
        uEML_InsertEntrySizeInBytes = c3hppc_tmu_get_eml_insert_cmd_entry_size_in_64b( g_auEmlFirstLookUp[nSubKey] ) * sizeof(uint64);

        uDeadlineOffset = 0;
        uColumnOffset = 0;
        c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)uEML_RootTable, g_auEmlFirstLookUp[nSubKey],  
                                uEML_RootTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                                0, uRegionRowOffset, uColumnOffset,
                                uEML_RootEntriesPerRow, uDeadlineOffset, 0, ((uint32) pc3hppcTestInfo->bEMLInsertEnable),
                                0, (uEML_ChainLimit-1), uEML_FifoChainPoolMask, 0, 0, &uRegionRowOffset );

        uDeadlineOffset = 64;
        c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)uEML_ChainTable, (g_auEmlFirstLookUp[nSubKey] + C3HPPC_TMU_LOOKUP__2ND_EML64),
                                uEML_RootTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                                0, uRegionRowOffset, uColumnOffset,
                                uEML_ChainEntriesPerRow, uDeadlineOffset, 0, ((uint32) pc3hppcTestInfo->bEMLInsertEnable), 0,
                                (uEML_ChainLimit-1), uEML_FifoChainPoolMask, 0, 0, &uRegionRowOffset );
      
        pEML_RootTable = (uint32 *) sal_alloc( (uEML_RootTableEntryNum * uEML_RootEntrySizeIn64b * sizeof(uint64)),
                                                "EML Root Table");
        if ( pc3hppcTestInfo->bEMLInsertEnable ) {
          /* With DRAM initialization the EM-LXXX-C chain table entries will read out with bad ECC.  The chain table dump routines use the 
             provisioned chain limit which will induce ECC errors.  Therefore initializing the chain table with 0's.
          */ 
          pEML_ChainTable = (uint32 *) sal_alloc( (uEML_RootTableEntryNum * uEML_ChainEntrySizeIn64b * sizeof(uint64)),
                                                  "EML Chain Table");
          sal_memset( pEML_ChainTable, 0x00, (uEML_RootTableEntryNum * uEML_RootEntrySizeIn64b * sizeof(uint64)) );
          c3hppc_tmu_xl_write( (int)(uEML_RootTable & 1), (int)uEML_ChainTable, 0,
                               uEML_RootTableEntryNum, 0, pEML_ChainTable );
          sal_free( pEML_ChainTable );

          pEML_ChainTable = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uEML_InsertEntrySizeInBytes),
                                                  "EML Insert Table");
          pEML_InsertList = pEML_ChainTable;

        } else {
          pEML_ChainTable = (uint32 *) sal_alloc( (uEML_RootTableEntryNum * uEML_ChainEntrySizeIn64b * sizeof(uint64)),
                                                  "EML Chain Table");
        }
    
        if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML64 ) {
          nSetupOptions = ( pc3hppcTestInfo->bEMLInsertEnable ) ? C3HPPC_TMU_TEST1__EML_INSERT : pc3hppcTestInfo->nSetupOptions; 
          setup_eml64_table_contents( nSetupOptions, pc3hppcTestInfo->nMaxKey, uEML_ChainTable, pEML_RootTable, pEML_ChainTable );
        } else if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML176 ) {
          nSetupOptions = (int) pc3hppcTestInfo->bEML144Enable;
          setup_eml176_table_contents( nSetupOptions, pc3hppcTestInfo->nMaxKey, uEML_ChainTable, pEML_RootTable, pEML_ChainTable );
        } else if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML304 ) {
          setup_eml304_table_contents( 0, pc3hppcTestInfo->nMaxKey, uEML_ChainTable, pEML_RootTable, pEML_ChainTable );
        } else if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML424 ) {
          setup_eml424_table_contents( 0, pc3hppcTestInfo->nMaxKey, uEML_ChainTable, pEML_RootTable, pEML_ChainTable );
        }


        if ( pc3hppcTestInfo->bSkipCiDramInit || pc3hppcTestInfo->bEMLInsertEnable == 0 ) {
          cli_out("\nINFO:  Initializing EML Root Table ...\n");
          c3hppc_tmu_xl_write( (int)(uEML_RootTable & 1), (int)uEML_RootTable, 0,
                               uEML_RootTableEntryNum, 0, pEML_RootTable );
          if ( g_bDoInitXLreads ) {
            c3hppc_tmu_xl_read(  (int)(uEML_RootTable & 1), (int)uEML_RootTable, 0,
                                 uEML_RootTableEntryNum, 0, pEML_RootTable );
          }
        }

        if ( pc3hppcTestInfo->bEMLInsertEnable ) {
          nTimeOut = 1000;
          while ( c3hppc_tmu_are_free_chain_fifos_empty(pc3hppcTestInfo->nUnit) ) {
            sal_usleep(1);
            if ( !(--nTimeOut) ) break;
          }
          if ( !nTimeOut ) {
            cli_out("\nERROR:  TIMEOUT occurred waiting for population of FREE CHAIN Fifo(s)\n");
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          } else {
            cli_out("\nINFO:  Inserting EML Table entries ...\n");
            c3hppc_tmu_eml_insert( (int)(uEML_RootTable & 1), (int)uEML_RootTable, 
                                   pc3hppcTestInfo->nMaxKey, pEML_InsertList, C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE );
            if ( !SAL_BOOT_QUICKTURN && !bEML_MultiTableSetup ) {
              /* The following c3hppc_tmu_xl_read() calls are used for the c3hppc_tmu_display_andor_scoreboard_eml_tables()
                 check of the tables.  This takes way too long in emulation so turning it off as it really is no longer 
                 necessary with the maturity of the TMU.
              */ 
              c3hppc_tmu_xl_read( (int)(uEML_RootTable & 1), (int)uEML_RootTable, 0,
                                  uEML_RootTableEntryNum, 0, NULL );
              c3hppc_tmu_xl_read( (int)(uEML_RootTable & 1), (int)uEML_ChainTable, 0,
                                  uEML_RootTableEntryNum, 0, NULL );
            }
          }
        } else {
          cli_out("\nINFO:  Initializing EML Chain Table ...\n");
          c3hppc_tmu_xl_write( (int)(uEML_ChainTable & 1), (int)uEML_ChainTable, 0,
                               uEML_RootTableEntryNum, 0, pEML_ChainTable );
          if ( g_bDoInitXLreads ) {
            c3hppc_tmu_xl_read( (int)(uEML_ChainTable & 1), (int)uEML_ChainTable, 0,
                                uEML_RootTableEntryNum, 0, pEML_ChainTable );
          }
        }
      }


      if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML64 ) {

        c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM, g_auEmlFirstLookUp[nSubKey], uEML_RootTable, 0,
                                    ( nSubKey == 1 ? 62 : 0 ), 2, 0, 0, 0 );

        /* Primarily used for UE injection testing */
        c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey,
                                    ( nSubKey == 1 ? C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM1 : C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM0 ),
                                    g_auEmlFirstLookUp[nSubKey], uEML_RootTable, 0,
                                    ( nSubKey == 1 ? 62 : 0 ), 2, 0, 0, 0 );

      } else if ( g_auEmlFirstLookUp[nSubKey] == C3HPPC_TMU_LOOKUP__1ST_EML176 && pc3hppcTestInfo->bEML144Enable ) {

        c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM, g_auEmlFirstLookUp[nSubKey], uEML_RootTable, 0,
                                    ( nSubKey == 1 ? 46 : 0 ), 18, 0, 0, 0 );

      } else {

        c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM, g_auEmlFirstLookUp[nSubKey], uEML_RootTable, 0,
                                    0, c3hppc_tmu_get_eml_key_size_in_bytes( g_auEmlFirstLookUp[nSubKey] ), 0, 0, 0 );
      }

      if ( nSubKey == 0 && bEML_MultiTableSetup ) {
        /* EML dynamic table modifications not allowed with mixed sub-key resulting in multiple EML tables. */
        pc3hppcTestInfo->nHostActivityControl = 0;
        sal_free( pEML_RootTable ); 
        sal_free( pEML_ChainTable );
      }

    }  /* for ( nSubKey = 0; nSubKey < C3HPPC_TMU_SUBKEY_NUM; ++nSubKey ) {  */


    /*
     * The "pc3hppcTestInfo->bIPV6Enable" condition is to support the "tmu_test1_1xdm_eml144_ipv6_emc64" testcase.
     * The "pc3hppcTestInfo->bIPV4Enable" condition is to support the "tmu_test1_1xdm_eml144_ipv4_emc64" testcase.
     * Even though only a single EML subkey is launched the LRP program needs to be setup to expect 2 sub-key results.
     * This had to be done here or in the IPv4/IPv6 code.  Chose to do it here.
    */
    if ( g_auEmlFirstLookUp[1] == C3HPPC_TMU_LOOKUP__DO_NOTHING && pc3hppcTestInfo->bIPV6Enable == 0 && pc3hppcTestInfo->bIPV4Enable == 0 ) {
      c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__EML_LRP_KEY_PROGRAM, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM, 1, 0 );
    } else {
      c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__EML_LRP_KEY_PROGRAM, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM, 1, 1 );
    }

    /* Primarily used for UE injection testing */
    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__EML64_UE_LRP_KEY_PROGRAM0, C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM0, 1, 0 );
    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__EML64_UE_LRP_KEY_PROGRAM1, C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM1, 0, 1 );

    if ( pc3hppcTestInfo->bIPV4Enable == 0 || pc3hppcTestInfo->bEML144Enable == 1 ) {
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 0, 0, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 0, 1, C3HPPC_TMU_TEST1__EML_TMU_PROGRAM );
    } else {
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 6, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM0 );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 7, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__EML64_UE_TMU_PROGRAM0 );
    }

  } else {
    uEML_ChainTable = (uint32) nTable;
  }  /* if ( pc3hppcTestInfo->bEML*Enable ) { */


  /****************************************************************************************************************************
   *
   * EMC64 Root and Next table setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->bEMC64Enable ) {

    uEMC_RootTable = uEML_ChainTable + 1;
    uEMC_NextTable = uEMC_RootTable + 1;
    uEMC_EntrySizeIn64b = C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b; 
    if ( c3hppc_tmu_get_emc128mode() ) --uEMC_EntrySizeIn64b;
    uEMC_EntriesPerRow = 1;

    uDeadlineOffset = 0;
    uColumnOffset = 0;
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)uEMC_RootTable, C3HPPC_TMU_LOOKUP__1ST_EMC64,
                            C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, uColumnOffset,
                            uEMC_EntriesPerRow, uDeadlineOffset, uEMC_NextTable, 0, 0, 0, 0, 0, 0, &uRegionRowOffset );

    uDeadlineOffset = 64;
    uColumnOffset = 0;
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)uEMC_NextTable, C3HPPC_TMU_LOOKUP__2ND_EMC64,
                            C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, uColumnOffset,
                            uEMC_EntriesPerRow, uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
    pEMC64RootTable = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM * uEMC_EntrySizeIn64b * sizeof(uint64)),
                                            "EMC64 Root Table");
    pEMC64NextTable = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM * uEMC_EntrySizeIn64b * sizeof(uint64)),
                                            "EMC64 Next Table");

    nSetupOptions = ( pc3hppcTestInfo->bBypassHash ) ? 0 : C3HPPC_TMU_TEST1__DO_HASHING; 
    setup_emc64_table_contents( nSetupOptions, pEMC64RootTable, pEMC64NextTable, (int)uEMC_RootTable, (int)uEMC_NextTable );

    cli_out("\nINFO:  Initializing EMC64 Root Table ...\n");
    c3hppc_tmu_xl_write( (int)(uEMC_RootTable & 1), (int)uEMC_RootTable, 0,
                         C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, 0, pEMC64RootTable );
    if ( g_bDoInitXLreads ) {
      c3hppc_tmu_xl_read(  (int)(uEMC_RootTable & 1), (int)uEMC_RootTable, 0,
                           C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, 0, pEMC64RootTable );
    }

    cli_out("\nINFO:  Initializing EMC64 Next Table ...\n");
    c3hppc_tmu_xl_write( (int)(uEMC_NextTable & 1), (int)uEMC_NextTable, 0,
                         C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, 0, pEMC64NextTable );
    if ( g_bDoInitXLreads ) {
      c3hppc_tmu_xl_read(  (int)(uEMC_NextTable & 1), (int)uEMC_NextTable, 0,
                           C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM, 0, pEMC64NextTable );
    }

    c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, 0, C3HPPC_TMU_TEST1__EMC64_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__1ST_EMC64, uEMC_RootTable, 0,
                                0, 2, 0, 0, 0 );

    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__EMC64_LRP_KEY_PROGRAM, C3HPPC_TMU_TEST1__EMC64_TMU_PROGRAM, 1, 0 );

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 6, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__EMC64_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 7, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__EMC64_TMU_PROGRAM );

  } else {
    uEMC_NextTable = uEML_ChainTable;
  }  /* if ( pc3hppcTestInfo->bEMC64Enable ) { */



  /****************************************************************************************************************************
   *
   * LPM IPV4 setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->bIPV4Enable && pc3hppcTestInfo->bTapsUnified == 0 ) {

    auIPV4BucketTable[0] = uEMC_NextTable + 1;
    auIPV4BucketTable[1] = auIPV4BucketTable[0] + 2;
    uIPV4BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV4BucketPrefixNum ); 
    uIPV4BucketEntriesPerRow = C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV4AssocDataTable[0] = auIPV4BucketTable[0] + 1;
    auIPV4AssocDataTable[1] = auIPV4AssocDataTable[0] + 2;
    uIPV4AssocDataTableEntryNum = C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntriesPerRow = uIPV4BucketEntriesPerRow * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntrySizeIn64b = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b; 

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET,
                              C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV4BucketEntriesPerRow, uDeadlineOffset, auIPV4AssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV4BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );
  
      if ( nTP == 0 ) {
        pIPV4BucketTable = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * uIPV4BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV4 Bucket Table");
        sal_memset( pIPV4BucketTable, 0, (C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * uIPV4BucketEntrySizeIn64b * sizeof(uint64)) );
      }

      uDeadlineOffset = 64;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4AssocDataTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_ASSOC_DATA,
                              uIPV4AssocDataTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV4AssocDataEntriesPerRow, uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
      apIPV4AssocDataTable[nTP] = (uint32 *) sal_alloc( (uIPV4AssocDataTableEntryNum *
                                                         uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)), "LPM IPV4 AssocData Table");
      sal_memset( apIPV4AssocDataTable[nTP], 0,
                  (uIPV4AssocDataTableEntryNum * uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)) );

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                     C3HPPC_TMU_TEST1__LPMIPV4_RPB_PREFIX_SIZE,
                                     C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM, C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT_BASE,
                                     C3HPPC_TMU_TEST1__LPMIPV4_BBX_PREFIX_SIZE,
                                     (2 * C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM * pc3hppcTestInfo->nIPV4BucketPrefixNum),
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV4 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV4 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV4 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_lpmipv4_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH,
                                               pc3hppcTestInfo->nIPV4BucketPrefixNum, auIPV4BucketTable[0],
                                               C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM, C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                               C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM,
                                               C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM,
                                               C3HPPC_TMU_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                               pRPBcommands, pBBXcommands, pBRRcommands, pIPV4BucketTable, apIPV4AssocDataTable);

    cli_out("\nINFO:  Initializing TAPS IPV4 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV4 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV4 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      /* The "tmu_test1_1xdm_eml144_ipv4_emc64" testcase assigns sub-key1 for IPv4. */
      if ( nTP == 0 && pc3hppcTestInfo->bEML144Enable == 1 ) continue;

      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( (int)(auIPV4BucketTable[nTP] & 1), (int)auIPV4BucketTable[nTP], 0,
                           C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, 0, pIPV4BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  (int)(auIPV4BucketTable[nTP] & 1), (int)auIPV4BucketTable[nTP], 0,
                             C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, 0, pIPV4BucketTable );
      }

      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( (int)(auIPV4AssocDataTable[nTP] & 1), (int)auIPV4AssocDataTable[nTP], 0,
                           uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  (int)(auIPV4AssocDataTable[nTP] & 1), (int)auIPV4AssocDataTable[nTP], 0,
                             uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP,
                                  (( pc3hppcTestInfo->bEML144Enable ) ? C3HPPC_TMU_TEST1__EML_TMU_PROGRAM :
                                                                        C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM ),
                                  C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                  auIPV4BucketTable[nTP],
                                  C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }

    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__LPMIPV4_LRP_KEY_PROGRAM,
                                  C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1 );

    if ( pc3hppcTestInfo->bEML144Enable == 0 ) {
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 0, 0, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 0, 1, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
    }

    sal_free( pRPBcommands );
    sal_free( pBBXcommands );
    sal_free( pBRRcommands );
    sal_free( pIPV4BucketTable );
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV4AssocDataTable[nTP] );
    }

  } else if ( pc3hppcTestInfo->bIPV4Enable && pc3hppcTestInfo->bTapsUnified == 1 ) {

    auIPV4BucketTable[0] = uEMC_NextTable + 1;
    uIPV4BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV4BucketPrefixNum ); 
    uIPV4BucketEntriesPerRow = C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW;
    uIPV4BucketTableEntryNum = C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM;
    auIPV4AssocDataTable[0] = auIPV4BucketTable[0] + 1;
    uIPV4AssocDataTableEntryNum = uIPV4BucketTableEntryNum * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntriesPerRow = uIPV4BucketEntriesPerRow * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntrySizeIn64b = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b; 

    nTP = 0;
    uDeadlineOffset = 0;
    uColumnOffset = 0;
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET,
                            uIPV4BucketTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, uColumnOffset,
                            uIPV4BucketEntriesPerRow, uDeadlineOffset, auIPV4AssocDataTable[nTP], 0, 0,
                            pc3hppcTestInfo->nIPV4BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );
  
    pIPV4BucketTable = (uint32 *) sal_alloc( (uIPV4BucketTableEntryNum * uIPV4BucketEntrySizeIn64b * sizeof(uint64)),
                                              "LPM IPV4 Bucket Table");
    sal_memset( pIPV4BucketTable, 0, (uIPV4BucketTableEntryNum * uIPV4BucketEntrySizeIn64b * sizeof(uint64)) );

    uDeadlineOffset = 64;
    uColumnOffset = 0;
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4AssocDataTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_ASSOC_DATA,
                            uIPV4AssocDataTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, uColumnOffset,
                            uIPV4AssocDataEntriesPerRow, uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
    apIPV4AssocDataTable[nTP] = (uint32 *) sal_alloc( (uIPV4AssocDataTableEntryNum *
                                                       uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)), "LPM IPV4 AssocData Table");
    sal_memset( apIPV4AssocDataTable[nTP], 0,
                (uIPV4AssocDataTableEntryNum * uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)) );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                     C3HPPC_TMU_TEST1__LPMIPV4_RPB_PREFIX_SIZE,
                                     C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM, 
                                     C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT_BASE,
                                     C3HPPC_TMU_TEST1__LPMIPV4_BBX_PREFIX_SIZE,
                                     (2 * C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM * pc3hppcTestInfo->nIPV4BucketPrefixNum),
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV4 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV4 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                  C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV4 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_unified_lpmipv4_table_contents( pc3hppcTestInfo->nSetupOptions,
                                                       pc3hppcTestInfo->nIPV4BucketPrefixNum, auIPV4BucketTable[0],
                                                       (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM),
                                                       C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                                       C3HPPC_TMU_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM,
                                                       C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM,
                                                       C3HPPC_TMU_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                                       pRPBcommands, pBBXcommands, pBRRcommands, pIPV4BucketTable, apIPV4AssocDataTable[0]);

    cli_out("\nINFO:  Initializing TAPS Unified IPv4 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM), pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS Unified IPv4 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                            C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS Unified IPv4 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV4_UNIFIED_FACTOR * C3HPPC_TMU_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                            C3HPPC_TMU_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    nTP = 0;

    cli_out("\nINFO:  Initializing TAPS Unified IPv4 Dram Bucket Table ...\n");
    c3hppc_tmu_xl_write( (int)(auIPV4BucketTable[nTP] & 1), (int)auIPV4BucketTable[nTP], 0,
                         uIPV4BucketTableEntryNum, 0, pIPV4BucketTable );
    if ( g_bDoInitXLreads ) {
      c3hppc_tmu_xl_read(  (int)(auIPV4BucketTable[nTP] & 1), (int)auIPV4BucketTable[nTP], 0,
                           uIPV4BucketTableEntryNum, 0, pIPV4BucketTable );
    }

    cli_out("\nINFO:  Initializing TAPS Unified IPV4 Dram Associated Data Table ...\n");
    c3hppc_tmu_xl_write( (int)(auIPV4AssocDataTable[nTP] & 1), (int)auIPV4AssocDataTable[nTP], 0,
                         uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
    if ( g_bDoInitXLreads ) {
      c3hppc_tmu_xl_read(  (int)(auIPV4AssocDataTable[nTP] & 1), (int)auIPV4AssocDataTable[nTP], 0,
                           uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
    }

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_UNIFIED_IPV4,
                                  auIPV4BucketTable[0],
                                  C3HPPC_TMU_TEST1__LPMIPV4_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }

    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__LPMIPV4_LRP_KEY_PROGRAM,
                                  C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1 );

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 0, 0, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 0, 1, C3HPPC_TMU_TEST1__LPMIPV4_TMU_PROGRAM );

    sal_free( pRPBcommands );
    sal_free( pBBXcommands );
    sal_free( pBRRcommands );
    sal_free( pIPV4BucketTable );
    sal_free( apIPV4AssocDataTable[0] );

  } else {
    auIPV4AssocDataTable[1] = uEMC_NextTable;
  }  /* if ( pc3hppcTestInfo->bIPV4Enable ) { */



  /****************************************************************************************************************************
   *
   * LPM IPV6 setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->bIPV6Enable ) {

    auIPV6BucketTable[0] = auIPV4AssocDataTable[1] + 1;
    auIPV6BucketTable[1] = auIPV6BucketTable[0] + 2;
    uIPV6BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv6_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV6BucketPrefixNum ); 
    uIPV6BucketEntriesPerRow = C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV6KeyAndAssocDataTable[0] = auIPV6BucketTable[0] + 1;
    auIPV6KeyAndAssocDataTable[1] = auIPV6KeyAndAssocDataTable[0] + 2;
    uIPV6KeyAndAssocDataTableEntryNum = C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntriesPerRow = uIPV6BucketEntriesPerRow * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntrySizeIn64b = C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_64b; 

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV6BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV6_BUCKET,
                              C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV6BucketEntriesPerRow, uDeadlineOffset, auIPV6KeyAndAssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV6BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );
  
      if ( nTP == 0 ) {
        pIPV6BucketTable = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * uIPV6BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV6 Bucket Table");
        sal_memset( pIPV6BucketTable, 0, (C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * uIPV6BucketEntrySizeIn64b * sizeof(uint64)) );
      }

      uDeadlineOffset = 64;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV6KeyAndAssocDataTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV6_ASSOC_DATA,
                              uIPV6KeyAndAssocDataTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV6KeyAndAssocDataEntriesPerRow, uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
      apIPV6KeyAndAssocDataTable[nTP] = (uint32 *) sal_alloc( (uIPV6KeyAndAssocDataTableEntryNum *
                                                         uIPV6KeyAndAssocDataEntrySizeIn64b * sizeof(uint64)), "LPM IPV6 AssocData Table");
      sal_memset( apIPV6KeyAndAssocDataTable[nTP], 0,
                  (uIPV6KeyAndAssocDataTableEntryNum * uIPV6KeyAndAssocDataEntrySizeIn64b * sizeof(uint64)) );

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT,
                                     C3HPPC_TMU_TEST1__LPMIPV6_RPB_PREFIX_SIZE,
                                     C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM, C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT_BASE,
                                     C3HPPC_TMU_TEST1__LPMIPV6_BBX_PREFIX_SIZE,
                                     (2 * C3HPPC_TMU_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM * pc3hppcTestInfo->nIPV6BucketPrefixNum),
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV6 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV6 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM *
                                          C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV6 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    if ( pc3hppcTestInfo->bIPV6CollisionEnable ) {
      c3hppc_test__setup_lpmipv6_table_contents_with_collisions( C3HPPC_TMU_TAPS_INSTANCE_NUM, pc3hppcTestInfo->nSetupOptions,
                                                                 pc3hppcTestInfo->nIPV6BucketPrefixNum, auIPV6BucketTable[0],
                                                                 C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM, C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT,
                                                                 C3HPPC_TMU_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM,
                                                                 C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM,
                                                                 C3HPPC_TMU_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                                                 pRPBcommands, pBBXcommands, pBRRcommands,
                                                                 pIPV6BucketTable, apIPV6KeyAndAssocDataTable, 0 );
    } else {
      c3hppc_test__setup_lpmipv6_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, pc3hppcTestInfo->nSetupOptions,
                                                 pc3hppcTestInfo->nIPV6BucketPrefixNum, auIPV6BucketTable[0],
                                                 C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM, C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT,
                                                 C3HPPC_TMU_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM,
                                                 C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM,
                                                 C3HPPC_TMU_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                                 pRPBcommands, pBBXcommands, pBRRcommands, pIPV6BucketTable, apIPV6KeyAndAssocDataTable );
    }

    cli_out("\nINFO:  Initializing TAPS IPV6 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV6 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV6 BBR memories ...\n");
    c3hppc_tmu_taps_write( 1,
                           (C3HPPC_TMU_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_TMU_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      /* The "tmu_test1_1xdm_eml144_ipv6_emc64" testcase assigns sub-key1 for IPv6. */
      if ( nTP == 0 && pc3hppcTestInfo->bEML144Enable == 1 ) continue;

      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( (int)(auIPV6BucketTable[nTP] & 1), (int)auIPV6BucketTable[nTP], 0,
                           C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, 0, pIPV6BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  (int)(auIPV6BucketTable[nTP] & 1), (int)auIPV6BucketTable[nTP], 0,
                             C3HPPC_TMU_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, 0, pIPV6BucketTable );
      }

      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( (int)(auIPV6KeyAndAssocDataTable[nTP] & 1), (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                           uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  (int)(auIPV6KeyAndAssocDataTable[nTP] & 1), (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                             uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP,
                                  (( pc3hppcTestInfo->bEML144Enable ) ? C3HPPC_TMU_TEST1__EML_TMU_PROGRAM : 
                                                                        C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM ),
                                  C3HPPC_TMU_LOOKUP__TAPS_IPV6,
                                  auIPV6BucketTable[nTP],
                                  C3HPPC_TMU_TEST1__LPMIPV6_SEGMENT,
                                  ((nTP == 0) ? 0 : 40), 18, 0, 0, 0 );
    }

    c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, C3HPPC_TMU_TEST1__LPMIPV6_LRP_KEY_PROGRAM,
                                  C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1 );

    if ( pc3hppcTestInfo->bEML144Enable == 0 ) {
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 0, 0, C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM );
      c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 0, 1, C3HPPC_TMU_TEST1__LPMIPV6_TMU_PROGRAM );
    }

    sal_free( pRPBcommands );
    sal_free( pBBXcommands );
    sal_free( pBRRcommands );
    sal_free( pIPV6BucketTable );
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV6KeyAndAssocDataTable[nTP] );
    }

  } /* if ( pc3hppcTestInfo->bIPV6Enable ) { */






  /****************************************************************************************************************************
   * Wait for Updater operations to complete before advancing to enable traffic.
   *****************************************************************************************************************************/
  nTimeOut = ( pc3hppcTestInfo->bIPV4Enable || pc3hppcTestInfo->bIPV6Enable ) ? 1800 : 600;
  if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,nTimeOut) == 0 ) {

    cli_out("ERROR:  Initial setup Updater TIMEOUT failure!\n");
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else {


    /****************************************************************************************************************************
     * If doing HW allocation then dump/display EML tables and verify their integrity.
     *   NOTE:  No longer doing this in emulation as it takes way too long.  If this condition is removed then be sure
     *          to remove the condition surrounding the calls to c3hppc_tmu_xl_read() which dump the tables to be checked.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->bEMLInsertEnable && !SAL_BOOT_QUICKTURN && !bEML_MultiTableSetup ) {
      if ( c3hppc_tmu_display_andor_scoreboard_eml_tables( pc3hppcTestInfo->nUnit, 
                                                           g_bDumpTmuDebugInfo, "EML_TableDump_Initial",
                                                           uEML_RootTable, uEML_ChainTable,
                                                           1, pc3hppcTestInfo->nMaxKey ) ) { 
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }

    /****************************************************************************************************************************
     * Setting up uncorrectable errors in the DM/EML tables.  When this feature was added to this test the CI had a limitation
     * that host access could not be done with lookup traffic.  Therefore the UE injection was placed here.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nErrorInject ) { 

      nUeCi = sal_rand() % C3HPPC_TMU_DRAM_INSTANCE_NUM;
      CiMemAccAddr.value = 0;
      CiMemAccAddr.bits.Bank = sal_rand() % C3HPPC_TMU_BANK_NUM;
      uRegionRowOffset = ( sal_rand() % C3HPPC_TMU_PER_BANK_REGION_BASE_OFFSET_NUM ) * C3HPPC_TMU_ROWS_PER_REGION_PER_BANK;

      CiMemAccAddr.bits.Row = uRegionRowOffset;
      nTable = sal_rand() % (pc3hppcTestInfo->nActiveDmNum ? pc3hppcTestInfo->nActiveDmNum : 1);
      CiMemAccAddr.bits.Column = 2 * nTable;
      c3hppc_tmu_ci_read_write( pc3hppcTestInfo->nUnit, nUeCi, 16, 0, CiMemAccAddr.value, auCiMemAccEntryData );
      for ( nIndex = 0; nIndex < 4; ++nIndex ) {
        if ( nIndex == 0 ) {
          uUeEntryAddr = ( auCiMemAccEntryData[nIndex] & 0xfff );
          uDynamicUpdateControl = 0x80000000 | (nTable << 16) | uUeEntryAddr;
          c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, uDynamicUpdateControl );
          cli_out("\nINFO:  UE Injected on DM%d table entry address --> 0x%03x\n", nTable, uUeEntryAddr );
        }
        cli_out("<c3hppc_tmu_test1__run> -- CI%d Addr[0x%08x] Word[%d] --> 0x%08x \n", 
                nUeCi, CiMemAccAddr.value, nIndex, auCiMemAccEntryData[nIndex] );
      }
      auCiMemAccEntryData[0] ^= 0x3;
      c3hppc_tmu_ci_read_write( pc3hppcTestInfo->nUnit, nUeCi, 16, 1, CiMemAccAddr.value, auCiMemAccEntryData );

      /* Go to chain when divisible by 32 else hit the root */
      nTable = ( uUeEntryAddr % 32 ) ? 0 : 1;
      CiMemAccAddr.bits.Row = uRegionRowOffset + 1 + nTable;
      CiMemAccAddr.bits.Column = 0;
      c3hppc_tmu_ci_read_write( pc3hppcTestInfo->nUnit, nUeCi, 32, 0, CiMemAccAddr.value, auCiMemAccEntryData );
      for ( nIndex = 0; nIndex < 8; ++nIndex ) {
        cli_out("<c3hppc_tmu_test1__run> -- CI%d Addr[0x%08x] Word[%d] --> 0x%08x \n", 
                nUeCi, CiMemAccAddr.value, nIndex, auCiMemAccEntryData[nIndex] );
      }
      cli_out("\nINFO:  UE Injected on EML %s table entry address --> 0x%03x\n", 
              ((nTable == 0) ? "ROOT" : "CHAIN"), uUeEntryAddr );
      auCiMemAccEntryData[0] ^= 0x3;
      c3hppc_tmu_ci_read_write( pc3hppcTestInfo->nUnit, nUeCi, 32, 1, CiMemAccAddr.value, auCiMemAccEntryData );

    }


    /****************************************************************************************************************************
     * Activate performance monitor utility.  The PM logic has a bug whereby if activated while traffic is running a few bogus
     * samples are captured which need to be discarded when analyzing the data.
     *****************************************************************************************************************************/
    c3hppc_tmu_pm_activate( pc3hppcTestInfo->nUnit, 0, g_uPmBucketShift, 0 );

    /* Clear counters before accumulating ... */
    c3hppc_tmu_collect_cache_hit_counts( pc3hppcTestInfo->nUnit, 1 );
    c3hppc_tmu_collect_cache_miss_counts( pc3hppcTestInfo->nUnit, 1 );
    c3hppc_tmu_collect_cache_hit_for_pending_counts( pc3hppcTestInfo->nUnit, 1 );



    /****************************************************************************************************************************
     * Enable lookups ...
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable ) {
        /* coverity[stack_use_overflow] */
      c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 0, 0, 0 );
    }

    READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
    uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
    COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, 
                    COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
    COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, C3HPPC_TMU_TEST1__STREAM_NUM * uContexts);
    if ( pc3hppcTestInfo->nPeriodic ) {
      COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, 16);
    } else {
      COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, 1);
    }
    {uint64 uuVal;
      COMPILER_64_SET(uuVal, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
      COMPILER_64_ADD_64(uuVal, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
      c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuVal);
    }


    /****************************************************************************************************************************
     * Check for the proper detection/handling of the CI injected uncorrectable errors (done above).
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nErrorInject ) { 

      bDmUEseen = 0; 
      bEmlSK0UEseen = 0; 
      bEmlSK1UEseen = 0;
      while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) && (!bDmUEseen || !bEmlSK0UEseen || !bEmlSK1UEseen) ) {

        if ( !bDmUEseen ) {
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 33 );
          if ( uReg ) {
            cli_out("\nINFO:  DM UE seen!\n");
            bDmUEseen = 1;
          }
        }
        if ( !bEmlSK0UEseen ) {
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 34 );
          if ( uReg ) {
            cli_out("\nINFO:  EML SK0 UE seen!\n");
            bEmlSK0UEseen = 1;
          }
        }
        if ( !bEmlSK1UEseen ) {
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 35 );
          if ( uReg ) {
            cli_out("\nINFO:  EML SK1 UE seen!\n");
            bEmlSK1UEseen = 1;
          }
        }

      }

    }  /* if ( pc3hppcTestInfo->nErrorInject ) */



    /****************************************************************************************************************************
     * The following sequence verifies clean transitioning of table entries in the presence of traffic.  A handshake occurs 
     * between this sequence and the LRP microcode to oversee a table entry changing to a new value and being restored to the 
     * original.  The microcode verifies no "bounce" during the transitions.
     *****************************************************************************************************************************/
    uUpdateCounter = 0;
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE ) { 

      if (soc_sbx_div64(pc3hppcTestInfo->uuIterations, 500, &uUpdateNum) == SOC_E_PARAM) return SOC_E_INTERNAL;
      u1KblockNum = C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM / 1024;

      for ( uUpdateCounter = 1; uUpdateCounter <= uUpdateNum && !c3hppc_is_test_done(pc3hppcTestInfo->nUnit); ++uUpdateCounter ) {

        nTable = sal_rand() % pc3hppcTestInfo->nActiveDmNum;
        nTableIndex = (1024 * (sal_rand() % u1KblockNum)) + (sal_rand() % (128 * uContexts));
        c3hppc_test__setup_dm_table_contents( g_uDmLookUp, nTable, C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM, 0x11100000, pDmTableDmaSratchPad );
        uDynamicUpdateControl = 0x80000000 | (nTable << 16) | nTableIndex;
        c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, uDynamicUpdateControl );
        cli_out("\nINFO:  Initiated Dynamic Update #%d --> 0x%08x\n", uUpdateCounter, uDynamicUpdateControl );
        pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] |= 0x55500000;
        c3hppc_tmu_xl_write( (nTable & 1), nTable, nTableIndex, 1,
                             0, &pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] );

        uDynamicUpdateControl |= 0x40000000;
        nTimeOut = 60;
        while ( --nTimeOut ) {
          sal_sleep(1);
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 32 );
          if ( uReg == uDynamicUpdateControl ) {
            cli_out("\nINFO:  New value seen for Dynamic Update #%d --> 0x%08x\n", uUpdateCounter, uDynamicUpdateControl );
            uDynamicUpdateControl |= 0x20000000;
            c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, uDynamicUpdateControl );
            pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] &= 0xbbbfffff;
            c3hppc_tmu_xl_write( (nTable & 1), nTable, nTableIndex, 1,
                                 0, &pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] );
            cli_out("\nINFO:  Initiated Dynamic Update #%d to original value --> 0x%08x\n", uUpdateCounter, uDynamicUpdateControl );
            break;
          }
        }

        if ( nTimeOut == 0 ) {
          cli_out("ERROR:  New value NOT seen for Dynamic Update #%d --> 0x%08x, last read --> 0x%08x\n",
                  uUpdateCounter, uDynamicUpdateControl, uReg );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          c3hppc_tmu_xl_read( (nTable & 1), nTable, nTableIndex, 1,
                              0, &pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] );
          break;
        } else {
          uDynamicUpdateControl |= 0x10000000;
          nTimeOut = 60;
          while ( --nTimeOut ) {
            sal_sleep(1);
            uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 32 );
            if ( uReg == uDynamicUpdateControl ) {
              cli_out("\nINFO:  Original value seen for Dynamic Update #%d --> 0x%08x\n", uUpdateCounter, uDynamicUpdateControl );
              uDynamicUpdateControl = 0;
              c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, uDynamicUpdateControl );
              break;
            }
          }
          if ( nTimeOut == 0 ) {
            cli_out("ERROR:  Original value not seen for Dynamic Update #%d --> 0x%08x, last read --> 0x%08x\n",
                    uUpdateCounter, uDynamicUpdateControl, uReg );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
            c3hppc_tmu_xl_read( (nTable & 1), nTable, nTableIndex, 1,
                                0, &pDmTableDmaSratchPad[uDmEntrySizeIn32b*nTableIndex] );
            break;
          }
        }

      }

    }



    /****************************************************************************************************************************
     * Table contention noise.  No value changes just contention.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_CONTENTION ) { 

      while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) { 
        nTableIndex = C3HPPC_MIN(pc3hppcTestInfo->nMaxKey,C3HPPC_TMU_TEST1__DM_TABLE_ENTRY_NUM);
        for ( nTable = 0; nTable < pc3hppcTestInfo->nActiveDmNum; ++nTable ) {
          c3hppc_test__setup_dm_table_contents( g_uDmLookUp, nTable, nTableIndex, 0x11100000, pDmTableDmaSratchPad );
          c3hppc_tmu_xl_write( (nTable & 1), nTable, 0, nTableIndex,
                               0, pDmTableDmaSratchPad );
          if ( g_bDoXLreads ) {
            c3hppc_tmu_xl_read(  (nTable & 1), nTable, 0, nTableIndex,
                                 0, pDmTableDmaSratchPad );
          }
        }

        if ( pc3hppcTestInfo->bEML64Enable && !pc3hppcTestInfo->bEMLInsertEnable ) {
          nTableIndex = C3HPPC_MIN(pc3hppcTestInfo->nMaxKey,uEML_RootTableEntryNum);
          c3hppc_tmu_xl_write( (int)(uEML_RootTable & 1), (int)uEML_RootTable, 0,
                               nTableIndex, 0, pEML_RootTable );
          if ( g_bDoXLreads ) {
            c3hppc_tmu_xl_read(  (int)(uEML_RootTable & 1), (int)uEML_RootTable, 0,
                                 nTableIndex, 0, pEML_RootTable );
          }

          nTableIndex = C3HPPC_MIN(pc3hppcTestInfo->nMaxKey,uEML_RootTableEntryNum);
          c3hppc_tmu_xl_write( (int)(uEML_ChainTable & 1), (int)uEML_ChainTable, 0,
                               nTableIndex, 0, pEML_ChainTable );
          if ( g_bDoXLreads ) {
            c3hppc_tmu_xl_read(  (int)(uEML_ChainTable & 1), (int)uEML_ChainTable, 0,
                                 nTableIndex, 0, pEML_ChainTable );
          }
        }

        if ( pc3hppcTestInfo->bEMC64Enable ) {
          nTableIndex = C3HPPC_MIN(pc3hppcTestInfo->nMaxKey,C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM);
          c3hppc_tmu_xl_write( (int)(uEMC_RootTable & 1), (int)uEMC_RootTable, 0,
                               nTableIndex, 0, pEMC64RootTable );
          if ( g_bDoXLreads ) {
            c3hppc_tmu_xl_read(  (int)(uEMC_RootTable & 1), (int)uEMC_RootTable, 0,
                                 nTableIndex, 0, pEMC64RootTable );
          }

          c3hppc_tmu_xl_write( (int)(uEMC_NextTable & 1), (int)uEMC_NextTable, 0,
                               nTableIndex, 0, pEMC64NextTable );
          if ( g_bDoXLreads ) {
            c3hppc_tmu_xl_read(  (int)(uEMC_NextTable & 1), (int)uEMC_NextTable, 0,
                                 nTableIndex, 0, pEMC64NextTable );
          }
        }

        if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,600) == 0 ) {
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          cli_out("ERROR:  Background XL Write/Read FAILURE\n" );
          break;
        }

      }

    }



    /****************************************************************************************************************************
     * EML chain destruction/construction inducing de-allocation and re-allocation of table entries.
     * Added "pEML_InsertList" due to a coverity issue.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING && pEML_InsertList ) {
      uint64 uuTmp;

      uInsertEntrySizeIn32b = 2 * c3hppc_tmu_get_eml_insert_cmd_entry_size_in_64b( g_auEmlFirstLookUp[0] );
      uKeySizeIn32b = 2 * c3hppc_tmu_get_eml_key_size_in_64b( g_auEmlFirstLookUp[0] );
      for ( nCmdFifoSelect = 0; nCmdFifoSelect < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifoSelect ) { 
        apKeyDeleteList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)), "Key Delete List");
        apKeyReInsertList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uInsertEntrySizeIn32b * sizeof(uint32)),
                                                  "ReInsert List");
        sal_memset( apKeyDeleteList[nCmdFifoSelect], 0x00, (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)) ); 
      }
      sal_memset( g_acKeyLookupMissScoreBoard, 0x00, sizeof(g_acKeyLookupMissScoreBoard) );

      nCmdFifoSelect = 0;

      COMPILER_64_SET(uuActionThreshold, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount),
                      COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
      COMPILER_64_UMUL_32(uuActionThreshold, 3);
      COMPILER_64_SHR(uuActionThreshold, 2);
      uuTmp = c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit );
      while ( COMPILER_64_LT(uuTmp, uuActionThreshold)  ) { 

#if 0
          cli_out("\n\n uuActionThreshold %lld   get_current_epoch_count %lld \n", uuActionThreshold, c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit ) );
#endif
        /*
           Bulk delete operations can only run on the "nCmdFifoSelect == 0" slot because the randomly 
           generated filter can not be easily modified to remove possbile collisions from the resultant delete list.
           The same key must NOT be present in both delete lists as this will cause results that are unpredictable.
        */ 
        nBulkDelete = 0;
        if ( nCmdFifoSelect == 0 && g_auEmlFirstLookUp[0] <= C3HPPC_TMU_LOOKUP__1ST_EML176 ) {
          if ( !(sal_rand() % 3) ) {
            nBulkDelete = ( sal_rand() % 10 ) ?  C3HPPC_TMU_TEST1__PHYSICAL : C3HPPC_TMU_TEST1__LOGICAL;
          }
        }

        if ( sal_rand() % 10 ) {
          uKeyFilterMask = sal_rand() % pc3hppcTestInfo->nMaxKey;
        } else {
          /* coverity[divide_by_zero] */
          uKeyFilterMask = (1 + (sal_rand() % 15)) << (sal_rand() % c3hppcUtils_ceil_power_of_2_exp(pc3hppcTestInfo->nMaxKey));
          if ( uKeyFilterMask >= pc3hppcTestInfo->nMaxKey ) {
            uKeyFilterMask >>= 3;
            uKeyFilterMask &= (pc3hppcTestInfo->nMaxKey - 1);
          }
        }
        if ( !uKeyFilterMask ) uKeyFilterMask = pc3hppcTestInfo->nMaxKey - 1;
        uKeyFilter = uKeyFilterMask;

        for ( nKey = 0, auKeyDeleteListLength[nCmdFifoSelect] = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
          if ( (pEML_InsertList[uInsertEntrySizeIn32b*nKey] & uKeyFilterMask) == uKeyFilter ) {
            if ( nCmdFifoSelect == 1 ) {
              for ( nKey1 = 0; nKey1 < auKeyDeleteListLength[0]; ++nKey1 ) {
                if ( apKeyDeleteList[0][uKeySizeIn32b*nKey1] == nKey ) break;
              }
            }
            if ( nCmdFifoSelect == 0 || nKey1 == auKeyDeleteListLength[0] ) {
              sal_memcpy( apKeyDeleteList[nCmdFifoSelect] + (uKeySizeIn32b*auKeyDeleteListLength[nCmdFifoSelect]),
                          pEML_InsertList + (uInsertEntrySizeIn32b*nKey),
                          uKeySizeIn32b * sizeof(uint32) );
              sal_memcpy( apKeyReInsertList[nCmdFifoSelect] + (uInsertEntrySizeIn32b*auKeyDeleteListLength[nCmdFifoSelect]),
                          pEML_InsertList + (uInsertEntrySizeIn32b*nKey),
                          uEML_InsertEntrySizeInBytes );
              g_acKeyLookupMissScoreBoard[nKey] = 1;
              ++auKeyDeleteListLength[nCmdFifoSelect];
            } 
          } 
        }

        if ( nBulkDelete ) {

          cli_out("\nINFO:  Bulk deleting %d EML table entries with KeyFilterMask[0x%04x] ...\n", 
                  auKeyDeleteListLength[nCmdFifoSelect], uKeyFilterMask);
          c3hppc_tmu_bulk_delete_setup( pc3hppcTestInfo->nUnit, (int)uEML_RootTable, &uKeyFilter, &uKeyFilterMask );
          if ( nBulkDelete == C3HPPC_TMU_TEST1__PHYSICAL ) {
            c3hppc_tmu_bulk_delete_start_scanner( pc3hppcTestInfo->nUnit );
            if ( c3hppc_tmu_wait_for_bulk_delete_done( pc3hppcTestInfo->nUnit, 5) ) {
              cli_out("\nERROR:  Bulk delete failed ...\n");
              pc3hppcTestInfo->nTestStatus = TEST_FAIL;
              break;
            } else {
              cli_out("\nINFO:  The bulk delete operation finished ...\n");
            }
          } else {
            sal_sleep( 20 );
          }
          c3hppc_tmu_bulk_delete_cancel( pc3hppcTestInfo->nUnit );

        } else {


          if ( auKeyDeleteListLength[nCmdFifoSelect] ) {
            cli_out("\nINFO:  Deleting %d EML table entries issued from CMD FIFO%d...\n",
                    auKeyDeleteListLength[nCmdFifoSelect], nCmdFifoSelect);
            c3hppc_tmu_eml_delete( nCmdFifoSelect, (int)uEML_RootTable, 
                                   auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect],
                                   C3HPPC_TMU_UPDATE_DELETE_OPTIONS__NONE );
            sal_sleep( 10 );
          }
        }

        if ( auKeyDeleteListLength[nCmdFifoSelect] ) {

          if ( nBulkDelete != C3HPPC_TMU_TEST1__LOGICAL ) {
            cli_out("\nINFO:  Verifying delete of EML table entries ...\n");
            c3hppc_tmu_eml_verify_delete( nCmdFifoSelect, (int)uEML_RootTable, 
                                          auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );

            cli_out("\nINFO:  Adding back the %d deleted EML table entries ...\n", auKeyDeleteListLength[nCmdFifoSelect] );
            c3hppc_tmu_eml_insert( nCmdFifoSelect, (int)uEML_RootTable, 
                                   auKeyDeleteListLength[nCmdFifoSelect], apKeyReInsertList[nCmdFifoSelect],
                                   C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE );
          }

          cli_out("\nINFO:  Verifying EML tables are fully populated  ...\n");
          c3hppc_tmu_eml_verify_insert( nCmdFifoSelect, (int)uEML_RootTable,
                                        auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );
        }


        if ( nCmdFifoSelect || nBulkDelete ) {
          if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,300) == 0 ) {
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
            break;
          }
          nCmdFifoSelect = 0;
        } else {
          nCmdFifoSelect = 1;
        }
        uuTmp = c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit );

      }

      if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,300) == 0 ) {
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      } else {
        if ( c3hppc_tmu_get_eml_tables(pc3hppcTestInfo->nUnit, nCmdFifoSelect, (int)uEML_RootTable, (int)uEML_ChainTable,
                                       uEML_RootTableEntryNum, 600) ) {
          cli_out("ERROR:  \"c3hppc_tmu_get_eml_tables\" TIMEOUT  FAILURE:  c3hppc_tmu_are_cmd_fifos_empty %d  c3hppc_tmu_are_rsp_fifos_empty %d\n",
                  c3hppc_tmu_are_cmd_fifos_empty(), c3hppc_tmu_are_rsp_fifos_empty() );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
        if ( c3hppc_tmu_display_andor_scoreboard_eml_tables( pc3hppcTestInfo->nUnit, 
                                                             g_bDumpTmuDebugInfo, "EML_TableDump_Final",
                                                             uEML_RootTable, uEML_ChainTable,
                                                             1, pc3hppcTestInfo->nMaxKey ) ) { 
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      }

      for ( nCmdFifoSelect = 0; nCmdFifoSelect < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifoSelect ) { 
        sal_free( apKeyDeleteList[nCmdFifoSelect] );
        sal_free( apKeyReInsertList[nCmdFifoSelect] ); 
      }

      cli_out("\nWaiting for the LRP \"offline\" event ....\n");

    }




/* JIRA CA3-2059
while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) { 
  CiMemAccAddr.value = 0;
  nUeCi = 0;
  c3hppc_tmu_ci_read_write( pc3hppcTestInfo->nUnit, nUeCi, 16, 0, CiMemAccAddr.value, auCiMemAccEntryData );
  for ( nIndex = 0; nIndex < 4; ++nIndex ) {
    cli_out("<c3hppc_tmu_test1__run> -- CI%d Addr[0x%08x] Word[%d] --> 0x%08x \n", 
            nUeCi, CiMemAccAddr.value, nIndex, auCiMemAccEntryData[nIndex] );
  }
}
*/


    /****************************************************************************************************************************
     * Wait for a "done" indication from the LRP.
     *****************************************************************************************************************************/
    c3hppc_wait_for_test_done( pc3hppcTestInfo->nUnit );
    cli_out("\nDetected the LRP \"offline\" event ...\n");



    /****************************************************************************************************************************
     * Ensure that there are no outstanding Updater operations.
     *****************************************************************************************************************************/
    if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,600) == 0 ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }


  }




  /****************************************************************************************************************************
   * Dump/display performance monitor statistics.  Set "nStartIndex" to '0' if histogram data is desired.
   *****************************************************************************************************************************/
  if ( g_bPerformanceTest && g_auEmlFirstLookUp[0] <= C3HPPC_TMU_LOOKUP__1ST_EML424 )
    nStartIndex = 0;
  else
    nStartIndex = ( pc3hppcTestInfo->nNumberOfCIs == C3HPPC_TMU_CI_INSTANCE_NUM ) ? 100 : 200;

  if ( pc3hppcTestInfo->nActiveDmNum ) {
    uint64 uuTmp;
    sal_free( pDmTableDmaSratchPad ); 

    COMPILER_64_ZERO(uuTotalSearches );
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 0, nStartIndex, g_uPmBucketShift, "DM0 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 1, nStartIndex, g_uPmBucketShift, "DM0 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    if ( g_bPerformanceTest && pc3hppcTestInfo->nActiveDmNum > 1 ) {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "DM1 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "DM1 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "DM2 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "DM2 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 6, nStartIndex, g_uPmBucketShift, "DM3 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 7, nStartIndex, g_uPmBucketShift, "DM3 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    }
    cli_out("\nINFO:  Total DM lookups --> %lld \n", uuTotalSearches );
    if ( g_bPerformanceTest ) {
      dMsps = (double) ( uuTotalSearches / c3hppc_test__get_test_duration() );
      dMsps /= 1000000.0;
      cli_out("\nINFO:  DM search rate --> %.3f Msps \n", dMsps );
    }
  }

  if ( g_auEmlFirstLookUp[0] <= C3HPPC_TMU_LOOKUP__1ST_EML424 ) {
    uint64 uuTmp;
    sal_free( pEML_RootTable ); 
    sal_free( pEML_ChainTable ); 

    COMPILER_64_ZERO(uuTotalSearches);
    if ( pc3hppcTestInfo->bIPV4Enable == 0 && pc3hppcTestInfo->bIPV6Enable == 0 ) {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "EML-SK0 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "EML-SK0 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "EML-SK1 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "EML-SK1 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    } else if ( pc3hppcTestInfo->bEML144Enable == 1 && (pc3hppcTestInfo->bIPV6Enable == 1 || pc3hppcTestInfo->bIPV4Enable == 1) ) {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "EML-SK0 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "EML-SK0 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    } else {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 6, nStartIndex, g_uPmBucketShift, "EML-SK0 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 7, nStartIndex, g_uPmBucketShift, "EML-SK0 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    }
    cli_out("\nINFO:  Total EML searches --> %lld \n", uuTotalSearches );
    if ( g_bPerformanceTest ) {
      dMsps = (double) ( uuTotalSearches / c3hppc_test__get_test_duration() );
      dMsps /= 1000000.0;
      cli_out("\nINFO:  EML search rate --> %.3f Msps \n", dMsps );
    }
  }

  if ( pc3hppcTestInfo->bEMC64Enable ) {
    sal_free( pEMC64RootTable ); 
    sal_free( pEMC64NextTable ); 

    c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 6, nStartIndex, g_uPmBucketShift, "EMC64 EVEN" );
    c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 7, nStartIndex, g_uPmBucketShift, "EMC64 ODD" );
  }

  if ( pc3hppcTestInfo->bIPV4Enable ) {
    uint64 uuTmp;

    COMPILER_64_ZERO(uuTotalSearches);
    if ( pc3hppcTestInfo->bEML144Enable == 0 ) {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "IPV4-SK0 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "IPV4-SK0 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    }
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "IPV4-SK1 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "IPV4-SK1 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    cli_out("\nINFO:  Total IPV4 searches --> %lld \n", uuTotalSearches );
    if ( g_bPerformanceTest ) {
      dMsps = (double) ( uuTotalSearches / c3hppc_test__get_test_duration() );
      dMsps /= 1000000.0;
      cli_out("\nINFO:  IPV4 search rate --> %.3f Msps \n", dMsps );
    }
  }

  if ( pc3hppcTestInfo->bIPV6Enable ) {
    uint64 uuTmp;

    COMPILER_64_ZERO(uuTotalSearches);
    if ( pc3hppcTestInfo->bEML144Enable == 0 ) {
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "IPV6-SK0 EVEN" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
      uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "IPV6-SK0 ODD" );
      COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    }
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "IPV6-SK1 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "IPV6-SK1 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
    if ( g_bPerformanceTest ) {
      dMsps = (double) ( uuTotalSearches / c3hppc_test__get_test_duration() );
      dMsps /= 1000000.0;
      cli_out("\nINFO:  IPV6 search rate --> %.3f Msps \n", dMsps );
    }
  }

  /* Dump counters before reporting accumulated values */
  c3hppc_tmu_collect_cache_hit_counts( pc3hppcTestInfo->nUnit, 0 );
  c3hppc_tmu_collect_cache_miss_counts( pc3hppcTestInfo->nUnit, 0 );
  c3hppc_tmu_collect_cache_hit_for_pending_counts( pc3hppcTestInfo->nUnit, 0 );

  cli_out("\nINFO:  Total Cache HITS --> %lld \n", c3hppc_tmu_get_cache_hit_count() );
  cli_out("\nINFO:  Total Cache MISSES --> %lld \n", c3hppc_tmu_get_cache_miss_count() );
  cli_out("\nINFO:  Total Cache HITS for pending requests --> %lld \n", c3hppc_tmu_get_cache_hit_for_pending_count() );


  if ( pc3hppcTestInfo->nErrorInject ) { 
    READ_TM_QE_ERRORr( pc3hppcTestInfo->nUnit, nUeCi, &uReg );
    uUE = soc_reg_field_get( pc3hppcTestInfo->nUnit, TM_QE_ERRORr, uReg, DRAM_ECC_UNCOR_ERRORf );
    if ( uUE ) {
      cli_out("\nINFO:  DRAM UEs detected by QE%d \n", nUeCi );
      uReg = 0;
      soc_reg_field_set( pc3hppcTestInfo->nUnit, TM_QE_ERRORr, &uReg, DRAM_ECC_UNCOR_ERRORf, 1 );
      WRITE_TM_QE_ERRORr( pc3hppcTestInfo->nUnit, nUeCi, uReg ); 
    } 
  }

  return 0;
#endif
}

int
c3hppc_tmu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else
  uint32 uReg;
  int nErrorCnt;
  uint64 *pTotalLookupCount;
  uint64 *pMatchLookupCount;
  uint64 *pMissLookupCount;
  int nKey, nCounter;
  c3hppc_cmu_segment_info_t *pCmuSegment0Info, *pCmuSegment1Info;
  char sMessage[16];


  if ( pc3hppcTestInfo->bEMLInsertEnable && !g_bPerformanceTest ) {
    nErrorCnt = 0;
    pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
    pCmuSegment1Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
    for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
      uint64 uuTmp;
      pTotalLookupCount = pCmuSegment0Info->pSegmentPciBase + ((2 * nKey) + 1);
      pMatchLookupCount = pCmuSegment1Info->pSegmentPciBase + ((2 * nKey) + 1);
      pMissLookupCount =  pMatchLookupCount + (2 * pc3hppcTestInfo->nMaxKey);
      COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pMatchLookupCount), COMPILER_64_LO(*pMatchLookupCount));
      COMPILER_64_ADD_64(uuTmp, *pMissLookupCount);
      if ( COMPILER_64_NE(*pTotalLookupCount,uuTmp) ) {
        if ( nErrorCnt < 128 ) {
          cli_out("<c3hppc_tmu_test1__done> -- Lookup count MISCOMPARE for Key[0x%04x]  Actual: 0x%llx   Expect: 0x%llx \n",
                  nKey, uuTmp, *pTotalLookupCount );
        }
        ++nErrorCnt;
      }

      if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_TMU_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 
        if ( !COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 0 ) {
          cli_out("\nERROR: -- UNEXPECTED non-zero MISS Lookup count for Key[0x%04x]  Actual: 0x%llx \n",
                  nKey, *pMissLookupCount );
          ++nErrorCnt;
        }
        if ( COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 1 ) {
          cli_out("<c3hppc_tmu_test1__done> -- UNEXPECTED zero MISS Lookup count for Key[0x%04x]\n", nKey );
          ++nErrorCnt;
        }
      }
    }

    if ( nErrorCnt ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      cli_out("\n<c3hppc_tmu_test1__done> -- Total error count --> %d\n", nErrorCnt );
    }

    if ( g_bDumpTmuDebugInfo ) c3hppcUtils_enable_output_to_file( "/home/morrier/UNTOUCHED_KEYS" );
    for ( nKey = 0, nCounter = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
      if ( g_acKeyLookupMissScoreBoard[nKey] == 0 ) {
        if ( g_bDumpTmuDebugInfo ) {
            cli_out("0x%04x\n", nKey );
        }
        ++nCounter;
      }
    }
    if ( g_bDumpTmuDebugInfo ) c3hppcUtils_disable_output_to_file();

    cli_out("\n<c3hppc_tmu_test1__done> -- %d keys should have been left UN-TOUCHED ...\n", nCounter );

  }

/*
  uint64 uuLaunchCount, uuLaunchCountTotal, uuReturnCount;
  pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
  uuReturnCount = *(pCmuSegment0Info->pSegmentPciBase + 1);
  uuLaunchCount = *(pCmuSegment0Info->pSegmentPciBase + 3);
  uuLaunchCountTotal = uuLaunchCount;
  cli_out("\nINFO:  S0 launch count --> %lld return count --> %lld\n", uuLaunchCount, uuReturnCount );
  pCmuSegment1Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
  uuReturnCount = *(pCmuSegment1Info->pSegmentPciBase + 1);
  uuLaunchCount = *(pCmuSegment1Info->pSegmentPciBase + 3);
  uuLaunchCountTotal += uuLaunchCount;
  cli_out("\nINFO:  S1 launch count --> %lld return count --> %lld\n", uuLaunchCount, uuReturnCount );
  cli_out("\nINFO:  Total Key Launch count --> %lld \n", 2*uuLaunchCountTotal );
*/




  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 48 );
  if ( uReg ) {
    cli_out("\nERROR:  DM -- TSR Error indication received on FlowID --> 0x%03x\n", (uReg & 0x3ff) );
  }
  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 16 );
  if ( uReg ) {
    cli_out("\nERROR:  DM -- MISCOMPARE indication received on FlowID --> 0x%03x\n", (uReg & 0x3ff) );
  }


/*
  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 0 );
  cli_out("\nERROR:  UCODE DEBUG DATA  --> 0x%08x\n", uReg );
*/
  

  if ( pc3hppcTestInfo->bIPV4Enable ) strcpy( sMessage, "IPV4" );
  else if ( pc3hppcTestInfo->bIPV6Enable ) strcpy( sMessage, "IPV6" );
  else strcpy( sMessage, "EML" );

  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 49 );
  if ( uReg ) {
    cli_out("\nERROR:  TSR Error indication  --> 0x%08x\n", uReg );
    cli_out("\nERROR:  %s -- TSR Error indication received on FlowID --> 0x%03x\n", sMessage, (uReg & 0x3ff) );
  }
  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 17 );
  if ( uReg ) {
    cli_out("\nERROR:  %s -- MISCOMPARE indication received on FlowID --> 0x%03x\n", sMessage, (uReg & 0x3ff) );
  }

  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 50 );
  if ( uReg ) {
    cli_out("\nERROR:  EMC -- TSR Error indication received on FlowID --> 0x%03x\n", (uReg & 0x3ff) );
  }
  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 18 );
  if ( uReg ) {
    cli_out("\nERROR:  EMC -- MISCOMPARE indication received on FlowID --> 0x%03x\n", (uReg & 0x3ff) );
  }

  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 32 );
  if ( uReg ) {
    cli_out("\nINFO:  Dynamic Control Register --> 0x%08x\n", uReg );
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
#endif
}

#if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG)
void  setup_eml64_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                  uint32 *pRootTableContents, uint32 *pChainTableContents ) {

  int n, nEntry, nIndex, nChainIndex, nChainElem;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nRootTableEntrySizeIn32b, nChainElementEntrySizeIn32b;
  uint32 uNL_LE;

  nRootTableEntrySizeIn32b = 2 * C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b;
  nChainElementEntrySizeIn32b = 2 * C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b;

  COMPILER_64_ZERO(EMLcontrolWord.value); 
  if ( nSetupOptions == C3HPPC_TMU_TEST1__EML_INSERT ) {
    EMLcontrolWord.bits.NL_GT = 15;
    EMLcontrolWord.bits.NL_LE_bits3to1 = 7;
    EMLcontrolWord.bits.NL_LE_bit0 = 1;
  } else {
    EMLcontrolWord.bits.NextTable = uChainTable;
    EMLcontrolWord.bits.NL_GT = 0;
    EMLcontrolWord.bits.Splitter = 0x07ffffff;
  }

  if ( nSetupOptions == C3HPPC_TMU_TEST1__EML_INSERT ) {
    for ( nEntry = 0; nEntry < nMaxKey; ++nEntry ) {
      nChainIndex = nChainElementEntrySizeIn32b * nEntry;
      pChainTableContents[nChainIndex+0] = nEntry;
      pChainTableContents[nChainIndex+1] = 0x00000000;
      pChainTableContents[nChainIndex+2] = 0x11100000 | nEntry;
      pChainTableContents[nChainIndex+3] = 0x22200000 | nEntry;
      pChainTableContents[nChainIndex+4] = 0x33300000 | nEntry;
      pChainTableContents[nChainIndex+5] = 0x00700000 | nEntry;
    }
  } else {
    for ( nEntry = 0; nEntry < C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM; ++nEntry ) {
      nIndex = C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM * 6 * nEntry;
      for ( nChainElem = 0; nChainElem < C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM; ++nChainElem ) { 
        if ( (nChainElem + 1) == C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM ) {
          for ( n = 0; n < nChainElementEntrySizeIn32b; ++n ) {
            pChainTableContents[nIndex+n] = 0xeeeeeeee;
          }
        } else {
          pChainTableContents[nIndex+0] = 0x11100000 | (nChainElem << 16) | nEntry;
          pChainTableContents[nIndex+1] = 0x22200000 | (nChainElem << 16) | nEntry;
          pChainTableContents[nIndex+2] = 0x33300000 | (nChainElem << 16) | nEntry;
          pChainTableContents[nIndex+3] = 0x44400000 | (nChainElem << 16) | nEntry;
          pChainTableContents[nIndex+4] = 0x55500000 | (nChainElem << 16) | nEntry;
          pChainTableContents[nIndex+5] = 0x00700000 | (nChainElem << 16) | nEntry;
        }
        nIndex += nChainElementEntrySizeIn32b;
      }
    }
  }

  for ( nEntry = 0; nEntry < C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM; ++nEntry ) {
    nIndex = nRootTableEntrySizeIn32b * nEntry;

    if ( nSetupOptions == C3HPPC_TMU_TEST1__EML_INSERT ) {
      pRootTableContents[nIndex+0] = COMPILER_64_LO( EMLcontrolWord.value );
      pRootTableContents[nIndex+1] = COMPILER_64_HI( EMLcontrolWord.value );
      for ( n = 2; n <= (nRootTableEntrySizeIn32b-1); ++n ) {
        pRootTableContents[nIndex+n] = 0;
      }
    } else {
      EMLcontrolWord.bits.NextEntry = nEntry;
      EMLcontrolWord.bits.NL_LE_bits3to1 = 0;
      EMLcontrolWord.bits.NL_LE_bit0 = 1;
      if ( nSetupOptions ==  C3HPPC_TMU_TEST1__NO_ROOT_HITS || 
           (nEntry % 1024) == 0 || (nEntry % 512) == 0 || (nEntry % 256) == 0 || (nEntry % 32) == 0 ) {
        if ( (nEntry % 1024) == 0 ) nChainElem = 3;
        else if ( (nEntry % 512) == 0 ) nChainElem =  2;
        else if ( (nEntry % 256) == 0 ) nChainElem =  1;
        else nChainElem = 0;
        uNL_LE = (EMLcontrolWord.bits.NL_LE_bits3to1 << 1) | EMLcontrolWord.bits.NL_LE_bit0;
        uNL_LE += nChainElem;
        EMLcontrolWord.bits.NL_LE_bits3to1 = uNL_LE >> 1;
        EMLcontrolWord.bits.NL_LE_bit0 = uNL_LE & 1;
        nChainIndex = (C3HPPC_TMU_TEST1__EML64_CHAIN_TABLE_CHAIN_ELEMENT_NUM * 6 * nEntry) + (6 * nChainElem);
        pRootTableContents[nIndex+2] = nEntry;
        pRootTableContents[nIndex+3] = 0xdeadbeef;
        pChainTableContents[nChainIndex+0] = nEntry;
        pChainTableContents[nChainIndex+1] = 0x00000000;
        pChainTableContents[nChainIndex+2] = 0x11100000 | (nChainElem << 16) | nEntry;
        pChainTableContents[nChainIndex+3] = 0x22200000 | (nChainElem << 16) | nEntry;
        pChainTableContents[nChainIndex+4] = 0x33300000 | (nChainElem << 16) | nEntry;
        pChainTableContents[nChainIndex+5] = 0x00700000 | (nChainElem << 16) | nEntry;
      } else {
        pRootTableContents[nIndex+2] = nEntry;
        pRootTableContents[nIndex+3] = 0x00000000;
      }
      pRootTableContents[nIndex+0] = COMPILER_64_LO( EMLcontrolWord.value );
      pRootTableContents[nIndex+1] = COMPILER_64_HI( EMLcontrolWord.value );

      pRootTableContents[nIndex+4] = 0x111f0000 | nEntry;
      pRootTableContents[nIndex+5] = 0x222f0000 | nEntry;
      pRootTableContents[nIndex+6] = 0x333f0000 | nEntry;
      pRootTableContents[nIndex+7] = 0x007f0000 | nEntry;
    }

  }

  return;
}

void  setup_emc64_table_entry( uint32 u128Mode, uint32 uEntryKey, uint32 uIndex, uint32 *pTable ) {

  pTable[uIndex+0] = uEntryKey;
  pTable[uIndex+1] = 0x00000000;
  pTable[uIndex+2] = 0x888c0000 | uEntryKey;
  if ( u128Mode ) {
    pTable[uIndex+3] = 0x007c0000 | uEntryKey;
  } else {
    pTable[uIndex+3] = 0x999c0000 | uEntryKey;
    pTable[uIndex+4] = 0xaaac0000 | uEntryKey;
    pTable[uIndex+5] = 0x007c0000 | uEntryKey;
  }

  return;
}


void  setup_emc64_table_contents( int nSetupOptions, uint32 *pRootTableContents, uint32 *pNextTableContents,
                                  int nRootTable, int nNextTable ) {

  uint32 uEntry, uEntryKey, uIndex, uEntrySizeIn32b, uRootIndex, uNextIndex, u128Mode;
  int nRootTableSizePowerOf2, nRootTableHashAdjustSelect, nNextTableSizePowerOf2, nCuckooFlag;
  c3hppc_tmu_key_t auuKey;


  uEntrySizeIn32b = 2 * c3hppc_tmu_get_table_entry_size_in_64b( nRootTable );
  u128Mode = c3hppc_tmu_get_emc128mode();

  sal_memset( pRootTableContents, 0xff, (C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM * uEntrySizeIn32b * sizeof(uint32)) ); 
  sal_memset( pNextTableContents, 0xff, (C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM * uEntrySizeIn32b * sizeof(uint32)) );


  if ( nSetupOptions == C3HPPC_TMU_TEST1__DO_HASHING ) {

    nRootTableSizePowerOf2 = c3hppc_tmu_get_table_size( nRootTable );
    nRootTableHashAdjustSelect = c3hppc_tmu_get_hash_adjust_select( nRootTable );
    nNextTableSizePowerOf2 = c3hppc_tmu_get_table_size( nNextTable );
    sal_memset( auuKey, 0x00, sizeof(c3hppc_tmu_key_t) );


    for ( uEntry = 0; uEntry < C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM; ++uEntry ) {

      uEntryKey = uEntry; 
      COMPILER_64_SET(auuKey[0],0, uEntryKey);
      nCuckooFlag = 0;

      while ( 1 ) {
        uRootIndex = c3hppc_tmu_1stLookup_hash( auuKey, nRootTableHashAdjustSelect, nRootTableSizePowerOf2 ) * uEntrySizeIn32b; 
        uNextIndex = c3hppc_tmu_2ndEmcLookup_hash( auuKey, nNextTableSizePowerOf2 ) * uEntrySizeIn32b;
        if ( pRootTableContents[uRootIndex+1] ) {

          setup_emc64_table_entry( u128Mode, uEntryKey, uRootIndex, pRootTableContents );
          break;

        } else if ( pNextTableContents[uNextIndex+1] ) {

          setup_emc64_table_entry( u128Mode, uEntryKey, uNextIndex, pNextTableContents );
          break;

        } else {

/*
          cli_out("CUCKOO%d   CUCKOO%d   CUCKOO%d   CUCKOO%d   CUCKOO%d  \n", 
                  nCuckooFlag, nCuckooFlag, nCuckooFlag, nCuckooFlag, nCuckooFlag );
*/

          if ( (nCuckooFlag & 1) == 0 ) {
            COMPILER_64_SET(auuKey[0],0, pRootTableContents[uRootIndex+0]);
            setup_emc64_table_entry( u128Mode, uEntryKey, uRootIndex, pRootTableContents );
          } else {
            COMPILER_64_SET(auuKey[0],0, pNextTableContents[uNextIndex+0]);
            setup_emc64_table_entry( u128Mode, uEntryKey, uNextIndex, pNextTableContents );
          }
          uEntryKey = COMPILER_64_LO(auuKey[0]);
          ++nCuckooFlag;

        } 
      } 

    }

  } else {

    for ( uEntry = 0; uEntry < C3HPPC_TMU_TEST1__EMC64_TABLE_ENTRY_NUM; ++uEntry ) {

      uIndex = uEntry * uEntrySizeIn32b;

      if ( (sal_rand() % 2) ) {

        setup_emc64_table_entry( u128Mode, uEntry, uIndex, pRootTableContents );

      } else {

        setup_emc64_table_entry( u128Mode, uEntry, uIndex, pNextTableContents );

      }
    }
  }

  return;
}



void  setup_eml176_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                   uint32 *pRootTableContents, uint32 *pChainTableContents ) {

  int n, nEntry, nIndex, nChainIndex;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nRootTableEntrySizeIn32b, nChainElementEntrySizeIn32b;

  nRootTableEntrySizeIn32b = 2 * C3HPPC_TMU_EML176_ROOT_TABLE_ENTRY_SIZE_IN_64b;
  nChainElementEntrySizeIn32b = 2 * C3HPPC_TMU_EML176_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b;

  COMPILER_64_ZERO(EMLcontrolWord.value); 
  EMLcontrolWord.bits.NL_GT = 15;
  EMLcontrolWord.bits.NL_LE_bits3to1 = 7;
  EMLcontrolWord.bits.NL_LE_bit0 = 1;

  for ( nEntry = 0; nEntry < nMaxKey; ++nEntry ) {
    nChainIndex = nChainElementEntrySizeIn32b * nEntry;
    pChainTableContents[nChainIndex+0] = nEntry;
    pChainTableContents[nChainIndex+1] = 0x00000000;
    pChainTableContents[nChainIndex+2] = 0x00000000;
    pChainTableContents[nChainIndex+3] = 0x00000000;
    if ( nSetupOptions == 1 ) {
      /* EML 144b mode is enabled */
      pChainTableContents[nChainIndex+4] = 0x00008000;
      pChainTableContents[nChainIndex+5] = 0x00000000;
    } else {
      pChainTableContents[nChainIndex+4] = 0x00000000;
      pChainTableContents[nChainIndex+5] = 0x00008000;
    }
    pChainTableContents[nChainIndex+6] = 0x11100000 | nEntry;
    pChainTableContents[nChainIndex+7] = 0x22200000 | nEntry;
    pChainTableContents[nChainIndex+8] = 0x33300000 | nEntry;
    pChainTableContents[nChainIndex+9] = 0x00700000 | nEntry;
  }

  for ( nEntry = 0; nEntry < C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM; ++nEntry ) {
    nIndex = nRootTableEntrySizeIn32b * nEntry;
    pRootTableContents[nIndex+0] = COMPILER_64_LO( EMLcontrolWord.value );
    pRootTableContents[nIndex+1] = COMPILER_64_HI( EMLcontrolWord.value );
    for ( n = 2; n <= (nRootTableEntrySizeIn32b-1); ++n ) {
      pRootTableContents[nIndex+n] = 0;
    }
  }

  return;
}



void  setup_eml304_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                   uint32 *pRootTableContents, uint32 *pChainTableContents ) {

  int n, nEntry, nIndex, nChainIndex;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nRootTableEntrySizeIn32b, nChainElementEntrySizeIn32b;

  nRootTableEntrySizeIn32b = 2 * C3HPPC_TMU_EML304_ROOT_TABLE_ENTRY_SIZE_IN_64b;
  nChainElementEntrySizeIn32b = 2 * C3HPPC_TMU_EML304_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b;

  COMPILER_64_ZERO(EMLcontrolWord.value); 
  EMLcontrolWord.bits.NL_GT = 15;
  EMLcontrolWord.bits.NL_LE_bits3to1 = 7;
  EMLcontrolWord.bits.NL_LE_bit0 = 1;

  for ( nEntry = 0; nEntry < nMaxKey; ++nEntry ) {
    nChainIndex = nChainElementEntrySizeIn32b * nEntry;
    pChainTableContents[nChainIndex+0] = nEntry;
    pChainTableContents[nChainIndex+1] = 0x00000000;
    pChainTableContents[nChainIndex+2] = 0x00000000;
    pChainTableContents[nChainIndex+3] = 0x00000000;
    pChainTableContents[nChainIndex+4] = 0x00000000;
    pChainTableContents[nChainIndex+5] = 0x00000000;
    pChainTableContents[nChainIndex+6] = 0x00000000;
    pChainTableContents[nChainIndex+7] = 0x00000000;
    pChainTableContents[nChainIndex+8] = 0x00000000;
    pChainTableContents[nChainIndex+9] = 0x01000000;
    pChainTableContents[nChainIndex+10] = 0x11100000 | nEntry;
    pChainTableContents[nChainIndex+11] = 0x22200000 | nEntry;
    pChainTableContents[nChainIndex+12] = 0x33300000 | nEntry;
    pChainTableContents[nChainIndex+13] = 0x00700000 | nEntry;
  }

  for ( nEntry = 0; nEntry < C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM; ++nEntry ) {
    nIndex = nRootTableEntrySizeIn32b * nEntry;
    pRootTableContents[nIndex+0] = COMPILER_64_LO( EMLcontrolWord.value );
    pRootTableContents[nIndex+1] = COMPILER_64_HI( EMLcontrolWord.value );
    for ( n = 2; n <= (nRootTableEntrySizeIn32b-1); ++n ) {
      pRootTableContents[nIndex+n] = 0;
    }
  }

  return;
}



void  setup_eml424_table_contents( int nSetupOptions, int nMaxKey, uint32 uChainTable,
                                   uint32 *pRootTableContents, uint32 *pChainTableContents ) {

  int n, nEntry, nIndex, nChainIndex;
  c3hppc_tmu_eml_table_control_word_ut EMLcontrolWord;
  int nRootTableEntrySizeIn32b, nChainElementEntrySizeIn32b;

  nRootTableEntrySizeIn32b = 2 * C3HPPC_TMU_EML424_ROOT_TABLE_ENTRY_SIZE_IN_64b;
  nChainElementEntrySizeIn32b = 2 * C3HPPC_TMU_EML424_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b;

  COMPILER_64_ZERO(EMLcontrolWord.value); 
  EMLcontrolWord.bits.NL_GT = 15;
  EMLcontrolWord.bits.NL_LE_bits3to1 = 7;
  EMLcontrolWord.bits.NL_LE_bit0 = 1;

  for ( nEntry = 0; nEntry < nMaxKey; ++nEntry ) {
    nChainIndex = nChainElementEntrySizeIn32b * nEntry;
    pChainTableContents[nChainIndex+0] = nEntry;
    pChainTableContents[nChainIndex+1] = 0x00000000;
    pChainTableContents[nChainIndex+2] = 0x00000000;
    pChainTableContents[nChainIndex+3] = 0x00000000;
    pChainTableContents[nChainIndex+4] = 0x00000000;
    pChainTableContents[nChainIndex+5] = 0x00000000;
    pChainTableContents[nChainIndex+6] = 0x00000000;
    pChainTableContents[nChainIndex+7] = 0x00000000;
    pChainTableContents[nChainIndex+8] = 0x00000000;
    pChainTableContents[nChainIndex+9] = 0x00000000;
    pChainTableContents[nChainIndex+10] = 0x00000000;
    pChainTableContents[nChainIndex+11] = 0x00000000;
    pChainTableContents[nChainIndex+12] = 0x00000000;
    pChainTableContents[nChainIndex+13] = 0x00010000;
    pChainTableContents[nChainIndex+14] = 0x11100000 | nEntry;
    pChainTableContents[nChainIndex+15] = 0x22200000 | nEntry;
    pChainTableContents[nChainIndex+16] = 0x33300000 | nEntry;
    pChainTableContents[nChainIndex+17] = 0x00700000 | nEntry;
  }

  for ( nEntry = 0; nEntry < C3HPPC_TMU_TEST1__EML_ROOT_TABLE_ENTRY_NUM; ++nEntry ) {
    nIndex = nRootTableEntrySizeIn32b * nEntry;
    pRootTableContents[nIndex+0] = COMPILER_64_LO(EMLcontrolWord.value);
    pRootTableContents[nIndex+1] = COMPILER_64_HI(EMLcontrolWord.value);
    for ( n = 2; n <= (nRootTableEntrySizeIn32b-1); ++n ) {
      pRootTableContents[nIndex+n] = 0;
    }
  }

  return;
}
#endif /* #if defined(COMPILER_HAS_DOUBLE) && defined(COMPILER_HAS_LONGLONG) */
#endif /* #ifdef BCM_CALADAN3_SUPPORT */

