/* $Id: ppc_api_eg_filter.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_filter.h
*
* MODULE PREFIX:  soc_ppc_eg
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

#ifndef __SOC_PPC_API_EG_FILTER_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

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

typedef enum
{
  /*
   *  Port which is allowed to send and receive frames from
   *  any other port on the VLAN. Usually connects to a router
   */
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_PROMISCUOUS = 0,
  /*
   *  Port is only allowed to communicate with
   *  Promiscuous-ports - they are "stub". This type of ports
   *  usually connects to hosts
   */
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_ISOLATED = 1,
  /*
   *  Community ports are allowed to talk to other ports in
   *  same community, sharing the same group and to
   *  Promiscuous-ports.
   */
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_COMMUNITY = 2,
  /*
   *  Number of types in SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE
   */
  SOC_PPC_NOF_EG_FILTER_PVLAN_PORT_TYPES = 3
}SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE;

typedef enum
{
  /*
   *  Disable egress filtering.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_NONE = 0x0,
  /*
   *  Enable filtering according to VSI membership. Filter
   *  packets designated to port that is not member in the VSI
   *  Soc_petra-B: if VSI updated upon egress encapsulation
   *  and the out-port is not member in this VSI then the packet
   *  will be dropped, (regardless egress action profile)
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_VSI_MEMBERSHIP = 0x1,
  /*
   *  Filter packets incoming interface equal to outgoing
   *  interface (Hair-Pin). In order to perform this filtering
   *  over packet also the in-port has to enable this
   *  filtering. See soc_ppd_port_info_set().
   *  in trap-mgmt referred by SOC_PPC_TRAP_EG_TYPE_HAIR_PIN
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_SAME_INTERFACE = 0x2,
  /*
   *  Filter UC packets with unknown DA.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_UC_UNKNOW_DA = 0x4,
  /*
   *  Filter MC packets with unknown DA.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_MC_UNKNOW_DA = 0x8,
  /*
   *  Filter BC packets with unknown DA.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_BC_UNKNOW_DA = 0x10,
  /*
   *  Perform MTU check, and filter packets their size exceed
   *  the MTU limit. T20E only.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_MTU = 0x20,
  /*
   *  Filter packets with STP state discard.
   *  if port state is block/discard then packet will be discarded
   *  regardless the setting of the egress-action-profile
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_STP = 0x40,
  /*
   *  Perform PEP acceptable frame types filtering and filter
   *  unaccepted frames. T20E only.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_PEP_ACCEPTABLE_FRM_TYPES = 0x80,
  /*
   *  Perform Split Horizon Filtering and Filter packets from
   *  Horizon to Horizon. T20E only.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_SPLIT_HORIZON = 0x100,
  /*
   *  Enable all filters in the egress.
   */
  SOC_PPC_EG_FILTER_PORT_ENABLE_ALL = (int)0xffffffff,
  /*
   *  Number of types in SOC_PPC_EG_FILTER_PORT_ENABLE
   */
  SOC_PPC_NOF_EG_FILTER_PORT_ENABLES = 11
}SOC_PPC_EG_FILTER_PORT_ENABLE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mask to enable/disable filtering. See
   *  SOC_PPC_EG_FILTER_PORT_ENABLE. For example to set enable
   *  filtering for VSI membership and STP, set filter_mask =
   *  SOC_PPC_EG_FILTER_PORT_ENABLE_VSI_MEMBERSHIP |
   *  SOC_PPC_EG_FILTER_PORT_ENABLE_STP;
   */
  uint32 filter_mask;
  /*
   *  Profile of out-port to be used in the setting of
   *  acceptable frames types. Used by the API
   *  soc_ppd_eg_filter_port_acceptable_frames_set.
   *  Soc_petra-B/ARAD only. Range 0-3
   */
  uint32 acceptable_frames_profile;

} SOC_PPC_EG_FILTER_PORT_INFO;

typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
  
  /*
   *  In-LIF-profile bitmap to disable same interface filter.
   *  Field is In-LIF_profile bitmap. Valid values are 0-0xF (since only the 2 lower bits are considered).
   *  Each bit in the bitmap represents the value of the two LSBs of an inlif profile.
   *  Bit 0 represents a value of 0 for the 2 LSBs of the inlif profile, Bit 1 a value of 1 and so on.
   *  In case the value of the 2 LSBs of In-LIF-profile x is set in the bitmap,
   *  the same interface filter check is disabled for this inlif profile.
   *  Example:
   *  The inlif profile (in binary) 1101 has same i/f filter disabled if bit 1 (=0b01) is set (=1).
   *  Valid for ARAD plus and above.
   */
  uint32 in_lif_profile_disable_same_interface_filter_bitmap;
} SOC_PPC_EG_FILTER_GLOBAL_INFO;

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

void
  SOC_PPC_EG_FILTER_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_FILTER_PORT_INFO *info
  );

#ifdef BCM_88660
void
  SOC_PPC_EG_FILTER_GLOBAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_FILTER_GLOBAL_INFO *info
  );
#endif /* BCM_88660 */

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PVLAN_PORT_TYPE enum_val
  );

const char*
  SOC_PPC_EG_FILTER_PORT_ENABLE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_ENABLE enum_val
  );

void
  SOC_PPC_EG_FILTER_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_FILTER_PORT_INFO *info
  );

#ifdef BCM_88660

void 
  SOC_PPC_EG_FILTER_GLOBAL_INFO_print(
    SOC_SAND_IN SOC_PPC_EG_FILTER_GLOBAL_INFO *info
  );
#endif /* BCM_88660 */

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_FILTER_INCLUDED__*/
#endif

