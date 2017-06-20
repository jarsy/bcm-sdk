/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_MULTICAST_EGRESS_INCLUDED__
/* { */
#define __SOC_TMC_API_MULTICAST_EGRESS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_general.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* $Id$
 *     In order to configure the egress replication bitmap in
 *     ARAD and above we need a bitmap of at least 256 bits,
 *     In Soc_petra-B we need 80 bits.
 */

#define SOC_TMC_MULT_EG_NOF_UINT32S_IN_BITMAP_PETRA   SOC_SAND_DIV_ROUND_UP(SOC_TMC_NOF_FAP_PORTS_PETRA, SOC_SAND_REG_SIZE_BITS)
#define SOC_TMC_MULT_EG_NOF_UINT32S_IN_BITMAP_ARAD    SOC_SAND_DIV_ROUND_UP(SOC_TMC_NOF_FAP_PORTS_ARAD, SOC_SAND_REG_SIZE_BITS)
#define SOC_TMC_MULT_EG_NOF_UINT32S_IN_BITMAP_MAX     SOC_SAND_DIV_ROUND_UP(SOC_TMC_NOF_FAP_PORTS_JERICHO, SOC_SAND_REG_SIZE_BITS)

/* mark to mbcm_dpp_mult_copy_replications_from_arrays that a bitmap is being passed */
#define ARAD_MC_EGR_IS_BITMAP_BIT 0x80000000


/*
 *  Maximal number of MC Id-s used for VLAN membership MC
 */
#define SOC_TMC_MULT_EG_VLAN_NOF_IDS_MAX (4 * 1024)

/*
 *  Maximal number of MC Id-s used for VLAN membership MC in Soc_petra
 */

/*
 *  Maximal number of MC Id-s used for VLAN membership MC in ARAD 
 */
#define SOC_TMC_MULT_EG_VLAN_NOF_IDS_MAX_ARAD (8 * 1024)

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
   *  The entry is an OFP port.
   */
  SOC_TMC_MULT_EG_ENTRY_TYPE_OFP = 0,
  /*
   *  The entry is a pointer to an existing VLAN multicast
   *  group. Not valid for Soc_petra-A.
   */
  SOC_TMC_MULT_EG_ENTRY_TYPE_VLAN_PTR = 1,
  /*
   *  Total number of entry types.
   */
  SOC_TMC_MULT_EG_ENTRY_NOF_TYPES = 2
}SOC_TMC_MULT_EG_ENTRY_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The lowest value of MC-ID that is mapped to replication
   *  with bitmap. This range configures a continuous range of
   *  MC-IDs to be used for Vlan-Membership Multicast
   *  purposes. Range: 0.
   */
  uint32 mc_id_low;
  /*
   *  The upper value of MC-ID that is a direct bitmap group.
   *  Supported till Jericho. Range: 0 - 8K-1.
   */
  int32 mc_id_high;

}SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Entry type: Port or VLAN Multicast group.
   */
  SOC_TMC_MULT_EG_ENTRY_TYPE type;
  /*
   *  Out Going FAP-Port Id. 0 to 79
   */
  SOC_TMC_FAP_PORT_ID port;
  /*
   *  VLAN multicast group Id which the linked list member
   *  points to. Range: 0 - 4K-1. Not valid for Soc_petra-A.
   */
  uint32 vlan_mc_id;
  /*
   *  Copy-Unique-Data for the replication copy (resp. copies,
   *  in case of a VLAN replication) of the packet. This is
   *  the value in the Outgoing TM Header (outlif) when the
   *  packet is sent out of a TM-type port. That is, user
   *  should first allocate the egress-cud in specific FAP,
   *  then add this value to the needed ingress-multicast
   *  groups. Ports of Packet Processing types use this field
   *  as the ARP-Pointer. Range: 0 - 65535.
   */
  uint32 cud;

}SOC_TMC_MULT_EG_ENTRY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Array containing a bitmap of the output TM/PP ports of a device to be replicated to by an egress multicast bitmap.
   */
  uint32 bitmap[SOC_TMC_MULT_EG_NOF_UINT32S_IN_BITMAP_MAX + 1]; 
 
}SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP;

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
  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_clear(
    SOC_SAND_OUT SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
  );

void
  SOC_TMC_MULT_EG_ENTRY_clear(
    SOC_SAND_OUT SOC_TMC_MULT_EG_ENTRY *info
  );

void
  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_clear(
    SOC_SAND_OUT SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_MULT_EG_ENTRY_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_MULT_EG_ENTRY_TYPE enum_val
  );

void
  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_print(
    SOC_SAND_IN SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
  );

void
  SOC_TMC_MULT_EG_ENTRY_print(
    SOC_SAND_IN SOC_TMC_MULT_EG_ENTRY *info
  );

void
  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_print(
    SOC_SAND_IN SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_MULTICAST_EGRESS_INCLUDED__*/
#endif
