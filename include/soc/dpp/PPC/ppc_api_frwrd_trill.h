/* $Id: ppc_api_frwrd_trill.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_trill.h
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifndef __SOC_PPC_API_FRWRD_TRILL_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_TRILL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     When Adding multicast route, the following fields of the
 *     route key are optional: FID, Ingress NickName. and
 *     Adjacent NickName.                                      */
#define  SOC_PPC_TRILL_MC_IGNORE_FIELD (0xFFFFFFFF)

/*     Use to accept SA from all source system ports in
 *     soc_ppd_frwrd_trill_ adj _info_set ()                       */
#define  SOC_PPC_TRILL_ACCEPT_ALL_PORTS (0xffffffff)

#define DEFAULT_TRILL_ETHER_TYPE                                0x2109

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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mask Ingress NickName:When FALSE: The tree route
   *  forwarding depends on packet Ingress NickName. The
   *  Ingress Nick name is the packet originator. Used for
   *  TRILL RPF
   */
  uint8 mask_ing_nickname;
  /*
   *  Mask Adjacent NickName:When FALSE: The tree route
   *  forwarding depends on packet Adjacent NickName. Used for
   *  TRILL RPF
   */
  uint8 mask_adjacent_nickname;
  /*
   *  Mask FID:When FALSE: The tree route forwarding depends
   *  on packet FID
   */
  uint8 mask_fid;

} SOC_PPC_TRILL_MC_MASKED_FIELDS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Dist-Tree-Nick: The multicast tree nickname
   */
  uint32 tree_nick;
  /*
   *  MACT Filter ID.'SOC_PPC_TRILL_MC_IGNORE_FIELD': The FID in
   *  the TRILL multicast lookup. The FID is used as part of
   *  the key to:1. Attach the assigned forwarders of the
   *  VLAN. When masked, the multicast group should contain
   *  the assigned forwarders of all the VLANs.2. Enable
   *  pruning when the IS-IS protocol finds out that there is
   *  no purpose to replicate to a specific adjacent, since
   *  none of the RBridges behind have assigned forwarder for
   *  the VLAN.
   *  Relevant for PetraB, or Arad in VL mode */
  SOC_PPC_FID fid;

  /* Relevant for ARAD, in FGL mode */
  SOC_PPC_FID outer_vid; /* Used in both FGL and VL packets */
  SOC_PPC_FID inner_vid; /* Used in FGL */

  /*
   *  The Ingress Nick-Name as it arrives on the packet TRILL
   *  header. Indicates the originator RBridge. The TRILL RPF
   *  enables discarding the packet, when the ingress
   *  nick-name is invalid for packets that arrive from the
   *  adjacent RBridge.'SOC_PPC_TRILL_MC_IGNORE_FIELD': Indicates
   *  that the ing_nickname is masked
   */
  uint32 ing_nick;
  /*
   *  The Adjacent egress encapsulation pointer. Indicates the
   *  adjacent RBridge that the packet arrived from. The TRILL
   *  RPF enables discarding the packet, when the ingress
   *  nick-name is invalid for packets that arrive from the
   *  adjacent RBridge.'SOC_PPC_TRILL_MC_IGNORE_FIELD': Indicates
   *  that the adjacent_eep is masked
   */
  uint32 adjacent_eep;

  /*
   * esadi bit
   */
  uint8 esadi;

  /*
   * Trill transparent service bit
   */
  uint8 tts;
} SOC_PPC_TRILL_MC_ROUTE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Expected adjacent EEP. If the EEP of the incoming packet
   *  is not equal to this, the check fails and proper action
   *  is taken.
   */
  uint32 expect_adjacent_eep;
  /*
   *  Expected in system port. If the incoming packet enters
   *  from another system port, then this check fails and
   *  proper action is taken. Set id to
   *  SOC_PPD_TRILL_SA_BASED_EEP_ACCEPT_ALL_PORTS in order to skip
   *  this check. System port can be also LAG.
   */
  SOC_SAND_PP_SYS_PORT_ID expect_system_port;

} SOC_PPC_TRILL_ADJ_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Initial TRILL time to live to be assigned for any trill
   *  packet.
   */
  uint32 cfg_ttl;
  uint32 ethertype;
} SOC_PPC_FRWRD_TRILL_GLOBAL_INFO;


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
  SOC_PPC_TRILL_MC_MASKED_FIELDS_clear(
    SOC_SAND_OUT SOC_PPC_TRILL_MC_MASKED_FIELDS *info
  );

void
  SOC_PPC_TRILL_MC_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_TRILL_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_TRILL_ADJ_INFO_clear(
    SOC_SAND_OUT SOC_PPC_TRILL_ADJ_INFO *info
  );

void
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_TRILL_MC_MASKED_FIELDS_print(
    SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS *info
  );

void
  SOC_PPC_TRILL_MC_ROUTE_KEY_print(
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY *info
  );

void
  SOC_PPC_TRILL_ADJ_INFO_print(
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO *info
  );

void
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_TRILL_INCLUDED__*/
#endif

