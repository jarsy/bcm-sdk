/*
 * $Id: c3hppc_test.h,v 1.60 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        c3hppc_test.h
 * Purpose:     Extern declarations for test functions.
 */


#if !defined(_TEST_C3HPPC_TEST_H)
#define _TEST_C3HPPC_TEST_H

#include <sal/core/time.h>
#include <sal/core/alloc.h>
#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/io.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <sal/appl/sal.h>
#include <sal/core/time.h>
#include <appl/diag/test.h> 
#include <sal/core/thread.h>

#include <appl/test/caladan3/c3hppc_lrp.h>
#include <appl/test/caladan3/c3hppc_ocm.h>
#include <appl/test/caladan3/c3hppc_cop.h>
#include <appl/test/caladan3/c3hppc_cmu.h>
#include <appl/test/caladan3/c3hppc_rce.h>
#include <appl/test/caladan3/c3hppc_tmu.h>
#include <appl/test/caladan3/c3hppc_etu.h>
#include <appl/test/caladan3/c3hppc_sws.h>
#include <appl/test/caladan3/c3hppc_utils.h>


#define TEST_FAIL -1
#define TEST_OK    0


#define C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE   1024
#define C3HPPC_TEST_150MPPS_PACKET_SIZE       48



/*
 * HPPC bring-up control structure
 */
typedef struct c3hppc_bringup_control_s {
  uint8     uLrpBringUp;
  uint8     uOcmBringUp;
  uint8     uCopBringUp[C3HPPC_COP_INSTANCE_NUM];
  uint8     uRceBringUp;
  uint8     uTmuBringUp;
  uint8     uEtuBringUp;
  uint8     uCmuBringUp;
  uint8     uSwsBringUp;
  uint8     auIlBringUp[C3HPPC_SWS_IL_INSTANCE_NUM];

  uint8     u100Gduplex;

  uint8     bLrpLoaderEnable;
  uint8     bLrpLearningEnable;
  uint8     bOamEnable;
  uint8     bLrpMaximizeActiveContexts;

  uint32    uDefaultPhysicalSegmentSizeIn16kBlocks;
  uint32    uDefaultCmuPhysicalSegmentSizeIn16kBlocks;

  uint8     bTmuCacheEnable;
  uint8     bTmuBypassScrambler;
  uint8     bTmuBypassHash;
  uint8     bTmuHwEmlChainManagement;
  uint8     bTmuSkipCiDramInit;
  uint8     bTmuSkipCiDramSelfTest;
  int       nTmuDramFreq;
  uint8     bTmuEMC128Mode;
  uint32    uTmuEmlMaxProvisionedKey;
  uint32    uTmuNumberOfCIs;

  uint8     bCopWDT_ScoreBoardMode;

  uint8     bSwsOnlyTest;
  uint8     bSwsBypassPpParsing;
  uint8     bMultiStreamUcode;
  uint8     bSwsBypassPrParsing;
  uint8     bXLportAndCmicPortsActive;

  c3hppc_ocm_port_info_t     aOcmPortInfo[C3HPPC_NUM_OF_OCM_PORTS];
  c3hppc_cop_segment_info_t  aCopSegmentInfo[C3HPPC_COP_INSTANCE_NUM][C3HPPC_COP_SEGMENT_NUM];
  c3hppc_cop_profile_info_t  aCopProfileInfo[C3HPPC_COP_INSTANCE_NUM][C3HPPC_COP_PROFILE_NUM];
  c3hppc_cmu_segment_info_t  aCmuSegmentInfo[C3HPPC_CMU_SEGMENT_NUM];
  uint32    uSTACEseed;

  char      sUcodeFileName[64];
} c3hppc_bringup_control_t;


/*
 * Test information structure -- contains test status/control and 
 * parsed command line arguments.
 */
typedef struct c3hppc_test_info_s {
  int       nUnit;          /* Which unit to test on */

  /* Add test parameters here */
  char     *sTestName;              /* Test Name */
  int      nTestSeed;               /* Test Seed */
  int      nTransferSize;           /* OCM transfer size */
  uint64   uuEpochCount;            /* Raw number of epochs to run */
  uint64   uuIterations;            /* Number of epochs to run is derived in the given test */
  int      bBankSwap;               /* Bank swap feature enable */
  int      nSwitchCount;            /* Additional switch instructions enable */
  int      nProgramSelect;          /* RCE program select */
  int      nHostActivityControl;    /* Concurrent host activity control */
  int      nActiveDmNum;            /* Active DM interface number */
  int      nDmType;                 /* DM interface type (119,247,366,494) */
  int      bEML64Enable;            /* EML-64 lookup enable */
  int      bEML176Enable;           /* EML-176 lookup enable */
  int      bEML304Enable;           /* EML-304 lookup enable */
  int      bEML424Enable;           /* EML-424 lookup enable */
  int      bEMC64Enable;            /* EMC-64 lookup enable */
  int      bIPV4Enable;             /* LPM IPV4 lookup enable */
  int      bIPV6Enable;             /* LPM IPV6 lookup enable */
  int      nIPV4BucketPrefixNum;    /* Number of prefixes in IPV4 bucket table entry */
  int      nIPV6BucketPrefixNum;    /* Number of prefixes in IPV6 bucket table entry */
  int      bTapsUnified;            /* TAPS instances running in unified mode */
  int      bBulkDeleteEnable;       /* Bulk delte enable */
  int      bEMLInsertEnable;        /* HW EML chain management enable */
  int      bLrpLearningEnable;      /* HW LRP learning enable */
  int      bOamEnable;              /* OAM frame generation enable */
  int      bIPV6CollisionEnable;    /* Setup for IPV6 collisions */
  int      bEML144Enable;           /* Setup for dual EML lookup combinations less than 144b */
  int      bCacheEnable;            /* TMU cache enable */
  int      bBypassScrambler;        /* Bypass TMU scrambler control */
  int      bBypassHash;             /* Bypass TMU hash control */
  int      bSkipCiDramInit;         /* Skip CI init of DRAM */
  int      bSkipCiDramSelfTest;     /* Skip CI init of DRAM */
  int      bXLportEnable;           /* Enable XL port activity */
  int      bPauseDisable;           /* Disable MAC pause frames */
  int      bLineExternalLoopBack;   /* Line side exteranl loopback plugs are present */
  int      bFabricExternalLoopBack; /* Fabric side exteranl loopback plugs are present */
  int      bInternalLoopBack;       /* Force internal loopbacks for tr172/173 */
  int      bLineTrafficGenOnly;     /* Line side traffic gen with fabric side in loopback */
  int      bFabricTrafficGenOnly;   /* Fabric side traffic gen with line side in loopback */
  int      bPerformanceTest;        /* Performance type test */
  int      nMaxKey;                 /* Max key value to exercise QE cache logic */
  int      nPeriodic;               /* Periodic select for lookups */
  int      nSetupOptions;           /* General user parameter */
  int      nCounterNum;             /* Active counter number */
  int      nErrorInject;            /* Error feature(s) test enable  */
  int      nNumberOfCIs;            /* Number of CIs  */
  uint32   uReplicationFactor;      /* Global Replication Factor  */
  int      nDramFreq;               /* DRAM frequency  */
  int      nLrpFreq;                /* LRP frequency  */
  int      nSwsFreq;                /* LRP frequency  */
  int      nTmuFreq;                /* TMU frequency  */
  int      nNL11KFreq;              /* NL11K core frequency  */
  int      nShowCurrent;            /* Display bb current output */
  int      nShowTemp;               /* Display pvt monitor temperature info */
  int      bNoTmu;                  /* Run without the TMU */
  int      bNoEtu;                  /* Run without the ETU */
  int      bNoRce;                  /* Run without the RCE */
  int      bNoPpe;                  /* Run without the PPE */
  int      bShmooUnderLoad;         /* Shmoo while running test */
  int      nPacketSize;             /* Packet size */
  int      nLineSidePortNum;        /* Active line-side ports */
  int      nIPG;                    /* Inter-Packet-Gap */
  int      nModulateTimer;          /* Modulation timer to control low frequency transients */
  int      nHotSwap;                /* Hot swap of line TDMs */

  /* Add test management/control here */
  c3hppc_bringup_control_t BringUpControl;
  int                      nTestStatus;
  int                      nTDM;
  uint64                   uuExtraEpochsDueToSwitchStartupCondition;
} c3hppc_test_info_t;

typedef int     (*c3hppc_test_func_t)(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

typedef struct c3hppc_test_list_entry_s {
  char                *sTestName;                 /* Test Name */
  c3hppc_test_func_t   pfuncTestInit;             /* Initialization routine */
  c3hppc_test_func_t   pfuncTestRun;              /* Run routine */
  c3hppc_test_func_t   pfuncTestDone;             /* Completion routine */
} c3hppc_test_list_entry_t;


/*
 * Counter ring manager control structure
 */
typedef struct c3hppc_counter_ring_manager_cb_s {
  int      nUnit;
  uint8    bExit;
  uint32   uPciCounterBase;
  uint64   *pPciCounterLoLimit;
  uint64   *pPciCounterHiLimit;
  c3hppc_bringup_control_t *pBringUpControl;
} c3hppc_counter_ring_manager_cb_t;



typedef struct c3hppc_test_dm_table_parameters_s {
  int                  nTableID;
  int                  nLrpDmInterface;
  uint32               uNumEntries;
  uint32               uDmLookUp;
  uint32               uLrpDmLookUp;
  uint32               uColumnOffset;
  uint32               uDmEntrySizeIn64b;
  uint32               uDmEntriesPerRow;
  uint32               uDeadlineOffset;
} c3hppc_test_dm_table_parameters_t;


typedef struct c3hppc_test_em_table_parameters_s {
  char                 cType;
  int                  nTableID;
  uint32               uNumEntries;
  uint32               uLookUp;
  uint32               uColumnOffset;
  uint32               uEntrySizeIn64b;
  uint32               uEntriesPerRow;
  uint32               uDeadlineOffset;
  uint32               *pContents;
} c3hppc_test_em_table_parameters_t;


typedef struct c3hppc_test_rce_program_parameters_s {
  int                  nNumber;
  uint32               uBaseAddress;
  uint32               uFilterSetNumber;
  uint32               uFilterSetLength;
  uint32               uKeyLength;
  uint32               uKeyStartIndex;
} c3hppc_test_rce_program_parameters_t;


typedef struct c3hppc_test_lrp_search_program_parameters_s {
  int                  nLrpKeyProgram;
  uint32               uTmuProgram;
  uint8                bSubKey0Valid;
  uint8                bSubKey1Valid;
  uint32               uRceProgram;
  uint32               uEtuProgram;
} c3hppc_test_lrp_search_program_parameters_t;



#define C3HPPC_TEST__LINE_INTERFACE    0
#define C3HPPC_TEST__FABRIC_INTERFACE  1


c3hppc_test_list_entry_t *c3hppc_find_test(char *sKey);
void                      c3hppc_populate_with_defaults(int nUnit, c3hppc_bringup_control_t *pBringUpControl);
int                       c3hppc_bringup(int nUnit, c3hppc_bringup_control_t *pBringUpControl);
int                       c3hppc_is_test_done(int nUnit);
int                       c3hppc_wait_for_test_done(int nUnit);
void                      c3hppc_cmu_counter_ring_manager(void *pCounterRingManagerCB_arg);
int                       c3hppc_check_stats( int nUnit, int nSrcInterface, int nDstInterface, int nPacketSizeChange ); 




void   c3hppc_test__setup_dm_table_contents( uint32 uDmLookUp, int nTableNumber, int nTableEntryNum,
                                             uint32 uFirst4Bytes, uint32 *pBuffer );
uint64 c3hppc_test__get_current_epoch_count( int nUnit );
int    c3hppc_test__wait_for_updaters_to_be_idle( int nUnit, int nTimeOut );
uint64 c3hppc_test__display_pm_stats( int nUnit, int nPmInstance, int nStartIndex, uint32 uPmBucketShift, char *pStatName );
void   c3hppc_test__setup_eml_table_contents( int nSetupOptions, int nMaxKey, int nRootTable,
                                              uint32 *pRootTableContents, uint32 *pEML_InsertList );
void   c3hppc_test__setup_eml64_key_list_entry( uint32 nInsertIndex, uint32 nEntryKey, uint32 *pList );
void   c3hppc_test__setup_emc_table_contents( int nSetupOptions, uint32 *pRootTableContents, uint32 *pNextTableContents,
                                              int nRootTable, int nNextTable );
void   c3hppc_test__setup_emc64_table_entry( uint32 u128Mode, uint32 uEntryKey, uint32 uIndex, uint32 *pTable );
void   c3hppc_test__setup_lpmipv4_table_contents( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                  int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                  int nBucketPopulatedPrefixNumber,
                                                  uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                  uint32 *pIPV4BucketTableContents, uint32 **apIPV4AssocDataTableContents );
void   c3hppc_test__setup_unified_lpmipv4_table_contents( int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                          int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                          int nBucketPopulatedPrefixNumber,
                                                          uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                          uint32 *pIPV4BucketTableContents, uint32 *pIPV4AssocDataTableContents );
void   c3hppc_test__setup_lpmipv6_table_contents( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                  int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                  int nBucketPopulatedPrefixNumber,
                                                  uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                  uint32 *pIPV6BucketTableContents, uint32 **apIPV6KeyAndAssocDataTableContents );
void   c3hppc_test__setup_lpmipv6_table_contents_with_collisions( int nTapsInstance, int nSetupOptions, int nBucketPrefixNumber, uint32 uBucketTable,
                                                                  int nRPBpivotNumber, int nSegment, int nBBXmaxPivotNumber, int nBBXpopulatedPivotNumber,
                                                                  int nBucketPopulatedPrefixNumber,
                                                                  uint32 *pRPBcommands, uint32 *pBBXcommands, uint32 *pBRRcommands,
                                                                  uint32 *pIPV6BucketTableContents, uint32 **apIPV6KeyAndAssocDataTableContents,
                                                                  int nCollisionControl );
int    c3hppc_test__get_error_state_summary( int nUnit );
int    c3hppc_test__get_ocm_next_physical_block_0to63( void );
int    c3hppc_test__get_ocm_next_physical_block_64to127( void );
int    c3hppc_test__get_test_duration( void );
uint32 c3hppc_test__get_interrupt_summary( int nUnit );


/****************************************************************
 *                                                              *
 *                      Test Functions                          *
 *                                                              *
 * Each test function may have an optional corresponding        *
 * "init" function and "done" function.                         *
 *                                                              *
 ****************************************************************/

/* c3hppc_ocm_store_test1.c */
extern  int     c3hppc_ocm_store_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_store_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_store_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_ocm_load_test1.c */
extern  int     c3hppc_ocm_load_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_load_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_load_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_ocm_store_load_test1.c */
extern  int     c3hppc_ocm_store_load_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_store_load_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ocm_store_load_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_cmu_test1.c */
extern  int     c3hppc_cmu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cmu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cmu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_cop_test1.c */
extern  int     c3hppc_cop_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_cop_test2.c */
extern  int     c3hppc_cop_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_cop_test3.c */
extern  int     c3hppc_cop_test3__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test3__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test3__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_cop_test4.c */
extern  int     c3hppc_cop_test4__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test4__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_cop_test4__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_sws_test1.c */
extern  int     c3hppc_sws_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_sws_test2.c */
extern  int     c3hppc_sws_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_sws_test3.c */
extern  int     c3hppc_sws_test3__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test3__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test3__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_sws_test4.c */
extern  int     c3hppc_sws_test4__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test4__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test4__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_sws_test5.c */
extern  int     c3hppc_sws_test5__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test5__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_sws_test5__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_rce_test1.c */
extern  int     c3hppc_rce_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_rce_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_rce_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_rce_test2.c */
extern  int     c3hppc_rce_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_rce_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_rce_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_tmu_test1.c */
extern  int     c3hppc_tmu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_tmu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_tmu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_ddr_test_suite.c */
extern  int     c3hppc_ddr_test_suite__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ddr_test_suite__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_ddr_test_suite__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_etu_test1.c */
extern  int     c3hppc_etu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_etu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_etu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_exerciser_test1.c */
extern  int     c3hppc_exerciser_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_exerciser_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3hppc_exerciser_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

/* c3hppc_exerciser_test1.c */
extern  int     c3_exerciser_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3_exerciser_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3_exerciser_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);

extern  int     c3_ut_ocm_test1_init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3_ut_ocm_test1_run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);
extern  int     c3_ut_ocm_test1_done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData);


#endif  /* _TEST_C3HPPC_TEST_H */

