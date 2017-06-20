/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_mgmt.h
 */

#ifndef __QAX_MGMT_INCLUDED__

#define __QAX_MGMT_INCLUDED__

/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 qax_mgmt_system_fap_id_set(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32  sys_fap_id
  );


/*********************************************************************
* Get the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32
  qax_mgmt_system_fap_id_get(
    SOC_SAND_IN  int       unit,
    SOC_SAND_OUT uint32    *sys_fap_id
  );

/*********************************************************************
* set all the bits controlling the revision fixes (chicken bits) in the device.
*********************************************************************/
uint32 qax_mgmt_revision_fixes(int unit);

#endif /*__QAX_MGMT_INCLUDED__*/

