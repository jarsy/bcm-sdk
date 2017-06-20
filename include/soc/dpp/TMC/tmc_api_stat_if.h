/* $Id: tmc_api_stat_if.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_stat_if.h
*
* MODULE PREFIX:  soc_tmcstat
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_TMC_API_STAT_IF_INCLUDED__
/* { */
#define __SOC_TMC_API_STAT_IF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_TMC_STAT_NOF_THRESHOLD_RANGES 15
#define SOC_TMC_STAT_IF_COUNTERS_COMMANDS 2
#define SOC_TMC_STAT_IF_REPORT_SIZE_61b  (SOC_IS_QAX(unit) ? 1 : 0)
#define SOC_TMC_STAT_IF_REPORT_SIZE_64b  (SOC_IS_QAX(unit) ? 0 : 1)


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
   *  Statistics data is in phase with the clock
   */
  SOC_TMC_STAT_IF_PHASE_000=0,
  /*
   *  Statistics data is 90 degrees to the clock
   */
  SOC_TMC_STAT_IF_PHASE_090=1,
  /*
   *  Statistics data is 180 degrees to the clock
   */
  SOC_TMC_STAT_IF_PHASE_180=2,
  /*
   *  Statistics data is 270 degrees to the clock
   */
  SOC_TMC_STAT_IF_PHASE_270=3,
  /*
   *  Number of types in SOC_TMC_STAT_IF_PHASE
   */
  SOC_TMC_STAT_NOF_IF_PHASES = 4
}SOC_TMC_STAT_IF_PHASE;

typedef enum
{
  /*
   *  The statistics report is composed in billing
   *  mode:Ingress enqueue and Egress enqueue reports
   */
  SOC_TMC_STAT_IF_REPORT_MODE_BILLING = 0,
  /*
   *  SOC_SAND_FAP20V-compatible mode is presented: Ingress enqueue and
   *  Ingress dequeue reports
   */
  SOC_TMC_STAT_IF_REPORT_MODE_FAP20V = 1,

    /* Number of modes in Soc_petra */
    SOC_TMC_STAT_NOF_IF_REPORT_MODES_PETRA = 2,

  /* 
   * QSIZE mode
   * Ingress-Enq, Ingress-Deq, Scrubber report
   */
  SOC_TMC_STAT_IF_REPORT_QSIZE = SOC_TMC_STAT_NOF_IF_REPORT_MODES_PETRA,

  /*
   *  The statistics report is composed in billing
   *  mode:Ingress enqueue and Egress enqueue reports
   *  stat-key indication is the Queue-Number in ingress billing.
   */
  SOC_TMC_STAT_IF_REPORT_MODE_BILLING_QUEUE_NUMBER = 3,

  /* Ingress billing, available in jericho and above*/

  SOC_TMC_STAT_IF_REPORT_MODE_BILLING_INGRESS = 4,
  /*
   *  report mode not configured
   */
  SOC_TMC_STAT_IF_REPORT_MODE_NOT_CONFIGURE = 5,

  /*
   *  Number of statistic report modes.
   */
  SOC_TMC_STAT_NOF_IF_REPORT_MODES = 6
}SOC_TMC_STAT_IF_REPORT_MODE;

typedef enum
{
  /*
   * Each core works with a seperate interface
   */
  SOC_TMC_SIF_CORE_MODE_DEDICATED,
  /*
   * Both cores with the same interface
   */
  SOC_TMC_SIF_CORE_MODE_COMBINED,

  SOC_SIF_CORE_NOF_MODES
}SOC_TMC_SIF_CORE_MODE;


typedef enum
{
  /*
   *  Egress queue number presentation in the egress report.
   */
  SOC_TMC_STAT_IF_BILLING_MODE_EGR_Q_NUM = 0,
  /*
   *  Copy-Unique-Data (Out-LIF) presentation in the egress
   *  report.
   */
  SOC_TMC_STAT_IF_BILLING_MODE_CUD = 1,
  /*
   *  Not applicable for Soc_petra-A. Egress statistics according
   *  to the VSI (VLAN).
   */
  SOC_TMC_STAT_IF_BILLING_MODE_VSI_VLAN = 2,
  /*
   *  Not applicable for Soc_petra-A. Both In-LIF and Out-LIF
   *  presentations in the egress report.
   */
  SOC_TMC_STAT_IF_BILLING_MODE_BOTH_LIFS = 3,
  /*
   *  Number of types in SOC_TMC_STAT_IF_BILLING_MODE
   */
  SOC_TMC_STAT_NOF_IF_BILLING_MODES = 4
}SOC_TMC_STAT_IF_BILLING_MODE;

typedef enum
{
  /*
   *  Queue size report format.
   */
  SOC_TMC_STAT_IF_FAP20V_MODE_Q_SIZE = 0,
  /*
   *  Packet size report format.
   */
  SOC_TMC_STAT_IF_FAP20V_MODE_PKT_SIZE = 1,
  /*
   *  Number of types in SOC_TMC_STAT_IF_FAP20V_MODE
   */
  SOC_TMC_STAT_NOF_IF_FAP20V_MODES = 2
}SOC_TMC_STAT_IF_FAP20V_MODE;

typedef enum
{
  /*
   *  The multicast packets are reported with the Queue
   *  number.
   */
  SOC_TMC_STAT_IF_MC_MODE_Q_NUM = 0,
  /*
   *  The multicast packets are reported with the Multicast
   *  ID.
   */
  SOC_TMC_STAT_IF_MC_MODE_MC_ID = 1,
  /*
   *  Number of types in SOC_TMC_STAT_IF_MC_MODE
   */
  SOC_TMC_STAT_NOF_IF_MC_MODES = 2
}SOC_TMC_STAT_IF_MC_MODE;

typedef enum
{
  /*
   *  The CNM packet statistics are not reported.
   */
  SOC_TMC_STAT_IF_CNM_MODE_DIS = 0,
  /*
   *  The CNM packet statistics are reported.
   */
  SOC_TMC_STAT_IF_CNM_MODE_EN = 1,
  /*
   *  Number of types in SOC_TMC_STAT_IF_CNM_MODE
   */
  SOC_TMC_STAT_NOF_IF_CNM_MODES = 2
}SOC_TMC_STAT_IF_CNM_MODE;

typedef enum
{
  /*
   *  The parity bit/bits errors identification is disabled.
   */
  SOC_TMC_STAT_IF_PARITY_MODE_DIS = 0,
  /*
   *  The parity bit/bits errors identification is enabled.
   */
  SOC_TMC_STAT_IF_PARITY_MODE_EN = 1,
  /*
   *  Number of types in SOC_TMC_STAT_IF_PARITY_MODE
   */
  SOC_TMC_STAT_NOF_IF_PARITY_MODES = 2
}SOC_TMC_STAT_IF_PARITY_MODE;

typedef enum
{
  /*
   *  BIST commands on Statistic interface are disabled.
   *  Single mode which enables the transmission of statistic
   *  reports.
   */
  SOC_TMC_STAT_IF_BIST_EN_MODE_DIS = 0,
  /*
   *  The BIST commands write alternatively two pattern words
   *  in continuous.
   */
  SOC_TMC_STAT_IF_BIST_EN_MODE_PATTERN = 1,
  /*
   *  The BIST commands perform a walking one writing.
   */
  SOC_TMC_STAT_IF_BIST_EN_MODE_WALKING_ONE = 2,
  /*
   *  The BIST patterns are generated under a PRBS mode.
   */
  SOC_TMC_STAT_IF_BIST_EN_MODE_PRBS = 3,
  /*
   *  Number of types in SOC_TMC_STAT_IF_BIST_EN_MODE
   */
  SOC_TMC_STAT_NOF_IF_BIST_EN_MODES = 4
}SOC_TMC_STAT_IF_BIST_EN_MODE;

typedef enum
{

  /* Possible packet size (not inculded CRC) are 61B (65B with CRC) */
  /* This constant should not be changed, because it is used for hardware initialization */
  SOC_TMC_STAT_IF_PKT_SIZE_65B = 0,

  /* Possible packet size (not inculded CRC) are 122B (126B with CRC) */
  /* This constant should not be changed, because it is used for hardware initialization */
  SOC_TMC_STAT_IF_PKT_SIZE_126B = 1,

  /* Possible packet size (not inculded CRC) are 244B (248B with CRC) */
  /* This constant should not be changed, because it is used for hardware initialization */
  SOC_TMC_STAT_IF_PKT_SIZE_248B = 2,

  /* Possible packet size (not inculded CRC) are 488B (492B with CRC) */
  /* This constant should not be changed, because it is used for hardware initialization */
  SOC_TMC_STAT_IF_PKT_SIZE_492B = 3,

    /* This constant should not be changed, because it is used for hardware initialization */
    SOC_TMC_STAT_IF_PKT_SIZE_1024B = 4,

  /* Number of types in SOC_TMC_STAT_IF_PKT_SIZE */
  SOC_TMC_NOF_STAT_IF_PKT_SIZES = 5

}SOC_TMC_STAT_IF_PKT_SIZE;

typedef enum
{
  /* Possible number of statistic reports per packet */
  SOC_TMC_STAT_IF_NOF_REPORTS_8 = 0,

  SOC_TMC_STAT_IF_NOF_REPORTS_16 = 1,

  SOC_TMC_STAT_IF_NOF_REPORTS_32 = 2,

  SOC_TMC_STAT_IF_NOF_REPORTS_64 = 3,

  SOC_TMC_STAT_IF_NOF_REPORTS_128 = 4

}SOC_TMC_STAT_IF_NOF_STAT_REPORTS_PER_PKT;


typedef enum
{
    STAT_IF_ETPP_COUNTERS_MODE_NONE = 0x0,
    STAT_IF_ETPP_COUNTERS_MODE_LATENCY = 0x1, /* not working for SIF */
    STAT_IF_ETPP_COUNTERS_MODE_PORT = 0x2,
    STAT_IF_ETPP_COUNTERS_MODE_VSI = 0x4, 
    STAT_IF_ETPP_COUNTERS_MODE_LIF = 0x8
}SOC_TMC_STAT_IF_ETPP_COUNTERS_MODE;




typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable the statistics interface
   */
  uint8 enable;
  /*
   *  Select the phase between the incoming data and clock
   */
  SOC_TMC_STAT_IF_PHASE if_phase;
}SOC_TMC_STAT_IF_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Billing mode for the egress report: with Egress Queue
   *  number / CUD / VSI-VLAN / In-LIF and Out-LIF
   */
  SOC_TMC_STAT_IF_BILLING_MODE mode;

} SOC_TMC_STAT_IF_BILLING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Select between Packet-size and Queue-size formats
   */
  SOC_TMC_STAT_IF_FAP20V_MODE mode;
  /*
   *  Multicast report format for the Fabric Multicast: report
   *  of the copies with their Queue number or with their
   *  Multicast-Ids. Valid only if the mode is 'PKT_SIZE'.
   */
  SOC_TMC_STAT_IF_MC_MODE fabric_mc;
  /*
   *  Multicast report format for the Ingress Replication
   *  Multicast: report of the copies with their Queue number
   *  or with their Multicast-Ids. Valid only if the mode is
   *  'PKT_SIZE'.
   */
  SOC_TMC_STAT_IF_MC_MODE ing_mc;
  /*
   *  If True, then the reported packet size is the one at the
   *  packet reception. Otherwise, the reported packet size is
   *  the one after the header editing.
   */
  uint8 is_original_pkt_size;
  /*
   *  If True, then only a single copy per multicast packet
   *  (for ingress replication multicast packets) is reported.
   *  Otherwise, all the packets are reported.
   */
  uint8 ing_mc_is_report_single;
  /*
   *  Applicable for Soc_petra-B only. TRUE - snoop/mirror packets
   *  are also counted in the Copy-Count
   */
  uint8 count_snoop;
  /*
   *  Applicable for Soc_petra-B only. CNM (Congestion
   *  Notification Message) report mode. Valid only if the mode
   *  is 'PKT_SIZE'.
   */
  SOC_TMC_STAT_IF_CNM_MODE cnm_report;

} SOC_TMC_STAT_IF_FAP20V_INFO;

typedef union
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Billing mode configuration
   */
  SOC_TMC_STAT_IF_BILLING_INFO billing;
  /*
   *  Fap20v mode configuration
   */
  SOC_TMC_STAT_IF_FAP20V_INFO fap20v;

} SOC_TMC_STAT_IF_REPORT_MODE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
#ifdef BCM_88650_B0
  /* 
   * Statistics per port enable  
   */
  uint8 stat_if_port_enable[SOC_MAX_NUM_PORTS];
#endif
  /*
   *  Statistics report mode
   */
  SOC_TMC_STAT_IF_REPORT_MODE mode;

  /*
   *  Statistics report interface Rate
   */
  uint32 rate;

  /*
   *  Format mode configuration
   */
  SOC_TMC_STAT_IF_REPORT_MODE_INFO format;
  /*
   *  Statistics' interface sync period. Defines the maximal
   *  period between consecutive sync patterns transmitted on
   *  the statistics interface. Zero disables sync patterns
   *  transmission. Units: Nanoseconds.
   */
  uint32 sync_rate;
  /*
   *  Not applicable for Soc_petra-A. If TRUE, parity checking is
   *  enabled. Reports with parity-errors are discarded. The
   *  parity indications are on expense of some other fields,
   *  as described in the statistics report format
   *  documentation
   */
  uint8 parity_enable;

  /*  is idle reports present
  *  If False, no idle period and set to 0 (StBillNullPrd & StQszIdlePrd) otherwise, to the maximum value. 
  */ 
  uint8  is_idle_reports_present;

  /*
  * If set ,then the billing statistics report includes the original packet size (before header editing), 
  * otherwise the current packet size is reported 
  */ 
  uint8              if_report_original_pkt_size;

  /* Statistics carrying packets have a predefined size */
  SOC_TMC_STAT_IF_PKT_SIZE   if_pkt_size ;

  /* Scrubber bottom queue */  
   uint32              if_scrubber_queue_min; 
  
   /* Scrubber top queue */  
   uint32              if_scrubber_queue_max; 
  
   /* Minimal scrubber report */
   uint32              if_scrubber_rate_min;

   /* Maximal scrubber report */
   uint32              if_scrubber_rate_max;

#ifdef BCM_88650_B0
   /* Qsize bottom queue */  
   uint32              if_qsize_queue_min; 
  
   /* Qsize top queue */  
   uint32              if_qsize_queue_max; 
#endif

   /* Set the Buffer descriptor threshold.
    * Set to -1 to ignore the threshold. Default: -1.
    * Range: 0 - 0x7FFFF and -1.
    * not relevant for QAX */
   uint32              if_scrubber_buffer_descr_th[SOC_TMC_STAT_NOF_THRESHOLD_RANGES];

   /* Set the Buffer descriptor buffers (BDBs) threshold.
    * Set to -1 to ignore the threshold. Default: -1.
    * Range: 0 - 0x3FFFFF and -1.
    */ 
   uint32              if_scrubber_bdb_th[SOC_TMC_STAT_NOF_THRESHOLD_RANGES];

   /* Set the unicast DRAM buffers threshold.
    * Set to -1 to ignore the threshold. Default: -1.
    * Range: 0 - 0x3FFFFF and -1.
    * not relevant for QAX */ 
   uint32              if_scrubber_uc_dram_buffer_th[SOC_TMC_STAT_NOF_THRESHOLD_RANGES];
   /* SRAM buffers threshold, relevant for QAX */
   uint32              if_scrubber_sram_buffers_th[SOC_TMC_STAT_NOF_THRESHOLD_RANGES];   
   /* SRAM packets descriptors buffers threshold, relevant for QAX */   
   uint32              if_scrubber_sram_pdbs_th[SOC_TMC_STAT_NOF_THRESHOLD_RANGES];      
   
   /*  If set, then a single ingress-billing report will be generated for the whole set of copies*/
   uint8               if_report_mc_once;
  /* if set Enable statistics reports on EnQueue - relevent to Ardon only */
   uint8 if_report_enqueue_enable;
  /* if set Enable statistics reports on DeQueue - relevent to Ardon only */
   uint8 if_report_dequeue_enable;
   /* Set the number of statistic reports per packet */
   SOC_TMC_STAT_IF_NOF_STAT_REPORTS_PER_PKT if_nof_reports_per_packet;
   /* Set the report size when working in queue size mode */
   uint8 report_size;
   /* If set, enables queue stamping on ingress reports in billing mode */
   uint8 if_billing_ingress_queue_stamp;
   /* If set, enables drop reason on ingress reports in billing mode */
   uint8 if_billing_ingress_drop_reason;
   /* Filter ingress/egress reports generation */
   uint8 if_billing_filter_reports_ingress;
   uint8 if_billing_filter_reports_egress;
   /* counter mode for statistic interface commands */
   SOC_TMC_STAT_IF_ETPP_COUNTERS_MODE if_etpp_counter_mode[SOC_TMC_STAT_IF_COUNTERS_COMMANDS];
} SOC_TMC_STAT_IF_REPORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First word of the pattern. Range: 0 - 0xFFFFF.
   */
  uint32 word1;
  /*
   *  Second word of the pattern. Range: 0 - 0xFFFFF.
   */
  uint32 word2;

} SOC_TMC_STAT_IF_BIST_PATTERN;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable (in the specified mode)/Disable BIST commands on
   *  the Statistic Interface.
   */
  SOC_TMC_STAT_IF_BIST_EN_MODE en_mode;
  /*
   *  Pattern parameters. Relevant only if 'en_mode' is set to
   *  'PATTERN'.
   */
  SOC_TMC_STAT_IF_BIST_PATTERN pattern;
  /*
   *  Number of successive duplications of the BIST commands
   *  in a single burst. Range: 0 - 63.
   */
  uint32 nof_duplications;
  /*
   *  Number of clock cycles representing the time interval
   *  between two BIST command bursts. Must be a multiple of
   *  2. Range: 0 - 126.
   */
  uint32 nof_idle_clocks;

} SOC_TMC_STAT_IF_BIST_INFO;


typedef struct {
   /* packet size */
   uint32 packet_size ;

   /* records in packet */
   uint32 records_in_packet ;

} SOC_TMC_STAT_IF_PACKET_INFO;

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
  SOC_TMC_STAT_IF_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_INFO *info
  );

void
  SOC_TMC_STAT_IF_BILLING_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_BILLING_INFO *info
  );

void
  SOC_TMC_STAT_IF_FAP20V_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_FAP20V_INFO *info
  );

void
  SOC_TMC_STAT_IF_REPORT_MODE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_REPORT_MODE_INFO *info
  );

void
  SOC_TMC_STAT_IF_REPORT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_REPORT_INFO *info
  );

void
  SOC_TMC_STAT_IF_BIST_PATTERN_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_BIST_PATTERN *info
  );

void
  SOC_TMC_STAT_IF_BIST_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IF_BIST_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_STAT_IF_PHASE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_PHASE enum_val
  );

const char*
  SOC_TMC_STAT_IF_REPORT_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_REPORT_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_BILLING_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_BILLING_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_FAP20V_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_FAP20V_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_MC_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_MC_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_CNM_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_CNM_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_PARITY_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_PARITY_MODE enum_val
  );

const char*
  SOC_TMC_STAT_IF_BIST_EN_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_STAT_IF_BIST_EN_MODE enum_val
  );

void
  SOC_TMC_STAT_IF_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_INFO *info
  );

void
  SOC_TMC_STAT_IF_BILLING_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_BILLING_INFO *info
  );

void
  SOC_TMC_STAT_IF_FAP20V_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_FAP20V_INFO *info
  );

void
  SOC_TMC_STAT_IF_REPORT_MODE_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_REPORT_MODE_INFO *info
  );

void
  SOC_TMC_STAT_IF_REPORT_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_REPORT_INFO *info
  );

void
  SOC_TMC_STAT_IF_BIST_PATTERN_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_BIST_PATTERN *info
  );

void
  SOC_TMC_STAT_IF_BIST_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IF_BIST_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_STAT_IF_INCLUDED__*/
#endif
