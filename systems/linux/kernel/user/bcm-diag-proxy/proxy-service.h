/***********************************************************************
 *
 * $Id: proxy-service.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 ***********************************************************************/

#ifndef __PROXY_SERVICE_H__
#define __PROXY_SERVICE_H__

#include <stdlib.h>
#include <stdio.h>

/*
 * Proxy service control data 
 */
struct proxy_ctrl_s; 

typedef int (*proxy_data_cb_t)(struct proxy_ctrl_s* ctrl, 
			       unsigned char* data, 
			       unsigned int* len); 

typedef struct proxy_ctrl_s {
    proxy_data_cb_t input_cb; 
    proxy_data_cb_t output_cb; 
    proxy_data_cb_t exit_cb; 
    unsigned int max_data_size; 
    volatile int exit;
} proxy_ctrl_t; 


/*
 * Function:
 *	proxy_service_readr_start
 * Purpose:
 *	Start a proxy thread/loop
 * Parameters:
 *	ctrl	-	Proxy control structure
 *	fork	-	Indicates whether a new thread should be created. 
 *			TRUE: run loop in a new thread. FALSE: Run loop (blocking)
 * Returns:
 *	0
 * Notes:
 */
extern int proxy_service_start(proxy_ctrl_t* ctrl, int fork); 


#endif /* __PROXY_SERVICE_H__ */


