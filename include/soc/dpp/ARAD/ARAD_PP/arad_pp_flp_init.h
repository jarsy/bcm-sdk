

/* $Id: arad_pp_flp_init.h,v 1.59 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_PP_FLP_INIT_INCLUDED__
/* { */
#define __ARAD_PP_FLP_INIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ce_instruction.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_trap_mgmt.h>
#include <soc/dpp/PPC/ppc_api_fp.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* KBR Defines */


/* 
The instruction formulas are(same for Arad and Jericho):
16 Bit Instruction:
   ce_value = (buffer_size - offset - 16)/4
32 Bit Instruction:
   ce_value = (buffer_size - offset - 32)/8
 
Offset - the offset in the buffer. 
 
Buffer_size:
Arad:    
1024 
 
Jericho(two buffers(LSB and MSB)):
LSB 1024
MSB 360 
 
16bit field inst =  8 + (ce_value *16) + (nof_bits-1)*4096 
32bit field inst =  8 + (ce_value *16) + (nof_bits-1)*2048 
*/
 



/* Jericho only instructions */
#define ARAD_PP_FLP_32B_INST_P6_GLOBAL_IN_LIF_D                                                          (0x8A28) 


/* Arad only instructions no implementation for Jericho for following instructions */
#define ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D                                                   (0x2af8) 
#define ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D                                                     (0xf118)
#define ARAD_PP_FLP_16B_INST_P6_IN_PORT_D                                                                (0x7ec8)


#define ARAD_PP_FLP_16B_INST_P6_TT_LOOKUP0_PAYLOAD_D                                      ((SOC_IS_JERICHO(unit) || SOC_IS_QAX(unit))? 0xfcac: 0xf068)
#define ARAD_PP_FLP_16B_INST_P6_IN_LIF_D                                                  ((SOC_IS_JERICHO(unit) || SOC_IS_QAX(unit))? 0xfaac: 0xf688)


/* Generic instractions in build n zeros, ones using 16 bits instruction */
#define ARAD_PP_FLP_16B_INST_N_ZEROS(_n)                                                             (((_n-1) << 12) | (SOC_IS_JERICHO(unit)? 0x0fc8: 0x0fc8))
#define ARAD_PP_FLP_16B_INST_8_ZEROS                                                                 (ARAD_PP_FLP_16B_INST_N_ZEROS(8))
#define ARAD_PP_FLP_16B_INST_12_ZEROS                                                                (ARAD_PP_FLP_16B_INST_N_ZEROS(12))
#define ARAD_PP_FLP_16B_INST_15_ZEROS                                                                (ARAD_PP_FLP_16B_INST_N_ZEROS(15))

#define ARAD_PP_FLP_32B_INST_N_ZEROS(_n)                                                             (((_n-1) << 11) | (SOC_IS_JERICHO(unit)? 0x07c8: 0x07c8))

#define ARAD_PP_FLP_16B_INST_0_PROGRAM_KEY_GEN_VAR(_n)                                               (((_n-1) << 12)|(SOC_IS_JERICHO(unit)? 0x0248: 0x04e8))
#define ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR(_n)                                              (((_n-1) << 12)|(SOC_IS_JERICHO(unit)? 0x0e88: 0x0e88))
#define ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_13_BITS                                        (ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR(13))
#define ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_3_BITS                                         (ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR(3))
#define ARAD_PP_FLP_16B_INST_P6_VSI(_n)                                                              (((_n-1) << 12)|(SOC_IS_JERICHO(unit)? 0x08c8: 0x0a18))

#define ARAD_PP_FLP_16B_INST_P6_TT_LOOKUP0_PAYLOAD_D_BITS(_n)                                        (((_n-1) << 12)|(SOC_IS_JERICHO(unit)? 0x0cac : 0x0068))


/* FCoe*/

#define ARAD_PP_FLP_VFT_NOF_BITS (13)

 /* FlpKeyConstruction_fcf*/
#define ARAD_PP_FLP_32B_INST_ARAD_FC_D_ID                   arad_pp_ce_instruction_composer(24,2,0,ARAD_PP_CE_IS_CE32);
#define ARAD_PP_FLP_32B_INST_ARAD_FC_S_ID                   arad_pp_ce_instruction_composer(24,2,4*8,ARAD_PP_CE_IS_CE32);

/* FlpKeyConstruction_fcf with vft*/
/* VFT (priority + id+ reserved R_CTRL+Virtual_fabric */
#define ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_D_ID          arad_pp_ce_instruction_composer(24,3,0,ARAD_PP_CE_IS_CE32);
#define ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_S_ID          arad_pp_ce_instruction_composer(24,3,4*8,ARAD_PP_CE_IS_CE32);


/* FlpKeyConstruction_fcf remote*/
#define ARAD_PP_FLP_16B_INST_ARAD_FC_D_ID_8_MSB             arad_pp_ce_instruction_composer(8,2,0,ARAD_PP_CE_IS_CE16);
#define ARAD_PP_FLP_16B_INST_ARAD_FC_S_ID_8_MSB             arad_pp_ce_instruction_composer(8,2,4*8,ARAD_PP_CE_IS_CE16);


/* FlpKeyConstruction_fcf with vft remote*/
 /* VFT (priority + id+ reserved R_CTRL+Virtual_fabric  */
#define ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_D_ID_8_MSB   arad_pp_ce_instruction_composer(8,3,0,ARAD_PP_CE_IS_CE16);
#define ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_S_ID_8_MSB   arad_pp_ce_instruction_composer(8,3,4*8,ARAD_PP_CE_IS_CE16);


/* VFT using 16b instruction */
#define ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID        arad_pp_ce_instruction_composer(ARAD_PP_FLP_VFT_NOF_BITS,2,16,ARAD_PP_CE_IS_CE16);








#define ARAD_PP_FLP_ETH_KEY_OR_MASK(unit)   ((SOC_IS_ARAD_B0_AND_ABOVE(unit)) ? 0x0 : 0x1)
#define ARAD_PP_FLP_DUMMY_KEY_OR_MASK(unit)   ((SOC_IS_ARAD_B0_AND_ABOVE(unit)) ? 0x1 : 0x0)
/* prefix is 4 b', but only 3 MSB are written to tbl, so must be of type XXX0 (even) */
#define ARAD_PP_FLP_B_ETH_KEY_OR_MASK(unit) ((SOC_IS_ARAD_B0_AND_ABOVE(unit)) ? ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SA_AUTH : 0x0)
#define ARAD_PP_FLP_IPV4_KEY_OR_MASK                0x2
#define ARAD_PP_FLP_LSR_KEY_OR_MASK                 0x3
#define ARAD_PP_FLP_P2P_KEY_OR_MASK                 0x4
#define ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK       0x8 /* cover 8-15 */
#define ARAD_PP_FLP_IPV4_COMP_KEY_OR_MASK           0x6
#define ARAD_PP_FLP_MAC_IN_MAC_TUNNEL_KEY_OR_MASK   0x7
#define ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK   0x5 
#define ARAD_PP_FLP_TRILL_KEY_OR_MASK               0xa
#define ARAD_PP_FLP_TRILL_KEY_OR_MASK_MC            0xb

/* HW value to lookup the default key FID-FWD MAC instead of building a key */
#define ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL  (SOC_IS_JERICHO(unit)? 0x7: 0x3)

/* As of this KEY_OR_MASK, the values are app_ids and NOT prefixes. The prefixes are allocated dynamically on demand. */
#define ARAD_PP_FLP_FC_KEY_OR_MASK                  0xc
#define ARAD_PP_FLP_FC_ZONING_KEY_OR_MASK           0xd
#define ARAD_PP_FLP_FC_REMOTE_KEY_OR_MASK           0xf
#define ARAD_PP_FLP_IPMC_BIDIR_KEY_OR_MASK          0x10
#define ARAD_PP_FLP_OMAC_2_VMAC_KEY_OR_MASK         0x11
#define ARAD_PP_FLP_VMAC_KEY_OR_MASK                0x12
#define ARAD_PP_FLP_VMAC_2_OMAC_KEY_OR_MASK         0x13
#define ARAD_PP_FLP_TRILL_ADJ_KEY_OR_MASK           0x14
#define ARAD_PP_FLP_SLB_KEY_OR_MASK                 0x15
#define ARAD_PP_FLP_GLOBAL_IPV4_KEY_OR_MASK         0x18 /* prefix value will be cover 12-15 */
#define ARAD_PP_FLP_FC_N_PORT_KEY_OR_MASK           0x19
#define ARAD_PP_FLP_BFD_SINGLE_HOP_KEY_OR_MASK		0x20
#define ARAD_PP_FLP_IP6_SPOOF_STATIC_KEY_OR_MASK           0x22
#define ARAD_PP_FLP_IP6_COMPRESSION_DIP_KEY_OR_MASK        0x23
#define ARAD_PP_FLP_BFD_STATISTICS_KEY_OR_MASK		0x24
#define ARAD_PP_FLP_ETHERNET_ING_IVL_LEARN_KEY_OR_MASK          0x28


#define ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_BASE               0x29
#define ARAD_PP_FLP_BFD_ECHO_KEY_OR_MASK                        0x30
#define ARAD_PP_FLP_LSR_CNT_KEY_OR_MASK            0x1b  /* for lsr counting*/

#define ARAD_PP_FLP_DYNAMIC_DBAL_KEY_OR_MASK_END                0x5b
#define ARAD_PP_FLP_KEY_OR_MASK_LAST                0x5c /* need to update this value when adding new define */




/* FLP port profiles */
#define ARAD_PP_FLP_PORT_PROFILE_DEFAULT      (0)
#define ARAD_PP_FLP_PORT_PROFILE_FC_N_PORT     (1)
#define ARAD_PP_FLP_PORT_PROFILE_PBP          (2)/* mac-in-mac*/

/* Number of FLP instructions */
#define ARAD_PP_FLP_INSTRUCTIONS_NOF (SOC_DPP_IMP_DEFS_GET(unit, flp_instructions_nof))

#define ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP     (1)
#define ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV     (2)
#define ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_ALL     (0)
#define ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE    (SOC_IS_JERICHO(unit)? 0x1F: 0x3)

/* KBP defines } */
#define ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS           	120
#define ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_BIT_IN_BUFFER      		728
#define ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS       	128
#define ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_BIT_IN_BUFFER      	856
#define ARAD_PP_FLP_KBP_MAX_RESULTS_LENGTH_IN_32b_ALIGNMENT		((ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS+31)/32)
#define ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS                  	6
#define ARAD_PP_FLP_KBP_ROP_HEADERS_SIZE_IN_BITS 	            24


#define PROG_FLP_TM                                 0x00
#define PROG_FLP_ETHERNET_ING_LEARN                 0x01
#define PROG_FLP_IPV4UC_PUBLIC                      0x02
#define PROG_FLP_IPV4UC_RPF                         0x03
#define PROG_FLP_IPV4UC                             0x04
#define PROG_FLP_IPV6UC                             0x05
#define PROG_FLP_P2P                                0x06
#define PROG_FLP_IPV6MC                             0x07
#define PROG_FLP_LSR                                0x08
#define PROG_FLP_TRILL_UC                           0x0a /* Trill */
#define PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC     0x0a /* PON */
#define PROG_FLP_TRILL_MC_ONE_TAG                   0x0b /* Trill */
#define PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC     0x0b /* PON */
#define PROG_FLP_TRILL_MC_TWO_TAGS                  0x0c /* Trill */
#define PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM       0x0c
#define PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP       0x0d /* PON */
#define PROG_FLP_VPWS_TAGGED_SINGLE_TAG             0x0d /* cannot live with pon */
#define PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP       0x0e /* PON */
#define PROG_FLP_VPWS_TAGGED_DOUBLE_TAG             0x0e /* cannot live with pon */
#define PROG_FLP_ETHERNET_MAC_IN_MAC                0x0f
#define PROG_FLP_IP4UC_CUSTOM_ROUTE                 0x10 /* Jericho B0 prgrm */
#define PROG_FLP_TRILL_AFTER_TERMINATION            0x10 /* Trill, arad+ and below prgrm */
#define PROG_FLP_IPV4MC_BRIDGE                      0x11 /* bridging */
#define PROG_FLP_IPV4COMPMC_WITH_RPF                0x12 /* routing */
#define PROG_FLP_IPV4_DC                            0x13
#define PROG_FLP_BIDIR                              0x19
#define PROG_FLP_IPV6UC_RPF                         0x20
#define PROG_FLP_VPLSOGRE                           0x22
#define PROG_FLP_VMAC_UPSTREAM                      0x23 /* VMAC */
#define PROG_FLP_VMAC_DOWNSTREAM                    0x24 /* VMAC */
#define PROG_FLP_ETHERNET_PON_LOCAL_ROUTE           0x25 /* PON local route enabled */
#define PROG_FLP_ETHERNET_PON_DEFAULT_UPSTREAM      0x26 /* PON local route disabled upstream */
#define PROG_FLP_ETHERNET_PON_DEFAULT_DOWNSTREAM    0x27 /* PON local route disabled downstream */

#define PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF         0x2a /* global routing */
#define PROG_FLP_PON_ARP_DOWNSTREAM                 0x2b
#define PROG_FLP_PON_ARP_UPSTREAM                   0x2c
#define PROG_FLP_FC_TRANSIT                         0x2d /* for FC bridging similar to ethernet, but needed to enable FC-traps, learning should be disabled for these packets*/
#define PROG_FLP_IPV6UC_WITH_RPF_2PASS              0x2e
#define PROG_FLP_BFD_IPV4_SINGLE_HOP                0x2f
#define PROG_FLP_BFD_IPV6_SINGLE_HOP                0x30
#define PROG_FLP_OAM_STATISTICS                     0x31
#define PROG_FLP_BFD_STATISTICS                     0x32
#define PROG_FLP_OAM_DOWN_UNTAGGED_STATISTICS       0x33
#define PROG_FLP_OAM_SINGLE_TAG_STATISTICS          0x34
#define PROG_FLP_OAM_DOUBLE_TAG_STATISTICS          0x35
#define PROG_FLP_BFD_MPLS_STATISTICS                0x36
#define PROG_FLP_BFD_PWE_STATISTICS                 0x37
#define PROG_FLP_ETHERNET_ING_IVL_LEARN             0x38
#define PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN       0x39
#define PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN   0x3a
#define PROG_FLP_IPV4UC_PUBLIC_RPF                  0x3b
#define PROG_FLP_FC_REMOTE                          0x3c /* FCoE*/
#define PROG_FLP_FC_WITH_VFT_REMOTE                 0x3d /* FCoE*/
#define PROG_FLP_FC                                 0x3e /* FCoE*/
#define PROG_FLP_FC_WITH_VFT                        0x3f /* FCoE*/
#define PROG_FLP_FC_N_PORT                          0x40 /* FCoE*/
#define PROG_FLP_FC_WITH_VFT_N_PORT                 0x41 /* FCoE*/
#define PROG_FLP_IPV6UC_PUBLIC                      0x42
#define PROG_FLP_IPV6UC_PUBLIC_RPF                  0x43

#define PROG_FLP_LAST                               (PROG_FLP_IPV6UC_PUBLIC_RPF)

#define ARAD_PP_FLP_MAP_PROG_NOT_SET                (0xff)

/* total programs for FCoE are 7, one for trasit switch, 4 fcf and 2 for fcf n_port the last 2 are created only when NPV switch is enabled */
#define ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS                 4

#define ARAD_PP_FLP_DEFAULT_FWD_RES_SIZE            48
#define ARAD_PP_FLP_DEFAULT_RPF_RES_SIZE            24

/* Define an FLP Profile that is uses a signaling between FLP Programs and PMF
 * used for the FLP & PMF for */
#define ARAD_PP_FLP_PROGRAM_FWD_PROCESS_PROFILE_REPLACE_FWD_CODE    (0x01)


/* HW value to use Key-C for TCAM lookup */
#define ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL  (SOC_IS_JERICHO(unit)? 0x2: 0x1)

/* macro for supported programs in IPv6 DIP SIP sharing feature */
#define ARAD_PP_FLP_IPV6_DIP_SIP_SHARING_IS_PROG_VALID(opcode) ( \
    opcode == PROG_FLP_IPV6UC || \
    opcode == PROG_FLP_IPV6UC_RPF || \
    opcode == PROG_FLP_IPV6MC || \
    opcode == PROG_FLP_IPV6UC_PUBLIC || \
    opcode == PROG_FLP_IPV6UC_PUBLIC_RPF \
) \

/*************
 * ENUMERATIONS *
 *************/

/*
 * This enumeration is for the TCAM use modes for L3 MC entries (Jericho and above)
 */
typedef enum
{
   /*
    * Don't use the TCAM for MC entries (use KAPS/KBP).
    */
    ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE = 0,
   /*
    * Use the TCAM for IPV4/IPV6 MC entries .
    */
    ARAD_PP_FLP_L3_MC_USE_TCAM_ENABLE = 1,
    /*
     * Use the TCAM for IPV4/IPV6 MC entries But don't use VRF as a qualifier in
     * the IPV4 MC entries (in this case each entry is less than 80 bits doubling
     * the number of entries possible in the TCAM with the VRF field) .
     */
    ARAD_PP_FLP_L3_MC_USE_TCAM_NO_IPV4_VRF = 2
} ARAD_PP_FLP_L3_MC_USE_TCAM;


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

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

uint32
    arad_pp_flp_elk_prog_config_max_key_size_get(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  uint32  prog_id,
	   SOC_SAND_IN  uint32  zone_id,
       SOC_SAND_OUT uint32   *max_key_size_in_bits
    );

uint32
    arad_pp_flp_elk_prog_config(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  uint32  prog_id,
       SOC_SAND_IN  uint32  opcode,
       SOC_SAND_IN  uint32  key_size_msb,
       SOC_SAND_IN  uint32  key_size
    );

#endif

/* For TM Init */
uint32
   arad_pp_flp_prog_sel_cam_key_program_tm(
     int unit
   );
uint32
   arad_pp_flp_process_key_program_tm(
     int unit
   );

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_elk_result_configure(int unit, int prog_id, int fwd_res_size,  int fwd_res_found_bit_offset, int fec_size, 
                                    int fec_res_data_start, int fec_res_found_bit_offset );
#endif 

/* initialize FLP programs */
uint32
  arad_pp_flp_init(
     int unit,
     uint8 ingress_learn_enable, /* = 1*/
     uint8 ingress_learn_oppurtunistic, /* = 0 */
     uint32  sa_lookup_type /*hex 2'b10*/
   );

/* update ethernet program to perform ingress/egress learning */
uint32
  arad_pp_flp_ethernet_prog_update(
     int unit,
     uint8 learn_enable,
     uint8 ingress_learn_enable, /* = 1*/
     uint8 ingress_learn_oppurtunistic, /* = 0 */
     uint32  sa_lookup_type /*hex 2'b10*/
   );

uint32
  arad_pp_flp_ethernet_prog_learn_get(
     int unit,
     uint8 *learn_enable /* = 1*/
   );


/* update trap configuration */
uint32
  arad_pp_flp_trap_config_update(
     int unit,
     SOC_PPC_TRAP_CODE_INTERNAL trap_code_internal, 
     int trap_strength,  /* -1 for don't update */
     int snoop_strength/* -1 for don't update */
   );

uint32
  arad_pp_flp_trap_config_get(
     int unit,
     SOC_PPC_TRAP_CODE_INTERNAL trap_code_internal, 
     uint32 *trap_strength, /* null for ignore */
     uint32  *snoop_strength/* null for ignore */
   );

/* set learning for FLP programs */
uint32
    arad_pp_flp_prog_learn_set(
       int unit,
       int32  fwd_code,
       uint8  learn_enable
    );

/* get learning for FLP programs */
uint32
  arad_pp_flp_prog_learn_get(
     int unit,
     int32  fwd_code,
     uint8  *learn_enable
  );

/* 
 *  for IPMC programs wether to flood unknown packets,
 *  or to forward accroding to VRF-default destination
 */ 
uint32
   arad_pp_ipmc_not_found_proc_update(
     int unit,
     uint8  flood
   );

uint32
   arad_pp_ipmc_not_found_proc_get(
     int unit,
     uint8  *flood
   );


uint32
   arad_pp_ipv4mc_bridge_lookup_update(
     int unit,
     uint8  mode /* 0:<FID,DA>, 1:<FID,DIP>*/
   );

uint32
   arad_pp_ipv4mc_bridge_lookup_get(
     int unit,
     uint8  *mode
   );


/* Update LSR key construction */
uint32
   arad_pp_flp_key_const_lsr(
     int unit,
     uint8  in_port,
     uint8  in_rif,
     uint8  in_exp
   );

uint32
   arad_pp_flp_key_const_pwe_gre(
     int unit,     
     uint8  in_rif,
     uint8  in_exp
   );

uint32
   arad_pp_flp_lookups_tcam_profile_set(
     int unit,
     uint32 tcam_access_profile_ndx,
     uint32 tcam_access_profile_id,
     uint32 prog_id
   );


uint32
    arad_pp_flp_lookups_TRILL_mc_update(int unit,
     uint32 is_ingress_learn);

/* Update TRILL MC/IPV4MC / IPV6UC / IPV6MC according to usage of TCAM. */
uint32
   arad_pp_flp_lookups_TRILL_mc(
     int unit,
     uint32 is_ingress_learn,
     uint32 tcam_access_profile_id
   );
   
uint32
   arad_pp_flp_lookups_ipv4mc_with_rpf(
     int unit,
     uint8 ingress_learn_enable, /* = 1,*/
     uint8 ingress_learn_oppurtunistic /* = 0*/
   );


uint32
   arad_pp_flp_lookups_ipv4compmc_with_rpf(
     int unit,
     uint32 tcam_access_profile_id
   );

uint32
   arad_pp_flp_lookups_global_ipv4compmc_with_rpf(
     int    unit,
     int prog_id,
     uint32 tcam_access_profile_id
   );

uint32
   arad_pp_flp_lookups_ipv6uc(
     int unit,
     uint32 tcam_access_profile_id
   );

uint32
   arad_pp_flp_lookups_ipv6mc(
     int unit,
     uint32 tcam_access_profile_id
   );

uint32
   arad_pp_flp_lookups_oam(
     int unit,
     uint32 tcam_access_profile_id_0,
     uint32 tcam_access_profile_id_1,
     uint32 flp_key_program
   );

uint32
   arad_pp_flp_lookups_ethernet_tk_epon_uni_v6(
     int unit,
     uint32 tcam_access_profile_id,
     uint8 ingress_learn_enable,
     uint8 ingress_learn_oppurtunistic
   );

uint32
   arad_pp_flp_lookups_ethernet_pon_default_downstream(
     int unit,
     uint32 tcam_access_profile_id,
     uint8 ingress_learn_enable, /* = 1,*/
     uint8 ingress_learn_oppurtunistic, /* = 0*/
     int32 prog_id
   );

/* Get ingress learn mode of anti-spoofing V6*/
uint32
   arad_pp_flp_tk_epon_uni_v6_ing_learn_get(
     int unit,
     uint8 *ingress_learn_enable,
     uint8 *ingress_learn_oppurtunistic
   );


char*
  arad_pp_flp_prog_id_to_prog_name(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint8 prog_id
);

uint32
  arad_pp_flp_access_print_all_programs_data(
    SOC_SAND_IN  int unit
  );

/* Retrieve the FLP-Program according to the application type */
uint32 arad_pp_flp_app_to_prog_index_get(
   int unit,
   uint32 app_id,
   uint8  *prog_index
);

uint32 arad_pp_prog_index_to_flp_app_get(
   int unit,
   uint32 prog_index,
   uint8  *app_id
);

uint32
arad_pp_flp_fcoe_zoning_set(
     int unit,
     int enable
   );

uint32
arad_pp_flp_fcoe_vsan_mode_set(
     int unit,
     int is_vsan_from_vsi
   );

uint32
   arad_pp_flp_n_port_programs_disable(
     int unit
   );

uint32
   arad_pp_flp_npv_programs_init(
     int unit
   );

uint32
arad_pp_flp_fcoe_is_zoning_enabled(
     int unit,
     int* is_enabled
   );

uint32
arad_pp_flp_fcoe_is_vsan_from_vsi_mode(
     int unit,
     int* is_vsan_from_vsi
   );
uint32   arad_pp_flp_lsr_stat_init(int unit);


/*********************************************************************
* NAME:
 *   arad_pp_flp_access_print_last_programs_data
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Dump last FLP program invoked.
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT uint32                *last_program_id -
 *     Last FLP program invoked.
 * REMARKS:
 *   This API must be called during a continuous stream of
 *   the identical packets coming from the same source.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_flp_access_print_last_programs_data(
    SOC_SAND_IN   int                 unit,
    SOC_SAND_IN   int                 core_id,
    SOC_SAND_IN   int                 to_print,
    SOC_SAND_OUT  int                 *prog_id,
    SOC_SAND_OUT  int                 *sec_prog_id
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FLP_INIT_INCLUDED__*/
#endif

