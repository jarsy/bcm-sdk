/* $Id: tmc_api_pmf_low_level_pgm.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_pmf_low_level.h
*
* MODULE PREFIX:  soc_tmcpmf
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

#ifndef __SOC_TMC_API_PMF_LOW_LEVEL_PGM_INCLUDED__
/* { */
#define __SOC_TMC_API_PMF_LOW_LEVEL_PGM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_ports.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of PMF Programs.                                 */
#define  SOC_TMC_PMF_NOF_PGMS (32)

/*     Number of cycles in the PMF process.                    */
#define  SOC_TMC_PMF_NOF_CYCLES (2)

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
   *  Set the program selection parameter table according to
   *  the EEI-or-Out-LIF[15:8]
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_EEI_OUTLIF_15_8 = 0,
  /*
   *  Set the program selection parameter table according to
   *  and EEI-or-Out-LIF[7:0]
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_EEI_OUTLIF_7_0 = 1,
  /*
   *  Set the program selection parameter table according to
   *  SEM-Index[7:0]
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_SEM_NDX_7_0 = 2,
  /*
   *  Set the program selection parameter table according to
   *  the Packet-Format-Qualifier[0][1:0] and SEM-Index[13:8]
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_PFQ_SEM_NDX_13_8 = 3,
  /*
   *  Set the program selection parameter table according to
   *  the Forwarding Code and Tunnel-Termination code
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_FWDING_TTC_CODE = 4,
  /*
   *  Set the program selection parameter table according to:
   *  Large/Small Exact Match First/Second Lookup-Found, LPM
   *  1st/2nd Lookup-Not-Default, TCAM-Found and ELK-Found
   *  parameters
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_LOOKUPS = 5,
  /*
   *  Set the program selection parameter table according to
   *  the Parser-PMF-Profile and the Port-PMF-profile
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_PARSER_PMF_PRO = 6,
  /*
   *  Set the program selection parameter table according to
   *  the LLVP-Incoming-Tag-Structure and the PMF-Profile of
   *  the Packet-Format-Code
   */
  SOC_TMC_PMF_PGM_SEL_TYPE_LLVP_PFC = 7,
  /*
   *  Number of types in SOC_TMC_PMF_PGM_SEL_TYPE
   */
  SOC_TMC_NOF_PMF_PGM_SEL_TYPES = 8
}SOC_TMC_PMF_PGM_SEL_TYPE;

typedef enum
{
  /*
   *  Start of the packet
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR_START = 0,
  /*
   *  First header of the packet
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR_1ST = 1,
  /*
   *  Forwarding header of the packet
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR_FWDING = 2,
  /*
   *  The header next after the forwarding header of the
   *  packet
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR_POST_FWDING = 3,
  /*
   *  Number of types in SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR
   */
  SOC_TMC_NOF_PMF_PGM_BYTES_TO_RMV_HDRS = 4
}SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR;

typedef enum
{
  /*
  *  TM Multicast header profile. 
  */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_TM_MULTICAST = 0,
  /*
   *  TM outlif header profile.
   */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_TM_OUTLIF = 5,
  /*
   *  TM Unicast header profile. 
   */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_TM_UNICAST = 7,
  /*
  *     stacking header profile. 
  */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_STACKING = 8,
  /*
  *  Ethernet header profile.
  */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_ETHERNET = 10,
  /*
   *  Ethernet with DSP stacking header profile.
  */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_ETHERNET_DSP = 12,
  /*
   *  Regular Ethernet, using FTMH and PPH fabric header with Learn Extension.
  */
  SOC_TMC_PMF_PGM_HEADER_PROFILE_ETHERNET_LEARN = 13,
  /* Regular Ethernet, using FTMH with OAM-TS-LATENCY header */ 
  SOC_TMC_PMF_PGM_HEADER_PROFILE_ETHERNET_LATENCY = 14

}SOC_TMC_PMF_PGM_HEADER_PROFILE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Packet-Format-Qualifier[0][1:0]. Range: 0 - 3.
   */
  uint32 pfq;
  /*
   *  SEM-Index[13:8]. Range: 0 - 63.
   */
  uint32 sem_13_8_ndx;

} SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Forwarding Code
   */
  SOC_TMC_PKT_FRWRD_TYPE fwd;
  /*
   *  Tunnel-Termination code. Range: 0 - 15.
   */
  SOC_TMC_TUNNEL_TERM_CODE ttc;

} SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Parser-PMF-Profile. Range: 0 - 15.
   */
  uint32 prsr;
  /*
   *  Port-PMF-profile. Range: 0 - 7.
   */
  uint32 port_pmf;

} SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Large Exact Match First Lookup-Found
   */
  uint8 lem_1st_found;
  /*
   *  Large Exact Match Second Lookup-Found
   */
  uint8 lem_2nd_found;
  /*
   *  Small Exact Match First Lookup-Found
   */
  uint8 sem_1st_found;
  /*
   *  Small Exact Match Second Lookup-Found
   */
  uint8 sem_2nd_found;
  /*
   *  LPM 1st Lookup-Not-Default
   */
  uint8 lpm_1st_not_dflt;
  /*
   *  LPM 2nd Lookup-Not-Default
   */
  uint8 lpm_2nd_not_dflt;
  /*
   *  TCAM-Found
   */
  uint8 tcam_found;
  /*
   *  ELK-Found
   */
  uint8 elk_found;

} SOC_TMC_PMF_PGM_SEL_VAL_LKP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  LLVP-Incoming-Tag-Structure. Range: 0 - 15.
   */
  uint32 llvp;
  /*
   *  PMF-Profile of the Packet-Format-Code. Range: 0 - 7.
   */
  uint32 pmf_pro;

} SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC;

typedef union
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  EEI-or-Out-LIF value[15:8]. Range: 0 - 255.
   */
  uint32 eei_outlif_15_8;
  /*
   *  EEI-or-Out-LIF value[7:0]. Range: 0 - 255.
   */
  uint32 eei_outlif_7_0;
  /*
   *  Sem-Index[7:0]. Range: 0 - 255.
   */
  uint32 sem_7_0_ndx;
  /*
   *  Packet-Format-Qualifier[0][1:0] and SEM-Index[13:8]
   */
  SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM pfq_sem;
  /*
   *  Forwarding Code and Tunnel-Termination code
   */
  SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC fwd_ttc;
  /*
   *  Parser-PMF-Profile and Port-PMF-profile
   */
  SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF prsr_pmf;
  /*
   *  Lookup parameters
   */
  SOC_TMC_PMF_PGM_SEL_VAL_LKP lkp;
  /*
   *  LLVP-Incoming-Tag-Structure and PMF-Profile of the
   *  Packet-Format-Code
   */
  SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC llvp_pfc;

} SOC_TMC_PMF_PGM_SEL_VAL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Type of the Program selection entry.
   */
  SOC_TMC_PMF_PGM_SEL_TYPE type;
  /*
   *  Value of the Program selection entry. The sub-structure
   *  is selected according to the 'type' value.
   */
  SOC_TMC_PMF_PGM_SEL_VAL val;

} SOC_TMC_PMF_PGM_SELECTION_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the considered program is valid for this
   *  entry.
   */
  uint8 is_pgm_valid[SOC_TMC_PMF_NOF_PGMS];

} SOC_TMC_PMF_PGM_VALIDITY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Header base for the byte removal
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR header_type;
  /*
   *  Number of bytes to remove. Range: 0 - 31. (Soc_petra-B)
   */
  uint32 nof_bytes;

} SOC_TMC_PMF_PGM_BYTES_TO_RMV;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup-Profile-Ids for the program (one per cycle).
   *  Range: 0 - 7. (Soc_petra-B only)
   */
  uint32 lkp_profile_id[SOC_TMC_PMF_NOF_CYCLES];
  /*
   *  Tag-Profile Index. Range: 0 - 7. (Soc_petra-B only)
   */
  uint32 tag_profile_id;
  /*
   *  The Header type sets which system headers (FTMH, PPH)
   *  are part of the packet at the end of the ingress
   *  processing
   */
  SOC_TMC_PORT_HEADER_TYPE header_type;
  /*
   *  Specify how to remove bytes from header: which header
   *  and how many bytes
   */
  SOC_TMC_PMF_PGM_BYTES_TO_RMV bytes_to_rmv;
  /*
   *  Copy-Program-Variable. Field of the IRPP info, it can be
   *  used in the Copy Engine output vector
   *  construction. Range: 0 - 255. (Soc_petra-B), 0 - 0xFFFFFFFF. (Arad)
   */
  uint32 copy_pgm_var;
  /*
   *  Flow-Control type (None is SOC_TMC_PORTS_NOF_FC_TYPES)
   */
  SOC_TMC_PORTS_FC_TYPE fc_type;
  /*
   *  Header Profile
   */
  int header_profile;

} SOC_TMC_PMF_PGM_INFO;


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
  SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_LKP_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL_LKP *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_clear(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_TYPE sel_type,
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SEL_VAL *info
  );

void
  SOC_TMC_PMF_PGM_SELECTION_ENTRY_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_SELECTION_ENTRY *info
  );

void
  SOC_TMC_PMF_PGM_VALIDITY_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_VALIDITY_INFO *info
  );

void
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_BYTES_TO_RMV *info
  );

void
  SOC_TMC_PMF_PGM_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_PGM_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_PMF_PGM_SEL_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_TYPE enum_val
  );

const char*
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_BYTES_TO_RMV_HDR enum_val
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL_PFQ_SEM *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL_FWD_TTC *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL_PRSR_PMF *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_LKP_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL_LKP *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL_LLVP_PFC *info
  );

void
  SOC_TMC_PMF_PGM_SEL_VAL_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SEL_VAL *info
  );

void
  SOC_TMC_PMF_PGM_SELECTION_ENTRY_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_SELECTION_ENTRY *info
  );

void
  SOC_TMC_PMF_PGM_VALIDITY_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_VALIDITY_INFO *info
  );

void
  SOC_TMC_PMF_PGM_BYTES_TO_RMV_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_BYTES_TO_RMV *info
  );

void
  SOC_TMC_PMF_PGM_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_PGM_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_PMF_LOW_LEVEL_PGM_INCLUDED__*/
#endif

