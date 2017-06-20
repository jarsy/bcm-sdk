/* $Id: arad_pp_oamp_pe.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_OAMP_PE_INCLUDED__
/* { */
#define __ARAD_PP_OAMP_PE_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* Following is definition for MEP_PE_PROFILE bits offset in Egress Injection and CCM Counting features*/
#define ARAD_PP_OAMP_MEP_PE_PROFILE_OUTLIF_OFFSET    0
#define ARAD_PP_OAMP_MEP_PE_PROFILE_SLM_OFFSET       2
#define ARAD_PP_OAMP_MEP_PE_PROFILE_MAID48_OFFSET    3

typedef enum
{
    ARAD_PP_OAMP_PE_PROGS_DEFAULT, /*Default null program. Always loaded, must be first.*/
    ARAD_PP_OAMP_PE_PROGS_DEFAULT_CCM, /*Default program for CCM packets*/
    ARAD_PP_OAMP_PE_PROGS_OAMP_SERVER,
    ARAD_PP_OAMP_PE_PROGS_OAMP_SERVER_JER,/* Server program for Jericho*/
    ARAD_PP_OAMP_PE_PROGS_PPH_ADD_UDH_JER, /* Adds a UDH after the system headers for Jericho. PPH and OAM-TS are optional*/
    ARAD_PP_OAMP_PE_PROGS_OAMP_SERVER_CCM_JER, /*Server program for CCM packets*/
    ARAD_PP_OAMP_PE_PROGS_1DM, /*gerenerating 1DM from DMM*/
    ARAD_PP_OAMP_PE_PROGS_1DM_DOWN, /* gerenerating 1DM from DMM + UDH*/
    ARAD_PP_OAMP_PE_PROGS_ETH_TLV_ON_SERVER,  /*WA for Eth-TLV bug on server MEP.  AKA Port/interface status TLV bug fix for OAMP server */
    ARAD_PP_OAMP_PE_PROGS_BFD_ECHO,
    ARAD_PP_OAMP_PE_PROGS_BFD_UDP_CHECKSUM, /* Arad+ only */
    ARAD_PP_OAMP_PE_PROGS_UP_MEP_MAC, /* WA for up-MEP mac LSB restrictions */
    ARAD_PP_OAMP_PE_PROGS_DOWN_MEP_TLV_FIX, /* Same actual program as UP_MEP_MAC. two MEP-PE profiles. */
    ARAD_PP_OAMP_PE_PROGS_BFD_IPV4_SINGLE_HOP, /* Arad+ only */
    ARAD_PP_OAMP_PE_PROGS_MAID_11B_END_TLV, /* Arad+: MAID 11B + End tlv bugfix, Jericho: MAID 11B */
    ARAD_PP_OAMP_PE_PROGS_MAID_11B_END_TLV_UDH, /* Arad+: MAID 11B + End tlv bugfix +UDH, Jericho: MAID 11B */
    ARAD_PP_OAMP_PE_PROGS_MAID_11B_END_TLV_ON_SERVER, /* Arad+: OAMP server + MAID 11B + End tlv bugfix*/
    ARAD_PP_OAMP_PE_PROGS_OAMP_SERVER_MAID_11B_JER, /*Server program + MAID_11B for Jericho */
    ARAD_PP_OAMP_PE_PROGS_OAMTS_DOWN_MEP, /* Arad+ only,currently use if UDH != 0 */
    ARAD_PP_OAMP_PE_PROGS_PPH_ADD_UDH, /* Adds a UDH, assumes a PPH is present. OAM-TS is optional*/
    ARAD_PP_OAMP_PE_PROGS_PPH_ADD_UDH_CCM_JER, /* Adds statistics and loads ARAD_PP_OAMP_PE_PROGS_PPH_ADD_UDH_CCM_JER program*/
    ARAD_PP_OAMP_PE_PROGS_UP_MEP_UDH_DEFAULT_JER, /* Jericho only for UP MEP once UDH is on(actually default program is used)*/
    ARAD_PP_OAMP_PE_PROGS_UDH_PUNT_JER, /* Jericho only for Punt/Events packets with UDH*/
    ARAD_PP_OAMP_PE_PROGS_MICRO_BFD, /* Jericho only */
    ARAD_PP_OAMP_PE_PROGS_BFD_PWE_BOS_FIX, /* Jericho only */
    ARAD_PP_OAMP_PE_PROGS_MAID_48, /* Jericho and QAX */
    ARAD_PP_OAMP_PE_PROGS_MAID_48_UDH, /* Jericho and QAX */
    ARAD_PP_OAMP_PE_PROGS_UP_MEP_MAC_MC, /* sets MC destination on ITMH*/
    ARAD_PP_OAMP_PE_PROGS_CONSTRUCT_1711_PWE,
    ARAD_PP_OAMP_PE_PROGS_CONSTRUCT_1711_MPLS_TP,	
    ARAD_PP_OAMP_PE_PROGS_SLM_DOWN, /*QAX sets OAM-TS.Syb-type */
    ARAD_PP_OAMP_PE_PROGS_EGR_INJ, /*QAX only used for Egress Injection */
    ARAD_PP_OAMP_PE_PROGS_SLM_EGR_INJ, /*QAX only used for Egress Injection */
    ARAD_PP_OAMP_PE_PROGS_LM_DM_EGR_INJ, /*QAX only used for Egress Injection */
    ARAD_PP_OAMP_PE_PROGS_EGR_INJ_48_MAID_QAX,/*QAX only used for Egress Injection + 48MAID*/
    ARAD_PP_OAMP_PE_PROGS_SLM_EGR_INJ_48_MAID_QAX,/*QAX only used for Egress Injection + 48MAID,actually ARAD_PP_OAMP_PE_PROGS_SLM_EGR_INJ program will be used*/
    /*The programs takes PWE/MPLS packets and adds a EEI header with a pointer to an additional encap pointer.
    Encap pointer is built by programs with {MEP pe profile-lsb (2), pe-gen-mem(16)}. The different enums are used to distinguish between the different msbs in the PE profile.*/
    ARAD_PP_OAMP_PE_PROGS_CCM_COUNT_QAX,/*QAX only used for ccm cointing.*/
    ARAD_PP_OAMP_PE_PROGS_SLM_CCM_COUNT_QAX, /*QAX only used for ccm cointing, actually ARAD_PP_OAMP_PE_PROGS_SLM_DOWN program will be used*/
   /*The programs takes CCM over PWE/MPLS packets and adds a EEI header with a pointer to an additional encap pointer.
    Also the program uses the extra data,that come before the packet,to build 48 MEG-ID. 
    Encap pointer is built by programs with {MEP pe profile-lsb (2), pe-gen-mem(16)}. The different enums are used to distinguish between the different msbs in the PE profile.*/
    ARAD_PP_OAMP_PE_PROGS_CCM_COUNT_48_MAID_QAX,/*QAX only used for ccm cointing + 48MAID.There are 4 values for mep_pe_profile,so any endpoint that needs this feature will use one of these 4 mep_pe_profiles.*/
    ARAD_PP_OAMP_PE_PROGS_SLM_CCM_COUNT_48_MAID_QAX, /*QAX only used for ccm cointing + 48MAID, actually ARAD_PP_OAMP_PE_PROGS_SLM_DOWN program will be used*/
    /* nof progs - always last */
    ARAD_PP_OAMP_PE_PROGS_NOF_PROGS
} ARAD_PP_OAMP_PE_PROGRAMS;


/* To accomodate the field size and location differences between devices
   and the fact that the excels don't divide the OAMP-PE instruction to fields,
   a mapping from field name to size and start bit is neccessary.
   This enum supplies the field names */
typedef enum
{
    ARAD_PP_OAMP_PE_IN_FIFO_RD_BITS = 0     ,
    ARAD_PP_OAMP_PE_FEM1_SEL_BITS           ,
    ARAD_PP_OAMP_PE_FEM2_SEL_BITS           ,
    ARAD_PP_OAMP_PE_MUX1_SRC_BITS           ,
    ARAD_PP_OAMP_PE_MERGE1_INST_BITS        ,
    ARAD_PP_OAMP_PE_SHIFT1_SRC_BITS         ,
    ARAD_PP_OAMP_PE_SHIFT2_SRC_BITS         ,
    ARAD_PP_OAMP_PE_FDBK_FF_WR_BIT          ,
    ARAD_PP_OAMP_PE_BUFF_WR_BIT             ,
    ARAD_PP_OAMP_PE_BUFF_WR_BITS            ,
    ARAD_PP_OAMP_PE_BUFF_SIZE_SRC_BITS      ,
    ARAD_PP_OAMP_PE_BUFF1_SIZE_SRC_BITS     ,
    ARAD_PP_OAMP_PE_OP1_SEL_BITS            ,
    ARAD_PP_OAMP_PE_OP2_SEL_BITS            ,
    ARAD_PP_OAMP_PE_ALU_ACT_BITS            ,
    ARAD_PP_OAMP_PE_CMP1_ACT_BITS           ,
    ARAD_PP_OAMP_PE_ALU_DST_BITS            ,
    ARAD_PP_OAMP_PE_BUF_EOP_BIT             ,
    ARAD_PP_OAMP_PE_BUF_EOP_BITS            ,
    ARAD_PP_OAMP_PE_BUF_EOP_FRC_BIT         ,
    ARAD_PP_OAMP_PE_INST_CONST_BITS         ,
    ARAD_PP_OAMP_PE_FDBK_FF_RD_BIT          ,
    ARAD_PP_OAMP_PE_OP3_SEL_BITS            ,
    ARAD_PP_OAMP_PE_CMP2_ACT_BITS           ,
    ARAD_PP_OAMP_PE_INST_SRC_BITS           ,
    ARAD_PP_OAMP_PE_MUX2_SRC_BITS           ,
    ARAD_PP_OAMP_PE_BUFF2_SIZE_BITS         ,
    ARAD_PP_OAMP_PE_MERGE2_INST_BITS        ,

    ARAD_PP_OAMP_PE_INSTRUCTION_FIELDS_NOF
} ARAD_PP_OAMP_PE_INSTRUCTION_FIELDS;

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
/*Represents the whole key for the protection packets */
#define _OAMP_PE_TCAM_KEY_PROTECTION_PACKET 0x7

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

/**
* NAME:
 *   arad_pp_oamp_pe_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize the OAMP Programable Editor.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 * REMARKS:
 * None.
 * RETURNS:
 *   OK or ERROR indication.
**/
uint32
  arad_pp_oamp_pe_unsafe(
    SOC_SAND_IN  int                                 unit
  );



/**
* NAME:
 *   arad_pp_oamp_pe_profile_get
 * TYPE:
 *   PROC
 * FUNCTION:
*    Returns a program pointer (first instruction of program in
*  OAMP PE Program table)
*  INPUT:
*    SOC_SAND_IN  int                   unit -
*   SOC_SAND_IN   ARAD_PP_OAMP_PE_PROGRAMS     program_id -
*  program to set
*  SOC_SAND_OUT  int *program_profile
*     Result goes there. Program profile (0-7) if found,
*  -1 otherwise
*  REMARKS:
 * None for now.
 * RETURNS:
 *   Nothing.
**/
void
  arad_pp_oamp_pe_program_profile_get(
    SOC_SAND_IN   int                                 unit,
    SOC_SAND_IN   ARAD_PP_OAMP_PE_PROGRAMS     program_id,
    SOC_SAND_OUT  uint32                                 *program_profile
  );


/**
 * Set the OAMP GEN MEM. Currently Used only for server program. 
 * Data represents local port on inner PTCH, index represents 
 * local port on mep db, which is the LSB of the src mac 
 * address. 
 * 
 * @author sinai (28/10/2014)
 * 
 * @param unit 
 * @param gen_mem_index 
 * @param gen_mem_data 
 * 
 * @return soc_error_t 
 */
soc_error_t arad_pp_oam_oamp_pe_gen_mem_set(int unit, int gen_mem_index, int gen_mem_data);

/**
 * Get the OAMP GEN MEM. 
 * 
 * @author Amir Gazit (27/10/2016)
 * 
 * @param unit 
 * @param gen_mem_index 
 * @param gen_mem_data 
 * 
 * @return soc_error_t 
 */
soc_error_t arad_pp_oam_oamp_pe_gen_mem_get(int unit, int gen_mem_index, uint32 *gen_mem_data);


/*********************************************************************
* NAME:
 *   arad_pp_oamp_pe_print_all_programs_data
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Dump ALL OAMP PE program.
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   This API must be called during a continuous stream of
 *   the identical packets coming from the same source.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_oamp_pe_print_all_programs_data(int unit);
  
/*********************************************************************
* NAME:
 *   arad_pp_oamp_pe_print_last_program_data
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Dump last OAMP PE program invoked.
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   This API must be called during a continuous stream of
 *   the identical packets coming from the same source.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_oamp_pe_print_last_program_data(int unit);

/* Set GEN_MEM entries for a MEP */
/* Sets MEP's 2 values of 32 bit in gen_mem 
   All first data of the MEP stored sequentially by mep_id.
   Then all second data.
   --------------
   | data_1 Mep 0|
   | data_1 Mep 1|
   |     ...     |
   |     ...     |
   | data_1 Mep N|
   | data_2 Mep 0|
   |     ...     |
   |     ...     |
   | data_2 Mep N|
   ------_--------
*/
int soc_arad_pp_set_mep_data_in_gen_mem(int unit,uint32 mep_id, uint16 data_1,uint16 data_2);

/* Gets MEP's 2 values of 32 bit in gen_mem 
   All first data of the MEP stored sequentially by mep_id.
   Then all second data.
   --------------
   | data_1 Mep 0|
   | data_1 Mep 1|
   |     ...     |
   |     ...     |
   | data_1 Mep N|
   | data_2 Mep 0|
   |     ...     |
   |     ...     |
   | data_2 Mep N|
   ------_--------
*/
int soc_arad_pp_get_mep_data_in_gen_mem(int unit,uint32 mep_id, uint32 *data_1,uint32 *data_2);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_OAMP_PE_INCLUDED__*/
#endif




