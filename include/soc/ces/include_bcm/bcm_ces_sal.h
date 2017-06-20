/*
 * $Id: bcm_ces_sal.h,v 1.12 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/*bcm_ces_sal.h*/
/*Bcm sal*/
#ifndef BCM_CES_SAL_H 
#define BCM_CES_SAL_H 

#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <sal/core/dpc.h>
#include <sal/core/libc.h>
#include <sal/appl/io.h>
#include "ag_basic_types.h"

extern int      bsl_printf(const char *format, ...);
#define printf bsl_printf
#define sprintf sal_sprintf

#define ONE_SECOND_IN_MILLISECOND 1
#define AGOS_SWERR(swerr_severity,message,parm1,parm2,parm3,parm4,parm5)
#define GEN_ERR_LOG1(str,arg1)    
#define AG_LOG_MESSAGE(app_id, level, format, param1, param2, param3, param4, param5, param6)


#define         NU_OR                           0
#define         NU_OR_CONSUME                   1
#define         NU_AND                          2
#define         NU_AND_CONSUME                  3
#define         NU_PREEMPT                      10
#define         NU_START                        12
#define AG_HL_TASK_TIME_SLICE   1   /* time slice for each task setup by HL APIs */
#define AGOS_TSK_PREEMPT        NU_PREEMPT
#define AGOS_TSK_START          NU_START

#define AGOS_WAIT_FOR_EVER 0xffffffff


/*Temporary definitions*/
#define AGOS_SWERR_WARNING 1
#define AGOS_SWERR_SERIOUS -1

#define AGOS_TRACE_LEVEL1 1
/******************************************
Macro Name: AGOS_TRACE
Description:
	check trace_level's validation and if the trace is open call to 
	agos_internal_prepare_sending_trace_to_queue().
Parameters:
	IN AgosTraceLevels e_trace_level: trace level
	IN AG_U8 *message:shoth text that describe the trace,do not exceed MAX_TRACE_MESSAGE_SIZE
	IN AG_U32 par1 - par5:5 variables that helps to describe the trace
Returns AgResult :
History:
    Shay Had      ar 8/3/2001   Initial Creation
******************************************/
#if 0
#define AGOS_TRACE(trace_level,message,parm1,parm2,parm3,parm4,parm5) \
		AG_LOG_MESSAGE(SW_LOG_GENERAL_APP_ID, aTrace2SwLoggerLevels[trace_level], \
							"%s %d %d %d %d %d", message, parm1,parm2,parm3,parm4,parm5);

#else
#define AGOS_TRACE(trace_level,message,parm1,parm2,parm3,parm4,parm5) 
#endif




/*For debug*/
#define AG_ND_PRINTF_MAX       1024

/* */
/* timers */
/* */
#define  AgosTimerInfo int
typedef struct
{
    AgosTimerInfo x_timer;

} AgNdTimer;

AgResult    ag_nd_timer_set(
    AgNdTimer   *p_timer, 
    AG_U8       *p_name, 
    void        (*p_callback_function)(AG_U32), 
    AG_U32      n_callpack_parameter,
    AG_U32      n_timer_value,
    AG_BOOL     b_periodic);

AgResult    ag_nd_timer_cancel(AgNdTimer *p_timer);

/* */
/* events */
/* */
enum AgosEventWaitOperation_E
{
#if (!defined NO_OS_SUPPORT && !defined AG_PC_PROG)
    /*#ifdef RTOS_NUCLEUS*/
    AGOS_EVENTS_OR = NU_OR,	                 /* Any of the requested flag is ok for
                                                return. */
    AGOS_EVENTS_OR_CLEARED = NU_OR_CONSUME,	 /* Any of the requested flags are ok for
                                                return. Clear them before returning. */
    AGOS_EVENTS_AND = NU_AND,	             /* All of the requested flags should be set
                                                before returning. */
    AGOS_EVENTS_AND_CLEARED = NU_AND_CONSUME/* All of the requested flags should be
                                                set before returning.
                                                Clear them before returning. */
#else
    /* just that the code will compile */
    AGOS_EVENTS_OR = 1,	      
    AGOS_EVENTS_OR_CLEARED = 2,
    AGOS_EVENTS_AND = 3,
    AGOS_EVENTS_AND_CLEARED = 4
#endif
    /*#endif*/
};
typedef enum AgosEventWaitOperation_E AgosEventWaitOperation;



/* $Id: bcm_ces_sal.h,v 1.12 Broadcom SDK $
 * Events
 */
typedef struct {
    char name[32];
    AG_U32 events;
} AgosEventInfo;

#define AGOS_EVENTS_AND_CLEARED 1
#define AGOS_EVENTS_OR_CLEARED  2
 
typedef struct
{
    AgosEventInfo x_event_group;

} AgNdEvent;

typedef enum
{
    AG_ND_EVENT_AND_CLEAR   = AGOS_EVENTS_AND_CLEARED,
    AG_ND_EVENT_OR_CLEAR   = AGOS_EVENTS_OR_CLEARED
} AgNdEventOp;

AgResult    ag_nd_event_create(AgNdEvent *p_event, AG_U8 *p_name);
AgResult    ag_nd_event_set(AgNdEvent *p_event, AG_U32 n_flags);
AgResult    ag_nd_event_wait(
    AgNdEvent   *p_event, 
    AG_U32      n_requested_flags, 
    AgNdEventOp e_operation,
    AG_U32      *n_retrieved_flags, 
    AG_U32      n_timeout);
AgResult    ag_nd_event_delete(AgNdEvent *p_event);

/* */
/* tasks */
/* */
typedef struct {
  sal_thread_t thread;
} AgosTaskInfo;

typedef struct
{
    AgosTaskInfo x_task_info;

} AgNdTask;

AgResult    ag_nd_task_create(
    AgNdTask    *p_task,
    AG_U8       *p_name,
    void        (*p_task_entry_function)(AG_U32, void*),
    AG_U32      n_param,
    void        *p_stack,
    AG_U32      n_stack_size,
    AG_U32      n_priority);


/* */
/* mutexes */
/*  */
typedef struct {
  sal_sem_t sal_sem;
} AgosBiSemaphoreInfo;

typedef struct
{
    AgosBiSemaphoreInfo x_bi_semaphore;

} AgNdMutex;


/*BCM moved here*/
struct AgosQueueInfo_S
{
    /*#ifdef RTOS_NUCLEUS*/
#if 0 /*(!defined NO_OS_SUPPORT && !defined AG_PC_PROG)*/
    NU_QUEUE	x_nu_queue;
    AG_U32		n_message_size; /* for internal use in Nucleus, the fixed message size */
	void		*p_start_address;
#else /* NO_OS_SUPPORT */
    int filler;
#endif /* NO_OS_SUPPORT */
    /*#endif*/
};
typedef struct AgosQueueInfo_S AgosQueueInfo;
typedef struct
{
    AgosQueueInfo x_queue;

} AgNdQueue;






/* Log levels. */
enum SwLogLevelType_E
{
	SW_LOG_LEVEL_NONE = 1,
	SW_LOG_LEVEL_FATAL,
	SW_LOG_LEVEL_ERROR,
	SW_LOG_LEVEL_INFO,
	SW_LOG_LEVEL_DEBUG
};
typedef enum SwLogLevelType_E SwLogLevelType;
#define SW_LOG_NUM_LEVELS 5 /*This must change if number of SwLogLevelType changes!!! */



AgResult agos_create_bi_semaphore(AgosBiSemaphoreInfo *sem, AG_CHAR *p_name);
AgResult agos_get_bi_semaphore(AgosBiSemaphoreInfo *sem, AG_U32 n_timeout);
AgResult agos_give_bi_semaphore(AgosBiSemaphoreInfo *sem);
AgResult agos_delete_bi_semaphore(AgosBiSemaphoreInfo *sem);

void ag_nd_sw_log_app_init(void);
void ag_nd_sw_log(AG_U16 n_level, char* msg,const char *p_file,int n_line);

void *ag_nd_memset(void *s, int c, size_t n);
void *ag_nd_memcpy(void * s1, const void * s2, size_t n);

AgResult ag_nd_mutex_create(AgNdMutex*, AG_CHAR *p_name);
AgResult ag_nd_mutex_lock(AgNdMutex*, AG_U32 n_timeout);
AgResult ag_nd_mutex_unlock(AgNdMutex*);
AgResult ag_nd_mutex_destroy(AgNdMutex*);

void    ag_nd_sleep_msec(AG_U32 ms);
AG_U32  ag_nd_time_usec(void);

AgResult agos_set_timer(
		    AgosTimerInfo *x_timer,
    AG_U8       *p_name, 
    void        (*p_callback_function)(AG_U32), 
    AG_U32      n_callpack_parameter,
    AG_U32      n_timer_value,
    AG_BOOL     b_periodic);
AgResult agos_cancel_timer(AgosTimerInfo *x_timer);
void agos_wake_after(AG_U32 after);
AG_U32 ag_get_micro_timestamp(void);
int ag_wait_micro_sec_delay(AG_U32 delay);

 /*Events*/
AgResult agos_create_event_group(AgosEventInfo *x_event_group, AG_U8 *p_name);
AgResult agos_set_event(AgosEventInfo *x_event_group, AG_U32 n_flags);
AgResult agos_wait_on_event(AgosEventInfo *x_event_group,
    AG_U32      n_requested_flags,
    AgosEventWaitOperation e_operation,
    AG_U32      *n_retrieved_flags, 
    AG_U32      n_timeout);
AgResult agos_delete_event_group(AgosEventInfo *x_event_group);
AgResult agos_create_task(AgosTaskInfo *x_task_info,
    AG_U8       *p_name,
    void        (*p_task_entry_function)(AG_U32, void*),
    AG_U32      n_param,
    void        *p_stack,
    AG_U32      n_stack_size,
    AG_U32      n_priority,
    AG_U32 slice,
    AG_U32 preempt,
    AG_U32 start);


AgResult agos_calloc(int n_size, void *ptr);
AgResult agos_malloc(int n_size, void *ptr);
AgResult agos_free(void *ptr);


#endif /*BCM_CES_SAL_H*/
