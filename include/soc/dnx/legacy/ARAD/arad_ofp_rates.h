/* $Id: jer2_arad_ofp_rates.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_OFP_RATES_INCLUDED__
/* { */
#define __JER2_ARAD_OFP_RATES_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/ARAD/arad_api_ofp_rates.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* 
 * Calender maximum length for regular channelize arbiter
 */
#define JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_SMALL_MAX    SOC_DNX_DEFS_MAX(SMALL_CHANNELIZED_CAL_SIZE)

/* 
 * Calender maximum length for first 2 channelize arbiters
 */
#define JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_BIG_MAX      SOC_DNX_DEFS_MAX(BIG_CHANNELIZED_CAL_SIZE)

#define JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX          (DNX_SAND_MAX(JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_SMALL_MAX,JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_BIG_MAX))

#define JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_PORT_PRIO_MAX 0x0100
#define JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_TCG_MAX       0x0100

#define JER2_ARAD_OFP_RATES_CALCAL_LEN_EGQ_MAX        0x0080
#define JER2_ARAD_OFP_RATES_CAL_LEN_SCH_MAX           0x0400

#define JER2_ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_CAL_BURST      0x1fff
#define JER2_ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_IF_RATE        0xffff
#define JER2_ARAD_OFP_RATES_EGRESS_SHAPER_MAX_INTERNAL_IF_BURST       0x4000 /* value of 0xffff causes for traffic drop */

#define JER2_ARAD_OFP_RATES_CHAN_ARB_NOF_BITS  (4)
#define JER2_ARAD_OFP_RATES_IFC_NOF_BITS (5)

#define JER2_ARAD_OFP_RATES_DEFAULT_PACKET_SIZE (64)

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

/*
 * A single entry of OFP calendar - EGQ. 
 * Consists of: base_q_pair and credit. 
 */
typedef struct  {
  uint32 base_q_pair;
  uint32 credit;
}JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY;

/*
 * A single entry of OFP calendar - SCH. 
 * Consists of: base_port_tc. 
 */
typedef uint32 JER2_ARAD_OFP_SCH_RATES_CAL_ENTRY;

/*
 *  EGQ calendar
 */
typedef struct
{
  JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY slots[JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX];
}JER2_ARAD_OFP_RATES_CAL_EGQ;

/*
 *  Scheduler calendar.
 *  Note: credit is always '1' for scheduler calendar.
 */
typedef struct
{
  JER2_ARAD_OFP_SCH_RATES_CAL_ENTRY slots[JER2_ARAD_OFP_RATES_CAL_LEN_SCH_MAX];
}JER2_ARAD_OFP_RATES_CAL_SCH;

typedef enum
{
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE = 0,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE = 1,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST = 2,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST = 3,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_RATE = 4,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_RATE = 5,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_BURST = 6,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_BURST = 7,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_EMPTY_Q_BURST = 8,
    JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_FC_Q_BURST = 9

}JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE;

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

uint32
  jer2_arad_ofp_rates_init(
    DNX_SAND_IN  int                    unit
  );

/*Verify*/
uint32
  jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rate
  );
uint32
  jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rate
  );
uint32
  jer2_arad_ofp_rates_tcg_shaper_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX           tcg_ndx,
    DNX_SAND_IN  uint32                 rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE set_state
  );
uint32
  jer2_arad_ofp_rates_port_priority_shaper_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 priority_ndx,
    DNX_SAND_IN  uint32                 rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE set_state
  );
int
  jer2_arad_ofp_rates_single_port_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  uint32                    *rate
  );
uint32
  jer2_arad_ofp_rates_interface_shaper_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port

  );
/*Auxiliary*/
int
    jer2_arad_ofp_rates_port2chan_arb_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                fap_port,
    DNX_SAND_OUT  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
    );
int
    jer2_arad_ofp_rates_port2chan_cal_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_OUT uint32                *calendar
    );
/*Arad+ max burst features*/
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_set_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 max_burst_fc_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_get_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT  uint32 *max_burst_fc_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_set_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 max_burst_empty_queues
    );
uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_get_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT  uint32 *max_burst_empty_queues
    );

int
    jer2_arad_ofp_rates_is_channalized(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_OUT uint32                *is_channalzied
    );

/*Single port rate setting*/
int
  jer2_arad_ofp_rates_sch_single_port_rate_sw_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 rate
  );

int
  jer2_arad_ofp_rates_sch_single_port_rate_hw_set_unsafe(
    DNX_SAND_IN  int                    unit
  );
int
  jer2_arad_ofp_rates_sch_single_port_rate_hw_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *rate
  );
int
  jer2_arad_ofp_rates_egq_single_port_rate_sw_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 rate
  );
int
    jer2_arad_ofp_rates_egq_single_port_rate_sw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_OUT uint32                *rate
    );
int
  jer2_arad_ofp_rates_egq_single_port_rate_hw_set_unsafe(
    DNX_SAND_IN  int                    unit
  );
uint32
  jer2_arad_ofp_rates_egq_single_port_rate_hw_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *rate
  );
/*Single port burst setting*/
int
  jer2_arad_ofp_rates_single_port_max_burst_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 max_burst
  );
int
  jer2_arad_ofp_rates_single_port_max_burst_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *max_burst
  );
uint32
  jer2_arad_ofp_rates_egq_interface_shaper_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    DNX_SAND_IN  uint32                 if_shaper_rate
  );
uint32
  jer2_arad_ofp_rates_egq_interface_shaper_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT  uint32                *if_shaper_rate
  );
/*TCG max burst setting*/
uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,     
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_OUT uint32                *max_burst
    );
/*TCG rate setting*/
int
    jer2_arad_ofp_rates_egq_tcg_rate_sw_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core, 
    DNX_SAND_IN  uint32                 tm_port,  
    DNX_SAND_IN  JER2_ARAD_TCG_NDX           tcg_ndx,   
    DNX_SAND_IN  uint32                 tcg_rate  
    );
int
    jer2_arad_ofp_rates_egq_tcg_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core
    );
int
    jer2_arad_ofp_rates_egq_tcg_rate_hw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID      tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_OUT  uint32               *tcg_rate
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_rate_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                rate
    );
uint32
    jer2_arad_ofp_rates_sch_tcg_rate_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT  uint32               *rate
    );

/*PTC rate setting*/
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_sw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                ptc_rate
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          ptc_ndx,   
    DNX_SAND_OUT uint32                *ptc_rate
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                rate
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_sw_set_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                rate
    );

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_OUT uint32                *rate
    );
/*PTC max burst setting*/
uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                port_priority_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                max_burst
    );
uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID      tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_OUT uint32                *max_burst
    );

/*********************************************************************
* NAME:
*     jer2_arad_ofp_rates_egq_chnif_shaper_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set Egress transmit Schduler shaper. 
*    This shaper might be different than Calender interface shaper. 
*    In this case, shaper will always limit bandwidth of total interface rate,
*    as expected from serdes rates. 
* INPUT:
*  DNX_SAND_IN  int                    unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID          if_id -
*     Interface ID.
*  DNX_SAND_IN  uint32 	           rate -
*     rate [kbps].
*  DNX_SAND_IN  uint32 max_burst -
*     max burst.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ofp_rates_egq_chnif_shaper_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                  rate_kbps,
    DNX_SAND_IN  uint32                  max_burst
  );

/*********************************************************************
* NAME:
*   jer2_arad_ofp_static_cal_build_from_rates
* TYPE:
*   PROC
* FUNCTION:
*   generate a calendar by assign reserved slots for each physical port.
*   unused slots filled with JER2_ARAD_EGQ_NIF_PORT_CAL_BW_INVALID
* INPUT:
*   DNX_SAND_IN  uint32                   ports_rates[JER2_ARAD_NIF_NOF_NIFS] -
*   array of the ports rates in gbits per sec
*   DNX_SAND_OUT DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_SCH         *calendar-
*   the calendar built by the function
*   
* REMARKS:
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ofp_if_spr_rate_by_reg_val_set(
     DNX_SAND_IN int                   unit,
     DNX_SAND_IN uint32                port,                            
     DNX_SAND_IN uint32                shpr_rate_reg_val
     );

uint32
  jer2_arad_ofp_if_shaper_disable(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  soc_port_t            port,
    DNX_SAND_OUT uint32*               shpr_rate_reg_val
  );

uint32
  jer2_arad_ofp_otm_shapers_disable(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  uint32     queue_rates_size,
    DNX_SAND_OUT uint32*    queue_rates
  );

uint32
  jer2_arad_ofp_otm_shapers_set(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  uint32     queue_rates_size,
    DNX_SAND_IN  uint32*    queue_rates
  );

uint32
  jer2_arad_ofp_tcg_shapers_enable(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  uint32                   enable
  );
uint32
  jer2_arad_ofp_q_pair_shapers_enable(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  uint32                   enable
  );
uint32
  jer2_arad_ofp_rates_max_credit_empty_port_set(
     DNX_SAND_IN int                                    unit,
     DNX_SAND_IN int                                    arg
     );
uint32
  jer2_arad_ofp_rates_max_credit_empty_port_get(
     DNX_SAND_IN int                                    unit,
     DNX_SAND_OUT int*                                    arg
     );

uint32
  jer2_arad_ofp_rates_egq_shaper_rate_from_internal(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CAL_TYPE cal_type,
    DNX_SAND_IN  uint32                      rate_internal,
    DNX_SAND_OUT uint32                     *rate_kbps
  );

uint32
  jer2_arad_ofp_rates_egq_shaper_rate_to_internal(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CAL_TYPE cal_type,
    DNX_SAND_IN  uint32                      rate_kbps,
    DNX_SAND_OUT uint32                     *rate_internal
  );

int
  jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_SET                   cal2set,    
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE   field_type,
    DNX_SAND_OUT soc_reg_t                                *register_name,
    DNX_SAND_OUT soc_field_t                              *field_name
  );

int
  jer2_arad_ofp_rates_egress_shaper_reg_field_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_reg_t                                register_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   *data
    );

int 
jer2_arad_ofp_rates_egress_shaper_reg_field_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_reg_t                                register_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   data
    );

int
    jer2_arad_ofp_rates_packet_mode_packet_size_get (
        DNX_SAND_IN   int                   unit, 
        DNX_SAND_IN   int                   core,  
        DNX_SAND_OUT  uint32                *num_of_bytes
        );

int
jer2_arad_ofp_rates_egress_shaper_cal_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_IN  uint32                                   data
    );

int
jer2_arad_ofp_rates_egress_shaper_cal_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_OUT uint32                                   *data
    );

int
  jer2_arad_ofp_rates_from_rates_to_calendar(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  uint32                    *ports_rates,
    DNX_SAND_IN  uint32                    nof_ports,
    DNX_SAND_IN  uint32                    total_credit_bandwidth,
    DNX_SAND_IN  uint32                    max_calendar_len,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_SCH    *calendar,
    DNX_SAND_OUT uint32                    *calendar_len
  );

uint32
  jer2_arad_ofp_rates_fixed_len_cal_build(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                   *port_nof_slots,
    DNX_SAND_IN  uint32                   nof_ports,
    DNX_SAND_IN  uint32                    calendar_len,
    DNX_SAND_IN  uint32                    max_calendar_len,
    DNX_SAND_IN  uint32                    is_fqp_pqp,
    DNX_SAND_OUT uint32                   *calendar
  );
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_OFP_RATES_INCLUDED__*/
#endif



