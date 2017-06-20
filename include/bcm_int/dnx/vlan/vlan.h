/*! \file bcm_int/dnx/vlan/vlan.h
 * 
 * Internal DNX VLAN APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _VLAN_API_INCLUDED__
/* { */
#define _VLAN_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*!
 * \brief - Initialize VLAN resources. \n 
 *  The function will initalize and create all required VLAN res-manager.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID 
 * \par INDIRECT INPUT:
 *   * DNX-Data information such as nof VSIs , which features are enabled.
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * Initalize Res manager pools of VLAN module see the dnx_vlan_algo_res_init .c file which pools are initalized.
 * \remark
 *   * Assume Resrouce Manager is initalized \see dnx_algo_res_init
 * \see
 *   * None
 */
shr_error_e dnx_vlan_algo_res_init(
    int unit);
/*
 * MACROs
 * {
 */

/**
 * \brief  
 * Verify vlanId is within range (0-4K, not including 4K).
 */
#define BCM_DNX_VLAN_CHK_ID(unit, vid) { \
  if (vid > BCM_VLAN_MAX) SHR_ERR_EXIT(_SHR_E_PARAM, "Invalid VID\r\n"); \
  }
/*
 * }
 */

/* } */
#endif/*_VLAN_API_INCLUDED__*/
