/*
 * $Id: TkDebug.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkDebug.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkDebug_H
#define _SOC_EA_TkDebug_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <shared/bsl.h>

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsCm.h>


#define TkDbgMaxMsgLength   2048

#define TkDbgLevelOff       0x00000000
#define TkDbgErrorEnable    1
#define TkDbgLogTraceEnable (1<<1)
#define TkDbgRxDataEnable   (1<<2)
#define TkDbgTxDataEnable   (1<<3)
#define TkDbgMsgEnable      (1<<4)
#define TkDbgAlmEnable      (1<<5)
#define TkDbgOamEnable      (1<<6)
#define TkDbgOamTkEnable    (1<<7)
#define TkDbgOamCtcEnable   (1<<8)

#define TkDbgPrintf(x)      bsl_printf x

#define TkDbgTrace(errLevel) if(TkDbgLevelIsSet(errLevel))  \
                TkDbgPrintf(("\r\n%s,%s,%d\n", __FILE__,FUNCTION_NAME(),__LINE__))

#define TkDbgInfoTrace(errLevel,x) if(TkDbgLevelIsSet(errLevel))\
                {                                               \
                TkDbgPrintf(x);                                 \
                TkDbgPrintf((":%s,%s,%d\n", __FILE__,FUNCTION_NAME(),__LINE__)); \
                }

#define TkDbgInfoDataDump(errLevel,x,pData,len,width) if(TkDbgLevelIsSet(errLevel))\
                {                               \
                TkDbgPrintf(x);                 \
                TkDbgDataDump(pData,len,width); \
                }

void TkDbgDataDump (uint8 * p, uint16 len, uint16 width);

Bool TkDbgLevelIsSet (uint32 lvl);

void TkDbgLevelSet(uint32 lvl);

void TkDbgLevelDump(void);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkDebug_H */
