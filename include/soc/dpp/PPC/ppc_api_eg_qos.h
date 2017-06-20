/* $Id: ppc_api_eg_qos.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_qos.h
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

#ifndef __SOC_PPC_API_EG_QOS_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_QOS_INCLUDED__

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
   *  Packet was forwarded according to MPLS label (LSR), and
   *  the label was poped (uniform) into ipv4.
   */
  SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV4 = 0,
  /*
   *  Packet was forwarded according to MPLS label (LSR), and
   *  the label was poped (uniform) into ipv6.
   */
  SOC_PPC_EG_QOS_UNIFORM_PHP_POP_INTO_IPV6 = 1,
  /*
   *  Number of types in SOC_PPC_EG_QOS_UNIFORM_PHP_TYPE
   */
  SOC_PPC_NOF_EG_QOS_UNIFORM_PHP_TYPES = 2
}SOC_PPC_EG_QOS_UNIFORM_PHP_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The DSCP/EXP value assigned by the ingress pipe as
   *  follows:- if packet forwarded according to IP header
   *  then equal to packet's DSCP (IPv4: TOS field, in IPv6:
   *  TC field)- else if packet forwarded according to MLPS
   *  label then equal to packet's EXP- else if packet was
   *  terminated then equal to the Qos value of the terminated
   *  packet.note that in_dscp_exp value may be updated by
   *  egress if PHP was performed:- If pipe pop was performed
   *  then the in_dscp_exp equal to the qos parameter of the
   *  internal header, (TC of IPv6, TOS of IPv4, EXP of MPLS
   *  label).- If uniform pop was performed then the
   *  in_dscp_exp updated according to the EXP of the poped
   *  MPLS label see soc_ppd_eg_qos_params_php_remark_set()
   *  for EXP remark only 3 lsb are considered in the remak
   */
  uint32 in_dscp_exp;
  /*
   *  Drop precedence, calculated by ingress pipe.
   *  Range: Soc_petraB: 0-1, ARAD: 0-3. 
   */
  SOC_SAND_PP_DP dp;
  /* 
   * Remark-profile. Range: 0-15.
   * valid only for ARAD   
   */ 
  uint32 remark_profile;

} SOC_PPC_EG_QOS_MAP_KEY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The DSCP/EXP value assigned by the ingress pipe as
   *  follows:- if packet forwarded according to IP header
   *  then equal to packet's DSCP (IPv4: TOS field, in IPv6:
   *  TC field)- else if packet forwarded according to MLPS
   *  label then equal to packet's EXP- else if packet was
   *  terminated then equal to the Qos value of the terminated
   *  packet.note that in_dscp_exp value may be updated by
   *  egress if PHP was performed:- If pipe pop was performed
   *  then the in_dscp_exp equal to the qos parameter of the
   *  internal header, (TC of IPv6, TOS of IPv4, EXP of MPLS
   *  label).- If uniform pop was performed then the
   *  in_dscp_exp updated according to the EXP of the poped
   *  MPLS label see soc_ppd_eg_qos_params_php_remark_set()
   *  for EXP remark only 3 lsb are considered in the remak
   */
  uint32 in_dscp_exp;
  /* 
   * Remark-profile. Range: 0-15.
   * valid only for ARAD. 
   */ 
  uint32 remark_profile;
  /*
   * The incoming packet header type. 
   * acceptable types: ipv4, ipv6 and mpls only. 
   * acceptable type ethernet is valid for ARAD-B0 and above.    
   */
  SOC_PPC_PKT_HDR_TYPE pkt_hdr_type;
  /*
   *  Drop precedence, calculated by ingress pipe.
   *  Range: 0-3.                                 
   *  Used in case acceptable type is ethernet. 
   *  valid only for ARAD-B0 and above.
   */
  SOC_SAND_PP_DP dp;

} SOC_PPC_EG_ENCAP_QOS_MAP_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ipv4 TOS. Range 0-255. This value is used when - packet
   *  is forwarded according to IPv4 header- or when packet is
   *  forwarded according to MPLS label (LSR), and label was
   *  poped into ipv4. (in this case the in-DSCP-Exp is mapped
   *  from the EXP field of the poped header)
   */
  SOC_SAND_PP_IPV4_TOS ipv4_tos;
  /*
   *  Ipv6 TC. Range 0-255. This value is used when - packet
   *  is forwarded according to IPv6 header- or when packet is
   *  forwarded according to MPLS label (LSR), and label was
   *  poped into ipv6. (in this case the in-DSCP-Exp is mapped
   *  from the EXP field of the poped header)
   */
  SOC_SAND_PP_IPV6_TC ipv6_tc;
  /*
   *  MPLS exp. Range 0-7. This value is used when - packet is
   *  forwarded according to MPLS label (LSR)
   */
  SOC_SAND_PP_MPLS_EXP mpls_exp;

} SOC_PPC_EG_QOS_PARAMS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ipv4 TOS. Range 0-255. This value is used when - packet
   *  is encapsulated according to IPv4/6 header- or when packet is
   *  forwarded according to MPLS label (LSR), and label was
   *  poped into ipv4/6. (in this case the in-DSCP-Exp is mapped
   *  from the EXP field of the poped header)
   */
  SOC_SAND_PP_IPV4_TOS ip_dscp;
  /*
   *  MPLS exp. Range 0-7. This value is used when - packet is
   *  encapsulated with MPLS tunnel
   */
  SOC_SAND_PP_MPLS_EXP mpls_exp;

} SOC_PPC_EG_ENCAP_QOS_PARAMS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  EXP map profile set according to out-port, see
   *  soc_ppd_eg_qos_port_info_set()Range: 0 - 3.
   */
  uint32 exp_map_profile;
  /*
   *  Php type pop into IPv4 or pop into IPv6
   */
  SOC_PPC_EG_QOS_UNIFORM_PHP_TYPE php_type;
  /*
   *  EXP value. From the poped headerRange: 0-7.
   */
  SOC_SAND_PP_MPLS_EXP exp;

} SOC_PPC_EG_QOS_PHP_REMARK_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  EXP map profile used to remark the qos parameter
   *  (mapping EXP to dscp/exp) when PHP is performed. see
   *  soc_ppd_eg_qos_params_php_remark_set(). Range: 0 - 3.
   */
  uint32 exp_map_profile;
  
#ifdef BCM_88660
  /*
  * Marking map profile used to DSCP, EXP marking according
  * to internal fields (TC, DP). Used when DSCP marking
  * is enabled and packet is bridged.
  * Mapping is done according to: soc_ppd_eg_qos_params_marking_set.
  * Range: 0-3.
  * Valid for ARAD Plus and above
  */
  uint32 marking_profile;
#endif /* BCM_88660 */

} SOC_PPC_EG_QOS_PORT_INFO;

#ifdef BCM_88660
typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * Resolved Drop precedence, calculated according to output Egress TM DP (2 bits).
   * Mapping is done using soc_ppd_eg_qos_global_info_set
   * Range: 0-1.
   * Valid for ARAD plus and above.
   */
  SOC_SAND_PP_DP resolved_dp_ndx;
  /*
   * Egress traffic class, passed according to egress TM pipe.
   * Range: 0-7.
   * Valid for ARAD plus and above.
   */
  SOC_SAND_PP_TC tc_ndx;
  /*
   * The LIF profile value assigned by the ingress pipe as
   * follows:
   * - Initially LIF-profile is set according to the LIF table.
   * - Can be updated according to VSI.In-LIF-profile in case VSI is BVID.
   * - Can be updated using PMF.
   * Only 2 lsbs of In-LIF-profile are passed to the egress.
   * Range: 0-3.
   * Note in_lif_profile is valid for mapping only if globally set to it.
   * Use soc_ppd_api_eg_qos_global_info_set.
   * Valid for ARAD plus and above.
   */
  uint32 in_lif_profile;
  /*
   * Marking profile set according to out-port, see
   * soc_ppd_api_eg_qos_port_info_set.
   * Range: 0-3.
   * Valid for ARAD plus and above.
   */
  uint32 marking_profile;
  /*
   * DP map mode. This is the value of DP_MAP_FIX_ENABLEDf on Jerico_B0.
   * This is the value of CFG_MARKING_DP_MAP_DISABLE on QAX.
   * Range: 0-1.
   * Valid for Jerico_B0 and QAX.
   */
  uint32 dp_map_disabled;
} SOC_PPC_EG_QOS_MARKING_KEY;

typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  DSCP. Used for IPV4 TOS or IPV6.
   *  Range 0-255.
   *  This value is used when
   *  - packet is bridged
   *  - incoming logical interface is marked enabled.
   *  - ethertype is IPV4/6.
   *  Valid for ARAD plus and above.
   */
  SOC_SAND_PP_IPV4_TOS ip_dscp;
  /*
   *  MPLS exp. Range 0-7. This value is used when
   *  - packet is bridged
   *  - incoming logical interface is marked enabled
   *  - ethertype is MPLS.
   *  Valid for ARAD plus and above.
   */
  SOC_SAND_PP_MPLS_EXP mpls_exp;
} SOC_PPC_EG_QOS_MARKING_PARAMS;


typedef struct 
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mapped Egress DP to resolved DP.
   *  Used for DSCP marking.
   *  Field is DP bitmap. Each DP 0-3 is bit.
   *  In case DP x is set then its resolved DP is 1,
   *  otherwise 0.
   *  Valid for ARAD plus and above.
   */
  uint32 resolved_dp_bitmap;
  /*
   *  In-LIF-profile bitmap enable.
   *  Used for DSCP marking.
   *  Field is In-LIF_profile bitmap. 0-1 LIF profiles are valid range.
   *  In case In-LIF-profile x is set then DSCP marking is enabled.
   *  Valid for ARAD plus and above.
   */
  uint32 in_lif_profile_bitmap;
  /*
   *  Mapped In-LIF-Profile to resolved In-LIF-Profile.
   *  Used for DSCP marking.
   *  Field is In-LIF_profile bitmap. 0-3 LIF profiles are valid range.
   *  In case In-LIF-profile x is set then its resolved IN-LIF-Profile is 1,
   *  otherwise 0.
   *  Valid for Jericho B0.
   */
  uint32 resolved_in_lif_profile_bitmap;
  /*
   *  TC-DP mapping mode.
   *  Used for DSCP marking.
   *  mode 0: {TC, DP(2),Mapped-IN-LIF-Profile(1),PP-Port-Profile}
   *  mode 1: {TC,Mapped-DP(1),IN-LIF-Profile(2),PP-Port-Profile}
   *  Valid for Jericho B0.
   */
  uint32 dp_map_mode;
} SOC_PPC_EG_QOS_GLOBAL_INFO;

#endif /* BCM_88660 */

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
  SOC_PPC_EG_QOS_MAP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_MAP_KEY *info
  );

void
  SOC_PPC_EG_ENCAP_QOS_MAP_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_QOS_MAP_KEY *info
  );

void
  SOC_PPC_EG_QOS_PARAMS_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_PARAMS *info
  );

void
  SOC_PPC_EG_ENCAP_QOS_PARAMS_clear(
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_QOS_PARAMS *info
  );

void
  SOC_PPC_EG_QOS_PHP_REMARK_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_PHP_REMARK_KEY *info
  );

void
  SOC_PPC_EG_QOS_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_PORT_INFO *info
  );

#ifdef BCM_88660
void
  SOC_PPC_EG_QOS_MARKING_KEY_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_MARKING_KEY *info
  );

void
  SOC_PPC_EG_QOS_MARKING_PARAMS_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_MARKING_PARAMS *info
  );

void
  SOC_PPC_EG_QOS_GLOBAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );
#endif /* BCM_88660 */

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_EG_QOS_UNIFORM_PHP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_EG_QOS_UNIFORM_PHP_TYPE enum_val
  );

void
  SOC_PPC_EG_QOS_MAP_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY *info
  );

void
  SOC_PPC_EG_ENCAP_QOS_MAP_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY *info
  );

void
  SOC_PPC_EG_QOS_PARAMS_print(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS *info
  );

void
  SOC_PPC_EG_ENCAP_QOS_PARAMS_print(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS *info
  );

void
  SOC_PPC_EG_QOS_PHP_REMARK_KEY_print(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY *info
  );

void
  SOC_PPC_EG_QOS_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO *info
  );

#ifdef BCM_88660
void
  SOC_PPC_EG_QOS_MARKING_KEY_print(
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *info
  );

void
  SOC_PPC_EG_QOS_MARKING_PARAMS_print(
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *info
  );

void 
  SOC_PPC_EG_QOS_GLOBAL_INFO_print(
    SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );
#endif /* BCM_88660 */

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_QOS_INCLUDED__*/
#endif

