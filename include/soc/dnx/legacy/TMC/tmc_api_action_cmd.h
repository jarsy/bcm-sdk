/* $Id: jer2_jer2_jer2_tmc_api_action_cmd.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_action_cmd.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcaction
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

#ifndef __DNX_TMC_API_ACTION_CMD_INCLUDED__
/* { */
#define __DNX_TMC_API_ACTION_CMD_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_pp_general.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*Flags for DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO*/
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_IN_PORT           (1 << 0)
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_VSQ               (1 << 1)
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_CNM_CANCEL        (1 << 2)
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_TRUNK_HASH_RESULT (1 << 3)
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_ECN_VALUE         (1 << 4)

/*Flags for DNX_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO*/
#define DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_STAMPING_FHEI     (1 << 0)

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
  DNX_TMC_ACTION_CMD_SIZE_BYTES_64 = 0,
  /*
   *  Snoop the first 128 bytes of the packet.
   */
  DNX_TMC_ACTION_CMD_SIZE_BYTES_128 = 1,
  /*
   *  Snoop the first 192 bytes of the packet.
   */
  DNX_TMC_ACTION_CMD_SIZE_BYTES_192 = 2,

  DNX_TMC_ACTION_CMD_SIZE_BYTES_256 = 3,
  /*
   *  Snoop the whole packet
   */
  DNX_TMC_ACTION_CMD_SIZE_BYTES_ALL_PCKT = 4,
  /*
   *  Number of types in DNX_TMC_ACTION_CMD_SIZE_BYTES
   */
  DNX_TMC_ACTION_NOF_CMD_SIZE_BYTESS = 5
}DNX_TMC_ACTION_CMD_SIZE_BYTES;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The override value
   */
  uint32 value;
  /*
   *  If True, the override is enabled with the new value
   *  'value'.
   */
  uint8 enable;

} DNX_TMC_ACTION_CMD_OVERRIDE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Destination ID of the packet. Can be a System-Port, a
   *  Flow-ID, a Multicast-ID, or a FEC-Pointer.
   */
  DNX_TMC_DEST_INFO dest_id;
  /*
   *  Override Traffic Class, if enabled. Value Range: 0 - 7.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE tc;
  /*
   *  Override Drop precedence, if enabled. Value Range: 0 -
   *  3.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE dp;
  /*
   *  Override low meter pointer, if enabled. Value Range: 0 -
   *  8K-1.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE meter_ptr_low;
  /*
   *  Override up meter pointer, if enabled. Value Range: 0 -
   *  8K-1.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE meter_ptr_up;
  /*
   *  Override DP meter command, if enabled. Indicate how to
   *  apply the meter result. Must be set only if there is a
   *  valid meter pointer. Value Range: 0 - 3.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE meter_dp;
  /*
   *  Override the first counter pointer, if enabled. Indicate
   *  a counter-set to apply to the packet. Value Range: 0 -
   *  4K-1.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE counter_ptr_1;
  /*
   *  Override the second counter pointer, if enabled.
   *  Indicate a counter-set to apply to the packet. Value
   *  Range: 0 - 4K-1.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE counter_ptr_2;
  /*
   *  If TRUE, then the command is of type ingress multicast.
   */
  uint8 is_ing_mc;
  /*
   *  Relevant for mirroring with Unicast destination,
   *  to set the outlif for mirrored/snooped copy
   *  JER2_ARAD only.
   */
  DNX_TMC_ACTION_CMD_OVERRIDE outlif;

} DNX_TMC_ACTION_CMD;

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
} DNX_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The action command parameters
   */
  DNX_TMC_ACTION_CMD cmd;
  /*
   *  The size of the mirrored/snooped packet that is generated.
   */
  DNX_TMC_ACTION_CMD_SIZE_BYTES size;
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
                        DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO_*
                        */
  bcm_gport_t    in_port; /*In PP port*/
  uint16         vsq; /*Statistics Vsq pointer*/
  uint8          cnm_cancel; /*Ignore Congestion point*/
  uint32         trunk_hash_result; /*LAG load-balancing key*/
  uint8          ecn_value; /*Ethernet encapsulation*/  
  DNX_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO stamping;
} DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO;

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
  DNX_TMC_ACTION_CMD_OVERRIDE_clear(
    DNX_SAND_OUT DNX_TMC_ACTION_CMD_OVERRIDE *info
  );

void
  DNX_TMC_ACTION_CMD_clear(
    DNX_SAND_OUT DNX_TMC_ACTION_CMD *info
  );

void
    DNX_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO_clear( 
       DNX_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO *info
       );
void
  DNX_TMC_ACTION_CMD_SNOOP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  DNX_TMC_ACTION_CMD_MIRROR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_ACTION_CMD_SIZE_BYTES_to_string(
    DNX_SAND_IN  DNX_TMC_ACTION_CMD_SIZE_BYTES enum_val
  );

void
  DNX_TMC_ACTION_CMD_OVERRIDE_print(
    DNX_SAND_IN  DNX_TMC_ACTION_CMD_OVERRIDE *info
  );

void
  DNX_TMC_ACTION_CMD_print(
    DNX_SAND_IN  DNX_TMC_ACTION_CMD *info
  );

void
  DNX_TMC_ACTION_CMD_SNOOP_INFO_print(
    DNX_SAND_IN  DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  DNX_TMC_ACTION_CMD_MIRROR_INFO_print(
    DNX_SAND_IN  DNX_TMC_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_ACTION_CMD_INCLUDED__*/
#endif
