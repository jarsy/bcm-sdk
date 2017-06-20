/* $Id: jer2_jer2_jer2_tmc_api_ofp_rates.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_OFP_RATES_INCLUDED__
/* { */
#define __DNX_TMC_API_OFP_RATES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* $Id: jer2_jer2_jer2_tmc_api_ofp_rates.h,v 1.9 Broadcom SDK $
 *  Setting  this value as maximal burst will result in no burst limitation
 */
#define DNX_TMC_OFP_RATES_BURST_LIMIT_MAX               (0xFFFF)
#define DNX_TMC_OFP_RATES_BURST_DEFAULT                 (0x4000)
#define DNX_TMC_OFP_RATES_BURST_EMPTY_Q_LIMIT_MAX       (0x3FFF)
#define DNX_TMC_OFP_RATES_BURST_FC_Q_LIMIT_MAX          (0x3FFF)
#define DNX_TMC_OFP_RATES_SCH_BURST_LIMIT_MAX           (0x7FFF)
#define DNX_TMC_OFP_RATES_ILLEGAL_PORT_ID     (DNX_TMC_NOF_FAP_PORTS)

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
  DNX_TMC_OFP_RATES_CAL_SET_A=0,
  /*
   *  Calendar Set B - scheduler and EGQ.
   */
  DNX_TMC_OFP_RATES_CAL_SET_B=1,
  /*
   *  Total number of calendar sets.
   */
  DNX_TMC_OFP_NOF_RATES_CAL_SETS=2
}DNX_TMC_OFP_RATES_CAL_SET;

typedef enum
{
  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE = 0,

  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST = 1,

  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN = 2,

  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_NOF_FIELD_TYPE = 3
} DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE;

typedef enum
{
  /* Represents Channelize arbiter calendar */
  DNX_TMC_OFP_RATES_EGQ_CAL_CHAN_ARB = 0,

  /* Represents TCG calendar */
  DNX_TMC_OFP_RATES_EGQ_CAL_TCG = 1,

  /* Represents Port Priority calendar */
  DNX_TMC_OFP_RATES_EGQ_CAL_PORT_PRIORITY = 2

} DNX_TMC_OFP_RATES_EGQ_CAL_TYPE;

typedef struct
{
  /*
   *  Calendar type.   
   */
  DNX_TMC_OFP_RATES_EGQ_CAL_TYPE cal_type;
  /*
   *  More information:
   *  currently only channelize arbiter id in case of Channelize calendar
   */
  uint32 chan_arb_id;
} DNX_TMC_OFP_RATES_CAL_INFO;

typedef enum
{
  /*
   *  Set the shaper value according to the accumulated value
   *  of the interface ports.
   */
  DNX_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS=0,
  /*
   *  Set the shaper value as specified.
   */
  DNX_TMC_OFP_SHPR_UPDATE_MODE_OVERRIDE=1,
  /*
   *  Do not change shaper value
   */
  DNX_TMC_OFP_SHPR_UPDATE_MODE_DONT_TUCH=2,
  /*
   *  Total number of shaper update modes.
   */
  DNX_TMC_OFP_NOF_SHPR_UPDATE_MODES=3
}DNX_TMC_OFP_SHPR_UPDATE_MODE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Update mode: update according to accumulated port
   *  rate/burst value/override/don't touch
   */
  DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode;
  /*
   *  Maximal MAL shaper rate in Kbps. Relevant only if the
   *  mode is 'OVERRIDDE'.
   */
  uint32 rate;
}DNX_TMC_OFP_RATES_MAL_SHPR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Update mode: update according to accumulated port
   *  rate/burst value/override/don't touch
   */
  DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode;
  /*
   *  Maximal Interface shaper rate in Kbps. Relevant only if the
   *  mode is 'OVERRIDDE'.
   */
  uint32 rate;
}DNX_TMC_OFP_RATES_INTERFACE_SHPR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration - per-MAL rate
   */
  DNX_TMC_OFP_RATES_MAL_SHPR sch_shaper;
  /*
   *  Egress shaper configuration - per-MAL rate and burst
   */
  DNX_TMC_OFP_RATES_MAL_SHPR egq_shaper;
}DNX_TMC_OFP_RATES_MAL_SHPR_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration - per-Interface rate
   */
  DNX_TMC_OFP_RATES_INTERFACE_SHPR sch_shaper;
  /*
   *  Egress shaper configuration - per-Interface rate and burst
   */
  DNX_TMC_OFP_RATES_INTERFACE_SHPR egq_shaper;
}DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

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
   *  Valid for JER2_ARAD only.
   *  Relavant for Port-Priority configuration only. 
   */
  uint32 port_priority;
  /* 
   *  TCG index. Range: 0-7.
   *  Valid for JER2_ARAD only.
   *  Relavant for TCG configuration only.
   */
  DNX_TMC_TCG_NDX tcg_ndx;
}DNX_TMC_OFP_RATE_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Number of valid entries in DNX_TMC_EGR_PORT_RATES
   *  table. Range: 1 - DNX_TMC_NOF_FAP_PORTS_MAX.
   */
  uint32 nof_valid_entries;
  /*
   *  Shaper rates and credit rates for all OFP-s, per
   *  interface.
   */
  DNX_TMC_OFP_RATE_INFO rates[DNX_TMC_NOF_FAP_PORTS_MAX];
}DNX_TMC_OFP_RATES_TBL_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
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
}DNX_TMC_OFP_FAT_PIPE_RATE_INFO;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
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
}DNX_TMC_OFP_RATE_SHPR_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration -  rate and burst
   */
  DNX_TMC_OFP_RATE_SHPR_INFO sch_shaper;
  /*
   *  Egress shaper configuration - rate and burst
   */
  DNX_TMC_OFP_RATE_SHPR_INFO egq_shaper;
}DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  End-to-end Scheduler shaper configuration -  rate and burst
   */
  DNX_TMC_OFP_RATE_SHPR_INFO sch_shaper;
  /*
   *  Egress shaper configuration - rate and burst
   */
  DNX_TMC_OFP_RATE_SHPR_INFO egq_shaper;
}DNX_TMC_OFP_RATES_TCG_SHPR_INFO;

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
  DNX_TMC_OFP_RATES_MAL_SHPR_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_MAL_SHPR *info
  );

void
  DNX_TMC_OFP_RATES_MAL_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_MAL_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_INTERFACE_SHPR *info
  );

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info 
  );

void
  DNX_TMC_OFP_RATE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_INFO *info
  );

void
  DNX_TMC_OFP_RATES_TBL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TBL_INFO *info
  );

void
  DNX_TMC_OFP_FAT_PIPE_RATE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_FAT_PIPE_RATE_INFO *info
  );

void
  DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATE_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATES_TCG_SHPR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TCG_SHPR_INFO *info
  );


#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_OFP_RATES_CAL_SET_to_string(
    DNX_SAND_IN DNX_TMC_OFP_RATES_CAL_SET enum_val
  );

const char*
  DNX_TMC_OFP_SHPR_UPDATE_MODE_to_string(
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE enum_val
  );

void
  DNX_TMC_OFP_RATES_MAL_SHPR_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_MAL_SHPR *info
  );

void
  DNX_TMC_OFP_RATES_MAL_SHPR_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_MAL_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_INTERFACE_SHPR *info
  );

void
  DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATE_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATE_INFO *info
  );

void
  DNX_TMC_OFP_RATES_TBL_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_RATES_TBL_INFO *info
  );

void
  DNX_TMC_OFP_FAT_PIPE_RATE_INFO_print(
    DNX_SAND_IN DNX_TMC_OFP_FAT_PIPE_RATE_INFO *info
  );

void
  DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_PORT_PRIORITY_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATE_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATE_SHPR_INFO *info
  );

void
  DNX_TMC_OFP_RATES_TCG_SHPR_INFO_print(
    DNX_SAND_OUT DNX_TMC_OFP_RATES_TCG_SHPR_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_OFP_RATES_INCLUDED__*/
#endif
