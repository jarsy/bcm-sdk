#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_mymac.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_VSI

#include <soc/mem.h>
/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/dpp_config_defs.h>


#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_pp_mac.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mymac.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/register.h>

#include <soc/dpp/ARAD/arad_general.h>

#include <soc/dpp/PPD/ppd_api_mymac.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_MYMAC_MODE_MAX                                   (SOC_PPC_NOF_MYMAC_VRRP_MODES-1)
#define ARAD_PP_MYMAC_MY_NICK_NAME_MAX                           ((1<<16)-1)
#define ARAD_PP_MYMAC_VSI_LSB_MAX                                ((1<<12)-1)

#define ARAD_PP_MY_BMAC_MSB_LSB (8)
#define ARAD_PP_MY_BMAC_MSB_MSB (47)


/* Set bitmap key according to device, vsi and mac_lsb (or cam_table, if aradplus or jericho) */
#define ARAD_PP_MYMAC_VRRP_BITMAP_KEY_ALL_VSI_BASED(_unit, _vsi, _mac_lsb)                                      \
  ((SOC_IS_JERICHO(_unit)) ? ((_vsi<<4) + SOC_SAND_GET_BITS_RANGE(_mac_lsb,3,0)) /* key = {15bit VSI, 4bit CAM INDEX} */ \
                            : (_vsi<<3) + SOC_SAND_GET_BITS_RANGE(_mac_lsb,2,0)) /* key = {12bit VSI, 3bit MAC LSB} */
  

   


/*
 *  The LSB of the MAC addresses is set per VSI. Up to 8 MAC
 *  addresses (bitmap of 8 VRIDs) may be set per VSI (4K VSIs).
 *  IPV4 only.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_4K_VSI_8_VRID (0)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,2k]. Up to 2 MAC addresses may be set for
 *  each such VSI. IPV4 only.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_2_VRID  (1)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,1k]. Up to 4 MAC addresses may be set for
 *  each such VSI. IPV4 only.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_4_VRID  (2)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,1k]. For each VSI: up to 2 MAC addresses may be set for IPV4
 *  and up to 2 MAC addresses may be set for IPV6.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_2_VRID_IPV4_2_VRID_IPV6  (3)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,512]. Up to 8 MAC addresses may be set for each VSI.
 *  IPV4 only.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_512_VSI_8_VRID  (4)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,512]. For each VSI: up to 4 MAC addresses may be set for IPV4
 *  and up to 4 MAC addresses may be set for IPV6.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_512_VSI_4_VRID_IPV4_4_VRID_IPV6   (5)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,256]. Up to 16 MAC addresses may be set For each VSI.
 *  IPV4 only.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_256_VSI_16_VRID   (6)
/*
 *  The LSB of the MAC addresses is set for each VSI-only
 *  for VSIs [0,256]. For each VSI: up to 8 MAC addresses may be set for IPV4
 *  and up to 8 MAC addresses may be set for IPV6.
 */
#define  SOC_PPC_MYMAC_VRRP_MODE_256_VSI_8_VRID_IPV4_8_VRID_IPV6   (7)


/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_MYMAC_VRRP_CALCULATE_TBL_OFFSET(unit, global_bitmap_key) \
        (global_bitmap_key / SOC_DPP_DEFS_GET(unit, vrrp_mymac_map_width))

#define ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_REG_OFFSET(global_bitmap_key, nof_bits_in_line) \
            ((global_bitmap_key % nof_bits_in_line) / SOC_SAND_REG_SIZE_BITS)

#define ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_ENTRY_OFFSET(global_bitmap_key, nof_bits_in_line) \
            ((global_bitmap_key % nof_bits_in_line) % SOC_SAND_REG_SIZE_BITS)

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
  Arad_pp_procedure_desc_element_mymac[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_MSB_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VSI_LSB_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_VRRP_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_VRRP_MAC_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_MYMAC_TRILL_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_MSB_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MYMAC_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_mymac[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'vrrp_id_ndx' is out of range. \n\r "
    "The range is: No min - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR,
    "ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR",
    "The parameter 'mode' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_MYMAC_VRRP_MODES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_MY_NICK_NAME_OUT_OF_RANGE_ERR,
    "ARAD_PP_MYMAC_MY_NICK_NAME_OUT_OF_RANGE_ERR",
    "The parameter 'my_nick_name' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */

  {
    ARAD_PP_MYMAC_VSI_LSB_OUT_OF_RANGE,
    "ARAD_PP_MYMAC_VSI_LSB_OUT_OF_RANGE",
    "Vsi lsb is out of range. \n\r "
    "Maximal value is 00:00:00:00:0f:ff.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_IS_DISABLED_ERR,
    "ARAD_PP_MYMAC_VRRP_IS_DISABLED_ERR",
    "Mymac should be enabled and configured prior to performing\n\r "
    "this operation.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_LOCAL_PORT_OUT_OF_RANGE_ERR,
    "ARAD_PP_MYMAC_LOCAL_PORT_OUT_OF_RANGE_ERR",
    "The parameter 'local_port' is out of range. \n\r "
    "The range is: 0 - 63.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VSI_OUT_OF_RANGE_ERR,
    "ARAD_PP_MYMAC_VSI_OUT_OF_RANGE_ERR",
    "Vsi index is out of range. \n\r "
    "The range is: 1-4095.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MY_BMAC_MSB_LOW_BITS_NOT_ZERO_ERR,
    "ARAD_PP_MY_BMAC_MSB_LOW_BITS_NOT_ZERO_ERR",
    "'my_bmac_msb' defines bits 47:8. Bits 7:0 are ignored in this \n\r"
    "API, and therefor are validated to be zero.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_SYS_PORT_NON_SINGLE_PORT_NOT_SUPPORTED_ERR,
    "ARAD_PP_MYMAC_SYS_PORT_NON_SINGLE_PORT_NOT_SUPPORTED_ERR",
    "Non single port is not supported for this operation.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_SYS_PORT_OUT_OF_RANGE,
    "ARAD_PP_MYMAC_SYS_PORT_OUT_OF_RANGE",
    "System port is out of range.\n\r"
    "The range is: 1 to 4095.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR,
    "ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR",
    "MAC-IN-MAC not supported according to auxiliary table mode.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_MODE_IPV6_NOT_SUPPORTED_ERR,
    "ARAD_PP_MYMAC_MODE_IPV6_NOT_SUPPORTED_ERR",
    "The 'mode' does not support IPV6.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_4K_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_4K_ERR",
    "The 'mode' does not support 'vrrp_id_ndx' >= 4K.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_2K_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_2K_ERR",
    "The 'mode' does not support 'vrrp_id_ndx' >= 2K.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_1K_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_1K_ERR",
    "The 'mode' does not support 'vrrp_id_ndx' >= 1K.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_512_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_512_ERR",
    "The 'mode' does not support 'vrrp_id_ndx' >= 512.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_256_ERR,
    "ARAD_PP_MYMAC_VRRP_ID_NDX_ABOVE_256_ERR",
    "The 'mode' does not support 'vrrp_id_ndx' >= 256.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRID_MAP_FULL_ERR,
    "ARAD_PP_MYMAC_VRID_MAP_FULL_ERR",
    "The table is full for this 'vrrp_id_ndx'.\n\r "
    "To enter a new MAC you delete one of the entries first.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_MYMAC_VRID_MAP_ENTRY_NOT_FOUND_ERR,
    "ARAD_PP_MYMAC_VRID_MAP_ENTRY_NOT_FOUND_ERR",
    "The MAC does not exist in the table.\n\r ",
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

uint32
  arad_pp_mymac_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_init_unsafe()", 0, 0);
}

/*********************************************************************
*     Set the MSB of the MAC address of the device. Used for
 *     ingress termination and egress encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_msb_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_msb
  )
{
  uint32
	  res = SOC_SAND_OK,
	  fld_val = 0,
      mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S];
  uint64
	  mac_in_long,
	  reg_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_MSB_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(my_mac_msb);
  COMPILER_64_ZERO(mac_in_long);
  COMPILER_64_ZERO(reg_val);

  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
    my_mac_msb,
    mac_in_longs
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  COMPILER_64_SET(mac_in_long, mac_in_longs[1], mac_in_longs[0]);

  ARAD_FLD_TO_REG64(IHP_MY_MACr, MY_MACf, mac_in_long, reg_val, 40, exit);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHP_MY_MACr(unit, SOC_CORE_ALL,  reg_val));

  /* 
   * set global my-mac LSB (12b). Not relevant for Jericho.
   */
  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      fld_val = mac_in_longs[0] & 0xfff;
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_HIGH_VSI_CONFIGr, REG_PORT_ANY, 0, HIGH_VSI_MY_MACf,  fld_val));
  }

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 13, exit, WRITE_EPNI_MY_MACr(unit, REG_PORT_ANY, mac_in_long));
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_msb_set_unsafe()", 0, 0);
}

uint32
  arad_pp_mymac_msb_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_msb
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_MSB_SET_VERIFY);

  /* Nothing to verify */
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_msb_set_verify()", 0, 0);
}

uint32
  arad_pp_mymac_msb_get_verify(
    SOC_SAND_IN  int                                     unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_MSB_GET_VERIFY);

  /* Nothing to do */

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_msb_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the MSB of the MAC address of the device. Used for
 *     ingress termination and egress encapsulation.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_msb_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                           *my_mac_msb
  )
{
  uint32
    res = SOC_SAND_OK,
    mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S];
  uint64
	  fld_val,
	  reg_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_MSB_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(my_mac_msb);
  COMPILER_64_ZERO(fld_val);
  COMPILER_64_ZERO(reg_val);
  soc_sand_SAND_PP_MAC_ADDRESS_clear(my_mac_msb);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,READ_IHP_MY_MACr(unit, SOC_CORE_ALL, &reg_val));
  ARAD_FLD_FROM_REG64(IHP_MY_MACr, MY_MACf, fld_val, reg_val, 40, exit);

  mac_in_longs[0] = COMPILER_64_LO(fld_val);
  mac_in_longs[1] = COMPILER_64_HI(fld_val);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
    mac_in_longs,
    my_mac_msb
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_msb_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set MAC address LSB according to VSI.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vsi_lsb_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                                vsi_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IHP_VSI_LOW_CFG_1_TBL_DATA
    vsi_low_cfg_1_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VSI_LSB_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(my_mac_lsb);

  res = arad_pp_ihp_vsi_low_cfg_1_tbl_get_unsafe(
          unit,
          vsi_ndx,
          &vsi_low_cfg_1_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  /* Field size is 12b, so first 2 chars are relevant */
  vsi_low_cfg_1_tbl_data.my_mac = 0;
  vsi_low_cfg_1_tbl_data.my_mac = (my_mac_lsb->address[1] << 8) | my_mac_lsb->address[0];

  res = arad_pp_ihp_vsi_low_cfg_1_tbl_set_unsafe(
          unit,
          vsi_ndx,
          &vsi_low_cfg_1_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vsi_lsb_set_unsafe()", vsi_ndx, 0);
}

uint32
  arad_pp_mymac_vsi_lsb_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                                vsi_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    my_mac_lsb_max_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S] =
      {ARAD_PP_MYMAC_VSI_LSB_MAX+1, 0};
  SOC_SAND_PP_MAC_ADDRESS
    my_mac_lsb_max;
  uint8
    is_smaller;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VSI_LSB_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(vsi_ndx, ARAD_PP_VSI_ID_MAX, SOC_PPC_VSI_ID_OUT_OF_RANGE_ERR, 10, exit);
  
  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
          my_mac_lsb_max_longs,
          &my_mac_lsb_max
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  res = soc_sand_pp_mac_address_is_smaller(
          my_mac_lsb,
          &my_mac_lsb_max,
          &is_smaller
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  if (!is_smaller)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_VSI_LSB_OUT_OF_RANGE, 30, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vsi_lsb_set_verify()", vsi_ndx, 0);
}

uint32
  arad_pp_mymac_vsi_lsb_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                                vsi_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VSI_LSB_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(vsi_ndx, ARAD_PP_VSI_ID_MAX, SOC_PPC_VSI_ID_OUT_OF_RANGE_ERR, 10, exit);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vsi_lsb_get_verify()", vsi_ndx, 0);
}

/*********************************************************************
*     Set MAC address LSB according to VSI.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vsi_lsb_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_PP_IHP_VSI_LOW_CFG_1_TBL_DATA
    vsi_low_cfg_1_tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VSI_LSB_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(my_mac_lsb);

  res = arad_pp_ihp_vsi_low_cfg_1_tbl_get_unsafe(
          unit,
          vsi_ndx,
          &vsi_low_cfg_1_tbl_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  /* Field size is 12b */
  my_mac_lsb->address[0] = (uint8)vsi_low_cfg_1_tbl_data.my_mac;
  my_mac_lsb->address[1] = (uint8)((vsi_low_cfg_1_tbl_data.my_mac >> 8) & 0x0f);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vsi_lsb_get_unsafe()", vsi_ndx, 0);
}

/*********************************************************************
*     Set My-MAC according to Virtual Router Redundancy
 *     Protocol.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                    *vrrp_info
  )
{
  uint32
    res = SOC_SAND_OK,
    fld_val = 0,
    mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S];
  uint64 
      reg64_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_VRRP_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(vrrp_info);

  fld_val = vrrp_info->enable ? 1 : 0;

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_0r, SOC_CORE_ALL, 0, VRID_MY_MAC_ENABLEf,  fld_val));

#ifdef BCM_JERICHO_SUPPORT
  if (SOC_IS_JERICHO(unit)) {   /* None of the other registers are relevant for Jericho */
      SOC_SAND_EXIT_NO_ERROR; 
  }
#endif
  
  switch (vrrp_info->mode)
  {
  case SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED:
    if (vrrp_info->ipv6_enable && SOC_IS_ARAD_B1_AND_BELOW(unit)) {    /* ARADPLUS supports this mode and ipv6 distinction */
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_MODE_IPV6_NOT_SUPPORTED_ERR, 10, exit);
    }
    fld_val = SOC_PPC_MYMAC_VRRP_MODE_4K_VSI_8_VRID;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED:
    fld_val = vrrp_info->ipv6_enable ? SOC_PPC_MYMAC_VRRP_MODE_256_VSI_8_VRID_IPV4_8_VRID_IPV6 : SOC_PPC_MYMAC_VRRP_MODE_256_VSI_16_VRID;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_512_VSI_BASED:
    fld_val = vrrp_info->ipv6_enable ? SOC_PPC_MYMAC_VRRP_MODE_512_VSI_4_VRID_IPV4_4_VRID_IPV6 : SOC_PPC_MYMAC_VRRP_MODE_512_VSI_8_VRID;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_BASED:
    fld_val = vrrp_info->ipv6_enable ? SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_2_VRID_IPV4_2_VRID_IPV6 : SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_4_VRID;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_BASED:
    if (vrrp_info->ipv6_enable) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_MODE_IPV6_NOT_SUPPORTED_ERR, 10, exit);
    }
    fld_val = SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_2_VRID;
    break;
  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR, 20, exit);
    break;
  }

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_0r, SOC_CORE_ALL, 0, VRID_MY_MAC_MODEf,  fld_val));


#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit) && vrrp_info->mode != SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED) {
      fld_val = !(vrrp_info->ipv6_enable);
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, VRID_MY_MAC_IS_IPV4_MASKf,  fld_val));
  }
#endif /* BCM_88660_A0 */

  /* VRID_MY_MAC for IPV4 */
  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
    &vrrp_info->vrid_my_mac_msb,
    mac_in_longs
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  COMPILER_64_SET(reg64_val, mac_in_longs[1], mac_in_longs[0]);
  res = WRITE_IHP_VRID_MY_MACr(unit, 0, reg64_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  if (vrrp_info->ipv6_enable) {
      /* VRID_MY_MAC for IPV6 */
      /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
      /* coverity[overrun-buffer-val : FALSE] */   
      res = soc_sand_pp_mac_address_struct_to_long(
          &vrrp_info->ipv6_vrid_my_mac_msb,
          mac_in_longs
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      COMPILER_64_SET(reg64_val, mac_in_longs[1], mac_in_longs[0]);
      res = WRITE_IHP_VRID_MY_MACr(unit, 1, reg64_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_mymac_vrrp_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO                    *vrrp_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_VRRP_INFO_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_MYMAC_VRRP_INFO, vrrp_info, 10, exit);
  
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_info_set_verify()", 0, 0);
}

uint32
  arad_pp_mymac_vrrp_info_get_verify(
    SOC_SAND_IN  int                                     unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_VRRP_INFO_GET_VERIFY);

  /* Nothing to verify */
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set My-MAC according to Virtual Router Redundancy
 *     Protocol.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_VRRP_INFO                    *vrrp_info
  )
{
  uint32
    res = SOC_SAND_OK,
    fld_val,
    mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S];
  uint64
    reg64_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_VRRP_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(vrrp_info);

  soc_sand_SAND_PP_MAC_ADDRESS_clear(&(vrrp_info->vrid_my_mac_msb));
  soc_sand_SAND_PP_MAC_ADDRESS_clear(&(vrrp_info->ipv6_vrid_my_mac_msb));

  /* Read vrrp enable field */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_VTT_GENERAL_CONFIGS_0r, SOC_CORE_ALL, 0, VRID_MY_MAC_ENABLEf, &fld_val));
  vrrp_info->enable = fld_val ? TRUE: FALSE;

#ifdef BCM_JERICHO_SUPPORT
  /* In Jericho, mode is always bitmap, and all other fields are irrelevant. */
  if (SOC_IS_JERICHO(unit)) {
      vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED;
      SOC_SAND_EXIT_NO_ERROR;
  }
#endif /*BCM_JERICHO_SUPPORT*/

  /* Read the vrrp mode */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_VTT_GENERAL_CONFIGS_0r, SOC_CORE_ALL, 0, VRID_MY_MAC_MODEf, &fld_val));

  vrrp_info->ipv6_enable = 0;/* No need. The value will be set in this switch. To avoid coverity defect only. */
  switch (fld_val)
  {
  case SOC_PPC_MYMAC_VRRP_MODE_4K_VSI_8_VRID:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED;
    vrrp_info->ipv6_enable = 0;

#ifdef BCM_88660_A0 

  /* In ARADPLUS, a different register decides IPv6 distinction. It's on when distinction is disabled and
     vice versa */
  if (SOC_IS_ARADPLUS_A0(unit)) {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, VRID_MY_MAC_IS_IPV4_MASKf, &fld_val));
      vrrp_info->ipv6_enable = !fld_val;
  }

#endif /* BCM_88660_A0 */  
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_256_VSI_16_VRID:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED;
    vrrp_info->ipv6_enable = 0;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_256_VSI_8_VRID_IPV4_8_VRID_IPV6:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED;
    vrrp_info->ipv6_enable = 1;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_512_VSI_8_VRID:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_512_VSI_BASED;
    vrrp_info->ipv6_enable = 0;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_512_VSI_4_VRID_IPV4_4_VRID_IPV6:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_512_VSI_BASED;
    vrrp_info->ipv6_enable = 1;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_4_VRID:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_BASED;
    vrrp_info->ipv6_enable = 0;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_2_VRID_IPV4_2_VRID_IPV6:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_BASED;
    vrrp_info->ipv6_enable = 1;
    break;
  case SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_2_VRID:
    vrrp_info->mode = SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_BASED;
    vrrp_info->ipv6_enable = 0;
    break;
  }


  /* VRID MY MAC for IPV4 */
  res = READ_IHP_VRID_MY_MACr(unit, 0, &reg64_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  mac_in_longs[0] = COMPILER_64_LO(reg64_val);
  mac_in_longs[1] = COMPILER_64_HI(reg64_val);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
    mac_in_longs,
    &(vrrp_info->vrid_my_mac_msb)
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  if (vrrp_info->ipv6_enable) {
      /* VRID MY MAC for IPV6 */
      res = READ_IHP_VRID_MY_MACr(unit, 1, &reg64_val);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      mac_in_longs[0] = COMPILER_64_LO(reg64_val);
      mac_in_longs[1] = COMPILER_64_HI(reg64_val);

      /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
      /* coverity[overrun-buffer-val : FALSE] */   
      res = soc_sand_pp_mac_address_long_to_struct(
          mac_in_longs,
          &(vrrp_info->ipv6_vrid_my_mac_msb)
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_info_get_unsafe()", 0, 0);
}


/*********************************************************************
* NAME:
 *   arad_pp_mymac_vrrp_info_get_table_entry
 * TYPE:
 *   PROC
 * FUNCTION:
 *  Given the vsi, vrid and ipv6 distinction, get the VridMyMacMap entry associated with this vsi,
 *  as well as some other relevant parameters.
 *  In addition, the function validates that the mode is correct for ipv6 distinction.
 * INPUT:
 *  SOC_SAND_IN  int           unit                 Identifier of the device to access.
 *  SOC_SAND_IN  uint8         enable
 *  SOC_SAND_IN  uint32        vrrp_id_ndx
 *  SOC_SAND_IN  uint8         vrid
 *  SOC_SAND_OUT uint32        *entry_size
 *  SOC_SAND_OUT uint32        *nof_entries
 *  SOC_SAND_OUT uint8         *mode_support_ipv6
 *  SOC_SAND_OUT uint32        *global_bitmap_key
 *  SOC_SAND_OUT ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_DATA    *vrid_my_mac_map_tbl_data
       
 * REMARKS:
 *   
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
STATIC uint32 
    arad_pp_mymac_vrrp_info_get_table_entry(
       SOC_SAND_IN  int                                     unit,
       SOC_SAND_IN  uint32                                  vrrp_id_ndx,
       SOC_SAND_IN  uint8                                   vrid,
       SOC_SAND_OUT uint32                                  *entry_size,
       SOC_SAND_OUT uint32                                  *nof_entries,
       SOC_SAND_OUT uint8                                   *mode_support_ipv6,
       SOC_SAND_OUT uint32                                  *global_bitmap_key,
       SOC_SAND_OUT ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_DATA    *vrid_my_mac_map_tbl_data
       ){
    uint32
      res = SOC_SAND_OK,
      tbl_offset;
    SOC_PPC_MYMAC_VRRP_INFO
      vrrp_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* By default, entry is 8 bits. */
    *entry_size = 8;

    res = arad_pp_mymac_vrrp_info_get_unsafe(
      unit,
      &vrrp_info
      );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    switch(vrrp_info.mode)
    {
    case SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED:
        /* 4K VSIs x bitmap of 8 VRIDs:
            My-MAC is set if DA[47:3] == Cfg-VRID-My-MAC-0[47:3] and VRID-My-MAC[{VSI, DA[2:0]}] is set.
            In case of aradplus, the 3 lsbs stand for cam table index. */
        *global_bitmap_key = ARAD_PP_MYMAC_VRRP_BITMAP_KEY_ALL_VSI_BASED(unit, vrrp_id_ndx, vrid);
        *entry_size = 1;
        break;
    case SOC_PPC_MYMAC_VRRP_MODE_2K_VSI_BASED:
        /* 2K VSIs x 2 VRIDs:
            Set {VRID-0, VRID1} = VRID-My-MAC[VSI] (2 x 8 bits) */
        *nof_entries = 2;
        break;
    case SOC_PPC_MYMAC_VRRP_MODE_1K_VSI_BASED:
        /* 1K VSIs x (2 IPv4 VRIDs and 2 IPv6 VRIDs)
           or
           1K VSIs x 4 VRIDs 
                     Set {VRID-0..3} = VRID-My-MAC[VSI] (4 x 8 bits) */
        *nof_entries = 4;
        break;
    case SOC_PPC_MYMAC_VRRP_MODE_512_VSI_BASED:
        /* mode 5: 512 VSIs x (4 IPv4 VRIDs and 4 IPv6 VRIDs)
           or
           mode 4: 512 VSIs x 8 VRIDs
                     Set {VRID-0..7} = VRID-My-MAC[VSI] (8 x 8 bits) */
        *nof_entries = 8;
        break;
    case SOC_PPC_MYMAC_VRRP_MODE_256_VSI_BASED:
        /* mode 7: 256 VSIs x (8 IPv4 VRIDs and 8 IPv6 VRIDs)
           or
           mode 8: 256 VSIs x 16 VRIDs
                     Set {VRID-0..15} = VRID-My-MAC[VSI] (16 x 8 bits) */
        *nof_entries = 16;
        break;
    default:
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR, 80, exit);
      break;
    }

    if (vrrp_info.mode != SOC_PPC_MYMAC_VRRP_MODE_ALL_VSI_BASED) {
        *global_bitmap_key = (*entry_size) * (*nof_entries) * vrrp_id_ndx;
    }
    tbl_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_TBL_OFFSET(unit, *global_bitmap_key);

    if (vrrp_info.ipv6_enable) {
        *mode_support_ipv6 = 1;
    }

    /* max nof_entries is 16 and each entry is 8 bits, so all entries are in the same reg_offset (nof_bits_in_line bits reg) */
    res = arad_pp_ihp_vrid_my_mac_map_tbl_get_unsafe(
      unit,
      tbl_offset,
      vrid_my_mac_map_tbl_data
      );
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_info_get_table_entry()", 0, 0);
}


#ifdef BCM_88660_A0

/*********************************************************************
* NAME:
 *   arad_pp_mymac_vrid_mymac_mac_allocate_full_table
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Allocate memory of size table_size_bytes, using dma, if active, or regular
 *  dpp allocation.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN  uint32         table_size_bytes -
 *      The size, in bytes, to be allocated.
 *  SOC_SAND_IN  int            use_dma -
 *      Whether to allocate using dma (dma must be on in the device).
 *  SOC_SAND_OUT uint32**       alloced_mem -
 *      A pointer to a the table pointer. The table's
 *      pointer will be filled with a pointer to the allocated mem.
 *
 * REMARKS:
 *   
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
STATIC uint32 
arad_pp_mymac_vrid_mymac_mac_allocate_full_table(
   SOC_SAND_IN  int         unit,
   SOC_SAND_IN  uint32      table_size_bytes,
   SOC_SAND_IN  int         use_dma,
   SOC_SAND_OUT uint32**    alloced_mem){

    uint32
        res = SOC_SAND_OK;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (use_dma) { /* use DMA */
        *alloced_mem = soc_cm_salloc(unit, sizeof(uint32) * table_size_bytes, "my_table");
    } else { /* do not use DMA, read tables entry by entry */
        ARAD_ALLOC_ANY_SIZE(*alloced_mem, uint32, table_size_bytes, "my_table");
    }

    if (*alloced_mem == NULL) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 10, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrid_mymac_mac_allocate_full_table()", 0, 0);
}


/*********************************************************************
* NAME:
 *   arad_pp_mymac_vrid_mymac_map_set_to_all_vsi
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set or unset the cam entry for all VSIs on device.
 * INPUT:
 *   int    unit:       (IN) unit indetifier
 *  int     cam_index   (IN) Cam index to be set or unset on all VSIs.
 *  int     enable      (IN) 1 means set cam entry on all VSIs, 0 means unset.
 *
 * REMARKS:
 *  We assume that the device is ARADPLUS and above (in bmp mode in case of ARADPLUS)
 *  And that the given cam index is legal.
 *   
 * RETURNS:
 *   SOC_E_NONE or errors.
*********************************************************************/
soc_error_t
arad_pp_mymac_vrid_mymac_map_set_to_all_vsi(int unit, int cam_index, int enable){
    int rv, i;
    uint32  bits_per_vsi;
    uint32  table_size_bytes;
    uint32  cam_index_mask = 0;
    uint32  mask;
    uint32  nof_lines;

    uint32 *alloced_mem = NULL;
    int core;

    int use_dma =
#ifdef PLISIM
    (SAL_BOOT_PLISIM) ? 0 :
#endif
      soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IHP_VRID_MY_MAC_MAPm)); /* check if we can use DMA */


    SOCDNX_INIT_FUNC_DEFS;

    /** 
      * To configure all VSIs, take the following actions:
      * 1. Read whole VridMyMacMap table. The table will be in the format of a uint32 array.
      * 2. Create the mask for the relevant bit on all entries that fit on a uint32.
      * 3. For every uint32 in the table, apply the mask.
      * 4. Write back to HW.
      */

    /* 1. Read vrid table. */

    /* Prepare variables */
    bits_per_vsi = SOC_PPC_VRRP_CAM_MAX_NOF_ENTRIES(unit);

    table_size_bytes = (bits_per_vsi * SOC_PPC_VRRP_MAX_VSI(unit)); /* The size is currently in bits, we'll translate it to bytes. */

    nof_lines = (table_size_bytes / SOC_DPP_DEFS_GET(unit, vrrp_mymac_map_width));  /* Since table width is also in bits, nof_lines is in bytes. */

    table_size_bytes = SOC_SAND_TRANSLATE_BITS_TO_BYTES(table_size_bytes);

    table_size_bytes += nof_lines * SOC_SAND_REG_SIZE_BYTES; 

    rv = arad_pp_mymac_vrid_mymac_mac_allocate_full_table(unit, table_size_bytes, use_dma, &alloced_mem );
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* 2. Create mask */
    SOC_SAND_SET_BIT(cam_index_mask, TRUE, cam_index);  /* Mask the bit representing the cam index. */
    mask = 0;
    for (i = 0 ; i < SOC_SAND_REG_SIZE_BITS ; i += bits_per_vsi) {
        mask |= cam_index_mask << i;
    }


          
    rv = soc_mem_read_range(unit, IHP_VRID_MY_MAC_MAPm, IHP_BLOCK(unit, 0), 0, nof_lines - 1 /*Index of the last line is nof - 1 */, alloced_mem);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* 3. Iterate over the table, set the mask on each entry. */
    for (i = 0 ; i < table_size_bytes ; i++) {
        SOC_SAND_APPLY_MASK(alloced_mem[i], enable, mask);
    }

    for (core = 0 ; core < SOC_DPP_DEFS_GET(unit, nof_cores) ; core++) {
        /* 4. Write back to HW. */
        rv = soc_mem_write_range(unit, IHP_VRID_MY_MAC_MAPm, IHP_BLOCK(unit, core), 0, nof_lines - 1 /*Index of the last line is nof - 1 */, alloced_mem);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
    }

exit:
    /* Free the allocated memory */
    if (alloced_mem) {
        if (use_dma) {
            soc_cm_sfree(unit, alloced_mem);
        } else {
            ARAD_FREE(alloced_mem);
        }
    }
    SOCDNX_FUNC_RETURN;
}

#endif

/*********************************************************************
*     Enable / Disable MyMac/MyVRID according to VRRP-ID and
 *     Mac Address LSB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_mac_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_IN  uint8                                  enable
  )
{
  uint32
    res = SOC_SAND_OK,
    global_bitmap_key = 0,
    tbl_offset,
    reg_offset = 0,
    entry_offset = 0,
    entry_size = 8, /* how many bits to write */
    nof_entries = 0,
    curr_data,
    expected_data,
    data,
    i,
    first_free_entry = 0,
    first_free_reg = 0,
    nof_bits_in_line = SOC_DPP_DEFS_GET(unit, vrrp_mymac_map_width);
  ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_DATA
    vrid_my_mac_map_tbl_data;
  uint8
    mode_support_ipv6 = 0,
    mac_enable,
    mac_is_ipv6,
    entry_in_use,
    found_first_free = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VRRP_MAC_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(vrrp_mac_lsb_key);

  mac_enable = SOC_SAND_GET_BIT(enable, 0);
  mac_is_ipv6 = SOC_SAND_GET_BIT(enable, 1);

  /* Get all vrrp info, and the table entry. */
  res =  arad_pp_mymac_vrrp_info_get_table_entry(unit,
                                                   vrrp_id_ndx,
                                                   vrrp_mac_lsb_key->address[0],
                                                   &entry_size,
                                                   &nof_entries,
                                                   &mode_support_ipv6,
                                                   &global_bitmap_key,
                                                   &vrid_my_mac_map_tbl_data
                                                   );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  tbl_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_TBL_OFFSET(unit, global_bitmap_key);

  if (entry_size == 1) { /* 8 bit bitmap */
      reg_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_REG_OFFSET(global_bitmap_key, nof_bits_in_line);
      entry_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_ENTRY_OFFSET(global_bitmap_key, nof_bits_in_line);
      SOC_SAND_SET_BIT(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset], mac_enable, entry_offset);
  }
  else {
      if (mac_enable) {
          data = vrrp_mac_lsb_key->address[0] & 0xff;

          /* go over all used entries to make sure this mac is not already there
             and find first free entry */
          for (i = 0; i < nof_entries; i++) {
              if (mode_support_ipv6) {
                  /* ipv6: VRID is odd, ipv4: VRID is even */
                  if ((mac_is_ipv6 && !(i % 2)) || (!mac_is_ipv6 && (i % 2))) {
                      continue;
                  }
              }

              reg_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) / SOC_SAND_REG_SIZE_BITS;
              entry_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) % SOC_SAND_REG_SIZE_BITS;

              /* check if entry is in use */
              arad_pp_sw_db_vrrp_mac_entry_use_get(unit, tbl_offset, (entry_offset / entry_size) + (reg_offset * 4), &entry_in_use);

              if (entry_in_use) {
                  /* get entry */
                  curr_data = 0;
                  res = soc_sand_bitstream_get_any_field(&(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset]), entry_offset, entry_size, &curr_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

                  if (curr_data == data) { /* entry is already there */
                      break;
                  }
              }
              else { /* entry is not in use */
                  if (!found_first_free) {
                      first_free_entry = entry_offset;
                      first_free_reg = reg_offset;
                      found_first_free = 1;
                  }
              }
          }

          if (i == nof_entries) { /* entry is not in the table */
              if (found_first_free) {
                  /* add new entry in the first free place */
                  res = soc_sand_bitstream_set_any_field(&data, first_free_entry, entry_size, &(vrid_my_mac_map_tbl_data.vrid_my_mac_map[first_free_reg]));
                  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

                  /* mark entry as in use */
                  arad_pp_sw_db_vrrp_mac_entry_use_set(unit, tbl_offset, (first_free_entry / entry_size) + (first_free_reg * 4), 1);
              }
              else { /* table is full */
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_VRID_MAP_FULL_ERR, 120, exit);
              }
          }
      }
      else { /* delete */
          expected_data = vrrp_mac_lsb_key->address[0] & 0xff;
          data = 0;

          /* search for the entry */
          for (i = 0; i < nof_entries; i++) {
              if (mode_support_ipv6) {
                  /* ipv6: VRID is odd, ipv4: VRID is even */
                  if ((mac_is_ipv6 && !(i % 2)) || (!mac_is_ipv6 && (i % 2))) {
                      continue;
                  }
              }

              reg_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) / SOC_SAND_REG_SIZE_BITS;
              entry_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) % SOC_SAND_REG_SIZE_BITS;

              /* check if entry is in use */
              arad_pp_sw_db_vrrp_mac_entry_use_get(unit, tbl_offset, (entry_offset / entry_size) + (reg_offset * 4), &entry_in_use);

              if (entry_in_use) {
                  /* get entry */
                  curr_data = 0;
                  res = soc_sand_bitstream_get_any_field(&(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset]), entry_offset, entry_size, &curr_data);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

                  if (curr_data == expected_data) { /* found the entry */
                      break;
                  }
              }
          }

          if (i == nof_entries) { /* entry was not found */
              goto exit;
              /*SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_VRID_MAP_ENTRY_NOT_FOUND_ERR, 120, exit);*/
          }
          else {
              /* delete the entry */
              res = soc_sand_bitstream_set_any_field(&data, entry_offset, entry_size, &(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset]));
              SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

              /* mark entry as not in use */
              arad_pp_sw_db_vrrp_mac_entry_use_set(unit, tbl_offset, (entry_offset / entry_size) + (reg_offset * 4), 0);
          }
      }
  }
 
  res = arad_pp_ihp_vrid_my_mac_map_tbl_set_unsafe(
    unit,
    tbl_offset,
    &vrid_my_mac_map_tbl_data
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_mac_set_unsafe()", vrrp_id_ndx, 0);
}

uint32
  arad_pp_mymac_vrrp_mac_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_IN  uint8                                  enable
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VRRP_MAC_SET_VERIFY);

  res = arad_pp_mymac_vrrp_mac_get_verify(
    unit,
    vrrp_id_ndx,
    vrrp_mac_lsb_key
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_mac_set_verify()", vrrp_id_ndx, 0);
}

uint32
  arad_pp_mymac_vrrp_mac_get_verify(
    SOC_SAND_IN  int              unit,
    SOC_SAND_IN  uint32               vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS    *vrrp_mac_lsb_key
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_MYMAC_VRRP_INFO
    vrrp_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VRRP_MAC_GET_VERIFY);

  res = soc_ppd_mymac_vrrp_info_get(
    unit,
    &vrrp_info
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  if (!vrrp_info.enable)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_VRRP_IS_DISABLED_ERR, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_mac_get_verify()", vrrp_id_ndx, 0);
}

/*********************************************************************
*     Enable / Disable MyMac/MyVRID according to VRRP-ID and
 *     Mac Address LSB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_vrrp_mac_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 vrrp_id_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                *vrrp_mac_lsb_key,
    SOC_SAND_OUT uint8                                  *enable
  )
{
  uint32
    res = SOC_SAND_OK,
    global_bitmap_key = 0,
    tbl_offset,
    reg_offset,
    entry_offset = 0,
    entry_size = 8, /* how many bits to read */
    nof_entries = 0,
    curr_data,
    expected_data,
    i,
    nof_bits_in_line = SOC_DPP_DEFS_GET(unit, vrrp_mymac_map_width);
  ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_DATA
    vrid_my_mac_map_tbl_data;
  uint8
    mode_support_ipv6 = 0,
    entry_in_use;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MYMAC_VRRP_MAC_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(vrrp_mac_lsb_key);
  SOC_SAND_CHECK_NULL_INPUT(enable);

  /* Get all vrrp info, and the table entry. */
  res =  arad_pp_mymac_vrrp_info_get_table_entry(unit,
                                                   vrrp_id_ndx,
                                                   vrrp_mac_lsb_key->address[0],
                                                   &entry_size,
                                                   &nof_entries,
                                                   &mode_support_ipv6,
                                                   &global_bitmap_key,
                                                   &vrid_my_mac_map_tbl_data
                                                   );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  tbl_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_TBL_OFFSET(unit, global_bitmap_key);

  if (entry_size == 1) { /* 8 bit bitmap */
      reg_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_REG_OFFSET(global_bitmap_key, nof_bits_in_line);
      entry_offset = ARAD_PP_MYMAC_VRRP_CALCULATE_BMP_ENTRY_OFFSET(global_bitmap_key, nof_bits_in_line);
      *enable = SOC_SAND_GET_BIT(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset], entry_offset);
  }
  else {
      *enable = 0;
      expected_data = vrrp_mac_lsb_key->address[0] & 0xff;

      for (i = 0; i < nof_entries; i++) {
          reg_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) / SOC_SAND_REG_SIZE_BITS;
          entry_offset = ((global_bitmap_key + (entry_size * i)) % nof_bits_in_line) % SOC_SAND_REG_SIZE_BITS;

          /* check if entry is in use */
          arad_pp_sw_db_vrrp_mac_entry_use_get(unit, tbl_offset, (entry_offset / 8) + (reg_offset * 4), &entry_in_use);

          if (entry_in_use) {
              /* get entry */
              curr_data = 0;
              res = soc_sand_bitstream_get_any_field(&(vrid_my_mac_map_tbl_data.vrid_my_mac_map[reg_offset]), entry_offset, entry_size, &curr_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

              if (curr_data == expected_data) { /* found the entry */
                  if (mode_support_ipv6) {
                      if (i % 2) { /* VRID is odd -> ipv6 */
                          SOC_SAND_SET_BIT(*enable, 1, 1); 
                      }
                      else { /* VRID is even -> ipv4 */
                          SOC_SAND_SET_BIT(*enable, 1, 0); 
                      }
                      /* do not break - continue to check if both ipv4 and ipv6 are enabled for this address */
                  }
                  else {
                      SOC_SAND_SET_BIT(*enable, 1, 0); 
                      break;
                  }
              }
          }
      }
  }
      
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_vrrp_mac_get_unsafe()", vrrp_id_ndx, 0);
}

#ifdef BCM_88660_A0

soc_error_t
arad_pp_mymac_vrrp_cam_info_set(int unit,SOC_PPC_VRRP_CAM_INFO *info){
    int rv;
    uint32
      mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S] = {0},
      data[2] = {0},
      entry_offset = info->vrrp_cam_index;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* Convert the mac struct to long so we can write into the register. */
    rv = soc_sand_pp_mac_address_struct_to_long(
        &info->mac_addr,
        mac_in_longs
      );
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* Fill the register entries. */

    soc_IHP_VRID_MY_MAC_CAMm_field32_set(unit, data, IS_IPV4f, info->is_ipv4_entry);
    soc_IHP_VRID_MY_MAC_CAMm_field32_set(unit, data, VALIDf, 1);      
    soc_IHP_VRID_MY_MAC_CAMm_field_set(unit, data, DAf, mac_in_longs);

    /* Write the register. */
    rv = WRITE_IHP_VRID_MY_MAC_CAMm(unit, MEM_BLOCK_ANY, entry_offset, data);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*DEPECRATED. Get functionality is taken from the template manager.*/
soc_error_t
arad_pp_mymac_vrrp_cam_info_get(int unit, SOC_PPC_VRRP_CAM_INFO *info){
    int rv;
    uint32
      data[2] = {0},
      entry_offset = info->vrrp_cam_index;
    uint8 exists;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* Read entry */
    rv = READ_IHP_VRID_MY_MAC_CAMm(unit, MEM_BLOCK_ANY, entry_offset, data);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* Make sure that the entry is not empty */
    exists = soc_IHP_VRID_MY_MAC_CAMm_field32_get(unit, data, VALIDf);
    if (!exists) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("Cam entry doesn't exist")));
    }

    /* Copy the mac address from the register. */
    rv = soc_sand_pp_mac_address_long_to_struct(data, &(info->mac_addr));
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    /* Copy the ipv4 distinction bit */
    info->is_ipv4_entry = soc_IHP_VRID_MY_MAC_CAMm_field32_get(unit, data, IS_IPV4f);
    
exit:
    SOCDNX_FUNC_RETURN;
}


soc_error_t
arad_pp_mymac_vrrp_cam_info_delete(int unit, uint8 cam_index){
    int rv;
    uint32
      empty_data[2] = {0},
      entry_offset = cam_index;
    SOCDNX_INIT_FUNC_DEFS;

    /* Write an empty register into the field */
    rv = WRITE_IHP_VRID_MY_MAC_CAMm(unit, MEM_BLOCK_ANY, entry_offset, empty_data);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

#endif /* BCM_88660_A0 */  

/*********************************************************************
*     Set TRILL My-Nick-Name and reserved Nick-Name.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_trill_info_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                      *trill_info
  )
{
  uint32
    res = SOC_SAND_OK,
    mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S], 
    fld_val_64_lo, 
    fld_val_64_hi,
    fld_val_32;
  uint64
	  mac_in_long,
	  reg_val,
      fld_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_TRILL_INFO_SET_UNSAFE);
  SOC_SAND_CHECK_NULL_INPUT(trill_info);

  COMPILER_64_ZERO(reg_val);
  COMPILER_64_ZERO(mac_in_long);
  fld_val_64_lo = 0;
  fld_val_64_hi = 0;
  fld_val_32 = 0; 

  /* Nickname */
  COMPILER_64_SET(reg_val,0,trill_info->my_nick_name );
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_TRILL_CONFIGr, SOC_CORE_ALL, 0, TRILL_NICKNAMEf,  reg_val));

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  112,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_CFG_TRILL_NICKNAME_ROUTINGr, SOC_CORE_ALL, 0, CFG_TRILL_NICKNAME_ROUTINGf,  reg_val));
      /* update EPNI nicknames for TRILL */
      SHR_BITCOPY_RANGE(&fld_val_64_lo, 0, &(trill_info->my_nick_name), 0, 16);
      SHR_BITCOPY_RANGE(&fld_val_64_lo, 16, &(trill_info->virtual_nick_names[0]) ,0, 16);
      SHR_BITCOPY_RANGE(&fld_val_64_hi, 0, &(trill_info->virtual_nick_names[1]), 0, 16);
      SHR_BITCOPY_RANGE(&fld_val_64_hi, 16, &(trill_info->virtual_nick_names[2]), 0, 16);
      COMPILER_64_SET(fld_val, fld_val_64_hi, fld_val_64_lo);

      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  113,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_modify(unit, EPNI_CFG_TRILL_NICKNAME_BRIDGINGr, SOC_CORE_ALL, 0, CFG_TRILL_NICKNAME_BRIDGINGf,  fld_val));


      /* update IHP nicknames */
      /* update register fields in variable */
      COMPILER_64_SET(fld_val, 0, trill_info->my_nick_name);
      ARAD_FLD_TO_REG64(IHP_MY_TRILL_NICKNAMEr, MY_TRILL_NICKNAME_0f, fld_val, reg_val, 114, exit);
      COMPILER_64_SET(fld_val, 0, trill_info->virtual_nick_names[0]);
      ARAD_FLD_TO_REG64(IHP_MY_TRILL_NICKNAMEr, MY_TRILL_NICKNAME_1f, fld_val, reg_val, 115, exit);
      COMPILER_64_SET(fld_val, 0, trill_info->virtual_nick_names[1]);
      ARAD_FLD_TO_REG64(IHP_MY_TRILL_NICKNAMEr, MY_TRILL_NICKNAME_2f, fld_val, reg_val, 116, exit);
      COMPILER_64_SET(fld_val, 0, trill_info->virtual_nick_names[2]);
      ARAD_FLD_TO_REG64(IHP_MY_TRILL_NICKNAMEr, MY_TRILL_NICKNAME_3f, fld_val, reg_val, 117, exit);
      /* update register with variable */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  118,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHP_MY_TRILL_NICKNAMEr(unit, SOC_CORE_ALL,  reg_val));

      /* update skip ethernet per nickname */
      SHR_BITCOPY_RANGE(&fld_val_32, 0, &(trill_info->skip_ethernet_my_nickname), 0, 1); 
      SHR_BITCOPY_RANGE(&fld_val_32, 1, &(trill_info->skip_ethernet_virtual_nick_names[0]), 0, 1); 
      SHR_BITCOPY_RANGE(&fld_val_32, 2, &(trill_info->skip_ethernet_virtual_nick_names[1]), 0, 1); 
      SHR_BITCOPY_RANGE(&fld_val_32, 3, &(trill_info->skip_ethernet_virtual_nick_names[2]), 0, 1); 
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  119,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, TRILL_SKIP_ETHERNETf, fld_val_32)); 
  } else
#endif /* BCM_88660_A0 */
	{
		 /* ARAD */
 	   SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, MY_TRILL_NICKNAMEf,  trill_info->my_nick_name));
  }

  /* Rbridges mac */
  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
    &trill_info->all_rbridges_mac,
    mac_in_longs
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  COMPILER_64_SET(mac_in_long, mac_in_longs[1], mac_in_longs[0]);
  COMPILER_64_ZERO(reg_val);

  soc_reg64_field_set(unit, IHP_ALL_RBRIDGES_MAC_CONFIGr, &reg_val, ALL_RBRIDGES_MACf, mac_in_long);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHP_ALL_RBRIDGES_MAC_CONFIGr(unit, SOC_CORE_ALL,  reg_val));

  /* Esadi mac */
  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
    &trill_info->all_esadi_rbridges,
    mac_in_longs
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  COMPILER_64_SET(mac_in_long, mac_in_longs[1], mac_in_longs[0]);

  soc_reg64_field_set(unit, IHP_ALL_ESADI_RBRIDGESr, &reg_val, ALL_ESADI_RBRIDGESf, mac_in_long);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHP_ALL_ESADI_RBRIDGESr(unit, SOC_CORE_ALL, reg_val));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_trill_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_mymac_trill_info_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO                        *trill_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_TRILL_INFO_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_MYMAC_TRILL_INFO, trill_info, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_trill_info_set_verify()", 0, 0);
}

uint32
  arad_pp_mymac_trill_info_get_verify(
    SOC_SAND_IN  int                                     unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_TRILL_INFO_GET_VERIFY);

  /* Nothing to verify */
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_trill_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Set TRILL My-Nick-Name and reserved Nick-Name.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mymac_trill_info_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_PPC_MYMAC_TRILL_INFO                        *trill_info
  )
{
  uint32
    res = SOC_SAND_OK,
    mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S], 
    fld_val_32;
  uint64 
	  mac_in_long,
	  reg_val,
      fld_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_MYMAC_TRILL_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(trill_info);
  COMPILER_64_ZERO(mac_in_long);
  COMPILER_64_ZERO(reg_val);
  COMPILER_64_ZERO(fld_val);
  fld_val_32 = 0; 

  SOC_PPC_MYMAC_TRILL_INFO_clear(trill_info);

  /* my nickname */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, EPNI_TRILL_CONFIGr, SOC_CORE_ALL, 0, TRILL_NICKNAMEf, &fld_val));
  trill_info->my_nick_name = COMPILER_64_LO(fld_val);

  #ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
      /* virtual nicknames */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, IHP_MY_TRILL_NICKNAMEr, SOC_CORE_ALL, 0, MY_TRILL_NICKNAME_1f, &fld_val));
      trill_info->virtual_nick_names[0] = COMPILER_64_LO(fld_val);

      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, IHP_MY_TRILL_NICKNAMEr, SOC_CORE_ALL, 0, MY_TRILL_NICKNAME_2f, &fld_val));
      trill_info->virtual_nick_names[1] = COMPILER_64_LO(fld_val);

      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  13,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, IHP_MY_TRILL_NICKNAMEr, SOC_CORE_ALL, 0, MY_TRILL_NICKNAME_3f, &fld_val));
      trill_info->virtual_nick_names[2] = COMPILER_64_LO(fld_val);

      /* skip ethernet per nicknames */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  14,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, TRILL_SKIP_ETHERNETf, &fld_val_32));
      SHR_BITCOPY_RANGE(&(trill_info->skip_ethernet_my_nickname),           0, &fld_val_32, 0, 1); 
      SHR_BITCOPY_RANGE(&(trill_info->skip_ethernet_virtual_nick_names[0]), 0, &fld_val_32, 1, 1); 
      SHR_BITCOPY_RANGE(&(trill_info->skip_ethernet_virtual_nick_names[1]), 0, &fld_val_32, 2, 1); 
      SHR_BITCOPY_RANGE(&(trill_info->skip_ethernet_virtual_nick_names[2]), 0, &fld_val_32, 3, 1); 
  }
  #endif

  /* Rbridges mac */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,READ_IHP_ALL_RBRIDGES_MAC_CONFIGr(unit, SOC_CORE_ALL, &reg_val));
  mac_in_long = soc_reg64_field_get(unit, IHP_ALL_RBRIDGES_MAC_CONFIGr, reg_val, ALL_RBRIDGES_MACf);

  mac_in_longs[0] = COMPILER_64_LO(mac_in_long);
  mac_in_longs[1] = COMPILER_64_HI(mac_in_long);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
    mac_in_longs,
    &trill_info->all_rbridges_mac
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  /* Esadi mac */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,READ_IHP_ALL_ESADI_RBRIDGESr(unit, 0, &reg_val));
  mac_in_long = soc_reg64_field_get(unit, IHP_ALL_ESADI_RBRIDGESr, reg_val, ALL_ESADI_RBRIDGESf);

  mac_in_longs[0] = COMPILER_64_LO(mac_in_long);
  mac_in_longs[1] = COMPILER_64_HI(mac_in_long);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
    mac_in_longs,
    &trill_info->all_esadi_rbridges
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mymac_trill_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set the MSB of the My-B-MAC. My-B-MAC forwards the
 *     packets to the I Component, and is added as the SA when
 *     sending toward the backbone network.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_msb_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                    *my_bmac_msb
  )
{
  uint32
      mac_in_long[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S],
      reg_vals[2],
      field_val,
      res = SOC_SAND_OK;
  uint64
      reg_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_MSB_SET_UNSAFE);

  COMPILER_64_ZERO(reg_val);
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf, &field_val));
  if (field_val != 0x2)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR, 7, exit);
  }

  SOC_SAND_CHECK_NULL_INPUT(my_bmac_msb);

  ARAD_CLEAR(mac_in_long, uint32, SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S);
  ARAD_CLEAR(reg_vals, uint32, 2);

  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_struct_to_long(
    my_bmac_msb,
    mac_in_long
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  res = soc_sand_bitstream_get_any_field(
	  mac_in_long,
	  0,
	  ARAD_PP_MY_BMAC_MSB_MSB + 1,
	  reg_vals
	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  COMPILER_64_SET(reg_val, reg_vals[1], reg_vals[0]);

  /*
   * set mymac msb in egress
   */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,WRITE_EPNI_BSAr(unit, SOC_CORE_ALL,  reg_val));

  ARAD_CLEAR(reg_vals, uint32, 2);

  res = soc_sand_bitstream_get_any_field(
    mac_in_long,
	ARAD_PP_MY_BMAC_MSB_LSB,
    ARAD_PP_MY_BMAC_MSB_MSB + 1 - ARAD_PP_MY_BMAC_MSB_LSB,
    reg_vals
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  COMPILER_64_SET(reg_val, reg_vals[1], reg_vals[0]);

  /*
   * set mymac msb in ingress
   */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,WRITE_IHP_MY_B_MAC_MSBr(unit, SOC_CORE_ALL,  reg_val));

  /* Pass the BSA to the prge variable */
  if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "mim_spb_enable", 0)) {
      uint64 var_check;
      COMPILER_64_ZERO(var_check);
      res = arad_pp_prge_program_var_get(unit, ARAD_EGR_PROG_EDITOR_PROG_MIM_SPB_BRIDGE_TO_OVERLAY, &var_check);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      if (COMPILER_64_IS_ZERO(var_check) || COMPILER_64_IS_ZERO(reg_val)) {
          res = soc_sand_bitstream_get_any_field(
            mac_in_long,
            0,
            ARAD_PP_MY_BMAC_MSB_MSB + 1,
            reg_vals
            );
          SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

          COMPILER_64_SET(reg_val, reg_vals[1], reg_vals[0]);
          res = arad_pp_prge_program_var_set(unit, ARAD_EGR_PROG_EDITOR_PROG_MIM_SPB_BRIDGE_TO_OVERLAY, reg_val);
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

          res = arad_pp_prge_program_var_set(unit, ARAD_EGR_PROG_EDITOR_PROG_MIM_SPB_UC_ROUTE_TO_OVERLAY, reg_val);
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          if (SOC_DPP_CONFIG(unit)->pp.custom_ip_route) {
              /* 0x0800 should be preserved in the highest 2 bytes of this prge var*/
              COMPILER_64_SET(reg_val, reg_vals[1] | 0x08000000, reg_vals[0]);
              res = arad_pp_prge_program_var_set(unit, ARAD_EGR_PROG_EDITOR_PROG_MIM_TO_IPOETH, reg_val);
              SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
          }

      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_lif_my_bmac_msb_set_unsafe()",0,0);
}

uint32
  arad_pp_lif_my_bmac_msb_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_bmac_msb
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_MSB_SET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_msb_set_verify()", 0, 0);
}

uint32
  arad_pp_lif_my_bmac_msb_get_verify(
    SOC_SAND_IN  int                                     unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_MSB_GET_VERIFY);

  /* Do nothing */
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_msb_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the MSB of the My-B-MAC. My-B-MAC forwards the
 *     packets to the I Component, and is added as the SA when
 *     sending toward the backbone network.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_msb_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_OUT SOC_SAND_PP_MAC_ADDRESS                           *my_bmac_msb
  )
{
  uint32
	  mac_in_long[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S],
	  reg_vals[2],
      res = SOC_SAND_OK,
	  field_val;
  uint64
	  reg_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_MSB_GET_UNSAFE);
  COMPILER_64_ZERO(reg_val);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf, &field_val));
  if (field_val != 0x2)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR, 7, exit);
  }

  SOC_SAND_CHECK_NULL_INPUT(my_bmac_msb);

  soc_sand_SAND_PP_MAC_ADDRESS_clear(my_bmac_msb);

  ARAD_CLEAR(mac_in_long, uint32, SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S);
  ARAD_CLEAR(reg_vals, uint32, 2);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,READ_EPNI_BSAr(unit, SOC_CORE_ALL, &reg_val));
 
  reg_vals[0] = COMPILER_64_LO(reg_val);
  reg_vals[1] = COMPILER_64_HI(reg_val);

  res = soc_sand_bitstream_get_any_field(
	  reg_vals,
	  0,
	  ARAD_PP_MY_BMAC_MSB_MSB + 1,
	  mac_in_long
	  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  /* The function soc_sand_pp_mac_address_long_to_struct reads from indecies 0 and 1 of the first parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
  res = soc_sand_pp_mac_address_long_to_struct(
	mac_in_long,
	my_bmac_msb
	);
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_lif_my_bmac_msb_get_unsafe()",0,0);
}

/*********************************************************************
*     Set the My-B-MAC LSB according to the source system
 *     port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_port_lsb_set_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                           *src_sys_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb,
    SOC_SAND_IN  uint8                                     enable
  )
{
  uint32
    res = SOC_SAND_OK,
    reg_offset,
    bit_offset,
    phy_sys_port_ndx,
    entry_offset,
    entry_index,
    mac_lsb,
    *tmp,
    field_val;
  ARAD_PP_EGQ_AUX_TABLE_TBL_DATA
    aux_table_tbl_data;
  soc_reg_above_64_val_t
    fld_above64_val,
    reg_above64_val;
  ARAD_REG_FIELD
    phys_port_id_fld;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_sys_port_ndx);
  SOC_SAND_CHECK_NULL_INPUT(my_mac_lsb);

  /* verify aux data table mode is MiM (PBP) */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf, &field_val));
  if (field_val != 0x2)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR, 7, exit);
  }

  /* check if port is PBP (backbone port) or CBP (customer port)
     for backbone port - set the my-BMAC-LSB for termination
     for customer port - set the B-SA-LSB for encapsultation
     (also encapsulation is done in the backbone port, the encasulated B-SA-LSB is according to the src port) */

  
  if (src_sys_port_ndx->sys_port_type == SOC_SAND_PP_NOF_SYS_PORT_TYPES) { /* port is invalid - set global LSB */
      
      SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
      SOC_REG_ABOVE_64_CLEAR(reg_above64_val);

      /* Set enable bit for this lsb */
      reg_offset = my_mac_lsb->address[0] / 32;
      bit_offset = my_mac_lsb->address[0] % 32;

      SOC_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_IHP_MY_B_MAC_LSB_BITMAPr(unit, REG_PORT_ANY, reg_above64_val));
      soc_reg_above_64_field_get(unit, IHP_MY_B_MAC_LSB_BITMAPr, reg_above64_val, MY_B_MAC_LSB_BITMAPf, fld_above64_val);

      tmp = &fld_above64_val[reg_offset];
      SOC_SAND_SET_BIT(*tmp, enable, bit_offset);

      soc_reg_above_64_field_set(unit, IHP_MY_B_MAC_LSB_BITMAPr, reg_above64_val, MY_B_MAC_LSB_BITMAPf, fld_above64_val);
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_IHP_MY_B_MAC_LSB_BITMAPr(unit, REG_PORT_ANY, reg_above64_val));
  }
  else { /* port is valid: set per-port LSB in egress (for encapsulation) */

      /* Configure entry in aux table only take the first 13 bits */      
      phys_port_id_fld.lsb = 0;
      phys_port_id_fld.msb = 12;
      phy_sys_port_ndx = (src_sys_port_ndx->sys_id & ARAD_FLD_MASK_OLD(phys_port_id_fld));
  
      /* no need to check if port is_pbp, this was already done in BCM */

      /* there are 4 8b' entries in each 32b' tbl-data */
      entry_index = phy_sys_port_ndx / 4;
      entry_offset = phy_sys_port_ndx % 4;

      res = arad_pp_egq_aux_table_tbl_get_unsafe(
          unit,
          entry_index,
          &aux_table_tbl_data
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      mac_lsb = my_mac_lsb->address[0];
      res = soc_sand_bitstream_set_any_field(
          &mac_lsb,
          entry_offset * ARAD_PP_MY_BMAC_MSB_LSB,
          ARAD_PP_MY_BMAC_MSB_LSB,
          &aux_table_tbl_data.aux_table_data
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      res = arad_pp_egq_aux_table_tbl_set_unsafe(
          unit,
          entry_index,
          &aux_table_tbl_data
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_port_lsb_set_unsafe()", 0, 0);
}

uint32
  arad_pp_lif_my_bmac_port_lsb_set_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                           *src_sys_port_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb,
    SOC_SAND_IN  uint8                                     enable
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_PORT_LSB_SET_VERIFY);

  res = arad_pp_lif_my_bmac_port_lsb_get_verify(
    unit,
    src_sys_port_ndx
    );
  SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_port_lsb_set_verify()", 0, 0);
}

uint32
  arad_pp_lif_my_bmac_port_lsb_get_verify(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                           *src_sys_port_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_VERIFY);

  if (src_sys_port_ndx->sys_port_type == SOC_SAND_PP_SYS_PORT_TYPE_LAG)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_MYMAC_SYS_PORT_NON_SINGLE_PORT_NOT_SUPPORTED_ERR, 10, exit);
  }
  else if (src_sys_port_ndx->sys_port_type == SOC_SAND_PP_SYS_PORT_TYPE_SINGLE_PORT)
  {
      SOC_SAND_ERR_IF_ABOVE_MAX(src_sys_port_ndx->sys_id, ARAD_PP_SYS_PORT_MAX, ARAD_PP_MYMAC_SYS_PORT_OUT_OF_RANGE, 20, exit);
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_port_lsb_get_verify()", 0, 0);
}

/*********************************************************************
*     Set the My-B-MAC LSB according to the source system
 *     port.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_lif_my_bmac_port_lsb_get_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                           *src_sys_port_ndx,
    SOC_SAND_INOUT SOC_SAND_PP_MAC_ADDRESS                           *my_mac_lsb,
    SOC_SAND_OUT uint8                                     *enable
  )
{
  uint32
    res = SOC_SAND_OK,
    phy_sys_port_ndx,
    entry_offset,
    entry_index,
    reg_offset,
    bit_offset,
    field_val,
    mac_lsb,
    i;
  ARAD_PP_EGQ_AUX_TABLE_TBL_DATA
    aux_table_tbl_data;
  soc_reg_above_64_val_t
    fld_above64_val,
    reg_above64_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_LIF_MY_BMAC_PORT_LSB_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(src_sys_port_ndx);
  SOC_SAND_CHECK_NULL_INPUT(my_mac_lsb);
  SOC_SAND_CHECK_NULL_INPUT(enable);

  /* verify aux data table mode is MiM (PBP) */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf, &field_val));
  if (field_val != 0x2)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_LIF_MY_BMAC_PORT_LSB_AUX_TABLE_MODE_MISMATCH_ERR, 7, exit);
  }

  if (src_sys_port_ndx->sys_port_type == SOC_SAND_PP_NOF_SYS_PORT_TYPES) { /* port is invalid - get global LSB */

      SOC_REG_ABOVE_64_CLEAR(fld_above64_val);
      SOC_REG_ABOVE_64_CLEAR(reg_above64_val);

      /* Get enable bit for this lsb */
      reg_offset = my_mac_lsb->address[0] / 32;
      bit_offset = my_mac_lsb->address[0] % 32;

      SOC_SAND_SOC_IF_ERROR_RETURN(res, 1110, exit, READ_IHP_MY_B_MAC_LSB_BITMAPr(unit, REG_PORT_ANY, reg_above64_val));
      soc_reg_above_64_field_get(unit, IHP_MY_B_MAC_LSB_BITMAPr, reg_above64_val, MY_B_MAC_LSB_BITMAPf, fld_above64_val);
      
      field_val = fld_above64_val[reg_offset];
      *enable = (SOC_SAND_GET_BIT(field_val, bit_offset)) ? TRUE : FALSE;
  }
  else { /* port is valid - get per-port LSB */

      /* Get entry from aux table */
      phy_sys_port_ndx = ARAD_PP_SAND_SYS_PORT_ENCODE(src_sys_port_ndx);

      /* no need to check if port is_pbp, this was already done in BCM */

      /* there are 4 8b' entries in each 32b' tbl-data */
      entry_index = phy_sys_port_ndx / 4;
      entry_offset = phy_sys_port_ndx % 4;

      res = arad_pp_egq_aux_table_tbl_get_unsafe(
          unit,
          entry_index,
          &aux_table_tbl_data
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

      res = soc_sand_bitstream_get_any_field(
          &aux_table_tbl_data.aux_table_data,
          entry_offset * ARAD_PP_MY_BMAC_MSB_LSB,
          ARAD_PP_MY_BMAC_MSB_LSB,
          &mac_lsb
        );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      /* init mac address, except for LSB */
      for (i = 1; i < SOC_SAND_PP_MAC_ADDRESS_NOF_U8; i++) {
          my_mac_lsb->address[i] = 0;
      }
      my_mac_lsb->address[0] = mac_lsb;
      *enable = TRUE;
  }
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_lif_my_bmac_port_lsb_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_mymac module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_mymac_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_mymac;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_mymac module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_mymac_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_mymac;
}

uint32
  SOC_PPC_MYMAC_VRRP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MYMAC_VRRP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->mode, ARAD_PP_MYMAC_MODE_MAX, ARAD_PP_MYMAC_MODE_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_MYMAC_VRRP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_MYMAC_TRILL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_MYMAC_TRILL_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);
 
  SOC_SAND_ERR_IF_ABOVE_MAX(info->my_nick_name, ARAD_PP_MYMAC_MY_NICK_NAME_MAX, ARAD_PP_MYMAC_MY_NICK_NAME_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_MYMAC_TRILL_INFO_verify()",0,0);
}

    

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

