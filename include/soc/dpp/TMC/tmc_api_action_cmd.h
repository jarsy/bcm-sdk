/* $Id: tmc_api_action_cmd.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_action_cmd.h
*
* MODULE PREFIX:  soc_tmcaction
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

#ifndef __SOC_TMC_API_ACTION_CMD_INCLUDED__
/* { */
#define __SOC_TMC_API_ACTION_CMD_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*Flags for SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO*/
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_IN_PORT           (1 << 0)
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_VSQ               (1 << 1)
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_CNM_CANCEL        (1 << 2)
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_TRUNK_HASH_RESULT (1 << 3)
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_ECN_VALUE         (1 << 4)

/*Flags for SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO*/
#define SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_STAMPING_FHEI     (1 << 0)

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
   *  Snoop the first 64 bytes of the packet.
   */
  SOC_TMC_ACTION_CMD_SIZE_BYTES_64 = 0,
  /*
   *  Snoop the first 128 bytes of the packet.
   */
  SOC_TMC_ACTION_CMD_SIZE_BYTES_128 = 1,
  /*
   *  Snoop the first 192 bytes of the packet.
   */
  SOC_TMC_ACTION_CMD_SIZE_BYTES_192 = 2,

  SOC_TMC_ACTION_CMD_SIZE_BYTES_256 = 3,
  /*
   *  Snoop the whole packet
   */
  SOC_TMC_ACTION_CMD_SIZE_BYTES_ALL_PCKT = 4,
  /*
   *  Number of types in SOC_TMC_ACTION_CMD_SIZE_BYTES
   */
  SOC_TMC_ACTION_NOF_CMD_SIZE_BYTESS = 5
}SOC_TMC_ACTION_CMD_SIZE_BYTES;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The override value
   */
  uint32 value;
  /*
   *  If True, the override is enabled with the new value
   *  'value'.
   */
  uint8 enable;

} SOC_TMC_ACTION_CMD_OVERRIDE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Destination ID of the packet. Can be a System-Port, a
   *  Flow-ID, a Multicast-ID, or a FEC-Pointer.
   */
  SOC_TMC_DEST_INFO dest_id;
  /*
   *  Override Traffic Class, if enabled. Value Range: 0 - 7.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE tc;
  /*
   *  Override Drop precedence, if enabled. Value Range: 0 -
   *  3.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE dp;
  /*
   *  Override low meter pointer, if enabled. Value Range: 0 -
   *  8K-1.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE meter_ptr_low;
  /*
   *  Override up meter pointer, if enabled. Value Range: 0 -
   *  8K-1.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE meter_ptr_up;
  /*
   *  Override DP meter command, if enabled. Indicate how to
   *  apply the meter result. Must be set only if there is a
   *  valid meter pointer. Value Range: 0 - 3.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE meter_dp;
  /*
   *  Override the first counter pointer, if enabled. Indicate
   *  a counter-set to apply to the packet. Value Range: 0 -
   *  4K-1.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE counter_ptr_1;
  /*
   *  Override the second counter pointer, if enabled.
   *  Indicate a counter-set to apply to the packet. Value
   *  Range: 0 - 4K-1.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE counter_ptr_2;
  /*
   *  If TRUE, then the command is of type ingress multicast.
   */
  uint8 is_ing_mc;
  /*
   *  Relevant for mirroring with Unicast destination,
   *  to set the outlif for mirrored/snooped copy
   *  ARAD only.
   */
  SOC_TMC_ACTION_CMD_OVERRIDE outlif;

} SOC_TMC_ACTION_CMD;

typedef struct {
    uint32 valid;
    /*
     *  Each packet that is trapped to the CPU from the PP
     *  engine is attached with a trap code ID, specifying the
     *  location in the processing Pipe that causes the
     *  TRAP. Range: 0 - 255.
     */
    uint32 cpu_trap_code;
    /*
     *  Additional information on top of the Trap-Code, e.g, the
     *  VSI that the packet was assigned to. Range: 0 - 4095.
     */
    uint32 cpu_trap_qualifier;
    /* snoop with new encapsulation */
    uint32         encap_id;
} SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The action command parameters
   */
  SOC_TMC_ACTION_CMD cmd;
  /*
   *  The size of the mirrored/snooped packet that is generated.
   */
  SOC_TMC_ACTION_CMD_SIZE_BYTES size;
  /*
   *  Probability of mirror/snoop execution.
   */
  uint32 prob;
  /*
  *   Probability of outbound mirror/snoop execution.
  */
  uint32 outbound_prob;
  /*

     info for EPNI_REC_CMD_CONF memory 

  */
  uint32         is_trap; /* If set packet goes to Trap FIFO, otherwise it goes to Mirror FIFO. */
  uint32         crop_pkt;  /* If set EGQ will send to mirror: first 32B of the original headers, time stamp, 
                             *  and maximum first 128 bytes of the packet after editing 
                             *(AddOrigHead means also cropping the edited packet to mirror).
                             */
  uint32         add_orig_head; /* The start of the original packet system header will be included in the outbound mirrored packets. */

  /* 
   
    Jericho new features
   
  */
  uint32         valid; /*Used to specify which fields to use.
                        Possible values will be named 
                        SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_*
                        */
  bcm_gport_t    in_port; /*In PP port*/
  uint16         vsq; /*Statistics Vsq pointer*/
  uint8          cnm_cancel; /*Ignore Congestion point*/
  uint32         trunk_hash_result; /*LAG load-balancing key*/
  uint8          ecn_value; /*Ethernet encapsulation*/  
  SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO stamping;
} SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO;

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
  SOC_TMC_ACTION_CMD_OVERRIDE_clear(
    SOC_SAND_OUT SOC_TMC_ACTION_CMD_OVERRIDE *info
  );

void
  SOC_TMC_ACTION_CMD_clear(
    SOC_SAND_OUT SOC_TMC_ACTION_CMD *info
  );

void
    SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO_clear( 
       SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO *info
       );
void
  SOC_TMC_ACTION_CMD_SNOOP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  SOC_TMC_ACTION_CMD_MIRROR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_ACTION_CMD_SIZE_BYTES_to_string(
    SOC_SAND_IN  SOC_TMC_ACTION_CMD_SIZE_BYTES enum_val
  );

void
  SOC_TMC_ACTION_CMD_OVERRIDE_print(
    SOC_SAND_IN  SOC_TMC_ACTION_CMD_OVERRIDE *info
  );

void
  SOC_TMC_ACTION_CMD_print(
    SOC_SAND_IN  SOC_TMC_ACTION_CMD *info
  );

void
  SOC_TMC_ACTION_CMD_SNOOP_INFO_print(
    SOC_SAND_IN  SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  SOC_TMC_ACTION_CMD_MIRROR_INFO_print(
    SOC_SAND_IN  SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_ACTION_CMD_INCLUDED__*/
#endif
