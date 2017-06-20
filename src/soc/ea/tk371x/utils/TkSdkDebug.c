/*
 * $Id: TkSdkDebug.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkSdkDebug.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkOsCm.h>

uint32 g_ulTkSdkDbgLevelSwitch = 0x0;

char *TkDbgStr[] = {
    "TkDbgLogTraceEnable",
    "TkDbgRxDataEnable",
    "TkDbgTxDataEnable",
    "TkDbgMsgEnable",
    "TkDbgAlmEnable",
    "TkDbgOamEnable",
    "TkDbgOamTkEnable",
    "TkDbgOamCtcEnable",
    NULL
};

Bool
TkDbgLevelIsSet(uint32 lvl)
{
    if (lvl & g_ulTkSdkDbgLevelSwitch) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void
TkDbgDataDump(uint8 * p, uint16 len, uint16 width)
{
    static char  stringbuff[TkDbgMaxMsgLength];
    int  index;
    char *pstr = stringbuff; 
    int  i;

    if(len*3 >= TkDbgMaxMsgLength){
        len = TkDbgMaxMsgLength/3;
    }
    
    for(index = 0; index < (len/width +1); index++){
        for(i = 0; i < width; i++){
            sal_sprintf((char *)(pstr + i*3), "%02x ", p[index*width + i]);
        }
        TkDbgPrintf(("%s\n", stringbuff));
    }
}

void 
TkDbgLevelSet(uint32 lvl)
{
    g_ulTkSdkDbgLevelSwitch = lvl;
}

void
TkDbgLevelDump(void)
{
    int i;
    
    TkDbgPrintf(("DbgBitMap = 0x%08x\n",g_ulTkSdkDbgLevelSwitch));
    
    for(i = 0; TkDbgStr[i] != NULL; i++){
         TkDbgPrintf(("Bit % 2d:%s\n",i,TkDbgStr[i]));   
    }
}


