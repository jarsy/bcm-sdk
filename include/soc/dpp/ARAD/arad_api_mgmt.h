/* $Id: arad_api_mgmt.h,v 1.80 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_API_MGMT_INCLUDED__
/* { */
#define __ARAD_API_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/dpp_config_defs.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_mgmt.h>
#include <soc/dpp/ARAD/arad_api_fabric.h>
#include <soc/dpp/ARAD/arad_api_nif.h>
#include <soc/dpp/ARAD/arad_api_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_api_egr_queuing.h>
#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dpp/ARAD/arad_api_dram.h>
#include <soc/dpp/ARAD/arad_api_stat_if.h>
#include <soc/dpp/ARAD/arad_api_flow_control.h>
#include <soc/dpp/ARAD/arad_stat.h>
#include <soc/dpp/DRC/drc_combo28.h>

#include <soc/dcmn/dcmn_defs.h>
#include <soc/dcmn/fabric.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */



#define  ARAD_MAX_NOF_REVISIONS ARAD_REVISION_NOF_IDS

/*     Maximal length of DRAM configuration sequence.          */
#define  ARAD_HW_DRAM_CONF_SIZE_MAX 100

/*     Out of total of 15 SerDes quartets, two (one per
*     internal NIF Group consisting of 4 MAL-s) may be
*     assigned to either Network or Fabric interfaces.        */
#define  ARAD_MGMT_NOF_SHARED_SERDES_QUARTETS 2

#define ARAD_MGMT_VER_REG_BASE       0
#define ARAD_MGMT_CHIP_TYPE_FLD_LSB   4
#define ARAD_MGMT_CHIP_TYPE_FLD_MSB   23
#define ARAD_MGMT_DBG_VER_FLD_LSB     24
#define ARAD_MGMT_DBG_VER_FLD_MSB     27
#define ARAD_MGMT_CHIP_VER_FLD_LSB    28
#define ARAD_MGMT_CHIP_VER_FLD_MSB    31

/*
 *  This value disables limitation based on external (original)
 *  packet size
 */
#define ARAD_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT 0

#define ARAD_HW_DRAM_CONF_MODE_MIN   (ARAD_HW_DRAM_CONF_MODE_BUFFER)
#define ARAD_HW_DRAM_CONF_MODE_MAX   (ARAD_HW_DRAM_CONF_MODE_PARAMS)

#define ARAD_MGMT_FDR_TRFC_DISABLE         0x0
#define ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_LSB 0x1FB007E8
#define ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_MSB 0x10

/* OCB range: -1 to disable the range verification */
#define ARAD_MGMT_OCB_MC_RANGE_DISABLE 0xFFFFFFFF

/*
 *  Packet Size adjustments
 */
/* The offset decremented by the HW before checking packet range */
#define ARAD_MGMT_PCKT_RNG_HW_OFFSET 1

/* The CRC size assumed by the packet range API (external size).    */
/* Note: for 3-byte CRC (SPAUI), add one to the requested external  */
/* packet size range                                                */
#define ARAD_MGMT_PCKT_RNG_NIF_CRC_BYTES 4

/* DRAM CRC */
#define ARAD_MGMT_PCKT_RNG_DRAM_CRC_BYTES 2

/* The value to decrement from the requested when */
/* limiting the internal packet size              */
#define ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL \
  (ARAD_MGMT_PCKT_RNG_HW_OFFSET)


/* The value to decrement from the requested when */
/* limiting the external packet size              */
#define ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL \
  (ARAD_MGMT_PCKT_RNG_NIF_CRC_BYTES - ARAD_MGMT_PCKT_RNG_HW_OFFSET  - ARAD_MGMT_PCKT_RNG_DRAM_CRC_BYTES)


#define ARAD_MGMT_PCKT_MAX_SIZE_EXTERNAL_MAX        ((0x3FFF)+ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL)
#define ARAD_MGMT_PCKT_MAX_SIZE_INTERNAL_MAX        ((0x3FFF)+ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL)


#define ARAD_MGMT_STAT_IF_REPORT_INFO_ARRAY_SIZE_MAX (2)

/*
  * Indicator to the Software that the device was initialized, and traffic was enabled
  * Upon warm-start initialization, the software may read this register, and see
  * if the traffic was enabled.
  * If it was, the SW may utilize the SSR capabilities, to initialize the software,
  * without affecting the device
  * Used by arad_mgmt_enable_traffic_set()
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

#define ARAD_MGMT_FABRIC_HDR_TYPE_ARAD                     SOC_TMC_MGMT_FABRIC_HDR_TYPE_ARAD
#define ARAD_MGMT_FABRIC_HDR_TYPE_FAP20                    SOC_TMC_MGMT_FABRIC_HDR_TYPE_FAP20
#define ARAD_MGMT_FABRIC_HDR_TYPE_FAP10M                   SOC_TMC_MGMT_FABRIC_HDR_TYPE_FAP10M
#define ARAD_MGMT_NOF_FABRIC_HDR_TYPES                     SOC_TMC_MGMT_NOF_FABRIC_HDR_TYPES
typedef SOC_TMC_MGMT_FABRIC_HDR_TYPE                           ARAD_MGMT_FABRIC_HDR_TYPE;

#define ARAD_MGMT_TDM_MODE_PACKET                          SOC_TMC_MGMT_TDM_MODE_PACKET
#define ARAD_MGMT_TDM_MODE_TDM_OPT                         SOC_TMC_MGMT_TDM_MODE_TDM_OPT
#define ARAD_MGMT_TDM_MODE_TDM_STA                         SOC_TMC_MGMT_TDM_MODE_TDM_STA
typedef SOC_TMC_MGMT_TDM_MODE                                  ARAD_MGMT_TDM_MODE;

#define ARAD_MGMT_NOF_TDM_MODES                            SOC_TMC_MGMT_NOF_TDM_MODES

typedef SOC_TMC_MGMT_PCKT_SIZE                                 ARAD_MGMT_PCKT_SIZE;

#define ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN               SOC_TMC_MGMT_PCKT_SIZE_CONF_MODE_EXTERN     
#define ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN               SOC_TMC_MGMT_PCKT_SIZE_CONF_MODE_INTERN     
#define ARAD_MGMT_NOF_PCKT_SIZE_CONF_MODES                 SOC_TMC_MGMT_NOF_PCKT_SIZE_CONF_MODES       
typedef SOC_TMC_MGMT_PCKT_SIZE_CONF_MODE                       ARAD_MGMT_PCKT_SIZE_CONF_MODE;

typedef SOC_TMC_MGMT_OCB_VOQ_INFO                              ARAD_MGMT_OCB_VOQ_INFO;


typedef enum
{
  /*
   * FTMH load balancing extension is disabled.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED = 0,
  /*
   * FTMH load balancing extension is enabled, and contains an 8-bit
   * load balancing key and an 8-bit stacking route history bitmap.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE_8B_LB_KEY_8B_STACKING_ROUTE_HISTORY = 1,
  /*
   * FTMH load balancing extension is enabled, and contains a 16-bit
   * stacking route history.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE_16B_STACKING_ROUTE_HISTORY = 2,
  /*
   * FTMH load balancing extension is enabled.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE_ENABLED = 3,
   /*
   * FTMH load balancing extension is enabled, end 2 lb-key range table are used.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE_STANDBY_MC_LB = 4,
   /*
   * FTMH load balancing extension is enabled, and contains a 16-bit, Transmit on the 8b FTMH LB-Key Hash[15:8] , and on User-Header-2 .
   */  
  ARAD_MGMT_FTMH_LB_EXT_MODE_FULL_HASH = 5,
  /*
   *  Number of types in ARAD_MGMT_FTMH_LB_EXT_MODE
   */
  ARAD_MGMT_NOF_FTMH_LB_EXT_MODES = 6

} ARAD_MGMT_FTMH_LB_EXT_MODE;

typedef enum
{
  /*
   * Use 8 bits LAG-LB-Key.
   */

  ARAD_MGMT_TRUNK_HASH_FORMAT_NORMAL = 0,  
  /*
   *  Use 16 bits LAG-LB-Key, Switch the LAG-LB-Key transmitted from PP to TM: {LAG-LB-Key[7:0], LAG-LB-Key[15:8]}.  
   */
  ARAD_MGMT_TRUNK_HASH_FORMAT_INVERTED = 1,
  /*
   *  Use 16 bits LAG-LB-Key, Duplicate the TM LAG-LB-Key to {LAG-LB-Key[15:8], LAG-LB-Key[15:8]}. 
   */
  ARAD_MGMT_TRUNK_HASH_FORMAT_DUPLICATED = 2,
  /*
   *  Number of types in ARAD_MGMT_TRUNK_HASH_FORMAT
   */
  ARAD_MGMT_NOF_TRUNK_HASH_FORMAT = 3
  
} ARAD_MGMT_TRUNK_HASH_FORMAT;

typedef enum
{
  /*
   *  Arad device revision: 1
   */
  ARAD_REVISION_ID_1=0,
  /*
   *  Total number of Arad revisions
   */
  ARAD_REVISION_NOF_IDS=1
}ARAD_REVISION_ID;

typedef enum
{
  /*
   * Full Multicast 4K replecations with 64K DBuff mode
   */
  ARAD_INIT_FMC_4K_REP_64K_DBUFF_MODE = 0,
  /*
   * Full Multicast 64 replecations with 128K DBuff mode
   */
  ARAD_INIT_FMC_64_REP_128K_DBUFF_MODE = 1,
  /*
   * number of modes
   */
  ARAD_INIT_NBR_FULL_MULTICAST_DBUFF_NOF_MODES = 2
}ARAD_INIT_NBR_FULL_MULTICAST_DBUFF_MODES;

#define ARAD_INIT_PDM_MODE_SIMPLE SOC_TMC_INIT_PDM_MODE_SIMPLE
#define ARAD_INIT_PDM_MODE_REDUCED SOC_TMC_INIT_PDM_MODE_REDUCED
#define ARAD_INIT_PDM_NOF_MODES SOC_TMC_INIT_PDM_NOF_MODES
typedef SOC_TMC_INIT_PDM_MODE ARAD_INIT_PDM_MODE;

typedef enum
{
  /*
   *  Configure dram according to a buffer of registers to write
   *  and their values (<addr, val>)
   */
  ARAD_HW_DRAM_CONF_MODE_BUFFER=0,
  /*
   *  Configure dram according to logical parameters provided in
   *  the dram's vendor data sheet
   */
  ARAD_HW_DRAM_CONF_MODE_PARAMS=1,
  /*
   *  Total number of DRAM configuration modes
   */
  ARAD_HW_NOF_DRAM_CONF_MODES
}ARAD_HW_DRAM_CONF_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
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

}ARAD_HW_PLL_PARAMS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  
  /*
   *  Range: 0 - 7.
   */
  uint32  r;
  /*
   *  Range: 17 - 63.
   */
  uint32  f;
  /*
   *  Range: 1 - 4.
   */
  uint32  q;
}ARAD_INIT_DRAM_PLL;

#define ARAD_INIT_SERDES_REF_CLOCK_125 soc_dcmn_init_serdes_ref_clock_125
#define ARAD_INIT_SERDES_REF_CLOCK_156_25 soc_dcmn_init_serdes_ref_clock_156_25
#define ARAD_INIT_SREDES_NOF_REF_CLOCKS soc_dcmn_init_serdes_nof_ref_clocks
#define ARAD_INIT_SERDES_REF_CLOCK soc_dcmn_init_serdes_ref_clock_t
#define ARAD_INIT_SERDES_REF_CLOCK_DISABLE soc_dcmn_init_serdes_ref_clock_disable

typedef enum{
  /*
   * 25 Mhz
   */
  ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY_25_MHZ = 0,
  /*
   * 125 Mhz
   */
  ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY_125_MHZ = 1,
  /*
   *  Number of frequencies in ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY
   */
  ARAD_INIT_SYNTHESIZER_NOF_CLOCK_FREQUENCIES = 2
}ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY;

typedef enum{
  /* Support port rate of up to 400G */
  ARAD_MGMT_CREDIT_WORTH_RESOLUTION_LOW = 0,

  /* Support port rate of up to 200G */
  ARAD_MGMT_CREDIT_WORTH_RESOLUTION_MEDIUM = 1,

  /* Support port rate of up to 50G */
  ARAD_MGMT_CREDIT_WORTH_RESOLUTION_HIGH = 2,

  /* Automatically set according to interfaces in-use */
  ARAD_MGMT_CREDIT_WORTH_RESOLUTION_AUTO = 3,

  ARAD_MGMT_CREDIT_WORTH_RESOLUTIONS
}ARAD_MGMT_CREDIT_WORTH_RESOLUTION;

typedef enum
{
  /* For short range connection - 1.5V : Arad */
  ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V=0,
  /* For long range connection - 3.3V : Arad */
  ARAD_MGMT_EXT_VOL_MOD_3p3V,
  /* Short range connection- 1.5V HSTL internal reference VDDO/2 : Arad */
  ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V_VDDO,
  /* For short range connection - 1.8V : Jericho/Qmx */
  ARAD_MGMT_EXT_VOL_MOD_HSTL_1p8V,

  ARAD_MGMT_EXT_VOL_NOF_MODES
} ARAD_MGMT_EXT_VOLT_MOD;

typedef struct
{
  /*
   * dram_pll
   */
  ARAD_INIT_DRAM_PLL dram_pll;
  /*
   * nif clock frequency
   */
  ARAD_INIT_SERDES_REF_CLOCK nif_clk_freq;
  /*
   * fabric clock frequency
   */
  ARAD_INIT_SERDES_REF_CLOCK fabric_clk_freq;
  /*
   * external synthesizer clock frequency
   */
  ARAD_INIT_SYNTHESIZER_CLOCK_FREQUENCY synthesizer_clock_freq;
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

}ARAD_INIT_PLL; 

typedef struct
{
  /*
   *  Number of valid entries in dram_conf.
   */
  uint32 buff_len;

  ARAD_REG_INFO  buff_seq[ARAD_HW_DRAM_CONF_SIZE_MAX];

  uint32 appl_max_buffer_crc_err;

}ARAD_HW_DRAM_CONF_BUFFER;

typedef struct
{
  uint32  dram_freq;

  ARAD_DRAM_INFO params;

}ARAD_HW_DRAM_CONF_PARAMS;

typedef union
{
  /*
   *  DRAM configuration sequence. The sequence is a buffer of
   *  address-value pairs. This defines the dram configuration
   *  (DRC)
   */
   ARAD_HW_DRAM_CONF_BUFFER buffer_mode;

  /*
   *  DRAM auto configuration. The driver automatically configures DRAM
   *  operation modes. This defines the dram configuration (DRC)
   */
   ARAD_HW_DRAM_CONF_PARAMS params_mode;

}ARAD_HW_DRAM_CONF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable DRAM configuration
   */
  uint8 enable;
  /*
   *  Per DRAM interface, defines if exists and needs to be
   *  configured. Note: The following number of DRAM
   *  interfaces can be configured: 2, 3, 4, 6, 8
   */
  uint8 is_valid[SOC_DPP_DEFS_MAX(HW_DRAM_INTERFACES_MAX)];
  /* Number of drams */
  int nof_drams;
  /*
   *  Enable/disable Dram BIST on initialization
   */
  uint8 bist_enable;
  /*
   *  DRAM type.
   */
  ARAD_DRAM_TYPE dram_type;
  /*
   *  DRAM Pll configuration as derived from the DRAM
   *  reference clock.
   */
  /*  New PLL this is not a DPRC configuration but Arad core configuration, frequency range is different */
  ARAD_HW_PLL_PARAMS pll_conf;
  /*
   *  Number of Banks.
   */
  ARAD_DRAM_NUM_BANKS nof_banks;
  /*
   *  Number of DRAM columns. Range:
   *  256/512/1024/2048/4096/8192
   */
  ARAD_DRAM_NUM_COLUMNS nof_columns;
  /*
   *  Number of DRAM rows. Range:
   *  8192/16384
   */
  ARAD_DRAM_NUM_ROWS nof_rows;
  /*
   *  Summarized DRAM size of all DRAM interfaces. Units: Mbytes.
   */
  int dram_size_total_mbyte;
  /*
   *  Total buffer size allocated for User buffer. Units: Mbytes
   */
  int dram_user_buffer_size_mbytes;
  /*
   *  User dram buffer start ptr (first buffer number)
   */
  int dram_user_buffer_start_ptr;
  /*
   *  Total Dram size allocated for Device buffers. Units: Mbytes
   */
  int dram_device_buffer_size_mbytes;
  /*
   *  The size of a single data buffer in the DRAM
   */
  ARAD_ITM_DBUFF_SIZE_BYTES dbuff_size;
  /*
   *  Total number of device buffers allocated in Dram
   */
  int nof_dram_buffers;
  /*
   *  Full multicast DBuff mode
   */
  ARAD_INIT_NBR_FULL_MULTICAST_DBUFF_MODES fmc_dbuff_mode;
    /*
   * pdm mode
   */
  ARAD_INIT_PDM_MODE pdm_mode;
  /*
   *  T - Configure DRAM operation modes automatically
   *  F - Configure DRAM operation modes manually
   */
  ARAD_HW_DRAM_CONF_MODE conf_mode;

  /*
   *  Dram configuration mode
   */
  ARAD_HW_DRAM_CONF dram_conf;

  /*
   * Dram clam shell mode
   */
  ARAD_DDR_CLAM_SHELL_MODE dram_clam_shell_mode[SOC_DPP_DEFS_MAX(HW_DRAM_INTERFACES_MAX)];

  /*
   * MMU BANK interleaving mode  
   */
  uint32 interleaving_bits;
  
}ARAD_INIT_DDR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable fabric configuration
   */
  uint8 enable;
  /*
   *  The way the device is connected to fabric.
   */
  ARAD_FABRIC_CONNECT_MODE connect_mode;
  /*
   *  FTMH Header configuration: always allow, never allow,
   *  allow only when the packet is multicast.
   */
  ARAD_PORTS_FTMH_EXT_OUTLIF ftmh_extension;
  /*
   *  FTMH load balancing mode.
   */
  ARAD_MGMT_FTMH_LB_EXT_MODE ftmh_lb_ext_mode;
   /*
   *  FTMH Trunk Hash format.
   */
  ARAD_MGMT_TRUNK_HASH_FORMAT trunk_hash_format;
  /*
   *  FTMH Stacking mode.
   */
  uint8 ftmh_stacking_ext_mode;
  /* 
   * Presence of SOC_SAND_FE600 in the system
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
  soc_dcmn_fabric_pipe_map_t fabric_pipe_map_config;

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

}ARAD_INIT_FABRIC;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable Clock frequency configuration
   */
  uint8 enable;
  /*
   *  Number of ticks the ARAD device clock ticks per second
   *  (about a tick every 4.00 nano-seconds). Default value:
   *  600000. Units: kHz. 
   */
  uint32 frequency;
  /*
   *  System Reference clock. Units: kHz. 
   */
  uint32 system_ref_clock;

} ARAD_INIT_CORE_FREQ;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  PP Port index to configure. Range: 0 - 63.
   */
  uint32 profile_ndx;
  /*
   *  PP Port configuration.
   */
  ARAD_PORT_PP_PORT_INFO conf;

} ARAD_INIT_PP_PORT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 79.
   */
  uint32 port_ndx;
  /*
   *  PP Port for this TM Port.
   */
  uint32 pp_port;

} ARAD_INIT_PP_PORT_MAP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
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
  ARAD_EGR_PORT_PRIORITY_MODE priority_mode;
  /* 
   *  Egress base-q pair number. Range: 0-256
   */
  uint32 base_q_pair;
  /* 
   *  Egress port shaper mode
   */
  ARAD_EGR_PORT_SHAPER_MODE shaper_mode;

} ARAD_INIT_EGR_Q_PROFILE_MAP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TM Port index to configure. Range: 0 - 255.
   */
  uint32 port_ndx;
  /* 
   *  multicast_offset
   */
  uint32 multicast_offset;

} ARAD_MULTICAST_ID_OFFSET_MAP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 79
   */
  uint32 port_ndx;
  /*
   *  RAW/TM/ETH/Programmable header parsing type in the incoming direction.
   */
  ARAD_PORT_HEADER_TYPE header_type_in;
  /*
   *  header type in the outgoing direction.
   */
  ARAD_PORT_HEADER_TYPE header_type_out;
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

}ARAD_INIT_PORT_HDR_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FAP Port index to configure. Range: 0 - 255
   */
  uint32 port_ndx;

}ARAD_INIT_PORT_TO_IF_MAP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * Lag Mode - Arad only
   */
  ARAD_PORT_LAG_MODE lag_mode;
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
  soc_arad_stat_ilkn_counters_mode_t      ilkn_counters_mode;

  /* 
   *ilkn configration
   */
  ARAD_PORTS_ILKN_CONFIG ilkn[SOC_DPP_DEFS_MAX(NOF_INTERLAKEN_PORTS)];

  /*
   *ilkn first packet SW bypass
   */
  int ilkn_first_packet_sw_bypass;

  /* 
   *CAUI configration
   */
  ARAD_PORTS_CAUI_CONFIG caui[SOC_DPP_DEFS_MAX(NOF_CAUI_PORTS)];

  /*
   *  Device swap mode for incoming packets
   */
  ARAD_SWAP_INFO    swap_info;

} ARAD_INIT_PORTS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
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
  ARAD_MGMT_CREDIT_WORTH_RESOLUTION credit_worth_resolution;

} ARAD_INIT_CREDIT;

typedef enum
{
  /*
   * 80% unicast, 20 % multicat
   */
  ARAD_OCB_REPARTITION_MODE_80_PRESENTS_UNICAST = 0,
  /*
   * 100 % unicast
   */
  ARAD_OCB_REPARTITION_MODE_ALL_UNICAST = 1,
  /*
   * nof modes of ARAD_OCB_REPARTION_MODE
   */
  ARAD_OCB_NOF_REPARTITION_MODES = 2
}ARAD_OCB_REPARTITION_MODE;

typedef enum
{
  /*
   * No OCB use.
   */
  OCB_DISABLED = 0,
  /*
   * Like in Arad-A0/B0. Some packets may use both DRAM and OCB resources
   */
  OCB_ENABLED = 1,
  /*
   * ONE_WAY_BYPASS : OCB ONLY
   */
  OCB_ONLY = 2,
  /*
   * ONE_WAY_BYPASS : OCB-only with 1 DRAM for the free pointers
   */
  OCB_ONLY_1_DRAM=3,
  /*
   * ONE_WAY_BYPASS : OCB and DRAM coexist separately 
   */
  OCB_DRAM_SEPARATE=4
}ARAD_OCB_ENABLE_MODE;

typedef enum
{
  /*
   * Work in regular priority mode
   */
  ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_OFF,
  /*
   * Work in strict priority mode
   */
  ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON,

  ARAD_MGMT_ILKN_TDM_SP_NOF_MODES
}ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE;

typedef struct{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * enable/disable ocb
   */
  ARAD_OCB_ENABLE_MODE ocb_enable;
  /*
   * data buffer size
   */
  uint32 databuffer_size;
  /*
   * repartition mode
   */
  ARAD_OCB_REPARTITION_MODE repartition_mode;
}ARAD_INIT_OCB;


typedef struct{
  SOC_SAND_MAGIC_NUM_VAR
  
  /*
   * enable/disable stat_info 
   */
  uint8 stat_if_enable;
  /*
   * store soc property stat_if_etpp_mode value
   */
  int stat_if_etpp_mode;
  /*
   * statistic info
   */

  SOC_TMC_SIF_CORE_MODE core_mode;

  ARAD_STAT_IF_REPORT_INFO stat_if_info[MAX_NUM_OF_CORES];


}ARAD_INIT_STAT_IF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* OOB TX CLK Speed */
  ARAD_FC_OOB_TX_SPEED oob_tx_speed[SOC_TMC_FC_NOF_OOB_IDS];
  
  /* OOB Type - SPI / ILKN / HCFC / NONE */
  SOC_TMC_FC_OOB_TYPE fc_oob_type[SOC_TMC_FC_NOF_OOB_IDS];

  /* Bitwise mapping */
  /* 0x1 - RX */
  /* 0x2 - TX */
  /* 0x3 - Both */
  uint32 fc_directions[SOC_TMC_FC_NOF_OOB_IDS];

  /* OOB Calendar length per interface per direction [2][2] */
  uint32 fc_oob_calender_length[SOC_TMC_FC_NOF_OOB_IDS][SOC_TMC_CONNECTION_DIRECTION_BOTH];

  /* OOB Calendar reps per interface per direction [2][2] */
  uint32 fc_oob_calender_rep_count[SOC_TMC_FC_NOF_OOB_IDS][SOC_TMC_CONNECTION_DIRECTION_BOTH];

  /* Enable CL SCHEDULER Flow Control (Ardon - priority over port) */
  int cl_sch_enable;

}ARAD_INIT_FC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* RCPU slave port */
  soc_pbmp_t slave_port_pbmp;

} ARAD_INIT_RCPU;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
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
   *  DRAM configuration. 
   *  DRAM is used by the Arad to store packets at the ingress.
   */
  /* DDR3 structure. Devices: Arad, Arad+ ... */
  ARAD_INIT_DDR dram;
  /* DDR4, GDDR5 structure. Devices: Ardon, Jericho ... */
  soc_dpp_drc_combo28_info_t drc_info;
  /*
   *  Fabric configuration.
   */
  ARAD_INIT_SYNCE synce;
  
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /*
   *  ELK configuration.
   */
  ARAD_INIT_ELK elk;
#endif

  /*
   *  Fabric configuration.
   */
  ARAD_INIT_FABRIC fabric;
  /*
   *  Core clock frequency
   *  configuration.
   */
  ARAD_INIT_CORE_FREQ core_freq;
  /* 
   * Ports configuration
   */
  ARAD_INIT_PORTS ports;
  /*
   *  TDM traffic mode and FTMH format configuration.
   */
  ARAD_MGMT_TDM_MODE tdm_mode;
  /*
   *  ILKN TDM strict priority mode
   */
  ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
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
  ARAD_INIT_CREDIT credit;
  /*
   * Dram pll
   */
  ARAD_INIT_PLL pll;
  /*
   * Ocb
   */
  ARAD_INIT_OCB ocb;

  /*
   * statistic
   */
  ARAD_INIT_STAT_IF stat_if;

  /* Flow Control */
  ARAD_INIT_FC fc;
  /* 
   * Egress Shared CGM mode 
   */ 
  ARAD_EGR_QUEUING_PARTITION_SCHEME eg_cgm_scheme;
  /* 
   * RCPU 
   */ 
  ARAD_INIT_RCPU rcpu;
  /*
   * Dynamic port
   */
  uint8 dynamic_port_enable;
  /*
   * OOB External voltage mode
   */
  ARAD_MGMT_EXT_VOLT_MOD ex_vol_mod;
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
}ARAD_MGMT_INIT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * minimum
   */
  uint32 min;
  /*
   * maximum
   */
  uint32 max;
}ARAD_MGMT_OCB_MC_RANGE;

typedef struct
{
  /* is reserved port */
  int is_reserved;

  /* core id of reserved port */
  int core;

  /* tm_port of reserved port */
  uint32 tm_port;
}ARAD_MGMT_RESERVED_PORT_INFO;



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
*     arad_register_device
* TYPE:
*   PROC
* FUNCTION:
*     This procedure registers a new device to be taken care
*     of by this device driver. Physical device must be
*     accessible by CPU when this call is made..
* INPUT:
*  SOC_SAND_IN  uint32                  *base_address -
*     Base address of direct access memory assigned for
*     device's registers. This parameter needs to be specified
*     even if physical access to device is not by direct
*     access memory since all logic, within driver, up to
*     actual physical access, assumes 'virtual' direct access
*     memory. Memory block assigned by this pointer must not
*     overlap other memory blocks in user's system and
*     certainly not memory blocks assigned to other ARAD
*     devices using this procedure.
*  SOC_SAND_IN  SOC_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr -
*     BSP-function for device reset. Refer to
*     'SOC_SAND_RESET_DEVICE_FUNC_PTR' definition.
*  SOC_SAND_OUT uint32                 *unit_ptr -
*     This procedure loads pointed memory with identifier of
*     newly added device. This identifier is to be used by the
*     caller for further accesses to this device..
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_register_device(
             uint32                  *base_address,
    SOC_SAND_IN  SOC_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr,
    SOC_SAND_INOUT int                 *unit_ptr
  );

/*********************************************************************
* NAME:
*     arad_unregister_device
* TYPE:
*   PROC
* FUNCTION:
*     Undo arad_register_device()
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     The device ID to be unregistered.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_unregister_device(
    SOC_SAND_IN  int                 unit
  );

/***********************************************************************************
* NAME:
*     arad_calc_assigned_rebounded_credit_conf
* TYPE:
*   PROC
* FUNCTION:
*   Helper utility for *_mgmt_credit_worth_set():
*   Calculate value for SCH_ASSIGNED_CREDIT_CONFIGURATION, SCH_REBOUNDED_CREDIT_CONFIGURATION,
*   given input 'credit_worth'
*   Output goes into *fld_val to be loaded into registers specified above, into 
*   ASSIGNED_CREDIT_WORTH, REBOUNDED_CREDIT_WORTH fields, correspondingly
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32              credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
*  SOC_SAND_OUT uint32              *fld_val -
*     Value to load into fields specified in 'FUNCTION' above.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
***********************************************************************************/
uint32
  arad_calc_assigned_rebounded_credit_conf(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              credit_worth,
    SOC_SAND_OUT uint32              *fld_val
  ) ;
/*********************************************************************
* NAME:
*     arad_mgmt_credit_worth_set
* TYPE:
*   PROC
* FUNCTION:
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_mgmt_credit_worth_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              credit_worth
  );

/*
 * Arad+ only: set local and remote (1 and 2) credit worth values
 */
uint32
  arad_plus_mgmt_credit_worth_remote_set(
    SOC_SAND_IN  int    unit,
	SOC_SAND_IN  uint32    credit_worth_remote
  );

/*
 * Arad+ only: get local and remote (1 and 2) credit worth values
 */
uint32
  arad_plus_mgmt_credit_worth_remote_get(
    SOC_SAND_IN  int    unit,
	SOC_SAND_OUT uint32    *credit_worth_remote
  );

/*
 * Arad+ only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  arad_plus_mgmt_module_to_credit_worth_map_set(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32    fap_id,
    SOC_SAND_IN  uint32    credit_value_type /* should be one of ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );

/*
 * Arad+ only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  arad_plus_mgmt_module_to_credit_worth_map_get(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32    fap_id,
    SOC_SAND_OUT uint32    *credit_value_type /* will be one of ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );

uint32
  arad_plus_mgmt_change_all_faps_credit_worth_unsafe(
    SOC_SAND_IN  int    unit,
    SOC_SAND_OUT uint8     credit_value_to_use
  );

/*********************************************************************
* NAME:
*     arad_mgmt_init_sequence_phase1
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
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_MGMT_INIT            *init -
*     Contains user-defined initialization information for
*     hardware interfaces.
*  SOC_SAND_IN  uint8                 silent -
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
  arad_mgmt_init_sequence_phase1(
    SOC_SAND_IN     int                 unit,
    SOC_SAND_IN     ARAD_MGMT_INIT           *init,
    SOC_SAND_IN     uint8                 silent
  );

/*********************************************************************
* NAME:
*     arad_mgmt_init_sequence_phase2
* TYPE:
*   PROC
* FUNCTION:
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint8                 silent -
*     TRUE - Print progress messages. FALSE - Do not print
*     progress messages.
* REMARKS:
*     1. After phase 2 initialization, traffic can be enabled.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_init_sequence_phase2(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 silent
  );

/*********************************************************************
* NAME:
*     arad_mgmt_system_fap_id_set
* TYPE:
*   PROC
* FUNCTION:
*     Set the fabric system ID of the device. Must be unique
*     in the system.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 sys_fap_id -
*     The system ID of the device (Unique in the system).
 *     Range: 0 - 2047.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_system_fap_id_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 sys_fap_id
  );

/*********************************************************************
* NAME:
*     arad_mgmt_tm_domain_set
* TYPE:
*   PROC
* FUNCTION:
*     Set the device TM-Domain. Must be unique
*     in a stackable system.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 tm_domain -
*     The tmd of the device (Unique in a stackable  system).
 *     Range: 0 - 15.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_tm_domain_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 tm_domain
  );

/*********************************************************************
* NAME:
*     arad_mgmt_tm_domain_get
* TYPE:
*   PROC
* FUNCTION:
*     Get the device TM-Domain. Must be unique
*     in a stackable system.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint32                 *tm_domain -
*     The tmd of the device (Unique in a stackable  system).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_tm_domain_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT uint32                 *tm_domain
  );

uint32
  arad_mgmt_all_ctrl_cells_enable_get(
    SOC_SAND_IN  int  unit,
    SOC_SAND_OUT uint8  *enable
  );

/*********************************************************************
* NAME:
*     arad_mgmt_all_ctrl_cells_enable_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting control cells.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint8                 enable -
*     SOC_SAND_IN uint8 enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_all_ctrl_cells_enable_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 enable
  );

/*********************************************************************
*     Enable / Disable the forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  arad_force_tdm_bypass_traffic_to_fabric_set(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  int     enable
  );
/*********************************************************************
*     Check if forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  arad_force_tdm_bypass_traffic_to_fabric_get(
    SOC_SAND_IN  int     unit,
    SOC_SAND_OUT int     *enable
  );

/*********************************************************************
* NAME:
*     arad_mgmt_enable_traffic_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint8                 enable -
*     SOC_SAND_IN uint8 enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_enable_traffic_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     arad_mgmt_enable_traffic_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint8                 *enable -
*     Enable/disable indication
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_enable_traffic_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT uint8                 *enable
  );

/*********************************************************************
* NAME:
 *   arad_mgmt_max_pckt_size_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the maximal allowed packet size. The limitation can
 *   be performed based on the packet size before or after
 *   the ingress editing (external and internal configuration
 *   mode, accordingly). Packets above the specified value
 *   are dropped.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      port_ndx -
 *     Incoming port index. Range: 0 - 79.
 *   SOC_SAND_IN  ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx -
 *     External mode filters the packets according to there
 *     original size. Internal mode filters the packets
 *     according to their size inside the device, after ingress
 *     editing.
 *   SOC_SAND_IN  uint32                       max_size -
 *     Maximal allowed packet size per incoming port. Packets
 *     above this value will be dropped. Units: bytes.
 * REMARKS:
 *   1. This API gives a better resolution (i.e., per
 *   incoming port) than arad_mgmt_pckt_size_range_set. 2.
 *   If both APIs are used to configure the maximal packet
 *   size, the value configured is set by the API called at
 *   last.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_max_pckt_size_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      port_ndx,
    SOC_SAND_IN  ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    SOC_SAND_IN  uint32                       max_size
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_mgmt_max_pckt_size_set" API.
 *     Refer to "arad_mgmt_max_pckt_size_set" API for details.
*********************************************************************/
uint32
  arad_mgmt_max_pckt_size_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      port_ndx,
    SOC_SAND_IN  ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    SOC_SAND_OUT uint32                       *max_size
  );

/*********************************************************************
* NAME:
 *   arad_mgmt_ocb_mc_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb muliticast range.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      range_ndx -
 *     Incoming range index. Range: 0 - 1.
 *   SOC_SAND_IN  ARAD_MGMT_OCB_MC_RANGE *range -
 *     Structure with minimum and maximum.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_ocb_mc_range_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      range_ndx,
    SOC_SAND_IN  ARAD_MGMT_OCB_MC_RANGE         *range
  );

uint32
  arad_mgmt_ocb_mc_range_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      range_ndx,
    SOC_SAND_OUT ARAD_MGMT_OCB_MC_RANGE         *range
  );

/*********************************************************************
* NAME:
 *   arad_mgmt_ocb_voq_eligible_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb queue parameters.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN  uint32                    q_category_ndx -
 *     Queue category. Range: 0 - 3.
 *  SOC_SAND_IN  uint32                    rate_class_ndx -
 *     Queue rate class index. Range: 0 - 63
 *  SOC_SAND_IN  uint32                    tc_ndx - 
 *     Traffic class. Range: 0 - 7.
 *  SOC_SAND_IN  ARAD_MGMT_OCB_VOQ_INFO       *info - 
 *     Structure with the required data:
 *      - enable/diasable the ocb.
 *      - 2 word thresholds
 *      - 2 buffers thresholds
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_ocb_voq_eligible_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    q_category_ndx,
    SOC_SAND_IN  uint32                    rate_class_ndx,
    SOC_SAND_IN  uint32                    tc_ndx,
    SOC_SAND_IN  ARAD_MGMT_OCB_VOQ_INFO       *info,
    SOC_SAND_OUT ARAD_MGMT_OCB_VOQ_INFO    *exact_info
  );

uint32
  arad_mgmt_ocb_voq_eligible_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    q_category_ndx,
    SOC_SAND_IN  uint32                    rate_class_ndx,
    SOC_SAND_IN  uint32                    tc_ndx,
    SOC_SAND_OUT ARAD_MGMT_OCB_VOQ_INFO       *info
  );

/*********************************************************************
* NAME:
 *   arad_mgmt_ocb_voq_eligible_dynamic_set
 * FUNCTION:
 *   Set the  IDR_MEM_1F0000 Stores1 bit per queue number (128k queues) which indicates if the queue can be used.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  SOC_SAND_IN  uint32                    qid -
 *     the qid range 0-128K
 *  SOC_SAND_IN  uint32                    enable -
 *     enable q FALSE or TRUE
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
  arad_mgmt_ocb_voq_eligible_dynamic_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    qid,
    SOC_SAND_IN  uint32                    enable
  );


/*********************************************************************
*     Set the Soc_ARAD B0 revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_arad_mgmt_rev_b0_set(
    SOC_SAND_IN  int       unit);

#ifdef BCM_88660_A0
/*********************************************************************
*     Set the Soc_ARAD PLUS revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_arad_mgmt_rev_arad_plus_set(
    SOC_SAND_IN  int       unit);
    
#endif

void
  arad_ARAD_HW_PLL_PARAMS_clear(
    SOC_SAND_OUT ARAD_HW_PLL_PARAMS *info
  );

void
  arad_ARAD_INIT_DDR_clear(
    SOC_SAND_OUT ARAD_INIT_DDR *info
  );

void
  ARAD_INIT_DRAM_PLL_clear(
    SOC_SAND_OUT ARAD_INIT_DRAM_PLL *info
  );

void
  ARAD_INIT_PLL_clear(
    SOC_SAND_OUT ARAD_INIT_PLL *info
  );

void
  arad_ARAD_INIT_FABRIC_clear(
    SOC_SAND_OUT ARAD_INIT_FABRIC *info
  );

void
  ARAD_INIT_STAT_IF_clear(
    SOC_SAND_OUT ARAD_INIT_STAT_IF *info
  );

void
  arad_ARAD_INIT_CORE_FREQ_clear(
    SOC_SAND_OUT ARAD_INIT_CORE_FREQ *info
  );

void
  arad_ARAD_MGMT_INIT_clear(
    SOC_SAND_OUT ARAD_MGMT_INIT *info
  );

void
  arad_ARAD_MGMT_OCB_MC_RANGE_clear(
    SOC_SAND_OUT ARAD_MGMT_OCB_MC_RANGE *info
  );

void
  arad_ARAD_MGMT_OCB_VOQ_INFO_clear(
    SOC_SAND_OUT ARAD_MGMT_OCB_VOQ_INFO *info
  );

void
  arad_ARAD_MGMT_PCKT_SIZE_clear(
    SOC_SAND_OUT ARAD_MGMT_PCKT_SIZE *info
  );


void
  arad_ARAD_INIT_PORT_HDR_TYPE_clear(
    SOC_SAND_OUT ARAD_INIT_PORT_HDR_TYPE *info
  );

void
  arad_ARAD_INIT_PORT_TO_IF_MAP_clear(
    SOC_SAND_OUT ARAD_INIT_PORT_TO_IF_MAP *info
  );

void
  ARAD_INIT_PP_PORT_clear(
    SOC_SAND_OUT ARAD_INIT_PP_PORT *info
  );

void
  ARAD_INIT_PP_PORT_MAP_clear(
    SOC_SAND_OUT ARAD_INIT_PP_PORT_MAP *info
  );

void
  ARAD_INIT_EGR_Q_PROFILE_MAP_clear(
    SOC_SAND_OUT ARAD_INIT_EGR_Q_PROFILE_MAP *info
  );

void
  ARAD_INIT_CREDIT_clear(
    SOC_SAND_OUT ARAD_INIT_CREDIT *info
  );

void
  ARAD_INIT_OCB_clear(
    SOC_SAND_OUT ARAD_INIT_OCB *info
  );
  
void
  ARAD_INIT_PORTS_clear(
    SOC_SAND_OUT ARAD_INIT_PORTS *info
  );

void
  arad_ARAD_INIT_PORTS_clear(
    SOC_SAND_OUT ARAD_INIT_PORTS *info
  );

void
  arad_ARAD_INIT_FC_clear(
    SOC_SAND_OUT ARAD_INIT_FC *info
  );

#if ARAD_DEBUG_IS_LVL1
void
  arad_ARAD_HW_PLL_PARAMS_print(
    SOC_SAND_IN ARAD_HW_PLL_PARAMS *info
  );

void
  arad_ARAD_INIT_DDR_print(
    SOC_SAND_IN ARAD_INIT_DDR *info
  );

void
  arad_ARAD_INIT_FABRIC_print(
    SOC_SAND_IN ARAD_INIT_FABRIC *info
  );

void
  arad_ARAD_INIT_CORE_FREQ_print(
    SOC_SAND_IN  ARAD_INIT_CORE_FREQ *info
  );

void
  arad_ARAD_MGMT_INIT_print(
    SOC_SAND_IN ARAD_MGMT_INIT *info
  );

void
  arad_ARAD_INIT_PORT_HDR_TYPE_print(
    SOC_SAND_IN ARAD_INIT_PORT_HDR_TYPE *info
  );

void
  arad_ARAD_INIT_PORT_TO_IF_MAP_print(
    SOC_SAND_IN ARAD_INIT_PORT_TO_IF_MAP *info
  );

void
  ARAD_INIT_PP_PORT_print(
    SOC_SAND_IN  ARAD_INIT_PP_PORT *info
  );

void
  ARAD_INIT_PP_PORT_MAP_print(
    SOC_SAND_IN  ARAD_INIT_PP_PORT_MAP *info
  );

void
  ARAD_INIT_EGR_Q_PROFILE_MAP_print(
    SOC_SAND_IN  ARAD_INIT_EGR_Q_PROFILE_MAP *info
  );

void
  ARAD_INIT_PORTS_print(
    SOC_SAND_IN  ARAD_INIT_PORTS *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_MGMT_INCLUDED__*/
#endif

