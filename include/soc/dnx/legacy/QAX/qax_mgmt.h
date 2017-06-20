/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_mgmt.h
 */

#ifndef __JER2_QAX_MGMT_INCLUDED__

#define __JER2_QAX_MGMT_INCLUDED__

/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 jer2_qax_mgmt_system_fap_id_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  sys_fap_id
  );


/*********************************************************************
* Get the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32
  jer2_qax_mgmt_system_fap_id_get(
    DNX_SAND_IN  int       unit,
    DNX_SAND_OUT uint32    *sys_fap_id
  );

/*********************************************************************
* set all the bits controlling the revision fixes (chicken bits) in the device.
*********************************************************************/
uint32 jer2_qax_mgmt_revision_fixes(int unit);

#endif /*__JER2_QAX_MGMT_INCLUDED__*/

