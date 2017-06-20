/* $Id: jer2_arad_sw_db.h,v 1.102 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright
 * $
*/


#ifndef _JER2_ARAD_SW_DB_H_
/* { */
#define _JER2_ARAD_SW_DB_H_

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/sw_state.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/multicast.h>
#include <soc/dnx/legacy/multicast_imp.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_api_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_cell.h>
#include <soc/dnx/legacy/ARAD/arad_tdm.h>
#include <soc/dnx/legacy/SAND/Utils/sand_occupation_bitmap.h>
#include <soc/dnx/legacy/SAND/Utils/sand_hashtable.h>
#include <soc/dnx/legacy/SAND/Utils/sand_hashtable.h>
#include <soc/dnx/legacy/SAND/Utils/sand_sorted_list.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_traffic_mgmt.h>

#include <soc/dnx/legacy/SAND/Utils/sand_multi_set.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/JER/jer_fabric.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */


typedef uint16 JER2_ARAD_SYSPORT; /* type for system physical port */
#define JER2_ARAD_NOF_MODPORT (JER2_ARAD_NOF_FAPS_IN_SYSTEM*JER2_ARAD_NOF_FAP_PORTS) /* number of devices x number of device ports 2048*256=512K */
#define JER2_ARAD_MODPORT2SYSPORT_SIZE (sizeof(JER2_ARAD_SYSPORT[JER2_ARAD_NOF_MODPORT]))
#define JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND ((uint32)(-1)) /* return value of jer2_arad_sw_db_modport2sysport_reverse_get when it fails the search */
#define JER2_ARAD_SW_DB_SYSPORT2MODPORT_INVALID_ID ((uint32)(-1)) /* return value of jer2_arad_sw_db_sysport2modport_get when it fails the search */
#define JER2_ARAD_SW_DB_MODPORT2SYSPORT_INVALID_SYSPORT 0xffff /*Mark invalid sysport in the SW DB by using all-ones for JER2_ARAD_SYSPORT types - uint16*/


#define JER2_ARAD_SW_DB_NOF_QUEUE_TYPES(unit) SOC_DNX_DEFS_GET(unit, nof_credit_request_profiles)

#ifdef BCM_JER2_JERICHO_SUPPORT
#define JER2_ARAD_SW_DB_NOF_DYNAMIC_QUEUE_TYPES (SOC_DNX_DEFS_MAX(NOF_CREDIT_REQUEST_PROFILES) - DNX_TMC_ITM_NOF_QT_STATIC)
#define JER2_ARAD_SW_DB_NOF_LEGAL_DYNAMIC_QUEUE_TYPES(unit) (SOC_IS_JERICHO(unit) ? JER2_ARAD_SW_DB_NOF_DYNAMIC_QUEUE_TYPES : 13)
#else
#define JER2_ARAD_SW_DB_NOF_DYNAMIC_QUEUE_TYPES 13
#define JER2_ARAD_SW_DB_NOF_LEGAL_DYNAMIC_QUEUE_TYPES(unit) JER2_ARAD_SW_DB_NOF_DYNAMIC_QUEUE_TYPES
#endif

#define JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE 255

/*ingress rate class mode representing undifed mode*/
#define JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING_IS_ADVANCED 0x0
#define JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING_IS_SIMPLE 0x1
#define JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING_IS_UNDEFINED 0x2

#define JER2_ARAD_SW_DB_CORE_ANY (-1)
#define JER2_ARAD_SW_DB_CORE_0	(0)
#define JER2_ARAD_SW_DB_CORE_1	(1)

#define JER2_ARAD_EGQ_NOF_IFCS             32
/* } */

/*************
 * GLOBALS   *
 *************/


/*************
 *  MACROS   *
 *************/
/* { */

#define JER2_ARAD_SW_DB_INIT_DEFS                    \
    uint32                                    \
    dnx_sand_ret = DNX_SAND_OK

#define JER2_ARAD_SW_DB_FIELD_SET(res, unit, field_name, val)           \
    if (Jer2_arad_sw_db.jer2_arad_device_sw_db[unit] == NULL)                \
    {                                                                   \
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 1, exit);                         \
    }                                                                   \
    res = dnx_sand_os_memcpy(&(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]->field_name), \
                         val,                                           \
                         sizeof(*val)                                   \
                         );                                             \
    DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);


#define JER2_ARAD_SW_DB_FIELD_GET(unit, field_name, val)         \
  if (Jer2_arad_sw_db.jer2_arad_device_sw_db[unit] == NULL)          \
  {                                                               \
    return DNX_SAND_ERR;                                              \
  }                                                               \
  dnx_sand_ret = dnx_sand_os_memcpy(                                      \
      val,                                                        \
      &(Jer2_arad_sw_db.jer2_arad_device_sw_db[unit]->field_name),   \
      sizeof(*val)                                                \
    );                                                            \
  return dnx_sand_ret;

#define JER2_ARAD_SW_DB_GLOBAL_FIELD_SET(field_name, val)             \
  uint32                                                        \
    dnx_sand_ret = DNX_SAND_OK;                                           \
  dnx_sand_ret = dnx_sand_os_memcpy(                                      \
      &(Jer2_arad_sw_db.field_name),                                  \
      val,                                                        \
      sizeof(*val)                                                \
    );                                                            \
                                                                  \
  return dnx_sand_ret;

#define JER2_ARAD_SW_DB_GLOBAL_FIELD_GET(field_name, val)             \
  uint32                                                        \
    dnx_sand_ret = DNX_SAND_OK;                                           \
  dnx_sand_ret = dnx_sand_os_memcpy(                                      \
      val,                                                        \
      &(Jer2_arad_sw_db.field_name),                                  \
      sizeof(*val)                                                \
    );                                                            \
                                                                  \
  return dnx_sand_ret;

/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
  uint8
    valid;
  /* EGQ rate shaper */
  uint32
    egq_rates;
  /* EGQ burst max */
  uint32
    egq_bursts;
} JER2_ARAD_SW_DB_DEV_RATE;

typedef struct
{
  /* 
   * EGQ rate for each TCG entity. Summerize by NOF_PS * NOF_TCGS_IN_PS
   */
  JER2_ARAD_SW_DB_DEV_RATE
    tcg_rate[JER2_ARAD_EGR_NOF_PS][JER2_ARAD_NOF_TCGS];

} JER2_ARAD_SW_DB_DEV_EGR_TCG;

typedef struct
{
  /* 
   * EGQ rate for each Q-Pair
   */
  JER2_ARAD_SW_DB_DEV_RATE
    queue_rate[JER2_ARAD_EGR_NOF_Q_PAIRS];

} JER2_ARAD_SW_DB_DEV_EGR_PORT_PRIORITY;

typedef struct
{
  uint8
    valid;
  uint32
    sch_rates;
  uint32
    egq_rates;
  uint32
    egq_bursts;
}  JER2_ARAD_SW_DB_DEV_EGR_RATE;


typedef struct
{
    int
        priority_shaper_rate;
    uint8
        valid;

} JER2_ARAD_SW_DB_DEV_EGR_SCH_PORT_PRIORITY_SHAPER;  

typedef struct
{
    int
        tcg_shaper_rate;
    uint8
        valid;
    

} JER2_ARAD_SW_DB_DEV_EGR_SCH_TCG_SHAPER;  

typedef struct 
{
  uint32
    nof_calcal_instances;

} JER2_ARAD_SW_DB_DEV_EGR_CHAN_ARB;

typedef struct 
{
 /* 
  * Indicates if each (egress) local port has a reassembly context reserved for it
  * for a non mirroring application. 1 means reserved.
  */
  uint32
    port_reserved_reassembly_context[SOC_DNX_DEFS_MAX(NOF_LOGICAL_PORTS) / DNX_SAND_NOF_BITS_IN_UINT32];

  /*  
   * Calendar information of Port-Priority
   */
  JER2_ARAD_SW_DB_DEV_EGR_PORT_PRIORITY
    port_priority_cal[SOC_DNX_DEFS_MAX(NOF_CORES)];

  /* 
   * Calendar information of TCG
   */
  JER2_ARAD_SW_DB_DEV_EGR_TCG
    tcg_cal[SOC_DNX_DEFS_MAX(NOF_CORES)];

  JER2_ARAD_SW_DB_DEV_EGR_RATE
    rates[SOC_DNX_DEFS_MAX(NOF_CORES)][JER2_ARAD_EGR_NOF_BASE_Q_PAIRS];

  JER2_ARAD_SW_DB_DEV_EGR_SCH_PORT_PRIORITY_SHAPER
    port_priority[SOC_DNX_DEFS_MAX(NOF_CORES)][JER2_ARAD_EGR_NOF_BASE_Q_PAIRS];

  JER2_ARAD_SW_DB_DEV_EGR_CHAN_ARB
    chan_arb[SOC_DNX_DEFS_MAX(NOF_CORES)][SOC_DNX_DEFS_MAX(NOF_CHANNELIZED_CALENDARS)];

  uint32
    calcal_length[SOC_DNX_DEFS_MAX(NOF_CORES)];

  /* 
   * Disable / Enable EGQ TCG and QPair shapers
   */
  uint8
    egq_tcg_qpair_shaper_enable;

  /* 
   * ERP interface ID (taken from NIF avaiable interface)
   */
  JER2_ARAD_INTERFACE_ID
    erp_interface_id[SOC_DNX_DEFS_MAX(NOF_CORES)];


  JER2_ARAD_SW_DB_DEV_EGR_SCH_TCG_SHAPER
    tcg_shaper[SOC_DNX_DEFS_MAX(NOF_CORES)][JER2_ARAD_EGR_NOF_BASE_Q_PAIRS];

  /* 
   * Bimtap occupation to allocate channelize arbiter for requested interface
   */
  DNX_SAND_OCC_BM_PTR
    chanif2chan_arb_occ;
  /* 
   * Bimtap occupation to allocate non channelize arbiter for requested scheduler interfaces
   */
  DNX_SAND_OCC_BM_PTR
    nonchanif2sch_offset_occ;

  /*
   * Bitmap occupation to allocate channelized calendars
   */
  DNX_SAND_OCC_BM_PTR
    channelized_cals_occ[SOC_DNX_DEFS_MAX(NOF_CORES)];

  /*
   * indicates which calendars were modified (which cals should be recalculated) 
   */
  DNX_SAND_OCC_BM_PTR
    modified_channelized_cals_occ[SOC_DNX_DEFS_MAX(NOF_CORES)];

  /*
   * Bitmap occupation to allocate e2e interfaces
   */
  DNX_SAND_OCC_BM_PTR
    e2e_interfaces_occ[SOC_DNX_DEFS_MAX(NOF_CORES)];

  /*
   * indicates which e2e interfaces were modified (which cals should be recalculated) 
   */
  DNX_SAND_OCC_BM_PTR
    modified_e2e_interfaces_occ[SOC_DNX_DEFS_MAX(NOF_CORES)];

} JER2_ARAD_DEV_EGR_PORTS;

/* } */

typedef struct
{
  /*
   *  If TRUE, soc_petrab devices exist in the system. This imposes
   *  certain limitations on Arad behavior (e.g. TDM optimize
   *  cells must be fixed size etc.).
   */
  uint8 is_petrab_in_system;
  /*
   * TDM traffic mode and FTMH format configuration
   */
  JER2_ARAD_MGMT_TDM_MODE tdm_mode;
  /*
   * ILKN TDM SP mode - does tdm get presedence over regular packets 
   */
  JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;

  /* 
   * Multicast table mode
   */

}  JER2_ARAD_SW_DB_OP_MODE;

typedef struct
{

 /* 
  * mapped fap id
  */
  uint32 fap_id;
 /*
  *  mapped fap port
  */
  uint32 fap_port_id;

}  JER2_ARAD_MODPORT_INFO;

typedef struct
{
  /*  Buffers and index for FIFO read results. */
  uint32 *host_buff[DNX_TMC_CNT_NOF_PROCESSOR_IDS_JER2_ARAD]; 
  uint32 buff_line_ndx[DNX_TMC_CNT_NOF_PROCESSOR_IDS_JER2_ARAD]; 
}  JER2_ARAD_SW_DB_CNT;
/*
 *  SW-DB
 */

typedef enum
{
  JER2_ARAD_NOF_SW_DB_MULTI_SETS
} JER2_ARAD_SW_DB_MULTI_SET;

typedef struct
{
  uint32 vsi_to_isid[32*1024];
} JER2_ARAD_SW_DB_VSI;

typedef struct
{
    uint8  is_simple_mode;
    uint32 ref_count[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_ITM_NOF_RATE_CLASSES]; /*Number of queues pointing to rate class at offset i*/
    uint32 ocb_only_ref_count[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_ITM_NOF_RATE_CLASSES]; /*Number of OCB-only queues pointing to rate class at offset i*/
} JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING;

typedef struct 
{
    uint8  valid_flags;
    uint32 base_queue;
} JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE;


/* supported values for the credit_watchdog_mode field, must be the same as in include/bcm/fabric.h */
#define CREDIT_WATCHDOG_UNINITIALIZED 0 /* BCM_FABRIC_WATCHDOG_QUEUE_DISABLE */
/* BCM_FABRIC_WATCHDOG_QUEUE_ENABLE_NORMAL the default mode supporting both fsm and aging (queue deletion) */
#define CREDIT_WATCHDOG_NORMAL_MODE 1
/* BCM_FABRIC_WATCHDOG_QUEUE_ENABLE_FAST_STATUS_MESSAGE support aggressive (small)
   watchdog status message thresholds and not support aging (queue deletion) */
#define CREDIT_WATCHDOG_FAST_STATUS_MESSAGE_MODE 2
/* BCM_FABRIC_WATCHDOG_QUEUE_ENABLE_COMMON_STATUS_MESSAGE Arad+ mode whic common FSM time for the chip */
#define CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE 3

typedef struct 
{
    /* 
     * supported values are: 
     *  CREDIT_WATCHDOG_AGGRESSIVE_STATUS_MSG_MODE,
     *  CREDIT_WATCHDOG_NORMAL, 
     *  CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE, 
     *  CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE+1, The last value means generate a message after two scans
     */ 
    int8 credit_watchdog_mode;
    /* exact credit watchdog scan time in nano seconds */
    uint32 exact_credit_watchdog_scan_time_nano;
} JER2_ARAD_SW_DB_CREDIT_WATCHDOG;

typedef struct
{
  uint8  is_power_saving_called;
  JER2_ARAD_SW_DB_QUEUE_TO_RATE_CLASS_MAPPING queue_to_rate_class_mapping;
  PARSER_HINT_ARR JER2_ARAD_SW_DB_SYSPORT_TO_BASE_QUEUE* sysport2basequeue;
  PARSER_HINT_ARR JER2_ARAD_SYSPORT*                     queuequartet2sysport;
  int groups_bw[SOC_DNX_DEFS_MAX(NOF_CORES)][JER2_ARAD_SCH_NOF_GROUPS];
  int hr_group_bw[SOC_DNX_DEFS_MAX(NOF_CORES)][JER2_ARAD_EGR_NOF_BASE_Q_PAIRS];
  int rcy_single_context_port[SOC_DNX_DEFS_MAX(NOF_CORES)][SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES)];
  uint32 rcy_reassembly_ctxt[SOC_DNX_DEFS_MAX(NOF_CORES)][SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES)];
  int rcy_channels_to_egr_nif_mapping[SOC_DNX_DEFS_MAX(NOF_CORES)][SOC_DNX_MAX_NOF_CHANNELS];
  uint8 pg_numq[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_ITM_JER2_JERICHO_VSQ_GROUPF_SZE];
  JER2_ARAD_SW_DB_CREDIT_WATCHDOG credit_watchdog;
  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE ipf_config_mode;
}JER2_ARAD_SW_DB_TM;


typedef struct
{
  JER2_ARAD_SW_DB_CNT
    cnt;
}  JER2_ARAD_SW_DB_DEVICE;

/* NEW SW STATE STRUCT */
typedef struct soc_jer2_arad_sw_state_tm_s{
    PARSER_HINT_PTR JER2_ARAD_SW_DB_OP_MODE         *op_mode;
    PARSER_HINT_PTR JER2_JER_MODID_GROUP_MAP        *jer2_jer_modid_group_map;
    PARSER_HINT_PTR soc_dnx_config_jer2_arad_plus_t *jer2_arad_plus;
    PARSER_HINT_PTR JER2_ARAD_SW_DB_TM              *tm_info;
    PARSER_HINT_PTR JER2_ARAD_DEV_EGR_PORTS         *jer2_arad_egr_ports;
    PARSER_HINT_PTR JER2_ARAD_CELL                  *cell;
    PARSER_HINT_PTR JER2_ARAD_TDM                   *tdm;
    PARSER_HINT_PTR JER2_ARAD_MULTICAST             *jer2_arad_multicast;
    PARSER_HINT_ARR uint32                     *q_type_ref_count;
    /* maps user defined queue types to (the smaller amount of) hardware types (credit request profiled) */
    PARSER_HINT_ARR uint8                      *q_type_map;
    PARSER_HINT_PTR JER2_ARAD_SW_DB_VSI             *vsi;
    /* Array representing the modport to sysport mapping */
    PARSER_HINT_ARR JER2_ARAD_SYSPORT               *modport2sysport;
    /* Array representing the sysport to modport mapping */
    PARSER_HINT_ARR JER2_ARAD_MODPORT_INFO          *sysport2modport;
    PARSER_HINT_PTR JER2_ARAD_CHIP_DEFINITIONS      *chip_definitions;
    PARSER_HINT_ARR PARSER_HINT_ALLOW_WB_ACCESS dnx_phy_port_sw_db_t       *phy_ports_info;
    PARSER_HINT_ARR PARSER_HINT_ALLOW_WB_ACCESS dnx_logical_port_sw_db_t   *logical_ports_info;
    PARSER_HINT_PTR DNX_TMC_REASSBMEBLY_CTXT      *reassembly_ctxt;
    /* book keeping of the guaranteed resource for VOQs */
    PARSER_HINT_ARR soc_dnx_guaranteed_q_resource_t     *guaranteed_q_resource;
} soc_jer2_arad_sw_state_tm_t;

typedef struct
{
  JER2_ARAD_SW_DB_DEVICE*
    jer2_arad_device_sw_db[DNX_SAND_MAX_DEVICE];
}  JER2_ARAD_SW_DB;

extern uint8 Jer2_arad_sw_db_initialized;
extern JER2_ARAD_SW_DB_DEVICE **_jer2_arad_device_sw_db;

/*************
 * FUNCTIONS *
 *************/
/* { */

/********************************************************************************************
 * Initialization
 * {
 ********************************************************************************************/
uint32
  jer2_arad_sw_db_init(void);

void
  jer2_arad_sw_db_close(void);

/*
 *  Per-device software database initializations
 */
int
    jer2_arad_sw_db_sw_state_alloc(
        DNX_SAND_IN int     unit
  );

int
    jer2_arad_sw_db_sw_state_free(
        DNX_SAND_IN int     unit
  );

uint32
  jer2_arad_sw_db_device_init(
    DNX_SAND_IN int     unit
  );

/*
 *  Per-device software database closing
 */
uint32
  jer2_arad_sw_db_device_close(
    DNX_SAND_IN int unit
  );

/*********************************************************************************************
 * }
 * jer2_arad_egr_ports
 * {
 *********************************************************************************************/

uint32
  jer2_arad_sw_db_egr_ports_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  int                     core,
    DNX_SAND_IN  uint32                  base_q_pair,
    DNX_SAND_OUT JER2_ARAD_SW_DB_DEV_EGR_RATE *val
  );

/*********************************************************************************************
 * }
 * jer2_arad_scheduler
 * {
 *********************************************************************************************/

uint32
  jer2_arad_sw_db_sch_max_expected_port_rate_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 port_ndx,
    DNX_SAND_IN uint32  rate
  );

uint32
  jer2_arad_sw_db_sch_max_expected_port_rate_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 port_ndx
  );

uint32
  jer2_arad_sw_db_sch_accumulated_grp_port_rate_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 grp_ndx,
    DNX_SAND_IN uint32  rate
  );

uint32
  jer2_arad_sw_db_sch_accumulated_grp_port_rate_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 grp_ndx
  );

/*********************************************************************************************
 * }
 * jer2_arad_multicast
 * {
 *********************************************************************************************/

/* Mark the given egress multicast group as open or not in SWDB */
uint32 jer2_arad_sw_db_egress_group_open_set(
    DNX_SAND_IN  int     unit, /* device */
    DNX_SAND_IN  uint32  group_id,  /* multicast ID */
    DNX_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
);

/* Mark all egress multicast groups as open or not in SWDB */
uint32 jer2_arad_sw_db_egress_group_open_set_all(
    DNX_SAND_IN  int     unit, /* device */
    DNX_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
);

uint32
  jer2_arad_sw_db_op_mode_init(
    DNX_SAND_IN int unit
  );

void
  jer2_arad_sw_db_is_petrab_in_system_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint8 is_petrab_in_system
  );

uint8
  jer2_arad_sw_db_is_petrab_in_system_get(
    DNX_SAND_IN int unit
  );


void
  jer2_arad_sw_db_tdm_mode_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_MGMT_TDM_MODE tdm_mode
  );

JER2_ARAD_MGMT_TDM_MODE
  jer2_arad_sw_db_tdm_mode_get(
    DNX_SAND_IN int unit
  );


void
  jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_set(
     DNX_SAND_IN int unit,
     DNX_SAND_IN JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing
  );

JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE
  jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_get(
     DNX_SAND_IN int unit
  );

/*
 * Cnt
 */
uint32
  jer2_arad_sw_db_cnt_buff_and_index_set(
    DNX_SAND_IN int                     unit,
    DNX_SAND_IN uint16                     proc_id,
    DNX_SAND_IN uint32                     *buff,
    DNX_SAND_IN uint32                     index
  );
uint32
  jer2_arad_sw_db_cnt_buff_and_index_get(
    DNX_SAND_IN int                     unit,
    DNX_SAND_IN uint16                     proc_id,
    DNX_SAND_OUT uint32                     **buff,
    DNX_SAND_OUT uint32                    *index
   );
  
uint32
  jer2_arad_sw_db_dram_deleted_buff_list_add(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff
  );

uint32
  jer2_arad_sw_db_dram_deleted_buff_list_remove(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff
  );
 
int
  jer2_arad_sw_db_dram_deleted_buff_list_get(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32     buff,
    DNX_SAND_OUT uint32*     is_deleted
  ); 
 
int 
  jer2_arad_sw_db_dram_deleted_buff_list_get_all(
    DNX_SAND_IN int    unit,
    DNX_SAND_OUT uint32*    buff_list_arr,
    DNX_SAND_IN uint32      arr_size,
    DNX_SAND_OUT uint32*    buff_list_num);
  
/*
 * check/set if a (egress) local port has a reassembly context reserved for it
 * for a non mirroring application. 
 */
uint32
  jer2_arad_sw_db_is_port_reserved_for_reassembly_context(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32  local_port,
    DNX_SAND_OUT uint8  *is_reserved /* returns one of: 0 for not reserved, 1 for reserved */
  );

uint32
  jer2_arad_sw_db_set_port_reserved_for_reassembly_context(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32  local_port,
    DNX_SAND_IN uint8   reserve /* 0 will cancel reservation, other values will reserve */
  );

DNX_SAND_OCC_BM_PTR
  jer2_arad_sw_db_egr_ports_nonchanif2sch_offset_occ_get(
    DNX_SAND_IN int unit
  );

DNX_SAND_OCC_BM_PTR
  jer2_arad_sw_db_egr_e2e_interfaces_occ_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core
  );

uint32
  jer2_arad_sw_db_sysport2queue_set(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN JER2_ARAD_SYSPORT    sysport,
    DNX_SAND_IN uint8           valid,
    DNX_SAND_IN uint8           sw_only,
    DNX_SAND_IN uint32          base_queue
   );
uint32
  jer2_arad_sw_db_sysport2queue_get(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core_id,
    DNX_SAND_IN  JER2_ARAD_SYSPORT    sysport,
    DNX_SAND_OUT uint8          *valid,
    DNX_SAND_OUT uint8          *sw_only,
    DNX_SAND_OUT uint32         *base_queue
   );

uint32
  jer2_arad_sw_db_queuequartet2sysport_set(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN uint32          queue_quartet,
    DNX_SAND_IN JER2_ARAD_SYSPORT    sysport
   );

uint32
  jer2_arad_sw_db_queuequartet2sysport_get(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN uint32          core_id,
    DNX_SAND_IN uint32          queue_quartet,
    DNX_SAND_OUT JER2_ARAD_SYSPORT    *sysport
   );

/*
 * SW DB multi-sets {
 */
uint32
  jer2_arad_sw_db_buffer_set_entry(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_INOUT  uint8                           *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_IN  uint8                              *data
  );

uint32
  jer2_arad_sw_db_buffer_get_entry(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  uint32                             sec_hanlde,
    DNX_SAND_IN  uint8                              *buffer,
    DNX_SAND_IN  uint32                             offset,
    DNX_SAND_IN  uint32                             len,
    DNX_SAND_OUT uint8                              *data
  );

uint32
  jer2_arad_sw_db_multiset_add(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int				      core_id,
	DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 *val,
    DNX_SAND_OUT  uint32                *data_indx,
    DNX_SAND_OUT  uint8               *first_appear,
    DNX_SAND_OUT  DNX_SAND_SUCCESS_FAILURE    *success
  );

uint32
  jer2_arad_sw_db_multiset_remove(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 *val,
    DNX_SAND_OUT  uint32                *data_indx,
    DNX_SAND_OUT  uint8               *last_appear
  );

uint32
  jer2_arad_sw_db_multiset_lookup(
    DNX_SAND_IN  int       unit,
	DNX_SAND_IN	 int	   core_id,
	DNX_SAND_IN  uint32       multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32        *val,
    DNX_SAND_OUT  uint32       *data_indx,
    DNX_SAND_OUT  uint32       *ref_count
  );

uint32
  jer2_arad_sw_db_multiset_add_by_index(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   			core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 *val,
    DNX_SAND_OUT  uint32                data_indx,
    DNX_SAND_OUT  uint8               *first_appear,
    DNX_SAND_OUT  DNX_SAND_SUCCESS_FAILURE    *success
  );

uint32
  jer2_arad_sw_db_multiset_remove_by_index(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   core_id,
    DNX_SAND_IN  uint32                multiset_type, /* JER2_ARAD_SW_DB_MULTI_SET */
    DNX_SAND_IN  uint32                 data_indx,
    DNX_SAND_OUT  uint8               *last_appear
  );

uint32
  jer2_arad_sw_db_multiset_clear(
    DNX_SAND_IN  int                unit,
	DNX_SAND_IN	 int	   core_id,
    DNX_SAND_IN  uint32                multiset_type /* JER2_ARAD_SW_DB_MULTI_SET */
  );

uint32
  jer2_arad_sw_db_multiset_get_enable_bit(
    DNX_SAND_IN  int                	unit,
    DNX_SAND_IN  uint32                	core_id, /* JER2_ARAD_SW_DB_MULTI_SET */
	DNX_SAND_IN  uint32					tbl_offset,
	DNX_SAND_OUT uint8					*enable
  );

/*
 * } Configuration
 */

/*********************************************************************************************
* }
* jer2_arad interrupt
* {
*********************************************************************************************/

uint8
  jer2_arad_sw_db_interrupt_mask_on_get(
    DNX_SAND_IN  int                        unit
  );

void
  jer2_arad_sw_db_interrupt_mask_on_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint8                        val
  );

/* } */


int jer2_arad_sw_db_sw_dump(int unit);


/*
Set a device x port to system physical port mapping.
Performs allocation inside the data structure if needed.
*/
uint32 jer2_arad_sw_db_modport2sysport_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport
  );

/*
Set a system physical port to device x port mapping.
Performs allocation inside the data structure if needed.
*/
uint32 jer2_arad_sw_db_sysport2modport_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id
  );

/*
Get a device x port to system physical port mapping.
If the mapping does not exist, the value of JER2_ARAD_NOF_SYS_PHYS_PORTS is returned
*/
uint32 jer2_arad_sw_db_modport2sysport_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id,
    DNX_SAND_OUT JER2_ARAD_SYSPORT *sysport
  );

/*
Get system physical port to device x port mapping.
*/
uint32 jer2_arad_sw_db_sysport2modport_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_OUT uint32 *fap_id,
    DNX_SAND_OUT uint32 *fap_port_id
  );

/*
Get a reverse system physical port to device x port mapping.
Works by searching the mapping till finding the system physical port.
If the mapping does not exist, the value of JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND is returned
*/
uint32 jer2_arad_sw_db_modport2sysport_reverse_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_SYSPORT sysport,
    DNX_SAND_OUT uint32 *fap_id,
    DNX_SAND_OUT uint32 *fap_port_id
  );

/*
Remove a device x port to system physical port mapping.
*/
uint32 jer2_arad_sw_db_modport2sysport_remove(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 fap_id,
    DNX_SAND_IN uint32 fap_port_id
  );

/* 
 *  Move a credit request profile-queue mapping from orig_q_type to new_q_type, 
 *  if core==SOC_CORE_ALL than we move nof_active_cores reference count.
 */ 
uint32
  jer2_arad_sw_db_queue_type_ref_count_exchange(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  int            core,
    DNX_SAND_IN  uint8          orig_q_type,
    DNX_SAND_IN  uint8          new_q_type,
    DNX_SAND_IN  int            nof_additions);

/* Get the hardware queue type mapped to from the user queue type. Returns JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE in mapped_q_type if not found */
uint32
  jer2_arad_sw_db_queue_type_map_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          user_q_type,  /* input user queue type (predefined type or user defined allocated type) */
    DNX_SAND_OUT uint8*         mapped_q_type /* output hardware queue type, 0 if not mapped */
  );

/*
 * Get the hardware queue type mapped to from the user queue type, allocating it if it was not allocated before.
 * Returns JER2_ARAD_SW_DB_QUEUE_TYPE_NOT_AVAILABLE in mapped_q_type if mapping is not possible since all hardware types (credit request profiles) are used.
 * If given a predefined queue type, will just return it as output as it does not use dynamic allocation.
 */
uint32
  jer2_arad_sw_db_queue_type_map_get_alloc(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          user_q_type,  /* input user queue type (predefined type or user defined allocated type) */
    DNX_SAND_OUT uint8*         mapped_q_type /* output hardware queue type, 0 if not mapped */
  );

/* Get the user queue type mapped from the given hardware queue type. */
uint32
  jer2_arad_sw_db_queue_type_map_reverse_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8          mapped_q_type,  /* input hardware queue type, 0 if not mapped */
    DNX_SAND_OUT uint8*         user_q_type     /* output user queue type (predefined type or user defined allocated type) */
  );

uint32
  jer2_arad_sw_db_is_port_valid_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_OUT uint8             *is_valid
   );
uint32
  jer2_arad_sw_db_is_port_valid_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint8             is_valid
   );
uint32
  jer2_arad_sw_db_egq_port_rate_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint32            rate
   );
uint32
  jer2_arad_sw_db_egq_port_rate_get(
   DNX_SAND_IN   int               unit,
   DNX_SAND_IN   int               core,
   DNX_SAND_IN   uint32            base_q_pair,
   DNX_SAND_OUT  uint32            *rate
   );
uint32
  jer2_arad_sw_db_sch_port_rate_set(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_IN  uint32            rate
   );
uint32
  jer2_arad_sw_db_sch_port_rate_get(
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            base_q_pair,
   DNX_SAND_OUT uint32            *rate
   );

uint32
  jer2_arad_sw_db_sch_priority_port_rate_set( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_IN  uint32            rate,
   DNX_SAND_IN  uint8             valid
   );
uint32
  jer2_arad_sw_db_sch_priority_port_rate_get( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_OUT int               *rate,
   DNX_SAND_OUT uint8             *valid
   );

uint32
  jer2_arad_sw_db_sch_port_tcg_rate_set( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_IN  uint32            rate,
   DNX_SAND_IN  uint8             valid
   );

uint32
  jer2_arad_sw_db_sch_port_tcg_rate_get( 
   DNX_SAND_IN  int               unit,
   DNX_SAND_IN  int               core,
   DNX_SAND_IN  uint32            offset,
   DNX_SAND_OUT int               *rate,
   DNX_SAND_OUT uint8             *valid
   );
  
uint32 
    jer2_arad_sw_db_rate_class_ref_count_get(
       DNX_SAND_IN  int                         unit, 
       DNX_SAND_IN  int                         core_id, 
       DNX_SAND_IN  uint32                      is_ocb_only, 
       DNX_SAND_IN  uint32                      rate_class, 
       DNX_SAND_OUT uint32*                     ref_count);
uint32 
    jer2_arad_sw_db_tm_queue_to_rate_class_mapping_ref_count_exchange(
       DNX_SAND_IN  int                         unit,
       DNX_SAND_IN  int                         core, 
       DNX_SAND_IN  uint32                      is_ocb_only,
       DNX_SAND_IN  uint32                      old_rate_class,
       DNX_SAND_IN  uint32                      new_rate_class,
       DNX_SAND_IN  int                         nof_additions);

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_SW_DB_INCLUDED__*/

#endif
