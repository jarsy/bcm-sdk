#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_lag.c,v 1.35 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TRUNK

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>


#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lag.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/arad_parser.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>

#include <soc/dpp/ARAD/arad_ports.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/JER/JER_PP/jer_pp_diag.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LAG_LAG_NDX_MAX(__unit)                     (arad_ports_lag_nof_lag_groups_get_unsafe(__unit))
#define ARAD_PP_LAG_SYS_PORT_MAX                                 (SOC_TMC_NOF_SYS_PHYS_PORTS_ARAD)
#define ARAD_PP_LAG_SEED_MAX                                     (SOC_SAND_U16_MAX)
#define ARAD_PP_LAG_HASH_FUNC_ID_MIN                             (0x3)
#define ARAD_PP_LAG_HASH_FUNC_ID_MAX                             (SOC_IS_ARADPLUS(unit)? SOC_PPC_LAG_LB_CRC_0x9019: SOC_PPC_LAG_LB_CRC_0x1105D)
#define ARAD_PP_LAG_KEY_SHIFT_MAX                                (15)
#define ARAD_PP_LAG_NOF_HEADERS_MAX                              (3)
#define ARAD_PP_LAG_FIRST_HEADER_TO_PARSE_MAX                    (SOC_PPC_NOF_LAG_HASH_FRST_HDRS-1)
#define SOC_PPC_LAG_MEMBER_ID_MAX                                (255)
#define ARAD_PP_LAG_NOF_ENTRIES_MAX(__unit)                  (1 << (ARAD_PORT_LAG_TOTAL_MEMBER_NOF_BITS - \
                                                                   (ARAD_PORT_LAG_GROUP_NOF_BITS - \
                                                                   SOC_DPP_CONFIG(__unit)->arad->init.ports.lag_mode)))
#define SOC_PPC_LAG_LB_TYPE_MAX                                  (SOC_PPC_NOF_LAG_LB_TYPES-1)
#define ARAD_PP_LAG_MAX_ECMP_ENTRY                               (4095)
/*
 * Defines for hash vectors
 */

#define ARAD_PP_LB_VECTOR_INDEX_NONE             0x0
#define ARAD_PP_LB_VECTOR_INDEX_ETHERNET         0x1
#define ARAD_PP_LB_VECTOR_INDEX_FC               0x2
#define ARAD_PP_LB_VECTOR_INDEX_IPV4             0x3
#define ARAD_PP_LB_VECTOR_INDEX_IPV6             0x4
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX1           0x5
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1    0x6
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL2    0x7
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1    0x8
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL2    0x9
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL3    0xa
#define ARAD_PP_LB_VECTOR_INDEX_TRILL            0xb
#define ARAD_PP_LB_VECTOR_INDEX_L4               0xc
#define ARAD_PP_LB_VECTOR_INDEX_FC_VFT           0xd
#define ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4    0xe
#define ARAD_PP_LB_VECTOR_INDEX_CALC             0xf
#define ARAD_PP_LB_NOF_VECTOR_INDEXES            0xf


#define SOC_DPP_LB_VECTOR_INDEX_NONE             (SOC_IS_JERICHO(unit) ? 0x0  : 0x0)
#define SOC_DPP_LB_VECTOR_INDEX_ETHERNET         (SOC_IS_JERICHO(unit) ? 0x1  : 0x1)
#define SOC_DPP_LB_VECTOR_INDEX_ETHERNET_1       (SOC_IS_JERICHO(unit) ? 0x2  : 0x1)
#define SOC_DPP_LB_VECTOR_INDEX_ETHERNET_2       (SOC_IS_JERICHO(unit) ? 0x3  : 0x1)
#define SOC_DPP_LB_VECTOR_INDEX_ETHERNET_3       (SOC_IS_JERICHO(unit) ? 0x4  : 0x1)
#define SOC_DPP_LB_VECTOR_INDEX_FC               (SOC_IS_JERICHO(unit) ? 0x5  : 0x2)
#define SOC_DPP_LB_VECTOR_INDEX_IPV4             (SOC_IS_JERICHO(unit) ? 0x6  : 0x3)
#define SOC_DPP_LB_VECTOR_INDEX_IPV6             (SOC_IS_JERICHO(unit) ? 0x7  : 0x4)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX1           (SOC_IS_JERICHO(unit) ? 0x8  : 0x5)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL1    (SOC_IS_JERICHO(unit) ? 0x9  : 0x6)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL2    (SOC_IS_JERICHO(unit) ? 0xa  : 0x7)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL1    (SOC_IS_JERICHO(unit) ? 0xb  : 0x8)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL2    (SOC_IS_JERICHO(unit) ? 0xc  : 0x9)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL3    (SOC_IS_JERICHO(unit) ? 0xd  : 0xa)
#define SOC_DPP_LB_VECTOR_INDEX_MPLSX5_LABEL4    (SOC_IS_JERICHO(unit) ? 0x11 : 0xe)
#define SOC_DPP_LB_VECTOR_INDEX_TRILL            (SOC_IS_JERICHO(unit) ? 0xe  : 0xb)
#define SOC_DPP_LB_VECTOR_INDEX_L4               (SOC_IS_JERICHO(unit) ? 0xf  : 0xc)
#define SOC_DPP_LB_VECTOR_INDEX_FC_VFT           (SOC_IS_JERICHO(unit) ? 0x10 : 0xd)
#define SOC_DPP_LB_VECTOR_INDEX_CALC             (SOC_IS_JERICHO(unit) ? 0x1f : 0xf)
#define SOC_DPP_LB_NOF_VECTOR_INDEXES            (SOC_IS_JERICHO(unit) ? 0x1f : 0xf)


#define ARAD_PP_LB_VECTOR_NOF_BITS 48

/* } */
/*************
 * MACROS    *
 *************/
/* { */
#define ARAD_PP_LB_FIELD_TO_OFFSET(field) (soc_sand_log2_round_down(field))

/*
 * given chunk offset return the start place (lsb to msb) of the 2bits to control this chunk
 */
#define ARAD_PP_LB_FIELD_TO_NIBLE(chunk_ofst, chunk_size)   (ARAD_PP_LB_VECTOR_NOF_BITS - chunk_ofst*2 - 2)

#define ARAD_PP_LB_MPLS_LBL1_VLD(key_indx) \
  ((key_indx== SOC_DPP_LB_VECTOR_INDEX_MPLSX1)||(key_indx== SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL1)||(key_indx==SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL1))

#define ARAD_PP_LB_MPLS_LBL2_VLD(key_indx) \
  ((key_indx >= SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL1) && (key_indx <= SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL2))

#define ARAD_PP_LB_MPLS_LBL3_VLD(key_indx) \
  ((key_indx >= SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL1) && (key_indx <= SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL3))

#define ARAD_PP_LB_MPLS_LBL4_VLD(key_indx) \
  (key_indx == SOC_DPP_LB_VECTOR_INDEX_MPLSX5_LABEL4)

#define ARAD_PP_LB_MPLS_LBL5_VLD(key_indx) \
  (key_indx == SOC_DPP_LB_VECTOR_INDEX_MPLSX5_LABEL4)

#define ARAD_PP_FRWR_ACTION_TO_FEC(fwrd_action)\
  ((0x1FFFF & fwrd_action))


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
	uint32 key_index;
	uint32 nof_nibles;
	uint32 nbls[8];
}ARAD_PP_LB_HASH_FIELD_INFO;

typedef struct
{
	uint32 offset;
	uint32 index5;
	uint32 index4;
	uint32 index3;
	uint32 index2;
	uint32 index1;
}ARAD_PP_LB_HASH_MAP;

typedef struct
{
    uint32 key_index;
    uint8 chunk_size;
}ARAD_PP_LB_HASH_CHUNK_SIZE;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_lag[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_LAG_MEMBER_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_PORT_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_HASHING_MASK_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LAG_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_lag[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'lag_ndx' is out of range. \n\r "
    "The range is: 0 - 255.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_SUCCESS_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_SUCCESS_OUT_OF_RANGE_ERR",
    "The parameter 'success' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_NOF_SUCCESS_FAILURES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_SYS_PORT_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_SYS_PORT_OUT_OF_RANGE_ERR",
    "The parameter 'sys_port' is out of range. \n\r "
    "The range is: 0 - 4095.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_MASKS_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_MASKS_OUT_OF_RANGE_ERR",
    "The parameter 'masks' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_HASH_MASKS-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_SEED_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_SEED_OUT_OF_RANGE_ERR",
    "The parameter 'seed' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U16_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_HASH_FUNC_ID_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_HASH_FUNC_ID_OUT_OF_RANGE_ERR",
    "The parameter 'hash_func_id' is out of range. \n\r "
    "The range is: 3 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_KEY_SHIFT_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_KEY_SHIFT_OUT_OF_RANGE_ERR",
    "The parameter 'key_shift' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U8_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_NOF_HEADERS_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_NOF_HEADERS_OUT_OF_RANGE_ERR",
    "The parameter 'nof_headers' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U8_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_FIRST_HEADER_TO_PARSE_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_FIRST_HEADER_TO_PARSE_OUT_OF_RANGE_ERR",
    "The parameter 'first_header_to_parse' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_LAG_HASH_FRST_HDRS-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_MASK_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_MASK_OUT_OF_RANGE_ERR",
    "The parameter 'mask' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_HASH_MASKSS-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },

  {
    SOC_PPC_LAG_MEMBER_ID_OUT_OF_RANGE_ERR,
    "SOC_PPC_LAG_MEMBER_ID_OUT_OF_RANGE_ERR",
    "The parameter 'member_id' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LAG_NOF_ENTRIES_OUT_OF_RANGE_ERR,
    "ARAD_PP_LAG_NOF_ENTRIES_OUT_OF_RANGE_ERR",
    "The parameter 'nof_entries' is out of range. \n\r "
    "The range is: 0 - 16.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_LAG_LB_TYPE_OUT_OF_RANGE_ERR,
    "SOC_PPC_LAG_LB_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'lb_type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_LAG_LB_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */
   {
    ARAD_PP_LAG_ASYMETRIC_ERR,
    "ARAD_PP_LAG_ASYMETRIC_ERR",
    "When retrieving the LAG configuration, \n\r "
    "The incoming and the outgoing direction\n\r "
    "are configured differently",
    SOC_SAND_SVR_ERR,
    FALSE
  },
   {
    ARAD_PP_LAG_DOUPLICATE_MEMBER_ERR,
    "ARAD_PP_LAG_DOUPLICATE_MEMBER_ERR",
    "define lag with duplicated member",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  

  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* key = {In-Port.Lb-Profile, Parser-Leaf-Context.LB-Profile, Packet-Format-Code (6b)}
   assuming In-Port.Lb-Profile = 0 */
#define ARAD_PP_LB_PFC_OFFSET(_pfc_, _plc_) (_plc_ << 6 | _pfc_)

#define ARAD_PP_PLC_FC_VFT 1
#define ARAD_PP_PLC_FC_NO_VFT 0

CONST STATIC ARAD_PP_LB_HASH_MAP
  Arad_pp_lb_hash_map_table[] =
{
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_CALC,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_ETHERNET,      ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS1_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_CALC,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS1_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS2_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_CALC,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS2_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS3_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_CALC,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_MPLS3_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_MPLS1_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_MPLS2_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_MPLS3_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},

  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS1_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS2_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS3_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},

  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS1_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS2_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_MPLS3_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},

  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS1_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS2_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS3_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS1_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX1,        ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS2_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_MPLS3_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_IPV4_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV4,     ARAD_PP_LB_VECTOR_INDEX_IPV4,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_IPV4_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,   ARAD_PP_LB_VECTOR_INDEX_IPV6,     ARAD_PP_LB_VECTOR_INDEX_IPV4,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_IPV4,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV4_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,   ARAD_PP_LB_VECTOR_INDEX_L4,     ARAD_PP_LB_VECTOR_INDEX_IPV4,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,       ARAD_PP_LB_VECTOR_INDEX_IPV6,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_IPV6_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_L4,       ARAD_PP_LB_VECTOR_INDEX_IPV6,          ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_TRILL_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_TRILL,         ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_IPV4_ETH,0), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_IPV4,         ARAD_PP_LB_VECTOR_INDEX_ETHERNET},
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_ETH_IPV4_ETH,1), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_IPV4, ARAD_PP_LB_VECTOR_INDEX_ETHERNET, ARAD_PP_LB_VECTOR_INDEX_IPV4,         ARAD_PP_LB_VECTOR_INDEX_ETHERNET},

  /* FCOE */
  /* Standard Header: removed as conflicted with Ethernet Vector */
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_FC_STD_ETH,ARAD_PP_PLC_FC_NO_VFT), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_NONE,          ARAD_PP_LB_VECTOR_INDEX_FC, ARAD_PP_LB_VECTOR_INDEX_NONE},
  /* VFT,Standard Header*/
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_FC_STD_ETH,ARAD_PP_PLC_FC_VFT), ARAD_PP_LB_VECTOR_INDEX_NONE,  ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_FC,          ARAD_PP_LB_VECTOR_INDEX_FC_VFT, ARAD_PP_LB_VECTOR_INDEX_NONE},
  /* Encapsulation,VFT,Standard Header */
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_FC_ENCAP_ETH,ARAD_PP_PLC_FC_VFT),  ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_FC,          ARAD_PP_LB_VECTOR_INDEX_FC_VFT, ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE},
  /* Encapsulation, Standard Header*/
  { ARAD_PP_LB_PFC_OFFSET(ARAD_PARSER_PFC_FC_ENCAP_ETH,ARAD_PP_PLC_FC_NO_VFT), ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE,     ARAD_PP_LB_VECTOR_INDEX_FC,         ARAD_PP_LB_VECTOR_INDEX_NONE, ARAD_PP_LB_VECTOR_INDEX_NONE},
};


CONST STATIC ARAD_PP_LB_HASH_FIELD_INFO Arad_pp_lag_hash_field_info []=
{
	/*Indexes in this table matches SOC_PPC_HASH_MASKS_* index*/
	/* Ethernet */
	/* MAC_SA*/
	{ARAD_PP_LB_VECTOR_INDEX_ETHERNET, 6 , {5,6,9,10,13,14}},
	/* MAC_DA*/
	{ARAD_PP_LB_VECTOR_INDEX_ETHERNET, 6 , {3,4,7,8,11,12}},
	/* VSI*/
	{ARAD_PP_LB_VECTOR_INDEX_ETHERNET, 2 , {0,1,0,0,0,0}},
	/* ETH_TYPE_CODE*/
	{ARAD_PP_LB_VECTOR_INDEX_ETHERNET, 1 , {2,0,0,0,0,0}},
	/* MPLS_LABEL_1*/
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX1, 5 , {0,1,2,3,4,0}},
	/* MPLS_LABEL_2*/
	{ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, 5 , {8,9,10,11,12,0}},
	/* MPLS_LABEL_3*/
	{ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, 5 , {16,17,18,19,20,0}},
	/* IPV4_SIP*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV4, 4 , {12,13,14,15,0,0}},
	/* IPV4_DIP*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV4, 4 , {16,17,18,19,0,0}},
	/* IPV4_PROTOCOL*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV4, 1 , {9,0,0,0,0,0}},
	/* IPV6_SIP*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV6, 6 , {0,1,4,5,8,9}},
	/* IPV6_DIP*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV6, 6 , {2,3,6,7,10,11}},
	/* IPV6_PROTOCOL*/
	{ARAD_PP_LB_VECTOR_INDEX_IPV6, 1 , {18,0,0,0,0,0}},
	/* L4_SRC_PORT*/
	{ARAD_PP_LB_VECTOR_INDEX_L4, 2 , {0,1,0,0,0,0}},
	/* L4_DEST_PORT*/
	{ARAD_PP_LB_VECTOR_INDEX_L4, 2 , {2,3,0,0,0,0}},
    /* FC_DEST_ID*/
    {ARAD_PP_LB_VECTOR_INDEX_FC, 3 , {1,2,3,0,0,0}},
    /* FC_SRC_ID*/
    {ARAD_PP_LB_VECTOR_INDEX_FC, 3 , {5,6,7,0,0,0}},
    /* FC_SEQ_ID*/
    {ARAD_PP_LB_VECTOR_INDEX_FC, 1 , {12,0,0,0,0,0}},
    /* FC_ORG_EX_ID*/
    {ARAD_PP_LB_VECTOR_INDEX_FC, 2 , {16,17,0,0,0,0}},
    /* FC_RES_EX_ID*/
    {ARAD_PP_LB_VECTOR_INDEX_FC, 2 , {18,19,0,0,0,0}},
    /* VFI */
    {ARAD_PP_LB_VECTOR_INDEX_FC_VFT, 3 , {5,6,7,0,0,0}},
    /* TRILL_EG_NICK*/
    {ARAD_PP_LB_VECTOR_INDEX_TRILL, 2 , {1,2,0,0,0,0}},
	/* MPLS_Label 4*/
	{ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4, 5, {0,1,2,3,4,0}},
	/* MPLS_Label 5*/
	{ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4, 5, {8,9,10,11,12,0}},
};

CONST STATIC ARAD_PP_LB_HASH_CHUNK_SIZE Arad_pp_lb_key_chunk_size[] = 
{   
    {ARAD_PP_LB_VECTOR_INDEX_NONE         , 0},
    {ARAD_PP_LB_VECTOR_INDEX_ETHERNET     , 1},
    {ARAD_PP_LB_VECTOR_INDEX_FC           , 1},
    {ARAD_PP_LB_VECTOR_INDEX_IPV4         , 1},
    {ARAD_PP_LB_VECTOR_INDEX_IPV6         , 1},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX1       , 0},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1, 0},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL2, 0},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1, 0},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL2, 0},
    {ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL3, 0},
    {ARAD_PP_LB_VECTOR_INDEX_TRILL        , 0},
    {ARAD_PP_LB_VECTOR_INDEX_L4           , 1},
    {ARAD_PP_LB_VECTOR_INDEX_FC_VFT       , 0},
	{ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4, 0},
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  arad_pp_lag_lb_vector_index_map(int unit, uint32 lb_vector_index_id)
{
    uint32 dev_lb_vector_index;

    switch (lb_vector_index_id) {
    case ARAD_PP_LB_VECTOR_INDEX_ETHERNET:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_ETHERNET;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_FC:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_FC;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_IPV4:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_IPV4;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_IPV6:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_IPV6;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX1:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX1;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL1:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL1;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX2_LABEL2:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX2_LABEL2;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL1:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL1;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL2:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL2;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_MPLSX3_LABEL3:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_MPLSX3_LABEL3;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_TRILL:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_TRILL;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_L4:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_L4;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_FC_VFT:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_FC_VFT;
        break;
    case ARAD_PP_LB_VECTOR_INDEX_CALC:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_CALC;
        break;
	case ARAD_PP_LB_VECTOR_INDEX_MPLSX5_LABEL4:
		dev_lb_vector_index= SOC_DPP_LB_VECTOR_INDEX_MPLSX5_LABEL4;
		break;
    case ARAD_PP_LB_VECTOR_INDEX_NONE:
    default:
        dev_lb_vector_index = SOC_DPP_LB_VECTOR_INDEX_NONE;
        break;
    }

    return dev_lb_vector_index;
}

uint32
  arad_pp_lag_hash_func_to_hw_val(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint8         hash_func_id,
    SOC_SAND_OUT  uint32        *hw_val
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch (hash_func_id)
  {
    case SOC_PPC_LAG_LB_KEY:
     *hw_val = 10;
    break;
    case SOC_PPC_LAG_LB_ROUND_ROBIN:
     *hw_val = 11;
    break;
    case SOC_PPC_LAG_LB_2_CLOCK:
     *hw_val = 12;
    break;
    case SOC_PPC_LAG_LB_CRC_0x10861:
    case SOC_PPC_LAG_LB_CRC_0x8003: /* Arad+ */
     *hw_val = 0;
    break;
    case SOC_PPC_LAG_LB_CRC_0x10285:
    case SOC_PPC_LAG_LB_CRC_0x8011: /* Arad+ */
     *hw_val = 1;
    break;
    case SOC_PPC_LAG_LB_CRC_0x101A1:
    case SOC_PPC_LAG_LB_CRC_0x8423: /* Arad+ */
     *hw_val = 2;
    break;
    case SOC_PPC_LAG_LB_CRC_0x12499:
    case SOC_PPC_LAG_LB_CRC_0x8101: /* Arad+ */
     *hw_val = 3;
    break;
    case SOC_PPC_LAG_LB_CRC_0x1F801:
    case SOC_PPC_LAG_LB_CRC_0x84a1: /* Arad+ */
     *hw_val = 4;
    break;
    case SOC_PPC_LAG_LB_CRC_0x172E1:
    case SOC_PPC_LAG_LB_CRC_0x9019: /* Arad+ */
     *hw_val = 5;
    break;
    case SOC_PPC_LAG_LB_CRC_0x1EB21:
     *hw_val = 6;
    break;
    case SOC_PPC_LAG_LB_CRC_0x13965:
     *hw_val = 7;
    break;
    case SOC_PPC_LAG_LB_CRC_0x1698D:
     *hw_val = 8;
    break;
    case SOC_PPC_LAG_LB_CRC_0x1105D:
     *hw_val = 9;
    break;

    case SOC_PPC_LAG_LB_CRC_0x14D:
    case SOC_PPC_LAG_LB_CRC_0x1C3:
    case SOC_PPC_LAG_LB_CRC_0x1CF:
  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LAG_HASH_FUNC_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hash_func_to_hw_val()", 0, 0);
}

uint32
  arad_pp_lag_hash_func_from_hw_val(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint32        hw_val,
    SOC_SAND_OUT  uint8        *hash_func_id
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch (hw_val)
  {
    case 10:
     *hash_func_id = SOC_PPC_LAG_LB_KEY;
    break;
    case 11:
     *hash_func_id = SOC_PPC_LAG_LB_ROUND_ROBIN;
    break;
    case 12:
     *hash_func_id = SOC_PPC_LAG_LB_2_CLOCK;
    break;
    case 0:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x8003;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x10861;
        }
    break;
    case 1:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x8011;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x10285;
        }
    break;
    case 2:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x8423;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x101A1;
        }
    break;
    case 3:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x8101;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x12499;
        }
    break;
    case 4:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x84a1;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x1F801;
        }
    break;
    case 5:
#ifdef BCM_88660_A0
        /* Supported polynom after Arad plus */
        if (SOC_IS_ARADPLUS(unit)) {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x9019;
        }
        else
#endif /* BCM_88660_A0 */
        {
            *hash_func_id = SOC_PPC_LAG_LB_CRC_0x172E1;
        }
    break;
    case 6:
     *hash_func_id = SOC_PPC_LAG_LB_CRC_0x1EB21;
    break;
    case 7:
     *hash_func_id = SOC_PPC_LAG_LB_CRC_0x13965;
    break;
    case 8:
     *hash_func_id = SOC_PPC_LAG_LB_CRC_0x1698D;
    break;
    case 9:
     *hash_func_id = SOC_PPC_LAG_LB_CRC_0x1105D;
    break;
  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LAG_HASH_FUNC_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hash_func_to_hw_val()", 0, 0);
}
/*********************************************************************
*     Add nof_entries to the IHB_LB_PROFILE_TBL from the
*     lb_hash_map_table array
*********************************************************************/
uint32
  arad_pp_lag_set_pfc_profile_tbl(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  nof_entries,
    SOC_SAND_IN  ARAD_PP_LB_HASH_MAP  *lb_hash_map_table
  )
{
  uint32
    idx;
  ARAD_PP_IHB_LB_PFC_PROFILE_TBL_DATA
    ihb_lb_pfc_profile_tbl;
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (idx = 0; idx < nof_entries; ++idx)
  {
      ihb_lb_pfc_profile_tbl.lb_vector_index_1   = arad_pp_lag_lb_vector_index_map(unit, lb_hash_map_table[idx].index1);
      ihb_lb_pfc_profile_tbl.lb_vector_index_2   = arad_pp_lag_lb_vector_index_map(unit, lb_hash_map_table[idx].index2);
      ihb_lb_pfc_profile_tbl.lb_vector_index_3   = arad_pp_lag_lb_vector_index_map(unit, lb_hash_map_table[idx].index3);
      ihb_lb_pfc_profile_tbl.lb_vector_index_4   = arad_pp_lag_lb_vector_index_map(unit, lb_hash_map_table[idx].index4);
      ihb_lb_pfc_profile_tbl.lb_vector_index_5   = arad_pp_lag_lb_vector_index_map(unit, lb_hash_map_table[idx].index5);

      res = arad_pp_ihb_lb_pfc_profile_tbl_set_unsafe(
                unit,
                lb_hash_map_table[idx].offset,
                &ihb_lb_pfc_profile_tbl);

      SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_set_pfc_profile_tbl()", 0, 0);
}



uint32
  arad_pp_lag_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  int
    port_ndx,
    core_id,
    nof_pp_ports_per_core;
  SOC_PPC_HASH_MASK_INFO
    hash_info;
  ARAD_PP_IHB_PINFO_FER_TBL_DATA
    ihb_pinfo_fer_tbl_data;
  uint32
    res = SOC_SAND_OK,
    hw_val;
  SHR_BITDCL
    plc_profile_bitmap = 0;
  uint64
    reg_64;
  uint64
    field64_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_PCID_LITE_SKIP(unit);


  res = arad_pp_lag_set_pfc_profile_tbl(unit, sizeof(Arad_pp_lb_hash_map_table) / sizeof(ARAD_PP_LB_HASH_MAP), Arad_pp_lb_hash_map_table);
  SOC_SAND_CHECK_FUNC_RESULT(res, 19, exit);

  SOC_PPC_HASH_MASK_INFO_clear(&hash_info);
  hash_info.mask = 0;

  res = arad_pp_lag_hashing_mask_set_unsafe(
          unit,
          &hash_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 21, exit);

  /* set Hashing = no-hashing for stacking ports (profile 1) */
#ifdef BCM_88675_A0
  if(SOC_IS_JERICHO(unit)) {

    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR, READ_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_64));

    COMPILER_64_SET(field64_val, 0x0, 0);
    ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEXf, field64_val, reg_64, 71, exit);
    /* Set the second LAG polynomial to an invalid value (0x6) as this feature isn't supported yet */
    COMPILER_64_SET(field64_val, 0x0, 0x6);
    ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEX_1f, field64_val, reg_64, 72, exit);

    COMPILER_64_SET(field64_val, 0x0, SOC_PPC_MPLS_TERM_RESERVED_LABEL_ELI);
    ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LB_MPLS_ELI_LABELf, field64_val, reg_64, 73, exit);

    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 77,  exit, ARAD_REG_ACCESS_ERR, WRITE_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, reg_64));
  } else
#endif /* BCM_88675_A0 */
  if(SOC_IS_ARAD_B0_AND_ABOVE(unit)) {

    uint32 reg_val;

    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, ARAD_REG_ACCESS_ERR,READ_IHB_REG_0090r(unit, &reg_val));

    ARAD_FLD_TO_REG(IHB_REG_0090r, ITEM_0_7f, 0xf0 , reg_val, 62, exit);
    ARAD_FLD_TO_REG(IHB_REG_0090r, ITEM_8_11f, 0xa, reg_val, 64, exit);

    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  66,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHB_REG_0090r(unit, reg_val));
  }

  /*
   * Starting from Jericho B0 the SLB can use the configured LB result as a seed, as
   * this option is enabled by default it changes the seed automatically to any SLB
   * users (disabled here for backward compatibility).
   */
  if(SOC_IS_JERICHO_B0(unit) || SOC_IS_QMX_B0(unit) || SOC_IS_QAX_B0(unit)) {
      arad_pp_lag_hashing_ecmp_hash_slb_combine_set(unit, FALSE);
  }

  /* The following PLC's are configured with (Parser-Leaf-Context.LB-Profile=1)*/
  SHR_BITSET(&plc_profile_bitmap, ARAD_PARSER_PLC_MPLS_5);
  SHR_BITSET(&plc_profile_bitmap, ARAD_PARSER_PLC_FCOE_VFT);
  SHR_BITSET(&plc_profile_bitmap, ARAD_PARSER_PLC_GAL_GACH_BFD);
  SHR_BITSET(&plc_profile_bitmap, ARAD_PARSER_PLC_PP_L4);
  if (((SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop) || (SOC_DPP_CONFIG(unit)->pp.bfd_ipv6_enable == SOC_DPP_ARAD_BFD_IPV6_SUPPORT_WITH_LEM)) ){
      SHR_BITSET(&plc_profile_bitmap, ARAD_PARSER_PLC_BFD_SINGLE_HOP);
  }


  /*
   * Unlike all the other DNX devices Jericho+ can support 0-3 ECMP headers (instead of 1-2) and the LAG starting header
   * field is now also controls the ECMP starting header.
   * As the PINFO_FER register is initialize with zeroes the NOF ECMP headers in JR+ will be 0 and the starting header will
   * be one header below the forwarding.
   * For backward computability we're setting the NOF ECMP headers to 1 and the starting header to the forwarding header
   * (note that the LAG will be now start by default in the forwarding and not in the header below as it was).
   */
  if(SOC_IS_JERICHO_PLUS_A0(unit) || SOC_IS_QUX(unit)) {
      nof_pp_ports_per_core = SOC_DPP_DEFS_GET(unit, nof_pp_ports_per_core);
      SOC_DPP_CORES_ITER(SOC_CORE_ALL, core_id) {
          for (port_ndx = 0; port_ndx < nof_pp_ports_per_core; port_ndx++) {
              res = arad_pp_ihb_pinfo_fer_tbl_get_unsafe(unit, core_id, port_ndx, &ihb_pinfo_fer_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
              ihb_pinfo_fer_tbl_data.ecmp_lb_key_count = 1;
              ihb_pinfo_fer_tbl_data.lag_lb_key_start  = 1;

              res = arad_pp_ihb_pinfo_fer_tbl_set_unsafe(unit, core_id, port_ndx, &ihb_pinfo_fer_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 71, exit);
          }
      }
      /* Set the second hierarchy polynomial of the configured load balancing. */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  72,  exit, ARAD_REG_ACCESS_ERR, READ_IHB_ECMP_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_64));
      hw_val = arad_pp_frwrd_fec_hash_index_to_hw_val(SOC_PPC_FEC_LB_CRC_0x8101);
      COMPILER_64_SET(field64_val, 0x0, hw_val);
      ARAD_FLD_TO_REG64(IHB_ECMP_LB_KEY_CFGr, ECMP_2ND_HIER_LB_HASH_INDEXf, field64_val, reg_64, 74, exit);
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 75,  exit, ARAD_REG_ACCESS_ERR, WRITE_IHB_ECMP_LB_KEY_CFGr(unit, SOC_CORE_ALL, reg_64));
  }

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, WRITE_IHB_LB_KEY_PARSER_LEAF_CONTEXT_PROFILEr(unit, REG_PORT_ANY, plc_profile_bitmap));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_init_unsafe()", 0, 0);
}



/*********************************************************************
 *     Initialize polynomial indexes for TM mode.
 *     Details: When working in TM mode only the LAG polynomial is in
 *     used and shouldn't be set the same way as other hashing indexes.
 *     This function sets all the polynomial to unique values to prevent
 *     confliction between them.
*********************************************************************/
uint32
    arad_pp_lag_init_polynomial_for_tm_mode(
    SOC_SAND_IN int unit
  )
{
  uint32 res = SOC_SAND_OK;
  uint64 reg_64, field64_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_PCID_LITE_SKIP(unit);

/*
 * LAG
 * Set the first LAG polynomial into a valid value and the second LAG polynomial into a none valid value as it's not supported.
 */
 SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR, READ_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_64));
 COMPILER_64_SET(field64_val, 0x0, 0);
 ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEXf, field64_val, reg_64, 71, exit);
 if (SOC_IS_JERICHO(unit)) {
     COMPILER_64_SET(field64_val, 0x0, 0x6);
     ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEX_1f, field64_val, reg_64, 72, exit);
 }
 SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 71,  exit, ARAD_REG_ACCESS_ERR, WRITE_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, reg_64));


  /*
   * ECMP
   * The ECMP isn't used in TM mode so just set both ECMP hierarchies into an invalid value (They
   * are valid in ARAD+ and below devices and in case needed they should be changed by the switch as in PP mode).
   */
 SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  72,  exit, ARAD_REG_ACCESS_ERR, READ_IHB_ECMP_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_64));
 COMPILER_64_SET(field64_val, 0x0, 0x7);
 ARAD_FLD_TO_REG64(IHB_ECMP_LB_KEY_CFGr, ECMP_LB_HASH_INDEXf, field64_val, reg_64, 74, exit);
 if (SOC_IS_JERICHO_PLUS_A0(unit) || SOC_IS_QUX(unit)) {
     COMPILER_64_SET(field64_val, 0x0, 0x8);
     ARAD_FLD_TO_REG64(IHB_ECMP_LB_KEY_CFGr, ECMP_2ND_HIER_LB_HASH_INDEXf, field64_val, reg_64, 74, exit);
 }
 SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 73,  exit, ARAD_REG_ACCESS_ERR, WRITE_IHB_ECMP_LB_KEY_CFGr(unit, SOC_CORE_ALL, reg_64));

 /*
  * Consistent
  * Not used in TM mode, set into an invalid value (this value is valid in ARAD+ and below devices and
  * in case needed it should be changed by the switch as in PP mode).
  */
 if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
     SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  74,  exit, ARAD_REG_ACCESS_ERR, READ_IHB_CONSISTENT_HASHING_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_64));
     COMPILER_64_SET(field64_val, 0x0, 0x9);
     ARAD_FLD_TO_REG64(IHB_CONSISTENT_HASHING_LB_KEY_CFGr, CONSISTENT_HASHING_LAG_LB_HASH_INDEXf, field64_val, reg_64, 74, exit);
     SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 75,  exit, ARAD_REG_ACCESS_ERR, WRITE_IHB_CONSISTENT_HASHING_LB_KEY_CFGr(unit, SOC_CORE_ALL, reg_64));
 }

 exit:
     SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_init_polynomial_for_tm_mode()", 0, 0);
}


STATIC void
  arad_pp_lag_members_ppd2tm(
    SOC_SAND_IN   SOC_PPC_LAG_INFO        *soc_ppd_lag,
    SOC_SAND_OUT  SOC_PPC_LAG_INFO  *tm_lag
  )
{
  uint32
    entry_idx;

  tm_lag->soc_sand_magic_num = SOC_SAND_MAGIC_NUM_VAL;
  tm_lag->nof_entries = soc_ppd_lag->nof_entries;

  for (entry_idx = 0; entry_idx < soc_ppd_lag->nof_entries; entry_idx++)
  {
    tm_lag->members[entry_idx].member_id = soc_ppd_lag->members[entry_idx].member_id;
    tm_lag->members[entry_idx].sys_port  = soc_ppd_lag->members[entry_idx].sys_port;
    tm_lag->members[entry_idx].flags     = soc_ppd_lag->members[entry_idx].flags;
  }
}

STATIC void
  arad_pp_lag_members_tm2ppd(
    SOC_SAND_IN  SOC_PPC_LAG_INFO  *tm_lag,
    SOC_SAND_OUT SOC_PPC_LAG_INFO        *soc_ppd_lag
  )
{
  uint32
    entry_idx;

  soc_ppd_lag->nof_entries = tm_lag->nof_entries;

  for (entry_idx = 0; entry_idx < tm_lag->nof_entries; entry_idx++)
  {
    soc_ppd_lag->members[entry_idx].member_id = tm_lag->members[entry_idx].member_id;
    soc_ppd_lag->members[entry_idx].sys_port = tm_lag->members[entry_idx].sys_port;
  }
}


 /*********************************************************************
 *     Configure a LAG. A LAG is defined by a group of System
 *     Physical Ports that compose it. This configuration
 *     affects 1. LAG resolution: when the destination of
 *     packet is LAG 2. Learning: when packet source port
 *     belongs to LAG, then the LAG is learnt.
 *     Details: in the H file. (search for prototype)
 *********************************************************************/
uint32
  arad_pp_lag_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO                            *lag_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA
    lag2lag_rng_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(lag_info);

  /*
   * Set Mode (Hash / RR / Smooth Division)
   */
  res = arad_irr_lag_to_lag_range_tbl_get_unsafe(unit, lag_ndx, &lag2lag_rng_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* 
   * 0x0 - Multiplication mode.
   * 0x1 - Round-Robin mode.
   * 0x2 - Smooth division mode: Jericho only. 
   */
  switch (lag_info->lb_type) {
  case SOC_PPC_LAG_LB_TYPE_HASH:
      lag2lag_rng_tbl.mode = 0x0;
      break;
  case SOC_PPC_LAG_LB_TYPE_ROUND_ROBIN:
      lag2lag_rng_tbl.mode = 0x1;
      break;
  case SOC_PPC_LAG_LB_TYPE_SMOOTH_DIVISION:
      lag2lag_rng_tbl.mode = 0x2;
      break;
  default:
      lag2lag_rng_tbl.mode = 0x3; /* invalid */
      break;
  }

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
    lag2lag_rng_tbl.is_stateful = lag_info->is_stateful;
  }
#endif /* BCM_88660_A0 */

  res = arad_irr_lag_to_lag_range_tbl_set_unsafe(unit, lag_ndx, &lag2lag_rng_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /************************************************************************/
  /* Set Members                                                          */
  /************************************************************************/
  res = arad_ports_lag_set_unsafe(unit, lag_ndx, lag_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_set_unsafe()", lag_ndx, 0);
}

uint32
  arad_pp_lag_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO                          *lag_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(lag_ndx, ARAD_PP_LAG_LAG_NDX_MAX(unit), ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR, 10, exit);
  res = SOC_PPC_LAG_INFO_verify(unit,lag_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_set_verify()", lag_ndx, 0);
}

uint32
  arad_pp_lag_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(lag_ndx, ARAD_PP_LAG_LAG_NDX_MAX(unit), ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_get_verify()", lag_ndx, 0);
}

uint32
  arad_pp_lag_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_INFO                            *lag_info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_LAG_INFO
    *tm_lag_info_in = NULL,
    *tm_lag_info_out = NULL;
  ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA
    lag2lag_rng_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(lag_info);

  SOC_PPC_LAG_INFO_clear(lag_info);

  /*
   * Get Mode (Hash/RR/Smooth Division)
   */
  res = arad_irr_lag_to_lag_range_tbl_get_unsafe(
          unit,
          lag_ndx,
          &lag2lag_rng_tbl
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  switch (lag2lag_rng_tbl.mode) {
  case 0:
      lag_info->lb_type = SOC_PPC_LAG_LB_TYPE_HASH;
      break;
  case 1:
      lag_info->lb_type = SOC_PPC_LAG_LB_TYPE_ROUND_ROBIN;
      break;
  case 2:
      lag_info->lb_type = SOC_PPC_LAG_LB_TYPE_SMOOTH_DIVISION;
      break;
  }

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
    lag_info->is_stateful = lag2lag_rng_tbl.is_stateful;
  }
#endif /* BCM_88660_A0 */

  /************************************************************************/
  /* Get Members                                                          */
  /************************************************************************/

  ARAD_ALLOC(tm_lag_info_in, SOC_PPC_LAG_INFO, 1, "arad_pp_lag_get_unsafe.tm_lag_info_in");
  ARAD_ALLOC(tm_lag_info_out, SOC_PPC_LAG_INFO, 1, "arad_pp_lag_get_unsafe.tm_lag_info_out");

  res = arad_ports_lag_get_unsafe(
          unit,
          lag_ndx,
          tm_lag_info_in,
          tm_lag_info_out
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  arad_pp_lag_members_tm2ppd(tm_lag_info_out, lag_info);
  
exit:
  ARAD_FREE(tm_lag_info_in);
  ARAD_FREE(tm_lag_info_out);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_get_unsafe()", lag_ndx, 0);
}

/*********************************************************************
*     Add a system port as a member in LAG.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_member_add_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  uint32                             lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                 *member,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PORTS_LAG_MEMBER
    lag_member;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LAG_MEMBER_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(member);
  SOC_SAND_CHECK_NULL_INPUT(success);

  arad_ARAD_PORTS_LAG_MEMBER_clear(&lag_member);
  lag_member.member_id = member->member_id;
  lag_member.sys_port = member->sys_port;
  lag_member.flags    = member->flags;

  res = arad_ports_lag_member_add_unsafe( unit, lag_ndx, &lag_member );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *success = SOC_SAND_SUCCESS;
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_member_add_unsafe()", lag_ndx, 0);
}

uint32
  arad_pp_lag_member_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                          *member
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LAG_MEMBER_ADD_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(lag_ndx, ARAD_PP_LAG_LAG_NDX_MAX(unit), ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LAG_MEMBER, member, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_member_add_verify()", lag_ndx, 0);
}

/*********************************************************************
*     Remove a system port from a LAG.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_member_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                     *member
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PORTS_LAG_MEMBER
    lag_member;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LAG_MEMBER_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(member);

  arad_ARAD_PORTS_LAG_MEMBER_clear(&lag_member);
  lag_member.member_id = member->member_id;
  lag_member.sys_port = member->sys_port;
  lag_member.flags    = member->flags;

  res = arad_ports_lag_sys_port_remove_unsafe( unit, lag_ndx, &lag_member );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_member_remove_unsafe()", lag_ndx, 0);
}

uint32
  arad_pp_lag_member_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                     *member
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_LAG_MEMBER_REMOVE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(lag_ndx, ARAD_PP_LAG_LAG_NDX_MAX(unit), ARAD_PP_LAG_LAG_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(member->sys_port, ARAD_PP_LAG_SYS_PORT_MAX, ARAD_PP_LAG_SYS_PORT_OUT_OF_RANGE_ERR, 20, exit);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LAG_MEMBER, member, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_member_remove_verify()", lag_ndx, 0);
}

/*********************************************************************
*     Set the LAG hashing global attributes
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_hashing_global_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info
  )
{
  uint64
    reg_val;
  uint64
      fld64_val;
  uint32
    fld_val;
  uint32
      res = SOC_SAND_OK;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET_UNSAFE);
  SOC_SAND_CHECK_NULL_INPUT(glbl_hash_info);
  

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, ARAD_REG_ACCESS_ERR,READ_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_val));

  COMPILER_64_SET(fld64_val, 0, glbl_hash_info->seed);  
  ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_KEY_SEEDf, fld64_val, reg_val, 10, exit);

  COMPILER_64_SET(fld64_val, 0, SOC_SAND_BOOL2NUM(glbl_hash_info->use_port_id));  
  ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_KEY_USE_IN_PORTf, fld64_val, reg_val, 14, exit);

  res = arad_pp_lag_hash_func_to_hw_val(unit,glbl_hash_info->hash_func_id,&fld_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  if (glbl_hash_info->hash_func_id != SOC_PPC_LAG_LB_KEY) {
      res = arad_pp_frwrd_fec_unique_polynomial_check(unit, fld_val, ARAD_PP_FRWRD_FEC_HASH_INDEX_LAG);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);
  }

  COMPILER_64_SET(fld64_val, 0, fld_val);  
  ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEXf, fld64_val, reg_val, 16, exit);

  COMPILER_64_SET(fld64_val, 0, glbl_hash_info->key_shift);  
  ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_KEY_SHIFTf, fld64_val, reg_val, 18, exit);

  if(SOC_IS_JERICHO(unit)) {
      COMPILER_64_SET(fld64_val, 0, glbl_hash_info->eli_search);
      ARAD_FLD_TO_REG64(IHB_LAG_LB_KEY_CFGr, LB_MPLS_ELI_LABEL_SEARCHf, fld64_val, reg_val, 22, exit);
  }

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL,  reg_val));

 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_global_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_lag_hashing_global_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_GLOBAL_INFO_SET_VERIFY);

  res = SOC_PPC_LAG_HASH_GLOBAL_INFO_verify(unit, glbl_hash_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_global_info_set_verify()", 0, 0);
}

uint32
  arad_pp_lag_hashing_global_info_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_global_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the LAG hashing global attributes
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_hashing_global_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info
  )
{
  uint64
    reg_val;
  uint64
    fld_val;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_GLOBAL_INFO_GET_UNSAFE);
  SOC_SAND_CHECK_NULL_INPUT(glbl_hash_info);
  SOC_PPC_LAG_HASH_GLOBAL_INFO_clear(glbl_hash_info);

  

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, ARAD_REG_ACCESS_ERR,READ_IHB_LAG_LB_KEY_CFGr(unit, SOC_CORE_ALL, &reg_val));

  glbl_hash_info->seed = soc_reg64_field32_get(unit, IHB_LAG_LB_KEY_CFGr, reg_val, LAG_LB_KEY_SEEDf);

  ARAD_FLD_FROM_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_KEY_USE_IN_PORTf, fld_val, reg_val, 14, exit);
  glbl_hash_info->use_port_id = SOC_SAND_NUM2BOOL(COMPILER_64_LO(fld_val));

  ARAD_FLD_FROM_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_HASH_INDEXf, fld_val, reg_val, 16, exit);

  res = arad_pp_lag_hash_func_from_hw_val(unit,COMPILER_64_LO(fld_val), &glbl_hash_info->hash_func_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);
   
  ARAD_FLD_FROM_REG64(IHB_LAG_LB_KEY_CFGr, LAG_LB_KEY_SHIFTf, fld_val, reg_val, 18, exit);
  glbl_hash_info->key_shift = (uint8)COMPILER_64_LO(fld_val);

  if(SOC_IS_JERICHO(unit)) {
      ARAD_FLD_FROM_REG64(IHB_LAG_LB_KEY_CFGr, LB_MPLS_ELI_LABEL_SEARCHf, fld_val, reg_val, 22, exit);
      glbl_hash_info->eli_search = (uint8)COMPILER_64_LO(fld_val);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_global_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set the LAG hashing per-lag attributes
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_hashing_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IHB_PINFO_FER_TBL_DATA
    pinfo_fer_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_PORT_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(lag_hash_info);

  res = arad_pp_ihb_pinfo_fer_tbl_get_unsafe(
          unit,
          core_id,
          port_ndx,
          &pinfo_fer_tbl
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  pinfo_fer_tbl.lag_lb_key_start = (lag_hash_info->first_header_to_parse == SOC_PPC_LAG_HASH_FRST_HDR_FARWARDING)?0x1:0x0;
  pinfo_fer_tbl.lag_lb_key_count = lag_hash_info->nof_headers;
  pinfo_fer_tbl.lb_bos_search = SOC_SAND_BOOL2NUM(lag_hash_info->start_from_bos);
  pinfo_fer_tbl.lb_include_bos_hdr = SOC_SAND_BOOL2NUM(lag_hash_info->include_bos);

  res = arad_pp_ihb_pinfo_fer_tbl_set_unsafe(
          unit,
		  core_id,
          port_ndx,
          &pinfo_fer_tbl
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_port_info_set_unsafe()", port_ndx, 0);
}

uint32
  arad_pp_lag_hashing_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_PORT_INFO_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_LAG_HASH_PORT_INFO, lag_hash_info, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_port_info_set_verify()", port_ndx, 0);
}

uint32
  arad_pp_lag_hashing_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_PORT_INFO_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_port_info_get_verify()", port_ndx, 0);
}

/*********************************************************************
*     Set the LAG hashing per-lag attributes
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_hashing_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IHB_PINFO_FER_TBL_DATA
    pinfo_fer_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_PORT_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(lag_hash_info);

  SOC_PPC_LAG_HASH_PORT_INFO_clear(lag_hash_info);

  res = arad_pp_ihb_pinfo_fer_tbl_get_unsafe(
          unit,
          core_id,
          port_ndx,
          &pinfo_fer_tbl
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  lag_hash_info->first_header_to_parse = (pinfo_fer_tbl.lag_lb_key_start == 0x1)?SOC_PPC_LAG_HASH_FRST_HDR_FARWARDING:SOC_PPC_LAG_HASH_FRST_HDR_LAST_TERMINATED;
  lag_hash_info->nof_headers = (uint8)pinfo_fer_tbl.lag_lb_key_count;
  lag_hash_info->start_from_bos = SOC_SAND_NUM2BOOL(pinfo_fer_tbl.lb_bos_search);
  lag_hash_info->include_bos = SOC_SAND_NUM2BOOL(pinfo_fer_tbl.lb_include_bos_hdr);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_port_info_get_unsafe()", port_ndx, 0);
}

/*********************************************************************
*     Set the port lb_profile.
*********************************************************************/

int
  arad_pp_lag_hashing_port_lb_profile_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  int                            core,
    SOC_SAND_IN  uint32                         pp_port,
    SOC_SAND_IN  uint32                         lb_profile
  )
{
    ARAD_PP_IHB_PINFO_FER_TBL_DATA pinfo_fer_tbl;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_ihb_pinfo_fer_tbl_get_unsafe(
          unit,
		  core,
          pp_port,
          &pinfo_fer_tbl
        ));

    pinfo_fer_tbl.lb_profile = lb_profile;

    SOCDNX_IF_ERR_EXIT(arad_pp_ihb_pinfo_fer_tbl_set_unsafe(
          unit,
          core,
          pp_port,
          &pinfo_fer_tbl
        ));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
arad_pp_lag_hashing_ecmp_hash_slb_combine_set(
   int            unit,
   int            combine_slb
)
{
uint32
    rv, reg_val, fld_val;

	SOCDNX_INIT_FUNC_DEFS;

	reg_val = fld_val = 0;

	if (!SOC_IS_JERICHO_B0_AND_ABOVE(unit)) {
		SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Combination of SLB and configured LB keys isn't supported on this device.")));
	}

	rv = soc_reg32_get(unit, IHB_RESERVED_SPARE_0r, REG_PORT_ANY, 0, &reg_val);
	SOCDNX_IF_ERR_EXIT(rv);

	fld_val = soc_reg_field_get(unit, IHB_RESERVED_SPARE_0r, reg_val, RESERVED_SPARE_0f);

	if (combine_slb) {
	    fld_val |= 0x02;
	} else {
	    fld_val &= 0xFFFFFFFD;
	}

	rv = soc_reg_above_64_field32_modify(unit, IHB_RESERVED_SPARE_0r, REG_PORT_ANY, 0, RESERVED_SPARE_0f, fld_val);
	SOCDNX_IF_ERR_EXIT(rv);

exit:
	SOCDNX_FUNC_RETURN;
}

uint32
arad_pp_lag_hashing_ecmp_hash_slb_combine_get(
   int            unit,
   int            *combine_slb
)
{
uint32
	rv, reg_val, fld_val;

	SOCDNX_INIT_FUNC_DEFS;
	SOCDNX_NULL_CHECK(combine_slb);

	reg_val = fld_val = 0;

	if (!SOC_IS_JERICHO_B0_AND_ABOVE(unit)) {
		SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Combination of SLB and configured LB keys isn't supported on this device.")));
	}

	rv = soc_reg32_get(unit, IHB_RESERVED_SPARE_0r, REG_PORT_ANY, 0, &reg_val);
	SOCDNX_IF_ERR_EXIT(rv);

	fld_val = soc_reg_field_get(unit, IHB_RESERVED_SPARE_0r, reg_val, RESERVED_SPARE_0f);

	*combine_slb = (fld_val & 0x02) ? 1 : 0;

exit:
	SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Mask / unmask fields from the packet header. Masked
 *     fields are ignored by the hashing function
 *     Details: in the H file. (search for prototype)
*********************************************************************/
STATIC SOC_PPC_HASH_MASKS
  arad_pp_lag_hash_sym_peer(
    SOC_SAND_IN  SOC_PPC_HASH_MASKS            field
  )
{
	switch (field)
	{
  /* first part of symmetric, XOR + 2*/
	case SOC_PPC_HASH_MASKS_MAC_DA:
    return SOC_PPC_HASH_MASKS_MAC_SA;
	case SOC_PPC_HASH_MASKS_IPV6_SIP:
    return SOC_PPC_HASH_MASKS_IPV6_DIP;
  case SOC_PPC_HASH_MASKS_FC_ORG_EX_ID:
		return SOC_PPC_HASH_MASKS_FC_RES_EX_ID;
  /* first part of symmetric, XOR + 4*/
	case SOC_PPC_HASH_MASKS_IPV4_SIP:
    return SOC_PPC_HASH_MASKS_IPV4_DIP;
	case SOC_PPC_HASH_MASKS_FC_DEST_ID:
		return SOC_PPC_HASH_MASKS_FC_SRC_ID;
  case SOC_PPC_HASH_MASKS_L4_SRC_PORT:
    return SOC_PPC_HASH_MASKS_L4_DEST_PORT;
  /* first part of symmetric, XOR + 2*/
	case SOC_PPC_HASH_MASKS_MAC_SA:
    return SOC_PPC_HASH_MASKS_MAC_DA;
	case SOC_PPC_HASH_MASKS_IPV6_DIP:
    return SOC_PPC_HASH_MASKS_IPV6_SIP;
  case SOC_PPC_HASH_MASKS_FC_RES_EX_ID:
		return SOC_PPC_HASH_MASKS_FC_ORG_EX_ID;
  /* first part of symmetric, XOR + 4*/
	case SOC_PPC_HASH_MASKS_IPV4_DIP:
    return SOC_PPC_HASH_MASKS_IPV4_SIP;
	case SOC_PPC_HASH_MASKS_FC_SRC_ID:
		return SOC_PPC_HASH_MASKS_FC_DEST_ID;
  case SOC_PPC_HASH_MASKS_L4_DEST_PORT:
    return SOC_PPC_HASH_MASKS_L4_SRC_PORT;
  /* not part of symmetric: just take as is*/
	case SOC_PPC_HASH_MASKS_ETH_TYPE_CODE:
	case SOC_PPC_HASH_MASKS_VSI:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_1:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_2:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_3:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_4:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_5:
	case SOC_PPC_HASH_MASKS_IPV4_PROTOCOL:
	case SOC_PPC_HASH_MASKS_FC_SEQ_ID:
	default:
		return field;
	}

}


/*********************************************************************
*     Mask / unmask fields from the packet header. Masked
 *     fields are ignored by the hashing function
 *     Details: in the H file. (search for prototype)
*********************************************************************/
STATIC uint32
  arad_pp_lag_calc_nible_val(
    SOC_SAND_IN  SOC_PPC_HASH_MASKS            field,
    SOC_SAND_IN  uint8                   enable,
    SOC_SAND_IN  uint8                   peer_enable, /* second part for symmertic */
    SOC_SAND_IN  uint8                   is_symmetric
  )
{
	/* ignored */
  if (!enable)
	{
		return 0;
	}
  /* enabled and not symmetric */
	if (!is_symmetric)
	{
		return 1;
	}
  /* symmetric & enabled but peer is not enable */
  if (!peer_enable)
  {
    return 1; /* just enabled */
  }
	switch (field)/* set symmetric configuration */
	{
  /* first part of symmetric, XOR + 2*/
	case SOC_PPC_HASH_MASKS_MAC_DA:
	case SOC_PPC_HASH_MASKS_IPV6_SIP:
	case SOC_PPC_HASH_MASKS_L4_SRC_PORT:
	case SOC_PPC_HASH_MASKS_FC_ORG_EX_ID:
		return 2;
  /* first part of symmetric, XOR + 4*/
	case SOC_PPC_HASH_MASKS_IPV4_SIP:
	case SOC_PPC_HASH_MASKS_FC_DEST_ID:
		return 3;
  /* second part of symmetric, ignore*/
  case SOC_PPC_HASH_MASKS_MAC_SA:
  case SOC_PPC_HASH_MASKS_IPV4_DIP:
  case SOC_PPC_HASH_MASKS_IPV6_DIP:
  case SOC_PPC_HASH_MASKS_L4_DEST_PORT:
  case SOC_PPC_HASH_MASKS_FC_SRC_ID:
  case SOC_PPC_HASH_MASKS_FC_RES_EX_ID:
    return 0;
  /* not part of symmetric: just take as is*/
	case SOC_PPC_HASH_MASKS_ETH_TYPE_CODE:
	case SOC_PPC_HASH_MASKS_VSI:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_1:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_2:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_3:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_4:
	case SOC_PPC_HASH_MASKS_MPLS_LABEL_5:
	case SOC_PPC_HASH_MASKS_IPV4_PROTOCOL:
	case SOC_PPC_HASH_MASKS_FC_SEQ_ID:
	default:
		return 1;
	}

}


uint32
  arad_pp_lag_hashing_mask_set_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO  *mask_info
  )
{
  uint32
   mask_idx,
   map_index,
   vc_indx,
   fld_val,
   field_nbl_indx,
   field_nof_nbls,
   field_info_offset,
   is_equal,
   nible_val;
  ARAD_PP_IHB_LB_VECTOR_PROGRAM_MAP_TBL_DATA
    lb_vector_program_map_tbl_data,
    old_val;
  uint8
    is_symmetric = FALSE,
    skip;
  SOC_PPC_HASH_MASKS
    masks,
    fld,
    peer_fld;
  uint32
    chunk_bitmap[2] = {0};
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_MASK_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mask_info);

  masks = mask_info->mask;

  is_symmetric = mask_info->is_symmetric_key;

  /*Iterate over vector map table, update relevant vectors*/
  for (vc_indx = 0; vc_indx < sizeof(Arad_pp_lb_key_chunk_size) / sizeof(ARAD_PP_LB_HASH_CHUNK_SIZE); ++vc_indx)
  {
    map_index = arad_pp_lag_lb_vector_index_map(unit, Arad_pp_lb_key_chunk_size[vc_indx].key_index);

    /*
     * Get existing vector map
     */
    res = arad_pp_ihb_lb_vector_program_map_tbl_get_unsafe(
            unit,
            map_index,
            &lb_vector_program_map_tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 19, exit);

    ARAD_COPY(&old_val,&lb_vector_program_map_tbl_data,ARAD_PP_IHB_LB_VECTOR_PROGRAM_MAP_TBL_DATA,1);

    chunk_bitmap[0] = 0;
    chunk_bitmap[1] = 0;

    lb_vector_program_map_tbl_data.chunk_size = Arad_pp_lb_key_chunk_size[vc_indx].chunk_size;

	/*Iterate over masks*/
    for (mask_idx = 0; mask_idx < SOC_PPC_NOF_HASH_MASKS; ++mask_idx)
    {
      skip = TRUE;
      field_info_offset = mask_idx;

      if(arad_pp_lag_lb_vector_index_map(unit, Arad_pp_lag_hash_field_info[field_info_offset].key_index) == map_index)
      {
        skip = FALSE;
      }
      if ((SOC_SAND_BIT(mask_idx) == SOC_PPC_HASH_MASKS_MPLS_LABEL_1) && (ARAD_PP_LB_MPLS_LBL1_VLD(map_index)))
      {
        skip = FALSE;
      }
      else if ((SOC_SAND_BIT(mask_idx) == SOC_PPC_HASH_MASKS_MPLS_LABEL_2) && (ARAD_PP_LB_MPLS_LBL2_VLD(map_index)))
      {
        skip = FALSE;
      }
      else if ((SOC_SAND_BIT(mask_idx) == SOC_PPC_HASH_MASKS_MPLS_LABEL_3) && (ARAD_PP_LB_MPLS_LBL3_VLD(map_index)))
      {
        skip = FALSE;
      }
	  else if ((SOC_SAND_BIT(mask_idx) == SOC_PPC_HASH_MASKS_MPLS_LABEL_4) && (ARAD_PP_LB_MPLS_LBL4_VLD(map_index))) {
		skip = FALSE;
	  }
	  else if ((SOC_SAND_BIT(mask_idx) == SOC_PPC_HASH_MASKS_MPLS_LABEL_5) && (ARAD_PP_LB_MPLS_LBL5_VLD(map_index))) {
		skip = FALSE;
	  }
      if (skip)
      {
        continue;
      }

      fld = SOC_SAND_BIT(mask_idx);
      peer_fld = arad_pp_lag_hash_sym_peer(fld);
      /* if (masks & fld) -> enable = 0 */
	  nible_val = arad_pp_lag_calc_nible_val(fld, (uint8)((masks & fld)== 0), (uint8)((masks & peer_fld)== 0), is_symmetric);
	  field_nof_nbls = Arad_pp_lag_hash_field_info[field_info_offset].nof_nibles;

	  for (field_nbl_indx = 0 ; field_nbl_indx < field_nof_nbls; ++field_nbl_indx)
	  {
	      res = soc_sand_bitstream_set_any_field(
	              &nible_val,
	   	          ARAD_PP_LB_FIELD_TO_NIBLE(Arad_pp_lag_hash_field_info[field_info_offset].nbls[field_nbl_indx],lb_vector_program_map_tbl_data.chunk_size),
		            2/*(lb_vector_program_map_tbl_data.chunk_size)?8:4*/,
		            chunk_bitmap
    	        );
	      SOC_SAND_CHECK_FUNC_RESULT(res, 19, exit);
      }
    }

    COMPILER_64_SET(lb_vector_program_map_tbl_data.chunk_bitmap, chunk_bitmap[1], chunk_bitmap[0]);
    ARAD_COMP(&old_val,&lb_vector_program_map_tbl_data,ARAD_PP_IHB_LB_VECTOR_PROGRAM_MAP_TBL_DATA,1,is_equal);

    if (!is_equal)
    {
      res = arad_pp_ihb_lb_vector_program_map_tbl_set_unsafe(
                unit,
                map_index,
                &lb_vector_program_map_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 19, exit);
    }
  }
 /*
  * set CW
  */
  fld_val = SOC_SAND_BOOL2NUM(mask_info->expect_cw);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  112,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_LAG_LB_KEY_CFGr, SOC_CORE_ALL, 0, LB_MPLS_CONTROL_WORDf,  fld_val));

  /* store key in SW DB */
  res = sw_state_access[unit].dpp.soc.arad.pp.lag.masks.set(unit, masks);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 120, exit);
  res = sw_state_access[unit].dpp.soc.arad.pp.lag.lb_key_is_symtrc.set(unit, is_symmetric);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_mask_set_unsafe()", 0, 0);
}

uint32
  arad_pp_lag_hashing_mask_set_verify(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO       *mask_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_MASK_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_HASH_MASK_INFO, mask_info, 10, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_mask_set_verify()", 0, 0);
}

uint32
  arad_pp_lag_hashing_mask_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_MASK_GET_VERIFY);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_mask_get_verify()", 0, 0);
}

/*********************************************************************
*     Mask / unmask fields from the packet header. Masked
 *     fields are ignored by the hashing function
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lag_hashing_mask_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_HASH_MASK_INFO       *mask_info
  )
{
  uint32
    res,
    fld_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LAG_HASHING_MASK_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(mask_info);

  SOC_PPC_HASH_MASK_INFO_clear(mask_info);
  
  res = sw_state_access[unit].dpp.soc.arad.pp.lag.masks.get(unit, &(mask_info->mask));
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = sw_state_access[unit].dpp.soc.arad.pp.lag.lb_key_is_symtrc.get(unit, &(mask_info->is_symmetric_key));
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

 /*
  * get CW
  */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  112,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_LAG_LB_KEY_CFGr, SOC_CORE_ALL, 0, LB_MPLS_CONTROL_WORDf, &fld_val));
  mask_info->expect_cw = SOC_SAND_NUM2BOOL(fld_val);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_hashing_mask_get_unsafe()", 0, 0);
}

uint32
  arad_pp_lag_lb_key_range_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_LAG_INFO                            *lag_info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_LAG_INFO
    tm_lag_info;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(lag_info);

  /************************************************************************/
  /* Set Members                                                          */
  /************************************************************************/
  arad_pp_lag_members_ppd2tm(lag_info, &tm_lag_info);

  res = arad_ports_lag_lb_key_range_set_unsafe(
          unit,
          &tm_lag_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lag_lb_key_range_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_lag module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lag_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_lag;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_lag module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lag_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_lag;
}

uint32
  SOC_PPC_LAG_HASH_GLOBAL_INFO_verify(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

   SOC_SAND_ERR_IF_OUT_OF_RANGE(info->hash_func_id, ARAD_PP_LAG_HASH_FUNC_ID_MIN, ARAD_PP_LAG_HASH_FUNC_ID_MAX, ARAD_PP_LAG_HASH_FUNC_ID_OUT_OF_RANGE_ERR, 14, exit);
   SOC_SAND_ERR_IF_ABOVE_MAX(info->key_shift, ARAD_PP_LAG_KEY_SHIFT_MAX, ARAD_PP_LAG_KEY_SHIFT_OUT_OF_RANGE_ERR, 15, exit);
   SOC_SAND_ERR_IF_ABOVE_MAX(info->seed, ARAD_PP_LAG_SEED_MAX, ARAD_PP_LAG_SEED_OUT_OF_RANGE_ERR, 16, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LAG_HASH_GLOBAL_INFO_verify()",0,0);
}

uint32
  SOC_PPC_LAG_HASH_PORT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* SOC_SAND_ERR_IF_ABOVE_MAX(info->nof_headers, ARAD_PP_LAG_NOF_HEADERS_MAX, ARAD_PP_LAG_NOF_HEADERS_OUT_OF_RANGE_ERR, 10, exit); */
  SOC_SAND_ERR_IF_ABOVE_MAX(info->first_header_to_parse, ARAD_PP_LAG_FIRST_HEADER_TO_PARSE_MAX, ARAD_PP_LAG_FIRST_HEADER_TO_PARSE_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LAG_HASH_PORT_INFO_verify()",0,0);
}

uint32
  SOC_PPC_HASH_MASK_INFO_verify(
    SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO *info
  )
{
  uint32
    invalid_mask;
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  invalid_mask = SOC_SAND_BITS_MASK(31,SOC_PPC_NOF_HASH_MASKS+1);
  
  if ((invalid_mask & info->mask) != 0)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LAG_MASK_OUT_OF_RANGE_ERR, 12, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_HASH_MASK_INFO_verify()",info->mask,0);
}

uint32
  SOC_PPC_LAG_MEMBER_verify(
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->sys_port, ARAD_PP_LAG_SYS_PORT_MAX, ARAD_PP_LAG_SYS_PORT_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->member_id, SOC_PPC_LAG_MEMBER_ID_MAX, SOC_PPC_LAG_MEMBER_ID_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LAG_MEMBER_verify()",0,0);
}

uint32
  SOC_PPC_LAG_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_LAG_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    ind/*,ind2 */;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->nof_entries, ARAD_PP_LAG_NOF_ENTRIES_MAX(unit), ARAD_PP_LAG_NOF_ENTRIES_OUT_OF_RANGE_ERR, 10, exit);
  for (ind = 0; ind < SOC_PPC_LAG_MEMBERS_MAX; ++ind)
  {
    ARAD_PP_STRUCT_VERIFY(SOC_PPC_LAG_MEMBER, &(info->members[ind]), 11, exit);
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->lb_type, SOC_PPC_LAG_LB_TYPE_MAX, SOC_PPC_LAG_LB_TYPE_OUT_OF_RANGE_ERR, 12, exit);
/*
  for (ind = 0; ind < info->nof_entries; ++ind)
  {
    for (ind2 = 0; ind2 < info->nof_entries; ++ind2)
    {
      if (info->members[ind].sys_port == info->members[ind2].sys_port && (ind != ind2))
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_LAG_DOUPLICATE_MEMBER_ERR, 12, exit);
      }
    }
  }
*/
  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_LAG_INFO_verify()", info->nof_entries, 0);
}

/*Currently the valid signal isn't working so this check is performed*/
uint32
ARAD_PP_LAG_is_ecmp_valid(
        int     unit,
        int     core_id,
        uint8*  ecmp_valid,
        uint32  ecmp_fec,
        uint32* start_pointer,
        uint32* group_size,
        uint32* is_protected,
        uint32* ecmp_fec_pointer
        )
{
    uint32 ecmp_entry;
    ARAD_PP_DIAG_REG_FIELD fld;
    uint32 regs_val[ARAD_PP_DIAG_DBG_VAL_LEN];
    uint32 rv = SOC_SAND_OK;
    SOCDNX_INIT_FUNC_DEFS;

    *ecmp_valid = FALSE;

    if(ecmp_fec <= ARAD_PP_LAG_MAX_ECMP_ENTRY) {

        READ_IHB_FEC_ECMPm(unit, MEM_BLOCK_ANY, ecmp_fec, &ecmp_entry);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(&ecmp_entry, 0,  17, start_pointer));
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(&ecmp_entry, 17, 11, group_size));
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(&ecmp_entry, 28,  1, is_protected));

        /* dpp_dsig_read(unit, core_id, "IRPP", "FEC Resolution", NULL, "FEC_ECMP_Ptr", ecmp_fec_pointer, 1); */
        JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,2,3,53,37,5);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val, 0, 17, ecmp_fec_pointer));

        if( *group_size > 0 && *ecmp_fec_pointer > ARAD_PP_LAG_MAX_ECMP_ENTRY && *start_pointer > ARAD_PP_LAG_MAX_ECMP_ENTRY) {
            *ecmp_valid = TRUE;
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}



/*This function is used for diagnostics of the ECMP information.*/
soc_error_t
  soc_jer_pp_lag_print_ecmp_lb_data(
  SOC_SAND_INOUT  int  unit
  )
{
    int    core_id;
    int    cores_num = SOC_DPP_DEFS_GET(unit, nof_cores);
    uint8  ecmp_valid;
    uint32 regs_val[ARAD_PP_DIAG_DBG_VAL_LEN];
    uint32 flow_label_dst[3];
    uint32 is_stateful, plc, pfc, ecmp_fec, ecmp_lb_key, fwrd_action, ecmp_fec_pointer = 0, offset, resolution_fec;
    uint32 group_size = 0, is_protected = 0, start_pointer = 0;
    uint32 rv = SOC_SAND_OK;
    ARAD_PP_DIAG_REG_FIELD fld;
    bcm_l3_egress_t egr;

    SOCDNX_INIT_FUNC_DEFS;

    for(core_id = 0; core_id < cores_num ; core_id++)
    {
        ecmp_valid = FALSE;
        LOG_CLI((BSL_META_U(unit, "\n\n\r")));
        LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
        LOG_CLI((BSL_META_U(unit, "    |-------------------------------  CORE %d  --------------------------------|\n\r"),core_id));
        LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
        /* dpp_dsig_read(unit, core_id, "IRPP", "FLP", "PMF", "Fwd_Action_Dst", fwrd_action, 1); */
        JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHP_ID,4,8,127,109,5);
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,19,&fwrd_action));
        ecmp_fec = ARAD_PP_FRWR_ACTION_TO_FEC(fwrd_action);

        SOCDNX_SAND_IF_ERR_EXIT(ARAD_PP_LAG_is_ecmp_valid(unit, core_id, &ecmp_valid, ecmp_fec, &start_pointer, &group_size, &is_protected, &ecmp_fec_pointer));

        if(ecmp_valid ) {

            /* dpp_dsig_read(unit, core_id, "IRPP", "FEC Resolution", NULL, "FEC_ECMP_Ptr_is_Stateful", is_stateful, 1); */
            JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,2,3,36,36,5);
            SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,1,&is_stateful));
            /* dpp_dsig_read(unit, core_id, "IRPP", "FEC Resolution", NULL, "ECMP_LB_Key", ecmp_lb_key, 1); */
            JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,3,0,15,0,5);
            SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,16,&ecmp_lb_key));
            offset = (group_size * ecmp_lb_key) >> 16;
            resolution_fec = start_pointer + (is_protected + 1) * offset;

            LOG_CLI((BSL_META_U(unit, "    |                              \033[1m ECMP information\033[0m                          |\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |             ECMP Group               |               0x%03x              |\n\r"), ecmp_fec));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |            Start Pointer             |              0x%04x              |\n\r"), start_pointer));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |             Group Size               |               %3d                |\n\r"), group_size));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |             Protected                |                 %d                |\n\r"), is_protected));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |             Stateful                 |                 %d                |\n\r"), is_stateful));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |       ECMP Load Balancing Key        |               0x%04x             |\n\r"), ecmp_lb_key));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |           FEC (Calculated)           |               0x%04x             |\n\r"), resolution_fec));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |           FEC (Received)             |               0x%04x             |\n\r"), ecmp_fec_pointer));
            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
            LOG_CLI((BSL_META_U(unit, "    |             FEC Offset               |               %3d                |\n\r"), offset));

            SOCDNX_SAND_IF_ERR_EXIT(bcm_petra_l3_egress_get(unit,resolution_fec,&egr));
            _SHR_GPORT_FORWARD_GROUP_TO_L3_ITF_FEC(ecmp_fec, egr.port);

            /* In case the egress port is a FEC, this could be a cascaded ECMP */
            if(_SHR_GPORT_IS_FORWARD_PORT(egr.port) && BCM_L3_ITF_TYPE_IS_FEC(ecmp_fec))
            {
                ecmp_fec = _SHR_L3_ITF_VAL_GET(ecmp_fec);
                SOCDNX_SAND_IF_ERR_EXIT(ARAD_PP_LAG_is_ecmp_valid(unit, core_id, &ecmp_valid, ecmp_fec, &start_pointer, &group_size, &is_protected, &ecmp_fec_pointer));

                if(ecmp_valid) {
                    LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                    LOG_CLI((BSL_META_U(unit, "    |         Cascaded ECMP Group          |               0x%03x              |\n\r"), ecmp_fec));
                    LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                    LOG_CLI((BSL_META_U(unit, "    |            Start Pointer             |              0x%04x              |\n\r"), start_pointer));
                    LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                    LOG_CLI((BSL_META_U(unit, "    |             Group Size               |               %3d                |\n\r"), group_size));
                    LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                    LOG_CLI((BSL_META_U(unit, "    |             Protected                |                 %d                |\n\r"), is_protected));
                }
            }

            if(is_stateful) {

                /* dpp_dsig_read(unit, core_id, "IRPP", "PMF", NULL, "Consistent_Hashing_LEM_Key", flow_label_dst, 1); */
                JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,0,11,255,182,5);
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,74,flow_label_dst));

                LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                LOG_CLI((BSL_META_U(unit, "    |        Flow Label Destination        |      0x%03x%08x%08x       |\n\r"),flow_label_dst[2], flow_label_dst[1],flow_label_dst[0]));
                LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                LOG_CLI((BSL_META_U(unit, "    |  Flow Label Destination (last 47)    |           0x%04x%08x         |\n\r"), 0x7FFF & flow_label_dst[1],flow_label_dst[0] ));
            } else {
                /* dpp_dsig_read(unit, core_id, "IRPP", "PMF", NULL, "Parser_Leaf_Context", plc, 1); */
                JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,0,12,18,15,5);
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,4,&plc));
                /* dpp_dsig_read(unit, core_id, "IRPP", "PMF", NULL, "Packet_Format_Code", pfc, 1); */
                JER_PP_DIAG_FLD_READ(&fld,core_id,ARAD_IHB_ID,0,12,24,19,5);
                SOCDNX_SAND_IF_ERR_EXIT(soc_sand_bitstream_get_any_field(regs_val,0,6,&pfc));

                LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                LOG_CLI((BSL_META_U(unit, "    |                PFC                   |               %3d                |\n\r"), pfc));
                LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
                LOG_CLI((BSL_META_U(unit, "    |                PLC                   |               %3d                |\n\r"), plc));
            }

            LOG_CLI((BSL_META_U(unit, "    |-------------------------------------------------------------------------|\n\r")));
        } else {
            LOG_CLI((BSL_META_U(unit, "           No valid ECMP information was found for core %d !!! \n\r"),core_id));
        }

        LOG_CLI((BSL_META_U(unit, "\n\r")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}



/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */


