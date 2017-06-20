#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_frwrd_bmact.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/Arad/ARAD_PP/src/arad_pp_frwrd_bmact.c
*
* MODULE PREFIX:  arad_pp
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD
#include <shared/bsl.h>
#include <soc/mem.h>
/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_bmact.h>
#define ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_MAX                  (0xf)
#define ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_MAX                    (1)

#define ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_MAX                      (SOC_SAND_UINT_MAX)
#define ARAD_PP_FRWRD_BMACT_PCP_NOF_BITS                         (3)
#define ARAD_PP_FRWRD_BMACT_DEI_NOF_BITS                         (1)

#define ARAD_PP_FRWRD_BMACT_MY_B_MAC_MC_PREFIX                   (0x011e83)
/*
 * MACRO to check if Mac in mac is enabled.
 * will be called in beginning of each API
 */
#define ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit)  \
    do{ \
        uint8 __mim_enabled__;  \
        if (arad_pp_is_mac_in_mac_enabled(unit, &__mim_enabled__) != SOC_SAND_OK || !__mim_enabled__) {    \
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FRWRD_BMACT_MAC_IN_MAC_CHECK_IF_ENABLED_ERR, 150, exit);   \
        } \
    }while(0)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_frwrd_bmact[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_ENTRY_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_GET_ERRS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_INIT_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_BMACT_INIT_PRINT),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_frwrd_bmact[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_FRWRD_BMACT_SUCCESS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_SUCCESS_OUT_OF_RANGE_ERR",
    "The parameter 'success' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_NOF_SUCCESS_FAILURES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_OUT_OF_RANGE_ERR",
    "The parameter 'stp_topology_id' is out of range. \n\r "
    "The range is: 0 - 15.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_OUT_OF_RANGE_ERR",
    "The parameter 'b_fid_profile' is out of range. \n\r "
    "The range is: 0 - 1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_OUT_OF_RANGE_ERR",
    "The parameter 'sys_port_id' is out of range. \n\r "
    "The range is: 0 - 31.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_BMACT_I_SID_DOMAIN_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_I_SID_DOMAIN_OUT_OF_RANGE_ERR",
    "The parameter 'i_sid_domain' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_BMACT_KEY_FLAGS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_BMACT_KEY_FLAGS_OUT_OF_RANGE_ERR",
    "The parameter 'flags' is out of range. \n\r "
    "The range is: SOC_PPC_BMACT_ENTRY_TYPE_FRWRD - SOC_PPC_BMACT_ENTRY_TYPE_LEARN.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
  ARAD_PP_FRWRD_BMACT_DA_NOT_FOUND_ACTION_PROFILE_NDX_OUT_OF_RANGE_ERR,
  "ARAD_PP_FRWRD_BMACT_DA_NOT_FOUND_ACTION_PROFILE_NDX_OUT_OF_RANGE_ERR",
  "The parameter 'da_not_found_action_profile_ndx' is out of range. \n\r "
  "The range is: 0 - 1.\n\r ",
  SOC_SAND_SVR_ERR,
  FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */
  {
  ARAD_PP_FRWRD_BMACT_MAC_IN_MAC_CHECK_IF_ENABLED_ERR,
  "ARAD_PP_FRWRD_BMACT_MAC_IN_MAC_CHECK_IF_ENABLED_ERR",
  "Mac in mac is not enabled on this device. \n\r ",
  SOC_SAND_SVR_ERR,
  FALSE
  },
  {
  ARAD_PP_FRWRD_BMACT_SA_AUTH_ENABLED_ERR,
  "ARAD_PP_FRWRD_BMACT_SA_AUTH_ENABLED_ERR",
  "SA authentication is enabled on this device.\n\r "
  "SA authentication and mac in mac cannot be both enabled.\n\r ",
  SOC_SAND_SVR_ERR,
  FALSE
  },
  {
  ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR,
  "ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR",
  "The parameter 'pcp_profile_ndx' is out of range. \n\r "
  "The range is: 0.\n\r ",
  SOC_SAND_SVR_ERR,
  FALSE
  },
  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
*     Init device to support Mac-in-Mac.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_init_unsafe(
      SOC_SAND_IN  int                                 unit
  )
{
  uint32
      fld_val,
      res = SOC_SAND_OK;
  soc_reg_above_64_val_t 
      reg_above64_val;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_INIT_UNSAFE);

  /*
  * set SA lookup type:
  */
  fld_val = (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BMAC(unit));
  if (SOC_IS_ARAD_A0(unit)) {
      fld_val = fld_val >> 1; /* prefix is XXX0, only 3 MSB are written */
  }
  
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_LINK_LAYER_LOOKUP_CFGr, SOC_CORE_ALL, 0, PBP_MACT_PREFIXf,  fld_val));
  
  /* SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_GLOBAL_REGISTER_6r, REG_PORT_ANY, 0, SA_LOOKUP_TYPEf,  fld_val)); */

  /* Set My-B-Mac MC contant prefix. if prefix = 0x011e83 then Mac-in-Mac is enabled */
  fld_val = ARAD_PP_FRWRD_BMACT_MY_B_MAC_MC_PREFIX;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_MY_B_MAC_MCr, SOC_CORE_ALL, 0, MY_B_MAC_MCf,  fld_val));

  /* for P2P to MiM (2PBB) - set the PP port VSI profile = 1 to 0xffff
     so that VSI-Base would be 0xffff for Mac-in-Mac-P2P-VSI */
  res = READ_IHP_VSI_PROFILE_TO_VSI_TABLEr(unit, REG_PORT_ANY, reg_above64_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

  /* each VSI profile is 16 bits, we are configuring VSI profile = 1 */
  reg_above64_val[0] = (0xffff << 16);

  res = WRITE_IHP_VSI_PROFILE_TO_VSI_TABLEr(unit, REG_PORT_ANY, reg_above64_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_init_unsafe()", 0, 0);
}

uint32
  arad_pp_is_mac_in_mac_enabled(
  SOC_SAND_IN  int     unit,
  SOC_SAND_OUT  uint8     *enabled
  )
{
  uint32
      fld_val,
      res = SOC_SAND_OK;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(enabled);

  /* Get SrcPortIsPbp */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EGQ_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUX_DATA_SRC_PORT_IS_PBPf, &fld_val));
  *enabled = (uint8)(fld_val == 0xff);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_is_mac_in_mac_enabled()", 0, 0);
}

/*********************************************************************
*     Set Mac-in_mac TPID profile.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_mac_in_mac_enable(
    SOC_SAND_IN  int           unit
  )
{
  uint32
      fld_val,
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Set SrcPortIsPbp = -1 in EPNI and EGQ */
  fld_val = 0xff;
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUX_DATA_SRC_PORT_IS_PBPf,  fld_val));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUX_DATA_SRC_PORT_IS_PBPf,  fld_val));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_mac_in_mac_enable()", 0, 0);
}

/*********************************************************************
*     Set the B-VID bridging attributes. Backbone Mac
 *     addresses that do not serve as MyMAC for I-components
 *     that are processed according to their B-VID
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_bvid_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                *bvid_info
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_val,
    fid_class,
    stp_topology,
    dest_encoded,
    asd_encoded;
  soc_field_t
    fld;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(bvid_info);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  res = READ_IHP_BVD_CFGm(unit, MEM_BLOCK_ANY, bvid_ndx, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = arad_pp_fwd_decision_in_buffer_build( 
      unit,
      SOC_PPC_FRWRD_DECISION_TYPE_MC,
      &(bvid_info->uknown_da_dest), 
      &dest_encoded,
      &asd_encoded
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  
  soc_mem_field_set(unit, IHP_BVD_CFGm, &reg_val, DA_NOT_FOUND_DESTINATIONf, &dest_encoded);

  res = WRITE_IHP_BVD_CFGm(unit, MEM_BLOCK_ANY, bvid_ndx, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* set FID class */
  res = READ_IHP_BVD_FID_CLASSm(unit, MEM_BLOCK_ANY, bvid_ndx / 4, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (bvid_info->b_fid_profile == SOC_PPC_BFID_EQUAL_TO_BVID) {
      fid_class = 1;
  }
  else { /* SOC_PPC_BFID_IS_0 */
      fid_class = 0;
  }

  switch (bvid_ndx % 4) {
  case 0:
      fld = FID_CLASSf;
      break;
  case 1:
      fld = FID_CLASS_1f;
      break;
  case 2:
      fld = FID_CLASS_2f;
      break;
  default: /* 3 */
      fld = FID_CLASS_3f;
  }

  soc_mem_field_set(unit, IHP_BVD_FID_CLASSm, &reg_val, fld, &fid_class);
    
  res = WRITE_IHP_BVD_FID_CLASSm(unit, MEM_BLOCK_ANY, bvid_ndx / 4, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = READ_IHP_BVD_TOPOLOGY_IDm(unit, MEM_BLOCK_ANY, bvid_ndx / 8, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  switch (bvid_ndx % 8) {
  case 0:
      fld = LSBf;
      break;
  case 1:
      fld = LSB_1f;
      break;
  case 2:
      fld = LSB_2f;
      break;
  case 3:
      fld = LSB_3f;
      break;
  case 4:
      fld = LSB_4f;
      break;
  case 5:
      fld = LSB_5f;
      break;
  case 6:
      fld = LSB_6f;
      break;
  default:
      fld = LSB_7f;
  }

  stp_topology = bvid_info->stp_topology_id;
  soc_mem_field_set(unit, IHP_BVD_TOPOLOGY_IDm, &reg_val, fld, &stp_topology);
  
  res = WRITE_IHP_BVD_TOPOLOGY_IDm(unit, MEM_BLOCK_ANY, bvid_ndx / 8, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_bvid_info_set_unsafe()", bvid_ndx, 0);
}

uint32
  arad_pp_frwrd_bmact_bvid_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                *bvid_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(bvid_ndx, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_BMACT_BVID_INFO, bvid_info, 20, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_bvid_info_set_verify()", bvid_ndx, 0);
}

uint32
  arad_pp_frwrd_bmact_bvid_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(bvid_ndx, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_bvid_info_get_verify()", bvid_ndx, 0);
}

/*********************************************************************
*     Set the B-VID bridging attributes. Backbone Mac
 *     addresses that do not serve as MyMAC for I-components
 *     that are processed according to their B-VID
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_bvid_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_OUT SOC_PPC_BMACT_BVID_INFO                *bvid_info
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_val,
    fid_class,
    dest_encoded;
  soc_field_t
    fld;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(bvid_info);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  SOC_PPC_BMACT_BVID_INFO_clear(bvid_info);

  res = READ_IHP_BVD_CFGm(unit, MEM_BLOCK_ANY, bvid_ndx, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  soc_mem_field_get(unit, IHP_BVD_CFGm, &reg_val, DA_NOT_FOUND_DESTINATIONf, &dest_encoded);

  res = arad_pp_fwd_decision_in_buffer_parse(
      unit,
      dest_encoded,
      0,
      ARAD_PP_FWD_DECISION_PARSE_DEST,
      &(bvid_info->uknown_da_dest)
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = READ_IHP_BVD_FID_CLASSm(unit, MEM_BLOCK_ANY, bvid_ndx / 4, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  switch (bvid_ndx % 4) {
  case 0:
      fld = FID_CLASSf;
      break;
  case 1:
      fld = FID_CLASS_1f;
      break;
  case 2:
      fld = FID_CLASS_2f;
      break;
  default: /* 3 */
      fld = FID_CLASS_3f;
  }

  soc_mem_field_get(unit, IHP_BVD_FID_CLASSm, &reg_val, fld, &fid_class);

  if (fid_class) {
      bvid_info->b_fid_profile = SOC_PPC_BFID_EQUAL_TO_BVID; /* independent */
  }
  else {
      bvid_info->b_fid_profile = SOC_PPC_BFID_IS_0; /* shared */
  }

  res = READ_IHP_BVD_TOPOLOGY_IDm(unit, MEM_BLOCK_ANY, bvid_ndx / 8, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  switch (bvid_ndx % 8) {
  case 0:
      fld = LSBf;
      break;
  case 1:
      fld = LSB_1f;
      break;
  case 2:
      fld = LSB_2f;
      break;
  case 3:
      fld = LSB_3f;
      break;
  case 4:
      fld = LSB_4f;
      break;
  case 5:
      fld = LSB_5f;
      break;
  case 6:
      fld = LSB_6f;
      break;
  default:
      fld = LSB_7f;
  }

  soc_mem_field_get(unit, IHP_BVD_TOPOLOGY_IDm, &reg_val, fld, &(bvid_info->stp_topology_id));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_bvid_info_get_unsafe()", bvid_ndx, 0);
}

/*********************************************************************
*     Set the BVID range for Traffic Engineered Provider
 *     Backbone Bridging
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_val = 0;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(pbb_te_bvids);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  soc_reg_field_set(unit, IHP_PBP_TE_BVID_RANGEr, &reg_val, PBP_TE_BVID_RANGE_LOWf, pbb_te_bvids->first_vid);
  soc_reg_field_set(unit, IHP_PBP_TE_BVID_RANGEr, &reg_val, PBP_TE_BVID_RANGE_HIGHf, pbb_te_bvids->last_vid);

  res = WRITE_IHP_PBP_TE_BVID_RANGEr(unit, REG_PORT_ANY, reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_BMACT_PBB_TE_VID_RANGE, pbb_te_bvids, 10, exit);

  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_pbb_te_bvid_range_set_verify()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
   uint32
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_pbb_te_bvid_range_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the BVID range for Traffic Engineered Provider
 *     Backbone Bridging
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_val;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(pbb_te_bvids);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  SOC_PPC_BMACT_PBB_TE_VID_RANGE_clear(pbb_te_bvids);

  res = READ_IHP_PBP_TE_BVID_RANGEr(unit, REG_PORT_ANY, &reg_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  pbb_te_bvids->first_vid = soc_reg_field_get(unit, IHP_PBP_TE_BVID_RANGEr, reg_val, PBP_TE_BVID_RANGE_LOWf);
  pbb_te_bvids->last_vid = soc_reg_field_get(unit, IHP_PBP_TE_BVID_RANGEr, reg_val, PBP_TE_BVID_RANGE_HIGHf);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_pbb_te_bvid_range_get_unsafe()", 0, 0);
}

/* 
 * ASD = {16'b Learn info (15'b FEC_ptr + 1'b0/16'b LIF), 1'b Learn_type (FEC/LIF), 5'b domain} 
 * Learn info not used anymore in BMACT DB. Learn info should be always 0 (not used). 
 */
STATIC uint32
  arad_pp_mim_asd_build(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    flags,
    SOC_SAND_IN SOC_PPC_BMACT_ENTRY_INFO   *bmact_entry_info,
    SOC_SAND_OUT uint32                    *asd
  )
{
  uint32    
    tmp,
    dest_encoded,
    asd_encoded,
    asd_lcl[1],
    res = SOC_SAND_OK;
  SOC_PPC_FRWRD_DECISION_INFO
    fwd_decision;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *asd_lcl = 0;
  SOC_PPC_FRWRD_DECISION_INFO_clear(&fwd_decision);

  
  if (flags & SOC_PPC_BMACT_ENTRY_TYPE_FRWRD) /* BMACT DB */
  {      
      tmp = bmact_entry_info->i_sid_domain;
      res = soc_sand_bitstream_set_any_field(
           &(tmp),
           ARAD_PP_LEM_ACCESS_ASD_DOMAIN,
           ARAD_PP_LEM_ACCESS_ASD_DOMAIN_LEN,
           asd_lcl
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      /* 
       * get encoded dest for the learn fec
       */
      ARAD_PP_FRWRD_DECISION_FEC_SET(unit, &fwd_decision, bmact_entry_info->sys_port_id);
      
      res = arad_pp_fwd_decision_in_buffer_build( 
          unit,
          ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE_FEC,
          &fwd_decision, 
          &dest_encoded,
          &asd_encoded
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      
      res = soc_sand_bitstream_set_any_field(
           &(dest_encoded),
           ARAD_PP_LEM_ACCESS_ASD_LEARN_FEC_PTR,
           SOC_DPP_DEFS_GET(unit, fec_nof_bits),
           asd_lcl
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  } 

  if (flags & SOC_PPC_BMACT_ENTRY_TYPE_LEARN) /* MIM tunnel learn DB */
  {
      /* Learn encoding {0x0(1 Type-A),0x0(3 CTL), 0x0 (1 OutLIF-Valid), 0x0(16 OutLIF)} */
      /* All zeros */
      *asd_lcl = 0;    
  }
  
  *asd = *asd_lcl;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mim_asd_build()", 0, 0);
}

STATIC uint32
  arad_pp_frwrd_bmact_key_convert(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY  *bmac_key,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY   *key
   )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(key);
  SOC_SAND_CHECK_NULL_INPUT(bmac_key);

  if (bmac_key->flags & SOC_PPC_BMACT_ENTRY_TYPE_FRWRD) /* BMACT DB */
  {
      key->type = ARAD_PP_LEM_ACCESS_KEY_TYPE_BACKBONE_MAC;
      key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_BACKBONE_MAC;

      /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
      /* coverity[overrun-buffer-val : FALSE] */   
      res = soc_sand_pp_mac_address_struct_to_long(
        &(bmac_key->b_mac_addr),
        key->param[0].value
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

      key->param[1].value[0] = bmac_key->b_vid;
      key->param[1].value[1] = 0;
      key->param[0].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_BACKBONE_MAC;
      key->param[1].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_BACKBONE_MAC;
      key->prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_BACKBONE_MAC;
      key->prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_BACKBONE_MAC(unit);
  }

  if (bmac_key->flags & SOC_PPC_BMACT_ENTRY_TYPE_LEARN) { 
      key->type = ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC_IN_MAC_TUNNEL;
      key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_MAC_IN_MAC_TUNNEL;

      /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
      /* coverity[overrun-buffer-val : FALSE] */   
      res = soc_sand_pp_mac_address_struct_to_long(
        &(bmac_key->b_mac_addr),
        key->param[0].value
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

      key->param[1].value[0] = bmac_key->b_vid;
      key->param[1].value[1] = 0;      
      key->param[0].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;
      key->param[1].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;

      key->prefix.nof_bits   = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;
      key->prefix.value      = ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_MAC_IN_MAC_TUNNEL;

      
      if (SOC_DPP_CONFIG(unit)->pp.test2) {
          key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_MAC_IN_MAC_TEST2;
      } else {          
          key->param[2].value[0] = bmac_key->local_port_ndx;
          key->param[2].value[1] = 0;   
          if (SOC_DPP_DEFS_GET(unit, nof_cores) == 2) {
              /* in Jericho the port is 9 bits, 1bit Core-Bit and 8bits In-PP-Port */
              key->param[2].value[0] |= bmac_key->core << ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;
              key->param[2].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;
          } else {
              key->param[2].nof_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_MAC_IN_MAC_TUNNEL;
          }
      }
  }

  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_key_convert()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_payload_convert(
      SOC_SAND_IN  int                             unit,
      SOC_SAND_IN  uint32                             flags,
      SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD         *payload,
      SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO           *bmact_entry_info
    )
{
  uint32
    /*dest_encoded = 0,*/
    tmp = 0,
    res/*,
    asd = 0*/;
  SOC_PPC_FRWRD_DECISION_INFO
    frwrd_decision_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(bmact_entry_info);

  SOC_PPC_FRWRD_DECISION_INFO_clear(&frwrd_decision_info);
  SOC_PPC_BMACT_ENTRY_INFO_clear(bmact_entry_info);

  /*
   * Get the sys_port from the encoded destination
   */
  res = arad_pp_fwd_decision_in_buffer_parse(
      unit,      
      payload->dest,
      payload->asd,
      ARAD_PP_FWD_DECISION_PARSE_DEST,
      &frwrd_decision_info
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (frwrd_decision_info.type == SOC_PPC_FRWRD_DECISION_TYPE_FEC) 
  {
      bmact_entry_info->sa_learn_fec_id = frwrd_decision_info.dest_id; /* MACT Tunnel DB */
  } 
  else if (frwrd_decision_info.type == SOC_PPC_FRWRD_DECISION_TYPE_MC)
  {
      bmact_entry_info->mc_id = frwrd_decision_info.dest_id; /* BMACT DB MC destination*/
  }
  else
  {
      bmact_entry_info->sys_port_id = frwrd_decision_info.dest_id; /* BMACT DB */
  }
  
  if (flags & SOC_PPC_BMACT_ENTRY_TYPE_FRWRD) 
  {
    /*
     * Get the i-sid domain from ASD
     */
    res = soc_sand_bitstream_get_any_field(
         &(payload->asd),
         ARAD_PP_LEM_ACCESS_ASD_DOMAIN, 
         ARAD_PP_LEM_ACCESS_ASD_DOMAIN_LEN,
         &tmp
      );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    bmact_entry_info->i_sid_domain = tmp;
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_payload_convert()", 0, 0);
}

/*********************************************************************
*     Add an entry to the B-MACT DB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
  )
{
  uint32
  dest_encoded,
  asd_encoded,
    res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_PAYLOAD
    payload;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack_status;
  SOC_PPC_FRWRD_DECISION_INFO
    frwrd_decision_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(bmac_key);
  SOC_SAND_CHECK_NULL_INPUT(bmact_entry_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack_status);
  SOC_PPC_FRWRD_DECISION_INFO_clear(&frwrd_decision_info);

  /*
   *  Adapt the variables for the generic access to LEM (Large exact match)
   */
  request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT; 
  request.stamp = 0;
  res = arad_pp_frwrd_bmact_key_convert(
          unit,
          bmac_key,
          &(request.key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  
  /* Destination */
  if (bmac_key->flags & SOC_PPC_BMACT_ENTRY_TYPE_FRWRD) 
  {
  	  /* FRWRD */	
      if (bmac_key->flags & SOC_PPC_BMACT_ENTRY_TYPE_MC_DEST)
      {
          /* Destination is a multicast group. */
          ARAD_PP_FRWRD_DECISION_MC_GROUP_SET(unit, &frwrd_decision_info, bmact_entry_info->mc_id)
      }
      else
      {
          /* Destination is a system port. */
          ARAD_PP_FRWRD_DECISION_PHY_SYS_PORT_SET(unit, &frwrd_decision_info, bmact_entry_info->sys_port_id);
      }
  } 
  else 
  {
    /* LEARN */
    ARAD_PP_FRWRD_DECISION_FEC_SET(unit, &frwrd_decision_info, bmact_entry_info->sa_learn_fec_id);
  }
  
  /*
   *  Get the encoded destination. ASD will be built separately
   */
  res = arad_pp_fwd_decision_in_buffer_build(
      unit,
      ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE_MACT, 
      &frwrd_decision_info, 
      &dest_encoded,
      &asd_encoded
   );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  payload.dest = dest_encoded;
  
  /*
   *  Get ASD. Built from fec_id and i-sid domain
   */
  res = arad_pp_mim_asd_build(
     unit,
     bmac_key->flags,
     bmact_entry_info,
     &asd_encoded
  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  payload.asd = asd_encoded;
  payload.sa_drop = 0;
  
  /* aging is static, so all fields are 0 */
  payload.age = 0;
  payload.is_dynamic = 0;

  /* sa_drop */
  payload.sa_drop = bmact_entry_info->drop_sa;

  /*
   *  Access to the LEM (i.e., B-MAC Table or MIM Tunnel Learn Table)
   */
  res = arad_pp_lem_access_entry_add_unsafe(
          unit,
          &request,
          &payload,
          &ack_status
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  if (ack_status.is_success == TRUE)
  {
    *success = SOC_SAND_SUCCESS;
  }
  else
  {
    switch(ack_status.reason)
    {
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_MACT_FULL:
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_CAM_FULL:
      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      break;

    case ARAD_PP_LEM_ACCESS_FAIL_REASON_FID_LIMIT:
      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES_2;
      break;

    case ARAD_PP_LEM_ACCESS_FAIL_REASON_REQUEST_NOT_SENT:
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_WRONG_STAMP:
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_FID_UNKNOWN:
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_UNKNOWN:
      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES_3;
      break;

    case ARAD_PP_LEM_ACCESS_FAIL_REASON_CHANGE_STATIC:
    case ARAD_PP_LEM_ACCESS_FAIL_REASON_LEARN_STATIC:
      *success = SOC_SAND_FAILURE_REMOVE_ENTRY_FIRST;
      break;

    case ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN:
    default:
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR, 50, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_add_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_ADD_VERIFY);

  if (!SOC_DPP_CONFIG(unit)->pp.test2){
      ARAD_PP_STRUCT_VERIFY(SOC_PPC_BMACT_ENTRY_KEY, bmac_key, 10, exit);
  }
  
  res = SOC_PPC_BMACT_ENTRY_INFO_verify(unit, bmact_entry_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_add_verify()", 0, 0);
}

/*********************************************************************
*     Remove an entry from the B-MACT DB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_REQUEST
    request;
  ARAD_PP_LEM_ACCESS_ACK_STATUS
    ack_status;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(bmac_key);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack_status);

  /*
   * Adapt the variables for the generic access to LEM (Large exact match)
   */
  request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;
  request.stamp = 0;

  res = arad_pp_frwrd_bmact_key_convert(
          unit,
          bmac_key,
          &(request.key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /*
   * Access the LEM (i.e., B-MAC Table)
   */
  res = arad_pp_lem_access_entry_remove_unsafe(
          unit,
          &request,
          &ack_status
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  if (!((ack_status.is_success == TRUE) || (ack_status.reason == ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN)))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR, 50, exit);
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_remove_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_BMACT_ENTRY_KEY, bmac_key, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_remove_verify()", 0, 0);
}

/*********************************************************************
*     Get an entry from the B-MACT DB.
*     Details: in the H file. (search for prototype)
*********************************************************************/
 uint32
  arad_pp_frwrd_bmact_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info,
    SOC_SAND_OUT uint8                                  *found
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_KEY
   key;
  ARAD_PP_LEM_ACCESS_PAYLOAD
   payload;
  uint8
   is_found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(bmac_key);
  SOC_SAND_CHECK_NULL_INPUT(bmact_entry_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);

  SOC_PPC_BMACT_ENTRY_INFO_clear(bmact_entry_info);
  ARAD_PP_LEM_ACCESS_KEY_clear(&key);
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);

  /*
   * Adapt the variables for the generic access to LEM (Large exact match)
   */
  res = arad_pp_frwrd_bmact_key_convert(
          unit,
          bmac_key,
          &(key)
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /*
   * Access to the B-MAC Table
   */
  res = arad_pp_lem_access_entry_by_key_get_unsafe(
          unit,
          &key,
          &payload,
          &is_found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   * Convert the payload
   */
  res = arad_pp_frwrd_bmact_payload_convert(
          unit,
          bmac_key->flags,
          &payload,
          bmact_entry_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  *found = is_found;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_get_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_ENTRY_GET_VERIFY);

  if (!SOC_DPP_CONFIG(unit)->pp.test2) {
      ARAD_PP_STRUCT_VERIFY(SOC_PPC_BMACT_ENTRY_KEY, bmac_key, 10, exit); 
  }
  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_entry_get_verify()", 0, 0);
}

/*********************************************************************
*   Set mapping from COS parameters (DP and TC) to the PCP
*   and DEI values to be set in the transmitted packet's
*   I-tag. This is the mapping to be used when the incoming
*   packet has no tags or pcp profile is set to use TC and
*   DP for the mapping.
*   Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  )
{
    uint32
        dp_tc = 0,
        reg_offset,
        fld_offset,
        pcp_dei,
        res = SOC_SAND_OK;
    soc_reg_above_64_val_t
        reg_above64_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_UNSAFE);
    
    ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);
    
    /*
     * the key is {system-TC(3),system-DP(2)} 
     * the value is I-Tag.PCP-DEI(4): Bits [4xn+3:4xn] are the I-Tag.PCP-DEI value, where n is by {system-TC(3), system-DP(2)}  
     */
    
    dp_tc = (tc_ndx << 2) | dp_ndx;
    reg_offset = dp_tc / 8; 
    fld_offset = (dp_tc % 8) * 4;

    res = READ_EPNI_ITAG_TC_DP_MAPr(unit, REG_PORT_ANY, reg_above64_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
    pcp_dei = (out_pcp << ARAD_PP_FRWRD_BMACT_DEI_NOF_BITS) | out_dei;  
    res = soc_sand_bitstream_set_any_field(
        &pcp_dei,
        fld_offset,
        ARAD_PP_FRWRD_BMACT_PCP_NOF_BITS + ARAD_PP_FRWRD_BMACT_DEI_NOF_BITS,
        &(reg_above64_val[reg_offset])
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = WRITE_EPNI_ITAG_TC_DP_MAPr(unit, REG_PORT_ANY, reg_above64_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_unsafe()", 0, 0);  
}

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_VERIFY);

    /* SOC_SAND_ERR_IF_ABOVE_MAX(pcp_profile_ndx, ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_MAX, ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);*/
    SOC_SAND_ERR_IF_ABOVE_MAX(tc_ndx, SOC_SAND_PP_TC_MAX, SOC_SAND_PP_TC_OUT_OF_RANGE_ERR, 20, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(dp_ndx, SOC_SAND_PP_DP_MAX, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 30, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(out_pcp, SOC_SAND_PP_PCP_UP_MAX, SOC_SAND_PP_PCP_UP_OUT_OF_RANGE_ERR, 40, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(out_dei, SOC_SAND_PP_DEI_CFI_MAX, SOC_SAND_PP_DEI_CFI_OUT_OF_RANGE_ERR, 50, exit);
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_verify()", 0, 0);
}

/*********************************************************************
*     Gets the configuration set by the
*     "arad_pp_frwrd_bmact_eg_vlan_pcp_map_set" API.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  )
{
    uint32
        dp_tc = 0,
        reg_offset,
        fld_offset,
        dei = 0,
        pcp = 0,
        res = SOC_SAND_OK;
    soc_reg_above_64_val_t
        reg_above64_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET_UNSAFE);
    
    SOC_SAND_CHECK_NULL_INPUT(out_pcp);
    SOC_SAND_CHECK_NULL_INPUT(out_dei);
    
    ARAD_PP_MAC_IN_MAC_CHECK_IF_ENABLED(unit);
    
    /*
     * the key is {system-TC(3),system-DP(2)} 
     */
    
    dp_tc = (tc_ndx << 2) | dp_ndx;
    reg_offset = dp_tc / 8; 
    fld_offset = (dp_tc % 8) * 4;

    res = READ_EPNI_ITAG_TC_DP_MAPr(unit, REG_PORT_ANY, reg_above64_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = soc_sand_bitstream_get_any_field(
        &(reg_above64_val[reg_offset]),
        fld_offset,
        ARAD_PP_FRWRD_BMACT_DEI_NOF_BITS,
        &dei
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = soc_sand_bitstream_get_any_field(
        &(reg_above64_val[reg_offset]),
        fld_offset + ARAD_PP_FRWRD_BMACT_DEI_NOF_BITS,
        ARAD_PP_FRWRD_BMACT_PCP_NOF_BITS,
        &pcp
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    
    *out_pcp = (SOC_SAND_PP_PCP_UP)pcp;
    *out_dei = (SOC_SAND_PP_DEI_CFI)dei;
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_VERIFY);

    /*SOC_SAND_ERR_IF_ABOVE_MAX(pcp_profile_ndx, ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_MAX, ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);*/
    SOC_SAND_ERR_IF_ABOVE_MAX(tc_ndx, SOC_SAND_PP_TC_MAX, SOC_SAND_PP_TC_OUT_OF_RANGE_ERR, 20, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(dp_ndx, SOC_SAND_PP_DP_MAX, SOC_SAND_PP_DP_OUT_OF_RANGE_ERR, 30, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_verify()", 0, 0);
}
/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_frwrd_bmact module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_bmact_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_frwrd_bmact;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_frwrd_bmact module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_bmact_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_frwrd_bmact;
}

uint32
  SOC_PPC_BMACT_BVID_INFO_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->stp_topology_id, ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_MAX, ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->b_fid_profile, ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_MAX, ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_OUT_OF_RANGE_ERR, 11, exit);

 

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_BMACT_BVID_INFO_verify()",0,0);
}

uint32
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->first_vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->last_vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_BMACT_PBB_TE_VID_RANGE_verify()",0,0);
}

uint32
  SOC_PPC_BMACT_ENTRY_KEY_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->b_vid, SOC_SAND_PP_VLAN_ID_MAX, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 10, exit);  
   
  SOC_SAND_ERR_IF_ABOVE_MAX(info->local_port_ndx, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 50, exit);

  ARAD_PP_STRUCT_VERIFY(SOC_SAND_PP_MAC_ADDRESS, &(info->b_mac_addr), 60, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->flags, SOC_PPC_BMACT_ENTRY_TYPE_MC_DEST | SOC_PPC_BMACT_ENTRY_TYPE_FRWRD, SOC_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR, 70, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_BMACT_ENTRY_KEY_verify()",0,0);
}

uint32
  SOC_PPC_BMACT_ENTRY_INFO_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->sys_port_id, ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_MAX, ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->i_sid_domain, ARAD_PP_ISID_DOMAIN_MAX, ARAD_PP_FRWRD_BMACT_I_SID_DOMAIN_OUT_OF_RANGE_ERR, 30, exit);
  SOC_SAND_ERR_IF_ABOVE_NOF(info->sa_learn_fec_id, SOC_DPP_DEFS_GET(unit, nof_fecs), SOC_PPC_FEC_ID_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_BMACT_ENTRY_INFO_verify()",0,0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88650_A0) */


