/*
 * $Id: nlmcmdevice.h,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 #ifndef INCLUDED_NLMCMDEVICE_H
#define INCLUDED_NLMCMDEVICE_H


/* This file contains common device attributes among all devices supported by Cynapse S/W */

/* Enum for device type. One specific usage of device type is that it is passed as
 * one of the input parameters during Table Manager Init(s).
 */
typedef enum NlmDevType
{
    NLM_DEVTYPE_9K,     /* non-sahasra NL9K device       */
    NLM_DEVTYPE_9K_S,   /* Sahasra NL9K device           */

    NLM_DEVTYPE_10K,        /* non-sahasra NL10K device       */
    NLM_DEVTYPE_10K_S,      /* Sahasra NL10K device           */
    NLM_DEVTYPE_10K_80M,    /* non-sahasra NL10K 80M device   */
    NLM_DEVTYPE_10K_80M_S,  /* Sahasra NL10K 80M device       */

    NLM_DEVTYPE_11K,    /* non-sahasra NL11K device       */
    NLM_DEVTYPE_11K_S,  /* Sahasra NL11K device           */

    NLM_DEVTYPE_12K,    /*non sahasra NL12K device */ 

    NLM_DEVTYPE_0,          
    NLM_DEVTYPE_0_S,        

    NLM_DEVTYPE_1,          
    NLM_DEVTYPE_1_S,        

    NLM_DEVTYPE_1_80M,      
    NLM_DEVTYPE_1_80M_S,    

    NLM_DEVTYPE_2,          
    NLM_DEVTYPE_2_S,        

    NLM_DEVTYPE_3,          
    NLM_DEVTYPE_3_N,
    NLM_DEVTYPE_3_40M,
    NLM_DEVTYPE_3_N_40M,

    NLM_DEVTYPE_END  /* must be the last element */

} NlmDevType;

#define NLM_DEV_NUM_KEYS    4

#endif
/* */
