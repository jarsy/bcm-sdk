/*
 * $Id: KaminoDriver.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef AB_KaminoDriver_H
#define AB_KaminoDriver_H


#include <sal/types.h>

#define SAND_DRV_KA_STATUS_OK     0 
#define SAND_DRV_KA_STATUS_ERROR -1


int sandDrvKaSetDefaultProfile(uint32 unit);
int sandDrvKaVerifyProfile(uint32 unit);

int sandDrvKaQsRankWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaQsRankRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaQsRandWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaQsRandRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaQsLnaRankWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaQsLnaRankRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaQsLnaRandWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaQsLnaRandRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaQsMemWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaQsMemRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData);

int sandDrvKaQsMemLnaWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaQsMemLnaRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData);

int sandDrvKaQmMemWrite(uint32 unit, uint32 nTableId, uint32 nAddress,  uint32 *pData);
int sandDrvKaQmMemRead(uint32 unit,  uint32 nTableId, uint32 nAddress,  uint32 *pData);

int sandDrvKaQmFbCacheFifoWrite(uint32 unit, uint32 *pData);
int sandDrvKaQmFbCacheFifoRead(uint32 unit,  uint32 *pData);

int sandDrvKaPmMemWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaPmMemRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaSfMemWrite(uint32 unit, uint32 nInstance, uint32 nAddress, uint32 *pData);
int sandDrvKaSfMemRead(uint32 unit,  uint32 nInstance, uint32 nAddress, uint32 *pData);

int sandDrvKaEgMemWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaEgMemRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData);

int sandDrvKaEiMemWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData0);
int sandDrvKaEiMemRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData0);

int sandDrvKaEbMemWrite(uint32 unit, uint32 nAddress,  uint32 pData[8]);
int sandDrvKaEbMemRead(uint32 unit, uint32 nAddress,  uint32 pData[8]);

int sandDrvKaEpAmClMemWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaEpAmClMemRead(uint32 unit, uint32 nAddress,  uint32 *pData);

int sandDrvKaEpBmBfMemWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaEpBmBfMemRead(uint32 unit,  uint32 nAddress, uint32 *pData);

int sandDrvKaEpMmIpMemWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaEpMmIpMemRead(uint32 unit, uint32 nAddress,  uint32 *pData);

int sandDrvKaEpMmBfMemWrite(uint32 unit, uint32 nAddress, uint32 *pData);
int sandDrvKaEpMmBfMemRead(uint32 unit, uint32 nAddress,  uint32 *pData);

int sandDrvKaRbPolMemWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaRbPolMemRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData);

int sandDrvKaRbClassMemWrite(uint32 unit, uint32 nSpi, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaRbClassMemRead(uint32 unit,  uint32 nSpi, uint32 nTableId, uint32 nAddress, uint32 *pData);

int sandDrvKaPmDllLutWrite(uint32 unit, uint32 nTableId, uint32 nAddress, uint32 *pData);
int sandDrvKaPmDllLutRead(uint32 unit,  uint32 nTableId, uint32 nAddress, uint32 *pData);

#ifdef __cplusplus
}
#endif



#endif
