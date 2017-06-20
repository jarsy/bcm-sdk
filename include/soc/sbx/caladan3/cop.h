/*
 * $Id: cop.h,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    cop.h
 * Purpose: Caladan3 COP defines
 * Requires:
 */

#ifndef _SBX_CALADN3_COP_H_
#define _SBX_CALADN3_COP_H_

#if !defined(SWIG)
#include <soc/mcm/memregs.h>
#endif

#define SOC_SBX_CALADAN3_COP_NUM_SEGMENT                 (32)
#define SOC_SBX_CALADAN3_COP_NUM_COP                     (2)
#define SOC_SBX_CALADAN3_COP_NUM_PROFILE                 (1024)
#define SOC_SBX_CALADAN3_COP_NULL_PROFILE                (0)
#define SOC_SBX_CALADAN3_COP_POLICER_MAX_SCALEFACTOR     (12)

#define SOC_SBX_CALADAN3_COP_RING_THREAD_DEFAULT_PRI     (60)

/* policer monitor definition */
#define SOC_SBX_CALADAN3_COP_POLICER_MAX_MONITOR         (8)
#define SOC_SBX_CALADAN3_COP_POLICER_MONITOR_COUNTERS    (16)

#define SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_SET(cop, monitor) \
    ((((cop) & 0x1) << 3) | ((monitor) & 0x7))

#define SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_COP(handle) \
    (((handle) >> 3) & 0x1)

#define SOC_SBX_CALADAN3_COP_MONITOR_HANDLE_GET_MONITOR(handle) \
    ((handle) & 0x7)

#define _COP_HANDLE_COP_SHIFT        (31)
#define _COP_HANDLE_COP_MASK         (0x1)
#define _COP_HANDLE_SEGMENT_SHIFT    (26)
#define _COP_HANDLE_SEGMENT_MASK     (0x1F)
#define _COP_HANDLE_ID_SHIFT         (0)
#define _COP_HANDLE_ID_MASK          (0x3ffFFFF)

#define _COP_HANDLE_GET_COP(handle)   \
    (((handle)>>_COP_HANDLE_COP_SHIFT) & _COP_HANDLE_COP_MASK)

#define _COP_HANDLE_GET_SEGMENT(handle)   \
    (((handle)>>_COP_HANDLE_SEGMENT_SHIFT) & _COP_HANDLE_SEGMENT_MASK)

#define _COP_HANDLE_GET_ID(handle)   \
    (((handle)>>_COP_HANDLE_ID_SHIFT) & _COP_HANDLE_ID_MASK)

#define _COP_HANDLE_SET(cop, segment, id) \
    (((cop & _COP_HANDLE_COP_MASK) << _COP_HANDLE_COP_SHIFT) |    \
     ((segment & _COP_HANDLE_SEGMENT_MASK) << _COP_HANDLE_SEGMENT_SHIFT) |    \
     ((id & _COP_HANDLE_ID_MASK) << _COP_HANDLE_ID_SHIFT))


typedef struct soc_sbx_caladan3_cop_policer_monitor_counter_s {
    uint64 green_to_green_pkts;                  /* green to green update packets */
    uint64 green_to_green_bytes;                 /* green to green update bytes */
    uint64 green_to_yellow_pkts;                 /* green to yellow update packets */
    uint64 green_to_yellow_bytes;                /* green to yellow update bytes */  
    uint64 green_to_red_pkts;                    /* green to red update packets */ 
    uint64 green_to_red_bytes;			 /* green to red update bytes */   
    uint64 green_to_drop_pkts;			 /* green to drop update packets */
    uint64 green_to_drop_bytes;			 /* green to drop update bytes */     
    uint64 yellow_to_green_pkts;                 /* yellow to green update packets */ 
    uint64 yellow_to_green_bytes;		 /* yellow to green update bytes */   
    uint64 yellow_to_yellow_pkts;		 /* yellow to yellow update packets */
    uint64 yellow_to_yellow_bytes;		 /* yellow to yellow update bytes */  
    uint64 yellow_to_red_pkts;			 /* yellow to red update packets */   
    uint64 yellow_to_red_bytes;			 /* yellow to red update bytes */     
    uint64 yellow_to_drop_pkts;			 /* yellow to drop update packets */  
    uint64 yellow_to_drop_bytes;		 /* yellow to drop update bytes */    
    uint64 red_to_green_pkts;                    /* red to green update packets */ 
    uint64 red_to_green_bytes;			 /* red to green update bytes */   
    uint64 red_to_yellow_pkts;			 /* red to yellow update packets */
    uint64 red_to_yellow_bytes;			 /* red to yellow update bytes */  
    uint64 red_to_red_pkts;			 /* red to red update packets */   
    uint64 red_to_red_bytes;			 /* red to red update bytes */     
    uint64 red_to_drop_pkts;			 /* red to drop update packets */  
    uint64 red_to_drop_bytes;			 /* red to drop update bytes */    
    uint64 nop_pkts;                             /* nop (cop_dp=3) packets */ 
    uint64 nop_bytes;				 /* nop (cop_dp=3) bytes */   
    uint64 meter_bucket_overflow_error_pkts;	 /* meter bucket overflow packets */
    uint64 meter_bucket_overflow_error_bytes;	 /* meter bucket overflow bytes */  
    uint64 ocm_error_pkts;			 /* ocm error packets */   
    uint64 ocm_error_bytes;			 /* ocm error bytes */     
    uint64 ecc_error_pkts;			 /* ecc error on profile memory or cache packets */  
    uint64 ecc_error_bytes;    			 /* ecc error on profile memory or cache bytes */ 
} soc_sbx_caladan3_cop_policer_monitor_counter_t;

typedef enum soc_sbx_caladan3_cop_segment_type_e_s {
    SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER = 0,
    SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER,
    SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER,
    SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_MAX
} soc_sbx_caladan3_cop_segment_type_e_t;

typedef enum soc_sbx_caladan3_cop_policer_error_color_e_s {
    SOC_SBX_CALADAN3_COP_POLICER_ERROR_COLOR_GREEN = 0,
    SOC_SBX_CALADAN3_COP_POLICER_ERROR_COLOR_YELLOW,
    SOC_SBX_CALADAN3_COP_POLICER_ERROR_COLOR_RED,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_POLICER_ERROR_COLOR_MAX
} soc_sbx_caladan3_cop_policer_error_color_e_t;

typedef enum soc_sbx_caladan3_cop_policer_rfc_mode_e_s {
    SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2697 = 0,
    SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2698,
    SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_4115,
    SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_MEF,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_MAX
} soc_sbx_caladan3_cop_policer_rfc_mode_e_t;

/*== segment configs */
typedef struct soc_sbx_caladan3_cop_policer_segment_config_s {
    int8   bErrorMask;
    int8   nErrorColor;
    uint32 uMaxRateKbps;
    uint32 uMaxBurstBits;
} soc_sbx_caladan3_cop_policer_segment_config_t;

typedef struct soc_sbx_caladan3_cop_timer_segment_config_s {
    int8  bMode64;
    int32 nTimerTickUs;
} soc_sbx_caladan3_cop_timer_segment_config_t;

typedef struct soc_sbx_caladan3_cop_sn_checker_segment_config_s {
    int8   bMode32;
    uint32 uSequenceRange;
} soc_sbx_caladan3_cop_sn_checker_segment_config_t;

typedef enum soc_sbx_caladan3_cop_coherent_overflow_mode_e_s {
    SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_ROLLOVER = 0,
    SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_SATURATE,
    SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_STICKY,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_MAX
} soc_sbx_caladan3_cop_coherent_overflow_mode_e_t;

typedef enum soc_sbx_caladan3_cop_coherent_format_e_s {
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_1BIT = 0,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_4BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_8BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_16BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_32BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_64BIT,
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_2x32BIT,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_MAX
} soc_sbx_caladan3_cop_coherent_format_e_t;

typedef enum soc_sbx_caladan3_cop_coherent_command_e_s {
    SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_READ = 0,
    SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_READCLEAR,
    SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_WRITE,
    SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_COUNT,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_COHERENT_COMMAND_MAX
} soc_sbx_caladan3_cop_coherent_command_e_t;

typedef struct soc_sbx_caladan3_cop_coherent_segment_config_s {
    int8 bReturnNext;
    int8 nOverflowMode;    
    int8 nFormat;
} soc_sbx_caladan3_cop_coherent_segment_config_t;

typedef union soc_sbx_caladan3_cop_segment_type_specific_config_s {
    soc_sbx_caladan3_cop_policer_segment_config_t    sPolicer;
    soc_sbx_caladan3_cop_timer_segment_config_t      sTimer;
    soc_sbx_caladan3_cop_sn_checker_segment_config_t sChecker;
    soc_sbx_caladan3_cop_coherent_segment_config_t   sCoherent;
} soc_sbx_caladan3_cop_segment_type_specific_config_t;

typedef struct soc_sbx_caladan3_cop_segment_config_s {
    int8     bEnabled;
    int8     nSegmentType;
    int32    nSegmentLimit;
    int32    nSegmentOcmBase;    /* allocated by COP driver */
    int32    nSegmentOcmSize;    /* calculated by COP driver */
    soc_sbx_caladan3_cop_segment_type_specific_config_t u;
} soc_sbx_caladan3_cop_segment_config_t;

typedef struct soc_sbx_caladan3_cop_profile_s {
    uint32                   uRefCount;
    co_meter_profile_entry_t sHwProfile;
    uint32                   uRfcMode;
    uint32                   uCIR;
    uint32                   uEIR;
    uint32                   uCBS;
    uint32                   uEBS;
}soc_sbx_caladan3_cop_profile_t;

typedef struct soc_sbx_caladan3_cop_ring_config_s {
    int32         nRingEntries;
    int32         nRingThresh;
    int32        *pRingPciBase[SOC_SBX_CALADAN3_COP_NUM_COP];
    int32        *pRingPciRead[SOC_SBX_CALADAN3_COP_NUM_COP];
    /* ring processing thread control */
    int           thread_pri;
    char          name[SOC_SBX_CALADAN3_COP_NUM_COP][16];
    sal_thread_t  pid[SOC_SBX_CALADAN3_COP_NUM_COP];
    sal_sem_t     trigger[SOC_SBX_CALADAN3_COP_NUM_COP];
    VOL int       running[SOC_SBX_CALADAN3_COP_NUM_COP];
} soc_sbx_caladan3_cop_ring_config_t;

typedef struct soc_sbx_caladan3_cop_timer_expire_event_s {
    uint8  bForced;
    uint8  bActiveWhenForced;
    uint8  uCop;
    uint8  uSegment;
    uint32 uTimer;
} soc_sbx_caladan3_cop_timer_expire_event_t;

typedef struct soc_sbx_caladan3_cop_timer_queue_s {
    int    nMaxDepth;    /* max queue depth */
    int    nReadTail;    /* last queue tail */
    int    nReadHead;    /* last queue tail */
    int    nTail;        /* queue tail, also serve as queue count */
    int    nQId;         /* active queue id */
    soc_sbx_caladan3_cop_timer_expire_event_t *sQueue[2]; /* ping-pong buffer head */
} soc_sbx_caladan3_cop_timer_queue_t;

/* timer event callback function */
typedef void (*soc_sbx_caladan3_cop_timer_event_callback_f)(int unit, int cop);

typedef struct soc_sbx_caladan3_cop_config_s {
    soc_sbx_caladan3_cop_segment_config_t *segments[SOC_SBX_CALADAN3_COP_NUM_COP];
    soc_sbx_caladan3_cop_profile_t *profiles[SOC_SBX_CALADAN3_COP_NUM_COP];    
    sal_mutex_t nRingLock[SOC_SBX_CALADAN3_COP_NUM_COP];
    sal_mutex_t nInjectLock[SOC_SBX_CALADAN3_COP_NUM_COP];
    soc_sbx_caladan3_cop_timer_queue_t sTimerQueue[SOC_SBX_CALADAN3_COP_NUM_COP];
    soc_sbx_caladan3_cop_ring_config_t sRing;
    VOL soc_sbx_caladan3_cop_timer_event_callback_f fNotifyCb;
    uint8 bDriverInit;
    co_meter_monitor_counter_entry_t *uMonitorDmaBuffer;
    /* size of OCM memory in bytes
     * Only thing requires config by user, 
     */
    uint32      uCOPOcmSize[SOC_SBX_CALADAN3_COP_NUM_COP];
} soc_sbx_caladan3_cop_config_t;

typedef enum soc_sbx_caladan3_cop_command_operation_e_s {
    SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT = 0,
    SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_INJECT_STICKY,
    /* leave as last */
    SOC_SBX_CALADAN3_COP_COMMAND_OPERATION_MAX
} soc_sbx_caladan3_cop_command_operation_e_t;

extern int
soc_sbx_caladan3_cop_hw_init(int unit);

extern int
soc_sbx_caladan3_cop_driver_init(int unit);

extern int
soc_sbx_caladan3_cop_driver_uninit(int unit);

extern int
soc_sbx_caladan3_cop_ocm_memory_size_set(int unit,
					 int32 cop,
					 uint32 size);
extern int
soc_sbx_caladan3_cop_timer_event_queue_size_set(int unit,
						int cop,
						int size);

extern int
soc_sbx_caladan3_cop_timer_event_callback_register(int unit,
						   soc_sbx_caladan3_cop_timer_event_callback_f cb);

extern int
soc_sbx_caladan3_cop_segment_register(int unit,
				      int cop,
				      int segment,
				      int num_entry,
				      soc_sbx_caladan3_cop_segment_type_e_t type,
				      soc_sbx_caladan3_cop_segment_type_specific_config_t *config);

extern int
soc_sbx_caladan3_cop_segment_unregister(int unit,
					int cop,
					int segment);

extern int
soc_sbx_caladan3_cop_segment_read(int unit,
				  int cop,
				  int segment,
				  int *num_entry,
				  soc_sbx_caladan3_cop_segment_type_e_t *type,
				  soc_sbx_caladan3_cop_segment_type_specific_config_t *config);

/************************** Policer ENTITIES ******************************/
typedef struct soc_sbx_caladan3_cop_policer_config_s {
    uint32 uRfcMode;
    uint32 uCIR;         /* in kbps */
    uint32 uCBS;         /* in bytes */
    uint32 uEIR;         /* in kbps */
    uint32 uEBS;         /* in bytes */
    uint32 bBlindMode;
    uint32 bDropOnRed;
    uint32 bCoupling;
    uint32 bCBSNoDecrement;
    uint32 bEBSNoDecrement;
    uint32 bCIRStrict;
    uint32 bEIRStrict;
    int8   nLenAdjust;   /* in bytes */
    uint32 bPktMode;
} soc_sbx_caladan3_cop_policer_config_t;

extern int 
soc_sbx_caladan3_cop_policer_create(int unit,
				    uint32 cop,
				    uint32 segment,
				    uint32 policer,
				    soc_sbx_caladan3_cop_policer_config_t *config,
				    uint32 *handle);

extern int 
soc_sbx_caladan3_cop_policer_delete(int unit,
				    uint32 handle);

extern int
soc_sbx_caladan3_cop_policer_read(int unit,
				  uint32 handle,
				  soc_sbx_caladan3_cop_policer_config_t *config);

extern int
soc_sbx_caladan3_cop_policer_read_ext(int unit,
				      uint32 handle,
				      soc_sbx_caladan3_cop_policer_config_t *config,
				      uint32 *profile);

extern int
soc_sbx_caladan3_cop_policer_pkt_mode_len_get(int unit, int cop, uint32* len);

extern int
soc_sbx_caladan3_cop_policer_pkt_mode_len_set(int unit, int cop, uint32 len);

extern int
soc_sbx_caladan3_cop_policer_token_number_get(int unit,
				      uint32 handle,
				      uint32 *token_c,
				      uint32 *token_e);

/************************** Timer ENTITIES ******************************/
typedef struct soc_sbx_caladan3_cop_timer_config_s {
    uint32 uTimeout;
    uint32 bInterrupt;
    uint32 bStart;
} soc_sbx_caladan3_cop_timer_config_t;

extern int
soc_sbx_caladan3_cop_timer_create(int unit,
				  uint32 cop,
				  uint32 segment,
				  uint32 timer,
				  soc_sbx_caladan3_cop_timer_config_t *config,
				  uint32 *handle);

extern int
soc_sbx_caladan3_cop_timer_delete(int unit,
				  uint32 handle);

extern int
soc_sbx_caladan3_cop_timer_read(int unit,
				uint32 handle,
				soc_sbx_caladan3_cop_timer_config_t *config);

extern int
soc_sbx_caladan3_cop_timer_event_dequeue(int unit,
					 int cop,
					 soc_sbx_caladan3_cop_timer_expire_event_t *event);

int
soc_sbx_caladan3_cop_ring_process_thread_start(int unit, int cop);

int
soc_sbx_caladan3_cop_ring_process_thread_stop(int unit, int cop);


/************************** Sequece number checker ENTITIES ******************************/
extern int
soc_sbx_caladan3_cop_seq_checker_create(int unit,
					uint32 cop,
					uint32 segment,
					uint32 seq_checker,
					uint32 init_value,
					uint32 *handle);

extern int
soc_sbx_caladan3_cop_seq_checker_delete(int unit,
					uint32 handle);


/************************** Coherent table ENTITIES ******************************/
extern int
soc_sbx_caladan3_cop_coherent_table_create(int unit,
					   uint32 cop,
					   uint32 segment,
					   uint32 coherent_table,
					   uint32 init_bits31_0,
					   uint32 init_bits63_32,
					   uint32 *handle);

extern int
soc_sbx_caladan3_cop_coherent_table_get(int unit,
                       uint32 handle,
                       uint32 entry,
                       uint32 *bits31_0,
                       uint32 *bits63_32);

extern int
soc_sbx_caladan3_cop_coherent_table_set(int unit,
                           uint32 handle,
                           uint32 entry,
                           uint32 bits31_0,
                           uint32 bits63_32);

extern int
soc_sbx_caladan3_cop_coherent_table_delete(int unit,
					   uint32 handle);



/************************** Misc utilities ******************************/
extern int
soc_sbx_caladan3_cop_recover_policer(int unit,
				     uint32 policer,
				     uint32 profile);

extern void
soc_sbx_caladan3_cop_sw_dump(int unit);

extern int
soc_sbx_caladan3_cop_diag_policer_monitor_setup(int unit, int cop, int segment,
						int policer, int *monitor);
extern int
soc_sbx_caladan3_cop_diag_policer_monitor_free(int unit, int monitor);

extern int
soc_sbx_caladan3_cop_diag_policer_monitor_read(int unit, int monitor, int clear_on_read,
					       soc_sbx_caladan3_cop_policer_monitor_counter_t *counter);

/************************** Not exposed or now ******************************/
int
soc_sbx_caladan3_cop_encode_rate(int unit,
				 soc_mem_t mem,
				 soc_field_t field,
				 uint32 raw,
				 uint32 *exp,
				 uint32 *mant);

#endif /* _SBX_CALADN3_COP_H_ */
