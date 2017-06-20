/*
 * $Id: c3hppc_etu.c,v 1.37 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_etu.c
 * Purpose: Caladan3 ETU test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>
#include <appl/diag/shell.h>
#include <soc/cmic.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_etu.h>

static int g_anErrorRegisters[] = { ETU_WRAP_INTERRUPTr,
                                    ETU_WRAP_TRACE_INTERRUPTr
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { ETU_WRAP_INTERRUPT_MASKr,
                                        ETU_WRAP_TRACE_INTERRUPT_MASKr
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);

static int g_anEtErrorRegisters[] = { ILAMAC_RX_INTF_INTR0_STSr,
                                      ILAMAC_RX_INTF_INTR1_STSr,
                                      ILAMAC_RX_LANE_INTR0_STSr,
                                      ILAMAC_RX_LANE_INTR1_STSr,
                                      ILAMAC_RX_LANE_INTR2_STSr,
                                      ILAMAC_RX_LANE_INTR3_STSr,
                                      ILAMAC_TX_INTR_STSr,
                                      ETU_CP_FIFO_INTR_STSr,
                                      ETU_TX_REQ_FIFO_INTR_STSr,
                                      ETU_TX_PIPE_CTL_FIFO_INTR_STSr,
                                      ETU_RX_RSP_FIFO_INTR_STSr,
                                      WCL_INTR_STSr,
                                      ETU_GLOBAL_INTR_STSr,
                                      ILAMAC_TX_INTF_STATEr,
                                      ILAMAC_RX_INTF_STATE0r,
                                      ILAMAC_RX_INTF_STATE1r,
                                      ILAMAC_RX_WORD_SYNC_ERRORS_COUNTr,
                                      ILAMAC_RX_FRAMING_ERRORS_COUNTr,
                                      ILAMAC_RX_BAD_TYPE_ERRORS_COUNTr,
                                      ILAMAC_RX_DESCRAM_ERRORS_COUNTr,
                                      ILAMAC_RX_ALIGNMENT_ERRORS_COUNTr,
                                      ILAMAC_RX_ALIGNMENT_FAILURES_ERRORS_COUNTr,
                                      ILAMAC_RX_CRC_ERRORS_COUNTr,
                                      ILAMAC_RX_BURSTMAX_ERRORS_COUNTr,
                                      ILAMAC_RX_LANE_CRC_ERRORS_COUNTr,
                                      ILAMAC_RX_BAD_PACKETS_COUNTr,
                                      ILAMAC_DEBUG_COUNTr,
                                      ILAMAC_TX_PACKETS_COUNTr,
                                      ILAMAC_RX_PACKETS_COUNTr,
                                      WCL_RX_LOST_LOCK_COUNTr,
                                      ETU_RX_CW_ERR_INFO1r,
                                      ETU_RX_CW_ERR_CHANOUTr,
                                      ETU_RX_CW_ERR_DW0_HIr,
                                      ETU_RX_CW_ERR_DW1_LOr,
                                      ETU_RX_CW_ERR_DW1_HIr,
                                      ETU_RX_ERS0_CHANOUTr,
                                      ETU_RX_ERS0_DW0_LOr,
                                      ETU_RX_ERS0_DW0_HIr,
                                      ETU_RX_ERS0_DW1_LOr,
                                      ETU_RX_ERS0_DW1_HIr,
                                      ETU_CP_FIFO_STSr,
                                      ETU_CP_FIFO_SBE_STSr,
                                      ETU_CP_FIFO_DBE_STSr,
                                      ETU_DBG_IPIPE_REQ_RSP_COUNTr,
                                      ETU_DBG_IPIPE_ERR_RSP_COUNTr
                                    };
static int g_nEtErrorRegistersCount = COUNTOF(g_anEtErrorRegisters);

static int g_anEtErrorMaskRegisters[] = { ILAMAC_RX_INTF_INTR0_ENABLEr,
                                          ILAMAC_RX_INTF_INTR1_ENABLEr,
                                          ILAMAC_RX_LANE_INTR0_ENABLEr,
                                          ILAMAC_RX_LANE_INTR1_ENABLEr,
                                          ILAMAC_RX_LANE_INTR2_ENABLEr,
                                          ILAMAC_RX_LANE_INTR3_ENABLEr,
                                          ILAMAC_TX_INTR_ENABLEr,
                                          ETU_CP_FIFO_INTR_ENABLEr,
                                          ETU_TX_REQ_FIFO_INTR_ENABLEr,
                                          ETU_TX_PIPE_CTL_FIFO_INTR_ENABLEr,
                                          ETU_RX_RSP_FIFO_INTR_ENABLEr,
                                          WCL_INTR_ENABLEr,
                                          ETU_GLOBAL_INTR_ENABLEr
                                        };
static int g_nEtErrorMaskRegistersCount = COUNTOF(g_anEtErrorMaskRegisters);

static int g_anEtErrorClearRegisters[] = { ILAMAC_RX_INTF_INTR0_CLEARr,
                                           ILAMAC_RX_INTF_INTR1_CLEARr,
                                           ILAMAC_RX_LANE_INTR0_CLEARr,
                                           ILAMAC_RX_LANE_INTR1_CLEARr,
                                           ILAMAC_RX_LANE_INTR2_CLEARr,
                                           ILAMAC_RX_LANE_INTR3_CLEARr,
                                           ILAMAC_TX_INTR_CLEARr,
                                           ETU_CP_FIFO_INTR_CLRr,
                                           ETU_TX_REQ_FIFO_INTR_CLRr,
                                           ETU_TX_PIPE_CTL_FIFO_INTR_CLRr,
                                           ETU_RX_RSP_FIFO_INTR_CLRr,
                                           WCL_INTR_CLEARr,
                                           ETU_GLOBAL_INTR_CLEARr,
                                           ILAMAC_RX_WORD_SYNC_ERRORS_COUNTr,
                                           ILAMAC_RX_FRAMING_ERRORS_COUNTr,
                                           ILAMAC_RX_BAD_TYPE_ERRORS_COUNTr,
                                           ILAMAC_RX_DESCRAM_ERRORS_COUNTr,
                                           ILAMAC_RX_ALIGNMENT_ERRORS_COUNTr,
                                           ILAMAC_RX_ALIGNMENT_FAILURES_ERRORS_COUNTr,
                                           ILAMAC_RX_CRC_ERRORS_COUNTr,
                                           ILAMAC_RX_BURSTMAX_ERRORS_COUNTr,
                                           ILAMAC_RX_LANE_CRC_ERRORS_COUNTr,
                                           ILAMAC_RX_BAD_PACKETS_COUNTr,
                                           ILAMAC_DEBUG_COUNTr,
                                           WCL_RX_LOST_LOCK_COUNTr,
                                           ETU_RX_CW_ERR_INFO1r,
                                           ETU_RX_CW_ERR_CHANOUTr,
                                           ETU_RX_CW_ERR_DW0_HIr,
                                           ETU_RX_CW_ERR_DW1_LOr,
                                           ETU_RX_CW_ERR_DW1_HIr,
                                           ETU_RX_ERS0_CHANOUTr,
                                           ETU_RX_ERS0_DW0_LOr,
                                           ETU_RX_ERS0_DW0_HIr,
                                           ETU_RX_ERS0_DW1_LOr,
                                           ETU_RX_ERS0_DW1_HIr,
                                           ETU_CP_FIFO_SBE_STSr,
                                           ETU_CP_FIFO_DBE_STSr
                                         };
static int g_nEtErrorClearRegistersCount = COUNTOF(g_anEtErrorClearRegisters);

static sal_thread_t g_EtuUpdateCmdManagerThreadID = NULL;
static c3hppc_etu_update_manager_cb_t g_c3hppcEtuUpdateManagerCB;

static int g_anESM_PhyIDs[] = { 0xe1, 0xe5, 0xe9, 0xed, 0xf1, 0xf5 };
static int g_nNL11K_PhyID = 0x60;

static uint16 g_NL11K_ErrorStatusRegisterStoredValue[C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS];

#define C3HPPC_ETU__RX_PCS_UI_VIOLATION_ERR_15to0_INDEX 19

static char g_NL11K_ErrorStatusRegisterNames[C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS][48] =
                                                 { "COMMON_STATUS",
                                                   "GENERAL_PURPOSE_STATUS0",
                                                   "GENERAL_PURPOSE_STATUS1",
                                                   "GENERAL_PURPOSE_STATUS2",
                                                   "GENERAL_PURPOSE_STATUS3",
                                                   "RX_IDLE_CRC24_ERROR0",
                                                   "RX_IDLE_CRC24_ERROR1",
                                                   "RX_INTERFACE_ERROR0",
                                                   "RX_INTERFACE_ERROR1",
                                                   "RX_NMAC_CSM_FIFO_FULL_CRX_PORT",
                                                   "RX_NMAC_CSM_FIFO_FULL_UPPER",
                                                   "RX_NMAC_CSM_FIFO_FULL_LOWER",
                                                   "RX_NMAC_CSM_PKT_RECV_31to16",
                                                   "RX_NMAC_CSM_PKT_RECV_15to0",
                                                   "RX_NMAC_CSM_ERR_PKT_31to16",
                                                   "RX_NMAC_CSM_ERR_PKT_15to0",
                                                   "RX_NMAC_CSM_CRC_ERR_31to16",
                                                   "RX_NMAC_CSM_CRC_ERR_15to0",
                                                   "RX_PCS_UI_VIOLATION_ERR_31to16",
                                                   "RX_PCS_UI_VIOLATION_ERR_15to0",
                                                   "RX_NMAC_CSM_CREQ_PKT_SENT_31to16",
                                                   "RX_NMAC_CSM_CREQ_PKT_SENT_15to0",
                                                   "RX_NMAC_CSM_CRES_PKT_RECVD_31to16",
                                                   "RX_NMAC_CSM_CRES_PKT_RECVD_15to0",
                                                   "RX_NMAC_CSM_CRES_ERR_PKT_RECVD_31to16",
                                                   "RX_NMAC_CSM_CRES_ERR_PKT_RECVD_15to0",
                                                   "RX_NMAC_CSM_CRES_CRC_ERR_PKT_31to16",
                                                   "RX_NMAC_CSM_CRES_CRC_ERR_PKT_15to0",
                                                   "RX_PCS_STATUS__LANES_3to0",
                                                   "RX_PCS_STATUS__LANES_7to4",
                                                   "RX_PCS_STATUS__LANES_11to8",
                                                   "RX_PCS_STATUS__LANES_15to12",
                                                   "RX_PCS_STATUS__LANES_19to16",
                                                   "RX_PCS_STATUS__LANES_23to20",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_3to0",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_7to4",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_11to8",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_15to12",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_19to16",
                                                   "RX_PCS_WORD_ALIGN_STATUS__LANES_23to20",
                                                   "RX_PCS_WORD_ERROR__LANES_3to0",
                                                   "RX_PCS_WORD_ERROR__LANES_7to4",
                                                   "RX_PCS_WORD_ERROR__LANES_11to8",
                                                   "RX_PCS_WORD_ERROR__LANES_15to12",
                                                   "RX_PCS_WORD_ERROR__LANES_19to16",
                                                   "RX_PCS_WORD_ERROR__LANES_23to20",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_3to0",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_7to4",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_11to8",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_15to12",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_19to16",
                                                   "RX_PCS_BLOCK_TYPE_ERROR__LANES_23to20",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_3to0",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_7to4",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_11to8",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_15to12",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_19to16",
                                                   "RX_PCS_METAFRAME_ERROR__LANES_23to20",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_3to0",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_7to4",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_11to8",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_15to12",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_19to16",
                                                   "RX_PCS_DSCR_SYNC_LOSS_ERROR__LANES_23to20",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_3to0",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_7to4",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_11to8",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_15to12",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_19to16",
                                                   "RX_PCS_DSCR_SINGLE_ERROR__LANES_23to20",
                                                   "RX_PCS_eFIFO_ERROR__LANES_3to0",
                                                   "RX_PCS_eFIFO_ERROR__LANES_7to4",
                                                   "RX_PCS_eFIFO_ERROR__LANES_11to8",
                                                   "RX_PCS_eFIFO_ERROR__LANES_15to12",
                                                   "RX_PCS_eFIFO_ERROR__LANES_19to16",
                                                   "RX_PCS_eFIFO_ERROR__LANES_23to20",
                                                   "RX_PCS_CRC32_ERROR__LANES_3to0",
                                                   "RX_PCS_CRC32_ERROR__LANES_7to4",
                                                   "RX_PCS_CRC32_ERROR__LANES_11to8",
                                                   "RX_PCS_CRC32_ERROR__LANES_15to12",
                                                   "RX_PCS_CRC32_ERROR__LANES_19to16",
                                                   "RX_PCS_CRC32_ERROR__LANES_23to20",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_3to0",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_7to4",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_11to8",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_15to12",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_19to16",
                                                   "RX_PCS_0_ALIGN_FAILED_31to16__LANES_23to20",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_3to0",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_7to4",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_11to8",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_15to12",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_19to16",
                                                   "RX_PCS_0_ALIGN_FAILED_15to0__LANES_23to20",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_3to0",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_7to4",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_11to8",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_15to12",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_19to16",
                                                   "RX_PCS_1_ALIGN_FAILED_31to16__LANES_23to20",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_3to0",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_7to4",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_11to8",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_15to12",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_19to16",
                                                   "RX_PCS_1_ALIGN_FAILED_15to0__LANES_23to20",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_3to0",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_7to4",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_11to8",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_15to12",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_19to16",
                                                   "RX_PCS_2_ALIGN_FAILED_31to16__LANES_23to20",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_3to0",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_7to4",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_11to8",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_15to12",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_19to16",
                                                   "RX_PCS_2_ALIGN_FAILED_15to0__LANES_23to20",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_3to0",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_7to4",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_11to8",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_15to12",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_19to16",
                                                   "RX_PCS_3_ALIGN_FAILED_31to16__LANES_23to20",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_3to0",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_7to4",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_11to8",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_15to12",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_19to16",
                                                   "RX_PCS_3_ALIGN_FAILED_15to0__LANES_23to20",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_0_WORD_SYNC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_0_WORD_SYNC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_1_WORD_SYNC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_1_WORD_SYNC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_2_WORD_SYNC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_2_WORD_SYNC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_3_WORD_SYNC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_3_WORD_SYNC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_0_CDR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_0_CDR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_1_CDR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_1_CDR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_2_CDR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_2_CDR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_3_CDR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_3_CDR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_0_BAD_CNTR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_0_BAD_CNTR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_1_BAD_CNTR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_1_BAD_CNTR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_2_BAD_CNTR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_2_BAD_CNTR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_3_BAD_CNTR_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_3_BAD_CNTR_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_0_CRC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_0_CRC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_1_CRC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_1_CRC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_2_CRC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_2_CRC_ERR_15to0__LANES_23to20",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_3to0",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_7to4",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_11to8",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_15to12",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_19to16",
                                                   "RX_PCS_3_CRC_ERR_31to16__LANES_23to20",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_3to0",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_7to4",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_11to8",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_15to12",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_19to16",
                                                   "RX_PCS_3_CRC_ERR_15to0__LANES_23to20" };
static uint16 g_NL11K_ErrorStatusRegisters[C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS][2] =
                                                 { {1,0x8180},
                                                   {1,0x8181},
                                                   {1,0x8182},
                                                   {1,0x8183},
                                                   {1,0x8184},
                                                   {1,0x8186},
                                                   {1,0x8187},
                                                   {1,0x8189},
                                                   {1,0x818a},
                                                   {1,0x8203},
                                                   {1,0x8204},
                                                   {1,0x8205},
                                                   {1,0x8280},
                                                   {1,0x8281},
                                                   {1,0x8284},
                                                   {1,0x8285},
                                                   {1,0x8288},
                                                   {1,0x8289},
                                                   {1,0x828a},
                                                   {1,0x828b},
                                                   {1,0x828c},
                                                   {1,0x828d},
                                                   {1,0x828e},
                                                   {1,0x828f},
                                                   {1,0x8290},
                                                   {1,0x8291},
                                                   {1,0x8292},
                                                   {1,0x8293},
                                                   {2,0x8300},
                                                   {3,0x8300},
                                                   {4,0x8300},
                                                   {5,0x8300},
                                                   {6,0x8300},
                                                   {7,0x8300},
                                                   {2,0x8301},
                                                   {3,0x8301},
                                                   {4,0x8301},
                                                   {5,0x8301},
                                                   {6,0x8301},
                                                   {7,0x8301},
                                                   {2,0x8302},
                                                   {3,0x8302},
                                                   {4,0x8302},
                                                   {5,0x8302},
                                                   {6,0x8302},
                                                   {7,0x8302},
                                                   {2,0x8303},
                                                   {3,0x8303},
                                                   {4,0x8303},
                                                   {5,0x8303},
                                                   {6,0x8303},
                                                   {7,0x8303},
                                                   {2,0x8304},
                                                   {3,0x8304},
                                                   {4,0x8304},
                                                   {5,0x8304},
                                                   {6,0x8304},
                                                   {7,0x8304},
                                                   {2,0x8305},
                                                   {3,0x8305},
                                                   {4,0x8305},
                                                   {5,0x8305},
                                                   {6,0x8305},
                                                   {7,0x8305},
                                                   {2,0x8306},
                                                   {3,0x8306},
                                                   {4,0x8306},
                                                   {5,0x8306},
                                                   {6,0x8306},
                                                   {7,0x8306},
                                                   {2,0x8307},
                                                   {3,0x8307},
                                                   {4,0x8307},
                                                   {5,0x8307},
                                                   {6,0x8307},
                                                   {7,0x8307},
                                                   {2,0x8308},
                                                   {3,0x8308},
                                                   {4,0x8308},
                                                   {5,0x8308},
                                                   {6,0x8308},
                                                   {7,0x8308},
                                                   {2,0x8380},
                                                   {3,0x8380},
                                                   {4,0x8380},
                                                   {5,0x8380},
                                                   {6,0x8380},
                                                   {7,0x8380},
                                                   {2,0x8381},
                                                   {3,0x8381},
                                                   {4,0x8381},
                                                   {5,0x8381},
                                                   {6,0x8381},
                                                   {7,0x8381},
                                                   {2,0x8382},
                                                   {3,0x8382},
                                                   {4,0x8382},
                                                   {5,0x8382},
                                                   {6,0x8382},
                                                   {7,0x8382},
                                                   {2,0x8383},
                                                   {3,0x8383},
                                                   {4,0x8383},
                                                   {5,0x8383},
                                                   {6,0x8383},
                                                   {7,0x8383},
                                                   {2,0x8384},
                                                   {3,0x8384},
                                                   {4,0x8384},
                                                   {5,0x8384},
                                                   {6,0x8384},
                                                   {7,0x8384},
                                                   {2,0x8385},
                                                   {3,0x8385},
                                                   {4,0x8385},
                                                   {5,0x8385},
                                                   {6,0x8385},
                                                   {7,0x8385},
                                                   {2,0x8386},
                                                   {3,0x8386},
                                                   {4,0x8386},
                                                   {5,0x8386},
                                                   {6,0x8386},
                                                   {7,0x8386},
                                                   {2,0x8387},
                                                   {3,0x8387},
                                                   {4,0x8387},
                                                   {5,0x8387},
                                                   {6,0x8387},
                                                   {7,0x8387},
                                                   {2,0x8388},
                                                   {3,0x8388},
                                                   {4,0x8388},
                                                   {5,0x8388},
                                                   {6,0x8388},
                                                   {7,0x8388},
                                                   {2,0x8389},
                                                   {3,0x8389},
                                                   {4,0x8389},
                                                   {5,0x8389},
                                                   {6,0x8389},
                                                   {7,0x8389},
                                                   {2,0x838a},
                                                   {3,0x838a},
                                                   {4,0x838a},
                                                   {5,0x838a},
                                                   {6,0x838a},
                                                   {7,0x838a},
                                                   {2,0x838b},
                                                   {3,0x838b},
                                                   {4,0x838b},
                                                   {5,0x838b},
                                                   {6,0x838b},
                                                   {7,0x838b},
                                                   {2,0x838c},
                                                   {3,0x838c},
                                                   {4,0x838c},
                                                   {5,0x838c},
                                                   {6,0x838c},
                                                   {7,0x838c},
                                                   {2,0x838d},
                                                   {3,0x838d},
                                                   {4,0x838d},
                                                   {5,0x838d},
                                                   {6,0x838d},
                                                   {7,0x838d},
                                                   {2,0x838e},
                                                   {3,0x838e},
                                                   {4,0x838e},
                                                   {5,0x838e},
                                                   {6,0x838e},
                                                   {7,0x838e},
                                                   {2,0x838f},
                                                   {3,0x838f},
                                                   {4,0x838f},
                                                   {5,0x838f},
                                                   {6,0x838f},
                                                   {7,0x838f},
                                                   {2,0x8390},
                                                   {3,0x8390},
                                                   {4,0x8390},
                                                   {5,0x8390},
                                                   {6,0x8390},
                                                   {7,0x8390},
                                                   {2,0x8391},
                                                   {3,0x8391},
                                                   {4,0x8391},
                                                   {5,0x8391},
                                                   {6,0x8391},
                                                   {7,0x8391},
                                                   {2,0x8392},
                                                   {3,0x8392},
                                                   {4,0x8392},
                                                   {5,0x8392},
                                                   {6,0x8392},
                                                   {7,0x8392},
                                                   {2,0x8393},
                                                   {3,0x8393},
                                                   {4,0x8393},
                                                   {5,0x8393},
                                                   {6,0x8393},
                                                   {7,0x8393},
                                                   {2,0x8394},
                                                   {3,0x8394},
                                                   {4,0x8394},
                                                   {5,0x8394},
                                                   {6,0x8394},
                                                   {7,0x8394},
                                                   {2,0x8395},
                                                   {3,0x8395},
                                                   {4,0x8395},
                                                   {5,0x8395},
                                                   {6,0x8395},
                                                   {7,0x8395},
                                                   {2,0x8396},
                                                   {3,0x8396},
                                                   {4,0x8396},
                                                   {5,0x8396},
                                                   {6,0x8396},
                                                   {7,0x8396},
                                                   {2,0x8397},
                                                   {3,0x8397},
                                                   {4,0x8397},
                                                   {5,0x8397},
                                                   {6,0x8397},
                                                   {7,0x8397},
                                                   {2,0x8398},
                                                   {3,0x8398},
                                                   {4,0x8398},
                                                   {5,0x8398},
                                                   {6,0x8398},
                                                   {7,0x8398},
                                                   {2,0x8399},
                                                   {3,0x8399},
                                                   {4,0x8399},
                                                   {5,0x8399},
                                                   {6,0x8399},
                                                   {7,0x8399},
                                                   {2,0x839a},
                                                   {3,0x839a},
                                                   {4,0x839a},
                                                   {5,0x839a},
                                                   {6,0x839a},
                                                   {7,0x839a},
                                                   {2,0x839b},
                                                   {3,0x839b},
                                                   {4,0x839b},
                                                   {5,0x839b},
                                                   {6,0x839b},
                                                   {7,0x839b},
                                                   {2,0x839c},
                                                   {3,0x839c},
                                                   {4,0x839c},
                                                   {5,0x839c},
                                                   {6,0x839c},
                                                   {7,0x839c},
                                                   {2,0x839d},
                                                   {3,0x839d},
                                                   {4,0x839d},
                                                   {5,0x839d},
                                                   {6,0x839d},
                                                   {7,0x839d},
                                                   {2,0x839e},
                                                   {3,0x839e},
                                                   {4,0x839e},
                                                   {5,0x839e},
                                                   {6,0x839e},
                                                   {7,0x839e},
                                                   {2,0x839f},
                                                   {3,0x839f},
                                                   {4,0x839f},
                                                   {5,0x839f},
                                                   {6,0x839f},
                                                   {7,0x839f},
                                                   {2,0x83a0},
                                                   {3,0x83a0},
                                                   {4,0x83a0},
                                                   {5,0x83a0},
                                                   {6,0x83a0},
                                                   {7,0x83a0},
                                                   {2,0x83a1},
                                                   {3,0x83a1},
                                                   {4,0x83a1},
                                                   {5,0x83a1},
                                                   {6,0x83a1},
                                                   {7,0x83a1},
                                                   {2,0x83a2},
                                                   {3,0x83a2},
                                                   {4,0x83a2},
                                                   {5,0x83a2},
                                                   {6,0x83a2},
                                                   {7,0x83a2},
                                                   {2,0x83a3},
                                                   {3,0x83a3},
                                                   {4,0x83a3},
                                                   {5,0x83a3},
                                                   {6,0x83a3},
                                                   {7,0x83a3},
                                                   {2,0x83a4},
                                                   {3,0x83a4},
                                                   {4,0x83a4},
                                                   {5,0x83a4},
                                                   {6,0x83a4},
                                                   {7,0x83a4},
                                                   {2,0x83a5},
                                                   {3,0x83a5},
                                                   {4,0x83a5},
                                                   {5,0x83a5},
                                                   {6,0x83a5},
                                                   {7,0x83a5},
                                                   {2,0x83a6},
                                                   {3,0x83a6},
                                                   {4,0x83a6},
                                                   {5,0x83a6},
                                                   {6,0x83a6},
                                                   {7,0x83a6},
                                                   {2,0x83a7},
                                                   {3,0x83a7},
                                                   {4,0x83a7},
                                                   {5,0x83a7},
                                                   {6,0x83a7},
                                                   {7,0x83a7} };

int c3hppc_etu_hw_init( int nUnit, c3hppc_etu_control_info_t *pC3CmuControlInfo ) {

  uint32 uRegisterValue;
  int nIndex, nAttempts;
  int nESM, rc;


  if ( SAL_BOOT_QUICKTURN ) {
    uRegisterValue = 0x002c007f;
    WRITE_ILAMAC_RX_CONFIGr( nUnit, uRegisterValue );
    uRegisterValue = 0x0005007f;
    WRITE_ILAMAC_TX_CONFIG0r( nUnit, uRegisterValue );
    uRegisterValue = 0x0405007f;
    WRITE_ILAMAC_TX_CONFIG0r( nUnit, uRegisterValue );
    uRegisterValue = 0x1405007f;
    WRITE_ILAMAC_TX_CONFIG0r( nUnit, uRegisterValue );

    uRegisterValue = 0x00120013;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013fff3;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013fff7;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013ffff;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013fffe;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013fffc;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013fffd;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x0013ffff;
    WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
    uRegisterValue = 0x00010000;
    WRITE_ETU_BIST_CTLr( nUnit, uRegisterValue );

    READ_ILAMAC_TX_CONFIG0r( nUnit, &uRegisterValue );
    soc_reg_field_set(nUnit, ILAMAC_TX_CONFIG0r, &uRegisterValue, BIT_ORDER_INVERTf, 1);
    WRITE_ILAMAC_TX_CONFIG0r( nUnit, uRegisterValue );

    READ_ILAMAC_RX_CONFIGr( nUnit, &uRegisterValue );
    soc_reg_field_set(nUnit, ILAMAC_RX_CONFIGr, &uRegisterValue, BIT_ORDER_INVERTf, 1);
    WRITE_ILAMAC_RX_CONFIGr( nUnit, uRegisterValue );
  }

/*
    sh_rcload_file(nUnit, NULL, "/tftpboot/caladan3/sv/c3hppc/etu_nl11k_bringup.soc", FALSE);
*/
  rc = 1;
  nAttempts = C3HPPC_ETU_INTERFACE_BRINGUP_ATTEMPTS;
  while ( rc && nAttempts ) {
    rc = c3hppc_etu_interface_bringup( nUnit );
    --nAttempts;
  }

  if ( nAttempts == 0 && rc ) {
    cli_out("\nERROR: ETU/NL11K interface NOT up!\n");

    c3hppc_etu_display_error_state( nUnit );

    for ( nESM = 0; nESM < 3; ++nESM ) {
/*
      c3hppc_etu_dsc_dump( nUnit, nESM );
*/
    }
    cli_out("\nINFO: Running PRBS31 test ... \n");
    c3hppc_etu_prbs31_test( nUnit );

    return 1;
  } else {
    nAttempts = C3HPPC_ETU_INTERFACE_BRINGUP_ATTEMPTS - nAttempts;
    cli_out("\n\nETU/NL11K interface up after %d attempt%s ...\n\n", nAttempts, ((nAttempts == 1) ? "" : "s") );
/*
    cli_out("\nETU/NL11K interface is UP ...\n\n" );
*/
  }


  for ( nIndex = 0; nIndex < C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS; ++nIndex ) {
    g_NL11K_ErrorStatusRegisterStoredValue[nIndex] = 0xffff;
  }

  g_c3hppcEtuUpdateManagerCB.nUnit = nUnit;
  g_c3hppcEtuUpdateManagerCB.bExit = 0;
  g_c3hppcEtuUpdateManagerCB.nUpdateCmdQWrPtr = 0;
  g_c3hppcEtuUpdateManagerCB.nUpdateCmdQRdPtr = 0;
  g_c3hppcEtuUpdateManagerCB.nUpdateCmdQCount = 0;
  for ( nIndex = 0; nIndex < C3HPPC_ETU_TABLE_NUM; ++nIndex ) {
    g_c3hppcEtuUpdateManagerCB.aTableParameters[nIndex].bValid = 0;
  }

  g_EtuUpdateCmdManagerThreadID = sal_thread_create( "tEtuUpdateCmdManager",
                                                     SAL_THREAD_STKSZ,
                                                     100,
                                                     c3hppc_etu_update_cmd_manager,
                                                     (void *) &g_c3hppcEtuUpdateManagerCB);
  if ( g_EtuUpdateCmdManagerThreadID == NULL || g_EtuUpdateCmdManagerThreadID == SAL_THREAD_ERROR ) {
    cli_out("\nERROR: Can not create ETU update CMD manager thread\n");
  }

  return 0;
}


int c3hppc_etu_interface_bringup( int nUnit ) {
  int rc, nESM;
  uint32 uRegisterValue, uRegisterValueTx;
  uint16 u16bRegisterValue;
  int nTimeout;
  uint64 uuDW0, uuDW1;

  READ_CX_SOFT_RESET_0r( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, ET_RESET_Nf, 0 );
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );
  soc_reg_field_set( nUnit, CX_SOFT_RESET_0r, &uRegisterValue, ET_RESET_Nf, 1 );
  WRITE_CX_SOFT_RESET_0r( nUnit, uRegisterValue );

  rc = c3hppc_etu_esm_bringup( nUnit );
  if ( rc ) return 1;
  rc = c3hppc_etu_nl11k_bringup( nUnit );
  if ( rc ) return 1;

  /* Assert "reset_rx_asic"  */
  for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
/*
    c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x8345, 0x8000 );
*/
  }

  /* Disable Rx Sequencer */
  for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
/*
    c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x821e, 0xa000 );
*/
  }

  /* Enable Rx Sequencer */
  for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
/*
    c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x821e, 0x2000 );
*/
  }

  nTimeout = 1000;
  while ( nTimeout-- ) {
    READ_WCL_CUR_STSr( nUnit, &uRegisterValue );
    if ( (uRegisterValue & 0xfff) == 0xfff ) {
      break;
    } else {
      sal_usleep( 100 );  /* Wait 100us */
    } 
  }

  if ( !nTimeout ) {
    cli_out("\n<c3hppc_etu_interface_bringup>ERROR: ESMs \"RXSEQDONE1G\" TIMEOUT!  Lane State --> 0x%03x\n", uRegisterValue );
    return 1;
  }

  /* De-assert "reset_rx_asic" procedure */
  for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
/*
    c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x8345, 0x0000 );
*/
  }

  READ_ETU_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, ILAMAC_RX_LBUS_RST_f, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
  sal_usleep( 100000 );     /* Wait 100 ms */
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, ILAMAC_RX_SERDES_RST_f, 0xfff );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );


  /* Clear Satellite sticky registers */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811b, 0x2040 );
  sal_usleep( 1000000 );
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811b, 0x0000 );


  sal_usleep( 2000000 );

  READ_ILAMAC_RX_INTF_STATE0r( nUnit, &uRegisterValue );
  READ_ILAMAC_TX_INTF_STATEr( nUnit, &uRegisterValueTx );
  soc_miimc45_read( nUnit, g_nNL11K_PhyID, (uint8)g_NL11K_ErrorStatusRegisters[C3HPPC_ETU__RX_PCS_UI_VIOLATION_ERR_15to0_INDEX][0],
                    g_NL11K_ErrorStatusRegisters[C3HPPC_ETU__RX_PCS_UI_VIOLATION_ERR_15to0_INDEX][1], &u16bRegisterValue );
  soc_miimc45_read( nUnit, g_nNL11K_PhyID, (uint8)g_NL11K_ErrorStatusRegisters[C3HPPC_ETU__RX_PCS_UI_VIOLATION_ERR_15to0_INDEX][0],
                    g_NL11K_ErrorStatusRegisters[C3HPPC_ETU__RX_PCS_UI_VIOLATION_ERR_15to0_INDEX][1], &u16bRegisterValue );
  if ( uRegisterValue != 0x1cffffff || uRegisterValueTx != 0x01000fff || u16bRegisterValue != 0 ) return 1;

  /*
     Sending out a "trail-blazing" transaction across the serdes interface to ensure the link is truly in alignment and operational.
  */
  COMPILER_64_SET(uuDW0,0xffffffff,0xffffffff);
  if ( c3hppc_etu_read_nl11k_register( nUnit, C3HPPC_ETU_NL11K_DEVICE_ID_REGISTER_OFFSET, &uuDW1, &uuDW0 ) ) {
    cli_out("\nWARNING:  Read of Register[NL11K DEVICE ID] failed -- strongly suspect a NL11K detected alignment issue.\n\n" );
    return 1;
  }

  return 0;
}


int c3hppc_etu_esm_bringup( int nUnit ) {

  int nESM, rc;
  uint32 uRegisterValue;


  READ_ETU_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, ILAMAC_TX_SERDES_REFCLK_SELf, C3HPPC_ETU_TX_SERDES_REFCLK_SEL );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );

  rc = c3hppc_etu_wcl_reset_seq( nUnit, C3HPPC_ETU_MASTER_ESM );
  if ( rc ) return 1;

  READ_ETU_BIST_CTLr( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_BIST_CTLr, &uRegisterValue, ENABLEf, 0);
  WRITE_ETU_BIST_CTLr( nUnit, uRegisterValue );

  READ_ILAMAC_RX_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ILAMAC_RX_CONFIGr, &uRegisterValue, BIT_ORDER_INVERTf, 1);
  WRITE_ILAMAC_RX_CONFIGr( nUnit, uRegisterValue );

  READ_ILAMAC_TX_CONFIG0r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ILAMAC_TX_CONFIG0r, &uRegisterValue, BIT_ORDER_INVERTf, 1);
  soc_reg_field_set(nUnit, ILAMAC_TX_CONFIG0r, &uRegisterValue, XON_RX_CH0f, 1);
  soc_reg_field_set(nUnit, ILAMAC_TX_CONFIG0r, &uRegisterValue, TX_ENABLEf, 1);
  WRITE_ILAMAC_TX_CONFIG0r( nUnit, uRegisterValue );

  /* Error injection fields. Some default values of non-zero - so make it 0 */
  WRITE_ILAMAC_TX_CONFIG4r( nUnit, 0x00000000 );

  READ_ETU_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, ILAMAC_TX_LBUS_RST_f, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
  sal_usleep( 100000 );     /* Wait 100 ms */
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, ILAMAC_TX_SERDES_RST_f, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );


  /* Do "tx1g" fifo reset procedure */
  for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
    SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &uRegisterValue));
    soc_reg_field_set(nUnit, WCL_CTLr, &uRegisterValue, TXD1G_FIFO_RSTBf, 0xf);
    SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, uRegisterValue));
  }

  /* Do "tx1g" fifo reset procedure via MDIO.  This is required due to CRC24 errors on the NL11K side. */
  for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8061, 0x2000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8071, 0x2000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8081, 0x2000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8091, 0x2000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8061, 0x0000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8071, 0x0000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8081, 0x0000 ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8091, 0x0000 ); 
  }

  /*  This had no impact on the XOFF behavior I was seeing with 640b look-ups.  

  READ_ETU_RX_RSP_FIFO_CTLr( nUnit, &uRegisterValue ); 
  soc_reg_field_set(nUnit, ETU_RX_RSP_FIFO_CTLr, &uRegisterValue, XOFF_RX_THRf, 0x1c0 );
  soc_reg_field_set(nUnit, ETU_RX_RSP_FIFO_CTLr, &uRegisterValue, XON_RX_THRf, 0x180 );
  WRITE_ETU_RX_RSP_FIFO_CTLr( nUnit, uRegisterValue );
  */

  return 0;
}


int c3hppc_etu_prbs31_test( int nUnit ) {
  int nESM, nTimes, rc;
  uint16 uReg0, uReg1, uReg2, uReg3;

  rc = 0;

  for ( nESM = 0; nESM < 3; ++nESM ) {
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8019, 0xbbbb );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80b1, 0x1c47 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80c1, 0x1c47 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80d1, 0x1c47 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80e1, 0x1c47 );
    soc_miimc45_write( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0011, 1 );
    soc_miimc45_write( nUnit, g_nNL11K_PhyID, (0xb+nESM), 0x0011, 1 );
  }

  sal_usleep( 1000 );

  for ( nESM = 0; nESM < 3; ++nESM ) {
    for ( nTimes = 0; nTimes < 2; ++nTimes ) {
      soc_miimc45_read( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80b0, &uReg0 );
      soc_miimc45_read( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80c0, &uReg1 );
      soc_miimc45_read( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80d0, &uReg2 );
      soc_miimc45_read( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80e0, &uReg3 );
    }
    if ( uReg0 != 0x8000 || uReg1 != 0x8000 || uReg2 != 0x8000 || uReg3 != 0x8000 ) {
      cli_out("\n<c3hppc_etu_prbs31_test>ERROR: NL11K to ESM%d PRBS failure -->    Lane0[0x%04x] Lane1[0x%04x] Lane2[0x%04x] Lane3[0x%04x]\n",
              nESM, uReg0, uReg1, uReg2, uReg3 );
      rc = 1;
    }

    for ( nTimes = 0; nTimes < 2; ++nTimes ) {
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0014, &uReg0 );
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0015, &uReg1 );
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0016, &uReg2 );
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0017, &uReg3 );
    }
    if ( uReg0 || uReg1 || uReg2 || uReg3 ) {
      cli_out("\n<c3hppc_etu_prbs31_test>ERROR: ESM%d to NL11K PRBS failure -->    Lane0[0x%04x] Lane1[0x%04x] Lane2[0x%04x] Lane3[0x%04x]\n",
              nESM, uReg0, uReg1, uReg2, uReg3 );
      rc = 1;
    }
  }


  for ( nESM = 0; nESM < 3; ++nESM ) {
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8019, 0x0000 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80b1, 0x1c40 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80c1, 0x1c40 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80d1, 0x1c40 );
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80e1, 0x1c40 );
    soc_miimc45_write( nUnit, g_nNL11K_PhyID, (0x5+nESM), 0x0011, 0 );
    soc_miimc45_write( nUnit, g_nNL11K_PhyID, (0xb+nESM), 0x0011, 0 );
  }

  return rc;
}


int c3hppc_etu_nl11k_bringup( int nUnit ) {
  uint32 uRegisterValue;
  uint16 uRegValue;
  int nTimeout;
  int nLane;
  char sConfigVariable[32];
  char *psValue, *psEnd;
  int nValue;
  int nPhyBlock;
  int nLaneWithinQuad;
  uint16 uAddr;


  READ_ETU_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_SRST_Lf, 1 );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_CRST_Lf, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_SRST_Lf, 0 );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_CRST_Lf, 0 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );
  sal_usleep( 3 );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_SRST_Lf, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );

  /* Enable Tx squelch */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8117, 0x0fff ); 
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8118, 0xffff ); 
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8119, 0x00ff ); 

  /* Disable PCS for Rx & Tx */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x810c, 0x0000 );

  /* Enter Global SW Reset (Bit0 =Core PLL, Bit1=Core Logic, Bit2=Serdes Init Seq Trigger */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811c, 0x0007 );

  /* Tx tuning */
  for ( nLane = 0; nLane < C3HPPC_ETU_NL11K_TO_C3_LANE_NUM; ++nLane ) {
    sal_sprintf(sConfigVariable, "nl11k_lane%d_preemphasis", nLane);
    if ( ( psValue = sal_config_get(sConfigVariable) ) != NULL ) {
      nValue = sal_ctoi( psValue, &psEnd ); 
      nPhyBlock = 11 + ( nLane / 4 );
      nLaneWithinQuad = nLane & 3; 
      uAddr = 0x104 | (nLaneWithinQuad << 4);
      soc_miimc45_write( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr, (uint16) nValue ); 
      soc_miimc45_write( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr+1, 0x0001 ); 
      cli_out("\n<c3hppc_etu_nl11k_bringup>INFO: Setting Tx Lane[%d] pre-emphasis --> %d.\n", nLane, nValue);
    }

    sal_sprintf(sConfigVariable, "nl11k_lane%d_amplitude", nLane);
    if ( ( psValue = sal_config_get(sConfigVariable) ) != NULL ) {
      nValue = sal_ctoi( psValue, &psEnd );
      cli_out("\n<c3hppc_etu_nl11k_bringup>INFO: Setting Tx Lane[%d] amplitude --> %d.\n", nLane, nValue);
      nValue |= nValue << 2;
      nPhyBlock = 22;
      uAddr = 0;
      switch ( nLane ) {
        case 2:  uAddr = 0x0116; break;
        case 6:  uAddr = 0x010f; break;
        case 10: uAddr = 0x0108; break;
      }
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr, &uRegValue );
      uRegValue &= 0x00ff;
      uRegValue |= (nValue << 8);
      uRegValue |= (nValue << 12);
      soc_miimc45_write( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr, uRegValue );
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr+1, &uRegValue );
      uRegValue &= 0xfff0;
      uRegValue |= nValue;
      soc_miimc45_write( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr+1, uRegValue );
      soc_miimc45_read( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr+2, &uRegValue );
      uRegValue &= 0xf0ff;
      uRegValue |= (nValue << 8);
      soc_miimc45_write( nUnit, g_nNL11K_PhyID, nPhyBlock, uAddr+2, uRegValue );
    }
  } 


  /* Set UI Violation limit */
/*
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811e, 0x0001 );
*/


  /* Set Tx/Rx Serializer speed to 6.25Gbps */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811d, 0x0044 );

  /* Enable 24 Rx lanes */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8100, 0xffff );
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8101, 0x00ff );

  /* Enable 12 Tx lanes */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8102, 0x0fff );

  /* Lane swap */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8108, 0x0003 );

  /* Exit Global SW Reset (Bit0 =Core PLL, Bit1=Core Logic, Bit2=Serdes Init Seq Trigger */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x811c, 0x0000 );
  
  nTimeout = 2000;
  while ( nTimeout-- ) {
    soc_miimc45_read( nUnit, g_nNL11K_PhyID, 1, 0x8183, &uRegValue );
    if ( uRegValue & 0x0008 ) {
      break;
    } else {
      sal_usleep( 1000 );  /* Wait 1 ms */
    } 
  }

  if ( !nTimeout ) {
    cli_out("\n<c3hppc_etu_nl11k_bringup>ERROR: SerDes Reset Sequence Done TIMEOUT!\n");
    return 1;
  }


  /* Global enable for Rx & Tx PCS */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x810c, 0x0003 );

  /* Enable Serdes by removing Squelch */
  soc_miimc45_write( nUnit, g_nNL11K_PhyID, 0x1, 0x8117, 0x0000 );

  /* Read the General Purpose Status Register 3, 0x8184 to release assertion of GIO_L that may happen due to
     errors on the lanes during initialization.
  */
  soc_miimc45_read( nUnit, g_nNL11K_PhyID, 0x1, 0x8184, &uRegValue );


  sal_usleep( 3000 );

  READ_ETU_CONFIG4r( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_CONFIG4r, &uRegisterValue, EXT_TCAM_CRST_Lf, 1 );
  WRITE_ETU_CONFIG4r( nUnit, uRegisterValue );

  return 0;
}



int c3hppc_etu_wcl_reset_seq(int nUnit, unsigned master_esm)
{
    uint32   wcl_ctl_buf, uRegisterValue;
    unsigned nESM;
    uint16 uRegValue, uLane;
    int nTimeout;
    
    /* Don't rely on previous state - restore default state of WCL_CTL
       wcl - assert pwrdwn, iddq, etc.
       Set iddq to '0' and rstb_pll to '0' (if it is not '0' to start with, this
       is to keep pll in reset).
    */

    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, PWRDWNf, 1);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, IDDQf, 1);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_HWf, 0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_MDIOREGSf, 0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_PLLf, 0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, TXD1G_FIFO_RSTBf, 0x0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, PLL_BYPASSf, 0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, LCREF_ENf, 0); /* LCREF_EN=0 means PLL input clock = pad_refclk_p/n */
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, REFOUT_ENf, 0); /* REFOUT_EN=0 means lcrefoutp/n = HiZ */
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }

    /* Note: By default PLL_BYPASS=0 and we will keep it this way */

    /* Note: following sequence is based on email dated 6/17/2011, from Prasun Paul */

    /* wcl - remove iddq
       Set iddq to 0 and rstb_pll to 0 (if it is not 0 to start with, this is to keep pll in reset).  */ 

    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, IDDQf, 0);
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_PLLf, 0);
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }
    
    /* wcl - remove powerdown
       Set pwrdwn_tx[3:0]/ pwrdwn_rx[3:0] to '0' to release power down of Tx and Rx path.
       Can be controlled thru internal MDIO register bits as well to overwrite pin info.
       Note: wcl_ctl_pwrdwn from etu controls both pwrdwn_tx and pwrdwn_rx,
       so if we want to turn-off rx only, then we will have to do it thro mdio
    */
    
    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, PWRDWNf, 0);
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }

    /* wcl - master, slave clock assignment
       Set lcref_en and refout_en as desired. Do you need any explanation here?
       By default: LCREF_EN=0, REFOUT_EN=0 for all WCLs
    */
    
    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      if (nESM == master_esm) {
        /* Master */
        soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, LCREF_ENf, 0);
        soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, REFOUT_ENf, 1);
      } else {
        /* Slave */
        soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, LCREF_ENf, 1);
        soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, REFOUT_ENf, 0);
      }
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }
    
    sal_usleep(1000000);           /* Wait 1 s <HP> Tune this </HP> */

    /* wcl - De-assert RSTB_HW - hardware_reset
       Release rstb_hw and rstb_mdioregs. That means set to '1's
    */
    
    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_HWf, 1);
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }
    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_MDIOREGSf, 1);
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }

    /* wcl - Set all the MDIO registers inside the ESM SerDes. Take care of any
       necessary initial programming. 
       For example, we may have to set the PLL divider, multiplier, etc thro
       mdio writes, reads
    */



    /* Rx polarity swap */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80fa, 0x000c ); 
    }

    /* Rx lane swap on ESM0 and ESM2 */
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[0], 0x1, 0x816b, 0x004e ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[2], 0x1, 0x816b, 0x004e ); 

    /* Tx lane swap on ESM1 and ESM4 */
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[1], 0x1, 0x8169, 0x004e ); 
    soc_miimc45_write( nUnit, g_anESM_PhyIDs[4], 0x1, 0x8169, 0x004e ); 

    /* Disable Rx Sequencer */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x821e, 0xa000 );
    }

    /* Do Rx tuning procedure -- write (cdros_phase_sat_ctrl, 2) */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      for ( uLane = 0; uLane < 4; ++uLane ) {
        uRegValue = c3hppc_etu_esm_aer_read( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x8200 );
        uRegValue &= 0xfe7f;
        uRegValue |= (0x2 << 7);
        c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x8200, uRegValue );
      }
    }
    /* Do Rx tuning procedure -- write (tuning_sm_en, 1) */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      for ( uLane = 0; uLane < 4; ++uLane ) {
        uRegValue = c3hppc_etu_esm_aer_read( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x8230 );
        uRegValue |= 1;
        c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x8230, uRegValue );
      }
    }
    

    /* Set the speed to 6.25G */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      for ( uLane = 0; uLane < 4; ++uLane ) {
        c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x8308, 0x0003 );
        c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], uLane, 0x833e, 0x0040 );
      }
    }

    /* Set TxDriver parameters -- Anthony Brewster needs to confirm these values */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80a7, 0x2ff0 );
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80a5, 0x0680 );
/*
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80a7, 0x0240 );
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80a5, 0x0480 );
*/
    }

    /* Reset PLL Sequencer */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8000, 0x053f ); 
    }

    /* Start PLL Sequencer */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      soc_miimc45_write( nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x8000, 0x253f ); 
    }

    /* Enable Rx Sequencer */
    for ( nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM ) {
      c3hppc_etu_esm_aer_write( nUnit, g_anESM_PhyIDs[nESM], 0x01ff, 0x821e, 0x2000 );
    }

    /* wcl - De-assert RSTB_PLL - pll_reset
       Release rstb_pll
    */
    
    for (nESM = 0; nESM < C3HPPC_ETU_ESM_NUM; ++nESM) {
      SOC_IF_ERROR_RETURN(soc_reg32_get(nUnit, WCL_CTLr, 0, nESM, &wcl_ctl_buf));
      soc_reg_field_set(nUnit, WCL_CTLr, &wcl_ctl_buf, RSTB_PLLf, 1);
      SOC_IF_ERROR_RETURN(soc_reg32_set(nUnit, WCL_CTLr, 0, nESM, wcl_ctl_buf));
    }

    nTimeout = 1000;
    while ( nTimeout-- ) {
      READ_WCL_CUR_STSr( nUnit, &uRegisterValue );
      if ( (uRegisterValue & 0x0003f000) == 0x0003f000 ) {
        break;
      } else {
        sal_usleep( 100 );  /* Wait 100us */
      } 
    }

    if ( !nTimeout ) {
      cli_out("\n<c3hppc_etu_wcl_reset_seq>ERROR: ESMs \"TXPLL_LOCK\" TIMEOUT!  PLL State[5:0] --> 0x%02x\n",
              ((uRegisterValue >> 12) & 0x3f) );
      return 1;
    }

    return 0;
}




int c3hppc_etu_esm_aer_write( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr, uint16 uData ) {

  uint16 uPage, uOffset;
  
  uPage = uAddr & 0xfff0; 
  uOffset = 0x0010 | (uAddr & 0x000f); 

  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, uLane ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, uPage ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, uOffset, uData ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, 0x0000 ) ); 

  return 0;
}


uint16 c3hppc_etu_esm_aer_read( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr ) {

  uint16 uPage, uOffset, uRegValue;
  
  uPage = uAddr & 0xfff0; 
  uOffset = 0x0010 | (uAddr & 0x000f); 

  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, uLane ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, uPage ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_read(  nUnit, nPhyID, uOffset, &uRegValue ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) ); 
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, 0x0000 ) ); 

  return uRegValue;
}



int c3hppc_etu_dsc_dump( int nUnit, int nESM ) {

  uint16 uLane, uRegValue, uOffset, mask16;
  int nPhyID, cx4_sigdet, ppm, pf, txdrv, pre, main_, post, pe, ze, me, po, zo, mo;

  cli_out("ESM LN SD   PPM   PF    TX_DRVR PRE MAIN POST  PE   ZE   ME   PO   ZO   MO \n");  

  nPhyID = g_anESM_PhyIDs[nESM];

  for ( uLane = 0; uLane < 4; ++uLane ) {

    uOffset = 0x80b0 + (uLane << 4);
    soc_miimc45_read( nUnit, nPhyID, 0x1, uOffset, &uRegValue );
    cx4_sigdet = (int) (( uRegValue >> 15 ) & 0x1);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x8220 );
    ppm = (int) (uRegValue & 0xffff);
    mask16 = (0xffff >> 1) + 1;
    if ( ppm >= mask16 ) {
      ppm -= mask16 << 1;
    }
    ppm /= 84;

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822b );
    pf = (int) ( uRegValue & 0xf );
    
    uOffset = 0x8067 + (uLane << 4);
    soc_miimc45_read( nUnit, nPhyID, 0x1, uOffset, &uRegValue );
    txdrv = (int) uRegValue;

    uOffset = 0x8063 + (uLane << 4);
    soc_miimc45_read( nUnit, nPhyID, 0x1, uOffset, &uRegValue );
    uRegValue &= 0x3fff;
    uRegValue |= 0x4000;
    soc_miimc45_write( nUnit, nPhyID, 0x1, uOffset, uRegValue ); 
    uOffset = 0x8060 + (uLane << 4);
    soc_miimc45_read( nUnit, nPhyID, 0x1, uOffset, &uRegValue );
    pre = (int) ( uRegValue & 0xf );
    main_ = (int) ( (uRegValue >> 4) & 0x3f );
    post = (int) ( (uRegValue >> 10) & 0x1f );
    uOffset = 0x8063 + (uLane << 4);
    soc_miimc45_read( nUnit, nPhyID, 0x1, uOffset, &uRegValue );
    uRegValue &= 0x3fff;
    soc_miimc45_write( nUnit, nPhyID, 0x1, uOffset, uRegValue ); 

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822c );
    pe = (int) (uRegValue & 0x003f);
    pe = ( pe < 32 ) ? pe : (pe - 64);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822d );
    ze = (int) (uRegValue & 0x003f);
    ze = ( ze < 32 ) ? ze : (ze - 64);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822e );
    me = (int) (uRegValue & 0x003f);
    me = ( me < 32 ) ? me : (me - 64);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822c );
    po = (int) ( (uRegValue >> 6) & 0x003f);
    po = ( po < 32 ) ? po : (po - 64);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822d );
    zo = (int) ( (uRegValue >> 6) & 0x003f);
    zo = ( zo < 32 ) ? zo : (zo - 64);

    uRegValue = c3hppc_etu_esm_aer_read( nUnit, nPhyID, uLane, 0x822e );
    mo = (int) ( (uRegValue >> 6) & 0x003f);
    mo = ( mo < 32 ) ? mo : (mo - 64);



    cli_out(" %d   %d  %d %5d %4d     0x%04x %2d  %2d   %2d   %3d  %3d  %3d  %3d  %3d  %3d\n",
            nESM, uLane, cx4_sigdet, ppm, pf, txdrv, pre, main_, post, pe, ze, me, po, zo, mo );

  }

  return 0;
}



int c3hppc_etu_adjust_control_path_latency( int nUnit ) {

  uint32 uRegisterValue;


  READ_ETU_TX_REQ_FIFO_CTLr( nUnit, &uRegisterValue );
  soc_reg_field_set(nUnit, ETU_TX_REQ_FIFO_CTLr, &uRegisterValue, CP_ACC_THRf, 0);
  WRITE_ETU_TX_REQ_FIFO_CTLr( nUnit, uRegisterValue );

  READ_ETU_TX_CP_MAX_LATENCYr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, ETU_TX_CP_MAX_LATENCYr, &uRegisterValue, CNTf, 0x50 );
  WRITE_ETU_TX_CP_MAX_LATENCYr( nUnit, uRegisterValue );

  READ_ETU_WRAP_GLOBAL_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, ETU_WRAP_GLOBAL_CONFIGr, &uRegisterValue, PULSE_GEN_ENABLEf, 0 );
  WRITE_ETU_WRAP_GLOBAL_CONFIGr( nUnit, uRegisterValue );
  
  soc_reg_field_set( nUnit, ETU_WRAP_GLOBAL_CONFIGr, &uRegisterValue, PULSE_GEN_COUNTf, 0x3b );
  WRITE_ETU_WRAP_GLOBAL_CONFIGr( nUnit, uRegisterValue );
  
  soc_reg_field_set( nUnit, ETU_WRAP_GLOBAL_CONFIGr, &uRegisterValue, PULSE_GEN_ENABLEf, 1 );
  WRITE_ETU_WRAP_GLOBAL_CONFIGr( nUnit, uRegisterValue );

  READ_ETU_CP_FIFO_CTLr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, ETU_CP_FIFO_CTLr, &uRegisterValue, EARLY_FULL_THRf, 250 );
  WRITE_ETU_CP_FIFO_CTLr( nUnit, uRegisterValue );

  /* "SBUS_STALL_MAX_LAT" and "SBUS_STALL_GTE_CP_ACC_THR" need to be ignored. */
  WRITE_ETU_TX_REQ_FIFO_INTR_ENABLEr( nUnit, 0x5ff );

  /* "EARLY_FULL" and "FULL" need to be ignored. */
  WRITE_ETU_CP_FIFO_INTR_ENABLEr( nUnit, 0x73 );
  WRITE_ETU_CP_FIFO_INTR_CLRr( nUnit, 0x0c );

  return 0;
}


int c3hppc_etu_hw_cleanup( int nUnit ) {

  int nIndex;
  uint32 uRegisterValue;
  uint64 uuDW0, uuDW1;
  uint16 u16bRegisterValue;

  for ( nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0,
                              ((nIndex < g_nErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nErrorMaskRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0, 0x00000000 );
  }

  for ( nIndex = 0; nIndex < g_nEtErrorClearRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anEtErrorClearRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0,
                              ((nIndex < g_nEtErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }

  for ( nIndex = 0; nIndex < g_nEtErrorMaskRegistersCount; ++nIndex ) {
    uRegisterValue = 0xffffffff;
    if ( g_anEtErrorMaskRegisters[nIndex] == ETU_GLOBAL_INTR_ENABLEr ) {
      soc_reg_field_set( nUnit, ETU_GLOBAL_INTR_ENABLEr, &uRegisterValue, ENABLEf, 0x1cf );
    }
    soc_reg32_set( nUnit, g_anEtErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0, uRegisterValue );
  }


  for ( nIndex = 0; nIndex < C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS; ++nIndex ) {

    soc_miimc45_read( nUnit, g_nNL11K_PhyID, (uint8)g_NL11K_ErrorStatusRegisters[nIndex][0],
                                             g_NL11K_ErrorStatusRegisters[nIndex][1], &u16bRegisterValue );
    soc_miimc45_read( nUnit, g_nNL11K_PhyID, (uint8)g_NL11K_ErrorStatusRegisters[nIndex][0],
                                             g_NL11K_ErrorStatusRegisters[nIndex][1], &u16bRegisterValue );

    if ( u16bRegisterValue != 0 && !(g_NL11K_ErrorStatusRegisters[nIndex][1] == 0x8183 && u16bRegisterValue == 0x000e) &&
                                   !(g_NL11K_ErrorStatusRegisters[nIndex][1] == 0x8300 && u16bRegisterValue == 0x0ff0) &&
                                   !(g_NL11K_ErrorStatusRegisters[nIndex][1] == 0x8281 && u16bRegisterValue == 0x0001) ) {
      cli_out("\nINFO: Non-Zero clean-up state for Register[NL11K %s] -->   DATA = 0x%04x \n", g_NL11K_ErrorStatusRegisterNames[nIndex], u16bRegisterValue );
    }

    g_NL11K_ErrorStatusRegisterStoredValue[nIndex] = u16bRegisterValue;
  }




  /* Based on the NL11K spec not sure whether this register is W1TC or not. */
  /* Apologies for the delayed response. It is a write 1 to clear register.
     I agree that the documentation is a little unclear. I'll work on getting that fixed.
     Thanks,
     Zayan
  */
  COMPILER_64_SET(uuDW0,0xffffffff,0xffffffff);
  COMPILER_64_SET(uuDW1,0xffffffff,0xffffffff);
  if ( c3hppc_etu_write_nl11k_register( nUnit, C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTER_OFFSET, uuDW1, uuDW0 ) ) {
    return -1;
  }


  return 0;
}



int c3hppc_etu_display_error_state( int nUnit ) {

  int rc, nIndex;
  uint64 uuDW0, uuDW1;

  for ( rc = 0, nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anErrorRegisters[nIndex] );
  }

  for ( nIndex = 0; nIndex < g_nEtErrorRegistersCount; ++nIndex ) {
    if ( g_anEtErrorRegisters[nIndex] == ILAMAC_TX_PACKETS_COUNTr ) {
      SOC_IF_ERROR_RETURN( READ_ILAMAC_TX_PACKETS_COUNTr( nUnit, &uuDW0 ) );
      cli_out("\nRegister[ILAMAC_TX_PACKETS_COUNT] -->   DATA = 0x%08x%08x \n\n", COMPILER_64_HI(uuDW0), COMPILER_64_LO(uuDW0));
    } else if ( g_anEtErrorRegisters[nIndex] == ILAMAC_RX_PACKETS_COUNTr ) { 
      SOC_IF_ERROR_RETURN( READ_ILAMAC_RX_PACKETS_COUNTr( nUnit, &uuDW0 ) );
      cli_out("\nRegister[ILAMAC_RX_PACKETS_COUNT] -->   DATA = 0x%08x%08x \n\n", COMPILER_64_HI(uuDW0), COMPILER_64_LO(uuDW0));
    } else {
      rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anEtErrorRegisters[nIndex] );
    }
  }

  COMPILER_64_SET(uuDW0,0xffffffff,0xffffffff);
  if ( c3hppc_etu_read_nl11k_register( nUnit, C3HPPC_ETU_NL11K_DEVICE_ID_REGISTER_OFFSET, &uuDW1, &uuDW0 ) ) {
    cli_out("\nERROR:  Read of Register[NL11K DEVICE ID] FAILED!\n\n" );
  } else {
    cli_out("\nRegister[NL11K DEVICE ID] -->   DATA = 0x%05x \n\n", COMPILER_64_LO(uuDW0) );

    COMPILER_64_SET(uuDW0,0xffffffff,0xffffffff);
    if ( c3hppc_etu_read_nl11k_register( nUnit, C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTER_OFFSET, &uuDW1, &uuDW0 ) ) {
      cli_out("\nERROR:  Read of Register[NL11K ERROR STATUS] FAILED!\n\n" );
    } else {
      cli_out("\nRegister[NL11K ERROR STATUS] -->   DATA = 0x%08x%08x \n\n", COMPILER_64_HI(uuDW0), COMPILER_64_LO(uuDW0));
    }
  }

  rc += c3hppc_etu_display_nl11k_error_state( nUnit );

  return rc;
}

int c3hppc_etu_display_nl11k_error_state( int nUnit ) {

  int rc, nIndex;
  uint16 uRegisterValue;

  for ( nIndex = 0, rc = 0; nIndex < C3HPPC_ETU_NL11K_ERROR_STATUS_REGISTERS; ++nIndex ) {
    soc_miimc45_read( nUnit, g_nNL11K_PhyID, (uint8)g_NL11K_ErrorStatusRegisters[nIndex][0],
                                             g_NL11K_ErrorStatusRegisters[nIndex][1], &uRegisterValue ); 
    if ( uRegisterValue != 0 && !(g_NL11K_ErrorStatusRegisters[nIndex][1] == 0x8183 && uRegisterValue == 0x000e) &&
         !(g_NL11K_ErrorStatusRegisters[nIndex][1] == 0x8300 && uRegisterValue == 0x0ff0) ) {
      ++rc;
      cli_out("\nRegister[NL11K %s] -->   DATA = 0x%04x \n", g_NL11K_ErrorStatusRegisterNames[nIndex], uRegisterValue );
    }
    if ( g_NL11K_ErrorStatusRegisterStoredValue[nIndex] != uRegisterValue ) {
      cli_out("Register[NL11K %s] -->   CURRENT = 0x%04x  --  STORED = 0x%04x \n", g_NL11K_ErrorStatusRegisterNames[nIndex],
              uRegisterValue, g_NL11K_ErrorStatusRegisterStoredValue[nIndex] );
    }
  }

  return rc;
}


int c3hppc_etu_setup_search_program( int nUnit, int nProgram, int nLTRsel, int nLayoutSelect ) {

  uint32 uFieldValue;
  uint32 auProgramMemEntry[2];
 

  auProgramMemEntry[0] = auProgramMemEntry[1] = 0;
  nLayoutSelect &= 3;

  uFieldValue = 0;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, NOT_SEARCHf, &uFieldValue );
  uFieldValue = 1;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, USE_TAGIDf, &uFieldValue );
  switch ( nLayoutSelect ) {
    case C3HPPC_ETU_LOOKUP__80:
    case C3HPPC_ETU_LOOKUP__4x80: uFieldValue = 2; break;
    case C3HPPC_ETU_LOOKUP__160:  uFieldValue = 3; break;
    case C3HPPC_ETU_LOOKUP__320:  uFieldValue = 5; break;
    case C3HPPC_ETU_LOOKUP__640:  uFieldValue = 10; break;
  }
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, DW_REQUIREDf, &uFieldValue );
  uFieldValue = (uint32) nLTRsel;  /* LTR select */
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, APP1f, &uFieldValue );
  uFieldValue = ( nLayoutSelect == C3HPPC_ETU_LOOKUP__640 ) ? 2 : 1;  /* Write and Compare 1|2 opcode */
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, APP2f, &uFieldValue );
  uFieldValue = 8;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, IL_EOPf, &uFieldValue );
  SOC_IF_ERROR_RETURN( soc_mem_write(nUnit, PROG_MEMm, MEM_BLOCK_ANY, nProgram, (void *) auProgramMemEntry) );

  return 0;
}


int c3hppc_etu_setup_nop_program( int nUnit, int nProgram ) {

  uint32 uFieldValue;
  uint32 auProgramMemEntry[2];
 

  auProgramMemEntry[0] = auProgramMemEntry[1] = 0;

  uFieldValue = 1;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, USE_TAGIDf, &uFieldValue );
  uFieldValue = 2;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, DW_REQUIREDf, &uFieldValue );
  uFieldValue = 8;
  soc_mem_field_set( nUnit, PROG_MEMm, auProgramMemEntry, IL_EOPf, &uFieldValue );
  SOC_IF_ERROR_RETURN( soc_mem_write(nUnit, PROG_MEMm, MEM_BLOCK_ANY, nProgram, (void *) auProgramMemEntry) );

  return 0;
}

uint32 c3hppc_etu_get_tcam_table_key_size( int nTable ) {
  return g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments; 
}


int c3hppc_etu_tcam_table_layout_setup( int nUnit, int nTable, int nLayoutSelect, int nMaxKeys ) {

  uint32 uAddress, uLTRbaseAddress;
  uint32 uLTR;
  uint64 uuReg79_64;
  uint64 uuReg63_0;
  c3hppc_etu_nl11k_ltr_misc_format_ut LTRmisc;
  c3hppc_etu_nl11k_ltr_key_construct0_format_ut LTRkeyConstruct0;
  c3hppc_etu_nl11k_ltr_key_construct1_format_ut LTRkeyConstruct1;
  c3hppc_etu_nl11k_block_config_format_ut BlockConfig;
  int nBlock, nBlockNum, nParallelSearchNum, nParallelSearch;


  COMPILER_64_ZERO(uuReg79_64);
  uLTR = (uint32) nTable;
  uLTRbaseAddress = C3HPPC_ETU_NL11K_LTR_BASE_ADDRESS + (uLTR * C3HPPC_ETU_NL11K_LTR_SET_OFFSET); 

  g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].bValid = 1;
  g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].nTableSize = nMaxKeys;

  nParallelSearchNum = ( nLayoutSelect == C3HPPC_ETU_LOOKUP__4x80 ) ? 4 : 1;
  nLayoutSelect &= 3;
  switch ( nLayoutSelect ) {
    case C3HPPC_ETU_LOOKUP__80:   g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments = 1; break;
    case C3HPPC_ETU_LOOKUP__160:  g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments = 2; break;
    case C3HPPC_ETU_LOOKUP__320:  g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments = 4; break;
    case C3HPPC_ETU_LOOKUP__640:  g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments = 8; break;
  }
  
  nBlockNum = (g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].nTableSize *
               g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments) / C3HPPC_ETU_NL11K_BLOCK_ENTRY_NUM_IN_80b;
  nBlockNum = C3HPPC_MAX( nBlockNum, 1 );

  COMPILER_64_ZERO(uuReg63_0);
  for ( nBlock = 0; nBlock < C3HPPC_MIN(64,nBlockNum); ++nBlock ) {
    uint64 uuTmp = COMPILER_64_INIT(0x00000000,0x00000001);
    COMPILER_64_SHL(uuTmp, nBlock);
    COMPILER_64_OR(uuReg63_0, uuTmp);
  }
  if ( nParallelSearchNum == 4 ) {
    uint64 uuTmp;
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuReg63_0), COMPILER_64_LO(uuReg63_0));
    COMPILER_64_SHL(uuTmp, 32);
    COMPILER_64_OR(uuReg63_0,uuTmp);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_BLOCK_SELECT0_OFFSET;   
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1; 

  if ( nBlockNum > 64 || nParallelSearchNum == 4 ) {
    if ( nBlockNum > 64 ) {
      COMPILER_64_ZERO(uuReg63_0);
      for ( nBlock = 0; nBlock < (nBlockNum - 64); ++nBlock ) {
        uint64 uuTmp = COMPILER_64_INIT(0x00000000,0x00000001);
        COMPILER_64_SHL(uuTmp, nBlock);
        COMPILER_64_OR(uuReg63_0, uuTmp);
      }
    }
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_BLOCK_SELECT1_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1; 
  }

  if ( nParallelSearchNum == 4 ) {
    COMPILER_64_SET(uuReg63_0, 0xffffaaaa,0x55550000);
  } else {
    COMPILER_64_ZERO(uuReg63_0);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_SUPER_BLOCK_SELECT_OFFSET;
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1; 


  if ( nParallelSearchNum == 4 ) {
    COMPILER_64_ZERO(uuReg63_0);
  } else {
    COMPILER_64_ZERO(uuReg63_0);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH0_OFFSET;
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1;
  if ( nParallelSearchNum == 4 ) {
    COMPILER_64_SET(uuReg63_0, 0x55555555,0x55555555);
  } else {
    COMPILER_64_ZERO(uuReg63_0);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH1_OFFSET;
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1;
  if ( nParallelSearchNum == 4 ) {
    COMPILER_64_SET(uuReg63_0, 0xaaaaaaaa,0xaaaaaaaa);
  } else {
    COMPILER_64_ZERO(uuReg63_0);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH2_OFFSET;
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1;
  if ( nParallelSearchNum == 4 ) {
    COMPILER_64_SET(uuReg63_0, 0xffffffff,0xffffffff);
  } else {
    COMPILER_64_ZERO(uuReg63_0);
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_PARALLEL_SEARCH3_OFFSET;
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, uuReg63_0 ) ) return -1;


  COMPILER_64_ZERO(LTRmisc.value);
  LTRmisc.bits.SearchResultNum = ( nParallelSearchNum == 4 ) ? 0 : 1;
  LTRmisc.bits.BMR_Select0 = 7;       /* BMR disabled */
  LTRmisc.bits.BMR_Select1 = 7;
  LTRmisc.bits.BMR_Select2 = 7;
  LTRmisc.bits.BMR_Select3 = 7;
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_MISC_OFFSET;   
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRmisc.value ) ) return -1; 

  COMPILER_64_ZERO(LTRkeyConstruct0.value);
  COMPILER_64_ZERO(LTRkeyConstruct1.value);
  if ( nLayoutSelect == C3HPPC_ETU_LOOKUP__80 ) {
    LTRkeyConstruct0.bits.StartByte0 = 0;  /* Extract the 80 lsbs of the master key for lookup */
    LTRkeyConstruct0.bits.NumBytes0 = 9;
  } else if ( nLayoutSelect == C3HPPC_ETU_LOOKUP__160 ) {
    LTRkeyConstruct0.bits.StartByte0 = 0;  /* Extract the 160 lsbs of the master key for lookup */
    LTRkeyConstruct0.bits.NumBytes0 = 9;
    LTRkeyConstruct0.bits.StartByte1 = 10;
    LTRkeyConstruct0.bits.NumBytes1 = 9;
  } else if ( nLayoutSelect == C3HPPC_ETU_LOOKUP__320 ) {
    LTRkeyConstruct0.bits.StartByte0 = 0;  /* Extract the 320 lsbs of the master key for lookup */
    LTRkeyConstruct0.bits.NumBytes0 = 9;
    LTRkeyConstruct0.bits.StartByte1 = 10;
    LTRkeyConstruct0.bits.NumBytes1 = 9;
    LTRkeyConstruct0.bits.StartByte2 = 20;
    LTRkeyConstruct0.bits.NumBytes2_bit3 = 1;
    LTRkeyConstruct0.bits.NumBytes2_bits2to0 = 1;
    LTRkeyConstruct0.bits.StartByte3 = 30;
    LTRkeyConstruct0.bits.NumBytes3 = 9;
  } else if ( nLayoutSelect == C3HPPC_ETU_LOOKUP__640 ) {
    LTRkeyConstruct0.bits.StartByte0 = 0;  /* Extract the 640 lsbs of the master key for lookup */
    LTRkeyConstruct0.bits.NumBytes0 = 9;
    LTRkeyConstruct0.bits.StartByte1 = 10;
    LTRkeyConstruct0.bits.NumBytes1 = 9;
    LTRkeyConstruct0.bits.StartByte2 = 20;
    LTRkeyConstruct0.bits.NumBytes2_bit3 = 1;
    LTRkeyConstruct0.bits.NumBytes2_bits2to0 = 1;
    LTRkeyConstruct0.bits.StartByte3 = 30;
    LTRkeyConstruct0.bits.NumBytes3 = 9;
    LTRkeyConstruct0.bits.StartByte4 = 40;
    LTRkeyConstruct0.bits.NumBytes4 = 9;
    LTRkeyConstruct1.bits.StartByte5 = 50;
    LTRkeyConstruct1.bits.NumBytes5 = 9;
    LTRkeyConstruct1.bits.StartByte6 = 60;
    LTRkeyConstruct1.bits.NumBytes6 = 9;
    LTRkeyConstruct1.bits.StartByte7 = 70;
    LTRkeyConstruct1.bits.NumBytes7_bit3 = 1;
    LTRkeyConstruct1.bits.NumBytes7_bits2to0 = 1;
  }
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU0_KEY_CONSTRUCT0_OFFSET;   
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct0.value ) ) return -1;
  uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU0_KEY_CONSTRUCT1_OFFSET;   
  if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct1.value ) ) return -1;
  if ( nParallelSearchNum == 4 ) {
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU1_KEY_CONSTRUCT0_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct0.value ) ) return -1;
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU1_KEY_CONSTRUCT1_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct1.value ) ) return -1;
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU2_KEY_CONSTRUCT0_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct0.value ) ) return -1;
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU2_KEY_CONSTRUCT1_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct1.value ) ) return -1;
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU3_KEY_CONSTRUCT0_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct0.value ) ) return -1;
    uAddress = uLTRbaseAddress + C3HPPC_ETU_NL11K_LTR_KPU3_KEY_CONSTRUCT1_OFFSET;   
    if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, LTRkeyConstruct1.value ) ) return -1;
  }

  COMPILER_64_ZERO(BlockConfig.value);
  BlockConfig.bits.Enable = 1;
  BlockConfig.bits.Width = (uint32) nLayoutSelect;
  for ( nParallelSearch = 0; nParallelSearch < nParallelSearchNum; ++nParallelSearch ) {
    for ( nBlock = 0; nBlock < nBlockNum; ++nBlock ) {
      uAddress = C3HPPC_ETU_NL11K_BLOCK_BASE_ADDRESS +
                 (((nParallelSearch * 32) + nBlock) * C3HPPC_ETU_NL11K_BLOCK_SET_OFFSET) + C3HPPC_ETU_NL11K_BLOCK_CONFIG_OFFSET;
      if ( c3hppc_etu_write_nl11k_register( nUnit, uAddress, uuReg79_64, BlockConfig.value ) ) return -1;
    }
  }


  return 0;
}


int c3hppc_etu_write_nl11k_register( int nUnit, uint32 uAddress, uint64 uuData79_64, uint64 uuData63_0 ) {

  int rc;
  uint32 uRegisterValue;
  c3hppc_etu_cp_fifo_entry_t CPfifoEntry;
  uint32 auFieldValue[2];
  c3hppc_etu_nl11k_address_format_ut AddressFormat;
  sal_time_t TimeStamp;


  READ_ETU_GLOBAL_INTR_STSr( nUnit, &uRegisterValue ); 
  soc_reg_field_set(nUnit, ETU_GLOBAL_INTR_STSr, &uRegisterValue, CP_FIFO_CAPTURE_DONEf, 0); 
  WRITE_ETU_GLOBAL_INTR_STSr( nUnit, uRegisterValue );


  sal_memset( &CPfifoEntry, 0x00, sizeof(c3hppc_etu_cp_fifo_entry_t) );
  auFieldValue[0] = auFieldValue[1] = 0;
  AddressFormat.value = 0;
  AddressFormat.bits.Address = uAddress;

  auFieldValue[0] = 1;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, OPCf, auFieldValue );
  auFieldValue[0] = 8;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, EOPf, auFieldValue );
  auFieldValue[0] = 3;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, NUM_DWf, auFieldValue );
  auFieldValue[0] = 1;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, CAPTUREf, auFieldValue );
  auFieldValue[0] = AddressFormat.value;
  auFieldValue[1] = COMPILER_64_LO(uuData63_0);
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, DW0f, auFieldValue );
  auFieldValue[0] = COMPILER_64_HI(uuData63_0);
  auFieldValue[1] = COMPILER_64_LO(uuData79_64) & 0x0000ffff;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, DW1f, auFieldValue );
  SOC_IF_ERROR_RETURN( soc_mem_write(nUnit, ETU_CP_FIFOm, MEM_BLOCK_ANY, 0, (void *) CPfifoEntry.Words) );

  rc = 0;
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, ETU_GLOBAL_INTR_STSr, CP_FIFO_CAPTURE_DONEf, 1,
                               ((SAL_BOOT_QUICKTURN) ? 10 : 1), 0, &TimeStamp ) ) {
    cli_out("<c3hppc_etu_write_nl11k_register> -- CP_FIFO_CAPTURE_DONE event TIMEOUT!!!\n");
    c3hppc_etu_display_nl11k_error_state( nUnit );
    g_c3hppcEtuUpdateManagerCB.bExit = 1;
    rc = 1;
  }


  return rc;
}


int c3hppc_etu_read_nl11k_register( int nUnit, uint32 uAddress, uint64 *puuData79_64, uint64 *puuData63_0 ) {

  int rc;
  uint32 uRegisterValue;
  c3hppc_etu_cp_fifo_entry_t CPfifoEntry;
  uint32 auCPfifoRspEntry[3];
  uint32 auFieldValue[2];
  uint64 uuBuf;
  c3hppc_etu_nl11k_address_format_ut AddressFormat;
  sal_time_t TimeStamp;


  READ_ETU_GLOBAL_INTR_STSr( nUnit, &uRegisterValue ); 
  soc_reg_field_set(nUnit, ETU_GLOBAL_INTR_STSr, &uRegisterValue, CP_FIFO_CAPTURE_DONEf, 0); 
  WRITE_ETU_GLOBAL_INTR_STSr( nUnit, uRegisterValue );


  sal_memset( &CPfifoEntry, 0x00, sizeof(c3hppc_etu_cp_fifo_entry_t) );
  auFieldValue[0] = auFieldValue[1] = 0;
  AddressFormat.value = 0;
  AddressFormat.bits.Address = uAddress;

  auFieldValue[0] = 2;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, OPCf, auFieldValue );
  auFieldValue[0] = 8;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, EOPf, auFieldValue );
  auFieldValue[0] = 2;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, NUM_DWf, auFieldValue );
  auFieldValue[0] = 1;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, CAPTUREf, auFieldValue );
  auFieldValue[0] = AddressFormat.value;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, CPfifoEntry.Words, DW0f, auFieldValue );
  SOC_IF_ERROR_RETURN( soc_mem_write(nUnit, ETU_CP_FIFOm, MEM_BLOCK_ANY, 0, (void *) CPfifoEntry.Words) );

  rc = 0;
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, ETU_GLOBAL_INTR_STSr, CP_FIFO_CAPTURE_DONEf, 1,
                               (SAL_BOOT_QUICKTURN ? 10 : 1), 0, &TimeStamp ) ) {
    cli_out("<c3hppc_etu_read_nl11k_register> -- CP_FIFO_CAPTURE_DONE event TIMEOUT!!!\n");
    c3hppc_etu_display_nl11k_error_state( nUnit );
    g_c3hppcEtuUpdateManagerCB.bExit = 1;
    rc = 1;
  } else {
    SOC_IF_ERROR_RETURN( soc_mem_read(nUnit, ETU_DBG_CP_FIFO_RSPm, MEM_BLOCK_ANY, 0, (void *) auCPfifoRspEntry) );
    soc_mem_field_get( nUnit, ETU_DBG_CP_FIFO_RSPm, auCPfifoRspEntry, DW0f, auFieldValue );
    COMPILER_64_SET(uuBuf, auFieldValue[1], auFieldValue[0]);
    uuBuf = c3hppcUtils_64b_byte_reflect( uuBuf );
    *puuData63_0 = uuBuf; 
    soc_mem_field_get( nUnit, ETU_DBG_CP_FIFO_RSPm, auCPfifoRspEntry, DW1f, auFieldValue );
    COMPILER_64_SET(uuBuf, auFieldValue[1], auFieldValue[0]);
    uuBuf = c3hppcUtils_64b_byte_reflect( uuBuf );
    *puuData79_64 = uuBuf; 
  }


  return rc;
}


int c3hppc_etu_populate_cpfifo_entry_for_database_write( int nUnit, uint32 uBlock, uint32 uRow, uint32 uVBIT,
                                                         uint64 *puuData, uint64 *puuMask, 
                                                         uint32 uCapture, c3hppc_etu_cp_fifo_entry_t *pCPfifoEntry ) {
  uint32 auFieldValue[2];
  c3hppc_etu_nl11k_address_format_ut AddressFormat;


  sal_memset( pCPfifoEntry, 0x00, sizeof(c3hppc_etu_cp_fifo_entry_t) );
  auFieldValue[0] = auFieldValue[1] = 0;
  AddressFormat.value = 0;
  AddressFormat.bits.AT = 1;
  AddressFormat.bits.Address = (uBlock << 12) | uRow;
  AddressFormat.bits.VBIT = uVBIT;

  auFieldValue[0] = 1;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, OPCf, auFieldValue );

  auFieldValue[0] = 0; 
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, CT_ADDR_LSBf, auFieldValue );
  auFieldValue[0] = 1 << 10;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, CT_ADDR_MSBf, auFieldValue );

  auFieldValue[0] = 8;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, EOPf, auFieldValue );

  auFieldValue[0] = 3;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, NUM_DWf, auFieldValue );

  auFieldValue[0] = uCapture;
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, CAPTUREf, auFieldValue );

  auFieldValue[0] = AddressFormat.value;
  auFieldValue[1] = COMPILER_64_LO(puuData[0]);
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, DW0f, auFieldValue );

  auFieldValue[0] = COMPILER_64_HI(puuData[0]);
  auFieldValue[1] = COMPILER_64_LO(puuData[1]) & 0x0000ffff;
  auFieldValue[1] |= ((COMPILER_64_LO(puuMask[0]) & 0x0000ffff) << 16);
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, DW1f, auFieldValue );

  auFieldValue[0] = (COMPILER_64_HI(puuMask[0]) << 16) | (COMPILER_64_LO(puuMask[0]) >> 16);
  auFieldValue[1] = (COMPILER_64_HI(puuMask[0]) >> 16);
  auFieldValue[1] |= ((COMPILER_64_LO(puuMask[1]) & 0x0000ffff) << 16);
  soc_mem_field_set( nUnit, ETU_CP_FIFOm, pCPfifoEntry->Words, DW2f, auFieldValue );


  return 0;
}


int c3hppc_etu_key_insert( int nTable, uint32 uStartingEntryIndex, int nNumberOfEntries,
                           c3hppc_etu_80b_data_t *pKeyData, c3hppc_etu_80b_data_t *pKeyMask, int nInsertOptions )
{
  c3hppc_etu_update_command_info_t *pActiveUpdateCmdInfo;
  uint32 uEntrySizeInBytes;

  pActiveUpdateCmdInfo =
      &(g_c3hppcEtuUpdateManagerCB.UpdateCmdQ[g_c3hppcEtuUpdateManagerCB.nUpdateCmdQWrPtr]);

  uEntrySizeInBytes = g_c3hppcEtuUpdateManagerCB.aTableParameters[nTable].uKeySizeIn80bSegments * sizeof( c3hppc_etu_80b_data_t );

  pActiveUpdateCmdInfo->nOptions = nInsertOptions;
  pActiveUpdateCmdInfo->nTable = nTable;
  pActiveUpdateCmdInfo->uStartingEntryIndex = uStartingEntryIndex;
  pActiveUpdateCmdInfo->nNumberOfEntries = nNumberOfEntries;
  pActiveUpdateCmdInfo->pKeyData = (c3hppc_etu_80b_data_t *) sal_alloc( (nNumberOfEntries * uEntrySizeInBytes), "Insert Data" );
  sal_memcpy( pActiveUpdateCmdInfo->pKeyData, pKeyData, (nNumberOfEntries * uEntrySizeInBytes) );
  pActiveUpdateCmdInfo->pKeyMask = (c3hppc_etu_80b_data_t *) sal_alloc( (nNumberOfEntries * uEntrySizeInBytes), "Insert Data" );
  sal_memcpy( pActiveUpdateCmdInfo->pKeyMask, pKeyMask, (nNumberOfEntries * uEntrySizeInBytes) );

  g_c3hppcEtuUpdateManagerCB.nUpdateCmdQWrPtr = (g_c3hppcEtuUpdateManagerCB.nUpdateCmdQWrPtr + 1) & (C3HPPC_ETU_UPDATE_QUEUE_SIZE - 1);
  ++g_c3hppcEtuUpdateManagerCB.nUpdateCmdQCount;


  return 0;
}


int c3hppc_etu_is_cmd_fifo_empty( void ) {
  if ( g_c3hppcEtuUpdateManagerCB.nUpdateCmdQCount ) return 0;
  else return 1;
}

int c3hppc_etu_exit_update_manager_thread( void ) {
  int rc;

  if ( g_c3hppcEtuUpdateManagerCB.bExit ) {
    rc = 1;
  } else {
    rc = 0;
    g_c3hppcEtuUpdateManagerCB.bExit = 1;
    sal_sleep(1);
  }

  return rc;
}


void c3hppc_etu_update_cmd_manager( void *pUpdateManagerCB_arg ) 
{

  int nUnit, rc COMPILER_ATTRIBUTE((unused));
  c3hppc_etu_update_manager_cb_t *pUpdateManagerCB;
  uint32 uRegisterValue;
  c3hppc_etu_update_command_info_t *pActiveUpdateCmdInfo;
  c3hppc_etu_cp_fifo_entry_t *pDmaData;
  uint32 uIndex, uDmaIndex;
  uint32 uCmdFifoFreeSpace;
  uint32 uSingleEntryCmdSize;
  uint32 uOperationSize;
  uint32 uKeyDataMaskIndex, uTableIndex;
  uint32 uCPfifoEntryNum;
  uint32 uBlock, uRow, uVBIT;


  uSingleEntryCmdSize = 0;
  uOperationSize = 0;
  uTableIndex = 0;
  rc = 0;

  cli_out("  Entering ETU Updater COMMAND Manager thread .... \n\n");

  pUpdateManagerCB = (c3hppc_etu_update_manager_cb_t *) pUpdateManagerCB_arg;
  nUnit = pUpdateManagerCB->nUnit;

  READ_CMIC_CMC2_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CMIC_CMC2_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 );
  WRITE_CMIC_CMC2_CONFIGr( nUnit, uRegisterValue );

/*
  READ_TMB_UPDATER_FIFO_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, TMB_UPDATER_FIFO_CONFIGr, &uRegisterValue, CMD_FIFO_AFULL_THRESHf, 1 );
  WRITE_TMB_UPDATER_FIFO_CONFIGr( nUnit, uRegisterValue );
*/

  pActiveUpdateCmdInfo = NULL;
  uKeyDataMaskIndex = 0;
  

  uCPfifoEntryNum = soc_mem_index_max( nUnit, ETU_CP_FIFOm ) + 1;
  pDmaData = (c3hppc_etu_cp_fifo_entry_t *) soc_cm_salloc( nUnit, (uCPfifoEntryNum * sizeof(c3hppc_etu_cp_fifo_entry_t)), "cmd fifo" );


  while ( !pUpdateManagerCB->bExit ) {

    READ_ILAMAC_RX_INTF_STATE0r( nUnit, &uRegisterValue );
    if ( uRegisterValue != 0x1cffffff ) {
      cli_out("\nERROR-2: ETU/NL11K interface went DOWN!!!\n");
      cli_out("  XON_TX_CH1=%d XON_TX_CH0=%d RX_WORD_SYNC=0x%03x RX_SYNCED=0x%03x RX_MISALIGNED=%d RX_ALIGNED_ERR=%d RX_ALIGNED=%d\n",
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, XON_TX_CH1f ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, XON_TX_CH0f ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_WORD_SYNCf ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_SYNCEDf ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_MISALIGNEDf ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_ALIGNED_ERRf ),
              soc_reg_field_get( nUnit, ILAMAC_RX_INTF_STATE0r, uRegisterValue, RX_ALIGNEDf ) );

      c3hppc_etu_display_nl11k_error_state( nUnit );
      pUpdateManagerCB->bExit = 1;
      break;
    }


    if ( pUpdateManagerCB->nUpdateCmdQCount || pActiveUpdateCmdInfo != NULL ) {

      if ( pActiveUpdateCmdInfo == NULL ) {
        pActiveUpdateCmdInfo = &(pUpdateManagerCB->UpdateCmdQ[pUpdateManagerCB->nUpdateCmdQRdPtr]);
        uSingleEntryCmdSize = pUpdateManagerCB->aTableParameters[pActiveUpdateCmdInfo->nTable].uKeySizeIn80bSegments;
        uOperationSize = uSingleEntryCmdSize * pActiveUpdateCmdInfo->nNumberOfEntries;
        uTableIndex = pActiveUpdateCmdInfo->uStartingEntryIndex;
        uKeyDataMaskIndex = 0;
      }

      uCmdFifoFreeSpace = uCPfifoEntryNum;  

      uDmaIndex = 0;
      while ( uOperationSize != 0 && 
              (uCmdFifoFreeSpace - uDmaIndex) >= uSingleEntryCmdSize ) {

        for ( uIndex = 0; uIndex < uSingleEntryCmdSize; ++uIndex, ++uDmaIndex, ++uKeyDataMaskIndex ) { 
          uBlock = uTableIndex >> 12;
          uRow = uTableIndex & (C3HPPC_ETU_NL11K_BLOCK_ENTRY_NUM_IN_80b - 1);
          uVBIT = 1;
          c3hppc_etu_populate_cpfifo_entry_for_database_write( nUnit, uBlock, uRow, uVBIT,
                                                               pActiveUpdateCmdInfo->pKeyData[uKeyDataMaskIndex].Words,
                                                               pActiveUpdateCmdInfo->pKeyMask[uKeyDataMaskIndex].Words,
                                                               0, (pDmaData+uDmaIndex) );
#if 0
          cli_out("uTableIndex  %x uBlock %x  uRow %x \n", uTableIndex, uBlock, uRow );
          cli_out("Key0  %llx Key1 %llx uSingleEntryCmdSize %d\n", pActiveUpdateCmdInfo->pKeyData[uKeyDataMaskIndex].Words[0],
                  pActiveUpdateCmdInfo->pKeyData[uKeyDataMaskIndex].Words[1], uSingleEntryCmdSize  );
#endif
          ++uTableIndex;
        }

        uOperationSize -= uSingleEntryCmdSize;
      }

      if ( uDmaIndex ) {
        /*    coverity[negative_returns : FALSE]    */
        rc = soc_mem_write_range( nUnit, ETU_CP_FIFOm, MEM_BLOCK_ANY, 0, (int)(uDmaIndex - 1), (void *) pDmaData);
        if ( uOperationSize == 0 ) {
          pUpdateManagerCB->nUpdateCmdQRdPtr = (pUpdateManagerCB->nUpdateCmdQRdPtr + 1) & (C3HPPC_ETU_UPDATE_QUEUE_SIZE - 1);
          --pUpdateManagerCB->nUpdateCmdQCount;
          if ( pActiveUpdateCmdInfo->pKeyData != NULL ) sal_free( pActiveUpdateCmdInfo->pKeyData );
          if ( pActiveUpdateCmdInfo->pKeyMask != NULL ) sal_free( pActiveUpdateCmdInfo->pKeyMask );
          pActiveUpdateCmdInfo = NULL;
        }
      }

    }

#if (defined(LINUX))
    sal_usleep(1);
#else
    sal_sleep(0);
#endif

  }  /* while ( !pUpdateManagerCB->bExit ) */

  soc_cm_sfree( nUnit, pDmaData );

  cli_out("  Exiting ETU Updater COMMAND Manager thread .... \n\n");

  sal_thread_exit(0);

  return;

}

#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
