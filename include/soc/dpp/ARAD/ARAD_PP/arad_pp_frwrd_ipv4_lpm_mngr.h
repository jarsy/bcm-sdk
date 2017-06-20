
/* $Id: arad_pp_frwrd_ipv4_lpm_mngr.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_PP_IPV4_LPM_INCLUDED__
/* { */
#define __ARAD_PP_IPV4_LPM_INCLUDED__

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>
#include <soc/dpp/SAND/Utils/sand_u64.h>
#include <soc/dpp/SAND/Utils/sand_group_member_list.h>
#include <soc/dpp/SAND/Utils/sand_pat_tree.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_array_memory_allocator.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>

/* } */
/*************
* DEFINES   *
*************/
/* { */
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
typedef SOC_SAND_U64 ARAD_PP_IPV4_LPM_MNGR_INST;

/*
 * LPM model
 */
typedef enum {
  ARAD_PP_IPV4_LPM_PXX_MODEL_UNKNOWN = 0,
  ARAD_PP_IPV4_LPM_PXX_MODEL_P4X = 1, /* Timna with  elide enc. only */
  ARAD_PP_IPV4_LPM_PXX_MODEL_P6X = 2, /* Petra A */ 
  ARAD_PP_IPV4_LPM_PXX_MODEL_P6N = 3, /* Petra B */
  ARAD_PP_IPV4_LPM_PXX_MODEL_P4X_COMPLETE = 4, /* Timna */
  ARAD_PP_IPV4_LPM_PXX_MODEL_P6M = 5, /* for Arad */
  ARAD_PP_IPV4_LPM_PXX_MODEL_LP4 = 6  /* PCP (Ramon) */
} ARAD_PP_IPV4_LPM_PXX_MODEL;


/*********************************************************************
* NAME:
*     ARAD_PP_IPV4_LPM_MNGR_SW_DB_TREE_NODE_INFO_SET
* FUNCTION:
*  call back to get the memory allocator manager for given memory
* INPUT:
*  SOC_SAND_IN  uint32              memory_ndx -
*   memory management ID.
*  SOC_SAND_OUT ARAD_PP_ARR_MEM_ALLOCATOR_INFO   **mem_allocator -
*   memory allocator manager
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*ARAD_PP_IPV4_LPM_MNGR_SW_DB_MEM_ALLOCATOR_GET)(
      SOC_SAND_IN  int                      prime_handle,
      SOC_SAND_IN  uint32                      sec_handle,
      SOC_SAND_IN  uint32                      memory_ndx,
      SOC_SAND_OUT ARAD_PP_ARR_MEM_ALLOCATOR_INFO   **mem_allocator
    );


/*********************************************************************
* NAME:
*     ARAD_PP_IPV4_LPM_MNGR_SW_DB_TREE_NODE_INFO_SET
* FUNCTION:
*  call back to get the LPM DB (PAT tree) for given vrf_id
* INPUT:
*  SOC_SAND_IN  uint32              vrf_ndx -
*   VRF index.
*  SOC_SAND_OUT SOC_SAND_PAT_TREE_INFO            **pat_tree -
*   PAT tree
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*ARAD_PP_IPV4_LPM_MNGR_SW_DB_PAT_TREE_GET)(
      SOC_SAND_IN  int                      prime_handle,
      SOC_SAND_IN  uint32                      sec_handle,
      SOC_SAND_IN  uint32                      vrf_ndx,
      SOC_SAND_OUT SOC_SAND_PAT_TREE_INFO            **pat_tree
    );

typedef
  int
    (*ARAD_PP_IPV4_LPM_MNGR_SW_DB_NOF_ENTRIES_FOR_HW_LPM_SET_FUN)(
      SOC_SAND_IN int                     unit,
      SOC_SAND_IN uint32                      nof_lpm_entries_in_lpm
   );

typedef
  int
    (*ARAD_PP_IPV4_LPM_MNGR_SW_DB_NOF_ENTRIES_FOR_HW_LPM_GET_FUN)(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_OUT  uint32                    *nof_lpm_entries_in_lpm
    );

typedef struct {
  SOC_SAND_MAGIC_NUM_VAR

  uint32 *bit_depth_per_bank;


} ARAD_PP_IPV4_LPM_MNGR_T;

#define ARAD_PP_LPV4_LPM_SUPPORT_CACHE  (1)
#define ARAD_PP_LPV4_LPM_SUPPORT_DEFRAG  (2)
#define ARAD_PP_LPV4_LPM_SHARED_MEM      (4)



typedef enum {
  ARAD_PP_IPV4_LPM_MEMORY_1,
  ARAD_PP_IPV4_LPM_MEMORY_2,
  ARAD_PP_IPV4_LPM_MEMORY_3,
  ARAD_PP_IPV4_LPM_MEMORY_4,
  ARAD_PP_IPV4_LPM_MEMORY_5,
  ARAD_PP_IPV4_LPM_MEMORY_6,

  ARAD_PP_IPV4_LPM_NOF_MEMS

}ARAD_PP_IPV4_LPM_MEMORY;

/*
 * includes the information user has to supply in the LPM manager creation
 */
typedef struct {
  SOC_SAND_MAGIC_NUM_VAR

 /* primary handle of the LPM manager to identify the LPM manager instance */
  int prime_handle;
  /* secondary handle of the LPM manager to identify the LPM manager instance */
  uint32 sec_handle;
  /* number of bits for use of VRF */
  uint32 nof_vrf_bits;
  /* number of banks */
  uint32 nof_banks;
  /* number of bits considered by each bank: Length nof_banks */
  uint32 *nof_bits_per_bank;
  /* mapping from bank to memories. Length  = nof_banks */
  uint32 *bank_to_mem;
  uint32 *mem_to_bank;
  uint32 flags;
  /* number of memories */
  uint32 nof_mems;
  /* size of each memory */
  uint32 *nof_rows_per_mem;
  /* p6x, p4x, p6n, etc. */
  ARAD_PP_IPV4_LPM_PXX_MODEL pxx_model;
  /* memory managements length nof_memories */
  ARAD_PP_ARR_MEM_ALLOCATOR_INFO *mem_allocators;

  /* backward pointers, for defragement */
  SOC_SAND_GROUP_MEM_LL_INFO **rev_ptrs;

  /* Pat Tree: could be of number of VRFs
   *  pat_tree_get_fun maps from VRF to PAT tree instance
   */
  SOC_SAND_PAT_TREE_INFO *lpms;
  uint32 nof_lpms;
  /*  mem_alloc_get_fun maps memory to memory manager */
  ARAD_PP_IPV4_LPM_MNGR_SW_DB_MEM_ALLOCATOR_GET mem_alloc_get_fun;
/*  pat_tree_get_fun maps from VRF to PAT tree instance */
  ARAD_PP_IPV4_LPM_MNGR_SW_DB_PAT_TREE_GET pat_tree_get_fun;

  ARAD_PP_IPV4_LPM_MNGR_SW_DB_NOF_ENTRIES_FOR_HW_LPM_SET_FUN
    nof_entries_for_hw_lpm_set_fun;
  ARAD_PP_IPV4_LPM_MNGR_SW_DB_NOF_ENTRIES_FOR_HW_LPM_GET_FUN
    nof_entries_for_hw_lpm_get_fun;
  
  uint32
    max_nof_entries_for_hw_lpm;
} ARAD_PP_IPV4_LPM_MNGR_INIT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  ARAD_PP_IPV4_LPM_MNGR_INIT_INFO
  init_info;

  ARAD_PP_IPV4_LPM_MNGR_T
    data_info;
} ARAD_PP_IPV4_LPM_MNGR_INFO;



typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
  uint32  total_nodes;
  uint32  prefix_nodes;
  uint32  free_nodes;
} ARAD_PP_IPV4_LPM_MNGR_STATUS;

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_create
* FUNCTION:
*  Create LPM manager instance.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   includes the information how to create/use the lpm_mngr
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_create(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  int                   unit
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_vrf_init
* FUNCTION:
*  Init specific VRF with default forwarding.
*  should be called in the initialization of the routing table
*  can be used also later to clear the VRF routing information.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF index to init.
*  SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec -
*   default FEC to init the VRF with.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_vrf_init(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                    vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_vrf_clear
* FUNCTION:
*  clear specific VRF with default forwarding.
*  should be called when clear routing table
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF index to init.
*  SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec -
*   default FEC to init the VRF with.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_vrf_clear(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                    vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_ROUTE_STATUS   pending_type
  );

/* remove all routes in VRF by traverse the LPM entries and call remove */
uint32
  arad_pp_ipv4_lpm_mngr_vrf_routes_clear(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                    vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_sys_fec,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_ROUTE_STATUS   pending_type
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_all_vrfs_clear
* FUNCTION:
*  clear all VRFs with default forwarding.
*  should be called when clear routing table
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    nof_vrfs -
*   number of VRFs.
*  SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec -
*   default FEC to init the VRF with.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_all_vrfs_clear(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                    nof_vrfs,
    SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID       default_fec
  );
/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_clear
* FUNCTION:
*  Clear all information of LPM manager including PAT trees info and Memory allocators.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_ipv4_lpm_mngr_clear(
  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr
);

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_destroy
* FUNCTION:
*  Destroy LPM manager instance and free all used memories.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   includes the information how to create/use the lpm_mngr
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_destroy(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr
  );

/*********************************************************************
* NAME:
 *   arad_pp_ipv4_lpm_mngr_hw_sync
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Synchronize the routing table in the Data plane (HW)
 *   with the routing table in the control plane (SW)
 *   This API effects entries with target LPM only.
 * INPUT:
 *  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
 *   LPM manager to manipulate
 **   SOC_SAND_IN  uint32                                      vrf_ndx -
 *     Virtual router id. Range: 0 - 255.use VRF 0 for default
 *     routing table. SOC_SAND_IN SOC_PPC_FRWRD_IPV4_VRF_INFO *vrf_info
 *     - VRF information.
 * REMARKS:
 *   - while updating (Add/remove) the UC/MC/VPN IP routing
 *   table, user can choose whether to update both the HW and
 *   SW, or SW only. When user select to update the SW only,
 *   this API comes to synchronize the HW with the changes
 *   made in the SW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_hw_sync(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                                      vrf_ndx
  );

/*********************************************************************
* NAME:
*   arad_pp_ipv4_lpm_mngr_mark_non_lpm_route_pending_type_set
* TYPE:
*   PROC
* FUNCTION:
*   Set pending type of non-LPM route (route that is just stored in
*    LPM mngr as a cache of other memories, e.g. LEM)
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key.
* REMARKS:
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_mark_non_lpm_route_pending_type_set(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                       vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET            *key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_ROUTE_STATUS   pending_type
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_prefix_add
* FUNCTION:
*  Add route to VRF.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key.
*  SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID         sys_fec_id -
*   sys-FEC-id to bind with the given key.
*  SOC_SAND_IN  uint8                    is_pending_op -
*   whether the operation is pending (entry is added
*   to the SW, and later is added to the HW upon
*   synchronize operation) or the entry should be
*   immediately written to the HW.
*   If is_write_to_hw is false, pending_op is "don't care".
*  SOC_SAND_IN  uint8                    is_write_to_hw,
*   whether an entry should be written to the HW, or
*   just stored in the SW
*  SOC_SAND_OUT  uint8                    *success -
*   whether the add operation succeeded or failed.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_prefix_add(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO    *lpm_mngr,
    SOC_SAND_IN  uint32                      vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key,
    SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID         sys_fec_id,
    SOC_SAND_IN  uint8                    is_pending_op,
    SOC_SAND_IN SOC_PPC_FRWRD_IP_ROUTE_LOCATION hw_target_type,
    SOC_SAND_OUT  uint8                    *success
  );
    
/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_sync
* FUNCTION:
*  Sync hardware with VRF SW shadow
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key.
*  SOC_SAND_IN  SOC_SAND_PP_SYSTEM_FEC_ID         sys_fec_id -
*   sys-FEC-id to bind with the given key.
*  SOC_SAND_IN  uint8                    is_pending_op -
*   whether the operation is pending (entry is added
*   to the SW, and later is added to the HW upon
*   synchronize operation) or the entry should be
*   immediately written to the HW.
*   If is_write_to_hw is false, pending_op is "don't care".
*  SOC_SAND_IN  uint8                    is_write_to_hw,
*   whether an entry should be written to the HW, or
*   just stored in the SW
*  SOC_SAND_OUT  uint8                    *success -
*   whether the add operation succeeded or failed.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_sync(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO    *lpm_mngr,
    SOC_SAND_IN  uint32                         vrf_ndx,
    SOC_SAND_IN  uint32                         *vrf_bitmap,
    SOC_SAND_IN  uint32                         nof_vrfs,
    SOC_SAND_IN  uint8                         update_lpm,
    SOC_SAND_OUT  SOC_SAND_SUCCESS_FAILURE            *success
  );

uint32
  arad_pp_ipv4_lpm_mngr_cache_set(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO    *lpm_mngr,
    SOC_SAND_IN  uint32                         vrf_ndx,
    SOC_SAND_IN  uint32                          nof_vrfs,
    SOC_SAND_IN  uint8                         enable
  );
/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_prefix_remove
* FUNCTION:
*  Remove route from VRF.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key.
*  SOC_SAND_IN  uint8                    is_pending_op -
*   whether the operation is pending (entry is added
*   to the SW, and later is added to the HW upon
*   synchronize operation) or the entry should be
*   immediately written to the HW.
*   If is_write_to_hw is false, pending_op is "don't care".
*  SOC_SAND_OUT  uint8                    *success -
*   whether the remove operation succeeded or failed.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_prefix_remove(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                     vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET          *key,
    SOC_SAND_IN  uint8                    is_pending_op,
    SOC_SAND_OUT  uint8                   *success
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_prefix_lookup
* FUNCTION:
*  return an LPM key for a given route key.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key to lookup.
*  SOC_SAND_OUT  SOC_SAND_PP_IPV4_SUBNET         *found_key -
*   the LPM route key for the given key.
*  SOC_SAND_OUT  uint8                   *exact_match -
*   was it an exact match.
*  SOC_SAND_OUT  uint8                   *found -
*   whether the key was found in the correspond LPM DB.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_prefix_lookup(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                     vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET          *key,
    SOC_SAND_OUT  SOC_SAND_PP_IPV4_SUBNET         *found_key,
    SOC_SAND_OUT  uint8                   *exact_match,
    SOC_SAND_OUT  uint8                   *found
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_prefix_is_exist
* FUNCTION:
*  check if a key is exist in the VRF LPM DB.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key to lookup.
*  SOC_SAND_OUT  uint8                   *found -
*   whether the exact key was found in the correspond LPM DB.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_prefix_is_exist(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                     vrf_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET          *key,
    SOC_SAND_OUT  uint8                   *exist
  );

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_sys_fec_get
* FUNCTION:
*  return the sys-fec attached with the given route or LPM
*  of that key.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key to lookup.
*  SOC_SAND_OUT  uint8                   *exact_match -
*   to lookup for the exact key or LPM key.
*  SOC_SAND_OUT  SOC_SAND_PP_SYSTEM_FEC_ID        *sys_fec_id -
*   sys-fec-id binded with the given key.
*  SOC_SAND_OUT uint8                     *is_write_to_hw -
*   flag indicates whether the entry is HW LPM entry, or is just stored
*   in the SW with no intention to be written to the HW LPM
*  SOC_SAND_OUT  uint8                   *found -
*   whether the key was found in the correspond LPM DB.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32 arad_pp_ipv4_lpm_mngr_sys_fec_get(
  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
  SOC_SAND_IN  uint32                     vrf_ndx,
  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key,
  SOC_SAND_IN  uint8                     exact_match,
  SOC_SAND_OUT  SOC_SAND_PP_SYSTEM_FEC_ID        *sys_fec_id,
  SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS *pending_type,
  SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION *hw_target_type,
  SOC_SAND_OUT  uint8                    *found
);

/*********************************************************************
* NAME:
*     arad_pp_ipv4_lpm_mngr_get_stat
* FUNCTION:
*  return the status of LPM manager.
*  of that key.
* INPUT:
*  SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr -
*   LPM manager to manipulate
*  SOC_SAND_IN  uint32                    vrf_ndx -
*   VRF id.
*  SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET           *key -
*   routing key to lookup.
*  SOC_SAND_OUT  uint8                   *exact_match -
*   to lookup for the exact key or LPM key.
*  SOC_SAND_OUT  SOC_SAND_PP_SYSTEM_FEC_ID        *sys_fec_id -
*   sys-fec-id binded with the given key..
*  SOC_SAND_OUT  uint8                   *found -
*   whether the key was found in the correspond LPM DB.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_ipv4_lpm_mngr_get_stat(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO   *lpm_mngr,
    SOC_SAND_IN  uint32                      vrf_ndx,
    SOC_SAND_OUT  ARAD_PP_IPV4_LPM_MNGR_STATUS  *lpm_stat
  );

uint32
  arad_pp_ipv4_lpm_mngr_lookup0(
    SOC_SAND_IN ARAD_PP_IPV4_LPM_MNGR_INFO    *lpm_mngr,
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32         vrf_ndx,
    SOC_SAND_IN  uint32         addr,
    SOC_SAND_OUT uint32        *lookup
  );

uint32
  arad_pp_ipv4_lpm_mngr_get_block(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *lpm_mngr,
    SOC_SAND_IN  uint32                     vrf_ndx,
    SOC_SAND_IN SOC_SAND_PAT_TREE_ITER_TYPE       iter_type,
    SOC_SAND_INOUT  SOC_SAND_U64                  *iter,
    SOC_SAND_IN  uint32                     entries_to_scan,
    SOC_SAND_IN  uint32                     entries_to_get,
    SOC_SAND_OUT  SOC_SAND_PP_IPV4_SUBNET         *route_table,
    SOC_SAND_OUT  uint32                    *sys_fec_ids,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS *pending_type,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION *hw_target_type,
    SOC_SAND_OUT uint32                     *nof_entries
  );
 
void
  arad_pp_ipv4_lpm_entry_decode(
    SOC_SAND_IN ARAD_PP_IPV4_LPM_MNGR_INFO      *lpm_mngr,
    SOC_SAND_IN  uint32                       depth,
    SOC_SAND_IN  ARAD_PP_ARR_MEM_ALLOCATOR_ENTRY   *data,
    SOC_SAND_IN  uint32                       data_len,
    SOC_SAND_OUT  uint32                       *raw
  ) ;

uint32
  arad_pp_ipv4_lpm_mngr_row_to_base_addr(
    SOC_SAND_IN  ARAD_PP_IPV4_LPM_MNGR_INFO *lpm_mngr,
    SOC_SAND_IN ARAD_PP_IPV4_LPM_MNGR_INST *inst,
    SOC_SAND_IN uint32 depth /* depth of the instruction inst */,
    SOC_SAND_OUT int32 *result
  );

int32
  arad_pp_ipv4_lpm_mngr_update_base_addr(
    SOC_SAND_IN  ARAD_PP_IPV4_LPM_MNGR_INFO *lpm_mngr,
    SOC_SAND_INOUT ARAD_PP_IPV4_LPM_MNGR_INST *inst,
    SOC_SAND_IN uint32 depth,
    SOC_SAND_IN uint32 new_ptr
  );

void
  arad_pp_ARAD_PP_IPV4_LPM_MNGR_INFO_clear(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_INFO  *info
  );

void
  arad_pp_ARAD_PP_IPV4_LPM_MNGR_STATUS_clear(
    SOC_SAND_INOUT  ARAD_PP_IPV4_LPM_MNGR_STATUS  *info
  );

uint8
  arad_pp_ipv4_lpm_mngr_pat_tree_node_data_is_identical_fun(
  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info_0,
  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info_1
  );

uint8
  arad_pp_ipv4_lpm_mngr_pat_node_is_skip_in_lpm_identical_data_query_fun(
    SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info
  );

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#include <soc/dpp/SAND/Utils/sand_footer.h>
/* } __ARAD_PP_IPV4_LPM_INCLUDED__*/
#endif

