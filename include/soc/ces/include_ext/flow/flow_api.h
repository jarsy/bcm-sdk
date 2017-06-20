/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
     flow

File Name:
     flow_api.h

File Description:
	 flow api

$Revision: 1.1.2.1 $ - visual sourcsafe revision number

History:
	Shay Hadar 17/4/2001   Initial Creation

******************************************/
#ifndef FLOW_API_H
#define FLOW_API_H

#ifndef BCM_CES_SDK
#include "drivers/bmd_types.h"
#endif

/***  extern definition - for functions prototype ***/

#ifdef FLOW_API_C
#define AG_EXTERN
#else
#define AG_EXTERN extern
#endif


/********************************
 *  Functions declaration
 ********************************/

#ifdef __cplusplus
extern "C"
{
#endif



/******************************************
Function Name: ag_flow_put_in_q
Description:
	put the bundle in the sw q. in case that the q is full 
	frame's treatment depends on e_action:
	1.FLOW_QFULL_OVERWRITE - remove the latest bundle in the q(the first one)
	and then insert the bundle to the q's tail.
	2.FLOW_QFULL_DISCARD - discard the frame.
	3.FLOW_QFULL_NONE - return error indication
Parameters:
	IN AgosQueueInfo	*p_queue_handle: handle to the q
	IN AgBmdBundPtr		p_bundle: pointer to the bundle
	IN DispQFullAction	e_action: action enum
	IN void*			p_arg:parameter or the target application
Returns :
	AG_S_OK	- Success
	AG_E_FULL- The q is full and e_action= FLOW_QFULL_NONE. 
	AG_FLOW_S_DISCARDED- The q is full and e_action= FLOW_QFULL_DISCARD
	AG_FLOW_S_OVERWRITE- The q is full and e_action= FLOW_QFULL_OVERWRITE
	AG_E_FAIL- Failed to put the frame in the q (e.g. bad params not AG_E_FULL).
			   The frame should be discarded by the application.
History:
    Shay Hadar 18/4/2001   Initial Creation
******************************************/

AG_EXTERN AgResult ag_flow_put_in_q
(
 AgosQueueInfo		*p_queue_handle,
 AgBmdBundPtr		p_bundle,
 AgQFullAction		e_action,
 void				*p_arg
);

AG_EXTERN AgResult ag_flow_free_all_q_bundles(AG_U8 n_que, AG_U16 *p_que_len);

/******************************************
Function Name: ag_flow_init
Description:
	currently call to dispatcher, transmitter init functions
Parameters:
Returns :
	AG_S_OK -Success
	AG_E_FAIL -	failed In one of the actions.
History:
    Shay Hadar 23/5/2001   Initial Creation
******************************************/

AG_EXTERN AgResult ag_flow_init(void);


/******************************************
Function Name: ag_flow_get_all_q_bundles
Description: get all bundles from HW queue.
     
Parameters:
	AG_U8 q_num:
	AG_U16 *p_q_len:
	AgBmdBundPtr *p_bundle:
Returns AgResult :
  like of ag_qmd_clr_q_first
History:
    Yaeli Karni 4/7/2003    Initial Creation
******************************************/
AgResult ag_flow_get_all_q_bundles(AG_U8 q_num, AG_U16 *p_q_len, AgBmdBundPtr	*p_bundle);

#ifdef __cplusplus
} /*end of extern "C"*/
#endif



#undef AG_EXTERN
#endif  /* FLOW_API_H */
