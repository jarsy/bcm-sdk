
/* $Id: arad_pp_kbp_entry_mgmt.h
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ARAD/ARAD_PP/include/arad_pp_kbp_entry_mgmt.h
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

#ifndef __ARAD_PP_KBP_ENTRY_MGMT_INCLUDED__
/* { */
#define __ARAD_PP_KBP_ENTRY_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_hashtable.h>

#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv6.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
/*#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>*/

#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_tcam.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <shared/swstate/sw_state.h>
#include <shared/swstate/sw_state_workarounds.h>
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES (40) /* For 320b key size */
#define ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_LONGS (ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES / sizeof(uint32)) /* For 320b key size */

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
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* 
 * Conversion functions from application-dependent 
 * structure (already transformed in a LEM access key) 
 * to the regular TCAM entry handling functions: 
 * add, remove, get and get-block 
 *  
 * In ARAD_PP_LEM_ACCESS_KEY, only nof_params and param[] 
 * are used. 
 */
typedef struct
{
  uint32 lsb_nof_bits[ARAD_PP_LEM_KEY_MAX_NOF_PARAMS][2]; /* 0 - lsb, 1 - nof-bits */
  uint32 logical_entry_size_in_bytes; /* The size of data to be searched in entry (without padding) */
} ARAD_PP_TCAM_KBP_TABLE_ATTRIBUTES;

/* 
 * Key structure for ELK in the IP TCAM management 
 * common code for add/get/get_block/remove/table_clear code 
 */
typedef struct
{
  uint8 m_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES];
  uint8 m_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES];
  uint32 priority;
} ARAD_PP_FRWRD_IP_ELK_FWD_KEY;

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
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

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* 
 * KBP access level functions: 
 * - once the buffer of the content and the actions is obtained, 
 * insertion / removal / get / get-block at low level 
 */


/* Encode the LEM key under 32 bits */
#define ARAD_PP_FRWRD_IP_TCAM_LEM_KEY_ENCODE(unit, frwrd_table_id, param_ndx, fld_val, route_key)            \
        {                                                                                                         \
            res = arad_pp_tcam_kbp_lem_key_encode(unit, frwrd_table_id, param_ndx, fld_val, route_key); \
            SOC_SAND_CHECK_FUNC_RESULT(res, 1000 + param_ndx, exit);                                              \
        }
#define ARAD_PP_FRWRD_IP_TCAM_LEM_KEY_DECODE(unit, frwrd_table_id, param_ndx, fld_val, route_key_data, route_key_mask)             \
        SHR_BITCOPY_RANGE(fld_val, 0, route_key_data->param[param_ndx].value, 0, route_key_data->param[param_ndx].nof_bits);            \
        SHR_BITAND_RANGE(fld_val, route_key_mask->param[param_ndx].value, 0, route_key_mask->param[param_ndx].nof_bits, fld_val);       

uint32
  arad_pp_tcam_kbp_lem_key_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     param_ndx,
    SOC_SAND_IN  uint32                     fld_val, /* only for values with less than 32 bits */
    SOC_SAND_INOUT ARAD_PP_LEM_ACCESS_KEY     *route_key
  );

uint32
    arad_pp_tcam_kbp_table_clear(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  uint32   frwrd_table_id
    );

 uint32
    arad_pp_kbp_do_search(
       int     unit,
       uint32  ltr_idx,
       uint8   *master_key
	);

 uint32
  arad_pp_kbp_route_payload_decode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     lookup_id,
    SOC_SAND_IN  uint8                      *asso_data,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD *payload
  );

uint32
    arad_pp_tcam_kbp_lpm_route_get(
       SOC_SAND_IN  int         unit,
       SOC_SAND_IN  uint8       frwrd_table_id,
       SOC_SAND_IN  uint32      prefix_len,
       SOC_SAND_IN  uint8       *data,
       SOC_SAND_OUT uint8       *assoData,
       SOC_SAND_OUT uint8       *found
    );

uint32
    arad_pp_tcam_kbp_lpm_route_add(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx,
       SOC_SAND_IN  uint32                      prefix_len,
       SOC_SAND_IN  uint8                       *data,
       SOC_SAND_IN  uint8                       *assoData,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    );

uint32
    arad_pp_tcam_kbp_lpm_route_remove(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      prefix_len,
       SOC_SAND_IN  uint8                       *data
    );

uint32
    arad_pp_tcam_kbp_route_get(
       SOC_SAND_IN  int             unit,
       SOC_SAND_IN  uint8           frwrd_table_id,
       SOC_SAND_IN  uint32          data_indx,
       SOC_SAND_OUT uint8           *data,
       SOC_SAND_OUT uint8           *mask,
       SOC_SAND_OUT uint8           *assoData,
       SOC_SAND_OUT uint32          *priority,
       SOC_SAND_OUT uint8           *found,
       SOC_SAND_OUT int             *valid_bytes
    );

uint32
    arad_pp_tcam_kbp_route_add(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  uint32                      frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx,
       SOC_SAND_IN  uint8                       is_for_update,
       SOC_SAND_IN  uint32                      priority,
       SOC_SAND_IN  uint8                       *m_data,
       SOC_SAND_IN  uint8                       *m_mask,
       SOC_SAND_IN  uint8                       *assoData,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    );

uint32
    arad_pp_tcam_kbp_route_remove(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint8                       frwrd_table_id,
       SOC_SAND_IN  uint32                      data_indx
    );

uint32
  arad_pp_tcam_kbp_tcam_entry_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint32                     entry_id,
    SOC_SAND_OUT uint32                     data[ARAD_TCAM_ENTRY_MAX_LEN],
    SOC_SAND_OUT uint32                     mask[ARAD_TCAM_ENTRY_MAX_LEN],
    SOC_SAND_OUT uint32                     value[ARAD_TCAM_ACTION_MAX_LEN],
    SOC_SAND_OUT uint32                     *priority,    
    SOC_SAND_OUT uint8                      *found,
    SOC_SAND_OUT uint8                      *hit_bit
  );

uint32
  arad_pp_tcam_kbp_tcam_entry_add(
	int 								unit,
	ARAD_KBP_FRWRD_IP_TBL_ID 			frwrd_table_id,
	uint32 								entry_id_ndx,
	uint8 								is_for_update,
	uint32 								priority,
	uint32 								data[ARAD_TCAM_ENTRY_MAX_LEN],
	uint32  							mask[ARAD_TCAM_ENTRY_MAX_LEN],
	uint32 								value[ARAD_TCAM_ACTION_MAX_LEN],
	SOC_SAND_IN SOC_PPC_FP_ENTRY_INFO	*info,
	SOC_SAND_SUCCESS_FAILURE 			*success
  );

uint32
  arad_pp_tcam_route_buffer_to_kbp_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     logical_entry_size_in_bytes,
    SOC_SAND_IN  uint32                     *buffer_data,
    SOC_SAND_IN  uint32                     *buffer_mask,
    SOC_SAND_OUT uint8                      *data,
    SOC_SAND_OUT uint8                      *mask
  );

uint32
  arad_pp_kbp_route_payload_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT uint8                      *asso_data
  );

uint32
  arad_pp_tcam_route_kbp_add_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask,
    SOC_SAND_IN  uint32                     priority,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );

uint32
  arad_pp_tcam_route_kbp_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask,
    SOC_SAND_OUT uint32                     *priority,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD *payload,
    SOC_SAND_OUT uint8                      *found
  );

uint32
  arad_pp_tcam_route_kbp_remove_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_data,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *route_key_mask
  );

uint32
  arad_pp_tcam_route_kbp_payload_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     table_payload_in_bytes,
    SOC_SAND_IN  uint32                     *asso_data_buffer,
    SOC_SAND_OUT uint8                      *asso_data
  );

uint32
  arad_pp_kbp_route_kbp_payload_buffer_encode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     table_payload_in_bytes,
    SOC_SAND_IN  uint32                     *asso_data_buffer,
    SOC_SAND_OUT uint8                      *asso_data
  );

uint32
  arad_pp_tcam_route_kbp_payload_buffer_decode(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id,
    SOC_SAND_IN  uint8                      *asso_data,
    SOC_SAND_OUT uint32                     *asso_data_buffer
  );

uint32
  arad_pp_tcam_route_kbp_get_block_unsafe(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID                   frwrd_table_id,
    SOC_SAND_INOUT  SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY                    *route_key_data,
    SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_KEY                    *route_key_mask,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                 *payload,
    SOC_SAND_OUT uint32                                     *nof_entries
  );

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* } __ARAD_PP_KBP_ENTRY_MGMT_INCLUDED__*/

