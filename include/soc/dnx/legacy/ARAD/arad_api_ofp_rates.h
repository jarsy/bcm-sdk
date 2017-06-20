/* $Id: jer2_arad_api_ofp_rates.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_API_OFP_RATES_INCLUDED__
/* { */
#define __JER2_ARAD_API_OFP_RATES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ofp_rates.h>
#include <soc/dnx/legacy/port_sw_db.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/*
 *  Setting  this value as maximal burst will result in no burst limitation
 */
#define JER2_ARAD_OFP_RATES_SCH_BURST_LIMIT_MAX          (DNX_TMC_OFP_RATES_SCH_BURST_LIMIT_MAX)
#define JER2_ARAD_OFP_RATES_BURST_LIMIT_MAX              (DNX_TMC_OFP_RATES_BURST_LIMIT_MAX)
#define JER2_ARAD_OFP_RATES_BURST_DEAULT                 (DNX_TMC_OFP_RATES_BURST_DEFAULT)
#define JER2_ARAD_OFP_RATES_EMPTY_Q_BURST_LIMIT_MAX      (DNX_TMC_OFP_RATES_BURST_EMPTY_Q_LIMIT_MAX)
#define JER2_ARAD_OFP_RATES_FC_Q_BURST_LIMIT_MAX         (DNX_TMC_OFP_RATES_BURST_FC_Q_LIMIT_MAX)     
#define JER2_ARAD_OFP_RATES_CHNIF_BURST_LIMIT_MAX        (0xFFC0)

#define JER2_ARAD_OFP_RATES_ILLEGAL_PORT_ID              (JER2_ARAD_NOF_FAP_PORTS)

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

#define JER2_ARAD_OFP_RATES_CAL_SET_A                          DNX_TMC_OFP_RATES_CAL_SET_A
#define JER2_ARAD_OFP_RATES_CAL_SET_B                          DNX_TMC_OFP_RATES_CAL_SET_B
#define JER2_ARAD_OFP_NOF_RATES_CAL_SETS                       DNX_TMC_OFP_NOF_RATES_CAL_SETS
typedef DNX_TMC_OFP_RATES_CAL_SET                         JER2_ARAD_OFP_RATES_CAL_SET;
typedef DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE         JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE;

#define JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB                   DNX_TMC_OFP_RATES_EGQ_CAL_CHAN_ARB
#define JER2_ARAD_OFP_RATES_EGQ_CAL_TCG                        DNX_TMC_OFP_RATES_EGQ_CAL_TCG
#define JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY              DNX_TMC_OFP_RATES_EGQ_CAL_PORT_PRIORITY
typedef DNX_TMC_OFP_RATES_EGQ_CAL_TYPE                    JER2_ARAD_OFP_RATES_EGQ_CAL_TYPE;
typedef DNX_TMC_OFP_RATES_CAL_INFO                        JER2_ARAD_OFP_RATES_CAL_INFO;

#define JER2_ARAD_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS            DNX_TMC_OFP_SHPR_UPDATE_MODE_SUM_OF_PORTS
#define JER2_ARAD_OFP_SHPR_UPDATE_MODE_OVERRIDE                DNX_TMC_OFP_SHPR_UPDATE_MODE_OVERRIDE
#define JER2_ARAD_OFP_SHPR_UPDATE_MODE_DONT_TUCH               DNX_TMC_OFP_SHPR_UPDATE_MODE_DONT_TUCH
#define JER2_ARAD_OFP_NOF_SHPR_UPDATE_MODES                    DNX_TMC_OFP_NOF_SHPR_UPDATE_MODES
typedef DNX_TMC_OFP_SHPR_UPDATE_MODE                           JER2_ARAD_OFP_SHPR_UPDATE_MODE;

typedef DNX_TMC_OFP_RATES_INTERFACE_SHPR_INFO                  JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO;
typedef DNX_TMC_OFP_RATE_INFO                                  JER2_ARAD_OFP_RATE_INFO;

typedef enum
{
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_00   =  0,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_01   =  1,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_02   =  2,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_03   =  3,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_04   =  4,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_05   =  5,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_06   =  6,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_07   =  7,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_CPU  = 8,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_08 = 8,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_RCY  = 9,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_09 = 9,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_NON_CHAN = 10,
  JER2_ARAD_OFP_RATES_EGQ_NOF_CHAN_ARB = 11,
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_INVALID = INVALID_CALENDAR
}JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID;

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

/*Auxiliary*/
uint32
    jer2_arad_ofp_rates_port2chan_arb(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                fap_port,
    DNX_SAND_OUT  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
    );
/*Arad+ max burst features*/
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 max_burst_fc_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT  uint32 *max_burst_fc_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 max_burst_empty_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT  uint32 *max_burst_empty_queues
    );
/*Single port rate setting*/
int
  jer2_arad_ofp_rates_egq_single_port_rate_sw_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 rate
  );
int
  jer2_arad_ofp_rates_egq_single_port_rate_hw_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port
  );
int
  jer2_arad_ofp_rates_sch_single_port_rate_hw_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port

  );
int
  jer2_arad_ofp_rates_egq_single_port_rate_hw_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *rate
  );
/*Single port max burst setting*/
int
  jer2_arad_ofp_rates_single_port_max_burst_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 max_burst
  );
int
  jer2_arad_ofp_rates_single_port_max_burst_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *max_burst
  );
/*Interface rate setting*/
int
  jer2_arad_ofp_rates_egq_interface_shaper_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 ofp_ndx,
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    DNX_SAND_IN  uint32                 if_shaper_rate
  );
int
  jer2_arad_ofp_rates_egq_interface_shaper_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT  uint32                *if_shaper_rate
  );
/*TCG max burst setting*/
uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT uint32             *max_burst
    );
/*TCG rate setting*/
uint32
    jer2_arad_ofp_rates_egq_tcg_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                rate
    );
uint32
    jer2_arad_ofp_rates_egq_tcg_rate_hw_set(
    DNX_SAND_IN  int                   unit
    );
uint32
    jer2_arad_ofp_rates_egq_tcg_rate_hw_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             tcg_ndx, 
    DNX_SAND_OUT uint32             *rate
    );

uint32
    jer2_arad_ofp_rates_sch_tcg_rate_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                rate
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_rate_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT uint32                *rate
    );

/*PTC rate setting*/
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                rate
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_set(
    DNX_SAND_IN  int                   unit
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT uint32             *rate
    );
uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                rate
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_sw_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_IN  uint32             rate
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_hw_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_OUT uint32                *rate
    );
/*PTC max burst setting*/
uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core,
    DNX_SAND_IN  uint32             tm_port,   
    DNX_SAND_IN  uint32             port_priority_ndx, 
    DNX_SAND_OUT uint32             *max_burst
    );

void
  jer2_arad_JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO *info
  );

void
  jer2_arad_JER2_ARAD_OFP_RATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_OFP_RATE_INFO *info
  );

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_OFP_RATES_CAL_SET_to_string(
    DNX_SAND_IN JER2_ARAD_OFP_RATES_CAL_SET enum_val
  );

const char*
  jer2_arad_JER2_ARAD_OFP_SHPR_UPDATE_MODE_to_string(
    DNX_SAND_IN JER2_ARAD_OFP_SHPR_UPDATE_MODE enum_val
  );

void
  jer2_arad_JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO_print(
    DNX_SAND_IN JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO *info
  );

void
  jer2_arad_JER2_ARAD_OFP_RATE_INFO_print(
    DNX_SAND_IN JER2_ARAD_OFP_RATE_INFO *info
  );

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_API_OFP_RATES_INCLUDED__*/
#endif


