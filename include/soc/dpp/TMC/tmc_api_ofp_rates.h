/* $Id: tmc_api_ofp_rates.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_OFP_RATES_INCLUDED__
/* { */
#define __SOC_TMC_API_OFP_RATES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_general.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* $Id: tmc_api_ofp_rates.h,v 1.9 Broadcom SDK $
 *  Setting  this value as maximal burst will result in no burst limitation
 */
#define SOC_TMC_OFP_RATES_BURST_LIMIT_MAX               (0xFFFF)
#define SOC_TMC_OFP_RATES_BURST_DEFAULT                 (0x4000)
#define SOC_TMC_OFP_RATES_BURST_EMPTY_Q_LIMIT_MAX       (0x3FFF)
#define SOC_TMC_OFP_RATES_BURST_FC_Q_LIMIT_MAX          (0x3FFF)
#define SOC_TMC_OFP_RATES_SCH_BURST_LIMIT_MAX           (0x7FFF)
#define SOC_TMC_OFP_RATES_ILLEGAL_PORT_ID     (SOC_TMC_NOF_FAP_PORTS)

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
   *  Calendar Set A - scheduler and EGQ.
   */
  SOC_TMC_OFP_RATES_CAL_SET_A=0,
  /*
   *  Calendar Set B - scheduler and EGQ.
   */
  SOC_TMC_OFP_RATES_CAL_SET_B=1,
  /*
   *  Total number of calendar sets.
   */
  SOC_TMC_OFP_NOF_RATES_CAL_SETS=2
}SOC_TMC_OFP_RATES_CAL_SET;

typedef enum
{
  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE = 0,

  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST = 1,

  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN = 2,

  SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_NOF_FIELD_TYPE = 3
} SOC_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE;

typedef enum
{
  /* Represents Channelize arbiter calendar */
  SOC_TMC_OFP_RATES_EGQ_CAL_CHAN_ARB = 0,

  /* Represents TCG calendar */
  SOC_TMC_OFP_RATES_EGQ_CAL_TCG = 1,

  /* Represents Port Priority calendar */
  SOC_TMC_OFP_RATES_EGQ_CAL_PORT_PRIORITY = 2

} SOC_TMC_OFP_RATES_EGQ_CAL_TYPE;

typedef struct
{
  /*
   *  Calendar type.   
   */
  SOC_TMC_OFP_RATES_EGQ_CAL_TYPE cal_type;
  /*
   *  More information:
   *  currently only channelize arbiter id in case of Channelize calendar
   */
  uint32 chan_arb_id;
} SOC_TMC_OFP_RATES_CAL_INFO;

typedef enum
{
  /*
   *  Set the shaper value according to the accumulated value
   *  of the interface ports.
   */
  SOC_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS=0,
  /*
   *  Set the shaper value as specified.
   */
  SOC_TMC_OFP_SHPR_UPDATE_MODE_OVERRIDE=1,
  /*
   *  Do not change shaper value
   */
  SOC_TMC_OFP_SHPR_UPDATE_MODE_DONT_TUCH=2,
  /*
   *  Total number of shaper update modes.
   */
  SOC_TMC_OFP_NOF_SHPR_UPDATE_MODES=3
}SOC_TMC_OFP_SHPR_UPDATE_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Update mode: update according to accumulated port
   *  rate/burst value/override/don't touch
   */
  SOC_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode;
  /*
   *  Maximal MAL shaper rate in Kbps. Relevant only if the
   *  mode is 'OVERRIDDE'.
   */
  uint32 rate;
}SOC_TMC_OFP_RATES_MAL_SHPR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Update mode: update according to accumulated port
   *  rate/burst value/override/don't touch
   */
  SOC_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode;
  /*
   *  Maximal Interface shaper rate in Kbps. Relevant only if the
   *  mode is 'OVERRIDDE'.
   */
  uint32 rate;
}SOC_TMC_OFP_RATES_INTERFACE_SHPR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration - per-MAL rate
   */
  SOC_TMC_OFP_RATES_MAL_SHPR sch_shaper;
  /*
   *  Egress shaper configuration - per-MAL rate and burst
   */
  SOC_TMC_OFP_RATES_MAL_SHPR egq_shaper;
}SOC_TMC_OFP_RATES_MAL_SHPR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration - per-Interface rate
   */
  SOC_TMC_OFP_RATES_INTERFACE_SHPR sch_shaper;
  /*
   *  Egress shaper configuration - per-Interface rate and burst
   */
  SOC_TMC_OFP_RATES_INTERFACE_SHPR egq_shaper;
}SOC_TMC_OFP_RATES_INTERFACE_SHPR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  uint32 flags;
  /*
   *  Port index. Range: 0-79
   */
  uint32 port_id;
  /*
   *  End-to-end scheduler credit rate, in Kbps. Typically - a
   *  nominal port rate + scheduler speedup.
   */
  uint32 sch_rate;
  /*
   *  Egress shaping rate, in Kbps. Typically - a nominal port
   *  rate.
   */
  uint32 egq_rate;
  /*
   *  Egress maximum burst.
   *  Maximum credit balance in Bytes, that the port can
   *  accumulate, indicating the burst size of the OFP. Range:
   *  0 - 0xFFFF.
   */
  uint32 max_burst;
  /*
   *  End-to-end maximum burst.
   *  Maximum credit balance in Bytes, that the port can
   *  accumulate, indicating the burst size of the OFP. Range:
   *  0 - 0xFFFF.
   */
  uint32 sch_max_burst;
  /* 
   *  Priority index. Range: 0-7.
   *  Valid for ARAD only.
   *  Relavant for Port-Priority configuration only. 
   */
  uint32 port_priority;
  /* 
   *  TCG index. Range: 0-7.
   *  Valid for ARAD only.
   *  Relavant for TCG configuration only.
   */
  SOC_TMC_TCG_NDX tcg_ndx;
}SOC_TMC_OFP_RATE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of valid entries in SOC_TMC_EGR_PORT_RATES
   *  table. Range: 1 - SOC_TMC_NOF_FAP_PORTS_MAX.
   */
  uint32 nof_valid_entries;
  /*
   *  Shaper rates and credit rates for all OFP-s, per
   *  interface.
   */
  SOC_TMC_OFP_RATE_INFO rates[SOC_TMC_NOF_FAP_PORTS_MAX];
}SOC_TMC_OFP_RATES_TBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end scheduler credit rate, in Kbps. Typically - a
   *  nominal fat pipe rate + scheduler speedup.
   */
  uint32 sch_rate;
  /*
   *  Egress shaping rate, in Kbps. Typically - a nominal fat
   *  pipe rate.
   */
  uint32 egq_rate;
  /*
   *  Maximum credit balance in Bytes, that the fat pipe can
   *  accumulate, indicating the burst size of the fat pipe.
   *  Range: 0 - 0xFFFF.
   */
  uint32 max_burst;
}SOC_TMC_OFP_FAT_PIPE_RATE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  shaping rate, in Kbps. For the Egress Trnasmit: Typically - a nominal 
   *  rate. For the End-to-end scheduler it is the credit rate. Typically
   *  - a nominal rate + scheduelr speedup.
   */
  uint32 rate;
  /*
   *  Maximum credit balance in Bytes, indicating the burst size. Range:
   *  0 - 0xFFFF.
   */
  uint32 max_burst;
}SOC_TMC_OFP_RATE_SHPR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration -  rate and burst
   */
  SOC_TMC_OFP_RATE_SHPR_INFO sch_shaper;
  /*
   *  Egress shaper configuration - rate and burst
   */
  SOC_TMC_OFP_RATE_SHPR_INFO egq_shaper;
}SOC_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration -  rate and burst
   */
  SOC_TMC_OFP_RATE_SHPR_INFO sch_shaper;
  /*
   *  Egress shaper configuration - rate and burst
   */
  SOC_TMC_OFP_RATE_SHPR_INFO egq_shaper;
}SOC_TMC_OFP_RATES_TCG_SHPR_INFO;

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
  SOC_TMC_OFP_RATES_MAL_SHPR_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_MAL_SHPR *info
  );

void
  SOC_TMC_OFP_RATES_MAL_SHPR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_MAL_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATES_INTERFACE_SHPR_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_INTERFACE_SHPR *info
  );

void
  SOC_TMC_OFP_RATES_INTERFACE_SHPR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info 
  );

void
  SOC_TMC_OFP_RATE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATE_INFO *info
  );

void
  SOC_TMC_OFP_RATES_TBL_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_TBL_INFO *info
  );

void
  SOC_TMC_OFP_FAT_PIPE_RATE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_FAT_PIPE_RATE_INFO *info
  );

void
  SOC_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATE_SHPR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATE_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATES_TCG_SHPR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_TCG_SHPR_INFO *info
  );


#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_OFP_RATES_CAL_SET_to_string(
    SOC_SAND_IN SOC_TMC_OFP_RATES_CAL_SET enum_val
  );

const char*
  SOC_TMC_OFP_SHPR_UPDATE_MODE_to_string(
    SOC_SAND_IN SOC_TMC_OFP_SHPR_UPDATE_MODE enum_val
  );

void
  SOC_TMC_OFP_RATES_MAL_SHPR_print(
    SOC_SAND_IN SOC_TMC_OFP_RATES_MAL_SHPR *info
  );

void
  SOC_TMC_OFP_RATES_MAL_SHPR_INFO_print(
    SOC_SAND_IN SOC_TMC_OFP_RATES_MAL_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATES_INTERFACE_SHPR_print(
    SOC_SAND_IN SOC_TMC_OFP_RATES_INTERFACE_SHPR *info
  );

void
  SOC_TMC_OFP_RATES_INTERFACE_SHPR_INFO_print(
    SOC_SAND_IN SOC_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATE_INFO_print(
    SOC_SAND_IN SOC_TMC_OFP_RATE_INFO *info
  );

void
  SOC_TMC_OFP_RATES_TBL_INFO_print(
    SOC_SAND_IN SOC_TMC_OFP_RATES_TBL_INFO *info
  );

void
  SOC_TMC_OFP_FAT_PIPE_RATE_INFO_print(
    SOC_SAND_IN SOC_TMC_OFP_FAT_PIPE_RATE_INFO *info
  );

void
  SOC_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_print(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATE_SHPR_INFO_print(
    SOC_SAND_OUT SOC_TMC_OFP_RATE_SHPR_INFO *info
  );

void
  SOC_TMC_OFP_RATES_TCG_SHPR_INFO_print(
    SOC_SAND_OUT SOC_TMC_OFP_RATES_TCG_SHPR_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_OFP_RATES_INCLUDED__*/
#endif
