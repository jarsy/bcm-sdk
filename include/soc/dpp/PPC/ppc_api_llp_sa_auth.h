/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_llp_sa_auth.h
*
* MODULE PREFIX:  soc_ppc_llp
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

#ifndef __SOC_PPC_API_LLP_SA_AUTH_INCLUDED__
/* { */
#define __SOC_PPC_API_LLP_SA_AUTH_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Use to accept SA from all VIDS. Note that when SA is not
 *     found in the authentication DB, then it will be
 *     accepted.                                               */
#define  SOC_PPC_LLP_SA_AUTH_ACCEPT_ALL_VIDS (0xFFFFFFFF)

/*     Use to accept SA from all source system ports. Note that
 *     when SA is not found in the authentication DB, then it
 *     will be accepted.                                       */
#define  SOC_PPC_LLP_SA_AUTH_ACCEPT_ALL_PORTS (0xFFFFFFFF)

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
   *  Entries that used for port authentication
   */
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE_PORT = 0x1,
  /*
   *  Entries that used for VID authentication
   */
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE_VID = 0x2,
  /*
   *  All enitres used for SA-authentication
   */
  SOC_PPC_LLP_SA_MATCH_RULE_TYPE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE
   */
  SOC_PPC_NOF_LLP_SA_AUTH_MATCH_RULE_TYPES = 3
}SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Whether to enable MAC SA authentication over packets
   *  entering the device from this port.
   */
  uint8 sa_auth_enable;

} SOC_PPC_LLP_SA_AUTH_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE and packets are untagged, then the
   *  Authentication test fails and proper action is taken.
   */
  uint8 tagged_only;
  /*
   *  Expected outer VLAN ID. If the outer VID of the incoming
   *  packet is not equal to this, the VID check fails and
   *  proper action is taken. Set to
   *  SOC_PPC_LLP_SA_AUTH_ACCEPT_ALL_VIDS in order to skip this
   *  check.
   */
  SOC_SAND_PP_VLAN_ID expect_tag_vid;
  /*
   *  Expected in system port. If the incoming packet enters
   *  from another system port, then this check fails and
   *  proper action is taken. Set id to
   *  SOC_PPC_LLP_SA_AUTH_ACCEPT_ALL_PORTS in order to skip this
   *  check. System port can be also LAG.
   */
  SOC_SAND_PP_SYS_PORT_ID expect_system_port;

} SOC_PPC_LLP_SA_AUTH_MAC_INFO;
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Rule type specifies which entries to return
   */
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE rule_type;
  /*
   *  Port value to match, use SOC_PPD_IGNORE_VAL to not compare
   *  to port.id value.
   */
  SOC_SAND_PP_SYS_PORT_ID port;
  /*
   *  VID value to match, use SOC_PPD_IGNORE_VAL to not compare to
   *  VID value.
   */
  SOC_SAND_PP_VLAN_ID vid;

} SOC_PPC_LLP_SA_AUTH_MATCH_RULE;


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
  SOC_PPC_LLP_SA_AUTH_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_SA_AUTH_PORT_INFO *info
  );

void
  SOC_PPC_LLP_SA_AUTH_MAC_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_SA_AUTH_MAC_INFO *info
  );

void
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_clear(
    SOC_SAND_OUT SOC_PPC_LLP_SA_AUTH_MATCH_RULE *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_TYPE enum_val
  );

void
  SOC_PPC_LLP_SA_AUTH_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_SA_AUTH_PORT_INFO *info
  );

void
  SOC_PPC_LLP_SA_AUTH_MAC_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_SA_AUTH_MAC_INFO *info
  );

void
  SOC_PPC_LLP_SA_AUTH_MATCH_RULE_print(
    SOC_SAND_IN  SOC_PPC_LLP_SA_AUTH_MATCH_RULE *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LLP_SA_AUTH_INCLUDED__*/
#endif

