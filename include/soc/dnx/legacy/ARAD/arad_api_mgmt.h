/* $Id: jer2_arad_api_mgmt.h,v 1.80 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_API_MGMT_INCLUDED__
/* { */
#define __JER2_ARAD_API_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_api_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_api_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_api_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_api_flow_control.h>
#include <soc/dnx/legacy/ARAD/arad_stat.h>

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/fabric.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */



#define  JER2_ARAD_MAX_NOF_REVISIONS JER2_ARAD_REVISION_NOF_IDS

/*     Out of total of 15 SerDes quartets, two (one per
*     internal NIF Group consisting of 4 MAL-s) may be
*     assigned to either Network or Fabric interfaces.        */
#define  JER2_ARAD_MGMT_NOF_SHARED_SERDES_QUARTETS 2

#define JER2_ARAD_MGMT_VER_REG_BASE       0
#define JER2_ARAD_MGMT_CHIP_TYPE_FLD_LSB   4
#define JER2_ARAD_MGMT_CHIP_TYPE_FLD_MSB   23
#define JER2_ARAD_MGMT_DBG_VER_FLD_LSB     24
#define JER2_ARAD_MGMT_DBG_VER_FLD_MSB     27
#define JER2_ARAD_MGMT_CHIP_VER_FLD_LSB    28
#define JER2_ARAD_MGMT_CHIP_VER_FLD_MSB    31

/*
 *  This value disables limitation based on external (original)
 *  packet size
 */
#define JER2_ARAD_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT 0

#define JER2_ARAD_MGMT_FDR_TRFC_DISABLE         0x0
#define JER2_ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_LSB 0x1FB007E8
#define JER2_ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_MSB 0x10

/* OCB range: -1 to disable the range verification */
#define JER2_ARAD_MGMT_OCB_MC_RANGE_DISABLE 0xFFFFFFFF

/*
 *  Packet Size adjustments
 */
/* The offset decremented by the HW before checking packet range */
#define JER2_ARAD_MGMT_PCKT_RNG_HW_OFFSET 1

/* The CRC size assumed by the packet range API (external size).    */
/* Note: for 3-byte CRC (SPAUI), add one to the requested external  */
/* packet size range                                                */
#define JER2_ARAD_MGMT_PCKT_RNG_NIF_CRC_BYTES 4

/* The value to decrement from the requested when */
/* limiting the internal packet size              */
#define JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL \
  (JER2_ARAD_MGMT_PCKT_RNG_HW_OFFSET)


/* The value to decrement from the requested when */
/* limiting the external packet size              */
#define JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL \
  (JER2_ARAD_MGMT_PCKT_RNG_NIF_CRC_BYTES - JER2_ARAD_MGMT_PCKT_RNG_HW_OFFSET  - JER2_ARAD_MGMT_PCKT_RNG_DRAM_CRC_BYTES)
#define JER2_ARAD_MGMT_PCKT_MAX_SIZE_EXTERNAL_MAX        ((0x3FFF)+JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL)
#define JER2_ARAD_MGMT_PCKT_MAX_SIZE_INTERNAL_MAX        ((0x3FFF)+JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL)

/*
  * Indicator to the Software that the device was initialized, and traffic was enabled
  * Upon warm-start initialization, the software may read this register, and see
  * if the traffic was enabled.
  * If it was, the SW may utilize the SSR capabilities, to initialize the software,
  * without affecting the device
  * Used by jer2_arad_mgmt_enable_traffic_set()
  */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

#define JER2_ARAD_MGMT_FABRIC_HDR_TYPE_JER2_ARAD                     DNX_TMC_MGMT_FABRIC_HDR_TYPE_JER2_ARAD
#define JER2_ARAD_MGMT_FABRIC_HDR_TYPE_FAP20                    DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP20
#define JER2_ARAD_MGMT_FABRIC_HDR_TYPE_FAP10M                   DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP10M
#define JER2_ARAD_MGMT_NOF_FABRIC_HDR_TYPES                     DNX_TMC_MGMT_NOF_FABRIC_HDR_TYPES
typedef DNX_TMC_MGMT_FABRIC_HDR_TYPE                           JER2_ARAD_MGMT_FABRIC_HDR_TYPE;

#define JER2_ARAD_MGMT_TDM_MODE_PACKET                          DNX_TMC_MGMT_TDM_MODE_PACKET
#define JER2_ARAD_MGMT_TDM_MODE_TDM_OPT                         DNX_TMC_MGMT_TDM_MODE_TDM_OPT
#define JER2_ARAD_MGMT_TDM_MODE_TDM_STA                         DNX_TMC_MGMT_TDM_MODE_TDM_STA
typedef DNX_TMC_MGMT_TDM_MODE                                  JER2_ARAD_MGMT_TDM_MODE;

#define JER2_ARAD_MGMT_NOF_TDM_MODES                            DNX_TMC_MGMT_NOF_TDM_MODES

typedef DNX_TMC_MGMT_PCKT_SIZE                                 JER2_ARAD_MGMT_PCKT_SIZE;

#define JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN               DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_EXTERN     
#define JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN               DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_INTERN     
#define JER2_ARAD_MGMT_NOF_PCKT_SIZE_CONF_MODES                 DNX_TMC_MGMT_NOF_PCKT_SIZE_CONF_MODES       
typedef DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE                       JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE;

typedef DNX_TMC_MGMT_OCB_VOQ_INFO                              JER2_ARAD_MGMT_OCB_VOQ_INFO;


typedef enum
{
  /*
   * FTMH load balancing extension is disabled.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED = 0,
  /*
   * FTMH load balancing extension is enabled, and contains an 8-bit
   * load balancing key and an 8-bit stacking route history bitmap.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_8B_LB_KEY_8B_STACKING_ROUTE_HISTORY = 1,
  /*
   * FTMH load balancing extension is enabled, and contains a 16-bit
   * stacking route history.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_16B_STACKING_ROUTE_HISTORY = 2,
  /*
   * FTMH load balancing extension is enabled.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_ENABLED = 3,
   /*
   * FTMH load balancing extension is enabled, end 2 lb-key range table are used.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_STANDBY_MC_LB = 4,
   /*
   * FTMH load balancing extension is enabled, and contains a 16-bit, Transmit on the 8b FTMH LB-Key Hash[15:8] , and on User-Header-2 .
   */  
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_FULL_HASH = 5,
  /*
   *  Number of types in JER2_ARAD_MGMT_FTMH_LB_EXT_MODE
   */
  JER2_ARAD_MGMT_NOF_FTMH_LB_EXT_MODES = 6

} JER2_ARAD_MGMT_FTMH_LB_EXT_MODE;

typedef enum
{
  /*
   * Use 8 bits LAG-LB-Key.
   */

  JER2_ARAD_MGMT_TRUNK_HASH_FORMAT_NORMAL = 0,  
  /*
   *  Use 16 bits LAG-LB-Key, Switch the LAG-LB-Key transmitted from PP to TM: {LAG-LB-Key[7:0], LAG-LB-Key[15:8]}.  
   */
  JER2_ARAD_MGMT_TRUNK_HASH_FORMAT_INVERTED = 1,
  /*
   *  Use 16 bits LAG-LB-Key, Duplicate the TM LAG-LB-Key to {LAG-LB-Key[15:8], LAG-LB-Key[15:8]}. 
   */
  JER2_ARAD_MGMT_TRUNK_HASH_FORMAT_DUPLICATED = 2,
  /*
   *  Number of types in JER2_ARAD_MGMT_TRUNK_HASH_FORMAT
   */
  JER2_ARAD_MGMT_NOF_TRUNK_HASH_FORMAT = 3
  
} JER2_ARAD_MGMT_TRUNK_HASH_FORMAT;

typedef enum
{
  /*
   *  Arad device revision: 1
   */
  JER2_ARAD_REVISION_ID_1=0,
  /*
   *  Total number of Arad revisions
   */
  JER2_ARAD_REVISION_NOF_IDS=1
}JER2_ARAD_REVISION_ID;

typedef enum
{
  /*
   * Full Multicast 4K replecations with 64K DBuff mode
   */
  JER2_ARAD_INIT_FMC_4K_REP_64K_DBUFF_MODE = 0,
  /*
   * Full Multicast 64 replecations with 128K DBuff mode
   */
  JER2_ARAD_INIT_FMC_64_REP_128K_DBUFF_MODE = 1,
  /*
   * number of modes
   */
  JER2_ARAD_INIT_NBR_FULL_MULTICAST_DBUFF_NOF_MODES = 2
}JER2_ARAD_INIT_NBR_FULL_MULTICAST_DBUFF_MODES;

#define JER2_ARAD_INIT_PDM_MODE_SIMPLE DNX_TMC_INIT_PDM_MODE_SIMPLE
#define JER2_ARAD_INIT_PDM_MODE_REDUCED DNX_TMC_INIT_PDM_MODE_REDUCED
#define JER2_ARAD_INIT_PDM_NOF_MODES DNX_TMC_INIT_PDM_NOF_MODES
typedef DNX_TMC_INIT_PDM_MODE JER2_ARAD_INIT_PDM_MODE;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Pll-M
   */
  uint32 m;
  /*
   *  Pll-N
   */
  uint32 n;
  /*
   *  Pll-P
   */
  uint32 p;

}JER2_ARAD_HW_PLL_PARAMS;


#define JER2_ARAD_INIT_SERDES_REF_CLOCK_125 soc_dnxc_init_serdes_ref_clock_125
#define JER2_ARAD_INIT_SERDES_REF_CLOCK_156_25 soc_dnxc_init_serdes_ref_clock_156_25
#define JER2_ARAD_INIT_SREDES_NOF_REF_CLOCKS soc_dnxc_init_serdes_nof_ref_clocks
#define JER2_ARAD_INIT_SERDES_REF_CLOCK soc_dnxc_init_serdes_ref_clock_t
#define JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE soc_dnxc_init_serdes_ref_clock_disable

typedef enum{
  /*
   * 25 Mhz
   */
  JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY_25_MHZ = 0,
  /*
   * 125 Mhz
   */
  JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY_125_MHZ = 1,
  /*
   *  Number of frequencies in JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY
   */
  JER2_ARAD_INIT_SYNTHESIZER_NOF_CLOCK_FREQUENCIES = 2
}JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY;

typedef enum{
  /* Support port rate of up to 400G */
  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_LOW = 0,

  /* Support port rate of up to 200G */
  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_MEDIUM = 1,

  /* Support port rate of up to 50G */
  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_HIGH = 2,

  /* Automatically set according to interfaces in-use */
  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_AUTO = 3,

  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTIONS
}JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION;

typedef enum
{
  /* For short range connection - 1.5V : Arad */
  JER2_ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V=0,
  /* For long range connection - 3.3V : Arad */
  JER2_ARAD_MGMT_EXT_VOL_MOD_3p3V,
  /* Short range connection- 1.5V HSTL internal reference VDDO/2 : Arad */
  JER2_ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V_VDDO,
  /* For short range connection - 1.8V : Jericho/Qmx */
  JER2_ARAD_MGMT_EXT_VOL_MOD_HSTL_1p8V,

  JER2_ARAD_MGMT_EXT_VOL_NOF_MODES
} JER2_ARAD_MGMT_EXT_VOLT_MOD;

typedef struct
{
  
  /*
   * nif clock frequency
   */
  JER2_ARAD_INIT_SERDES_REF_CLOCK nif_clk_freq;
  /*
   * fabric clock frequency
   */
  JER2_ARAD_INIT_SERDES_REF_CLOCK fabric_clk_freq;
  /*
   * external synthesizer clock frequency
   */
  JER2_ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY synthesizer_clock_freq;
  /* 
   * IEEE1588 DPLL mode, 0 - eci ts pll clk disabled, 1 - configure eci ts pll clk
   */
  uint32 ts_clk_mode;
  /* 
   * broadsync clock enable, 0 - disable, 1 - enable
   */
  uint32 bs_clk_mode;
  /* 
   * Initial phase values for the IEEE1588 DPLL, lower 32 bits
   */
  uint32 ts_pll_phase_initial_lo;
  /* 
   * Initial phase values for the IEEE1588 DPLL, upper 32 bits
   */
  uint32 ts_pll_phase_initial_hi;

}JER2_ARAD_INIT_PLL; 


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable fabric configuration
   */
  uint8 enable;
  /*
   *  The way the device is connected to fabric.
   */
  JER2_ARAD_FABRIC_CONNECT_MODE connect_mode;
  /*
   *  FTMH Header configuration: always allow, never allow,
   *  allow only when the packet is multicast.
   */
  DNX_TMC_PORTS_FTMH_EXT_OUTLIF ftmh_extension;
  /*
   *  FTMH load balancing mode.
   */
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE ftmh_lb_ext_mode;
   /*
   *  FTMH Trunk Hash format.
   */
  JER2_ARAD_MGMT_TRUNK_HASH_FORMAT trunk_hash_format;
  /*
   *  FTMH Stacking mode.
   */
  uint8 ftmh_stacking_ext_mode;
  /* 
   * Presence of DNX_SAND_FE600 in the system
   */ 
  uint8 is_fe600;
  /* 
   * Presence of 128B cells in the system
   */ 
  uint8 is_128_in_system;

  /*
   *  Dual Pipe mode
   */
  uint8 dual_pipe_tdm_packet;

  /*
   *  Presence of Dual Pipe mode in system
   */
  uint8 is_dual_mode_in_system;

  /*
   * Fabric cell segmentation enable
   */
  uint8 segmentation_enable;
  /*
  * Scheduler adaptation to links' states
  */
  uint8 scheduler_adapt_to_links;
  /*
  * System contains a device with multiple pipes
  */
  uint8 system_contains_multiple_pipe_device;
  /*
  * Fabric pipes mapping configuration
  */
  soc_dnxc_fabric_pipe_map_t fabric_pipe_map_config;

  /*
   *  fabric PCP enable: enable / disable packet cell packing
   */
  uint8 fabric_pcp_enable;

  /*
   *  fabric min TDM priority: NONE / 0-3
   */
  uint8 fabric_tdm_priority_min;

  /*
   *  fabric links to core mapping mode: SHARED / DEDICATED 
   */
  uint8 fabric_links_to_core_mapping_mode;
  
  /*
   *  fabric mesh multicast enable: enable / disable multicast in MESH mode
   */
  uint8 fabric_mesh_multicast_enable;

  /*
   *  mesh_topology fast - should be used for debug only
   */
  uint8 fabric_mesh_topology_fast;

}JER2_ARAD_INIT_FABRIC;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable Clock frequency configuration
   */
  uint8 enable;
  /*
   *  Number of ticks the JER2_ARAD device clock ticks per second
   *  (about a tick every 4.00 nano-seconds). Default value:
   *  600000. Units: kHz. 
   */
  uint32 frequency;
  /*
   *  System Reference clock. Units: kHz. 
   */
  uint32 system_ref_clock;

} JER2_ARAD_INIT_CORE_FREQ;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  PP Port index to configure. Range: 0 - 63.
   */
  uint32 profile_ndx;
  /*
   *  PP Port configuration.
   */
  DNX_TMC_PORT_PP_PORT_INFO conf;

} JER2_ARAD_INIT_PP_PORT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 79.
   */
  uint32 port_ndx;
  /*
   *  PP Port for this TM Port.
   */
  uint32 pp_port;

} JER2_ARAD_INIT_PP_PORT_MAP;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 79.
   */
  uint32 port_ndx;
  /*
   *  Egress queue priority profile index. Range: 0 - 3.
   */
  uint32 conf;
  /* 
   *  Egress port priority (number of q-pairs). Values: 1,2,8
   */
  JER2_ARAD_EGR_PORT_PRIORITY_MODE priority_mode;
  /* 
   *  Egress base-q pair number. Range: 0-256
   */
  uint32 base_q_pair;
  /* 
   *  Egress port shaper mode
   */
  JER2_ARAD_EGR_PORT_SHAPER_MODE shaper_mode;

} JER2_ARAD_INIT_EGR_Q_PROFILE_MAP;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  TM Port index to configure. Range: 0 - 255.
   */
  uint32 port_ndx;
  /* 
   *  multicast_offset
   */
  uint32 multicast_offset;

} JER2_ARAD_MULTICAST_ID_OFFSET_MAP;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 79
   */
  uint32 port_ndx;
  /*
   *  RAW/TM/ETH/Programmable header parsing type in the incoming direction.
   */
  DNX_TMC_PORT_HEADER_TYPE header_type_in;
  /*
   *  header type in the outgoing direction.
   */
  DNX_TMC_PORT_HEADER_TYPE header_type_out;
  /*
   *  If True, then Packets coming from this TM Port have a
   *  first header to strip before any processing. For
   *  example, in the Fat Pipe processing a Sequence Number
   *  header (2 Bytes) must be stripped.
   *  For injected packets, the PTCH Header must be
   *  removed (4 Bytes).
   *  Units: Bytes. Range: 0 - 63.
   *  In Petra-B, it was per PP-Port
   */
  uint32 first_header_size;

}JER2_ARAD_INIT_PORT_HDR_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 255
   */
  uint32 port_ndx;

}JER2_ARAD_INIT_PORT_TO_IF_MAP;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /* 
   * Lag Mode - Arad only
   */
  DNX_TMC_PORT_LAG_MODE lag_mode;
  /*
   *  stacking is enabled in the system.
   *    0 - not a stacking system.
   *    1-  stacking system.
   */
  uint8 is_stacking_system;
 /* 
  * use trunk as ingress MC destination. 
  *   0 - don't use trunk, use instead ports in trunk in ingress MC
  *   1 - use trunk as ingress mc destination
  */
  uint8 use_trunk_as_ingress_mc_dest;

  /*
   *  DSP-stacking is enabled in the system.
   *    0 - not enabled.
   *    1-  enabled.
   */
  uint8 add_dsp_extension_enable;
  /*
   *   PPH learn extension is Disabled in the system.
   *    0 - PPH learn extension is not disabled.
   *    1-  never add the PPH learn extension (unless explictly required in FP action).
   */
  uint8 pph_learn_extension_disable;
  /* 
   * use msb of lb-key in stack trunk resolutions
   */
  uint8   stack_resolve_using_msb;
  /* 
   * use msb of lb-key in smooth-division trunk resolutions
   */
  uint8   smooth_division_resolve_using_msb;
  /*
   *  System RED is enabled in the system.
   *    0 - Disabled.
   *    1 - Enabled.
   */
  uint8 is_system_red;
  /*
   *  The TM Domin of current device.
   */
  uint8 tm_domain;
  /*
   * Enable OAMP port
   */
  uint8 oamp_port_enable;
  /* 
   *ilkn counters mode
   */
  soc_jer2_arad_stat_ilkn_counters_mode_t      ilkn_counters_mode;

  /* 
   *ilkn configration
   */
  DNX_TMC_PORTS_ILKN_CONFIG ilkn[SOC_DNX_DEFS_MAX(NOF_INTERLAKEN_PORTS)];
  /* 
   *CAUI configration
   */
  DNX_TMC_PORTS_CAUI_CONFIG caui[SOC_DNX_DEFS_MAX(NOF_CAUI_PORTS)];

  /*
   *  Device swap mode for incoming packets
   */
  JER2_ARAD_SWAP_INFO    swap_info;

} JER2_ARAD_INIT_PORTS;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable credit worth configuration.
   */
  uint8 credit_worth_enable;
  /*
   *  Bytes-worth of a single credit. Units: bytesRange: 256 -
   *  8K. Resolution: 1 byte
   */
  uint32 credit_worth;

  /* Credit Worth Resolution */
  JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION credit_worth_resolution;

} JER2_ARAD_INIT_CREDIT;

typedef enum
{
  /*
   * 80% unicast, 20 % multicat
   */
  JER2_ARAD_OCB_REPARTITION_MODE_80_PRESENTS_UNICAST = 0,
  /*
   * 100 % unicast
   */
  JER2_ARAD_OCB_REPARTITION_MODE_ALL_UNICAST = 1,
  /*
   * nof modes of JER2_ARAD_OCB_REPARTION_MODE
   */
  JER2_ARAD_OCB_NOF_REPARTITION_MODES = 2
}JER2_ARAD_OCB_REPARTITION_MODE;

typedef enum
{
  /*
   * Work in regular priority mode
   */
  JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_OFF,
  /*
   * Work in strict priority mode
   */
  JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON,

  JER2_ARAD_MGMT_ILKN_TDM_SP_NOF_MODES
}JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /* OOB TX CLK Speed */
  JER2_ARAD_FC_OOB_TX_SPEED oob_tx_speed[DNX_TMC_FC_NOF_OOB_IDS];
  
  /* OOB Type - SPI / ILKN / HCFC / NONE */
  DNX_TMC_FC_OOB_TYPE fc_oob_type[DNX_TMC_FC_NOF_OOB_IDS];

  /* Bitwise mapping */
  /* 0x1 - RX */
  /* 0x2 - TX */
  /* 0x3 - Both */
  uint32 fc_directions[DNX_TMC_FC_NOF_OOB_IDS];

  /* OOB Calendar length per interface per direction [2][2] */
  uint32 fc_oob_calender_length[DNX_TMC_FC_NOF_OOB_IDS][DNX_TMC_CONNECTION_DIRECTION_BOTH];

  /* OOB Calendar reps per interface per direction [2][2] */
  uint32 fc_oob_calender_rep_count[DNX_TMC_FC_NOF_OOB_IDS][DNX_TMC_CONNECTION_DIRECTION_BOTH];

  /* Enable CL SCHEDULER Flow Control (Ardon - priority over port) */
  int cl_sch_enable;

}JER2_ARAD_INIT_FC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /* RCPU slave port */
  soc_pbmp_t slave_port_pbmp;

} JER2_ARAD_INIT_RCPU;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Enable Packet Processing features. Valid only for Arad
   *  revisions that support Packet Processing.
   */
  uint8 pp_enable;
  /* 
   *  Enable Petra-B in the system.
   */
  uint8 is_petrab_in_system;

  
  /*
   *  Fabric configuration.
   */
  JER2_ARAD_INIT_FABRIC fabric;
  /*
   *  Core clock frequency
   *  configuration.
   */
  JER2_ARAD_INIT_CORE_FREQ core_freq;
  /* 
   * Ports configuration
   */
  JER2_ARAD_INIT_PORTS ports;
  /*
   *  TDM traffic mode and FTMH format configuration.
   */
  JER2_ARAD_MGMT_TDM_MODE tdm_mode;
  /*
   *  ILKN TDM strict priority mode
   */
  JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
  /*
   *  TDM egress priority mode
   */
  uint32 tdm_egress_priority;
  /*
   *  TDM egress drop precedence
   */
  uint32 tdm_egress_dp;
  /* 
   * Credit configuration
   */
  JER2_ARAD_INIT_CREDIT credit;
  /*
   *  pll
   */
  JER2_ARAD_INIT_PLL pll;

  /* Flow Control */
  JER2_ARAD_INIT_FC fc;
  /* 
   * Egress Shared CGM mode 
   */ 
  JER2_ARAD_EGR_QUEUING_PARTITION_SCHEME eg_cgm_scheme;
  /* 
   * RCPU 
   */ 
  JER2_ARAD_INIT_RCPU rcpu;
  /*
   * Dynamic port
   */
  uint8 dynamic_port_enable;
  /*
   * OOB External voltage mode
   */
  JER2_ARAD_MGMT_EXT_VOLT_MOD ex_vol_mod;
  /* 
   * enable nif recovery check
   */
  uint32 nif_recovery_enable; 
  /* 
   * max num of iterations to allow
   * on nif recovery check (if enabled)
   */
  uint32 nif_recovery_iter;

  uint32 max_burst_default_value_bucket_width;

  /*
   * if set, mirror/snooped packets DSP-Ext field  
   * will be stamped with the SYS dsp. 
   * FTMH extension must be enabled. 
   */
  uint8 mirror_stamp_sys_dsp_ext;

  /* 
   * enable allocating rcy port (termination context) for each mirrored channel in a channelized interface, otherwise only 1 termination context (tm port) 
   * is allocated for the whole interface rather than seperate termination context for each channel 
   */
  uint32 rcy_channelized_shared_context_enable; 
}JER2_ARAD_MGMT_INIT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   * minimum
   */
  uint32 min;
  /*
   * maximum
   */
  uint32 max;
}JER2_ARAD_MGMT_OCB_MC_RANGE;

typedef struct
{
  /* is reserved port */
  int is_reserved;

  /* core id of reserved port */
  int core;

  /* tm_port of reserved port */
  uint32 tm_port;
}JER2_ARAD_MGMT_RESERVED_PORT_INFO;



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
/*********************************************************************
* NAME:
*     jer2_arad_register_device
* TYPE:
*   PROC
* FUNCTION:
*     This procedure registers a new device to be taken care
*     of by this device driver. Physical device must be
*     accessible by CPU when this call is made..
* INPUT:
*  DNX_SAND_IN  uint32                  *base_address -
*     Base address of direct access memory assigned for
*     device's registers. This parameter needs to be specified
*     even if physical access to device is not by direct
*     access memory since all logic, within driver, up to
*     actual physical access, assumes 'virtual' direct access
*     memory. Memory block assigned by this pointer must not
*     overlap other memory blocks in user's system and
*     certainly not memory blocks assigned to other JER2_ARAD
*     devices using this procedure.
*  DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr -
*     BSP-function for device reset. Refer to
*     'DNX_SAND_RESET_DEVICE_FUNC_PTR' definition.
*  DNX_SAND_OUT uint32                 *unit_ptr -
*     This procedure loads pointed memory with identifier of
*     newly added device. This identifier is to be used by the
*     caller for further accesses to this device..
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_register_device(
             uint32                  *base_address,
    DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr,
    DNX_SAND_INOUT int                 *unit_ptr
  );

/*********************************************************************
* NAME:
*     jer2_arad_unregister_device
* TYPE:
*   PROC
* FUNCTION:
*     Undo jer2_arad_register_device()
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     The device ID to be unregistered.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_unregister_device(
    DNX_SAND_IN  int                 unit
  );

/***********************************************************************************
* NAME:
*     jer2_arad_calc_assigned_rebounded_credit_conf
* TYPE:
*   PROC
* FUNCTION:
*   Helper utility for *_mgmt_credit_worth_set():
*   Calculate value for SCH_ASSIGNED_CREDIT_CONFIGURATION, SCH_REBOUNDED_CREDIT_CONFIGURATION,
*   given input 'credit_worth'
*   Output goes into *fld_val to be loaded into registers specified above, into 
*   ASSIGNED_CREDIT_WORTH, REBOUNDED_CREDIT_WORTH fields, correspondingly
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32              credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
*  DNX_SAND_OUT uint32              *fld_val -
*     Value to load into fields specified in 'FUNCTION' above.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
***********************************************************************************/
uint32
  jer2_arad_calc_assigned_rebounded_credit_conf(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth,
    DNX_SAND_OUT uint32              *fld_val
  ) ;
/*********************************************************************
* NAME:
*     jer2_arad_mgmt_credit_worth_set
* TYPE:
*   PROC
* FUNCTION:
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                  credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_mgmt_credit_worth_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  );

/*
 * Arad+ only: set local and remote (1 and 2) credit worth values
 */
uint32
  jer2_arad_plus_mgmt_credit_worth_remote_set(
    DNX_SAND_IN  int    unit,
	DNX_SAND_IN  uint32    credit_worth_remote
  );

/*
 * Arad+ only: get local and remote (1 and 2) credit worth values
 */
uint32
  jer2_arad_plus_mgmt_credit_worth_remote_get(
    DNX_SAND_IN  int    unit,
	DNX_SAND_OUT uint32    *credit_worth_remote
  );

/*
 * Arad+ only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );

/*
 * Arad+ only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );

uint32
  jer2_arad_plus_mgmt_change_all_faps_credit_worth_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint8     credit_value_to_use
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_init_sequence_phase1
* TYPE:
*   PROC
* FUNCTION:
*     Initialize the device, including:1. Prevent all the
*     control cells. 2. Initialize the device tables and
*     registers to default values. 3. Initialize
*     board-specific hardware interfaces according to
*     configurable information, as passed in 'hw_adjust'. 4.
*     Perform basic device initialization. The configuration
*     can be enabled/disabled as passed in 'enable_info'.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  JER2_ARAD_MGMT_INIT            *init -
*     Contains user-defined initialization information for
*     hardware interfaces.
*  DNX_SAND_IN  uint8                 silent -
*     If TRUE, progress printing will be suppressed.
* REMARKS:
*     1. For all configurations that can be done per-direction
*     (e.g. NIF - rx/tx, FAP - incoming/outgoing) - the
*     configuration is performed for both directions if
*     enabled. It may be overridden before phase2 if needed.
*     2. For all input structures, NULL pointer may be passed.
*     If input structure is passed as NULL, the appropriate
*     configuration will not be performed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase1(
    DNX_SAND_IN     int                 unit,
    DNX_SAND_IN     JER2_ARAD_MGMT_INIT           *init,
    DNX_SAND_IN     uint8                 silent
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_init_sequence_phase2
* TYPE:
*   PROC
* FUNCTION:
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint8                 silent -
*     TRUE - Print progress messages. FALSE - Do not print
*     progress messages.
* REMARKS:
*     1. After phase 2 initialization, traffic can be enabled.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase2(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_system_fap_id_set
* TYPE:
*   PROC
* FUNCTION:
*     Set the fabric system ID of the device. Must be unique
*     in the system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 sys_fap_id -
*     The system ID of the device (Unique in the system).
 *     Range: 0 - 2047.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_tm_domain_set
* TYPE:
*   PROC
* FUNCTION:
*     Set the device TM-Domain. Must be unique
*     in a stackable system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 tm_domain -
*     The tmd of the device (Unique in a stackable  system).
 *     Range: 0 - 15.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_tm_domain_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_tm_domain_get
* TYPE:
*   PROC
* FUNCTION:
*     Get the device TM-Domain. Must be unique
*     in a stackable system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint32                 *tm_domain -
*     The tmd of the device (Unique in a stackable  system).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_tm_domain_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                 *tm_domain
  );

uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_get(
    DNX_SAND_IN  int  unit,
    DNX_SAND_OUT uint8  *enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_all_ctrl_cells_enable_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting control cells.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint8                 enable -
*     DNX_SAND_IN uint8 enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

/*********************************************************************
*     Enable / Disable the forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  int     enable
  );
/*********************************************************************
*     Check if forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_get(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT int     *enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_enable_traffic_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint8                 enable -
*     DNX_SAND_IN uint8 enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_enable_traffic_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint8                 *enable -
*     Enable/disable indication
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *enable
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_max_pckt_size_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the maximal allowed packet size. The limitation can
 *   be performed based on the packet size before or after
 *   the ingress editing (external and internal configuration
 *   mode, accordingly). Packets above the specified value
 *   are dropped.
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                      port_ndx -
 *     Incoming port index. Range: 0 - 79.
 *   DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx -
 *     External mode filters the packets according to there
 *     original size. Internal mode filters the packets
 *     according to their size inside the device, after ingress
 *     editing.
 *   DNX_SAND_IN  uint32                       max_size -
 *     Maximal allowed packet size per incoming port. Packets
 *     above this value will be dropped. Units: bytes.
 * REMARKS:
 *   1. This API gives a better resolution (i.e., per
 *   incoming port) than jer2_arad_mgmt_pckt_size_range_set. 2.
 *   If both APIs are used to configure the maximal packet
 *   size, the value configured is set by the API called at
 *   last.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_mgmt_max_pckt_size_set" API.
 *     Refer to "jer2_arad_mgmt_max_pckt_size_set" API for details.
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT uint32                       *max_size
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_mc_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb muliticast range.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                      range_ndx -
 *     Incoming range index. Range: 0 - 1.
 *   DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE *range -
 *     Structure with minimum and maximum.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_ocb_mc_range_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  );

uint32
  jer2_arad_mgmt_ocb_mc_range_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_voq_eligible_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb queue parameters.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  DNX_SAND_IN  uint32                    q_category_ndx -
 *     Queue category. Range: 0 - 3.
 *  DNX_SAND_IN  uint32                    rate_class_ndx -
 *     Queue rate class index. Range: 0 - 63
 *  DNX_SAND_IN  uint32                    tc_ndx - 
 *     Traffic class. Range: 0 - 7.
 *  DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info - 
 *     Structure with the required data:
 *      - enable/diasable the ocb.
 *      - 2 word thresholds
 *      - 2 buffers thresholds
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_ocb_voq_eligible_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO    *exact_info
  );

uint32
  jer2_arad_mgmt_ocb_voq_eligible_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO       *info
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_voq_eligible_dynamic_set
 * FUNCTION:
 *   Set the  IDR_MEM_1F0000 Stores1 bit per queue number (128k queues) which indicates if the queue can be used.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  DNX_SAND_IN  uint32                    qid -
 *     the qid range 0-128K
 *  DNX_SAND_IN  uint32                    enable -
 *     enable q FALSE or TRUE
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_mgmt_ocb_voq_eligible_dynamic_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    qid,
    DNX_SAND_IN  uint32                    enable
  );


/*********************************************************************
*     Set the Soc_JER2_ARAD B0 revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_jer2_arad_mgmt_rev_b0_set(
    DNX_SAND_IN  int       unit);

#ifdef BCM_88660_A0
/*********************************************************************
*     Set the Soc_JER2_ARAD PLUS revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_jer2_arad_mgmt_rev_jer2_arad_plus_set(
    DNX_SAND_IN  int       unit);
    
#endif

void
  jer2_arad_JER2_ARAD_HW_PLL_PARAMS_clear(
    DNX_SAND_OUT JER2_ARAD_HW_PLL_PARAMS *info
  );

void
  JER2_ARAD_INIT_PLL_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PLL *info
  );

void
  jer2_arad_JER2_ARAD_INIT_FABRIC_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_FABRIC *info
  );

void
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_CORE_FREQ *info
  );

void
  jer2_arad_JER2_ARAD_MGMT_INIT_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_INIT *info
  );

void
  jer2_arad_JER2_ARAD_MGMT_OCB_MC_RANGE_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE *info
  );

void
  jer2_arad_JER2_ARAD_MGMT_OCB_VOQ_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MGMT_PCKT_SIZE_clear(
    DNX_SAND_OUT JER2_ARAD_MGMT_PCKT_SIZE *info
  );


void
  jer2_arad_JER2_ARAD_INIT_PORT_HDR_TYPE_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PORT_HDR_TYPE *info
  );

void
  jer2_arad_JER2_ARAD_INIT_PORT_TO_IF_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PORT_TO_IF_MAP *info
  );

void
  JER2_ARAD_INIT_PP_PORT_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PP_PORT *info
  );

void
  JER2_ARAD_INIT_PP_PORT_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PP_PORT_MAP *info
  );

void
  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_EGR_Q_PROFILE_MAP *info
  );

void
  JER2_ARAD_INIT_CREDIT_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_CREDIT *info
  );
  
void
  JER2_ARAD_INIT_PORTS_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PORTS *info
  );

void
  jer2_arad_JER2_ARAD_INIT_PORTS_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_PORTS *info
  );

void
  jer2_arad_JER2_ARAD_INIT_FC_clear(
    DNX_SAND_OUT JER2_ARAD_INIT_FC *info
  );

#if JER2_ARAD_DEBUG_IS_LVL1
void
  jer2_arad_JER2_ARAD_HW_PLL_PARAMS_print(
    DNX_SAND_IN JER2_ARAD_HW_PLL_PARAMS *info
  );

void
  jer2_arad_JER2_ARAD_INIT_FABRIC_print(
    DNX_SAND_IN JER2_ARAD_INIT_FABRIC *info
  );

void
  jer2_arad_JER2_ARAD_INIT_CORE_FREQ_print(
    DNX_SAND_IN  JER2_ARAD_INIT_CORE_FREQ *info
  );

void
  jer2_arad_JER2_ARAD_MGMT_INIT_print(
    DNX_SAND_IN JER2_ARAD_MGMT_INIT *info
  );

void
  jer2_arad_JER2_ARAD_INIT_PORT_HDR_TYPE_print(
    DNX_SAND_IN JER2_ARAD_INIT_PORT_HDR_TYPE *info
  );

void
  jer2_arad_JER2_ARAD_INIT_PORT_TO_IF_MAP_print(
    DNX_SAND_IN JER2_ARAD_INIT_PORT_TO_IF_MAP *info
  );

void
  JER2_ARAD_INIT_PP_PORT_print(
    DNX_SAND_IN  JER2_ARAD_INIT_PP_PORT *info
  );

void
  JER2_ARAD_INIT_PP_PORT_MAP_print(
    DNX_SAND_IN  JER2_ARAD_INIT_PP_PORT_MAP *info
  );

void
  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP_print(
    DNX_SAND_IN  JER2_ARAD_INIT_EGR_Q_PROFILE_MAP *info
  );

void
  JER2_ARAD_INIT_PORTS_print(
    DNX_SAND_IN  JER2_ARAD_INIT_PORTS *info
  );

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_API_MGMT_INCLUDED__*/
#endif

