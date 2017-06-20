/* $Id: tmc_api_cnt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_cnt.h
*
* MODULE PREFIX:  soc_tmccnt
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

#ifndef __SOC_TMC_API_CNT_INCLUDED__
/* { */
#define __SOC_TMC_API_CNT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/TMC/tmc_api_general.h>
#include <soc/types.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/*Invalid DMA-chanel*/
#define  SOC_TMC_CNT_INVALID_DMA_CHANNEL (0xff)
#define  SOC_TMC_CNT_RESERVED_DMA_CHANNEL (0xfe)
#define  SOC_TMC_CNT_INVALID_FIFO (0xff)

/*     Cache length containing counters chosen by the polling
 *     algorithm.                                              */
#define  SOC_TMC_CNT_CACHE_LENGTH_PB (16)
#define  SOC_TMC_CNT_CACHE_LENGTH_ARAD (16*1024)  
#define  SOC_TMC_CNT_MAX_NUM_OF_FIFOS_PER_PROC (6)

#define SOC_TMC_CNT_BMAP_OFFSET_INVALID_VAL (7)

#define SOC_TMC_NOF_COUNTERS_IN_ENGINE(unit, proc_id) \
                                    ((proc_id < SOC_DPP_DEFS_GET(unit, nof_counter_processors)) ? \
                                     (SOC_DPP_DEFS_GET(unit, counters_per_counter_processor)) : \
                                     (SOC_DPP_DEFS_GET(unit, counters_per_small_counter_processor)))

#define SOC_TMC_COUNTER_NDX_MAX(unit, proc_id) (SOC_TMC_NOF_COUNTERS_IN_ENGINE(unit, proc_id) - 1)


#define EIGHT_BITS (8)
#define MAX_ING_COMP_LIF_NUMBER (32)
#define MAX_ING_COMP_DELTA_VALUE (127)
#define MIN_ING_COMP_DELTA_VALUE (-128)

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
    SOC_TMC_CNT_PROCESSOR_ID_A = 0,
    SOC_TMC_CNT_PROCESSOR_ID_B = 1,
   /*
    *  Number of types in SOC_TMC_CNT_PROCESSOR_ID
    */
    SOC_TMC_CNT_NOF_PROCESSOR_IDS_PETRA_B = 2,
    SOC_TMC_CNT_PROCESSOR_ID_C = 2,
    SOC_TMC_CNT_PROCESSOR_ID_D = 3,
    /*
     *  Number of types in SOC_TMC_CNT_PROCESSOR_ID
     */
    SOC_TMC_CNT_NOF_PROCESSOR_IDS_ARAD = 4,
    SOC_TMC_CNT_PROCESSOR_ID_E = 4,
    SOC_TMC_CNT_PROCESSOR_ID_F = 5,
    SOC_TMC_CNT_PROCESSOR_ID_G = 6,
    SOC_TMC_CNT_PROCESSOR_ID_H = 7,
    SOC_TMC_CNT_PROCESSOR_ID_I = 8,
    SOC_TMC_CNT_PROCESSOR_ID_J = 9,
    SOC_TMC_CNT_PROCESSOR_ID_K = 10,
    SOC_TMC_CNT_PROCESSOR_ID_L = 11,
    SOC_TMC_CNT_PROCESSOR_ID_M = 12,
    SOC_TMC_CNT_PROCESSOR_ID_N = 13,
    SOC_TMC_CNT_PROCESSOR_ID_O = 14,
    SOC_TMC_CNT_PROCESSOR_ID_P = 15,
    SOC_TMC_CNT_PROCESSOR_ID_Q = 16,
    SOC_TMC_CNT_PROCESSOR_ID_R = 17,

   /*
    *  Number of types in SOC_TMC_CNT_PROCESSOR_ID
    */
    SOC_TMC_CNT_NOF_PROCESSOR_IDS_JERICHO = 12
}SOC_TMC_CNT_PROCESSOR_ID;

#define SOC_TMC_CNT_NOF_PROCESSOR_IDS                           SOC_TMC_CNT_NOF_PROCESSOR_IDS_PETRA_B

typedef enum
{
  /*
   *  Sets of two consecutive counters: enqueued data and
   *  discarded data. Must be set if the source type is
   *  'CNM_ID'.
   */
  SOC_TMC_CNT_MODE_STATISTICS_NO_COLOR = 0,
  /*
   *  Sets of four consecutive counters: enqueued green,
   *  discarded green, enqueued yellow, and discarded yellow
   *  and red data.
   */
  SOC_TMC_CNT_MODE_STATISTICS_COLOR_RES_LOW = 1,
  /*
   *  Sets of five consecutive counters: enqueued green,
   *  discarded green, enqueued yellow, discarded yellow, and
   *  discarded red data.
   */
  SOC_TMC_CNT_MODE_STATISTICS_COLOR_RES_HI = 2,
  /*
   *  Number of types in SOC_TMC_CNT_MODE_STATISTICS
   */
  SOC_TMC_CNT_NOF_MODE_INGS_PETRA_B = 3,

  /*
   *  Only EnQueue packets, 2 counters per set: green vs yellow & red.
   */
  SOC_TMC_CNT_MODE_STATISTICS_COLOR_RES_ENQ_HI = 4,
  /*
   *  Sets of one counter: enqueued data
   */
  SOC_TMC_CNT_MODE_STATISTICS_FWD_NO_COLOR = 5,
  /*
   *  Sets of one counter: discarded data.
   */
  SOC_TMC_CNT_MODE_STATISTICS_DROP_NO_COLOR = 6,
  /*
   *  Sets of one counter: data
   */
  SOC_TMC_CNT_MODE_STATISTICS_ALL_NO_COLOR = 7,
  /*
   *  Only EnQueue packets, 2 counters per set: fwd green vs fwd not-green 
   */
  SOC_TMC_CNT_MODE_STATISTICS_FWD_SIMPLE_COLOR = 8,
  /*
   *  Only EnQueue packets, 2 counters per set: drop green vs drop not-green 
   */
  SOC_TMC_CNT_MODE_STATISTICS_DROP_SIMPLE_COLOR = 9,
  /*
   * Config the entry to count according to custom_mode_params
   */
  SOC_TMC_CNT_MODE_STATISTICS_CONFIGURABLE_OFFSETS = 10,
  /*
   *  Number of types in SOC_TMC_CNT_MODE_STATISTICS
   */
  SOC_TMC_CNT_NOF_MODE_INGS_ARAD = 10

}SOC_TMC_CNT_MODE_STATISTICS;
             
#define SOC_TMC_CNT_NOF_MODE_INGS                           SOC_TMC_CNT_NOF_MODE_INGS_PETRA_B

typedef enum
{
  /*
   *  Sets of one counter: only enqueued data.
   */
  SOC_TMC_CNT_MODE_EG_RES_NO_COLOR = 0,
  /*
   *  Sets of two consecutive counters: enqueued green and
   *  enqueued yellow.
   */
  SOC_TMC_CNT_MODE_EG_RES_COLOR = 1,
  /*
   *  Number of types in SOC_TMC_CNT_MODE_EG
   */
  SOC_TMC_CNT_NOF_MODE_EGS = 2
}SOC_TMC_CNT_MODE_EG_RES;




typedef enum
{
  /*
   *  The counting command source is the ingress Dune packet
   *  processor.
   */
  SOC_TMC_CNT_SRC_TYPE_ING_PP = 0,
  /*
   *  The counting is done on the VOQs. If this option is
   *  chosen, the 'voq_cnt' parameter must be set to define
   *  which VOQ are counted.
   */
  SOC_TMC_CNT_SRC_TYPE_VOQ = 1,
  /*
   *  The counting is done according to 12 configurable
   *  consecutive bits in the Statistic tag. If this option is
   *  chosen, the 'stag_first_bit' parameter must be set to
   *  define which bit range in the Statistic-Tag is
   *  significant. The effective range depends on the counting
   *  mode: e.g., with the 'NO_COLOR' mode, the all range (4K)
   *  is significant. With the 'COLOR_RES_LOW' (resp. 'HIGH'),
   *  the effective range is 0 - 2K-1 (resp. 1638, i.e.
   *  8K/5-1).
   */
  SOC_TMC_CNT_SRC_TYPE_STAG = 2,
  /*
   *  The counting is done on the VSQs.
   */
  SOC_TMC_CNT_SRC_TYPE_VSQ = 3,
  /*
   *  The counting is done per CNM id. The count starts only
   *  if a CNM packet is generated. The counting mode must be
   *  'NO_COLOR'.
   */
  SOC_TMC_CNT_SRC_TYPE_CNM_ID = 4,
  /*
   *  The counting source is the egress Dune packet processor.
   *  This mode can be used only if the processor index is B.
   */
  SOC_TMC_CNT_SRC_TYPE_EGR_PP = 5,
  /*
   *  Number of types in SOC_TMC_CNT_SRC_TYPE
   */
  SOC_TMC_CNT_NOF_SRC_TYPES_PETRA_B = 6,
  /*
   *  Ingress OAM 1-LSB / 1-MSB / 2-LSB / 2-MSB 
   */
  SOC_TMC_CNT_SRC_TYPE_OAM = 10,
  /*
   *  Egress EPNI 1-LSB / 1-MSB / 2-LSB / 2-MSB 
   */
  SOC_TMC_CNT_SRC_TYPE_EPNI = 11,
  /*
   *  Number of types in SOC_TMC_CNT_SRC_TYPE
   */
  SOC_TMC_CNT_NOF_SRC_TYPES_ARAD = 12,
  SOC_TMC_CNT_SRC_TYPES_EGQ_TM = 13,
  SOC_TMC_CNT_SRC_TYPES_IPT_LATENCY = 14,
  SOC_TMC_CNT_NOF_SRC_TYPES_JERICHO = 15
}SOC_TMC_CNT_SRC_TYPE;

#define SOC_TMC_CNT_NOF_SRC_TYPES(unit)                 (SOC_IS_ARADPLUS_AND_BELOW(unit) ? SOC_TMC_CNT_NOF_SRC_TYPES_ARAD : SOC_TMC_CNT_NOF_SRC_TYPES_JERICHO)

#define SOC_TMC_CNT_SRC_IS_EGRESS_TM(src_type, cnt_mode) (((src_type == SOC_TMC_CNT_SRC_TYPE_EPNI) || \
                                                           (src_type == SOC_TMC_CNT_SRC_TYPE_EGR_PP)) && \
                                                          ((cnt_mode == SOC_TMC_CNT_MODE_EG_TYPE_TM) || \
                                                          (cnt_mode == SOC_TMC_CNT_MODE_EG_TYPE_TM_PORT)))
typedef enum
{
  /*
   *  Each counter counts 1 consecutive queue.
   */
  SOC_TMC_CNT_Q_SET_SIZE_1_Q = 1,
  /*
   *  Each counter counts 2 consecutive queues.
   */
  SOC_TMC_CNT_Q_SET_SIZE_2_Q = 2,
  /*
   *  Each counter counts 4 consecutive queues.
   */
  SOC_TMC_CNT_Q_SET_SIZE_4_Q = 4,
  /*
   *  Each counter counts 8 consecutive queues.
   */
  SOC_TMC_CNT_Q_SET_SIZE_8_Q = 8,
  /*
   *  Number of types in SOC_TMC_CNT_Q_SET_SIZE
   */
  SOC_TMC_CNT_NOF_Q_SET_SIZES = 9
}SOC_TMC_CNT_Q_SET_SIZE;


typedef enum
{
  /*
   *  Counter-Id has the format: {Egress-MC(1),
   *  not(System-MC(1)), Queue-Pair(7), Traffic-Class(3)}.
   *  Must be set for Outgoing Ports of type TM.
   *  Configure per Queue-pair (and not per PP-Port)
   */
  SOC_TMC_CNT_MODE_EG_TYPE_TM = 0,
  /*
   *  Counter-Id equals the Packet VSI
   */
  SOC_TMC_CNT_MODE_EG_TYPE_VSI = 1,
  /*
   *  The Counter-Id equals the Packet Out-LIF
   */
  SOC_TMC_CNT_MODE_EG_TYPE_OUTLIF = 2,
  /*
   *  Number of types in SOC_TMC_CNT_MODE_EG_TYPE
   */
  SOC_TMC_CNT_NOF_MODE_EG_TYPES_PETRA_B = 3,
  /*
   *  The Counter-Id equals the Packet ACE-Pointer from PMF
   */
  SOC_TMC_CNT_MODE_EG_TYPE_PMF = 3,
  /*
   * Same as SOC_TMC_CNT_MODE_EG_TYPE_TM, but 
   *  Configure per PP-Port (and not per Queue-pair)
   */
  SOC_TMC_CNT_MODE_EG_TYPE_TM_PORT = 4,
  /*
   *  Number of types in SOC_TMC_CNT_MODE_EG_TYPE
   */
  SOC_TMC_CNT_NOF_MODE_EG_TYPES_ARAD = 5
}SOC_TMC_CNT_MODE_EG_TYPE;

/* Filter types */
typedef enum {
    SOC_TMC_CNT_TOTAL_PDS_THRESHOLD_VIOLATED, 
    SOC_TMC_CNT_TOTAL_PDS_UC_POOL_SIZE_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_PORT_UC_PDS_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_QUEUE_UC_PDS_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_PORT_UC_DBS_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_QUEUE_UC_DBS_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_QUEUE_DISABLE_BIT,
    SOC_TMC_CNT_TOTAL_PDS_MC_POOL_SIZE_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_INTERFACE_PDS_THREHOLD_VIOLATED,
    SOC_TMC_CNT_MC_SP_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_PER_MC_TC_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_MC_PDS_PER_PORT_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_MC_PDS_PER_QUEUE_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_MC_PER_PORT_SIZE_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_MC_PER_QUEUE_SIZE_THRESHOLD_VIOLATED,
    SOC_TMC_CNT_GLOBAL_REJECT_DISCARDS,
    SOC_TMC_CNT_DRAM_REJECT_DISCARDS,
    SOC_TMC_CNT_VOQ_TAIL_DROP_DISCARDS,
    SOC_TMC_CNT_VOQ_STATISTICS_DISCARDS,
    SOC_TMC_CNT_VSQ_TAIL_DROP_DISCARDS,
    SOC_TMC_CNT_VSQ_STATISTICS_DISCARDS,
    SOC_TMC_CNT_QUEUE_NOT_VALID_DISCARD,
    SOC_TMC_CNT_OTHER_DISCARDS,
    SOC_TMC_CNT_DROP_REASON_COUNT
} SOC_TMC_CNT_FILTER_TYPE;

#define SOC_TMC_CNT_NOF_MODE_EG_TYPES                           SOC_TMC_CNT_NOF_MODE_EG_TYPES_PETRA_B

typedef enum
{
    SOC_TMC_CNT_FORMAT_PKTS_AND_BYTES = 0,
    SOC_TMC_CNT_FORMAT_PKTS = 1,
    SOC_TMC_CNT_FORMAT_BYTES = 2,
    SOC_TMC_CNT_FORMAT_MAX_QUEUE_SIZE = 3,
    SOC_TMC_CNT_FORMAT_IHB_COMMANDS = 4, /*unused*/
    SOC_TMC_CNT_FORMAT_PKTS_AND_PKTS = 5,
    SOC_TMC_CNT_FORMAT_IPT_LATENCY = 6,
    SOC_TMC_CNT_NOF_FORMATS_ARAD = 6,
    SOC_TMC_CNT_NOF_FORMATS_JERICHO = 7
} SOC_TMC_CNT_FORMAT;

#define SOC_TMC_CNT_FORMAT_IS_WIDE(format) (format == SOC_TMC_CNT_FORMAT_PKTS || \
                                            format == SOC_TMC_CNT_FORMAT_BYTES || \
                                            format == SOC_TMC_CNT_FORMAT_MAX_QUEUE_SIZE || \
                                            format == SOC_TMC_CNT_FORMAT_IHB_COMMANDS || \
                                            format == SOC_TMC_CNT_FORMAT_IPT_LATENCY)
typedef enum
{
    /* count all packets, including replicated packets*/
    SOC_TMC_CNT_REPLICATED_PKTS_ALL = 0,
    /*count only forwarded packets, including Multicast replicated packets (but not snooped / mirrored packets)*/
    SOC_TMC_CNT_REPLICATED_PKTS_FRWRD_AND_MC = 1,
    /*count only forwarded packets*/
    SOC_TMC_CNT_REPLICATED_PKTS_FRWRD = 2,
    /*
    *  Number of types in SOC_TMC_CNT_REPLICATED_PKTS
    */
    SOC_TMC_CNT_NOF_REPLICATED_PKTS = 3

} SOC_TMC_CNT_REPLICATED_PKTS;

typedef struct
{
    int                     command_id;     /* Defines what counting command to read. */
    SOC_TMC_CNT_SRC_TYPE    source_type;    /* Defines the source type*/
} SOC_TMC_CNT_SOURCE;

typedef struct
{
  /*
   *  Define the egress counting resolution.
   */
  SOC_TMC_CNT_MODE_EG_RES resolution;
  /*
   *  Define the Counter-ID format.
   */
  SOC_TMC_CNT_MODE_EG_TYPE type;
  /*
   *  Counter-ID base value. Range: 0 - 8K-1.
   */
  uint32 base_val;

} SOC_TMC_CNT_MODE_EG;

typedef enum {
	SOC_TMC_CNT_BMAP_OFFSET_GREEN_FWD = 0,
	SOC_TMC_CNT_BMAP_OFFSET_GREEN_DROP,
	SOC_TMC_CNT_BMAP_OFFSET_YELLOW_FWD,
	SOC_TMC_CNT_BMAP_OFFSET_YELLOW_DROP,
	SOC_TMC_CNT_BMAP_OFFSET_RED_FWD,
	SOC_TMC_CNT_BMAP_OFFSET_RED_DROP,
	SOC_TMC_CNT_BMAP_OFFSET_BLACK_FWD,
	SOC_TMC_CNT_BMAP_OFFSET_BLACK_DROP,
	SOC_TMC_CNT_BMAP_OFFSET_COUNT
}SOC_TMC_CNT_BMAP_OFFSET_MAPPING;

/*each entry in the array is bitmap of which counters from SOC_TMC_CNT_BMAP_OFFSET_MAPPING is set*/
typedef struct
{
	uint32 entries_bmaps[SOC_TMC_CNT_BMAP_OFFSET_COUNT]; 
	uint32 set_size;
}SOC_TMC_CNT_CUSTOM_MODE_PARAMS;

typedef uint32 SOC_TMC_CNT_COUNTER_WE_BITMAP;

/* define the engine configuration */
typedef struct
{
  /*
   *  Define the counting source type.
   */
  SOC_TMC_CNT_SRC_TYPE src_type;
  /* this processor core ID of the source */
  int src_core;
  /*Select between different command per source types*/
  int command_id;
  /*
   *  Define the ingress counting mode. Soc_petra-B: Must be set only if
   *  the counting source is not 'EGR_PP'.
   */
  SOC_TMC_CNT_MODE_STATISTICS mode_statistics;
  /*
   *  Define the egress counting mode. Must be set only if the
   *  counting source is 'EGR_PP'.
   */
  SOC_TMC_CNT_MODE_EG mode_eg;
  /*
   *  Configuration of the VOQ counters. Must be set only if
   *  the counting source is 'VOQ'.
   *  Number of consecutive queues to count together per
   *  counter.
   */
  SOC_TMC_CNT_Q_SET_SIZE q_set_size;
  /*
   *  Define the first significant bit (for a range of
   *  consecutive 12 bits) in the Statistic-Tag for the
   *  counting. Must be set only if the counting source is
   *  'STAG'.
   */
  uint32 stag_lsb;
  /* Counter format: packet, bytes, both of them or queue size. Arad-only */
  SOC_TMC_CNT_FORMAT            format;
  /*Defines what range we should count from, relevent for Jericho and above*/
  SOC_TMC_CNT_COUNTER_WE_BITMAP we_val;
  /* Counter replication format. Arad-only */
  SOC_TMC_CNT_REPLICATED_PKTS   replicated_pkts;
  /* Custom mode params valid only when mode_ing. Arad plus only*/
  SOC_TMC_CNT_CUSTOM_MODE_PARAMS custom_mode_params;
  /* number of counter pairs */
  unsigned int num_counters;
  /* number of counter sets */
  unsigned int num_sets;
  /* set flag if multiple sources per engine can and will be used - QAX */
  uint8 multiple_sources;
} SOC_TMC_CNT_COUNTERS_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, at least a counter overflowed.
   */
  uint8 is_overflow;
  /*
   *  Counter index of the last counter pair that overflowed.
   *  Valid only if the overflow interrupt is set.
   */
  uint32 last_cnt_id;
  /*
   *  If True, the packet count overflowed for this last
   *  counter which overflowed.
   */
  uint8 is_pckt_overflow;
  /*
   *  If True, the byte count overflowed for this last counter
   *  which overflowed.
   */
  uint8 is_byte_overflow;

} SOC_TMC_CNT_OVERFLOW;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the cache for the polling algorithm is
   *  full. The user must read the cache immediately to avoid
   *  counter overflows.
   */
  uint8 is_cache_full;
  /*
   *  Indicate if a counter overflowed.
   */
  SOC_TMC_CNT_OVERFLOW overflow_cnt;
  /*
   *  Number of active (non-empty) pair of counters, i.e. at
   *  least one count (byte or octet) is not null.
   */
  uint32 nof_active_cnts;
  /*
   *  If True, at least a counter command tried to access an
   *  invalid counter index.
   */
  uint8 is_cnt_id_invalid;

} SOC_TMC_CNT_STATUS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Index of the returned counter entry, and counter proc.
   */
  uint32 counter_id;
  /* 
   * Index of the returned the countet engine
   */
  uint32 engine_id;
  /*
   *  Value of the packet counter. Units: Packets. Range: 0 -
   *  2^25-1.
   */
  uint64 pkt_cnt;
  /*
   *  Value of the byte counter. Units: Bytes. Range: 0 -
   *  2^32-1.
   */
  uint64 byte_cnt;

} SOC_TMC_CNT_RESULT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Array of the cache counters. Only the first
   *  'nof_counters' counters are relevant.
   */
  SOC_TMC_CNT_RESULT* cnt_result;
  /*
   *  Number of valid counters in the counter result
   *  'cnt_result'. Range: 0 - 16.
   */
  uint32 nof_counters;

} SOC_TMC_CNT_RESULT_ARR;

#define SOC_TMC_CNT_LIF_COUNTING_NUMBER_OF_STACK_IDS(unit, source) (((SOC_IS_JERICHO_B0_AND_ABOVE (unit) == TRUE) || (source == SOC_TMC_CNT_SRC_TYPE_ING_PP)) ? 4 : 3)
#define SOC_TMC_CNT_LIF_COUNTING_MASK_SINGLE_SIZE 2
#define SOC_TMC_CNT_LIF_COUNTING_MASK_SIZE(unit, source) (SOC_TMC_CNT_LIF_COUNTING_MASK_SINGLE_SIZE * SOC_TMC_CNT_LIF_COUNTING_NUMBER_OF_STACK_IDS(unit, source))
#define SOC_TMC_CNT_LIF_COUNTING_MASK_MAX(unit, source) ((1 << SOC_TMC_CNT_LIF_COUNTING_MASK_SIZE(unit, source)) - 1)

#define SOC_TMC_CNT_LIF_COUNTING_NOF_SOURCES 2
#define SOC_TMC_CNT_LIF_COUNTING_MAX_NOF_RANGES_PER_SOURCE 64 
#define SOC_TMC_CNT_LIF_COUNTING_MIN_NOF_RANGES_PER_SOURCE 2 /* In lif contain up to 2 ranges, also out lif in jericho */
#define SOC_TMC_CNT_LIF_COUNTING_NOF_RANGES_PER_SRC(_unit) (SOC_IS_JERICHO_PLUS(_unit) ? SOC_TMC_CNT_LIF_COUNTING_MAX_NOF_RANGES_PER_SOURCE : SOC_TMC_CNT_LIF_COUNTING_MIN_NOF_RANGES_PER_SOURCE) 
#define SOC_TMC_CNT_LIF_COUNTING_NOF_PROFILES(_unit) (SOC_IS_JERICHO_PLUS(_unit) ? \
        (SOC_TMC_CNT_LIF_COUNTING_MAX_NOF_RANGES_PER_SOURCE + SOC_TMC_CNT_LIF_COUNTING_MIN_NOF_RANGES_PER_SOURCE) : \
        (SOC_TMC_CNT_LIF_COUNTING_MIN_NOF_RANGES_PER_SOURCE*SOC_TMC_CNT_LIF_COUNTING_NOF_SOURCES))
#define SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, lif_stack_level) ((lif_stack_level >= SOC_TMC_CNT_LIF_COUNTING_NUMBER_OF_STACK_IDS(unit, source)) ? 0x0 : 0x3)
#define SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(lif_stack_level) (lif_stack_level * SOC_TMC_CNT_LIF_COUNTING_MASK_SINGLE_SIZE)
#define SOC_TMC_CNT_LIF_COUNTING_MASK_SET(unit, source, lif_counting_mask, lif_range_stack_0, lif_range_stack_1, lif_range_stack_2, lif_range_stack_3) \
            (lif_counting_mask = \
            ((SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, 0) & lif_range_stack_0) << SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(0)) | \
            ((SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, 1) & lif_range_stack_1) << SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(1)) | \
            ((SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, 2) & lif_range_stack_2) << SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(2)) | \
            ((SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, 3) & lif_range_stack_3) << SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(3)))

#define SOC_TMC_CNT_LIF_COUNTING_MASK_GET(unit, source, lif_counting_mask, lif_stack_level) \
                                ((lif_counting_mask >> SOC_TMC_CNT_LIF_COUNTING_MASK_SHIFT(lif_stack_level)) & SOC_TMC_CNT_LIF_COUNTING_MASK_MASK(unit, source, lif_stack_level))

#define SOC_TMC_CNT_LIF_COUNTING_MASK_TO_FIELD_VAL(lif_counting_mask_i)  (((~lif_counting_mask_i) & 0x2) | (lif_counting_mask_i & 0x1))

#define SOC_TMC_CNT_LIF_COUNTING_MASK_FIELD_VAL_GET(unit, source, lif_counting_mask, lif_stack_level) \
            SOC_TMC_CNT_LIF_COUNTING_MASK_TO_FIELD_VAL(SOC_TMC_CNT_LIF_COUNTING_MASK_GET(unit, source, lif_counting_mask, lif_stack_level))

/*In the ERPP - we cannot count the ountter-most-LIF and field-val 0 is don't count anything*/
#define SOC_TMC_CNT_LIF_COUNTING_STACK_LEVEL_TO_FIELD_VAL_GET(lif_stack_level, source) ((source == SOC_TMC_CNT_SRC_TYPE_EPNI) ? (lif_stack_level + 1) : lif_stack_level)
#define SOC_TMC_CNT_LIF_COUNTING_FIELD_VAL_TO_STACK_LEVEL_GET(lif_stack_level, source) ((source == SOC_TMC_CNT_SRC_TYPE_EPNI) ? (lif_stack_level - 1) : lif_stack_level)
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
  SOC_TMC_CNT_CUSTOM_MODE_PARAMS_clear(
    SOC_SAND_OUT SOC_TMC_CNT_CUSTOM_MODE_PARAMS *info
  );

void
  SOC_TMC_CNT_COUNTERS_INFO_clear(
     SOC_SAND_OUT int unit,
     SOC_SAND_OUT SOC_TMC_CNT_COUNTERS_INFO *info
  );

void
  SOC_TMC_CNT_OVERFLOW_clear(
    SOC_SAND_OUT SOC_TMC_CNT_OVERFLOW *info
  );

void
  SOC_TMC_CNT_STATUS_clear(
    SOC_SAND_OUT SOC_TMC_CNT_STATUS *info
  );

void
  SOC_TMC_CNT_RESULT_clear(
    SOC_SAND_OUT SOC_TMC_CNT_RESULT *info
  );

void
  SOC_TMC_CNT_RESULT_ARR_clear(
    SOC_SAND_OUT SOC_TMC_CNT_RESULT_ARR *info
  );

void
SOC_TMC_CNT_MODE_EG_clear(
  SOC_SAND_OUT SOC_TMC_CNT_MODE_EG *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_CNT_PROCESSOR_ID_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_PROCESSOR_ID enum_val
  );

const char*
  SOC_TMC_CNT_MODE_STATISTICS_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_MODE_STATISTICS enum_val
  );

const char*
  SOC_TMC_CNT_MODE_EG_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_MODE_EG_RES enum_val
  );

const char*
  SOC_TMC_CNT_SRC_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_SRC_TYPE enum_val
  );

const char*
  SOC_TMC_CNT_Q_SET_SIZE_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_Q_SET_SIZE enum_val
  );
const char*
SOC_TMC_CNT_MODE_EG_TYPE_to_string(
  SOC_SAND_IN  SOC_TMC_CNT_MODE_EG_TYPE enum_val
  );

const char*
SOC_TMC_CNT_FORMAT_to_string(
  SOC_SAND_IN  SOC_TMC_CNT_FORMAT enum_val
  );

const char*
  SOC_TMC_CNT_BMAP_OFFSET_MAPPING_to_string(
    SOC_SAND_IN  SOC_TMC_CNT_BMAP_OFFSET_MAPPING enum_val
  );

void
  SOC_TMC_CNT_CUSTOM_MODE_PARAMS_print(
    SOC_SAND_IN  SOC_TMC_CNT_CUSTOM_MODE_PARAMS *info
  );


void
  SOC_TMC_CNT_COUNTERS_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNT_COUNTERS_INFO *info
  );

void
  SOC_TMC_CNT_OVERFLOW_print(
    SOC_SAND_IN  SOC_TMC_CNT_OVERFLOW *info
  );

void
  SOC_TMC_CNT_STATUS_print(
    SOC_SAND_IN  SOC_TMC_CNT_STATUS *info
  );

void
  SOC_TMC_CNT_RESULT_print(
    SOC_SAND_IN  SOC_TMC_CNT_RESULT *info
  );
void SOC_TMC_CNT_MODE_EG_print
    (
    SOC_SAND_IN  SOC_TMC_CNT_MODE_EG *info
    );
void
  SOC_TMC_CNT_RESULT_ARR_print(
    SOC_SAND_IN  SOC_TMC_CNT_RESULT_ARR *info
  );


#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_CNT_INCLUDED__*/
#endif
