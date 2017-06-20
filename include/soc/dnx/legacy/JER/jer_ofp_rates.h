/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_jer_ofp_rates.h
 */

#ifndef __JER2_JER_OFP_RATES_INCLUDED__

#define __JER2_JER_OFP_RATES_INCLUDED__

/*
 * Includes
 */
#include <soc/dnx/legacy/ARAD/arad_api_ofp_rates.h>

/*
 * Defines
 */

/* calednars 0...31 are reserved for channelized interfaces */
#define JER2_JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_NON_CHN     32      
#define JER2_JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_QUEUE_PAIR  33
#define JER2_JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_CAL_ID_TCG         34
#define JER2_JERICHO_OFP_RATES_EGRESS_SHAPER_CONFIG_NUM_OF_CALS        35           



/*
 * functions definitions
 */
int
soc_jer2_jer_ofp_rates_init(
    DNX_SAND_IN  int                         unit
  );

int
soc_jer2_jer_ofp_rates_egq_interface_shaper_set(
    DNX_SAND_IN  int                          unit,
    DNX_SAND_IN  int                          core,
    DNX_SAND_IN  uint32                       tm_port,
    DNX_SAND_IN  DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    DNX_SAND_IN  uint32                       if_shaper_rate
  );


int
soc_jer2_jer_ofp_rates_egq_interface_shaper_get(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_OUT uint32                    *if_shaper_rate
  );

int
soc_jer2_jer_ofp_rates_interface_internal_rate_get(
    DNX_SAND_IN   int                   unit, 
    DNX_SAND_IN   int                   core, 
    DNX_SAND_IN   uint32                egr_if_id, 
    DNX_SAND_OUT  uint32                *internal_rate);

int
soc_jer2_jer_ofp_rates_egq_single_port_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_IN  uint32                rate
    );

int soc_jer2_jer_ofp_rates_calendar_allocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core,
    DNX_SAND_IN  uint32          tm_port,
    DNX_SAND_IN  uint32          num_required_slots,
    DNX_SAND_IN  uint32          old_cal,
    DNX_SAND_OUT uint32          *new_cal);

int
soc_jer2_jer_ofp_rates_calendar_deallocate(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core,
    DNX_SAND_IN  uint32          cal_id);

int
soc_jer2_jer_ofp_rates_port2chan_cal_get(
    DNX_SAND_IN  int        unit, 
    DNX_SAND_IN  int        core, 
    DNX_SAND_IN  uint32     tm_port, 
    DNX_SAND_OUT uint32     *calendar);

int
soc_jer2_jer_ofp_rates_retrieve_egress_shaper_reg_field_names(
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_SET                   cal2set,    
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE   field_type,
    DNX_SAND_OUT soc_mem_t                                *memory_name,
    DNX_SAND_OUT soc_field_t                              *field_name
  );

int 
soc_jer2_jer_ofp_rates_egress_shaper_mem_field_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_reg_t                                register_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   *data
    );

int
soc_jer2_jer_ofp_rates_egress_shaper_mem_field_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_mem_t                                mem_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   data
    );

soc_mem_t
  soc_jer2_jer_ofp_rates_egq_scm_chan_arb_id2scm_id(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 chan_arb_id
  );


int
soc_jer2_jer_ofp_rates_egress_shaper_cal_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_IN  uint32                                   data
    );

int
soc_jer2_jer_ofp_rates_egress_shaper_cal_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_OUT uint32                                   *data
    );
#endif /*__JER2_JER_OFP_RATES_INCLUDED__*/

