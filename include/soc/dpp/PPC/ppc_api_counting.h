/* $Id: ppc_api_counting.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_counting.h
*
* MODULE PREFIX:  soc_ppc_counting
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

#ifndef __SOC_PPC_API_COUNTING_INCLUDED__
/* { */
#define __SOC_PPC_API_COUNTING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_64cnt.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     indicates not to assign counter to the traffic          */

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
   *  Counts bytes
   */
  SOC_PPC_CNT_COUNTER_TYPE_BYTES = 0,
  /*
   *  Counts packets
   */
  SOC_PPC_CNT_COUNTER_TYPE_PACKETS = 1,
  /*
   *  Counts both packets and bytes
   */
  SOC_PPC_CNT_COUNTER_TYPE_BYTES_AND_PACKETS = 2,
  /*
   *  Six counters: Three colors, each counted both per bytes
   *  and per packets
   */
  SOC_PPC_CNT_COUNTER_TYPE_PER_COLOR = 3,
  /*
   *  Number of types in SOC_PPC_CNT_COUNTER_TYPE
   */
  SOC_PPC_NOF_CNT_COUNTER_TYPES = 4
}SOC_PPC_CNT_COUNTER_TYPE;

typedef enum
{
  /*
   *  Packets are counted according to the incoming logical
   *  interface they attached to. Soc_petra-B only.
   */
  SOC_PPC_CNT_TYPE_INLIF = 0,
  /*
   *  Packets are counted according to the Outgoing AC they
   *  are sent from. Soc_petra-B only.
   */
  SOC_PPC_CNT_TYPE_OUTLIF = 1,
  /*
   *  Packets are counted according to the Ingress AC. T20E
   *  only.
   */
  SOC_PPC_CNT_TYPE_IN_AC = 2,
  /*
   *  Packets are counted according to the Egress AC they are
   *  sent from. T20E only.
   */
  SOC_PPC_CNT_TYPE_OUT_AC = 3,
  /*
   *  Packets are counted according to the VSID they are
   *  attached to
   */
  SOC_PPC_CNT_TYPE_VSID = 4,
  /*
   *  Packets are counted according to the incoming PWE label.
   *  T20E only.
   */
  SOC_PPC_CNT_TYPE_IN_PWE = 5,
  /*
   *  Packets are counted according to the outgoing PWE label
   *  . T20E only.
   */
  SOC_PPC_CNT_TYPE_OUT_PWE = 6,
  /*
   *  Packets are counted according to the Ingress terminated
   *  label (Tunnel/LSR label)
   */
  SOC_PPC_CNT_TYPE_IN_LABEL = 7,
  /*
   *  Packets are counted according to the Egress label they
   *  sent with. In T20E this will count according to EPP used
   *  for tunnel encapsulation, and then counter_id = EEP
   *  value.
   */
  SOC_PPC_CNT_TYPE_OUT_LABEL = 8,
  /*
   *  Soc_petra-B only, Packets are counted according to the FEC
   *  entry they were forwarded by.
   */
  SOC_PPC_CNT_TYPE_FEC_ID = 9,
  /*
   *  Number of types in SOC_PPC_CNT_TYPE
   */
  SOC_PPC_NOF_CNT_TYPES = 10
}SOC_PPC_CNT_TYPE;

typedef enum
{
  /*
   *  Ingress Counter.
   */
  SOC_PPC_CNT_COUNTER_STAGE_INGRESS = 0,
  /*
   *  Egress Counter.
   */
  SOC_PPC_CNT_COUNTER_STAGE_EGRESS = 1,
  /*
   *  Number of types in SOC_PPC_CNT_COUNTER_STAGE
   */
  SOC_PPC_NOF_CNT_COUNTER_STAGES = 2
}SOC_PPC_CNT_COUNTER_STAGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Counter stage: ingress or egress.
   */
  SOC_PPC_CNT_COUNTER_STAGE stage;
  /*
   *  Group selection. There are two groups of counters, and
   *  each packet may be assigned one counter from each group.
   *  Soc_petra-B only. In T20E has to be zero. Note: In T20E
   *  packet is assigned two counters according to Tunnel and
   *  PWE.
   */
  uint32 group;
  /*
   *  number that Identifies the counter
   */
  uint32 id;

} SOC_PPC_CNT_COUNTER_ID;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  LIF/VSI/...
   */
  SOC_PPC_CNT_TYPE type;
  /*
   *  Id according to type (LIF ID/ VSID / ...)
   */
  uint32 id;

} SOC_PPC_CNT_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Value of the packet counter. Units: Packets
   */
  SOC_SAND_64CNT pkt_cnt;
  /*
   *  Value of the byte counter. Units: Bytes
   */
  SOC_SAND_64CNT byte_cnt;

} SOC_PPC_CNT_RESULT;


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
  SOC_PPC_CNT_COUNTER_ID_clear(
    SOC_SAND_OUT SOC_PPC_CNT_COUNTER_ID *info
  );

void
  SOC_PPC_CNT_KEY_clear(
    SOC_SAND_OUT SOC_PPC_CNT_KEY *info
  );

void
  SOC_PPC_CNT_RESULT_clear(
    SOC_SAND_OUT SOC_PPC_CNT_RESULT *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_CNT_COUNTER_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_CNT_COUNTER_TYPE enum_val
  );

const char*
  SOC_PPC_CNT_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_CNT_TYPE enum_val
  );

const char*
  SOC_PPC_CNT_COUNTER_STAGE_to_string(
    SOC_SAND_IN  SOC_PPC_CNT_COUNTER_STAGE enum_val
  );

void
  SOC_PPC_CNT_COUNTER_ID_print(
    SOC_SAND_IN  SOC_PPC_CNT_COUNTER_ID *info
  );

void
  SOC_PPC_CNT_KEY_print(
    SOC_SAND_IN  SOC_PPC_CNT_KEY *info
  );

void
  SOC_PPC_CNT_RESULT_print(
    SOC_SAND_IN  SOC_PPC_CNT_RESULT *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_COUNTING_INCLUDED__*/
#endif

