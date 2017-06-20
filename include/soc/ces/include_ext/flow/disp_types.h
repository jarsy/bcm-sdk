/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
     dispatcher

File Name:
     disp_types.h

File Description:
	 
$Revision: 1.1.2.1 $ - visual sourcsafe revision number

History:
	Shay Hadar 18/4/2001   Initial Creation

******************************************/
#ifndef DISP_TYPES_H
#define DISP_TYPES_H


/********************************
 *  General Structures
 ********************************/


struct DispHoldFrameInfo_S
{
	AgosQueueInfo		*p_q_handle;
	AgBmdBundPtr		p_bundle;
	void				*p_arg;
	AgQFullAction		e_action;
	AG_U32				n_of_frames;
};
       
typedef struct DispHoldFrameInfo_S DispHoldFrameInfo;

typedef void (*dispBundlCallBack)(AgBmdBundPtr p_bundle, AG_U8 n_cpu_hw_queue);

#endif  /* DISP_TYPES_H */
