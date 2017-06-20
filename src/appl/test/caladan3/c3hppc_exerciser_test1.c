/* $Id: c3hppc_exerciser_test1.c,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>



#ifdef BCM_CALADAN3_SUPPORT

/* ################### UNCOMMENT WHEN COMPILING FOR EMULATION ######################

#define C3HPPC_EXERCISER_TEST1__EMULATION 1

*/


#include "../c3hppc_test.h"
 
#define C3HPPC_EXERCISER_TEST1__STREAM_NUM                                (12)

#define C3HPPC_EXERCISER_TEST1_RCE_RESULT_PORT                            (9)
#define C3HPPC_EXERCISER_TEST1_WDT_PORT                                   (2)
#define C3HPPC_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET       (0x0000c000)
#define C3HPPC_EXERCISER_TEST1_WDT_PORT_LRP_STORE_ACCESS_BASE_OFFSET      (0x0000d000)


#define C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID                           (0xffff)
#define C3HPPC_EXERCISER_TEST1__STREAMX_S0_LRP_KEY_PROGRAM_NUMBER         (0)
#define C3HPPC_EXERCISER_TEST1__STREAM1_S0_LRP_KEY_PROGRAM_NUMBER         (1)
#define C3HPPC_EXERCISER_TEST1__STREAM2_S0_LRP_KEY_PROGRAM_NUMBER         (13)
#define C3HPPC_EXERCISER_TEST1__STREAM3_S0_LRP_KEY_PROGRAM_NUMBER         (2)
#define C3HPPC_EXERCISER_TEST1__STREAM5_S0_LRP_KEY_PROGRAM_NUMBER         (5)
#define C3HPPC_EXERCISER_TEST1__STREAM7_S0_LRP_KEY_PROGRAM_NUMBER         (15)
#define C3HPPC_EXERCISER_TEST1__STREAM9_S0_LRP_KEY_PROGRAM_NUMBER         (6)
#define C3HPPC_EXERCISER_TEST1__STREAM11_S0_LRP_KEY_PROGRAM_NUMBER        (14)
#define C3HPPC_EXERCISER_TEST1__STREAMX_S1_LRP_KEY_PROGRAM_NUMBER         (3)
#define C3HPPC_EXERCISER_TEST1__STREAM1_S1_LRP_KEY_PROGRAM_NUMBER         (4)
#define C3HPPC_EXERCISER_TEST1__STREAM2_S1_LRP_KEY_PROGRAM_NUMBER         (7)
#define C3HPPC_EXERCISER_TEST1__STREAM3_S1_LRP_KEY_PROGRAM_NUMBER         (8)
#define C3HPPC_EXERCISER_TEST1__STREAM6_S1_LRP_KEY_PROGRAM_NUMBER         (9)
#define C3HPPC_EXERCISER_TEST1__STREAM7_S1_LRP_KEY_PROGRAM_NUMBER         (10)
#define C3HPPC_EXERCISER_TEST1__STREAM10_S1_LRP_KEY_PROGRAM_NUMBER        (11)
#define C3HPPC_EXERCISER_TEST1__STREAM11_S1_LRP_KEY_PROGRAM_NUMBER        (12)
#define C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM               (0)
#define C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM               (1)

#define C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM                        (0x01000)
#define C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRIES_PER_ROW                  (1)

#define C3HPPC_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM                  (0x01000)

/*
  When running on the emulator modify C3HPPC_EXERCISER_TEST1__MAX_KEYS to be < 4096 and re-compile.
  Change C3HPPC_EXERCISER_TEST1__MAX_KEYS_EMULATOR_FACTOR from 1 to 16 when emulation.
*/
#ifdef C3HPPC_EXERCISER_TEST1__EMULATION
#define C3HPPC_EXERCISER_TEST1__MAX_KEYS_EMULATOR_FACTOR                  16
#else
#define C3HPPC_EXERCISER_TEST1__MAX_KEYS_EMULATOR_FACTOR                  1
#endif

#define C3HPPC_EXERCISER_TEST1__MAX_KEYS                                  (0x10000 / C3HPPC_EXERCISER_TEST1__MAX_KEYS_EMULATOR_FACTOR)
#define C3HPPC_EXERCISER_TEST1__EML_MAX_KEYS                              (C3HPPC_EXERCISER_TEST1__MAX_KEYS) 
#define C3HPPC_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM                (C3HPPC_EXERCISER_TEST1__EML_MAX_KEYS)
#define C3HPPC_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRIES_PER_ROW          (8) 
#define C3HPPC_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRY_NUM               C3HPPC_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM
#define C3HPPC_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRIES_PER_ROW         (1)
#define C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM                         (0)

#define C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM                     (C3HPPC_EXERCISER_TEST1__MAX_KEYS)
#define C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW               (16)
#define C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM                         (1)

#define C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS                              (C3HPPC_EXERCISER_TEST1__MAX_KEYS) 
#define C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0                  (0)
#define C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM                        (0)

#define C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT                           (0)
/* Values of 0 and 704 have been used */
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE                      (0)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM                     (256)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PREFIX_SIZE                   (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_PREFIX_SIZE                   (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM                 (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM           (32)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
/* The multiply by 2 is due to the bucket pair concept */
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM            (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * \
                                                                           C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM * 2)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW      (4)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM                       (4)

#define C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT                           (1)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT_BASE                      (C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE + \
                                                                           (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM / 4)) 
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM                     (256)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PREFIX_SIZE                   (144)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_PREFIX_SIZE                   (128)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM                 (72)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM           (32)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_MAX_PREFIX_NUM        (5)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM  (4)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM            (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * \
                                                                           C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM * 2)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW      (4)
#define C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM                       (5)

#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT                     (2)
/*
  When running on the emulator modify C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT_BASE to be 0 as the image
  fails to contain TAPS TCAM rows greater than 320.  A WARNING is given in the "memories" step when compiling
  the TCAM models.
*/
#ifdef C3HPPC_EXERCISER_TEST1__EMULATION
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT_BASE                (0)
#else
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT_BASE                (C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT_BASE + \
                                                                           C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM)
#endif
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM               (256)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PREFIX_SIZE             (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_PREFIX_SIZE             (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_MAX_PIVOT_NUM           (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM     (32)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_TMU_PROGRAM                 (6)

#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT                     (3)
/*
  When running on the emulator modify C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT_BASE to be 0 as the image
  fails to contain TAPS TCAM rows greater than 320.  A WARNING is given in the "memories" step when compiling
  the TCAM models.
*/
#ifdef C3HPPC_EXERCISER_TEST1__EMULATION
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT_BASE                (0) 
#else
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT_BASE                (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT_BASE + \
                                                                           (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM / 4)) 
#endif

#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM               (256)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PREFIX_SIZE             (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_PREFIX_SIZE             (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_MAX_PIVOT_NUM           (48)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM     (32)
#define C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_TMU_PROGRAM                 (7)

#define C3HPPC_EXERCISER_TEST1__LPMIPV4_EML64_TMU_PROGRAM                 (8)



#define C3HPPC_EXERCISER_TEST1__UPDATER_TIMEOUT                     (20)

#define C3HPPC_EXERCISER_TEST1__TSR_ERROR_MASK                      (0x40000000)

#define C3HPPC_EXERCISER_TEST1__NO_ROOT_HITS                        (1) 
#define C3HPPC_EXERCISER_TEST1__EML_INSERT                          (2) 
#define C3HPPC_EXERCISER_TEST1__DO_HASHING                          (3) 
#define C3HPPC_EXERCISER_TEST1__LOGICAL                             (1) 
#define C3HPPC_EXERCISER_TEST1__PHYSICAL                            (2) 

#define C3HPPC_EXERCISER_TEST1__TABLE_CONTENTION                    (1)
#define C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE                  (2)
#define C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING    (4)


static c3hppc_test_dm_table_parameters_t
               g_aDmTableParameters[] = {
  /* lrp_segment/tmu_table_id = 128/0 */  { 0, 0, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM494,
                                            C3HPPC_LRP_LOOKUP__DM494, 0,  8, 1, 64 },
  /* lrp_segment/tmu_table_id = 129/1 */  { 1, 0, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 8,  2, 1, 64 },
  /* lrp_segment/tmu_table_id = 130/2 */  { 2, 1, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 10, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 131/3 */  { 3, 2, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 12, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 132/4 */  { 4, 3, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM119,
                                            C3HPPC_LRP_LOOKUP__DM119, 14, 2, 1, 64 },
  /* lrp_segment/tmu_table_id = 133/5 */  { 5, 0, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 16, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 134/6 */  { 6, 2, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 20, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 135/7 */  { 7, 1, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM247,
                                            C3HPPC_LRP_LOOKUP__DM247, 24, 4, 1, 64 },
  /* lrp_segment/tmu_table_id = 136/8 */  { 8, 0, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM366,
                                            C3HPPC_LRP_LOOKUP__DM366, 28, 6, 1, 64 },
  /* lrp_segment/tmu_table_id = 137/9 */  { 9, 1, C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM, C3HPPC_TMU_LOOKUP__DM366,
                                            C3HPPC_LRP_LOOKUP__DM366, 34, 6, 1, 64 }
                                        };
static c3hppc_test_em_table_parameters_t
               g_aEmlTableParameters[] = {
                                          { 'R', 10, C3HPPC_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__1ST_EML64, 0, C3HPPC_TMU_EML64_ROOT_TABLE_ENTRY_SIZE_IN_64b,
                                            C3HPPC_EXERCISER_TEST1__EML64_ROOT_TABLE_ENTRIES_PER_ROW, 0, (uint32 *)NULL },
                                          { 'C', 11, C3HPPC_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__2ND_EML64, 0, 
                                            (C3HPPC_TMU_EML64_CHAIN_TABLE_CHAIN_ELEMENT_SIZE_IN_64b * C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM),
                                            C3HPPC_EXERCISER_TEST1__EML64_CHAIN_TABLE_ENTRIES_PER_ROW, 64, (uint32 *)NULL }
                                         };
/* For EMC the "next" table entry must preceed the associated "root" entry !!!! */
static c3hppc_test_em_table_parameters_t
               g_aEmcTableParameters[] = {
                                          { 'N', 13, C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__2ND_EMC64, 0, C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b, 
                                            C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW, 64, (uint32 *)NULL },
                                          { 'R', 12, C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRY_NUM,
                                            C3HPPC_TMU_LOOKUP__1ST_EMC64, 0, C3HPPC_TMU_EMC64_TABLE_ENTRY_SIZE_IN_64b,
                                            C3HPPC_EXERCISER_TEST1__EMC64_TABLE_ENTRIES_PER_ROW, 0, (uint32 *)NULL }
                                         };

static c3hppc_test_rce_program_parameters_t
               g_aRceProgramParameters[] = {
                                            { C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM, 0x0000,                         86, 32, 16, 0 },
                                            { C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM, (C3HPPC_RCE_INSTRUCTION_NUM/2), 43, 32, 15, 0 }
                                           };

static c3hppc_test_lrp_search_program_parameters_t
               g_aLrpSearchProgramParameters[] = {
                                                  { C3HPPC_EXERCISER_TEST1__STREAMX_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM1_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM3_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3HPPC_EXERCISER_TEST1__STREAMX_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM1_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM5_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM9_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM2_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM3_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM6_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM7_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM10_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM11_S1_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_43x32_15BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM2_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__RCE_86x32_16BIT_LPM_PROGRAM,
                                                    C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM11_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID },
                                                  { C3HPPC_EXERCISER_TEST1__STREAM7_S0_LRP_KEY_PROGRAM_NUMBER,
                                                    C3HPPC_EXERCISER_TEST1__LPMIPV4_EML64_TMU_PROGRAM, 1, 1,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID,
                                                    C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID }
                                                 };


static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;

static uint32 g_uPmBucketShift = 2;

static uint8 g_bDoInitXLreads = 0;
static uint8 g_bDumpEmlTables = 0;
static uint8 g_bAllowLateResults = 0;
static uint8 g_bDumpTmuDebugInfo = 0;

static char g_acKeyLookupMissScoreBoard[C3HPPC_EXERCISER_TEST1__MAX_KEYS];

static int g_nWatchDogTimerCopSegment;

int
c3hppc_exerciser_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance, nSegment;
  uint32 uResultsTimer;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  c3hppc_cop_watchdogtimer_state32b_entry_ut WatchDogTimer32bStateEntry;
  c3hppc_cop_watchdogtimer_state64b_entry_ut WatchDogTimer64bStateEntry;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nOcmPort, nDmaBlockSize, nTimer, nMode32;
  uint64 uuWatchDogTimerStateEntry;
  

  g_bDoInitXLreads = ( SAL_BOOT_QUICKTURN ) ? 0 : 1;
  
  pc3hppcTestInfo->BringUpControl.uDefaultPhysicalSegmentSizeIn16kBlocks = 4;
  pc3hppcTestInfo->BringUpControl.uDefaultCmuPhysicalSegmentSizeIn16kBlocks = 4; 
  if ( !SAL_BOOT_QUICKTURN ) {
    /* Need to run TURBO_64b counters for SV due to counter manager ring performance limitations. */
    /* For TURBO_64b need twice as much space in OCM to hold counters.                            */
    pc3hppcTestInfo->BringUpControl.uDefaultCmuPhysicalSegmentSizeIn16kBlocks *= 2;
  }
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  pc3hppcTestInfo->BringUpControl.uTmuBringUp = 1;
  pc3hppcTestInfo->BringUpControl.uRceBringUp = ( pc3hppcTestInfo->bNoRce == 1 ) ? 0 : 1;
  pc3hppcTestInfo->BringUpControl.uEtuBringUp = ( pc3hppcTestInfo->bNoEtu == 1 ) ? 0 : 1;
  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;
  pc3hppcTestInfo->bIPV4Enable = 1;
  pc3hppcTestInfo->bIPV6Enable = 1;

  if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 1 ) {
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "exerciser_test1a.oasm");
  } else if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 0 && pc3hppcTestInfo->BringUpControl.uRceBringUp == 0 ) {
    if ( SAL_BOOT_QUICKTURN ) {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "exerciser_test1a_noetu_norce_emul.oasm");
    } else {
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "exerciser_test1a_noetu_norce.oasm");
    }
    g_aLrpSearchProgramParameters[C3HPPC_EXERCISER_TEST1__STREAM1_S0_LRP_KEY_PROGRAM_NUMBER].uTmuProgram =
                                                           C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_TMU_PROGRAM; 
  } else {
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "exerciser_test1a_noetu.oasm");
  }
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0, 3);

  pc3hppcTestInfo->BringUpControl.bTmuEMC128Mode = 1;
  pc3hppcTestInfo->BringUpControl.uTmuEmlMaxProvisionedKey = C3HPPC_EXERCISER_TEST1__MAX_KEYS;


  /* CMU counters for EML lookups */
  if ( pc3hppcTestInfo->BringUpControl.bTmuHwEmlChainManagement ) {
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[0].uSegmentLimit = pc3hppcTestInfo->nMaxKey - 1;
    pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[1].uSegmentLimit = 
                                    C3HPPC_EXERCISER_TEST1__MAX_KEYS + pc3hppcTestInfo->nMaxKey - 1;
  }

  /* CMU segments for the begin and end of epoch counters used to detect proper bank swapping */
  for ( nSegment = 2; nSegment < 6; ++nSegment ) {
    pCmuSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[nSegment]);
    pCmuSegmentInfo->bValid = 1;
    pCmuSegmentInfo->uSegment = (uint32) nSegment;
    pCmuSegmentInfo->uSegmentType = ( SAL_BOOT_QUICKTURN ) ? C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b :
                                                             C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
    pCmuSegmentInfo->uSegmentLimit = C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE - 1;
    pCmuSegmentInfo->uSegmentPort = nSegment & 1;
    pCmuSegmentInfo->nStartingPhysicalBlock = ( nSegment < 4 ) ? 62 : 63;
    pCmuSegmentInfo->uSegmentOcmBase = 
                     (uint32) pCmuSegmentInfo->nStartingPhysicalBlock << C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT;
  }

  g_nWatchDogTimerCopSegment = 1;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][g_nWatchDogTimerCopSegment]);
    pCopSegmentInfo->uMode64 = 1;
    pCopSegmentInfo->bValid = 1;
    pCopSegmentInfo->uSegmentLimit = pc3hppcTestInfo->nCounterNum - 1;
    pCopSegmentInfo->uSegmentOcmLimit = (((1 + pCopSegmentInfo->uMode64) * pc3hppcTestInfo->nCounterNum) / 2) - 1;
    pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__TIMER;
    pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    pCopSegmentInfo->nStartingPhysicalBlock = ( nCopInstance ) ? 28 : 40;
    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod,0, 32 * pc3hppcTestInfo->nCounterNum);
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

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    /* Initialize timer state */
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][g_nWatchDogTimerCopSegment]);
    nMode32 = ( pCopSegmentInfo->uMode64 ) ? 0 : 1;
    nDmaBlockSize = pc3hppcTestInfo->nCounterNum / (1 + nMode32);
    pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                  nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                  "ocm_block");
    for ( nTimer = 0; nTimer <= (int) pCopSegmentInfo->uSegmentLimit; nTimer += (1+nMode32) ) {
      if ( nMode32 ) {
        WatchDogTimer32bStateEntry.bits.Timer_1 = (1 * (nTimer+1));
        WatchDogTimer32bStateEntry.bits.Timer_0 = (1 * (nTimer+2));
        uuWatchDogTimerStateEntry = WatchDogTimer32bStateEntry.value;
      } else {
        WatchDogTimer64bStateEntry.bits.Timer = (1 * (nTimer+1));
        WatchDogTimer64bStateEntry.bits.Timeout = WatchDogTimer64bStateEntry.bits.Timer;
        uuWatchDogTimerStateEntry = WatchDogTimer64bStateEntry.value;
      }
      pOcmBlock[nTimer>>nMode32].uData[1] = COMPILER_64_HI(uuWatchDogTimerStateEntry);
      pOcmBlock[nTimer>>nMode32].uData[0] = COMPILER_64_LO(uuWatchDogTimerStateEntry);
    }
    nOcmPort = c3hppc_ocm_map_cop2ocm_port(nCopInstance);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, pCopSegmentInfo->uSegmentBase,
                               (pCopSegmentInfo->uSegmentBase+nDmaBlockSize-1), 1, pOcmBlock->uData );
    soc_cm_sfree( pc3hppcTestInfo->nUnit, pOcmBlock );

  }


  c3hppc_tmu_region_map_setup( pc3hppcTestInfo->nUnit, C3HPPC_TMU_REGION_LAYOUT__RANDOM );

  uResultsTimer = 768;
  if ( pc3hppcTestInfo->nNumberOfCIs != C3HPPC_TMU_CI_INSTANCE_NUM ) {
    uResultsTimer *= 2;
  }
  c3hppc_lrp_set_results_timer( pc3hppcTestInfo->nUnit, uResultsTimer );


  /* Enable COP segment 0 used for test management. */
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    c3hppc_cop_program_segment_enable( pc3hppcTestInfo->nUnit, nCopInstance, 0 );
  }


  c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                              pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                       pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );


  return 0;
}

int
c3hppc_exerciser_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int  nIndex, nTimeOut, nTable, nProgram, nOcmPort, nCopInstance, nLrpPort, nTP, nSetupOptions, nKey, nKey1, nSubKey;
  uint32 *pDmTableDmaSratchPad;
  uint32 uReg, uContexts, uBaseOffset;
  uint32 uColumnOffset, uRegionRowOffset; 
  uint32 uDeadlineOffset;
  uint32 *pEML_InsertList, uEML_InsertEntrySizeInBytes;
  int    nEML_RootTableIndex, nEML_ChainTableIndex, nEMC_RootTableIndex, nEMC_NextTableIndex;
  uint32 uEMC_EntrySizeIn64b;
  uint32 *pIPV4BucketTable, *apIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], uIPV4AssocDataTableEntryNum;
  uint32 uIPV4BucketEntrySizeIn64b, uIPV4BucketEntriesPerRow;
  uint32 uIPV4AssocDataEntrySizeIn64b, uIPV4AssocDataEntriesPerRow;
  uint32 auIPV4BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV4AssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 *pIPV6BucketTable, *apIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV6BucketEntrySizeIn64b, uIPV6BucketEntriesPerRow;
  uint32 uIPV6KeyAndAssocDataEntrySizeIn64b, uIPV6KeyAndAssocDataEntriesPerRow;
  uint32 auIPV6BucketTable[C3HPPC_TMU_TAPS_INSTANCE_NUM], auIPV6KeyAndAssocDataTable[C3HPPC_TMU_TAPS_INSTANCE_NUM];
  uint32 uIPV6KeyAndAssocDataTableEntryNum;
  uint32 *pRPBcommands, *pBBXcommands, *pBRRcommands;
  uint32 *apKeyDeleteList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM], *apKeyReInsertList[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 auKeyDeleteListLength[C3HPPC_TMU_UPDATE_CMD_FIFO_NUM];
  uint32 uInsertEntrySizeIn32b, uKeySizeIn32b, uKeyFilter, uKeyFilterMask, uKey;
  int nCmdFifoSelect;
  int nBulkDelete;
  uint64 uuActionThreshold;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  c3hppc_64b_ocm_entry_template_t uOcmEntryInit;
  int nDmaBlockSize;
  uint64 auuKeyData[8];
  uint32 uFilterSetIndex, uSBlkIndex, uColumnIndex;
  char sUcodeFileName[64];
  int nTargetBankSelect;
  int nEpochSize;
  uint32 uEtuKeySizeIn80bSegments, uSegment;
  c3hppc_etu_80b_data_t *pEtuKeyData, *pEtuKeyMask;
  uint64 uuEtuKeyMask, uuTmp;

  /* uint64 uuBubbleCntr; */
  /* int uBubbleCntrState;*/


  nSetupOptions = 0;
  nKey1 = 0;
  nEML_RootTableIndex = 0;
  nEML_ChainTableIndex = 0;
  nEMC_RootTableIndex = 0;
  nEMC_NextTableIndex = 0;
  pIPV4BucketTable = NULL;
  pIPV6BucketTable = NULL;
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
    g_pFlowTable[nIndex].uData[0] = (pc3hppcTestInfo->nMaxKey - 1) & 0xffff;
    g_pFlowTable[nIndex].uData[1] = sal_rand() & g_pFlowTable[nIndex].uData[0];
    g_pFlowTable[nIndex].uData[1] |= (sal_rand() & g_pFlowTable[nIndex].uData[0]) << 16;
    if ( pc3hppcTestInfo->nSetupOptions == C3HPPC_EXERCISER_TEST1__NO_ROOT_HITS ) g_pFlowTable[nIndex].uData[0] |= 0x00100000;
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) {
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
/*
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );
*/



  /* The following allocation is used to initialize OCM memory with RCE results.  */
  nDmaBlockSize = C3HPPC_EXERCISER_TEST1__MAX_KEYS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(C3HPPC_EXERCISER_TEST1_RCE_RESULT_PORT);

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
  nDmaBlockSize = C3HPPC_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    if ( nOcmPort != c3hppc_ocm_map_lrp2ocm_port(C3HPPC_EXERCISER_TEST1_RCE_RESULT_PORT) ) {
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
      uBaseOffset = ( nOcmPort == c3hppc_ocm_map_lrp2ocm_port(C3HPPC_EXERCISER_TEST1_WDT_PORT) ) ?
                                            C3HPPC_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET : 0;
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
            sal_alloc( (C3HPPC_EXERCISER_TEST1__DM_TABLE_ENTRY_NUM * C3HPPC_TMU_DM494_DATA_SIZE_IN_64b * sizeof(uint64)),
                       "dma scratch pad");

  for ( nTable = 0; nTable < COUNTOF(g_aDmTableParameters); ++nTable ) {
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
  uEML_InsertEntrySizeInBytes = C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b * sizeof(uint64);
  pEML_InsertList = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uEML_InsertEntrySizeInBytes),
                                          "EML64 Insert Table");

  for ( nTable = 0, nCmdFifoSelect = 0; nTable < COUNTOF(g_aEmlTableParameters); ++nTable ) {
    c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, g_aEmlTableParameters[nTable].nTableID, g_aEmlTableParameters[nTable].uLookUp,
                            g_aEmlTableParameters[nTable].uNumEntries, pc3hppcTestInfo->uReplicationFactor,
                            0, uRegionRowOffset, g_aEmlTableParameters[nTable].uColumnOffset,
                            g_aEmlTableParameters[nTable].uEntriesPerRow, g_aEmlTableParameters[nTable].uDeadlineOffset,
                            0, 1, 0, (C3HPPC_TMU_MAX_CHAIN_ELEMENT_NUM-1), 0xf, 0, 0, &uRegionRowOffset );

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
    c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM,
                                g_aEmlTableParameters[nEML_RootTableIndex].uLookUp,
                                (uint32) g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                                ((nSubKey == 1) ? 62 : 0), 2, 0, 0, 0 );
  }

  /* This is for the program that does IPV4 on sub-key0 and EML64 on sub-key1 */
  nSubKey = 1;
  /* coverity[dead_error_line] */
  c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_EXERCISER_TEST1__LPMIPV4_EML64_TMU_PROGRAM,
                              g_aEmlTableParameters[nEML_RootTableIndex].uLookUp,
                              (uint32) g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 0,
                              ((nSubKey == 1) ? 62 : 0), 2, 0, 0, 0 );

  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 0, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM );
  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 1, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_EXERCISER_TEST1__EML64_TMU_PROGRAM );

  for ( nTable = 0; nTable < COUNTOF(g_aEmlTableParameters); ++nTable ) {
    sal_free( g_aEmlTableParameters[nTable].pContents );
  }


  /****************************************************************************************************************************
   *
   * EMC Root and Next table setup
   *
   *****************************************************************************************************************************/

  for ( nTable = 0; nTable < COUNTOF(g_aEmcTableParameters); ++nTable ) {
    /* coverity[assigned_value] */
    nCmdFifoSelect = nTable & 1;
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
    c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nSubKey, C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM,
                                g_aEmcTableParameters[nEMC_RootTableIndex].uLookUp,
                                g_aEmcTableParameters[nEMC_RootTableIndex].nTableID, 0,
                                ((nSubKey == 1) ? 62 : 32), 2, 0, 0, 0 );
  }

  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 2, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM );
  c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 3, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_EXERCISER_TEST1__EMC64_TMU_PROGRAM );




  if ( pc3hppcTestInfo->bIPV4Enable ) {
    /****************************************************************************************************************************
     *
     * LPM IPV4 Mode2 setup
     *
     *****************************************************************************************************************************/

    auIPV4BucketTable[0] = g_aEmcTableParameters[nEMC_NextTableIndex].nTableID + 1;
    auIPV4BucketTable[1] = auIPV4BucketTable[0] + 2;
    uIPV4BucketEntrySizeIn64b = c3hppc_tmu_calc_ipv4_bucket_table_entry_size_in_64b( pc3hppcTestInfo->nIPV4BucketPrefixNum );
    uIPV4BucketEntriesPerRow = C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV4AssocDataTable[0] = auIPV4BucketTable[0] + 1;
    auIPV4AssocDataTable[1] = auIPV4AssocDataTable[0] + 2;
    uIPV4AssocDataTableEntryNum = C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntriesPerRow = uIPV4BucketEntriesPerRow * pc3hppcTestInfo->nIPV4BucketPrefixNum;
    uIPV4AssocDataEntrySizeIn64b = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b; 
    pIPV4BucketTable = NULL;

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_BUCKET,
                              C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV4BucketEntriesPerRow, uDeadlineOffset, auIPV4AssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV4BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );

      if ( pIPV4BucketTable == NULL ) {
        pIPV4BucketTable = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * uIPV4BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV4 Bucket Table");
        sal_memset( pIPV4BucketTable, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM * uIPV4BucketEntrySizeIn64b * sizeof(uint64)) );
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

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PREFIX_SIZE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT_BASE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_PREFIX_SIZE, pc3hppcTestInfo->nIPV4BucketPrefixNum,
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV4 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV4 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV4 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_lpmipv4_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH,
                                               pc3hppcTestInfo->nIPV4BucketPrefixNum, auIPV4BucketTable[0],
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_MAX_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                               pRPBcommands, pBBXcommands, pBRRcommands, pIPV4BucketTable, apIPV4AssocDataTable);

    cli_out("\nINFO:  Initializing TAPS IPV4 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV4 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV4 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      nCmdFifoSelect = (int)(auIPV4BucketTable[nTP] & 1);

      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV4BucketTable[nTP], 0,
                           C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, 0, pIPV4BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV4BucketTable[nTP], 0,
                             C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM, 0, pIPV4BucketTable );
      }

      nCmdFifoSelect = (int)(auIPV4AssocDataTable[nTP] & 1);
      cli_out("\nINFO:  Initializing TAPS%d IPV4 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                           uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                             uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                  auIPV4BucketTable[nTP],
                                  C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }

    /* This is for the program that does IPV4 on sub-key0 and EML64 on sub-key1 */
    nTP = 0;
    /* coverity[dead_error_line] */
    c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_EML64_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                auIPV4BucketTable[nTP],
                                C3HPPC_EXERCISER_TEST1__LPMIPV4_SEGMENT,
                                ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 
    sal_free( pIPV4BucketTable ); 
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV4AssocDataTable[nTP] );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 4, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 5, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_EXERCISER_TEST1__LPMIPV4_TMU_PROGRAM );


    /****************************************************************************************************************************
     *
     * LPM IPV4 Mode0 setup
     *
     *****************************************************************************************************************************/
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PREFIX_SIZE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT_BASE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_PREFIX_SIZE, 0, 0, C3HPPC_TMU_TAPS_MODE__FULLY_ON_CHIP );

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                  0,
                                  C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV4 Mode0 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV4 Mode0 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV4 Mode0 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_lpmipv4_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, C3HPPC_TMU_TAPS_MODE__FULLY_ON_CHIP,
                                               0, 0,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_SEGMENT,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_MAX_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM,
                                               0,
                                               pRPBcommands, pBBXcommands, pBRRcommands, NULL, NULL);

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode0 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode0 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode0 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE0_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 


    /****************************************************************************************************************************
     *
     * LPM IPV4 Mode1 setup
     *
     *****************************************************************************************************************************/

    auIPV4AssocDataTable[0] = auIPV4AssocDataTable[1] + 1;
    auIPV4AssocDataTable[1] = auIPV4AssocDataTable[0] + 1;
    uIPV4AssocDataTableEntryNum = C3HPPC_EXERCISER_TEST1__LPMIPV4_BUCKET_TABLE_ENTRY_NUM / 2;  /* Remove "pair" factor in define */
    uIPV4AssocDataEntriesPerRow = 1;   /* 2KB / 16B(128b) */
    uIPV4AssocDataEntrySizeIn64b = C3HPPC_TMU_ASSOC_DATA_SIZE_IN_64b; 

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV4AssocDataTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV4_ASSOC_DATA,
                              uIPV4AssocDataTableEntryNum, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV4AssocDataEntriesPerRow, uDeadlineOffset, auIPV4AssocDataTable[nTP], 0, 0, 0, 0, 0, 0, &uRegionRowOffset );
  
      apIPV4AssocDataTable[nTP] = (uint32 *) sal_alloc( (uIPV4AssocDataTableEntryNum *
                                                         uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)), "LPM IPV4 Mode1 AssocData Table");
      sal_memset( apIPV4AssocDataTable[nTP], 0,
                  (uIPV4AssocDataTableEntryNum * uIPV4AssocDataEntrySizeIn64b * sizeof(uint64)) );

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PREFIX_SIZE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT_BASE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_PREFIX_SIZE, 0,
                                     0, C3HPPC_TMU_TAPS_MODE__SEARCH_ON_CHIP );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV4 Mode1 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV4 Mode1 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV4 Mode1 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_lpmipv4_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, C3HPPC_TMU_TAPS_MODE__SEARCH_ON_CHIP,
                                               0, 0,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_MAX_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM,
                                               0,
                                               pRPBcommands, pBBXcommands, pBRRcommands, NULL, apIPV4AssocDataTable);

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode1 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode1 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV4 Mode1 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      nCmdFifoSelect = (int)(auIPV4AssocDataTable[nTP] & 1);

      cli_out("\nINFO:  Initializing TAPS%d IPV4 Mode1 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                           uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV4AssocDataTable[nTP], 0,
                             uIPV4AssocDataTableEntryNum, 0, apIPV4AssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV4,
                                  auIPV4AssocDataTable[nTP],
                                  C3HPPC_EXERCISER_TEST1__LPMIPV4_MODE1_SEGMENT,
                                  ((nTP == 0) ? 0 : 56), 6, 0, 0, 0 );
    }

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV4AssocDataTable[nTP] );
    }



  } else {
    auIPV4AssocDataTable[1] = g_aEmcTableParameters[nEMC_NextTableIndex].nTableID;
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
    uIPV6BucketEntriesPerRow = C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRIES_PER_ROW;
    auIPV6KeyAndAssocDataTable[0] = auIPV6BucketTable[0] + 1;
    auIPV6KeyAndAssocDataTable[1] = auIPV6KeyAndAssocDataTable[0] + 2;
    uIPV6KeyAndAssocDataTableEntryNum = C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntriesPerRow = uIPV6BucketEntriesPerRow * pc3hppcTestInfo->nIPV6BucketPrefixNum;
    uIPV6KeyAndAssocDataEntrySizeIn64b = C3HPPC_TMU_IPV6_KEY_AND_ASSOC_DATA_TABLE_ENTRY_SIZE_IN_64b;
    pIPV6BucketTable = NULL;

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {

      uDeadlineOffset = 0;
      uColumnOffset = 0;
      c3hppc_tmu_table_setup( pc3hppcTestInfo->nUnit, (int)auIPV6BucketTable[nTP], C3HPPC_TMU_LOOKUP__TAPS_IPV6_BUCKET,
                              C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, pc3hppcTestInfo->uReplicationFactor,
                              0, uRegionRowOffset, uColumnOffset,
                              uIPV6BucketEntriesPerRow, uDeadlineOffset, auIPV6KeyAndAssocDataTable[nTP], 0, 0,
                              pc3hppcTestInfo->nIPV6BucketPrefixNum, 0, 0, 0, &uRegionRowOffset );
  
      if ( pIPV6BucketTable == NULL ) {
        pIPV6BucketTable = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * uIPV6BucketEntrySizeIn64b * sizeof(uint64)),
                                                 "LPM IPV6 Bucket Table");
        sal_memset( pIPV6BucketTable, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM * uIPV6BucketEntrySizeIn64b * sizeof(uint64)) );
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

      c3hppc_tmu_taps_segment_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PREFIX_SIZE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT_BASE,
                                     C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_PREFIX_SIZE, pc3hppcTestInfo->nIPV6BucketPrefixNum,
                                     pc3hppcTestInfo->bTapsUnified, C3HPPC_TMU_TAPS_MODE__3_LEVEL_SEARCH );
    }
  
    pRPBcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)),
                                         "LPM IPV6 RPB Commands");
    sal_memset( pRPBcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * sizeof(c3hppc_tmu_taps_rpb_ccmd_t)) );
  
    pBBXcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_bbx_ccmd_t)),
                                         "LPM IPV6 BBX Commands");
    sal_memset( pBBXcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_bbx_ccmd_t)) );
  
    pBRRcommands = (uint32 *) sal_alloc( (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM *
                                          C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                          sizeof(c3hppc_tmu_taps_brr_ccmd_t)),
                                         "LPM IPV6 BRR Commands");
    sal_memset( pBRRcommands, 0, (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM *
                                  sizeof(c3hppc_tmu_taps_brr_ccmd_t)) );

    c3hppc_test__setup_lpmipv6_table_contents( C3HPPC_TMU_TAPS_INSTANCE_NUM, pc3hppcTestInfo->nSetupOptions,
                                               pc3hppcTestInfo->nIPV6BucketPrefixNum, auIPV6BucketTable[0],
                                               C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM, C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_MAX_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM,
                                               C3HPPC_EXERCISER_TEST1__LPMIPV6_DRAM_BUCKET_POPULATED_PREFIX_NUM,
                                               pRPBcommands, pBBXcommands, pBRRcommands, pIPV6BucketTable, apIPV6KeyAndAssocDataTable );

    cli_out("\nINFO:  Initializing TAPS IPV6 RPB memories ...\n");
    c3hppc_tmu_taps_write( 0, C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM, pRPBcommands ); 

    cli_out("\nINFO:  Initializing TAPS IPV6 BBX memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBBXcommands );

    cli_out("\nINFO:  Initializing TAPS IPV6 BRR memories ...\n");
    c3hppc_tmu_taps_write( 1, (C3HPPC_EXERCISER_TEST1__LPMIPV6_RPB_PIVOT_NUM * C3HPPC_EXERCISER_TEST1__LPMIPV6_BBX_POPULATED_PIVOT_NUM),
                           pBRRcommands );

    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      nCmdFifoSelect = (int)(auIPV6BucketTable[nTP] & 1);

      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Bucket Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV6BucketTable[nTP], 0,
                           C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, 0, pIPV6BucketTable );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV6BucketTable[nTP], 0,
                             C3HPPC_EXERCISER_TEST1__LPMIPV6_BUCKET_TABLE_ENTRY_NUM, 0, pIPV6BucketTable );
      }

      nCmdFifoSelect = (int)(auIPV6KeyAndAssocDataTable[nTP] & 1);
      cli_out("\nINFO:  Initializing TAPS%d IPV6 Dram Associated Data Table ...\n", nTP);
      c3hppc_tmu_xl_write( nCmdFifoSelect, (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                           uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      if ( g_bDoInitXLreads ) {
        c3hppc_tmu_xl_read(  nCmdFifoSelect, (int)auIPV6KeyAndAssocDataTable[nTP], 0,
                             uIPV6KeyAndAssocDataTableEntryNum, 0, apIPV6KeyAndAssocDataTable[nTP] );
      }

      c3hppc_tmu_keyploder_setup( pc3hppcTestInfo->nUnit, nTP, C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM, C3HPPC_TMU_LOOKUP__TAPS_IPV6,
                                  auIPV6BucketTable[nTP],
                                  C3HPPC_EXERCISER_TEST1__LPMIPV6_SEGMENT,
                                  ((nTP == 0) ? 2 : 40), 18, 0, 0, 0 );
    }

    sal_free( pRPBcommands ); 
    sal_free( pBBXcommands ); 
    sal_free( pBRRcommands ); 
    sal_free( pIPV6BucketTable ); 
    for ( nTP = 0; nTP < C3HPPC_TMU_TAPS_INSTANCE_NUM; ++nTP ) {
      sal_free( apIPV6KeyAndAssocDataTable[nTP] );
    }

    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 6, C3HPPC_TMU_PM_INTF__KEY, 1, 0, C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM );
    c3hppc_tmu_pm_filter_setup( pc3hppcTestInfo->nUnit, 7, C3HPPC_TMU_PM_INTF__KEY, 1, 1, C3HPPC_EXERCISER_TEST1__LPMIPV6_TMU_PROGRAM );

  } /* if ( pc3hppcTestInfo->bIPV6Enable ) { */



  /****************************************************************************************************************************
   *
   * ETU/NL11K setup
   *
   *****************************************************************************************************************************/
  if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 1 ) {
    c3hppc_etu_setup_search_program( pc3hppcTestInfo->nUnit, C3HPPC_EXERCISER_TEST1__ETU_SEARCH_PROGRAM,
                                     C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, C3HPPC_ETU_LOOKUP__4x80 );
    c3hppc_etu_tcam_table_layout_setup( pc3hppcTestInfo->nUnit, C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0,
                                        C3HPPC_ETU_LOOKUP__4x80, C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS );

    uEtuKeySizeIn80bSegments = c3hppc_etu_get_tcam_table_key_size( C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0 );
    pEtuKeyData = (c3hppc_etu_80b_data_t *) sal_alloc( (C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                       "ETU Key Data");
    pEtuKeyMask = (c3hppc_etu_80b_data_t *) sal_alloc( (C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                       "ETU Key Mask");
    sal_memset( pEtuKeyData, 0x00, (C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );
    sal_memset( pEtuKeyMask, 0x00, (C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS * uEtuKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );

    COMPILER_64_SET(uuTmp, 0xffffffff, 0xffffffff);
    COMPILER_64_SET(uuEtuKeyMask, 0, (C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS - 1));
    COMPILER_64_XOR(uuEtuKeyMask, uuTmp);
    for ( nKey = 0; nKey < C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS; ++nKey ) {
      for ( uSegment = 0; uSegment < uEtuKeySizeIn80bSegments; ++uSegment ) {
        nIndex = (nKey * uEtuKeySizeIn80bSegments) + uSegment;
        if ( uSegment == 0 ) {
          COMPILER_64_SET(pEtuKeyData[nIndex].Words[0],0, nKey);
          COMPILER_64_ZERO(pEtuKeyData[nIndex].Words[1]);
          pEtuKeyMask[nIndex].Words[0] = uuEtuKeyMask;
          COMPILER_64_SET(pEtuKeyMask[nIndex].Words[1],0x00000000,0x0000ffff);
        } else if ( (uSegment+1) == uEtuKeySizeIn80bSegments ) {
          COMPILER_64_SET(pEtuKeyData[nIndex].Words[1],0x00000000,0x00008000);
        }
      }
    }
    cli_out("\nINFO:  Initializing NL11K TCAM Table ...\n");
    c3hppc_etu_key_insert( C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x00000,
                           C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x20000,
                           C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x40000,
                           C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    c3hppc_etu_key_insert( C3HPPC_EXERCISER_TEST1__ETU_NL11K_LOGICAL_TABLE0, 0x60000,
                           C3HPPC_EXERCISER_TEST1__ETU_MAX_KEYS, pEtuKeyData, pEtuKeyMask, 0 );
    sal_free( pEtuKeyData );
    sal_free( pEtuKeyMask );
  }


  /****************************************************************************************************************************
   * Configure the LRP program translation tables.
   *****************************************************************************************************************************/
  for ( nProgram = 0; nProgram < COUNTOF(g_aLrpSearchProgramParameters); ++nProgram ) {

    if ( pc3hppcTestInfo->BringUpControl.uRceBringUp == 1 && 
         g_aLrpSearchProgramParameters[nProgram].uRceProgram != C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_rce_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uRceProgram );
    }
    if ( g_aLrpSearchProgramParameters[nProgram].uTmuProgram != C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_tmu_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uTmuProgram,
                                    g_aLrpSearchProgramParameters[nProgram].bSubKey0Valid,
                                    g_aLrpSearchProgramParameters[nProgram].bSubKey1Valid);
    }
    if ( pc3hppcTestInfo->BringUpControl.uEtuBringUp == 1 &&
         g_aLrpSearchProgramParameters[nProgram].uEtuProgram != C3HPPC_EXERCISER_TEST1__PROGRAM_INVALID ) {
      c3hppc_lrp_setup_etu_program( pc3hppcTestInfo->nUnit, g_aLrpSearchProgramParameters[nProgram].nLrpKeyProgram,
                                    g_aLrpSearchProgramParameters[nProgram].uEtuProgram );
    }

  }




  /****************************************************************************************************************************
   * Wait for Updater operations to complete before advancing to enable traffic.
   *****************************************************************************************************************************/
  nTimeOut = C3HPPC_EXERCISER_TEST1__UPDATER_TIMEOUT;
  if ( SAL_BOOT_QUICKTURN ) nTimeOut *= 100; 
  if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit, nTimeOut ) == 0 ) {

    cli_out("ERROR:  Initial setup Updater TIMEOUT failure!\n");
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else {


    /****************************************************************************************************************************
     * If doing HW allocation then dump/display EML tables and verify their integrity.
     *****************************************************************************************************************************/
    if ( g_bDumpEmlTables ) {
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
    c3hppc_tmu_pm_activate( pc3hppcTestInfo->nUnit, 0, g_uPmBucketShift, 0 );



    /****************************************************************************************************************************
     * Enable lookups ...
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable ) {
        /* coverity[stack_use_overflow] */
      c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 1, C3HPPC_EXERCISER_TEST1__STREAM_NUM, 0 );

/* Run streams 0 through 2
      c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 1, 3, 0 );
*/
/* Run just stream 2
      c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 0, 2, 0 );
*/
    }



    /****************************************************************************************************************************
     * Enable watch dog timer refresh ....
     *****************************************************************************************************************************/
    c3hppc_cop_set_watchdogtimer_ring_manager_timer_num( pc3hppcTestInfo->nCounterNum );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_svp( 2 );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_lm( 0 );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_size( C3HPPC_COP_INSTANCE_NUM * pc3hppcTestInfo->nCounterNum );
    c3hppc_cop_set_watchdogtimer_ring_manager_lrp_list_base( 0x00000001 );
    c3hppc_lrp_setup_host_producer_ring( pc3hppcTestInfo->nUnit, 2, 0, (C3HPPC_COP_INSTANCE_NUM * pc3hppcTestInfo->nCounterNum), 0 );
    for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
      c3hppc_cop_program_segment_enable( pc3hppcTestInfo->nUnit, nCopInstance, g_nWatchDogTimerCopSegment );
    }


    READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
    uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
    COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, 
                    COMPILER_64_HI(pc3hppcTestInfo->uuIterations),
                    COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
    COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, uContexts);
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
    COMPILER_64_ADD_64(uuTmp,pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition); 
    c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp);



    /****************************************************************************************************************************
     * EML chain destruction/construction inducing de-allocation and re-allocation of table entries.
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 

      uInsertEntrySizeIn32b = 2 * C3HPPC_TMU_EML64_INSERT_COMMAND_SIZE_IN_64b;
      uKeySizeIn32b = 2 * C3HPPC_TMU_EML64_KEY_SIZE_IN_64b;
      for ( nCmdFifoSelect = 0; nCmdFifoSelect < C3HPPC_TMU_UPDATE_CMD_FIFO_NUM; ++nCmdFifoSelect ) { 
        apKeyDeleteList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)), "Key Delete List");
        apKeyReInsertList[nCmdFifoSelect] = (uint32 *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uInsertEntrySizeIn32b * sizeof(uint32)),
                                                  "ReInsert List");
        sal_memset( apKeyDeleteList[nCmdFifoSelect], 0x00, (pc3hppcTestInfo->nMaxKey * uKeySizeIn32b * sizeof(uint32)) ); 
      }
      sal_memset( g_acKeyLookupMissScoreBoard, 0x00, sizeof(g_acKeyLookupMissScoreBoard) );

      nCmdFifoSelect = 0;


      nEpochSize = 0;
      /* coverity[secure_coding] */
      sal_strcpy( sUcodeFileName, pc3hppcTestInfo->BringUpControl.sUcodeFileName );
      nTargetBankSelect = 1;
      COMPILER_64_SET(uuActionThreshold, 
                      COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount),
                      COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
      COMPILER_64_UMUL_32(uuActionThreshold, 3);
      COMPILER_64_SHR(uuActionThreshold, 2);
      uuTmp = c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit );
      while ( COMPILER_64_LT(uuTmp, uuActionThreshold) &&
              c3hppc_test__get_error_state_summary( pc3hppcTestInfo->nUnit ) == 0  ) { 


#if 0
          cli_out("\n\n uuActionThreshold %lld   get_current_epoch_count %lld \n", uuActionThreshold, c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit ) );
#endif

        /*
           Bulk delete operations can only run on the "nCmdFifoSelect == 0" slot because the randomly 
           generated filter can not be easily modified to remove possbile collisions from the resultant delete list.
           The same key must NOT be present in both delete lists as this will cause results that are unpredictable.
        */ 
        if ( nCmdFifoSelect == 0 ) {
          nBulkDelete = sal_rand() %  (C3HPPC_EXERCISER_TEST1__PHYSICAL + 1);
        }

        uKeyFilterMask = sal_rand() % pc3hppcTestInfo->nMaxKey;
        uKeyFilter = uKeyFilterMask;

        for ( nKey = 0, auKeyDeleteListLength[nCmdFifoSelect] = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
          if ( (pEML_InsertList[uInsertEntrySizeIn32b*nKey] & uKeyFilterMask) == uKeyFilter ) {
            if ( nCmdFifoSelect == 1 ) {
              for ( nKey1 = 0; nKey1 < auKeyDeleteListLength[0]; ++nKey1 ) {
                if ( apKeyDeleteList[0][uKeySizeIn32b*nKey1] == nKey ) break;
              }
            }
            if ( nCmdFifoSelect == 0 || nKey1 == auKeyDeleteListLength[0] ) {
              apKeyDeleteList[nCmdFifoSelect][uKeySizeIn32b*auKeyDeleteListLength[nCmdFifoSelect]] = nKey; 
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
          c3hppc_tmu_bulk_delete_setup( pc3hppcTestInfo->nUnit, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                        &uKeyFilter, &uKeyFilterMask );
          if ( nBulkDelete == C3HPPC_EXERCISER_TEST1__PHYSICAL ) {
            c3hppc_tmu_bulk_delete_start_scanner( pc3hppcTestInfo->nUnit );
            if ( c3hppc_tmu_wait_for_bulk_delete_done( pc3hppcTestInfo->nUnit, 5) ) {
              cli_out("\nERROR:  Bulk delete failed ...\n");
              pc3hppcTestInfo->nTestStatus = TEST_FAIL;
              break;
            } else {
              cli_out("\nINFO:  The bulk delete operation finished ...\n");
            }
          } else {
            sUcodeFileName[15] = ( nTargetBankSelect ) ? 'b' : 'a';
            if ( !c3hppc_lrp_load_ucode(pc3hppcTestInfo->nUnit, sUcodeFileName, nTargetBankSelect, 
                                        pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable,
                                        pc3hppcTestInfo->BringUpControl.bLrpMaximizeActiveContexts, &nEpochSize) ) {
              c3hppc_lrp_bank_swap( pc3hppcTestInfo->nUnit );
              nTargetBankSelect ^= 1;
              c3hppc_lrp_bank_corrupt( pc3hppcTestInfo->nUnit, nTargetBankSelect );
            }
          }
          c3hppc_tmu_bulk_delete_cancel( pc3hppcTestInfo->nUnit );

        } else {


          if ( auKeyDeleteListLength[nCmdFifoSelect] ) {
            cli_out("\nINFO:  Deleting %d EML table entries issued from CMD FIFO%d...\n",
                    auKeyDeleteListLength[nCmdFifoSelect], nCmdFifoSelect);
            c3hppc_tmu_eml_delete( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                   auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect],
                                   C3HPPC_TMU_UPDATE_DELETE_OPTIONS__NONE );

            sUcodeFileName[15] = ( nTargetBankSelect ) ? 'b' : 'a';
            if ( !c3hppc_lrp_load_ucode(pc3hppcTestInfo->nUnit, sUcodeFileName, nTargetBankSelect,
                                        pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable,
                                        pc3hppcTestInfo->BringUpControl.bLrpMaximizeActiveContexts, &nEpochSize) ) {
              c3hppc_lrp_bank_swap( pc3hppcTestInfo->nUnit );
              nTargetBankSelect ^= 1;
              c3hppc_lrp_bank_corrupt( pc3hppcTestInfo->nUnit, nTargetBankSelect );
            }
          }
        }

        if ( auKeyDeleteListLength[nCmdFifoSelect] ) {

          if ( nBulkDelete != C3HPPC_EXERCISER_TEST1__LOGICAL ) {
            cli_out("\nINFO:  Verifying delete of EML table entries ...\n");
            c3hppc_tmu_eml_verify_delete( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                          auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );

            cli_out("\nINFO:  Adding back the %d deleted EML table entries ...\n", auKeyDeleteListLength[nCmdFifoSelect] );
            c3hppc_tmu_eml_insert( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID, 
                                   auKeyDeleteListLength[nCmdFifoSelect], apKeyReInsertList[nCmdFifoSelect],
                                   C3HPPC_TMU_UPDATE_INSERT_OPTIONS__NONE );
          }

          cli_out("\nINFO:  Verifying EML tables are fully populated  ...\n");
          c3hppc_tmu_eml_verify_insert( nCmdFifoSelect, g_aEmlTableParameters[nEML_RootTableIndex].nTableID,
                                        auKeyDeleteListLength[nCmdFifoSelect], apKeyDeleteList[nCmdFifoSelect] );
        }


        if ( nCmdFifoSelect || nBulkDelete ) {
          if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3HPPC_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
            break;
          }
          nCmdFifoSelect = 0;
        } else {
          nCmdFifoSelect = 1;
        }

        uuTmp = c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit );
      }

      if ( c3hppc_test__wait_for_updaters_to_be_idle( pc3hppcTestInfo->nUnit,C3HPPC_EXERCISER_TEST1__UPDATER_TIMEOUT ) == 0 ) {
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      } else if ( g_bDumpEmlTables ) {
        if ( c3hppc_tmu_get_eml_tables(pc3hppcTestInfo->nUnit, nCmdFifoSelect,
                                       g_aEmlTableParameters[nEML_RootTableIndex].nTableID, g_aEmlTableParameters[nEML_ChainTableIndex].nTableID,
                                       g_aEmlTableParameters[nEML_RootTableIndex].uNumEntries, C3HPPC_EXERCISER_TEST1__UPDATER_TIMEOUT) ) {
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

      cli_out("\nWaiting for the LRP \"offline\" event ....\n");

    }

/*
    uuBubbleCntr = 0;
    uBubbleCntrState = 0;

    cli_out("\n00000000 Bubbles STARTED at time %d!\n", (int) sal_time() );
    while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
      READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uReg );
      if ( uBubbleCntrState == 0 && uReg == 0 ) {
        cli_out("\n11111111 Bubbles STOPPED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 1;
      } else if ( uBubbleCntrState == 1 && uReg != 0 ) {
        cli_out("\n22222222 Bubbles RE-STARTED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 2;
      } else if ( uBubbleCntrState == 2 && uReg == 0 ) {
        cli_out("\n33333333 Bubbles STOPPED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 3;
      }
      uuBubbleCntr += (uint64) uReg;
      sal_usleep(1000);
    }
    READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uReg );
    uuBubbleCntr += (uint64) uReg;
    cli_out("\n\nINFO:  LRA_BUBBLE_CNT accumulated value --> %lld \n\n", uuBubbleCntr );
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


  sal_free( pEML_InsertList ); 

  return 0;
}




int
c3hppc_exerciser_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg, uBaseOffset;
  int nErrorCnt, nTotalErrorCnt, nStartIndex, nIndex, nOcmPort;
  uint64 *pTotalLookupCount, uuTotalEmlLookups_cmu;
  uint64 *pMatchLookupCount;
  uint64 *pMissLookupCount;
  uint64 *pStartCountBankA, *pEndCountBankA, *pStartCountBankB, *pEndCountBankB;
  uint64 uuTotalStartCount, uuTotalEndCount, uuDiff, uuTotalSearches, uuTmp;
  int nKey, nCounter, nFlow, nLrpContext;
  c3hppc_cmu_segment_info_t *pCmuSegment0Info, *pCmuSegment1Info;
  c3hppc_cmu_segment_info_t *pCmuSegment2Info, *pCmuSegment3Info;
  c3hppc_cmu_segment_info_t *pCmuSegment4Info, *pCmuSegment5Info;
  char sMessage[16];
  uint8 bLrpContextIs3or4or5;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock0, *pOcmBlock1;
  int nDmaBlockSize;
  int nCopyWordCount;

  cli_out("\n\nINFO:  COP counted EPOCH value --> %lld \n\n", c3hppc_test__get_current_epoch_count( pc3hppcTestInfo->nUnit ) );

  /****************************************************************************************************************************
   * Dump/display performance monitor statistics.  Set "nStartIndex" to '0' if histogram data is desired.
   *****************************************************************************************************************************/
  nStartIndex = ( pc3hppcTestInfo->nNumberOfCIs == C3HPPC_TMU_CI_INSTANCE_NUM ) ? 425 : 500;

  COMPILER_64_ZERO(uuTotalSearches);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 0, nStartIndex, g_uPmBucketShift, "EML-SK0 EVEN" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 1, nStartIndex, g_uPmBucketShift, "EML-SK0 ODD" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  cli_out("\nINFO:  SubKey0 EML search count --> %lld \n\n", uuTotalSearches );

  COMPILER_64_ZERO(uuTotalSearches);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 2, nStartIndex, g_uPmBucketShift, "EMC-SK0 EVEN" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 3, nStartIndex, g_uPmBucketShift, "EMC-SK0 ODD" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  cli_out("\nINFO:  SubKey0 EMC search count --> %lld \n\n", uuTotalSearches );

  COMPILER_64_ZERO(uuTotalSearches);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 4, nStartIndex, g_uPmBucketShift, "IPV4-SK0 EVEN" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 5, nStartIndex, g_uPmBucketShift, "IPV4-SK0 ODD" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  cli_out("\nINFO:  SubKey0 IPV4 search count --> %lld \n\n", uuTotalSearches );

  COMPILER_64_ZERO(uuTotalSearches);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 6, nStartIndex, g_uPmBucketShift, "IPV6-SK0 EVEN" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  uuTmp = c3hppc_test__display_pm_stats( pc3hppcTestInfo->nUnit, 7, nStartIndex, g_uPmBucketShift, "IPV6-SK0 ODD" );
  COMPILER_64_ADD_64(uuTotalSearches,uuTmp);
  cli_out("\nINFO:  SubKey0 IPV6 search count --> %lld \n\n", uuTotalSearches );



  nErrorCnt = 0;
  COMPILER_64_ZERO(uuTotalEmlLookups_cmu);
  pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
  pCmuSegment1Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
  for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
    pTotalLookupCount = pCmuSegment0Info->pSegmentPciBase + ((2 * nKey) + 1);
    COMPILER_64_ADD_64(uuTotalEmlLookups_cmu, *pTotalLookupCount);
    pMatchLookupCount = pCmuSegment1Info->pSegmentPciBase + ((2 * nKey) + 1);
    pMissLookupCount =  pMatchLookupCount + (2 * C3HPPC_EXERCISER_TEST1__MAX_KEYS);
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pMatchLookupCount), COMPILER_64_LO(*pMatchLookupCount));
    COMPILER_64_ADD_64(uuTmp, *pMissLookupCount);
    if ( COMPILER_64_NE(*pTotalLookupCount, uuTmp) ) {
      if ( nErrorCnt < 128 ) {
        cli_out("<c3hppc_exerciser_test1__done> -- Lookup count MISCOMPARE for Key[0x%04x]  Actual: %lld   Expect: %lld -- match[%lld] miss[%lld] \n",
                nKey, uuTmp, *pTotalLookupCount, *pMatchLookupCount, *pMissLookupCount );
      }
      ++nErrorCnt;
    }

    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 
      if ( !COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 0 ) {
        cli_out("\nERROR: -- UNEXPECTED non-zero MISS Lookup count for Key[0x%04x]  Actual: 0x%llx \n",
                nKey, *pMissLookupCount );
        ++nErrorCnt;
      }
      if ( COMPILER_64_IS_ZERO(*pMissLookupCount) && g_acKeyLookupMissScoreBoard[nKey] == 1 ) {
        cli_out("<c3hppc_exerciser_test1__done> -- UNEXPECTED zero MISS Lookup count for Key[0x%04x]\n", nKey );
        ++nErrorCnt;
      }
    }

  }

  cli_out("\n\n<c3hppc_exerciser_test1__done> -- Total EML look-ups counted by the CMU --> %lld\n\n", uuTotalEmlLookups_cmu );

  nTotalErrorCnt = nErrorCnt;
  nErrorCnt = 0;
  pCmuSegment2Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 2;
  pCmuSegment3Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 3;
  pCmuSegment4Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 4;
  pCmuSegment5Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 5;
  for ( nFlow = 0; nFlow < C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE; ++nFlow ) {
    nLrpContext = (nFlow >> 7);
    pStartCountBankA = pCmuSegment2Info->pSegmentPciBase + ((2 * nFlow) + 1);
    pEndCountBankA = pCmuSegment3Info->pSegmentPciBase + ((2 * nFlow) + 1);
    pStartCountBankB = pCmuSegment4Info->pSegmentPciBase + ((2 * nFlow) + 1);
    pEndCountBankB = pCmuSegment5Info->pSegmentPciBase + ((2 * nFlow) + 1);
    COMPILER_64_SET(uuTotalStartCount, COMPILER_64_HI(*pStartCountBankA), COMPILER_64_LO(*pStartCountBankA));
    COMPILER_64_ADD_64(uuTotalStartCount, *pStartCountBankB);
    COMPILER_64_SET(uuTotalEndCount, COMPILER_64_HI(*pEndCountBankA), COMPILER_64_LO(*pEndCountBankA));
    COMPILER_64_ADD_64(uuTotalEndCount, *pEndCountBankB);
    if ( nLrpContext >= 3 && nLrpContext <= 5 ) {
      bLrpContextIs3or4or5 = 1;
    } else {
      COMPILER_64_ADD_32(uuTotalEndCount,1);
      bLrpContextIs3or4or5 = 0;
    }

    if ( COMPILER_64_NE(uuTotalStartCount, pc3hppcTestInfo->uuIterations) ) {
      if ( nErrorCnt < 128 ) {
        cli_out("<c3hppc_exerciser_test1__done>  -->  Start count MISCOMPARE for Flow[0x%04x]  Actual: 0x%llx   Expect: 0x%llx \n",
                nFlow, uuTotalStartCount, pc3hppcTestInfo->uuIterations );
      }
      ++nErrorCnt;
    }
    if ( COMPILER_64_NE(uuTotalEndCount, pc3hppcTestInfo->uuIterations) ) {
      if ( nErrorCnt < 128 ) {
        cli_out("<c3hppc_exerciser_test1__done>  -->  End count MISCOMPARE for Flow[0x%04x]  Actual: 0x%llx   Expect: 0x%llx \n",
                nFlow, uuTotalEndCount, pc3hppcTestInfo->uuIterations );
      }
      ++nErrorCnt;
    }

    COMPILER_64_SET(uuDiff, COMPILER_64_HI(*pStartCountBankA), COMPILER_64_LO(*pStartCountBankA));
    COMPILER_64_SUB_64(uuDiff, *pEndCountBankA);
    COMPILER_64_SET(uuTmp, 0, 1);
    if ( (!COMPILER_64_IS_ZERO(uuDiff) && bLrpContextIs3or4or5) || COMPILER_64_GT(uuDiff, uuTmp) ) {
      if ( nErrorCnt < 128 ) {
        cli_out("<c3hppc_exerciser_test1__done>  -->  StartA-EndA count MISCOMPARE for Flow[0x%04x]  Start: 0x%llx   End: 0x%llx \n",
                nFlow, *pStartCountBankA, *pEndCountBankA );
      }
      ++nErrorCnt;
    }

    COMPILER_64_SET(uuDiff, COMPILER_64_HI(*pStartCountBankB), COMPILER_64_LO(*pStartCountBankB));
    COMPILER_64_SUB_64(uuDiff, *pEndCountBankB);

    if ( (!COMPILER_64_IS_ZERO(uuDiff) && bLrpContextIs3or4or5) || COMPILER_64_GT(uuDiff, uuTmp) ) {
      if ( nErrorCnt < 128 ) {
        cli_out("<c3hppc_exerciser_test1__done>  -->  StartA-EndA count MISCOMPARE for Flow[0x%04x]  Start: 0x%llx   End: 0x%llx \n",
                nFlow, *pStartCountBankB, *pEndCountBankB );
      }
      ++nErrorCnt;
    }
  }
  nTotalErrorCnt += nErrorCnt;



  if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_EXERCISER_TEST1__TABLE_STATE_CHANGE_W_HW_CHAINING ) { 
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
    cli_out("\n<c3hppc_exerciser_test1__done> -- %d keys should have been left UN-TOUCHED ...\n", nCounter );
  }


  /****************************************************************************************************************************
   * Verify data block whereby an affective "copy" operation is done using a load-store sequence.
   ***************************************************************************************************************************/
  nDmaBlockSize = C3HPPC_EXERCISER_TEST1__LRP_PORT_TABLE_ENTRY_NUM;
  pOcmBlock0 = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                 nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                 "ocm_block");
  pOcmBlock1 = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                 nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                 "ocm_block");
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(C3HPPC_EXERCISER_TEST1_WDT_PORT);
  uBaseOffset = C3HPPC_EXERCISER_TEST1_WDT_PORT_LRP_LOAD_ACCESS_BASE_OFFSET; 
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                             uBaseOffset, (uBaseOffset+nDmaBlockSize-1), 0, pOcmBlock0->uData );
  uBaseOffset = C3HPPC_EXERCISER_TEST1_WDT_PORT_LRP_STORE_ACCESS_BASE_OFFSET; 
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                             uBaseOffset, (uBaseOffset+nDmaBlockSize-1), 0, pOcmBlock1->uData );

  nTotalErrorCnt = nErrorCnt;
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
  if ( nCopyWordCount == 0 ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_exerciser_test1__done> -- NO words copied by load-store sequence on STREAM 1!\n" );
  }
  nTotalErrorCnt += nErrorCnt;
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock0);
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock1);



  if ( nTotalErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_exerciser_test1__done> -- Total error count --> %d\n", nTotalErrorCnt );
  }


  uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 48 );
/*
 cli_out("\n<c3hppc_exerciser_test1__done> -- uReg  --> 0x%08x\n", uReg );
*/
  if ( uReg ) {
    if (      uReg & 0x80000000 ) strcpy( sMessage, "TSR ERROR" );
    else if ( uReg & 0x40000000 ) strcpy( sMessage, "RCE MISCOMPARE" );
    else if ( uReg & 0x20000000 ) strcpy( sMessage, "EML MISCOMPARE" );
    else if ( uReg & 0x10000000 ) strcpy( sMessage, "EMC MISCOMPARE" );
    else if ( uReg & 0x08000000 ) strcpy( sMessage, "DM MISCOMPARE" );
    else if ( uReg & 0x04000000 ) strcpy( sMessage, "PORT MISCOMPARE" );
    else if ( uReg & 0x02000000 ) strcpy( sMessage, "ETU MISCOMPARE" );
    else if ( uReg & 0x01000000 ) strcpy( sMessage, "IPV4 MISCOMPARE" );
    else if ( uReg & 0x00800000 ) strcpy( sMessage, "IPV6 MISCOMPARE" );

    if (      uReg & 0x80000000 ) {
      cli_out("\nERROR:  %s indication received on FlowID[0x%03x] Stream[%d] Segment[%d] SK1ErrCode[0x%x] SK0ErrCode[0x%x]\n",
              sMessage, (uReg & 0x3ff), ((uReg >> 16) & 0xf), ((uReg >> 12) & 0xf), ((uReg >> 24) & 0xf), ((uReg >> 20) & 0xf) );
    } else {
      cli_out("\nERROR:  %s indication received on FlowID[0x%03x] Stream[%d] Segment[%d]\n",
              sMessage, (uReg & 0x3ff), ((uReg >> 16) & 0xf), ((uReg >> 12) & 0xf) );
    }
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
#endif
}




#endif /* #ifdef BCM_CALADAN3_SUPPORT */
