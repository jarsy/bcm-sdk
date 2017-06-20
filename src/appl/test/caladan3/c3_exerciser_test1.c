/*
 * $Id: c3_exerciser_test1.c,v 1.49 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/sbx/sbx_drv.h>

#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"

#include <soc/sbx/caladan3/ppe.h>
 
#define C3_EXERCISER_TEST1__STREAM_NUM                                (12)
#define C3_EXERCISER_TEST1__IL_STATS_COLLECTION_INTERVAL              (2000)

#define C3_EXERCISER_TEST1_RCE_RESULT_PORT                            (9)
#define C3_EXERCISER_TEST1_WDT_PORT                                   (0)
#define C3_EXERCISER_TEST1_WDT_LIST_MANAGER                           (0)
#define C3_EXERCISER_TEST1_WDT_LIST_BASE_OFFSET                       (0x00000001)
#define C3_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET       (0x0000c000)
#define C3_EXERCISER_TEST1_WDT_PORT_LRP_STORE_ACCESS_BASE_OFFSET      (0x0000d000)
#define C3_EXERCISER_TEST1_LRP_LEARNING_SEMAPHORE_PORT                C3_EXERCISER_TEST1_WDT_PORT 
#define C3_EXERCISER_TEST1_LRP_LEARNING_LIST_MANAGER                  (1)
#define C3_EXERCISER_TEST1_LRP_LEARNING_LIST_BASE_OFFSET              (0x00008000)
#define C3_EXERCISER_TEST1_LRP_LEARNING_LIST_SIZE                     (0x8000)


#define C3_EXERCISER_TEST1__PROGRAM_INVALID                           (0xffff)
#define C3_EXERCISER_TEST1__STREAMX_S0_LRP_KEY_PROGRAM_NUMBER         (0)
#define C3_EXERCISER_TEST1__STREAM1_S0_LRP_KEY_PROGRAM_NUMBER         (1)
#define C3_EXERCISER_TEST1__STREAM2_S0_LRP_KEY_PROGRAM_NUMBER         (13)
#define C3_EXERCISER_TEST1__STREAM3_S0_LRP_KEY_PROGRAM_NUMBER         (2)
#define C3_EXERCISER_TEST1__STREAM5_S0_LRP_KEY_PROGRAM_NUMBER         (5)
#define C3_EXERCISER_TEST1__STREAM9_S0_LRP_KEY_PROGRAM_NUMBER         (6)
#define C3_EXERCISER_TEST1__STREAMX_S1_LRP_KEY_PROGRAM_NUMBER         (3)
#define C3_EXERCISER_TEST1__STREAM1_S1_LRP_KEY_PROGRAM_NUMBER         (4)
#define C3_EXERCISER_TEST1__STREAM2_S1_LRP_KEY_PROGRAM_NUMBER         (7)
#define C3_EXERCISER_TEST1__STREAM3_S1_LRP_KEY_PROGRAM_NUMBER         (8)
#define C3_EXERCISER_TEST1__STREAM6_S1_LRP_KEY_PROGRAM_NUMBER         (9)
#define C3_EXERCISER_TEST1__STREAM7_S1_LRP_KEY_PROGRAM_NUMBER         (10)
#define C3_EXERCISER_TEST1__STREAM10_S1_LRP_KEY_PROGRAM_NUMBER        (11)
#define C3_EXERCISER_TEST1__STREAM11_S1_LRP_KEY_PROGRAM_NUMBER        (12)
#define C3_EXERCISER_TEST1__STREAM11_S2_LRP_KEY_PROGRAM_NUMBER        (14)
#define C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM               (0)
#define C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM               (1)

#define C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM                        (0x01000)
#define C3_EXERCISER_TEST1__DM_TABLE_ENTRIES_PER_ROW                  (1)

#define C3_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM                  (0x01000)

#define C3_EXERCISER_TEST1__MAX_KEYS                                  (0x10000)
#define C3_EXERCISER_TEST1__EML_MAX_KEYS                              (C3_EXERCISER_TEST1__MAX_KEYS) 
#define C3_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM                (2*C3_EXERCISER_TEST1__EML_MAX_KEYS)
#define C3_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRIES_PER_ROW          (32)
#define C3_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRY_NUM               C3_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM
#define C3_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRIES_PER_ROW         (4)
#define C3_EXERCISER_TEST1__EML64_TMU_PROGRAM                         (0)

#define C3_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM                     (C3_EXERCISER_TEST1__MAX_KEYS)
#define C3_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW               (16)
#define C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM                         (1)

#define C3_EXERCISER_TEST1__ETU_MAX_KEYS                              (C3_EXERCISER_TEST1__MAX_KEYS) 
#define C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0                  (0)
#define C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM                        (0)

#define C3_EXERCISER_TEST1__LPMIPV4_SEGMENT                           (0)
/* Values of 0 and 704 have been used */
#define C3_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE                      (0)
#define C3_EXERCISER_TEST1__LPMIPV4_FULLY_POPULATED_TCAM_FACTOR       (8)
#define C3_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM                     (C3_EXERCISER_TEST1__LPMIPV4_FULLY_POPULATED_TCAM_FACTOR * 256)
#define C3_EXERCISER_TEST1__LPMIPV4_RPB_PREFIX_SIZE                   (48)
#define C3_EXERCISER_TEST1__LPMIPV4_BBX_PREFIX_SIZE                   (48)
#define C3_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM                 (48)
#define C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM           (32)
#define C3_EXERCISER_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
/* The multiply by 2 is due to the bucket pair concept */
#define C3_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM            (C3_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * \
                                                                       C3_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM * 2)
#define C3_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW      (4)
#define C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM                       (4)

#define C3_EXERCISER_TEST1__LPMIPV6_SEGMENT                           (1)
#define C3_EXERCISER_TEST1__LPMIPV6_SEGMENT_BASE                      (C3_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE + \
                                                                       (C3_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM / 4))
#define C3_EXERCISER_TEST1__LPMIPV6_FULLY_POPULATED_TCAM_FACTOR       (2)
#define C3_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM                     (C3_EXERCISER_TEST1__LPMIPV6_FULLY_POPULATED_TCAM_FACTOR * 256)
#define C3_EXERCISER_TEST1__LPMIPV6_RPB_PREFIX_SIZE                   (144)
#define C3_EXERCISER_TEST1__LPMIPV6_BBX_PREFIX_SIZE                   (128)
#define C3_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM                 (72)
#define C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM           (32)
#define C3_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_MAX_PREFIX_NUM        (5)
#define C3_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
/* The multiply by 2 is due to the bucket pair concept */
#define C3_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM            (C3_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * \
                                                                       C3_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM * 2)
#define C3_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW      (4)
#define C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM                       (5)

#define C3_EXERCISER_TEST1__LRP_LEARNING_TMU_PROGRAM                  (6)


#define C3_EXERCISER_TEST1__UPDATER_TIMEOUT                     (20)

#define C3_EXERCISER_TEST1__TSR_ERROR_MASK                      (0x40000000)

#define C3_EXERCISER_TEST1__NO_ROOT_HITS                        (1) 
#define C3_EXERCISER_TEST1__EML_INSERT                          (2) 
#define C3_EXERCISER_TEST1__DO_HASHING                          (3) 
#define C3_EXERCISER_TEST1__LOGICAL                             (1) 
#define C3_EXERCISER_TEST1__PHYSICAL                            (2) 

#define C3_EXERCISER_TEST1__TABLE_CONTENTION                    (1)
#define C3_EXERCISER_TEST1__TABLE_STATE_CHANGE                  (2)
#define C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING    (4)




static c3hppc_test_dm_table_parameters_t
               g_aDmTableParameters[] = {
  /* lrp_segment/tmu_table_id = 128/0 */  { 0, 0, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM494,
                                            C3HPPC_LRP_LOOKUP__DM494, 0,  8, 1, 64 },
  /* lrp_segment/tmu_table_id = 129/1 */  { 1, 0, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 8,  2, 1, 64 },
  /* lrp_segment/tmu_table_id = 130/2 */  { 2, 1, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 10, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 131/3 */  { 3, 2, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 12, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 132/4 */  { 4, 3, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 14, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 133/5 */  { 5, 0, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 16, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 134/6 */  { 6, 2, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 20, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 135/7 */  { 7, 1, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 24, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 136/8 */  { 8, 0, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM366,
                                            C3HPPC_LRP_LOOKUP__DM366, 28, 6, 1, 64 },
  /* lrp_segment/tmu_table_id = 137/9 */  { 9, 1, C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM366,
                                            C3HPPC_LRP_LOOKUP__DM366, 34, 6, 1, 64 }
                                        };
static c3hppc_test_em_table_parameters_t
               g_aEmlTableParameters[] = {
                                          { 'R', 10, C3_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__1ST_EML64, 0, C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b,
                                            C3_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRIES_PER_ROW, 0, (uint32 *)NULL },
                                          { 'C', 11, C3_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__2ND_EML64, 0, 
                                            (C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM),
                                            C3_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRIES_PER_ROW, 64, (uint32 *)NULL }
                                         };
/* For EMC the "next" table entry must preceed the associated "root" entry !!!! */
static c3hppc_test_em_table_parameters_t
               g_aEmcTableParameters[] = {
                                          { 'N', 13, C3_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__2ND_EMC64, 0, C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b, 
                                            C3_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW, 64, (uint32 *)NULL },
                                          { 'R', 12, C3_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__1ST_EMC64, 0, C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b,
                                            C3_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW, 0, (uint32 *)NULL }
                                         };

static c3hppc_test_rce_program_parameters_t
               g_aRceProgramParameters[] = {
                                            { C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM, 0x0000,                         85, 32, 15, 0 },
                                            { C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM, (C3HPPC_RCE_INSTRUCTION_NUM/2), 42, 32, 14, 0 }
                                           };

static c3hppc_test_lrp_search_program_parameters_t
               g_aLrpSearchProgramParameters[] = {
                                                  { C3_EXERCISER_TEST1__STREAMX_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__EML64_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM1_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM3_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__EML64_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAMX_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM1_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM5_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3_EXERCISER_TEST1__STREAM9_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3_EXERCISER_TEST1__STREAM2_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM3_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM6_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM7_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM10_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM11_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_42x32_14BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM2_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3_EXERCISER_TEST1__RCE_85x32_15BIT_LPM_PROGRAM,
                                                    C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3_EXERCISER_TEST1__STREAM11_S2_LRP_KEY_PROGRAM_NUMBER,
                                                    C3_EXERCISER_TEST1__LRP_LEARNING_TMU_PROGRAM, 0, 0,
                                                    C3_EXERCISER_TEST1__PROGRAM_INVALID,
                                                    C3_EXERCISER_TEST1__PROGRAM_INVALID }
                                                 };



static uint32 g_uPmBucketShift = 2;

static uint8 g_bDoInitXLreads = 1;
static uint8 g_bDumpEmlTables = 0;
static uint8 g_bDumpTmuDebugInfo = 0;
static uint32 g_uUpdaterFreeChainFifoPoolMask = 0xf;

static char g_acKeyLookupMissScoreBoard[C3_EXERCISER_TEST1__MAX_KEYS];

static int g_nWatchDogTimerCopSegment;

static uint16 g_dev_id = 0;
static uint8 g_rev_id = 0;

int
c3_exerciser_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nInstance, nSegment;
  uint32 uResultsTimer;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  c3hppc_cop_watchdogtimer_state32b_entry_ut WatchDogTimer32bStateEntry;
  c3hppc_cop_watchdogtimer_state64b_entry_ut WatchDogTimer64bStateEntry;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nOcmPort, nDmaBlockSize, nTimer, nMode32;
  uint64 uuWatchDogTimerStateEntry;
  

  soc_cm_get_id( pc3hppcTestInfo->nUnit, &g_dev_id, &g_rev_id);
  
  pc3hppcTestInfo->BringUpControl.uDefaultPhysicalSegmentSizeIn16kBlocks = 4;
  pc3hppcTestInfo->BringUpControl.uDefaultCmuPhysicalSegmentSizeIn16kBlocks = 4; 
  if ( !SAL_BOOT_QUICKTURN ) {
    /* Need to run TURBO_64b counters for SV due to counter manager ring performance limitations. */
    /* For TURBO_64b need twice as much space in OCM to hold counters.                            */
    pc3hppcTestInfo->BringUpControl.uDefaultCmuPhysicalSegmentSizeIn16kBlocks *= 2;
  }
  pc3hppcTestInfo->bEMLInsertEnable = 1;
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  pc3hppcTestInfo->BringUpControl.uSwsBringUp = 1;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS ||
         pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL ||
         nInstance == C3HPPC_TEST__FABRIC_INTERFACE ) {  
      pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] = 1;
    }
  }
  pc3hppcTestInfo->BringUpControl.uTmuBringUp = ( pc3hppcTestInfo->bNoTmu == 1 ) ? 0 : 1;
  pc3hppcTestInfo->BringUpControl.uRceBringUp = ( pc3hppcTestInfo->bNoRce == 1 ) ? 0 : 1;
  pc3hppcTestInfo->BringUpControl.uEtuBringUp = ( pc3hppcTestInfo->bNoEtu == 1 ) ? 0 : 1;
  if ( pc3hppcTestInfo->bNoPpe == 1 ) {
    pc3hppcTestInfo->BringUpControl.bSwsBypassPpParsing = 1;
    pc3hppcTestInfo->BringUpControl.bMultiStreamUcode = 1;
  }
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->bIPV4Enable = 1;
  pc3hppcTestInfo->bIPV6Enable = 1;
  pc3hppcTestInfo->BringUpControl.bLrpLearningEnable = pc3hppcTestInfo->bLrpLearningEnable;

  
  if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp == 0 ) {

    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_notmu.oasm");

  } else if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 0 ) {

    if ( pc3hppcTestInfo->nNumberOfCIs == C3HPPC_TMU_CI_INSTANCE_NUM ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_noetu.oasm");
    } else {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_lt16ddrs_noetu.oasm");
    }

  } else if ( pc3hppcTestInfo->BringUpControl.uRceBringUp == 0 ) {

    /* coverity[secure_coding] */
    sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_norce.oasm");

  } else {

    if ( pc3hppcTestInfo->bLrpLearningEnable ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_lrplearn.oasm");
    } else if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_lt16ddrs.oasm");
    } else if ( pc3hppcTestInfo->nLrpFreq == 1100 ) {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1_lrp1100.oasm");
    } else {
      /* coverity[secure_coding] */
      sal_strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "c3_exerciser_test1.oasm");
    }

  }


  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
  pc3hppcTestInfo->BringUpControl.bTmuEMC128Mode = 1;
  pc3hppcTestInfo->BringUpControl.uTmuEmlMaxProvisionedKey = C3_EXERCISER_TEST1__MAX_KEYS;


  /* CMU counters for EML lookups */
  if ( pc3hppcTestInfo->BringUpControl.bTmuHwEmlChainManagement ) {
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[0].uSegmentLimit = pc3hppcTestInfo->nMaxKey - 1;
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[1].uSegmentLimit = 
                                    C3_EXERCISER_TEST1__MAX_KEYS + pc3hppcTestInfo->nMaxKey - 1;
  }

  /* CMU segments for per channel counts */
  for ( nSegment = 2; nSegment < 4; ++nSegment ) {
    pCmuSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[nSegment]);
    pCmuSegmentInfo->bValid = 1;
    pCmuSegmentInfo->uSegment = (uint32) nSegment;
    pCmuSegmentInfo->uSegmentType = ( SAL_BOOT_QUICKTURN ) ? C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b :
                                                             C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
    pCmuSegmentInfo->uSegmentLimit = C3HPPC_SWS_SOURCE_QUEUE_NUM - 1;
/*
pCmuSegmentInfo->uSegmentLimit = 2048;
*/
    pCmuSegmentInfo->uSegmentPort = nSegment & 1;
    pCmuSegmentInfo->nStartingPhysicalBlock = 63;
    pCmuSegmentInfo->uSegmentOcmBase = 
                     (uint32) pCmuSegmentInfo->nStartingPhysicalBlock << C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT;
  }

  g_nWatchDogTimerCopSegment = 1;
  for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nInstance][g_nWatchDogTimerCopSegment]);
    pCopSegmentInfo->uMode64 = 1;
    pCopSegmentInfo->bValid = 1;
    pCopSegmentInfo->uSegmentLimit = pc3hppcTestInfo->nCounterNum - 1;
    pCopSegmentInfo->uSegmentOcmLimit = (((1 + pCopSegmentInfo->uMode64) * pc3hppcTestInfo->nCounterNum) / 2) - 1;
    pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__TIMER;
    pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    pCopSegmentInfo->nStartingPhysicalBlock = ( nInstance ) ? 28 : 40;
    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod, 0, pc3hppcTestInfo->nCounterNum);
    COMPILER_64_UMUL_32(pCopSegmentInfo->uuRefreshVisitPeriod, 32);
    pCopSegmentInfo->uSegmentBase =
                       (uint32) pCopSegmentInfo->nStartingPhysicalBlock << C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT; 
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;


  WatchDogTimer32bStateEntry.bits.Started_1 = 1;
  WatchDogTimer32bStateEntry.bits.Started_0 = 1;
  WatchDogTimer32bStateEntry.bits.Interrupt_1 = 1;
  WatchDogTimer32bStateEntry.bits.Interrupt_0 = 1;
  WatchDogTimer64bStateEntry.bits.Started = 1;
  WatchDogTimer64bStateEntry.bits.Interrupt = 1;

  for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    /* Initialize timer state */
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nInstance][g_nWatchDogTimerCopSegment]);
    nMode32 = ( pCopSegmentInfo->uMode64 ) ? 0 : 1;
    nDmaBlockSize = pc3hppcTestInfo->nCounterNum / (1 + nMode32);
    pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                  nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                  "ocm_block");
    for ( nTimer = 0; nTimer <= (int) pCopSegmentInfo->uSegmentLimit; nTimer += (1+nMode32) ) {
      if ( nMode32 ) {
        WatchDogTimer32bStateEntry.bits.Timer_1 = (1 * (nTimer+1));
        WatchDogTimer32bStateEntry.bits.Timer_0 = (1 * (nTimer+2));
        COMPILER_64_SET(uuWatchDogTimerStateEntry, COMPILER_64_HI(WatchDogTimer32bStateEntry.value), COMPILER_64_LO(WatchDogTimer32bStateEntry.value));
      } else {
        WatchDogTimer64bStateEntry.bits.Timer = (1 * (nTimer+1));
        WatchDogTimer64bStateEntry.bits.Timeout = WatchDogTimer64bStateEntry.bits.Timer;
        uuWatchDogTimerStateEntry = WatchDogTimer64bStateEntry.value;
      }
      pOcmBlock[nTimer>>nMode32].uData[1] = COMPILER_64_HI(uuWatchDogTimerStateEntry);
      pOcmBlock[nTimer>>nMode32].uData[0] = COMPILER_64_LO(uuWatchDogTimerStateEntry);
    }
    nOcmPort = c3hppc_ocm_map_cop2ocm_port(nInstance);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, pCopSegmentInfo->uSegmentBase,
                               (pCopSegmentInfo->uSegmentBase+nDmaBlockSize-1), 1, pOcmBlock->uData );
    soc_cm_sfree( pc3hppcTestInfo->nUnit, pOcmBlock );

  }


  if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {
    c3hppc_tmu_region_map_setup( pc3hppcTestInfo->nUnit, C3HPPC_TMU_REGION_LAYOUT__RANDOM );

    uResultsTimer = 768;
    if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
      uResultsTimer *= 2;
    }
    c3hppc_lrp_set_results_timer( pc3hppcTestInfo->nUnit, uResultsTimer );
  }


  /* Enable COP segment 0 used for test management. */
  for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
    c3hppc_cop_program_segment_enable( pc3hppcTestInfo->nUnit, nInstance, 0 );
  }


  c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                              pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                       pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );


  return 0;
}

int
c3_exerciser_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nTimeOut, nTable, nProgram, nOcmPort, nInstance, nLrpPort, nTP, nSetupOptions, nKey, nKey1, nSubKey, nAllocSize;
  uint32 *pDmTableDmaSratchPad;
  uint32 uBaseOffset;
  uint32 uColumnOffset, uRegionRowOffset; 
  uint32 uDeadlineOffset;
  uint32 *pEML_InsertList, uEML_InsertEntrySizeInBytes;
  int    nEML_RootTableIndex, nEML_ChainTableIndex, nEMC_RootTableIndex, nEMC_NextTableIndex;
  uint32 uEMC_EntrySizeIn64b;
  uint32 *pIPV4BucketTable, *apIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], uIPV4AssocDataTableEntryNum;
  uint32 uIPV4RootPivotNum, uIPV4BucketTableEntryNum, uIPV4BucketEntrySizeIn64b, uIPV4BucketEntriesPerRow;
  uint32 uIPV4AssocDataEntrySizeIn64b, uIPV4AssocDataEntriesPerRow;
  uint32 auIPV4BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 *pIPV6BucketTable, *apIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV6RootPivotNum, uIPV6BucketTableEntryNum, uIPV6BucketEntrySizeIn64b, uIPV6BucketEntriesPerRow;
  uint32 uIPV6KeyAndAssocDataEntrySizeIn64b, uIPV6KeyAndAssocDataEntriesPerRow;
  uint32 auIPV6BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV6KeyAndAssocDataTableEntryNum;
  uint32 *pRPBcommands, *pBBXcommands, *pBRRcommands;
  uint32 *apKeyDeleteList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM], *apKeyReInsertList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 auKeyDeleteListLength[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM], auKeyReInsertListLength[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 uInsertEntrySizeIn32b, uKeySizeIn32b, uKeyFilter, uKeyFilterMask, uKey;
  int nCmdFifoSelect;
  int nBulkDelete;
  sal_time_t  tTableChangesAllowedTime;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  c3hppc_64b_ocm_entry_template_t uOcmEntryInit;
  int nDmaBlockSize;
  uint64 auuKeyData[8];
  uint32 uFilterSetIndex, uSBlkIndex, uColumnIndex;
  uint32 uEtuKeySizeIn80bSegments, uSegment;
  c3hppc_etu_80b_data_t *pEtuKeyData, *pEtuKeyMask;
  uint64 uuEtuKeyMask;
  uint64 uuTransmitDuration, uuMaxDurationB4StatsCollect, uuIlStatsCollectionInterval;
  uint32 uPacketCaptureBuf[64], uPacketCaptureChannel;
  uint32 uPacketCaptureLen, uPacketCaptureSOP, uPacketCaptureEOP, uPacketCaptureERR, uPacketCaptureBV;
  int nIlInstanceNum;
  uint8 bShutDown;
  uint32 uLrpLearningListWriteOffset;
  int nDeleteOperationCounter;
  sal_time_t TimeStamp;
  uint64 uuTimer;
  uint64 uuVal;
  int nGo, nActiveChannelNum;
  c3hppc_tmu_control_info_t   c3hppcTmuControlInfo;








  nSetupOptions = 0;
  nKey1 = 0;
  nEML_RootTableIndex = 0;
  nEML_ChainTableIndex = 0;
  nEMC_RootTableIndex = 0;
  nEMC_NextTableIndex = 0;
  pIPV4BucketTable = NULL;
  pIPV6BucketTable = NULL;
  nBulkDelete = 0;
  uLrpLearningListWriteOffset = 0;



  /* The following allocation is used to initialize OCM memory with RCE results.  */
  nDmaBlockSize = C3_EXERCISER_TEST1__MAX_KEYS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(C3_EXERCISER_TEST1_RCE_RESULT_PORT);

  /****************************************************************************************************************************
   * Build RCE program and filter pattern image.
   *****************************************************************************************************************************/
  for ( nProgram = 0; 
        nProgram < ( pc3hppcTestInfo->BringUpControl.uRceBringUp ? COUNTOF(g_aRceProgramParameters) : 0 );
        ++nProgram ) {

    c3hppc_rce_create_program_for_lpm_exact_match( pc3hppcTestInfo->nUnit, g_aRceProgramParameters[nProgram].nNumber,
                                                   g_aRceProgramParameters[nProgram].uBaseAddress,
                                                   g_aRceProgramParameters[nProgram].uFilterSetNumber,
                                                   g_aRceProgramParameters[nProgram].uFilterSetLength,
                                                   g_aRceProgramParameters[nProgram].uKeyLength,
                                                   g_aRceProgramParameters[nProgram].uKeyStartIndex, 0 );
    COMPILER_64_ZERO(auuKeyData[0]);
    for ( uKey = 0; uKey < (g_aRceProgramParameters[nProgram].uFilterSetNumber * C3HPPC_RCE_TOTAL_COLUMN_NUM); ++uKey ) {
      COMPILER_64_SET(auuKeyData[0],0, uKey);

      uFilterSetIndex = uKey / C3HPPC_RCE_TOTAL_COLUMN_NUM;
      uSBlkIndex = (uKey % C3HPPC_RCE_TOTAL_COLUMN_NUM) / C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK;
      uColumnIndex = (uKey % C3HPPC_RCE_TOTAL_COLUMN_NUM) & (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK - 1);

      c3hppc_rce_add_filter_for_lpm_exact_match( g_aRceProgramParameters[nProgram].uFilterSetLength,
                                                 g_aRceProgramParameters[nProgram].uKeyLength,
                                                 uFilterSetIndex, uSBlkIndex, uColumnIndex,
                                                 g_aRceProgramParameters[nProgram].uBaseAddress,
                                                 auuKeyData );

      if ( nProgram == 0 ) {
        if ( uKey < nDmaBlockSize ) {
          pOcmBlock[uKey].uData[0] = (uFilterSetIndex << 12) | (uSBlkIndex << 5) | uColumnIndex;
          pOcmBlock[uKey].uData[1] = 0;
        }
      }
    }

    if ( nProgram == 0 ) {
      cli_out("\nLoading RCE results database into OCM ...\n");
      c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                 0, (nDmaBlockSize-1), 1, pOcmBlock->uData );
    }
  }


  if ( pc3hppcTestInfo->BringUpControl.uRceBringUp ) {
    cli_out("\nLoading the RCE image ...\n");
    c3hppc_rce_dma_image( pc3hppcTestInfo->nUnit );
  }




  /****************************************************************************************************************************
   * Initialize OCM contents for background load activity on all ports except the one used for RCE results.
   ***************************************************************************************************************************/
  nDmaBlockSize = C3_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    if ( nOcmPort != c3hppc_ocm_map_lrp2ocm_port(C3_EXERCISER_TEST1_RCE_RESULT_PORT) ) {
      uOcmEntryInit.uData[0] = 0xee000000 | (nLrpPort << 20) | (nLrpPort << 16);
      uOcmEntryInit.uData[1] = 0xff000000 | (nLrpPort << 20) | (nLrpPort << 16);
      for ( nIndex = 0; nIndex < nDmaBlockSize; ++nIndex ) {
        uOcmEntryInit.uData[0] &= 0xffff0000;
        uOcmEntryInit.uData[0] |= nIndex;
        uOcmEntryInit.uData[1] &= 0xffff0000;
        uOcmEntryInit.uData[1] |= nIndex;
        pOcmBlock[nIndex].uData[1] = uOcmEntryInit.uData[1];
        pOcmBlock[nIndex].uData[0] = uOcmEntryInit.uData[0];
      }
      cli_out("\nInitializing OCM contents for LRP Port %d\n", nLrpPort);
      uBaseOffset = ( nOcmPort == c3hppc_ocm_map_lrp2ocm_port(C3_EXERCISER_TEST1_WDT_PORT) ) ?
                                            C3_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET : 0;
      c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                 uBaseOffset, (uBaseOffset+nDmaBlockSize-1), 1, pOcmBlock->uData );
    }
  }
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock);


  uRegionRowOffset = 0;
  /****************************************************************************************************************************
   *
   * DM Tables setup
   *
   *****************************************************************************************************************************/
  pDmTableDmaSratchPad = (uint32 *)
            sal_alloc( (C3_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM * C3HPPC_TMU_DM494_DATA_SIZE_IN_64b * sizeof(uint64)),
                       "dma scratch pad");

  for ( nTable = 0; 
        nTable < ( pc3hppcTestInfo->BringUpControl.uTmuBringUp ? COUNTOF(g_aDmTableParameters) : 0 );
        ++nTable ) {
    nCmdFifoSelect = nTable & 1;
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, g_aDmTableParameters[nTable].nTableID, g_aDmTableParameters[nTable].uDmLookUp,
                            g_aDmTableParameters[nTable].uNumEntries, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, g_aDmTableParameters[nTable].uColumnOffset,
                            g_aDmTableParameters[nTable].uDmEntriesPerRow,
                            g_aDmTableParameters[nTable].uDeadlineOffset, 0, 0, 0, 0, 0, 0, 0,
                            ( ((nTable+1) == COUNTOF(g_aDmTableParameters)) ? &uRegionRowOffset : (uint32 *)NULL ) );

    c3hppc_lrp_setup_dm_segment_table( pc3hppcTestInfo->nUnit, g_aDmTableParameters[nTable].nTableID,
                                       g_aDmTableParameters[nTable].nLrpDmInterface, g_aDmTableParameters[nTable].uLrpDmLookUp );

    c3hppc_test__setup_dm_table_contents( g_aDmTableParameters[nTable].uDmLookUp, g_aDmTableParameters[nTable].nTableID,
                                          g_aDmTableParameters[nTable].uNumEntries, 0x11100000, pDmTableDmaSratchPad );
    cli_out("\nINFO:  Initializing DM Table ID[%d] content ...\n", g_aDmTableParameters[nTable].nTableID );
    c3hppc_tmu_xl_write( nCmdFifoSelect, g_aDmTableParameters[nTable].nTableID, 0,
                         g_aDmTableParameters[nTable].uNumEntries, 0, pDmTableDmaSratchPad );
    if ( g_bDoInitXLreads ) {
      c3hppc_tmu_xl_read(  nCmdFifoSelect, g_aDmTableParameters[nTable].nTableID, 0,
                           g_aDmTableParameters[nTable].uNumEntries, 0, pDmTableDmaSratchPad );
    }
  }
  
/*
  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 0, C3HPPC_TMU_PM_INTF__DM0, 0, 0, 0 );
  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 1, C3HPPC_TMU_PM_INTF__DM0, 0, 1, 0 );
*/

  sal_free( pDmTableDmaSratchPad );



  /****************************************************************************************************************************
   *
   * EML Root and Chain table setup
   *
   *****************************************************************************************************************************/
  pEML_InsertList = NULL;
  uEML_InsertEntrySizeInBytes = 0;
  if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {

    uEML_InsertEntrySizeInBytes = C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b * sizeof(uint64);
    pEML_InsertList = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uEML_InsertEntrySizeInBytes),
                                            "EML64 Insert Table");

    for ( nTable = 0, nCmdFifoSelect = 0; nTable < COUNTOF(g_aEmlTableParameters); ++nTable ) {
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, g_aEmlTableParameters[nTable].nTableID, g_aEmlTableParameters[nTable].uLookUp,
                              g_aEmlTableParameters[nTable].uNumEntries, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, g_aEmlTableParameters[nTable].uColumnOffset,
                              g_aEmlTableParameters[nTable].uEntriesPerRow, g_aEmlTableParameters[nTable].uDeadlineOffset,
                              0, 1, 0, (C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM-1), g_uUpdaterFreeChainFifoPoolMask, 0, 0, &uRegionRowOffset );
  
      g_aEmlTableParameters[nTable].pContents =
                           (uint32 *) sal_alloc( (g_aEmlTableParameters[nTable].uNumEntries *
                                                g_aEmlTableParameters[nTable].uEntrySizeIn64b * sizeof(uint64)),
                                                "EML Table"); 
  
      if ( g_aEmlTableParameters[nTable].cType == 'R' ) {
        nEML_RootTableIndex = nTable;
      } else {
        nEML_ChainTableIndex = nTable;
        /* With DRAM initialization the EM-L64-C chain table entries will read out with bad ECC.  The chain table dump routines use the 
           provisioned chain limit which will induce ECC errors.  Therefore initializing the chain table with 0's.
        */ 
        sal_memset( g_aEmlTableParameters[nTable].pContents, 0x00, (g_aEmlTableParameters[nTable].uNumEntries *
                                            g_aEmlTableParameters[nTable].uEntrySizeIn64b * sizeof(uint64)) );
        c3hppc_tmu_xl_write( nCmdFifoSelect, g_aEmlTableParameters[nTable].nTableID, 0,
                             g_aEmlTableParameters[nTable].uNumEntries, 0, g_aEmlTableParameters[nTable].pContents );
      }
    }

    c3hppc_test__setup_eml_table_contents( nSetupOptions, pc3hppcTestInfo->nMaxKey, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                           g_aEmlTableParameters[nEML_RootTableIndex].pContents, pEML_InsertList );

    if ( pc3hppcTestInfo->bSkipCiDramInit ) {
      cli_out("\nINFO:  Initializing EML Root Table ...\n");
      c3hppc_tmu_xl_write( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                           g_aEmlTableParameters[nEML_RootTableIndex].uNumEntries, 0, g_aEmlTableParameters[nEML_RootTableIndex].pContents );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                             g_aEmlTableParameters[nEML_RootTableIndex].uNumEntries, 0, g_aEmlTableParameters[nEML_RootTableIndex].pContents );
      }
    }

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
      c3hppc_tmu_eml_insert( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                             pc3hppcTestInfo->nMaxKey, pEML_InsertList, C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE );
      if ( g_bDumpEmlTables ) {
        c3hppc_tmu_xl_read( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                            g_aEmlTableParameters[nEML_RootTableIndex].uNumEntries, 0, NULL );
        c3hppc_tmu_xl_read( nCmdFifoSelect, g_aEmlTableParameters[nEML_ChainTableIndex].nTableID, 0,
                            g_aEmlTableParameters[nEML_ChainTableIndex].uNumEntries, 0, NULL );
      }
    }

    for ( nSubKey = 0; nSubKey < C3HPPC_TMU_SUBKEY_NUM; ++nSubKey ) {
      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3_EXERCISER_TEST1__EML64_TMU_PROGRAM,
                                  g_aEmlTableParameters[nEML_RootTableIndex].uLookUp,
                                  (uint32) g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                                  ((nSubKey == 1) ? 62 : 0), 2, 0, 0, 0 );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 0, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3_EXERCISER_TEST1__EML64_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 1, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3_EXERCISER_TEST1__EML64_TMU_PROGRAM );

    for ( nTable = 0; nTable < COUNTOF(g_aEmlTableParameters); ++nTable ) {
      sal_free( g_aEmlTableParameters[nTable].pContents );
    }

  }


  /****************************************************************************************************************************
   *
   * EMC Root and Next table setup
   *
   *****************************************************************************************************************************/

  if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {
    for ( nTable = 0; nTable < COUNTOF(g_aEmcTableParameters); ++nTable ) {
      uEMC_EntrySizeIn64b = g_aEmcTableParameters[nTable].uEntrySizeIn64b; 
      if ( c3hppc_tmu_get_emc128mode() ) --uEMC_EntrySizeIn64b;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, g_aEmcTableParameters[nTable].nTableID, g_aEmcTableParameters[nTable].uLookUp,
                              g_aEmcTableParameters[nTable].uNumEntries, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, g_aEmcTableParameters[nTable].uColumnOffset,
                              g_aEmcTableParameters[nTable].uEntriesPerRow, g_aEmcTableParameters[nTable].uDeadlineOffset,
                              ( (g_aEmcTableParameters[nTable].cType == 'R') ? g_aEmcTableParameters[nEMC_NextTableIndex].nTableID : 0),
                              0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
      g_aEmcTableParameters[nTable].pContents = 
                                     (uint32 *) sal_alloc( (g_aEmcTableParameters[nTable].uNumEntries * uEMC_EntrySizeIn64b * sizeof(uint64)),
                                     "EMC Table");
      if ( g_aEmcTableParameters[nTable].cType == 'R' ) {
        nEMC_RootTableIndex = nTable;
      } else {
        nEMC_NextTableIndex = nTable;
      }
    }
  
    c3hppc_test__setup_emc_table_contents( 0, g_aEmcTableParameters[nEMC_RootTableIndex].pContents,
                                           g_aEmcTableParameters[nEMC_NextTableIndex].pContents,
                                           g_aEmcTableParameters[nEMC_RootTableIndex].nTableID,
                                           g_aEmcTableParameters[nEMC_NextTableIndex].nTableID );

    for ( nTable = 0; nTable < COUNTOF(g_aEmcTableParameters); ++nTable ) {
      if ( g_aEmcTableParameters[nTable].cType == 'R' ) {
        cli_out("\nINFO:  Initializing EMC Root Table ...\n");
      } else {
        cli_out("\nINFO:  Initializing EMC Next Table ...\n");
      }
      nCmdFifoSelect = g_aEmcTableParameters[nTable].nTableID & 1;
      c3hppc_tmu_xl_write( nCmdFifoSelect, g_aEmcTableParameters[nTable].nTableID, 0,
                           g_aEmcTableParameters[nTable].uNumEntries, 0,
                           g_aEmcTableParameters[nTable].pContents );
      if ( pc3hppcTestInfo->bSkipCiDramInit && g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, g_aEmcTableParameters[nTable].nTableID, 0,
                             g_aEmcTableParameters[nTable].uNumEntries, 0,
                             g_aEmcTableParameters[nTable].pContents );
      }
      sal_free( g_aEmcTableParameters[nTable].pContents ); 
    }

    for ( nSubKey = 0; nSubKey < C3HPPC_TMU_SUBKEY_NUM; ++nSubKey ) {
      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM,
                                  g_aEmcTableParameters[nEMC_RootTableIndex].uLookUp,
                                  g_aEmcTableParameters[nEMC_RootTableIndex].nTableID, 0,
                                  ((nSubKey == 1) ? 62 : 32), 2, 0, 0, 0 );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3_EXERCISER_TEST1__EMC64_TMU_PROGRAM );

  }



  /****************************************************************************************************************************
   *
   * LPM IPV4 setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->bIPV4Enable && pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {

    auIPV4BucketTable[0] = g_aEmcTableParameters[nEMC_NextTableIndex].nTableID + 1;
    auIPV4BucketTable[1] = auIPV4BucketTable[0] + 2;
    uIPV4RootPivotNum = C3_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM;
    uIPV4BucketTableEntryNum = C3_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM;
    if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
      uIPV4RootPivotNum /= C3_EXERCISER_TEST1__LPMIPV4_FULLY_POPULATED_TCAM_FACTOR; 
      uIPV4BucketTableEntryNum /= C3_EXERCISER_TEST1__LPMIPV4_FULLY_POPULATED_TCAM_FACTOR;
    }
    uIPV4BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV4BucketPrefixNum );
    uIPV4BucketEntriesPerRow = C3_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV4AssocDataTable[0] = auIPV4BucketTable[0] + 1;
    auIPV4AssocDataTable[1] = auIPV4AssocDataTable[0] + 2;
    uIPV4AssocDataTableEntryNum = uIPV4BucketTableEntryNum * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntriesPerRow = uIPV4BucketEntriesPerRow * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntrySizeIn64b = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b; 
    pIPV4BucketTable = NULL;

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET,
                              uIPV4BucketTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV4BucketEntriesPerRow, uDeadlineOffset, auIPV4AssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV4BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );

      if ( pIPV4BucketTable == NULL ) {
        pIPV4BucketTable = (uint32 *) sal_alloc( (uIPV4BucketTableEntryNum * uIPV4BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV4 Bucket Table");
        sal_memset( pIPV4BucketTable, 0, (uIPV4BucketTableEntryNum * uIPV4BucketEntrySizeIn64b * sizeof(uint64)) );
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

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                     C3_EXERCISER_TEST1__LPMIPV4_RPB_PREFIX_SIZE,
                                     uIPV4RootPivotNum, C3_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE,
                                     C3_EXERCISER_TEST1__LPMIPV4_BBX_PREFIX_SIZE, pc3hppcTestInfo->nIPV4BucketPrefixNum,
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }

    nAllocSize = uIPV4RootPivotNum * sizeof(c3hppc_tmu_taps_rpb_ccmd_t);
    pRPBcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV4 RPB Commands" );
    sal_memset( pRPBcommands, 0, nAllocSize );

    nAllocSize = uIPV4RootPivotNum * C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_bbx_ccmd_t);
    pBBXcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV4 BBX Commands");
    sal_memset( pBBXcommands, 0, nAllocSize );

    nAllocSize = uIPV4RootPivotNum * C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_brr_ccmd_t);
    pBRRcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV4 BRR Commands");
    sal_memset( pBRRcommands, 0, nAllocSize );
  

    c3hppc_test__setup_lpmipv4_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH,
                                               pc3hppcTestInfo->nIPV4BucketPrefixNum, auIPV4BucketTable[0],
                                               uIPV4RootPivotNum, C3_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                               C3_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM,
                                               C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM,
                                               C3_EXERCISER_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                               pRPBcommands, pBBXcommands, pBRRcommands, pIPV4BucketTable, apIPV4AssocDataTable);

    cli_out("\nINFO:  Initializing TAPS IPV4 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, uIPV4RootPivotNum, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV4 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (uIPV4RootPivotNum * C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV4 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (uIPV4RootPivotNum * C3_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      nCmdFifoSelect = (int)(auIPV4BucketTable[nTP] & 1);

      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV4BucketTable[nTP], 0,
                           uIPV4BucketTableEntryNum, 0, pIPV4BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV4BucketTable[nTP], 0,
                             uIPV4BucketTableEntryNum, 0, pIPV4BucketTable );
      }

      nCmdFifoSelect = (int)(auIPV4AssocDataTable[nTP] & 1);
      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                           uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                             uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                  auIPV4BucketTable[nTP],
                                  C3_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 
    sal_free( pIPV4BucketTable ); 
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV4AssocDataTable[nTP] );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM );

  } else {
    auIPV4AssocDataTable[1] = g_aEmcTableParameters[nEMC_NextTableIndex].nTableID;
  }  /* if ( pc3hppcTestInfo->bIPV4Enable ) { */



  /****************************************************************************************************************************
   *
   * LPM IPV6 setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->bIPV6Enable && pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {

    auIPV6BucketTable[0] = auIPV4AssocDataTable[1] + 1;
    auIPV6BucketTable[1] = auIPV6BucketTable[0] + 2;
    uIPV6RootPivotNum = C3_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM;
    uIPV6BucketTableEntryNum = C3_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM;
    if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
      uIPV6RootPivotNum /= C3_EXERCISER_TEST1__LPMIPV6_FULLY_POPULATED_TCAM_FACTOR; 
      uIPV6BucketTableEntryNum /= C3_EXERCISER_TEST1__LPMIPV6_FULLY_POPULATED_TCAM_FACTOR;
    }
    uIPV6BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv6_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV6BucketPrefixNum );
    uIPV6BucketEntriesPerRow = C3_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV6KeyAndAssocDataTable[0] = auIPV6BucketTable[0] + 1;
    auIPV6KeyAndAssocDataTable[1] = auIPV6KeyAndAssocDataTable[0] + 2;
    uIPV6KeyAndAssocDataTableEntryNum = uIPV6BucketTableEntryNum * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntriesPerRow = uIPV6BucketEntriesPerRow * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntrySizeIn64b = C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_64b;
    pIPV6BucketTable = NULL;

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV6BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV6_BUCKET,
                              uIPV6BucketTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV6BucketEntriesPerRow, uDeadlineOffset, auIPV6KeyAndAssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV6BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );
  
      if ( pIPV6BucketTable == NULL ) {
        pIPV6BucketTable = (uint32 *) sal_alloc( (uIPV6BucketTableEntryNum * uIPV6BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV6 Bucket Table");
        sal_memset( pIPV6BucketTable, 0, (uIPV6BucketTableEntryNum * uIPV6BucketEntrySizeIn64b * sizeof(uint64)) );
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

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                     C3_EXERCISER_TEST1__LPMIPV6_RPB_PREFIX_SIZE,
                                     uIPV6RootPivotNum, C3_EXERCISER_TEST1__LPMIPV6_SEGMENT_BASE,
                                     C3_EXERCISER_TEST1__LPMIPV6_BBX_PREFIX_SIZE, pc3hppcTestInfo->nIPV6BucketPrefixNum,
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    nAllocSize = uIPV6RootPivotNum * sizeof(c3hppc_tmu_taps_rpb_ccmd_t);
    pRPBcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV6 RPB Commands");
    sal_memset( pRPBcommands, 0, nAllocSize );

    nAllocSize = uIPV6RootPivotNum * C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_bbx_ccmd_t);
    pBBXcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV6 BBX Commands" );
    sal_memset( pBBXcommands, 0, nAllocSize );

    nAllocSize = uIPV6RootPivotNum * C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM * sizeof(c3hppc_tmu_taps_brr_ccmd_t);
    pBRRcommands = (uint32 *) sal_alloc( nAllocSize, "LPM IPV6 BRR Commands" );
    sal_memset( pBRRcommands, 0, nAllocSize );

    c3hppc_test__setup_lpmipv6_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, pc3hppcTestInfo->nSetupOptions,
                                               pc3hppcTestInfo->nIPV6BucketPrefixNum, auIPV6BucketTable[0],
                                               uIPV6RootPivotNum, C3_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                               C3_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM,
                                               C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM,
                                               C3_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                               pRPBcommands, pBBXcommands, pBRRcommands, pIPV6BucketTable, apIPV6KeyAndAssocDataTable );

    cli_out("\nINFO:  Initializing TAPS IPV6 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, uIPV6RootPivotNum, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV6 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (uIPV6RootPivotNum * C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV6 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (uIPV6RootPivotNum * C3_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      nCmdFifoSelect = (int)(auIPV6BucketTable[nTP] & 1);

      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV6BucketTable[nTP], 0,
                           uIPV6BucketTableEntryNum, 0, pIPV6BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV6BucketTable[nTP], 0,
                             uIPV6BucketTableEntryNum, 0, pIPV6BucketTable );
      }

      nCmdFifoSelect = (int)(auIPV6KeyAndAssocDataTable[nTP] & 1);
      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                           uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                             uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV6,
                                  auIPV6BucketTable[nTP],
                                  C3_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                  ((nTP == 0) ? 2 : 40), 18, 0, 0, 0 );
    }

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 
    sal_free( pIPV6BucketTable ); 
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV6KeyAndAssocDataTable[nTP] );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 6, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 7, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM );

  } /* if ( pc3hppcTestInfo->bIPV6Enable ) { */



  /****************************************************************************************************************************
   *
   * ETU/NL11K setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 1 ) {
    c3hppc_etu_setup_search_program( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1__ETU_SEARCH_PROGRAM,
                                     C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, C3HPPC_ETU_LOOKUP__4x80 );
    if ( c3hppc_etu_tcam_table_layout_setup( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0,
                                             C3HPPC_ETU_LOOKUP__4x80, C3_EXERCISER_TEST1__ETU_MAX_KEYS ) ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      if ( pEML_InsertList != NULL ) sal_free( pEML_InsertList );
      return -1; 
    }

    uEtuKeySizeIn80bSegments = c3hppc_etu_get_tcam_table_key_size( C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0 );
    pEtuKeyData = (c3hppc_etu_80b_data_t *) sal_alloc( (C3_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                       "ETU Key Data");
    pEtuKeyMask = (c3hppc_etu_80b_data_t *) sal_alloc( (C3_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                       "ETU Key Mask");
    sal_memset( pEtuKeyData, 0x00, (C3_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );
    sal_memset( pEtuKeyMask, 0x00, (C3_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );

    COMPILER_64_SET(uuVal, 0xffffffff, 0xffffffff);
    COMPILER_64_SET(uuEtuKeyMask, 0, (C3_EXERCISER_TEST1__ETU_MAX_KEYS - 1));
    COMPILER_64_XOR(uuEtuKeyMask, uuVal);
    for ( nKey = 0; nKey < C3_EXERCISER_TEST1__ETU_MAX_KEYS; ++nKey ) {
      for ( uSegment = 0; uSegment < uEtuKeySizeIn80bSegments; ++uSegment ) {
        nIndex = (nKey * uEtuKeySizeIn80bSegments) + uSegment;
        if ( uSegment == 0 ) {
          COMPILER_64_SET(pEtuKeyData[nIndex].Words[0],0, nKey);
          COMPILER_64_ZERO(pEtuKeyData[nIndex].Words[1]);
          COMPILER_64_SET(pEtuKeyMask[nIndex].Words[0], COMPILER_64_HI(uuEtuKeyMask), COMPILER_64_LO(uuEtuKeyMask));
          COMPILER_64_SET(pEtuKeyMask[nIndex].Words[1],0x00000000,0x0000ffff);
        } else if ( (uSegment+1) == uEtuKeySizeIn80bSegments ) {
          COMPILER_64_SET(pEtuKeyData[nIndex].Words[1],0x00000000,0x00008000);
        }
      }
    }
    cli_out("\nINFO:  Initializing NL11K TCAM Table ...\n");
    c3hppc_etu_key_insert( C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x00000,
                           C3_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x20000,
                           C3_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x40000,
                           C3_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x60000,
                           C3_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    sal_free( pEtuKeyData );
    sal_free( pEtuKeyMask );
  }


  /****************************************************************************************************************************
   * Configure the LRP program translation tables.
   *****************************************************************************************************************************/
  for ( nProgram = 0; nProgram < COUNTOF(g_aLrpSearchProgramParameters); ++nProgram ) {

    if ( pc3hppcTestInfo->BringUpControl.uRceBringUp == 1 &&
         g_aLrpSearchProgramParameters[nProgram].uRceProgram != C3_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_rce_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uRceProgram );
    }
    if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp == 1 && 
         g_aLrpSearchProgramParameters[nProgram].uTmuProgram != C3_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uTmuProgram,
                                    g_aLrpSearchProgramParameters[nProgram].bSubKey0Valid,
                                    g_aLrpSearchProgramParameters[nProgram].bSubKey1Valid);
    }
    if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 1 &&
         g_aLrpSearchProgramParameters[nProgram].uEtuProgram != C3_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_etu_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uEtuProgram );
    }

  }




  /****************************************************************************************************************************
   * Wait for Updater operations to complete before advancing to enable traffic.
   *****************************************************************************************************************************/
  if ( (pc3hppcTestInfo->BringUpControl.uTmuBringUp || pc3hppcTestInfo->BringUpControl.uEtuBringUp) &&
       c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {

    cli_out("ERROR:  Initial setup Updater TIMEOUT failure!\n");
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else {


    /****************************************************************************************************************************
     * If doing HW allocation then dump/display EML tables and verify their integrity.
     *****************************************************************************************************************************/
    if ( g_bDumpEmlTables && pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {
      if ( c3hppc_tmu_display_andor_scoreboard_eml_tables( pc3hppcTestInfo->nUnit, 
                                                           g_bDumpTmuDebugInfo, "/home/morrier/EML_TableDump_Initial",
                                                           (uint32) g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                                           (uint32) g_aEmlTableParameters[nEML_ChainTableIndex].nTableID,
                                                           1, pc3hppcTestInfo->nMaxKey ) ) { 
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }


    /****************************************************************************************************************************
     * Activate performance monitor utility.  The PM logic has a bug whereby if activated while traffic is running a few bogus
     * samples are captured which need to be discarded when analyzing the data.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp ) {
      c3hppc_tmu_pm_activate( pc3hppcTestInfo->nUnit, 0, g_uPmBucketShift, 0 );
    }



    /****************************************************************************************************************************
     * Enable watch dog timer refresh ....
     *****************************************************************************************************************************/
    c3hppc_cop_set_watchdogtimer_ring_manager_timer_num( pc3hppcTestInfo->nCounterNum );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_svp( C3_EXERCISER_TEST1_WDT_PORT );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_lm( C3_EXERCISER_TEST1_WDT_LIST_MANAGER );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_size( C3HPPC_COP_INSTANCE_NUM * pc3hppcTestInfo->nCounterNum );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_base( C3_EXERCISER_TEST1_WDT_LIST_BASE_OFFSET );
    c3hppc_lrp_setup_host_producer_ring( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1_WDT_PORT, 0, 
                                         (C3HPPC_COP_INSTANCE_NUM * pc3hppcTestInfo->nCounterNum), 0 );
    for ( nInstance = 0; nInstance < C3HPPC_COP_INSTANCE_NUM; ++nInstance ) {
      if ( !(pc3hppcTestInfo->bLrpLearningEnable == 1 && nInstance == 1) ) {
        c3hppc_cop_program_segment_enable( pc3hppcTestInfo->nUnit, nInstance, g_nWatchDogTimerCopSegment );
      }
    }


    /****************************************************************************************************************************
     * Do setup for LRP based learning ...
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->bLrpLearningEnable ) {
      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, 0, C3_EXERCISER_TEST1__LRP_LEARNING_TMU_PROGRAM,
                                  C3HPPC_TMU_LOOKUP__EML_INSERT_DELETE, 0, 0, 0, 0, 0, 0, 0 );
      c3hppc_lrp_setup_host_producer_ring( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1_LRP_LEARNING_SEMAPHORE_PORT,
                                           C3_EXERCISER_TEST1_LRP_LEARNING_LIST_MANAGER,
                                           C3_EXERCISER_TEST1_LRP_LEARNING_LIST_SIZE, 0 );
    }

    COMPILER_64_SET(uuVal, 0, 0); /* C3HPPC_LRP__CONTINUOUS_EPOCHS */
    c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuVal );



    nIlInstanceNum = C3HPPC_SWS_IL_INSTANCE_NUM;
    uuTransmitDuration = pc3hppcTestInfo->uuIterations;
    if ( g_rev_id != BCM88030_A0_REV_ID ) {
      uuIlStatsCollectionInterval = uuTransmitDuration;
    } else {
      COMPILER_64_SET( uuIlStatsCollectionInterval, 0, C3_EXERCISER_TEST1__IL_STATS_COLLECTION_INTERVAL );
    }

    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
        uint64 uuTmp = COMPILER_64_INIT(0xffffffff,0xffffffff);
        c3hppc_sws_il_pktcap_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTCAP__CAPTURE_COMING_FROM_PT, 1, uuTmp);
        c3hppc_sws_il_pktcap_arm( pc3hppcTestInfo->nUnit, nInstance );
      }
    }

/*
    soc_sbx_caladan3_ppe_hc_control( pc3hppcTestInfo->nUnit, 0, 1, -1, -1, 0, 0 );
    soc_sbx_caladan3_ppe_hc_control( pc3hppcTestInfo->nUnit, 1, 0, -1, -1, 0, 0 );
*/

    /****************************************************************************************************************************
     * Setup for EML chain destruction/construction inducing de-allocation and re-allocation of table entries.
     *****************************************************************************************************************************/
    tTableChangesAllowedTime = sal_time() + ((sal_time_t) COMPILER_64_LO(uuTransmitDuration)) - 60;
    uInsertEntrySizeIn32b = 2 * C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b;
    uKeySizeIn32b = 2 * C3HPPC_TMU_EML64_KEY_SIZE_IN_64b;
    nCmdFifoSelect = 0;
    if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) {
      for ( nCmdFifoSelect = 0; nCmdFifoSelect < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifoSelect ) { 
        apKeyDeleteList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)), "Key Delete List");
        apKeyReInsertList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uInsertEntrySizeIn32b * sizeof(uint32)),
                                                  "ReInsert List");
        sal_memset( apKeyDeleteList[nCmdFifoSelect], 0x00, (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)) ); 
      }
      sal_memset( g_acKeyLookupMissScoreBoard, 0x00, sizeof(g_acKeyLookupMissScoreBoard) );
    }
	nCmdFifoSelect = 0;
    switch ( pc3hppcTestInfo->nTDM ) {
      case C3HPPC_SWS_TDM__100G_BY_100IL:
      case C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL:  nActiveChannelNum = 1;  break;
      case C3HPPC_SWS_TDM__48x1G_BY_100IL:        nActiveChannelNum = 48; break;
      case C3HPPC_SWS_TDM__12x10G_BY_100IL:       nActiveChannelNum = 12; break;   /* Change to 10 (not 12) for Russ's power test */
      case C3HPPC_SWS_TDM__3x40G_BY_100IL:        nActiveChannelNum = 3; break;
      default:                                    nActiveChannelNum = C3HPPC_SWS_IL_MAX_CHANNEL_NUM;
    }

    if ( pc3hppcTestInfo->nLineSidePortNum ) {
      nActiveChannelNum = pc3hppcTestInfo->nLineSidePortNum;
    }


    while ( !COMPILER_64_IS_ZERO(uuTransmitDuration) ) {
     
      COMPILER_64_SET(uuMaxDurationB4StatsCollect,0, 
                      C3HPPC_MIN( COMPILER_64_LO(uuIlStatsCollectionInterval), COMPILER_64_LO(uuTransmitDuration) ));

      for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
        if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
             (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {
          uint64 uuTmp = COMPILER_64_INIT(0,0);
          if (pc3hppcTestInfo->nModulateTimer == 0) {
            COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuMaxDurationB4StatsCollect), COMPILER_64_LO(uuMaxDurationB4StatsCollect));
          }
          c3hppc_sws_il_pktgen_setup( pc3hppcTestInfo->nUnit, nInstance, ILPKTGEN__INJECT_TOWARDS_PRE, 
                                      0,
                                      uuTmp,
                                      ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD :
                                                                                   ILPKTGEN__PAYLOAD_PATTEN__SSO ),
                                      pc3hppcTestInfo->nIPG,
                                      0, (nActiveChannelNum-1), ( nActiveChannelNum == 1 ? ILPKTGEN__FIXED : ILPKTGEN__INCREMENT ),
                                 /*   48, 48, ILPKTGEN__FIXED, */
                                 /*   48, 256, ILPKTGEN__INCREMENT, */
                                 /*   64, 256, ILPKTGEN__RANDOM, */
                                      ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? pc3hppcTestInfo->nPacketSize  :
                                                          ((pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL &&
                                                            g_rev_id != BCM88030_B0_REV_ID) ? 64 : pc3hppcTestInfo->nPacketSize) ),
                                      ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? pc3hppcTestInfo->nPacketSize  : 
                                                          ((pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL && 
                                                            g_rev_id != BCM88030_B0_REV_ID) ? 64 : pc3hppcTestInfo->nPacketSize) ),
                                      ( nInstance == C3HPPC_TEST__LINE_INTERFACE ? ILPKTGEN__FIXED  : ILPKTGEN__FIXED ),
                                      NULL,
                                      ( (nInstance == C3HPPC_TEST__LINE_INTERFACE || pc3hppcTestInfo->bLineTrafficGenOnly ||
                                         pc3hppcTestInfo->bFabricTrafficGenOnly) ? ILPKTGEN__BUILTIN_HEADERS__NULL0 :
                                                                                   ILPKTGEN__BUILTIN_HEADERS__NULL1 ),
                                      0 ); 
        }
      }

      for ( nIndex = 0; nIndex < nIlInstanceNum; ++nIndex ) {
        if ( nIndex == 0 ) {
          nInstance = sal_rand() % nIlInstanceNum;
        } else {
          nInstance = ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) ? C3HPPC_TEST__FABRIC_INTERFACE : C3HPPC_TEST__LINE_INTERFACE; 
        }
        if ( (nInstance == C3HPPC_TEST__LINE_INTERFACE && pc3hppcTestInfo->bFabricTrafficGenOnly == 0) ||
             (nInstance == C3HPPC_TEST__FABRIC_INTERFACE && pc3hppcTestInfo->bLineTrafficGenOnly == 0) ) {
          c3hppc_sws_il_pktgen_start( pc3hppcTestInfo->nUnit, nInstance );
        }
      }

/*
      if ( pc3hppcTestInfo->bLrpLearningEnable ) {
        for ( nIndex = 0; nIndex < 128; ++nIndex ) {
          uOcmEntryInit.uData[0] = nIndex;
          uOcmEntryInit.uData[1] = 0;
          c3hppc_cop_coherent_table_read_write( pc3hppcTestInfo->nUnit, 0, 0, (0x8000 + nIndex), 1, uOcmEntryInit.uData );
        }
        c3hppc_lrp_set_host_producer_ring_write_offset( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1_LRP_LEARNING_SEMAPHORE_PORT,
                                                        C3_EXERCISER_TEST1_LRP_LEARNING_LIST_MANAGER, 16 );
        c3hppc_lrp_setup_host_bubble( pc3hppcTestInfo->nUnit, 11, 0, &uKeyFilter );
      }
*/

      bShutDown = 0;
      nDeleteOperationCounter = 0;
      nCmdFifoSelect = 0;
      while ( (!c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE) ||
               !c3hppc_sws_il_pktgen_is_done(pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE)) && !bShutDown ) { 

        if ( c3hppc_test__get_interrupt_summary( pc3hppcTestInfo->nUnit) ) {
          bShutDown = 1;
        }


        /****************************************************************************************************************************
         * EML chain destruction/construction ...
         *****************************************************************************************************************************/
        if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING &&
             sal_time() < tTableChangesAllowedTime ) { 

          /*
             Bulk delete operations can only run on the "nCmdFifoSelect == 0" slot because the randomly 
             generated filter can not be easily modified to remove possbile collisions from the resultant delete list.
             The same key must NOT be present in both delete lists as this will cause results that are unpredictable.
          */ 
          nBulkDelete = 0;
          if ( nCmdFifoSelect == 0 ) {
            if ( !(sal_rand() % 3) ) {
              nBulkDelete = ( sal_rand() % 10 ) ?  C3_EXERCISER_TEST1__PHYSICAL : C3_EXERCISER_TEST1__LOGICAL;
            }
          }

          if ( sal_rand() % 10 ) {
            uKeyFilterMask = sal_rand() % pc3hppcTestInfo->nMaxKey;
          } else {
            uint32 uTmp = c3hppcUtils_ceil_power_of_2_exp(pc3hppcTestInfo->nMaxKey); /* to avoid div by 0 if function returns 0 */
            uKeyFilterMask = (1 + (sal_rand() % 15)) << (sal_rand() % (uTmp ? uTmp : 1));
            if ( uKeyFilterMask >= pc3hppcTestInfo->nMaxKey ) {
              uKeyFilterMask >>= 3;
              uKeyFilterMask &= (pc3hppcTestInfo->nMaxKey - 1);
            }
          }
          if ( !uKeyFilterMask ) uKeyFilterMask = pc3hppcTestInfo->nMaxKey - 1;
          uKeyFilter = uKeyFilterMask;

          for ( nKey = 0, auKeyDeleteListLength[nCmdFifoSelect] = 0, auKeyReInsertListLength[nCmdFifoSelect] = 0;
                nKey < pc3hppcTestInfo->nMaxKey;
                ++nKey ) {
            if ( (pEML_InsertList[uInsertEntrySizeIn32b*nKey] & uKeyFilterMask) == uKeyFilter ) {
              if ( nCmdFifoSelect == 1 ) {
                /*
                   If any key is found to match an entry in "apKeyDeleteList[0]" then don't do any deletes
                   for the "nCmdFifoSelect == 1" slot, "nKey1" will never equal "auKeyDeleteListLength[0]" due to the "break".
                */
                for ( nKey1 = 0; nKey1 < auKeyDeleteListLength[0]; ++nKey1 ) {
                  if ( apKeyDeleteList[0][uKeySizeIn32b*nKey1] == nKey ) break;
                }
              }
              if ( nCmdFifoSelect == 0 || nKey1 == auKeyDeleteListLength[0] ) {
                apKeyDeleteList[nCmdFifoSelect][uKeySizeIn32b*auKeyDeleteListLength[nCmdFifoSelect]] = nKey; 
                g_acKeyLookupMissScoreBoard[nKey] = 1;
                ++auKeyDeleteListLength[nCmdFifoSelect];
                if ( nBulkDelete != C3_EXERCISER_TEST1__LOGICAL ) {
                  if ( pc3hppcTestInfo->bLrpLearningEnable && (sal_rand() % 2) ) {
                    uOcmEntryInit.uData[0] = nKey;
                    uOcmEntryInit.uData[1] = 0;
                    c3hppc_cop_coherent_table_read_write( pc3hppcTestInfo->nUnit, 0, 0,
                                                          (C3_EXERCISER_TEST1_LRP_LEARNING_LIST_BASE_OFFSET + uLrpLearningListWriteOffset),
                                                          1, uOcmEntryInit.uData );
                    if ( (++uLrpLearningListWriteOffset) == C3_EXERCISER_TEST1_LRP_LEARNING_LIST_SIZE ) uLrpLearningListWriteOffset = 0;
                  } else {
                    sal_memcpy( apKeyReInsertList[nCmdFifoSelect] + (uInsertEntrySizeIn32b*auKeyReInsertListLength[nCmdFifoSelect]),
                                pEML_InsertList + (uInsertEntrySizeIn32b*nKey),
                                uEML_InsertEntrySizeInBytes );
                    ++auKeyReInsertListLength[nCmdFifoSelect];
                  }
                }
              } 
            } 
          }

          if ( nBulkDelete ) {
  
            cli_out("\nINFO:  Bulk[%s] deleting %d EML table entries with KeyFilterMask[0x%04x] ...\n", 
                    ( nBulkDelete == C3_EXERCISER_TEST1__PHYSICAL ? "physical" : "logical" ),
                    auKeyDeleteListLength[nCmdFifoSelect], uKeyFilterMask);
  
            c3hppc_tmu_bulk_delete_setup( pc3hppcTestInfo->nUnit, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                          &uKeyFilter, &uKeyFilterMask );
            if ( nBulkDelete == C3_EXERCISER_TEST1__PHYSICAL ) {
              c3hppc_tmu_bulk_delete_start_scanner( pc3hppcTestInfo->nUnit );
              if ( c3hppc_tmu_wait_for_bulk_delete_done( pc3hppcTestInfo->nUnit, 5) ) {
                cli_out("\nERROR:  Bulk delete failed ...\n");
                pc3hppcTestInfo->nTestStatus = TEST_FAIL;
                break;
              } else {
                /* cli_out("\nINFO:  The bulk delete operation finished ...\n"); */
              }
            } else {

              sal_usleep(3000000); 

            }
            c3hppc_tmu_bulk_delete_cancel( pc3hppcTestInfo->nUnit );

          } else {

            if ( auKeyDeleteListLength[nCmdFifoSelect] ) {
  
              cli_out("\nINFO:  Deleting %d EML table entries issued from CMD FIFO%d...\n",
                      auKeyDeleteListLength[nCmdFifoSelect], nCmdFifoSelect);
  
              c3hppc_tmu_eml_delete( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                     auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect],
                                     C3HPPC_TMU_UPDATE_DELETE_OPTIONS__NONE );
            }
          }

          if ( auKeyDeleteListLength[nCmdFifoSelect] ) {

            if ( nBulkDelete != C3_EXERCISER_TEST1__LOGICAL ) {
              /* cli_out("\nINFO:  Verifying delete of EML table entries ...\n"); */
              c3hppc_tmu_eml_verify_delete( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                            auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );

              if ( pc3hppcTestInfo->bLrpLearningEnable ) {
                if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
                  pc3hppcTestInfo->nTestStatus = TEST_FAIL;
                  break;
                }
              }
  
              /* cli_out("\nINFO:  Adding back the %d deleted EML table entries ...\n", auKeyDeleteListLength[nCmdFifoSelect] ); */
              if ( auKeyReInsertListLength[nCmdFifoSelect] != 0 ) {
                c3hppc_tmu_eml_insert( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                       auKeyReInsertListLength[nCmdFifoSelect], apKeyReInsertList[nCmdFifoSelect],
                                       C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE );
              }

              if ( pc3hppcTestInfo->bLrpLearningEnable ) {
/*
                c3hppc_tmu_lock( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                 C3HPPC_TMU_UPDATE_GLOBAL_LOCK );
                if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
                  pc3hppcTestInfo->nTestStatus = TEST_FAIL;
                  break;
                }
*/
                c3hppc_lrp_set_host_producer_ring_write_offset( pc3hppcTestInfo->nUnit, C3_EXERCISER_TEST1_LRP_LEARNING_SEMAPHORE_PORT,
                                                                C3_EXERCISER_TEST1_LRP_LEARNING_LIST_MANAGER, uLrpLearningListWriteOffset );
                c3hppc_lrp_setup_host_bubble( pc3hppcTestInfo->nUnit, 11, 0, &uKeyFilter );

/*
                sal_usleep( 1000 );
             
                c3hppc_tmu_release( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID );
*/

                if ( c3hppcUtils_poll_field( pc3hppcTestInfo->nUnit, REG_PORT_ANY, QM_PED_INGRESS_DROP_COUNTr,
                                             PED_DROPPED_PACKETSf, 1, 10, 0, &TimeStamp ) ) {
                  cli_out("\nERROR: -- LRP Learning TIMEOUT!!!\n");
                  pc3hppcTestInfo->nTestStatus = TEST_FAIL;
                  break;
                }
              }
     
            }

            ++nDeleteOperationCounter;

            /* cli_out("\nINFO:  Verifying EML tables are fully populated  ...\n"); */
            c3hppc_tmu_eml_verify_insert( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                          auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );
          }

          if ( nCmdFifoSelect || nBulkDelete ) {
            if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
              pc3hppcTestInfo->nTestStatus = TEST_FAIL;
              break;
            }
            nCmdFifoSelect = 0;
          } else {
            nCmdFifoSelect = 1;
          }

        } else {

          if ( pc3hppcTestInfo->nModulateTimer ) {
            uint32 tmp;
            COMPILER_64_SET(uuTimer, COMPILER_64_HI(uuMaxDurationB4StatsCollect), COMPILER_64_LO(uuMaxDurationB4StatsCollect));
            COMPILER_64_UMUL_32(uuTimer, 1000000);
            if(soc_sbx_div64(uuTimer, pc3hppcTestInfo->nModulateTimer, &tmp) == SOC_E_PARAM) return SOC_E_INTERNAL;
            COMPILER_64_SET(uuTimer, 0, tmp);
            COMPILER_64_UMUL_32(uuTimer, pc3hppcTestInfo->nModulateTimer);
            nGo = 0;
            while ( !COMPILER_64_IS_ZERO(uuTimer) ) {
              c3hppc_sws_il_pktgen_go_control( pc3hppcTestInfo->nUnit, 0, nGo );
              sal_usleep( pc3hppcTestInfo->nModulateTimer );
              COMPILER_64_SUB_32(uuTimer, pc3hppcTestInfo->nModulateTimer);
              nGo ^= 1;
            }
            c3hppc_sws_il_pktgen_stop( pc3hppcTestInfo->nUnit, 0 ); 

          } else if ( pc3hppcTestInfo->bShmooUnderLoad ) {
      
            sal_memset( &c3hppcTmuControlInfo, 0x00, sizeof(c3hppc_tmu_control_info_t) );
            c3hppcTmuControlInfo.bSkipCiDramInit = 1;
            c3hppcTmuControlInfo.bSkipCiDramSelfTest = 0;
            c3hppcTmuControlInfo.nDramFreq = pc3hppcTestInfo->BringUpControl.nTmuDramFreq;
            c3hppcTmuControlInfo.uNumberOfCIs = pc3hppcTestInfo->BringUpControl.uTmuNumberOfCIs;
            c3hppc_tmu_hw_init( pc3hppcTestInfo->nUnit, &c3hppcTmuControlInfo );
            c3hppc_tmu_hw_cleanup( pc3hppcTestInfo->nUnit );

            pc3hppcTestInfo->bShmooUnderLoad = 0;

          } else {

            sal_usleep( 1000000 );

/* Did this to try and aggravate SBUS_NACK issue
            READ_TMB_UPDATER_FREE_CHAIN_FIFO_FLUSHr( pc3hppcTestInfo->nUnit, &uRegisterValue );
            soc_reg_field_set( pc3hppcTestInfo->nUnit, TMB_UPDATER_FREE_CHAIN_FIFO_FLUSHr, &uRegisterValue, FLUSH0f, 1 );
            WRITE_TMB_UPDATER_FREE_CHAIN_FIFO_FLUSHr( pc3hppcTestInfo->nUnit, uRegisterValue );
*/
          }

        }

      }

      if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) {
        cli_out("\nINFO:  Total number of Delete/Re-Insert operations --> %d\n", nDeleteOperationCounter );
      }

      if ( bShutDown && pc3hppcTestInfo->nModulateTimer == 0 ) {
        for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
          c3hppc_sws_il_pktgen_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, COMPILER_64_LO(uuMaxDurationB4StatsCollect) );
        }
      }

      sal_sleep( 2 );
      /* coverity[stack_use_overflow] */
      c3hppc_sws_il_stats_collect( pc3hppcTestInfo->nUnit );

      if (bShutDown) {
        COMPILER_64_SUB_64(uuTransmitDuration, uuTransmitDuration);
      } else {
        COMPILER_64_SUB_64(uuTransmitDuration, uuMaxDurationB4StatsCollect);
      }
    }


/*
    soc_sbx_caladan3_ppe_hd( pc3hppcTestInfo->nUnit, 1, NULL );
*/



    for ( nInstance = 0; nInstance < nIlInstanceNum; ++nInstance ) {
      if ( pc3hppcTestInfo->BringUpControl.auIlBringUp[nInstance] == 1 ) {
        c3hppc_sws_il_pktcap_wait_for_done( pc3hppcTestInfo->nUnit, nInstance, 1 );
        c3hppc_sws_il_pktcap_get( pc3hppcTestInfo->nUnit, nInstance, uPacketCaptureBuf, &uPacketCaptureLen,
                                  &uPacketCaptureSOP, &uPacketCaptureEOP, &uPacketCaptureERR,
                                  &uPacketCaptureBV, &uPacketCaptureChannel );
        cli_out("\nPacket of length [%d] captured on channel [%d] coming from PT%d:\n",
                uPacketCaptureLen, uPacketCaptureChannel, nInstance );
        for ( nIndex = 0; nIndex < ((uPacketCaptureLen + 3) / 4); ++nIndex ) {
          cli_out("Word[%d] --> 0x%08x\n", nIndex, uPacketCaptureBuf[nIndex] );
        }
      }
    }




    /****************************************************************************************************************************
     * EML chain destruction/construction final checking.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 

      if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      } else if ( g_bDumpEmlTables ) {
        if ( c3hppc_tmu_get_eml_tables(pc3hppcTestInfo->nUnit, nCmdFifoSelect,
                                       g_aEmlTableParameters[nEML_RootTableIndex].nTableID, g_aEmlTableParameters[nEML_ChainTableIndex].nTableID,
                                       g_aEmlTableParameters[nEML_RootTableIndex].uNumEntries, C3_EXERCISER_TEST1__UPDATER_TIMEOUT) ) {
          cli_out("ERROR:  \"c3hppc_tmu_get_eml_tables\" TIMEOUT  FAILURE:  c3hppc_tmu_are_cmd_fifos_empty %d  c3hppc_tmu_are_rsp_fifos_empty %d\n",
                  c3hppc_tmu_are_cmd_fifos_empty(), c3hppc_tmu_are_rsp_fifos_empty() );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
        if ( c3hppc_tmu_display_andor_scoreboard_eml_tables( pc3hppcTestInfo->nUnit, 
                                                             g_bDumpTmuDebugInfo, "/home/morrier/EML_TableDump_Final",
                                                             g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                                             g_aEmlTableParameters[nEML_ChainTableIndex].nTableID,
                                                             1, pc3hppcTestInfo->nMaxKey ) ) { 
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      }

      for ( nCmdFifoSelect = 0; nCmdFifoSelect < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifoSelect ) { 
        sal_free( apKeyDeleteList[nCmdFifoSelect] );
        sal_free( apKeyReInsertList[nCmdFifoSelect] ); 
      }

    }



    /****************************************************************************************************************************
     * Ensure that there are no outstanding Updater operations.
     *****************************************************************************************************************************/
    if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,600) == 0 ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }


  }   /* if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp &&  */


  if ( pEML_InsertList != NULL ) sal_free( pEML_InsertList );

  return 0;
}




int
c3_exerciser_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg, uBaseOffset;
  int nErrorCnt, nTotalErrorCnt, nMissLookupErrorCnt, nStartIndex, nIndex, nOcmPort;
  uint64 *pTotalLookupCount, uuTotalEmlLookups_cmu;
  uint64 *pMatchLookupCount;
  uint64 *pMissLookupCount;
  uint64 *pPktCount, *pByteCount, uuCopByteCount, uuExpectPktCount, uuExpectByteCount;
  uint64 uuTotalSearches, uuTotalPktCount, uuTotalByteCount;
  int nKey, nCounter, nInstance, nChannel, nExpectInterface;
  c3hppc_cmu_segment_info_t *pCmuSegment0Info, *pCmuSegment1Info;
  char sMessage[16];
  c3hppc_64b_ocm_entry_template_t *pOcmBlock0, *pOcmBlock1;
  int nDmaBlockSize;
  int nCopyWordCount;
  c3hppc_64b_ocm_entry_template_t OcmEntry;
  uint32 uBWinMpps, uExpectedMinBWinMpps;
  double dBWinMpps;

/*
  c3hppc_sws_il_stats_display_nonzero( 0 );
  c3hppc_sws_il_stats_display_nonzero( 1 );
*/
  uExpectedMinBWinMpps = uBWinMpps = 0; 

  if ( pc3hppcTestInfo->bLineTrafficGenOnly ) {

    if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE, C3HPPC_TEST__LINE_INTERFACE, 0 ) )
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else if ( pc3hppcTestInfo->bFabricTrafficGenOnly ) {

    if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE, C3HPPC_TEST__FABRIC_INTERFACE, 0 ) )
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else if ( c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__LINE_INTERFACE, C3HPPC_TEST__FABRIC_INTERFACE, 0 ) ||
              c3hppc_check_stats( pc3hppcTestInfo->nUnit, C3HPPC_TEST__FABRIC_INTERFACE, C3HPPC_TEST__LINE_INTERFACE, 0 ) ) {

    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
  }



  /****************************************************************************************************************************
   * Dump/display performance monitor statistics.  Set "nStartIndex" to '0' if histogram data is desired.
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->BringUpControl.uTmuBringUp == 1 ) { 
    uint64 uuRes;
    nStartIndex = ( pc3hppcTestInfo->nNumberOfCIs == C3HPPC_TMU_CI_INSTANCE_NUM ) ? 425 : 500;

    COMPILER_64_ZERO(uuTotalSearches);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 0, nStartIndex, g_uPmBucketShift, "EML-SK0 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 1, nStartIndex, g_uPmBucketShift, "EML-SK0 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    cli_out("\nINFO:  SubKey0 EML search count --> %lld \n\n", uuTotalSearches );

    COMPILER_64_ZERO(uuTotalSearches);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "EMC-SK0 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "EMC-SK0 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    cli_out("\nINFO:  SubKey0 EMC search count --> %lld \n\n", uuTotalSearches );

    COMPILER_64_ZERO(uuTotalSearches);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "IPV4-SK0 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "IPV4-SK0 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches,uuRes);
    cli_out("\nINFO:  SubKey0 IPV4 search count --> %lld \n\n", uuTotalSearches );

    COMPILER_64_ZERO(uuTotalSearches);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 6, nStartIndex, g_uPmBucketShift, "IPV6-SK0 EVEN" );
    COMPILER_64_ADD_64(uuTotalSearches, uuRes);
    uuRes = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 7, nStartIndex, g_uPmBucketShift, "IPV6-SK0 ODD" );
    COMPILER_64_ADD_64(uuTotalSearches, uuRes);
    cli_out("\nINFO:  SubKey0 IPV6 search count --> %lld \n\n", uuTotalSearches );
  }



  nErrorCnt = 0;
  nMissLookupErrorCnt = 0;
  COMPILER_64_ZERO(uuTotalEmlLookups_cmu);
  pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
  pCmuSegment1Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
  for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
    uint64 uuTmp;
    pTotalLookupCount = pCmuSegment0Info->pSegmentPciBase + ((2 * nKey) + 1);
    COMPILER_64_ADD_64(uuTotalEmlLookups_cmu, *pTotalLookupCount);
    pMatchLookupCount = pCmuSegment1Info->pSegmentPciBase + ((2 * nKey) + 1);
    pMissLookupCount =  pMatchLookupCount + (2 * C3_EXERCISER_TEST1__MAX_KEYS);
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pMatchLookupCount), COMPILER_64_LO(*pMatchLookupCount));
    COMPILER_64_ADD_64(uuTmp, *pMissLookupCount);
    if ( COMPILER_64_NE(*pTotalLookupCount, uuTmp) ) {
      if ( nErrorCnt < 128 ) {
        uint64 uuTmp2;
        COMPILER_64_SET(uuTmp2, COMPILER_64_HI(*pMatchLookupCount), COMPILER_64_LO(*pMatchLookupCount));
        COMPILER_64_ADD_64(uuTmp2, *pMissLookupCount);
        cli_out("<c3_exerciser_test1__done> -- Lookup count MISCOMPARE for Key[0x%04x]  Actual: %lld   Expect: %lld -- match[%lld] miss[%lld] \n",
                nKey, uuTmp2, *pTotalLookupCount, *pMatchLookupCount, *pMissLookupCount );
      }
      ++nErrorCnt;
    }

    if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 
      if ( !COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 0 ) {
        cli_out("\nERROR: -- UNEXPECTED non-zero MISS Lookup count for Key[0x%04x]  Actual: 0x%llx \n",
                nKey, *pMissLookupCount );
        ++nErrorCnt;
      }
      if ( COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 1 ) {
        cli_out("<c3_exerciser_test1__done> -- UNEXPECTED zero MISS Lookup count for Key[0x%04x]\n", nKey );
        ++nMissLookupErrorCnt;
      }
    }

  }
  if ( nMissLookupErrorCnt > 4 ) nErrorCnt += nMissLookupErrorCnt;

  cli_out("\n\n<c3_exerciser_test1__done> -- Total EML look-ups counted by the CMU --> %lld\n\n", uuTotalEmlLookups_cmu );


  nTotalErrorCnt = nErrorCnt;
  nErrorCnt = 0;
  for ( nInstance = 0; nInstance < C3HPPC_SWS_IL_INSTANCE_NUM; ++nInstance ) {
    pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 2 + nInstance;

    if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
        /* coverity[secure_coding] */
        sal_strcpy( sMessage, "INGRESS" );
    } else {
        /* coverity[secure_coding] */
        sal_strcpy( sMessage, "EGRESS" );
    }

    COMPILER_64_ZERO(uuTotalPktCount);
    COMPILER_64_ZERO(uuTotalByteCount);

    if ( pc3hppcTestInfo->bLineTrafficGenOnly ) nExpectInterface = C3HPPC_TEST__LINE_INTERFACE;
    else if ( pc3hppcTestInfo->bFabricTrafficGenOnly ) nExpectInterface = C3HPPC_TEST__FABRIC_INTERFACE;
    else nExpectInterface = nInstance;

    for ( nChannel = 0; nChannel < C3HPPC_SWS_IL_MAX_CHANNEL_NUM; ++nChannel ) {
      pByteCount = pCmuSegment0Info->pSegmentPciBase + ( 2 * nChannel);
      pPktCount = pByteCount + 1;
      COMPILER_64_ADD_64(uuTotalPktCount, *pPktCount);
      COMPILER_64_ADD_64(uuTotalByteCount, *pByteCount);
      uuExpectPktCount = c3hppc_sws_il_stats_getrx( nExpectInterface, nChannel, ILRXSTAT__PKT );
      uuExpectByteCount = c3hppc_sws_il_stats_getrx( nExpectInterface, nChannel, ILRXSTAT__BYTE );
      if ( nInstance == C3HPPC_TEST__LINE_INTERFACE ) {
        uint64 uuTmp;
        
        COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuExpectPktCount), COMPILER_64_LO(uuExpectPktCount));
        COMPILER_64_UMUL_32(uuTmp, 0);
        COMPILER_64_ADD_64(uuExpectByteCount, uuTmp);
      }
      c3hppc_cop_coherent_table_read_write( pc3hppcTestInfo->nUnit, nInstance, 0, nChannel, 0, OcmEntry.uData );
      COMPILER_64_SET(uuCopByteCount, OcmEntry.uData[1],OcmEntry.uData[0]); 
      if ( COMPILER_64_NE(*pPktCount, uuExpectPktCount) ) {
        cli_out("<c3_exerciser_test1__done> -- %s CMU channel[%d] \"PKT\" count MISCOMPARE   Actual: %lld   Expect: %lld \n",
                sMessage, nChannel, *pPktCount, uuExpectPktCount );
        ++nErrorCnt;
      }
      if ( COMPILER_64_NE(*pByteCount, uuExpectByteCount) ) {
        cli_out("<c3_exerciser_test1__done> -- %s CMU channel[%d] \"BYTE\" count MISCOMPARE   Actual: %lld   Expect: %lld \n",
                sMessage, nChannel, *pByteCount, uuExpectByteCount );
        ++nErrorCnt;
      }

      if ( COMPILER_64_NE(uuCopByteCount, uuExpectByteCount) ) {
        cli_out("<c3_exerciser_test1__done> -- %s COP channel[%d] \"BYTE\" count MISCOMPARE   Actual: %lld   Expect: %lld \n",
                sMessage, nChannel, uuCopByteCount, uuExpectByteCount );
        ++nErrorCnt;
      }
    }

    cli_out("\nINFO: Total %s packet/byte counts --> %lld/%lld \n\n", sMessage, uuTotalPktCount, uuTotalByteCount );

    /* C3HPPC_TEST_150MPPS_PACKET_SIZE only applies to A0/A1 with the IL packet generator bug */
    if ( pc3hppcTestInfo->nNumberOfCIs == C3HPPC_TMU_CI_INSTANCE_NUM && pc3hppcTestInfo->nDramFreq >= 933 &&
         (g_rev_id == BCM88030_B0_REV_ID || pc3hppcTestInfo->nPacketSize == C3HPPC_TEST_150MPPS_PACKET_SIZE) ) {
      if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {
        uExpectedMinBWinMpps = 71;
      } else if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100G_BY_100IL ) {
        /* A randomly selected stream can result in constant IPv4/IPv6 lookups in conjuction with all the other TMU
           activity that will extend the epoch and not maintain line-rate. */ 
        uExpectedMinBWinMpps = 110;
      } else if ( g_rev_id == BCM88030_B0_REV_ID ) {
        switch (pc3hppcTestInfo->nPacketSize) {
          case 128:                             uExpectedMinBWinMpps = 95; break;
          default:                              uExpectedMinBWinMpps = 140;
        }
      } else {
        switch (pc3hppcTestInfo->nPacketSize) {
          case 64:                              uExpectedMinBWinMpps = 115; break;
          case 80:                              uExpectedMinBWinMpps = 95;  break;
          case 110:                             uExpectedMinBWinMpps = 70;  break;
          case 164:                             uExpectedMinBWinMpps = 45;  break;
          case C3HPPC_TEST_150MPPS_PACKET_SIZE: 
          default:                              uExpectedMinBWinMpps = 140;
        }
      }
      if ( pc3hppcTestInfo->nTDM != C3HPPC_SWS_TDM__48x1G_BY_100IL && pc3hppcTestInfo->nDramFreq == 1066 ) {
        /*
           A small hit in bandwidth is likely to occur due to the search record sizes in conjunction with
           the inefficiency of the 1066 DRAM parameters.
        */
        uExpectedMinBWinMpps -= 4;
      }

      /*
        uBWinMpps = (uint32) ( uuTotalPktCount / pc3hppcTestInfo->uuIterations );
      */
      if (soc_sbx_div64(uuTotalPktCount, COMPILER_64_LO(pc3hppcTestInfo->uuIterations), &uBWinMpps) == SOC_E_PARAM) return SOC_E_INTERNAL;
      uBWinMpps /= 1000000;
      if ( uBWinMpps < uExpectedMinBWinMpps && pc3hppcTestInfo->nModulateTimer == 0 ) {
        cli_out("\nERROR: %s Minimum bandwidth check MISCOMPARE -->   Actual: %dMpps   Expect(min): %dMpps  \n\n",
                sMessage, uBWinMpps, uExpectedMinBWinMpps );
        ++nErrorCnt;
      } else {
        dBWinMpps = (double) ( ((double) uuTotalPktCount) / ((double) pc3hppcTestInfo->uuIterations) );
        dBWinMpps /= 1000000.0;
        cli_out("INFO: %s bandwidth --> %.2fMpps \n\n", sMessage, dBWinMpps );
      }
    } else {
      dBWinMpps = (double) ( ((double) uuTotalPktCount) / ((double) pc3hppcTestInfo->uuIterations) );
      dBWinMpps /= 1000000.0;
      cli_out("INFO: %s bandwidth --> %.2fMpps \n\n", sMessage, dBWinMpps );
    }

  }
  nTotalErrorCnt += nErrorCnt;


  if ( pc3hppcTestInfo->nHostActivityControl == C3_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 
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
    cli_out("\n<c3_exerciser_test1__done> -- %d keys should have been left UN-TOUCHED ...\n\n", nCounter );
  }


  /****************************************************************************************************************************
   * Verify data block whereby an affective "copy" operation is done using a load-store sequence.
   ***************************************************************************************************************************/
  nDmaBlockSize = C3_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM;
  pOcmBlock0 = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                 nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                 "ocm_block");
  pOcmBlock1 = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                 nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                 "ocm_block");
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(C3_EXERCISER_TEST1_WDT_PORT);
  uBaseOffset = C3_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET; 
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                             uBaseOffset, (uBaseOffset+nDmaBlockSize-1), 0, pOcmBlock0->uData );
  uBaseOffset = C3_EXERCISER_TEST1_WDT_PORT_LRP_STORE_ACCESS_BASE_OFFSET; 
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                             uBaseOffset, (uBaseOffset+nDmaBlockSize-1), 0, pOcmBlock1->uData );

  nErrorCnt = 0;
  nCopyWordCount = 0;
  for ( nIndex = 0; nIndex < nDmaBlockSize; ++nIndex ) {
    if ( (pOcmBlock1[nIndex].uData[1] && pOcmBlock0[nIndex].uData[1] != pOcmBlock1[nIndex].uData[1]) ||
         (pOcmBlock1[nIndex].uData[0] && pOcmBlock0[nIndex].uData[0] != pOcmBlock1[nIndex].uData[0]) ) {
      cli_out("\nERROR: -- STORE MISCOMPARE at Index[%d] -->  Actual: 0x%08x_%08x  Expect: 0x%08x_%08x \n",
              nIndex, pOcmBlock1[nIndex].uData[1], pOcmBlock1[nIndex].uData[0], pOcmBlock0[nIndex].uData[1], pOcmBlock0[nIndex].uData[0] );
      ++nErrorCnt;
    }
    if ( pOcmBlock1[nIndex].uData[1] ) ++nCopyWordCount;
  }
  /* 
     If "bFabricTrafficGenOnly" the ingress PP parsing will not match and the initial stream 0 will always be selected.
     Therefore, no words will be copied as this is done in stream 1.  Disable the check!
  if ( nCopyWordCount == 0 && pc3hppcTestInfo->bFabricTrafficGenOnly == 0 ) {
  */
  if ( nCopyWordCount == 0 ) {
    /*
       Stream selection is random so with the ethernet TDMs (low number of active channels) it is possible to not select STREAM 1.
       Not failing the test with those TDMs.
    */
    if ( pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS ||
         pc3hppcTestInfo->nTDM == C3HPPC_SWS_TDM__48x1G_BY_100IL ) {
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }
    cli_out("\n<c3_exerciser_test1__done> -- NO words copied by load-store sequence on STREAM 1!\n\n" );
  }
  nTotalErrorCnt += nErrorCnt;
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock0);
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock1);



  if ( nTotalErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3_exerciser_test1__done> -- Total error count --> %d\n", nTotalErrorCnt );
  }


  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 48 );
#if 0
  cli_out("\nMAILBOX: 0x%08x\n", uReg );
#endif
  if ( uReg ) {
      if (      uReg & 0x80000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "TSR ERROR" );
      } else if ( uReg & 0x40000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "RCE MISCOMPARE" );
      } else if ( uReg & 0x20000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "EML MISCOMPARE" );
      } else if ( uReg & 0x10000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "EMC MISCOMPARE" );
      } else if ( uReg & 0x08000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "DM MISCOMPARE" );
      } else if ( uReg & 0x04000000 ) { 
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "PORT MISCOMPARE" );
      } else if ( uReg & 0x02000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "ETU MISCOMPARE" );
      } else if ( uReg & 0x01000000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "IPV4 MISCOMPARE" );
      } else if ( uReg & 0x00800000 ) {
          /* coverity[secure_coding] */
          sal_strcpy( sMessage, "IPV6 MISCOMPARE" );
      }

    if (      uReg & 0x80000000 ) {
      cli_out("\nERROR:  %s indication received on FlowID[0x%03x] Stream[%d] Segment[%d] SK1ErrCode[0x%x] SK0ErrCode[0x%x]\n",
              sMessage, (uReg & 0x3ff), ((uReg >> 16) & 0xf), ((uReg >> 12) & 0xf), ((uReg >> 24) & 0xf), ((uReg >> 20) & 0xf) );
    } else {
      cli_out("\nERROR:  %s indication received on FlowID[0x%03x] Stream[%d] Segment[%d]\n",
              sMessage, (uReg & 0x3ff), ((uReg >> 16) & 0xf), ((uReg >> 12) & 0xf) );
    }
  }


  return 0;

#endif
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
