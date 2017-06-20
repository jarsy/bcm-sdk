/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
     dispatcher

File Name:
     disp_api.h

File Description:
	 dispatcher api
$Revision: 1.1.2.1 $ - visual sourcsafe revision number

History:
	Shay Hadar 15/4/2001   Initial Creation

******************************************/
#ifndef DISP_API_H
#define DISP_API_H

#if (!defined NO_OS_SUPPORT && !defined AG_PC_PROG)
#include "nucleus.h"
#endif
#include "drivers/qmd_api.h"
#include "flow/disp_types.h"

/***  extern definition - for functions prototype ***/

#ifdef DISP_API_C
#define AG_EXTERN
#else
#define AG_EXTERN extern
#endif


#define AG_DISP_MAX_NUMBER_OF_CPU_QUEUES 8


/********************************
 *  General Enums
 ********************************/

/*q handle for special frames:*/
enum DispRegFrameQHandle_E
{    
    AG_DISP_ERROR,
	AG_DISP_FRAGMENT,
	AG_DISP_NUM_OF_SPECIAL_FRAMES
};
typedef enum DispRegFrameQHandle_E DispRegFrameQHandle;


/********************************
 *  General Defines
 ********************************/

#define AG_DISP_REGIST_BUFF_NUM_OF_RECORD AG_DISP_NUM_OF_SPECIAL_FRAMES
#define AG_DISP_QWEIGHT_NOT_CHANGE (-10)
#define AG_DISP_QWEIGHT_AS_QLENGTH (-1)
#define AG_DISP_Q_NOT_ACTIVE		(-1)
#define AG_DISP_QNUMBER_NOT_CHANGE (-10)

/********************************
 *  Functions declaration
 ********************************/

#ifdef __cplusplus
extern "C"
{
#endif



/******************************************
Function Name: ag_disp_task
Description:
	the function implement the dispatcher task.
Parameters:
	IN UNSIGNED argc: Number of argument that may pass to the function.
	IN VOID *argv: Pointer to the information that may pass to the function.
Returns :
History:
    Shay Hadar 15/4/2001   Initial Creation
******************************************/

AG_EXTERN void ag_disp_task(
#if (!defined NO_OS_SUPPORT && !defined AG_PC_PROG)
  UNSIGNED argc, 
  VOID *argv);
#else
   unsigned long argc,
   void *argv);
#endif

/******************************************
Function Name: ag_disp_set_qnumbers 
Description:
	When one or more than one of the CPU's queues became active, the dispatcher
	get event on it. There should be Configuration mechanisms that will translate
	from event number to the actual queue number. The default configuration number
	of the CPU's queues will be 0 to 7 in correlation to event number.
	This function gets the actual CPU's queues numbers. The function checks the validity
	of the queue number (between 0 to 255, entry value modulo 8 = entry index, if the value
	of an entry is DISP_Q_NOT_ACTIVE it means that this event is not active, if the value of
	an entry is DISP_QNUMBER_NOT_CHANGE it means to leave the previous value) and then stored them.
Parameters:
	IN AG_S16 *p_qnumbers: poinetr to array of the real queues number
Returns :
	AG_S_OK -Success
	AG_E_NULL_PTR -	p_qnumbers = null
	AG_E_BAD_PARAMS - One of the validity checks is incorrect.

History:
    Shay Hadar 18/4/2001   Initial Creation
******************************************/

AG_EXTERN AgResult ag_disp_set_qnumbers 
(
 AG_S16	*p_qnumbers
);

/******************************************
Function Name: ag_disp_get_qnumbers 
Description:
	The function retrieves the actual CPU's queues numbers
Parameters:
	OUT AG_S16 *p_qnumbers: poinetr to array of the real queues number
Returns :

History:
    Shay Hadar 18/4/2001   Initial Creation
******************************************/

AG_EXTERN void ag_disp_get_qnumbers 
(
 AG_S16	*p_qnumbers
);

/******************************************
Function Name: ag_disp_register_qhandle
Description:
register q handle for special frames treatment
Parameters:
	IN DispRegFrameQHandle e_reg_qhandle: enum of the special frame .
	IN AgosQueueInfo *p_queue_handle: handle to the special frame's application queue
	IN DispQFullAction e_action:action to do with bundle that it target SW queue is full
	IN void	*p_arg: another argument for the q handle
Returns :
	AG_S_OK	- Success
	AG_E_NULL_PTR - p_queue_handle = null
	AG_E_OUT_OF_RANGE - e_reg_qhandle is out of range.

History:
    Shay Hadar 18/4/2001   Initial Creation
******************************************/

AgResult ag_disp_register_qhandle
(
 DispRegFrameQHandle	e_reg_qhandle,
 AgosQueueInfo			*p_queue_handle,
 AgQFullAction			e_action,
 void					*p_arg
);

/******************************************
Function Name: ag_disp_set_sw_qweight
Description:
	This function is used in the cases that we want to change the target SW-queues
	length without the necessary of changing the queue length (this requires new SW load).
	The function get array of queue's weight (each entry represent a queue). The default
	value of an entry is DISP_QWEIGHT_AS_QLENGTH.If the value of an entry is 
	DISP_QWEIGHT_NOT_CHANGE it means to leave the previous value. 
Parameters:
	IN AG_S16 *p_qweight: poinetr to array of the queues weights
Returns :
	AG_S_OK -Success
	AG_E_NULL_PTR -	p_qweight = null
	AG_E_OUT_OF_RANGE - the weight is out of range

History:
    Shay Hadar 18/4/2001   Initial Creation
******************************************/

AgResult ag_disp_set_sw_qweight
(
 AG_S16	*p_qweight
);

/******************************************
Function Name: ag_disp_get_sw_qweight
Description:
	get the configuration weights of the SW-Qs
Parameters:
	OUT AG_S16 *p_qweight: poinetr to array of the queues weights
Returns :
History:
    Shay Hadar 16/5/2001   Initial Creation
******************************************/

void ag_disp_get_sw_qweight
(
 AG_S16	*p_qweight
);

/******************************************
Function Name: disp_init
Description:
	1.create xDispTimerEventInfo group event.
	2.set event timer.
Parameters:
Returns :
	AG_S_OK -Success
	AG_E_FAIL -	failed In one of the actions.

History:
    Shay Hadar 2/5/2001   Initial Creation
******************************************/

AgResult disp_init(void);

/******************************************
Function Name: ag_disp_set_time_period
Description:
	Set the time (T) period between the rounds. In case that the timer is still
	not set the value is saved for future use (when the function ag_disp_init ()
	is called). The default value of time (T) period is 50 milliseconds.
	The range of time (T) period is from 1 to 10000. 
	If the value of n_time_period is smaller than MILLISECONDSPERTICK the timer
	will set with MILLISECONDSPERTICK value.
Parameters:
	AG_U32 n_time_period:The period between the rounds in millisecond. 
Returns :
	AG_S_OK	-Success.
	AG_E_OUT_OF_RANGE -Time (T) period is out of range.
	AG_E_FAIL -Failed to set the timer.

History:
    Shay Hadar 2/5/2001   Initial Creation
******************************************/

AgResult ag_disp_set_time_period (AG_U32 n_time_period);

/******************************************
Function Name: ag_disp_get_time_period
Description:
	Get the time (T) period between the rounds.
Parameters:
	AG_U32 *p_time_period:pointer to the period between the rounds in millisecond. 
Returns :
History:
    Shay Hadar 2/5/2001   Initial Creation
******************************************/

void ag_disp_get_time_period (AG_U32 *p_time_period);

/******************************************
Function Name: ag_disp_set_n_size
Description:
	Number of message to leave in the SW-Q.
Parameters:
	AG_U8 n_leave_mes_num: number of messages to leave in the SW-Q.
Returns :
History:
    Shay Hadar 2/5/2001   Initial Creation
******************************************/

void ag_disp_set_n_size (AG_U8 n_leave_mes_num);

/******************************************
Function Name: ag_disp_get_n_size
Description:
	Get the number of message to leave in the SW-Q.
Parameters:
	AG_U8 *p_leave_mes_num: number of messages to leave in the SW-Q.
Returns :
History:
    Shay Hadar 2/5/2001   Initial Creation
******************************************/

void ag_disp_get_n_size (AG_U8 *p_leave_mes_num);


/* API to set callback function and work mode in callback */
void ag_disp_set_callback(dispBundlCallBack cb_func);

/* API to set no check of SW q size (work by the HW-Q-Len) */
void ag_disp_set_no_q_size(void);

/* API to set no check of next frame (from the same HW-q) SW-Q details (assume it should
   be assigned to the same queue */
void ag_disp_set_no_next_frame_details(void);

/* API to set option to wait on timer if we have any 'hold' frames - for
   congestion times
*/
void ag_disp_set_timer_only_on_hold(void);

/* API for internal use by rules DB - get queue number checks if in 
   dispatcher list of HW-queues (destined to CPU) */
void ag_disp_is_q_in_cpu_list(AG_U8 n_queue, AG_BOOL *p_exist_queue);

/* API to set dispatcher work mode by TIMER ONLY - always wait
   for next timer to round on CPU-HW-Qs */
void ag_disp_set_work_by_timer_only(void);

/* API to set CPU-HW-Q data to the sw as one message of linked bundles 
   q-num is assumed to be the actual cpu-hw-q number */
AgResult ag_disp_set_q_for_list_of_bundles(AG_U8 q_num);


#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#undef AG_EXTERN
#endif  /* DISP_API_H */                    
