#ifndef __SB_ELIB_STATUS_H__
#define __SB_ELIB_STATUS_H__
/**
 * @file sbElibStatus.h Enumerated Return Codes
 *
 * <pre>
 * ====================================================
 * ==  sbElibStatus.h - Egress Library public API
 * ====================================================
 *
 * WORKING REVISION: $Id: sbElibStatus.h,v 1.6 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *
 * MODULE NAME:
 *
 *     sbElibStatus.h
 *
 * ABSTRACT:
 *
 *     Egress Library Error Enumeration
 *
 * </pre>
 */

#include <soc/sbx/sbStatus.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Enumerated Type of Return Codes
 *
 **/
typedef enum sbElibStatus_et_e {
  SB_ELIB_OK = 0,
  SB_ELIB_BAD_ARGS = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_QE_ELIB, 0x1),
  SB_ELIB_BAD_IDX,
  SB_ELIB_INIT_FAIL,
  SB_ELIB_IND_MEM_TIMEOUT,
  SB_ELIB_INIT_CIT_FAIL,
  SB_ELIB_INIT_CRT_FAIL,
  SB_ELIB_GENERAL_FAIL,
  SB_ELIB_PORT_ST_GET_FAIL,
  SB_ELIB_PORT_ST_SET_FAIL,
  SB_ELIB_VIT_SET_FAIL,
  SB_ELIB_VIT_GET_FAIL,
  SB_ELIB_VRT_SET_FAIL,
  SB_ELIB_VRT_GET_FAIL,
  SB_ELIB_MEM_ALLOC_FAIL,
  SB_ELIB_PORT_CFG_GET_FAIL,
  SB_ELIB_PORT_CFG_SET_FAIL,
  SB_ELIB_BIST_FAIL,
  SB_ELIB_SEM_GET_FAIL,
  SB_ELIB_SEM_GIVE_FAIL,
  SB_ELIB_DMA_FAIL,
  SB_ELIB_COUNT_FAIL,
  SB_ELIB_VLAN_MEM_ALLOC_FAIL,
  SB_ELIB_VLAN_MEM_FREE_FAIL,
  SB_ELIB_BAD_VID,
  /* leave as last */
  SB_ELIB_STATUS_LAST
} sbElibStatus_et;

char * sbGetElibStatusString(sbStatus_t status);
#ifdef __cplusplus
}
#endif

#endif /* __SB_ELIB_STATUS_H__ */
