#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pmf_low_level.c,v 1.88 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FP

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/dcmn/error.h>
#include <soc/mem.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/arad_action_cmd.h>
#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_ce.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_db.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_fem_tag.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_pgm.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_init.h>
#include <soc/dpp/ARAD/arad_ports.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/arad_pmf_prog_select.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_trap_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Multicast ID prefix in PP encoding: 101 in Arad, 10 in Jericho */
#define ARAD_PMF_DESTINATION_MULTICAST_ID_PREFIX    (SOC_IS_JERICHO(unit)? 0x2: 0x5) 
#define ARAD_PMF_DESTINATION_FLOW_ID_PREFIX         (0x6) /* 110 - Flow-ID (16b) */
#define ARAD_PMF_DESTINATION_SYSTEM_PORT_PREFIX     (0x1) /* 001 - System-Port-ID (16b) in Jericho PP destination encoding */

/* TM PMF Program offsets */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
/* LBP Header Profile for CPU to CPU WA */
#define ARAD_PMF_CPU_TO_CPU_HEADER_PROFILE_OFFSET   (8) /* Bits 11:8 */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
#define ARAD_PMF_SNOOP_TRAP_ID_BASE_OFFSET   (12) /* Bits 17:12 */
#define ARAD_PMF_TM_PGM_SYSTEM_PORT_OFFSET   (24) /* Bits 26:24 */

/* Length of data and mask in Ingress PMF program selection table */
#define ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS (2 * SOC_DPP_DEFS_GET(unit, ihb_pmf_program_selection_cam_mask_nof_bits)) 

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
    ARAD_PMF_KEY_A = 0,
    ARAD_PMF_KEY_B,
    ARAD_PMF_KEY_C,
    ARAD_PMF_KEY_D
} ARAD_PMF_KEY;

typedef enum
{
    ARAD_PMF_KEY_LSB_0 = 0,
    ARAD_PMF_KEY_LSB_16 = 16,
    ARAD_PMF_KEY_LSB_32 = 32,
    ARAD_PMF_KEY_LSB_48 = 48,
    ARAD_PMF_KEY_LSB_80 = 80,
    ARAD_PMF_KEY_LSB_96 = 96,
    ARAD_PMF_KEY_LSB_112 = 112,
    ARAD_PMF_KEY_LSB_128 = 128
} ARAD_PMF_KEY_LSB;

typedef struct
{
    /* Is-Not-TM: FALSE for regular TM, True otherwise */
    uint32 in_port_profile;

    /* Is-MSB group */
    uint32 is_msb;

    /* Copy-Engine index. Range: 0 - 7. */
    uint32 ce_id;

  /* Qualifier value */
  SOC_PPC_FP_QUAL_TYPE   qual_type;

  /* 
   * Qualifier number of bits by default
   * 0 indicates all the fields 
   */ 
  uint32 qual_nof_bits;

  /* 
   * Qualifier LSB
   */ 
  uint32 qual_lsb;

  /* Key bitmap: each of the four first bits  */
  uint32 key_supported_bitmap;

} ARAD_PMF_KEY_CONSTR;

typedef struct
{
    /* Is-Not-TM: FALSE for regular TM, True otherwise */
    uint32 in_port_profile;

    /* FES index. Range: 0 - 7. */
    uint32 fes_id;
  /* 
   * FES info: action type, shift, and is-always-valid bit
   */ 
  ARAD_PMF_FES_INPUT_INFO fes_info;

  /* Key source of this FES  */
  uint32 input_key_id;

  /* Key-LSB source of this FES  */
  ARAD_PMF_KEY_LSB input_key_lsb;

} ARAD_PMF_FES_CONSTR;



/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_stacking[] = {
            /* Stacking TM Program */
            /* Is-Not-TM, Is-MSB, CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
    /* KeyC - first lines for conditional setting when parsing Arad_pmf_key_construction_info_stacking */
#ifdef BCM_88660_A0
    {2, 0,  8, SOC_PPC_FP_QUAL_HDR_FTMH_LB_KEY_EXT_AFTER_FTMH,  8,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Same LB KEY at MSB. Only if SOC property LAG-Key Duplicate is set */
#endif /* BCM_88660_A0 */
    {2, 0,  9, SOC_PPC_FP_QUAL_HDR_FTMH_LB_KEY_EXT_AFTER_FTMH,  8,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* LB KEY*/
#ifdef BCM_88660_A0
    {2, 0, 10, SOC_PPC_FP_QUAL_HDR_FTMH_LB_KEY_START_OF_PACKET, 8,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* LB KEY[7:0] from User-Header-2. Only if SOC property FULL_HASH is set */
#endif /* BCM_88660_A0 */
    
    /* First lines also because modified sometimes */
#ifdef ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED
    {2, 1, 10, SOC_PPC_FP_QUAL_HDR_FTMH,                       16, 52,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* FTMH bits 19:4 for Out-LIF - inverted with DSP-Extension if custom_feature_egress_snooping_advanced = 1*/
#endif /* ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED */
    {2, 1,  2, SOC_PPC_FP_QUAL_HDR_DSP_EXTENSION_AFTER_FTMH,   16,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* DSP_EXTENSION - inverted with Out-LIF if custom_feature_egress_snooping_advanced = 1*/

     /* KeyA */
    {2, 0, 11, SOC_PPC_FP_QUAL_HDR_FTMH,                        8,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* FTMH bits 64-71*/
    {2, 0, 14, SOC_PPC_FP_QUAL_HDR_FTMH,                       32,  8,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* FTMH bits 32-63*/
    {2, 0, 15, SOC_PPC_FP_QUAL_HDR_FTMH,                       32, 40,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* FTMH bits 0-31*/

    /* Do not change the 2 following instruction IDs: (1,0) and (1,1) since they are inverted in Jericho */
    {2, 1,  0, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* 1 bit one*/ /* Invert the encoding in Jericho: 001 instead of 100 */
    {2, 1,  1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 2,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* 2 bits zeroes*/
/* Copied before:{2, 1,  2, SOC_PPC_FP_QUAL_HDR_DSP_EXTENSION_AFTER_FTMH,   16,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, *//* DSP_EXTENSION - inverted with Out-LIF if custom_feature_egress_snooping_advanced = 1*/
    {2, 1,  3, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT, 16,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* STACKING_EXTENTION*/

    /* KeyB - = {MCID_PREFIX=101,MCID[15:0]*/
    /* Do not change the Copy Engine IDs (0,1/2/4) because they have different content in Jericho */
    {2, 0,  0, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit one*/
    {2, 0,  1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit zero - it will be one in Jericho */
    {2, 0,  2, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit one  - it will be zero in Jericho */
    {2, 0,  3, SOC_PPC_FP_QUAL_HDR_FTMH,                       16, 52,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* MCID[15:0]  - it will be MCID[16:0] in Jericho */

#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
    /* CPU to CPU wa imp. */
    {2, 1,  4, SOC_PPC_FP_QUAL_IRPP_SRC_TM_PORT,                 8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* PTC port */
    {2, 1,  5, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT,   8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* FTMH Stacking-Extension[15:8] */
    {2, 1,  6, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,         4, 8,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ProgramKeyGenVar with the new Header-Profile */
    {2, 1,  7, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT,   8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* FTMH Stacking-Extension[15:8] */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */



    /* KeyD*/
    {2, 1,  8, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 1 bit one*/
    {2, 1,  9, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 7,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 7 bits zero*/

    

        };

CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_stacking_petra_mode[] = {
            /* Stacking TM Program */
            /* Is-Not-TM, Is-MSB, CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
     /* KeyA */
    {2, 0, 11, SOC_PPC_FP_QUAL_HDR_FTMH,                        1, 19,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* Petra FTMH bits 28 Src System Port LAG */
    {2, 0, 12, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 3,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* 3 bits zero*/
    {2, 0, 13, SOC_PPC_FP_QUAL_HDR_FTMH,                       12, 20,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* Petra FTMH bits 27-16 Src System Port */
    {2, 0, 14, SOC_PPC_FP_QUAL_HDR_FTMH,                       32,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* Petra FTMH bits 32-63*/
    {2, 0, 15, SOC_PPC_FP_QUAL_HDR_FTMH,                       32, 32,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* Petra FTMH bits 0-31*/

    {2, 1,  6, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT_PETRA, 16,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* STACKING_EXTENTION*/

    /* KeyB - = {Sys-port_PREFIX=1000000,Dst-Syst-Port[11:0]*/

    {2, 0,  0, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit one*/
    {2, 0,  1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 6,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 6 bits zero*/
    {2, 0,  2, SOC_PPC_FP_QUAL_HDR_FTMH,                       12, 52,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* Dst-port[11:0]*/

    /* KeyB - = {MCID_PREFIX=10100,MC-ID[13:0], Is-MC*/
    {2, 1,  0, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit one*/
    {2, 1,  1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit zero*/
    {2, 1,  2, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 1 bit one*/
    {2, 1,  3, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 2,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* 2 bits zero*/
    {2, 1,  4, SOC_PPC_FP_QUAL_HDR_FTMH,                       14, 50,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* MC-id[13:0]*/
    {2, 1,  5, SOC_PPC_FP_QUAL_HDR_FTMH,                        1, 47,  SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* Is-MC */

    /* KeyC  = {Flow_PREFIX=1000,Flow[14:0], Is-Flow*/
    {2, 0,  3, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   2,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 2 bits one*/
    {2, 0,  4, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 2,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 2 bits zero*/
    {2, 0,  5, SOC_PPC_FP_QUAL_HDR_FTMH,                       15, 49,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow[14:0]*/
    {2, 0,  6, SOC_PPC_FP_QUAL_HDR_FTMH,                        8, 48,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Is-Flow | 7'b other */

    /* KeyD*/
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
    /* CPU to CPU wa imp. */
    {2, 0,  7, SOC_PPC_FP_QUAL_IRPP_SRC_TM_PORT,                    8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* PTC port */
    {2, 0,  8, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT_PETRA,8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* FTMH Stacking-Extension[15:8] */
    {2, 0,  9, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,            4, 8,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* ProgramKeyGenVar with the new Header-Profile */
    {2, 0, 10, SOC_PPC_FP_QUAL_HDR_STACKING_EXT_AFTER_DSP_EXT_PETRA,8, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* FTMH Stacking-Extension[15:8] */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */

    {2, 1,  8, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,                   1,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 1 bit one*/
    {2, 1,  9, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,                 7,  0,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 7 bits zero*/

        };

        
/* 
 * XGS TM: Differv, HQoS first encoding mode. 
 * Note: In case key construction is changed in this mode, need to relate also in Arad_pmf_key_construction_info_xgs_8_modid_7_port  
 */ 
CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_xgs_7_modid_8_port[] = {
            /* Regular TM Program */
            /* Is-Not-TM, Is-MSB            , CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */

    /* KeyA */
    {0, 0, 8 , SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT, 2 ,  2, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
    {0, 0, 14, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 32,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*FRC_HEADER_64_32*/
    {0, 0, 15, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 32, 32, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*FRC_HEADER_31_0*/
    /* UC Destination Port */
    {0, 1, 0,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
    {0, 1, 1,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 3 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*this and the above are destination-system-port-prefix*/
    {0, 1, 2,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 15, 17, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*destination-system-port port */
    {0, 1, 3/*15*/, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*MSB diffserv*/

    /* KeyB */
    /* Multicast */
    {0, 0, 0,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES           , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 0, 1,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES         , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 0, 2,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES           , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*this and the above are MC-ID-prefix*/
    {0, 0, 6,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*destination-system-port*/
    {0, 0, 11, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 1 , 11, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*is_mc*/
    {0, 0, 12, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 20, 8 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*recycle port*/
    
    {0, 1, 4,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES     , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 1, 8,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES   , 2 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*this and the above are destination-system-port-prefix*/
    {0, 1, 9,  SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 1, 10, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC     , 2 , 58, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*EHV_1_MORE_BITS*/

    /* KeyC */
    {0, 0, 3,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES  , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, 
    {0, 0, 7,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES, 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)},  /*this and the above are destination-system-port-prefix*/
    {0, 0, 13, SOC_PPC_FP_QUAL_HDR_HIGIG_PPD  , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ppd destination*/

    {0, 1, 11, SOC_PPC_FP_QUAL_IRPP_ALL_ONES          , 2 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_C)},   /* do 1_ONES_4_BITS_ZEROS usind 2 ce commands */
    {0, 1, 12, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES        , 3 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* do 1_ONES_4_BITS_ZEROS usind 2 ce commands */
    {0, 1, 13/*12*/, SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT, 14, 18, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*DESTINATION_SHORT*/
    {0, 1, 15/*13*/, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 6 , 58, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*EHV_5_MORE_BITS*/

    /* KeyD */
    /*{0, 0, 4,  SOC_PPC_FP_QUAL_IHP_PMF_LSB_32B_INST_IN_PORT, Nof_bits, Qual_LSB, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, for ICNM */
    /*{0, 0, 5,  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 17, 40, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, */ /*INGRESS_SHAPING*/
    /*{0, 0, 9,  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1,  4,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, */ /*not diffserv*/
    {0, 0, 4,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*source-system-port*/

    {0, 1, 5,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 8, 48, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*FRC_LBID*/
    {0, 1, 7,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 8, 48, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*FRC_LBID*/
    {0, 1, 14, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1,  32, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*USE_EXT_HEADER*/
     
        };


/* 
 * XGS TM: Differv, HQoS second encoding mode. 
 * Note: In case key construction is changed in this mode, need to relate also in Arad_pmf_key_construction_info_xgs_7_modid_8_port  
 */ 
CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_xgs_8_modid_7_port[] = {
            /* Regular TM Program */
            /* Is-Not-TM, Is-MSB            , CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */

    /* KeyA */     
    {0, 0, 8 , SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT, 2 ,  2, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
    {0, 0, 14, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 32,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*FRC_HEADER_64_32*/
    {0, 0, 15, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 32, 32, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*FRC_HEADER_31_0*/
    /* UC Destination Port */
    {0, 1, 0,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
    {0, 1, 1,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 3 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*this and the above are destination-system-port-prefix*/
    {0, 1, 2,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 8,   16, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*destination-system-port modid */
    {0, 1, 3,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 7,   25, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*destination-system-port port */
    {0, 1, 6, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /*MSB diffserv*/

    /* KeyB */
    /* Multicast */
    {0, 0, 0,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES           , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 0, 1,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES         , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 0, 2,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES           , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*this and the above are MC-ID-prefix*/
    {0, 0, 6,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*destination-system-port*/
    {0, 0, 11, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 1 , 11, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*is_mc*/
    {0, 0, 12, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 20, 8 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*recycle port*/
    
    {0, 1, 4,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES     , 1 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 1, 8,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES   , 2 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*this and the above are destination-system-port-prefix*/
    {0, 1, 9,  SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_B)},
    {0, 1, 10, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC     , 2 , 58, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*EHV_1_MORE_BITS*/

    /* KeyC */
    {0, 0, 3,  SOC_PPC_FP_QUAL_IRPP_ALL_ONES  , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, 
    {0, 0, 7,  SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES, 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)},  /*this and the above are destination-system-port-prefix*/
    {0, 0, 13, SOC_PPC_FP_QUAL_HDR_HIGIG_PPD  , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ppd destination*/

    {0, 1, 11, SOC_PPC_FP_QUAL_IRPP_ALL_ONES          , 2 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_C)},   /* do 1_ONES_4_BITS_ZEROS usind 2 ce commands */
    {0, 1, 12, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES        , 3 , 0 , SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* do 1_ONES_4_BITS_ZEROS usind 2 ce commands */
    {0, 1, 13/*12*/, SOC_PPC_FP_QUAL_HDR_HIGIG_PPD_EXT, 14, 18, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*DESTINATION_SHORT*/
    {0, 1, 15/*13*/, SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 6 , 58, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*EHV_5_MORE_BITS*/

    /* KeyD */
  /*{0, 0, 4,  SOC_PPC_FP_QUAL_IHP_PMF_LSB_32B_INST_IN_PORT, Nof_bits, Qual_LSB, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, for ICNM */
  /*{0, 0, 5,  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 17, 40, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, */ /*INGRESS_SHAPING*/
  /*{0, 0, 9,  SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1,  4,  SOC_SAND_BIT(ARAD_PMF_KEY_D)}, */ /*not diffserv*/
    {0, 0, 4,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC    , 16, 16, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*outlif-dest-system-port-without-encoding*/

    {0, 1, 5,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 8, 48, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*FRC_LBID*/
    {0, 1, 7,  SOC_PPC_FP_QUAL_HDR_HIGIG_FRC           , 8, 48, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*FRC_LBID*/
    {0, 1, 14, SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR, 1,  32, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /*USE_EXT_HEADER*/
     
        };


/* This table defines the key of the predefined program */

/* Tip: sharing PFC can be done by invlaidating instead of FES-not-always-True */
/* 
 * TM: description of the Key 
 * A LSB: {5InPortKeyGenVar[31:0], 7PFC-FMC[1:0]}  
 * B LSB: {101-ITMH-4Dest=Multicast-Id (19b)} 
 * C LSB: {11-3ITMH-Flow-MC-Id (16b)- 7PFC-FMC[1:0]}  
 * D LSB: {6ITMH[31:0]}  
 * A MSB: {1All-ones(32b), 7ITMH[28:0]}  
 * B MSB: {4ITMH-Ext[31:0], 6PFC-DestInExtension[4:0]}    
 * C MSB: {4ITMH-Ext[31:0], 5PFC-FMC[1:0]}  
 * D MSB: {2LLR-Mirror-Command 4b, 4ITMH[29:0]}  
 */
/* Any change should be reported also to Jericho Arad-ITMH parsing */
CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_tm[] = {
            /* Regular TM Program */
            /* Is-Not-TM, Is-MSB, CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
#if 1
     /* KeyA: 160b after ITMH for TM and FP */
    {2, 0, 3, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 80, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 79:64 */
    {2, 0, 4, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32, 96, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 63:32 */
    {2, 0, 5, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,128, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 31:0 */

    {2, 1, 9, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 111:80 */
    {2, 1,12, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,16, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 143:112 */
    {2, 1,13, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,48, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 159:144 */

     /* KeyB */
/* Defined in Key-C {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)},   */
    {2, 0, 13, SOC_PPC_FP_QUAL_HDR_ITMH,                 32, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH 32b */
    {2, 0, 14, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* PFC0 of IS */
    {2, 0, 15, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,        20, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* Forwarding Destination according to the PortForwarding Header*/

    {2, 1,  1, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,  8,12, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* All ones for snoop MSB */
    {2, 1,  2, SOC_PPC_FP_QUAL_HDR_ITMH,                  5, 3, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH.SnoopCommand */
    {2, 1,  5, SOC_PPC_FP_QUAL_IRPP_PTC_KEY_GEN_VAR,      8, 8, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* BYTES_TO_REMOVE_HEADER_AND_FIX: PTC_KeyGenVar 15:8 
      Use PTC instead of InPort due to injected TM traffic on any PP-Port. */

    /* KeyC */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
    /* Reserve-bits fo PTCH to copy */
    {2, 0, 0, SOC_PPC_FP_QUAL_HDR_PTCH_RESERVE_LSB,        1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, 
        /* PTCH-Reserve bit LSB - Stacking-Ext.MSB to be set */
    {2, 0, 1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,            15, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 15 bits zero for the 15 LSBs of Stacking-Ext */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
#if (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED)
    {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFQ-0 bit 0 to signal it is from injected port*/
#endif /* (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED) */
    {2, 0,  6, SOC_PPC_FP_QUAL_HDR_ITMH,                  32, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH */
    {2, 0,  9, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         3, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC[2:0] */


    {2, 1,  0, SOC_PPC_FP_QUAL_HDR_ITMH,                   4,31-19, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH.19:16 - Assume there is a 0 afterwards*/
    {2, 1,  3, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,   4, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id prefix */
    {2, 1,  8, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,          16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id from ITMH[15:0] */
    {2, 1, 10, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,          2, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC-1 of MC-Flow */

    /* KeyD */
    {2, 0, 7, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,            24, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 0, 8, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          6, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 6 zeroes */
    {2, 0,10, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,            1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 16 ones */
    {2, 0,11, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          5, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 5 zeroes */
    {2, 0,12, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     0, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context */

    {2, 1, 4, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR, 4, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Multicast-Id prefix */
    {2, 1, 6, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,        16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 1, 7, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,               20, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Destination Extension */
    {2, 1,11, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context bit 0 */
#endif

};

/* Arad ITMH in Jericho: must be different since the PP-destination has a different encoding */
CONST STATIC
    ARAD_PMF_KEY_CONSTR
        JerichoAsArad_pmf_key_construction_info_tm[] = {
            /* Regular TM Program */
            /* Is-Not-TM, Is-MSB, CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
     /* KeyA: 160b after ITMH for TM and FP */
    {2, 0, 3, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 80, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 79:64 */
    {2, 0, 4, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32, 96, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 63:32 */
    {2, 0, 5, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,128, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 31:0 */

    {2, 1, 9, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 111:80 */
    {2, 1,12, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,16, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 143:112 */
    {2, 1,13, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,48, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 159:144 */

     /* KeyB */
/* Defined in Key-C {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)},   */
    {2, 0,  6, SOC_PPC_FP_QUAL_HDR_ITMH,                 32, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH 32b */
    {2, 0, 14, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* PFC0 of IS */
    {2, 0, 15, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,        20, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* Forwarding Destination according to the PortForwarding Header*/

    {2, 1,  1, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,  8,12, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* All ones for snoop MSB */
    {2, 1,  2, SOC_PPC_FP_QUAL_HDR_ITMH,                  5, 3, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH.SnoopCommand */
    {2, 1,  5, SOC_PPC_FP_QUAL_IRPP_PTC_KEY_GEN_VAR,      8, 8, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* BYTES_TO_REMOVE_HEADER_AND_FIX: PTC_KeyGenVar 15:8 
      Use PTC instead of InPort due to injected TM traffic on any PP-Port. */

    /* KeyC */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
    /* Reserve-bits fo PTCH to copy */
    {2, 0, 0, SOC_PPC_FP_QUAL_HDR_PTCH_RESERVE_LSB,        1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, 
        /* PTCH-Reserve bit LSB - Stacking-Ext.MSB to be set */
    {2, 0, 1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,            15, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 15 bits zero for the 15 LSBs of Stacking-Ext */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
#if (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED)
    {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFQ-0 bit 0 to signal it is from injected port*/
#endif /* (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED) */
    {2, 0, 12, SOC_PPC_FP_QUAL_HDR_ITMH,                  17,15, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH.OutLIF with zero as bit 18 */
    {2, 0, 13, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         3, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC[2:0] */


    {2, 1,  0, SOC_PPC_FP_QUAL_HDR_ITMH,                   4,31-19, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH.19:16 - Assume there is a 0 afterwards*/
    {2, 1,  3, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,   4, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id prefix */
    {2, 1,  8, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,          16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id from ITMH[15:0] */
    {2, 1, 10, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,          2, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC-1 of MC-Flow */

    /* KeyD */
    {2, 0, 7, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,            24, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 0, 8, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          6, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 6 zeroes */
    {2, 0, 9, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,            1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 16 ones */
    {2, 0,10, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          5, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D) | SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 5 zeroes - also for OutLIF */
    {2, 0,11, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     0, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context */

    {2, 1, 4, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR, 2, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Multicast-Id prefix */
    {2, 1, 6, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,       17, 3, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 1, 7, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR, 3,24, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* System-Port prefix */
    {2, 1,11, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,       16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination - for System-Port */
    {2, 1,14, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,            20, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Destination Extension */
    {2, 1,15, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context bit 0 */
};

/* Jericho parsing of Jericho-ITMH */
CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Jericho_pmf_key_construction_info_tm[] = {
            /* Regular TM Program */
            /* Is-Not-TM, Is-MSB, CE-id   Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
     /* KeyA: 160b after ITMH for TM and FP */
    {2, 0, 3, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 80, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 79:64 */
    {2, 0, 4, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32, 96, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 63:32 */
    {2, 0, 5, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,128, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 31:0 */

    {2, 1, 9, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     16, 0, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 111:80 */
    {2, 1,12, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,16, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 143:112 */
    {2, 1,13, SOC_PPC_FP_QUAL_HDR_ITMH_PMF_HDR_EXT,     32,48, SOC_SAND_BIT(ARAD_PMF_KEY_A)}, /* PMF Header Extension 159:144 */

     /* KeyB */
/* Defined in Key-C {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)},   */
    {2, 0,  6, SOC_PPC_FP_QUAL_HDR_ITMH,                 32, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH 32b */
    {2, 0, 14, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* PFC0 of IS */
    {2, 0, 15, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,        20, 0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* Forwarding Destination according to the PortForwarding Header*/

    {2, 1,  1, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,  8,12, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* All ones for snoop MSB */
    {2, 1,  2, SOC_PPC_FP_QUAL_HDR_ITMH,                  5, 3, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* ITMH.SnoopCommand */
    {2, 1,  5, SOC_PPC_FP_QUAL_IRPP_PTC_KEY_GEN_VAR,      8, 8, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /* BYTES_TO_REMOVE_HEADER_AND_FIX: PTC_KeyGenVar 15:8 
      Use PTC instead of InPort due to injected TM traffic on any PP-Port. */

    /* KeyC */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
    /* Reserve-bits fo PTCH to copy */
    {2, 0, 0, SOC_PPC_FP_QUAL_HDR_PTCH_RESERVE_LSB,        1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, 
        /* PTCH-Reserve bit LSB - Stacking-Ext.MSB to be set */
    {2, 0, 1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,            15, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 15 bits zero for the 15 LSBs of Stacking-Ext */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
#if (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED)
    {2, 0, 2, SOC_PPC_FP_QUAL_PACKET_FORMAT_QUALIFIER0,    1, 0,  SOC_SAND_BIT(ARAD_PMF_KEY_B) | SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFQ-0 bit 0 to signal it is from injected port*/
#endif /* (ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED || ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED) */
    {2, 0, 12, SOC_PPC_FP_QUAL_HDR_ITMH,                  17,15, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH.OutLIF with zero as bit 18 */
    {2, 0, 13, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,         3, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC[2:0] */


    {2, 1,  0, SOC_PPC_FP_QUAL_HDR_ITMH,                   4,31-19 - 8, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* ITMH.19:16 - Assume there is a 0 afterwards*/
    {2, 1,  3, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR,   4, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id prefix */
    {2, 1,  8, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,          16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* Flow-Id from ITMH[15:0] */
    {2, 1, 10, SOC_PPC_FP_QUAL_IRPP_PKT_HDR_TYPE,          2, 0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* PFC-1 of MC-Flow */

    /* KeyD */
    {2, 0, 7, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,            24, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 0, 8, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          6, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 6 zeroes */
    {2, 0, 9, SOC_PPC_FP_QUAL_IRPP_ALL_ONES,            1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* 16 ones */
    {2, 0,10, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES,          5, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D) | SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /* 5 zeroes - also for OutLIF */
    {2, 0,11, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     0, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context */

    {2, 1, 4, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR, 2, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Multicast-Id prefix */
    {2, 1, 6, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,       17, 3, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination */
    {2, 1, 7, SOC_PPC_FP_QUAL_IRPP_PMF_PGM_KEY_GEN_VAR, 3,24, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* System-Port prefix */
    {2, 1,11, SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD,       16, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Forwarding Destination - for System-Port */
    {2, 1,14, SOC_PPC_FP_QUAL_HDR_ITMH_EXT,            20, 4, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Destination Extension */
    {2, 1,15, SOC_PPC_FP_QUAL_IRPP_PROCESSING_TYPE,     1, 0, SOC_SAND_BIT(ARAD_PMF_KEY_D)}, /* Parser-Leaf-Context bit 0 */
};

CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_mh[] = {
            /* Regular TM Program */
/* Is-Not-TM, Is-MSB,CE-id,Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
	/* KeyA */
	{5, 0, 15, SOC_PPC_FP_QUAL_HDR_MH_FLOW	  	, 16,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*copy 16 bits of the flow id*/
	{5, 0, 14, SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*++++++++++++*/
	{5, 0, 13, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*++ set action code for MC*/
	{5, 0, 12, SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*++++++++++++*/
	{5, 0, 11, SOC_PPC_FP_QUAL_HDR_MH_CAST      , 8 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*Coy MC/UC indictaion from SOC_PPC_FP_QUAL_HDR_MH_CAST[0..7]. Bit 0 is the indication*/
    {5, 0, 10, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 4 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*align to 32 bit*/
	{5, 0,  9, SOC_PPC_FP_QUAL_HDR_MH_FLOW	  	, 16,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*Copy 16 bits of the flow id*/
	{5, 0,  8, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*++++++++++++++++*/
	{5, 0,  7, SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 3 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},/*+ set action code for UC*/
																					/*+++++++++++++++*/
																					/**/

    /* KeyB VSQ ptr = TM poty + TC*/  																	/**/
	{5, 1,11, SOC_PPC_FP_QUAL_HDR_MH_TC10      , 2 ,  6, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*Copy bit 0 and 1 of TC from the packet*/
	{5, 1,10, SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*add constant 1*/
	{5, 1, 9, SOC_PPC_FP_QUAL_IRPP_SRC_TM_PORT , 5 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*add constant 5 bits of TM port*/
	{5, 1, 8, SOC_PPC_FP_QUAL_HDR_MH_TC10      , 2 ,  6, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*Copy bit 0 and 1 of TC from the packet*/
	{5, 1, 7, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 1 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*Add constant 0*/
	{5, 1, 6, SOC_PPC_FP_QUAL_IRPP_SRC_TM_PORT , 5 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*add constant 5 bits of TM port*/
	{5, 1, 5, SOC_PPC_FP_QUAL_HDR_MH_TC2       , 5 ,  3, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, /*Copy TC2 bit form PPC_FP_QUAL_HDR_MH_TC2[0..7] Bit 5 is the indication*/
	/* KeyC VSQ ptr = TM poty + TC*/  														/**/
	{5, 1, 4, SOC_PPC_FP_QUAL_HDR_MH_DP0       , 6 ,  2, SOC_SAND_BIT(ARAD_PMF_KEY_C)},/*Copy bit 0 of DP from the packet*/
	{5, 1, 3, SOC_PPC_FP_QUAL_IRPP_ALL_ONES    , 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*add constant 1*/
	{5, 1, 2, SOC_PPC_FP_QUAL_HDR_MH_DP0       , 6 ,  2, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*Copy bit 0 of DP from the packet*/
	{5, 1, 1, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES  , 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*Add constant */
	{5, 1, 0, SOC_PPC_FP_QUAL_HDR_MH_DP1       , 5 ,  3, SOC_SAND_BIT(ARAD_PMF_KEY_C)}, /*Copy DP1 bit form PPC_FP_QUAL_HDR_MH_DP1[0..7] Bit 2 is the indication*/

	/*For all cases the indication bit will define whenever we use MC or UC, DP#1 high or low, TC2 high or low*/
    /* KeyC */
    /* KeyD */

        };

CONST STATIC
    ARAD_PMF_KEY_CONSTR
        Arad_pmf_key_construction_info_inpHeader[] = {        
            /* Regular TM Program */
/* Is-Not-TM, Is-MSB,CE-id,Qualifier type,   Nof-bits, Qual-LSB,   Key-Bitmap        */
	/* KeyA */
	{5, 0, 11, SOC_PPC_FP_QUAL_HDR_INPHEADER_UC	  	, 4,   0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
	{5, 0, 10, SOC_PPC_FP_QUAL_HDR_INPHEADER_UC_TC  , 3 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
	{5, 0,  9, SOC_PPC_FP_QUAL_HDR_INPHEADER_TB     , 8 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
	{5, 0,  6, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES      , 6 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
	{5, 0,  5, SOC_PPC_FP_QUAL_IRPP_ALL_ONES        , 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
	{5, 0,  3, SOC_PPC_FP_QUAL_HDR_INPHEADER_DP	  	, 5,   0, SOC_SAND_BIT(ARAD_PMF_KEY_A)},
																					
	/* KeyB */																		
	{5, 0, 13, SOC_PPC_FP_QUAL_HDR_INPHEADER_MC_TC   , 3 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, 
	{5, 0,  9, SOC_PPC_FP_QUAL_HDR_INPHEADER_TB      , 8 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, 
	{5, 0,  6, SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES       , 6 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, 
	{5, 0,  5, SOC_PPC_FP_QUAL_IRPP_ALL_ONES         , 2 ,  0, SOC_SAND_BIT(ARAD_PMF_KEY_B)}, 

    /* KeyC */
    /* KeyD */

        };


CONST STATIC 
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_stacking[] = {
            /* Stacking Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
            /* Next action only if custom_feature_egress_snooping_advanced=1 - otherwise skipped */
#ifdef ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED
  {2, 0,  {SOC_SAND_MAGIC_NUM_VAL,    0, SOC_PPC_FP_ACTION_TYPE_OUTLIF, TRUE},                      ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80}, 
#endif /* ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED */
            /* FIRST actions are for stacking only - otherwise skipped */
  {2, 1,  {SOC_SAND_MAGIC_NUM_VAL,    16, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80},
  {2, 2,  {SOC_SAND_MAGIC_NUM_VAL,    23, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {2, 3,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80},
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* CPU to CPU WA imp. */
  {2, 4,  {SOC_SAND_MAGIC_NUM_VAL,    23, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0}, /* WA only for UC packets */
  {2, 5,  {SOC_SAND_MAGIC_NUM_VAL,    7, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* Set the System-Header profile if the PTCH-Reserved-LSB is set */
  {2, 6,  {SOC_SAND_MAGIC_NUM_VAL,    23, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0}, /* WA only for UC packets */
  {2, 7,  {SOC_SAND_MAGIC_NUM_VAL,   19, SOC_PPC_FP_ACTION_TYPE_OUTLIF, FALSE},                    ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* Override the Out-LIF with PTC */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */

  {2, 8,  {SOC_SAND_MAGIC_NUM_VAL,     9, SOC_PPC_FP_ACTION_TYPE_PPH_TYPE,      TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2, 9,  {SOC_SAND_MAGIC_NUM_VAL,    13, SOC_PPC_FP_ACTION_TYPE_DP,            TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2, 10,  {SOC_SAND_MAGIC_NUM_VAL,     7, SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT, TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_32},
  {2, 11,  {SOC_SAND_MAGIC_NUM_VAL,    23, SOC_PPC_FP_ACTION_TYPE_TC,            TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_32},
  {2, 12,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_DEST,          TRUE},              ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0},
  {2, 13,  {SOC_SAND_MAGIC_NUM_VAL,    23, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {2, 14,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_DEST,          TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_96},
  {2, 15, {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET,    TRUE},              ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80},
  {2, 16, {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_LAG_LB,    TRUE},              ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0},
        };

/* FTMH parsing in case of Petra mode (Petra FTMH)*/
CONST STATIC
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_stacking_petra_mode[] = {
            /* Stacking Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
      /* FIRST actions are for stacking only - otherwise skipped */
/*  {2, 0,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80}, */
  {2, 1,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80},
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* CPU to CPU WA imp. */
  {2, 2,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80}, /* WA only for UC packets */
  {2, 3,  {SOC_SAND_MAGIC_NUM_VAL,    0, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, FALSE},  ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0}, /* Set the System-Header profile if the PTCH-Reserved-LSB is set */
  {2, 4,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80}, /* WA only for UC packets */
  {2, 5,  {SOC_SAND_MAGIC_NUM_VAL,   19, SOC_PPC_FP_ACTION_TYPE_OUTLIF, FALSE},                    ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0}, /* Override the Out-LIF with PTC */
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
  

  {2, 6,  {SOC_SAND_MAGIC_NUM_VAL,     1, SOC_PPC_FP_ACTION_TYPE_EXC_SRC,       TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2, 7,  {SOC_SAND_MAGIC_NUM_VAL,    29, SOC_PPC_FP_ACTION_TYPE_TC,            TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2, 8,  {SOC_SAND_MAGIC_NUM_VAL,     6, SOC_PPC_FP_ACTION_TYPE_DP,            TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2, 9,  {SOC_SAND_MAGIC_NUM_VAL,     2, SOC_PPC_FP_ACTION_TYPE_MIR_DIS,       TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {2,10,  {SOC_SAND_MAGIC_NUM_VAL,     3, SOC_PPC_FP_ACTION_TYPE_PPH_TYPE,      TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16}, /* 1b is sufficient */
  {2,11,  {SOC_SAND_MAGIC_NUM_VAL,    16, SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT, TRUE},              ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_48},
  {2,12,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_DEST,          TRUE},              ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* destination as system port */
  {2,13,  {SOC_SAND_MAGIC_NUM_VAL,     7, SOC_PPC_FP_ACTION_TYPE_DEST,         FALSE},              ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* destination as flow-id */
  {2,14,  {SOC_SAND_MAGIC_NUM_VAL,     0, SOC_PPC_FP_ACTION_TYPE_DEST,         FALSE},              ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* if MC set the destination as MC_ID */

        };


CONST STATIC
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_xgs[] = {
            /* XGS Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
  {0, 0, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_OUTLIF,                TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},
  {0, 1, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_TC,                    TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_48},
  {0, 2, {SOC_SAND_MAGIC_NUM_VAL,     6,  SOC_PPC_FP_ACTION_TYPE_DP,                    TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {0, 3, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT,         TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {0, 4, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_DEST,                  TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0},
  {0, 5, {SOC_SAND_MAGIC_NUM_VAL,     5,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80},
  {0, 6, {SOC_SAND_MAGIC_NUM_VAL,     17, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,          FALSE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_48},
  {0, 7, {SOC_SAND_MAGIC_NUM_VAL,     1,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},
  {0, 8, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,          FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},
  {0, 9, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_DEST,                  TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0},
  {0, 10, {SOC_SAND_MAGIC_NUM_VAL,    0,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80},
  {0, 11, {SOC_SAND_MAGIC_NUM_VAL,    4,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {0, 12, {SOC_SAND_MAGIC_NUM_VAL,    16, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2,         TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {0, 13, {SOC_SAND_MAGIC_NUM_VAL,    1,  SOC_PPC_FP_ACTION_TYPE_LAG_LB,                TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80},
/*  {0, 14, {SOC_SAND_MAGIC_NUM_VAL,    1,  SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE,  TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},
  {0, 15, {SOC_SAND_MAGIC_NUM_VAL,    18, SOC_PPC_FP_ACTION_TYPE_STAT,                  TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0}, */
        };

CONST STATIC
    ARAD_PMF_FES_CONSTR
        Jericho_pmf_fes_construction_info_xgs[] = {
            /* XGS Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
  {0, 0, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_OUTLIF,                TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},
  {0, 1, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_TC,                    TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_48},
  {0, 2, {SOC_SAND_MAGIC_NUM_VAL,     6,  SOC_PPC_FP_ACTION_TYPE_DP,                    TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {0, 3, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT,         TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {0, 4, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_DEST,                  TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0},
  {0, 5, {SOC_SAND_MAGIC_NUM_VAL,     5,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80},
  {0, 6, {SOC_SAND_MAGIC_NUM_VAL,     17, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,          FALSE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_48},
  {0, 7, {SOC_SAND_MAGIC_NUM_VAL,     1,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},
  {0, 8, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,          FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},
  {0, 9, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_DEST,                  TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0},
  {0, 10, {SOC_SAND_MAGIC_NUM_VAL,    1,  SOC_PPC_FP_ACTION_TYPE_DEST,                  TRUE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_80},
  {0, 11, {SOC_SAND_MAGIC_NUM_VAL,    4,  SOC_PPC_FP_ACTION_TYPE_DEST,                  FALSE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {0, 12, {SOC_SAND_MAGIC_NUM_VAL,    16, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2,         TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_16},
  {0, 13, {SOC_SAND_MAGIC_NUM_VAL,    1,  SOC_PPC_FP_ACTION_TYPE_LAG_LB,                TRUE},   ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80},
};

/* This table is generated from \ARAD\PP\FP\excels\internal_fields.xlsx */
/* Any change in this table must be reported also to Jericho Arad-ITMH parsing */
CONST STATIC
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_tm[] = {
            /* Regular TM Program */
#if 1
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
  {2, 0, {SOC_SAND_MAGIC_NUM_VAL,        9, SOC_PPC_FP_ACTION_TYPE_SNP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* All ones + Snoop in ITMH (28:25) */
  {2, 1, {SOC_SAND_MAGIC_NUM_VAL,       27, SOC_PPC_FP_ACTION_TYPE_TC, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {2, 2, {SOC_SAND_MAGIC_NUM_VAL,       25, SOC_PPC_FP_ACTION_TYPE_DP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {2, 3, {SOC_SAND_MAGIC_NUM_VAL,         18, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* If set disable mirror by setting 0 in next action */
  {2, 4, {SOC_SAND_MAGIC_NUM_VAL,        31, SOC_PPC_FP_ACTION_TYPE_MIRROR, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_128}, /* Always set Profile=0 if enabled */
  {2, 5, {SOC_SAND_MAGIC_NUM_VAL,       19, SOC_PPC_FP_ACTION_TYPE_PPH_TYPE, TRUE},                 ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* ITMH 31:30 */
  {2, 6, {SOC_SAND_MAGIC_NUM_VAL,          4, SOC_PPC_FP_ACTION_TYPE_IS, FALSE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16}, /* In case of Ingress-Shaping, set the Ingress Shaping destination {IS,PFC[0]} */
  {2, 7, {SOC_SAND_MAGIC_NUM_VAL,          0, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* Dest = Flow-Id or System-Port (same PP encoding), otherwise it will be overridden*/
  {2, 8, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET, TRUE},                 ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},  /* Correct the Offset for Bytes2Remove */
  {2, 9, {SOC_SAND_MAGIC_NUM_VAL,          0, SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE, TRUE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, 
  {2,10, {SOC_SAND_MAGIC_NUM_VAL,          5, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, TRUE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16}, /* take all 19 bit dest from itmh without encoding */
  {2,11, {SOC_SAND_MAGIC_NUM_VAL,          0, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},             ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80}, /* In case of MC_FLOW_ID or IS, override the User-Header-2 with the Destination Extension */
  {2,12, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_OUTLIF, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the Out-LIF */
  {2,13, {SOC_SAND_MAGIC_NUM_VAL,          1, SOC_PPC_FP_ACTION_TYPE_DEST, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80}, /* Only in case of MC-Flow, extract the Flow from the Destination field */
  {2,14, {SOC_SAND_MAGIC_NUM_VAL,          19, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* Only in case of MC ITMH Dest, do the next action */
  {2,15, {SOC_SAND_MAGIC_NUM_VAL,          5, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_96}, /* Only in case of Multicast, override the destination with the Destination-Extension - 
                                                                                                                                        code the destination as MC = {prefix,dest[15:0]} */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* Set the Stacking-Route-History (MSB set, LSBs zero) if the PFQ0[0] is set */
  {2,16, {SOC_SAND_MAGIC_NUM_VAL,         0+3, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, FALSE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_32}, 
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
  {2,17, {SOC_SAND_MAGIC_NUM_VAL,        7, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, TRUE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_96}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,18, {SOC_SAND_MAGIC_NUM_VAL,        4, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, FALSE}, ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_48}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,19, {SOC_SAND_MAGIC_NUM_VAL,        2, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},           ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the User-header with Out-LIF */

  
  /* if (oam_test) begin
        PMF_FEM_SET_PROG(   14,           2,        2,  `PMF_KEY_SEL_C_111_80);
        PMF_FEM_SET_BIT_SELECT(14    , 2          , 5'd5);
      end */
#endif
	  
};

/* Jericho parsing of Arad-ITMH - different from Arad due to a different PP destination encoding */
CONST STATIC
    ARAD_PMF_FES_CONSTR
        JerichoAsArad_pmf_fes_construction_info_tm[] = {
            /* Regular TM Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
  {2, 0, {SOC_SAND_MAGIC_NUM_VAL,        9, SOC_PPC_FP_ACTION_TYPE_SNP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* All ones + Snoop in ITMH (28:25) */
  {2, 1, {SOC_SAND_MAGIC_NUM_VAL,       27, SOC_PPC_FP_ACTION_TYPE_TC, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {2, 2, {SOC_SAND_MAGIC_NUM_VAL,       25, SOC_PPC_FP_ACTION_TYPE_DP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {2, 3, {SOC_SAND_MAGIC_NUM_VAL,       18, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* If set disable mirror by setting 0 in next action */
  {2, 4, {SOC_SAND_MAGIC_NUM_VAL,       31, SOC_PPC_FP_ACTION_TYPE_MIRROR, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_128}, /* Always set Profile=0 if enabled */
  {2, 5, {SOC_SAND_MAGIC_NUM_VAL,       19, SOC_PPC_FP_ACTION_TYPE_PPH_TYPE, TRUE},                 ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* ITMH 31:30 */
  {2, 6, {SOC_SAND_MAGIC_NUM_VAL,        4, SOC_PPC_FP_ACTION_TYPE_IS, FALSE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16}, /* In case of Ingress-Shaping, set the Ingress Shaping destination {IS,PFC[0]} */
  {2, 7, {SOC_SAND_MAGIC_NUM_VAL,        0, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* Dest = Flow-Id or System-Port (same PP encoding), otherwise it will be overridden*/
  {2, 8, {SOC_SAND_MAGIC_NUM_VAL,       17, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* In case of Destination-System-Port, do the next action */
  {2, 9, {SOC_SAND_MAGIC_NUM_VAL,        5, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_96}, /* In case of System-Port (but not Flow), override the destination with {prefix,dest[15:0]} */
  {2,10, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET, TRUE},                 ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},  /* Correct the Offset for Bytes2Remove */
  {2,11, {SOC_SAND_MAGIC_NUM_VAL,          0, SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE, TRUE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, 
  {2,12, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_OUTLIF, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the Out-LIF */
  {2,13, {SOC_SAND_MAGIC_NUM_VAL,          1, SOC_PPC_FP_ACTION_TYPE_DEST, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80}, /* Only in case of MC-Flow, extract the Flow from the Destination field */
  {2,14, {SOC_SAND_MAGIC_NUM_VAL,          19, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* Only in case of MC ITMH Dest, do the next action */
  {2,15, {SOC_SAND_MAGIC_NUM_VAL,          8, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_112}, /* Only in case of Multicast, override the destination with the Destination-Extension - 
                                                                                                                                        code the destination as MC = {prefix,dest[15:0]} */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* Set the Stacking-Route-History (MSB set, LSBs zero) if the PFQ0[0] is set */
  {2,16, {SOC_SAND_MAGIC_NUM_VAL,         0+3+1+5, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, FALSE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_16}, 
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
  {2,17, {SOC_SAND_MAGIC_NUM_VAL,        7, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, TRUE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_96}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,18, {SOC_SAND_MAGIC_NUM_VAL,        4, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, FALSE}, ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_48}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,19, {SOC_SAND_MAGIC_NUM_VAL,        5, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, TRUE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16}, /* take all 19 bit dest from itmh without encoding */
  {2,20, {SOC_SAND_MAGIC_NUM_VAL,        0, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},             ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80}, /* In case of MC_FLOW_ID or IS, override the User-Header-2 with the Destination Extension */
  {2,21, {SOC_SAND_MAGIC_NUM_VAL,        2, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},           ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the User-header with Out-LIF */
};

/* Jericho parsing of Jericho-ITMH */
CONST STATIC
    ARAD_PMF_FES_CONSTR
        Jericho_pmf_fes_construction_info_tm[] = {
            /* Regular TM Program */
  /*  FES-ID,                       Shift  Action type,       Is-Always-Valid                      Key-source      Key LSB        */
  {2, 2, {SOC_SAND_MAGIC_NUM_VAL,       16, SOC_PPC_FP_ACTION_TYPE_DP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32},
  {2, 3, {SOC_SAND_MAGIC_NUM_VAL,       18, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* If set disable mirror by setting 0 in next action */
  {2, 4, {SOC_SAND_MAGIC_NUM_VAL,       31, SOC_PPC_FP_ACTION_TYPE_MIRROR, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_128}, /* Always set Profile=0 if enabled */
  {2, 5, {SOC_SAND_MAGIC_NUM_VAL,        9, SOC_PPC_FP_ACTION_TYPE_SNP, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, /* All ones + Snoop in ITMH (28:25) */
  {2, 6, {SOC_SAND_MAGIC_NUM_VAL,       6, SOC_PPC_FP_ACTION_TYPE_TC, TRUE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16},
  {2, 7, {SOC_SAND_MAGIC_NUM_VAL,       19, SOC_PPC_FP_ACTION_TYPE_PPH_TYPE, TRUE},                 ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_32}, /* ITMH 31:30 */
  {2, 8, {SOC_SAND_MAGIC_NUM_VAL,        4, SOC_PPC_FP_ACTION_TYPE_IS, FALSE},                         ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_16}, /* In case of Ingress-Shaping, set the Ingress Shaping destination {IS,PFC[0]} */
  {2, 9, {SOC_SAND_MAGIC_NUM_VAL,        0, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, /* Dest = Flow-Id or System-Port (same PP encoding), otherwise it will be overridden*/
/*  {2, 10, {SOC_SAND_MAGIC_NUM_VAL,       17, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, *//* In case of Destination-System-Port, do the next action */
/*  {2, 11, {SOC_SAND_MAGIC_NUM_VAL,        5, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_96}, *//* In case of System-Port (but not Flow), override the destination with {prefix,dest[15:0]} */
  {2,12, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET, TRUE},                 ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_0},  /* Correct the Offset for Bytes2Remove */
  {2,13, {SOC_SAND_MAGIC_NUM_VAL,          0, SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE, TRUE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80}, 
  {2,14, {SOC_SAND_MAGIC_NUM_VAL,          2, SOC_PPC_FP_ACTION_TYPE_OUTLIF, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the Out-LIF */
  {2,15, {SOC_SAND_MAGIC_NUM_VAL,          1, SOC_PPC_FP_ACTION_TYPE_DEST, FALSE},                     ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80}, /* Only in case of MC-Flow, extract the Flow from the Destination field */
/*  {2,16, {SOC_SAND_MAGIC_NUM_VAL,          19, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT, FALSE},             ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0}, */ /* Only in case of MC ITMH Dest, do the next action */
/*  {2,17, {SOC_SAND_MAGIC_NUM_VAL,          8, SOC_PPC_FP_ACTION_TYPE_DEST, TRUE},                     ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_112}, */ /* Only in case of Multicast, override the destination with the Destination-Extension - 
                                                                                                                                        code the destination as MC = {prefix,dest[15:0]} */
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* Set the Stacking-Route-History (MSB set, LSBs zero) if the PFQ0[0] is set */
  {2,18, {SOC_SAND_MAGIC_NUM_VAL,         0+3+1+5, SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST, FALSE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_16}, 
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */
  {2,19, {SOC_SAND_MAGIC_NUM_VAL,        7, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, TRUE}, ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_96}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,20, {SOC_SAND_MAGIC_NUM_VAL,        4, SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID, FALSE}, ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_48}, /* header profile is coded like the destination prefix. take 3 msb (19:17) */
  {2,21, {SOC_SAND_MAGIC_NUM_VAL,        5, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, TRUE},             ARAD_PMF_KEY_B, /* ARAD_PMF_KEY_LSB_16 */ ARAD_PMF_KEY_LSB_0}, /* take all 19 bit dest from itmh without encoding */
  {2,22, {SOC_SAND_MAGIC_NUM_VAL,        0, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},             ARAD_PMF_KEY_D, ARAD_PMF_KEY_LSB_80}, /* In case of MC_FLOW_ID or IS, override the User-Header-2 with the Destination Extension */
  {2,23, {SOC_SAND_MAGIC_NUM_VAL,        2, SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2, FALSE},           ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_0}, /* In case of Out-LIF ITMH Dest, override the User-header with Out-LIF */
};




CONST STATIC
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_mh[] = {
            /* XGS Program */
  /*  FES-ID,                       Shift  Action type,                     Is-Always-Valid                      Key-source      Key LSB        */

  {5, 0, {SOC_SAND_MAGIC_NUM_VAL,     5,  SOC_PPC_FP_ACTION_TYPE_DP,                   TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80},/*Set DP wiht DP1 = 0 and DP0 from packet*/
  {5, 1, {SOC_SAND_MAGIC_NUM_VAL,    18,  SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,        FALSE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80},/*If indication bit is set*/
  {5, 2, {SOC_SAND_MAGIC_NUM_VAL,    13,  SOC_PPC_FP_ACTION_TYPE_DP,                   TRUE},   ARAD_PMF_KEY_C, ARAD_PMF_KEY_LSB_80},/*update MSB of DP=1*/
  {5, 3, {SOC_SAND_MAGIC_NUM_VAL,     0 , SOC_PPC_FP_ACTION_TYPE_DEST,                 TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},/*Set MC destination form key B*/
  {5, 4, {SOC_SAND_MAGIC_NUM_VAL,    27 , SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,        FALSE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},/*Skip next line if cast==1 (indicaiton bit)*/
  {5, 5, {SOC_SAND_MAGIC_NUM_VAL,     0 , SOC_PPC_FP_ACTION_TYPE_DEST,                 TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_32},/*Set UC destination from key A (uc starts from bit 32)*/
  {5, 6, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_TC,                   TRUE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*Set TC wiht TC2 = 0 and TC10 from packet*/
  {5, 7, {SOC_SAND_MAGIC_NUM_VAL,     18, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,        FALSE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*If indication bit is set*/ 
  {5, 8, {SOC_SAND_MAGIC_NUM_VAL,     8,  SOC_PPC_FP_ACTION_TYPE_TC,                   TRUE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*update TC2=1*/       
  {5, 9, {SOC_SAND_MAGIC_NUM_VAL,     0,  SOC_PPC_FP_ACTION_TYPE_VSQ_PTR,              TRUE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*Set VSQ ptr according TC2=0*/
  {5,10, {SOC_SAND_MAGIC_NUM_VAL,     18, SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,        FALSE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*If indication bit is set*/ 
  {5,11, {SOC_SAND_MAGIC_NUM_VAL,     8,  SOC_PPC_FP_ACTION_TYPE_VSQ_PTR,              TRUE},   ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_80},/*Set VSQ ptr according TC2=1*/
};



CONST STATIC
    ARAD_PMF_FES_CONSTR
        Arad_pmf_fes_construction_info_inpHeader[] = {
            /* XGS Program */
  /*  FES-ID,                       Shift  Action type,                     Is-Always-Valid     Key-source      Key LSB        */
  {5, 0, {SOC_SAND_MAGIC_NUM_VAL,    3 , SOC_PPC_FP_ACTION_TYPE_DEST,            FALSE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {5, 1, {SOC_SAND_MAGIC_NUM_VAL,    3 , SOC_PPC_FP_ACTION_TYPE_TC,              FALSE},  ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {5, 2, {SOC_SAND_MAGIC_NUM_VAL,    26 , SOC_PPC_FP_ACTION_TYPE_DP,             TRUE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {5, 3, {SOC_SAND_MAGIC_NUM_VAL,    3,  SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,    FALSE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},
  {5, 4, {SOC_SAND_MAGIC_NUM_VAL,    0, SOC_PPC_FP_ACTION_TYPE_DEST,             TRUE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0},
  {5, 5, {SOC_SAND_MAGIC_NUM_VAL,    3,  SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT,    FALSE},   ARAD_PMF_KEY_A, ARAD_PMF_KEY_LSB_0},   
  {5, 6, {SOC_SAND_MAGIC_NUM_VAL,    0,  SOC_PPC_FP_ACTION_TYPE_TC,              TRUE},  ARAD_PMF_KEY_B, ARAD_PMF_KEY_LSB_0},

};


STATIC
uint32
  arad_pmf_low_level_ce_key_construction_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx,
    SOC_SAND_IN  uint32                in_port_profile, /* Identify the program type */
    SOC_SAND_IN  uint32                pmf_key_construction_info_length,
    SOC_SAND_IN  ARAD_PMF_KEY_CONSTR   *pmf_key_construction_info
  )
{
    uint32      
      table_line,      
      pmf_key,
      res = SOC_SAND_OK;
    uint8
      is_second_lookup,
      found;
    ARAD_PMF_CE_IRPP_QUALIFIER_INFO     
      qual_info;
    ARAD_PMF_CE_HEADER_QUAL_INFO          
      header_qual_info;
    ARAD_PMF_CE_PACKET_HEADER_INFO
      ce_packet_header_info;
    soc_reg_t
      global_f = SOC_IS_JERICHO(unit)? ECI_GLOBAL_SYS_HEADER_CFGr: ECI_GLOBALFr;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


  is_second_lookup = TRUE; /* No need to configure and use always the second lookup */
  for (table_line = 0; table_line < pmf_key_construction_info_length; ++table_line) {
#ifdef BCM_88660_A0
      /* Special case in stacking: skip first line in Arad mode, not duplicated SOC property set */
      if (in_port_profile == ARAD_PMF_PORT_PROFILE_FTMH) {
          uint32 headers_mode;

          /* Detect in Arad mode - Duplicate in Petra FTMH mode not supported (yet) */
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, global_f, REG_PORT_ANY, 0, SYSTEM_HEADERS_MODEf, &headers_mode));
          if (headers_mode ==  ARAD_PP_SYSTEM_HEADERS_MODE_ARAD) {
              if ((table_line == 0) && (!(SOC_DPP_CONFIG(unit)->arad->init.fabric.trunk_hash_format == ARAD_MGMT_TRUNK_HASH_FORMAT_DUPLICATED))) {
                  /* Skip the first line if not duplicated mode */
                  continue;
              }
              if ((table_line == 2) && (!(SOC_DPP_CONFIG(unit)->arad->init.fabric.ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_FULL_HASH))) {
                  /* Skip the first line if not duplicated mode */
                  continue;
              }
          }
      }
#endif /* BCM_88660_A0 */
      /* Apply only the correct Program */

      found = FALSE;
      /* See if it is in the internal fields */
      res = arad_pmf_ce_internal_field_info_find(
                unit,
                pmf_key_construction_info[table_line].qual_type,
                SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
                pmf_key_construction_info[table_line].is_msb,
                &found,
                &qual_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      if (found) {
          /* Same key at both lookup since it is much more simple */
          for (pmf_key = 0; pmf_key <= ARAD_PMF_KEY_D; pmf_key ++) {
              if ((1 << pmf_key) & pmf_key_construction_info[table_line].key_supported_bitmap) {
                  res = arad_pmf_ce_internal_info_entry_set_unsafe(
                            unit,
                            SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
                            pmf_pgm_ndx,
                            pmf_key,
                            pmf_key_construction_info[table_line].ce_id,
                            pmf_key_construction_info[table_line].is_msb,
                            is_second_lookup,
                            FALSE, /* is_update_key */
                            pmf_key_construction_info[table_line].qual_lsb, 
                            0, /* lost_bits */
                            pmf_key_construction_info[table_line].qual_nof_bits,
                            pmf_key_construction_info[table_line].qual_type
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
              }
          }
          /* Go to the next instruction */
          continue;
      }

      /* See if it is in the header packet */

      found = FALSE;
      res = arad_pmf_ce_header_info_find(
                unit,
                pmf_key_construction_info[table_line].qual_type,
                SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
                &found,
                &header_qual_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

      if (found) {
          ARAD_PMF_CE_PACKET_HEADER_INFO_clear(&ce_packet_header_info);
          ce_packet_header_info.sub_header = header_qual_info.header_ndx_0;
          ce_packet_header_info.offset = header_qual_info.msb + pmf_key_construction_info[table_line].qual_lsb;
          /* Set the number of bits to be the user if defined, otherwise all the field */
          ce_packet_header_info.nof_bits = (pmf_key_construction_info[table_line].qual_nof_bits != 0)? 
              pmf_key_construction_info[table_line].qual_nof_bits: (header_qual_info.lsb - header_qual_info.msb + 1);

          /* Same key at both lookup since it is much more simple */
          for (pmf_key = 0; pmf_key <= ARAD_PMF_KEY_D; pmf_key ++) {
              if ((1 << pmf_key) & pmf_key_construction_info[table_line].key_supported_bitmap) {
              res = arad_pmf_ce_packet_header_entry_set_unsafe(
                        unit,
                        SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
                        pmf_pgm_ndx,
                        pmf_key,
                        pmf_key_construction_info[table_line].ce_id,
                        pmf_key_construction_info[table_line].is_msb,
                        is_second_lookup,
                        &ce_packet_header_info
                    );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
              }
          }
          /* Go to the next instruction */
          continue;
      }


      if (!found) {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "Invalid Qualifier %d.\n\r"), pmf_key_construction_info[table_line].qual_type));
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_INCORRECT_INSTRUCTION_ERR, 40, exit);
      }
  }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_ce_key_construction_unsafe()", 0, 0);
}


STATIC
uint32
  arad_pmf_low_level_fes_action_construction_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx,
    SOC_SAND_IN  uint32                in_port_profile,
    SOC_SAND_IN  uint32                pmf_fes_construction_info_length,
    SOC_SAND_IN  ARAD_PMF_FES_CONSTR   *pmf_fes_construction_info
  )
{
    uint32
      table_line,    
      res = SOC_SAND_OK;
    ARAD_PMF_FEM_INPUT_INFO
      fem_input_info;
    ARAD_PMF_FES
        fes_sw_db_info;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  ARAD_PMF_FEM_INPUT_INFO_clear(&fem_input_info);
  fem_input_info.is_16_lsb_overridden = FALSE;
  fem_input_info.src_arad.is_key_src = TRUE;
  fem_input_info.src_arad.lookup_cycle_id = 0;

  for (table_line = 0; table_line < pmf_fes_construction_info_length; ++table_line) {
      /* Apply only the correct Program */
      fem_input_info.src_arad.key_tcam_id = pmf_fes_construction_info[table_line].input_key_id;
      fem_input_info.src_arad.key_lsb = pmf_fes_construction_info[table_line].input_key_lsb;
      res = arad_pmf_db_fem_input_set_unsafe(
                unit,
                pmf_pgm_ndx,
                TRUE, /* always FES */
                pmf_fes_construction_info[table_line].fes_id,
                &fem_input_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      res = arad_pmf_db_fes_set_unsafe(
                unit,
                pmf_pgm_ndx,
                pmf_fes_construction_info[table_line].fes_id,
                &(pmf_fes_construction_info[table_line].fes_info)
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

      res = soc_sand_os_memset(&fes_sw_db_info, 0, sizeof(fes_sw_db_info));
      SOC_SAND_CHECK_FUNC_RESULT(res, 99, exit);

      /* Set it in the SW DB also for the ITMH PMF Header Extension */
      fes_sw_db_info.is_used = 1;
      fes_sw_db_info.db_id = (~0); /* No interaction with existing DBs, never removed */
      fes_sw_db_info.action_type = pmf_fes_construction_info[table_line].fes_info.action_type;
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.set(
                unit,
                SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
                pmf_pgm_ndx,
                pmf_fes_construction_info[table_line].fes_id,
                &fes_sw_db_info
              );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_fes_action_construction_unsafe()", 0, 0);
}




uint32
  arad_pmf_low_level_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
      pmf_pgm_ndx,
      reserved_progs,
    res = SOC_SAND_OK;
  ARAD_PMF_PGM_TYPE
    pmf_pgm_type_ndx;
  ARAD_PMF_SEL_INIT_INFO  
      init_info;
  uint8
      is_tm_pmf_per_port_mode = FALSE,
      is_oam_snoop_ftmh_pmf_pgm_to_build = FALSE;
  soc_port_t local_port;


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_INIT_UNSAFE);

  res = arad_mgmt_ihb_tbls_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pmf_low_level_ce_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  res = arad_pmf_low_level_fem_tag_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = arad_pmf_low_level_pgm_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arad_pmf_prog_select_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /* 
   * Set all the possible PMF-Programs - possible 
   * update on the fly with switch control 
   * Do not set the Ethernet program at this stage, but 
   * as the first program of the program selection init 
   */ 
  /* Init the PMF programs for all these header types */
  ARAD_PMF_SEL_INIT_INFO_clear(&init_info);
  /* 
   * See if we are in the TM PMF mode: 8 programs are reserved 
   * for the per-port configuration 
   * See if there is a port with no zero header size 
   */
  for (local_port = 0; (local_port < ARAD_NOF_LOCAL_PORTS(unit)) && (!is_tm_pmf_per_port_mode); local_port++) {
      if (soc_property_port_get((unit), local_port, spn_POST_HEADERS_SIZE,  FALSE)) {
          is_tm_pmf_per_port_mode = TRUE;
      } 
  }

  init_info.pmf_pgm_default[ARAD_PMF_PSL_TYPE_TM] = ARAD_PMF_PROG_SELECT_TM_PMF_PGM_MIN;
  init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM] = 1 + 1 /* Reserved PMF-Program for switching between TM programs */
      + (is_tm_pmf_per_port_mode? 8 /* number of port-profiles */: 0);
  init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_ETH] = init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM];
  reserved_progs = 0;

#ifdef ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED
  if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "egress_snooping_advanced", 0)) {
      /* Build an extra-program in case of OAM snoop Walk-around, FTMH-like */
      is_oam_snoop_ftmh_pmf_pgm_to_build = 1;
  }
#endif /* ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED */

  for (pmf_pgm_type_ndx = 0; pmf_pgm_type_ndx < (ARAD_PMF_PGM_TYPE_TM + is_oam_snoop_ftmh_pmf_pgm_to_build); pmf_pgm_type_ndx++)
  {
      /* Set the Program index to be from 0 */
      pmf_pgm_ndx = ARAD_PMF_PROGRAM_STATIC_INDEX_GET(init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM], ARAD_PMF_PROG_SELECT_TM_PMF_PGM_MIN, pmf_pgm_type_ndx);
#ifdef ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED
      if (pmf_pgm_type_ndx == ARAD_PMF_PGM_TYPE_TM) {
          res = arad_pmf_low_level_stack_pgm_set_unsafe(
                  unit,
                  pmf_pgm_ndx,
                  TRUE /* OAM - WA */
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

      } else
#endif /* ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED */
      {
          res = arad_pmf_low_level_pgm_set(
              unit,
                  pmf_pgm_ndx, 
                  pmf_pgm_type_ndx
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }

      /* reserve line for each PMF-Program */
      ++init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_ETH];
      reserved_progs |= (1 << pmf_pgm_ndx);
  }
  /* set reserved programs */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.progs.set(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, 0, reserved_progs);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);

  /* At this stage, idx indicates the number of static PMF Programs initialized */
  init_info.pmf_pgm_default[ARAD_PMF_PSL_TYPE_ETH] = init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_ETH];
  res = arad_pmf_prog_select_eth_init(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, is_tm_pmf_per_port_mode, &init_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  /* Update mirorr raw profile with ETH PMF configuration 
   * MIRROR_RAW profile can be updated only after ETH PMF program is configured 
   */
  res = arad_pmf_low_level_mirror_raw_pgm_update(unit, init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM]);
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    
  /* No reserved program at egress */
  init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_ETH] = 0;
  init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM] = 0;
  /* 
   * 0 is the default PMF program for all  - not doing anything 
   * To get only TM or Ethernet packets, the user can preselect on the Forwarding-Type (TM vs Any).  
   */
  init_info.pmf_pgm_default[ARAD_PMF_PSL_TYPE_ETH] = init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_ETH]; 
  init_info.pmf_pgm_default[ARAD_PMF_PSL_TYPE_TM] = init_info.nof_reserved_lines[ARAD_PMF_PSL_TYPE_TM];
  res = arad_pmf_prog_select_eth_init(unit, SOC_PPC_FP_DATABASE_STAGE_EGRESS, FALSE, &init_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
      /* Identical init for SLB as at egress for program selection */
      res = arad_pmf_prog_select_eth_init(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB, FALSE, &init_info);
      SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
  }
#endif /* BCM_88660_A0 */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_init_unsafe()", 0, 0);
}

/*
 * TM PMF Program: one function for the static configuration,
 * the other per PMF Program
 */
STATIC
uint32
  arad_pmf_low_level_tm_pgm_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                   oam_test,
    SOC_SAND_IN  uint32                pmf_pgm_ndx
  )
{
  uint32
      fld_val = 0,
	  mask_five_bit = ((1<<5) - 1),
      pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      table_line,
      tm_pfc_idx,
      bit_idx,
      res = SOC_SAND_OK;
  uint32 pmf_key_construction_info_length=0;
  uint32 pmf_fes_construction_info_length=0;
  ARAD_PMF_PGM_INFO
      pgm_info;
  uint8
      in_port_profile;
  soc_reg_above_64_val_t
      reg_data;
   ARAD_PMF_KEY_CONSTR   
      *pmf_key_construction_info = NULL;
  ARAD_PMF_FES_CONSTR   
      *pmf_fes_construction_info = NULL;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_TM_PGM_SET_UNSAFE);

  /* Validate it is for TM packets */
  in_port_profile = ARAD_PMF_PORT_PROFILE_TM;
  if (oam_test) {
      in_port_profile = ARAD_PMF_PORT_PROFILE_TM_OAM_TEST;
  }

  /* 
   * VTT - Set the Offset index correction
   */
  SOC_REG_ABOVE_64_CLEAR(reg_data);
  for (tm_pfc_idx = 0; tm_pfc_idx < mask_five_bit; tm_pfc_idx++) {
      /* TmForwardingOffsetIndexMapping.rvalue[i*3+:3] = i[1:0]+1; */
      fld_val = (tm_pfc_idx & 0x3) + 1;
      SHR_BITCOPY_RANGE(reg_data, (3 * tm_pfc_idx), &fld_val, 0, 3);
  }
  /* TmForwardingOffsetIndexMapping.rvalue[5'b11000*3 +:3] = 2; IHP_PFC_ITMH_OUT_LIF */
  fld_val = 0x2;
  SHR_BITCOPY_RANGE(reg_data, (3 * (ARAD_PARSER_PFC_TM_OUT_LIF & mask_five_bit)), &fld_val, 0, 3);
  /* TmForwardingOffsetIndexMapping.rvalue[5'b10001*3 +:3] = 2; IHP_PFC_ITMH_IS */
  SHR_BITCOPY_RANGE(reg_data, (3 * (ARAD_PARSER_PFC_TM_IS & mask_five_bit)), &fld_val, 0, 3);
  /* TmForwardingOffsetIndexMapping.rvalue[5'b11010*3 +:3] = 1; IHP_PFC_ITMH_MC_FLOW */
  fld_val = 0x1;
  SHR_BITCOPY_RANGE(reg_data, (3 * (ARAD_PARSER_PFC_TM_MC_FLOW & mask_five_bit)), &fld_val, 0, 3);
  res = WRITE_IHP_TM_FORWARDING_OFFSET_INDEX_MAPPINGr(unit, REG_PORT_ANY, reg_data);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 372, exit);


  /* 
   * PMF Program selection
   */
  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  /* Set all ones in the value */
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
      SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }

  /* Set the Value and mask of InPortProfile and PacketFormatCode - others are masked */
  if (in_port_profile != ARAD_PMF_PORT_PROFILE_TM) {
      /* Let the TM PMF application set the Port profile if needed */
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, in_port_profile);
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3b valid mask */
  }
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, ARAD_PARSER_PFC_TM);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F); /* Only MSB valid mask */

  /* Valid it and set the program */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  if(SOC_IS_JERICHO_PLUS(unit)) {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, pmf_pgm_ndx); /* PMF-Program index */
  }
  else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, pmf_pgm_ndx); /* PMF-Program index */
  }

  /* Fill from the end to simplify the FP dynamic work */
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
      pgm_sel_tbl_data
      );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);


  /* when oam statistics feature is enabled we need to set the counter to be the LEM result 
      in case of oam down mep and bfd packets which are TM packets*/
  if ((SOC_DPP_CONFIG(unit)->pp.oam_statistics)&&(soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) == 0)) {

      if (SOC_IS_JERICHO(unit) &&
          !soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0)) {
          /*
           * For this version, we do NOT support static program for Jericho. Eject an error!
           */
          LOG_ERROR(
              BSL_LS_SOC_FP,
              (BSL_META_U(
                  unit,
                  "\n\r"
                  "%s(): The soc property ITMH_PROGRAMMABLE_MODE_ENABLE must be set for Jericho in 'non-Arad' mode.\n\r"
                  "Static programs are NOT supported for Jericho\n\r"
                  ),__FUNCTION__
              )
          );
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_INCORRECT_INSTRUCTION_ERR, 55, exit);
          
      }
      pmf_fes_construction_info_length = (sizeof(Arad_pmf_fes_construction_info_tm) / sizeof(ARAD_PMF_FES_CONSTR));
      ARAD_ALLOC(pmf_fes_construction_info, ARAD_PMF_FES_CONSTR, pmf_fes_construction_info_length+1, "Allocation of stacking PMF FES");
      sal_memcpy(pmf_fes_construction_info, Arad_pmf_fes_construction_info_tm, pmf_fes_construction_info_length * sizeof(ARAD_PMF_FES_CONSTR));

      pmf_key_construction_info_length = (sizeof(Arad_pmf_key_construction_info_tm) / sizeof(ARAD_PMF_KEY_CONSTR));
      ARAD_ALLOC(pmf_key_construction_info, ARAD_PMF_KEY_CONSTR, pmf_key_construction_info_length, "Allocation of stacking PMF key");
      sal_memcpy(pmf_key_construction_info, Arad_pmf_key_construction_info_tm, pmf_key_construction_info_length * sizeof(ARAD_PMF_KEY_CONSTR));

      pmf_key_construction_info[2].qual_type = SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_FOUND;
      pmf_key_construction_info[2].qual_nof_bits = 1;
      pmf_key_construction_info[2].qual_lsb = 0;
      pmf_key_construction_info[2].is_msb = 0; 


      pmf_key_construction_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ONES;
      pmf_key_construction_info[1].qual_nof_bits = 1;
      pmf_key_construction_info[1].qual_lsb = 0;
      pmf_key_construction_info[1].is_msb = 0;

      pmf_key_construction_info[0].qual_type = SOC_PPC_FP_QUAL_LEM_1ST_LOOKUP_RESULT;
      pmf_key_construction_info[0].qual_nof_bits = 16;
      pmf_key_construction_info[0].qual_lsb = 0;
      pmf_key_construction_info[0].is_msb = 0;

      ARAD_PMF_FES_INPUT_INFO_clear(&pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info);

      pmf_fes_construction_info[pmf_fes_construction_info_length].fes_id = 25;
      /* if oam_statistics==1 stat 0 will be used if if oam_statistics==2 stat 1 will be used */
      if (SOC_DPP_CONFIG(unit)->pp.oam_statistics==1){
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_COUNTER;
      }
      if (SOC_DPP_CONFIG(unit)->pp.oam_statistics==2){
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_COUNTER_B;
      }
      pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.is_action_always_valid = FALSE;
      pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.shift = 4; 
      pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_id = ARAD_PMF_KEY_A;
      pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_lsb = ARAD_PMF_KEY_LSB_0;
      pmf_fes_construction_info[pmf_fes_construction_info_length].in_port_profile = 2;

      /*
       *    1. Copy Engine Key construction -
       *    set all the instructions according to the table
       */
      res = arad_pmf_low_level_ce_key_construction_unsafe(
          unit,
          pmf_pgm_ndx,
          in_port_profile,
          pmf_key_construction_info_length,
          pmf_key_construction_info
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);


      /* FES actions */
      res = arad_pmf_low_level_fes_action_construction_unsafe(
          unit,
          pmf_pgm_ndx,
          in_port_profile,
          pmf_fes_construction_info_length + 1,
          pmf_fes_construction_info
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  }

  if ((soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) == 0) && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0)==0)) {
      /*
       *    1. Copy Engine Key construction -
       *    set all the instructions according to the table
       *    On Arad, use:
       *      Arad_pmf_key_construction_info_tm and Arad_pmf_fes_construction_info_tm
       *    On Jericho, usage depends on ITMH_ARAD_MODE_ENABLE:
       *      If ITMH_ARAD_MODE_ENABLE is set, use:
       *        JerichoAsArad_pmf_key_construction_info_tm, JerichoAsArad_pmf_fes_construction_info_tm
       *      If ITMH_ARAD_MODE_ENABLE is Not set, then SW rejects that combination. If that combination is
       *      enabled, we would use:
       *        Jericho_pmf_key_construction_info_tm, Jericho_pmf_fes_construction_info_tm
       */
      ARAD_PMF_KEY_CONSTR
          *pmf_key_construction_info ;
      uint32
          pmf_key_construction_info_length ;
      ARAD_PMF_FES_CONSTR
          *pmf_fes_construction_info ;
      uint32
          pmf_fes_construction_info_length ;
      if (SOC_IS_JERICHO(unit)) {
          if (!soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0)) {
              /*
               * Jericho ITMH header on Jericho
               *
               * For this version, we do NOT support static program for Jericho. Eject an error!
               */
              LOG_ERROR(
                  BSL_LS_SOC_FP,
                  (BSL_META_U(
                      unit,
                      "\n\r"
                      "%s(): The soc property ITMH_PROGRAMMABLE_MODE_ENABLE must be set for Jericho in 'non-Arad' mode.\n\r"
                      "Static programs are NOT supported for Jericho\n\r"
                      ),__FUNCTION__
                  )
              );
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_INCORRECT_INSTRUCTION_ERR, 40, exit) ;
              /*
               * This is dead code which should be revived if this setting (static program for Jericho) comes
               * to life again.
               */
#if (0)
/* { */
              pmf_key_construction_info = Jericho_pmf_key_construction_info_tm ;
              pmf_key_construction_info_length =
                  sizeof(Jericho_pmf_key_construction_info_tm) / sizeof(Jericho_pmf_key_construction_info_tm[0]) ;
              pmf_fes_construction_info = Jericho_pmf_fes_construction_info_tm ;
              pmf_fes_construction_info_length =
                  sizeof(Jericho_pmf_fes_construction_info_tm) / sizeof(Jericho_pmf_fes_construction_info_tm[0]) ;
/* } */
#endif
          } else {
              /*
               * Arad ITMH header on Jericho
               */
              pmf_key_construction_info = (ARAD_PMF_KEY_CONSTR *)JerichoAsArad_pmf_key_construction_info_tm ;
              pmf_key_construction_info_length =
                  sizeof(JerichoAsArad_pmf_key_construction_info_tm) / sizeof(JerichoAsArad_pmf_key_construction_info_tm[0]) ;
              pmf_fes_construction_info = (ARAD_PMF_FES_CONSTR *)JerichoAsArad_pmf_fes_construction_info_tm ;
              pmf_fes_construction_info_length =
                  sizeof(JerichoAsArad_pmf_fes_construction_info_tm) / sizeof(JerichoAsArad_pmf_fes_construction_info_tm[0]) ;
          }
      } else {
          /*
           * Arad ITMH header on Arad
           */
          pmf_key_construction_info = (ARAD_PMF_KEY_CONSTR *)Arad_pmf_key_construction_info_tm ;
          pmf_key_construction_info_length =
              sizeof(Arad_pmf_key_construction_info_tm) / sizeof(Arad_pmf_key_construction_info_tm[0]) ;
          pmf_fes_construction_info = (ARAD_PMF_FES_CONSTR *)Arad_pmf_fes_construction_info_tm ;
          pmf_fes_construction_info_length =
              sizeof(Arad_pmf_fes_construction_info_tm) / sizeof(Arad_pmf_fes_construction_info_tm[0]) ;
      }

      if (SOC_DPP_CONFIG(unit)->pp.oam_statistics == 0) {
          res = arad_pmf_low_level_ce_key_construction_unsafe(
              unit,
              pmf_pgm_ndx,
              in_port_profile,
              pmf_key_construction_info_length,
              pmf_key_construction_info
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

          /* FES actions */
          res = arad_pmf_low_level_fes_action_construction_unsafe(
              unit,
              pmf_pgm_ndx,
              in_port_profile,
              pmf_fes_construction_info_length,
              pmf_fes_construction_info
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

      }

  }

  /*
   *    3. Program properties
   */
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
      unit,
      pmf_pgm_ndx,
      &pgm_info
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_POST_FWDING; /* As HW */
  pgm_info.bytes_to_rmv.nof_bytes = 0; /* The ITMH */
  /* if (pp_port_info->first_header_size != 0) - The real number of bytes to remove is set per PP-Port */

  /* ProgramKeyGenVar */
  pgm_info.copy_pgm_var = ARAD_PMF_DESTINATION_MULTICAST_ID_PREFIX + (ARAD_PMF_DESTINATION_FLOW_ID_PREFIX << 4)
                            + (ARAD_PMF_DESTINATION_SYSTEM_PORT_PREFIX << ARAD_PMF_TM_PGM_SYSTEM_PORT_OFFSET); 
  /* pgm_info.fc_type = pp_port_info->fc_type; - The real Flow Control type is set per PP-Port in the header profile index */

  /* Set the snoop base according to the TIMNA traps */
  pgm_info.copy_pgm_var += (((0x3 << 4 /* Snoop stength */) + (SOC_PPC_TRAP_CODE_INTERNAL_IHP_TIMNA_PREFIX >> 4 /* Prefix base */)) 
                                << ARAD_PMF_SNOOP_TRAP_ID_BASE_OFFSET);

  pgm_info.header_type = ARAD_PORT_HEADER_TYPE_TM;
  pgm_info.header_profile = 11; 
  res = arad_pmf_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  ARAD_FREE(pmf_fes_construction_info);
  ARAD_FREE(pmf_key_construction_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_tm_pgm_set_unsafe()", 0, 0);
}

/*
 * raw PMF Program (not complete)
 */
STATIC
uint32
  arad_pmf_low_level_raw_pgm_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 pmf_pgm_ndx
  )
{
  uint32
    table_line,
    bit_idx,
    pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      res = SOC_SAND_OK;
  ARAD_PMF_PGM_INFO
      pgm_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_RAW_PGM_SET_UNSAFE);
  /* 
   * PMF Program selection
   */

  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  /* Set all ones in the value */
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
      SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }

  /* Set the Value and mask of InPortProfile and PacketFormatCode - others are masked */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, ARAD_PMF_PORT_PROFILE_RAW); 
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3'b000 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, ARAD_PARSER_PFC_RAW_AND_FTMH);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F); /* Only MSB valid mask */ /*6'b01_1111*/
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PARSER_LEAF_CONTEXTf, ARAD_PARSER_PLC_RAW);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PARSER_LEAF_CONTEXTf, 0x0);

  /* Valid it and set the program */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  if(SOC_IS_JERICHO_PLUS(unit))
  {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, pmf_pgm_ndx); /* PMF-Program index */
  } else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, pmf_pgm_ndx); /* PMF-Program index */
  }

  /* Fill from the end to simplify the FP dynamic work */
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
          pgm_sel_tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);


  /*
   *    1. Copy Engine Key construction -
   *    set all the instructions according to the table
   */

  /* 
   *   2. FES actions
   */

  /*
   *    3. Program properties
   */
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_FWDING; /* 2'b10 */
  pgm_info.bytes_to_rmv.nof_bytes = 0; 
  pgm_info.header_profile = 9;

  res = arad_pmf_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_raw_pgm_set_unsafe()", 0, 0);
}

/*
 * Mirror raw PMF Program
 */
STATIC
uint32
  arad_pmf_low_level_mirror_raw_pgm_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 pmf_pgm_ndx
  )
{
  uint32
    table_line,
    bit_idx,
    pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      res = SOC_SAND_OK;
  ARAD_PMF_PGM_INFO
      pgm_info;
    ARAD_PMF_PSL_LEVEL_INFO
      dflt_level_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_RAW_PGM_SET_UNSAFE);
  /* 
   * PMF Program selection
   */

  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  /* Set all ones in the value */
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
      SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }

  /* Set the Value and mask of InPortProfile and PacketFormatCode - others are masked */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, ARAD_PMF_PORT_PROFILE_MIRROR_RAW); 
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3'b000 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, ARAD_PARSER_PFC_RAW_AND_FTMH);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F); /* Only MSB valid mask */ /*6'b01_1111*/
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PARSER_LEAF_CONTEXTf, ARAD_PARSER_PLC_RAW);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PARSER_LEAF_CONTEXTf, 0x0);

  /* Valid it and set the program */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.levels_info.get(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF /*stage*/, 0x0 /*is_tm*/, 0, &dflt_level_info);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);


  if(SOC_IS_JERICHO_PLUS(unit)){
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, dflt_level_info.lines[0].prog_id /*pmf_pgm_ndx*/); /* PMF-Program index */
  } else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, dflt_level_info.lines[0].prog_id /*pmf_pgm_ndx*/); /* PMF-Program index */
  }

  /* Fill from the end to simplify the FP dynamic work */
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
          pgm_sel_tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);


  /*
   *    1. Copy Engine Key construction -
   *    set all the instructions according to the table
   */

  /* 
   *   2. FES actions
   */

  /*
   *    3. Program properties
   */
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_1ST;
  pgm_info.fc_type = SOC_TMC_PORTS_FC_TYPE_NONE;
  pgm_info.header_profile = ARAD_PMF_PGM_HEADER_PROFILE_ETHERNET;

  res = arad_pmf_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_mirror_raw_pgm_set_unsafe()", 0, 0);
}

uint32 arad_pmf_low_level_mirror_raw_pgm_update(int unit, int nof_reserved_lines)
{
    uint32
        table_line,
        pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
        res = SOC_SAND_OK;
    ARAD_PMF_PSL_LEVEL_INFO dflt_level_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_RAW_PGM_SET_UNSAFE);

    table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) 
                                        - ARAD_PMF_PROGRAM_STATIC_INDEX_GET(nof_reserved_lines, ARAD_PMF_PROG_SELECT_TM_PMF_PGM_MIN, ARAD_PMF_PGM_TYPE_MIRROR_RAW) 
                                        - 1;
    res = soc_mem_read(unit, IHB_PMF_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_line, &pgm_sel_tbl_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.levels_info.get(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF /*stage*/, 0x0 /*is_tm*/, 0, &dflt_level_info);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    
    if(SOC_IS_JERICHO_PLUS(unit)) {
        soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, dflt_level_info.lines[0].prog_id /*pmf_pgm_ndx*/); /* PMF-Program index */
    } else {
        soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, dflt_level_info.lines[0].prog_id /*pmf_pgm_ndx*/); /* PMF-Program index */
    }
    
    /* Fill from the end to simplify the FP dynamic work */
    res = soc_mem_write(unit, IHB_PMF_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_line, pgm_sel_tbl_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_mirror_raw_pgm_set_unsafe()", 0, 0);
}

/*
 * stacking PMF Program
 */
uint32
  arad_pmf_low_level_stack_pgm_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx,
    SOC_SAND_IN  uint8              is_for_oam_snoop_was
  )
{
  uint32
      pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      table_line,
      bit_idx,
      headers_mode,
      pmf_key_construction_info_length,
      pmf_fes_construction_info_length,
      nof_fes_actions_skip_no_stacking = 0,
      nof_fes_actions_skip_no_oam_snoop,
      line_ndx,
      line_min_ndx,
      res = SOC_SAND_OK;
  ARAD_PMF_PGM_INFO
      pgm_info;
  ARAD_PMF_KEY_CONSTR   
      *pmf_key_construction_info = NULL;
  ARAD_PMF_FES_CONSTR   
      *pmf_fes_construction_info = NULL;
  soc_reg_t
    global_f = SOC_IS_JERICHO(unit)? ECI_GLOBAL_SYS_HEADER_CFGr: ECI_GLOBALFr;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* 
   * PMF Program selection
   */
  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  /* Set all ones in the value */
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
      SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }

  /* Set the Value and mask of InPortProfile and PacketFormatCode - others are masked */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, ARAD_PMF_PORT_PROFILE_FTMH); /* FTMH */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3'b000 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, 0x20); /*6'b100000*/
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F);  /*6'b011111*/

  /* Valid it and set the program */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  if(SOC_IS_JERICHO_PLUS(unit))
  {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, pmf_pgm_ndx); /* PMF-Program index (should be 1 - FTMH)*/
  } else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, pmf_pgm_ndx); /* PMF-Program index (should be 1 - FTMH)*/
  }

  /* Fill from the end to simplify the FP dynamic work */
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
          pgm_sel_tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

  /* Select according to Petra mode */
  line_min_ndx = 0;
  /* TM custom protocols for ingress shaping {*/
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, global_f, REG_PORT_ANY, 0, SYSTEM_HEADERS_MODEf, &headers_mode));
  if (headers_mode == ARAD_PP_SYSTEM_HEADERS_MODE_ARAD || headers_mode == ARAD_PP_SYSTEM_HEADERS_MODE_JERICHO)
  {

     /* ARAD mode */
      pmf_key_construction_info_length = (sizeof(Arad_pmf_key_construction_info_stacking) / sizeof(ARAD_PMF_KEY_CONSTR));
      ARAD_ALLOC(pmf_key_construction_info, ARAD_PMF_KEY_CONSTR, pmf_key_construction_info_length, "Allocation of stacking PMF key");
      sal_memcpy(pmf_key_construction_info, Arad_pmf_key_construction_info_stacking, pmf_key_construction_info_length * sizeof(ARAD_PMF_KEY_CONSTR));

      if (SOC_IS_JERICHO(unit)) {
          /* Invert the Destination-System-Port encoding in Jericho from 100 to 001 */
          for (line_ndx = 0; line_ndx < pmf_key_construction_info_length; line_ndx++) {
              if ((pmf_key_construction_info[line_ndx].is_msb == 1) && (pmf_key_construction_info[line_ndx].ce_id == 0)) {
                  pmf_key_construction_info[line_ndx].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
                  pmf_key_construction_info[line_ndx].qual_nof_bits = 2;
              }
              else if ((pmf_key_construction_info[line_ndx].is_msb == 1) && (pmf_key_construction_info[line_ndx].ce_id == 1)) {
                  pmf_key_construction_info[line_ndx].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ONES;
                  pmf_key_construction_info[line_ndx].qual_nof_bits = 1;
              }
              else if ((pmf_key_construction_info[line_ndx].is_msb == 0) && (pmf_key_construction_info[line_ndx].ce_id == 2)) {
                  pmf_key_construction_info[line_ndx].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
              }
          }
      }


      if ((SOC_DPP_CONFIG(unit)->pp.oam_statistics) && !(is_for_oam_snoop_was)) {

          /* oam statistics - rx up mep (must be MSB qual_oam_id is only msb in arad plus)*/
          pmf_key_construction_info[18].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ONES;
          pmf_key_construction_info[18].qual_nof_bits = 2;
          pmf_key_construction_info[18].qual_lsb = 0;
          pmf_key_construction_info[18].key_supported_bitmap = SOC_SAND_BIT(ARAD_PMF_KEY_B);

          /* Arad+ counter id:
           *                   oam-id bits 0-13
           *                   direction(rx) bit 14
           * Jericho:
           *                   direction(rx) bit 0
           *                   oam-id bits 1-14
           */
          if(SOC_IS_JERICHO(unit))
          {
              pmf_key_construction_info[17].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ONES;
              pmf_key_construction_info[17].qual_nof_bits = 1;
              pmf_key_construction_info[17].qual_lsb = 0;

              pmf_key_construction_info[16].qual_type = SOC_PPC_FP_QUAL_TRAP_QUALIFIER_FHEI;
              pmf_key_construction_info[16].qual_nof_bits = 13;
              pmf_key_construction_info[16].qual_lsb = 0;
          } else {
              pmf_key_construction_info[17].qual_type = SOC_PPC_FP_QUAL_TRAP_QUALIFIER_FHEI;
              pmf_key_construction_info[17].qual_nof_bits = 13;
              pmf_key_construction_info[17].qual_lsb = 0;

              pmf_key_construction_info[16].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ONES;
              pmf_key_construction_info[16].qual_nof_bits = 1;
              pmf_key_construction_info[16].qual_lsb = 0;
          }

          /* for up mep miiror command on a port extender the port extender byte should not be removed*/

          pmf_key_construction_info[15].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
          pmf_key_construction_info[15].qual_nof_bits = 8;
          pmf_key_construction_info[15].qual_lsb = 0;

          pmf_fes_construction_info_length = (sizeof(Arad_pmf_fes_construction_info_stacking) / sizeof(ARAD_PMF_FES_CONSTR));
          ARAD_ALLOC(pmf_fes_construction_info, ARAD_PMF_FES_CONSTR, pmf_fes_construction_info_length + 2, "Allocation of stacking PMF FES");
          sal_memcpy(pmf_fes_construction_info, Arad_pmf_fes_construction_info_stacking, pmf_fes_construction_info_length * sizeof(ARAD_PMF_FES_CONSTR));

          ARAD_PMF_FES_INPUT_INFO_clear(&pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info);

          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_id = 24;
          /* if oam_statistics==1 stat 0 will be used if if oam_statistics==2 stat 1 will be used */
          if (SOC_DPP_CONFIG(unit)->pp.oam_statistics==1){
              pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_COUNTER;
          }
          if (SOC_DPP_CONFIG(unit)->pp.oam_statistics==2){
              pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_COUNTER_B;
          }
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.is_action_always_valid = FALSE;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.shift = 0;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_id = ARAD_PMF_KEY_B;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_lsb = ARAD_PMF_KEY_LSB_80;
          pmf_fes_construction_info[pmf_fes_construction_info_length].in_port_profile = 2;

          pmf_fes_construction_info_length++;

          ARAD_PMF_FES_INPUT_INFO_clear(&pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info);

          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_id = 25;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.is_action_always_valid = TRUE;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.shift = 16;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_id = ARAD_PMF_KEY_B;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_lsb = ARAD_PMF_KEY_LSB_80;
          pmf_fes_construction_info[pmf_fes_construction_info_length].in_port_profile = 2;
          pmf_fes_construction_info_length++;

      } else if (((SOC_DPP_CONFIG(unit)->pp.port_extender_map_lc_exists) == 1) && !(is_for_oam_snoop_was)) {

          pmf_key_construction_info[15].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
          pmf_key_construction_info[15].qual_nof_bits = 8;
          pmf_key_construction_info[15].qual_lsb = 0;

          pmf_fes_construction_info_length = (sizeof(Arad_pmf_fes_construction_info_stacking) / sizeof(ARAD_PMF_FES_CONSTR));
          ARAD_ALLOC(pmf_fes_construction_info, ARAD_PMF_FES_CONSTR, pmf_fes_construction_info_length + 1, "Allocation of stacking PMF FES");
          sal_memcpy(pmf_fes_construction_info, Arad_pmf_fes_construction_info_stacking, pmf_fes_construction_info_length * sizeof(ARAD_PMF_FES_CONSTR));

          ARAD_PMF_FES_INPUT_INFO_clear(&pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info);

          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_id = 25;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.is_action_always_valid = TRUE;
          pmf_fes_construction_info[pmf_fes_construction_info_length].fes_info.shift = 16;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_id = ARAD_PMF_KEY_B;
          pmf_fes_construction_info[pmf_fes_construction_info_length].input_key_lsb = ARAD_PMF_KEY_LSB_80;
          pmf_fes_construction_info[pmf_fes_construction_info_length].in_port_profile = 2;
          pmf_fes_construction_info_length++;

      } else {
          pmf_fes_construction_info_length = (sizeof(Arad_pmf_fes_construction_info_stacking) / sizeof(ARAD_PMF_FES_CONSTR));
          ARAD_ALLOC(pmf_fes_construction_info, ARAD_PMF_FES_CONSTR, pmf_fes_construction_info_length, "Allocation of stacking PMF FES");
          sal_memcpy(pmf_fes_construction_info, Arad_pmf_fes_construction_info_stacking, pmf_fes_construction_info_length * sizeof(ARAD_PMF_FES_CONSTR));
      }




      nof_fes_actions_skip_no_stacking = 7;
      nof_fes_actions_skip_no_oam_snoop = 1;
#ifdef ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED
      if (is_for_oam_snoop_was && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "egress_snooping_advanced", 0))) {
          /* Enable the first FES */
          line_min_ndx = 1;
          nof_fes_actions_skip_no_oam_snoop = 0;
          pmf_key_construction_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FTMH; 
          pmf_key_construction_info[4].qual_nof_bits = 16;
          pmf_key_construction_info[4].qual_lsb = 52;

          pmf_key_construction_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_DSP_EXTENSION_AFTER_FTMH;
          pmf_key_construction_info[3].qual_nof_bits = 16;
          pmf_key_construction_info[3].qual_lsb = 0;
      }
#endif /* ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED */

      
  }
  else {
      /* Petra mode */
      pmf_key_construction_info_length = (sizeof(Arad_pmf_key_construction_info_stacking_petra_mode) / sizeof(ARAD_PMF_KEY_CONSTR));
      ARAD_ALLOC(pmf_key_construction_info, ARAD_PMF_KEY_CONSTR, pmf_key_construction_info_length, "Allocation of stacking PMF key");
      sal_memcpy(pmf_key_construction_info, Arad_pmf_key_construction_info_stacking_petra_mode, pmf_key_construction_info_length * sizeof(ARAD_PMF_KEY_CONSTR));

      pmf_fes_construction_info_length = (sizeof(Arad_pmf_fes_construction_info_stacking_petra_mode) / sizeof(ARAD_PMF_FES_CONSTR));
      ARAD_ALLOC(pmf_fes_construction_info, ARAD_PMF_FES_CONSTR, pmf_fes_construction_info_length, "Allocation of stacking PMF FES");
      sal_memcpy(pmf_fes_construction_info, Arad_pmf_fes_construction_info_stacking_petra_mode, pmf_fes_construction_info_length * sizeof(ARAD_PMF_FES_CONSTR));

      nof_fes_actions_skip_no_stacking = 6;
      nof_fes_actions_skip_no_oam_snoop = 0;
  }
  /* Do not skip if stacking enabled */
  if ((SOC_DPP_CONFIG((unit))->arad->init.ports.is_stacking_system)) {
      nof_fes_actions_skip_no_stacking = 0;
  }
  for (line_ndx = line_min_ndx; line_ndx < nof_fes_actions_skip_no_stacking + nof_fes_actions_skip_no_oam_snoop; line_ndx++) {
      /* invalidate the action */
      pmf_fes_construction_info[line_ndx].fes_info.action_type = SOC_PPC_FP_ACTION_TYPE_NOP;
  }


  

  /*
   *    1. Copy Engine Key construction -
   *    set all the instructions according to the table
   */
  res = arad_pmf_low_level_ce_key_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_FTMH,
            pmf_key_construction_info_length,
            pmf_key_construction_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  

  /* 
   *   2. FES actions
   */
  res = arad_pmf_low_level_fes_action_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_FTMH,
            pmf_fes_construction_info_length,
            pmf_fes_construction_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  

  /*
   *    3. Program properties
   */
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_1ST; /* As HW */
  pgm_info.bytes_to_rmv.nof_bytes = 0; 
  pgm_info.header_profile = ARAD_PMF_PGM_HEADER_PROFILE_STACKING;
#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
  /* ProgramKeyGenVar */
  pgm_info.copy_pgm_var = (ARAD_PMF_HEADER_PROFILE_CPU_TO_CPU_FTMH_RESERVED << ARAD_PMF_CPU_TO_CPU_HEADER_PROFILE_OFFSET);
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */

  res = arad_pmf_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  ARAD_FREE(pmf_key_construction_info);
  ARAD_FREE(pmf_fes_construction_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_stack_pgm_set_unsafe()", 0, 0);
}



/*
 * XGS HIGIG PMF Program
 */
STATIC
uint32
  arad_pmf_low_level_xgs_pgm_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx
  )
{
  uint32
      pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      line_ndx,
      pmf_key_construction_info_length,
      pmf_fes_info_length,
      table_line,
      bit_idx,
      res = SOC_SAND_OK;
  ARAD_PMF_PGM_INFO
      pgm_info;
  ARAD_PMF_KEY_CONSTR *pmf_key_construction_info = NULL;
  CONST ARAD_PMF_FES_CONSTR *pmf_fes_construction_info = NULL;

 
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* 
   * PMF Program selection
   */
  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  /* Set all ones in the value */
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
      SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }

  /* Set the Value and mask of InPortProfile and PacketFormatCode - others are masked */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, ARAD_PMF_PORT_PROFILE_XGS_TM); /* XGS TM profile */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3'b000 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, ARAD_PARSER_PFC_RAW_AND_FTMH);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F); /* Only MSB valid mask */ /*6'b01_1111*/
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PARSER_LEAF_CONTEXTf, ARAD_PARSER_PLC_RAW);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PARSER_LEAF_CONTEXTf, 0x0);

  /* Valid it and set the program */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  if(SOC_IS_JERICHO_PLUS(unit))
  {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, pmf_pgm_ndx); /* PMF-Program index */
  } else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, pmf_pgm_ndx); /* PMF-Program index */
  }

  /* Fill from the end to simplify the FP dynamic work */
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
          pgm_sel_tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

  /*
   *    1. Copy Engine Key construction -
   *    set all the instructions according to the table
   */
  if (SOC_DPP_CONFIG((unit))->arad->xgs_compatability_tm_system_port_encoding == SOC_DPP_XGS_TM_7_MODID_8_PORT ) 
  {
      pmf_key_construction_info_length = (sizeof(Arad_pmf_key_construction_info_xgs_7_modid_8_port) / sizeof(ARAD_PMF_KEY_CONSTR));
      ARAD_ALLOC(pmf_key_construction_info, ARAD_PMF_KEY_CONSTR, pmf_key_construction_info_length, "Allocation of stacking PMF key");
      sal_memcpy(pmf_key_construction_info, Arad_pmf_key_construction_info_xgs_7_modid_8_port, pmf_key_construction_info_length * sizeof(ARAD_PMF_KEY_CONSTR));
  } 
  else
  { 
    /* xgs_compatability_tm_system_port_encoding = 1 */
      pmf_key_construction_info_length = (sizeof(Arad_pmf_key_construction_info_xgs_8_modid_7_port) / sizeof(ARAD_PMF_KEY_CONSTR));
      ARAD_ALLOC(pmf_key_construction_info, ARAD_PMF_KEY_CONSTR, pmf_key_construction_info_length, "Allocation of stacking PMF key");
      sal_memcpy(pmf_key_construction_info, Arad_pmf_key_construction_info_xgs_8_modid_7_port, pmf_key_construction_info_length * sizeof(ARAD_PMF_KEY_CONSTR));
  }
  
  if (SOC_IS_JERICHO(unit)) {
      /* Invert the Destination-System-Port encoding in Jericho from 100 to 001 */
      for (line_ndx = 0; line_ndx < pmf_key_construction_info_length; line_ndx++) {
          if (((pmf_key_construction_info[line_ndx].is_msb == 1) && (pmf_key_construction_info[line_ndx].ce_id == 1))
              || ((pmf_key_construction_info[line_ndx].is_msb == 0) && (pmf_key_construction_info[line_ndx].ce_id == 7))
              || ((pmf_key_construction_info[line_ndx].is_msb == 1) && (pmf_key_construction_info[line_ndx].ce_id == 8))) {
              /* Reduce the 3b 0 to one bit 1 for Dest-Port encoding */
              pmf_key_construction_info[line_ndx].qual_nof_bits = 1;
          }
          else if ((pmf_key_construction_info[line_ndx].is_msb == 0) && (pmf_key_construction_info[line_ndx].ce_id == 2)) {
              /* Replace the Multicast encoding from 101-MC(16) to 100-MC(16) */
              pmf_key_construction_info[line_ndx].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
          }
      }
  }

  res = arad_pmf_low_level_ce_key_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_RAW,
            pmf_key_construction_info_length,
            pmf_key_construction_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 72, exit);

  /* 
   *   2. FES actions
   */
  if (SOC_IS_JERICHO(unit)) {
      pmf_fes_info_length = sizeof(Jericho_pmf_fes_construction_info_xgs) / sizeof(ARAD_PMF_FES_CONSTR);
      pmf_fes_construction_info = Jericho_pmf_fes_construction_info_xgs;
  } else {
      pmf_fes_info_length = sizeof(Arad_pmf_fes_construction_info_xgs) / sizeof(ARAD_PMF_FES_CONSTR);
      pmf_fes_construction_info = Arad_pmf_fes_construction_info_xgs;
  }
  res = arad_pmf_low_level_fes_action_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_RAW,
            pmf_fes_info_length,
            pmf_fes_construction_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  

  /*
   *    3. Program properties
   */
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_START; /* As HW */
  pgm_info.bytes_to_rmv.nof_bytes = 6; /* The first 6 bytes of the FRC header */ 
  pgm_info.header_profile = 11;

  res = arad_pmf_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  ARAD_FREE(pmf_key_construction_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_xgs_pgm_set_unsafe()", 0, 0);
}


STATIC
uint32
  arad_pmf_low_level_mh_pgm_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx
  )
{
  uint32
      pgm_sel_tbl_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)],
      table_line,
      bit_idx,
      res = SOC_SAND_OK;
  ARAD_PMF_PGM_INFO
      pgm_info;
  uint8
	  in_port_profile;
    soc_port_t 
		local_port;
	 CONST ARAD_PMF_FES_CONSTR* feses  ; 
	 CONST ARAD_PMF_KEY_CONSTR* keys  ;
	int fes_array_size ;
	int key_array_size ;
	SOC_SAND_INIT_ERROR_DEFINITIONS(0);


	feses = Arad_pmf_fes_construction_info_mh ; 
	keys = Arad_pmf_key_construction_info_mh ;
	fes_array_size = sizeof(Arad_pmf_fes_construction_info_mh);
	key_array_size = sizeof(Arad_pmf_key_construction_info_mh); 
    for (local_port = 0; local_port < ARAD_NOF_LOCAL_PORTS(unit) ; local_port++) {
		if (soc_property_port_suffix_num_get(unit, local_port, -1, spn_CUSTOM_FEATURE, "vendor_custom_tm_port", FALSE)){
			feses = Arad_pmf_fes_construction_info_inpHeader ; 
			keys  = Arad_pmf_key_construction_info_inpHeader ;
			fes_array_size = sizeof(Arad_pmf_fes_construction_info_inpHeader);
			key_array_size = sizeof(Arad_pmf_key_construction_info_inpHeader); 
			break ;
		}
	}

  
  in_port_profile = ARAD_PMF_PORT_PROFILE_MH;
  sal_memset(pgm_sel_tbl_data, 0x0, SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS) * sizeof(uint32));
  for (bit_idx = 0; bit_idx < ARAD_PMF_PGM_SEL_TBL_LENGTH_BITS; bit_idx++) {
	  SHR_BITSET(pgm_sel_tbl_data, bit_idx);
  }
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, IN_PORT_PROFILEf, in_port_profile);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_IN_PORT_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PTC_PROFILEf, 0); /* Always PTC-Profile = 0 */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PTC_PROFILEf, 0x0); /* 3b valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PACKET_FORMAT_CODEf, ARAD_PARSER_PFC_TM);
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, MASK_PACKET_FORMAT_CODEf, 0x1F); /* Only MSB valid mask */
  soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, VALIDf, 0x1); /* Valid entry */
  if(SOC_IS_JERICHO_PLUS(unit))
  {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAM_DATAf, pmf_pgm_ndx); /* PMF-Program index */
  } else {
      soc_IHB_PMF_PROGRAM_SELECTION_CAMm_field32_set(unit, pgm_sel_tbl_data, PROGRAMf, pmf_pgm_ndx); /* PMF-Program index */
  }
  table_line = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_program_selection_lines) - pmf_pgm_ndx - 1;
  res = soc_mem_write(
          unit,
          IHB_PMF_PROGRAM_SELECTION_CAMm,
          MEM_BLOCK_ANY,
          table_line,
          pgm_sel_tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
  res = arad_pmf_low_level_ce_key_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_MH,
            (key_array_size / sizeof(ARAD_PMF_KEY_CONSTR)),
            keys
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  res = arad_pmf_low_level_fes_action_construction_unsafe(
            unit,
            pmf_pgm_ndx,
            ARAD_PMF_PORT_PROFILE_MH,
            (fes_array_size / sizeof(ARAD_PMF_FES_CONSTR)),
            feses
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  res = arad_pmf_pgm_get_unsafe(
          unit,
          pmf_pgm_ndx,
          &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
  pgm_info.lkp_profile_id[0] = 0;
  pgm_info.lkp_profile_id[1] = 0;
  pgm_info.tag_profile_id = 0;
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_START; /* As HW */
  pgm_info.bytes_to_rmv.nof_bytes = 0; 
  pgm_info.header_type = ARAD_PORT_HEADER_TYPE_PROG;
  pgm_info.header_profile = 9; /*Add FTMH header as usual*/
  res = arad_pmf_pgm_set_unsafe(
		  unit,
		  pmf_pgm_ndx,
		  &pgm_info
		);
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_mh_pgm_set_unsafe()", 0, 0);
}

STATIC
uint32
  arad_pmf_low_level_eth_pgm_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 pmf_pgm_ndx
  )
{
    uint32
      res = SOC_SAND_OK;
    ARAD_PMF_PGM_INFO          
        pgm_info;


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_LOW_LEVEL_ETH_PGM_SET_UNSAFE);

  ARAD_PMF_PGM_INFO_clear(&pgm_info);
  
  pgm_info.bytes_to_rmv.header_type = ARAD_PMF_PGM_BYTES_TO_RMV_HDR_1ST;
  pgm_info.fc_type = SOC_TMC_PORTS_FC_TYPE_NONE;
  pgm_info.header_profile = ARAD_PMF_PGM_HEADER_PROFILE_ETHERNET;

  /* write system headers for ethernet program  */
  res = arad_pmf_pgm_set_unsafe(
           unit,
           pmf_pgm_ndx,
           &pgm_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_eth_pgm_set_unsafe()", 0, 0);
}

/*
 * Update all the PMF-Programs with this PFG
 */
uint32
  arad_pmf_low_level_pgm_set(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                      pmf_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_PGM_TYPE           pmf_pgm_type
   )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_PGM_MGMT_SET);

  /*
   * Set the PMF Program
   */
  switch (pmf_pgm_type)
  {
  case ARAD_PMF_PGM_TYPE_RAW:
    res = arad_pmf_low_level_raw_pgm_set_unsafe(
            unit,
            pmf_pgm_ndx
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    break;

  case ARAD_PMF_PGM_TYPE_ETH: 
    res = arad_pmf_low_level_eth_pgm_set_unsafe(
            unit,
            pmf_pgm_ndx
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    break;

  case ARAD_PMF_PGM_TYPE_XGS:
    res = arad_pmf_low_level_xgs_pgm_set_unsafe(
            unit,
            pmf_pgm_ndx
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
    break;
      
  case ARAD_PMF_PGM_TYPE_STACK:
      res = arad_pmf_low_level_stack_pgm_set_unsafe(
          unit,
          pmf_pgm_ndx,
          FALSE
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      break;

  case ARAD_PMF_PGM_TYPE_TM:
      if ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0)) == 0) {
          res = arad_pmf_low_level_tm_pgm_set_unsafe(
              unit,
              0, /* OAM test - False until OAM implementation*/
              pmf_pgm_ndx
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }

      break;

  case ARAD_PMF_PGM_TYPE_MH: /* MH profile */
      res = arad_pmf_low_level_mh_pgm_set_unsafe(
                unit,
                pmf_pgm_ndx
                );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
      break;
  case ARAD_PMF_PGM_TYPE_MIRROR_RAW:
    res = arad_pmf_low_level_mirror_raw_pgm_set_unsafe(
            unit,
            pmf_pgm_ndx
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    break;
  default:
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Pmf Program index %d Header type %d - Failed to set program. Header type out of range.\n\r"),
                 unit, pmf_pgm_ndx, pmf_pgm_type));
    SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_PORT_HEADER_TYPE_OUT_OF_RANGE_ERR, 100, exit);
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_pgm_set()", 0, 0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME 

#endif /* of #if defined(BCM_88650_A0) */
