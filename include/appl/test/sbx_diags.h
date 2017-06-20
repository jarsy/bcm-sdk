 /*
 * $Id: sbx_diags.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _SBX_DIAGS_H_
#define _SBX_DIAGS_H_

#include <soc/defs.h>
#include <soc/sbx/sbTypesGlue.h>
#ifndef __KERNEL__
#include <string.h>
#endif
#include <appl/diag/sbx/brd_sbx.h>

#define REG_OFFSET(reg)    SAND_HAL_REG_OFFSET(CA, reg)
#define REG_OFFSET_C2(reg) SAND_HAL_REG_OFFSET(C2, reg)
#define BME32K_MIN_CLOCK_DELAY 500
#define SB_BME_MAX_TABLE_K 1
#define SB_BME_MAX_BANK_K 2
#define SB_BME_MAX_OFFSET_K 0x4000
#define ERR_LIST_CNT 29
#define FE2KXT_ERR_LIST_CNT 28
#define AG0_INT 2
#define AG1_INT 3
#define PCI_INT 

#define INTERPACKET_GAP() thin_delay(96)

#define DIAG_IF_ERROR_RETURN(op) \
  do { int __rv__; if ((__rv__ = (op)) < 0) { \
	 printf("line:%d,%s\n",__LINE__,bcm_errmsg(__rv__)); \
	 return(__rv__); } \
} while(0)

#ifdef BCM_SIRIUS_SUPPORT
typedef enum _siriusDiagsMemTests {
  SIRIUS_DDR_STANDARD_TEST = 0,
  SIRIUS_DATA_BUS_WALKING_ONES = 1,
  SIRIUS_DATA_BUS_WALKING_ZEROS = 2,
  SIRIUS_DDR_DATA_EQ_ADDR = 3,
  SIRIUS_DDR_INDIRECT_TEST = 4,
  /* leave as last */
  SIRIUS_MEM_TEST_LAST
} siriusDiagsMemTests_t;
#endif

/* get the board id */
extern uint8 sbx_diag_get_board_type(void);
/* get the board revision */
extern uint8 sbx_diag_get_board_rev(void);

typedef int (*sbBme32kIndirectMemRead_pf)
            (sbregshandle bmeaddr,
	     uint32     ulTblId,
	     uint32     ulOffset,
	     uint32     *pulData);


typedef int (*sbBme32kIndirectMemWrite_pf)
            (sbregshandle bmeaddr,
	     uint32     ulTblId,
	     uint32     ulOffset,
             uint32     ulData);

typedef enum BME32K_STATUS_ET_e {
    BME32K_SUCCESS_E = 0,
    BME32K_BAD_SIZE_E,
    BME32K_MEM_ACC_TIMEOUT_E
} BME32K_STATUS_ET;


typedef enum _sbBme3200DiagsMemTests {
  SB_BME3200_DATA_PATTERN = 0,
  SB_BME3200_AAA_TEST = 1,
  /* leave as last */
  SB_BME3200_MEM_TEST_LAST
} sbBme3200DiagsMemTests_t;

typedef enum _sbQe2000DiagsBistTests {
  SB_QE2000_INTERNAL_BIST = 0,
  SB_QE2000_EXTERNAL_BIST = 1,
  /* leave as last */
  SB_QE2000_BIST_TEST_LAST
} sbQe2000DiagsBistTests_t;

/* FE2000 */
typedef enum sbFe2000DiagsStatus_e {
    SB_FE2000_SUCCESS_E = 0,
    SB_FE2000_BAD_SIZE_E,
    SB_FE2000_MEM_ACC_READ_TIMEOUT_E,
    SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E,
    SB_FE2000_DATA_READ_MISMATCH_E
} sbFe2000DiagsStatus_t;

/* loopback tests */
typedef enum _sbFe2000DiagsLoopbackType {
    SB_FE2000_1G_LOOPBACK = 0,
    SB_FE2000_1G_UNIMAC_LOOPBACK=1,
    SB_FE2000_1G_PHY_LOOPBACK=2,
    SB_FE2000_10G_LOOPBACK=3, 
    SB_FE2000_10G_QE_LOOPBACK=4,
    /* leave as last */
    SB_FE2000_LOOPBACK_TEST_LAST
} sbFe2000DiagsLoopbackType_t;

typedef enum _sbFe2000DiagsMemTests {
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 0,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 1,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 2,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 3,
  SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT0 = 4,
  SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT0 = 5,
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 6,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 7,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 8,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 9,
  SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT1 = 10,
  SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT1 = 11,
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT = 12,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT = 13,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 14,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 15,
  SB_FE2000_MM0_ADDRESS_BUS_WIDE_PORT = 16,
  SB_FE2000_MM1_ADDRESS_BUS_WIDE_PORT = 17,
  SB_FE2000_MM0_RAND_NARROW_PORT0 = 18,
  SB_FE2000_MM1_RAND_NARROW_PORT0 = 19,
  SB_FE2000_MM0_RAND_NARROW_PORT1 = 20,
  SB_FE2000_MM1_RAND_NARROW_PORT1 = 21,
  SB_FE2000_MM0_RAND_WIDE_PORT = 22,
  SB_FE2000_MM1_RAND_WIDE_PORT = 23,
  /* leave as last */
  SB_FE2000_MEM_TEST_LAST
} sbFe2000DiagsMemTests_t;

typedef enum _sbFe2000DiagsMemType {
  SB_FE2000_NARROW_PORT_0=0,
  SB_FE2000_NARROW_PORT_1,
  SB_FE2000_WIDE_PORT,
  /* leave as last */
  SB_FE2000_MEM_TYPE_LAST
} sbFe2000DiagsMemType_t;


typedef enum _sbFe2000DiagsBistTests {
  SB_FE2000_PP_CAM_ALL,
  SB_FE2000_PP_CAM_0,
  SB_FE2000_PP_CAM_1,
  SB_FE2000_PP_CAM_2,
  SB_FE2000_PP_CAM_3,
  /* leave as last */
  SB_FE2000_BIST_LAST
} sbFe2000DiagsBistTests_t;

typedef struct sbFe2000DiagsWidePortConfig_s {
  uint32 addr_width;
  uint32 data_width;
} sbFe2000DiagsWidePortConfig_t;

typedef struct sbFe2000DiagsNarrowPortConfig_s {
  uint32 addr_width;
  uint32 data_width;
} sbFe2000DiagsNarrowPortConfig_t;

/* FE2KXT */
typedef enum sbFe2kxtDiagsStatus_e {
    SB_FE2KXT_SUCCESS_E = 0,
    SB_FE2KXT_BAD_SIZE_E,
    SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E,
    SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E,
    SB_FE2KXT_DATA_READ_MISMATCH_E
} sbFe2kxtDiagsStatus_t;

/* loopback tests */
typedef enum _sbFe2kxtDiagsLoopbackType {
    SB_FE2KXT_1G_LOOPBACK = 0,
    SB_FE2KXT_1G_UNIMAC_LOOPBACK=1,
    SB_FE2KXT_1G_PHY_LOOPBACK=2,
    SB_FE2KXT_10G_LOOPBACK=3, 
    SB_FE2KXT_10G_QE_LOOPBACK=4,
    /* leave as last */
    SB_FE2KXT_LOOPBACK_TEST_LAST
} sbFe2kxtDiagsLoopbackType_t;

typedef enum _sbFe2kxtDiagsMemTests {
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 0,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 1,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 2,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 3,
  SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT0 = 4,
  SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT0 = 5,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 6,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 7,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 8,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 9,
  SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT1 = 10,
  SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT1 = 11,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT = 12,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT = 13,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 14,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 15,
  SB_FE2KXT_MM0_ADDRESS_BUS_WIDE_PORT = 16,
  SB_FE2KXT_MM1_ADDRESS_BUS_WIDE_PORT = 17,
  SB_FE2KXT_MM0_RAND_NARROW_PORT0 = 18,
  SB_FE2KXT_MM1_RAND_NARROW_PORT0 = 19,
  SB_FE2KXT_MM0_RAND_NARROW_PORT1 = 20,
  SB_FE2KXT_MM1_RAND_NARROW_PORT1 = 21,
  SB_FE2KXT_MM0_RAND_WIDE_PORT = 22,
  SB_FE2KXT_MM1_RAND_WIDE_PORT = 23,
  /* leave as last */
  SB_FE2KXT_MEM_TEST_LAST
} sbFe2kxtDiagsMemTests_t;

typedef enum _sbFe2kxtDiagsMemType {
  SB_FE2KXT_NARROW_PORT_0=0,
  SB_FE2KXT_NARROW_PORT_1,
  SB_FE2KXT_WIDE_PORT,
  /* leave as last */
  SB_FE2KXT_MEM_TYPE_LAST
} sbFe2kxtDiagsMemType_t;


typedef enum _sbFe2kxtDiagsBistTests {
  SB_FE2KXT_PP_CAM_ALL,
  SB_FE2KXT_PP_CAM_0,
  SB_FE2KXT_PP_CAM_1,
  SB_FE2KXT_PP_CAM_2,
  SB_FE2KXT_PP_CAM_3,
  /* leave as last */
  SB_FE2KXT_BIST_LAST
} sbFe2kxtDiagsBistTests_t;

typedef struct sbFe2kxtDiagsWidePortConfig_s {
  uint32 addr_width;
  uint32 data_width;
} sbFe2kxtDiagsWidePortConfig_t;

typedef struct sbFe2kxtDiagsNarrowPortConfig_s {
  uint32 addr_width;
  uint32 data_width;
} sbFe2kxtDiagsNarrowPortConfig_t;

#ifdef BCM_SIRIUS_SUPPORT
typedef struct siriusDiagsParams_s {
  int ci_interface;                               /* CI Interface to run test on */
  int ddr_step_addr;                              /* step address increment */
  int ddr_start_addr;                             /* starting address */
  int ddr_burst;                                  /* specifies number of writes to burst followed by same number of reads */
  int ddr_iter;                                   /* how many iterations to run the test */
  siriusDiagsMemTests_t ddr_test_mode;            /* 0- standard,1-walk ones data,2-walk zeros data */
  int bank;                                       /* Internal DDR bank */
  int max_row;                                    /* max row within bank to test */
  int ddr3_col;                                   /* ddr3 columns configuration */
  int ddr3_row;                                   /* ddr3 rows configuration */
} siriusDiagsParams_t;
#endif


typedef struct _sbxDiagsInfo {
#ifdef BCM_SIRIUS_SUPPORT
  int inited;                                     /* a flag to allow TR50 to run on external memories */
#endif
  uint32 debug_level;                           /* set the debug level */
  uint8 bStopLoopingPacket;                     /* for snake test, to stop pkt or not */
  uint8 bLoopOnError;                           /* for debug, allow the same error condition to repeat so board can be probed */
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  unsigned long start_addr;                       /* starting address */
  unsigned long end_addr;                         /* ending address */
  uint8 bEndAddr;                               /* flag that is set when user specified an end_addr */
  unsigned long max_addr;                         /* temp max, will be broken up into np0,np1,wp for flexibility */
  int64  pattern;                               /* data pattern to use in random memory tests or bist test*/
  int32 cam;                                    /* which pp cam to test (-1 tests them all) */
  uint8  walkbit;                               /* flag to indicate walk ones(==1) or zeros */
  volatile uint64 *pDataRd;                     /* data read out of memory */
  sbFe2000DiagsMemTests_t e_mem_test;             /* Indicates the memory test to be performed */
  sbFe2000DiagsMemType_t  e_mem_type;             /* Indicates the type of memory to do the test on */
  sbFe2000DiagsLoopbackType_t e_loopback_type;    /* loopback test type */
  sbBme3200DiagsMemTests_t e_bme_mem_test;        /* Indicates type of memory test to do on BME */
  sbQe2000DiagsBistTests_t e_qe_bist_test;        /* Indicates type of bist to run */
#ifdef BCM_SIRIUS_SUPPORT
  siriusDiagsParams_t   siriusDiagParams;         /* Diag parameters for sirius chip */
#endif
  int seed;                                       /* seed - used in random testing */
  int32 nLSFR;                                  /* for SPI PRBS test which LSFR to test */
  int32 nInvert;                                /* for SPI PRBS to test with inverted data */
  int spi_interface;                              /* spi interface to test */
  uint8 mem_inited[2][SB_FE2000_MEM_TYPE_LAST]; /* flag to indicate memory has been intialized */
  uint8 mem_override;                           /* override protection scheme flag */
  uint8 DDRTrained;                             /* flag to indicate DDR has been trained */
  int32  nBmeRedundancyOff;                     /* enable redundancy (for BME3200 Memory Test) */
  uint32 uInstance;                             /* the MMU Instance */
  uint8 bPolicerRefreshWasOn;                   /* flag to indicate if background refresh was on */
  uint8 bInternalLpk;                           /* for loopback pkts internally (1g-Unimac,10g-BigMac)*/
  uint8 b1GPhyLpk;                              /* loopback pkts at the 1g phy (AGM0 and AGM1)*/
  uint8 b10GPhyLpk;                             /* loopback pkts at the 10G phy (XFP Loopback) */
  uint32 nInjectPackets;                        /* Number of packets to inject for loopback tests */
  uint32 nInjectPacketsLen;                     /* Length of packets to inject */
  uint8 bSnakeLoopOn;                           /* loop the packet forever (snake test ) */
  sbFe2000DiagsWidePortConfig_t wp_config;        /* wide port configuration */
  sbFe2000DiagsNarrowPortConfig_t np0_config;     /* Narrow port0 configuration */
  sbFe2000DiagsNarrowPortConfig_t np1_config;     /* Narrow port1 configuration */
  uint32 start_port;
  uint32 end_port;
  uint32 uSnakeRunTime;                         /* in seconds (approx) */
  uint32 u10gPort;                              /* (24 Cu, 25 Fiber ) */
  uint32 unit;                                  /* BCM unit # */
  unsigned long uMemExistsMask;                   /* linerate mem test (pop mems) */
  int dllphase;                                   /* Phase to use in DLL_DEBUG during line rate test */
  uint8 bForcePRBSError;
  int prbs_direction;                             /* to allow prbs direction QEXX <--> Polaris (0) --> PL, (1) --> QEXX*/
  uint32 qe_prbs_mask;                          /* enable mask for prbs testing */
  uint32 qe_prbs_link_mask;                     /* specify links to do prbs testing */
  int init_ports;                                 /* option to re-init ports during loopback testing */
  int reinit;                                     /* at test done, reinit chip */
} sbxDiagsInfo_t;


/* Diag Info for the Qe2000 Device */
typedef struct _sbxQe2000DiagsInfo {
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  uint32 unit;                                  /* BCM unit # */
  uint32 debug_level;                           /* Set the debug level */
  uint32 uPrintTime;                            /* Enabling Printing of Elapsed Seconds. */
  uint32 uForceTestPass;                        /* Enable TR test to return immediately with Success. Used for Script Testing. */
  uint32 uForceTestFail;                        /* Enable TR test to return immediately with Failure. Used for Script Testing. */
  uint32 uPackets;                              /* Number of Packets to inject for test */
  uint32 uPacketsLen;                           /* Length of Packets to inject */

  uint32 uTxPktCnt;                             /* Number of Packets Transmitted from Processor into QE PCI  */
  uint32 uTxByteCnt;                            /* Number of Bytes Transmitted from Processor into QE PCI */

  uint32 uRxPktCnt;                             /* Number of Packets received by Processor from QE PCI, unit0  */
  uint32 uRxByteCnt;                            /* Number of Bytes received by Processor from QE PCI, unit0 */

  uint32 uRxPktCnt1;                            /* Number of Packets received by Processor from QE PCI, unit1  */
  uint32 uRxByteCnt1;                           /* Number of Bytes received by Processor from QE PCI, unit1 */

  uint32 ulRb0Queue;                            /* Packet coming in from RB0 will be enqueued to this Queue */
  uint32 ulRb1Queue;                            /* Packet coming in from RB1 will be enqueued to this Queue */
  uint32 ulPciQueue;                            /* Packet coming in from PCI will be enqueued to this Queue */
  uint32 uRunTime;                              /* Duration for which this test will run, in seconds (approx) */
  uint32 uTimeOut;                              /* Duration to wait, in seconds(approx), for Rx packets to trickle out. */

  uint32 uUseFile;                              /* User has specified the packet in a file */
  uint32 uDualKa;                               /* Test is being run on the KA Benchscreen Board with dual KA's */

  char*    pInFile;                               /* Name of the Input Binary Packet File. User specifies the packet */
                                                  /*   in this binary file. */
  char*    pOutFile;                              /* Name of the Output Binary Packet File. Packets coming out of the PCI */
                                                  /*   end up in this binary file. */
  char*    pOutFile1;                             /* Name of the Output Binary Packet File. Packets coming out of the PCI */
                                                  /*   end up in this binary file. This binary file is used when test is  */
                                                  /*   run simultaneously on two KA devices, such as on the Kamino        */
                                                  /*   Benchscreen Board. */
  char*    pPattern;                              /* User specifies this 4Byte pattern(in hex format) to increment & fill */
                                                  /*   the payload. */
  char*    pNoincr;                               /* User specifies this nByte pattern(in hex format) to fill the payload. The */
                                                  /*   payload is filled repetitively with this pattern upto nInjectPacketsLen. */
  char*    pPayload;                              /* The payload is filled with this pattern and the rest is padded with zeros */
                                                  /*   upto nInjectPacketsLen. */

} sbxQe2000DiagsInfo_t;


/* Prbs Info for the Qe2000 Device */
typedef struct _sbxQe2000PrbsInfo {
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  uint32 unit;                                  /* BCM unit # */
  uint32 debug_level;                           /* Set the debug level, same as nVerbose */

  uint32 uNumIters;                             /* Number of PRBS Test Iterations */ 
  uint32 uRunTime;                              /* Duration for which this test will run, in seconds (approx) */
  uint32 uSleepTime; 
  uint32 uTimeOut;                              /* Duration to wait, in seconds(approx), for Test to end. */
  uint32 uPrintTime;                            /* Enabling Printing of Elapsed Seconds. */
  uint32 uForceTestPass;                        /* Enable TR test to return immediately with Success. Used for Script Testing. */
  uint32 uForceTestFail;                        /* Enable TR test to return immediately with Failure. Used for Script Testing. */
             
  uint32 uDualKa;                               /* Test is being run on the KA Benchscreen Board with dual KA's */
  uint32 uDo8b10b;                              /* Run the 8b/10b Test. */ 
  uint32 uUsePrbsPoly15;                        /* Use the 15th order PRBS Polynomial */ 
  uint32 uExitOnError;                          /* If set, Exit test on First Error */ 
  uint32 uUseSweep;                             /* If set, use the range values instead of TupleList */ 
             
  char*    pTupleList;                            /* The sfx_si_config3 fields are specified as a comma seperated */
                                                  /* list of Tuples, i.e. {lodrv, dtx, deq} e.g. "1_8_15,0_7_14,0_3_5" */

                                                  /* sf1_si_config3: 0x08200000 */             
  uint32 uDeqLo;                                /*     deq:  0x00000000 4Bits */
  uint32 uDeqHi;                                
  uint32 uDtxLo;                                /*     dtx:  0x00000008 4Bits */
  uint32 uDtxHi;                               
  uint32 uLoDrvLo;                              /*   lodrv:  0x00000001 1Bit  */
  uint32 uLoDrvHi;                            
             
  uint32 uSdLbm;                                /* Serdes Lane Bitmap */

} sbxQe2000PrbsInfo_t;


/*****************************************************************************
 * FUNCTION NAME:   sbBme32kSoftReset
 *
 * OVERVIEW:        Soft Reset the BME and run bist
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *
 * RETURNS:         None.
 *
 * DESCRIPTION:     Place the BME3200 into reset, run BIST
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    BME is reset, BIST is run
 *
 *****************************************************************************/
void sbBme32kSoftReset(sbhandle hdl);

/*****************************************************************************
 * FUNCTION NAME:   sbBme32KBwRedundSet
 *
 * OVERVIEW:        Set the redundancy for the BW banks
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  onoff - nonzero disables redundancy
 *
 * RETURNS:         0 - Success - Always
 *
 * DESCRIPTION:     Set the redundancy for the BW banks
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32KBwRedundSet(sbxDiagsInfo_t *pDiagsInfo);

/*****************************************************************************
 * FUNCTION NAME:   dgBme32KMBistSet
 *
 * OVERVIEW:        Set the BIST ENABLE for BME3200
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  onoff - nonzer enables master BIST.
 *
 * RETURNS:         0 - Success - Always
 *
 * DESCRIPTION:     Set the BIST ENABLE for BME3200
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32KMBistSet(sbhandle bmeaddr, int onoff);



/*
 *  BME3200 Diags
 */

int32 sbBme3200BistStart(sbxDiagsInfo_t *pDiagsInfo);
int bme32kBIST(sbxDiagsInfo_t *pDiagsInfo, uint32 *stat0_pul,
	       uint32 *stat1_pul, uint32 *stat2_pul,
	       uint32 *stat3_pul);


/* Runs all BME memory tests */
int sbBme3200DiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo);
/* Runs a specified bme mem test */
int sbBme3200DiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo);


/*****************************************************************************
 * FUNCTION NAME:   sbBme32SramDataPatTest
 *
 * OVERVIEW:        Perform a SRAM data pattern test on both banks of BME3200
 *                  memory
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulDataPattern - data pattern to fill & compare with
 *                  ulBailOnFail - return from the test on the first failure
 *
 * RETURNS:         0 - Success
 *                  -1 - Compare failure
 *                       pulAddress - offset on the first failure
 *                       pulTblId - table id of the failure
 *                       pulBankId - bank of the failure
 *                  -2 - Write/Read Failure (timeout)
 *                       pulAddress - offset of the r/w failure
 *                       pulTblId - table id of the r/w failure
 *                       pulBankId - bank id of the r/w failure
 *                       pulData - 1 = writing at time of failure
 *                                 0 = reading at time of failure
 *
 *
 * DESCRIPTION:     For each bank, fill the memory with the data pattern,
 *                  Then read the bank out and compare with the data pattern.
 *
 * ASSUMPTIONS:	    None.
 *
 * SIDE EFFECTS:    BME is reset, BIST is run, then turned off,
 *                  memory redundancy is turned off
 *
 *****************************************************************************/
int32 sbBmeDataPat(sbxDiagsInfo_t *pDiagsInfo);
int sbBme32SramDataPatTest(sbxDiagsInfo_t *pDiagsInfo, uint32 *pulAddress, 
			   uint32 *pulData, uint32 *pulTblId, 
			   uint32 *pulBankId);



/*****************************************************************************
 * FUNCTION NAME:   sbBme32SramAAAMemTest
 *
 * OVERVIEW:        Perform a SRAM Address At Address data pattern test on
 *                  both banks of BME3200 memory
 *
 * ARGUMENTS:       sbxDiagsInfo_t * (others see below)
 *
 * RETURNS:         0 - Success
 *                  -1 - Compare failure
 *                       pulAddress - offset on the first failure
 *                       pulTblId - table id of the failure
 *                       pulBankId - bank of the failure
 *                  -2 - Write/Read Failure (timeout)
 *                       pulAddress - offset of the r/w failure
 *                       pulTblId - table id of the r/w failure
 *                       pulBankId - bank id of the r/w failure
 *                       pulData - 1 = writing at time of failure
 *                                 0 = reading at time of failure
 *
 *
 * DESCRIPTION:     For each bank, fill each offset with its offset,
 *                  Then read the bank out and compare with the offset.
 *
 * ASSUMPTIONS:	    None.
 *
 * SIDE EFFECTS:    BME is reset, BIST is run, then turned off,
 *                  memory redundancy is turned off
 *
 *****************************************************************************/
int sbBme32SramAAAMemTest(sbxDiagsInfo_t *pDiagsInfo,
			  uint32 *pulAddress, uint32 *pulData, 
			  uint32 *pulTblId, uint32 *pulBankId);

/* starts sbBme32SramAAAMemTest, and reports pass/fail status */
int32 sbBme32SramAAATest(sbxDiagsInfo_t *pDiagsInfo);




/*****************************************************************************
 * FUNCTION NAME:   sbBme32kBwR0/1 Read
 *
 * OVERVIEW:        Perform an indirect memory read of BW Bank 0
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulTblId - the bank table id
 *                  ulOffset - offset into the bank
 *                  pulData - location to store the read data
 *
 * RETURNS:         0 - Success - read data stored in *pulData
 *                  -1 - Timeout waiting for acknowledge
 *
 * DESCRIPTION:     Perform an indirect memory write to BW Bank 0
 *
 * ASSUMPTIONS:	    BME is out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32kBwR0Read(sbhandle bmeaddr, uint32 ulTblId, uint32 ulOffset, uint32 *pulData);
int sbBme32kBwR1Read(sbhandle bmeaddr, uint32 ulTblId, uint32 ulOffset, uint32 *pulData);

/*****************************************************************************
 * FUNCTION NAME:   sbBme32kBwR0/R1 Write
 *
 * OVERVIEW:        Perform an indirect memory write to BW Bank 1
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulTblId - the bank table id
 *                  ulOffset - offset into the bank
 *                  ulData - pattern to fill the memory location with
 *
 * RETURNS:         0 - Success
 *                  -1 - Timeout waiting for acknowledge
 *
 * DESCRIPTION:     Perform an indirect memory write to BW Bank 1
 *
 * ASSUMPTIONS:	    BME is out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32kBwR1Write(sbhandle bmeaddr, uint32 ulTblId, uint32 ulOffset, uint32 ulData);
int sbBme32kBwR0Write(sbhandle bmeaddr, uint32 ulTblId, uint32 ulOffset, uint32 ulData);


/*
 *  QE2K Diags
 */
void sbQe2000_BringUpPmManually(sbhandle userDeviceHandle);
uint32 sbQe2000_PmManInitCmd(sbhandle userDeviceHandle, 
			       uint32 nCmd, 
			       uint32 nManInit);

uint32 sbQe2kPmDDRTrain(sbxDiagsInfo_t *pDiagsInfo,
			  int nHalfBus);

int sbQe2kPmDllWrite(sbhandle tKaAddr,
		     uint32 ulTableId,
		     uint32 ulAddr,
		     uint32 ulData);


int sbQe2kDDRIIExtBistMode0(sbxDiagsInfo_t *pDiagsInfo,
			    uint32 *pulStartAddr,
			    uint32 *pulEndAddr,
			    uint32 aulDataPat[8]);
    
int sbQe2kDDRIIExtBistMode1(sbxDiagsInfo_t *pDiagsInfo,
			    uint32 *pulStartAddr,
			    uint32 *pulEndAddr,
			    uint32 aulDataPat[8]);

int sbQe2kDDRIIExtBistMode2(sbxDiagsInfo_t *pDiagsInfo,
			    uint32 *pulStartAddr,
			    uint32 *pulEndAddr,
			    uint32 aulDataPat[8]);

int sbQe2kDDRIIExtBistTest(sbxDiagsInfo_t *pDiagsInfo,
			   uint32 *pulStartAddr,
			   uint32 *pulEndAddr,
			   uint32 aulDataPat[8]);


int sbQe2kDDRIIExtBistErrorsGet(sbhandle regaddr,
                              uint32 ulTestResults,
                              uint32 *pulUpperAddr,
                              uint32 *pulLowerAddr,
				uint32 aulFailedData[8]);

int sbQe2kPmMemRead(sbhandle tKaAddr,
		    uint32 ulAddr, 
		    uint32 *pulData3, 
		    uint32 *pulData2, 
		    uint32 *pulData1, 
		    uint32 *pulData0);

int sbQe2kPmMemWrite(sbhandle tKaAddr,
		     uint32 ulAddr, 
		     uint32 ulData3, 
		     uint32 ulData2,
		     uint32 ulData1, 
		     uint32 ulData0);

int sbQe2kExtBist(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2kSoftReset(sbhandle qeregaddr);
int sbQe2kIntBist(sbxDiagsInfo_t *pDiagsInfo);
void sbQe2kIntBistStartAll(sbhandle tKaAddr);
int sbQe2000DiagsBistTestAll(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2000DiagsBistTest(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2kIntBistTest(sbxDiagsInfo_t *pDiagsInfo,
		      uint32 *pulStatus,
		      uint32 aulBistStat[15]);

void sbQe2kIntBistCheckAll(sbxDiagsInfo_t *pDiagsInfo, 
			   uint32 *pulStatus, 
			   uint32 aulBistStatus[15]);

void sbQe2000BistSrStart(sbhandle tKaAddr, int nStride);
void sbQe2000BistStStart(sbhandle tKaAddr, int nStride);
void sbQe2000BistPcStart(sbhandle tKaAddr);
void sbQe2000BistPmStart(sbhandle tKaAddr);
void sbQe2000BistEbStart(sbhandle tKaAddr);
void sbQe2000BistTxStart(sbhandle tKaAddr);
void sbQe2000BistSvStart(sbhandle tKaAddr);
void sbQe2000BistQmStart(sbhandle tKaAddr);
void sbQe2000BistEgStart(sbhandle tKaAddr);
void sbQe2000BistEiStart(sbhandle tKaAddr);
void sbQe2000BistEpStart(sbhandle tKaAddr);
void sbQe2000BistRbStart(sbhandle tKaAddr);
void sbQe2000BistQsStart(sbhandle tKaAddr);


int sbQe2000BistSrCheck(sbxDiagsInfo_t *pDiagsInfo,
			int nStride, 
			uint32 *pulStatus0, 
			uint32 *pulStatus1);
int sbQe2000BistPmCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32 *pulStatus0,
			uint32 *pulStatus1);

int sbQe2000BistEbCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32 *pulStatus0, 
			uint32 *pulStatus1);

int sbQe2000BistStCheck(sbxDiagsInfo_t *pDiagsInfo, 
			int nStride, 
			uint32 *pulStatus0, 
			uint32 *pulStatus1);

int sbQe2000BistPcCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32 *pulStatus0, 
			uint32 *pulStatus1);

int sbQe2000BistTxCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32 *pulStatus0, 
			uint32 *pulStatus1);

int sbQe2000BistSvCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32 *pulStatus0,
			uint32 *pulStatus1);

int sbQe2000BistQmCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32 *pulStatus0, 
			uint32 *pulStatus1);

int sbQe2000BistEgCheck( sbxDiagsInfo_t *pDiagsInfo,
			 uint32 *pulStatus0,
			 uint32 *pulStatus1);

int sbQe2000BistEiCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32 *pulStatus0,
			uint32 *pulStatus1);

int sbQe2000BistEpCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32 *pulStatus0,
			uint32 *pulStatus1);

int sbQe2000BistRbCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32 *pulStatus0,
			uint32 *pulStatus1);

int sbQe2000BistQsCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32 *pulStatus0,
			uint32 *pulStatus1);

void sbQe2kPmDumpChannelStatus(uint8 uResultsArr[][16]);


#endif /* _SBX_DIAGS_H_ */
