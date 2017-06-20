/*
 * $Id: TkInit.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        name.h
 * Purpose:     Purpose of the file
 */

#ifndef _EA_SOC_TkInit_H_
#define _EA_SOC_TkInit_H_

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOsThread.h>
#include <soc/ea/tk371x/TkOsAlloc.h>
#include <soc/ea/tk371x/TkMsg.h>

#define SEM_VALID(sem)              1   

#define TK_OAM_REPLY_TIME_OUT_MS    40000000

typedef struct {
#define TK_VLAN_LINK_STRING_LEN     20
    int32             vlan;
    uint8             link;
    sal_msg_desc_t  * resMsgQid;
    char              resMsgName[TK_VLAN_LINK_STRING_LEN];
    uint8           * resMsgBuff;
    int16             resMsgLen;
    sal_msg_desc_t  * almMsgQid;
    char              almMsgName[TK_VLAN_LINK_STRING_LEN];
    uint8           * almMsgBuff;
    int16             almMsgLen;
    sal_sem_t         semId;
    char              oltSemName[TK_VLAN_LINK_STRING_LEN];
    sal_thread_t      almTaskId;
    char              almTaskName[TK_VLAN_LINK_STRING_LEN];
    uint8           * apiRxBuff;
    int32             timeOut;
} VlanLinkConfig;

extern VlanLinkConfig gTagLinkCfg[MAX_NUM_OF_PON_CHIP+1];

VlanLinkConfig  * TkGetVlanLink (uint8 pathId);
void            * TkGetApiBuf (uint8 pathId);

/* the system task exit, delete the tasks, message queue, semphore etc. */
void    TkExtOamTaskExit (uint8 pathId);

/* the system task init */
int32   TkExtOamTaskInit (uint8 pathId);



#if defined(__cplusplus)
}
#endif

#endif	/* !_EA_SOC_TkInit_H_ */
