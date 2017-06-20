/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_mgmt.h
 */

#ifndef __JER2_QAX_INGRESS_PACKET_QUEUING_INCLUDED__

#define __JER2_QAX_INGRESS_PACKET_QUEUING_INCLUDED__

#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_api_ingress_packet_queuing.h>
int
  jer2_qax_iqm_dynamic_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_DYNAMIC_TBL_DATA* IQM_dynamic_tbl_data
  );

int
jer2_qax_ipq_explicit_mapping_mode_info_get(
   DNX_SAND_IN  int                            unit,
   DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   );

int
jer2_qax_ipq_explicit_mapping_mode_info_set(
   DNX_SAND_IN  int                            unit,
   DNX_SAND_IN JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   );

int
jer2_qax_ipq_default_invalid_queue_set(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_IN  uint32         queue_id,
   DNX_SAND_IN  int            enable);

int
jer2_qax_ipq_default_invalid_queue_get(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_OUT uint32         *queue_id,
   DNX_SAND_OUT int            *enable);
#endif /*__JER2_QAX_INGRESS_PACKET_QUEUING_INCLUDED__ */

