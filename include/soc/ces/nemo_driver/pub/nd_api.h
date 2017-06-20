/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_API_H__
#define __NEMO_API_H__


#include "../nd_platform.h"
#include "../nd_results.h"
#include "../nd_protocols.h"
#include "../nd_framer.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef AgResult (*AgNdApi)(AgNdDevice*, void*);

typedef struct 
{
    AG_CHAR         a_name[80];         /* opcode name */
    AgNdApi         p_api;              /* callback */
    AgNdDeviceState e_required_state;   /* required device state */

    /* */
    /* profiling information */
    /* */
    AG_U32          n_prof_min;     /* min call time (usec) */
    AG_U32          n_prof_max;     /* max call time (usec) */
    AG_U32          n_prof_avg;     /* avg call time (usec) */
    AG_U32          n_prof_cnt;     /* number of calls */

} AgNdOpcodeTableEntry;


/*ORI*/
/*add opcode to config framer and pm*/
typedef enum
{
    AG_ND_OPCODE_CONFIG_INIT,
    AG_ND_OPCODE_CONFIG_CHIP_INFO,
    AG_ND_OPCODE_CONFIG_GLOBAL,
    AG_ND_OPCODE_CONFIG_NEMO_CCLK,
    AG_ND_OPCODE_CONFIG_NEMO_CIRCUIT,
    AG_ND_OPCODE_CONFIG_NEMO_BRG,
    AG_ND_OPCODE_CONFIG_NEPTUNE_OC3_HIERARCHY,
    AG_ND_OPCODE_CONFIG_NEPTUNE_CIRCUIT,
    AG_ND_OPCODE_CONFIG_NEPTUNE_BSG,
    AG_ND_OPCODE_CONFIG_CAS_DATA_REPLACE,
    AG_ND_OPCODE_CONFIG_CIRCUIT_ENABLE,
	AG_ND_OPCODE_CONFIG_JITTER_BUFFER_PARAMS,
    AG_ND_OPCODE_CONFIG_CHANNEL_EGRESS,
    AG_ND_OPCODE_CONFIG_CHANNEL_INGRESS,
    AG_ND_OPCODE_CONFIG_CHANNELIZER,
    AG_ND_OPCODE_CONFIG_CHANNEL_ENABLE,
    AG_ND_OPCODE_CONFIG_CONTROL_WORD,
    AG_ND_OPCODE_CONFIG_CHANNEL_PME,
    AG_ND_OPCODE_CONFIG_TIMESLOT,
    AG_ND_OPCODE_CONFIG_MAC,
    AG_ND_OPCODE_CONFIG_PHY,
    AG_ND_OPCODE_CONFIG_RPC_PORT_FWD,
    AG_ND_OPCODE_CONFIG_RPC_UCODE,
    AG_ND_OPCODE_CONFIG_RPC_POLICY,
    AG_ND_OPCODE_CONFIG_RPC_MAP_LABEL_TO_CHID,
    AG_ND_OPCODE_CONFIG_COMPUTE_RING_SIZE,
    AG_ND_OPCODE_CONFIG_DESTINATION_MAC,
    AG_ND_OPCODE_CONFIG_CAS_CHANNEL_INGRESS,
    AG_ND_OPCODE_CONFIG_CAS_CHANNEL_ENABLE,

    AG_ND_OPCODE_COMMAND_JBF_SLIP,
    AG_ND_OPCODE_COMMAND_JBF_RESTART,
    AG_ND_OPCODE_COMMAND_STOP_TRIMMING,
    AG_ND_OPCODE_COMMAND_CAS_CHANNEL_TX,

    AG_ND_OPCODE_DIAG,
    AG_ND_OPCODE_DIAG_BIT,
    AG_ND_OPCODE_DIAG_HW_TEST,

    AG_ND_OPCODE_DEBUG,

    AG_ND_OPCODE_PM_INGRESS,
    AG_ND_OPCODE_PM_EGRESS,
    AG_ND_OPCODE_PM_GLOBAL,
    AG_ND_OPCODE_PM_MAC,

    AG_ND_OPCODE_STATUS_INGRESS,
    AG_ND_OPCODE_STATUS_EGRESS,
    AG_ND_OPCODE_STATUS_NEPTUNE_CIRCUIT,
    AG_ND_OPCODE_STATUS_CAS_DATA,
    AG_ND_OPCODE_STATUS_CAS_CHANGE,

    AG_ND_OPCODE_RCR_CONFIG_TPP_TIMESTAMP_RATE,
    AG_ND_OPCODE_RCR_TIMESTAMP,
    AG_ND_OPCODE_RCR_CONFIG_CHANNEL,
    AG_ND_OPCODE_RCR_FLL,
    AG_ND_OPCODE_RCR_PLL,
    AG_ND_OPCODE_RCR_RECENT_PACKET,
    AG_ND_OPCODE_RCR_PM,

    AG_ND_OPCODE_REGISTER_ACCESS,

	AG_ND_OPCODE_CONFIG_NEPTUNE_DCR_CONFIGURATIONS,
	AG_ND_OPCODE_CONFIG_BSG,
    AG_ND_OPCODE_DIFF_TSO_POLL,

    AG_ND_OPCODE_CONFIG_NEMO_DCR_CONFIGURATIONS,
    AG_ND_OPCODE_CONFIG_NEMO_DCR_CLK_SOURCE,
	AG_ND_OPCODE_CONFIG_NEMO_DCR_SYSTEM_TSI_CLK_SOURCE,
    AG_ND_OPCODE_CONFIG_NEMO_DCR_LOCAL_TS_SAMPLE_PERIOD,
	AG_ND_OPCODE_NEMO_DCR_PRIME_LOCAL_TS,
	AG_ND_OPCODE_NEMO_DCR_SYSTEM_PRIME_LOCAL_TS,
	AG_ND_OPCODE_NEMO_DCR_LOCAL_TS_AND_FAST_PHASE_TS,
	AG_ND_OPCODE_NEMO_DCR_SYSTEM_LOCAL_TS_AND_FAST_PHASE_TS,
	AG_ND_OPCODE_CONFIG_NEMO_DCR_LOCAL_SAMPLE_PERIOD,
	AG_ND_OPCODE_CONFIG_NEMO_DCR_SYSTEM_LOCAL_SAMPLE_PERIOD,
	AG_ND_OPCODE_CONFIG_NEMO_DCR_SYSTEM_TSO_MODE,

    AG_ND_OPCODE_CONFIG_PTP_TSG,
    AG_ND_OPCODE_CONFIG_PTP_STATELESS,
    AG_ND_OPCODE_CONFIG_PTP_TSG_ENABLE,
    AG_ND_OPCODE_CONFIG_PTP_CLK_SOURCE,
    AG_ND_OPCODE_CONFIG_PTP_BRG,
    AG_ND_OPCODE_CONFIG_PTP_CHANNEL_INGRESS,
    AG_ND_OPCODE_CONFIG_PTP_CHANNEL_EGRESS,
    AG_ND_OPCODE_CONFIG_PTP_CHANNEL_ENABLE_INGRESS,
    AG_ND_OPCODE_CONFIG_PTP_CHANNEL_ENABLE_EGRESS,
    AG_ND_OPCODE_COMMAND_PTP_CORRECT_TIMESTAMP,
	AG_ND_OPCODE_COMMAND_PTP_ADJUSTMENT,
	AG_ND_OPCODE_COMMAND_PTP_READ_COUNTERS,
	AG_ND_OPCODE_CONFIG_SEQUENCE_NUMBER,
	AG_ND_OPCODE_CONFIG_VLAN_EGRESS,

	AG_ND_OPCODE_CHANNEL_PME_MISSIMG_STATUS,
	AG_ND_OPCODE_FRAMER_PORT_CONFIG,
	AG_ND_OPCODE_FRMER_PORT_STATUS,
	AG_ND_OPCODE_FRAMER_PORT_ALARM_CONTROL,
	AG_ND_OPCODE_FRAMER_PORT_PM_READ,
	AG_ND_OPCODE_FRAMER_PORT_TIME_SLOT_LOOPBACK_CONTROL,
	AG_ND_OPCODE_FRAMER_PORT_LOOPBACK_CONTROL,

    AG_ND_OPCODE_MAX

} AgNdOpcode;



typedef AG_U32  AgNdHandle;


typedef struct
{
    AG_U32              n_status_polarity;
    AG_U32              n_drop_unconditional;
    AG_U32              n_drop_if_not_forward;
    AG_U32              n_forward_to_ptp;
    AG_U32              n_forward_to_host_high;
    AG_U32              n_forward_to_host_low;
    AG_U32              n_channel_counter_1;
    AG_U32              n_channel_counter_2;
    AG_U32              n_channel_counter_3;
    AG_U32              n_channel_counter_4;
    AG_U32              n_global_counter_1;
    AG_U32              n_global_counter_2;
    AG_U32              n_global_counter_3;
    AG_U32              n_global_counter_4;
    AG_U32              n_global_counter_5;
    AG_U32              n_global_counter_6;
    AG_U32              n_global_counter_7;
    AG_U32              n_global_counter_8;

} AgNdRpcPolicy;


typedef struct AgNdMsgOpen
{
    AG_U32              n_base;
    AG_BOOL             b_use_hw;

    AG_U32              n_ext_mem_bank0_size; 
    AG_U32              n_ext_mem_bank1_size; 
    
} AgNdMsgOpen;


typedef enum
{
    AG_ND_OPFLAG_NOLOCK           = 0x01

} AgNdOpFlag;


AgResult ag_nd_module_trace(AG_U32 n_mask, AG_U16 n_level, AG_U32 *p_busAddr, AG_U16 n_busAddrSize);
AgResult ag_nd_module_create(void);
AgResult ag_nd_module_remove(void);

AgResult ag_nd_device_open(AgNdMsgOpen *p_msg, AgNdHandle *p_handle);
AgResult ag_nd_device_close(AgNdHandle);

AgResult ag_nd_device_read(AgNdHandle, AG_U16 n_opcode, void *p_msg);
AgResult ag_nd_device_write(AgNdHandle, AG_U16 n_opcode, void *p_msg);
AgResult ag_nd_device_read_ext(AgNdHandle, AG_U16 n_opcode, void *p_msg, AG_U32 n_flags);
AgResult ag_nd_device_write_ext(AgNdHandle, AG_U16 n_opcode, void *p_msg, AG_U32 n_flags);

AgNdCircuit ag_nd_create_circuit_id_nemo(AG_U32 n_port);
AgNdCircuit ag_nd_create_circuit_id_neptune(AG_U32 n_spe, AG_U32 n_vtg, AG_U32 n_vt);

void     ag_nd_interrupts_process(AgNdHandle n_handle);

int ag_nd_write_tx_signaling(unsigned long n_circuit_id,unsigned long n_ts,unsigned char n_new_ABCD);
int ag_nd_read_tx_signaling(unsigned long n_circuit_id,unsigned long n_ts,unsigned char * p_ABCD);
int ag_nd_read_rx_signaling(unsigned long n_circuit_id,unsigned long n_ts,unsigned char * p_ABCD);


#define AG_ND_IS_BAD_CIRCUIT_ID(cid)     ((AgNdCircuit)-1 == (cid))

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHIP_INFO */
/* */
typedef struct
{
    AG_U32                  n_architecture;
    AG_U32                  n_fpga_code_id;
    AG_U32                  n_fpga_revision;
    AG_U32                  n_total_channels;
    AG_U32                  n_total_ports;

} AgNdMsgChipInfo;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_INIT */
/* */
typedef AgResult (*AgNdPmiCbFunc)(void);
typedef AgResult (*AgNdPbiCbFunc)(AG_U32 n_channel_id, AG_U32 n_user_data);
typedef AgResult (*AgNdCwiCbFunc)(AG_U32 n_channel_id, AG_U32 n_user_data, AG_U16 n_cw);
typedef AgResult (*AgNdPsiCbFunc)(AG_U32 n_channel_id, AG_U32 n_user_data, AgNdSyncType n_sync);
typedef AgResult (*AgNdTpiCbFunc)(AG_U32 n_channel_id, AG_U32 user_data);

typedef struct AgNdMsgConfigInit
{
	AG_BOOL 				b_dynamic_memory;
    AgNdTdmBitOrder         e_bit_order;
    AgNd1SecDirection       e_one_sec_pulse_direction;

    AG_U32                  n_pbf_max;
    AG_U32                  n_pw_max;
    AG_U32                  n_packet_max;

    AG_BOOL                 b_isr_mode;
    AG_U32                  n_isr_task_priority;
    AG_U32                  n_isr_task_wakeup;

    AgNdPmiCbFunc           p_cb_pmi;

    AgNdPbiCbFunc           p_cb_pbi;
    AG_U32                  n_user_data_pbi;

    AgNdCwiCbFunc           p_cb_cwi;
    AG_U32                  n_user_data_cwi;

    AgNdPsiCbFunc           p_cb_psi;
    AG_U32                  n_user_data_psi;    

    AgNdTpiCbFunc           p_cb_tpi;
    AG_U32                  n_user_data_tpi;

    AG_BOOL                 b_rcr_support;
    AG_BOOL                 b_ptp_support;

} AgNdMsgConfigInit;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_GLOBAL */
/* */
typedef enum
{
    AG_ND_PRP_NO_CHANGE   = 0,
    AG_ND_PRP_FIRST       = AG_ND_PRP_NO_CHANGE,

    AG_ND_PRP_IDLE_BYTE   = 1,
    AG_ND_PRP_FILLER_BYTE = 2,

    AG_ND_PRP_ALL_ONES    = 3,
    AG_ND_PRP_LAST        = AG_ND_PRP_ALL_ONES
    
} AgNdPr;

typedef struct
{
    AgNdPr                  e_missing;
    AgNdPr                  e_l_bit;
    AgNdPr                  e_lops;
    AgNdPr                  e_rai_rdi;

} AgNdPrPolicy;

typedef enum
{
    AG_ND_PME_JIF   = 0x080,
    AG_ND_PME_JPM   = 0x040,
    AG_ND_PME_PWO   = 0x020,
    AG_ND_PME_PWI   = 0x010,
    AG_ND_PME_CL1   = 0x008,
    AG_ND_PME_CL2   = 0x004,
    AG_ND_PME_CL3   = 0x002,
    AG_ND_PME_CL4   = 0x001

} AgNdPmeSelect;

typedef struct
{
    AG_U32                  n_channels;
    AG_U32                  n_counters;
    AG_BOOL                 b_auto_export;

} AgNdPmeGroup;

typedef enum
{
    AG_ND_RAI_DETECT_M  = 0x0002,
    AG_ND_RAI_DETECT_R  = 0x0001

} AgNdRaiDetect;

typedef struct 
{
    AG_U8                   n_filler_byte;
    AG_U8                   n_idle_byte;
    AgNdPrPolicy            x_pr_structured;
    AgNdPrPolicy            x_pr_unstructured;

    AG_BOOL                 b_lan_dce_mode_enable;
    AG_BOOL                 b_wan_dce_mode_enable;

    AG_U16                  a_lops_threshold_table[AG_ND_SYNC_THRESHOLD_TABLE_SIZE];
    AG_U16                  a_aops_threshold_table[AG_ND_SYNC_THRESHOLD_TABLE_SIZE];

    AG_BOOL                 b_trimming;

    AG_BOOL                 b_hpl_tx_enable;
    AG_BOOL                 b_hpl_rx_enable;

    AG_BOOL                 b_pme_enable;
    AgNdPmeGroup            x_pme_group[AG_ND_PME_GROUP_MAX];

    AG_U16                  n_cw_mask;

    AG_U8                   n_cas_e1_idle_pattern;
    AG_U8                   n_cas_t1_idle_pattern;
    AG_U32                  n_cas_no_change_delay;
    AG_U32                  n_cas_change_delay;
    AG_U32                  n_cas_sqn_window;

    AG_U32                  n_rai_detect;

} AgNdMsgConfigGlobal;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_CIRCUIT */
/*  */
typedef struct 
{
    AgNdCircuit             n_circuit_id;
    AG_BOOL                 b_enable;

    AG_BOOL                 b_prime_enable;

    AgNdTdmProto            e_tdm_protocol;
    AG_BOOL                 b_structured;
    AG_BOOL                 b_octet_aligned;

    AG_BOOL                 b_signaling_enable;
    AG_BOOL                 b_T1_D4_framing;

    AgNdRxClkSelect         e_clk_rx_select;
    AgNdTxClkSelect         e_clk_tx_select;

} AgNdMsgConfigNemoCircuit;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_BRG */
/*  */
typedef struct 
{
    AgNdCircuit             n_circuit_id;
    AG_U32                  n_step_size;

} AgNdMsgConfigNemoBrg;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_CCLK */
/*  */
typedef struct 
{ 
    AG_BOOL                 b_cclk_enable;
    AgNdCClkSelect          e_cclk_select;
    AgNdTdmProto            e_ref_clk_proto;
    AgNdRefClkSelect        e_ref_clk_1_select;
    AgNdRefClkSelect        e_ref_clk_2_select;
    AG_U32                  n_ref_clk_1_port;
    AG_U32                  n_ref_clk_2_port;
    AG_U32                  n_ref_clk_1_brg;
    AG_U32                  n_ref_clk_2_brg;
    AgNdPtpClkSelect        e_ref_clk_1_ptp;
    AgNdPtpClkSelect        e_ref_clk_2_ptp;
    AgNdExtClkDir           e_ext_clk_1_dir;
    AgNdExtClkDir           e_ext_clk_2_dir;

} AgNdMsgConfigNemoCClk;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CAS_DATA_REPLACE */
/*  */
typedef struct 
{ 
    AgNdCircuit             n_circuit_id;
    AG_U32                  n_cas_idle_timeslots;

} AgNdMsgConfigCasDataReplace;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEPTUNE_OC3_HIERARCHY */
/* */
typedef struct 
{
    AgNdSpeMode             n_mode;
    AgNdTdmProto            a_vtg_mode[AG_ND_VTG_MAX];

} AgNdSpe;

typedef struct
{
    AgNdSpe                 x_spe[AG_ND_SPE_MAX];

} AgNdMsgConfigNeptuneOc3Hierarchy;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEPTUNE_CIRCUIT */
/* */
typedef struct
{
    AgNdCircuit             n_circuit_id;

    AG_BOOL                 b_structured;
    AG_BOOL                 b_octet_aligned;

    AG_BOOL                 b_cas_enable;
    AG_BOOL                 b_prime;
    AG_BOOL                 b_T1_D4_framing;

    AG_BOOL                 b_clock_master;

    AG_BOOL                 b_enable;

} AgNdMsgConfigNeptuneCircuit;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEPTUNE_BSG */
/* */
typedef struct
{
    AgNdCircuit             n_circuit_id;
    AG_U32                  n_step_size;
	AG_S32					n_bsg_count;
} AgNdMsgConfigNeptuneBsg;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_STATUS_NEPTUNE_CIRCUIT */
/* */
typedef struct
{
    AgNdCircuit             n_circuit_id;
    AG_BOOL                 b_adjustment_in_progress;
    AG_U32                  n_phase_shift;

} AgNdMsgStatusNeptuneCircut;
 
/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CIRCUIT_ENABLE */
/* */
typedef struct
{
    AgNdCircuit             n_circuit_id;
    AG_BOOL                 b_enable;

} AgNdMsgConfigCircuitEnable;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_TIMESLOT */
/* */
typedef struct 
{
    AgNdPath                e_path;
    AgNdTsId                x_id;
    AG_BOOL                 b_first;
    AG_BOOL                 b_enable;
    AgNdChannel             n_channel_id;

} AgNdMsgConfigTimeslot;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHANNELIZER */
/* */
typedef struct
{
    AgNdTsId                a_ts[AG_ND_SLOT_MAX * 84];  
    AG_U32                  n_size;                                    
    AG_U32                  n_first;

} AgNdChMap;

typedef struct 
{
    AgNdChannel             n_channel_id;
    AgNdPath                e_path;
    AgNdChMap               x_map;

} AgNdMsgConfigChannelizer;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHANNEL_ENABLE */
/* */
typedef struct
{
    AgNdPath                e_path;
    AgNdChannel             n_channel_id;
    AG_BOOL                 b_enable;
	AG_U32					n_jb_size_milli;

} AgNdMsgConfigChannelEnable;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHANNEL_INGRESS */
/* */
typedef struct 
{
    /* */
    /* rw parameters */
    /* */
    AgNdChannel             n_channel_id;
    AG_U32                  n_payload_size;
    AgNdPacketHeader        x_header;
    AG_BOOL                 b_dba;
    AG_BOOL                 b_auto_r_bit;
    AG_BOOL                 b_mef_len_support;
    AG_U32                  n_pbf_size;
	AG_BOOL					b_redundancy_enabled;
	AG_BOOL					b_redundancy_config;

    /* */
    /* ro parameters */
    /* */
    AG_BOOL                 b_enable;
    AG_U32                  n_pbf_addr;
	AG_U16                  n_ces_sqn;

} AgNdMsgConfigChannelIngress;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHANNEL_EGRESS */
/* */
typedef struct 
{
    /* */
    /* rw parameters */
    /* */
    AgNdChannel             n_channel_id;

    AG_U32                  n_packet_sync_selector;
    AG_U32                  n_payload_size;
    AG_U32                  n_jbf_ring_size;
    AG_U32                  n_jbf_win_size;
    AG_U32                  n_jbf_bop;

    AG_BOOL                 b_rtp_exists;
    AgNdPacketHeader        x_strict_data;

	AG_BOOL					b_drop_on_valid;

    
    /* */
    /* ro parameters */
    /* */
    AG_BOOL                 b_enable;
    AG_U32                  n_jbf_addr;   
    AG_U32                  n_vba_addr;


} AgNdMsgConfigChannelEgress;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_JITTER_BUFFER_PARAMS */
/* */
typedef struct 
{
    /* */
    /* rw parameters */
    /* */
    AgNdChannel             n_channel_id;

    AG_U32                  n_jbf_ring_size;
    AG_U32                  n_jbf_win_size;
    AG_U32                  n_jbf_bop;

} AgNdMsgConfigJitterBufferParams;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_STATUS_EGRESS */
/* */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AgNdSyncType            e_sync_state;
    AgNdJbfState            e_jbf_state;
    AG_U32                  n_ces_cw;
    AG_BOOL                 b_trimming;
    AG_BOOL                 b_slip;

} AgNdMsgStatusEgress;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_STOP_TRIMMING */
/* */
typedef struct
{
    AgNdChannel             n_channel_id;

} AgNdMsgCommandStopTrimming;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_JBF_SLIP */
/* */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U32                  n_size;
    AgNdSlipUnit            e_unit;
    AgNdSlipDir             e_dir;

} AgNdMsgCommandJbfSlip;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_JBF_RESTART */
/* */
typedef struct
{
    AgNdChannel             n_channel_id;

} AgNdMsgCommandJbfRestart;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DIAG  RW */
/* */
typedef enum
{
    AG_ND_LOOPBACK_NONE,
    AG_ND_LOOPBACK_TDM_REMOTE,
    AG_ND_LOOPBACK_TDM_LOCAL,
    AG_ND_LOOPBACK_CHANNELIZER,
    AG_ND_LOOPBACK_DECHANNELIZER,
    AG_ND_LOOPBACK_CLASSIFIER,
    AG_ND_LOOPBACK_PACKET_INTERFACE,
    AG_ND_LOOPBACK_PACKET_LOCAL,
    AG_ND_LOOPBACK_PACKET_REMOTE

} AgNdLoopback;


typedef struct 
{
    /* e_loopbacks */
    AgNdLoopback            e_loopback;
    AG_U32                  n_loopback_channel_select;
    AG_U32                  n_loopback_port_select;

    /* activities */
    AG_BOOL                 b_act_1_sec_pulse;
    AG_BOOL                 b_act_nomad_brg_clk;
    AG_BOOL                 b_act_miit_clk;
    AG_BOOL                 b_act_miir_clk;
    AG_BOOL                 b_act_ref_clk1;
    AG_BOOL                 b_act_ref_clk2;
    AG_BOOL                 b_act_ext_clk_sync;
    AG_BOOL                 b_act_int_cclk;

    AG_U16                  n_act_port;

    /* other */
    AgNdOscillatorType      e_oscillator;

} AgNdMsgDiag;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DIAG_BIT  RW */
/* */
typedef enum
{
    AG_ND_BIT_VERBOSE_LEVEL_QUIET          = 0x0000,
    AG_ND_BIT_VERBOSE_LEVEL_BRIEF          = 0x0001,
    AG_ND_BIT_VERBOSE_LEVEL_DETAILED       = 0x0002,
    AG_ND_BIT_VERBOSE_LEVEL_INSANE         = 0x0003

} AgNdBitVerboseLevel;

typedef struct 
{
    AgResult                (*p_print)(AG_CHAR *);
    AG_U32                  n_verbose;
    AG_U32                  n_msg_max;
    AG_BOOL                 b_passed;

} AgNdMsgBit;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DEBUG  RW */
/* */
typedef enum
{
    AG_ND_TRACE_NONE         = 0x0000,
    AG_ND_TRACE_ERROR        = 0x0001,
    AG_ND_TRACE_WARNING      = 0x0002,
    AG_ND_TRACE_DEBUG        = 0x0003

} AgNdTraceLevel;

typedef enum
{
    AG_ND_TRACE_BUS_ALL      = 0x0001,
    AG_ND_TRACE_BUS_SPECIFIC = 0x0002,
    AG_ND_TRACE_API          = 0x0004,
    AG_ND_TRACE_PERIODIC     = 0x0008,
    AG_ND_TRACE_ALL          = 0xffff

} AgNdTraceMask;


typedef struct 
{
    /* testbus */
    AG_U16              n_select_block;
    AG_U16              n_select_group;
    AG_BOOL             b_trace;
    AG_BOOL             b_rclr_pm_counters;

} AgNdMsgDebug;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_PM        RO */
/* */
typedef struct
{
    AG_BOOL             a_enable[AG_ND_PATH_MAX][AG_ND_CHANNEL_MAX];
    /* */
    /* ingress path */
    /* */
    AG_U16              a_pbf_utilization[AG_ND_CHANNEL_MAX];   /* PBFSTAT */

    AG_U32              a_tpe_pwe_out_byte_counter[AG_ND_CHANNEL_MAX];   /* PWIRBC */
    AG_U16              a_tpe_pwe_out_packet_counter[AG_ND_CHANNEL_MAX]; /* PWIRPC */
    
    /* */
    /* egress path */
    /* */
    AG_U32              a_jbf_depth_min[AG_ND_CHANNEL_MAX];       /* JBDMIN */
    AG_U32              a_jbf_depth_max[AG_ND_CHANNEL_MAX];        /* // JBDMIN */

    AG_U8               a_jbf_underrun_counter[AG_ND_CHANNEL_MAX];         /* JBPERFC */
    AG_U8               a_jbf_missing_packet_counter[AG_ND_CHANNEL_MAX];    /* JBPERFC */
    AG_U8               a_jbf_dropped_ooo_packet_counter[AG_ND_CHANNEL_MAX];     /* PORDRC */
    AG_U8               a_jbf_reordered_ooo_packet_counter[AG_ND_CHANNEL_MAX];   /* PORDRC */
    AG_U8               a_jbf_bad_length_packet_counter[AG_ND_CHANNEL_MAX];   /* PWIBLPC */
    AG_U32              a_rpc_pwe_in_byte_counter[AG_ND_CHANNEL_MAX];   /* PWIRBC */
    AG_U16              a_rpc_pwe_in_packet_counter[AG_ND_CHANNEL_MAX]; /* PWIRPC */
    AG_U8               a_rpc_pwe_in_error_packet_count[AG_ND_CHANNEL_MAX]; /* PWIERRPC */

    AG_U8               a_rpc_channel_specific_count[AG_ND_CHANNEL_MAX][AG_ND_RPC_CCNT_MAX]; /* CLGPMC */

    AgNdJbfState        a_jbf_state[AG_ND_CHANNEL_MAX];

} AgNdMsgPm;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_RCR_TIMESTAMP   RO */
/* */
typedef struct
{
    AG_U32              n_ts_125;
    AG_U32              n_ts_77;

} AgNdMsgRcrTs;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_RCR_CONFIG_CHANNEL      RW */
/* */
typedef enum
{
    AG_ND_RCR_TS_SAMPLE_PACKET  =   0x0,
    AG_ND_RCR_TS_SAMPLE_PERIOD  =   0x1 

} AgNdRcrTsSample;

typedef struct
{
    AgNdChannel         n_psn_channel_id;
    AgNdChannel         n_rcr_channel_id;

    AgNdRcrTsSample     e_rtp_ts_sample_period;

    AG_BOOL             b_pll_peak_detector_enable;
    AG_BOOL             b_fll_peak_detector_enable;

    AG_BOOL             b_phase_slope_limit_enable;
    AG_U16              n_phase_slope_threshold;

    AG_U16              n_sampling_period; /* number of packets - must be power of 2 in range 64..8192 */

    AG_BOOL             b_enable;

} AgNdMsgRcrConfigChannel;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_RCR_CONFIG_TPP_TIMESTAMP_RATE */
/* */
typedef struct
{
    AgNdRcrTs125Rate    e_tsr_125;
    AgNdRcrTs77Rate     e_tsr_77;

} AgNdMsgRcrConfigTppTimestampRate;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_RCR_PM             RO */
/* */
typedef struct
{
    AgNdChannel         n_rcr_channel_id;
    AG_U16              n_phase_slope_limiter_dropped;
    AG_U16              n_peak_detector_dropped;

} AgNdMsgRcrPm;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_RCR_FLL                RO */
/* */
typedef struct 
{
    AgNdChannel         n_rcr_channel_id;
    AG_U32              n_timestamp_base;
    AG_U32              n_timestamp_delta_sum;
    AG_U16              n_timestamp_count;
    AG_U32              n_sqn_filler_num;

} AgNdMsgRcrFll;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_RCR_PLL                RO */
/* */
typedef struct 
{
    AgNdChannel         n_rcr_channel_id;
    AG_U16              n_jbf_depth_count;
    AG_U32              n_jbf_depth_sum;
    AG_U32              n_jbf_depth_max;

} AgNdMsgRcrPll;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_RCR_RECENT_PACKET      RO */
/* */
typedef struct 
{
    AgNdChannel         n_rcr_channel_id;
    AG_U32              n_timestamp;

} AgNdMsgRcrRecent;


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_MAC */
/* */
typedef struct 
{
    AG_BOOL                 b_rx_error_discard_enable;
    AG_BOOL                 b_ena_10mbps;                           /* unsupported */
    AG_BOOL                 b_no_length_check;
    AG_BOOL                 b_control_frame_enable;
    AG_BOOL                 b_node_wake_up_request_indication;      /* read only */
    AG_BOOL                 b_put_core_in_sleep_mode;
    AG_BOOL                 b_enable_magic_packet_detection;
    AgNdMacCCTxAddrSel      e_tx_address_selection;                 /* unsupported */
    AG_BOOL                 b_enable_loopback;                      /* unsupported */
    AG_BOOL                 b_hash_24_bit_only;                     /* unsupported */
    AG_BOOL                 b_software_reset;
    AG_BOOL                 b_is_late_collision_condition;
    AG_BOOL                 b_is_excessive_collision_condition;
    AG_BOOL                 b_enable_half_duplex;
    AG_BOOL                 b_insert_mac_addr_on_transmit;
    AG_BOOL                 b_ignore_pause_frame_quanta;
    AG_BOOL                 b_fwd_pause_frames;
    AG_BOOL                 b_fwd_crc_field;
    AG_BOOL                 b_enable_frame_padding;
    AG_BOOL                 b_enable_promiscuous_mode;
    AG_BOOL                 b_enable_gigabit_ethernet;              /* unsupported */
    AG_BOOL                 b_enable_mac_receive;
    AG_BOOL                 b_enable_mac_transmit;
                           
} AgNdMacCmdConfig;        
                           
typedef struct             
{                          
    AG_U16                  n_core_revision;
    AG_U16                  n_customer_specific_revision;
    AG_U32                  n_scratch;
    AgNdMacCmdConfig        x_command_config;
    AG_U8                   a_mac[6];
    AG_U16                  n_frame_length;
    AG_U16                  n_pause_quanta;
    AG_U32                  n_rx_section_empty;
    AG_U32                  n_rx_section_full;
    AG_U32                  n_tx_section_empty;
    AG_U32                  n_tx_section_full;
    AG_U32                  n_rx_almost_empty;
    AG_U32                  n_rx_almost_full;
    AG_U32                  n_tx_almost_empty;
    AG_U32                  n_tx_almost_full;
    AG_U8                   n_mdio_address_0;
/*    AG_U8                   n_mdio_address_1; */
    AG_U32                  n_reg_stat;
    AG_U8                   n_tx_inter_packet_gap_length;

} AgNdMsgConfigMac;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_PHY */
/* */
typedef struct
{
    AG_U32                  n_reg_idx;
    AG_U16                  n_reg_value;

} AgNdMsgConfigPhy;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_PM_MAC */
/* */
typedef struct 
{
    AG_U8                   a_mac_id[6];
    AG_U32                  n_frames_transmitted_ok;
    AG_U32                  n_frames_received_ok;
    AG_U32                  n_frame_check_sequence_errors;
    AG_U32                  n_alignment_errors;
    AG_U32                  n_octets_transmitted_ok;
    AG_U32                  n_octets_received_ok;
    AG_U32                  n_tx_pause_mac_ctrl_frames;
    AG_U32                  n_rx_pause_mac_ctrl_frames;
    AG_U32                  n_if_in_errors;
    AG_U32                  n_if_out_errors;
    AG_U32                  n_if_in_ucast_pkts;
    AG_U32                  n_if_in_multicast_pkts;
    AG_U32                  n_if_in_broadcast_pkts;
    AG_U32                  n_if_out_disacrds;
    AG_U32                  n_if_out_ucast_pkts;
    AG_U32                  n_if_out_multicast_pkts;
    AG_U32                  n_if_out_broadcast_pkts;
    AG_U32                  n_ether_stats_drop_events;
    AG_U32                  n_ether_stats_octets;
    AG_U32                  n_ether_stats_pkts;
    AG_U32                  n_ether_stats_oversize_pkts;
    AG_U32                  n_ether_stats_undersize_pkts;
    AG_U32                  n_ether_stats_pkts_64_octets;
    AG_U32                  n_ether_stats_pkts_65_to_127_octets;
    AG_U32                  n_ether_stats_pkts_128_to_255_octets;
    AG_U32                  n_ether_stats_pkts_256_to_511_octets;
    AG_U32                  n_ether_stats_pkts_512_to_1023_octets;
    AG_U32                  n_ether_stats_pkts_1024_to_1518_octets;
    
} AgNdMsgPmMac;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_RPC_UCODE */
/* */
typedef struct
{
    AG_U8                   a_dest_mac[6];
    AG_U32                  n_dest_ipv4;
    AG_U8                   a_dest_ipv6[16];
    AG_BOOL                 b_udp_direct;
    AG_BOOL                 b_mpls_direct;
    AG_BOOL                 b_ecid_direct;
	AG_U16					n_vlan_mask;
    #ifdef CES16_BCM_VERSION /*TODl2TP*/
    AG_BOOL                 b_l2tp_direct;
    #endif

} AgNdMsgConfigRpcUcode;

/* $Id$
 * Copyright 2011 BATM
 */


/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_RPC_PORT_FWD */
/* */
typedef enum
{
    AG_ND_RPC_FWD_HP        = 0x0,

    AG_ND_RPC_FWD_LP        = 0x1,

    AG_ND_RPC_FWD_NONE      = 0x2,

} AgNdRpcFwdAction;

typedef struct 
{
    AG_U16                  n_port;
    AgNdRpcFwdAction        e_action;

} AgNdMsgConfigRpcPortFwd;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_RPC_MAP_LABEL_TO_CHID */
/* */
typedef struct
{
    AgNdChannel             n_channel_id;
    AG_BOOL                 b_enable;
    AgNdEncapsulation       e_encapsulation;
    AG_U32                  n_label;
    AG_U8                   a_ptp_port[10];

} AgNdMsgConfigRpcMapLabelToChid;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_RPC_POLICY */
/* */
typedef struct
{
    AgNdRpcPolicy           x_policy_matrix;
    AG_BOOL                 a_cc_prioritized[AG_ND_RPC_CCNT_MAX];
    AG_BOOL                 a_gc_prioritized[AG_ND_RPC_CGLB_MAX];
    AgNdRpcCntPolicy        a_gc_cnt_policy[AG_ND_RPC_CGLB_MAX];

} AgNdMsgConfigRpcPolicy;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_COMPUTE_RING_SIZE */
/* */
typedef struct
{
    AG_U32                  n_payload_size;
    AG_U32                  n_ring_size;

} AgNdMsgConfigComputeRingSize;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DIAG_HW_TEST */
/*  */
typedef enum
{
    AG_ND_TEST_QUICK_ACCESS     = 0x1,
    AG_ND_TEST_REGISTERS_FAST   = 0x2,
    AG_ND_TEST_REGISTERS_FULL   = 0x3,
    AG_ND_TEST_MEMORY_FAST      = 0x4,
    AG_ND_TEST_MEMORY_FULL      = 0x5,
    AG_ND_TEST_RESET            = 0x6

} AgNdHwTestCase;

typedef struct
{
    AgNdHwTestCase          n_test_case;
    AgResult                (*p_reset_cb)(void);
    AG_BOOL                 b_passed;
    AG_U32                  n_fail_addr;
    AG_U32                  n_fail_bank;
    AG_BOOL                 n_fail_bank_internal;

} AgNdMsgHwTest;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CONTROL_WORD */
/*  */
enum
{
    AG_ND_CES_CW_L              = 0x800,
    AG_ND_CES_CW_R              = 0x400,
    AG_ND_CES_CW_M              = 0x300
};

enum
{
    AG_ND_CES_CW_L_START_BIT    = 0xb,
    AG_ND_CES_CW_R_START_BIT    = 0xa,
    AG_ND_CES_CW_M_START_BIT    = 0x8
};

typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U16                  n_bit_selector;
    AG_U8                   n_l_bit;
    AG_U8                   n_r_bit;
    AG_U8                   n_m_bits;
	AG_BOOL					b_redundancy_config;

} AgNdMsgConfigCw;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_DESTINATION_MAC */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U8                   a_dest_mac[6];
	AG_BOOL					b_is_redundancy;

} AgNdMsgConfigDestMac;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_SEQUENCE_NUMBER */
/* */
typedef struct 
{
	AgNdChannel             n_channel_id;
	AG_U16					n_ces_sqn;
	
} AgNdMsgConfigSeqNumber;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_REGISTER_ACCESS */
/*  */
typedef struct 
{
    AG_U32                  n_addr;
    AG_U16                  n_value;

} AgNdMsgRegAccess;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_PM_GLOBAL */
/*  */
typedef struct
{
    AG_U8                   n_hpl_hp_dropped;
    AG_U8                   n_hpl_lp_dropped;
    AG_U16                  n_hpl_hp_forwarded;
    AG_U16                  n_hpl_lp_forwarded;
    AG_U16                  n_rpc_dropped;
    AG_U16                  a_rpc_global[AG_ND_RPC_CGLB_MAX];

} AgNdMsgPmGlobal;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CHANNEL_PME */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_BOOL                 a_group[AG_ND_PME_GROUP_MAX];

} AgNdMsgConfigChannelPme;

/*ORI*/
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U32                  n_missing_packet_count;
	AG_U32					n_underrun_packet_count;
	AG_U32					n_restart_count;

} AgNdMsgChannelPmeMessingStatus;

typedef enum
{
    AG_ND_DEBUG_BUF_PBF,
    AG_ND_DEBUG_BUF_JBF

} AgNdDebugBufType;

typedef enum
{
    AG_ND_DEBUG_BUF_START,
    AG_ND_DEBUG_BUF_END

} AgNdDebugBufDir;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DIFF_CONFIG */
/*  */
typedef struct 
{
    AgNdChannel             n_circuit_id;

    /*BSG related */
    AG_BOOL                 b_justification_request;
    AG_BOOL                 b_justification_polarity;
    AG_BOOL                 b_justification_type;
    AG_BOOL                 b_BSG_enable;
    AG_BOOL                 b_request_polarity;
    AG_BOOL                 b_request_type;
    AG_BOOL                 b_bsg_count_enable;
} AgNdMsgBsgConfig;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEPTUNE_DCR_CONFIGURATIONS */
/*  */
typedef struct 
{
    /*TSE related */
    AG_BOOL                 b_TSE_enable;
    AG_U32                  n_prime_timestamp_period;

    AG_U32                  n_frame_step_size;
    AG_U32                  n_bit_step_size;
	AG_U32					n_timestamp_rate;
	
	AG_BOOL					b_bit_justification_toggle;

} AgNdMsgConfigNeptuneDcrConfigurations;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_DIFF_POLL */
/*  */
typedef struct 
{
    AgNdChannel             n_circuit_id;
    AG_U32                  n_output_phase_shift;
} AgNdMsgDiffTsoPoll;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CAS_CHANNEL_INGRESS */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AgNdPacketHeader        x_header;
    AG_U16                  n_cas_sqn;

} AgNdMsgConfigCasChannelIngress;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_CAS_CHANNEL_ENABLE */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_BOOL                 b_enable;

} AgNdMsgConfigCasChannelEnable;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_CAS_CHANNEL_TX */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U32                  n_packets;

} AgNdMsgCommandCasChannelTx;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_PM_INGRESS */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U32                  n_pbf_utilization;
    AG_U32                  n_tpe_pwe_out_byte_counter;
    AG_U32                  n_tpe_pwe_out_packet_counter;
    AG_U32                  n_cas_tx;

} AgNdMsgPmIngress;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_PM_EGRESS */
/*  */
typedef struct 
{
    AgNdChannel             n_channel_id;
    AG_U32                  n_jbf_depth_min;
    AG_U32                  n_jbf_depth_max;
    AG_U32                  n_jbf_underruns;
    AG_U32                  n_jbf_missing_packets;
    AG_U32                  n_jbf_dropped_ooo_packets;
    AG_U32                  n_jbf_reordered_ooo_packets;
    AG_U32                  n_jbf_bad_length_packets;
    AG_U32                  n_rpc_pwe_in_bytes;
    AG_U32                  n_rpc_pwe_in_packets;
    AG_U32                  a_rpc_channel_specific[AG_ND_RPC_CCNT_MAX];
    AG_U32                  n_cas_dropped;

} AgNdMsgPmEgress;



typedef struct
{
	AgNdChannel             n_channel_id;
	AgNdProtoVlan           a_vlan[AG_ND_PROTO_VLAN_TAG_MAX];
	AG_U32                  n_vlan_count;
	AG_BOOL					b_ptp;
	
} AgNdVlanHeader;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_STATUS_CAS_DATA */
/*  */
typedef struct
{
    AgNdPath                e_path;
    AgNdCircuit             n_circuit_id;
    AG_U8                   a_abcd[AG_ND_SLOT_MAX];

} AgNdMsgStatusCasData;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_STATUS_CAS_CHANGE */
/*  */
typedef struct
{
    AgNdCircuit             n_circuit_id;
    AG_U32                  n_changed;

} AgNdMsgStatusCasChange;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_TSG */
/*  */
typedef struct
{
    AG_BOOL                 b_fast_phase_enable;
    AG_U32                  n_fast_phase_step_size;
    AG_BOOL                 b_error_correction_enable;
    AG_U32                  n_error_correction_period;
    AG_U32                  n_step_size_integer;
    AG_U32                  n_step_size_fraction;

    AG_BOOL                 n_enable; /* RO */

} AgNdMsgConfigPtpTsg;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_STATELESS */
/*  */
typedef struct
{
    AgNdPtpIdx              n_ptp_idx;
    AG_BOOL                 b_sync_enable;
    AG_BOOL                 b_delay_request_enable;
    AG_BOOL                 b_delay_response_enable;
    AG_BOOL                 b_pdelay_request_enable;
    AG_BOOL                 b_pdelay_response_enable;

} AgNdMsgConfigPtpStateless;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_TSG_ENABLE */
/*  */
typedef struct
{
    AG_BOOL                 b_enable;

} AgNdMsgConfigPtpTsgEnable;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_CLK_SOURCE */
/*  */
typedef struct
{
    AgNdPtpIdx              n_ptp_idx;
    AgPtpClkSrc             e_clk_src;
    AgNdCircuit             n_port;

} AgNdMsgConfigPtpClkSrc;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_BRG */
/*  */
typedef struct
{
    AgNdPtpIdx              n_ptp_idx;
    AG_U32                  n_step_size;

} AgNdMsgConfigPtpBrg;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_CHANNEL_INGRESS */
/*  */
typedef struct
{
    AgNdChannel             n_channel_id;
    AgNdPtpIdx              n_ptp_idx;
    AG_S32                  n_rx_response_rate;
    AG_S32                  n_tx_sync_rate;
	AG_BOOL					b_count_out_delay_req;
    AgNdPacketHeader        x_header;

} AgNdMsgConfigPtpCHannelIngress;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_CHANNEL_EGRESS */
/*  */
typedef struct
{
    AgNdChannel             n_channel_id;
    AgNdPtpIdx              n_ptp_idx;
    AgNdPacketHeader        x_strict_data;	
	AG_BOOL					b_count_in_delay_req;
} AgNdMsgConfigPtpCHannelEgress;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_CHANNEL_ENABLE_INGRESS */
/*  */
typedef struct
{
    AgNdChannel             n_channel_id;
    AG_BOOL                 b_sync_enable;
    AG_BOOL                 b_delay_request_enable;
    AG_BOOL                 b_delay_response_enable;
    AG_BOOL                 b_pdelay_request_enable;
    AG_BOOL                 b_pdelay_response_enable;
	AG_BOOL					b_random_tx_enable;

} AgNdMsgConfigPtpChannelEnableIngress;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_PTP_CHANNEL_ENABLE_EGRESS */
/*  */
typedef struct
{
    AgNdChannel             n_channel_id;
    AG_BOOL                 b_enable;

} AgNdMsgConfigPtpChannelEnableEgress;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_PTP_READ_COUNTERS */
/*  */
typedef struct
{
    AgNdChannel             n_channel_id;
	AG_U8					n_out_delay_req_or_resp;
	AG_U8					n_out_sync_plus_followup;

	AG_U8					n_in_sync;
	AG_U8					n_in_followup;
	AG_U8					n_in_delay_req_or_resp;
	
} AgNdMsgPtpCounters;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_PTP_CORRECT_TIMESTAMP */
/*  */
typedef struct
{
    AgNdPtpIdx              n_ptp_idx;
    AG_U8                   a_correction_value[10];

} AgNdMsgCommandPtpCorrectTs;


/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_COMMAND_PTP_ADJUSTMENT */
/*  */
typedef struct
{
    AgNdPtpIdx              n_ptp_idx;
    AgNdAdjustPolarity      e_polarity;

} AgNdMsgCommandPtpAdjustment;
/*
 * Copyright 2011 BATM
 */

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_DCR_CONFIGURATIONS */
/*  */
typedef struct 
{
    AG_BOOL             b_ts_generation_enable;
    AG_BOOL             b_fast_phase_enable;
    AG_BOOL             b_err_correction_enable;
    AG_U16              n_prime_ts_sample_period;
    AG_U32              n_rtp_tsx_ts_step_size;
    AG_U16              n_rtp_tsx_fast_phase_step_size;
    AG_U16              n_rtp_tsx_error_corrrection_period;

} AgNdMsgConfigNemoDcrConfigurations;



/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_DCR_CLK_SOURCE */
/*  */

typedef struct 
{
    AgNdNemoDcrClkSource    e_clk_source;
    AG_U8                   n_tdm_port;

} AgNdMsgConfigNemoDcrClkSource;

/*///////////////////////////////////////////////////////////////////////////// */
/* AG_ND_OPCODE_CONFIG_NEMO_DCR_SYSTEM_TSI_CLK_SOURCE */
/*  */
typedef struct 
{
	AG_U32 						n_circuit_id;
    AgNdNemoDcrSystemClkSource  e_clk_source;
    AG_U16                  	n_prime_sample_period;

} AgNdMsgConfigNemoDcrSystemClkInputSource;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_NEMO_DCR_LOCAL_TS_AND_FAST_PHASE_TS      RO */
/* */
typedef struct 
{
    AgNdCircuit         n_circuit_id;
    AG_U32              n_timestamp;
    AG_U8               n_fast_phase_timestamp;
} AgNdMsgNemoDcrLocalTs;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_NEMO_DCR_PRIME_LOCAL_TS      RO */
/* */
typedef struct 
{
    AgNdCircuit        n_circuit_id;
    AG_U32             n_prime_timestamp;

} AgNdMsgNemoDcrLocalPrimeTs;

/*///////////////////////////////////////////////////////////////////////////// */
/*  AG_ND_OPCODE_CONFIG_NEMO_DCR_LOCAL_SAMPLE_PERIOD      RW */
/* */
typedef struct 
{
    AgNdCircuit        n_circuit_id;
    AG_U32             n_sample_period;

} AgNdMsgConfigNemoDcrSamplePeriod;

typedef struct 
{
    AgNdCircuit        			n_circuit_id;
    AgNdNemoDcrSystemTsoMode    e_tso_mode;
	AgNdNemoDcrSystemTsoRefClockSelect e_clock_select;

} AgNdMsgConfigNemoDcrSystemTsoMode;

#ifdef __cplusplus
}
#endif

#endif /*  __NEMO_API_H__ */

