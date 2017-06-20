/* $Id: ppd_api_frwrd_fec.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_fec.h
*
* MODULE PREFIX:  soc_ppd_frwrd
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

#ifndef __SOC_PPD_API_FRWRD_FEC_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_FEC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_frwrd_fec.h>

#include <soc/dpp/PPD/ppd_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_PPD_FRWRD_FEC_NOF_FECS_PER_BANK(_unit)                              (SOC_DPP_DEFS_GET((_unit), nof_fecs) / SOC_DPP_DEFS_GET(_unit, nof_fec_banks))

/* Given a FEC ID, calculate the bank it is in. */
#define SOC_PPD_FRWRD_FEC_BANK_ID(_unit, _fec_idx)                              ((_fec_idx) / SOC_PPD_FRWRD_FEC_NOF_FECS_PER_BANK(_unit))

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPD_FRWRD_FEC_GLBL_INFO_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_FEC_FIRST,
  SOC_PPD_FRWRD_FEC_GLBL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_FEC_GLBL_INFO_GET,
  SOC_PPD_FRWRD_FEC_GLBL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ENTRY_ADD,
  SOC_PPD_FRWRD_FEC_ENTRY_ADD_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_ADD,
  SOC_PPD_FRWRD_FEC_ECMP_ADD_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_UPDATE,
  SOC_PPD_FRWRD_FEC_ECMP_UPDATE_PRINT,
  SOC_PPD_FRWRD_FEC_ENTRY_USE_INFO_GET,
  SOC_PPD_FRWRD_FEC_ENTRY_USE_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ENTRY_GET,
  SOC_PPD_FRWRD_FEC_ENTRY_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_GET,
  SOC_PPD_FRWRD_FEC_ECMP_GET_PRINT,
  SOC_PPD_FRWRD_FEC_REMOVE,
  SOC_PPD_FRWRD_FEC_REMOVE_PRINT,
  SOC_PPD_FRWRD_FEC_PROTECTION_OAM_INSTANCE_STATUS_SET,
  SOC_PPD_FRWRD_FEC_PROTECTION_OAM_INSTANCE_STATUS_SET_PRINT,
  SOC_PPD_FRWRD_FEC_PROTECTION_OAM_INSTANCE_STATUS_GET,
  SOC_PPD_FRWRD_FEC_PROTECTION_OAM_INSTANCE_STATUS_GET_PRINT,
  SOC_PPD_FRWRD_FEC_PROTECTION_SYS_PORT_STATUS_SET,
  SOC_PPD_FRWRD_FEC_PROTECTION_SYS_PORT_STATUS_SET_PRINT,
  SOC_PPD_FRWRD_FEC_PROTECTION_SYS_PORT_STATUS_GET,
  SOC_PPD_FRWRD_FEC_PROTECTION_SYS_PORT_STATUS_GET_PRINT,
  SOC_PPD_FRWRD_FEC_GET_BLOCK,
  SOC_PPD_FRWRD_FEC_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_FEC_ENTRY_ACCESSED_INFO_SET,
  SOC_PPD_FRWRD_FEC_ENTRY_ACCESSED_INFO_SET_PRINT,
  SOC_PPD_FRWRD_FEC_ENTRY_ACCESSED_INFO_GET,
  SOC_PPD_FRWRD_FEC_ENTRY_ACCESSED_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_GLOBAL_INFO_SET,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_GLOBAL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_GLOBAL_INFO_GET,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_GLOBAL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_PORT_INFO_SET,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_PORT_INFO_SET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_PORT_INFO_GET,
  SOC_PPD_FRWRD_FEC_ECMP_HASHING_PORT_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FEC_ECMP_INFO_SET,
  SOC_PPD_FRWRD_FEC_ECMP_INFO_GET,
  SOC_PPD_FRWRD_FEC_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_FEC_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_FEC_PROCEDURE_DESC;

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

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the FEC table (including
 *   resources to use)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_GLBL_INFO                 *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_GLBL_INFO                 *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_glbl_info_set" API.
 *     Refer to "soc_ppd_frwrd_fec_glbl_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_GLBL_INFO                 *glbl_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add FEC entry. May include protection of type Facility
 *   or Path.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383. Has to be even
 *     for protected FECs in Soc_petra-B.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_TYPE              protect_type -
 *     Protection type may be None, Path, or Facility.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *working_fec -
 *     Working FEC entry.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *protect_fec -
 *     Optional, can be NULL. Protecting FEC entry.
 *     Relevant when there is protection, i.e., protection type is not
 *     SOC_PPC_FRWRD_FEC_PROTECT_TYPE_NONE.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_INFO              *protect_info -
 *     Relevant only for Path protected
 *     FEC, i.e., if the protection type is
 *     SOC_PPC_FRWRD_FEC_PROTECT_TYPE_PATH. Includes the
 *     OAM-instance that the above FECs are associated with and
 *     determines which FEC to use in order to forward the
 *     packets.
 *   SOC_SAND_OUT uint8                               *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the FEC DB (LEM).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_entry_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_TYPE              protect_type,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *working_fec,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *protect_fec,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_PROTECT_INFO              *protect_info,
    SOC_SAND_OUT uint8                               *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add ECMP to the FEC table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383. Must be even.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array -
 *     The FEC array (ECMP).
 *   SOC_SAND_IN  uint32                                nof_entries -
 *     Size of ECMP and number of valid entries in fec_array.
 *   SOC_SAND_OUT uint8                               *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the FEC DB (LEM).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array,
    SOC_SAND_IN  uint32                                nof_entries,
    SOC_SAND_OUT uint8                               *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_update
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Update the content of block of entries from the ECMP.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383. Must be even.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array -
 *     FEC array to update the selected range of the ECMP. The
 *     size of this array should be as the length of the array
 *     (fec_range.end - fec_rang.start + 1)
 *   SOC_SAND_IN  SOC_SAND_U32_RANGE                          *fec_range -
 *     Range includes start FEC and end FEC to update.
 *     fec_range.end has to be smaller than the updated ECMP
 *     size.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_update(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array,
    SOC_SAND_IN  SOC_SAND_U32_RANGE                          *fec_range
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   set ECMP attributres
 *   relevant: Arad only.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              ecmp_ndx -
 *     ECMP index. Arad Range: 0 - 32K.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_INFO            *ecmp_info -
 *     ECMP info, including, size, base-fec,...
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_info_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_FEC_ID ecmp_ndx,
    SOC_SAND_IN SOC_PPC_FRWRD_FEC_ECMP_INFO *ecmp_info
  ); 

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   get ECMP attributres
 *   relevant: Arad only.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              ecmp_ndx -
 *     ECMP index. Arad Range: 0 - 32K.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_INFO            *ecmp_info -
 *     ECMP info, including, size, base-fec,...
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_info_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_FEC_ID ecmp_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_INFO *ecmp_info
  ); 

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_entry_use_info_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the usage/allocation of the FEC entry pointed by
 *   fec_ndx (ECMP/FEC/protected FEC/none).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO            *fec_entry_info -
 *     The usage of the FEC entry. - For ECMP/protection
 *     returns pointer to first FEC in the ECMP/protection.-
 *     For ECMP returns the size of the ECMP
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_entry_use_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_USE_INFO            *fec_entry_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get FEC entry from the FEC table. May include
 *   protection.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383. Has to be even
 *     for Protected FECs in Soc_petra-B.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_PROTECT_TYPE              *protect_type -
 *     Protection type may be None, Path, or Facility.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *working_fec -
 *     Working FEC entry.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *protect_fec -
 *     Protecting FEC entry. Relevant when there is protection,
 *     i.e., protection type is not
 *     SOC_PPC_FRWRD_FEC_PROTECT_TYPE_NONE.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_PROTECT_INFO              *protect_info -
 *     Protection information. Relevant only for Path protected
 *     FEC, i.e., if the protection type is
 *     SOC_PPC_FRWRD_FEC_PROTECT_TYPE_PATH. Includes the
 *     OAM-instance that the above FECs are associated with and
 *     determines which FEC to use in order to forward the
 *     packets.
 * REMARKS:
 *   - fec_ndx should be allocated and not point to ECMP,
 *   Error will be returned if so.- In case of protection
 *   fec_ndx has to point to the first FEC in the
 *   protection.- In order to get the allocation type of the
 *   FEC, use soc_ppd_frwrd_fec_entry_alloc_info_get(fec_ndx) -
 *   to get ECMP use soc_ppd_frwrd_fec_ecmp_get().
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_PROTECT_TYPE              *protect_type,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *working_fec,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *protect_fec,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_PROTECT_INFO              *protect_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Update content of range of the ECMP.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383. Must be even
 *     and start of ECMP.
 *   SOC_SAND_IN  SOC_SAND_U32_RANGE                          *fec_range -
 *     Range includes start FEC and end FEC to return.
 *     fec_range.end has to be smaller than the ECMP size.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array -
 *     Block of FEC entries of the ECMP.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of entries in the returned fec_array
 * REMARKS:
 *   - fec_ndx has to point to fec of type
 *   SOC_PPC_FRWRD_FEC_ENTRY_USE_TYPE_ECMP. Error will be
 *   returned otherwise.- fec_ndx has to point to the first
 *   FEC in the ECMP.- In order to get the allocation type of
 *   the FEC, use soc_ppd_frwrd_fec_entry_alloc_info_get(fec_ndx)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx,
    SOC_SAND_IN  SOC_SAND_U32_RANGE                          *fec_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_INFO                *fec_array,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove FEC entry/entries associated with fec_ndx.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx -
 *     Index in the FEC table. Range: 0 - 16383.
 * REMARKS:
 *   - When removing ECMP, then the fec_ndx must point to the
 *   first FEC of this ECMP (error will be returned
 *   otherwise), and then all FEC entries of the ECMP will be
 *   removed. - When removing protected FEC (path/facility),
 *   then the fec_ndx must point to the first FEC (even
 *   index) (error will be returned otherwise), and then both
 *   FEC entries will be removed.- User cannot remove FEC
 *   that is part of the ECMP/protection without moving all
 *   the FEC entries in the ECMP/protection.- It is the user
 *   responsibility to remove all usages/pointers to the FEC
 *   entry before removing it.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_protection_oam_instance_status_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the status of the OAM instance. For all PATH
 *   protected FECs that point to this instance, the working
 *   FEC will be used if up is TRUE, and the protect FEC will
 *   be used otherwise.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                oam_instance_ndx -
 *     OAM instance ID. Range: Soc_petra-B: 0-4K-1, T20E:0-16K-1.
 *   SOC_SAND_IN  uint8                               up -
 *     OAM instance status.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_protection_oam_instance_status_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                oam_instance_ndx,
    SOC_SAND_IN  uint8                               up
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_protection_oam_instance_status_set" API.
 *     Refer to
 *     "soc_ppd_frwrd_fec_protection_oam_instance_status_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_protection_oam_instance_status_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                oam_instance_ndx,
    SOC_SAND_OUT uint8                               *up
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_protection_sys_port_status_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the status of the System Port (LAG or Physical
 *   port).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *sys_port_ndx -
 *     System Port (LAG or Physical port).
 *   SOC_SAND_IN  uint8                               up -
 *     System port status.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_protection_sys_port_status_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *sys_port_ndx,
    SOC_SAND_IN  uint8                               up
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_protection_sys_port_status_set" API.
 *     Refer to "soc_ppd_frwrd_fec_protection_sys_port_status_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_protection_sys_port_status_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_SYS_PORT_ID                     *sys_port_ndx,
    SOC_SAND_OUT uint8                               *up
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the FEC table (in the specified range) and get
 *   all the FEC entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE                *rule -
 *     Get only entries that match this rule.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Range for iteration
 *   SOC_SAND_OUT uint32                                *fec_array -
 *     Array to include FEC IDs.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries in fec_array.
 * REMARKS:
 *   - if rule is according to application type, then for
 *   ECMP and protection only the id of the first FEC of the
 *   ECMP/protection is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE                *rule,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range,
    SOC_SAND_OUT uint32                                *fec_array,
    SOC_SAND_OUT uint32                                *nof_entries
  );

#if SOC_PPC_DEBUG_IS_LVL1
/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Traverse the FEC table (in the specified range) and get
 *   all the FEC entries that match the given rule.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE                *rule -
 *     Get only entries that match this rule.
 *   SOC_SAND_IN SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Range for iteration
 *   SOC_SAND_IN uint32                                *fec_array -
 *     Array to include FEC IDs.
 *   SOC_SAND_IN uint32                                *nof_entries -
 *     Number of valid entries in fec_array.
 * REMARKS:
 *   - if rule is according to application type, then for
 *   ECMP and protection only the id of the first FEC of the
 *   ECMP/protection is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_print_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_MATCH_RULE                *rule,
    SOC_SAND_IN SOC_SAND_TABLE_BLOCK_RANGE                   *block_range,
    SOC_SAND_IN uint32                                 *fec_array,
    SOC_SAND_IN uint32                                 nof_entries
  );
#endif
/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_entry_accessed_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set action to do by the device when a packet accesses
 *   the FEC entry.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id_ndx -
 *     Index in the FEC table. Range: 0 - 16383.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO       *accessed_info -
 *     Actions to do when packet accesses the FEC.
 * REMARKS:
 *   - May be set only for FEC entries in the Range 0-63- Not
 *   supported in T20E.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_entry_accessed_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO       *accessed_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_entry_accessed_info_set" API.
 *     Refer to "soc_ppd_frwrd_fec_entry_accessed_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_entry_accessed_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id_ndx,
    SOC_SAND_IN  uint8                               clear_access_stat,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ENTRY_ACCESSED_INFO       *accessed_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_hashing_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ECMP hashing global attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO     *glbl_hash_info -
 *     ECMP Hashing global settings
 * REMARKS:
 *   The hashing function result is 16 bit value. The PP use
 *   the value to choose the ECMP FEC member, to which the
 *   packet is sent
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_hashing_global_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO     *glbl_hash_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_ecmp_hashing_global_info_set" API.
 *     Refer to "soc_ppd_frwrd_fec_ecmp_hashing_global_info_set"
 *     API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_hashing_global_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_HASH_GLOBAL_INFO     *glbl_hash_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fec_ecmp_hashing_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ECMP hashing per-port attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                port_ndx -
 *     Port ID.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO       *port_hash_info -
 *     ECMP Hashing per-port settings
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_hashing_port_info_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO       *port_hash_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fec_ecmp_hashing_port_info_set" API.
 *     Refer to "soc_ppd_frwrd_fec_ecmp_hashing_port_info_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fec_ecmp_hashing_port_info_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_FEC_ECMP_HASH_PORT_INFO       *port_hash_info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_FEC_INCLUDED__*/
#endif

