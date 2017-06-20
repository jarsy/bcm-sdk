/*
 * $Id: KaminoDriver.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef SAND_ZDT
#define SAND_ZDT
#endif 

#include "KaminoDriver.h"
/* #include "string.h"  */
#include "assert.h"

#include "hal_user.h"
#include "hal_common.h"
#include "hal_ka_auto.h"
#include <bcm_int/sbx/lock.h>


#include "qe2000.h"

/* Indirect memory reads - note the use of macros here */
/* ASSUMES that stride will always be bit aligned */


#define _INDIRECT_READ_REQUEST(reg, otherAddr, stride) SAND_HAL_WRITE(unit | stride, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define _INDIRECT_READ_REQUEST_CLR_ON_RD(reg, otherAddr, stride) SAND_HAL_WRITE(unit | stride, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, CLR_ON_RD,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define _INDIRECT_WRITE_REQUEST(reg, otherAddr, stride) SAND_HAL_WRITE(unit | stride, KA, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(KA, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )



#define _INDIRECT_WAIT_FOR_ACK(reg, stride, timeoutTime, errorString) do{\
int tloops = 0; \
while (1) {\
int data = SAND_HAL_READ(unit, KA, reg##_ACC_CTRL);\
if (SAND_HAL_GET_FIELD(KA, reg##_ACC_CTRL, ACK, data)==1){\
  data = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, REQ, data, 0);\
  data = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, ACK, data, 1);\
  SAND_HAL_WRITE(unit, KA, reg##_ACC_CTRL, data);\
  break; }\
  tloops++; \
  if(tloops > timeoutTime) return -1; \
} \
}while(0)

#define _INDIRECT_WAIT_FOR_ACK_UNLOCK(reg, stride, timeoutTime, errorString) do{\
int tloops = 0; \
while (1) {\
int data = SAND_HAL_READ(unit, KA, reg##_ACC_CTRL);\
if (SAND_HAL_GET_FIELD(KA, reg##_ACC_CTRL, ACK, data)==1){\
  data = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, REQ, data, 0);\
  data = SAND_HAL_MOD_FIELD(KA, reg##_ACC_CTRL, ACK, data, 1);\
  SAND_HAL_WRITE(unit, KA, reg##_ACC_CTRL, data);\
  break; }\
  tloops++; \
  if(tloops > timeoutTime) { \
    BCM_SBX_UNLOCK(unit) \
    return -1; \
  } \
} \
}while(0)


/*  if(tloops > timeoutTime) break; \
*/
/*  if(tloops > timeoutTime) return -1; \
*/
int sandDrvKaQsRankWrite(uint unit, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, QS_RANK_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QS_RANK, 
			  SAND_HAL_SET_FIELD(KA, QS_RANK_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_RANK, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsRankRead(uint unit,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_RANK, (SAND_HAL_SET_FIELD(KA, QS_RANK_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_RANK, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QS_RANK_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsRandWrite(uint unit, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, QS_RAND_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QS_RAND, 
			  SAND_HAL_SET_FIELD(KA, QS_RAND_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_RAND, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsRandRead(uint unit,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_RAND, (SAND_HAL_SET_FIELD(KA, QS_RAND_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_RAND, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QS_RAND_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}
int sandDrvKaQsLnaRankWrite(uint unit, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, QS_LNA_RANK_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QS_LNA_RANK, 
			  SAND_HAL_SET_FIELD(KA, QS_LNA_RANK_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_RANK, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsLnaRankRead(uint unit,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_LNA_RANK, (SAND_HAL_SET_FIELD(KA, QS_LNA_RANK_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_RANK, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QS_LNA_RANK_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsLnaRandWrite(uint unit, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, QS_LNA_RAND_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QS_LNA_RAND, 
			  SAND_HAL_SET_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_RAND, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsLnaRandRead(uint unit,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_LNA_RAND, (SAND_HAL_SET_FIELD(KA, QS_LNA_RAND_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_RAND, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QS_LNA_RAND_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsMemWrite(uint unit, uint nTableId, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, QS_MEM_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QS_MEM, 
			  SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTableId) |
			  SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQsMemRead(uint unit,  uint nTableId,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_MEM, (SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, MEM_SEL, nTableId)|
				   SAND_HAL_SET_FIELD(KA, QS_MEM_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_MEM, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QS_MEM_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

/* int sandDrvKaQsMemLnaWrite(uint unit, uint nTableId, uint nAddress, uint nData4, uint nData3, uint nData2, uint nData1, uint nData0){*/
int sandDrvKaQsMemLnaWrite(uint unit, uint nTableId, uint nAddress, uint *pData){

  SAND_HAL_WRITE(unit, KA, QS_LNA_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, QS_LNA_MEM_ACC_DATA1, pData[1]);
  SAND_HAL_WRITE(unit, KA, QS_LNA_MEM_ACC_DATA2, pData[2]);
  SAND_HAL_WRITE(unit, KA, QS_LNA_MEM_ACC_DATA3, pData[3]);
  SAND_HAL_WRITE(unit, KA, QS_LNA_MEM_ACC_DATA4, pData[4]);
  _INDIRECT_WRITE_REQUEST(QS_LNA_MEM, 
			  SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTableId) |
			  SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

/*int sandDrvKaQsMemLnaRead(uint unit,  uint nTableId,  uint nAddress, uint* pData4, uint* pData3, uint* pData2, uint* pData1, uint* pData0){*/
int sandDrvKaQsMemLnaRead(uint unit,  uint nTableId,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(QS_LNA_MEM, (SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, MEM_SEL, nTableId)|
				   SAND_HAL_SET_FIELD(KA, QS_LNA_MEM_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(QS_LNA_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, QS_LNA_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, QS_LNA_MEM_ACC_DATA1);
  pData[2] = SAND_HAL_READ(unit, KA, QS_LNA_MEM_ACC_DATA2);
  pData[3] = SAND_HAL_READ(unit, KA, QS_LNA_MEM_ACC_DATA3);
  pData[4] = SAND_HAL_READ(unit, KA, QS_LNA_MEM_ACC_DATA4);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQmFbCacheFifoWrite(uint unit, uint *pData){
  SAND_HAL_WRITE(unit, KA, QM_FB_CACHE_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(QM_FB_CACHE, 0, 0);
  _INDIRECT_WAIT_FOR_ACK(QM_FB_CACHE, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaQmFbCacheFifoRead(uint unit, uint* pData){
  _INDIRECT_READ_REQUEST(QM_FB_CACHE, 0, 0);
  _INDIRECT_WAIT_FOR_ACK(QM_FB_CACHE, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, QM_FB_CACHE_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}
/* This function must only be called by application code to dump and write memory as it takes sbx lock */
/* currently internal code does not call this function.  Do not start calling this function.  Call     */
/* function in src/soc/sbx/qe2000.c                                                                    */
int sandDrvKaQmMemWrite(uint unit, uint nTableId, uint nAddress, uint *pData){

  BCM_SBX_LOCK(unit);
  SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA1, pData[1]);
  SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA2, pData[2]);
  SAND_HAL_WRITE(unit, KA, QM_MEM_ACC_DATA3, pData[3]);
  _INDIRECT_WRITE_REQUEST(QM_MEM, 
			  SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, nTableId) |
			  SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK_UNLOCK(QM_MEM, 0, 1000, errorString);
  BCM_SBX_UNLOCK(unit);
  return SAND_DRV_KA_STATUS_OK;
#if 0
  return soc_qe2000_qm_mem_write( unit, nAddress, nTableId,
                        pData[0],
                        pData[1],
                        pData[2],
                        pData[3]);
#endif

}

/* This function must only be called by application code to dump and write memory as it takes sbx lock */
/* currently internal code does not call this function.  Do not start calling this function.  Call     */
/* function in src/soc/sbx/qe2000.c                                                                    */
int sandDrvKaQmMemRead(uint unit,  uint nTableId,  uint nAddress, uint* pData){

  BCM_SBX_LOCK(unit);

  _INDIRECT_READ_REQUEST_CLR_ON_RD(QM_MEM, (SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, SELECT, nTableId)|
				    SAND_HAL_SET_FIELD(KA, QM_MEM_ACC_CTRL, ADDRESS, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK_UNLOCK(QM_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA1);
  pData[2] = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA2);
  pData[3] = SAND_HAL_READ(unit, KA, QM_MEM_ACC_DATA3);

  BCM_SBX_UNLOCK(unit);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEgMemWrite(uint unit, uint nTableId, uint nAddress, uint *pData){

  SAND_HAL_WRITE(unit, KA, EG_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, EG_MEM_ACC_DATA1, pData[1]);
  SAND_HAL_WRITE(unit, KA, EG_MEM_ACC_DATA2, pData[2]);
  _INDIRECT_WRITE_REQUEST(EG_MEM, 
			  SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, nTableId) |
			  SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(EG_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEgMemRead(uint unit,  uint nTableId,  uint nAddress, uint* pData){

  _INDIRECT_READ_REQUEST(EG_MEM, (SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, SELECT, nTableId)|
				   SAND_HAL_SET_FIELD(KA, EG_MEM_ACC_CTRL, ADDRESS, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(EG_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, EG_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, EG_MEM_ACC_DATA1);
  pData[2] = SAND_HAL_READ(unit, KA, EG_MEM_ACC_DATA2);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEbMemWrite(uint unit, uint nAddress, uint *pData){

  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE0, pData[0]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE1, pData[1]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE2, pData[2]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE3, pData[3]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE4, pData[4]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE5, pData[5]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE6, pData[6]);
  SAND_HAL_WRITE(unit, KA, EB_MEM_ACC_SLICE7, pData[7]);

  _INDIRECT_WRITE_REQUEST(EB_MEM,
                           SAND_HAL_SET_FIELD(KA, EB_MEM_ACC_CTRL, ADDR, nAddress) |
                           SAND_HAL_SET_FIELD(KA, EB_MEM_ACC_CTRL, WRITE_MASK, 0xFF), 0);
  _INDIRECT_WAIT_FOR_ACK(EB_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEbMemRead(uint unit, uint nAddress,  uint *pData){

  _INDIRECT_READ_REQUEST(EB_MEM, (SAND_HAL_SET_FIELD(KA, EB_MEM_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(EB_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE0);
  pData[1] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE1);
  pData[2] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE2);
  pData[3] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE3);
  pData[4] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE4);
  pData[5] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE5);
  pData[6] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE6);
  pData[7] = SAND_HAL_READ(unit, KA, EB_MEM_ACC_SLICE7);
  
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEiMemWrite(uint unit, uint nTableId, uint nAddress, uint *pData0){
  SAND_HAL_WRITE(unit, KA, EI_MEM_ACC_DATA, *pData0);
  _INDIRECT_WRITE_REQUEST(EI_MEM, 
			  SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, MEM_SEL, nTableId) |
			  SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(EI_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEiMemRead(uint unit,  uint nTableId,  uint nAddress, uint* pData0){
  _INDIRECT_READ_REQUEST(EI_MEM, (SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, MEM_SEL, nTableId)|
				   SAND_HAL_SET_FIELD(KA, EI_MEM_ACC_CTRL, ADDR, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(EI_MEM, 0, 1000, errorString);
  *pData0 = SAND_HAL_READ(unit, KA, EI_MEM_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaPmMemWrite(uint unit, uint nAddress, uint *pData){

  SAND_HAL_WRITE(unit, KA, PM_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, PM_MEM_ACC_DATA1, pData[1]);
  SAND_HAL_WRITE(unit, KA, PM_MEM_ACC_DATA2, pData[2]);
  SAND_HAL_WRITE(unit, KA, PM_MEM_ACC_DATA3, pData[3]);
  _INDIRECT_WRITE_REQUEST(PM_MEM, 
			  SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, nAddress) |
			  SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, HASH, ((nAddress>>25)&0x1)), 0);
  _INDIRECT_WAIT_FOR_ACK(PM_MEM, 0, 5000, errorString); /* ab 113004 TEM upped from 1000 to 5000 */
  return SAND_DRV_KA_STATUS_OK;
}

/*int sandDrvKaPmMemRead(uint unit,  uint nAddress, uint* pData3, uint* pData2, uint* pData1, uint* pData0){*/
int sandDrvKaPmMemRead(uint unit,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(PM_MEM, 
			 SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, nAddress)|
			 SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, HASH, ((nAddress>>25)&0x1)),
			 0);
  _INDIRECT_WAIT_FOR_ACK(PM_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, PM_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, PM_MEM_ACC_DATA1);
  pData[2] = SAND_HAL_READ(unit, KA, PM_MEM_ACC_DATA2);
  pData[3] = SAND_HAL_READ(unit, KA, PM_MEM_ACC_DATA3);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaSfMemWrite(uint unit, uint i, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit | SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i, KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(SF0_XCNFG_REMAP_MEM, 
			  SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, nAddress)
			  ,SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i);
  _INDIRECT_WAIT_FOR_ACK(SF0_XCNFG_REMAP_MEM, SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaSfMemRead(uint unit,  uint i, uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(SF0_XCNFG_REMAP_MEM, SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, nAddress),SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i);
  _INDIRECT_WAIT_FOR_ACK(SF0_XCNFG_REMAP_MEM, SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i, 1000, errorString);
  *pData = SAND_HAL_READ(unit | SAND_HAL_KA_SF_INSTANCE_ADDR_STRIDE*i, KA, SF0_XCNFG_REMAP_MEM_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}


int sandDrvKaEpAmClMemWrite(uint unit, uint nAddress, uint *pData) 
{

  SAND_HAL_WRITE(unit, KA, EP_AM_CL_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, EP_AM_CL_MEM_ACC_DATA1, pData[1]);
  _INDIRECT_WRITE_REQUEST(EP_AM_CL_MEM, SAND_HAL_SET_FIELD(KA, 
  			     EP_AM_CL_MEM_ACC_CTRL, 
			     ADDR, nAddress), 0);
  _INDIRECT_WAIT_FOR_ACK(EP_AM_CL_MEM, 0, 1000, errorString);

  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEpAmClMemRead(uint unit, uint nAddress, uint *pData)
{
  _INDIRECT_READ_REQUEST(EP_AM_CL_MEM, SAND_HAL_SET_FIELD(KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, nAddress), 0);
  
  _INDIRECT_WAIT_FOR_ACK(EP_AM_CL_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, EP_AM_CL_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, EP_AM_CL_MEM_ACC_DATA1);
  return SAND_DRV_KA_STATUS_OK;

}

int sandDrvKaEpBmBfMemWrite(uint unit, uint nAddress, uint *pData)
{
  SAND_HAL_WRITE(unit, KA, EP_BM_BF_MEM_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(EP_BM_BF_MEM, 
			  SAND_HAL_SET_FIELD(KA, 
					     EP_BM_BF_MEM_ACC_CTRL, 
					     ADDR, nAddress),
			  0);
  _INDIRECT_WAIT_FOR_ACK(EP_BM_BF_MEM, 0, 1000, errorString);

  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEpBmBfMemRead(uint unit, uint nAddress, uint* pData0)
{
  _INDIRECT_READ_REQUEST(EP_BM_BF_MEM, 
			 SAND_HAL_SET_FIELD(KA, 
					    EP_BM_BF_MEM_ACC_CTRL, 
					    ADDR, 
					    nAddress),
			 0);
  _INDIRECT_WAIT_FOR_ACK(EP_BM_BF_MEM, 0, 1000, errorString);
  *pData0 = SAND_HAL_READ(unit, KA, EP_BM_BF_MEM_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEpMmIpMemWrite(uint unit, uint nAddress, uint *pData)
{
  SAND_HAL_WRITE(unit, KA, EP_MM_IP_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, EP_MM_IP_MEM_ACC_DATA1, pData[1]);
  _INDIRECT_WRITE_REQUEST(EP_MM_IP_MEM, 
			  SAND_HAL_SET_FIELD(KA, 
					     EP_MM_IP_MEM_ACC_CTRL, 
					     ADDR, nAddress),
			  0);
  _INDIRECT_WAIT_FOR_ACK(EP_MM_IP_MEM, 0, 1000, errorString);

  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEpMmIpMemRead(uint unit, uint nAddress, uint* pData)
{
  _INDIRECT_READ_REQUEST(EP_MM_IP_MEM, SAND_HAL_SET_FIELD(KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, nAddress), 0);
  
  _INDIRECT_WAIT_FOR_ACK(EP_MM_IP_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, EP_MM_IP_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, EP_MM_IP_MEM_ACC_DATA1);
  return SAND_DRV_KA_STATUS_OK;

}

int sandDrvKaEpMmBfMemWrite(uint unit, uint nAddress, uint *pData)
{
  SAND_HAL_WRITE(unit, KA, EP_MM_BF_MEM_ACC_DATA0, pData[0]);
  SAND_HAL_WRITE(unit, KA, EP_MM_BF_MEM_ACC_DATA1, pData[1]);
  _INDIRECT_WRITE_REQUEST(EP_MM_BF_MEM, 
			  SAND_HAL_SET_FIELD(KA, 
					     EP_MM_BF_MEM_ACC_CTRL, 
					     ADDR, nAddress),
			  0);
  _INDIRECT_WAIT_FOR_ACK(EP_MM_BF_MEM, 0, 1000, errorString);

  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaEpMmBfMemRead(uint unit, uint nAddress,  uint *pData)
{
  _INDIRECT_READ_REQUEST(EP_MM_BF_MEM, SAND_HAL_SET_FIELD(KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, nAddress), 0);
  
  _INDIRECT_WAIT_FOR_ACK(EP_MM_BF_MEM, 0, 1000, errorString);
  pData[0] = SAND_HAL_READ(unit, KA, EP_MM_BF_MEM_ACC_DATA0);
  pData[1] = SAND_HAL_READ(unit, KA, EP_MM_BF_MEM_ACC_DATA1);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaRbPolMemWrite(uint unit, uint nTableId, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, RB_POL_MEM_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(RB_POL_MEM, 
			  SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, SELECT, nTableId) |
			  SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, ADDRESS, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(RB_POL_MEM, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaRbPolMemRead(uint unit,  uint nTableId,  uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(RB_POL_MEM, (SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, SELECT, nTableId)|
				   SAND_HAL_SET_FIELD(KA, RB_POL_MEM_ACC_CTRL, ADDRESS, nAddress)),0);
  _INDIRECT_WAIT_FOR_ACK(RB_POL_MEM, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, RB_POL_MEM_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaRbClassMemWrite(uint unit, uint nSpi, uint nTableId, uint nAddress, uint *pData) {
  if ( nSpi == 0 ) {
    SAND_HAL_WRITE(unit, KA, RB_CLASS0_MEM_ACC_DATA0, pData[0]);
    SAND_HAL_WRITE(unit, KA, RB_CLASS0_MEM_ACC_DATA1, pData[1]);
    _INDIRECT_WRITE_REQUEST(RB_CLASS0_MEM, 
  			    SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, SELECT, nTableId) |
  			    SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, ADDRESS, nAddress)
			    ,0);
    _INDIRECT_WAIT_FOR_ACK(RB_CLASS0_MEM, 0, 1000, errorString);
    return SAND_DRV_KA_STATUS_OK;
  } else if ( nSpi == 1 ) {
    SAND_HAL_WRITE(unit, KA, RB_CLASS1_MEM_ACC_DATA0, pData[0]);
    SAND_HAL_WRITE(unit, KA, RB_CLASS1_MEM_ACC_DATA1, pData[1]);
    _INDIRECT_WRITE_REQUEST(RB_CLASS1_MEM, 
  			    SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, SELECT, nTableId) |
  			    SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, ADDRESS, nAddress)
			    ,0);
    _INDIRECT_WAIT_FOR_ACK(RB_CLASS1_MEM, 0, 1000, errorString);
    return SAND_DRV_KA_STATUS_OK;
  } else {
    return SAND_DRV_KA_STATUS_ERROR;
  }
}

int sandDrvKaRbClassMemRead(uint unit,  uint nSpi, uint nTableId,  uint nAddress, uint* pData){

  if ( nSpi == 0 ) {
    _INDIRECT_READ_REQUEST(RB_CLASS0_MEM, (SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, SELECT, nTableId)|
	 			     SAND_HAL_SET_FIELD(KA, RB_CLASS0_MEM_ACC_CTRL, ADDRESS, nAddress)),0);
    _INDIRECT_WAIT_FOR_ACK(RB_CLASS0_MEM, 0, 1000, errorString);
    pData[0] = SAND_HAL_READ(unit, KA, RB_CLASS0_MEM_ACC_DATA0);
    pData[1] = SAND_HAL_READ(unit, KA, RB_CLASS0_MEM_ACC_DATA1);
    return SAND_DRV_KA_STATUS_OK;
  } else if ( nSpi == 1 ) {
    _INDIRECT_READ_REQUEST(RB_CLASS1_MEM, (SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, SELECT, nTableId)|
	 			     SAND_HAL_SET_FIELD(KA, RB_CLASS1_MEM_ACC_CTRL, ADDRESS, nAddress)),0);
    _INDIRECT_WAIT_FOR_ACK(RB_CLASS1_MEM, 0, 1000, errorString);
    pData[0] = SAND_HAL_READ(unit, KA, RB_CLASS1_MEM_ACC_DATA0);
    pData[1] = SAND_HAL_READ(unit, KA, RB_CLASS1_MEM_ACC_DATA1);
    return SAND_DRV_KA_STATUS_OK;
  } else {
    return SAND_DRV_KA_STATUS_ERROR;
  }
}


int sandDrvKaPmDllLutWrite(uint unit, uint nTableId, uint nAddress, uint *pData){
  SAND_HAL_WRITE(unit, KA, PM_DLL_ACC_DATA, *pData);
  _INDIRECT_WRITE_REQUEST(PM_DLL, 
			  SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, nTableId) |
			  SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(PM_DLL, 0, 1000, errorString);
  return SAND_DRV_KA_STATUS_OK;
}

int sandDrvKaPmDllLutRead(uint unit,  uint nTableId, uint nAddress, uint* pData){
  _INDIRECT_READ_REQUEST(PM_DLL, 
			  SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, nTableId) |
			  SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, nAddress)
			  ,0);
  _INDIRECT_WAIT_FOR_ACK(PM_DLL, 0, 1000, errorString);
  *pData = SAND_HAL_READ(unit, KA, PM_DLL_ACC_DATA);
  return SAND_DRV_KA_STATUS_OK;
}
