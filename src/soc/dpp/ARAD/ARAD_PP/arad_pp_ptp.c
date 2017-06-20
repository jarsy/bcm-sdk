#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_ptp.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#include <soc/mem.h>
#define _ERR_MSG_MODULE_NAME BSL_SOC_PTP

#include <shared/bsl.h>
/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>


#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ptp.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_parser.h>

#include <soc/dpp/ARAD/arad_general.h>

#include <soc/dpp/ARAD/arad_tbl_access.h>

#include <soc/dpp/drv.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* entries identifiers in identification cam table */
enum {

	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_0  =  0,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_1  =  1,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_2  =  2,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_3  =  3,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_4  =  4,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_5  =  5,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_6  =  6,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_7  =  7,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_8  =  8,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_9  =  9,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_10 = 10,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_11 = 11,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_12 = 12,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_13 = 13,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_14 = 14,
	ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRY_15 = 15,

    ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRIES_NUM
};


/* IHB_IEEE_1588_IDENTIFICATION_CAM PROGRAM field (5 bits) inner bits description  */
#define ARAD_PP_1588_PROGRAM_FIELD_PACKET_IS_IEEE_1588_SHIFT            0 
#define ARAD_PP_1588_PROGRAM_FIELD_PACKET_IS_IEEE_1588_MASK           0x1 /* 1 bit length */
#define ARAD_PP_1588_PROGRAM_FIELD_ENCAPSULATION_SHIFT                  1
#define ARAD_PP_1588_PROGRAM_FIELD_ENCAPSULATION_MASK                 0x1 /* 1 bit length */
#define ARAD_PP_1588_PROGRAM_FIELD_CARRY_1588_HEADER_INDEX_SHIFT        2
#define ARAD_PP_1588_PROGRAM_FIELD_CARRY_1588_HEADER_INDEX_MASK       0x7 /* 3 bit length */


/* IHB_IEEE_1588_ACTION  ACTION-[3] fields (6 bits) inner bits description  */
#define ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_SHIFT               0 
#define ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_MASK              0x1 /* 1 bit length */
#define ARAD_PP_1588_ACTION_FIELD_COMMAND_SHIFT                        1
#define ARAD_PP_1588_ACTION_FIELD_COMMAND_MASK                       0x3 /* 2 bit length */
#define ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_SHIFT                   3
#define ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_MASK                  0x7 /* 3 bit length */



/* IHB_IEEE_1588_ACTION table access key (8 bits) inner bits description */
#define ARAD_PP_1588_ACTION_TABLE_KEY_MESSAGE_TYPE_SHIFT               1 
#define ARAD_PP_1588_ACTION_TABLE_KEY_MESSAGE_TYPE_MASK              0xf 


enum {

	ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING       = 0,
	ARAD_PP_1588_EGRESS_COMMAND_1_RX_STAMPING_AND_TX_CF_STAMPING = 1,
	ARAD_PP_1588_EGRESS_COMMAND_2_TX_RECORDING                   = 2
};



/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_1588_SET_PROGRAM_FIELD(program_field, encap, carry_header)   \
    program_field = 0; \
    program_field |= ((1            & ARAD_PP_1588_PROGRAM_FIELD_PACKET_IS_IEEE_1588_MASK)     << ARAD_PP_1588_PROGRAM_FIELD_PACKET_IS_IEEE_1588_SHIFT); \
    program_field |= ((encap        & ARAD_PP_1588_PROGRAM_FIELD_ENCAPSULATION_MASK)           << ARAD_PP_1588_PROGRAM_FIELD_ENCAPSULATION_SHIFT); \
    program_field |= ((carry_header & ARAD_PP_1588_PROGRAM_FIELD_CARRY_1588_HEADER_INDEX_MASK) << ARAD_PP_1588_PROGRAM_FIELD_CARRY_1588_HEADER_INDEX_SHIFT)


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

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_ptp[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_INIT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_INIT_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_PORT_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_PORT_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_PORT_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_PORT_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_PTP_PORT_SET_ACTION_PROFILE),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_ptp[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_PTP_ACTION_PROFILE_OUT_OF_RANGE_ERR,
    "ARAD_PP_PTP_ACTION_PROFILE_OUT_OF_RANGE_ERR",
 	"ARAD_PP_PTP action profile is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },


  /*
   * } Auto generated. Do not edit previous section.
   */


  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


/*********************************************************************
*      1588 initialization
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
arad_pp_ptp_init_unsafe(
                        SOC_SAND_IN  int                                 unit
                        )
{
    uint32 res = SOC_SAND_OK;
    uint32 epni_reg = 0;
    uint32 tbl_data[2];
    uint32 program, command;
    uint32 i, stamp_bit_mask, val, mask;
    int egress_command, index_max, core_id;

#define ANY       -1 /* any protocol, mask=0xf */     

#ifdef NONE   
/* undefine "NONE" in order to prevent VxWorks compilation error, sice "NONE" also defined in VxWorks headers */
#undef NONE     
#endif

#define NONE      ARAD_PP_L3_NEXT_PRTCL_NDX_NONE    /*  0 */
/* 1588 over UDP */
#define I1588oU   ARAD_PARSER_ETHER_PROTO_6_1588    /*  5 */
/* 1588 over ethernet (adding 1 since table is 1 based) */
#define I1588oE   ARAD_PARSER_ETHER_PROTO_6_1588 + 1/*  6 */
#define UDP       ARAD_PP_L3_NEXT_PRTCL_NDX_UDP     /*  9 */
#define IPV4      ARAD_PP_L3_NEXT_PRTCL_NDX_IPV4    /* 13 1101b*/
#define IPV6      ARAD_PP_L3_NEXT_PRTCL_NDX_IPV6    /* 14 1110b*/
#define MPLS      ARAD_PP_L3_NEXT_PRTCL_NDX_MPLS    /* 15 1111b*/


#define ENC_UDP   0
#define ENC_ETH   1

/* we don't have enough entries, so uniting IPV4 & IPV6 by kind of compromise:
   IP, IP_MSK will catch also 12 & 15 values (in addition to 13 & 14), 
   but this is probably better than catching all values (by using ANY) */
#define IP        ((IPV4)&(IPV6))                   /* 12 1100b*/
#define IP_MSK    ((IPV4)^(IPV6))                   /*  3 0011b*/



/* 1588 supported encapsulation */
   int ieee_1588_identification_cam[ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRIES_NUM][9] =
       
       /*        input    input    input    input    input  input        output   putput */
       /* valid  q1       q2       q3       q4       q5     fwd_header   encup    1588_carry_header                                   */
        {{1,     I1588oE, NONE   , NONE   , NONE   , NONE   , 1,         ENC_ETH, 1 }, /*  0 1588oE               switched            */
         {1,     IP     , UDP    , I1588oU, NONE   , NONE   , 1,         ENC_UDP, 3 }, /*  1 1588oUDPoIPoE        switched            */
         {1,     IP     , UDP    , I1588oU, NONE   , NONE   , 2,         ENC_UDP, 3 }, /*  2 1588oUDPoIPoE        routed              */
         {1,     IP     , IP     , UDP    , I1588oU, NONE   , 1,         ENC_UDP, 4 }, /*  3 1588oUDPoIPoIPoE     switched            */
         {1,     IP     , IP     , UDP    , I1588oU, NONE   , 2,         ENC_UDP, 4 }, /*  4 1588oUDPoIPoIPoE     IP routed           */
         {1,     IP     , IP     , UDP    , I1588oU, NONE   , 3,         ENC_UDP, 4 }, /*  5 1588oUDPoIPoIPoE     IP terminated       */
         {1,     MPLS   , ANY    , UDP    , I1588oU, NONE   , 1,         ENC_UDP, 4 }, /*  6 1588oUDPoIPoMPLSoE   switched            */
         {1,     MPLS   , ANY    , UDP    , I1588oU, NONE   , 2,         ENC_UDP, 4 }, /*  7 1588oUDPoIPoMPLSoE   MPLS routed         */
         {1,     MPLS   , ANY    , UDP    , I1588oU, NONE   , 3,         ENC_UDP, 4 }, /*  8 1588oUDPoIPoMPLSoE   MPLS terminated     */
         {1,     MPLS   , ANY    , I1588oE, NONE   , NONE   , 1,         ENC_ETH, 3 }, /*  9 1588oEoMPLSoE        switched            */
         {1,     MPLS   , ANY    , I1588oE, NONE   , NONE   , 2,         ENC_ETH, 3 }, /* 10 1588oEoMPLSoE        MPLS routed         */
         {1,     MPLS   , ANY    , I1588oE, NONE   , NONE   , 3,         ENC_ETH, 3 }, /* 11 1588oEoMPLSoE        MPLS terminated     */
         {0,     NONE   , NONE   , NONE   , NONE   , NONE   , 0,         0      , 0 }, /* 12 */
         {0,     NONE   , NONE   , NONE   , NONE   , NONE   , 0,         0      , 0 }, /* 13 */
         {0,     NONE   , NONE   , NONE   , NONE   , NONE   , 0,         0      , 0 }, /* 14 */
         {0,     NONE   , NONE   , NONE   , NONE   , NONE   , 0,         0      , 0 }  /* 15 */
        };
        

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_PTP_INIT_UNSAFE);

    /* ARAD+ 1588 48b stamping */
#ifdef BCM_88660_A0
    if(SOC_IS_ARADPLUS(unit)) {

        SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_EPNI_CFG_48_BITS_1588_TS_ENABLEr(unit, REG_PORT_ANY, &epni_reg)); 
        soc_reg_field_set(unit, EPNI_CFG_48_BITS_1588_TS_ENABLEr, &epni_reg, CFG_48_BITS_1588_TS_ENABLEf, 
                          (SOC_DPP_CONFIG(unit))->pp.ptp_48b_stamping_enable ? 1 : 0); 
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_EPNI_CFG_48_BITS_1588_TS_ENABLEr(unit, REG_PORT_ANY, epni_reg)); 

     }
#endif /* BCM_88660_A0 */


    /* set 1588 time stamp offset to 0 this is the timestamp offset within the IEEE-1588 header */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_EPNI_IEEE_1588r(unit, REG_PORT_ANY, &epni_reg)); 
    soc_reg_field_set(unit, EPNI_IEEE_1588r, &epni_reg, IEEE_1588_TS_OFFSET_FIXf, 0); 

    /* according to the command result from IHB_IEEE_1588_ACTION table,
       an action is taken at the command bit place */
    stamp_bit_mask = 0;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING;
    stamp_bit_mask |= 1 << ARAD_PP_1588_EGRESS_COMMAND_1_RX_STAMPING_AND_TX_CF_STAMPING;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_2_TX_RECORDING;
    /* when bit is 1 the timestamp is taken when packet is received */
    soc_reg_field_set(unit, EPNI_IEEE_1588r, &epni_reg, IEEE_1588_RX_STAMPf     , stamp_bit_mask /* 0x2 = 00000010b */); 
    stamp_bit_mask = 0;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING;
    stamp_bit_mask |= 1 << ARAD_PP_1588_EGRESS_COMMAND_1_RX_STAMPING_AND_TX_CF_STAMPING;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_2_TX_RECORDING;
    /* when bit is 1 correction field is updated when packet is transmitted */
    soc_reg_field_set(unit, EPNI_IEEE_1588r, &epni_reg, IEEE_1588_TX_STAMP_CFf  , stamp_bit_mask /* 0x2 = 00000010b */); 
    stamp_bit_mask = 0;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING;
    stamp_bit_mask |= 0 << ARAD_PP_1588_EGRESS_COMMAND_1_RX_STAMPING_AND_TX_CF_STAMPING;
    stamp_bit_mask |= 1 << ARAD_PP_1588_EGRESS_COMMAND_2_TX_RECORDING;
    /* when bit is 1 tx time is recorded when packet is transmitted */
    soc_reg_field_set(unit, EPNI_IEEE_1588r, &epni_reg, IEEE_1588_RECORD_TSf  , stamp_bit_mask   /* 0x4 = 00000100b */); 

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_EPNI_IEEE_1588r(unit, REG_PORT_ANY, epni_reg)); 


    sal_memset(tbl_data, 0, sizeof (tbl_data));

    /* by default setting all values to 0 */
    /* in startup - all ports are pointing to profile 0 (ACTIONf) */
    index_max = soc_mem_index_max(unit, IHP_IEEE_1588_ACTIONm);

    for(i = 0; i <= index_max ; i++) {

        egress_command = ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING;

        command = 0;
        command |= ((0              & ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_MASK) << ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_SHIFT);
        command |= ((egress_command & ARAD_PP_1588_ACTION_FIELD_COMMAND_MASK)          << ARAD_PP_1588_ACTION_FIELD_COMMAND_SHIFT);
        command |= ((0              & ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_MASK)     << ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_SHIFT);

        soc_IHP_IEEE_1588_ACTIONm_field32_set(unit, tbl_data, ACTIONf,   0);
        soc_IHP_IEEE_1588_ACTIONm_field32_set(unit, tbl_data, ACTION_1f, 0);
        soc_IHP_IEEE_1588_ACTIONm_field32_set(unit, tbl_data, ACTION_2f, 0);
        soc_IHP_IEEE_1588_ACTIONm_field32_set(unit, tbl_data, ACTION_3f, 0);


        SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, 
                                     WRITE_IHP_IEEE_1588_ACTIONm(unit, 
                                                                 MEM_BLOCK_ANY, 
                                                                 i,
                                                                 tbl_data));
   }


    /* by default setting values of port IngressP2pDelay to 0, 
       later the 1588 software in the ARM can set it (in case Peer to Peer 1588 messages are used) */
    index_max = soc_mem_index_max(unit, IHB_PINFO_LBPm);

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core_id) {
        for(i = 0; i <= index_max ; i++) {

            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core_id), i, tbl_data));
            soc_IHB_PINFO_LBPm_field32_set(unit, tbl_data, INGRESS_P2P_DELAYf, 0);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core_id), i, tbl_data));

        }
    }
 

    sal_memset(tbl_data, 0, sizeof (tbl_data));

    /* configuring 1588 identification CAM */
    for(i = 0; i < ARAD_PP_1588_IDENTIFICATION_CAM_TABLE_ENTRIES_NUM; i++) {
        
        /* checking whether row is valid */
        if (ieee_1588_identification_cam[i][0] == 0) {
            /* marking the entry as not valid */
            soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, VALIDf, 0);
            continue;
        }

        /* marking the entry as valid */
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, VALIDf, 1);

        /* setting next protocols 1-5 */
        /* mask=0xf, accepts any qualifier value*/
        /* mask=IP_MSK(0011b), accepts QQXXb values (Q - bit value of qualifier, X - any bit, 0 or 1) */
        /* mask=0, accepts only qualifier value (QQQQb)  (Q - bit value of qualifier) */
        mask = (ANY == ieee_1588_identification_cam[i][1]) ? 0xf : ((IP == ieee_1588_identification_cam[i][1]) ? IP_MSK : 0); 
        val  = (ANY == ieee_1588_identification_cam[i][1]) ? 0   : ieee_1588_identification_cam[i][1];
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_1_NEXT_PROTOCOL_MASKf, mask);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_1_NEXT_PROTOCOLf,      val);

        mask = (ANY == ieee_1588_identification_cam[i][2]) ? 0xf : ((IP == ieee_1588_identification_cam[i][2]) ? IP_MSK : 0); 
        val  = (ANY == ieee_1588_identification_cam[i][2]) ? 0   : ieee_1588_identification_cam[i][2];
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_2_NEXT_PROTOCOL_MASKf, mask);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_2_NEXT_PROTOCOLf,      val);

        mask = (ANY == ieee_1588_identification_cam[i][3]) ? 0xf : ((IP == ieee_1588_identification_cam[i][3]) ? IP_MSK : 0); 
        val  = (ANY == ieee_1588_identification_cam[i][3]) ? 0   : ieee_1588_identification_cam[i][3];
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_3_NEXT_PROTOCOL_MASKf, mask);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_3_NEXT_PROTOCOLf,      val);

        mask = (ANY == ieee_1588_identification_cam[i][4]) ? 0xf : ((IP == ieee_1588_identification_cam[i][4]) ? IP_MSK : 0); 
        val  = (ANY == ieee_1588_identification_cam[i][4]) ? 0   : ieee_1588_identification_cam[i][4];
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_4_NEXT_PROTOCOL_MASKf, mask);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_4_NEXT_PROTOCOLf,      val);

        mask = (ANY == ieee_1588_identification_cam[i][5]) ? 0xf : ((IP == ieee_1588_identification_cam[i][5]) ? IP_MSK : 0); 
        val  = (ANY == ieee_1588_identification_cam[i][5]) ? 0   : ieee_1588_identification_cam[i][5];
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_5_NEXT_PROTOCOL_MASKf, mask);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, QUALIFIER_5_NEXT_PROTOCOLf,      val);

        /* forwarding offset index is always USED */
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, FORWARDING_OFFSET_INDEX_MASKf, 0);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, FORWARDING_OFFSET_INDEXf     , ieee_1588_identification_cam[i][6]);

        /* parser leaf context value is currently NOT USED */
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, PARSER_LEAF_CONTEXT_MASKf, 0xf);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, PARSER_LEAF_CONTEXTf, 0);

        /* PROGRAM field is composed of 5 bits: */
        /* I - is 1588       0 - no,  1 - yes*/
        /* E - encapsulation 0 - eth, 1 - udp */
        /* C - carry index header 0-7 */
        /* */
        /* bits     4   0 */
        /*          CCCEI */
        /*                                      1588 encapsulation 0-eth, 1-udp     1588 carry header index */
        ARAD_PP_1588_SET_PROGRAM_FIELD(program, ieee_1588_identification_cam[i][7], ieee_1588_identification_cam[i][8]);
        soc_IHP_IEEE_1588_IDENTIFICATION_CAMm_field32_set(unit, tbl_data, PROGRAMf, program);


 
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, 
                                     WRITE_IHP_IEEE_1588_IDENTIFICATION_CAMm(unit, 
                                                                             MEM_BLOCK_ANY, 
                                                                             i,
                                                                             tbl_data));
    } 


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ptp_init_unsafe()", 0, 0);
}


/*********************************************************************
*     Details: set the profile in the action 
*********************************************************************/
STATIC uint32
  arad_pp_ptp_port_set_action_profile(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_PTP_PORT_INFO                  *info,
    SOC_SAND_IN  SOC_PPC_PTP_IN_PP_PORT_PROFILE         profile
                              
  )
{
    uint32
        res = SOC_SAND_OK;
    int action_field = 0, i;
    int egress_command, index_max, action_index = 0, msg_type;
    uint32 command;
    uint32 tbl_data[2] = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_PTP_PORT_SET_ACTION_PROFILE);

    index_max = soc_mem_index_max(unit, IHP_IEEE_1588_ACTIONm);

    for(i = 0; i < index_max + 1; i++) {

        msg_type = ((i >> ARAD_PP_1588_ACTION_TABLE_KEY_MESSAGE_TYPE_SHIFT) & ARAD_PP_1588_ACTION_TABLE_KEY_MESSAGE_TYPE_MASK);

        /* table access 8 bits: EDDMMMMO */
        /*  */
        /*   E IEEE-1588-Encapsulation (1),  */
        /*   D IEEE-1588-Address (2),  */
        /*   M IEEE-1588-Header.Message-Type (4),  */
        /*   O one step 2 step IEEE-1588-Header.Flag-Field[9],  */
        /*  */
        /* the action field selection is via 2 bits port profile:  */
        /* In-PP-Port.IEEE-1588-Profile (2)  */

        
        /* by default setting command 0, no RX time record & no TX time stamping */
        egress_command = ARAD_PP_1588_EGRESS_COMMAND_0_NO_STAMPING_NO_RECORDING;

        /* when one step TC is enabled setting command 3, RX record & TX stamping */
        if(0 != (info->flags & SOC_PPC_PTP_PORT_TIMESYNC_ONE_STEP_TIMESTAMP)) {
            /* 1588 protocol TC implementation, need to updated CF in following 4 messages */
            if(SOC_PPC_PTP_1588_PKT_SYNC        == msg_type ||
               SOC_PPC_PTP_1588_PKT_DELAY_REQ   == msg_type ||
               SOC_PPC_PTP_1588_PKT_PDELAY_REQ  == msg_type ||
               SOC_PPC_PTP_1588_PKT_PDELAY_RESP == msg_type) {
                /* when one step TC is enabled setting command 1, RX time record & TX time stamping */
                egress_command = ARAD_PP_1588_EGRESS_COMMAND_1_RX_STAMPING_AND_TX_CF_STAMPING;
            }
        } else if(0 != (info->flags & SOC_PPC_PTP_PORT_TIMESYNC_TWO_STEP_TIMESTAMP)) {
            /* 1588 protocol TC implementation, need to updated CF in following 4 messages */
            if(SOC_PPC_PTP_1588_PKT_SYNC        == msg_type ||
               SOC_PPC_PTP_1588_PKT_DELAY_REQ   == msg_type ||
               SOC_PPC_PTP_1588_PKT_PDELAY_REQ  == msg_type ||
               SOC_PPC_PTP_1588_PKT_PDELAY_RESP == msg_type) {
                /* when two step TC is enabled setting command 2, TX time recording */
                egress_command = ARAD_PP_1588_EGRESS_COMMAND_2_TX_RECORDING;
            }
        }

        /*
         * pkt_drop, pkt_tocpu bitmaps of 1588 event and general packet types indicating whether to :
         *   1. forward    (drop-0,tocpu-0) 
         *   2. trap/snoop (drop-0,tocpu-1)
         *   3. drop       (drop-1,tocpu-0) 
         * the packet.
         */

        if( ((info->pkt_drop & (1 << msg_type)) == 0) && ((info->pkt_tocpu & (1 << msg_type)) == 0) ) {
            action_index = SOC_PPC_PTP_ACTION_FIELD_ACTION_INDEX_FWD;
        }
        else if( ((info->pkt_drop & (1 << msg_type)) != 0) ) {
            action_index = SOC_PPC_PTP_ACTION_FIELD_ACTION_INDEX_DROP;
        }
        else if( ((info->pkt_tocpu & (1 << msg_type)) != 0) ) {
            action_index = SOC_PPC_PTP_ACTION_FIELD_ACTION_INDEX_TRAP;
        }


        /* table action field 6 bits: AAACCS */
        /*  */
        /* S UpdateTimeStamp */
        /*   If set then time stamp should be updated by In-PP-Port value. */
        /*   0x0  Do not update timestamp by In-PP port value. */
        /*   0x1  Update timestamp by In-PP port value. */
        /* C Command */
        /*   The command passed to the egress */
        /* A ActionIndex */
          
        command = 0;
        /* index to the trap/snoop strength in IHB_ACTION_PROFILE_IEEE_1588 register */
        command |= ((action_index   & ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_MASK)     << ARAD_PP_1588_ACTION_FIELD_ACTION_INDEX_SHIFT);
        /* the command to pass to the egress */
        command |= ((egress_command & ARAD_PP_1588_ACTION_FIELD_COMMAND_MASK)          << ARAD_PP_1588_ACTION_FIELD_COMMAND_SHIFT);
        /* this bit indicates whether to updated the CF with IngressP2pDelay  */
        command |= ((1              & ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_MASK) << ARAD_PP_1588_ACTION_FIELD_UPDATE_TIMESTAMP_SHIFT);


        switch(profile) {
            case SOC_PPC_PTP_IN_PP_PORT_PROFILE_0:
                action_field = ACTIONf;
                break;
            case SOC_PPC_PTP_IN_PP_PORT_PROFILE_1:
                action_field = ACTION_1f;
                break;
            case SOC_PPC_PTP_IN_PP_PORT_PROFILE_2:
                action_field = ACTION_2f;
                break;
            case SOC_PPC_PTP_IN_PP_PORT_PROFILE_3:
                action_field = ACTION_3f;
                break;
            case SOC_PPC_PTP_IN_PP_PORT_PROFILES_NUM:
            default: 
                SOC_SAND_SET_ERROR_CODE(ARAD_PP_PTP_ACTION_PROFILE_OUT_OF_RANGE_ERR, 10, exit);
        }


        /* setting table field/column according to the port profile (0-3) */
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IHP_IEEE_1588_ACTIONm(unit, MEM_BLOCK_ANY, i, tbl_data));
        soc_IHP_IEEE_1588_ACTIONm_field32_set(unit, tbl_data, action_field, command);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_IHP_IEEE_1588_ACTIONm(unit, MEM_BLOCK_ANY, i, tbl_data));
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ptp_port_set_action_profile()", 0, 0);
}
/*********************************************************************
 *      1588 port initialization
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_ptp_port_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PTP_PORT_INFO                  *info,
    SOC_SAND_IN  SOC_PPC_PTP_IN_PP_PORT_PROFILE         profile
  )
{
    uint32
        res = SOC_SAND_OK;
    uint32 entry[4];
    uint8 ext_mac_enable = FALSE;
    char *propval;
    int core=0;
    uint32 pp_port;
    soc_mem_t
        pinfo_flp_mem = SOC_IS_JERICHO(unit) ? IHP_PINFO_FLP_1m : IHB_PINFO_FLPm;

    soc_mem_t
        packet_processing_port_mem = SOC_IS_JERICHO_PLUS(unit) ? EPNI_PACKETPROCESSING_PORT_CONFIGURATION_TABLEm : EPNI_PACKET_PROCESSING_PORT_CONFIGURATION_TABLE_PP_PCTm;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_PTP_PORT_SET_UNSAFE);

         
    res = arad_pp_ptp_port_set_action_profile(unit, info, profile);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = soc_port_sw_db_local_to_pp_port_get(unit, local_port_ndx, &pp_port, &core);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* when 1588 is enabled setting port profile */
    if (SOC_IS_JERICHO(unit)) {
        res = soc_mem_read(unit, pinfo_flp_mem, IHP_BLOCK(unit, core), pp_port, entry);
    }else {
        res = soc_mem_read(unit, pinfo_flp_mem, MEM_BLOCK_ANY, pp_port, entry);
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    soc_mem_field32_set(unit, pinfo_flp_mem, entry, IEEE_1588_PROFILEf, profile);
    if (SOC_IS_JERICHO(unit)) {
        res = soc_mem_write(unit, pinfo_flp_mem, IHP_BLOCK(unit, core), pp_port, entry);
    }else {
        res = soc_mem_write(unit, pinfo_flp_mem, MEM_BLOCK_ANY, pp_port, entry);
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    propval = soc_property_port_get_str(unit, pp_port, spn_EXT_1588_MAC_ENABLE);    
    if (propval) {
        if (sal_strcmp(propval, "1") == 0) {
            ext_mac_enable = TRUE;
        } else if (sal_strcmp(propval, "TRUE") == 0) {
            ext_mac_enable = TRUE;
        }
    } 
        
    if(0 != info->ptp_enabled) {
        if(TRUE == ext_mac_enable) {
            /* when external MAC is enabled on the port:  */
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry));
            /*   Incoming packets - setting timestamp to 0,
                 the external MAC already reduced the RX time from the correction field */            
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, RESET_TIME_STAMPf,  1);
            /*   outgoing packets  - indicating the MAC not to stamp the packet,
                 the EXTERNAL MAC will do the stamping (adding the TX time to the correction field) */            
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, EXTERNAL_BRCM_MACf, 1);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core), pp_port, entry));
        }else {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry));           
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, RESET_TIME_STAMPf,  0);            
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, EXTERNAL_BRCM_MACf, 0);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core), pp_port, entry));
        }

        /* enabling 1588 MAC Tx stamping */
        res = soc_mem_read(unit, packet_processing_port_mem, EPNI_BLOCK(unit, core), pp_port, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        soc_mem_field32_set(unit, packet_processing_port_mem, entry, IEEE_1588_MAC_ENABLEf, ext_mac_enable ? 0 : 1);
        res = soc_mem_write(unit, packet_processing_port_mem, EPNI_BLOCK(unit, core), pp_port, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }else {
        if (TRUE == ext_mac_enable) {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry));           
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, RESET_TIME_STAMPf,  0);            
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, EXTERNAL_BRCM_MACf, 0);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core), pp_port, entry));
        }else {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry));           
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, RESET_TIME_STAMPf,  1);            
            soc_IHB_PINFO_LBPm_field32_set(unit, entry, EXTERNAL_BRCM_MACf, 1);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core), pp_port, entry));
        }

        /* disablling 1588 MAC Tx stamping */
        res = soc_mem_read(unit, packet_processing_port_mem, EPNI_BLOCK(unit, core), pp_port, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        soc_mem_field32_set(unit, packet_processing_port_mem, entry, IEEE_1588_MAC_ENABLEf, 0);
        res = soc_mem_write(unit, packet_processing_port_mem, EPNI_BLOCK(unit, core), pp_port, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }


    

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ptp_port_set_unsafe()", 0, 0);
}


/*********************************************************************
*      get 1588 port status
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_ptp_port_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                           local_port_ndx,
    SOC_SAND_OUT SOC_PPC_PTP_IN_PP_PORT_PROFILE        *profile
  )
{
    uint32
        res = SOC_SAND_OK;
    int core=0;
    uint32 pp_port;
    uint32 entry[4];
    soc_mem_t
        pinfo_flp_mem = SOC_IS_JERICHO(unit) ? IHP_PINFO_FLP_1m : IHB_PINFO_FLPm;

     SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_PTP_PORT_GET_UNSAFE);

     res = soc_port_sw_db_local_to_pp_port_get(unit, local_port_ndx, &pp_port, &core);
     SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

     /* get PTP port profile */
     if (SOC_IS_JERICHO(unit)) {
        res = soc_mem_read(unit, pinfo_flp_mem, IHP_BLOCK(unit, core), pp_port, entry);
     }else {
        res = soc_mem_read(unit, pinfo_flp_mem, MEM_BLOCK_ANY, pp_port, entry);
     }
     SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
     *profile = soc_mem_field32_get(unit, pinfo_flp_mem, entry, IEEE_1588_PROFILEf);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ptp_port_get_unsafe()", 0, 0);
}







/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_ptp module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_ptp_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_ptp;
}

/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_oam module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_ptp_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_ptp;
}

/*********************************************************************
*     Update path delay in p2p delay mchanism.
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  arad_pp_ptp_p2p_delay_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_PORT               local_port_ndx,
    SOC_SAND_IN  int                        value
  )
{
    uint32 entry[2];
    int rv;
    int core=0;
    uint32 pp_port;

    SOCDNX_INIT_FUNC_DEFS;	  

    rv = soc_port_sw_db_local_to_pp_port_get(unit, local_port_ndx, &pp_port, &core);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry);
    SOCDNX_IF_ERR_EXIT(rv);
    soc_IHB_PINFO_LBPm_field32_set(unit, entry, INGRESS_P2P_DELAYf, value);
    /* Write data to HW. */
    rv = WRITE_IHB_PINFO_LBPm(unit, IHB_BLOCK(unit, core), pp_port, entry);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Read path delay in p2p delay mchanism.
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  arad_pp_ptp_p2p_delay_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_PORT               local_port_ndx,
    SOC_SAND_OUT  int*                      value
  )
{
    uint32 entry[2];
    int rv;
    int core=0;
    uint32 pp_port;

    SOCDNX_INIT_FUNC_DEFS;	  

    rv = soc_port_sw_db_local_to_pp_port_get(unit, local_port_ndx, &pp_port, &core);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = READ_IHB_PINFO_LBPm(unit,  IHB_BLOCK(unit, core), pp_port, entry);
    SOCDNX_IF_ERR_EXIT(rv);

    *value = soc_mem_field32_get(unit, IHB_PINFO_LBPm, entry, INGRESS_P2P_DELAYf);

exit:
    SOCDNX_FUNC_RETURN;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */


