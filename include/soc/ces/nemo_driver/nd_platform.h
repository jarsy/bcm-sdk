/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_PLATFORM_H__
#define __NEMO_PLATFORM_H__

#include <sal/core/libc.h>

#ifdef BCM_CES_SDK
#include <ag_basic_types.h>
#include "bcm_ces_sal.h"

#define GEN_ERR_LOG0(str) 

/* */
/* 32 <-> 16, 16 <-> 8 conversion macros */
/* */
#define AG_ND_MAKE32LH(low, high)   ( (AG_U32)(high) << 16 | (AG_U32)(low) )
#define AG_ND_LOW16(c)              ( (AG_U16)((c)&0x0000FFFF) )
#define AG_ND_HIGH16(c)             ( (AG_U16)(((c)>>16)&0x0000FFFF) )

#define AG_ND_MAKE16LH(low, high)   ( (AG_U16)(high) << 8 | (AG_U16)(low) )
#define AG_ND_LOW8(c)               ( (AG_U8)((c)&0x00FF) )
#define AG_ND_HIGH8(c)              ( (AG_U8)(((c)>>8)&0x00FF) )

/*BCM: For unused parameters: from ag_macros.h*/
extern AG_U32 AGOS_for_unused_params;
#define AG_UNUSED_PARAM(x)  AGOS_for_unused_params=(AG_U32)(x)


void ag_nd_sw_log_app_init(void);
void ag_nd_sw_log(AG_U16 n_level, char* msg,const char *p_file,int n_line);

void    ag_nd_sleep_msec(AG_U32 ms);
AG_U32  ag_nd_time_usec(void);
void    *ag_nd_memset(void *s, int c, size_t n);
void    *ag_nd_memcpy(void * s1, const void * s2, size_t n);

AgResult    ag_nd_mutex_create(AgNdMutex*, AG_CHAR *p_name);
AgResult    ag_nd_mutex_lock(AgNdMutex*, AG_U32 n_timeout);
AgResult    ag_nd_mutex_unlock(AgNdMutex*);
AgResult    ag_nd_mutex_destroy(AgNdMutex*);

int ag_wait_micro_sec_delay(AG_U32 micro_secounds_delay);
AgResult agos_malloc(int size, void* mem);
AgResult agos_calloc(int size, void* mem);
AgResult agos_free(void* mem);

#ifdef AG_LITTLE_ENDIAN
AG_U16  ag_hton16(AG_U16 n_val);
AG_U32  ag_hton32(AG_U32 n_val);
#else
#define ag_hton16(n_val)  n_val
#define ag_hton32(n_val)  n_val
#endif
	

#else   /*BCM:from here down for BCM*/

#include "ag_common.h"
#include "agos/agos_internals.h"
#include "ag_hl_api.h"

#ifdef AG_DEBUGGER
    #define AG_ND_ENABLE_TRACE
    #define AG_ND_ENABLE_ASSERT
    #define AG_ND_ENABLE_VALIDATION
#endif

#if defined WIN32

    #include <stdio.h>
    
    #define __func__    "__func__"

/*    #pragma warning( disable : 4244) */
    
    #ifdef __cplusplus
    extern "C"
    {
    #endif

    #define vsnprintf(s,n,format,arg)   _vsnprintf(s,n,format,arg)
    #define INLINE              __inline
    #define AG_ND_PRINTF_MAX       1024


    AG_U16 ag_hton16(AG_U16);
    AG_U32 ag_hton32(AG_U32);


    #ifdef __cplusplus
    }
    #endif

#else /* WIN32 */

    #include "agos/agos_trace.h"
    #include "utils/gen_net.h"

    
    #ifdef __cplusplus
    extern "C"
    {
    #endif

    #pragma check_printf_formats
    int ag_nd_printf(const char *format, ...);
    #pragma no_check_printf_formats

    #define AG_ND_PRINTF_MAX       1024

    #define INLINE              __inline       /*BCM , for other compiler might define as "inline" */

    #ifdef __cplusplus
    }
    #endif

    #ifndef __cplusplus
    #define printf(...)             ag_nd_printf(__VA_ARGS__)
    #define fprintf(stream, ...)    ag_nd_printf(__VA_ARGS__)
    #endif
    

#endif /* WIN32 */


#ifdef __cplusplus
extern "C"
{
#endif


/* */
/* 32 <-> 16, 16 <-> 8 conversion macros */
/* */
#define AG_ND_MAKE32LH(low, high)   ( (AG_U32)(high) << 16 | (AG_U32)(low) )
#define AG_ND_LOW16(c)              ( (AG_U16)((c)&0x0000FFFF) )
#define AG_ND_HIGH16(c)             ( (AG_U16)(((c)>>16)&0x0000FFFF) )

#define AG_ND_MAKE16LH(low, high)   ( (AG_U16)(high) << 8 | (AG_U16)(low) )
#define AG_ND_LOW8(c)               ( (AG_U8)((c)&0x00FF) )
#define AG_ND_HIGH8(c)              ( (AG_U8)(((c)>>8)&0x00FF) )

void    ag_nd_sleep_msec(AG_U32 ms);
AG_U32  ag_nd_time_usec(void);
void    *ag_nd_memset(void *s, int c, size_t n);
void    *ag_nd_memcpy(void * s1, const void * s2, size_t n);

/* */
/* mutexes */
/*  */
typedef struct
{
    AgosBiSemaphoreInfo x_bi_semaphore;

} AgNdMutex;

AgResult    ag_nd_mutex_create(AgNdMutex*, AG_CHAR *p_name);
AgResult    ag_nd_mutex_lock(AgNdMutex*, AG_U32 n_timeout);
AgResult    ag_nd_mutex_unlock(AgNdMutex*);
AgResult    ag_nd_mutex_destroy(AgNdMutex*);

/* */
/* queues */
/*  */
typedef struct
{
    AgosQueueInfo x_queue;

} AgNdQueue;

AgResult    ag_nd_queue_create(AgNdQueue*, AG_U8 *p_name, AG_U32 n_queue_size, AG_U32 n_message_size);
AgResult    ag_nd_queue_delete(AgNdQueue*);
AgResult    ag_nd_queue_receive(AgNdQueue*, void *p_message, AG_U32 n_timeout);
AgResult    ag_nd_queue_send(AgNdQueue*, void *p_message, AG_U32 n_timeout);
AgResult    ag_nd_queue_send_to_front(AgNdQueue*, void *p_message, AG_U32 n_timeout);

/* */
/* timers */
/*  */
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
/*  */
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

void ag_nd_sw_log_app_init(void);
void ag_nd_sw_log(AG_U16 n_level, char* msg,const char *p_file,int n_line);

#ifdef __cplusplus
}
#endif

#endif /*BCM_CES_SDK*/

#endif /* __NEMO_PLATFORM_H__ */

