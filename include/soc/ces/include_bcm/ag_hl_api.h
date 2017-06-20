/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 
     hl_api - AG High level APIs for RX100

File Name: 
     ag_hl_api.h

File Description: 

$Revision: 1.1.2.1 $ - Visual SourceSafe automatic revision number 

History:
    Yaeli Karni 1/24/2002   Initial Creation


******************************************/
#ifndef AG_HL_API_H 
#define AG_HL_API_H 


/********************************
 *  Main includes  - all needed includes for this .h file usage.
 ********************************/

#ifdef BCM_CES_SDK

#else

#include "ag_common.h"
#include "drivers/uart_ext.h"
#include "drivers/gpio.h"
#include "drivers/bmd_types.h"
#include "drivers/gpc_drv.h"
#include "rules/rules_ex_api.h"
#include "drivers/wan_config.h"
#include "management/ag_management.h"
#include "flow/flow_types.h"

#include "classification/cls_types.h"
#include "drivers/uart_api.h"
#include "reset/reset_ext.h"

#if defined RULES_API_CPP || defined AG_HL_API_INIT_CFG_C || defined AG_HL_API_SERVICES_C
#define AG_EXTERN 
#else 
#define AG_EXTERN extern 
#endif


#ifdef __cplusplus
extern "C"
{
#endif 

 /********************************
 *  General Define 
 ********************************/ 

#define AG_HL_TASK_STACK_SIZE       4096 /* 4096 stack size for tasks setup by HL APIs */

#define AG_HL_SNMP_TASK_STACK_SIZE	16384

/***************** TASKS PRIORITIES & PREEMPTION ***********************/
/* user can add his/her tasks with priorities between those set up here */
#define AG_HL_DISP_TASK_PRI     20   /* relatively high priority - brings frames from HW */
#define AG_HL_ATI_NET_TASK_PRI  100   /* The ATI NET tasks - set in the net libs. Just for
										 reference here */
#define AG_HL_HW_TO_NET_TASK_PRI  100  /* reletively medium priority - starts up the ATI NET task,
                                        and waits for frames (bundles) from the HW to put into
                                        the TCP/IP stack */
#define AG_HL_PPPSRV_TASK_PRI   100  /* reletively medium priority - serves as PPP server task
                                        for if (currently only UARTs) to set up PPP  and handle it*/
#define AG_HL_ATI_SNMP_TASK_PRI	105
#define AG_HL_TELNET_TASK_PRI   105
#define AG_HL_METER_TASK_PRI    120  /* reletively medium priority - handles Statistics update 
                                        and classification error condition frames */
#define AG_HL_GPIO_TASK_PRI     150  /* relatively medium low priority - handles GPIOs  */
#define AG_HL_TRACE_TASK_PRI    240  /* relatively low priority - output traces to COM */

#define AG_HL_RPLS_VRF_TASK_PRI 245  /* relatively low priority - for Replace verifying */

#define AG_HL_TASK_TIME_SLICE   1   /* time slice for each task setup by HL APIs */

/**************** CPU-TARGET QUEUES (HW queues destined to CPU *****************/
/* total of HW -queues destined to CPU is 8 */
#define AG_HL_API_FIRST_CPU_QUEUE   247 /* first CPU target queue */
#define AG_HL_API_LAST_CPU_QUEUE	(AG_HL_API_FIRST_CPU_QUEUE+7) /* last CPU target queue 254 
            (we reserve 255 for - special tags etc. so frames will not enter regular flow */

/* define specific offset from first CPU queue for queues with specific functionality */
#define AG_HL_API_FRAG_CPU_QUEUE_INDEX	5 /* need-fragmentation frames queue index */
#define AG_HL_API_TX_FW_CPU_QUEUE_INDEX	6 /* send frame to CPU after the transmit - queue index */
#define AG_HL_API_ERROR_CPU_QUEUE_INDEX	7 /* Errored frames queue index */
#define AG_HL_API_FRAG_CPU_QUEUE    (AG_HL_API_FIRST_CPU_QUEUE+AG_HL_API_FRAG_CPU_QUEUE_INDEX) /* need-fragmentation frames queue */
#define AG_HL_API_TX_FW_CPU_QUEUE   (AG_HL_API_FIRST_CPU_QUEUE+AG_HL_API_TX_FW_CPU_QUEUE_INDEX) /* send frame to CPU after the transmit - queue */
#define AG_HL_API_ERROR_CPU_QUEUE   (AG_HL_API_FIRST_CPU_QUEUE+AG_HL_API_ERROR_CPU_QUEUE_INDEX) /* Errored frames queue */

/********************************
 *  General Enums 
 ********************************/

/* Frame check options  */
enum AgFrmHdrChk_E
{
    AG_FRM_CHECK_TTL,
    AG_FRM_CHECK_IP_HDR,
    AG_FRM_CHECK_ALL,
    AG_FRM_CHECK_DISABLE,
    AG_FRM_CHECK_MAX=AG_FRM_CHECK_DISABLE   /* max number of options */
};
typedef enum AgFrmHdrChk_E AgFrmHdrChk;

/* GPIO trigger to activate function */
typedef enum AgGpioTrigger_E
{
	AG_GPIO_NO_TRIGGER,
	AG_GPIO_LOW_TO_HIGH,
	AG_GPIO_HIGH_TO_LOW,
	AG_GPIO_ON_CHANGE
} AgGpioTrigger;


/* HDLC encapsulation enums */
typedef enum AgHdlcEncaps_E
{
    AG_HDLC_NO_ETH_FCS,
    AG_HDLC_WITH_ETH_FCS
} AgHdlcEncaps;

typedef enum AgIntfClockMode_E
{
    AG_INTF_DCE, /*The Rx100 communication interface outputs the clock.*/
    AG_INTF_DTE  /*The Rx100 communication interface gets the clock as input.*/
} AgIntfClockMode;

typedef enum AgHdlcClockPol_E
{
    AG_HDLC_CLOCK_NORMAL = 0,
    AG_HDLC_CLOCK_INVERTED = 1
} AgHdlcClockPol;


typedef enum AgEthDupMode_E
{   
    AG_ETH_HALF_DUPLEX,
    AG_ETH_FULL_DUPLEX
} AgEthDupMode;

/********************************
 *  General Structures 
 ********************************/

/* Agos Memeory requirements */
struct AgosMemReq_S
{
    /* AGOS managed memory */
    AG_U32 n_heap_size; 	/*in bytes*/
    /* internal numbers deduced from the rules (classification) needs according to
       trial rules+conditions check program...
       The numbers indicates at the end, memeory requirement for internal rules needs.
    */
	AgClsMemUsage x_rule_mem;
};
typedef struct AgosMemReq_S AgosMemReq;

/* System wide memory requirements */
struct AgSysMemReq_S
{
    AG_U32 n_hdrp_size;			/*total memory size for header insertion programs */
    AgBmdBufSize e_size_of_buf; /* the size of each buffers 
                                         (used for receiving frames)*/
    AG_U32 n_num_of_buffers;    /* the total number of buffers*/
    AG_U32 n_ct_ent;			/* CT entries for rules */
};
typedef struct AgSysMemReq_S AgSysMemReq;

/* Frame configuration */
typedef struct AgFrmCfg_S
{
	/* maximal frame size (in bytes) that is allowed to enter the 
       associated interface. Longer frames are invalid and are counted 
       as "too long frames".*/
	AG_U32 n_max_rx_size; 

	/* frames that are longer than the specified size specified (in 
       bytes) are passed to the CPU for fragmentation.
       If this parameter is smaller then n_max_rx_size, then need to 
       register for receiving fragments.*/
	AG_U32 n_need_frag_size;

	AgFrmHdrChk e_check_mode;
} AgFrmCfg;


/* GPIO/LED registration structure */
typedef struct AgGpioCfg_S
{
	AgGpioType e_type; /*LED or GPIO*/
	AG_U32 n_gpio_num; /*0-13 for GPIO, 0-7 for LED*/
	AgGpioLedDir e_dir; /* input/output */
    AgGpioTrigger e_trigger; /* configures when 
    
    the hook 
                    function is triggered*/
	AG_U32 n_debounce_time;
	void* p_param; /* parameter for the function call*/
} AgGpioCfg;

/* phy callback structure  */
typedef struct AgInitPhyParams_S
{
   AgEthDupMode e_dup_mode;
} AgInitPhyParams;

typedef struct AgInitHdlcBgCfg_S
{
    /*internal clock is 100MHz or 125Mhz (depends on Chip frequency)*/
    AgBGClockSource e_clock_source;

    /*The divider value for the clock specified by e_clock_source.*/
    AG_U16 n_clock_divider; 
} AgInitHdlcBgCfg;

typedef struct AgInitBitStreamBgCfg_S
{
    /*internal clock is 100MHz or 125Mhz (depends on Chip frequency)*/
    AgBGClockSource e_clock_source;

    /*The divider value for the clock specified by e_clock_source.*/
    AG_U16 n_clock_divider; 
} AgInitBitStreamBgCfg;


/* Init the HDLC structure  */
typedef struct AgInitHdlcCfg_S
{
	AG_U32 n_num_of_flags;	  /* Number of flags between frames */
	AgCrcLength e_crc_length; 
	AgCtsMode e_cts_mode;	  

 	AgIntfClockMode e_clock;   
	AgHdlcClockPol  e_clock_tx_pol; /*normal, inverted*/
	AgHdlcClockPol  e_clock_rx_pol; /*normal, inverted*/

    /* The baud generator parameters are relevant only for DCE mode.
       For DTE mode they are ignored*/
	AgInitHdlcBgCfg x_bg_config; 

} AgInitHdlcCfg;


/* Init the Bit Stream structure  */
typedef struct AgInitBitStreamCfg_S
{
	AgBitstreamConfig	 x_bs_cfg;   /* Bit Stream specific parameters*/
 	AgIntfClockMode      e_clock;   
	AG_BOOL				 b_use_BG_in_DTE; /* force use of Baud rate generator also */
										/* in DTE mode */
    AgWanClockConfig	 x_wan_clk_cfg;
    AgInitBitStreamBgCfg x_bg_config;/* There are 2 BG in the chip. The Bit Stream uses teh accurate one.
								        For DTE mode they are ignored*/
	AG_U32				 speed;		/* in Kbits per second */
	AgComIntf			 e_intf_bg; /* indicator which BG to use (of IF-A, or of IF-B) */
	AgDataOrder			 e_data_order;
} AgInitBitStreamCfg;

typedef struct AgInitEthCfg_S
{
    AgIntfClockMode e_clock_mode; /*DCE or DTE*/
    AG_U8 a_mac_addr[6]; 
    void* p_phy_param; /* the parameter for ag_phy_config */
} AgInitEthCfg;


/*  UART configuration */
typedef struct AgUsartConfig_S 
{ 
    AgUartBaudRate e_baud_rate;
    AG_U32 n_stop_bits; /*0 or 1*/
    AgUartMode n_uart_mode;
} AgUsartConfig;

/* Logger configuration (when started by SW) */
typedef struct AgHlLoggerConfig_S
{
    AgLogMedia e_log_media; /* log to where */
    AG_U32 n_num_of_ids;    /* number of ids we want the logger to log */
    AG_U32* a_enabled_group_ids;    /* what ids we want the logger to log */
    AG_U32 n_dest_ip_add;   /* destination IP address (applicable only when log media is AG_LOG_IP_PORT) */
    AG_U32 n_dest_port;     /* destination PORT (applicable only when log media is AG_LOG_IP_PORT) */
    AgLogStartMode e_start_mode; /* the start mode - do we plan to use it? */
} AgHlLoggerConfig;

/* DRR QoS configuration */
typedef struct AgQosDrrQCfg_S
{
	AG_U32 n_queue_number;
	AG_U32 n_max_queue_length;
	AG_U32 n_min_bandwidth;
	AG_U32 n_max_allowed_delay;
	AG_S32 n_frame_delta;
	AG_U32 n_max_bandwidth;
} AgQosDrrQCfg;

/* QoS WFQ - next releases */
typedef struct AgQosWfqQCfg_S
{
	AG_U32 n_queue_number;
	AG_U32 n_max_queue_length;
	AG_U32 n_min_bandwidth;
	AG_U32 n_max_allowed_delay;
	AG_S32 n_frame_delta;
} AgQosWfqQCfg;

/* QoS strict priority - next releases */
typedef struct AgQosStpQCfg_S
{
	AG_U32 n_queue_number;
	AG_U32 n_max_queue_length;
	AG_U32 n_priority;
	AG_U32 n_max_allowed_delay;
	AG_S32 n_frame_delta;
	AG_U32 n_max_bandwidth;
} AgQosStpQCfg;


/********************************
 *  Functions declaration 
 ********************************/

/**************** Configuration Init APIs ************************/
AG_EXTERN AgResult ag_init_start(void* p_mem_start, 
    AG_U32 n_mem_size, AgosMemReq* p_required_mem, 
    AG_U32* actual_used_memory_size);

AG_EXTERN AgResult ag_init_set_base_mode(AgBaseMode e_base_mode);

AG_EXTERN AgResult ag_get_base_mode(AgBaseMode *p_base_mode);

AG_EXTERN AgResult ag_init_mem_layout(void* p_frame_buffer_base, void* p_mem_start, 
    AG_U32 n_mem_size, AgSysMemReq * p_required_mem, 
    AG_U32* actual_used_memory_size);

AG_EXTERN AgResult ag_init_modules(void);

AG_EXTERN AgResult ag_init_frame_handling(
    AgFrmCfg *p_ifc_cfg, AgFrmCfg *p_ifa_cfg,
    AgFrmCfg *p_ifb_cfg);

AG_EXTERN AgResult ag_app_init_end(AG_BOOL b_enable_ifc,
                         AG_BOOL b_enable_ifa,
                         AG_BOOL b_enable_ifb);


/* Phy call back function */
AG_EXTERN AgResult ag_phy_config(AgComIntf e_intf, void* p_phy_param, 
    AgInitPhyParams* p_cfg_result);


AG_EXTERN AgResult ag_init_hdlc_config(AgComIntf e_intf, AgInitHdlcCfg* p_cfg);


AG_EXTERN AgResult ag_init_set_eth_over_hdlc_encapsulation(AgHdlcEncaps e_encaps_mode);

AG_EXTERN AgResult ag_init_get_eth_over_hdlc_encapsulation(AG_U32 *e_encaps_mode);

AG_EXTERN AgResult ag_init_bitstream_config(AgComIntf e_intf, AgInitBitStreamCfg* p_cfg);

AG_EXTERN AgResult ag_init_uart_config(AgUart e_uart, AgUsartConfig* p_cfg);

AG_EXTERN AgResult ag_init_qos_drr_intf(AgComIntf e_tx_intf, 
    AG_U32 n_total_intf_bw, AG_BOOL b_bw_per_q, AG_U32 n_queues, 
    AgQosDrrQCfg *a_q_cfg);

AG_EXTERN AgResult ag_init_qos_sp_intf(AgComIntf e_tx_intf, 
    AG_U32 n_total_intf_bw, AG_BOOL b_bw_per_q, AG_U32 n_queues,
    AgQosStpQCfg *a_q_cfg);



/* Future release functions */
/*

AgResult ag_init_qos_wfq_intf(AgComIntf e_tx_intf, 
    AG_U32 n_total_intf_bw, AG_U32 n_queues,
    AgQosWfqQCfg *a_q_cfg);

AgResult ag_init_qos_strict_pri_intf(AgComIntf e_tx_intf, 
    AG_U32 n_total_intf_bw, AG_U32 n_queues,
    AgQosStpQCfg *a_q_cfg);

AgReuslt ag_init_red_config (TBD);
AgResult ag_init_register_for_fragmentation(TBD);
AgResult ag_init_super_gpio(TBD);

*/

/************************ Serivces setup APIs ****************************/
AG_EXTERN AgResult ag_init_metering_service(void* p_mem_start , 
    AG_U32 n_mem_size, 
    AgMtrCntCfg* p_cnt,  /* not implemented currently */
    AG_U32* actual_used_memory_size,
    AG_BOOL b_enable_hw_statistics);

AG_EXTERN AgResult ag_init_dispatcher_service(
	AG_S16 *a_q_numbers,
    AG_U32 *a_max_queue_length);

AG_EXTERN AgResult ag_init_gpio_service(AgGpioCfg *a_gpio_cfg, 
    AG_U32 n_num_of_gpios, AgLedActivationMode e_leds_act_mode);

/* for future release, we implement partially for our development */
AG_EXTERN AgResult ag_init_management_service(AgManagAppl e_appl_requested, 
                                    AG_U32 num_of_256_bufs);

/* call back GPIO function that should be implemented so the gpio service will work */
AG_EXTERN void ag_handle_gpio(AgGpioType e_type, /*LED or GPIO*/
    AG_U32 n_number, 	/* gpio number 0-13*/
	AG_U32 n_current_gpio_val, 	/*0 or 1*/
	void* p_param);

/* Future release functions */
/*
AgResult ag_init_error_handling_service(TBD);

*/



/************** FLOW APIs *******************/
AG_EXTERN AgResult ag_tx_frame(AgBmdBundPtr p_frame, AG_U8 n_queue);


/**************** Configuration Dynamic APIs ************************/
AG_EXTERN AgResult ag_qos_set_intf_bw (AgComIntf n_if_num, AG_U32 n_if_rate_limit);
AG_EXTERN AgResult ag_qos_drr_update_queue_weights(AgComIntf e_tx_intf, AG_U32 n_queues, AgQosDrrQCfg *a_q_cfg);
AG_EXTERN AgResult ag_qos_disable_bw_per_q(AgComIntf e_tx_intf);
AG_EXTERN AgResult ag_qos_set_bw_per_q(AgComIntf e_tx_intf, void* p_queue_list);

/************** Semi internal functions - to be used for information only ******/
AgResult ag_hl_get_end_of_heap(void **p_first_addr_after_heap);

/************** For advanced users - to alterate HL APIs defaults -  use with care ******/
/* set first buffer offset. Default is 32 bytes for BMH and PIH. This can be extended. 
   be carefull not to exceed the buffer size..
*/
AgResult ag_set_hl_first_buf_offset(AG_U32 n_first_buf_offset);

/*===============Offline rules Processing, PC simulation functions ================*/

AgResult ag_sim_init(AG_U32 n_dram_size);

AgResult ag_sim_init_sch_queues(AgComIntf e_tx_if, AG_U8 n_first_q, AG_U32 n_queues);

void ag_sim_set_silicon_id(AgSiliconId e_id);


#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#undef AG_EXTERN
 
#endif /*BCM_CES_SDK*/

#endif  /* AG_HL_API_H */
