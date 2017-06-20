/* $Id: jer2_jer2_jer2_tmc_api_end2end_scheduler.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_END2END_SCHEDULER_INCLUDED__
/* { */
#define __DNX_TMC_API_END2END_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ofp_rates.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_packet_queuing.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* total number of SE regions */
#define DNX_TMC_COSQ_TOTAL_FLOW_REGIONS            128

#define DNX_TMC_COSQ_ANY_NOF_REMOTE_CORES          0xffff


#define DNX_TMC_SCH_MAX_NOF_DISCRETE_WEIGHT_VALS     4

/*     Overall number of interfaces -
 *     Channelized/Port NIFs, CPU, RCY, OLP, ERP            */
#define DNX_TMC_SCH_TOTAL_NOF_IFS                36

/*     Number of entries in scheduler interfaces structure    */
#define DNX_TMC_IF_NOF_ENTRIES               DNX_TMC_SCH_TOTAL_NOF_IFS

/*   Number of links towards the fabric                       */
#define DNX_TMC_SCH_NOF_LINKS                    36

/*   Number of RCI bucket levels                              */
#define DNX_TMC_SCH_NOF_RCI_LEVELS               8

/* DRM; links (0..36) X RCI bucket level (0..7)  = 37 * 8 = 296 */
#define DNX_TMC_SCH_DRT_SIZE                     296

/*     Number of CL types that can be defined in the SCT
 *     (scheduler class type) table                           */
#define DNX_TMC_SCH_NOF_CLASS_TYPES              256

/*     Maximal number of subflows in single flow
 *     If the flow is not composite,
 *     only the first subflow is active                       */
#define DNX_TMC_SCH_NOF_SUB_FLOWS                2

/*     Base flow id for aggregates.
 *     Flows with lower id-s can only be
 *     simple (queue-bounded) flows                           */
#define DNX_TMC_SCH_FLOW_BASE_AGGR_FLOW_ID_PETRA (24*1024)
#define DNX_TMC_SCH_MAX_SE_ID_PETRA              (16*1024 - 1)
/*     invalid scheduling element id (outside the allowed range)
 *                                                            */
#define DNX_TMC_SCH_SE_ID_INVALID_PETRA          (16*1024)

#define DNX_TMC_SCH_FLOW_BASE_AGGR_FLOW_ID_JER2_ARAD  (64*1024)
#define DNX_TMC_SCH_MAX_SE_ID_JER2_ARAD               (32*1024 - 1)
#define DNX_TMC_SCH_SE_ID_INVALID_JER2_ARAD           (32*1024)

/* 
 * invalid flow id (outside the allowed range)
 */
#define DNX_TMC_SCH_FLOW_ID_INVALID_PETRA        (56*1024)
#define DNX_TMC_SCH_MAX_FLOW_ID_PETRA            (56*1024 - 1)

#define DNX_TMC_SCH_FLOW_ID_INVALID_JER2_ARAD         (128*1024)
#define DNX_TMC_SCH_MAX_FLOW_ID_JER2_ARAD             (128*1024 - 1)

/* $Id: jer2_jer2_jer2_tmc_api_end2end_scheduler.h,v 1.14 Broadcom SDK $
 * In Soc_petra: 0-80 are data ports, and port 80 is the ERP port, that enable sending
 * FAP-level scheduled traffic to each FAP, to be egress replicated.
 */
#define DNX_TMC_SCH_MAX_PORT_ID_PETRA            80

/* 
 * In JER2_ARAD: 0-255 are data ports, and port 248 is the ERP port, that enable sending
 * FAP-level scheduled traffic to each FAP, to be egress replicated.
 */
#define DNX_TMC_SCH_MAX_PORT_ID_JER2_ARAD             255

/* 
 * invalid port id (outside the allowed range)
 */
#define DNX_TMC_SCH_PORT_ID_INVALID_PETRA        81
#define DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD         256


#define DNX_TMC_SCH_FLOW_HR_MIN_WEIGHT           1
#define DNX_TMC_SCH_FLOW_HR_MAX_WEIGHT           4096

#define DNX_TMC_SCH_SUB_FLOW_CL_MIN_WEIGHT           1
#define DNX_TMC_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODES_3_4 64
#define DNX_TMC_SCH_SUB_FLOW_CL_MAX_WEIGHT_MODE_5    256

#define DNX_TMC_SCH_NOF_SLOW_RATES                   2

/*      Number of low flow control configuration
 *      the device can hold at any point                      */
#define DNX_TMC_SCH_LOW_FC_NOF_AVAIL_CONFS           4

/*      number of possible flow control configurations.
 *      At any point, the device can only hold
 *      DNX_TMC_SCH_LOW_FC_NOF_AVAIL_CONFS out of
 *      these configurations                                  */
#define DNX_TMC_SCH_LOW_FC_NOF_VALID_CONFS           5

#define DNX_TMC_SCH_NOF_GROUPS                       3

/*    Number of configurable interface weights
 *                                                            */
#define DNX_TMC_SCH_NOF_IF_WEIGHTS                   8

/* When configuring scheduler port, this value indicates
 * that port scheduler groups will be configured automatically,
 * and not according to the expected port rate                */
#define DNX_TMC_SCH_PORT_MAX_EXPECTED_RATE_AUTO  1

#define DNX_TMC_SCH_SUB_FLOW_SHAPE_NO_LIMIT        0xffffffff
/*
 *	This affectively does not limit the burst.
 *  In fact, bigger sizes can be configured for
 *  bigger credit worth size
 */
#define DNX_TMC_SCH_SUB_FLOW_SHAPER_BURST_NO_LIMIT (0x1ff*(256))

/* Scheduling elements id ranges { */

/* 
*  In Soc_petra 
*  CL: 0     - 8191
*  FQ: 8192  - 16127
*  HR: 16128 - 16383
*/
#define DNX_TMC_CL_SE_ID_MIN_PETRA   0
#define DNX_TMC_CL_SE_ID_MAX_PETRA   (8  * 1024 - 1      )
#define DNX_TMC_FQ_SE_ID_MIN_PETRA   (8  * 1024          )
#define DNX_TMC_FQ_SE_ID_MAX_PETRA   (16 * 1024 - 256 - 1)
#define DNX_TMC_HR_SE_ID_MIN_PETRA   (16 * 1024 - 256    )
#define DNX_TMC_HR_SE_ID_MAX_PETRA   (16 * 1024 - 1      )

/* 
*  In JER2_ARAD 
*  CL: 0     - 16383
*  FQ: 16384 - 32511
*  HR: 32512 - 32767
*/
#define DNX_TMC_CL_SE_ID_MIN_JER2_ARAD    0
#define DNX_TMC_CL_SE_ID_MAX_JER2_ARAD    (16 * 1024 - 1      )
#define DNX_TMC_FQ_SE_ID_MIN_JER2_ARAD    (16 * 1024          )
#define DNX_TMC_FQ_SE_ID_MAX_JER2_ARAD    (32 * 1024 - 256 - 1)
#define DNX_TMC_HR_SE_ID_MIN_JER2_ARAD    (32 * 1024 - 256    )
#define DNX_TMC_HR_SE_ID_MAX_JER2_ARAD    (32 * 1024 - 1      )
#define DNX_TMC_HR_SE_REGION_ID      DNX_TMC_COSQ_TOTAL_FLOW_REGIONS


#define DNX_TMC_SCH_CL_OFFSET_IN_QUARTET     0
#define DNX_TMC_SCH_FQ_HR_OFFSET_IN_QUARTET  1

#define DNX_TMC_SCH_DESCRETE_WEIGHT_MAX 1024

#define DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES  (10)
/* Scheduling elements id ranges } */
/* } */

/*************
 * MACROS    *
 *************/
/* { */

#define DNX_TMC_SCH_FLOW_TO_QRTT_ID(flow_id) ((flow_id)/4)
#define DNX_TMC_SCH_QRTT_TO_FLOW_ID(q_flow_id) ((q_flow_id)*4)
#define DNX_TMC_SCH_FLOW_TO_1K_ID(flow_id) ((flow_id)/1024)
#define DNX_TMC_SCH_1K_TO_FLOW_ID(k_flow_id) ((k_flow_id)*1024)

#define DNX_TMC_SCH_FLOW_IS_IN_AGGR_RANGE_PETRA(flow_id) \
          ((flow_id) >= DNX_TMC_SCH_FLOW_BASE_AGGR_FLOW_ID_PETRA)
#define DNX_TMC_SCH_1K_FLOWS_IS_IN_AGGR_RANGE_PETRA(k_flow_id) \
          (DNX_TMC_SCH_FLOW_IS_IN_AGGR_RANGE_PETRA(DNX_TMC_SCH_1K_TO_FLOW_ID(k_flow_id)))

#define DNX_TMC_SCH_FLOW_IS_IN_AGGR_RANGE_JER2_ARAD(flow_id) \
          ((flow_id) >= DNX_TMC_SCH_FLOW_BASE_AGGR_FLOW_ID_JER2_ARAD)
#define DNX_TMC_SCH_1K_FLOWS_IS_IN_AGGR_RANGE_JER2_ARAD(k_flow_id) \
          (DNX_TMC_SCH_FLOW_IS_IN_AGGR_RANGE_JER2_ARAD(DNX_TMC_SCH_1K_TO_FLOW_ID(k_flow_id)))

#define DNX_TMC_SCH_FLOW_IS_EVEN(f) ((f)&0x1)==0)
#define DNX_TMC_SCH_FLOW_IS_ODD(f) (!(DNX_TMC_SCH_FLOW_IS_EVEN(f)))
#define DNX_TMC_SCH_FLOW_BASE_QRTT_ID(f) (((f)/4)*4)
#define DNX_TMC_SCH_FLOW_ID_IN_QRTT(f) ((f)%4)

/* The first subflow of a composite flow */
#define DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_1(sf) ((sf) & ~0x1)
#define DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_2(sf) ((sf) & ~0x2)
#define DNX_TMC_SCH_SUB_FLOW_BASE_FLOW(sf,is_odd_even) \
          ((is_odd_even)                         ? \
          DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_1(sf) : \
          DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_2(sf))

/* TRUE if the subflow sf is a subflow of flow f, OddEven = TRUE */
#define DNX_TMC_SCH_IS_SUB_FLOW_OF_FLOW_0_1(f,sf)    \
  ( ((sf)==DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_1(f)) || \
   ((sf)==DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_1(f)+1) )

/* TRUE if the subflow sf is a subflow of flow f, OddEven = FALSE
 * The test: same quartet, both odd/even  */
#define DNX_TMC_SCH_IS_SUB_FLOW_OF_FLOW_0_2(f,sf)    \
  ( ((sf)==DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_2(f)) || \
   ((sf)==DNX_TMC_SCH_SUB_FLOW_BASE_FLOW_0_2(f)+2) )

/* TRUE if the subflow sf is a subflow of flow f */
#define DNX_TMC_SCH_IS_SUB_FLOW_OF_FLOW(f,sf,is_odd_even)            \
   ( ((is_odd_even)&&(DNX_TMC_SCH_IS_SUB_FLOW_OF_FLOW_0_1(f,sf))) || \
   ((!(is_odd_even)&&(DNX_TMC_SCH_IS_SUB_FLOW_OF_FLOW_0_2(f,sf)))) )

/* In composite mode (two active subflows), OddEven = TRUE
 * TRUE if sf is a second subflow, and not an independent flow */
#define DNX_TMC_SCH_COMPOSITE_IS_SECOND_SUBFLOW_0_1(sf) ( (sf)&0x1 )

/* In composite mode (two active subflows), OddEven = FALSE
 * TRUE if sf is a second subflow, and not an independent flow */
#define DNX_TMC_SCH_COMPOSITE_IS_SECOND_SUBFLOW_0_2(sf) \
  ( (DNX_TMC_SCH_FLOW_ID_IN_QRTT(sf)==2) ||  (DNX_TMC_SCH_FLOW_ID_IN_QRTT(sf)==3) )

#define DNX_TMC_SCH_COMPOSITE_IS_SECOND_SUBFLOW(sf, is_odd_even) \
  ( ((is_odd_even)&&DNX_TMC_SCH_COMPOSITE_IS_SECOND_SUBFLOW_0_1(sf)) || \
     ((!(is_odd_even))&&DNX_TMC_SCH_COMPOSITE_IS_SECOND_SUBFLOW_0_2(sf)) )

/*
 * The following macros check scheduling element and flow validity.
 * The precondition for using these macros is, that invalid id-s
 * will be indicated as DNX_TMC_SCH_SE_ID_INVALID or DNX_TMC_SCH_FLOW_ID_INVALID
 * ID_INDICATION_VALIDITY {
 */
#define DNX_TMC_SCH_INDICATED_SE_ID_IS_VALID_PETRA(id) ((id)!= DNX_TMC_SCH_SE_ID_INVALID_PETRA)
#define DNX_TMC_SCH_INDICATED_PORT_ID_IS_VALID_PETRA(id) ((id)!= DNX_TMC_SCH_PORT_ID_INVALID_PETRA)

#define DNX_TMC_SCH_INDICATED_SE_ID_IS_VALID_JER2_ARAD(id) ((id)!= DNX_TMC_SCH_SE_ID_INVALID_JER2_ARAD)
#define DNX_TMC_SCH_INDICATED_FLOW_ID_IS_VALID_JER2_ARAD(id) ((id)!= DNX_TMC_SCH_FLOW_ID_INVALID_JER2_ARAD)
#define DNX_TMC_SCH_INDICATED_PORT_ID_IS_VALID_JER2_ARAD(id) ((id)!= DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD)
/* ID_INDICATION_VALIDITY } */

#define DNX_TMC_SCH_IS_DISCRETE_WFQ_MODE(m) \
  ( ((m) == DNX_TMC_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_FLOW) || \
    ((m) == DNX_TMC_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_CLASS) )

#define DNX_TMC_SCH_IS_INDEPENDENT_WFQ_MODE(m) \
  ((m) == DNX_TMC_SCH_CL_WEIGHTS_MODE_INDEPENDENT_PER_FLOW)

#define DNX_TMC_SCH_IS_WFQ_CLASS_VAL(class_val)                 \
  ( ((class_val) >= DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ) && \
    ((class_val) <= DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2)   \
  )

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*     Class types. Range: 0 - 255                             */
typedef uint32 DNX_TMC_SCH_CL_CLASS_TYPE_ID;

/*     Scheduler element id. Valid range 0 - 16K-1.             */
typedef uint32 DNX_TMC_SCH_SE_ID;

/*     Port id. Range: 0 - 80.                                 */
typedef uint32 DNX_TMC_SCH_PORT_ID;

/*     Flow id. Range: 0 - 56K-1.                              */
typedef uint32 DNX_TMC_SCH_FLOW_ID;

typedef enum
{
  /*
   *  undefined value
   */
  DNX_TMC_SCH_CL_MODE_NONE=0,
  /*
   * [SP1, SP2, SP3, SP4]
   */
  DNX_TMC_SCH_CL_MODE_1=1,
  /*
   * [SP1, SP2, SP3-WFQ(2)]
   */
  DNX_TMC_SCH_CL_MODE_2=2,
  /*
   *  [SP1-WFQ(1:63), SP2] or [SP1 WFQ(3),SP2]
   */
  DNX_TMC_SCH_CL_MODE_3=3,
  /*
   *  [SP1,SP2-WFQ(3)] or [SP1, SP2 WFQ(1:63)]
   */
  DNX_TMC_SCH_CL_MODE_4=4,
  /*
   *  WFQ (1:253) or [SP-WFQ(4)]
   */
  DNX_TMC_SCH_CL_MODE_5=5,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_CL_CLASS_MODE_LAST
}DNX_TMC_SCH_CL_CLASS_MODE;

typedef enum
{
  /*
   *  Each flow installed on the CL-Scheduler WFQ is assigned
   *  one of 2, 3, or 4 weights. The number of available
   *  weights depends on the number of strict-priority levels
   *  (i.e.1 level => 4 weights, 2 levels =>3 weights, 3
   *  levels =>2 weights). All flows installed on the WFQ
   *  compete according to that weight.
   */
  DNX_TMC_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_FLOW=0,
  /*
   *  Each flow installed on the CL-Scheduler WFQ has its own
   *  independent weight
   */
  DNX_TMC_SCH_CL_WEIGHTS_MODE_INDEPENDENT_PER_FLOW=1,
  /*
   *  Each flow installed on the CL-Scheduler WFQ is assigned
   *  to a class. Each class is assigned a weight. Bandwidth
   *  is distributed among the classes according to the weight
   *  of the class. All flows belonging to the class share the
   *  class bandwidth equally.
   */
  DNX_TMC_SCH_CL_WEIGHTS_MODE_DISCRETE_PER_CLASS=2,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE_LAST
}DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE;

typedef enum
{
  /*
   *  CL not in enhanced mode
   */
  DNX_TMC_CL_ENHANCED_MODE_DISABLED=0,
  /*
   *  CL in enhanced mode, additional FQ at high priority
   *  (SP1)
   */
  DNX_TMC_CL_ENHANCED_MODE_ENABLED_HP=1,
  /*
   *  CL in enhanced mode, additional FQ at low priority (SP5)
   */
  DNX_TMC_CL_ENHANCED_MODE_ENABLED_LP=2,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_CL_ENHANCED_MODE_LAST
}DNX_TMC_SCH_CL_ENHANCED_MODE;

typedef enum
{
  /*
   *  undefined value
   */
  DNX_TMC_SCH_SE_TYPE_NONE=0,
  /*
   *  HR scheduler element
   */
  DNX_TMC_SCH_SE_TYPE_HR=1,
  /*
   *  CL scheduler element
   */
  DNX_TMC_SCH_SE_TYPE_CL=2,
  /*
   *  FQ scheduler element
   */
  DNX_TMC_SCH_SE_TYPE_FQ=3,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_SE_TYPE_LAST
}DNX_TMC_SCH_SE_TYPE;

typedef enum
{
  /*
   *  undefined value
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_NONE = DNX_TMC_SCH_SE_TYPE_NONE,
  /*
   *  HR scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_HR = DNX_TMC_SCH_SE_TYPE_HR,
  /*
   *  CL scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_CL = DNX_TMC_SCH_SE_TYPE_CL,
  /*
   *  FQ scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_FQ = DNX_TMC_SCH_SE_TYPE_FQ,
  /*
   *  Connector
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_CONNECTOR,
  /*
   *  HR composite scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_HR_COMPOSITE,
  /*
   *  CL composite scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_CL_COMPOSITE,
  /*
   *  FQ composite scheduler element
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_FQ_COMPOSITE,
  /*
   *   Composite connector
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_CONNECTOR_COMPOSITE,
  /*
   *  Must be the last value
   */
  DNX_TMC_AM_SCH_FLOW_TYPE_LAST
}DNX_TMC_AM_SCH_FLOW_TYPE;


typedef enum
{
  /*
   *  scheduler element disabled
   */
  DNX_TMC_SCH_SE_STATE_DISABLE=0,
  /*
   *  scheduler element enabled
   */
  DNX_TMC_SCH_SE_STATE_ENABLE=1,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_SE_STATE_LAST
}DNX_TMC_SCH_SE_STATE;

typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_SCH_HR_MODE_NONE=0,
  /*
   *  SP1, SP2, SP3, SP4 WFQ(1:4K), SP5
   */
  DNX_TMC_SCH_HR_MODE_SINGLE_WFQ=1,
  /*
   *  SP1, SP2, SP3, SP4 WFQ(1:4K), SP5 WFQ(1:4K), SP6
   */
  DNX_TMC_SCH_HR_MODE_DUAL_WFQ=2,
  /*
   *  SP1, SP2, SP3, SP4, SP5, SP6, SP7, SP8, SP9, SP10-
   *  WFQ(1:4K), SP11
   */
  DNX_TMC_SCH_HR_MODE_ENHANCED_PRIO_WFQ=3,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_SE_HR_MODE_LAST
}DNX_TMC_SCH_SE_HR_MODE;

typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_SCH_FLOW_HR_CLASS_NONE=0,
  /*
   *  SP1 -strict priority one, the highest
   */
  DNX_TMC_SCH_FLOW_HR_CLASS_EF1=1,
  /*
   *  SP2
   */
  DNX_TMC_SCH_FLOW_HR_CLASS_EF2=2,
  /*
   *  SP3
   */
  DNX_TMC_SCH_FLOW_HR_CLASS_EF3=3,
  /*
   *  SP4 (WFQ)
   */
  DNX_TMC_SCH_FLOW_HR_SINGLE_CLASS_AF1_WFQ=4,
  /*
   *  SP5
   */
  DNX_TMC_SCH_FLOW_HR_SINGLE_CLASS_BE1=5,
  /*
   *  SP4 (WFQ)
   */
  DNX_TMC_SCH_FLOW_HR_DUAL_CLASS_AF1_WFQ=6,
  /*
   *  SP5 (WFQ)
   */
  DNX_TMC_SCH_FLOW_HR_DUAL_CLASS_BE1_WFQ=7,
  /*
   *  SP6
   */
  DNX_TMC_SCH_FLOW_HR_DUAL_CLASS_BE2=8,
  /*
   *  SP4
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF1=9,
  /*
   *  SP5
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF2=10,
  /*
   *  SP6
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF3=11,
  /*
   *  SP7
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF4=12,
  /*
   *  SP8
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF5=13,
  /*
   *  SP9
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_AF6=14,
  /*
   *  SP10 (WFQ)
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_BE1_WFQ=15,
  /*
   *  SP11
   */
  DNX_TMC_SCH_FLOW_HR_ENHANCED_CLASS_BE2=16,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_SUB_FLOW_HR_CLASS_LAST
}DNX_TMC_SCH_SUB_FLOW_HR_CLASS;

typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_NONE=0,
  /*
   *  SP1 - strict priority one, the highest
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1=1,
  /*
   *  SP2
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP2=2,
  /*
   *  SP3
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP3=3,
  /*
   *  SP4
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP4=4,
  /*
   *  SP1 with independent flow weight on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ=5,
  /*
   *  SP1 with discrete weight 1 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ1=6,
  /*
   *  SP1 with discrete weight 2 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ2=7,
  /*
   *  SP1 with discrete weight 3 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ3=8,
  /*
   *  SP1 with discrete weight 4 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP1_WFQ4=9,
  /*
   *  SP2 with independent flow weight on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ=10,
  /*
   *  SP2 with discrete weight 1 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ1=11,
  /*
   *  SP2 with discrete weight 2 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ2=12,
  /*
   *  SP2 with discrete weight 3 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP2_WFQ3=13,
  /*
   *  SP3 with discrete weight 1 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ1=14,
  /*
   *  SP3 with discrete weight 2 on WFQ
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP3_WFQ2=15,
  /*
   *  SP 0 when using enhanced CL
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP_0_ENHANCED=16,
  /*
   *  SP 5 when using enhanced CL
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_SP_5_ENHANCED=17,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_LAST
}DNX_TMC_SCH_SUB_FLOW_CL_CLASS;


typedef enum
{
  /*
   *  Independent-Per-Flow Weight configuration mode: inverse
   *  proportional. This is the default mode. The lower the
   *  flow's specifiedweight, the higher the bandwidth portion
   *  that the flow is awarded. The exact portionof the rate
   *  distributed to an active flow with weight Wn, under a
   *  WFQ with activeflows weights W1 - Wk is (1/Wn)/Sum(1/Wi)
   *  for i in [1..k]. This mode achieves a similar behavior
   *  for flow weight configuration in all modes, but limits
   *  the weight resolution for independent-per-flow mode. For
   *  example, for weights in maximal allowed range [1..63],
   *  all weights above 32, i.e. in range [33..63], will be
   *  configured as 1/63
   */
  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE_INVERSE = 0,
  /*
   *  Independent-Per-Flow Weight configuration mode:
   *  proportional. The higher the flow's specified weight,
   *  the higher the bandwidth portion that the flow is
   *  awarded. The portion of the rate distributed to an
   *  active flow with weight Wn, under a WFQ with active
   *  flows weights W1 - Wk is is Wn/Sum(Wi) for i in [1..k].
   *  This mode results in having a different behavior for
   *  independent-per-flow mode as compared to other modes,
   *  but preserves the weight resolution as supported by the
   *  device. For example, for weights in maximal allowed
   *  range [1..63], each weight will be configured
   *  accordingly on the device.
   */
  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE_PROPORTIONAL = 1,
  /*
   *  Total number of Independent-Per-Flow Weight
   *  configuration modes
   */
  DNX_TMC_SCH_NOF_FLOW_IPF_CONFIG_MODES = 2
}DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE;


/*
 * One of two system slow rates.
 * Relevant to Queue bounded (simple) flows only.
 */
typedef enum
{
  /*
   *  Use ShaperSlowRate1 when in slow state
   */
  DNX_TMC_SCH_SLOW_RATE_NDX_1=0,
  /*
   *  Use ShaperSlowRate1 when in slow state
   */
  DNX_TMC_SCH_SLOW_RATE_NDX_2=1,
  /*
   *  Total number of Shaprt Slow Rate indexes.
   */
  DNX_TMC_SCH_NOF_SLOW_RATE_NDXS=2
}DNX_TMC_SCH_SLOW_RATE_NDX;

typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_FLOW_NONE=0,
  /*
   *  simple - not an aggregate
   */
  DNX_TMC_FLOW_SIMPLE=1,
  /*
   *  aggregate - used both as a scheduling element and as a
   *  scheduler flow.
   */
  DNX_TMC_FLOW_AGGREGATE=2,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_FLOW_TYPE_LAST
}DNX_TMC_SCH_FLOW_TYPE;


typedef enum
{
  /*
   *  The flow does not consume credits.
   */
  DNX_TMC_SCH_FLOW_OFF=0,
  /*
   *  The flow consumes credits.
   */
  DNX_TMC_SCH_FLOW_ON=2,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_FLOW_STATUS_LAST
}DNX_TMC_SCH_FLOW_STATUS;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  RCI bucket level. Range: 0 - 7
   */
  uint32 rci_level;
  /*
   *  number of current active links range: 0 - 36
   */
  uint32 num_active_links;
  /*
   *  The credit generation rate, in Mega-Bit-Sec.
   *  If 0 - no credits will be generated.
   */
  uint32 rate;
}DNX_TMC_SCH_DEVICE_RATE_ENTRY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  table of rates in Mega-Bit-Sec, according to RCI level
   *  and number of active links
   */
  DNX_TMC_SCH_DEVICE_RATE_ENTRY rates[DNX_TMC_SCH_DRT_SIZE];
}DNX_TMC_SCH_DEVICE_RATE_TABLE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Interface weight index Range: 0-7
   */
  uint32 id;
  /*
   *  range: 0 - 1023. The interface portion of the bandwidth
   *  distributed by the WFQ will be: (1/my_weight) / (SUM:
   *  1/i_weight), when i is running on all interfaces. A Zero
   *  value stands for a non-active interface.
   */
  uint32 val;
}DNX_TMC_SCH_IF_WEIGHT_ENTRY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Interface weights.
   */
  DNX_TMC_SCH_IF_WEIGHT_ENTRY weight[DNX_TMC_SCH_NOF_IF_WEIGHTS];
  /*
   *  Range: 0-7
   */
  uint32 nof_enties;
}DNX_TMC_SCH_IF_WEIGHTS;

/*
 * The classes with priority specified by
 * DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS value and above
 * will not be effected by low priority flow control.
 * Those classes will only be effected by
 * high priority flow control.
 * Note:
 * At any time, only 4 out of 5 possible configurations are available.
 */
typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_NONE=0,
  /*
   *  SP1 - strict priority one, the highest
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_EF1=1,
  /*
   *  SP2
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_EF2=2,
  /*
   *  SP3
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_EF3=3,
  /*
   *  Valid for single HR only SP4 (WFQ)
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_SINGLE_CLASS_AF1_WFQ=4,
  /*
   *  Valid for dual or enhanced HR onlyFor dual HR: SP4
   *  (WFQ)For enhanced HR: SP9
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_DUAL_OR_ENHANCED=5,
  /*
   *  Must be the last value
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_LAST
}DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /*
   *  A set of available configurations for port low flow control
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS lowest_hp_class[DNX_TMC_SCH_LOW_FC_NOF_AVAIL_CONFS];
}DNX_TMC_SCH_PORT_HP_CLASS_INFO;

typedef enum
{
  /*
   * Scheduler flow group A
   */
    DNX_TMC_SCH_GROUP_A = 0,
  /*
   * Scheduler flow group B
   */
  DNX_TMC_SCH_GROUP_B = 1,
  /*
   * Scheduler flow group C
   */
  DNX_TMC_SCH_GROUP_C = 2,
  /*
   * Scheduler flow group will be determined
   * by the driver, according to flow group assignment policy
   */
  DNX_TMC_SCH_GROUP_AUTO = 3,

  /*
   * Indicates a request not to assign flow group
   */
  DNX_TMC_SCH_GROUP_NONE = 4,

  DNX_TMC_SCH_GROUP_LAST
}DNX_TMC_SCH_GROUP;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Maximal rate in Mega-Bit-Sec, as determined by the port
   *  shaper in the traffic manager. Zero value stands for a
   *  non-active port. If one choose not to do shaping over
   *  the port, the redit_bandwidth value should also be set
   *  as the max_bandwidth value.
   */
  uint32 max_bandwidth;
  /*
   *  The bandwidth that the scheduler should generate to that
   *  portin Mega-Bit-Sec. Zero value stands for a
   *  non-existing port.
   */
  uint32 credit_bandwidth;
  /*
   *  The port priority for the egress strict priority
   *  scheduler
   */
  uint32 priority;

} DNX_TMC_EGRESS_PORT_QOS;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Egress port QoS
   */
  DNX_TMC_EGRESS_PORT_QOS ports[DNX_TMC_NOF_FAP_PORTS];

} DNX_TMC_EGRESS_PORT_QOS_TABLE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /*
   * Range: 0-255
   */
  DNX_TMC_SCH_CL_CLASS_TYPE_ID id;
  /*
   *  Class mode. Range: 1 - 5
   */
  DNX_TMC_SCH_CL_CLASS_MODE mode;
  /*
   *  The discrete weights per flow or per class when they
   *  compete for credits on a WFQ scheduler. TBD add tables of
   *  modes and weights. Range: 1 - 1024
   */
  uint32 weight[DNX_TMC_SCH_MAX_NOF_DISCRETE_WEIGHT_VALS];
  /*
   *  independent, discrete per-flow, discrete per-class
   */
  DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE weight_mode;
  /*
   *  One of three CL enhanced modes: disabled/enabled high
   *  priority/enabled low priority
   */
  DNX_TMC_SCH_CL_ENHANCED_MODE enhanced_mode;
}DNX_TMC_SCH_SE_CL_CLASS_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Number of class types.
   */
  uint32 nof_class_types;
  /*
   *  CL types definition
   */
  DNX_TMC_SCH_SE_CL_CLASS_INFO class_types[DNX_TMC_SCH_NOF_CLASS_TYPES];
}DNX_TMC_SCH_SE_CL_CLASS_TABLE;

/*
 * HR type scheduling element
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_TMC_SCH_SE_HR_MODE mode;
  /* 
   * TCG (Traffic class groups) that the HR is mapped to. 
   * The last four TCG are single-member groups         .
   * Range: 0-DNX_TMC_TCG_MAX.                                                                      .
   * Relevant when Port is 8 priorities                                                                                           .
   * Valid for JER2_ARAD only.                                                                       .
   */
  uint32 tcg_ndx;
} DNX_TMC_SCH_SE_HR;

/*
 * CL type scheduling element
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_TMC_SCH_CL_CLASS_TYPE_ID id;
} DNX_TMC_SCH_SE_CL;

/*
 * FQ type scheduling element
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  uint32 no_info;
} DNX_TMC_SCH_SE_FQ;

typedef union
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  High resolution (HR) scheduling element
   */
  DNX_TMC_SCH_SE_HR hr;
  /*
   *  Class (CL) scheduling element
   */
  DNX_TMC_SCH_SE_CL cl;
  /*
   *  Place holder - no configuration required.
   */
  DNX_TMC_SCH_SE_FQ fq;
}DNX_TMC_SCH_SE_PER_TYPE_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  scheduler element id,range: 0 - 16K-1
   */
  DNX_TMC_SCH_SE_ID id;
  /*
   *  enable/disable
   */
  DNX_TMC_SCH_SE_STATE state;
  /*
   *  HR/CL/FQ
   */
  DNX_TMC_SCH_SE_TYPE type;
  /*
   *  scheduling element per-type specific information
   */
  DNX_TMC_SCH_SE_PER_TYPE_INFO type_info;
  /*
   *  TRUE/FALSE indication. Dual configuration effects two
   *  consecutive SE-s, and is only relevant when both are
   *  enabled.
   */
  uint8 is_dual;
  /*
   *  Scheduler flow group (A-C/Auto/None).
   */
  DNX_TMC_SCH_GROUP group;
}DNX_TMC_SCH_SE_INFO;

/* FLOW / SUBFLOW CONFIGURATION {*/

/*
 * Per-subflow shaper
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Maximum rate in the flow in kbps.
   *  range: The shaper rate
   *  of a flow can be anything from the group's rate (1/3 of
   *  the scheduler rate) to 1/2^20 of the groups rate. For
   *  credit size of 512: group rate = (512 * 8) / (3*4*4ns) =
   *  85.3Gbps --> min shaper rate = 0.081Mbps.
   *  Note: setting '0' or 'SCH_SUB_FLOW_SHAPE_NO_LIMIT'
   *  as a shaper rate results in disabling the shaper
   */
  uint32 max_rate;
  /*
   *  Maximum bytes for a bursty flow. Range: 0 - 512 * credit
   *  size
   */
  uint32 max_burst;
}DNX_TMC_SCH_SUB_FLOW_SHAPER;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  On which of the sp classes is this sub-flow attached to.
   *  Values with _WFQ_ mean that the sub-flow competes on a
   *  WFQ scheduler.
   */
  DNX_TMC_SCH_SUB_FLOW_HR_CLASS sp_class;
  /*
   *  The sub-flow weight when it competes for credits on a
   *  WFQ scheduler. Range: 1 - 4K-1. low weight <-> high
   *  priority
   */
  uint32 weight;
}DNX_TMC_SCH_SUB_FLOW_HR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  To which of the sp classes and wfq weights is this
   *  sub-flow attached.
   */
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS sp_class;
  /*
   *  The sub-flow weight when it competes for credits on a
   *  WFQ schedulerThe weight range is dependent on the CL
   *  scheduler mode:DNX_TMC_SCH_CL_MODE_1: 4 strict-priority
   *  levels: [SP1, SP2, SP3, SP4] - N/ATMC_SCH_CL_MODE_2: 3
   *  strict-priority levels: [SP1, SP2,
   *  SP3-WFQ(2)]DNX_TMC_SCH_CL_MODE_3: 2 strict-priority
   *  levels: [SP1-WFQ(1:63), SP2] or [SP1
   *  WFQ(3),SP2]DNX_TMC_SCH_CL_MODE_4: 2 strict-priority
   *  levels: [SP1,SP2-WFQ(3)] or [SP1, SP2
   *  WFQ(1:63)]DNX_TMC_SCH_CL_MODE_5: 1 strict-priority level:
   *  WFQ (1:253)] or [SP-WFQ(4)]In the above description, the
   *  range is:for WFQ(X): range - 1-Xfor WFQ(1:X): range -
   *  1-X INDEPENDENT_PER_FLOW:low weight <-> high
   *  priorityDISCRETE_PER_FLOW:the weight is an index to a
   *  weight table. In that table,low weight <-> high
   *  priority. DISCRETE_PER_CLASS:low weight <-> low priority
   */
  uint32 weight;
}DNX_TMC_SCH_SUB_FLOW_CL;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /*
   * No configuration exists for fq - this value has no real meaning
   */
  uint32 no_val;
}DNX_TMC_SCH_SUB_FLOW_FQ;

typedef union
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  High resolution (HR) scheduling element
   */
  DNX_TMC_SCH_SUB_FLOW_HR hr;
  /*
   *  Class (CL) scheduling element
   */
  DNX_TMC_SCH_SUB_FLOW_CL cl;
  /*
   *  Place holder - no configuration required.
   */
  DNX_TMC_SCH_SUB_FLOW_FQ fq;
}DNX_TMC_SCH_SUB_FLOW_SE_INFO;

/*
 * Subflow's credit source
 */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  DNX_TMC_SCH_SE_TYPE se_type;
  /*
   *  credit source parameters
   */
  DNX_TMC_SCH_SUB_FLOW_SE_INFO se_info;
  /*
   *  Scheduler element id of the credit source. Range 0 - 16K-1.
   */
  DNX_TMC_SCH_SE_ID id;
}DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   * True for each valid subflow
   */
  uint8 is_valid;
  /*
   *  Subflow id (in flow id range). Range: 0 - 56K-1.
   */
  DNX_TMC_SCH_FLOW_ID id ;
  /*
   *  per-subflow shaper
   */
  DNX_TMC_SCH_SUB_FLOW_SHAPER shaper;
  /*
   *  slow rate index. Range 1 or 2
   */
  DNX_TMC_SCH_SLOW_RATE_NDX slow_rate_ndx;
  /*
   *  credit source id and parameters
   */
  DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE credit_source;
  /*
   *  Control flag, used by caller of 'mbcm_dnx_sch_flow_set'
   *  (specifically
   *    'jer2_jer2_jer2_arad_sch_flow_set->jer2_jer2_jer2_arad_sch_flow_set_unsafe->jer2_jer2_jer2_arad_sch_flow_subflow_set'
   *    'jer2_jer2_jer2_arad_sch_flow_set->jer2_jer2_jer2_arad_sch_flow_set_unsafe->
   *      jer2_jer2_jer2_arad_sch_flow_subflow_set->jer2_jer2_jer2_arad_sch_SUB_FLOW_to_INTERNAL_SUB_FLOW_convert')
   */
  uint8 update_bw_only ;
}DNX_TMC_SCH_SUBFLOW;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Unless in composite/dual mode - only the first subflow
   *  is active
   */
  DNX_TMC_SCH_SUBFLOW sub_flow[DNX_TMC_SCH_NOF_SUB_FLOWS];
  /*
   *  A legal combinations of flow attributes
   *  (Simple/aggregate/composite/dual etc.)
   */
  DNX_TMC_SCH_FLOW_TYPE flow_type;
  /*
   *  Relevant for Queue bounded (simple) flows only. If TRUE,
   *  shapes credit rate according to ShaperSlowRate1 or
   *  ShaperSlowRate2 (per-subflow configuration) when the
   *  flow is in slow state. If FALSE, slow state is treated as
   *  normal state (slow messages are ignored).
   */
  uint8 is_slow_enabled;

}DNX_TMC_SCH_FLOW;

/* FLOW / SUBFLOW CONFIGURATION }*/

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  TRUE/FALSE indication. Effects flow-queue mapping. A
   *  quartet configured with interdigitated = TRUE is
   *  combined of two scheduling elements and two simple
   *  flows. The simple flows are mapped to queues.
   */
  uint8 is_interdigitated;
  /*
   *  TRUE/FALSE indication. Effects composite flow
   *  configuration. In a quartet configured with odd_even =
   *  TRUE (AKA 0-1 configuration), the even flow is combined
   *  of even and the adjacent odd subflows. In a quartet
   *  configured with odd_even = FALSE (AKA 0-2
   *  configuration), the even flow x is combined of subflow x
   *  and subflow x+2.
   */
  uint8 is_odd_even;
  /*
   *  In dual shaper configuration - defines whether CL
   *  functions as CIR (and FQ/HR as EIR), or as EIR (and
   *  FQ/HR as CIR). The configuration is per 256 CL-FQ/HR
   *  pairs (covers 1K flow id-s)
   */
  uint8 is_cl_cir;
}DNX_TMC_SCH_GLOBAL_PER1K_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The index of the queue quartet to which the flow quartet
   *  is mapped. The queue-to-flow mapping of a single queue
   *  quartet effects one, two or four flow quartets,
   *  depending on the 'Interdigitated' and 'Composite' modes.
   *  Range: 0 - 8K-1
   */
  uint32 base_q_qrtt_id;
  /*
   *  TRUE/FALSE indication. If true, two adjacent subflows
   *  will be mapped to the same queue.this configuration must
   *  be coherent with FSF (flow-subflow) configuration,
   *  defining whether two subflows compose a single
   *  flow.per-quartet
   */
  uint8 is_composite;
  /*
   *  TRUE/FALSE indication. If true, the adjacent quartet
   *  is valid. Otherwise it is not. Note that entries are
   *  grouped in blocks of 8 queues. Examples: If 'base_q_qrtt_id'
   *  is 32 then this is the 8th quartet and its adjacent quartet
   *  is the 9th one ('qrtt_id' = 36). If 'base_q_qrtt_id' is 44
   *  then adjacent quartet is the one starting with 'qrtt_id' = 40.
   */
  uint8 other_quartet_is_valid ;
  /*
   *  source fap id. Range: 0 - 2047
   */
  uint32 fip_id;
}DNX_TMC_SCH_QUARTET_MAPPING_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Maximal rate for a flow with slow enabled, when in slow
   *  state.
   *  Units: Kbps.
   */
  uint32 rates[DNX_TMC_SCH_NOF_SLOW_RATES];
}DNX_TMC_SCH_SLOW_RATE;

typedef struct
{

  DNX_SAND_MAGIC_NUM_VAR

  /*
   *  enable == TRUE means port open/update.
   *  enable == FALSE means port close.
   */
   uint8 enable;

  /*
   *  Maximal expected port rate.
   *  Units: Mega-Bit-Sec.
   *  Note!:
   *  Typically, may be set to DNX_TMC_SCH_PORT_MAX_EXPECTED_RATE_AUTO
   *  In this case, the description below is not relevant.
   *
   *  When opening a scheduler port, this value is used to define port group.
   *  It should reflect the maximal rate the port is expected to have.
   *  Alternatively, DNX_TMC_SCH_PORT_MAX_EXPECTED_RATE_AUTO value may be
   *  set here.
   *  This will case even group distribution between the scheduler
   *  ports (A-B-C-A...).
   *  Example: when opening a single port, mapped to 10Gbps interface,
   *  max_expected_rate for this port should be set to 10Gbps.
   *  This does not effect the actual Port rate.
   */
  uint32  max_expected_rate;
  /*
   *  The mode of the HR scheduling element used by the scheduler port
   *  Invalid for JER2_ARAD.
   */
  DNX_TMC_SCH_SE_HR_MODE hr_mode;
  /*
   *  The mode of the HR scheduling element used by the scheduler port-priority tc
   *  Number of HRs depends on number of priorities in the given port.
   *  Valid for JER2_ARAD only.
   */
  DNX_TMC_SCH_SE_HR_MODE hr_modes[DNX_TMC_NOF_TRAFFIC_CLASSES];
  /* 
   *  The TCG (Traffic class groups) that each HR (Port-PriorityTC) is mapped to.
   *  Note: The last four TCG are single-member groups.                                                                          .
   *  Valid for JER2_ARAD only.
   */
  DNX_TMC_TCG_NDX tcg_ndx[DNX_TMC_NOF_TRAFFIC_CLASSES];
  /*
   *  The classes with priority specified by lowest_hp_class and above
   *  will not be effected by low priority flow control.
   *  Those classes will only be effected by
   *  high priority flow control.
   *  Invalid for JER2_ARAD.
   */
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS lowest_hp_class;

  /*
   *  Scheduler flow group (A-C/Auto/None).
   */
  DNX_TMC_SCH_GROUP group;
} DNX_TMC_SCH_PORT_INFO;

/* JER2_ARAD only defines { */
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The lower the weight the higher the bandwidth. 
   *  When both have equal weights it implies simple RR.
   *  Range: 1-1023
   */
  uint32 tcg_weight;

  /*
   *  If set, tcg weight is valid and taking part of the
   *  WFQ policy.
   */
  uint8 tcg_weight_valid;

} DNX_TMC_SCH_TCG_WEIGHT;

/*
 * Flow and up info get data structures
 */
typedef struct {
    DNX_TMC_SCH_PORT_INFO      port_info;
    DNX_TMC_OFP_RATE_INFO      ofp_rate_info;
    uint32                     credit_rate;
    uint32                     fc_cnt;
    uint32                     fc_percent;
} DNX_TMC_SCH_FLOW_AND_UP_PORT_INFO;

typedef struct {
    DNX_TMC_SCH_SE_INFO        se_info;
    DNX_TMC_SCH_FLOW           sch_consumer;
    uint32                     credit_rate;
    uint32                     credit_rate_overflow;
    DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE cl_mode;
    DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE ipf_mode;
} DNX_TMC_SCH_FLOW_AND_UP_SE_INFO;

typedef union  {
    DNX_TMC_SCH_FLOW_AND_UP_PORT_INFO        port_sch_info;
    DNX_TMC_SCH_FLOW_AND_UP_SE_INFO          se_sch_info;
} DNX_TMC_SCH_FLOW_AND_UP_UNION_INFO;

typedef struct  {
    uint32                          base_queue;
    DNX_TMC_IPQ_QUARTET_MAP_INFO    qrtt_map_info;
    DNX_TMC_SCH_FLOW                sch_consumer;
    uint32                          credit_rate;
    DNX_TMC_OFP_RATES_TBL_INFO      ofp_rates_table;
    uint32                          ofp_rate_valid;

    /*In*/
    /*Current level_credit_sources*/
    uint32                          credit_sources[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];
    uint32                          credit_sources_nof;

    /*Out*/
    /*Current level sch info*/
    uint32                              is_port_sch[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];
    DNX_TMC_SCH_FLOW_AND_UP_UNION_INFO  sch_union_info[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];
    DNX_TMC_SCH_PORT_ID                 sch_port_id[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];
    uint32                              sch_priority_ndx[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];

    /*Out*/
    /*Next level credit source - next_level_credit_sources_nof==0 ==> no next level*/
    uint32                          next_level_credit_sources[DNX_TMC_FLOW_AND_UP_MAX_CREDIT_SOURCES];
    uint32                          next_level_credit_sources_nof;
} DNX_TMC_SCH_FLOW_AND_UP_INFO;

/* JER2_ARAD only defines } */
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
  DNX_TMC_SCH_DEVICE_RATE_ENTRY_clear(
    DNX_SAND_OUT DNX_TMC_SCH_DEVICE_RATE_ENTRY *info
  );

void
  DNX_TMC_SCH_DEVICE_RATE_TABLE_clear(
    DNX_SAND_OUT DNX_TMC_SCH_DEVICE_RATE_TABLE *info
  );

void
  DNX_TMC_SCH_IF_WEIGHT_ENTRY_clear(
    DNX_SAND_OUT DNX_TMC_SCH_IF_WEIGHT_ENTRY *info
  );

void
  DNX_TMC_SCH_IF_WEIGHTS_clear(
    DNX_SAND_OUT DNX_TMC_SCH_IF_WEIGHTS *info
  );

void
  DNX_TMC_SCH_PORT_HP_CLASS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_PORT_HP_CLASS_INFO *info
  );

void
  DNX_TMC_SCH_PORT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_PORT_INFO *info
  );

void
  DNX_TMC_SCH_TCG_WEIGHT_clear(
    DNX_SAND_OUT DNX_TMC_SCH_TCG_WEIGHT *tcg_weight
  );

void
  DNX_TMC_SCH_SE_HR_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_HR *info
  );

void
  DNX_TMC_SCH_SE_CL_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_CL *info
  );

void
  DNX_TMC_SCH_SE_FQ_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_FQ *info
  );

void
  DNX_TMC_SCH_SE_CL_CLASS_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_CL_CLASS_INFO *info
  );

void
  DNX_TMC_SCH_SE_CL_CLASS_TABLE_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_CL_CLASS_TABLE *info
  );

void
  DNX_TMC_SCH_SE_PER_TYPE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_PER_TYPE_INFO *info
  );

void
  DNX_TMC_SCH_SE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SE_INFO *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_SHAPER_clear(
    int unit,
    DNX_SAND_OUT DNX_TMC_SCH_SUB_FLOW_SHAPER *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_HR_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SUB_FLOW_HR *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_CL_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SUB_FLOW_CL *info
  );


void
  DNX_TMC_SCH_SUB_FLOW_SE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SUB_FLOW_SE_INFO *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE *info
  );

void
  DNX_TMC_SCH_SUBFLOW_clear(
    int unit,
    DNX_SAND_OUT DNX_TMC_SCH_SUBFLOW *info
  );

void
  DNX_TMC_SCH_FLOW_clear(
    int unit,
    DNX_SAND_OUT DNX_TMC_SCH_FLOW *info
  );

void
  DNX_TMC_SCH_GLOBAL_PER1K_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_GLOBAL_PER1K_INFO *info
  );

void
  DNX_TMC_SCH_QUARTET_MAPPING_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SCH_QUARTET_MAPPING_INFO *info
  );

void
  DNX_TMC_SCH_SLOW_RATE_clear(
    DNX_SAND_OUT DNX_TMC_SCH_SLOW_RATE *info
  );

void
  DNX_TMC_SCH_FLOW_AND_UP_PORT_INFO_clear(
     DNX_SAND_OUT DNX_TMC_SCH_FLOW_AND_UP_PORT_INFO *info
  );

void
  DNX_TMC_SCH_FLOW_AND_UP_SE_INFO_clear(
     int unit,
     DNX_SAND_OUT DNX_TMC_SCH_FLOW_AND_UP_SE_INFO *info
  );

void
  DNX_TMC_SCH_FLOW_AND_UP_INFO_clear(
     int unit,
     DNX_SAND_OUT DNX_TMC_SCH_FLOW_AND_UP_INFO *info,
     DNX_SAND_IN uint32                         is_full /*is_full == false --> clear the relevant fields for the next stage algorithm*/
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS_to_string(
    DNX_SAND_IN DNX_TMC_SCH_PORT_LOWEST_HP_HR_CLASS enum_val
  );

const char*
  DNX_TMC_SCH_CL_CLASS_MODE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_CL_CLASS_MODE enum_val
  );

const char*
  DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_CL_CLASS_WEIGHTS_MODE enum_val
  );

const char*
  DNX_TMC_SCH_CL_ENHANCED_MODE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_CL_ENHANCED_MODE enum_val
  );

const char*
  DNX_TMC_SCH_GROUP_to_string(
    DNX_SAND_IN DNX_TMC_SCH_GROUP enum_val
  );

const char*
  DNX_TMC_SCH_SE_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SE_TYPE enum_val
  );

const char*
  DNX_TMC_SCH_SE_STATE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SE_STATE enum_val
  );

const char*
  DNX_TMC_SCH_SE_HR_MODE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SE_HR_MODE enum_val
  );

const char*
  DNX_TMC_SCH_SUB_FLOW_HR_CLASS_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_HR_CLASS enum_val
  );

const char*
  DNX_TMC_SCH_SUB_FLOW_CL_CLASS_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_CL_CLASS enum_val
  );

const char*
  DNX_TMC_SCH_SLOW_RATE_NDX_to_string(
    DNX_SAND_IN DNX_TMC_SCH_SLOW_RATE_NDX enum_val
  );

const char*
  DNX_TMC_SCH_FLOW_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_SCH_FLOW_TYPE enum_val
  );

const char*
  DNX_TMC_SCH_FLOW_STATUS_to_string(
    DNX_SAND_IN DNX_TMC_SCH_FLOW_STATUS enum_val
  );

const char*
  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE enum_val
  );

void
  DNX_TMC_SCH_DEVICE_RATE_ENTRY_print(
    DNX_SAND_IN DNX_TMC_SCH_DEVICE_RATE_ENTRY *info
  );

void
  DNX_TMC_SCH_DEVICE_RATE_TABLE_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_SCH_DEVICE_RATE_TABLE *info
  );

void
  DNX_TMC_SCH_IF_WEIGHT_ENTRY_print(
    DNX_SAND_IN DNX_TMC_SCH_IF_WEIGHT_ENTRY *info
  );

void
  DNX_TMC_SCH_IF_WEIGHTS_print(
    DNX_SAND_IN DNX_TMC_SCH_IF_WEIGHTS *info
  );

void
  DNX_TMC_SCH_PORT_HP_CLASS_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_PORT_HP_CLASS_INFO *info
  );

void
  DNX_TMC_SCH_PORT_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_PORT_INFO *info,
    DNX_SAND_IN uint32           port_id
  );

void
  DNX_TMC_SCH_TCG_WEIGHT_print(
    DNX_SAND_IN DNX_TMC_SCH_TCG_WEIGHT *tcg_weight
  );
void
  DNX_TMC_SCH_SE_HR_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_HR *info
  );

void
  DNX_TMC_SCH_SE_CL_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_CL *info
  );

void
  DNX_TMC_SCH_SE_FQ_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_FQ *info
  );

void
  DNX_TMC_SCH_SE_CL_CLASS_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_CL_CLASS_INFO *info
  );

void
  DNX_TMC_SCH_SE_CL_CLASS_TABLE_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_CL_CLASS_TABLE *info
  );

void
  DNX_TMC_SCH_SE_PER_TYPE_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_PER_TYPE_INFO *info,
    DNX_SAND_IN DNX_TMC_SCH_SE_TYPE type
  );

void
  DNX_TMC_SCH_SE_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_SE_INFO *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_SHAPER_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_SHAPER *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_HR_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_HR *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_CL_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_CL *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_FQ_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_FQ *info
  );

void
  DNX_TMC_SCH_SUB_FLOW_SE_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_SE_INFO *info,
    DNX_SAND_IN DNX_TMC_SCH_SE_TYPE se_type
  );

void
  DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE_print(
    DNX_SAND_IN DNX_TMC_SCH_SUB_FLOW_CREDIT_SOURCE *info
  );

void
  DNX_TMC_SCH_SUBFLOW_print(
    DNX_SAND_IN DNX_TMC_SCH_SUBFLOW *info,
    DNX_SAND_IN uint8 is_table_flow,
    DNX_SAND_IN uint32 subflow_id
  );

void
  DNX_TMC_SCH_FLOW_print(
    DNX_SAND_IN DNX_TMC_SCH_FLOW *info,
    DNX_SAND_IN uint8 is_table
  );

void
  DNX_TMC_SCH_GLOBAL_PER1K_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_GLOBAL_PER1K_INFO *info
  );

void
  DNX_TMC_SCH_QUARTET_MAPPING_INFO_print(
    DNX_SAND_IN DNX_TMC_SCH_QUARTET_MAPPING_INFO *info
  );

void
  DNX_TMC_SCH_SLOW_RATE_print(
    DNX_SAND_IN DNX_TMC_SCH_SLOW_RATE *info
  );
/* STRUCTURE MGMT } */

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_END2END_SCHEDULER_INCLUDED__*/
#endif
