#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_ofp_rates.c,v 1.70 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ

/*************
 * INCLUDES  *
#include <soc/mem.h>

 *************/
/* { */
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>


#include <soc/error.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/JER/jer_ofp_rates.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_device.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Utils/sand_conv.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_OFP_RATES_OLP_GUARANTEED_KBPS             (1 * 1024 * 1024)
#define JER2_ARAD_OFP_RATES_OAMP_GUARANTEED_KBPS             (1 * 1024 * 1024)
/* Use as static shaper with unlimited bandwdith */
#define JER2_ARAD_OFP_RATES_QPAIR_INITIAL_KBPS              (800 * 1024 * 1024)
#define JER2_ARAD_OFP_RATES_TCG_INITIAL_KBPS                (800 * 1024 * 1024)

/* Initial rate deviation for calendar-building algorithm. */
#define JER2_ARAD_OFP_RATES_CAL_RATE_DEVIATION              (100000000)

/* Number of clocks to traverse a single calendar slot in the EGQ */
#define JER2_ARAD_EGQ_CAL_SLOT_TRAVERSE_IN_CLOCKS           (2)

/* EGQ bandwidth The unit is 1/256th byte chanelized arbiters*/
#define JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_CHAN_ARB            (256 / 8)

/* EGQ bandwidth The unit is 1/128th byte for QPAIR and TCG*/
#define JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG           (128 / 8)

/* size of scm table portToCrAdd field */
#define JER2_ARAD_EGQ_NIF_MAL_SCM_TBL_ENTRY_PORT_CR_TO_ADD_NOF_BITS              (18)

/* Indication of an uninitialized entry */
#define JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID            (0xfff)

/* The initial rate given to all port before random test */

/* Undefined mapping to base_q_pair */
#define JER2_ARAD_OFP_RATES_UNMAP_BASE_Q_PAIR               (JER2_ARAD_NOF_FAP_PORTS)
/* 
 * Max ofp rates port id - used to configure dummies.
 * A dummy is defined as MAX_FAP_PORT_INDEX with 0 credits
 */
#define JER2_ARAD_OFP_RATES_DUMMY_PORT_ID                   (JER2_ARAD_MAX_FAP_PORT_ID)

/* */
#define JER2_ARAD_OFP_RATES_NOF_ITERATIONS                  (4)


#define JER2_ARAD_OFP_RATES_NOF_PRIORITIES_TCG_SUPPORT (8)
/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/*Reg access{*/

#define SOC_REG64_SET(reg_desc, index,reg_value) \
{\
    int res = SOC_E_NONE;\
    res = soc_reg64_set(unit, (reg_desc), (REG_PORT_ANY), (index), (reg_value));\
    DNXC_SAND_IF_ERR_EXIT(res);\
}
#define SOC_REG64_GET(reg_desc, index, reg_value)  \
{ \
    int res = SOC_E_NONE;\
    COMPILER_64_ZERO(reg_value); \
    res = soc_reg64_get(unit, reg_desc, REG_PORT_ANY, index, &(reg_value));    \
    DNXC_SAND_IF_ERR_EXIT(res);\
}

#define SOC_FIELD_GET(reg_desc, fld_desc, fld_value) \
{\
  int res = SOC_E_NONE;\
  uint32 reg_value; \
  fld_value = 0; \
  res = soc_reg32_get(unit, reg_desc, (REG_PORT_ANY), 0, &reg_value);\
  DNXC_SAND_IF_ERR_EXIT(res);\
  fld_value = soc_reg_field_get(unit, reg_desc, reg_value, fld_desc); \
}
#define SOC_FIELD64_GET(reg_desc, fld_desc, fld_value) \
{\
    uint64 reg_value; \
    COMPILER_64_ZERO(fld_value); \
    SOC_REG64_GET(reg_desc, 0, reg_value); \
    fld_value = soc_reg64_field_get(unit, reg_desc, reg_value, fld_desc); \
}
#define SOC_FIELD64_SET(reg_desc, fld_desc, fld_value) \
{\
  uint64 reg_value; \
  SOC_REG64_GET(reg_desc, 0, reg_value); \
  soc_reg64_field_set(unit, reg_desc, &reg_value,     fld_desc, fld_value); \
  SOC_REG64_SET(reg_desc, 0,reg_value); \
}

#define SOC_FIELD_SET(reg_desc, fld_desc, fld_value) \
{\
  int res;\
  uint32 reg_value; \
  res = soc_reg32_get(unit, reg_desc, (REG_PORT_ANY), 0, &reg_value);\
  DNXC_SAND_IF_ERR_EXIT(res);\
  soc_reg_field_set(unit, reg_desc, &reg_value,     fld_desc, fld_value); \
  res = soc_reg32_set(unit, reg_desc, REG_PORT_ANY, 0, reg_value); \
  DNXC_SAND_IF_ERR_EXIT(res);\
}
#define SOC_FIELD32_REG64_GET(reg_desc, fld_desc, fld_value) \
{\
    uint64 reg_value; \
    fld_value=0; \
    SOC_REG64_GET(reg_desc, 0, reg_value); \
    fld_value = soc_reg64_field32_get(unit, reg_desc, reg_value, fld_desc); \
}
#define SOC_FIELD32_REG64_SET(reg_desc, fld_desc, fld32_value) \
{\
    uint64 reg64_value; \
    SOC_REG64_GET(reg_desc, 0, reg64_value); \
    soc_reg64_field32_set(unit, reg_desc, &reg64_value, fld_desc, fld32_value); \
    SOC_REG64_SET(reg_desc, 0, reg64_value); \
}
/*Reg access}*/


/* Calculate CAL length EGQ depends on chan_arb_id */
#define JER2_ARAD_OFP_RATES_EGQ_CAL_LEN_EGQ(unit, chan_arb_id) \
  (((chan_arb_id < SOC_DNX_DEFS_GET(unit, nof_big_channelized_calendars)) || \
    (chan_arb_id > (SOC_DNX_DEFS_GET(unit, nof_channelized_calendars)-1))) || \
    (chan_arb_id == SOC_DNX_DEFS_GET(unit, non_channelized_cal_id) && SOC_IS_JERICHO(unit)) ? \
   (SOC_DNX_DEFS_GET(unit, big_channelized_cal_size)) : (SOC_DNX_DEFS_GET(unit, small_channelized_cal_size)))

/* Calculate offset for SCM table */
#define JER2_ARAD_OFP_RATES_EGQ_SCM_OFFSET_GET(unit, cal_info,cal2set) \
  ((cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB) ? \
  (cal2set * JER2_ARAD_OFP_RATES_EGQ_CAL_LEN_EGQ(unit, cal_info->chan_arb_id) ) : \
  ((cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY) ? \
  (cal2set * JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_PORT_PRIO_MAX) : cal2set * JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_TCG_MAX))

#define JER2_ARAD_OFP_RATES_EGQ_PMC_LEN(cal_info) \
  (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB ? JER2_ARAD_EGR_NOF_BASE_Q_PAIRS : \
    (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY ? JER2_ARAD_EGR_NOF_Q_PAIRS : \
    JER2_ARAD_EGR_NOF_PS*JER2_ARAD_NOF_TCGS)) 

/* Calculate offset for PMC table */
#define JER2_ARAD_OFP_RATES_EGQ_PMC_OFFSET_GET(cal_info,egq2set,ofp_index) \
  (JER2_ARAD_OFP_RATES_EGQ_PMC_LEN(cal_info) * egq2set + ofp_index)

/* Have a specific offset for TCG ID */
#define JER2_ARAD_OFP_RATES_TCG_ID_GET(ps,tcg_ndx) \
  (ps*JER2_ARAD_NOF_TCGS_IN_PS + tcg_ndx)


/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
  /*
   *  Base_q_pair index. Range: 0-255
   */
  uint32 base_q_pair;
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
  JER2_ARAD_TCG_NDX tcg_ndx;
}JER2_ARAD_OFP_RATES_INTERNAL_INFO;

typedef struct
{
  /*
   *  Number of valid entries in DNX_TMC_EGR_PORT_RATES
   *  table. Range: 1 - 79.
   */
  uint32 nof_valid_entries;
  /*
   *  Shaper rates and credit rates for all OFP-s, per
   *  interface.
   */
  JER2_ARAD_OFP_RATES_INTERNAL_INFO rates[JER2_ARAD_NOF_FAP_PORTS];
}JER2_ARAD_OFP_RATES_TBL_INTERNAL_INFO;

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

static uint32
  jer2_arad_ofp_rates_egq_shaper_config(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO       *cal_info,
    DNX_SAND_IN  uint32                         rate
  );


static
  void
    jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(
      DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_INFO *info
    );

static 
  uint32
    jer2_arad_ofp_rates_retrieve_egress_shaper_setting_field(
      DNX_SAND_IN  int                       unit,
      DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO      *cal_info,
      DNX_SAND_OUT soc_field_t                  *field_name
    )
{
   DNXC_INIT_FUNC_DEFS

   DNXC_NULL_CHECK(cal_info);
   DNXC_NULL_CHECK(field_name);

   /*
   * Check which calendars (EGQ & SCH - Calendars get 'A' or 'B')
   * are currently active.
   */
  switch (cal_info->cal_type)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB:
    *field_name = OTM_SPR_SET_SELf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
    *field_name = QPAIR_SPR_SET_SELf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_TCG:
    *field_name = TCG_SPR_SET_SELf;
    break;
  default:
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Unkonown cal info")));
    break;
  }

exit:
  DNXC_FUNC_RETURN
}


/*transform if_id+nif_type to nif_idx for api commands
  if_id : 0-31 
  nif_type : ILKN-0,ILKN-1, RXAUI-0 ...
  nif_idx:0-1000 (api "knows" only nif_idx values. 
*/

uint32
  jer2_arad_ofp_rates_init(
    DNX_SAND_IN  int unit
  )
{
  uint32
    fld_val,
    data,
    res = SOC_E_NONE;
  JER2_ARAD_OFP_RATES_CAL_INFO
    cal_info;
  uint8
    qpair_tcg_shapers_enable = 0x0,
    qpair_shapers_enable = 0x0,
    tcg_shapers_enable = 0x0;
  uint32 init_max_burst = JER2_ARAD_OFP_RATES_BURST_DEAULT;
   int core=0;
   int idx, ps_id, tcg_id;
   JER2_ARAD_SW_DB_DEV_RATE            
        tcg_rate;
   JER2_ARAD_SW_DB_DEV_RATE            
        queue_rate;

  DNXC_INIT_FUNC_DEFS

  if (!SOC_UNIT_NUM_VALID(unit)) {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "unit not valid")));
  }
  
  SOC_CLEAR(&cal_info, JER2_ARAD_OFP_RATES_CAL_INFO, 1);

  res = READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, REG_PORT_ANY, &data);
  DNXC_IF_ERR_EXIT(res);

  /*Init max burst entries to max value*/
  res = jer2_arad_fill_table_with_entry(unit, EGQ_PMCm, MEM_BLOCK_ANY, &init_max_burst);
  DNXC_SAND_IF_ERR_EXIT(res);
  res = jer2_arad_fill_table_with_entry(unit, EGQ_TCG_PMCm, MEM_BLOCK_ANY, &init_max_burst);
  DNXC_SAND_IF_ERR_EXIT(res);
  res = jer2_arad_fill_table_with_entry(unit, EGQ_QP_PMCm, MEM_BLOCK_ANY, &init_max_burst);
  DNXC_SAND_IF_ERR_EXIT(res);

  /* update sw db with default burst size */
  for (idx = 0; idx < JER2_ARAD_EGR_NOF_BASE_Q_PAIRS; idx++)
  {
      /* ports burst */
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.rates.egq_bursts.set(unit, core, idx, init_max_burst);
      DNXC_IF_ERR_EXIT(res);

      /* port priority burst */
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.queue_rate.get(unit, core, idx, &queue_rate);
      DNXC_IF_ERR_EXIT(res);
      queue_rate.egq_bursts = init_max_burst;
      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.queue_rate.set(unit, core, idx, &queue_rate);
      DNXC_IF_ERR_EXIT(res);
  }

  for (ps_id = 0; ps_id < JER2_ARAD_EGR_NOF_PS; ps_id++)
  {
      for (tcg_id = 0; tcg_id < JER2_ARAD_NOF_TCGS; tcg_id++)
      {
          /* tcg burst */
          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.tcg_rate.get(unit, core, ps_id, tcg_id, &tcg_rate);
          DNXC_IF_ERR_EXIT(res);
          tcg_rate.egq_bursts = init_max_burst;
          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.tcg_rate.set(unit, core, ps_id, tcg_id, &tcg_rate);
          DNXC_IF_ERR_EXIT(res);
      }
  }
  

  /* By default Disable / Enable QPair / TCG shapers depends on rev */
  if SOC_IS_ARAD_B0_AND_ABOVE(unit)/* will be implemented when resolution will be fixed as well */
  {
    fld_val = soc_property_get(unit, "qpair_tcg_shapers_enable",0x1);
    qpair_tcg_shapers_enable = fld_val;
    qpair_shapers_enable = fld_val;
    tcg_shapers_enable = fld_val;
  }
  else
  {
    qpair_tcg_shapers_enable = 0x0;
    qpair_shapers_enable = 0x0;/*Always disable the qpair shaper*/
    tcg_shapers_enable = 0x0;
  }
  /* Enable OTM shaper */
  fld_val = 0x1;
  soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, OTM_SPR_ENAf, fld_val);

  /* Enable/Disable QPairs shaper */
  fld_val = qpair_shapers_enable;
  soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, QPAIR_SPR_ENAf, fld_val);  

  /* Enable/Disable TCG shaper */
#if 1
  fld_val = tcg_shapers_enable;
#endif
  soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, TCG_SPR_ENAf, fld_val);   
  /* Enable resolution decrease for TCG and QPair shapers */
  if(SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
      soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, QPAIR_SPR_RESOLUTIONf, 1);  
      soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, TCG_SPR_RESOLUTIONf, 1);  
  }

  /* empty queues are ignored (field name is confusing, 0 means ignore) */
  soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, IGNORE_QEMPTYf, 0);

  /* set default packet size in bytes for packet mode shaping */
  soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &data, SHAPER_PACKET_RATE_CONSTf, JER2_ARAD_OFP_RATES_DEFAULT_PACKET_SIZE);

  res = WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, REG_PORT_ANY,data);
  DNXC_SAND_IF_ERR_EXIT(res);

  res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.set(unit,qpair_tcg_shapers_enable);
  DNXC_IF_ERR_EXIT(res);

  /* Set by default TCG and QPAIR total shapers to be maximal */
  cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY;  
  res = jer2_arad_ofp_rates_egq_shaper_config(
          unit,
          &cal_info,
          JER2_ARAD_OFP_RATES_QPAIR_INITIAL_KBPS
        );
  DNXC_IF_ERR_EXIT(res);

  cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_TCG;  
  res = jer2_arad_ofp_rates_egq_shaper_config(
          unit,
          &cal_info,
          JER2_ARAD_OFP_RATES_TCG_INITIAL_KBPS
        );
  DNXC_IF_ERR_EXIT(res);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  /*
   *    If PP is enabled, reserve some rate for the OLP/OAM.
   *  The OLP shares a calendar with the EGQ.
   *  This is an additional credit source for the OLP/OAM
   *  that guarantees the OLP/OAM is not under starvation
   *  even if the CPU is blocked (e.g. Flow Control).
   */
  pp_enable = SOC_DNX_PP_ENABLE(unit);
  if (pp_enable && (SOC_INFO(unit).olp_port[0]>=0))
  {
    res = jer2_arad_sch_if_shaper_rate_set(
            unit,
            core, 
            JER2_ARAD_OLP_PORT_ID,
            JER2_ARAD_OFP_RATES_OLP_GUARANTEED_KBPS
          );
    DNXC_IF_ERR_EXIT(res);
  }

  oamp_enable = SOC_DNX_CONFIG(unit)->pp.oamp_enable;
  if (oamp_enable)
  {
    res = jer2_arad_sch_if_shaper_rate_set(
            unit,
            core,
            JER2_ARAD_OAMP_PORT_ID,
            JER2_ARAD_OFP_RATES_OAMP_GUARANTEED_KBPS
          );
    DNXC_IF_ERR_EXIT(res);
  }
#endif 

exit:
  DNXC_FUNC_RETURN
}

/*
 * Indication for dummy entry
 */
static uint8
  jer2_arad_ofp_rates_is_cal_entry_dummy(
    DNX_SAND_IN  JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY  *cal_entry
  )
{
  JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY
    dummy_entry;

  dummy_entry.base_q_pair = JER2_ARAD_OFP_RATES_DUMMY_PORT_ID;
  dummy_entry.credit = 0;

  if (cal_entry)
  {
    return DNX_SAND_NUM2BOOL(0 == dnx_sand_os_memcmp(cal_entry, &dummy_entry, sizeof(JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY)));
  }

  return FALSE;
}

static uint32
  jer2_arad_ofp_rates_port2chan_arb_get_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  uint32                         tm_port,
    DNX_SAND_IN  int                            core,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    uint32
      res = SOC_E_NONE;
    soc_reg_above_64_val_t
      data,
      field;
    JER2_ARAD_INTERFACE_ID
      egr_interface_id;
    uint32
      chan_arb_field_val = 0,
      chan_arb_val = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(chan_arb_id);

    /* Retreive EGQ interface ID */
    res = jer2_arad_port2egress_offset(unit, core, tm_port, &egr_interface_id);
    DNXC_IF_ERR_EXIT(res);
    /* 
    * EGQ Interface ID to Channelize Arbiter ID 
    * Cannelize Arbiter ID 0 means non chanelized arbiter 
    */
    res = READ_EGQ_MAPPING_INTERFACES_TO_CHAN_ARBITERr(unit,data);
    DNXC_SAND_IF_ERR_EXIT(res);
    soc_reg_above_64_field_get(unit,EGQ_MAPPING_INTERFACES_TO_CHAN_ARBITERr,data,MAP_IFC_TO_CHAN_ARBf,field);
    SHR_BITCOPY_RANGE(&chan_arb_field_val,0,field,JER2_ARAD_OFP_RATES_CHAN_ARB_NOF_BITS*egr_interface_id,JER2_ARAD_OFP_RATES_CHAN_ARB_NOF_BITS);
    res = jer2_arad_nif_chan_arb_field_val_to_enum(unit,chan_arb_field_val,&chan_arb_val);
    DNXC_SAND_IF_ERR_EXIT(res);

    switch (chan_arb_val)
    {
        case JER2_ARAD_OFP_RATES_EGQ_NOF_CHAN_ARB:    
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "illegal chan arbiter")));
            break;
        default:
            *chan_arb_id = chan_arb_val;
            break;
    }

exit:
  DNXC_FUNC_RETURN;
#endif 
    return -1;
}

/*
 *  This functions returns channelize arbiter ID from 
 *  Interface-ID.
 *  Channelize arbiter ID is the same for EGQ and SCH. 
 *  Note: For non channelize interface returns NON channelize arbiter.
 */
static uint32
  jer2_arad_ofp_rates_if_id2chan_arb_get_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core,
    DNX_SAND_IN  uint32                         tm_port,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
  )
{
    uint32
      res = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    res = jer2_arad_ofp_rates_port2chan_arb_get_unsafe(unit, tm_port, core, chan_arb_id);
    DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;

}


/* JER2_ARAD calender algorithm { */

/* QPAIR\TCG calender algorithm
 * If an expected credit to a port has over 18 bits, add slot to it.
 */
static uint32
 jer2_arad_ofp_rates_cal_improve_nof_slots(
              DNX_SAND_IN    int    unit,
              DNX_SAND_IN    uint32 *ports_rates,
              DNX_SAND_IN    uint32 nof_ports,
              DNX_SAND_IN    uint32 max_calendar_len,
              DNX_SAND_INOUT uint32 *loc_calendar_len,
              DNX_SAND_INOUT uint32 *port_nof_slots,
              DNX_SAND_OUT   uint8 *slots_added
              )
{
    DNX_SAND_U64 
        u64_1, 
        u64_2;
    uint32 
        idx,
        rem, 
        expected_credits,
        mask = ~((1 << SOC_DNX_DEFS_GET(unit, scm_qp_tcg_cr_to_add_nof_bits)) - 1);
        
    DNXC_INIT_FUNC_DEFS
    DNXC_NULL_CHECK(ports_rates);
    DNXC_NULL_CHECK(port_nof_slots);

    *slots_added = FALSE;
    /*find a port that has to many credits, and increas its slots*/
    for (idx = 0; idx < nof_ports; ++idx) {
        /*if ports_rates[idx] > 0, validate that with a given nof_slots[idx] it doesn't recieve to many credits*/
        if (ports_rates[idx] > 0 ) {
            while(TRUE) {
                 /*Translate port rates to credits*/
                 dnx_sand_u64_multiply_longs(ports_rates[idx],(*loc_calendar_len) * /*calendar_len == num_of_calenders = loc_calendar_len*/ 
                                               JER2_ARAD_EGQ_CAL_SLOT_TRAVERSE_IN_CLOCKS * JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG, &u64_1);
                 /*Divide port credits between slots*/
                 rem = dnx_sand_u64_devide_u64_long(&u64_1,port_nof_slots[idx] * jer2_arad_chip_kilo_ticks_per_sec_get(unit), &u64_2);
                 dnx_sand_u64_to_long(&u64_2, &(expected_credits));
                 expected_credits += ((rem > 0 )? 1 : 0);
                
                 /*Continue until num of bits for credit per slot below SOC_DNX_DEFS_GET(unit, scm_qp_tcg_cr_to_add_nof_bits) */
                 if (!(expected_credits & mask)) {
                     break;
                 }

                  /*if we exceed beyond max_calendar_len return an error*/
                 (*loc_calendar_len)++;
                 port_nof_slots[idx]++;
                 (*slots_added) = TRUE;
                 if (max_calendar_len < *loc_calendar_len)
                 {
                     DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "calculated cal len bigger than max")));
                 }
            }
        }
    }
exit:
  DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_cal_per_len_build(
    DNX_SAND_IN   int                      unit,
    DNX_SAND_IN   uint32                    *ports_rates,
    DNX_SAND_IN   uint32                   nof_ports,
    DNX_SAND_IN   uint32                    total_credit_bandwidth,
    DNX_SAND_IN   uint32                    max_calendar_len,
    DNX_SAND_IN   uint32                    tentative_len,
    DNX_SAND_OUT  uint32                    *actual_len,
    DNX_SAND_OUT  uint32                   *port_nof_slots,
    DNX_SAND_OUT  uint32                    *deviation
  )
{
  uint32
    port,
    slots_rates[JER2_ARAD_NOF_FAP_PORTS],
    total_num_slots = 0,
    calc_deviation;
  uint32
    rem;
  uint32
    num_slots,
    temp1,
    temp2;
  DNX_SAND_U64
    u64_1,
    u64_2;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(actual_len);
  DNXC_NULL_CHECK(deviation);

  if (0 == tentative_len)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "illegal cal len")));
  }

  SOC_CLEAR(slots_rates, uint32, JER2_ARAD_NOF_FAP_PORTS);

  /*
   * Calculate the number of calendar slots per port
   */
  for (port = 0; port < nof_ports; ++port)
  {
    if (ports_rates[port] > 0)
    {
      /*
       *  Calculate number of slots for each port according to:
       *
       *                          port_credit_rate * cal_num_of_slots
       *  port_num_of_cal_slots = ------------------------------------
       *                               total_credit_bandwidth
       *
       *
       */

      dnx_sand_u64_multiply_longs(ports_rates[port], tentative_len, &u64_1);
      rem = dnx_sand_u64_devide_u64_long(&u64_1, total_credit_bandwidth, &u64_2);
      dnx_sand_u64_to_long(&u64_2, &(num_slots));

      /*Round up in case the remainder is greater then 0, or in case num_slots == 0*/
      num_slots = (rem > 0 ? (num_slots + 1) : ((num_slots == 0) ? num_slots + 1 : num_slots));

      slots_rates[port] = num_slots;
      total_num_slots += num_slots;
    }
  }
  if (total_num_slots > max_calendar_len ||
      total_credit_bandwidth == 0 || total_num_slots == 0) {
      /* This solution is not acceptable, so return zero */
      *actual_len = 0;
  } 
  else 
  {

      calc_deviation = 0;
      for (port = 0; port < nof_ports; ++port)
      {
        dnx_sand_u64_multiply_longs(JER2_ARAD_OFP_RATES_CAL_RATE_DEVIATION, ports_rates[port], &u64_1);
        rem = dnx_sand_u64_devide_u64_long(&u64_1, total_credit_bandwidth, &u64_2);
        dnx_sand_u64_to_long(&u64_2, &(temp1));
        temp1 = (rem > 0 ? temp1 + 1 : temp1);

        dnx_sand_u64_multiply_longs(JER2_ARAD_OFP_RATES_CAL_RATE_DEVIATION, slots_rates[port], &u64_1);
        rem = dnx_sand_u64_devide_u64_long(&u64_1, total_num_slots, &u64_2);
        dnx_sand_u64_to_long(&u64_2, &(temp2));
        temp2 = (rem > 0 ? temp2 + 1 : temp2);

        calc_deviation += dnx_sand_abs(temp2 - temp1);
      }

      *actual_len = total_num_slots;
      SOC_COPY(port_nof_slots, slots_rates, uint32, JER2_ARAD_NOF_FAP_PORTS);
      *deviation = calc_deviation;
  }

exit:
  DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_cal_len_calculate(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                    *ports_rates,
    DNX_SAND_IN  uint32                   nof_ports,
    DNX_SAND_IN  uint32                    total_credit_bandwidth,
    DNX_SAND_IN  uint32                    max_calendar_len,
    DNX_SAND_OUT uint32                    *actual_calendar_len,
    DNX_SAND_OUT uint32                   *port_nof_slots
  )
{
  uint32
    res,
    deviation,
    min_deviation,
    tentative_len,
    best_tentative_len,
    actual_len;
  uint32
    port_num_slots[JER2_ARAD_NOF_FAP_PORTS];
  uint32
    min_port_rate = 0x1;

  DNXC_INIT_FUNC_DEFS

  /*
   * Determine calendar len.
   * A possible algorithm is to go over all possible length values
   * which ranges from nof_active_ports (1 slot per port) to max_calendar_len.
   * For each value, calculate a solution and its deviation from the required
   * rates. Then pick the most accurate solution, which is the one with the smallest
   * deviation. The algorithm could also be stopped when the deviation is smaller than
   * a pre configured value.
   */
  min_deviation = JER2_ARAD_OFP_RATES_CAL_RATE_DEVIATION;
  best_tentative_len = 0;
  for (tentative_len = 1; tentative_len <= max_calendar_len; ++tentative_len)
  {
    res = jer2_arad_ofp_rates_cal_per_len_build(
            unit,
            ports_rates,
            nof_ports,
            total_credit_bandwidth,
            max_calendar_len,
            tentative_len,
            &actual_len,
            port_num_slots,
            &deviation
          );
    DNXC_IF_ERR_EXIT(res);

    /*
     * Check if we received a legal solution for this tentative length
     */
    if (actual_len == 0)
    {
      continue;
    }
    if (deviation < min_deviation)
    {
      min_deviation = deviation;
      best_tentative_len = tentative_len;
      if (0 == min_deviation)
      {
        break;
      }
    }
  }
  /*
   * sanity check
   */
  if (0 == best_tentative_len)
  {
    best_tentative_len = 1;

    /*
     *  Minimal calendar
     */
    res = jer2_arad_ofp_rates_cal_per_len_build(
          unit,
          &min_port_rate,
          1,
          min_port_rate,
          max_calendar_len,
          min_port_rate,
          &actual_len,
          port_num_slots,
          &deviation
        );
    DNXC_IF_ERR_EXIT(res);
  }
  else
  {
    /*
     * Rebuild the best calendar that we found
     */
    res = jer2_arad_ofp_rates_cal_per_len_build(
            unit,
            ports_rates,
            nof_ports,
            total_credit_bandwidth,
            max_calendar_len,
            best_tentative_len,
            &actual_len,
            port_num_slots,
            &deviation
          );
    DNXC_IF_ERR_EXIT(res);
  }

  *actual_calendar_len = actual_len;
  SOC_COPY(port_nof_slots, port_num_slots, uint32, JER2_ARAD_NOF_FAP_PORTS);

exit:
  DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_rates_fixed_len_cal_build(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                   *port_nof_slots,
    DNX_SAND_IN  uint32                   nof_ports,
    DNX_SAND_IN  uint32                    calendar_len,
    DNX_SAND_IN  uint32                    max_calendar_len,
    DNX_SAND_IN  uint32                    is_fqp_pqp,
    DNX_SAND_OUT uint32                   *calendar
  )
{
  uint32
    slot_idx,
    port_idx,
    nof_diff_ports,
    port_num_slots[JER2_ARAD_NOF_FAP_PORTS],
    alloc_slots,
    rem_cal_len,
    max_port_idx,
    hop_size,
    port_alloc_slots,
    free_slot_cnt,
    leftovers[JER2_ARAD_NOF_FAP_PORTS];

  DNXC_INIT_FUNC_DEFS

  /*
   *  Verify the input's integrity
   */
  alloc_slots = 0;
  for (port_idx = 0; port_idx < nof_ports; ++port_idx)
  {
    alloc_slots += port_nof_slots[port_idx];
  }
  if (alloc_slots != calendar_len)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal calendar len")));
  }

  /*
   * Clear the calendar
   */
  for (slot_idx = 0; slot_idx < max_calendar_len; ++slot_idx)
  {
    calendar[slot_idx] = JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID;
  }

  /*
   *  Count the number of distinct ports in the calendar and initialize the leftovers array
   */
  nof_diff_ports = 0;
  for (port_idx = 0; port_idx < nof_ports; ++port_idx)
  {
    if (port_nof_slots[port_idx] > 0)
    {
      ++nof_diff_ports;
    }
    leftovers[port_idx] = 0;
  }

  SOC_COPY(port_num_slots, port_nof_slots, uint32, JER2_ARAD_NOF_FAP_PORTS);

  /*
   *  First pass: for each port, try to allocate as many evenly-spaced slots as possible
   */
  alloc_slots = 0;
  rem_cal_len = calendar_len;
  max_port_idx = dnx_sand_get_index_of_max_member_in_array(
                   port_num_slots,
                   nof_ports
                 );
  while (port_num_slots[max_port_idx] > 0)
  {
    hop_size = DNX_SAND_MIN(rem_cal_len / port_num_slots[max_port_idx], nof_diff_ports);
    hop_size -= is_fqp_pqp ? 1 : 0;
    port_alloc_slots = 0;
    free_slot_cnt = hop_size;
    for (slot_idx = 0; slot_idx < calendar_len && port_alloc_slots < port_nof_slots[max_port_idx]; ++slot_idx)
    {
      if (calendar[slot_idx] == JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID)
      {
        if (free_slot_cnt < hop_size)
        {
          ++free_slot_cnt;
        }
        else
        {
          calendar[slot_idx] = max_port_idx;
          ++alloc_slots;
          ++port_alloc_slots;
          free_slot_cnt = 0;
        }
      }
    }

    port_num_slots[max_port_idx] = 0;
    rem_cal_len -= port_alloc_slots;

    /*
     *  Record the number of unallocated slots for the second pass
     */
    leftovers[max_port_idx] = port_nof_slots[max_port_idx] - port_alloc_slots;

    max_port_idx = dnx_sand_get_index_of_max_member_in_array(
                     port_num_slots,
                     nof_ports
                   );
  }

  /*
   *  Second pass: fill the holes in the calendar with the remaining slots in a round-robin
   *  fashion
   */
  port_idx = 0;
  free_slot_cnt = nof_diff_ports;
  for (slot_idx = 0; alloc_slots < calendar_len; slot_idx = (slot_idx + 1) % calendar_len)
  {
    if (calendar[slot_idx] == JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID)
    {
      if (free_slot_cnt == nof_diff_ports)
      {
        for ( ; leftovers[port_idx] == 0; port_idx = (port_idx + 1) % nof_ports);
        calendar[slot_idx] = port_idx;
        --leftovers[port_idx];
        port_idx = (port_idx + 1) % nof_ports;
        ++alloc_slots;
        free_slot_cnt = 0;
      }
      else
      {
        ++free_slot_cnt;
      }
    }
  }

exit:
  DNXC_FUNC_RETURN
}
/* JER2_ARAD calender algorithm } */

static uint32
  jer2_arad_ofp_rates_active_calendars_retrieve_sch(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     sch_offset,
    DNX_SAND_IN  int                        core,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_SCH     *sch_cal,
    DNX_SAND_OUT uint32                     *sch_cal_len
  )
{
    uint32
        res,
        data,
        fld_val = 0,
        sch_len = 0;
    uint32
        sch_to_get;

    DNXC_INIT_FUNC_DEFS;

    *sch_cal_len = 0;

    /* Retreieve SCH channelize arbiter data */
    res = READ_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm(unit, SCH_BLOCK(unit, core), sch_offset, &data);
    DNXC_IF_ERR_EXIT(res);

    /* Check which calendars (EGQ & SCH - Calendars get 'A' or 'B') are currently active. */
    fld_val = soc_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm_field32_get(unit, &data, DVS_CALENDAR_SEL_CH_NI_FXXf);
    sch_to_get = fld_val;
  
    /* Calendar Length */
    sch_len = soc_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm_field32_get(unit, &data, sch_to_get == 0 ? CAL_A_LENf : CAL_B_LENf);
    
    /* The device calendar length is the actual val minus 1 */
    *sch_cal_len = sch_len + 1;

    /* Read the -Active SCH calendar indirectly */
    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_cal_tbl_get, (unit, core, sch_offset, sch_to_get, *sch_cal_len, sch_cal->slots));
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_from_rates_to_calendar(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  uint32                    *ports_rates,
    DNX_SAND_IN  uint32                    nof_ports,
    DNX_SAND_IN  uint32                    total_credit_bandwidth,
    DNX_SAND_IN  uint32                    max_calendar_len,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_SCH    *calendar,
    DNX_SAND_OUT uint32                    *calendar_len
  )
{
  uint32
    res = SOC_E_NONE;
  uint32
    port_nof_slots[JER2_ARAD_NOF_FAP_PORTS];

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_ofp_rates_cal_len_calculate(
          unit,
          ports_rates,
          nof_ports,
          total_credit_bandwidth,
          max_calendar_len,
          calendar_len,
          port_nof_slots
        );
  DNXC_IF_ERR_EXIT(res);

  /*
   * Given the optimal calendar length and the
   * corresponding weight (in slots) of each port,
   * build a calendar that will avoid burstiness
   * behavior as much as possible.
   */
  res = jer2_arad_ofp_rates_fixed_len_cal_build(
          unit,
          port_nof_slots,
          nof_ports,
          *calendar_len,
          max_calendar_len,
          0,
          calendar->slots
        );
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

static uint32
  jer2_arad_ofp_rates_fill_shaper_generic_calendar_credits(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO  *cal_info,
    DNX_SAND_IN  uint32                     *ports_rates,
    DNX_SAND_IN  uint32                    nof_ports,
    DNX_SAND_IN  uint32                     calendar_len, /* Actual Calendar length */
    DNX_SAND_IN  uint8                      add_dummy_tail, /* Indicate if last entry is dummy or not */
    DNX_SAND_IN  uint32                    *slots_ports,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_EGQ       *calendar
  )
{
  uint32
    slot_idx = 0,
    port_idx = 0,
    egq_resolution,
    scm_cr_to_add_nof_bits,
    port_in_slot;
  uint32
    port_num_slots[JER2_ARAD_NOF_FAP_PORTS],
    port_credits[JER2_ARAD_NOF_FAP_PORTS];
  DNX_SAND_U64
    u64_1,
    u64_2;
  uint32
    rem;
  uint32
    temp_calendar_length,
    dummy_tail_entry,
    calcal_length,
    calcal_instances;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(ports_rates);
  DNXC_NULL_CHECK(slots_ports);
  DNXC_NULL_CHECK(calendar);

  SOC_CLEAR(port_num_slots, uint32, JER2_ARAD_NOF_FAP_PORTS);
  SOC_CLEAR(port_credits, uint32, JER2_ARAD_NOF_FAP_PORTS);

  dummy_tail_entry = DNX_SAND_BOOL2NUM(add_dummy_tail);
  temp_calendar_length = calendar_len - dummy_tail_entry; /* In case of dummy tail, all slots are taken into account without it */
  
  /* in order to validate the configured credits to add value, determine the field length in bits */
  if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB) { /* ports calendar */
      scm_cr_to_add_nof_bits = JER2_ARAD_EGQ_NIF_MAL_SCM_TBL_ENTRY_PORT_CR_TO_ADD_NOF_BITS;
  } else { /* TCG or PTC calendar */
      scm_cr_to_add_nof_bits = SOC_DNX_DEFS_GET(unit, scm_qp_tcg_cr_to_add_nof_bits);
  }

  /*
   * Fill the calendar slots with the ports_rates information
   */
  for (slot_idx = 0; slot_idx < temp_calendar_length; ++slot_idx)
  {
    port_in_slot = slots_ports[slot_idx];
    calendar->slots[slot_idx].base_q_pair = port_in_slot;

    if (port_in_slot == JER2_ARAD_OFP_RATES_UNMAP_BASE_Q_PAIR)
    {
      continue;
    }

    if (port_in_slot == JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID)
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Invalid port")));
    }

    ++(port_num_slots[port_in_slot]);
  }

 /*
  * Chanelized Arbiter Port Credits:
  * 
  *                                     port_credits_num * 8/256 [bits] * core_frequency [kilo 1/sec]
  *  port_egq_rate for [kbits/sec]=  ----------------------------------------------------------------
  *                                            calendar_length * slot_traverse_time
  * 
  * And Therefore:
  *
  *                         port_egq_rate [kbits/sec] * calendar_length * slot_traverse_time
  *  port_credits_num   = ---------------------------------------------------------------------
  *                          8/256 [bits] * core_frequency [kilo 1/sec]
  * And Therefore:
  *
  *                         256/8 [1/bits] * port_egq_rate [kbits/sec] * calendar_length * slot_traverse_time
  *  port_credits_num   = ---------------------------------------------------------------------
  *                           core_frequency [kilo 1/sec]
  * 
  * 
  * 
  * TCG/Q-Pair Port Credits:
  * 
  *                                     port_credits_num * 8/128 [bits] * core_frequency [kilo 1/sec]
  *  port_egq_rate for [kbits/sec]=  ----------------------------------------------------------------
  *                                            calendar_length * slot_traverse_time
  * 
  * 
  *                            port_egq_rate [kbits/sec] * calendar_length * slot_traverse_time
  *  port_credits_num   = ---------------------------------------------------------------------
  *                          8/128 [bits] * core_frequency [kilo 1/sec]
  * And Therefore:
  *
  *                         128/8 [1/bits] * port_egq_rate [kbits/sec] * calendar_length * slot_traverse_time
  *  port_credits_num   = ---------------------------------------------------------------------
  *                           core_frequency [kilo 1/sec]
  * 
  * 
  */

  if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)
  {
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.calcal_length.get(unit,
        core, &calcal_length));
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chan_arb.nof_calcal_instances.get(unit,
        core, cal_info->chan_arb_id, &calcal_instances));
  }
  else
  {
    /* When no calcal to be taken into consideration. assume lengths are 1 */
    calcal_length = 1;
    calcal_instances = 1;
  }
  
  for(port_idx = 0; port_idx < JER2_ARAD_NOF_FAP_PORTS; ++port_idx)
  {
    if(!ports_rates[port_idx])
    {
      continue;
    }

    if(port_num_slots[port_idx] == 0)
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "No slots port")));
    }

    if(SOC_IS_ARAD_A0(unit) || (cal_info->cal_type==JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)) {
        /*If callender type is chanelized arbiter EGQ resolution is 1/256 */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_CHAN_ARB;
    } else {
        /*If callender type is QPAIR or TCG, EGQ resolution is  1/128 (not supported for A0) */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG;
    }

    dnx_sand_u64_multiply_longs(ports_rates[port_idx], calcal_length * (calendar_len) * JER2_ARAD_EGQ_CAL_SLOT_TRAVERSE_IN_CLOCKS * egq_resolution, &u64_1);

    rem = dnx_sand_u64_devide_u64_long(&u64_1, calcal_instances * jer2_arad_chip_kilo_ticks_per_sec_get(unit), &u64_2);
    dnx_sand_u64_to_long(&u64_2, &(port_credits[port_idx]));
    port_credits[port_idx] = (rem > 0 ? port_credits[port_idx] + 1 : port_credits[port_idx]);
  }

  for (slot_idx = 0; slot_idx < temp_calendar_length; ++slot_idx)
  {
    port_idx = slots_ports[slot_idx];
    if (port_idx == JER2_ARAD_OFP_RATES_UNMAP_BASE_Q_PAIR)
    {
      calendar->slots[slot_idx].credit = 0;
    }
    else if(port_num_slots[port_idx] == 0)
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "No slots port")));
    }
    else if(port_num_slots[port_idx] == 1)
    {
      calendar->slots[slot_idx].credit = port_credits[port_idx];
      --port_num_slots[port_idx];
    }
    else
    {
      calendar->slots[slot_idx].credit = port_credits[port_idx] / port_num_slots[port_idx];

      port_credits[port_idx] -= calendar->slots[slot_idx].credit;
      --port_num_slots[port_idx];
    }
    if(calendar->slots[slot_idx].credit > (1 << scm_cr_to_add_nof_bits))
    {
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_LIMIT);
    }
  }

exit:
  DNXC_FUNC_RETURN
}

/* 
 * This function returns register and field name from 
 * given channelize arbiter enum. 
 */
int
  jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_SET                   cal2set,    
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE   field_type,
    DNX_SAND_OUT soc_reg_t                                *register_name,
    DNX_SAND_OUT soc_field_t                              *field_name
  )
{  

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(register_name);
  DNXC_NULL_CHECK(field_name);
  DNXC_NULL_CHECK(cal_info);

  if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)
  {
    switch (cal_info->chan_arb_id)
    {
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_00:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_0r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_0r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_0_SPR_RATE_Af:CH_0_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_0_SPR_MAX_BURST_Af:CH_0_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_0_SPR_CAL_LEN_Af:CH_0_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_01:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_1r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_1r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_1_SPR_RATE_Af:CH_1_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_1_SPR_MAX_BURST_Af:CH_1_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_1_SPR_CAL_LEN_Af:CH_1_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_02:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_2r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_2r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_2_SPR_RATE_Af:CH_2_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_2_SPR_MAX_BURST_Af:CH_2_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_2_SPR_CAL_LEN_Af:CH_2_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_03:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_3r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_3r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_3_SPR_RATE_Af:CH_3_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_3_SPR_MAX_BURST_Af:CH_3_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_3_SPR_CAL_LEN_Af:CH_3_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_04:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_4r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_4r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_4_SPR_RATE_Af:CH_4_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_4_SPR_MAX_BURST_Af:CH_4_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_4_SPR_CAL_LEN_Af:CH_4_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_05:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_5r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_5r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_5_SPR_RATE_Af:CH_5_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_5_SPR_MAX_BURST_Af:CH_5_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_5_SPR_CAL_LEN_Af:CH_5_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_06:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_6r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_6r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_6_SPR_RATE_Af:CH_6_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_6_SPR_MAX_BURST_Af:CH_6_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_6_SPR_CAL_LEN_Af:CH_6_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_07:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_7r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_7r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_7_SPR_RATE_Af:CH_7_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_7_SPR_MAX_BURST_Af:CH_7_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_7_SPR_CAL_LEN_Af:CH_7_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_CPU:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_8r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_8r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_8_SPR_RATE_Af:CH_8_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_8_SPR_MAX_BURST_Af:CH_8_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_8_SPR_CAL_LEN_Af:CH_8_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_RCY:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_CHAN_INTERFACE_9r : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_CHAN_INTERFACE_9r;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_9_SPR_RATE_Af:CH_9_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_9_SPR_MAX_BURST_Af:CH_9_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? CH_9_SPR_CAL_LEN_Af:CH_9_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_NON_CHAN:
      *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_NON_CHAN_INTERFACESr : 
        EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_NON_CHAN_INTERFACESr;
      switch (field_type)
      {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? NONCH_SPR_RATE_Af:NONCH_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? NONCH_SPR_MAX_BURST_Af:NONCH_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? NONCH_SPR_CAL_LEN_Af:NONCH_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
      }
      break;
    default:
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
    }
  } 
  else if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY)
  {
    *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_QPAIRr:
      EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_QPAIRr;
    switch (field_type)
    {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? QPAIR_SPR_RATE_Af:QPAIR_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? QPAIR_SPR_MAX_BURST_Af:QPAIR_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? QPAIR_SPR_CAL_LEN_Af:QPAIR_SPR_CAL_LEN_Bf;      
        break;
      default:
       DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
    }   
  } 
  else if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_TCG)
  {
    *register_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? EGQ_EGRESS_SHAPER_CONFIGURATION_A_FOR_TCGr:
      EGQ_EGRESS_SHAPER_CONFIGURATION_B_FOR_TCGr;
    switch (field_type)
    {
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? TCG_SPR_RATE_Af:TCG_SPR_RATE_Bf;
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? TCG_SPR_MAX_BURST_Af:TCG_SPR_MAX_BURST_Bf;      
        break;
      case DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN:
        *field_name = (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A) ? TCG_SPR_CAL_LEN_Af:TCG_SPR_CAL_LEN_Bf;      
        break;
      default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
    }
  } 
  else 
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
  }

exit:
  DNXC_FUNC_RETURN
}
  
/*
 *  This functions reads the calculated calendar values from the device
 *  It also reads per-port maximal burst configuration (EGQ).
 *  Note: rates_table is only used to get the per-port shaper (max burst)
 */
static uint32
  jer2_arad_ofp_rates_active_generic_calendars_retrieve_egq(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO      *cal_info,
    DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_EGQ       *egq_cal,
    DNX_SAND_OUT uint32                     *egq_cal_len,
    DNX_SAND_IN  uint8                    remove_dummies
  )
{
  uint32
    res,
    fld_val = 0,
    temp_egq_cal_len,
    reg_val,
    egq_len = 0;
  uint32
    offset = 0,
    slot = 0;
  JER2_ARAD_EGQ_SCM_TBL_DATA
    egq_data;
  uint32
    cal2get;
  JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY
    *cal_slot = NULL;
  soc_field_t
    field_cal_set;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(cal_info);
  DNXC_NULL_CHECK(egq_cal);
  DNXC_NULL_CHECK(egq_cal_len);

  *egq_cal_len = 0;

  res = jer2_arad_ofp_rates_retrieve_egress_shaper_setting_field(
          unit,
          cal_info,
          &field_cal_set
        );
  DNXC_IF_ERR_EXIT(res);

  DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
  fld_val = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, field_cal_set);
  cal2get = fld_val;

  /*
   * Read the Active EGQ calendar indirectly
   */  

  /* read calendar length */
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egress_shaper_cal_read, (unit, core, cal_info, cal2get, 
                                                                                DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN, &egq_len));
  DNXC_IF_ERR_EXIT(res);

  /*
   * EGQ tables are double in size -
   * the second half is for get 'B' calendars/shapers.
   */
  /* 
   * Calculate offset (depends on calender length and cal2get)
   */
  offset = JER2_ARAD_OFP_RATES_EGQ_SCM_OFFSET_GET(unit, cal_info,cal2get);
  for (slot = 0; slot < egq_len + 1; ++slot)
  {
    cal_slot = egq_cal->slots + slot;

    res = jer2_arad_egq_scm_tbl_get_unsafe(
            unit,
            core,
            cal_info,
            offset + slot,
            &egq_data
          );
    DNXC_SAND_IF_ERR_EXIT(res);
    
    cal_slot->credit = egq_data.port_cr_to_add;
    cal_slot->base_q_pair = egq_data.ofp_index;
  }

  /*
   * The device calendar length is the actual val plus 1
   */
  temp_egq_cal_len = egq_len + 1;

  /*
   * If the last dummy entry should be removed decrease cal_len by 1.
   * cal_slot holds the last entry.
   */
  if (remove_dummies)
  {
    for (slot = 0; slot < temp_egq_cal_len; ++slot)
    {
      cal_slot = egq_cal->slots + slot;
      if (jer2_arad_ofp_rates_is_cal_entry_dummy(cal_slot))
      {
        SOC_COPY(egq_cal->slots + slot, egq_cal->slots + slot + 1, JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY, (temp_egq_cal_len - slot - 1));
        temp_egq_cal_len -= 1;
      }
    }
  }

  *egq_cal_len = temp_egq_cal_len;

exit:
  DNXC_FUNC_RETURN
}

int 
jer2_arad_ofp_rates_egress_shaper_reg_field_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_reg_t                                register_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   *data
    )
{
    uint64 field64;

    DNXC_INIT_FUNC_DEFS

    SOC_FIELD64_GET(register_name, field_name, field64);
    *data = COMPILER_64_LO(field64);

exit:
    DNXC_FUNC_RETURN
}

int 
jer2_arad_ofp_rates_egress_shaper_reg_field_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO                  *cal_info,   
    DNX_SAND_IN  soc_reg_t                                register_name,
    DNX_SAND_IN  soc_field_t                              field_name,
    DNX_SAND_OUT uint32                                   data
    )
{
    uint64 field64;

    DNXC_INIT_FUNC_DEFS

    COMPILER_64_SET(field64,0,data);
    SOC_FIELD64_SET(register_name, field_name, field64);

exit:
    DNXC_FUNC_RETURN
}

static
  uint32
    jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar_chn_arb_aux(
      DNX_SAND_IN  int                       unit,   
      DNX_SAND_IN  uint32                     *ports_rates,
      DNX_SAND_IN  uint32                    nof_ports,
      DNX_SAND_IN  uint32                     total_shaper_bandwidth,
      DNX_SAND_IN  uint32                     max_calendar_len,
      DNX_SAND_OUT uint32                   *loc_calendar_len,
      DNX_SAND_OUT uint32                   *port_nof_slots,
      DNX_SAND_OUT uint32                   *slots_ports
    )
{
    uint32 res = SOC_E_NONE,
           idx,
           add_dummy_tail = 0,
           nof_dup_entries,
           multiplier = 1,
           last_port_entry = JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID,
           act_nof_ports = 0;

    DNXC_INIT_FUNC_DEFS
    /* Dummy tail is still needed for JER2_ARAD and below */
    if (SOC_IS_ARADPLUS_AND_BELOW(unit))
    {
        add_dummy_tail = 1;
    }

    /* Find the real number of ports (ports that have rate greater than zero) */
    for (idx = 0; idx < nof_ports; ++idx)
    {
        act_nof_ports += (ports_rates[idx] > 0 ? 1 : 0);
    }

    do
    {
        nof_dup_entries = 0;

        /* Minimum calendar length must be greater than the number of ports */
        if (max_calendar_len / multiplier < act_nof_ports)
        {
          DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "NOF ports greater than max calendar len")));
        }

        /*
         * Calculate the optimal calendar length and the
         * corresponding weight (in slots) of each port.
         */
        res = jer2_arad_ofp_rates_cal_len_calculate(
                unit,
                ports_rates,
                nof_ports,
                total_shaper_bandwidth,
                max_calendar_len / multiplier - add_dummy_tail,
                loc_calendar_len,
                port_nof_slots
              );
        DNXC_IF_ERR_EXIT(res);

        /*
         * Given the optimal calendar length and the
         * corresponding weight (in slots) of each port,
         * build a calendar that will avoid burstiness
         * behavior as much as possible.
         */
        res = jer2_arad_ofp_rates_fixed_len_cal_build(
                unit,
                port_nof_slots,
                nof_ports,
                *loc_calendar_len,
                max_calendar_len / multiplier - add_dummy_tail,
                0,
                slots_ports
              );
        DNXC_IF_ERR_EXIT(res);

        /*
         * For this calendar length calculate the number of
         * duplicate entries in the calendar
         */
        for (idx = 0; idx < *loc_calendar_len; ++idx)
        {
          nof_dup_entries += (last_port_entry == slots_ports[idx] ? 1 : 0);
          last_port_entry = slots_ports[idx];
        }

        if ((multiplier *= 2) > JER2_ARAD_OFP_RATES_NOF_ITERATIONS)
        {
          DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Failed to calculate calendar")));
        }

    } while(*loc_calendar_len + nof_dup_entries + add_dummy_tail > max_calendar_len);

exit:
  DNXC_FUNC_RETURN
}


static
  uint32
    jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar_ptc_tcg_aux(
      DNX_SAND_IN  int                       unit,   
      DNX_SAND_IN  uint32                     *ports_rates,
      DNX_SAND_IN  uint32                    nof_ports,
      DNX_SAND_IN  uint32                     max_calendar_len,
      DNX_SAND_OUT uint32                   *loc_calendar_len,
      DNX_SAND_OUT uint32                   *port_nof_slots,
      DNX_SAND_OUT uint32                   *slots_ports
    )
{
    uint32 res = SOC_E_NONE,
           idx,
           nof_dup_entries = 0,
           last_port_entry = JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID,
           act_nof_ports = 0;
    uint8  slots_added;

    DNXC_INIT_FUNC_DEFS
      
      /*prepare trivial setup - one slot per port*/
    for (idx = 0; idx < nof_ports; ++idx)
    {
        if (ports_rates[idx] > 0)
        {
          act_nof_ports++;
          port_nof_slots[idx] = 1;
        }        
    }

    *loc_calendar_len = act_nof_ports;

    if (max_calendar_len < act_nof_ports)
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "NOF ports greater than max_calendar_len")));
    }
    do {
      /* Repeat until there are no more improvements or an error is received:
       * Increase a port's number of slots if expected to be more than 16 bits credits per slot 
       */
      slots_added = FALSE;
      res = jer2_arad_ofp_rates_cal_improve_nof_slots(
            unit,
            ports_rates,
            nof_ports,
            max_calendar_len,
            loc_calendar_len,
            port_nof_slots,
            &slots_added
          );      
      DNXC_IF_ERR_EXIT(res);
    } while (slots_added);

    if (act_nof_ports == 0)
    {
        port_nof_slots[0] = 1;
        act_nof_ports = 1;
        *loc_calendar_len = 1;
    }

    /*
    * Given calendar length and the
    * corresponding weight (in slots) of each port,
    * build a calendar that will avoid burstiness
    * behavior as much as possible.
    */
    res = jer2_arad_ofp_rates_fixed_len_cal_build(
            unit,
            port_nof_slots,
            nof_ports,
            *loc_calendar_len,
            max_calendar_len, 
            0,
            slots_ports
          );
    DNXC_IF_ERR_EXIT(res);

    for (idx = 0; idx < *loc_calendar_len; ++idx)
    {
        nof_dup_entries += (last_port_entry == slots_ports[idx] ? 1 : 0);
        last_port_entry = slots_ports[idx];
    }
    /*if we exceed beyond max_calendar_len return an error*/
    if (*loc_calendar_len + nof_dup_entries > max_calendar_len)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Failed to calculate calendar")));
    }

exit:
  DNXC_FUNC_RETURN
}

static
  uint32
    jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar(
      DNX_SAND_IN  int                       unit,
      DNX_SAND_IN  int                       core,
      DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO  *cal_info,   
      DNX_SAND_IN  uint32                     *ports_rates,
      DNX_SAND_IN  uint32                    nof_ports,
      DNX_SAND_IN  uint32                     total_shaper_bandwidth,
      DNX_SAND_IN  uint32                     max_calendar_len,
      DNX_SAND_IN  uint32                     recalc,
      DNX_SAND_IN  uint8                    add_dummy_tail,
      DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_EGQ       *calendar,
      DNX_SAND_OUT uint32                     *calendar_len
    )
{
  uint32
    res = SOC_E_NONE;
  uint32
    idx = 0;
  uint32
    loc_calendar_len;
  uint32
    *port_nof_slots = NULL,
    *temp_buff = NULL,
    *slots_ports = NULL;
  uint32
    last_port_entry;

  DNXC_INIT_FUNC_DEFS;

  DNXC_ALLOC(port_nof_slots, uint32, JER2_ARAD_NOF_FAP_PORTS, "port_nof_slots");
  DNXC_ALLOC(slots_ports, uint32, JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX, "slots_ports");
  DNXC_ALLOC(temp_buff, uint32, JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX, "temp_buff");
  
  if (recalc) {
      /* Recalculation of CHAN ARB calendar tries to find the best calendar */
        if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB) {
            res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar_chn_arb_aux(unit,
                                                                                     ports_rates,
                                                                                     nof_ports,
                                                                                     total_shaper_bandwidth,
                                                                                     max_calendar_len,
                                                                                     &loc_calendar_len,
                                                                                     port_nof_slots,
                                                                                     slots_ports);
            DNXC_IF_ERR_EXIT(res);
        }
        else { /* Recalculation of TCG and QPair based on a fixed calendar length (num_of_ports) where each port has one slot */
            res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar_ptc_tcg_aux(unit,
                                                                                      ports_rates,
                                                                                      nof_ports,
                                                                                      max_calendar_len,
                                                                                      &loc_calendar_len,
                                                                                       port_nof_slots,
                                                                                      slots_ports);
            DNXC_IF_ERR_EXIT(res);
        }
  
        last_port_entry = JER2_ARAD_OFP_RATES_ILLEGAL_SCHEDULER_ID;
  
        for (idx = 0; idx < loc_calendar_len; ++idx)
        {
            if (last_port_entry == slots_ports[idx])
            {
                if (loc_calendar_len > JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX) {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "loc_calendar_len is above max")));
                }

                /* Space the calendar, inserting dummies between duplicate entries */
                SOC_COPY(temp_buff, slots_ports, uint32, JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX);
                SOC_COPY(temp_buff + idx + 1, slots_ports + idx, uint32, loc_calendar_len - idx);
                SOC_COPY(slots_ports, temp_buff, uint32, JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX);
                slots_ports[idx] = JER2_ARAD_OFP_RATES_DUMMY_PORT_ID;

                /* Increase the calendar length by 1 for each dummy inserted */
                loc_calendar_len += 1;
            }
            last_port_entry = slots_ports[idx];
        }
        /* Insert dummy tail at the end of each calendar */
        if (add_dummy_tail)
        {
            loc_calendar_len += 1;
        }

        if (loc_calendar_len > JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_MAX) {
              DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "loc_calendar_len is above max")));
        }
  } else {
      res = jer2_arad_ofp_rates_active_generic_calendars_retrieve_egq(
            unit,
            core,
            cal_info,
            calendar,
            &loc_calendar_len,
            FALSE
          );
      DNXC_IF_ERR_EXIT(res);

      for (idx = 0; idx < loc_calendar_len; ++idx)
      {
          slots_ports[idx] = calendar->slots[idx].base_q_pair;
      }
  }

  /* Fill calendar rates as if calendar length was bigger by 1 (dummy tail) */
  res = jer2_arad_ofp_rates_fill_shaper_generic_calendar_credits(
          unit,
          core,
          cal_info,
          ports_rates,
          nof_ports,
          loc_calendar_len,/* ((add_dummy_tail) ? (loc_calendar_len - 1):loc_calendar_len), */
          add_dummy_tail,
          slots_ports,
          calendar
        );
  /* Check if we fail because small calendar wasn't enough, if so try again with big cal*/
  if(res == SOC_E_LIMIT)
  {
    DNXC_EXIT_WITH_ERR_NO_MSG(res);
  }
  DNXC_IF_ERR_EXIT(res);

  if (add_dummy_tail)
  {
      /* Add the dummy tail */
      calendar->slots[loc_calendar_len - 1].base_q_pair = JER2_ARAD_OFP_RATES_DUMMY_PORT_ID;
      calendar->slots[loc_calendar_len - 1].credit = 0;    
  }
    *calendar_len = loc_calendar_len;

exit:
  DNXC_FREE(port_nof_slots);
  DNXC_FREE(slots_ports);
  DNXC_FREE(temp_buff);

  DNXC_FUNC_RETURN
}

/*to improve function efficiency can ask results for single base_q_pair (requested_base_q_pair), 
use JER2_ARAD_EGR_INVALID_BASE_Q_PAIR to calc all*/
static uint32
  jer2_arad_ofp_rates_from_calendar_to_ports_sch_rate(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_SCH    *calendar,
    DNX_SAND_IN  uint32                    calendar_len,
    DNX_SAND_IN  uint32                    total_sch_rate,
    DNX_SAND_IN  uint32                    nof_ports,
    DNX_SAND_IN  int                       requested_base_q_pair, 
    DNX_SAND_OUT uint32                    *ports_rates
  )
{
    uint32
        port,
        sched,
        calc,
        slot_id;
    DNX_SAND_U64
        u64_1,
        u64_2;
    uint32
        rem;

    DNXC_INIT_FUNC_DEFS

    if (0 == calendar_len)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Failed to calculate calendar")));
    }

    for (port = 0; port < nof_ports; ++port)
    {
        ports_rates[port] = 0;
    }

    if ((calendar_len == 1) && (calendar->slots[0] == JER2_ARAD_FAP_PORT_ID_INVALID))
    {
        JER2_ARAD_DO_NOTHING_AND_EXIT;
    }

    for (slot_id = 0; slot_id < calendar_len; ++slot_id)
    {
        sched = calendar->slots[slot_id];
        if (sched >= nof_ports)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Invalid port")));
        }

        /* Increase the number of calendar slots of this port by 1 */
        ++ports_rates[sched];
    }

    /*
     *  Calculate credit rate for each port according to:
     *
     *                                 port_num_of_cal_slots * total_sch_rate[kBits/sec]
     *  port_sch_rate [kbits/sec]= -----------------------------------------------------
     *                                                cal_num_of_slots
     *
     */
    if(requested_base_q_pair != JER2_ARAD_EGR_INVALID_BASE_Q_PAIR) {
        dnx_sand_u64_multiply_longs(ports_rates[requested_base_q_pair], total_sch_rate, &u64_1);
        rem = dnx_sand_u64_devide_u64_long(&u64_1, calendar_len, &u64_2);
        dnx_sand_u64_to_long(&u64_2, &(calc));
        calc = (rem > 0 ? calc + 1 : calc);

        ports_rates[requested_base_q_pair] = calc;
    } else {
        for (port = 0; port < nof_ports; ++port)
        {
            dnx_sand_u64_multiply_longs(ports_rates[port], total_sch_rate, &u64_1);
            rem = dnx_sand_u64_devide_u64_long(&u64_1, calendar_len, &u64_2);
            dnx_sand_u64_to_long(&u64_2, &(calc));
            calc = (rem > 0 ? calc + 1 : calc);

            ports_rates[port] = calc;
        }
    }

exit:
    DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_from_generic_calendar_to_ports_egq_rate(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO     *cal_info,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_EGQ      *calendar,
    DNX_SAND_IN  uint32                     calendar_len,
    DNX_SAND_IN  uint8                    dummy_entry_supported,
    DNX_SAND_IN  uint32                    nof_ports,
    DNX_SAND_OUT uint32                     *ports_rates
  )
{
  uint32
    port_idx,
    calc,
    slot_id,
    egq_resolution;
  const JER2_ARAD_OFP_EGQ_RATES_CAL_ENTRY
    *slot;
  DNX_SAND_U64
    u64_1,
    u64_2;
  uint32
    rem;
  uint32
    tmp_calendar_len,
    calcal_length,
    calcal_instances;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(cal_info);
  DNXC_NULL_CHECK(calendar);

  if (0 == calendar_len)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal calendar len")));
  }

  for (port_idx = 0; port_idx < nof_ports; ++port_idx)
  {
    ports_rates[port_idx] = 0;
  }

  /* Last entry is dummy. Traverse until calendar_len-1 */
  tmp_calendar_len = (dummy_entry_supported) ? calendar_len-1:calendar_len;

  for (slot_id = 0; slot_id < tmp_calendar_len; ++slot_id)
  {
    slot = &calendar->slots[slot_id];

    if (dummy_entry_supported && slot->base_q_pair == JER2_ARAD_OFP_RATES_DUMMY_PORT_ID)
    {
      continue;
    }
    if (slot->base_q_pair >= nof_ports)
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal base_q_pair")));
    }
    /*
     * Increase the total sum of credit of this port_idx by the val of this slot
     */
    ports_rates[slot->base_q_pair] += slot->credit;
  }

  /*
   *  Calculate shaper rate for each port_idx according to:
   *
   *                                 total_port_credit [bits] * core_frequency [kilo-clocks]
   *  port_egq_rate [kbits/sec]= ---------------------------------------------------------
   *                                           calendar_length * slot_traverse_time [clocks]
   *
   */  
  if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)
  {
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.calcal_length.get(unit,
        core, &calcal_length));
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chan_arb.nof_calcal_instances.get(unit,
        core, cal_info->chan_arb_id, &calcal_instances));
  }
  else
  {
    /* there is no calcal. Assume only one entry */
    calcal_length = 1;
    calcal_instances = 1;
  }

  if(SOC_IS_ARAD_A0(unit) || (cal_info->cal_type==JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)) {
    /*If callender type is chanelized arbiter EGQ resolution is 1/256 */
    egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_CHAN_ARB;
  } else {
    /*If callender type is QPAIR or TCG, EGQ resolution is  1/128 (not supported for A0) */
    egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG;
  }

  for (port_idx = 0; port_idx < nof_ports; ++port_idx)
  {
    dnx_sand_u64_multiply_longs(calcal_instances * ports_rates[port_idx], jer2_arad_chip_kilo_ticks_per_sec_get(unit), &u64_1);
    rem = dnx_sand_u64_devide_u64_long(&u64_1, calcal_length * calendar_len * egq_resolution * JER2_ARAD_EGQ_CAL_SLOT_TRAVERSE_IN_CLOCKS, &u64_2);
    dnx_sand_u64_to_long(&u64_2, &(calc));
    calc = (rem > 0 ? calc + 1 : calc);

    ports_rates[port_idx] = calc;
  }

exit:
  DNXC_FUNC_RETURN
}


/*********************************************************************
*     Translate from kilobit-per-second
*     to shaper internal representation:
*     units of 1/256 Bytes per clock
*********************************************************************/

uint32
  jer2_arad_ofp_rates_egq_shaper_rate_to_internal(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CAL_TYPE cal_type,
    DNX_SAND_IN  uint32                      rate_kbps,
    DNX_SAND_OUT uint32                     *rate_internal
  )
{
  uint32
    device_ticks_per_sec,
    rate_int,
    egq_resolution;
  DNX_SAND_U64
    calc,
    calc2;

  DNXC_INIT_FUNC_DEFS

  device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);

  if (device_ticks_per_sec == 0)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "NOF ticks equals 0")));
  }

  if (rate_kbps == 0)
  {
    rate_int = 0;
  }
  else
  {
    if(SOC_IS_ARAD_A0(unit) || (cal_type==JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)) {
        /*If callender type is chanelized arbiter EGQ resolution is 1/256 */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_CHAN_ARB;
    } else {
        /*If callender type is QPAIR or TCG, EGQ resolution is  1/128 (not supported for A0) */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG;
    }

    dnx_sand_u64_multiply_longs(rate_kbps, (1000 * egq_resolution), &calc);

    dnx_sand_u64_devide_u64_long(&calc, device_ticks_per_sec, &calc2);

    if (dnx_sand_u64_to_long(&calc2, &rate_int))
    {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Overflow")));
    }
  }

  *rate_internal =  rate_int;

exit:
  DNXC_FUNC_RETURN
}

/*********************************************************************
*     Translate from kilobit-per-second
*     to shaper internal representation:
*     units of 1/256 Bytes per clock
*********************************************************************/
uint32
  jer2_arad_ofp_rates_egq_shaper_rate_from_internal(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CAL_TYPE cal_type,
    DNX_SAND_IN  uint32                      rate_internal,
    DNX_SAND_OUT uint32                     *rate_kbps
  )
{
  uint32
    device_ticks_per_sec,
    rate_kb,
    egq_resolution;
  DNX_SAND_U64
    calc,
    calc2;

  DNXC_INIT_FUNC_DEFS

  device_ticks_per_sec = jer2_arad_chip_ticks_per_sec_get(unit);
  if (rate_internal == 0)
  {
    rate_kb = 0;
  }
  else
  {
      dnx_sand_u64_multiply_longs(rate_internal, device_ticks_per_sec, &calc);

      if(SOC_IS_ARAD_A0(unit) || (cal_type==JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB)) {
        /*If callender type is chanelized arbiter EGQ resolution is 1/256 */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_CHAN_ARB;
      } else {
        /*If callender type is QPAIR or TCG, EGQ resolution is  1/128 (not supported for A0) */
        egq_resolution = JER2_ARAD_EGQ_UNITS_VAL_IN_BITS_QPAIR_TCG;
      }

      dnx_sand_u64_devide_u64_long(&calc, (1000 * egq_resolution), &calc2);

      if (dnx_sand_u64_to_long(&calc2, &rate_kb))
      {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Overflow")));
      }
  }

  *rate_kbps =  rate_kb;

exit:
  DNXC_FUNC_RETURN
}


/*********************************************************************
*     Configure MAL-level shaping (rate and burst) for the EGQ.
*     This is required when the
*     shaping rate is different from the accumulated rate of
*     the OFP-s mapped to the NIF.
*     Note: both calendars (active and inactive) are configured
*********************************************************************/

static uint32
  jer2_arad_ofp_rates_egq_shaper_config(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO       *cal_info,
    DNX_SAND_IN  uint32                         rate
  )
{
  uint32
    res = SOC_E_NONE;
  uint32
    fld_val = 0;
  uint64
    field64;
  soc_reg_t
    reg_shpr_name[JER2_ARAD_OFP_NOF_RATES_CAL_SETS];
  soc_field_t
    field_shpr_rate_name[JER2_ARAD_OFP_NOF_RATES_CAL_SETS],
    field_shpr_burst_name[JER2_ARAD_OFP_NOF_RATES_CAL_SETS];
  JER2_ARAD_OFP_RATES_CAL_SET
    cal2set;

  DNXC_INIT_FUNC_DEFS  

  COMPILER_64_ZERO(field64);
  /* 
   * Retreive register and fields names for egress shaper: rate and max burst
   * It is depended on chan_arb_id and cal2set parameters
   */
    for (cal2set = JER2_ARAD_OFP_RATES_CAL_SET_A; cal2set < JER2_ARAD_OFP_NOF_RATES_CAL_SETS; ++cal2set) {

        res = jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
                unit,
                cal_info,
                cal2set,
                DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE,
                &(reg_shpr_name[cal2set]),
                &(field_shpr_rate_name[cal2set])
              );
        DNXC_IF_ERR_EXIT(res);

        res = jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
                unit,
                cal_info,
                cal2set,
                DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_MAX_BURST,
                &(reg_shpr_name[cal2set]),
                &(field_shpr_burst_name[cal2set])
              );
        DNXC_IF_ERR_EXIT(res);
    }

    res = jer2_arad_ofp_rates_egq_shaper_rate_to_internal(
            unit,
            cal_info->cal_type,
            rate,
            &fld_val
          );
    DNXC_IF_ERR_EXIT(res);

    /* Set shaper rate for both calenders */

    for (cal2set = JER2_ARAD_OFP_RATES_CAL_SET_A; cal2set < JER2_ARAD_OFP_NOF_RATES_CAL_SETS; ++cal2set) {
      COMPILER_64_ZERO(field64);
      COMPILER_64_ADD_32(field64,fld_val);
      SOC_FIELD64_SET(reg_shpr_name[cal2set], field_shpr_rate_name[cal2set], field64);
      COMPILER_64_SET(field64, 0, 0x1fff);
      SOC_FIELD64_SET(reg_shpr_name[cal2set], field_shpr_burst_name[cal2set], field64);
    }

exit:
  DNXC_FUNC_RETURN
}

/*********************************************************************
*     Retrieve MAL-level shaping (rate and burst) from the EGQ.
*********************************************************************/
static uint32
  jer2_arad_ofp_rates_egq_shaper_retrieve(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID   chan_arb_id,
    DNX_SAND_IN  uint8                    rate_enable,
    DNX_SAND_OUT  uint32                    *rate
  )
{
  uint32
    res = SOC_E_NONE;
  uint32
    fld_val;  
  soc_reg_t
    reg_egress_shpr;
  soc_field_t
    field_egress_shpr_rate;
  JER2_ARAD_OFP_RATES_CAL_INFO
    cal_info;
  uint64
    field64;

  DNXC_INIT_FUNC_DEFS

  jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(&cal_info);

  cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;
  cal_info.chan_arb_id = chan_arb_id;

  res = jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
          unit,
          &cal_info,
          JER2_ARAD_OFP_RATES_CAL_SET_A,
          DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE,
          &reg_egress_shpr,
          &field_egress_shpr_rate
        );
  DNXC_IF_ERR_EXIT(res);

  if (rate_enable)
  {
    SOC_FIELD64_GET(reg_egress_shpr, field_egress_shpr_rate, field64);
    fld_val = COMPILER_64_LO(field64);

    res = jer2_arad_ofp_rates_egq_shaper_rate_from_internal(
            unit,
            cal_info.cal_type,
            fld_val,
            rate
          );
    DNXC_IF_ERR_EXIT(res);
  }

exit:
  DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_rates_max_credit_empty_port_set(
     DNX_SAND_IN int    unit,
     DNX_SAND_IN int arg
     )
{
    int    core;
    uint32 reg_val;

    DNXC_INIT_FUNC_DEFS

    if (SOC_IS_ARADPLUS(unit)) {
        SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
            /* Enabling/Disabling shapers */
            DNXC_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
            soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, EMPTY_PORT_STOP_COLLECTING_ENf, arg ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val));

            /* Setting shapers */
            DNXC_IF_ERR_EXIT(READ_EGQ_EMPTY_PORT_MAX_CREDIT_VALUEr(unit, core, &reg_val));
            soc_reg_field_set(unit, EGQ_EMPTY_PORT_MAX_CREDIT_VALUEr, &reg_val, EMPTY_PORT_MAX_CREDITf, arg);
            DNXC_IF_ERR_EXIT(WRITE_EGQ_EMPTY_PORT_MAX_CREDIT_VALUEr(unit, core, reg_val));
        }
    }

exit:
  DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_rates_max_credit_empty_port_get(
     DNX_SAND_IN int    unit,
     DNX_SAND_OUT int* arg
     )
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS

    if (SOC_IS_ARADPLUS(unit)) {
        /* Getting shapers */
        DNXC_IF_ERR_EXIT(READ_EGQ_EMPTY_PORT_MAX_CREDIT_VALUEr(unit, 0, &reg_val));
        *arg = soc_reg_field_get(unit, EGQ_EMPTY_PORT_MAX_CREDIT_VALUEr, reg_val, EMPTY_PORT_MAX_CREDITf);
    }

exit:
  DNXC_FUNC_RETURN
}


static uint32
  jer2_arad_ofp_rates_tcg_id_egq_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx,
    DNX_SAND_OUT uint8                     *is_egq_valid
  )
{
  uint32
    res;
  uint32
    nof_priorities,
    priority_i;
  JER2_ARAD_EGR_QUEUING_TCG_INFO
    tcg_port_info;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(is_egq_valid);

  *is_egq_valid = FALSE;
    
  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
  DNXC_SAND_IF_ERR_EXIT(res);

  JER2_ARAD_EGR_QUEUING_TCG_INFO_clear(&tcg_port_info);
  res = jer2_arad_egr_queuing_ofp_tcg_get_unsafe(
          unit,
          core,
          tm_port,            
          &tcg_port_info
        );
  DNXC_SAND_IF_ERR_EXIT(res);


  for (priority_i = 0; priority_i < nof_priorities; ++priority_i)
  {
    if (tcg_port_info.tcg_ndx[priority_i] == tcg_ndx)
    {
      *is_egq_valid = TRUE;
      break;
    }
  }

exit:
  DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_tcg_id_sch_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core, 
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX              tcg_ndx,
    DNX_SAND_OUT uint8                     *is_sch_valid
  )
{
  uint32 
    res;
  uint32
    priority_i,
    nof_priorities;
  JER2_ARAD_SCH_PORT_INFO
    sch_port_info;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(is_sch_valid);

  jer2_arad_JER2_ARAD_SCH_PORT_INFO_clear(&sch_port_info);  

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
  DNXC_SAND_IF_ERR_EXIT(res);

  *is_sch_valid = FALSE;

  res = jer2_arad_sch_port_sched_get_unsafe(
            unit,
            core,
            tm_port,
            &sch_port_info
          );
  DNXC_SAND_IF_ERR_EXIT(res);

  for (priority_i = 0; priority_i < nof_priorities; ++priority_i)
  {
    if (sch_port_info.tcg_ndx[priority_i] == tcg_ndx)
    {
      *is_sch_valid = TRUE;
      break;
    }
  }

exit:
  DNXC_FUNC_RETURN
}

/* Set scheduling interface shaper. */
uint32
  jer2_arad_ofp_rates_egq_chnif_shaper_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                  rate_kbps,
    DNX_SAND_IN  uint32                  max_burst
  )
{
  uint32
    res,
    rate_internal,
    rate_nof_bits,
    burst_nof_bits;
  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID
    chan_arb_id;
  soc_reg_above_64_val_t
    data_above_64;
  soc_field_t
    field_rate_name,
    field_burst_name;
  
  DNXC_INIT_FUNC_DEFS

  SOC_REG_ABOVE_64_CLEAR(data_above_64);
  
  res = jer2_arad_ofp_rates_if_id2chan_arb_get_unsafe(
          unit,
          core,
          tm_port,
          &chan_arb_id
        );
  DNXC_IF_ERR_EXIT(res);

  if (chan_arb_id == JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_NON_CHAN)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Interface is not channelized")));
  }

  if (max_burst > JER2_ARAD_OFP_RATES_CHNIF_BURST_LIMIT_MAX) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "max burst is above max")));
  }

  switch(chan_arb_id)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_00:
    field_rate_name = CH_0_RATEf;
    field_burst_name = CH_0_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_01:
    field_rate_name = CH_1_RATEf;
    field_burst_name = CH_1_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_02:
    field_rate_name = CH_2_RATEf;
    field_burst_name = CH_2_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_03:
    field_rate_name = CH_3_RATEf;
    field_burst_name = CH_3_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_04:
    field_rate_name = CH_4_RATEf;
    field_burst_name = CH_4_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_05:
    field_rate_name = CH_5_RATEf;
    field_burst_name = CH_5_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_06:
    field_rate_name = CH_6_RATEf;
    field_burst_name = CH_6_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_07:
    field_rate_name = CH_7_RATEf;
    field_burst_name = CH_7_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_08:
    field_rate_name = CH_8_RATEf;
    field_burst_name = CH_8_MAX_BURSTf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_09:
    field_rate_name = CH_9_RATEf;
    field_burst_name = CH_9_MAX_BURSTf;
    break;
  default:
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal chan_arb")));
  }

  /* interface rate */
  res = READ_EGQ_CHANNELIZED_INTERFACE_RATEr(unit, data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

  res = jer2_arad_ofp_rates_egq_shaper_rate_to_internal(
          unit,
          JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB,
          rate_kbps,
          &rate_internal
        );
  DNXC_IF_ERR_EXIT(res);

  rate_nof_bits = soc_reg_field_length(unit, EGQ_CHANNELIZED_INTERFACE_RATEr, field_rate_name);

  res = dnx_sand_bitstream_set_any_field(&rate_internal,rate_nof_bits*chan_arb_id,rate_nof_bits,data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

  res = WRITE_EGQ_CHANNELIZED_INTERFACE_RATEr(unit, data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

  /* Max burst */
  SOC_REG_ABOVE_64_CLEAR(data_above_64);
  res = READ_EGQ_CHANNELIZED_INTERFACE_MAX_BURSTr(unit, data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

  burst_nof_bits = soc_reg_field_length(unit, EGQ_CHANNELIZED_INTERFACE_MAX_BURSTr, field_burst_name);

  res = dnx_sand_bitstream_set_any_field(&max_burst,burst_nof_bits*chan_arb_id,burst_nof_bits,data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

  res = WRITE_EGQ_CHANNELIZED_INTERFACE_MAX_BURSTr(unit, data_above_64);
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN
}

/*********************************************************************
*     Update the device after the computation of the
*     calendars.
*     Details: in the H file. (search for prototype)
*********************************************************************/

static
  void
    jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(
      DNX_SAND_OUT JER2_ARAD_OFP_RATES_CAL_INFO *info
    )
{
  if (info) {
      sal_memset(info, 0x0, sizeof(JER2_ARAD_OFP_RATES_CAL_INFO)); 
  }
}


/*********************************************/
/***************STOP PORT AUX*****************/
/*********************************************/

static uint32
  jer2_arad_ofp_shapers_generic_enable(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  soc_reg_t                reg_type,
    DNX_SAND_IN  uint32                   enable
  )
{
    uint32
        result;
    uint32
        base_q_pair,
        nof_pairs,
        tm_port;
    soc_reg_above_64_val_t
        reg_256_val;
    int core;

    DNXC_INIT_FUNC_DEFS

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core)); 

    switch (reg_type) {
        case EGQ_TCG_SPR_DISr:
              /*Reading register*/
               result = READ_EGQ_TCG_SPR_DISr(unit, core, reg_256_val);
               DNXC_IF_ERR_EXIT(result);

               DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_pairs));
               DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

               if (nof_pairs != JER2_ARAD_OFP_RATES_NOF_PRIORITIES_TCG_SUPPORT) {
                   goto exit;
               }
               /*Enabling/Disabling shapers*/
               if(enable) {
                   SHR_BITCLR_RANGE(reg_256_val,base_q_pair,nof_pairs);
               } else {
                   SHR_BITSET_RANGE(reg_256_val,base_q_pair,nof_pairs);
               }

               /*Writing back to register*/
               result=WRITE_EGQ_TCG_SPR_DISr(unit, core, reg_256_val);
               DNXC_IF_ERR_EXIT(result);
               break;
               
       case EGQ_QPAIR_SPR_DISr:
       default:
               /*Reading register*/
               result = READ_EGQ_QPAIR_SPR_DISr(unit, core, reg_256_val);
               DNXC_IF_ERR_EXIT(result);

               DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port,&nof_pairs));
               DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

               /*Enabling/Disabling shapers*/
               if(enable) {
                   SHR_BITCLR_RANGE(reg_256_val,base_q_pair,nof_pairs);
               } else {
                   SHR_BITSET_RANGE(reg_256_val,base_q_pair,nof_pairs);
               }
               /*Writing back to register*/
               result=WRITE_EGQ_QPAIR_SPR_DISr(unit, core, reg_256_val);
               DNXC_IF_ERR_EXIT(result);
               break;
       }
exit:
    DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_q_pair_shapers_enable(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  uint32                   enable
  )
{
   
    int res, core;
    uint32 q_pair_shp_en_f,reg_val, tm_port;

    DNXC_INIT_FUNC_DEFS;

    if (SOC_IS_ARAD_B0_AND_ABOVE(unit)){

        res =  dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core); 
        DNXC_SAND_IF_ERR_EXIT(res);

        res=READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val);
        DNXC_SAND_IF_ERR_EXIT(res);

        q_pair_shp_en_f = soc_reg_field_get( unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, QPAIR_SPR_ENAf);
        if (q_pair_shp_en_f) {
            res= jer2_arad_ofp_shapers_generic_enable( unit,port,EGQ_QPAIR_SPR_DISr,enable);
            DNXC_SAND_IF_ERR_EXIT(res);
        }

    }
exit:
    DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_tcg_shapers_enable(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  uint32                   enable
  )
{
    int res, core;
    uint32 tcg_shp_en_f,reg_val, tm_port;

    DNXC_INIT_FUNC_DEFS
    if (SOC_IS_ARAD_B0_AND_ABOVE(unit)){

        res =  dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core); 
        DNXC_SAND_IF_ERR_EXIT(res);

        res=READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr( unit, core,&reg_val);
        DNXC_SAND_IF_ERR_EXIT(res);

        tcg_shp_en_f = soc_reg_field_get( unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, TCG_SPR_ENAf);
        if (tcg_shp_en_f) {
            res= jer2_arad_ofp_shapers_generic_enable( unit,port,EGQ_TCG_SPR_DISr,enable);
            DNXC_IF_ERR_EXIT(res);
        }
    }
exit:
    DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_otm_shapers_disable(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  uint32     queue_rates_size,
    DNX_SAND_OUT uint32*    queue_rates

  )
{
    uint32
        res,
        otm_shp_en_f,
        reg_val,
        rate_mbps,
        egq_rate,
        channels,
        base_q_pair,
        tm_port;
    int
        core;

    DNXC_INIT_FUNC_DEFS;

    if(queue_rates_size < JER2_ARAD_EGR_NOF_Q_PAIRS) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "queue_rates is too small")));
    }


    res=dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core);
    DNXC_IF_ERR_EXIT(res);

    /*Check if the otm shaper is enabled*/
    res=READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core,&reg_val);
    DNXC_SAND_IF_ERR_EXIT(res);
    otm_shp_en_f = soc_reg_field_get( unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_ENAf);

    if (otm_shp_en_f) {
        
        res = dnx_port_sw_db_interface_rate_get(unit, port, &rate_mbps);
        DNXC_SAND_IF_ERR_EXIT(res);

        res = dnx_port_sw_db_num_of_channels_get(unit, port, &channels);
        DNXC_SAND_IF_ERR_EXIT(res);


        /* Set shaper rate to max*/
        egq_rate = (rate_mbps * JER2_ARAD_RATE_1K) / channels;

        /*store previous rate in user parameter*/
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_sw_db_egq_port_rate_get(unit, core, base_q_pair, &queue_rates[base_q_pair]));

        /* Enable port shaper */
        res = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_egq_single_port_rate_sw_set,(unit, core, tm_port, egq_rate));
        DNXC_IF_ERR_EXIT(res);

        /*Write changes to hardware*/
        res = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_egq_single_port_rate_hw_set,(unit, core, tm_port));
    }
    
exit:
    DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_otm_shapers_set(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  soc_port_t port,
    DNX_SAND_IN  uint32     queue_rates_size,
    DNX_SAND_IN  uint32*    queue_rates
  )
{
    uint32
        res,
        base_q_pair,
        tm_port;
    int
        core;

    DNXC_INIT_FUNC_DEFS;
    
    if(queue_rates_size < JER2_ARAD_EGR_NOF_Q_PAIRS) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "queue_rates is too small")));
    }    

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    /*Get base q-pair*/
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
   
    /* Enable port shaper */
    res = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_egq_single_port_rate_sw_set,
         (unit,
         core,
         tm_port,
         queue_rates[base_q_pair]));
    DNXC_IF_ERR_EXIT(res);


    /*Write changes to hardware*/
    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egq_single_port_rate_hw_set,
            (unit,
            core,
            tm_port));
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}


uint32
  jer2_arad_ofp_if_shaper_disable(
    DNX_SAND_IN  int                                         unit,
    DNX_SAND_IN  soc_port_t                                  port,
    DNX_SAND_OUT uint32*                                     shpr_rate_reg_val
  )
{
    uint32
        res,
        max_rate,
        tm_port;
    int core;

    DNXC_INIT_FUNC_DEFS 
        
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));            

    /* Save current interface shaper rate */
    res = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_egq_interface_shaper_get,(unit, core, tm_port, shpr_rate_reg_val));
    DNXC_IF_ERR_EXIT(res);

    /* Get the max bandwidth of the current interface */
    res=dnx_port_sw_db_interface_rate_get(unit, port, &max_rate);
    DNXC_SAND_IF_ERR_EXIT(res);

    /* Set shaper rate to max, Mb to Kb*/
    max_rate = max_rate*JER2_ARAD_RATE_1K;

    /* Set conifguration */    
    res = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_egq_interface_shaper_set,(unit, core, tm_port, DNX_TMC_OFP_SHPR_UPDATE_MODE_OVERRIDE,max_rate));
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}


uint32
  jer2_arad_ofp_if_spr_rate_by_reg_val_set(
     DNX_SAND_IN int                   unit,
     DNX_SAND_IN uint32                port,                            
     DNX_SAND_IN uint32                reg_val
     )
{
    uint32
        res,
        tm_port;
    soc_reg_t
        reg_shpr_name[JER2_ARAD_OFP_NOF_RATES_CAL_SETS];
    soc_field_t
        field_shpr_rate_name[JER2_ARAD_OFP_NOF_RATES_CAL_SETS];
    JER2_ARAD_OFP_RATES_CAL_INFO 
        cal_info;
    JER2_ARAD_OFP_RATES_CAL_SET
        cal2set;
    int
        core;
    DNXC_INIT_FUNC_DEFS;

    /* Set calendar info attributes */
    SOC_CLEAR(&cal_info, JER2_ARAD_OFP_RATES_CAL_INFO, 1);

    res = dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core);
    DNXC_IF_ERR_EXIT(res);

    res = jer2_arad_ofp_rates_if_id2chan_arb_get_unsafe(unit, core, tm_port, &(cal_info.chan_arb_id));
    DNXC_IF_ERR_EXIT(res);

    cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;

    /* Retreive register and fields names for egress shaper: rate and max burst */

    for (cal2set = JER2_ARAD_OFP_RATES_CAL_SET_A; cal2set < JER2_ARAD_OFP_NOF_RATES_CAL_SETS; ++cal2set) {

      res = jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
              unit,
              &cal_info,
              cal2set,
              DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE,
              &(reg_shpr_name[cal2set]),
              &(field_shpr_rate_name[cal2set])
            );
      DNXC_IF_ERR_EXIT(res);
    }
     

    /* Set shaper rate for both calenders */
    for (cal2set = JER2_ARAD_OFP_RATES_CAL_SET_A; cal2set < JER2_ARAD_OFP_NOF_RATES_CAL_SETS; ++cal2set) {
        SOC_FIELD32_REG64_SET(reg_shpr_name[cal2set], field_shpr_rate_name[cal2set], reg_val);
    }

exit:
    DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_active_egq_generic_calendars_config(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO   *cal_info,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_EGQ    *egq_cal,
    DNX_SAND_IN  uint32                     egq_cal_len
  )
{
    uint32  res,
            fld_val = 0,
            egq_len = 0,
            offset = 0,
            slot = 0,
            egq_to_set,
            reg_val,
            scm_cr_to_add_nof_bits;
    JER2_ARAD_EGQ_SCM_TBL_DATA
            egq_data;
    soc_field_t
            field_cal_set_name;

  DNXC_INIT_FUNC_DEFS

  DNXC_NULL_CHECK(egq_cal);
  DNXC_NULL_CHECK(cal_info);

  /* in order to validate the configured credits to add value, determine the field length in bits */
  if (cal_info->cal_type == JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB) { /* ports calendar */
      scm_cr_to_add_nof_bits = JER2_ARAD_EGQ_NIF_MAL_SCM_TBL_ENTRY_PORT_CR_TO_ADD_NOF_BITS;
  } else { /* TCG or PTC calendar */
      scm_cr_to_add_nof_bits = SOC_DNX_DEFS_GET(unit, scm_qp_tcg_cr_to_add_nof_bits);
  }

  /*
  *  Verify calendar length validity
  */
  if ((egq_cal_len > JER2_ARAD_OFP_RATES_EGQ_CAL_LEN_EGQ(unit, cal_info->chan_arb_id)) ||  (egq_cal_len < 1)){
  DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "egq_cal_len is out of range")));
  }


  /*
   * Check which calendars (Calendars set 'A' or 'B')
   * are currently active. Then build the non-active calendars,
   * and finally swap between the active calendars and the non-active ones.
   */

  res = jer2_arad_ofp_rates_retrieve_egress_shaper_setting_field(
          unit,
          cal_info,
          &field_cal_set_name
        );
  DNXC_IF_ERR_EXIT(res);

  DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
  fld_val = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, field_cal_set_name);
  egq_to_set = (fld_val == JER2_ARAD_OFP_RATES_CAL_SET_A)?JER2_ARAD_OFP_RATES_CAL_SET_B:JER2_ARAD_OFP_RATES_CAL_SET_A;

  /*
   * The device calendar length is the actual val minus 1
   */
  egq_len = egq_cal_len - 1;

  /* update calendar length */
  res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egress_shaper_cal_write, (unit, core, cal_info, egq_to_set, 
                                                                                DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_CAL_LEN, egq_len));
  DNXC_SAND_IF_ERR_EXIT(res);

  if(unit >= SOC_MAX_NUM_DEVICES) {
    DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG_STR( "ERROR: invalid unit")));
  }
  /*
   * Write to the non-Active EGQ calendar indirectly
   */

  /*
   * tables are double in size -
   * the second half is for set 'B' calendars/shapers.
   */
  for (slot = 0; slot < egq_cal_len; ++slot)
  {
    offset = JER2_ARAD_OFP_RATES_EGQ_SCM_OFFSET_GET(unit, cal_info,egq_to_set);
    egq_data.port_cr_to_add = egq_cal->slots[slot].credit;
    egq_data.ofp_index = egq_cal->slots[slot].base_q_pair;
    /* Check that the credit rate doesn't exceed the field's max value */
    if (egq_data.port_cr_to_add > (1 << scm_cr_to_add_nof_bits)) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "loc_calendar_len is above max")));
    }

    res = jer2_arad_egq_scm_tbl_set_unsafe(
            unit,
            core,
            cal_info,
            offset + slot,
            &egq_data
          );
    DNXC_SAND_IF_ERR_EXIT(res);
  }

exit:
  DNXC_FUNC_RETURN
}

static uint32
  jer2_arad_ofp_rates_active_sch_calendars_config(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     sch_offset,
    DNX_SAND_IN  int                        core,
                 JER2_ARAD_OFP_RATES_CAL_SCH     *sch_cal,
    DNX_SAND_IN  uint32                     sch_cal_len
  )
{
    uint32  res,
            data,
            fld_val = 0,
            sch_len = 0,
            sch_to_set;

    DNXC_INIT_FUNC_DEFS;
  
    /*  Verify calendar length validity */
    if ((sch_cal_len > JER2_ARAD_OFP_RATES_CAL_LEN_SCH_MAX) ||  (sch_cal_len < 1)){
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "sch_cal_len is out of range")));
    }
    
    /*
     * Check which calendars (SCH - Calendars set 'A' or 'B')
     * are currently active. Then build the non-active calendars,
     * and finally swap between the active calendars and the non-active ones.
     */
    res = READ_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm(unit, SCH_BLOCK(unit, core), sch_offset, &data);
    DNXC_IF_ERR_EXIT(res);

    fld_val = soc_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm_field32_get(unit, &data, DVS_CALENDAR_SEL_CH_NI_FXXf);
    sch_to_set = (fld_val == JER2_ARAD_OFP_RATES_CAL_SET_A) ? JER2_ARAD_OFP_RATES_CAL_SET_B : JER2_ARAD_OFP_RATES_CAL_SET_A;
    
    /* The device calendar length is the actual val minus 1 */
    sch_len = sch_cal_len - 1;

    /* Write to the non-Active SCH calendar indirectly */
    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_cal_tbl_set, (unit, core, sch_offset, sch_to_set, sch_cal_len, sch_cal->slots));
    DNXC_IF_ERR_EXIT(res);

    /* Calendar length SCH */ 
    soc_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm_field32_set(unit, &data, (sch_to_set == 0 ? CAL_A_LENf : CAL_B_LENf), sch_len);
    soc_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm_field32_set(unit, &data, DVS_CALENDAR_SEL_CH_NI_FXXf, sch_to_set);

    res = WRITE_SCH_CH_NIF_CALENDAR_CONFIGURATION_CNCCm(unit, SCH_BLOCK(unit, core), sch_offset, &data);    
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}


static uint32
 jer2_arad_ofp_rates_sw_db_port_priority_nof_valid_queues_get(
          DNX_SAND_IN  int    unit,
          DNX_SAND_IN  int    core,
          DNX_SAND_IN  uint32 tm_port,   
          DNX_SAND_IN  uint32 priority_ndx, 
          DNX_SAND_OUT uint32 *nof_valid_queues
        )
{
    uint32  
        res = DNX_SAND_NO_ERR,
        fap_port_ndx,
        base_q_pair,
        port_base_q_pair,
        ptc_ndx,
        is_ptc_found = FALSE,
        nof_priorities,
        flags;
    JER2_ARAD_SW_DB_DEV_EGR_PORT_PRIORITY
        rates;
    soc_pbmp_t
        pbmp;
    soc_port_t
        port_i;
    int core_i;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(nof_valid_queues);

    *nof_valid_queues = 0;
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &port_base_q_pair));

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pbmp));

    SOC_PBMP_ITER(pbmp, port_i) {

      DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
      if (DNX_PORT_IS_STAT_INTERFACE(flags))
      {
          continue;
      }

      DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &fap_port_ndx, &core_i));

      DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit,core_i,fap_port_ndx, &base_q_pair));
      
      if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
      {
        continue;
      }
      res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core_i, fap_port_ndx, &nof_priorities);
      DNXC_IF_ERR_EXIT(res);

      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.get(unit,core,&rates);
      DNXC_IF_ERR_EXIT(res);

      for (ptc_ndx = 0; ptc_ndx < nof_priorities; ++ptc_ndx)
      {
        if((port_base_q_pair == base_q_pair) && (ptc_ndx == priority_ndx)){
            is_ptc_found = TRUE;
        }
        if (rates.queue_rate[base_q_pair + ptc_ndx].valid)
        {
            *nof_valid_queues += 1;
        }
      }
    }
    if(!is_ptc_found){
        *nof_valid_queues += 1;
    }
exit:   
    DNXC_FUNC_RETURN;
}


static uint32
 jer2_arad_ofp_rates_sw_db_tcg_nof_valid_entries_get(
          DNX_SAND_IN  int    unit,
          DNX_SAND_IN  int    core,
          DNX_SAND_IN  uint32 tm_port,   
          DNX_SAND_IN  uint32 tcg, 
          DNX_SAND_OUT uint32 *nof_valid_entries
        )
{
    uint32  
        res = DNX_SAND_NO_ERR,
        fap_port_ndx,
        base_q_pair,
        ps,
        tcg_ps,
        is_tcg_found = FALSE,
        tcg_ndx,
        nof_priorities,
        port_i;
    JER2_ARAD_SW_DB_DEV_EGR_TCG 
        rates;
    soc_pbmp_t
        pbmp;
    int
      core_i;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(nof_valid_entries);
    *nof_valid_entries = 0;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    tcg_ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
    tcg_ps = base_q_pair;
#endif 

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i) 
    {
      DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &fap_port_ndx, &core_i));

      DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, fap_port_ndx,  &base_q_pair));
      
      if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
      {
        continue;
      }

      res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core_i, fap_port_ndx, &nof_priorities);
      DNXC_IF_ERR_EXIT(res);

      DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
      ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
      ps = base_q_pair;
#endif 


      res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.get(unit,core,&rates);
      DNXC_IF_ERR_EXIT(res);

      for (tcg_ndx = 0; tcg_ndx < nof_priorities; ++tcg_ndx)
      {
        if((ps == tcg_ps) && (tcg_ndx == tcg)){
            is_tcg_found = TRUE;
        }
        if (rates.tcg_rate[ps][tcg_ndx].valid)
        {
            *nof_valid_entries += 1;
        }
      }
    }

    if(!is_tcg_found){
        *nof_valid_entries += 1;
    }
exit:   
    DNXC_FUNC_RETURN
}


uint32
  jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rate
  )
{

    DNXC_INIT_FUNC_DEFS

    if (rate > JER2_ARAD_OFP_RATES_FC_Q_BURST_LIMIT_MAX) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate is above max")));
    }

exit:   
    DNXC_FUNC_RETURN
}

uint32
  jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rate
  )
{

    DNXC_INIT_FUNC_DEFS

    if (rate > JER2_ARAD_OFP_RATES_EMPTY_Q_BURST_LIMIT_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate is above max")));
    }

exit:   
    DNXC_FUNC_RETURN
}



uint32
  jer2_arad_ofp_rates_tcg_shaper_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  JER2_ARAD_TCG_NDX           tcg_ndx,
    DNX_SAND_IN  uint32                 rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE set_state
  )
{
  uint32
    res;
  uint32
    base_q_pair,
    nof_priorities,
    nof_valid_entries = 0,
    max_kbps;
  uint8
    is_sch_valid,
    is_egq_valid;

  DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_fap_port_id_verify(unit, tm_port);
  DNXC_SAND_IF_ERR_EXIT(res);

  DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
  if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal port")));
  }

  res = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
  DNXC_IF_ERR_EXIT(res);

  if (nof_priorities != JER2_ARAD_OFP_RATES_NOF_PRIORITIES_TCG_SUPPORT) {
   DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "nof prioritiees doesn't support tcg")));
  }

  /*
   * JER2_ARAD_TCG_MIN may be changed and be grater then zero.
   */
  /* coverity[unsigned_compare] */
  if ((tcg_ndx > JER2_ARAD_TCG_MAX) ||  (tcg_ndx < JER2_ARAD_TCG_MIN)){
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "tcg ndx is out of range")));
    }

  /* In case TCG is not being mapped by any other port priority. Rate must be 0 (disabled). */
  switch (set_state) {
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE:

      res = jer2_arad_ofp_rates_tcg_id_sch_verify(unit, core, tm_port, tcg_ndx, &is_sch_valid); 
      DNXC_IF_ERR_EXIT(res);

      max_kbps = (is_sch_valid) ? JER2_ARAD_IF_MAX_RATE_KBPS(unit):0;
      if (rate > max_kbps) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate is above max")));
      }
      break;

  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST:
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_BURST:

      if (rate > JER2_ARAD_OFP_RATES_BURST_LIMIT_MAX) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "burst is above max")));
      }
      break;

  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_EGQ_RATE:

      res = jer2_arad_ofp_rates_tcg_id_egq_verify(unit,core,tm_port,tcg_ndx,&is_egq_valid);
      DNXC_IF_ERR_EXIT(res);

      max_kbps = (is_egq_valid) ? JER2_ARAD_IF_MAX_RATE_KBPS(unit):0;
      if (rate > max_kbps) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate is above max")));
      }
      break;
  default:
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
  }
  

  res = jer2_arad_ofp_rates_sw_db_tcg_nof_valid_entries_get(unit,core,tm_port,tcg_ndx,&nof_valid_entries);
  DNXC_IF_ERR_EXIT(res);

  if (nof_valid_entries > JER2_ARAD_EGR_NOF_TCG_IDS){
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "egq_cal_len is out of range")));
    }

exit:
  DNXC_FUNC_RETURN
}


uint32
  jer2_arad_ofp_rates_port_priority_shaper_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_IN  uint32                 priority_ndx,
    DNX_SAND_IN  uint32                 rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE set_state
  )
{
  uint32
    res;
  uint32
    base_q_pair,
    nof_valid_queues,
    nof_priorities;

  DNXC_INIT_FUNC_DEFS


  res = jer2_arad_fap_port_id_verify(unit, tm_port);
  DNXC_SAND_IF_ERR_EXIT(res);

  DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

  if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "illegal port")));
  }

  DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities));

  if (priority_ndx > nof_priorities) {
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Priority index out of range")));
  }

  switch (set_state) {
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_RATE:
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE:
       if (rate > JER2_ARAD_IF_MAX_RATE_KBPS(unit)) {
         DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate above max")));
       }
       break;
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST:
       if (rate > JER2_ARAD_OFP_RATES_SCH_BURST_LIMIT_MAX) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "burst above max")));
       }
       break;
  case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_EGQ_BURST:
      if (rate > JER2_ARAD_OFP_RATES_BURST_LIMIT_MAX) {
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "burst above max")));
       }
      break;
 
  default:
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
  }
  
  res = jer2_arad_ofp_rates_sw_db_port_priority_nof_valid_queues_get(
          unit,
          core,
          tm_port,   
          priority_ndx, 
          &nof_valid_queues
        );
  DNXC_IF_ERR_EXIT(res);

  
  if (nof_valid_queues > JER2_ARAD_EGR_NOF_Q_PAIRS){
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "nof_valid_queues is out of range")));
    }

exit:
  DNXC_FUNC_RETURN
}

int
  jer2_arad_ofp_rates_single_port_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  uint32                    *rate
  )
{
  DNXC_INIT_FUNC_DEFS;

  /* Get interface id */

  if (rate) {
    if (*rate > JER2_ARAD_IF_MAX_RATE_KBPS(unit)) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR( "rate is above max")));
    }
  }
  
exit:
  DNXC_FUNC_RETURN
}


int
    jer2_arad_ofp_rates_port2chan_arb_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                fap_port,
    DNX_SAND_OUT  JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID *chan_arb_id
    )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    uint32 res = SOC_E_NONE;
    soc_reg_above_64_val_t data,
                           field;
    JER2_ARAD_INTERFACE_ID egr_interface_id;
    uint32  chan_arb_field_val = 0,
            chan_arb_val = 0;
    int core=0;
    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(chan_arb_id);

    res = jer2_arad_port2egress_offset(unit, core, fap_port, &egr_interface_id);
    DNXC_IF_ERR_EXIT(res);

    /* 
     * EGQ Interface ID to Channelize Arbiter ID 
     * Cannelize Arbiter ID 0 means non chanelized arbiter 
     */
    res = READ_EGQ_MAPPING_INTERFACES_TO_CHAN_ARBITERr(unit,data);
    DNXC_SAND_IF_ERR_EXIT(res);
    soc_reg_above_64_field_get(unit,EGQ_MAPPING_INTERFACES_TO_CHAN_ARBITERr,data,MAP_IFC_TO_CHAN_ARBf,field);
    SHR_BITCOPY_RANGE(&chan_arb_field_val,0,field,JER2_ARAD_OFP_RATES_CHAN_ARB_NOF_BITS*egr_interface_id,JER2_ARAD_OFP_RATES_CHAN_ARB_NOF_BITS);
    res = jer2_arad_nif_chan_arb_field_val_to_enum(unit,chan_arb_field_val,&chan_arb_val);
    DNXC_SAND_IF_ERR_EXIT(res);

    switch (chan_arb_val)
    {
    case JER2_ARAD_OFP_RATES_EGQ_NOF_CHAN_ARB:    
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Chan arbiter not allocated for port = %d"), fap_port));
        break;
    default:
        *chan_arb_id = chan_arb_val;
        break;
    }
    
    
exit:
    DNXC_FUNC_RETURN
#endif 
    return -1;
}
                        
int
    jer2_arad_ofp_rates_port2chan_cal_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_OUT uint32                *calendar
    )
{
    JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID chan_arb_id;

    DNXC_INIT_FUNC_DEFS

    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_port2chan_arb_unsafe(unit, tm_port, &chan_arb_id));
    *calendar = (uint32)(chan_arb_id);

exit:
    DNXC_FUNC_RETURN
}
                        
int
    jer2_arad_ofp_rates_sch_single_port_rate_hw_get(
        DNX_SAND_IN  int                    unit,
        DNX_SAND_IN  int                    core,
        DNX_SAND_IN  uint32                 tm_port,
        DNX_SAND_OUT uint32                 *rate
    )
{
    uint32 
        res = SOC_E_NONE,
        sch_cal_len,
        *sch_rates = NULL,
        is_channelized_id,
        sch_chan_arb_rate,
        sch_offset,
        egress_offset;
    soc_reg_above_64_val_t
        data;
    JER2_ARAD_OFP_RATES_CAL_SCH      
        sch_cal;
    uint32
        base_q_pair;

    DNXC_INIT_FUNC_DEFS;

    DNXC_ALLOC(sch_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"sch_rates");

    /* Get egress offset */
    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset));
    DNXC_IF_ERR_EXIT(res);

    /* Get scheduler offset */
    DNXC_IF_ERR_EXIT(READ_SCH_FC_MAP_FCMm(unit, SCH_BLOCK(unit, core), egress_offset, &data));
    sch_offset = soc_SCH_FC_MAP_FCMm_field32_get(unit,&data,FC_MAP_FCMf);

    is_channelized_id = JER2_ARAD_SCH_IS_CHNIF_ID(unit, sch_offset);

    if (is_channelized_id)
    {
        sch_cal_len = 0;

        /* Retrieve calendars */
        res = jer2_arad_ofp_rates_active_calendars_retrieve_sch(
                unit,
                sch_offset,
                core,
                &sch_cal,
                &sch_cal_len
            );
        DNXC_IF_ERR_EXIT(res);

        /*Retrieve total sch rate*/
        res = jer2_arad_sch_ch_if_rate_get_unsafe(
            unit,
            core,
            sch_offset, 
            &sch_chan_arb_rate);
        DNXC_IF_ERR_EXIT(res);

        /*Calculate sch rates*/
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
        if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal port")));
        }

        res = jer2_arad_ofp_rates_from_calendar_to_ports_sch_rate(
              unit,
              &sch_cal,
              sch_cal_len,
              sch_chan_arb_rate,
              JER2_ARAD_EGR_NOF_BASE_Q_PAIRS,
              base_q_pair,
              sch_rates
            );
        DNXC_IF_ERR_EXIT(res);
        
        *rate = sch_rates[base_q_pair];
    }
    else {/*This part serves mainly backward compatibility*/
        res = jer2_arad_sch_if_shaper_rate_get(unit, core, tm_port, rate);
        DNXC_IF_ERR_EXIT(res);
    }
                            
exit:
    DNXC_FREE(sch_rates);
    DNXC_FUNC_RETURN;
     
}
                        
int
    jer2_arad_ofp_rates_sch_single_port_rate_sw_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_IN  uint32                rate
    )
{
    uint32 res = SOC_E_NONE,
           base_q_pair,
           sch_offset,
           egress_offset;
    soc_reg_above_64_val_t
        data;

    DNX_SAND_OCC_BM_PTR modified_e2e_interfaces_occ;
    
    DNXC_INIT_FUNC_DEFS;

    /* Getting the fap port's base_q_pair */
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

    /* Setting rate to software */
    res = jer2_arad_sw_db_sch_port_rate_set(unit, core, base_q_pair, rate);
    DNXC_SAND_IF_ERR_EXIT(res);
    res = jer2_arad_sw_db_is_port_valid_set(unit, core, base_q_pair, TRUE);
    DNXC_SAND_IF_ERR_EXIT(res);


    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_e2e_interfaces_occ.get(unit,core, &modified_e2e_interfaces_occ);
    DNXC_IF_ERR_EXIT(res);

    /* Get egress offset */
    DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset)));

    /* Get scheduler offset */
    DNXC_IF_ERR_EXIT(READ_SCH_FC_MAP_FCMm(unit, SCH_BLOCK(unit, core), egress_offset, &data));
    sch_offset = soc_SCH_FC_MAP_FCMm_field32_get(unit,&data,FC_MAP_FCMf);

    /* mark E2E interface as modified */
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, modified_e2e_interfaces_occ, sch_offset, TRUE));

exit:
    DNXC_FUNC_RETURN;
}

int
    jer2_arad_ofp_rates_is_channalized(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_OUT uint32                *is_channalzied)
{
    uint32 res;
    JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_ID chan_arb_id;
    DNXC_INIT_FUNC_DEFS;

    /*Get channel arbiter*/
    res = jer2_arad_ofp_rates_port2chan_arb_unsafe(unit, tm_port, &chan_arb_id);
    DNXC_IF_ERR_EXIT(res);

    /*Only channelized ports need this configuration*/
    *is_channalzied = DNX_SAND_NUM2BOOL_INVERSE(chan_arb_id == JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_NON_CHAN);

exit:
    DNXC_FUNC_RETURN;
}

int
    jer2_arad_ofp_rates_sch_single_port_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit  
    )
{
    uint32 res = SOC_E_NONE,
        is_channelized_id,
        base_q_pair,
        sch_calendar_len,
        total_if_rate = 0,
        current_if_rate = 0,
        nof_valid_ports = 0,
        max_cal_size,
        sch_offset,
        sch_offset_i,
        tm_port_i = 0,
        tm_port = 0,
        egress_offset,
        egress_offset_i;
    uint32
        *sch_rates = NULL,
        flags;
    uint8 is_valid = FALSE, is_modified = FALSE;
    int core;
    JER2_ARAD_OFP_RATES_CAL_SCH
        *sch_calendar = NULL;
    soc_reg_above_64_val_t
        data;
    uint32
        is_rate_decreased =  FALSE;
    soc_pbmp_t ifs_bmp, pors_bmp;
    soc_port_t port, port_i;
    soc_port_if_t interface;
    DNX_SAND_OCC_BM_PTR modified_e2e_interfaces_occ;

    DNXC_INIT_FUNC_DEFS;

    DNXC_ALLOC(sch_rates, uint32, JER2_ARAD_NOF_FAP_PORTS, "sch_rates");
    DNXC_ALLOC(sch_calendar, JER2_ARAD_OFP_RATES_CAL_SCH, 1,"sch_calendar");

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pors_bmp));
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_e2e_interfaces_occ.get(unit, core, &modified_e2e_interfaces_occ);
        DNXC_IF_ERR_EXIT(res);

        for (sch_offset = 0; sch_offset < SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces); sch_offset++) {        

            sal_memset(sch_rates,0,sizeof(uint32)*JER2_ARAD_NOF_FAP_PORTS);
            sal_memset(sch_calendar,0,sizeof(JER2_ARAD_OFP_RATES_CAL_SCH));

            DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_is_occupied(unit, modified_e2e_interfaces_occ, sch_offset, &is_modified));
            if (!is_modified) {
                continue;
            }

            /* mark E2E interface as not modified */
            DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, modified_e2e_interfaces_occ, sch_offset, FALSE));

            sch_offset_i = INVALID_CALENDAR;
            /* find port that is mapped to the e2e interface */
            SOC_PBMP_ITER(pors_bmp, port) {
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags)); 
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface));
                if (DNX_PORT_IS_STAT_INTERFACE(flags) || DNX_PORT_IS_VIRTUAL_RCY_INTERFACE(flags) || interface == SOC_PORT_IF_NOCXN)
                {
                    continue;
                }
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
                if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
                {
                    continue;
                }

                /* Get egress offset */
                res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset));
                DNXC_IF_ERR_EXIT(res);

                /* Get scheduler offset */
                DNXC_IF_ERR_EXIT(READ_SCH_FC_MAP_FCMm(unit, SCH_BLOCK(unit, core), egress_offset, &data));
                sch_offset_i = soc_SCH_FC_MAP_FCMm_field32_get(unit,&data,FC_MAP_FCMf);
                if (sch_offset == sch_offset_i) {
                    break;
                }
            }

            if (sch_offset != sch_offset_i) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR("No port found with the modified E2E interface")));
            }

            is_channelized_id = JER2_ARAD_SCH_IS_CHNIF_ID(unit, sch_offset);
            if (is_channelized_id)
            {
                total_if_rate = 0;

                DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_ports_to_same_interface_get(unit, port, &ifs_bmp));
                DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port, &egress_offset)));


                /*Retrive rates from the software database*/
                SOC_PBMP_ITER(ifs_bmp, port_i){
                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port_i, &core));
                    DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core, tm_port_i, &egress_offset_i)));
                    if (egress_offset == egress_offset_i) {
                        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port_i, &base_q_pair));
                        DNXC_IF_ERR_EXIT(jer2_arad_sw_db_is_port_valid_get(unit, core, base_q_pair, &is_valid));
                        if(is_valid) {
                            DNXC_SAND_IF_ERR_EXIT(jer2_arad_sw_db_sch_port_rate_get(unit, core, base_q_pair, &sch_rates[base_q_pair]));
                            total_if_rate += sch_rates[base_q_pair];
                            nof_valid_ports += 1;
                        }
                    }
                }

                /* in order to avoid momentary packet loss, the order of setting the traversal rate and calendar is important.
                 * If rate is decreased then first calender should be set. Otherwise, if rate is increased, first traversal rate 
                 * should be set. 
                 */
                res = jer2_arad_sch_ch_if_rate_get_unsafe(unit, core, sch_offset, &current_if_rate);
                DNXC_IF_ERR_EXIT(res);

                /* is rate decreased? */
                if (current_if_rate > total_if_rate) {
                    is_rate_decreased = TRUE;
                }

                if (!is_rate_decreased) {
                    /*Configure traversal rate*/
                    res = jer2_arad_sch_ch_if_rate_set_unsafe(
                      unit,
                      core,
                      sch_offset,
                      total_if_rate
                    );
                    DNXC_IF_ERR_EXIT(res);
                }
                                                 
                res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_cal_max_size_get, (unit, sch_offset, &max_cal_size));
                DNXC_IF_ERR_EXIT(res);

                /*Configure calendar*/
                res = jer2_arad_ofp_rates_from_rates_to_calendar(unit,
                                                            sch_rates,
                                                            JER2_ARAD_EGR_NOF_BASE_Q_PAIRS,
                                                            total_if_rate,
                                                            max_cal_size,
                                                            sch_calendar,
                                                            &sch_calendar_len);
                DNXC_IF_ERR_EXIT(res);

                res = jer2_arad_ofp_rates_active_sch_calendars_config(unit,
                                                         sch_offset,
                                                         core,
                                                         sch_calendar,
                                                         sch_calendar_len);
                DNXC_IF_ERR_EXIT(res);

                if (is_rate_decreased) {
                    /*Configure traversal rate*/
                    res = jer2_arad_sch_ch_if_rate_set_unsafe(
                      unit,
                      core,
                      sch_offset,
                      total_if_rate
                    );
                    DNXC_IF_ERR_EXIT(res);
                }
                
            }

            else {/* For non-channalized interfaces - port rate is same as if rate*/
                res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
                DNXC_IF_ERR_EXIT(res);
                res = jer2_arad_sw_db_is_port_valid_get(unit, core, base_q_pair, &is_valid);
                DNXC_IF_ERR_EXIT(res);
                if (is_valid)
                {
                    nof_valid_ports += 1;
                    /*Get sch rate*/
                    res = jer2_arad_sw_db_sch_port_rate_get(unit, core, base_q_pair, &sch_rates[0]);
                    DNXC_SAND_IF_ERR_EXIT(res);
                    res = jer2_arad_sch_if_shaper_rate_set(unit, core, tm_port, sch_rates[0]);
                    DNXC_IF_ERR_EXIT(res);
                }
                else
                {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Invalid port")));
                }
            }
        }
    }
exit:
    DNXC_FREE(sch_rates);
    DNXC_FREE(sch_calendar);
    DNXC_FUNC_RETURN;
}

uint32
    jer2_arad_ofp_rates_egq_single_port_rate_hw_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT uint32                 *rate
    )
{
    uint32 res = SOC_E_NONE,
           egq_cal_len,
           *egq_rates = NULL,
           base_q_pair;
    uint8 add_dummy_tail = FALSE;
    JER2_ARAD_OFP_RATES_CAL_EGQ      
           egq_cal;
    JER2_ARAD_OFP_RATES_CAL_INFO
           cal_info;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(rate);

    DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"egq_rates");
    jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(&cal_info);
      

    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_port2chan_cal_get, (unit, core, tm_port, &cal_info.chan_arb_id));
    DNXC_IF_ERR_EXIT(res);

    /* all ports on the interface have rate=0, no calendar was allocated */
    if (cal_info.chan_arb_id == INVALID_CALENDAR) {
        *rate = 0;
    } else {
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;

        egq_cal_len = 0;
        /*Retrieve calendars*/
        res = jer2_arad_ofp_rates_active_generic_calendars_retrieve_egq(
                unit,
                core,
                &cal_info,
                &egq_cal,
                &egq_cal_len,
                FALSE
            );
        DNXC_IF_ERR_EXIT(res);
        
        /* Dummy tail is still needed for JER2_ARAD and below */
        if (SOC_IS_ARADPLUS_AND_BELOW(unit))
        {
            add_dummy_tail = TRUE;
        }
        /*Calculate egq rates*/
        res = jer2_arad_ofp_rates_from_generic_calendar_to_ports_egq_rate(
              unit,
              core,
              &cal_info,
              &egq_cal,
              egq_cal_len,
              add_dummy_tail,
              JER2_ARAD_EGR_NOF_BASE_Q_PAIRS,
              egq_rates
            );
        DNXC_IF_ERR_EXIT(res);
        /*Get the rate of the requested port*/
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
        DNXC_IF_ERR_EXIT(res);
        if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal port")));
        }
        *rate = egq_rates[base_q_pair];
    }
                            
exit:
    DNXC_FREE(egq_rates);
    DNXC_FUNC_RETURN;
     
}

int
    jer2_arad_ofp_rates_egq_single_port_rate_sw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_IN  uint32                rate
    )
{
    uint32 res = SOC_E_NONE,
           base_q_pair,
           chan_arb;
    DNX_SAND_OCC_BM_PTR 
           modified_cals_occ;
    soc_port_if_t   
           interface_type;
    soc_port_t      
           port;

    DNXC_INIT_FUNC_DEFS

    /* in case of ERP port do nothing*/
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
    if (interface_type == SOC_PORT_IF_ERP) { /* do nothing */
        SOC_EXIT;
    }

    /*
     * Getting the fap port's base_q_pair
     */
    res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
    DNXC_IF_ERR_EXIT(res);

    /*
     * Setting rate to sw
     */
    res = jer2_arad_sw_db_egq_port_rate_set(unit, core, base_q_pair, rate);
    DNXC_SAND_IF_ERR_EXIT(res);
    res = jer2_arad_sw_db_is_port_valid_set(unit, core, base_q_pair, TRUE);
    DNXC_SAND_IF_ERR_EXIT(res);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_channelized_cals_occ.get(unit, core, &modified_cals_occ);
    DNXC_IF_ERR_EXIT(res);

    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_port2chan_cal_get, (unit, core, tm_port, &chan_arb));
    DNXC_IF_ERR_EXIT(res);

    /* mark calendar as modified */
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, modified_cals_occ, chan_arb, TRUE));

exit:
    DNXC_FUNC_RETURN
     
}

int
    jer2_arad_ofp_rates_egq_single_port_rate_sw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_OUT uint32                *rate
    )
{
    uint32 res = SOC_E_NONE,
           base_q_pair;
    soc_port_if_t   
           interface_type;
    soc_port_t      
           port;

    DNXC_INIT_FUNC_DEFS

    /* in case of ERP port do nothing*/
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &port));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
    if (interface_type == SOC_PORT_IF_ERP) { /* do nothing */
        *rate = 0;
        SOC_EXIT;
    }

    /*
     * Getting the fap port's base_q_pair
     */
    res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
    DNXC_IF_ERR_EXIT(res);

    /*
     * Getting sw rate
     */
    res = jer2_arad_sw_db_egq_port_rate_get(unit, core, base_q_pair, rate);
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
     
}

static uint32
    jer2_arad_ofp_rates_calcal_config_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core
    )
{
    uint32 
        res = SOC_E_NONE,
        tm_port,
        idx,
        act_cal,
        chan_arb_i,
        egq_calcal_chan_arb_rate_requested = 0,
        egq_calcal_calendar_len = 0,
        egq_calcal_rates[SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS)],
        egq_calcal_instances[SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS)],
        base_q_pair,
        egq_rate,
        port_i,
        reg_val,
        flags;
    uint8  
        is_valid;
    JER2_ARAD_OFP_RATES_CAL_SCH
        *egq_calcal_calendar = NULL;
    JER2_ARAD_OFP_RATES_CAL_SET
        cal2set;
    JER2_ARAD_EGQ_CCM_TBL_DATA
        ccm_tbl_data;
    soc_pbmp_t
        pbmp;
    int
        core_i;
    DNXC_INIT_FUNC_DEFS;

    DNXC_ALLOC(egq_calcal_calendar, JER2_ARAD_OFP_RATES_CAL_SCH, 1, "egq_calcal_calendar");

    /*Clear*/
    egq_calcal_chan_arb_rate_requested = 0;
    sal_memset(egq_calcal_rates,0,sizeof(uint32)*SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS));
    sal_memset(egq_calcal_instances,0,sizeof(uint32)*SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS));

    /*Calculate CalCal according to the sum of ofp rates on each channelize arbiter */
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i)
    {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
        if (DNX_PORT_IS_STAT_INTERFACE(flags))
        {
            continue;
        }
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core_i));

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit,core_i, tm_port,  &base_q_pair));
        /* Check invalid fap port */
        if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR) {
            continue;
        }

        res = jer2_arad_sw_db_is_port_valid_get(unit, core_i, base_q_pair, &is_valid);
        DNXC_SAND_IF_ERR_EXIT(res);

        if(!is_valid) {
            continue;
        }

        res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_port2chan_cal_get, (unit, core_i, tm_port, &chan_arb_i));
        DNXC_IF_ERR_EXIT(res);

        if(chan_arb_i == JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_INVALID) {
            continue;
        }

        res = jer2_arad_sw_db_egq_port_rate_get(unit, core_i, base_q_pair, &egq_rate); 
        DNXC_SAND_IF_ERR_EXIT(res);

        egq_calcal_chan_arb_rate_requested += egq_rate;
        egq_calcal_rates[chan_arb_i] += egq_rate;

    }

    /* Convert rates to calendar */
    res = jer2_arad_ofp_rates_from_rates_to_calendar(
            unit,
            egq_calcal_rates,
            SOC_DNX_DEFS_GET(unit, nof_channelized_calendars),
            egq_calcal_chan_arb_rate_requested,
            JER2_ARAD_OFP_RATES_CALCAL_LEN_EGQ_MAX,
            egq_calcal_calendar,
            &egq_calcal_calendar_len
          );
    DNXC_IF_ERR_EXIT(res);

    /* Get the active calendar */
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
    act_cal = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_SET_SELf);
    cal2set = (act_cal == JER2_ARAD_OFP_RATES_CAL_SET_A)?JER2_ARAD_OFP_RATES_CAL_SET_B:JER2_ARAD_OFP_RATES_CAL_SET_A;

    /* Write CalCal length to the inactive calendar */
    /* read */
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_CALENDARS_ARBITRATION_CYCLE_LENGTHr(unit, core, &reg_val));
    /* modify */
    if (cal2set == JER2_ARAD_OFP_RATES_CAL_SET_A)
    {
      soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_CALENDARS_ARBITRATION_CYCLE_LENGTHr, &reg_val, CAL_CAL_LEN_Af, egq_calcal_calendar_len - 1);
    }
    else
    {
      soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_CALENDARS_ARBITRATION_CYCLE_LENGTHr, &reg_val, CAL_CAL_LEN_Bf, egq_calcal_calendar_len - 1);
    }
    /* write */
    DNXC_SAND_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_CALENDARS_ARBITRATION_CYCLE_LENGTHr(unit, core, reg_val));
    
    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.calcal_length.set(unit, core, egq_calcal_calendar_len);
    DNXC_IF_ERR_EXIT(res);

    /* Write CalCal entries, calculate nof_calal instances per chan_cal*/
    for (idx = 0; idx < egq_calcal_calendar_len; ++idx)
    {
      egq_calcal_instances[egq_calcal_calendar->slots[idx]] += 1;
      ccm_tbl_data.interface_select = egq_calcal_calendar->slots[idx];
      res = jer2_arad_egq_ccm_tbl_set_unsafe(
              unit,
              core,
              idx + (cal2set) * JER2_ARAD_OFP_RATES_CALCAL_LEN_EGQ_MAX,
              &ccm_tbl_data
            );
      DNXC_SAND_IF_ERR_EXIT(res);
    }   
    
    /*Set calal nof instances per chan_arb to sw_db*/
    for (chan_arb_i = 0; chan_arb_i < SOC_DNX_DEFS_GET(unit, nof_channelized_calendars); ++chan_arb_i)
    {
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chan_arb.nof_calcal_instances.set(unit, core, chan_arb_i, egq_calcal_instances[chan_arb_i]);
        DNXC_IF_ERR_EXIT(res);
    }

exit:
    DNXC_FREE(egq_calcal_calendar);
    DNXC_FUNC_RETURN
     
}

int
    jer2_arad_ofp_rates_egq_single_port_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit
    )
{
    uint32 
        res = SOC_E_NONE,
        tm_port_i,
        calendar_tm_port = 0,
        base_q_pair,
        egq_calendar_len,
        cur_chan_arb_i,
        egq_if_rate_requested = 0,
        recalc,
        act_cal,
        reg_val,
        if_internal_rate,
        if_id,
        nof_instances,
        *egq_rates = NULL,
        new_cal = INVALID_CALENDAR,
        flags;
    uint8 
        is_valid,
        is_cal_modified = FALSE,
        add_dummy_tail = FALSE;
    JER2_ARAD_OFP_RATES_CAL_EGQ
        *egq_calendar = NULL;
    JER2_ARAD_OFP_RATES_CAL_INFO
        cal_info ;
    JER2_ARAD_OFP_RATES_CAL_SET
        cal2set;
    soc_pbmp_t 
        pbmp;
    int 
        core_i,
        chan_arb_i,
        is_single_cal_mode,
        update_required,
        is_high_priority_port = 0,
        is_channelized;
    soc_port_t 
        port_i,
        calendar_port;
    DNX_SAND_OCC_BM_PTR 
        modified_cals_occ;
    DNXC_INIT_FUNC_DEFS

    DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS, "egq_rates");
    DNXC_ALLOC(egq_calendar, JER2_ARAD_OFP_RATES_CAL_EGQ, 1,"egq_calendar");

    /* iterate over all cores */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core_i) {

        update_required = 1;

        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.modified_channelized_cals_occ.get(unit, core_i, &modified_cals_occ);
        DNXC_IF_ERR_EXIT(res);

        for (chan_arb_i = 0 ; chan_arb_i < SOC_DNX_DEFS_GET(unit, nof_channelized_calendars) ; ++chan_arb_i) {
            DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_is_occupied(unit, modified_cals_occ, chan_arb_i, &is_cal_modified));
            if (is_cal_modified) {
                break;
            }
        }

        /* no modified calendar was found on this core, can skip the core */
        if (!is_cal_modified) {
            continue;
        }

        sal_memset(egq_rates,0,JER2_ARAD_NOF_FAP_PORTS*sizeof(uint32));
        sal_memset(egq_calendar,0,sizeof(JER2_ARAD_OFP_RATES_CAL_EGQ));

        /* Get active calendar */
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core_i, &reg_val));
        act_cal = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_SET_SELf);
        cal2set = (act_cal == JER2_ARAD_OFP_RATES_CAL_SET_A)?JER2_ARAD_OFP_RATES_CAL_SET_B:JER2_ARAD_OFP_RATES_CAL_SET_A;
        /* incase we replace small cal with big cal,new calendar and calcal need to be recalculated */
        while(update_required)
        {
        update_required = 0;
        /* Configure calcal */
        res = jer2_arad_ofp_rates_calcal_config_unsafe(unit, core_i);
        DNXC_IF_ERR_EXIT(res);
        
        for (chan_arb_i = 0 ; chan_arb_i < SOC_DNX_DEFS_GET(unit, nof_channelized_calendars) ; ++chan_arb_i) {

            DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.chan_arb.nof_calcal_instances.get(unit,
                core_i, chan_arb_i, &nof_instances));
            if (nof_instances > 0) {
                /*Retrive rates from the software database*/
                egq_if_rate_requested = 0;
                sal_memset(egq_rates,0,JER2_ARAD_NOF_FAP_PORTS*sizeof(uint32));
            
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core_i, 0, &pbmp));

                calendar_port = -1;
                SOC_PBMP_ITER(pbmp, port_i)
                {
                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
                    if (DNX_PORT_IS_STAT_INTERFACE(flags))
                    {
                        continue;
                    }
                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port_i, &core_i));

                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, tm_port_i,  &base_q_pair));
                    /* Check invalid fap port */
                    if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
                    {
                        continue;
                    }

                    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_port2chan_cal_get, (unit, core_i, tm_port_i, &cur_chan_arb_i));
                    DNXC_IF_ERR_EXIT(res);

                    if(cur_chan_arb_i != chan_arb_i) { /* handle current calendar */
                        continue; 
                    } else {    /* save one of the ports that belong to this calendar to retrieve its interface rate later on */
                        if (calendar_port == -1) {
                            calendar_port = port_i;
                            calendar_tm_port = tm_port_i;
                        }
                    }

                    res = jer2_arad_sw_db_is_port_valid_get(unit, core_i, base_q_pair, &is_valid);
                    DNXC_SAND_IF_ERR_EXIT(res);
                    if (is_valid)
                    {
                        /*Get egq rate*/
                        res = jer2_arad_sw_db_egq_port_rate_get(unit, core_i, base_q_pair, &egq_rates[base_q_pair]);
                        DNXC_SAND_IF_ERR_EXIT(res);
                        egq_if_rate_requested += egq_rates[base_q_pair];
                    }
                }

                /* Configure calendar */
                cal_info.chan_arb_id = chan_arb_i; 
                cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;

                /* check if calendar should be recalculated */
                recalc = FALSE;
                DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_is_occupied(unit, modified_cals_occ, chan_arb_i, &is_cal_modified));
                if (is_cal_modified == TRUE) {
                    recalc = TRUE;
                    /* unset modified cal indication */
                    DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, modified_cals_occ, chan_arb_i, FALSE));
                }
                /* Dummy tail is still needed for JER2_ARAD and below */
                if (SOC_IS_ARADPLUS_AND_BELOW(unit))
                {
                    add_dummy_tail = TRUE;
                }
                res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar(
                          unit,
                          core_i,
                          &cal_info,
                          egq_rates,
                          JER2_ARAD_EGR_NOF_BASE_Q_PAIRS,
                          egq_if_rate_requested,
                          JER2_ARAD_OFP_RATES_EGQ_CAL_LEN_EGQ(unit, cal_info.chan_arb_id),
                          recalc,
                          add_dummy_tail,
                          egq_calendar,
                          &egq_calendar_len  
                        );
                /* Check if current calendar size is enough, Jericho only */
                if (SOC_IS_JERICHO(unit) && calendar_port != -1)
                {
                    /* Check if we fail because small calendar wasn't enough, if so try again with big cal*/
                    if(res == SOC_E_LIMIT)
                    {
                        res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar(
                          unit,
                          core_i,
                          &cal_info,
                          egq_rates,
                          JER2_ARAD_EGR_NOF_BASE_Q_PAIRS,
                          egq_if_rate_requested,
                          SOC_DNX_DEFS_GET(unit, big_channelized_cal_size),
                          recalc,
                          add_dummy_tail,
                          egq_calendar,
                          &egq_calendar_len
                        );
                        DNXC_IF_ERR_EXIT(res);
                        DNXC_IF_ERR_EXIT(soc_jer2_jer_ofp_rates_calendar_allocate(unit, core_i, calendar_port, egq_calendar_len, cal_info.chan_arb_id, &new_cal));
                        /* mark calendar as modified */
                        if (new_cal != INVALID_CALENDAR)
                        {
                            DNXC_SAND_IF_ERR_EXIT(dnx_sand_occ_bm_occup_status_set(unit, modified_cals_occ, new_cal, TRUE));
                            DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_single_cal_mode_get(unit, calendar_port, &is_single_cal_mode));
                            if(is_single_cal_mode)
                            {
                                DNXC_IF_ERR_EXIT(dnx_port_sw_db_high_priority_cal_set(unit, calendar_port, new_cal));
                                DNXC_IF_ERR_EXIT(dnx_port_sw_db_low_priority_cal_set(unit, calendar_port, new_cal));
                            } else {
                                DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_egr_queuing_is_high_priority_port_get,(unit, \
                                                                                    core_i, calendar_tm_port, &is_high_priority_port)));
                                if(is_high_priority_port)
                                {
                                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_high_priority_cal_set(unit, calendar_port, new_cal));
                                } else {
                                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_low_priority_cal_set(unit, calendar_port, new_cal));
                                }
                            }
                            /*make sure we go over new_cal again,set update_required*/
                            update_required = 1;
                            continue;
                        } else {
                            DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("No calendars are left for port %d \n"), calendar_port));
                        }
                    }
                }
                DNXC_IF_ERR_EXIT(res);
                /* write to inactive calendar slots(ports rates) and length */
                res = jer2_arad_ofp_rates_active_egq_generic_calendars_config(
                                                                 unit,
                                                                 core_i,
                                                                 &cal_info,
                                                                 egq_calendar,
                                                                 egq_calendar_len);
                DNXC_IF_ERR_EXIT(res);

                
                if (SOC_IS_JERICHO(unit) && calendar_port != -1) {

                    /* start - cal rate set */
                    DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_single_cal_mode_get(unit, calendar_port, &is_single_cal_mode));
                    if (!is_single_cal_mode) { /* dual calendar mode => cal rate is set to maximum (becomes irrelevant) */
                        if_internal_rate = SOC_DNX_DEFS_GET(unit, cal_internal_rate_max);

                    } else { /* single calendar mode => cal rate is set to interface rate */
                        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_channelized_port_get(unit, calendar_port, &is_channelized));
                        if (is_channelized) {
                            res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port2egress_offset, (unit, core_i, calendar_tm_port, &if_id));
                            DNXC_IF_ERR_EXIT(res);
                        } else {
                            if_id = SOC_DNX_DEFS_GET(unit, non_channelized_cal_id);
                        }

                        res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_interface_internal_rate_get, (unit, core_i, if_id, &if_internal_rate));
                        DNXC_IF_ERR_EXIT(res);                        
                    }

                    /* update calendar rate */
                    res = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egress_shaper_cal_write, (unit, core_i, &cal_info, cal2set, 
                                                                                                  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_RATE, if_internal_rate));
                    DNXC_IF_ERR_EXIT(res);
                }
            }
        }

        }
        /* Set active calendar */
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core_i, &reg_val));
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, OTM_SPR_SET_SELf, cal2set);
        DNXC_SAND_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core_i, reg_val));
    }

exit:
    DNXC_FREE(egq_rates);
    DNXC_FREE(egq_calendar);
    DNXC_FUNC_RETURN
}

static uint32
    jer2_arad_ofp_rates_max_burst_generic_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  int                      core,
    DNX_SAND_IN  uint32                   base_q_pair,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_IN  uint32                   max_burst
    )
{
    uint32  res = SOC_E_NONE,
            fld_val,
            reg_val,
            egq_to_set,
            offset;
    soc_field_t
            field_cal_set_name;
    JER2_ARAD_EGQ_PMC_TBL_DATA
            pmc_tbl_data;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(cal_info);

    res = jer2_arad_ofp_rates_retrieve_egress_shaper_setting_field(
      unit,
      cal_info,
      &field_cal_set_name
    );

    DNXC_IF_ERR_EXIT(res);
    
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
    fld_val = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_SET_SELf);
    egq_to_set = (fld_val == JER2_ARAD_OFP_RATES_CAL_SET_A) ? JER2_ARAD_OFP_RATES_CAL_SET_A : JER2_ARAD_OFP_RATES_CAL_SET_B;

    /*
     * Setting max_rate, set both sets (A and B)
     */
    offset = JER2_ARAD_OFP_RATES_EGQ_PMC_OFFSET_GET(cal_info,egq_to_set,base_q_pair);
    pmc_tbl_data.port_max_credit = max_burst;

    res = jer2_arad_egq_pmc_tbl_set_unsafe(
              unit,
              core,
              cal_info,
              offset,
              &pmc_tbl_data
            );
    DNXC_SAND_IF_ERR_EXIT(res);

    egq_to_set = !egq_to_set;
    offset = JER2_ARAD_OFP_RATES_EGQ_PMC_OFFSET_GET(cal_info,egq_to_set,base_q_pair);
    res = jer2_arad_egq_pmc_tbl_set_unsafe(
              unit,
              core,
              cal_info,
              offset,
              &pmc_tbl_data
            );
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
     
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                priority_ndx, 
    DNX_SAND_IN  uint32                max_burst
    )
{
    uint32  res = SOC_E_NONE,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
    if (egq_tcg_qpair_shaper_enable)
    {
        /* Getting the fap port's base_q_pair*/
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
        DNXC_IF_ERR_EXIT(res);

        /* Setting max burst*/
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY;

        res = jer2_arad_ofp_rates_max_burst_generic_set_unsafe(unit,
                                                          core,
                                                          (base_q_pair + priority_ndx),
                                                          &cal_info,
                                                          max_burst);
        DNXC_IF_ERR_EXIT(res);
    }
    
exit:
    DNXC_FUNC_RETURN
     
}

uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_IN  uint32                max_burst
    )
{
    uint32  res = SOC_E_NONE,
            tcg_id,
            ps,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
#if 1
  if (egq_tcg_qpair_shaper_enable)
#endif
  {
        /* Getting the fap port's base_q_pair*/
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit,core,tm_port,&base_q_pair);
        DNXC_IF_ERR_EXIT(res);

        /*Getting tcg_id*/
       DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
        ps = base_q_pair;
#endif 

        tcg_id = JER2_ARAD_OFP_RATES_TCG_ID_GET(ps,tcg_ndx);

        /* Setting max burst*/
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_TCG;

        res = jer2_arad_ofp_rates_max_burst_generic_set_unsafe(unit,
                                                          core,
                                                          tcg_id,
                                                          &cal_info,
                                                          max_burst);
        DNXC_IF_ERR_EXIT(res);
  }
    
exit:
    DNXC_FUNC_RETURN
     
}

int
    jer2_arad_ofp_rates_single_port_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_IN  uint32                max_burst
    )
{
    uint32  res = SOC_E_NONE,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    DNXC_INIT_FUNC_DEFS

    /*
     * Getting the fap port's base_q_pair
     */
    res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
    DNXC_IF_ERR_EXIT(res);

    /* Setting max burst*/
    cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;

    res = jer2_arad_ofp_rates_max_burst_generic_set_unsafe(unit,
                                                      core,
                                                      base_q_pair,
                                                      &cal_info,
                                                      max_burst);
    DNXC_IF_ERR_EXIT(res);
    
exit:
    DNXC_FUNC_RETURN
     
}

static uint32
    jer2_arad_ofp_rates_max_burst_generic_get_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  int                     core,
    DNX_SAND_IN  uint32                  tm_port,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_OUT  uint32                 *max_burst
    )
{
    uint32  res = SOC_E_NONE,
            egq_to_get,
            fld_val,
            reg_val,
            offset;
    soc_field_t
            field_cal_set_name;
    JER2_ARAD_EGQ_PMC_TBL_DATA
            pmc_tbl_data;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst);
    DNXC_NULL_CHECK(cal_info);

    res = jer2_arad_ofp_rates_retrieve_egress_shaper_setting_field(
      unit,
      cal_info,
      &field_cal_set_name
    );
    DNXC_IF_ERR_EXIT(res);
    
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
    fld_val = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, OTM_SPR_SET_SELf);
    egq_to_get = (fld_val == JER2_ARAD_OFP_RATES_CAL_SET_A) ? JER2_ARAD_OFP_RATES_CAL_SET_A : JER2_ARAD_OFP_RATES_CAL_SET_B;

    /*
     * Setting max_rate
     */
    offset = JER2_ARAD_OFP_RATES_EGQ_PMC_OFFSET_GET(cal_info,egq_to_get,tm_port);

    res = jer2_arad_egq_pmc_tbl_get_unsafe(
              unit,
              core,
              cal_info,
              offset,
              &pmc_tbl_data
            );
    DNXC_SAND_IF_ERR_EXIT(res);

    *max_burst = pmc_tbl_data.port_max_credit;

exit:
    DNXC_FUNC_RETURN
     
}

int
    jer2_arad_ofp_rates_single_port_max_burst_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core,
    DNX_SAND_IN  uint32                 tm_port,
    DNX_SAND_OUT  uint32                *max_burst
    )
{
    uint32  res = SOC_E_NONE,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst);

    /* Getting the fap port's base_q_pair */
    res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
    DNXC_IF_ERR_EXIT(res);

    /* Getting max burst */
    cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;

    res = jer2_arad_ofp_rates_max_burst_generic_get_unsafe(unit,
                                                      core,
                                                      base_q_pair,
                                                      &cal_info,
                                                      max_burst);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
     
}

uint32
    jer2_arad_ofp_rates_egq_tcg_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,   
    DNX_SAND_IN  uint32                tcg_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    )
{
    uint32  res = SOC_E_NONE,
            tcg_id,
            ps,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(max_burst);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
#if 1
  if (egq_tcg_qpair_shaper_enable)
#endif
  {
        /* Getting the fap port's base_q_pair*/
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
        DNXC_IF_ERR_EXIT(res);

        /*Getting tcg_id*/
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
        ps = base_q_pair;
#endif 

        tcg_id = JER2_ARAD_OFP_RATES_TCG_ID_GET(ps,tcg_ndx);

        /* Setting max burst*/
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_TCG;

        res = jer2_arad_ofp_rates_max_burst_generic_get_unsafe(unit,
                                                          core,
                                                          tcg_id,
                                                          &cal_info,
                                                          max_burst);
        DNXC_IF_ERR_EXIT(res);
  }
    
exit:
    DNXC_FUNC_RETURN
     
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,     
    DNX_SAND_IN  uint32                priority_ndx, 
    DNX_SAND_OUT  uint32               *max_burst
    )
{
    uint32  res = SOC_E_NONE,
            base_q_pair;
    JER2_ARAD_OFP_RATES_CAL_INFO
            cal_info;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(max_burst);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
    if (egq_tcg_qpair_shaper_enable)
    {
        /* Getting the fap port's base_q_pair */
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
        DNXC_IF_ERR_EXIT(res);

        /* Getting max burst */
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY;

        res = jer2_arad_ofp_rates_max_burst_generic_get_unsafe(unit,
                                                          core,
                                                          (base_q_pair + priority_ndx),
                                                          &cal_info,
                                                          max_burst);
        DNXC_IF_ERR_EXIT(res);
    }
exit:
    DNXC_FUNC_RETURN
     
}

uint32
  jer2_arad_ofp_rates_egq_interface_shaper_set_unsafe(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  int                         core,
    DNX_SAND_IN  uint32                      tm_port,
    DNX_SAND_IN DNX_TMC_OFP_SHPR_UPDATE_MODE rate_update_mode,
    DNX_SAND_IN  uint32                      if_shaper_rate
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res = SOC_E_NONE,
    if_rate = if_shaper_rate;
  JER2_ARAD_OFP_RATES_CAL_INFO
    cal_info;
  JER2_ARAD_INTERFACE_ID         
      if_id;

  DNXC_INIT_FUNC_DEFS;

  /* Get interface id */
  res = jer2_arad_port_to_interface_map_get(
          unit,
          core,
          tm_port,
          &if_id,
          NULL
        );
 DNXC_IF_ERR_EXIT(res);

  if (if_id == JER2_ARAD_NIF_ID_NONE)
  {
    /*Probably a disabled interface - exit without any action*/
    JER2_ARAD_DO_NOTHING_AND_EXIT;
  }

  cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB;
  res = jer2_arad_ofp_rates_port2chan_arb_unsafe(unit,tm_port,&cal_info.chan_arb_id);
  DNXC_IF_ERR_EXIT(res);
   
  res = jer2_arad_ofp_rates_egq_shaper_config(
          unit,
          &cal_info,
          if_rate
        );
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN
#endif 
    return -1;
}

/*********************************************************************
*     Configure MAL-level shaping. This is required when the
*     shaping rate is different from the accumulated rate of
*     the OFP-s mapped to the NIF.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ofp_rates_interface_shaper_verify(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port

  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY


    uint32
        res = SOC_E_NONE;
  
    JER2_ARAD_INTERFACE_ID
        if_id;

    DNXC_INIT_FUNC_DEFS;

    /* Get interface id */
    res = jer2_arad_port_to_interface_map_get(
          unit,
          core,
          tm_port,
          &if_id,
          NULL
        );
    DNXC_IF_ERR_EXIT(res);

    res = jer2_arad_interface_id_verify(
          unit,
          if_id
        );
    DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
#endif 
    return -1;
}

uint32
  jer2_arad_ofp_rates_egq_interface_shaper_get_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  int                       core,
    DNX_SAND_IN  uint32                    tm_port,
    DNX_SAND_OUT uint32                    *if_shaper_rate
  )
{
  uint32
    res = SOC_E_NONE;
  uint32
    chan_arb_id;  

  DNXC_INIT_FUNC_DEFS;
  
  DNXC_NULL_CHECK(if_shaper_rate);

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  /* Get interface id */
  res = jer2_arad_port_to_interface_map_get(
          unit,
          core,
          tm_port,
          &if_id,
          NULL
        );
  DNXC_IF_ERR_EXIT(res);
#endif 


  DNXC_NULL_CHECK(if_shaper_rate);

  res = jer2_arad_ofp_rates_if_id2chan_arb_get_unsafe(unit, core, tm_port, &chan_arb_id);
  DNXC_IF_ERR_EXIT(res);

  res = jer2_arad_ofp_rates_egq_shaper_retrieve(
          unit,
          chan_arb_id,
          TRUE,
          if_shaper_rate
        );
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_set_unsafe(
       DNX_SAND_IN  int    unit,
       DNX_SAND_IN  uint32 max_burst_empty_queues
       )
{
    uint32 res = DNX_SAND_NO_ERR,
           reg_val;
    int    core;

    DNXC_INIT_FUNC_DEFS

    /*Max burst for empty and fc queues {*/
    if (SOC_IS_ARADPLUS(unit)) {
        SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
            /* Enabling/Disabling shapers */
            res=READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
            soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, EMPTY_QP_STOP_COLLECTING_ENf, max_burst_empty_queues ? 1 : 0);
            res=WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
            /* Setting shapers */
            res=READ_EGQ_QP_MAX_CREDIT_VALUEr(unit, core,&reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
            soc_reg_field_set(unit, EGQ_QP_MAX_CREDIT_VALUEr, &reg_val, EMPTY_QP_MAX_CREDITf, max_burst_empty_queues);
            res=WRITE_EGQ_QP_MAX_CREDIT_VALUEr(unit, core, reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
        }
    }
    /*Max burst for empty and fc queues }*/

exit:
  DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_set_unsafe(
       DNX_SAND_IN  int    unit,
       DNX_SAND_IN  uint32 max_burst_fc_queues
       )
{
    uint32 res = DNX_SAND_NO_ERR,
           reg_val;
    int    core;

    DNXC_INIT_FUNC_DEFS

    /*Max burst for empty and fc queues {*/
    if (SOC_IS_ARADPLUS(unit)) {
        SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
            /* Enabling/Disabling shapers */
            res=READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
            soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, FC_QP_STOP_COLLECTING_ENf, max_burst_fc_queues ? 1 : 0);
            res=WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
             /* Setting shapers */
            res=READ_EGQ_QP_MAX_CREDIT_VALUEr(unit, core,&reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
            soc_reg_field_set(unit, EGQ_QP_MAX_CREDIT_VALUEr, &reg_val, FC_QP_MAX_CREDITf, max_burst_fc_queues);
            res=WRITE_EGQ_QP_MAX_CREDIT_VALUEr(unit, core, reg_val);
            DNXC_SAND_IF_ERR_EXIT(res);
        }
    }
    /*Max burst for empty and fc queues }*/


exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_fc_queues_get_unsafe(
       DNX_SAND_IN  int    unit,
       DNX_SAND_OUT  uint32 *max_burst_fc_queues
       )
{
    uint32 res = DNX_SAND_NO_ERR,
           reg_val;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst_fc_queues);

    /*Max burst for empty and fc queues {*/
    if (SOC_IS_ARADPLUS(unit)) {
        res=READ_EGQ_QP_MAX_CREDIT_VALUEr(unit, 0,&reg_val);
        DNXC_SAND_IF_ERR_EXIT(res);
        *max_burst_fc_queues = soc_reg_field_get(unit, EGQ_QP_MAX_CREDIT_VALUEr, reg_val, FC_QP_MAX_CREDITf);
    }
    /*Max burst for empty and fc queues }*/


exit:
  DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_port_priority_max_burst_for_empty_queues_get_unsafe(
       DNX_SAND_IN  int    unit,
       DNX_SAND_OUT  uint32 *max_burst_empty_queues
       )
{
    uint32 res = DNX_SAND_NO_ERR,
           reg_val;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst_empty_queues);

    /*Max burst for empty and fc queues {*/
    if (SOC_IS_ARADPLUS(unit)) {
        res=READ_EGQ_QP_MAX_CREDIT_VALUEr(unit, 0,&reg_val);
        DNXC_SAND_IF_ERR_EXIT(res);
        *max_burst_empty_queues = soc_reg_field_get(unit, EGQ_QP_MAX_CREDIT_VALUEr, reg_val, EMPTY_QP_MAX_CREDITf);
    }
    /*Max burst for empty and fc queues }*/

exit:
  DNXC_FUNC_RETURN
}

int
    jer2_arad_ofp_rates_egq_tcg_rate_sw_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core, 
    DNX_SAND_IN  uint32                 tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX           tcg_ndx,   
    DNX_SAND_IN  uint32                 tcg_rate
    )
{
    uint32 res = SOC_E_NONE,
           base_q_pair,
           ps;
    JER2_ARAD_SW_DB_DEV_RATE
           rate;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
#if 1
  if (egq_tcg_qpair_shaper_enable)
#endif
  {

        /*Getting the fap port's base_q_pair*/
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

        /*Getting ps*/
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
        ps = base_q_pair;
#endif 

        /*Setting rate*/
        rate.valid = TRUE;
        rate.egq_rates  = tcg_rate;
        
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.tcg_rate.set(
                unit,
                core,
                ps,
                tcg_ndx,
                &rate
              );
        DNXC_IF_ERR_EXIT(res);

  }
exit:
    DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_sw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                ptc_rate
    )
{
    uint32 res = SOC_E_NONE,
           base_q_pair;
    JER2_ARAD_SW_DB_DEV_RATE
           rate;
    uint8
        egq_tcg_qpair_shaper_enable;
 
    DNXC_INIT_FUNC_DEFS;

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
    if (egq_tcg_qpair_shaper_enable)
    {
        /*Getting the fap port's base_q_pair*/
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));

        /*Setting rate*/
        rate.valid = TRUE;
        rate.egq_rates  = ptc_rate;
        
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.queue_rate.set(
                unit,
                core,
                (base_q_pair + prio_ndx)/*q_pair*/,
                &rate
              );
        DNXC_IF_ERR_EXIT(res);
    }
exit:
    DNXC_FUNC_RETURN
}


int
    jer2_arad_ofp_rates_egq_tcg_rate_hw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID      tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_OUT  uint32               *tcg_rate
    )
{
    uint32 res = SOC_E_NONE,
           egq_cal_len,
           *egq_rates = NULL,
           base_q_pair,
           ps,
           tcg_id;

    JER2_ARAD_OFP_RATES_CAL_INFO 
           cal_info;

    JER2_ARAD_OFP_RATES_CAL_EGQ      
           egq_cal;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(tcg_rate);

    DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"egq_rates");
    jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(&cal_info);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
#if 1
  if (egq_tcg_qpair_shaper_enable)
#endif
  {

    cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_TCG;

    egq_cal_len = 0;
    /*Retrieve calendars*/
    res = jer2_arad_ofp_rates_active_generic_calendars_retrieve_egq(
            unit,
            core,
            &cal_info,
            &egq_cal,
            &egq_cal_len,
            FALSE
        );
    DNXC_IF_ERR_EXIT(res);

    /*Calculate egq rates*/
    res = jer2_arad_ofp_rates_from_generic_calendar_to_ports_egq_rate(
          unit,
          core,
          &cal_info,
          &egq_cal,
          egq_cal_len,
          FALSE,
          JER2_ARAD_EGR_NOF_Q_PAIRS,
          egq_rates
        );
    DNXC_IF_ERR_EXIT(res);
    /*Get the rate of the requested port*/
    res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
    DNXC_IF_ERR_EXIT(res);
    if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal port")));
    }
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
    ps = base_q_pair;
#endif 

    tcg_id = JER2_ARAD_OFP_RATES_TCG_ID_GET(ps,tcg_ndx);
    *tcg_rate = egq_rates[tcg_id];
  }
exit:
    DNXC_FREE(egq_rates);
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          ptc_ndx,   
    DNX_SAND_OUT uint32                *ptc_rate
    )
{
   uint32 res = SOC_E_NONE,
           egq_cal_len,
           *egq_rates = NULL,
           base_q_pair;

    JER2_ARAD_OFP_RATES_CAL_EGQ      
           egq_cal;
    JER2_ARAD_OFP_RATES_CAL_INFO
           cal_info;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(ptc_rate);

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);

    if (egq_tcg_qpair_shaper_enable)
    {
        DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"egq_rates");
        jer2_arad_JER2_ARAD_OFP_RATES_CAL_INFO_clear(&cal_info);
          
        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY;

        egq_cal_len = 0;
        /*Retrieve calendars*/
        res = jer2_arad_ofp_rates_active_generic_calendars_retrieve_egq(
                unit,
                core,
                &cal_info,
                &egq_cal,
                &egq_cal_len,
                FALSE
            );
        DNXC_IF_ERR_EXIT(res);
        
        /*Calculate egq rates*/
        res = jer2_arad_ofp_rates_from_generic_calendar_to_ports_egq_rate(
              unit,
              core,
              &cal_info,
              &egq_cal,
              egq_cal_len,
              FALSE,
              JER2_ARAD_EGR_NOF_Q_PAIRS,
              egq_rates
            );
        DNXC_IF_ERR_EXIT(res);
        /*Get the rate of the requested port*/
        res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair);
        DNXC_IF_ERR_EXIT(res);
        if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal port")));
        }
        *ptc_rate = egq_rates[base_q_pair + ptc_ndx];
    }

exit:
    DNXC_FREE(egq_rates);
    DNXC_FUNC_RETURN
}


int
    jer2_arad_ofp_rates_egq_tcg_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core
    )
{
    uint32 
        res = SOC_E_NONE,
        *egq_rates = NULL,
        egq_calendar_len = 0,
        fap_port_ndx,
        base_q_pair,
        nof_priorities,
        ps,
        recalc = TRUE,
        tcg_ndx,
        tcg_id,
        act_cal,
        sum_of_tcg_requested = 0,
        reg_val,
        port_i,
        flags;
    JER2_ARAD_OFP_RATES_CAL_EGQ
        *egq_calendar = NULL;
    JER2_ARAD_OFP_RATES_CAL_INFO
        cal_info; 
    JER2_ARAD_SW_DB_DEV_EGR_TCG
        rates;
    JER2_ARAD_OFP_RATES_CAL_SET
        cal2set;
    soc_pbmp_t
        pbmp;
    int
      core_i;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS;
  
    SOC_CLEAR(&cal_info, JER2_ARAD_OFP_RATES_CAL_INFO, 1);
    DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"egq_rates");
    DNXC_ALLOC(egq_calendar, JER2_ARAD_OFP_RATES_CAL_EGQ, 1,"egq_calendar");

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
#if 1
  if (egq_tcg_qpair_shaper_enable)
#endif
  {
        /*Get active calendar*/
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
        act_cal = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, TCG_SPR_SET_SELf);
        cal2set = (act_cal == JER2_ARAD_OFP_RATES_CAL_SET_A)?JER2_ARAD_OFP_RATES_CAL_SET_B:JER2_ARAD_OFP_RATES_CAL_SET_A;

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pbmp));

        SOC_PBMP_ITER(pbmp, port_i)
        {
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
          if (DNX_PORT_IS_STAT_INTERFACE(flags))
          {
              continue;
          }
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &fap_port_ndx, &core_i));
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, fap_port_ndx, &base_q_pair));
          if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
          {
            continue;
          }

          DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core_i, fap_port_ndx, &nof_priorities));

          DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
          ps = JER2_ARAD_BASE_PORT_TC2PS(base_q_pair);
#else 
          ps = base_q_pair;
#endif 


          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.tcg_cal.get(unit,core_i,&rates);
          DNXC_IF_ERR_EXIT(res);

          for (tcg_ndx = 0; tcg_ndx < nof_priorities; ++tcg_ndx)
          {
            if (rates.tcg_rate[ps][tcg_ndx].valid)
            {
                tcg_id = JER2_ARAD_OFP_RATES_TCG_ID_GET(ps,tcg_ndx);
                egq_rates[tcg_id] = rates.tcg_rate[ps][tcg_ndx].egq_rates;
                sum_of_tcg_requested += rates.tcg_rate[ps][tcg_ndx].egq_rates;
            }
          }
        }

        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_TCG;
        res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar(unit,
                                                                      core,
                                                                      &cal_info,
                                                                      egq_rates,
                                                                      JER2_ARAD_EGR_NOF_TCG_IDS,
                                                                      sum_of_tcg_requested,
                                                                      JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_TCG_MAX,
                                                                      recalc,
                                                                      FALSE,
                                                                      egq_calendar,
                                                                      &egq_calendar_len);
        DNXC_IF_ERR_EXIT(res);

        /* Write to device */
        res = jer2_arad_ofp_rates_active_egq_generic_calendars_config(unit,
                                                           core,
                                                           &cal_info,
                                                           egq_calendar,
                                                           egq_calendar_len);
        DNXC_IF_ERR_EXIT(res);

        /*Set active calendar*/
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, TCG_SPR_SET_SELf, cal2set);
        DNXC_SAND_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val));
  }

exit:
    DNXC_FREE(egq_rates);
    DNXC_FREE(egq_calendar);
    DNXC_FUNC_RETURN;
}



uint32
    jer2_arad_ofp_rates_egq_port_priority_rate_hw_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core
    )
{
    uint32 
        res = SOC_E_NONE,
        *egq_rates = NULL,
        egq_calendar_len = 0,
        fap_port_ndx,
        base_q_pair,
        nof_priorities,
        ptc_ndx,
        recalc = TRUE,
        act_cal,
        sum_of_ptc_requested = 0,
        port_i,
        reg_val,
        flags;
    JER2_ARAD_OFP_RATES_CAL_EGQ
        *egq_calendar = NULL;
    JER2_ARAD_OFP_RATES_CAL_INFO
        cal_info; 
    JER2_ARAD_SW_DB_DEV_EGR_PORT_PRIORITY
        rates;
    JER2_ARAD_OFP_RATES_CAL_SET
        cal2set;
    soc_pbmp_t
        pbmp;
    int
        core_i;
    uint8
        egq_tcg_qpair_shaper_enable;

    DNXC_INIT_FUNC_DEFS
  
    SOC_CLEAR(&cal_info, JER2_ARAD_OFP_RATES_CAL_INFO, 1);
    DNXC_ALLOC(egq_rates, uint32, JER2_ARAD_NOF_FAP_PORTS,"egq_rates");
    DNXC_ALLOC(egq_calendar, JER2_ARAD_OFP_RATES_CAL_EGQ, 1,"egq_calendar");

    /*Get active calendar*/
    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
    act_cal = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, reg_val, QPAIR_SPR_SET_SELf);
    cal2set = (act_cal == JER2_ARAD_OFP_RATES_CAL_SET_A)?JER2_ARAD_OFP_RATES_CAL_SET_B:JER2_ARAD_OFP_RATES_CAL_SET_A;

    res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.egq_tcg_qpair_shaper_enable.get(unit, &egq_tcg_qpair_shaper_enable);
    DNXC_IF_ERR_EXIT(res);
    if (egq_tcg_qpair_shaper_enable)
    {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_core_get(unit, core, 0, &pbmp));

        SOC_PBMP_ITER(pbmp, port_i)
        {
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
          if (DNX_PORT_IS_STAT_INTERFACE(flags))
          {
              continue;
          }
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &fap_port_ndx, &core_i));
          DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, fap_port_ndx, &base_q_pair));
          
          if (base_q_pair == JER2_ARAD_EGR_INVALID_BASE_Q_PAIR)
          {
            continue;
          }

          DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core_i, fap_port_ndx,  &nof_priorities));

          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_egr_ports.port_priority_cal.get(unit,core_i,&rates);

          DNXC_IF_ERR_EXIT(res);

          for (ptc_ndx = 0; ptc_ndx < nof_priorities; ++ptc_ndx)
          {
            if (rates.queue_rate[base_q_pair + ptc_ndx].valid)
            {
                egq_rates[base_q_pair + ptc_ndx] = rates.queue_rate[base_q_pair + ptc_ndx].egq_rates;
                sum_of_ptc_requested += rates.queue_rate[base_q_pair + ptc_ndx].egq_rates;
            }
          }
        }

        cal_info.cal_type = JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY;
        res = jer2_arad_ofp_rates_from_egq_ports_rates_to_generic_calendar(
              unit,
              core,
              &cal_info,
              egq_rates,
              JER2_ARAD_EGR_NOF_TCG_IDS,
              sum_of_ptc_requested,
              JER2_ARAD_OFP_RATES_CAL_LEN_EGQ_TCG_MAX,
              recalc,
              FALSE,
              egq_calendar,
              &egq_calendar_len  
            );
        DNXC_IF_ERR_EXIT(res);

        /* Write to device */
        res = jer2_arad_ofp_rates_active_egq_generic_calendars_config(unit,
                                                           core,
                                                           &cal_info,
                                                           egq_calendar,
                                                           egq_calendar_len);
        DNXC_IF_ERR_EXIT(res);

        /*Set active calendar*/
        DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &reg_val));
        soc_reg_field_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, &reg_val, QPAIR_SPR_SET_SELf, cal2set);
        DNXC_SAND_IF_ERR_EXIT(WRITE_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, reg_val));
    }

exit:
    DNXC_FREE(egq_rates);
    DNXC_FREE(egq_calendar);
    DNXC_FUNC_RETURN
}



static uint32
    jer2_arad_ofp_rates_tcg_ptc_sch_generic_set_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                octet_ndx,   
    DNX_SAND_IN  uint32                rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE func_state
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    switch (func_state) {
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST:
        res = jer2_arad_sch_port_priority_shaper_max_burst_set_unsafe(unit, core, tm_port, octet_ndx, rate);
        DNXC_IF_ERR_EXIT(res);
        break;
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE:
        res = jer2_arad_sch_port_priority_shaper_rate_set_unsafe(unit, core, tm_port, octet_ndx, rate);
        DNXC_SAND_IF_ERR_EXIT(res);
        break;
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE:
        res = jer2_arad_sch_tcg_shaper_rate_set_unsafe(unit, core, tm_port, octet_ndx,rate);
        DNXC_SAND_IF_ERR_EXIT(res);
        break;
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST:
        res = jer2_arad_sch_tcg_shaper_max_burst_set_unsafe(unit, core, tm_port, octet_ndx, rate);
        DNXC_IF_ERR_EXIT(res);
        break;
         
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
    }

exit:
    DNXC_FUNC_RETURN
}

static uint32
    jer2_arad_ofp_rates_tcg_ptc_sch_generic_get_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port,
    DNX_SAND_IN  uint32                octet_ndx,   
    DNX_SAND_OUT uint32                *rate,
    DNX_SAND_IN  JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE func_state
    )
{
    uint32 res = SOC_E_NONE;
    JER2_ARAD_SCH_TCG_RATE_INFO
        tcg_sch_rate_info;
    JER2_ARAD_SCH_PORT_PRIORITY_RATE_INFO
        ptc_sch_rate_info;


    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(rate);

    switch (func_state) {
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST:
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE:
        res = jer2_arad_sch_port_priority_shaper_rate_get_unsafe(
              unit, core,
              tm_port,
              octet_ndx,
              &ptc_sch_rate_info
            );
        DNXC_SAND_IF_ERR_EXIT(res);

        switch (func_state) {
        case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE:
             *rate = ptc_sch_rate_info.rate;
             break;
        case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST:
             *rate =  ptc_sch_rate_info.max_burst;
             break;
        default:
           DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
        }
        break;
    break;

    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE:
    case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST:
    
        res = jer2_arad_sch_tcg_shaper_rate_get_unsafe(
              unit, core,
              tm_port,
              octet_ndx,
              &tcg_sch_rate_info
            );
        DNXC_SAND_IF_ERR_EXIT(res);

        switch (func_state) {
        case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE:
             *rate = tcg_sch_rate_info.rate;
             break;
        case JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST:
             *rate = tcg_sch_rate_info.max_burst;
             break;
        /* must default. Oveerwise - compilation error */
        /* coverity[dead_error_begin:FALSE] */
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
        }
        break;
    /* must default. Oveerwise - compilation error */
    /* coverity[dead_error_begin:FALSE] */
    default:
       DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG_STR( "Illegal enum")));
    }

exit:
    DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_sch_tcg_rate_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_IN  uint32                tcg_rate
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_set_unsafe(unit, core,
                                                      tm_port, 
                                                      tcg_ndx,   
                                                      tcg_rate,
                                                      JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_IN  uint32                max_burst
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_set_unsafe(unit, core,
                                                              tm_port, 
                                                              tcg_ndx,   
                                                              max_burst,
                                                              JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_set_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                tcg_rate
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS


    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_set_unsafe(unit, core,
                                                              tm_port, 
                                                              prio_ndx,   
                                                              tcg_rate,
                                                              JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_sw_set_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                rate
    )
{
    uint32 base_port_tc, offset;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_port_tc));
    offset = base_port_tc + prio_ndx;
    DNXC_IF_ERR_EXIT(jer2_arad_sw_db_sch_priority_port_rate_set(unit, core, offset, rate, 1));
    
exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_IN  uint32                max_burst
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_set_unsafe(unit, core,
                                                              tm_port, 
                                                              prio_ndx,   
                                                              max_burst,
                                                              JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}


uint32
    jer2_arad_ofp_rates_sch_tcg_rate_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core,     
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_OUT uint32                *tcg_rate
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(tcg_rate);


    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_get_unsafe(unit,
                                                        core,
                                                        tm_port, 
                                                        tcg_ndx,   
                                                        tcg_rate,
                                                        JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_RATE);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_tcg_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  JER2_ARAD_TCG_NDX          tcg_ndx,   
    DNX_SAND_OUT uint32                *max_burst
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst);


    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_get_unsafe(unit,
                                                        core,
                                                        tm_port, 
                                                        tcg_ndx,   
                                                        max_burst,
                                                        JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_TCG_SCH_BURST);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_rate_get_unsafe(
    DNX_SAND_IN  int                   unit,  
    DNX_SAND_IN  int                   core,  
    DNX_SAND_IN  uint32                tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_OUT  uint32               *tcg_rate
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(tcg_rate);

    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_get_unsafe(unit,
                                                        core,
                                                        tm_port, 
                                                        prio_ndx,   
                                                        tcg_rate,
                                                        JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_RATE);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

uint32
    jer2_arad_ofp_rates_sch_port_priority_max_burst_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core, 
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID      tm_port, 
    DNX_SAND_IN  uint32                prio_ndx,   
    DNX_SAND_OUT  uint32               *max_burst
    )
{
    uint32 res = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS

    DNXC_NULL_CHECK(max_burst);

    res = jer2_arad_ofp_rates_tcg_ptc_sch_generic_get_unsafe(unit,
                                                        core,
                                                        tm_port, 
                                                        prio_ndx,   
                                                        max_burst,
                                                        JER2_ARAD_OFP_RATES_GENERIC_FUNC_STATE_PTC_SCH_BURST);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN
}

int
    jer2_arad_ofp_rates_packet_mode_packet_size_get (
        DNX_SAND_IN   int                   unit, 
        DNX_SAND_IN   int                   core,  
        DNX_SAND_OUT  uint32                *num_of_bytes
        )
{
    uint32 data;

    DNXC_INIT_FUNC_DEFS

    DNXC_SAND_IF_ERR_EXIT(READ_EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr(unit, core, &data));

    /* get packet size in bytes for packet mode shaping */
    *num_of_bytes = soc_reg_field_get(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, data, SHAPER_PACKET_RATE_CONSTf);

exit:
    DNXC_FUNC_RETURN
}

int
jer2_arad_ofp_rates_egress_shaper_cal_write (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_IN  uint32                                   data
    )
{
    soc_field_t field_name;
    soc_reg_t register_name;

    DNXC_INIT_FUNC_DEFS

    /* retrieve memory and field names */
    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
                                                                unit,
                                                                cal_info,
                                                                cal2set,    
                                                                field_type,
                                                                &register_name,
                                                                &field_name));

    /* write */
    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_egress_shaper_reg_field_write (
                                                              unit,
                                                              core,
                                                              cal_info,   
                                                              register_name,
                                                              field_name,
                                                              data));
exit:
    DNXC_FUNC_RETURN
}

int
jer2_arad_ofp_rates_egress_shaper_cal_read (
    DNX_SAND_IN  int                                      unit,
    DNX_SAND_IN  int                                      core,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_INFO              *cal_info,
    DNX_SAND_IN  DNX_TMC_OFP_RATES_CAL_SET                cal2set,    
    DNX_SAND_IN  DNX_TMC_OFP_RATES_EGQ_CHAN_ARB_FIELD_TYPE field_type,
    DNX_SAND_OUT uint32                                   *data
    )
{
    soc_field_t field_name;
    soc_reg_t register_name;

    DNXC_INIT_FUNC_DEFS

    /* retrieve memory and field names */
    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_retrieve_egress_shaper_reg_field_names(
                                                                unit,
                                                                cal_info,
                                                                cal2set,    
                                                                field_type,
                                                                &register_name,
                                                                &field_name));

    /* write */
    DNXC_IF_ERR_EXIT(jer2_arad_ofp_rates_egress_shaper_reg_field_read (
                                                              unit,
                                                              core,
                                                              cal_info,   
                                                              register_name,
                                                              field_name,
                                                              data));
exit:
    DNXC_FUNC_RETURN
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

