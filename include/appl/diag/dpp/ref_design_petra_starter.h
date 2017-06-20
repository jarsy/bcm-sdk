/* $Id: ref_design_petra_starter.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __REF_DESIGN_PETRA_STARTER_INCLUDED__
/* { */
#define __REF_DESIGN_PETRA_STARTER_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

/*************
 * INCLUDES  *
 *************/
/* { */

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

uint32
  ref_design_petra_fap_id_from_slot_id_get(
    SOC_SAND_IN  uint32  slot_id,
    SOC_SAND_OUT uint32  *fap_id,
    SOC_SAND_IN  uint8  silent
  );

uint32
  ref_design_petra_app_init(
    SOC_SAND_IN  uint8    crate_type,
    SOC_SAND_IN  SOC_SAND_DEVICE_TYPE dev_type,
    SOC_SAND_IN  uint32  nof_fabric_cards,
    SOC_SAND_IN  uint8  is_hot_start,
    SOC_SAND_OUT uint32  *p_unit,
    SOC_SAND_IN  uint8  silent
  );

uint32
  ref_design_petra_device_info_get(
    SOC_SAND_OUT  uint32           *fap_id,
    SOC_SAND_OUT  uint32            *device_base_address
  );

/* } */

#ifdef  __cplusplus
}
#endif

/* } __REF_DESIGN_PETRA_STARTER_INCLUDED__*/
#endif
