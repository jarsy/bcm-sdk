/*
 * $Id: sirius_diags.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == sirius_diags.h - Sirius Diagnostics      ==
 */

#ifndef _SIRIUS_DIAGS_H_
#define _SIRIUS_DIAGS_H_

#include "sbx_diags.h"

/* for indirect mem test */
#define CI_WRITE_TIMEOUT (-2)
#define CI_READ_TIMEOUT (-3)
#define CI_DATA_MISMATCH_ERROR (-4)
/* for standard DDR test */
#define DDR_FAILED (-1)
#define DDR_PASSED (1)
#define DDR_TIMEOUT (-2)



/*
 * Runs all DDR Tests
 * @param sbxDiagsInfo Glue handle to the QE4000 + diag parameters
 * @return                 Status, BCM_E_NONE or failure code.
 */

int siriusDiagsDDRTestAll(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Runs a specific DDR test 
 * @param sbxDiagsInfo Glue handle to the QE4000 + diag parameters
 * @return                 Status, BCM_E_NONE or failure code.
 */

int siriusDiagsDDRTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Runs a specific DDR test 
 * @param sbxDiagsInfo Glue handle to the QE4000 + diag parameters
 * Collect passing/failing statistics for each ci being tested
 */

int siriusDiagsResultsDDRFunctionalTest(sbxDiagsInfo_t *pDiagsInfo,
					 int ci_start,
					 int ci_end,
					 uint64 *pFailedCount,
					 uint64 *pTimedOutCount,
					 uint64 *pPassedCount);

/* used when running multiple iterations */
int siriusDiagsDDRDumpCIResults(int unit,
				int ci_start,
				int ci_end,
				uint64 iter_count,
				uint64 *pFailedCount,
				uint64 *pTimedOutCount,
				uint64 *pPassedCount);

int siriusDiagsStartDDRFunctionalTest(sbxDiagsInfo_t *p,
				      int ci_start,
				      int ci_end,
				      int mode);

/*
 * Built in HW functional test to test memory at speed.
 * @param sbxDiagsInfo Glue handle to the QE4000 + diag parameters
 * @return                 Status, BCM_E_NONE or failure code.
 */

int siriusDiagsDDRFunctionalTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Test DDR using indirect read/writes
 * This is a backup test to the functional test.
 *
 * NOTE: This is not an exahustive test of the memory, it is more of
 * a sanity check. The hw functional test should be used to test data lines,
 * and the memory itself.
 *
 * @param sbxDiagsInfo Glue handle to the QE4000 + diag parameters
 * @return                 Status, BCM_E_NONE or failure code.
 *
 *
 * 
 * The indirect address is mapped to the DDR as follows:
 * 
 * PLA_ADDR
 * 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02   01   00
 * |--------- row[13:0] -------------------| |---- col[10:4] ---| |-bank[2:0]-|
 *
 *
 * Sirius supports 3 types for DDR3 memory:
 * 1G 8K  rows x 1k cols x8 banks x16 bits.
 * 2G 16k rows x 1k cols x8 banks x16 bits.
 * 2G 8k  rows x 2k cols x8 banks x16 bits.
 *
 * Indirect access to/from CMIC to external DDR are always on a 32B boundary.
 * Hence col[3:0] = 4'b0000. Not col mapping above starts at 4, so this is
 * taken care of already.
 *
 * For 1G DDR there are 10 bits of col address, 2^10 = 1K. The address above
 * supports up to 11 bits for col address, 2^11 = 2k. With 1G parts the col[10]
 * or pla_addr[9] is not used. 
 *
 *
 * The memory is accessed by first setting up the pla_addr, which will specify
 * the bank, and the col,row within that bank to access. Since all read/writes are 
 * on a 32B boundary the memory is accessed as follows: 
 *
 * Example: 1G DDR (bank0)
 *
 *    \ col
 * row \ 0                    0xf|0x10                    0x1f    0x3f0                0x3ff
 *      -------------------------------- -------------------- .... ----------------------| 
 *  0  |<----- 32B write ------> |<----- 2nd 32B write ----> ...   <-- 64th 32B write -->
 *  1  |
 *  .
 *  .
 *
 * One bank of DDR in this case is 8K rows x 1k cols x 16 bits = 128M
 * There are 8 banks for the DDR, 8*128M = 1024M (1G).
 *
 * To fill a row above within the bank there are 64 32B writes, the col is shifted
 * into the pla_addr so each access is on a 32B boundary. The memory is 16bits wide
 * so writing to cols[0-0xf] will give 16bits*16 = 32B. There are eight 32bit
 * indirect data registers [data0..data7] to fill a 256bit (32B) line with each write.
 *
 * row
 *  0  [ 64*32B = 16*1024 bits ]
 *  1
 *  2
 *  .
 *  .
 *  8k
 *
 *  There are 8k rows. So total bank mem = 8*1024*16*1024 = 128M
 *
 *  The test will fill each bank for each CI interface with a given pattern.
 *  Example run line below, do TR 50 M=EXT_DDR help=1.
 *  TR 50 M=EXT_DDR MemTest=3 pattern=0xdeadbeef. If no pattern is specified, the pla_addr
 *  is used as the data. You can specify the max_row, ci, and/or bank to access.
 *
 *  For Example to only fill row0,bank0 for ci0.
 *  TR 50 M=EXT_DDR MemTest=3 bank=0 max_row=1 ci=0 pattern=0x0
 *
 *  Once the writes to bank(s) are finished the test reads the data back to compare.
 *
 */

int siriusDiagsIndirectPatTest(sbxDiagsInfo_t *pDiagsInfo,
			       uint32 *pCi,
			       uint32 *pBank,
			       uint32 *pRow,
			       uint32 *pCol,
			       uint32 *pData);


int siriusDiagsDDRIndirectPat(sbxDiagsInfo_t *pDiagsInfo);
int siriusDDRWrite(int unit, int ci, uint32 addr, uint32 uData0,
		   uint32 uData1, uint32 uData2, uint32 uData3,
		   uint32 uData4, uint32 uData5, uint32 uData6,
		   uint32 uData7);

int siriusDDRRead(int unit, int ci, uint32 addr, uint32 *pData0,
		  uint32 *pData1, uint32 *pData2, uint32 *pData3,
		  uint32 *pData4, uint32 *pData5, uint32 *pData6,
		  uint32 *pData7);

sbBool_t siriusDiagsWrRdComplete(int unit, int ci, uint32 uTimeout);
extern void sigcatcher_sirius(int); /* handler for ctrl-c */


#endif /* _SIRIUS_DIAGS_H_ */
