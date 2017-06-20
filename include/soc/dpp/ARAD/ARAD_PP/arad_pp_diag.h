
/* $Id: arad_pp_diag.h,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_DIAG_INCLUDED__
/* { */
#define __ARAD_PP_DIAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/ARAD/arad_api_general.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/PPD/ppd_api_metering.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <bcm/policer.h>

#include <soc/dpp/PPC/ppc_api_diag.h>


#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define BLOCK_NUM   4

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_DIAG_FLD_FILL(prm_fld, prm_addr_msb, prm_addr_lsb, prm_fld_msb, prm_fld_lsb)  \
          (prm_fld)->base = (prm_addr_msb << 16) + prm_addr_lsb;  \
          (prm_fld)->msb = prm_fld_msb;  \
          (prm_fld)->lsb= prm_fld_lsb;

#define ARAD_PP_DIAG_FLD_READ(prm_fld, core_id, prm_blk, prm_addr_msb, prm_addr_lsb, prm_fld_msb, prm_fld_lsb, prm_err_num)  \
  ARAD_PP_DIAG_FLD_FILL(prm_fld, prm_addr_msb, prm_addr_lsb, prm_fld_msb, prm_fld_lsb);    \
  res = arad_pp_diag_dbg_val_get_unsafe(      \
          unit,      \
          core_id,   \
          prm_blk,      \
          prm_fld,      \
          regs_val      \
        );              \
  SOC_SAND_CHECK_FUNC_RESULT(res, prm_err_num, exit);


#define SOCDNX_DIAG_FLD_READ(prm_fld, core_id, prm_blk, prm_addr_msb, prm_addr_lsb, prm_fld_msb, prm_fld_lsb)  \
  ARAD_PP_DIAG_FLD_FILL(prm_fld, prm_addr_msb, prm_addr_lsb, prm_fld_msb, prm_fld_lsb);    \
  rv = arad_pp_diag_dbg_val_get_unsafe(      \
          unit,      \
          core_id,   \
          prm_blk,      \
          prm_fld,      \
          regs_val      \
        );              \
  SOCDNX_SAND_IF_ERR_EXIT(rv);


/* Get field value which is limited to 32 bits */
#define ARAD_PP_DIAG_FLD_GET(blk, core_id, addr_msb, addr_lsb, fld_msb, fld_lsb, fld_val)  \
    do { \
        ARAD_PP_DIAG_REG_FIELD fld; \
        uint32 tmp_buff[ARAD_PP_DIAG_DBG_VAL_LEN]; \
        fld.base = (addr_msb << 16) | (addr_lsb & 0xffff); \
        fld.msb = fld_msb; \
        fld.lsb = fld_lsb; \
        res = arad_pp_diag_dbg_val_get_unsafe(unit, core_id, blk, &fld, tmp_buff);  \
        SOC_SAND_CHECK_FUNC_RESULT(res, 123, exit); \
        fld_val = tmp_buff[0]; \
    } while (0); 


/* Get field value which is limited to 32 bits */
#define SOCDNX_DIAG_FLD_GET(blk, core_id, addr_msb, addr_lsb, fld_msb, fld_lsb, fld_val)  \
    do { \
        ARAD_PP_DIAG_REG_FIELD fld; \
        uint32 tmp_buff[ARAD_PP_DIAG_DBG_VAL_LEN]; \
        fld.base = (addr_msb << 16) | (addr_lsb & 0xffff); \
        fld.msb = fld_msb; \
        fld.lsb = fld_lsb; \
        rv = arad_pp_diag_dbg_val_get_unsafe(unit, core_id, blk, &fld, tmp_buff);  \
        SOCDNX_SAND_IF_ERR_EXIT(rv); \
        fld_val = tmp_buff[0]; \
    } while (0); 

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct {
    uint8 block_msb_max[BLOCK_NUM];
    uint8 block_lsb_max[BLOCK_NUM];
    uint8 block_id[BLOCK_NUM];
} debug_signals_t;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Register name of the register this field belongs to.
   */
  uint32 base;
  /*
   *  Field Most Significant Bit in the register.
   */
  uint32 msb;
  /*
   *  Field Least Significant Bit in the register.
   */
  uint32 lsb;

} ARAD_PP_DIAG_REG_FIELD;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_DIAG_SAMPLE_ENABLE_SET = ARAD_PP_PROC_DESC_BASE_DIAG_FIRST,
  ARAD_PP_DIAG_SAMPLE_ENABLE_SET_UNSAFE,
  ARAD_PP_DIAG_SAMPLE_ENABLE_SET_VERIFY,
  ARAD_PP_DIAG_SAMPLE_ENABLE_GET,
  ARAD_PP_DIAG_SAMPLE_ENABLE_GET_VERIFY,
  ARAD_PP_DIAG_SAMPLE_ENABLE_GET_UNSAFE,
  SOC_PPC_DIAG_MODE_INFO_SET,
  SOC_PPC_DIAG_MODE_INFO_SET_UNSAFE,
  SOC_PPC_DIAG_MODE_INFO_SET_VERIFY,
  SOC_PPC_DIAG_MODE_INFO_GET,
  SOC_PPC_DIAG_MODE_INFO_GET_VERIFY,
  SOC_PPC_DIAG_MODE_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_PKT_TRACE_CLEAR,
  SOC_PPC_DIAG_PKT_TRACE_CLEAR_UNSAFE,
  SOC_PPC_DIAG_PKT_TRACE_CLEAR_VERIFY,
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_GET,
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_GET_VERIFY,
  SOC_PPC_DIAG_PARSING_INFO_GET,
  SOC_PPC_DIAG_PARSING_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_PARSING_INFO_GET_VERIFY,
  ARAD_PP_DIAG_TERMINATION_INFO_GET,
  ARAD_PP_DIAG_TERMINATION_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_TERMINATION_INFO_GET_VERIFY,
  SOC_PPC_DIAG_FRWRD_LKUP_INFO_GET,
  SOC_PPC_DIAG_FRWRD_LKUP_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_FRWRD_LKUP_INFO_GET_VERIFY,
  ARAD_PP_DIAG_FRWRD_LPM_LKUP_GET,
  ARAD_PP_DIAG_FRWRD_LPM_LKUP_GET_UNSAFE,
  ARAD_PP_DIAG_FRWRD_LPM_LKUP_GET_VERIFY,
  SOC_PPC_DIAG_TRAPS_INFO_GET,
  SOC_PPC_DIAG_TRAPS_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_TRAPS_INFO_GET_VERIFY,
  ARAD_PP_DIAG_TRAPPED_PACKET_INFO_GET,
  ARAD_PP_DIAG_TRAPPED_PACKET_INFO_GET_PRINT,
  ARAD_PP_DIAG_TRAPPED_PACKET_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_TRAPPED_PACKET_INFO_GET_VERIFY,
  ARAD_PP_DIAG_TRAPS_ALL_TO_CPU,
  ARAD_PP_DIAG_TRAPS_ALL_TO_CPU_UNSAFE,
  ARAD_PP_DIAG_TRAPS_ALL_TO_CPU_VERIFY,
  ARAD_PP_DIAG_TRAPS_STAT_RESTORE,
  ARAD_PP_DIAG_TRAPS_STAT_RESTORE_UNSAFE,
  ARAD_PP_DIAG_TRAPS_STAT_RESTORE_VERIFY,
  ARAD_PP_DIAG_FRWRD_DECISION_TRACE_GET,
  ARAD_PP_DIAG_FRWRD_DECISION_TRACE_GET_UNSAFE,
  ARAD_PP_DIAG_FRWRD_DECISION_TRACE_GET_VERIFY,
  ARAD_PP_DIAG_LEARNING_INFO_GET,
  ARAD_PP_DIAG_LEARNING_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_LEARNING_INFO_GET_VERIFY,
  ARAD_PP_DIAG_ING_VLAN_EDIT_INFO_GET,
  ARAD_PP_DIAG_ING_VLAN_EDIT_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_ING_VLAN_EDIT_INFO_GET_VERIFY,
  ARAD_PP_DIAG_PKT_ASSOCIATED_TM_INFO_GET,
  ARAD_PP_DIAG_PKT_ASSOCIATED_TM_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_PKT_ASSOCIATED_TM_INFO_GET_VERIFY,
  SOC_PPC_DIAG_ENCAP_INFO_GET,
  SOC_PPC_DIAG_ENCAP_INFO_GET_UNSAFE,
  SOC_PPC_DIAG_ENCAP_INFO_GET_VERIFY,
  ARAD_PP_DIAG_EG_DROP_LOG_GET,
  ARAD_PP_DIAG_EG_DROP_LOG_GET_UNSAFE,
  ARAD_PP_DIAG_EG_DROP_LOG_GET_VERIFY,
  ARAD_PP_DIAG_DB_LIF_LKUP_INFO_GET,
  ARAD_PP_DIAG_DB_LIF_LKUP_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_DB_LIF_LKUP_INFO_GET_VERIFY,
  ARAD_PP_DIAG_DB_LEM_LKUP_INFO_GET,
  ARAD_PP_DIAG_DB_LEM_LKUP_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_DB_LEM_LKUP_INFO_GET_VERIFY,
  ARAD_PP_DIAG_DB_TCAM_LKUP_INFO_GET,
  ARAD_PP_DIAG_DB_TCAM_LKUP_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_DB_TCAM_LKUP_INFO_GET_VERIFY,
  ARAD_PP_DIAG_PKT_SEND,
  ARAD_PP_DIAG_PKT_SEND_UNSAFE,
  ARAD_PP_DIAG_PKT_SEND_VERIFY,
  ARAD_PP_DIAG_GET_PROCS_PTR,
  ARAD_PP_DIAG_GET_ERRS_PTR,
  ARAD_PP_DIAG_EGRESS_VLAN_EDIT_INFO_GET, 
  ARAD_PP_DIAG_EGRESS_VLAN_EDIT_INFO_GET_VERIFY, 
  ARAD_PP_DIAG_EGRESS_VLAN_EDIT_INFO_GET_UNSAFE, 
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_DIAG_DBG_VAL_GET_UNSAFE,
  ARAD_PP_DIAG_LIF_DB_ID_TO_DB_TYPE_MAP_GET,
  ARAD_PP_DIAG_TRAPS_RANGE_INFO_GET_UNSAFE,
  ARAD_PP_DIAG_PKT_ETH_HEADER_BUILD,

  SOC_PPC_DIAG_VSI_INFO_GET,
  SOC_PPC_DIAG_VSI_INFO_GET_VERIFY,
  SOC_PPC_DIAG_VSI_INFO_GET_UNSAFE,

  ARAD_PP_DIAG_GET_EPNI_RAW_SIGNAL,

	/*
   * Last element. Do no touch.
   */

  ARAD_PP_DIAG_PROCEDURE_DESC_LAST
} ARAD_PP_DIAG_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_DIAG_PKT_TRACE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_DIAG_FIRST,
  ARAD_PP_DIAG_MAX_SIZE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_VALID_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_BUFF_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_BUFF_LEN_OUT_OF_RANGE_ERR,
  SOC_PPC_DIAG_FLAVOR_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_LKUP_NUM_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_BANK_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_LKUP_USAGE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_IN_TM_PORT_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_PARSER_PROGRAM_POINTER_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_PACKET_QUAL_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_CODE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_CPU_DEST_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_IP_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_BASE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_OPCODE_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_LENGTH_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_FEC_PTR_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_ENCAP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_VLAN_TAG_FORMAT_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_NEXT_PRTCL_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_HDR_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_HDR_OFFSET_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_RANGE_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_TERM_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_LABEL_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_RANGE_BIT_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_FRWRD_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_VRF_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_TRILL_UC_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_FRWRD_HDR_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_VALID_FIELDS_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_METER1_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_METER2_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_DP_METER_CMD_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_COUNTER1_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_COUNTER2_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_CUD_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_EEP_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_DROP_LOG_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_ETHER_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_TOTAL_SIZE_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_NOF_PACKETS_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_INVALID_TM_PORT_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */


  ARAD_PP_DIAG_RESTORE_NOT_SAVED_ERR,
  ARAD_PP_DIAG_LIF_DB_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_DIAG_INVALID_LAST_PACKET_ERR,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_DIAG_ERR_LAST
} ARAD_PP_DIAG_ERR;

typedef struct
{
  uint32 trap_dest[SOC_PPC_NOF_TRAP_CODES * 4];

  uint8 already_saved;

  SOC_PPC_DIAG_MODE_INFO mode_info;
} ARAD_PP_SW_DB_DIAG;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* length of debug value */
#define ARAD_DIAG_BLK_NOF_BITS (256)
/* length of debug value */
#define ARAD_DIAG_DBG_VAL_LEN  (ARAD_DIAG_BLK_NOF_BITS/32)
#define ARAD_PP_DIAG_DBG_VAL_LEN        ARAD_DIAG_DBG_VAL_LEN

uint32
  arad_pp_diag_is_valid(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  int             core_id,
    SOC_SAND_IN  uint32          func_name,
    SOC_SAND_OUT uint32          *ret_val
);

uint32
  arad_pp_diag_init_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  int               core_id
);

uint32
  arad_pp_diag_dbg_val_get_unsafe(
    SOC_SAND_IN  int              unit,
    SOC_SAND_IN  int              core_id,
    SOC_SAND_IN  uint32               blk,
    SOC_SAND_IN  ARAD_PP_DIAG_REG_FIELD   *fld,
    SOC_SAND_OUT uint32               val[ARAD_PP_DIAG_DBG_VAL_LEN]
);


/*********************************************************************
 *     Stores an EPNI raw signal data to regs_val
 *********************************************************************/
uint32
  arad_pp_diag_get_raw_signal(
      int core_id,
      ARAD_MODULE_ID prm_blk,
      int prm_addr_msb,
      int prm_addr_lsb,
      int prm_fld_msb,
      int prm_fld_lsb,
      uint32 *regs_val
);

/*********************************************************************
* NAME:
 *   arad_pp_diag_sample_enable_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable/disable diagnostic APIs.affects only APIs with
 *   type: need_sample
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                                 enable -
 *     TRUE: diag APIs are enabled, FALSE diag APIs are
 *     disabled.
 * REMARKS:
 *   - when enabled will affect device power consuming
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_sample_enable_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_diag_sample_enable_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint8                                 enable
  );

uint32
  arad_pp_diag_sample_enable_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_diag_sample_enable_set_unsafe" API.
 *     Refer to "arad_pp_diag_sample_enable_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_diag_sample_enable_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT uint8                                 *enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_mode_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mode configuration for diag module, including
 *   diag-flavor
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO                      *mode_info -
 *     Mode of diagnsotcis
 * REMARKS:
 *   Diag Type: All-Packets
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_mode_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO                      *mode_info
  );

uint32
  arad_pp_diag_mode_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO                      *mode_info
  );

uint32
  arad_pp_diag_mode_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_diag_mode_info_set_unsafe" API.
 *     Refer to "arad_pp_diag_mode_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_diag_mode_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_DIAG_MODE_INFO                      *mode_info
  );

uint32
  arad_pp_diag_vsi_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
 * NAME:
 *   arad_pp_diag_vsi_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the VSI number to which the last packet was assigned.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_VSI_INFO           *vsi_info -
 *     Information regarding the Last packet VSI
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_vsi_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_VSI_INFO                      *vsi_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_diag_pkt_trace_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the trace of transmitted packet, so next trace
 *   info will relate to next packets to transmit
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  pkt_trace -
 *     Packet traces type as encoded by SOC_PPC_DIAG_PKT_TRACE.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_pkt_trace_clear_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  uint32                                  pkt_trace
  );

uint32
  arad_pp_diag_pkt_trace_clear_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  pkt_trace
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_received_packet_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the header of last received packet entered the
 *   device and the its association to TM/system/PP ports.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_RECEIVED_PACKET_INFO           *rcvd_pkt_info -
 *     Information regards Last received packet
 * REMARKS:
 *   Diag Type: Last-Packet, need-sampleif sample is disabled
 *   then: last-packet, clear-on-read
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_received_packet_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_RECEIVED_PACKET_INFO           *rcvd_pkt_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val
  );

uint32
  arad_pp_diag_received_packet_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_parsing_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns network headers qualified on packet upon
 *   parsing, including packet format, L2 headers fields,...
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_PARSING_INFO                   *pars_info -
 *     Information obtained from parsing including L2 headers,
 *     packet format,...
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_parsing_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_PARSING_INFO              *pars_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val

  );

uint32
  arad_pp_diag_parsing_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_termination_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns information obtained by termination including
 *   terminated headers
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_TERM_INFO                      *term_info -
 *     Includes terminated headers,
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_termination_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_TERM_INFO              *term_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                 *ret_val

  );

uint32
  arad_pp_diag_termination_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

#if defined(INCLUDE_KBP) && !defined(BCM_88030_A0)
uint32
  arad_pp_diag_get_request(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  int                     core_id,
    SOC_SAND_IN  uint32                     nof_bytes,
    SOC_SAND_OUT uint32                     *buffer
  );

uint32
    arad_pp_diag_get_frwrd_type_size(
        SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID type,
        SOC_SAND_OUT uint32                   *size
    );

ARAD_KBP_FRWRD_IP_TBL_ID
    arad_pp_diag_fwd_lkup_type_to_frwrd_ip_tbl_id(
        SOC_SAND_IN SOC_PPC_DIAG_FWD_LKUP_TYPE type
    );

void
  SOC_PPC_DIAG_IPV4_UNICAST_RPF_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_IPV4_UNICAST_RPF *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_IPV4_MULTICAST_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_IPV4_MULTICAST *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_IPV6_UNICAST_RPF_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_IPV6_UNICAST_RPF *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_IPV6_MULTICAST_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_IPV6_MULTICAST *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_MPLS_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_MPLS *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_TRILL_UNICAST_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_TRILL_UNICAST *info,
    SOC_SAND_IN uint32 offset
  );

void
  SOC_PPC_DIAG_TRILL_MULTICAST_print_with_offsets(
    SOC_SAND_IN SOC_PPC_DIAG_TRILL_MULTICAST *info,
    SOC_SAND_IN uint32 offset
  );

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030_A0) */

void
  ARAD_PP_DIAG_MTR_INFO_print(
	SOC_SAND_IN			int unit,
	SOC_SAND_IN			bcm_policer_t policer_id, 
	SOC_SAND_IN			bcm_policer_config_t *policer_cfg, 
	SOC_SAND_IN			int cbl, 
	SOC_SAND_IN			int ebl);

void
  ARAD_PP_DIAG_ETH_POLICER_INFO_print(
	SOC_SAND_IN		int 						              unit,
	SOC_SAND_IN		bcm_port_t 					          port,
	SOC_SAND_IN		SOC_PPC_MTR_BW_PROFILE_INFO   *policer_cfg, 
	SOC_SAND_IN		int 						              *bucket_lvl,
  SOC_SAND_IN		uint32 						            agg_policer_valid,
  SOC_SAND_IN		uint32 						            agg_policer_id);

void
  ARAD_PP_DIAG_AGGREGATE_ETH_POLICER_INFO_print(
	SOC_SAND_IN		int 						              unit,
	SOC_SAND_IN		bcm_policer_t 	              policer_id,
  SOC_SAND_IN		int 	                        nom_of_policers,
  SOC_SAND_IN		SOC_PPC_MTR_BW_PROFILE_INFO   *policer_cfg, 
  SOC_SAND_IN		int 						              *bucket_lvl);

/*********************************************************************
* NAME:
 *   arad_pp_diag_frwrd_lkup_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the forwarding lookup performed including:
 *   forwarding type (bridging, routing, ILM, ...), the key
 *   used for the lookup and the result of the lookup
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_INFO                *frwrd_info -
 *     forwarding lookup information including key and result
 * REMARKS:
 *   Diag Type: Last Packet, need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_frwrd_lkup_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_LKUP_INFO           *frwrd_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val
  );

uint32
  arad_pp_diag_frwrd_lkup_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_frwrd_lpm_lkup_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Simulate IP lookup in the device tables and return
 *   FEC-pointer
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY              *lpm_key -
 *     forwarding lookup information including key and result
 *   SOC_SAND_OUT uint32                             *fec_ptr -
 *     FEC pointer
 *   SOC_SAND_OUT uint8                            *found -
 *     Was key found
 * REMARKS:
 *   Diag Type: lookup a key, don't need-sample
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_frwrd_lpm_lkup_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY              *lpm_key,
    SOC_SAND_OUT uint32                             *fec_ptr,
    SOC_SAND_OUT uint8                            *found,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT              *ret_val
  );

uint32
  arad_pp_diag_frwrd_lpm_lkup_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY              *lpm_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_traps_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns information regard packet trapping/snooping,
 *   including which traps/snoops were fulfilled, which
 *   trap/snoop was committed, and whether packet was
 *   forwarded/processed according to trap or according to
 *   normal packet processing flow.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_TRAPS_INFO                     *traps_info -
 *     Information regarding the trapping
 * REMARKS:
 *   Diag Type: Last Packet, Clear-on-read. When called after
 *   injecting more than one packet then 'trap_stack' will
 *   hold the status for all injected packets from last call.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_traps_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_TRAPS_INFO                *traps_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val

  );

uint32
  arad_pp_diag_traps_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );
/*********************************************************************
* NAME:
 *   arad_pp_diag_frwrd_decision_trace_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return the trace (changes) for forwarding decision for
 *   last packet in several phases in processing
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO      *frwrd_trace_info -
 *     Forwarding decision in several phases in the processing
 * REMARKS:
 *   Diag Type: Last Packet, Need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_frwrd_decision_trace_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_FRWRD_DECISION_TRACE_INFO      *frwrd_trace_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                         *ret_val

  );

uint32
  arad_pp_diag_frwrd_decision_trace_get_verify(
    SOC_SAND_IN  int                                 unit
  );


/*********************************************************************
* NAME:
 *   arad_pp_diag_learning_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the information to be learned for the incoming
 *   packet. This is the information that the processing
 *   determine to be learned, the MACT supposed to learn this
 *   information.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEARN_INFO                     *learn_info -
 *     Learning information including key and value
 *     <destination and additional info (AC, EEP, MPLS command
 *     etc...)>
 * REMARKS:
 *   Diag Type: Last-Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_learning_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_LEARN_INFO                *learn_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val

  );

uint32
  arad_pp_diag_learning_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_ing_vlan_edit_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the result of ingress vlan editing,
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_VLAN_EDIT_RES                  *vec_res -
 *     Vlan edit command result, removed tags and build tags
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_ing_vlan_edit_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_VLAN_EDIT_RES             *vec_res,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val

  );

uint32
  arad_pp_diag_ing_vlan_edit_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_pkt_associated_tm_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   The Traffic management information associated with the
 *   packet including meter, DP, TC, etc...
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_PKT_TM_INFO                    *pkt_tm_info -
 *     Traffic management information associated with the
 *     packet
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_pkt_associated_tm_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_PKT_TM_INFO               *pkt_tm_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val
  );

uint32
  arad_pp_diag_pkt_associated_tm_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_encap_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the encapsulation and editing information applied to
 *   last packet
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO                     *encap_info -
 *     SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO *encap_info
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_encap_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_ENCAP_INFO             *encap_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                 *ret_val
  );

uint32
  arad_pp_diag_encap_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_out_rif_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the native out-RIF of last packet
 * INPUT:
 *   SOC_SAND_IN     int     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT    uint8   *rif_is_valid -
 *     Valid indication
 *   SOC_SAND_OUT    uint32   *out_rif -
 *     Native out-RIF
 * REMARKS:
 *   Diag Type: all Packets since last clear.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_out_rif_get_unsafe(
    SOC_SAND_IN     int     unit,
    SOC_SAND_IN     int     core_id,
    SOC_SAND_OUT    uint8   *rif_is_valid,
    SOC_SAND_OUT    uint32  *out_rif
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_eg_drop_log_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the reason for packet discard
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_EG_DROP_LOG_INFO               *eg_drop_log -
 *     Egress drop log, reason why packets were dropped.
 * REMARKS:
 *   Diag Type: all Packets since last clear.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_eg_drop_log_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_DIAG_EG_DROP_LOG_INFO               *eg_drop_log
  );

uint32
  arad_pp_diag_eg_drop_log_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_db_lif_lkup_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the lookup key and result used in the LIF DB
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info -
 *     The specific use of the DB, for example the lookup
 *     number
 *   SOC_SAND_OUT SOC_PPC_DIAG_LIF_LKUP_INFO                  *lkup_info -
 *     Lookup information, key and result
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_db_lif_lkup_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO               *db_info,
    SOC_SAND_OUT SOC_PPC_DIAG_LIF_LKUP_INFO             *lkup_info,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val

  );

uint32
  arad_pp_diag_db_lif_lkup_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_db_lem_lkup_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the lookup key and result used in the LEM DB
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info -
 *     The specific use of the DB, for example the lookup
 *     number
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_LKUP_TYPE                  *type -
 *     Lookup type
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_KEY                        *key -
 *     Lookup key
 *   SOC_SAND_OUT SOC_PPC_DIAG_LEM_VALUE                      *val -
 *     Lookup result
 *   SOC_SAND_OUT uint8                                 *valid -
 *     Is lookup result valid.
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_db_lem_lkup_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_LKUP_TYPE                  *type,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_KEY                        *key,
    SOC_SAND_OUT SOC_PPC_DIAG_LEM_VALUE                      *val,
    SOC_SAND_OUT uint8                                 *valid
  );

uint32
  arad_pp_diag_db_lem_lkup_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO                    *db_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_diag_egress_vlan_edit_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return the egress vlan edit command informations
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO           *prm_vec_res -
 * REMARKS:
 *   Diag Type: Last Packet. need-sample.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
  arad_pp_diag_egress_vlan_edit_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_OUT SOC_PPC_DIAG_EGRESS_VLAN_EDIT_INFO      *prm_vec_res,
    SOC_SAND_OUT SOC_PPC_DIAG_RESULT                    *ret_val
  );

uint32
  arad_pp_diag_egress_vlan_edit_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_diag_cos_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Return the cos info for each stage signal
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_diag_cos_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id
  );

uint32
  SOC_PPC_DIAG_MODE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_DIAG_MODE_INFO *info
  );

uint32
  SOC_PPC_DIAG_DB_USE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_DIAG_DB_USE_INFO *info
  );
uint32
  SOC_PPC_DIAG_IPV4_VPN_KEY_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_DIAG_IPV4_VPN_KEY *info
  );

uint32
  SOC_PPC_DIAG_IPV6_VPN_KEY_verify(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_DIAG_IPV6_VPN_KEY *info
  );


/************************************************************
  PRGE Dumps for DE
  ***********************************************************/


uint32
  arad_pp_diag_prge_first_instr_get(int unit, int core_id, uint32 *first_instruction);



/*
 * Function:     arad_pp_diag_ftmh_cfg_get
 * Purpose:      Call soc_ftmh_cfg_get
 */

int
arad_pp_diag_ftmh_cfg_get(int unit, int * p_cfg_ftmh_lb_key_ext_en, int * p_cfg_ftmh_stacking_ext_enable);



/*
 * Function:     arad_pp_diag_epni_prge_program_tbl_get
 * Purpose:      Call arad_pp_epni_prge_program_tbl_get_unsafe
 */

uint32
arad_pp_diag_epni_prge_program_tbl_get(int unit, uint32 offset, ARAD_PP_EPNI_PRGE_PROGRAM_TBL_DATA *tbl_data);

/*
 * Function:     arad_pp_diag_mem_read
 * Purpose:      Call soc_mem_read
 */

int
arad_pp_diag_mem_read(int unit,
                        soc_mem_t mem,
                        int copyno,
                        int in_line,
                        void* val);

uint32
  arad_pp_diag_fwd_decision_in_buffer_parse(
    SOC_SAND_IN  int                                  unit,    
    SOC_SAND_IN  ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE signal_type,
    SOC_SAND_IN  uint32                                  dest_buffer,
    SOC_SAND_IN  uint32                                  asd_buffer,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO             *fwd_decision
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_PP_DIAG_INCLUDED__*/
#endif
