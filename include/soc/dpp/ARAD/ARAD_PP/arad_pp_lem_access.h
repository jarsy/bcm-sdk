/* $Id: arad_pp_lem_access.h,v 1.59 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright
 * $
*/

#ifndef __ARAD_PP_LEM_ACCESS_INCLUDED__
/* { */
#define __ARAD_PP_LEM_ACCESS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/SAND/Utils/sand_pp_mac.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/arad_tcam.h>

#include <soc/dpp/PPC/ppc_api_llp_sa_auth.h>
#include <soc/dpp/PPC/ppc_api_llp_vid_assign.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ilm.h>



/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* this is effective number of entries, actual may be more */
#define DPP_PP_LEM_NOF_ENTRIES(unit)                          (SOC_DPP_DEFS_GET(unit, nof_lem_lines) + 32)

/* Common use of ARAD_PP_LEM_ACCESS_KEY for LEM and ELK buffer building */
#define ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_LEM              (3)
#define ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_ELK              (4)
#define ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S(type)            (ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_##type)
#define ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_MAX              (SOC_SAND_MAX(ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_LEM, ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_ELK))
#define ARAD_PP_LEM_KEY_MAX_NOF_PARAMS                      (5)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_EXTENDED      (1)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_MAC           (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_BACKBONE_MAC  (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IPV4_MC       (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IP_HOST       (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_BFD_SINGLE_HOP       (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_SA_AUTH       (1)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_SA_AUTH_ARAD_B0 (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_TRILL_ADJ     (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_ILM           (4)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_TRILL_UC      (1)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_TRILL_MC(unit) (SOC_IS_DPP_TRILL_FGL(unit) ? 4 : 3)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_MAC_IN_MAC_TUNNEL (3)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_MAC_IN_MAC_TEST2 (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IPMC_BIDIR    (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_GLOBAL_IPV4_MC (3)

/* VFT, D_ID */
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_FC_LOCAL        (2)
/* VFT, DOMAIN */
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_FC_REMOTE       (2)
/* VFT, D_ID, S_ID */
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_FC_ZONING       (3)

#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IP_SPOOF_DHCP   (3)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IPV4_SPOOF_STATIC (2)

#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_SLB             (3)

#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_IP6_SPOOF_STATIC (2)
#define ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_DIP6_COMPRSSION  (2)

/*
 *	Prefix characteristic: value and number of significant msb bits.
 *  For example, Backbone MAC keys have a '000' in their msbs:
 *  value is 0 ('000'), size (n bits) is 3. For MAC addresses, fix a number of bits of 1
 *  and a key prefix of 0
 */


#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_MAC(unit)                  (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_ETH(unit))
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_MAC           SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_BACKBONE_MAC(unit)         (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BMAC(unit))
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_BACKBONE_MAC  SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_IPV4_MC                    (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPV4_COMP)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPV4_MC       SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_IP_HOST                    (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP_HOST)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IP_HOST       SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_GLOBAL_IPV4_MC             (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_GLOBAL_IPV4_COMP) /*prefix value is 11xx*/
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_GLOBAL_IPV4_MC (SOC_IS_JERICHO(unit)? SOC_DPP_DEFS_GET(unit, nof_lem_prefixes): 2)


#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS                    SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_ALL_MASKED                     ((1 << SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)) - 1)

/* SA key is aligned differently than all other keys:
   All other keys: Prefix, Padding with Zeros, Key
   SA auth: 11'b0, 15b prefix, 48b sa key.
   In order to use the existing prefix logic, we define the prefix length to be 26b.
   IMPORTANT: This conflicts with prefix 4'b0 */
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_SA_AUTH              (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SA_AUTH)

#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_TRILL_ADJ            (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_ADJ)


#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_SA_AUTH(unit)    ((SOC_IS_ARAD_B0_AND_ABOVE(unit)) ? (SOC_IS_JERICHO(unit)? 4:SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)):26)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_TRILL_ADJ        SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_ILM                           (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_ILM)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_EXTENDED                      (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_EXTEND)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_ILM              SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_EXTENDED         SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_TRILL_UC                      (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_UC)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_TRILL_UC         SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_TRILL_MC                      (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_MC)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_TRILL_MC         SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_IPV4_SPOOF_DHCP                   (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP_SPOOF_DHCP)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_IPV4_SPOOF_STATIC                 (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPV4_SPOOF_STATIC)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IP_SPOOF_DHCP    (SOC_IS_JERICHO(unit)? SOC_DPP_DEFS_GET(unit, nof_lem_prefixes): 1)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPV4_SPOOF_STATIC SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_EXTENDED_KEY_PREFIX_NOF_BITS                            SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_MAC_IN_MAC_TUNNEL             (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_MAC_IN_MAC_TUNNEL)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_MAC_IN_MAC_TUNNEL SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_IPMC_BIDIR                    (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPMC_BIDIR)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IPMC_BIDIR       SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_SLB                           (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SLB)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_SLB              SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_FC                            (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPMC_BIDIR)

#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_IP6_SPOOF_STATIC                 (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP6_SPOOF_STATIC)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_DIP6_COMPRESSION                 (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_DIP6_COMPRESSION)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_IP6_SPOOF_STATIC SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)
#define ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_DIP6_COMPRESSION SOC_DPP_DEFS_GET(unit, nof_lem_prefixes)


#define ARAD_PP_LEM_ACCESS_NOF_PREFIXES                       (1 << SOC_DPP_DEFS_GET(unit, nof_lem_prefixes))
#define ARAD_PP_LEM_ACCESS_PREFIX_ALLOC_WITH_ID               0x1

#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_MAC          (48)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_MAC          (15) /* Consider FID and not System-VSI */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_BACKBONE_MAC (48)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_BACKBONE_MAC (12)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IPV4_MC      (28)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IPV4_MC_DBAL (16)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IPV4_MC      (15)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IPV4_MC_DBAL (12)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_IPV4_MC_DBAL (15)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP_HOST      (32)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP_HOST      (SOC_IS_JERICHO(unit) ? (14) : (12))
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_SA_AUTH      (48)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_SA_AUTH      (8)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_BFD_DETECT_MULTIPLIER    (8)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_TRILL_ADJ    (48)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_TRILL_ADJ    (SOC_IS_JERICHO(unit) ? (9) : (8))
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_ILM          (20)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_ILM          (3)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM          (8)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM_JERICHO  (9) /* Including Core */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_ILM          (12)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_VRF_IN_ILM   (15)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_TRILL_UC     (16)

#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_GLOBAL_IPV4_MC      (28) /* DIP 28LSBs*/
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_GLOBAL_IPV4_MC      (32) /* SIP 32 bits*/
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_GLOBAL_IPV4_MC      (12) /* RIF ID 12 bits*/

/* Two modes <msb ... lsb>:
   VL - <FID,ESADI,DIST-TREE>
   FGL - <Inner-vid,Outer-vid,ESADI,DIST-TREE> */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_TRILL_MC            (16) /* Dist-Tree */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_TRILL_MC            (1)  /* ESADI */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_TRILL_MC(unit) \
  (SOC_IS_DPP_TRILL_FGL(unit) ? (12) /* Outer-vid */ : (15) /* FID (VSI) */)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_TRILL_MC(unit) \
  (SOC_IS_DPP_TRILL_FGL(unit) ? (12) /* Inner-vid */ : (0) /* None */)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP_SPOOF_DHCP       (41)/* Arad:SA 41 LSBs*/
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP_SPOOF_DHCP_JERICHO       (18) /* Jericho: In-AC */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP_SPOOF_DHCP        (16) /*Arad: In-AC */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP_SPOOF_DHCP_JERICHO        (41)  /*Jericho:SA 41 LSBs */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_IP_SPOOF_DHCP       (16) /* Spoof-ID */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IPV4_SPOOF_STATIC   (32) /* SIP */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IPV4_SPOOF_STATIC(unit) \
      (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) ? (12) /* FID(VSI) */ : (SOC_IS_JERICHO(unit) ? 18 :16) /* In-AC */)
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_MAC_IN_MAC_TUNNEL   (48) /* BSA 24 bits*/
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_MAC_IN_MAC_TUNNEL   (12) /* BFID */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_MAC_IN_MAC_TUNNEL   (8)  /* In-PP-Port */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_MAC_IN_MAC_TUNNEL   (9)  /* Core-ID + In-PP-Port */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IPMC_BIDIR          (SOC_IS_JERICHO(unit) ? (15) : (12))
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IPMC_BIDIR          (8)

#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_SLB                 (47) /* Flow-Label */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_SLB                 (15) /* Destination. */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_SLB                 (1)  /* Is-destination-FEC*/
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_IP6_SPOOF_STATIC    (64) /* SIP bits[39:0] */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_IP6_SPOOF_STATIC    (9) /* TT TCAM lookup result  bits[8:1] is SIP[127:63] compression result, 1LSB always is 1 */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_DIP6_COMPRESSION    (56) /* SIP bits[39:0] */
#define ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_DIP6_COMPRESSION    (12) /* TT TCAM lookup result */



#define SOC_DPP_DEFS_LEM_WIDTH_IN_UINT32S(unit)  (SOC_DPP_DEFS_GET(unit, lem_width) / SOC_SAND_NOF_BITS_IN_UINT32 + 1)
#define SOC_DPP_DEFS_MAX_LEM_WIDTH_IN_UINT32S    (SOC_DPP_DEFS_MAX(LEM_WIDTH) / SOC_SAND_NOF_BITS_IN_UINT32 + 1)
#define ARAD_PP_LEM_ACCESS_ASD_NOF_BITS           (24)
#define ARAD_PP_LEM_ACCESS_DEST_NOF_BITS          (19)

#define ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_BITS       (ARAD_PP_LEM_ACCESS_ASD_NOF_BITS + ARAD_PP_LEM_ACCESS_DEST_NOF_BITS + 1)
#define ARAD_PP_LEM_ACCESS_PAYLOAD_IN_UINT32S       (ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_BITS / SOC_SAND_NOF_BITS_IN_UINT32 + 1)

/*
 *	Payload composition
 */
#define ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S                  (2)
#define ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_LSB                 (SOC_SAND_REG_SIZE_BITS - ARAD_PP_LEM_ACCESS_DEST_NOF_BITS)
#define ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_MSB                 (SOC_SAND_REG_MAX_BIT)
#define ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_NOF_BITS            \
  (ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_MSB - ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_LSB + 1)
#define ARAD_PP_LEM_ACCESS_ASD_SCND_REG_LSB                  (0)
#define ARAD_PP_LEM_ACCESS_ASD_SCND_REG_MSB                   \
  (ARAD_PP_LEM_ACCESS_ASD_NOF_BITS - ARAD_PP_LEM_ACCESS_ASD_FIRST_REG_NOF_BITS - 1)

#define ARAD_PP_LEM_ACCESS_IS_DYN_LSB                        (SOC_IS_JERICHO(unit) ? (44-32) : (42-32))
#define ARAD_PP_LEM_ACCESS_DROP_SA                           (SOC_IS_JERICHO(unit) ? (43-32) : (41-32))

#define ARAD_PP_LEM_ACCESS_GROUP_LSB                        (SOC_IS_JERICHO(unit) ? (38-32) : (36-32))
#define ARAD_PP_LEM_ACCESS_GROUP_MSB                         (SOC_IS_JERICHO(unit) ? (40-32) : (38-32))

#define ARAD_PP_LEM_ACCESS_ENTRY_TYPE_LSB                     (SOC_IS_JERICHO(unit) ? (41-32) : (39-32))
#define ARAD_PP_LEM_ACCESS_ENTRY_TYPE_MSB                      (SOC_IS_JERICHO(unit) ? (42-32) : (40-32))

/*
 *	Field values
 */
#define ARAD_PP_LEM_ACCESS_CMD_DELETE_FLD_VAL      0
#define ARAD_PP_LEM_ACCESS_CMD_INSERT_FLD_VAL      1
#define ARAD_PP_LEM_ACCESS_CMD_REFRESH_FLD_VAL     2
#define ARAD_PP_LEM_ACCESS_CMD_LEARN_FLD_VAL       3
#define ARAD_PP_LEM_ACCESS_CMD_DEFRAG_FLD_VAL      4
#define ARAD_PP_LEM_ACCESS_CMD_ACK_FLD_VAL         5
#define ARAD_PP_LEM_ACCESS_CMD_TRANSPLANT_FLD_VAL  6
#define ARAD_PP_LEM_ACCESS_CMD_ERROR_FLD_VAL       7


/*
* ASD formatting for different applications
*/

/*
 * ASD formatting for TRILL-SA-authentication
 */
 
#define ARAD_PP_LEM_ACCESS_ASD_TRILL_SA_AUTH_EEP_LSB       (0)
#define ARAD_PP_LEM_ACCESS_ASD_TRILL_SA_AUTH_EEP_MSB(unit)       (SOC_DPP_DEFS_GET(unit, out_lif_nof_bits) -1)
#define ARAD_PP_LEM_ACCESS_ASD_TRILL_SA_AUTH_EEP_LEN(unit)       (ARAD_PP_LEM_ACCESS_ASD_TRILL_SA_AUTH_EEP_MSB(unit) - ARAD_PP_LEM_ACCESS_ASD_TRILL_SA_AUTH_EEP_LSB + 1)


/*
 * ASD formatting for SA-authentication
 */
 /* updated for Arad, this is how to set 23 bits, in mact payload, not including destination */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LSB       (0) /* In MACT Payload: 19 */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_MSB       (11) /* In MACT Payload: 30 */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LEN       (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_VID_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_LSB     (12) /* In MACT Payload: 31 */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_MSB     (SOC_IS_JERICHO(unit)?18:16) /* In MACT Payload: 35 (Arad), 37 (Jericho) */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_LSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_RSRVD_MSB + 1) /* Arad: 17, Jericho: 19 - In MACT Payload: 36 (Arad), 38 (Jericho) */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_MSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_LSB) /* Arad: 17, Jericho: 19  - In MACT Payload: 36 (Arad), 38 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_LSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_ACCPT_UNTGD_MSB + 1) /* Arad: 18, Jericho: 20 - In MACT Payload: 37 (Arad), 39 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_MSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_LSB) /* Arad: 18, Jericho: 20 - In MACT Payload: 37 (Arad), 39 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_LSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_DROP_DIF_VID_MSB + 1) /* Arad: 19, Jericho: 21 - In MACT Payload: 38 (Arad), 40 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_MSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_LSB) /* Arad: 19, Jericho: 21 - In MACT Payload: 38 (Arad), 40 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_PRMT_ALL_PORTS_MSB + 1) /* Arad: 20, Jericho: 22 - In MACT Payload: 39 (Arad), 41 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_MSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LSB) /* Arad: 20, Jericho: 22 - In MACT Payload: 39 (Arad), 41 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_LSB + 1)

#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_LSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_TAGGED_MSB + 1) /* Arad: 21, Jericho: 23 - In MACT Payload: 40 (Arad), 42 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_MSB     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_LSB) /* Arad: 21, Jericho: 23 - In MACT Payload: 40 (Arad), 42 (Jericho)  */
#define ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_LEN     (ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_MSB - ARAD_PP_LEM_ACCESS_ASD_SA_AUTH_OVR_VID_IN_UNTAGGED_LSB + 1)


#define ARAD_PP_LEM_ENTRY_TYPE_OUTLIF     (0)
#define ARAD_PP_LEM_ENTRY_TYPE_EEI_FEC    (1)
#define ARAD_PP_LEM_ENTRY_TYPE_EEI_MC     (2)
#define ARAD_PP_LEM_ENTRY_TYPE_MIM        (3)
#define ARAD_PP_LEM_ENTRY_TYPE_FORMAT_3   (4) /* SA Authentication */
#define ARAD_PP_LEM_ENTRY_TYPE_FORMAT_2   (5)
#define ARAD_PP_LEM_ENTRY_TYPE_FORMAT_SLB (6)
#define ARAD_PP_LEM_ENTRY_TYPE_FORMAT_3B  (7)
#define ARAD_PP_LEM_ENTRY_TYPE_HOST_FORMAT (8)
#define ARAD_PP_LEM_ENTRY_TYPE_RAW_DATA   (10)
#define ARAD_PP_LEM_ENTRY_TYPE_UNKNOWN    (0xffffffff)

/* Flags for arad_pp_lem_access_parse */
#define ARAD_PP_LEM_ACCESS_PARSE_FLAGS_ACK (1 << 0) /* Return only the ack and stamp */
#define ARAD_PP_LEM_ACCESS_LEARN_EVENT (1 << 1) /* Signal the parse that the buffer is a learn/transplant/age out event */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_LEM_ACCESS_HI_FROM_EEI_MASK        (0xF)
#define ARAD_PP_LEM_ACCESS_HI_FROM_EEI_LSB         (16)
/* set HI (4 dest mac address lsbs). From EEI: eei_type[24:20] HI[16:19] eei[0:15] */
#define ARAD_PP_LEM_ACCESS_HI_FROM_EEI(eei, hi) \
   ((hi) = (eei >> ARAD_PP_LEM_ACCESS_HI_FROM_EEI_LSB) & ARAD_PP_LEM_ACCESS_HI_FROM_EEI_MASK)
/* set EEI field */
#define ARAD_PP_LEM_ACCESS_HI_TO_EEI(hi, eei) \
   ((eei) = (eei) | ((hi & ARAD_PP_LEM_ACCESS_HI_FROM_EEI_MASK) << ARAD_PP_LEM_ACCESS_HI_FROM_EEI_LSB))



/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_MC            (ARAD_PP_FLP_TRILL_KEY_OR_MASK_MC) /* TRILL MC */ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0000*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BMAC(unit)          (ARAD_PP_FLP_B_ETH_KEY_OR_MASK(unit)) /*b-mac*/ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0001*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPV4_COMP           (ARAD_PP_FLP_IPV4_COMP_KEY_OR_MASK) /* ipv4-mc comp*/ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0010 */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_MAC_IN_MAC_TUNNEL   (ARAD_PP_FLP_MAC_IN_MAC_TUNNEL_KEY_OR_MASK) /* learn b-mac */ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_1100 */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_EXTEND              (ARAD_PP_FLP_P2P_KEY_OR_MASK)  /* extended */ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0011 */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_UC            (ARAD_PP_FLP_TRILL_KEY_OR_MASK) /* TRILL UC */ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0100*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP_HOST             (ARAD_PP_FLP_IPV4_KEY_OR_MASK) /*IP-host*/ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0101*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_ILM                 (ARAD_PP_FLP_LSR_KEY_OR_MASK) /*ILM */ /* ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_0110 */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_LSR_CNT              (ARAD_PP_FLP_LSR_CNT_KEY_OR_MASK)  /*LSR_CNT_PREFIX_1110*/

#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SA_AUTH             (0xe) /* SA-AUTH*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_TRILL_ADJ           (ARAD_PP_FLP_TRILL_ADJ_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_ETH(unit)           (ARAD_PP_FLP_ETH_KEY_OR_MASK(unit)) /* was ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_1*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_DUMMY(unit)         (ARAD_PP_FLP_DUMMY_KEY_OR_MASK(unit))
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP_SPOOF_DHCP       (ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPV4_SPOOF_STATIC   (ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_GLOBAL_IPV4_COMP    (ARAD_PP_FLP_GLOBAL_IPV4_KEY_OR_MASK)
/* 11xx, take 4 prefix, from 12 to 15 */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_GLOBAL_IPV4_COMP_VAL (ARAD_PP_FLP_GLOBAL_IPV4_KEY_OR_MASK>>1)

/* The following (and also TRILL_ADJ) are actually app_ids only, and not prefixes. */
/* The prefixes of the IDs are allocated dynamically. */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_FC_LOCAL_N_PORT     (ARAD_PP_FLP_FC_N_PORT_KEY_OR_MASK)/*FC-local N_PORT*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_FC_LOCAL            (ARAD_PP_FLP_FC_KEY_OR_MASK)/* FC-local */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_FC_REMOTE           (ARAD_PP_FLP_FC_REMOTE_KEY_OR_MASK) /* FC-remote */
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_FC_ZONING           (ARAD_PP_FLP_FC_ZONING_KEY_OR_MASK) /* FC ZONING*/
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IPMC_BIDIR          (ARAD_PP_FLP_IPMC_BIDIR_KEY_OR_MASK) 
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_OMAC_2_VMAC         (ARAD_PP_FLP_OMAC_2_VMAC_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_VMAC                (ARAD_PP_FLP_VMAC_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_VMAC_2_OMAC         (ARAD_PP_FLP_VMAC_2_OMAC_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SLB                 (ARAD_PP_FLP_SLB_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_SINGLE_HOP      (ARAD_PP_FLP_BFD_SINGLE_HOP_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_STATISTICS      (ARAD_PP_FLP_BFD_STATISTICS_KEY_OR_MASK)
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)      (ARAD_PP_FLP_ETH_KEY_OR_MASK(unit))
#define  ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_ECHO      (ARAD_PP_FLP_BFD_ECHO_KEY_OR_MASK)

#define ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_DBAL_DYNAMIC_BASE    ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_BASE
#define ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_DBAL_DYNAMIC_END     ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_END


#define ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP6_SPOOF_STATIC     (ARAD_PP_FLP_IP6_SPOOF_STATIC_KEY_OR_MASK)
#define ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_DIP6_COMPRESSION     (ARAD_PP_FLP_IP6_COMPRESSION_DIP_KEY_OR_MASK)



/* Number of prefix value for application */
#define ARAD_PP_LEM_ACCESS_PREFIX_NUM_1 (1)
#define ARAD_PP_LEM_ACCESS_PREFIX_NUM_4 (4)


typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_LEM_ACCESS_GET_PROCS_PTR = ARAD_PP_PROC_DESC_BASE_LEM_ACCESS_FIRST,
  ARAD_PP_LEM_ACCESS_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LEM_KEY_ENCODED_BUILD,
  ARAD_PP_LEM_KEY_ENCODED_PARSE,
  ARAD_PP_LEM_ACCESS_HW_PAYLOAD_BUILD,
  ARAD_PP_LEM_ACCESS_HW_PAYLOAD_PARSE,
  ARAD_PP_LEM_ACCESS_PARSE,
  ARAD_PP_LEM_REQUEST_SEND,
  ARAD_PP_LEM_REQUEST_ANSWER_RECEIVE,
  ARAD_PP_LEM_ACCESS_ENTRY_ADD_UNSAFE,
  ARAD_PP_LEM_ACCESS_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_LEM_ACCESS_SW_ENTRY_BY_KEY_GET_UNSAFE,
  ARAD_PP_LEM_ACCESS_ENTRY_BY_KEY_GET_UNSAFE,
  ARAD_PP_LEM_ACCESS_ENTRY_BY_INDEX_GET_UNSAFE,
  ARAD_PP_LEM_ACCESS_AGE_FLD_SET,
  ARAD_PP_LEM_ACCESS_AGE_FLD_GET,
  ARAD_PP_LEM_ACCESS_SA_BASED_ASD_BUILD,
  ARAD_PP_LEM_ACCESS_SA_BASED_ASD_PARSE,
  ARAD_PP_LEM_ILM_KEY_BUILD_SET,
  ARAD_PP_LEM_ACCESS_FLP_PROGRAM_MAP_ENTRY_GET,
  ARAD_PP_LEM_ACCESS_TCAM_BANK_BITMAP_SET,
  ARAD_PP_LEM_ACCESS_TCAM_BANK_BITMAP_GET,
  ARAD_PP_LEM_ACCESS_IPV6_BANK_BITMAP_SET,
  ARAD_PP_LEM_ACCESS_IPV6_BANK_BITMAP_GET,
  ARAD_PP_LEM_ACCESS_TCAM_PREFIX_SET,
  ARAD_PP_LEM_ACCESS_TCAM_PREFIX_GET,
  ARAD_PP_LEM_ACCESS_IPV6_TCAM_PREFIX_SET,
  ARAD_PP_LEM_ACCESS_IPV6_TCAM_PREFIX_GET,
  ARAD_PP_LEM_ACCESS_SW_ENTRY_UPDATE_UNSAFE,
  ARAD_PP_LEM_ACCESS_REQUEST_CONVERT_TO_HW,
  ARAD_PP_LEM_ACCESS_CONVERT_KEY_VALUE_INTO_REQUEST_PAYLOAD,


  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LEM_ACCESS_PROCEDURE_DESC_LAST
} ARAD_PP_LEM_ACCESS_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_LEM_ACCESS_KEY_PARAM_SIZE_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_LEM_ACCESS_FIRST,
  ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_UNKNOWN_KEY_PREFIX_ERR,
  ARAD_PP_LEM_ENTRY_INDEX_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_CMD_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_STAMP_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_PREFIX_NOF_BITS_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_PREFIX_PREFIX_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_PARAM_NOF_BITS_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_KEY_PARAM_VALUE_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ASD_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_AGE_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_DEST_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_LOOKUP_POLL_TIMEOUT_ERR,
  ARAD_PP_LEM_ACCCESS_REQ_ORIGIN_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_FAIL_REASON_OUT_OF_RANGE_ERR,
  ARAD_PP_LEM_ACCESS_SA_BASED_ILLEGAL_VID_ERR,
  ARAD_PP_LEM_ACCESS_PROGRAM_NOT_FOUND,
  ARAD_PP_LEM_ACCESS_PREFIX_TOO_LONG,
  ARAD_PP_LEM_ACCESS_MALFORMED_PREFIX,
  ARAD_PP_LEM_ACCESS_KEYS_DONT_MATCH,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_LEM_ACCESS_ERR_LAST
} ARAD_PP_LEM_ACCESS_ERR;

typedef enum
{
  /*
   *  Delete an entry from the LEM DB.
   */
  ARAD_PP_LEM_ACCESS_CMD_DELETE = 0,
  /*
   *  Insert an entry to the DB
   */
  ARAD_PP_LEM_ACCESS_CMD_INSERT = 1,
  /*
   *  Refresh an entry
   */
   ARAD_PP_LEM_ACCESS_CMD_REFRESH = 2,
  /*
   *  Simulate a learned entry
   */
  ARAD_PP_LEM_ACCESS_CMD_LEARN = 3,
  /*
   *  Defrag command (i.e., transfer entries from CAM to LEM DB)
   */
  ARAD_PP_LEM_ACCESS_CMD_DEFRAG = 4,
  /*
   *  ACK to receive an Ack
   */
  ARAD_PP_LEM_ACCESS_CMD_ACK = 5,
  /*
   *  Transplant an entry
   */
  ARAD_PP_LEM_ACCESS_CMD_TRANSPLANT = 6,
  /*
   *  Error on a request
   */
  ARAD_PP_LEM_ACCESS_CMD_ERROR = 7,
  /*
   *  Number of request commands
   */
  ARAD_PP_LEM_ACCESS_NOF_CMDS = 8
}ARAD_PP_LEM_ACCESS_CMD;

typedef enum
{
  /*
   *  CPU
   */
  ARAD_PP_LEM_ACCCESS_REQ_ORIGIN_CPU = 0,
  /*
   *  OLP
   */
  ARAD_PP_LEM_ACCCESS_REQ_ORIGIN_OLP = 1,
  /*
   *  Number of request origins
   */
  ARAD_PP_LEM_ACCCESS_NOF_REQ_ORIGINS = 2
}ARAD_PP_LEM_ACCCESS_REQ_ORIGIN;

typedef enum
{
  /*
   *  MAC address (FID + MAC)
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC, /*= ARAD_PP_FLP_ETH_KEY_OR_MASK,*/
  /*
   *  Backbone MAC (B-FID + B-MAC)
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_BACKBONE_MAC, /*= ARAD_PP_FLP_B_ETH_KEY_OR_MASK,*/
  /*
   *  IPv4 Compatible Multicast
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_MC, /*= 13,*/
  /*
   *  IPv4 Compatible Multicast
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_HOST, /*= ARAD_PP_FLP_IPV4_KEY_OR_MASK,*/
  /*
   *  SA authentication (SA-MAC)
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_SA_AUTH, /*= 5,*/
   /*
   *  ILM
   *  currently: program lookup only for label.
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_ILM, /*= ARAD_PP_FLP_LSR_KEY_OR_MASK,*/
  /*
   *  extended p2p
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_EXTENDED, /*= ARAD_PP_FLP_P2P_KEY_OR_MASK,*/
  /*
   *  TRILL UC
   *  currently: program lookup only nickname.
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_TRILL_UC, /*= ARAD_PP_FLP_TRILL_KEY_OR_MASK,*/
   /*
   *  TRILL MC
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_TRILL_MC, /*= ARAD_PP_FLP_TRILL_KEY_OR_MASK_MC,*/
  /*
   *  IPV4 spoof DHCP
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_SPOOF_DHCP, /*= both ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK*/
  /*
   *  IPV4 spoof static
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_SPOOF_STATIC, /*= both ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK*/
  /*
   *  FCoE: local 
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_FC_LOCAL = ARAD_PP_FLP_FC_KEY_OR_MASK,
  /*
   *  FCoE: local N_PORT
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_FC_LOCAL_N_PORT = ARAD_PP_FLP_FC_N_PORT_KEY_OR_MASK,
  /*
   *  FCoE: zoning 
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_FC_ZONING = ARAD_PP_FLP_FC_ZONING_KEY_OR_MASK,
  /* 
   *  MAC-in-MAC tunnel learn DB
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC_IN_MAC_TUNNEL, /*= ARAD_PP_FLP_MAC_IN_MAC_TUNNEL_KEY_OR_MASK,*/
  /*
   *  FCoE: remote 
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_FC_REMOTE = ARAD_PP_FLP_FC_REMOTE_KEY_OR_MASK,
  /* 
   *  IPMC bidir
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_IPMC_BIDIR = ARAD_PP_FLP_IPMC_BIDIR_KEY_OR_MASK, 
   /*
   *  TRILL adjacent (SA-MAC)
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_TRILL_ADJ,
#ifdef BCM_88660_A0
   /*
   * SLB entry (Is-FEC,destination,Flow-Label). 
   *  
   * The LEM is accessed with the key composed of
   *  
   * |-------- 1b ----------||---- 15b(*) ---||---- 48b ----|
   * |--Is_Destination_Fec--||--Destination--||--Flow Label-| 
   * |------Param[2]--------||----Param[1]---||---Param[0]--| 
   *                                                        0 
   *  
   * Only the Is-Destination-FEC and Destination parameters can be matched against 
   * for value matching. 
   * Additionally the LSB of the destination is ignored for matching. 
   *  
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_SLB = ARAD_PP_FLP_SLB_KEY_OR_MASK,
#endif /* BCM_88660_A0 */
  /*
   *  Global IPv4 Compatible Multicast
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_GLOBAL_IPV4_MC,
  /*
   *  all types use for get block
   */
   ARAD_PP_LEM_ACCESS_ALL_TYPES, /*= 0xc,*/
   ARAD_PP_LEM_ACCESS_KEY_LSR_CUNT = ARAD_PP_FLP_LSR_CNT_KEY_OR_MASK,

  /*
   *  bfd single hop
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_SINGLE_HOP = ARAD_PP_FLP_BFD_SINGLE_HOP_KEY_OR_MASK,
  /*
   * oam_statistics
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_STATISTICS = ARAD_PP_FLP_BFD_STATISTICS_KEY_OR_MASK,
  
  /*bfd echo */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_ECHO = ARAD_PP_FLP_BFD_ECHO_KEY_OR_MASK,


  /* IVL Mode */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN  = ARAD_PP_FLP_ETHERNET_ING_IVL_LEARN_KEY_OR_MASK,
  /*
   *  Spoof compression ipv6 host. 
   *   TT TCAM lookup result x TT SEMB lookup result x SIP[39:0] -> expected LIF
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_IP6_SPOOF_STATIC = ARAD_PP_FLP_IP6_SPOOF_STATIC_KEY_OR_MASK,
  /*
   *  DIP compression . 
   *   TT TCAM lookup result x TT SEMB lookup result x SIP[39:0] -> compression result
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_IP6_COMPRESSION_DIP,


  /*
   *  generic key type used by DBAL for dynamic table creation in the LEM.
   */
  ARAD_PP_LEM_ACCESS_KEY_TYPE_DBAL_BASE = ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_BASE,
  ARAD_PP_LEM_ACCESS_KEY_TYPE_DBAL_END = ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_END,

  /*
   *  Number of key types
   */
   ARAD_PP_LEM_ACCESS_NOF_KEY_TYPES = ARAD_PP_FLP_KEY_OR_MASK_LAST
}ARAD_PP_LEM_ACCESS_KEY_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  Contents (span exactly 4 registers)
   */
  uint32 data[4];
} ARAD_PP_LEM_ACCESS_BUFFER;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

 /*
  *  Value
  */
  uint32 value[ARAD_PP_LEM_KEY_PARAM_MAX_IN_UINT32S_MAX];

 /*
  *	Number of significant bits in the value
  */
  uint32 nof_bits;

} ARAD_PP_LEM_ACCESS_KEY_PARAM;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

 /*
  *  Value
  */
  uint32 value;

 /*
  *	Number of significant bits in the value
  */
  uint32 nof_bits;

} ARAD_PP_LEM_ACCESS_KEY_PREFIX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  Type
   */
   ARAD_PP_LEM_ACCESS_KEY_TYPE type;

  /*
   *	Parameters of the key: at the most 5.
   *  For MAC addresses: param[0] is MAC (48b), param[1] is FID (14b)
   *  For Backbone MAC addresses: param[0] is B-MAC (48b), param[1] is B-FID (12b)
   *  For IPv4 Compatible Multicast addresses: param[0] is DIP (28b), param[1] is FID (14b)
   *  For IP Host addresses: param[0] is DIP (32b), param[1] is VRF (8b)
   *  For SA AUTH addresses: param[0] is SA-MAC (48b)
   *  For ILM: param[0] is DIP (32b), param[1] is VRF (8b)
   *  For SLB: param[0] is Flow-Label (47b), param[1] is Destination, param[2] is Is-Destination-FEC
   */
  ARAD_PP_LEM_ACCESS_KEY_PARAM param[ARAD_PP_LEM_KEY_MAX_NOF_PARAMS];

  /*
   *	Number of effective parameters (an additional verification is done on the nof_bits for each parameter)
   *  For MAC addresses: 2
   *  For Backbone MAC addresses: 2
   *  For IPv4 Compatible Multicast addresses: 2
   *  For IP Host addresses: 2
   *  For SA AUTH: 1
   *  For SLB: 3
   */
  uint8 nof_params;

  /*
   *	Prefix of the key
   *  For MAC addresses: '1'
   *  For Backbone MAC addresses: '000'
   *  For IPv4 Compatible Multicast addresses: '0010'
   *  For IPv6 Compatible Multicast addresses: '0011'
   *  For TP2PS addresses: '0100'
   *  For IPHT addresses: '0101'
   *  For ILMT addresses: '0110'
   *  For Trill (Sabva, Sabeep) addresses: '0111'
   */
  ARAD_PP_LEM_ACCESS_KEY_PREFIX prefix;

} ARAD_PP_LEM_ACCESS_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  Key for the request
   */
   ARAD_PP_LEM_ACCESS_KEY key;

  /*
   *	Request command
   */
  ARAD_PP_LEM_ACCESS_CMD command;

  /*
   *	Request stamp (for the ACK).
   *  Configured internally.
   */
  uint32 stamp;

} ARAD_PP_LEM_ACCESS_REQUEST;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  flags
   *  see ARAD_PP_LEM_ACCESS_xxx
   */
   uint32 flags;
  /*
   *  Destination buffer
   */
   uint32 dest;
  /*
   *  Application specific data
   *  outlif or EEI
   *  In case of format 3b lem payload (used for roo application), eei has the following encoding:
   *  arad+: EEI type[20:23] hi[16:19] 2b'0'  eei[0:13]
   *  jer:   EEI type[20:23] hi[16:19] 1b'0'  eei[0:14]
   *  instead of the usual: EEI type[20:23] hi[16:19] eei[0:15]
   */
  uint32 asd;
  /*
   *  If True, then the entry is marked as dynamic (otherwise static)
   */
  uint8 is_dynamic;
  /*
   *  If True, then the entry is marked as SA-drop
   */
  uint8 sa_drop;
  /*
   *  control word, for MPLS P2P
   */
  uint8 has_cw;
  /*
   *  TPID profile, for MPLS P2P
   */
  uint8 tpid_profile;
  /*
   *  Entry age
   */
  uint32 age;
#ifdef BCM_88660_A0
  /*
   * SLB LAG part of the payload.
   */
  uint32 slb_lag;
  /*
   * SLB FEC part of the payload.
   */
  uint32 slb_fec;
  /*
   * Is SLB LAG valid ?
   */
  uint8 slb_lag_valid;
  /*
   * Is SLB FEC valid ?
   */
  uint8 slb_fec_valid;
  
#endif
  /* 
   * native VSI (or outrif).
   * For ROO applications: Provide native-SA and native vlan for native ethernet
   */ 
  uint32 native_vsi;
  /*
   * Indicate that the payload is to be set as Leraning data in lif table. 
   * In Arad we use 15bit learning. In Jericho we use 17bits. 
   * However, in Jericho lif table Learning info is same as in Arad.
   * 
   */
  uint8 is_learn_data;

} ARAD_PP_LEM_ACCESS_PAYLOAD;

typedef enum
{
  /*
   *  FID limit
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_FID_LIMIT = 0,
  /*
   *  MACT Full
   */
   ARAD_PP_LEM_ACCESS_FAIL_REASON_MACT_FULL,
  /*
   *  CAM Full
   */
   ARAD_PP_LEM_ACCESS_FAIL_REASON_CAM_FULL,
  /*
   *  Delete unknown key
   */
   ARAD_PP_LEM_ACCESS_FAIL_REASON_DELETE_UNKNOWN,
  /*
   *  Wrong stamp - not sure the reply corresponds to the
   *  original request. Not supposed to happen.
   */
   ARAD_PP_LEM_ACCESS_FAIL_REASON_WRONG_STAMP,
  /*
   *  Request not sent due to no open slots for new CPU requests
   *  (the polling failed) - for the lookup, the polling has not finished
   */
   ARAD_PP_LEM_ACCESS_FAIL_REASON_REQUEST_NOT_SENT,
  /*
   *  Unknown FID
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_FID_UNKNOWN,
  /*
   *  Request of type learn for a static entry
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_LEARN_STATIC,
  /*
   *  Change a static entry
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_CHANGE_STATIC,
  /*
   *  EMC problem, should probe further to find the cause
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_EMC_PROBLEM,
  /*
   *  Unknown Reason
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON_UNKNOWN,
 /*
   *  Number of fail reasons
   */
   ARAD_PP_LEM_ACCESS_NOF_FAIL_REASONS
}ARAD_PP_LEM_ACCESS_FAIL_REASON;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  Success for the request operation
   */
   uint8 is_success;

  /*
   *  If failure for 'is_success', describe the reason
   */
  ARAD_PP_LEM_ACCESS_FAIL_REASON reason;

} ARAD_PP_LEM_ACCESS_ACK_STATUS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
   *  The original request
   */
   ARAD_PP_LEM_ACCESS_REQUEST request;

  /*
   * Is part of LAG
   */
  uint8 is_part_of_lag;

  /*
   *  The request stamp
   */
  uint32 stamp;

  /*
   *	The payload
   */
  ARAD_PP_LEM_ACCESS_PAYLOAD payload;

  /*
   *	Origin of the entry insertion: if True, this device, otherwise
   *  another device learned it.
   */
  uint8 is_learned_first_by_me;

  /*
   *	Request origin (OLP or CPU)
   */
  ARAD_PP_LEM_ACCCESS_REQ_ORIGIN req_origin;


} ARAD_PP_LEM_ACCESS_OUTPUT;
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /*
  *	Encoding of the key in bits
  */
  uint32 buffer[SOC_DPP_DEFS_MAX_LEM_WIDTH_IN_UINT32S];

} ARAD_PP_LEM_ACCESS_KEY_ENCODED;


typedef struct
{
  uint32 or_value;
  uint32 and_value;
  uint32 valid;
}ARAD_PP_LEM_ACCESS_MASK_INFO;


/**
 * Struct for filling bfd one hop LEM entry
 */
typedef struct
{
    SOC_SAND_MAGIC_NUM_VAR
    int is_ipv6;
    uint32 local_discriminator;
    uint16 oam_id;
    int is_accelerated;
    uint32 trap_code;
    uint8 fwd_strngth;
    uint8 snp_strength;
    uint8 remote_detect_mult; /*optional*/
    uint8 is_update;
}ARAD_PP_LEM_BFD_ONE_HOP_ENTRY_INFO;



/* } */
/*************
 * GLOBALS   *
 *************/
/* { */
extern uint32
  Arad_pp_lem_actual_stamp[SOC_SAND_MAX_DEVICE] ;

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  arad_pp_lem_access_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

  const char *
    arad_pp_lem_access_app_id_to_app_name(
                                                         SOC_SAND_IN  int    unit,
                                                         SOC_SAND_IN  uint32 app_id);

uint32
    arad_pp_lem_access_prefix_to_app_get(
       SOC_SAND_IN  int                        unit,
       SOC_SAND_IN  uint32                        key_type_msb,
       SOC_SAND_OUT  uint32                        *app_type);

uint32
    arad_pp_lem_access_prefix_dealloc(
       SOC_SAND_IN  int                        unit,
       SOC_SAND_IN  uint32                     key_type_msb);

uint32
    arad_pp_lem_access_app_to_prefix_get(
       SOC_SAND_IN  int                        unit,
       SOC_SAND_IN  uint32                        app_id,
       SOC_SAND_OUT  uint32                        *prefix);

/*
 *	Conversion functions for key construction / parsing
 */
uint32
  arad_pp_lem_key_encoded_parse(
     SOC_SAND_IN  int                        unit,
     SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY_ENCODED     *key_in_buffer,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY             *key
  );

uint32
  arad_pp_lem_key_encoded_parse_arad_format(
     SOC_SAND_IN  int                        unit,
     SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY_ENCODED     *key_in_buffer,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY             *key
  );
uint32
  arad_pp_lem_key_encoded_build(
     SOC_SAND_IN  int                            unit,
     SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY             *key,
     SOC_SAND_IN  uint32                             is_mask,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY_ENCODED     *key_in_buffer
  );

uint32
  arad_pp_lem_request_pack(
      SOC_SAND_IN  int                        unit,
      SOC_SAND_IN  ARAD_PP_LEM_ACCESS_OUTPUT    *request_all,
      SOC_SAND_OUT uint32                       *data,
      SOC_SAND_OUT uint8                        *data_len
    );

uint32
  arad_pp_lem_access_parse_only(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN uint32                         flags,
      SOC_SAND_IN  uint32                           *event_buff,
      SOC_SAND_IN  uint32                            event_buff_len,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_OUTPUT        *request_all,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS    *ack_status
    );

uint32
  arad_pp_lem_access_parse(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN uint32                         flags,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_OUTPUT     *request_all,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS *ack_status
  );

uint32
    arad_pp_lem_access_prefix_alloc(
       SOC_SAND_IN  int                        unit,
       SOC_SAND_IN  uint32                        flags,
       SOC_SAND_IN  uint32                        app_id,
       SOC_SAND_IN  uint32                        key_type_msb_nof,
       SOC_SAND_INOUT  uint32                     *key_type_msb
    );

uint32
  arad_pp_lem_access_payload_build(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD   *payload,
    SOC_SAND_OUT uint32                    payload_data[ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S]
  );

uint32
  arad_pp_lem_access_next_stamp_get(
      SOC_SAND_IN int  unit,
      SOC_SAND_OUT uint32 *next_stamp
    );

uint32
  arad_pp_lem_access_payload_parse(
      SOC_SAND_IN   int                   unit,      
      SOC_SAND_IN   uint32                    payload_data[ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S],
      SOC_SAND_IN   ARAD_PP_LEM_ACCESS_KEY_TYPE key_type,
      SOC_SAND_OUT  ARAD_PP_LEM_ACCESS_PAYLOAD   *payload      
   );

/*********************************************************************
 *     Add an entry to the Exact match table.
 *********************************************************************/
uint32
  arad_pp_lem_access_entry_add_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_REQUEST         *request,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD         *payload,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS      *ack_status
  );

/*********************************************************************
 *     Remove an entry to the Exact match table.
 *********************************************************************/
uint32
  arad_pp_lem_access_entry_remove_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_REQUEST         *request,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS      *ack_status
  );

/*********************************************************************
 *     Get an entry in the Exact match table according to its key
 *********************************************************************/
uint32
  arad_pp_lem_access_entry_by_key_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY             *key,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD         *payload,
    SOC_SAND_OUT uint8                        *is_found
  );

/*********************************************************************
 *     Get an entry in the Exact match table according to its index
 *********************************************************************/
uint32
  arad_pp_lem_access_entry_by_index_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                        entry_ndx,
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY             *key,
    SOC_SAND_OUT uint8                        *is_valid
  );

/*********************************************************************
 *     parse/build ASD field for SA-Atuh usages
 *********************************************************************/
uint32
  arad_pp_lem_access_sa_based_asd_build(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_LLP_SA_AUTH_MAC_INFO      *auth_info,
    SOC_SAND_IN  SOC_PPC_LLP_VID_ASSIGN_MAC_INFO   *vid_assign_info,
    SOC_SAND_OUT uint32                        *asd
  );


/*********************************************************************
 *     parse/build ASD field for Trill-SA-Atuh usages
 *********************************************************************/
uint32
  arad_pp_lem_access_trill_sa_based_asd_build(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO              *auth_info,
    SOC_SAND_OUT uint32                          *asd
  );


/* assumed key already cleared */
uint32
  arad_pp_lem_access_sa_based_asd_parse(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                        asd,
    SOC_SAND_OUT SOC_PPC_LLP_SA_AUTH_MAC_INFO      *auth_info,
    SOC_SAND_OUT SOC_PPC_LLP_VID_ASSIGN_MAC_INFO   *vid_assign_info
  );

uint32
  arad_pp_lem_access_trill_sa_based_asd_parse(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                        asd,
    SOC_SAND_OUT SOC_PPC_TRILL_ADJ_INFO            *auth_info
  );

uint32
  arad_pp_lem_trill_mc_key_build_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  uint8           mask_adjacent_nickname,
    SOC_SAND_IN  uint8           mask_fid,
    SOC_SAND_IN  uint8           mask_adjacent_eep
  );
uint32
  arad_pp_lem_trill_mc_key_build_get(
    SOC_SAND_IN   int           unit,
    SOC_SAND_OUT  uint8           *mask_adjacent_nickname,
    SOC_SAND_OUT  uint8           *mask_fid,
    SOC_SAND_OUT  uint8           *mask_adjacent_eep
  );


int 
  arad_pp_lem_access_bfd_one_hop_lem_entry_add(
    int unit,
    const ARAD_PP_LEM_BFD_ONE_HOP_ENTRY_INFO *
  );

int 
  arad_pp_lem_access_bfd_one_hop_lem_entry_remove(
    int unit,
    uint32 local_discriminator
  );

/*
 *	Clear functions
 */
void
  ARAD_PP_LEM_ACCESS_KEY_ENCODED_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY_ENCODED *info
  );

void
  ARAD_PP_LEM_ACCESS_KEY_PARAM_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY_PARAM *info
  );

void
  ARAD_PP_LEM_ACCESS_KEY_PREFIX_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY_PREFIX *info
  );

void
  ARAD_PP_LEM_ACCESS_KEY_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY *info
  );

void
  ARAD_PP_LEM_ACCESS_REQUEST_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_REQUEST *info
  );

void
  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD *info
  );

void
  ARAD_PP_LEM_ACCESS_OUTPUT_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_OUTPUT  *info
  );

void
  ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_ACK_STATUS  *info
  );

void
  ARAD_PP_LEM_ACCESS_BUFFER_clear(
    SOC_SAND_OUT ARAD_PP_LEM_ACCESS_BUFFER *info
  );

void
  ARAD_PP_LEM_BFD_ONE_HOP_ENTRY_INFO_clear(
    SOC_SAND_OUT ARAD_PP_LEM_BFD_ONE_HOP_ENTRY_INFO *info
  );



uint32
  ARAD_PP_LEM_ACCESS_KEY_PARAM_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY_TYPE   type,
    SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY_PARAM *info,
    SOC_SAND_IN uint32                   params_ndx
  );

uint32
  ARAD_PP_LEM_ACCESS_KEY_PREFIX_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY_TYPE type,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY_PREFIX *info
  );

uint32
  ARAD_PP_LEM_ACCESS_KEY_verify(
     SOC_SAND_IN  int                 unit,
     SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY *info
  );

uint32
  ARAD_PP_LEM_ACCESS_REQUEST_verify(
     SOC_SAND_IN  int                 unit,
     SOC_SAND_IN  ARAD_PP_LEM_ACCESS_REQUEST *info
  );

uint32
  ARAD_PP_LEM_ACCESS_PAYLOAD_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*                                        
  ARAD_PP_LEM_ACCESS_KEY_TYPE_to_string(              
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY_TYPE enum_val
  );

void
  ARAD_PP_LEM_ACCESS_KEY_print(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY *info
  );

void
  ARAD_PP_LEM_ACCESS_PAYLOAD_print(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD *info
  );

#endif



/*********************************************************************
* NAME:
 *   arad_pp_lem_access_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_lem_access module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_lem_access_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_lem_access_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_lem_access module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_lem_access_get_errs_ptr(void);

uint32
  arad_pp_lem_access_entry_type_format3b_get(
      SOC_SAND_IN   int                       unit, 
      SOC_SAND_IN   uint32                    payload_data[ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S],
      SOC_SAND_OUT  uint32                    *entry_type
      ) ;

uint32
arad_pp_lem_access_is_dynamic_get(
   SOC_SAND_IN   int                         unit, 
   SOC_SAND_IN   uint32                      payload_data[ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S],
   SOC_SAND_IN  uint32                       entry_type,
   SOC_SAND_OUT  uint8                       *is_dynamic
   ) ;

uint32 
arad_pp_lem_access_is_dynamic_set(
   SOC_SAND_IN   int                         unit, 
   SOC_SAND_IN  uint32                       entry_type,
   SOC_SAND_IN  uint8                        is_dynamic,
   SOC_SAND_OUT uint32                       payload_data[ARAD_PP_LEM_ACCESS_PAYLOAD_NOF_UINT32S]);

uint32
  arad_pp_lem_access_entry_type_get(
      SOC_SAND_IN   int                       unit, 
      SOC_SAND_IN   uint32                    *payload_data,
      SOC_SAND_IN   uint8                     is_learn_data,
      SOC_SAND_OUT  uint32                    *entry_type
    );

uint32
  arad_pp_lem_block_get(
     SOC_SAND_IN  int                                           unit,
     SOC_SAND_IN ARAD_PP_IHP_MACT_FLUSH_DB_TBL_DATA             *tbl_data,
     SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY                        *mact_keys,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                    *mact_vals,
     SOC_SAND_OUT uint32                                        *nof_entries
    );

uint32
arad_pp_lem_convert_key_value_into_request_payload(
            SOC_SAND_IN  int                                         unit,
            SOC_SAND_IN  int                                         insert,
            SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ADD_TYPE                 add_type,
            SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_KEY                *mac_entry_key,
            SOC_SAND_IN  SOC_PPC_FRWRD_MACT_ENTRY_VALUE              *mac_entry_value,
            SOC_SAND_OUT ARAD_PP_LEM_ACCESS_REQUEST                  *request,
            SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                  *payload
);

uint32
arad_pp_lem_sw_entry_update_unsafe(
          SOC_SAND_IN  int                        unit,
          SOC_SAND_IN ARAD_PP_LEM_ACCESS_REQUEST  *request,
          SOC_SAND_IN ARAD_PP_LEM_ACCESS_PAYLOAD  *payload,
          SOC_SAND_OUT uint8                      *success
  );

uint32
arad_pp_lem_access_get_entry_type_from_key_unsafe(
          SOC_SAND_IN  int                        unit,
          SOC_SAND_IN  ARAD_PP_LEM_ACCESS_KEY     *key
  );
/* } */

#ifdef PLISIM

uint32
  arad_pp_lem_block_get(
     SOC_SAND_IN  int                                           unit,
     SOC_SAND_IN ARAD_PP_IHP_MACT_FLUSH_DB_TBL_DATA             *tbl_data,
     SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY                        *mact_keys,
     SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                    *mact_vals,
     SOC_SAND_OUT uint32                                        *nof_entries
    );
#endif
#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_LEM_ACCESS_INCLUDED__*/
#endif


