/*
 * $Id: qe2000_diags.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_diags.c
 * Purpose:     QE2000-specific diagnostics tests
 * Requires:
 */
#ifdef BCM_SBX_SUPPORT
#include <shared/bsl.h>

#include <appl/test/qe2000_diags.h>

#include <appl/diag/sbx/register.h>
#include <appl/diag/system.h>
#include <appl/diag/test.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/sbFabCommon.h>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/hal_ka_auto.h>
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <bcm/rx.h>
#include <bcm/error.h>
#include <appl/diag/sbx/brd_sbx.h>


extern cmd_result_t cmd_soc_sbx_tx(int unit, args_t *args);
extern cmd_result_t cmd_soc_sbx_rx(int unit, args_t *args);
#define IS_ODD(p) (p%2)
#define IS_EVEN(p) (!IS_ODD(p))

#define QEDIAG_BUF_LEN 50
#define QEDIAG_PKT_BUF_LEN 2000
#define QEDIAG_RT_HDR_SZ 8
#define QEDIAG_SHIM_HDR_SZ 4


/*
 * Packet handler callback. This routine is called when the PCI port receives
 * a packet.
 * This function will be executed within the context of the RX Thread.
 *
    FILE *pCaptureFile = fopen(capture_file, "w");
     fwrite(pd.buf, 1, pd.buf_len, state.capture_file);

 */
typedef struct qediag_cookie_s {
  int unit;
#ifndef NO_SAL_APPL
  FILE* fOutFilePtr;
#endif
  uint32 uRxPktCnt;
  uint32 uRxByteCnt;
} qediag_cookie_t;

/* Cookie for KA0(default) */
static qediag_cookie_t RxInfo  = {0,
#ifndef NO_SAL_APPL 
                                  NULL,
#endif 
                                  0, 
                                  0};

/* Cookie for KA1 */
static qediag_cookie_t RxInfo1 = {0, 
#ifndef NO_SAL_APPL
                                  NULL,
#endif 
                                  0, 
                                  0};

static bcm_rx_t _Qe2000RxHandler(int unit, bcm_pkt_t *pPkt, void *cookie) {
  qediag_cookie_t* pQeDiagCookie = (qediag_cookie_t*)cookie;

  while ( pPkt != NULL ) {
    /* Update Rx Pkt and Byte Counts */
    pQeDiagCookie->uRxPktCnt++;
    pQeDiagCookie->uRxByteCnt += pPkt->pkt_data->len;
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "DEBUG: %s: Writing %dBytes of Rx packet %d \n"),
              FUNCTION_NAME(), pPkt->pkt_data->len, pQeDiagCookie->uRxPktCnt));
#ifndef __KERNEL__   
#ifndef NO_SAL_APPL 
    fwrite(pPkt->pkt_data->data, sizeof(char), pPkt->pkt_data->len, pQeDiagCookie->fOutFilePtr);
#endif
#else
    
#endif
    pPkt = pPkt->next;
  }

  return BCM_RX_HANDLED;
}


void sbQe2000DiagsGatherPkts(sbxQe2000DiagsInfo_t *pDiagsInfo, uint32 unit)
{
  uint32 ulPciQueue = pDiagsInfo->ulPciQueue;
  qediag_cookie_t* pRxInfo = NULL;
  sbhandle qehdl;
  uint32 uTimeOut;

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Gathering packets for unit%d.\n"),
            FUNCTION_NAME(), __LINE__, unit));

  qehdl = SOC_SBX_CONTROL(unit)->sbhdl;

  if (pDiagsInfo->uDualKa && (unit == 1)) {
    pRxInfo = &RxInfo1;
  } else {
    if (unit == 1) {
      pRxInfo = &RxInfo1;
    } else {
      pRxInfo = &RxInfo;
    }
  }

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Redirect Snake to come out of PCI Queue 0x%x.\n"),
            FUNCTION_NAME(), ulPciQueue));
  /* Redirect our 'snake' to come out PCI... Queue# 0x310 is headed to PCI.*/
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_BASE_NOHDR0, IDP0_NO_HEADER_BASE_Q, ulPciQueue);

  /* Start the Traffic, i.e. remove backpressure to SPI0. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Remove BackPressure to SPI0..\n"),
            FUNCTION_NAME()));
  SAND_HAL_WRITE(qehdl, KA, EG_FORCE_FULL_0, 0x0);

  /* The delay is in usec */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Waiting for Packets to tricle out of PCI. Timout: %d seconds\n"),
            FUNCTION_NAME(), pDiagsInfo->uTimeOut));

  uTimeOut = 0;
  while ((pRxInfo->uRxPktCnt < pDiagsInfo->uTxPktCnt) ) {
    if (uTimeOut >= pDiagsInfo->uTimeOut) {
      cli_out("WARNING: %s: unit%d: TimeOut(%d seconds) exceeded before all packets were received.\n",
              FUNCTION_NAME(), unit, pDiagsInfo->uTimeOut);
      cli_out("WARNING: %s: unit%d: Packets Expected: %d, Packets Received so far: %d \n",
              FUNCTION_NAME(), unit, pDiagsInfo->uTxPktCnt, pRxInfo->uRxPktCnt);
      break;
    }
    else {
      /* Sleep for 1 second */
      sal_usleep(1000000);
      /* Decrement the TimeOut */
      uTimeOut++;
    }
  }

  if (pDiagsInfo->uDualKa && (unit == 1)) {
    pDiagsInfo->uRxPktCnt1  = pRxInfo->uRxPktCnt;
    pDiagsInfo->uRxByteCnt1 = pRxInfo->uRxByteCnt;
  } else {
    pDiagsInfo->uRxPktCnt  = pRxInfo->uRxPktCnt;
    pDiagsInfo->uRxByteCnt = pRxInfo->uRxByteCnt;
  }

  /* Stop the Traffic, i.e. block the grants. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Blocking Grants yet again..\n"),
            FUNCTION_NAME()));
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000010);

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Gathering packets for unit%d. Done..\n"),
            FUNCTION_NAME(), __LINE__, unit));
}


int sbQe2000DiagsSetupRxThread(sbxQe2000DiagsInfo_t *pDiagsInfo, uint32 unit)
{
  /* Remember to fopen the file to output the data.  We need to close it too at some point when
   * the diags are done. Do not know how to do that right now.  Probably use a timeout and watch
   * the PCI rx count to determine if the all the injected packets came back.
   */
  char* sOutFileName  = NULL;
  qediag_cookie_t* pRxInfo = NULL;
  int rv = CMD_FAIL;

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Setting up Receive Thread for unit%d.\n"),
            FUNCTION_NAME(), __LINE__, unit));
  /* Turn on the Receive thread.  It will then be ready when we transmit the
   * the packets.
   */
  if (!bcm_rx_active(unit)) {
    rv = bcm_rx_init(unit);
    if (BCM_FAILURE(rv)) {
      cli_out("ERROR: %s: bcm_rx_init(%d) returns %d: %s\n", FUNCTION_NAME(), unit, rv, bcm_errmsg(rv));
      return CMD_FAIL;
    }
    rv = bcm_rx_start(unit, 0);
    if (BCM_FAILURE(rv)) {
      cli_out("ERROR: %s: bcm_rx_start(%d) failed:%d:%s\n", FUNCTION_NAME(), unit, rv, bcm_errmsg(rv));
      return CMD_FAIL;
    }
  }

  if (pDiagsInfo->uDualKa && (unit == 1)) {
    if (pDiagsInfo->pOutFile1 == NULL) {
      cli_out("ERROR: [%s:%d]: Out Filename not Provided for unit%d\n", FUNCTION_NAME(), __LINE__, unit);
      return CMD_FAIL;
    }
    sOutFileName = pDiagsInfo->pOutFile1;
    pRxInfo = &RxInfo1;

    /* Reset the Received Packet Counter */
    pDiagsInfo->uRxPktCnt1  = 0;
    pDiagsInfo->uRxByteCnt1 = 0;
  } else {
    if (pDiagsInfo->pOutFile == NULL) {
      cli_out("ERROR: [%s:%d]: Out Filename not Provided for unit%d\n", FUNCTION_NAME(), __LINE__, unit);
      return CMD_FAIL;
    }
    sOutFileName = pDiagsInfo->pOutFile;

    if (unit == 1) {
      pRxInfo = &RxInfo1;
    } else {
      pRxInfo = &RxInfo;
    }

    /* Reset the Received Packet Counter */
    pDiagsInfo->uRxPktCnt  = 0;
    pDiagsInfo->uRxByteCnt = 0;
  }

  /* Reset the Received Packet Counter */
  pRxInfo->uRxPktCnt  = 0;
  pRxInfo->uRxByteCnt = 0;

  /* The following will make it easier to identify the RxInfo pointer */
  pRxInfo->unit = unit;
  
#ifndef NO_SAL_APPL 
  /* Open the Output file for Writing out the received packets. The received packets
   * are always written out in this test.
   */
  if( (pRxInfo->fOutFilePtr = sal_fopen(sOutFileName, "w")) == NULL ) {
    cli_out("ERROR: %s: Error Opening file %s for Write\n", FUNCTION_NAME(), sOutFileName);
    return CMD_FAIL;
  }
#endif

  /* Register the Received Packet Handler */
  rv = bcm_rx_register(unit, "QE Traffic Test CB", _Qe2000RxHandler, 0x40, pRxInfo, 0);
  if (BCM_FAILURE(rv)) {
      cli_out("ERROR: bcm_register(%d) failed:%d:%s\n", unit, rv, bcm_errmsg(rv));
      return CMD_FAIL;
  }

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Setting up Receive Thread for unit%d. Done..\n"),
            FUNCTION_NAME(), __LINE__, unit));

  return CMD_OK;
}


int sbQe2000DiagsSetupRegisters(sbxQe2000DiagsInfo_t *pDiagsInfo, uint32 unit)
{
  uint32 ulData = 0;
  uint32 ulRb0Queue = pDiagsInfo->ulRb0Queue;
  uint32 ulRb1Queue = pDiagsInfo->ulRb1Queue;
  sbhandle qehdl;  /* Handle to KA device */

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Setting up Registers for unit%d.\n"),
            FUNCTION_NAME(), __LINE__, unit));

  qehdl = SOC_SBX_CONTROL(unit)->sbhdl;

  /* Turn on ECC Checking. */
  SAND_HAL_WRITE(qehdl, KA, EI_MEM_DEBUG0, 0x0);
  SAND_HAL_WRITE(qehdl, KA, EI_MEM_DEBUG1, 0x0);

  /* Set the register to block the grants so that we accumulate the packets that
   * we are about to send in
   */
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000010);

  /* Disable SPI0 and SPI1 interfaces while we make changes */
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_ENABLE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG0, RX_ENABLE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_ENABLE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG0, RX_ENABLE, 0);

  /* NOTE1:
   * Note that the changes to the SPI Calendar for both the interfaces is
   * automatically handled by SDK. We do not need to make them here. One only
   * needs to change the following config.bcm properties.
   * # These properties determine the number of SPI channels assigned to
   * # each bus in the QE.  The QE (fabric) ports are numbered contiguously
   * # across SPI 0 then SPI 1
   * qe_spi_0=1
   * qe_spi_1=1
   */

  /* Put SPI0 interface in internal loopback mode */
  SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG2, LOOPBACK_ENABLE, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, LOOPBACK_ENABLE, 1);

  /* Put SPI1 interface in internal loopback mode */
  SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG2, LOOPBACK_ENABLE, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, LOOPBACK_ENABLE, 1);

  /* Ignore status on the loopback ports for now (we'll re-enable this shortly) */
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_IGN_STAT, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_IGN_STAT, 1);

  /* spi0 starts at port 0, spi1 starts at port 10 */
  /* This is already handled by the SDK so we do nothing for this.
   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI0_CONFIG0, PORT_OFFSET, 0);
   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI1_CONFIG0, PORT_OFFSET, 10);

   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI0_CONFIG1, EI_PORTS31_0_ENABLE, 0x3ff);
   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI0_CONFIG2, EI_PORTS47_32_ENABLE, 0x0);

   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI1_CONFIG1, EI_PORTS31_0_ENABLE, 0x1);
   * SAND_HAL_RMW_FIELD(qehdl, KA, EI_SPI1_CONFIG2, EI_PORTS47_32_ENABLE, 0x0);
   */

  /* make sure our ports are enabled */
  SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG0, IGNORE_PORT_FIELD, 1);
  /* See NOTE1 above.
   * SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG1, RX_PORT_ENABLE, 0x1);
   * SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG2, RX_PORT_ENABLE, 0x0);
   */

  SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG0, IGNORE_PORT_FIELD, 1);
  /* See NOTE1 above.
   * SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG1, RX_PORT_ENABLE, 0x1);
   * SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG2, RX_PORT_ENABLE, 0x0);
   */

  /* Update length in erh */
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_CONFIG, IDP1_CALC_LENGTH, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_CONFIG, IDP1_STORE_AND_FWD, 1);

  SAND_HAL_RMW_FIELD(qehdl, KA, RB_CONFIG, IDP0_CALC_LENGTH, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_CONFIG, IDP0_STORE_AND_FWD, 1);

  /* Keep the erh on the loopback ports. We need to enable this only for the
   * for the ports that we have enabled, above(see NOTE1 above). In this case
   * currently it is ports 0 and 1 and the PCI port, 49.
   */
  SAND_HAL_WRITE(qehdl, KA, EP_BF_ERH0, 0x3);
  SAND_HAL_WRITE(qehdl, KA, EP_BF_ERH1, 0x10000);

  /* setup spi ports length min/max to allow loopback (see 21873) */
  SAND_HAL_WRITE(qehdl, KA, SR0_P0_FRAME_SIZE, 0xffff0000);
  SAND_HAL_WRITE(qehdl, KA, SR1_P0_FRAME_SIZE, 0xffff0000);

  /* if we come in on SPI0, switch to SPI1 */
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_BASE_NOHDR0, IDP0_REPLACE_QUEUE, 1);
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: SPI0: Redirecting Packets Queue:%d\n"),
            FUNCTION_NAME(), ulRb0Queue));
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_BASE_NOHDR0, IDP0_NO_HEADER_BASE_Q, ulRb0Queue);

  /* coming in on SPI1, switch to SPI0 */
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_BASE_NOHDR1, IDP1_REPLACE_QUEUE, 1);
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: SPI1: Redirecting Packets Queue:%d\n"),
            FUNCTION_NAME(), ulRb1Queue));
  SAND_HAL_RMW_FIELD(qehdl, KA, RB_BASE_NOHDR1, IDP1_NO_HEADER_BASE_Q, ulRb1Queue);

  /* Enable SPI0 and SPI1 interfaces once again. */
  SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG0, RX_ENABLE, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_ENABLE, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG0, RX_ENABLE, 1);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_ENABLE, 1);

  /* jts - bypass the EP entirely!  simple! */
  SAND_HAL_WRITE(qehdl, KA, EP_CONFIG, 0);

  /* clear any dip2 framing from status during loopback_enable switchover */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Clearing st0/st1, sr0/sr1 errors...\n"),
            FUNCTION_NAME()));
  ulData = SAND_HAL_READ(qehdl, KA, ST0_ERROR);
  SAND_HAL_WRITE(qehdl, KA, ST0_ERROR, ulData);
  ulData = SAND_HAL_READ(qehdl, KA, ST0_ERROR);
  if (ulData) {
    cli_out("ERROR: Cannot Clear Errors on ST0_ERROR: 0x%08x \n", ulData);
    return CMD_FAIL;
  }

  ulData = SAND_HAL_READ(qehdl, KA, ST1_ERROR);
  SAND_HAL_WRITE(qehdl, KA, ST1_ERROR, ulData);
  ulData = SAND_HAL_READ(qehdl, KA, ST1_ERROR);
  if (ulData) {
    cli_out("ERROR: Cannot Clear Errors on ST1_ERROR: 0x%08x \n", ulData);
    return CMD_FAIL;
  }

  ulData = SAND_HAL_READ(qehdl, KA, SR0_ERROR);
  SAND_HAL_WRITE(qehdl, KA, SR0_ERROR, ulData);
  ulData = SAND_HAL_READ(qehdl, KA, SR0_ERROR);
  if (ulData) {
    cli_out("ERROR: Cannot Clear Errors on SR0_ERROR: 0x%08x \n", ulData);
    return CMD_FAIL;
  }

  ulData = SAND_HAL_READ(qehdl, KA, SR1_ERROR);
  SAND_HAL_WRITE(qehdl, KA, SR1_ERROR, ulData);
  ulData = SAND_HAL_READ(qehdl, KA, SR1_ERROR);
  if (ulData) {
    cli_out("ERROR: Cannot Clear Errors on SR1_ERROR: 0x%08x \n", ulData);
    return CMD_FAIL;
  }

  /* Ok, pay attention to status again */
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_IGN_STAT, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_IGN_STAT, 0);

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Setting up Registers for unit%d. Done..\n"),
            FUNCTION_NAME(), __LINE__, unit));

  return CMD_OK;
}


int sbQe2000DiagsTrafficTestSingleKA(sbxQe2000DiagsInfo_t *pDiagsInfo)
{
  /* Remember to fopen the file to output the data.  We need to close it too at some point when
   * the diags are done. Do not know how to do that right now.  Probably use a timeout and watch
   * the PCI rx count to determine if the all the injected packets came back.
   */
  uint32 unit = pDiagsInfo->unit;
  /* REUSE: Use this as a flag to indicate that the packet data is in the binary file */
  uint32 nUseFile = pDiagsInfo->uUseFile;
  uint32 ulTestBit = 0;
  qediag_cookie_t* pRxInfo = NULL;
  sbhandle qehdl;
  char* rh_cmd = NULL;
  char* shim_cmd = NULL;
  char* pay_cmd = NULL;
  char* len_cmd = NULL;
  /* soc_sbx_chip_info_t *chip_info = NULL; */
  int i;
  args_t *args;
  cmd_result_t val;
  /**** TEMP: reuse for Debug ****/
  uint32 ulRb1Queue = pDiagsInfo->ulRb1Queue;

  /* RunTime is specified in seconds, but sal_usleep uses microsec. So, covert. */
  uint32 uRunTime = pDiagsInfo->uRunTime * 1000 * 1000;
  int rv = CMD_OK;

  if (!SOC_IS_SBX_QE2000(unit)) {
    cli_out("ERROR: unit%d needs to be a QE2000 Device for this test.\n", unit);
    return CMD_FAIL;
  }

  if (soc_property_get(unit, spn_QE_TME_MODE, 0) != 1) {
    cli_out("ERROR: unit%d(QE2000) needs to be setup to operate in TME Mode For this test.\n", unit);
    return CMD_FAIL;
  }

  qehdl = SOC_SBX_CONTROL(unit)->sbhdl;

  rh_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag rh_cmd string");
  if (!rh_cmd) {
    cli_out("ERROR: %s: RouteHeader pointer allocation failed\n", FUNCTION_NAME());
    return CMD_FAIL;
  }
  sal_memset(rh_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

  shim_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag shim_cmd string");
  if (!shim_cmd) {
    cli_out("ERROR: %s: ShimHeader pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(shim_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

  pay_cmd = sal_alloc((sizeof(char) * QEDIAG_PKT_BUF_LEN), "QE Diag pay_cmd string");
  if (!pay_cmd) {
    cli_out("ERROR: %s: Payload pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(pay_cmd, 0, sizeof(char) * QEDIAG_PKT_BUF_LEN);

  len_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag len_cmd string");
  if (!len_cmd) {
    cli_out("ERROR: %s: Length pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(len_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

/******************************************************************************************************/

  /* Setup Registers for KA */
  if ((rv = sbQe2000DiagsSetupRegisters(pDiagsInfo, unit)) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }

/******************************************************************************************************/

  /* The following is just a way to get access to registers by name, if needed.
   * We do not use it here.  Just leaving this code here if we need to use it
   * in the future.
   */
#if 0
  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
      cli_out("ERROR: Register info unknown for unit %d \n", unit);
      return CMD_FAIL;
  }
#endif

  /***************************************************************************/
  /* Setup the Receive Thread for KA */
  if ((rv = sbQe2000DiagsSetupRxThread(pDiagsInfo, unit)) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }
  /***************************************************************************/
  
  if (!nUseFile) {
    /* User has indicated to create the RH, SH and the Pkt Data on the fly.
     */
    /* The Queue Id occupies the top 14 bits of the 1st word in the ERH (i.e. 0xfffc0000)
     * The Packet Length occupies the lower 14 bits of the 1st word in the ERH (i.e. 0x00003fff)
     * The following ERH is for a 238Byte packet headed to Queue 0x10 
     * 0x004040e6 0x04000026  0x40000000
     * The TEST Bit is Bit 14 in the 1st word of ERH. Turn the test bit off to
     * see the port counts.
     */
    val = sal_sprintf(rh_cmd, "rh=0x%04x%04x04000026", 
                    ((ulRb1Queue & 0x3fff) << 2),
                    ((ulTestBit << 14) | ((pDiagsInfo->uPacketsLen)-8)));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, RH: %s\n"),
              FUNCTION_NAME(), val, rh_cmd));

    /* Shim Header is declared as follows. It is SHIM_HDR_SZ(i.e. 4) bytes long.  */
    val = sal_sprintf(shim_cmd, "shim=0x40000000");
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, SH: %s\n"),
              FUNCTION_NAME(), val, shim_cmd));

    /* Use the pattern that the user has provided or the default. */
    if (pDiagsInfo->pPattern != NULL) {
      val = sal_sprintf(pay_cmd, "pat=%s", pDiagsInfo->pPattern);
    } else if (pDiagsInfo->pPayload != NULL) {
      val = sal_sprintf(pay_cmd, "pay=%s", pDiagsInfo->pPayload);
    } else if (pDiagsInfo->pNoincr != NULL) {
      val = sal_sprintf(pay_cmd, "noincr=%s", pDiagsInfo->pNoincr);
    } else {
      val = sal_sprintf(pay_cmd, "noincr=0xaa55aa55");
    }
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, PAY: %s\n"),
              FUNCTION_NAME(), val, pay_cmd));

    val = sal_sprintf(len_cmd, "len=%d", pDiagsInfo->uPacketsLen);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, LEN: %s\n"),
              FUNCTION_NAME(), val, len_cmd));

  } else {
    /* User has indicated to use the supplied Binary File to fill in the info
     * for the RouteHeader(RH), ShimHeader(SH) and the rest of the PktData
     */
#ifndef NO_SAL_APPL
    FILE* fInFilePtr = NULL;
#endif
    char* sInFileNameDef = "/root/IN_DATA.bin";
    char* sInFileName = sInFileNameDef;
#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    char* pPktBuf = NULL;
#endif
#endif
    uint32 ulPktBufLen = 0;
    char aBuf[QEDIAG_BUF_LEN];
    uint32 ulBufLen = 0;

    if (pDiagsInfo->pInFile != NULL) {
      sInFileName = pDiagsInfo->pInFile;
    }

#ifndef NO_SAL_APPL
    /* Open the User supplied file for reading. */
    if( (fInFilePtr = sal_fopen(sInFileName, "r")) == NULL ) {
      cli_out("ERROR: %s: Error opening file %s for read\n", FUNCTION_NAME(), sInFileName);
      rv = CMD_FAIL;
      goto done;
    }
#endif

    sal_memset(aBuf, 0, sizeof(char) * QEDIAG_BUF_LEN);
#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    /* Read in the 1st 12bytes of data. It contains 8Bytes of RH and 4Bytes of SH */
    ulBufLen = fread(aBuf, sizeof(char), (QEDIAG_RT_HDR_SZ + QEDIAG_SHIM_HDR_SZ), fInFilePtr);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Read %u Bytes (RH + SHIM) from %s, sizeof(char):%d\n"),
              FUNCTION_NAME(), ulBufLen, sInFileName, (int)sizeof(char)));
#endif
#else
    
#endif

    /* Read in the rest of the raw packet data */
    val = sal_sprintf(pay_cmd, "%s", "raw=");

#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    /* Point pPktBuf to where the fread() call should start filling the packet
     * bytes that are going to be read in.
     */
    pPktBuf = pay_cmd +val;

    /* Note that the Data that is read in here contains only the raw packet data.
     * It does not contain the RouteHeader or the ShimHeader.  The length field
     * in the RH therefore has to be adjusted to include the length of the raw
     * packet data(that was read in) and 4bytes of SH.
     */
    ulPktBufLen = fread(pPktBuf, sizeof(char), (QEDIAG_PKT_BUF_LEN - val), fInFilePtr);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Read %u Bytes (Payload) from %s, sizeof(char):%d\n"),
              FUNCTION_NAME(), ulPktBufLen, sInFileName, (int)sizeof(char)));
    sal_fclose(fInFilePtr);
#endif
#else
    
#endif

    /* When the packet is read in from the file, we know the total length of the packet
     * (i.e. Headers + Payload) only after we read in the whole file. We compute it and
     * set it in the global for error-checking after the test is done.
     */
    pDiagsInfo->uPacketsLen = ulBufLen + ulPktBufLen;

    /* Note that we ignore the 1st 4Bytes of data in aBuf since we will generate
     * our own queue and packet lengths here
     */
    /* The Queue Id occupies the top 14 bits of the 1st word in the ERH (i.e. 0xfffc0000)
     * The Packet Length occupies the lower 14 bits of the 1st word in the ERH (i.e. 0x00003fff)
     * The following ERH is for a 238Byte packet headed to Queue 0x10 
     * 0x004040e6 0x04000026  0x40000000
     * The TEST Bit is Bit 14 in the 1st word of ERH. Turn the test bit off to
     * see the port counts.
     */
    val = sal_sprintf(rh_cmd, "rh=0x%04x%04x%02x%02x%02x%02x", 
                    ((ulRb1Queue & 0x3fff) << 2),
                    ((ulTestBit << 14) | (ulPktBufLen +4)),
                    aBuf[4], aBuf[5], aBuf[6], aBuf[7]);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, RH: %s\n"),
              FUNCTION_NAME(), val, rh_cmd));

    /* Shim Header is declared as follows. It is SHIM_HDR_SZ(i.e. 4) bytes long.  */
    val = sal_sprintf(shim_cmd, "shim=0x%02x%02x%02x%02x", aBuf[8], aBuf[9], aBuf[10], aBuf[11]);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, SH: %s\n"),
              FUNCTION_NAME(), val, shim_cmd));

    /* Print the payload command. */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: 00, RAW: Payload in non-printable format\n"),
              FUNCTION_NAME()));
    
    val = sal_sprintf(len_cmd, "len=%d", (QEDIAG_RT_HDR_SZ + QEDIAG_SHIM_HDR_SZ + ulPktBufLen));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, LEN: %s\n"),
              FUNCTION_NAME(), val, len_cmd));

  }

  /* Update the Number of Packets and Bytes that will be transmitted. */
  pDiagsInfo->uTxPktCnt  = pDiagsInfo->uPackets;
  pDiagsInfo->uTxByteCnt = pDiagsInfo->uPacketsLen * pDiagsInfo->uPackets;

  /* Transmit the Packet */
  val = 0;
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Injecting %d packets into QE PCI Port ..\n"),
            FUNCTION_NAME(), pDiagsInfo->uPackets));
  args = sal_alloc(sizeof(args_t), "args_t");
  if (args == NULL) {
    cli_out("ERROR: %s: args_t pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  for(i = 0; i < pDiagsInfo->uPackets; i++) {
    sal_memset(args, 0x0,sizeof(args_t));
    args->a_cmd="SBX_TX";
    args->a_argc = 5;
    args->a_arg=1;
    args->a_argv[0] = "sbx_tx";
    args->a_argv[1] = rh_cmd;
    args->a_argv[2] = shim_cmd;
    args->a_argv[3] = pay_cmd;
    args->a_argv[4] = len_cmd;

    /* Make the Call to shove the packet in thru the PCI port. */
    rv = cmd_soc_sbx_tx(unit, args);

    if (rv != CMD_OK) {
      LOG_ERROR(BSL_LS_APPL_COMMON,
                (BSL_META("ERROR: sbx_tx returned %d\n"),
                 rv));
      sal_free(args);
      rv = CMD_FAIL;
      goto done;
    }
  }
  sal_free(args);

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Unblocking Grants..\n"),
            FUNCTION_NAME()));
  /* Now Run the Traffic, i.e. unblock the grants. */
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000000);

  /* The delay is in usec */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Running test for %d seconds.\n"),
            FUNCTION_NAME(), pDiagsInfo->uRunTime));
  sal_usleep(uRunTime);

  /* Stop the Traffic, i.e. apply backpressure to SPI0. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Apply BackPressure to SPI0..\n"),
            FUNCTION_NAME()));
  SAND_HAL_WRITE(qehdl, KA, EG_FORCE_FULL_0, 0x3);

#if 0
  /* Stop the Traffic, i.e. block the grants. */
  SB_LOG("Blocking Grants again..\n");
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000010);
#endif

  /***************************************************************************/
  /* Gather packets from the KA0 device */
  sbQe2000DiagsGatherPkts(pDiagsInfo, unit);
  /***************************************************************************/

  if (unit == 1) {
    pRxInfo = &RxInfo1;
  } else {
    pRxInfo = &RxInfo;
  }

#ifndef NO_SAL_APPL
  /* Close the User supplied file for Writing out the received packets. */
  if (pRxInfo->fOutFilePtr) {
    sal_fclose(pRxInfo->fOutFilePtr);
  }
#endif

  rv = CMD_OK;
 done:
  /* Free all the allocated memory */
  if (rh_cmd) {
    sal_free(rh_cmd);
  }
  if (shim_cmd) {
    sal_free(shim_cmd);
  }
  if (pay_cmd) {
    sal_free(pay_cmd);
  }
  if (len_cmd) {
    sal_free(len_cmd);
  }

  return rv;
}


int sbQe2000DiagsTrafficTestDualKA(sbxQe2000DiagsInfo_t *pDiagsInfo)
{
  uint32 unit = pDiagsInfo->unit;
  /* REUSE: Use this as a flag to indicate that the packet data is in the binary file */
  uint32 nUseFile = pDiagsInfo->uUseFile;
  uint32 ulRb1Queue = pDiagsInfo->ulRb1Queue;
  uint32 ulTestBit = 0;
  sbhandle qehdl;  /* Handle to KA0 */
  sbhandle qehdl1; /* Handle to KA1 */
  char* rh_cmd = NULL;
  char* shim_cmd = NULL;
  char* pay_cmd = NULL;
  char* len_cmd = NULL;
  /* soc_sbx_chip_info_t *chip_info = NULL; */
  int i;
  args_t *args;
  cmd_result_t val;

  /* RunTime is specified in seconds, but sal_usleep uses microsec. So, covert. */
  uint32 uRunTime = pDiagsInfo->uRunTime * 1000 * 1000;
  int rv = CMD_FAIL;

  if (unit != 0) {
    cli_out("ERROR: This test can only be run on a QE2000 Device which is unit0.\n");
    return CMD_FAIL;
  }
  if (!SOC_IS_SBX_QE2000(unit)) {
    cli_out("ERROR: unit%d needs to be a QE2000 Device for this test.\n", unit);
    return CMD_FAIL;
  }

  if (!SOC_IS_SBX_QE2000((unit +1))) {
    cli_out("ERROR: unit%d needs to be a QE2000 Device for this test.\n", (unit +1));
    return CMD_FAIL;
  }

  if (soc_property_get(unit, spn_QE_TME_MODE, 0) != 1) {
    cli_out("ERROR: unit%d(QE2000) needs to be setup to operate in TME Mode For this test.\n", unit);
    return CMD_FAIL;
  }

  /* Get the handles for each KA device */
  qehdl  = SOC_SBX_CONTROL(unit)->sbhdl;
  qehdl1 = SOC_SBX_CONTROL(unit +1)->sbhdl;

  rh_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag rh_cmd string");
  if (!rh_cmd) {
    cli_out("ERROR: %s: RouteHeader pointer allocation failed\n", FUNCTION_NAME());
    return CMD_FAIL;
  }
  sal_memset(rh_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

  shim_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag shim_cmd string");
  if (!shim_cmd) {
    cli_out("ERROR: %s: ShimHeader pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(shim_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

  pay_cmd = sal_alloc((sizeof(char) * QEDIAG_PKT_BUF_LEN), "QE Diag pay_cmd string");
  if (!pay_cmd) {
    cli_out("ERROR: %s: Payload pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(pay_cmd, 0, sizeof(char) * QEDIAG_PKT_BUF_LEN);

  len_cmd = sal_alloc((sizeof(char) * QEDIAG_BUF_LEN), "QE Diag len_cmd string");
  if (!len_cmd) {
    cli_out("ERROR: %s: Length pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  sal_memset(len_cmd, 0, sizeof(char) * QEDIAG_BUF_LEN);

/******************************************************************************************************/

  /* Setup Registers for KA0 */
  if ((rv = sbQe2000DiagsSetupRegisters(pDiagsInfo, unit)) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }

  /* Setup Registers for KA1 */
  if ((rv = sbQe2000DiagsSetupRegisters(pDiagsInfo, (unit +1))) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }

/******************************************************************************************************/

  /* The following is just a way to get access to registers by name, if needed.
   * We do not use it here.  Just leaving this code here if we need to use it
   * in the future.
   */
#if 0
  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
      cli_out("ERROR: Register info unknown for unit %d \n", unit);
      return CMD_FAIL;
  }
#endif

  /***************************************************************************/
  /* Setup the Receive Thread for KA0 */
  if ((rv = sbQe2000DiagsSetupRxThread(pDiagsInfo, unit)) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }

  /* Setup the Receive Thread for KA1 */
  if ((rv = sbQe2000DiagsSetupRxThread(pDiagsInfo, (unit +1))) != CMD_OK) {
    rv = CMD_FAIL;
    goto done;
  }
  /***************************************************************************/
  
  if (!nUseFile) {
    /* User has indicated to create the RH, SH and the Pkt Data on the fly.
     */
    /* The Queue Id occupies the top 14 bits of the 1st word in the ERH (i.e. 0xfffc0000)
     * The Packet Length occupies the lower 14 bits of the 1st word in the ERH (i.e. 0x00003fff)
     * The following ERH is for a 238Byte packet headed to Queue 0x10 
     * 0x004040e6 0x04000026  0x40000000
     * The TEST Bit is Bit 14 in the 1st word of ERH. Turn the test bit off to
     * see the port counts.
     */
    val = sal_sprintf(rh_cmd, "rh=0x%04x%04x04000026", 
                    ((ulRb1Queue & 0x3fff) << 2),
                    ((ulTestBit << 14) | ((pDiagsInfo->uPacketsLen)-8)));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, RH: %s\n"),
              FUNCTION_NAME(), val, rh_cmd));

    /* Shim Header is declared as follows. It is SHIM_HDR_SZ(i.e. 4) bytes long.  */
    val = sal_sprintf(shim_cmd, "shim=0x40000000");
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, SH: %s\n"),
              FUNCTION_NAME(), val, shim_cmd));

    /* Use the pattern that the user has provided or the default. */
    if (pDiagsInfo->pPattern != NULL) {
      val = sal_sprintf(pay_cmd, "pat=%s", pDiagsInfo->pPattern);
    } else if (pDiagsInfo->pPayload != NULL) {
      val = sal_sprintf(pay_cmd, "pay=%s", pDiagsInfo->pPayload);
    } else if (pDiagsInfo->pNoincr != NULL) {
      val = sal_sprintf(pay_cmd, "noincr=%s", pDiagsInfo->pNoincr);
    } else {
      val = sal_sprintf(pay_cmd, "noincr=0xaa55aa55");
    }
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, PAY: %s\n"),
              FUNCTION_NAME(), val, pay_cmd));

    val = sal_sprintf(len_cmd, "len=%d", pDiagsInfo->uPacketsLen);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, LEN: %s\n"),
              FUNCTION_NAME(), val, len_cmd));

  } else {
    /* User has indicated to use the supplied Binary File to fill in the info
     * for the RouteHeader(RH), ShimHeader(SH) and the rest of the PktData
     */
#ifndef NO_SAL_APPL
    FILE* fInFilePtr = NULL;
#endif
    char* sInFileNameDef = "/root/IN_DATA.bin";
    char* sInFileName = sInFileNameDef;
#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    char* pPktBuf = NULL;
#endif
#endif
    uint32 ulPktBufLen = 0;
    char aBuf[QEDIAG_BUF_LEN];
    uint32 ulBufLen = 0;

    if (pDiagsInfo->pInFile != NULL) {
      sInFileName = pDiagsInfo->pInFile;
    }

#ifndef NO_SAL_APPL
    /* Open the User supplied file for reading. */
    if( (fInFilePtr = sal_fopen(sInFileName, "r")) == NULL ) {
      cli_out("ERROR: %s: Error opening file %s for read\n", FUNCTION_NAME(), sInFileName);
      rv = CMD_FAIL;
      goto done;
    }
#endif

    sal_memset(aBuf, 0, sizeof(char) * QEDIAG_BUF_LEN);
#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    /* Read in the 1st 12bytes of data. It contains 8Bytes of RH and 4Bytes of SH */
    ulBufLen = fread(aBuf, sizeof(char), (QEDIAG_RT_HDR_SZ + QEDIAG_SHIM_HDR_SZ), fInFilePtr);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Read %u Bytes (RH + SHIM) from %s, sizeof(char):%d\n"),
              FUNCTION_NAME(), ulBufLen, sInFileName, (int)sizeof(char)));
#endif
#else
    
#endif

    /* Read in the rest of the raw packet data */
    val = sal_sprintf(pay_cmd, "%s", "raw=");

#ifndef __KERNEL__
#ifndef NO_SAL_APPL
    /* Point pPktBuf to where the fread() call should start filling the packet
     * bytes that are going to be read in.
     */
    pPktBuf = pay_cmd +val;

    /* Note that the Data that is read in here contains only the raw packet data.
     * It does not contain the RouteHeader or the ShimHeader.  The length field
     * in the RH therefore has to be adjusted to include the length of the raw
     * packet data(that was read in) and 4bytes of SH.
     */
    ulPktBufLen = fread(pPktBuf, sizeof(char), (QEDIAG_PKT_BUF_LEN - val), fInFilePtr);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Read %u Bytes (Payload) from %s, sizeof(char):%d\n"),
              FUNCTION_NAME(), ulPktBufLen, sInFileName, (int)sizeof(char)));
    sal_fclose(fInFilePtr);
#endif
#else
    
#endif

    /* When the packet is read in from the file, we know the total length of the packet
     * (i.e. Headers + Payload) only after we read in the whole file. We compute it and
     * set it in the global for error-checking after the test is done.
     */
    pDiagsInfo->uPacketsLen = ulBufLen + ulPktBufLen;

    /* Note that we ignore the 1st 4Bytes of data in aBuf since we will generate
     * our own queue and packet lengths here
     */
    /* The Queue Id occupies the top 14 bits of the 1st word in the ERH (i.e. 0xfffc0000)
     * The Packet Length occupies the lower 14 bits of the 1st word in the ERH (i.e. 0x00003fff)
     * The following ERH is for a 238Byte packet headed to Queue 0x10 
     * 0x004040e6 0x04000026  0x40000000
     * The TEST Bit is Bit 14 in the 1st word of ERH. Turn the test bit off to
     * see the port counts.
     */
    val = sal_sprintf(rh_cmd, "rh=0x%04x%04x%02x%02x%02x%02x", 
                    ((ulRb1Queue & 0x3fff) << 2),
                    ((ulTestBit << 14) | (ulPktBufLen +4)),
                    aBuf[4], aBuf[5], aBuf[6], aBuf[7]);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, RH: %s\n"),
              FUNCTION_NAME(), val, rh_cmd));

    /* Shim Header is declared as follows. It is SHIM_HDR_SZ(i.e. 4) bytes long.  */
    val = sal_sprintf(shim_cmd, "shim=0x%02x%02x%02x%02x", aBuf[8], aBuf[9], aBuf[10], aBuf[11]);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, SH: %s\n"),
              FUNCTION_NAME(), val, shim_cmd));

    /* Print the payload command. */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: 00, RAW: Payload in non-printable format\n"),
              FUNCTION_NAME()));
    
    val = sal_sprintf(len_cmd, "len=%d", (QEDIAG_RT_HDR_SZ + QEDIAG_SHIM_HDR_SZ + ulPktBufLen));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Ret: %d, LEN: %s\n"),
              FUNCTION_NAME(), val, len_cmd));
  }

  /*
   * Update the Number of Packets and Bytes that will be transmitted. Note that
   * this number is the same for the Dual KA situation, i.e. we send the same
   * number of packets to both the KA devices on the Benchscreen board.
   */
  pDiagsInfo->uTxPktCnt  = pDiagsInfo->uPackets;
  pDiagsInfo->uTxByteCnt = pDiagsInfo->uPacketsLen * pDiagsInfo->uPackets;

  /* Transmit the Packet */
  rv = CMD_FAIL;
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Injecting %d packets into PCI Port of unit%d..\n"),
            FUNCTION_NAME(), pDiagsInfo->uPackets, unit));
  args = sal_alloc(sizeof(args_t), "args_t");
  if (args == NULL) {
    cli_out("ERROR: %s: args_t pointer allocation failed\n", FUNCTION_NAME());
    rv = CMD_FAIL;
    goto done;
  }
  for(i = 0; i < pDiagsInfo->uPackets; i++) {
    sal_memset(args, 0x0,sizeof(args_t));
    args->a_cmd="SBX_TX";
    args->a_argc = 5;
    args->a_arg=1;
    args->a_argv[0] = "sbx_tx";
    args->a_argv[1] = rh_cmd;
    args->a_argv[2] = shim_cmd;
    args->a_argv[3] = pay_cmd;
    args->a_argv[4] = len_cmd;

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ###########################\n"),
              FUNCTION_NAME()));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ## Packet %d -> unit%d\n"),
              FUNCTION_NAME(), i, unit));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ###########################\n"),
              FUNCTION_NAME()));
    /* Make the Call to shove the packet in thru the PCI port of KA0. */
    rv = cmd_soc_sbx_tx(unit, args);
    if (rv != CMD_OK) {
      LOG_ERROR(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "ERROR: [%s:%d]: sbx_tx returned %d for unit%d\n"),
                 FUNCTION_NAME(), __LINE__, rv, unit));
      sal_free(args);
      rv = CMD_FAIL;
      goto done;
    }
  }

  rv = CMD_FAIL;
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Injecting %d packets into PCI Port of unit%d..\n"),
            FUNCTION_NAME(), pDiagsInfo->uPackets, (unit +1)));
  for(i = 0; i < pDiagsInfo->uPackets; i++) {
    sal_memset(args, 0x0,sizeof(args_t));
    args->a_cmd="SBX_TX";
    args->a_argc = 5;
    args->a_arg=1;
    args->a_argv[0] = "sbx_tx";
    args->a_argv[1] = rh_cmd;
    args->a_argv[2] = shim_cmd;
    args->a_argv[3] = pay_cmd;
    args->a_argv[4] = len_cmd;

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ###########################\n"),
              FUNCTION_NAME()));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ## Packet %d -> unit%d\n"),
              FUNCTION_NAME(), i, (unit +1)));
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: ###########################\n"),
              FUNCTION_NAME()));
    /* Make the Call to shove the packet in thru the PCI port of KA1. */
    rv = cmd_soc_sbx_tx((unit +1), args);
    if (rv != CMD_OK) {
      LOG_ERROR(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "ERROR: [%s:%d]: sbx_tx returned %d for unit%d\n"),
                 FUNCTION_NAME(), __LINE__, rv, (unit +1)));
      sal_free(args);
      rv = CMD_FAIL;
      goto done;
    }
  }
  sal_free(args);

  /*
   * At this point the packets are all queued up into the respective KA devices.
   * Unblock the grants, next, so that the packets start circulating
   */

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Unblocking Grants to unit%d..\n"),
            FUNCTION_NAME(), __LINE__, unit));
  /* Now Run the Traffic, i.e. unblock the grants to KA0. */
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000000);

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Unblocking Grants to unit%d..\n"),
            FUNCTION_NAME(), __LINE__, (unit +1)));
  /* Now Run the Traffic, i.e. unblock the grants to KA1. */
  SAND_HAL_WRITE(qehdl1, KA, QS_DEBUG5, 0x00000000);

  /* The delay is in usec */
  /* Note that for the Dual KA Benchscreen board, this delay is common because
   * we want this test to run for approximately the same amount of time on both
   * the KA devices.
   */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: %s: Running test for %d seconds.\n"),
            FUNCTION_NAME(), pDiagsInfo->uRunTime));
  sal_usleep(uRunTime);

  /* Stop the Traffic, i.e. apply backpressure to SPI0 of KA0. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Apply BackPressure to SPI0 of unit%d..\n"),
            FUNCTION_NAME(), __LINE__, unit));
  SAND_HAL_WRITE(qehdl, KA, EG_FORCE_FULL_0, 0x3);

  /* Stop the Traffic, i.e. apply backpressure to SPI0 of KA1. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Apply BackPressure to SPI0 of unit%d..\n"),
            FUNCTION_NAME(), __LINE__, (unit +1)));
  SAND_HAL_WRITE(qehdl1, KA, EG_FORCE_FULL_0, 0x3);

#if 0
  /* Stop the Traffic, i.e. block the grants. */
  SB_LOG("Blocking Grants again to unit%d..\n", unit);
  SAND_HAL_WRITE(qehdl, KA, QS_DEBUG5, 0x00000010);

  /* For the Dual KA Benchscreen board, block the grants to the other KA device too */
  SB_LOG("Blocking Grants again to unit%d..\n", (unit +1));
  SAND_HAL_WRITE(qehdl1, KA, QS_DEBUG5, 0x00000010);
#endif

  /***************************************************************************/
  /* Gather packets from the KA0 device */
  sbQe2000DiagsGatherPkts(pDiagsInfo, unit);

  /* Gather packets from the KA1 device */
  sbQe2000DiagsGatherPkts(pDiagsInfo, (unit +1));
  /***************************************************************************/

#ifndef NO_SAL_APPL
  /* Close the User supplied file for Writing out the received packets. */
  if (RxInfo.fOutFilePtr) {
    sal_fclose(RxInfo.fOutFilePtr);
    RxInfo.fOutFilePtr = NULL;
  }
  if (RxInfo1.fOutFilePtr) {
    sal_fclose(RxInfo1.fOutFilePtr);
    RxInfo1.fOutFilePtr = NULL;
  }
#endif

  rv = CMD_OK;
 done:
  /* Free all the allocated memory */
  if (rh_cmd) {
    sal_free(rh_cmd);
  }
  if (shim_cmd) {
    sal_free(shim_cmd);
  }
  if (pay_cmd) {
    sal_free(pay_cmd);
  }
  if (len_cmd) {
    sal_free(len_cmd);
  }

  return rv;
}


/*
 * Entry point into the Traffic Test. This should be merged into 1 routine.
 */
int sbQe2000DiagsTrafficTest(sbxQe2000DiagsInfo_t *pDiagsInfo)
{
  int rv = CMD_FAIL;
  int unit = pDiagsInfo->unit;

  if (pDiagsInfo->uForceTestFail != 0x0) {
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "DEBUG: [%s:%d]: unit%d Forcing Test Failure Exit..\n"),
              FUNCTION_NAME(), __LINE__, unit));
    if ((pDiagsInfo->uForceTestFail & 0x1) != 0x0) {
      cli_out("ERROR: unit0 Forcing Test Failure Exit..\n");
    }
    if ((pDiagsInfo->uForceTestFail & 0x2) != 0x0) {
      cli_out("ERROR: unit1 Forcing Test Failure Exit..\n");
    }
    rv = CMD_FAIL;

    return rv;
  }

  if (pDiagsInfo->uForceTestPass != 0x0) {
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "DEBUG: [%s:%d]: unit%d Forcing Test Success Exit..\n"),
              FUNCTION_NAME(), __LINE__, unit));
    cli_out("INFO: unit%d Forcing Test Success Exit..\n", unit);
    rv = CMD_OK;

    return rv;
  }

  if (pDiagsInfo->uDualKa) {
    rv = sbQe2000DiagsTrafficTestDualKA(pDiagsInfo);
  } else {
    rv = sbQe2000DiagsTrafficTestSingleKA(pDiagsInfo);
  }

  return rv;
}


int sbQe2000PrbsSetupRegisters(sbxQe2000PrbsInfo_t *pPrbsInfo)
{
  uint32 unit = pPrbsInfo->unit;
  uint32 uTmpUnit = pPrbsInfo->unit;
  int rv = CMD_OK;
  sbhandle qehdl;  /* Handle to KA0 */
  uint32 uNumKa = 0;
  uint32 uData = 0;
  int nLink = 0;
  uint32 uIsMasterLink = 0;
  uint32 uPwrupDone = 0;

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin.\n"),
            FUNCTION_NAME(), __LINE__, unit));

  /* De-assert core reset on the SI_PORTs for the SFI */
  /***********************************************************************************/
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {
    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);
    /* SAND_HAL_WRITE(qehdl, KA, PC_CORE_RESET0, 0); */
    SAND_HAL_WRITE(qehdl, KA, PC_CORE_RESET1, 0); /* all SFI out of reset */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: [%s:%d]: De-assert core reset on SI_PORTS for unit%d.\n"),
              FUNCTION_NAME(), __LINE__, uTmpUnit));
  }
  /***********************************************************************************/

  /* Power-up the Serdes ports */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {
    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Only the Master Links are Enabled
       *
       * SD   A     B     C     D
       *---------------------------
       * 0   sf0   sf2   sf4   sf6
       * 1   sf8,  sf10  sf12  sf14
       * 2   sf16  sf1   sf3   sf5
       * 3   sf7   sf9   sf11  sf13
       * 4   sf15  sf17  sc0   sc1
       *
       */
      switch (nLink) {
        case 0:
        case 8:
        case 16:
        case 7:
        case 15:
        case 4:
        case 12:
        case 3:
        case 11:
        case 18: /* sc0 */
        {
          uIsMasterLink = 0x1;
          break;
        }
        default:
        {
          uIsMasterLink = 0x0;
          break;
        }
      }

      /* Only the Master Links are Powered-Up */
      if (uIsMasterLink == 0x0) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        SAND_HAL_RMW_FIELD_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_SD_CONFIG, ENABLE, 1);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Powering-up Serdes for unit%d, SF Lane %d.\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink));
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if ( nSciLink == 0 ) {
          SAND_HAL_RMW_FIELD((sbhandle)qehdl, KA, SC_SI0_SD_CONFIG, ENABLE, 1);
        } else {
          /* coverity[dead_error_begin] */
          SAND_HAL_RMW_FIELD((sbhandle)qehdl, KA, SC_SI1_SD_CONFIG, ENABLE, 1);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Powering-up Serdes for unit%d, SC Lane %d.\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink));
      } /* else process SCI links */
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/

  /* Sleep for 1 second. This may have to be tweaked. */
  /***********************************************************************************/
  /* RunTime is specified in seconds, but sal_usleep uses microsec. So, covert. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Power-up Wait for 1 second.\n"),
            FUNCTION_NAME(), __LINE__));
  sal_usleep(1*1000*1000);
  /***********************************************************************************/

  /* Check Power-up Done Status for Serdes Ports */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Only the Master Links are Enabled
       *
       * SD   A     B     C     D
       *---------------------------
       * 0   sf0   sf2   sf4   sf6
       * 1   sf8,  sf10  sf12  sf14
       * 2   sf16  sf1   sf3   sf5
       * 3   sf7   sf9   sf11  sf13
       * 4   sf15  sf17  sc0   sc1
       *
       */
      switch (nLink) {
        case 0:
        case 8:
        case 16:
        case 7:
        case 15:
        case 4:
        case 12:
        case 3:
        case 11:
        case 18: /* sc0 */
        {
          uIsMasterLink = 0x1;
          break;
        }
        default:
        {
          uIsMasterLink = 0x0;
          break;
        }
      }

      /* Only the Master Links are checked for Power-Up Done Status */
      if (uIsMasterLink == 0x0) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_SD_STATUS);
        uPwrupDone = SAND_HAL_GET_FIELD(KA, SF0_SI_SD_STATUS, PWRUP_DONE, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking Serdes Power-up for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));

        if (uPwrupDone == 0x0) {
          cli_out("ERROR: Serdes should have been Powered-up but is'nt for unit%d, SF Lane %d. Raw: 0x%x\n", 
                  uTmpUnit, nSfiLink, uData);
          return CMD_FAIL;
        }
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if (nSciLink == 0) {
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_SD_STATUS);
            uPwrupDone = SAND_HAL_GET_FIELD(KA, SC_SI0_SD_STATUS, PWRUP_DONE, uData);
        } else {
            /* coverity[dead_error_begin : FALSE] */
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_SD_STATUS);
            uPwrupDone = SAND_HAL_GET_FIELD(KA, SC_SI1_SD_STATUS, PWRUP_DONE, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking Serdes Power-up for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));

        if (uPwrupDone == 0x0) {
          cli_out("ERROR: Serdes should have been Powered-up but is'nt for unit%d, SC Lane %d. Raw: 0x%x\n", 
                  uTmpUnit, nSciLink, uData);
          return CMD_FAIL;
        }
      }
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin. Done...\n"),
            FUNCTION_NAME(), __LINE__, unit));

  return rv;
}


/* gsrao 071510
 * Some specific links always fail for a given tuple.  This is the characteristic
 * of those specific links, for all the parts.  It does not mean that the part is
 * bad. The tuple value in such a case is just mapped to another value which we 
 * know is good for those specific links. The idea is that if those specific links
 * fail PRBS for the remapped tuple then the part is bad.
 */
int sbQe2000DiagsPrbsLinkRemapTuple(uint32 uTmpUnit, int nLink, uint32* uLoDrv, uint32* uDtx, uint32* uDeq) {
  char sBuffer[20];
  static char* saBadTuples[][5] =
    {{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
     {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {"1_8_15", ""}};
  static char* saRemapTuples[][5] =  
    {{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
     {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {"1_8_12", ""}};
  int nCount = 0;

  sal_sprintf(sBuffer, "%d_%d_%d", *uLoDrv, *uDtx, *uDeq);
    
  nCount = 0;
  while (sal_strcmp(saBadTuples[nLink][nCount], "") != 0) {
    if (sal_strcmp(saBadTuples[nLink][nCount], sBuffer) == 0) {
      /* coverity[secure_coding] */
      sscanf(saRemapTuples[nLink][nCount], "%u_%u_%u", uLoDrv, uDtx, uDeq);
#if 0
      LOG_INFO(BSL_LS_APPL_TESTS,
               (BSL_META_U(uTmpUnit,
                           "DEBUG: [%s:%d]: Remap PRBS Tuple for unit%d, SERDES Link %d. %s -> %s\n"),
                FUNCTION_NAME(), __LINE__, uTmpUnit, nLink, saBadTuples[nLink][nCount], saRemapTuples[nLink][nCount]))
#endif
      return 1;
    } 
    nCount++;
  }
  return 0; 
}


/* The general algorithm for the Kamino PRBS test is as follows...
 * For specified value of Tuple {uLoDrv, uDtx, uDeq} {
 *   Start PRBS Generator
 *     Disable MSM (Master State Machine)
 *     Select Polynomial as specified by user
 *     Enable PRBS Generator
 *   Disable PRBS Monitor
 *   Change {lodrv, dtx, deq}
 *   Enable PRBS Monitor
 *   Sleep for y seconds
 *   Check for Errors
 *   Force Single Bit Error
 *   Check for Errors
 *   if (8b/10b Testing)
 *     Enable MSM (Master State Machine)
 *       Disable PRBS Generator
 *     Check 8b10 Status
 * }
 */
int sbQe2000PrbsRunTest(sbxQe2000PrbsInfo_t *pPrbsInfo, uint32 uLoDrv, uint32 uDtx, uint32 uDeq)
{
  uint32 unit = pPrbsInfo->unit;
  uint32 uTmpUnit = pPrbsInfo->unit;
  /* int rv = CMD_OK; */
  int nMasterRv = CMD_OK;
  sbhandle qehdl;  /* Handle to KA0 */
  uint32 uNumKa = 0;
  uint32 uData = 0;
  int nLink = 0;

  uint32 uPrbsErrorOccured = 0;
  uint32 uPrbsErrCnt = 0;
  uint32 uByteAligned = 0;
  uint32 uSerdesEnabled = 0;
  uint32 uDec8b10bErr = 0;
  uint32 uRunTime = pPrbsInfo->uRunTime * 1000 * 1000;
  sal_time_t hSalTimeStart;
  sal_time_t hSalTimeNow;
  int        nSecs = 0;


  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin.\n"),
            FUNCTION_NAME(), __LINE__, unit));
  /* Now that we have uLoDrv, uDtx, uDeq, etc. Run the PRBS Test on the
   * specified Lane's
   */

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Starting PRBS Test For Tuple: %d_%d_%d\n"),
            FUNCTION_NAME(), __LINE__, uLoDrv, uDtx, uDeq));

  /* Reset for Elapsed Time Calculation */
  hSalTimeStart = sal_time();
  if (pPrbsInfo->uPrintTime) {
    cli_out("INFO: PRBS Test Start Time....................: %d sec\n", nSecs);
  }
  
  /* Start PRBS Generator
   *   Disable MSM (Master State Machine)
   *   Select Polynomial as specified by user
   *   Enable PRBS Generator
   */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {
    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_GENERATOR_EN, uData, 1);
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_POLY_SEL, uData, pPrbsInfo->uUsePrbsPoly15?1:0);
        SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Enabling PRBS Generator for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if ( nSciLink == 0 ) {
          uData = 0;
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_GENERATOR_EN, uData, 1);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_POLY_SEL, uData, pPrbsInfo->uUsePrbsPoly15?1:0);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_CONFIG0, uData);
        } else {
          uData = 0;
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_GENERATOR_EN, uData, 1);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_POLY_SEL, uData, pPrbsInfo->uUsePrbsPoly15?1:0);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_CONFIG0, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Enabling PRBS Generator for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
      } /* else process SCI links */
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  /* Elapsed Time Calculation until Start PRBS Generator Done... */
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Enable PRBS Generator Done..............: %d secs\n", nSecs);
  }
  

  /* Disable PRBS Monitor */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {
    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0);
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
        SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Disabling PRBS Monitor for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));

        /* Read to clear prbs_err_cnt, then write '0' to clear Force-Error bit. */
        uData = 0;
        SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
        SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS, uData);
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if ( nSciLink == 0 ) {
          uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_CONFIG0);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_CONFIG0, uData);

          /* Read to clear prbs_err_cnt */
          uData = 0;
          SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_PRBS_STATUS);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_PRBS_STATUS, uData);
        } else {
          uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_CONFIG0);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_CONFIG0, uData);

          /* Read to clear prbs_err_cnt */
          uData = 0;
          SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_PRBS_STATUS);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_PRBS_STATUS, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Disabling PRBS Monitor for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
      } /* else process SCI links */
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Disable PRBS Monitor Done...............: %d secs\n", nSecs);
  }

  /* Change {lodrv, dtx, deq} */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    /* Break the changing of the tuples into two distinct loops so that the SCI
     * links are brought up before the SF links.  This is due to the pairing of 
     * link-0 with link-19. In the traditional for-loop, the PRBS Tx for link-19
     * is the last to be changed. Also the PRBS Monitor for link-0 is the first
     * one to be turned on. This causes a failure on link0 because it has not
     * has sufficient time to stabilize.  We tried using sal_usleep of 10us. But
     * the sal_usleep routine is not very reliable. Hence the breaking up of the
     * loops. 
     */
    /* First Process SCI links */
    for (nLink=SB_FAB_DEVICE_QE2000_SFI_LINKS; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      uint32 uTLoDrv = uLoDrv;
      uint32 uTDtx   = uDtx;
      uint32 uTDeq   = uDeq;
      /* SCI link 0 or 1 */
      int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      /* gsrao 071510
       * Some specific links always fail for a given tuple.  This is the characteristic
       * of those specific links, for all the parts.  It does not mean that the part is
       * bad. The tuple value in such a case is just mapped to another value which we 
       * know is good for those specific links. The idea is that if those specific links
       * fail PRBS for the remapped tuple then the part is bad.
       */
      if (pPrbsInfo->uUseSweep == 0x0 && sbQe2000DiagsPrbsLinkRemapTuple(uTmpUnit, nLink, &uTLoDrv, &uTDtx, &uTDeq)) {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Remap PRBS Tuple for unit%d, SERDES Link %d. %d_%d_%d -> %d_%d_%d\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nLink, uLoDrv, uDtx, uDeq, uTLoDrv, uTDtx, uTDeq));
      }

      /* Process SCI links */
      if ( nSciLink == 0 ) {
        uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_CONFIG3);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, LODRV, uData, uTLoDrv);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DTX,   uData, uTDtx);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG3, DEQ,   uData, uTDeq);
        SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_CONFIG3, uData);
      } else {
        uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_CONFIG3);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, LODRV, uData, uTLoDrv);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DTX,   uData, uTDtx);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG3, DEQ,   uData, uTDeq);
        SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_CONFIG3, uData);
      }
      LOG_INFO(BSL_LS_APPL_TESTS,
               (BSL_META("DEBUG: [%s:%d]: Changing PRBS Tuple for unit%d, SC Lane %d. Raw: 0x%x\n"),
                FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
    } /* for (nLink=SB_FAB_DEVICE_QE2000_SFI_LINKS; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */

    /* Next Process SFI links */
    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS; nLink++) {
      uint32 uTLoDrv = uLoDrv;
      uint32 uTDtx   = uDtx;
      uint32 uTDeq   = uDeq;
      int nSfiLink = nLink;

      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      /* gsrao 071510
       * Some specific links always fail for a given tuple.  This is the characteristic
       * of those specific links, for all the parts.  It does not mean that the part is
       * bad. The tuple value in such a case is just mapped to another value which we 
       * know is good for those specific links. The idea is that if those specific links
       * fail PRBS for the remapped tuple then the part is bad.
       */
      if (pPrbsInfo->uUseSweep == 0x0 && sbQe2000DiagsPrbsLinkRemapTuple(uTmpUnit, nLink, &uTLoDrv, &uTDtx, &uTDeq)) {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Remap PRBS Tuple for unit%d, SERDES Link %d. %d_%d_%d -> %d_%d_%d\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nLink, uLoDrv, uDtx, uDeq, uTLoDrv, uTDtx, uTDeq));
      }

      /* Process SFI links */
      uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG3);
      uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, LODRV, uData, uTLoDrv);
      uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DTX,   uData, uTDtx);
      uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG3, DEQ,   uData, uTDeq);
      SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG3, uData);
      LOG_INFO(BSL_LS_APPL_TESTS,
               (BSL_META("DEBUG: [%s:%d]: Changing PRBS Tuple for unit%d, SF Lane %d. Raw: 0x%x\n"),
                FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Changing PRBS Tuple Done................: %d secs\n", nSecs);
  }

  /* Enable PRBS Monitor */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0);
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
        SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Enabling PRBS Monitor for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if ( nSciLink == 0 ) {
          uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_CONFIG0);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_CONFIG0, uData);
        } else {
          uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_CONFIG0);
          uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
          SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_CONFIG0, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Enabling PRBS Monitor for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
      }
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Enable PRBS Monitor Done................: %d secs\n", nSecs);
    cli_out("INFO: Begin Runtime...........................: %d secs\n", nSecs);
  }

  /* Sleep for uRunTime seconds */
  /***********************************************************************************/
  /* RunTime is specified in seconds, but sal_usleep uses microsec. So, covert. */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Running PRBS test for %d seconds.\n"),
            FUNCTION_NAME(), __LINE__, pPrbsInfo->uRunTime));
  sal_usleep(uRunTime);
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: End Runtime.............................: %d secs\n", nSecs);
  }

  /* Check for Errors */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
        uPrbsErrCnt = SAND_HAL_GET_FIELD(KA, SF0_SI_PRBS_STATUS, PRBS_ERR_CNT, uData);
        uPrbsErrorOccured = SAND_HAL_GET_FIELD(KA, SF0_SI_PRBS_STATUS, PRBS_ERROR_OCCURED, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking PRBS Error for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));

        if (uPrbsErrorOccured == 0x1) {
          cli_out("ERROR: Tuple: %d_%d_%d: PRBS Error for unit%d, SF Link %d, PrbsErrCnt: %d. Raw: 0x%x\n", 
                  uLoDrv, uDtx, uDeq, uTmpUnit, nSfiLink, uPrbsErrCnt, uData);
          nMasterRv = CMD_FAIL;
          if (pPrbsInfo->uExitOnError == 0x1) {
            return CMD_FAIL;
          }
        }
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if (nSciLink == 0) {
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_PRBS_STATUS);
            uPrbsErrCnt = SAND_HAL_GET_FIELD(KA, SC_SI0_PRBS_STATUS, PRBS_ERR_CNT, uData);
            uPrbsErrorOccured = SAND_HAL_GET_FIELD(KA, SC_SI0_PRBS_STATUS, PRBS_ERROR_OCCURED, uData);
        } else {
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_PRBS_STATUS);
            uPrbsErrCnt = SAND_HAL_GET_FIELD(KA, SC_SI1_PRBS_STATUS, PRBS_ERR_CNT, uData);
            uPrbsErrorOccured = SAND_HAL_GET_FIELD(KA, SC_SI1_PRBS_STATUS, PRBS_ERROR_OCCURED, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking PRBS Error for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));

        if (uPrbsErrorOccured == 0x1) {
          cli_out("ERROR: Tuple: %d_%d_%d: PRBS Error for unit%d, SC Link %d, PrbsErrCnt: %d. Raw: 0x%x\n", 
                  uLoDrv, uDtx, uDeq, uTmpUnit, nSciLink, uPrbsErrCnt, uData);
          nMasterRv = CMD_FAIL;
          if (pPrbsInfo->uExitOnError == 0x1) {
            return CMD_FAIL;
          }
        }
      }
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Checking For Errors Done................: %d secs\n", nSecs);
  }

  /* Force Single Bit Error */
  /***********************************************************************************/
  /*
   * Force a single prbs error at the prbs generator. The error is forced by inverting
   * bit 0 of the 20 bit bitstream coming out of the generator. An error is forced
   * only when the FORCE_ERROR bit transitions from 0 to 1 in the S*_SI_PRBS_STATUS
   * register.
   */
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = 0;
        uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_PRBS_STATUS, FORCE_ERROR, uData, 1);
        SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS, uData);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Forcing Single Bit Error for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));
      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if (nSciLink == 0) {
            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_PRBS_STATUS, FORCE_ERROR, uData, 1);
            SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_PRBS_STATUS, uData);
        } else {
            uData = 0;
            uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_PRBS_STATUS, FORCE_ERROR, uData, 1);
            SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_PRBS_STATUS, uData);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Forcing Single Bit Error for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
      }
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Forcing Single Bit Error Done...........: %d secs\n", nSecs);
  }

  /* Check for Errors. */
  /* In this case, it is an error if the err_reg *DID NOT* get set.
   * This is because a Single Bit Error was Forced, above.
   */
  /***********************************************************************************/
  /* In a Dual Kamino situation, we need to run the PRBS test 
   * on the specified links for both the KA's. 
   */
  for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

    /* Get the handle for the appropriate Kamino(QE) */
    qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
    uTmpUnit = (pPrbsInfo->unit +uNumKa);

    for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
      /* Run test only on the lanes that the user has specified. Skip
       * the rest.
       */
      if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
        continue;
      }

      if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
        int nSfiLink = nLink;

        uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking PRBS Error for unit%d, SF Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));

        if (uData != 0xc0000001) {
          cli_out("ERROR: Tuple: %d_%d_%d: Unexpected value for unit%d, SF Link %d, after ForceErr Enabled, Raw: 0x%x\n", 
                  uLoDrv, uDtx, uDeq, uTmpUnit, nSfiLink, uData);
          nMasterRv = CMD_FAIL;
          if (pPrbsInfo->uExitOnError == 0x1) {
            return CMD_FAIL;
          }
        }

      } else {
        /* SCI link 0 or 1 */
        int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

        if (nSciLink == 0) {
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_PRBS_STATUS);
        } else {
            uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_PRBS_STATUS);
        }
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: [%s:%d]: Checking PRBS Error for unit%d, SC Lane %d. Raw: 0x%x\n"),
                  FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));

        if (uData != 0xc0000001) {
          cli_out("ERROR: Tuple: %d_%d_%d: Unexpected value for unit%d, SC Link %d, after ForceErr Enabled, Raw: 0x%x\n", 
                  uLoDrv, uDtx, uDeq, uTmpUnit, nSciLink, uData);
          nMasterRv = CMD_FAIL;
          if (pPrbsInfo->uExitOnError == 0x1) {
            return CMD_FAIL;
          }
        }
      }
    } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
  } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
  /***********************************************************************************/
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: Starting PRBS Test For Tuple: %d_%d_%d. Done...\n"),
            FUNCTION_NAME(), __LINE__, uLoDrv, uDtx, uDeq));

  if (pPrbsInfo->uPrintTime) {
    hSalTimeNow = sal_time();
    nSecs = hSalTimeNow - hSalTimeStart;
    cli_out("INFO: Checking Force Single Bit Error Done....: %d secs\n", nSecs);
  }

  /* If the user desires 8b/10 testing, do it here. */
  if (pPrbsInfo->uDo8b10b == 0x1) {
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: [%s:%d]: Starting 8b10b Test For Tuple: %d_%d_%d.\n"),
              FUNCTION_NAME(), __LINE__, uLoDrv, uDtx, uDeq));

    /* Enable MSM */
    /***********************************************************************************/
    /* In a Dual Kamino situation, we need to run the PRBS test 
     * on the specified links for both the KA's. 
     */
    for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

      /* Get the handle for the appropriate Kamino(QE) */
      qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
      uTmpUnit = (pPrbsInfo->unit +uNumKa);

      for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
        /* Run test only on the lanes that the user has specified. Skip
         * the rest.
         */
        if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
          continue;
        }

        if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
          int nSfiLink = nLink;

          uData = 0;
          uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, ENABLE, uData, 1);
          SAND_HAL_WRITE_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
          LOG_INFO(BSL_LS_APPL_TESTS,
                   (BSL_META("DEBUG: [%s:%d]: Enabling MSM (Master State Machine) for unit%d, SF Lane %d. Raw: 0x%x\n"),
                    FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));
        } else {
          /* SCI link 0 or 1 */
          int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

          if (nSciLink == 0) {
              uData = 0;
              uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, ENABLE, uData, 1);
              SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI0_CONFIG0, uData);
          } else {
              uData = 0;
              uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, ENABLE, uData, 1);
              SAND_HAL_WRITE((sbhandle)qehdl, KA, SC_SI1_CONFIG0, uData);
          }
          LOG_INFO(BSL_LS_APPL_TESTS,
                   (BSL_META("DEBUG: [%s:%d]: Enabling MSM (Master State Machine) for unit%d, SC Lane %d. Raw: 0x%x\n"),
                    FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));
        }
      } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
    } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
    /***********************************************************************************/
    if (pPrbsInfo->uPrintTime) {
      hSalTimeNow = sal_time();
      nSecs = hSalTimeNow - hSalTimeStart;
      cli_out("INFO: Enable MSM Done.........................: %d secs\n", nSecs);
    }

    /* Sleep for 1 second to allow the Links to settle i.e. achieve Byte-Alignment. */
    /***********************************************************************************/
    /* RunTime is specified in seconds, but sal_usleep uses microsec. So, covert. */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: %s: Waiting 1 second for links to achieve Byte-Alignment.\n"),
              FUNCTION_NAME()));
    sal_usleep(1*1000*1000);
    /***********************************************************************************/
    if (pPrbsInfo->uPrintTime) {
      hSalTimeNow = sal_time();
      nSecs = hSalTimeNow - hSalTimeStart;
      cli_out("INFO: Wait for Byte Alignment Done............: %d secs\n", nSecs);
    }

    /* Check 8b10 Status */
    /***********************************************************************************/
    /* In a Dual Kamino situation, we need to run the PRBS test 
     * on the specified links for both the KA's. 
     */
    for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) {

      /* Get the handle for the appropriate Kamino(QE) */
      qehdl = SOC_SBX_CONTROL(pPrbsInfo->unit +uNumKa)->sbhdl;
      uTmpUnit = (pPrbsInfo->unit +uNumKa);

      for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
        /* Run test only on the lanes that the user has specified. Skip
         * the rest.
         */
        if (((0x1 << nLink) & pPrbsInfo->uSdLbm) != (0x1 << nLink)) {
          continue;
        }

        if ( nLink < SB_FAB_DEVICE_QE2000_SFI_LINKS ) {
          int nSfiLink = nLink;

          uData = SAND_HAL_READ_STRIDE((sbhandle)qehdl, KA, SF, nSfiLink, SF0_SI_STATUS);
          uSerdesEnabled = SAND_HAL_GET_FIELD(KA, SF0_SI_STATUS, SERDES_ENABLED, uData);
          uDec8b10bErr = SAND_HAL_GET_FIELD(KA, SF0_SI_STATUS, DEC_8B10B_ERR, uData);
          uByteAligned = SAND_HAL_GET_FIELD(KA, SF0_SI_STATUS, BYTE_ALIGNED, uData);
          LOG_INFO(BSL_LS_APPL_TESTS,
                   (BSL_META("DEBUG: [%s:%d]: Checking 8b10b Error for unit%d, SF Lane %d. Raw: 0x%x\n"),
                    FUNCTION_NAME(), __LINE__, uTmpUnit, nSfiLink, uData));

          if (uSerdesEnabled == 0x0) {
            cli_out("ERROR: Tuple: %d_%d_%d: Serdes should have been Enabled but is'nt for unit%d, SF Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSfiLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
          if (uDec8b10bErr == 0x1) {
            cli_out("ERROR: Tuple: %d_%d_%d: 8b10b Error for unit%d, SF Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSfiLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
          if (uByteAligned == 0x0) {
            cli_out("ERROR: Tuple: %d_%d_%d: Serdes should have been Byte Aligned but is'nt for unit%d, SF Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSfiLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
        } else {
          /* SCI link 0 or 1 */
          int nSciLink = nLink - SB_FAB_DEVICE_QE2000_SFI_LINKS;

          if (nSciLink == 0) {
              uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI0_STATUS);
              uSerdesEnabled = SAND_HAL_GET_FIELD(KA, SC_SI0_STATUS, SERDES_ENABLED, uData);
              uDec8b10bErr = SAND_HAL_GET_FIELD(KA, SC_SI0_STATUS, DEC_8B10B_ERR, uData);
              uByteAligned = SAND_HAL_GET_FIELD(KA, SC_SI0_STATUS, BYTE_ALIGNED, uData);
          } else {
              uData = SAND_HAL_READ((sbhandle)qehdl, KA, SC_SI1_STATUS);
              uSerdesEnabled = SAND_HAL_GET_FIELD(KA, SC_SI1_STATUS, SERDES_ENABLED, uData);
              uDec8b10bErr = SAND_HAL_GET_FIELD(KA, SC_SI1_STATUS, DEC_8B10B_ERR, uData);
              uByteAligned = SAND_HAL_GET_FIELD(KA, SC_SI1_STATUS, BYTE_ALIGNED, uData);
          }
          LOG_INFO(BSL_LS_APPL_TESTS,
                   (BSL_META("DEBUG: [%s:%d]: Checking 8b10b Error for unit%d, SC Lane %d. Raw: 0x%x\n"),
                    FUNCTION_NAME(), __LINE__, uTmpUnit, nSciLink, uData));

          if (uSerdesEnabled == 0x0) {
            cli_out("ERROR: Tuple: %d_%d_%d: Serdes should have been Enabled but is'nt for unit%d, SC Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSciLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
          if (uDec8b10bErr == 0x1) {
            cli_out("ERROR: Tuple: %d_%d_%d: 8b10b Error for unit%d, SC Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSciLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
          if (uByteAligned == 0x0) {
            cli_out("ERROR: Tuple: %d_%d_%d: Serdes should have been Byte Aligned but is'nt for unit%d, SC Lane %d. Raw: 0x%x\n",
                    uLoDrv, uDtx, uDeq, uTmpUnit, nSciLink, uData);
            nMasterRv = CMD_FAIL;
            if (pPrbsInfo->uExitOnError == 0x1) {
              return CMD_FAIL;
            }
          }
        }
      } /* for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) */
    } /* for (uNumKa = 0; uNumKa <= pPrbsInfo->uDualKa; uNumKa++) */
    /***********************************************************************************/

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: [%s:%d]: Starting 8b10b Test For Tuple: %d_%d_%d. Done...\n"),
              FUNCTION_NAME(), __LINE__, uLoDrv, uDtx, uDeq));
    if (pPrbsInfo->uPrintTime) {
      hSalTimeNow = sal_time();
      nSecs = hSalTimeNow - hSalTimeStart;
      cli_out("INFO: Check 8b/10b Errors Done................: %d secs\n", nSecs);
    }

  } /* if (pPrbsInfo->uDo8b10b == 0x1) */
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin. Done...\n"),
            FUNCTION_NAME(), __LINE__, unit));

  return nMasterRv;
}


/*
 * Entry point into the Qe2000 PRBS Test.
 */
int sbQe2000DiagsPrbsTest(sbxQe2000PrbsInfo_t *pPrbsInfo)
{
  uint32 unit = pPrbsInfo->unit;
  int rv = CMD_OK;
  int nMasterRv = CMD_OK;
  /* sbhandle qehdl;  */ /* Handle to KA0 */
  /* sbhandle qehdl1; */ /* Handle to KA1 */
  char *tokstr=NULL;

  if (pPrbsInfo->uForceTestFail != 0x0) {
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: [%s:%d]: unit%d Forcing Test Failure Exit..\n"),
              FUNCTION_NAME(), __LINE__, unit));
    if ((pPrbsInfo->uForceTestFail & 0x1) != 0x0) {
      cli_out("ERROR: unit0 Forcing Test Failure Exit..\n");
    }
    if ((pPrbsInfo->uForceTestFail & 0x2) != 0x0) {
      cli_out("ERROR: unit1 Forcing Test Failure Exit..\n");
    }
    rv = CMD_FAIL;

    return rv;
  }

  if (pPrbsInfo->uForceTestPass != 0x0) {
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("DEBUG: [%s:%d]: unit%d Forcing Test Success Exit..\n"),
              FUNCTION_NAME(), __LINE__, unit));
    cli_out("INFO: unit%d Forcing Test Success Exit..\n", unit);
    rv = CMD_OK;

    return rv;
  }

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin.\n"),
            FUNCTION_NAME(), __LINE__, unit));

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]:                             \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * Port-SERDES mapping       \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: *---------------------------\n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * SD   A     B     C     D  \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: *---------------------------\n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * 0   sf0   sf2   sf4   sf6 \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * 1   sf8,  sf10  sf12  sf14\n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * 2   sf16  sf1   sf3   sf5 \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * 3   sf7   sf9   sf11  sf13\n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: * 4   sf15  sf17  sc0   sc1 \n"),
            FUNCTION_NAME(), __LINE__));
  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]:                             \n"),
            FUNCTION_NAME(), __LINE__));

  /* Get the handles for each KA device
   * qehdl  = SOC_SBX_CONTROL(unit)->sbhdl;
   * if (pPrbssInfo->uDualKa) {
   *   qehdl1 = SOC_SBX_CONTROL(unit +1)->sbhdl;
   * }
   */

  rv = sbQe2000PrbsSetupRegisters(pPrbsInfo);
  if (rv == CMD_FAIL) {
    return rv;
  }

  /* Parse and extract the Tuple list, i.e. if the user has provided one. */
  /* This is the general algorithm that will be followed.
   * foreach (list{lodrv, dtx, deq}) {
   *   Start PRBS Generator
   *     Disable MSM (Master State Machine)
   *     Select Polynomial as specified by user
   *     Enable PRBS Generator
   *   Disable PRBS Monitor
   *   Change {lodrv, dtx, deq}
   *   Enable PRBS Monitor
   *   Sleep for y seconds
   *   Check for Errors
   *   Force Single Bit Error
   *   Check for Errors
   *   if (8b/10b Testing)
   *     Enable MSM (Master State Machine)
   *       Disable PRBS Generator
   *     Check 8b10 Status
   * }
   */
  if (pPrbsInfo->uUseSweep == 0x0) {
    if (pPrbsInfo->pTupleList) {
      char* token;
      char delimiters[] = ",;-.";

      /* The token will be of the form "x_y_x" */
      token = sal_strtok_r(pPrbsInfo->pTupleList, delimiters, &tokstr);
      if (token == NULL) {
	return CMD_FAIL;
      }
      do {
        uint32 uLoDrv = 0;
        uint32 uDtx = 0;
        uint32 uDeq = 0;

        /* coverity[secure_coding] */
        sscanf(token, "%u_%u_%u", &uLoDrv, &uDtx, &uDeq);

        if (uLoDrv > 1) {
          cli_out("ERROR: uLoDrv can only have values in range [0,1]: Got %d in %s \n", uLoDrv, token);
          return CMD_FAIL;
        }
        if (uDtx > 15) {
          cli_out("ERROR: uDtx can only have values in range [0,15]: Got %d in %s \n", uDtx, token);
          return CMD_FAIL;
        }
        if (uDeq > 15) {
          cli_out("ERROR: uDeq can only have values in range [0,15]: Got %d in %s \n", uDeq, token);
          return CMD_FAIL;
        }

        /* Call the actual test with the uLoDrv, uDtx and uDeq Values */
        rv = sbQe2000PrbsRunTest(pPrbsInfo, uLoDrv, uDtx, uDeq);
        nMasterRv = rv;
        if ((rv == CMD_FAIL) && (pPrbsInfo->uExitOnError == 0x1)) {
          return rv;
        }

      } while ((token = sal_strtok_r(NULL, delimiters, &tokstr)));
    } /* if (pPrbsInfo->pTupleList) */
  } /* if (pPrbsInfo->uUseSweep == 0x0) */ else {
    /* else */
    uint32 uLoDrvCnt = 0;
    uint32 uDtxCnt = 0;
    uint32 uDeqCnt = 0;

    if (pPrbsInfo->uLoDrvLo > 1) {
      cli_out("ERROR: uLoDrvLo can only have values in range [0,1]: Got %d\n", pPrbsInfo->uLoDrvLo);
      return CMD_FAIL;
    }
    if (pPrbsInfo->uLoDrvHi > 1) {
      cli_out("ERROR: uLoDrvHi can only have values in range [0,1]: Got %d\n", pPrbsInfo->uLoDrvHi);
      return CMD_FAIL;
    }

    if (pPrbsInfo->uDtxLo > 15) {
      cli_out("ERROR: uDtxLo can only have values in range [0,15]: Got %d\n", pPrbsInfo->uDtxLo);
      return CMD_FAIL;
    }
    if (pPrbsInfo->uDtxHi > 15) {
      cli_out("ERROR: uDtxHi can only have values in range [0,15]: Got %d\n", pPrbsInfo->uDtxHi);
      return CMD_FAIL;
    }

    if (pPrbsInfo->uDeqLo > 15) {
      cli_out("ERROR: uDeqLo can only have values in range [0,15]: Got %d\n", pPrbsInfo->uDeqLo);
      return CMD_FAIL;
    }
    if (pPrbsInfo->uDeqHi > 15) {
      cli_out("ERROR: uDeqHi can only have values in range [0,15]: Got %d\n", pPrbsInfo->uDeqHi);
      return CMD_FAIL;
    }

    for (uLoDrvCnt = pPrbsInfo->uLoDrvLo; uLoDrvCnt <= pPrbsInfo->uLoDrvHi; uLoDrvCnt++) {
      for (uDtxCnt = pPrbsInfo->uDtxLo; uDtxCnt <= pPrbsInfo->uDtxHi; uDtxCnt++) {
        for (uDeqCnt = pPrbsInfo->uDeqLo; uDeqCnt <= pPrbsInfo->uDeqHi; uDeqCnt++) {

          /* Call the actual test with the uLoDrv, uDtx and uDeq Values */
          rv = sbQe2000PrbsRunTest(pPrbsInfo, uLoDrvCnt, uDtxCnt, uDeqCnt);
          nMasterRv = rv;
          if ((rv == CMD_FAIL) && (pPrbsInfo->uExitOnError == 0x1)) {
            return rv;
          }
        }
      }
    }
  }

  LOG_INFO(BSL_LS_APPL_TESTS,
           (BSL_META("DEBUG: [%s:%d]: unit%d Begin. Done...\n"),
            FUNCTION_NAME(), __LINE__, unit));

  return nMasterRv;
}
#else
int appl_test_qe2000_diags_not_empty;
#endif /* (BCM_SBX_SUPPORT) */
