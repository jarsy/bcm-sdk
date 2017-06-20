/*
 * $Id: TkSdkInitApi.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkSdkInitApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/OamProcess.h>
#include <soc/ea/tk371x/TkInit.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/AlarmProcess.h>

/*
 * Teknovus OUI 
 */
const IeeeOui   TeknovusOui = { {0x00, 0x0D, 0xB6} };

/*
 * CTC OUI 
 */
const IeeeOui   CTCOui = { {0x11, 0x11, 0x11} };

MacAddr         OamMcMacAddr = { {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02} };

                                             

/*
 *  Function:
 *      TkGetVlanLink
 * Purpose:
 *      Get the information about the OAM processing per EPON MAC
 * Parameters:
 *      pathId    - ONU index
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
VlanLinkConfig *
TkGetVlanLink(uint8 pathId)
{
    return &(gTagLinkCfg[pathId % 8]);
}

/*
 *  Function:
 *      TkGetApiBuf
 * Purpose:
 *      Get the api buffer for storing API response message
 * Parameters:
 *      pathId    - ONU index
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
void *
TkGetApiBuf(uint8 pathId)
{
    return gTagLinkCfg[pathId % 8].apiRxBuff;
}

/*
 *  Function:
 *      TkExtOamTaskExit
 * Purpose:
 *      Free the resource alloced for the unit.
 * Parameters:
 *      tagCount    - If use vlan to identify the physical path. this should set to no 0 number.
 *                         0 means not use VLAN to identify the physical path. no 0 value identify
 *                         how many physical EPON chipset the system support.
 *      tags    - Vlan tag list, and must fixed as 0x81000xxx or 0x88080xxx
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
void
TkExtOamTaskExit(uint8 pathId)
{
    if(pathId >= MAX_NUM_OF_PON_CHIP){
        return;
    }
    
    if (SAL_THREAD_ERROR != gTagLinkCfg[pathId].almTaskId)
        sal_thread_destroy(gTagLinkCfg[pathId].almTaskId);    
    if (SEM_VALID(gTagLinkCfg[pathId].semId))
        sal_sem_destroy(gTagLinkCfg[pathId].semId);
    if (gTagLinkCfg[pathId].almMsgQid)
        sal_msg_destory(gTagLinkCfg[pathId].almMsgQid);
    if (gTagLinkCfg[pathId].resMsgQid)
        sal_msg_destory(gTagLinkCfg[pathId].resMsgQid);
    
    TkOamMemsFree(pathId);
    return;
}


/*
 *  Function:
 *      TkExtOamTaskInit
 * Purpose:
 *      Requrest resource for the system
 * Parameters:
 *      pathId    - The EPON chipset unit
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamTaskInit(uint8 pathId)
{
    uint32          unit = pathId;
    int             ret;

    if(pathId >= MAX_NUM_OF_PON_CHIP){
        return ERROR;
    }

    ret = TkOamMemInit(pathId, 24, 1600);

    if(OK != ret){
        return ret;
    }
    
    sal_sprintf(gTagLinkCfg[pathId].oltSemName, "oltSem%d",pathId);
    sal_sprintf(gTagLinkCfg[pathId].resMsgName, "resMsg%d",pathId);
    sal_sprintf(gTagLinkCfg[pathId].almMsgName, "almMsg%d",pathId);
    sal_sprintf(gTagLinkCfg[pathId].almTaskName, "almTask%d",pathId);

    TkDbgInfoTrace(TkDbgLogTraceEnable,("%s\n",gTagLinkCfg[pathId].oltSemName));
    TkDbgInfoTrace(TkDbgLogTraceEnable,("%s\n",gTagLinkCfg[pathId].resMsgName));
    TkDbgInfoTrace(TkDbgLogTraceEnable,("%s\n",gTagLinkCfg[pathId].almMsgName));
    TkDbgInfoTrace(TkDbgLogTraceEnable,("%s\n",gTagLinkCfg[pathId].almTaskName));
    
    gTagLinkCfg[pathId].vlan = 0x81000001;
    gTagLinkCfg[pathId].resMsgQid = 
        sal_msg_create(gTagLinkCfg[pathId].resMsgName, 5);
    gTagLinkCfg[pathId].semId = 
        sal_sem_create(gTagLinkCfg[pathId].oltSemName, sal_sem_BINARY, 1);
    gTagLinkCfg[pathId].apiRxBuff = 
        (uint8 *)TkOamMemGet(pathId);
    gTagLinkCfg[pathId].timeOut = TK_OAM_REPLY_TIME_OUT_MS;
    gTagLinkCfg[pathId].almMsgQid = 
        sal_msg_create(gTagLinkCfg[pathId].almMsgName, 20);
    gTagLinkCfg[pathId].almTaskId =
        sal_thread_create(gTagLinkCfg[pathId].almTaskName, 
            2000, 7, TkAlarmRxThread, (void *)INT_TO_PTR(unit));
    
    TkAlmHandlerRegister(pathId, AlarmProcessHandler);

    return OK;
}

