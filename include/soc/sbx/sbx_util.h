/*
 * $Id: sbx_util.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * SBX utility macros and other useful accessor routines
 */

#ifndef _SBX_UTIL_H_
#define _SBX_UTIL_H_

#include <sal/core/thread.h> /* for sal_usleep */

#define SOC_SBX_UTIL_READ_REQUEST(unit, device, reg, otherAddr) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_SBX_UTIL_READ_REQUEST_CLR_ON_RD(unit, device, reg, otherAddr) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, CLR_ON_RD,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_SBX_UTIL_WRITE_REQUEST(unit, device, reg, otherAddr)\
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )

#define SOC_SBX_UTIL_WRITE_REQUEST_WITH_PARITY(unit, device, otherAddr, parity) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, CORRUPT_PARITY, parity)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )

#ifdef VXWORKS

#define SOC_SBX_UTIL_WAIT_FOR_ACK(unit, device, reg, nTimeoutInMs, nStatus) \
{ \
int32 nTimeCnt = 0;\
while (1) {\
  uint32 uDataWaitForAck = SAND_HAL_READ(unit, device, reg##_ACC_CTRL); \
  nStatus = 0;\
  if (SAND_HAL_GET_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){ \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0); \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1); \
    SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, uDataWaitForAck);\
    break; \
  }\
  if (nTimeCnt >= nTimeoutInMs){ \
   nStatus = -1; \
   break; \
  } \
  sal_udelay(1000); \
  nTimeCnt++; \
} \
}

#else /* VXWORKS */

#define SOC_SBX_UTIL_WAIT_FOR_ACK(unit, device, reg, nTimeoutInMs, nStatus) \
{ \
int32 nTimeCnt = 0;\
while (1) {\
  uint32 uDataWaitForAck = SAND_HAL_READ(unit, device, reg##_ACC_CTRL); \
  nStatus = 0;\
  if (SAND_HAL_GET_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){ \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0); \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1); \
    SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, uDataWaitForAck);\
    break; \
  }\
  if (nTimeCnt >= nTimeoutInMs){ \
   nStatus = -1; \
   break; \
  } \
  sal_usleep(1000); \
  nTimeCnt++; \
} \
}

#endif /* VXWORKS */

/* handle CA vs C2 differences */
#define SOC_SBX_UTIL_FE2000_WAIT_FOR_ACK(unit, reg, nTimeoutInMs, nStatus) \
    if (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) {\
        SOC_SBX_UTIL_WAIT_FOR_ACK(unit, C2, reg, nTimeoutInMs, nStatus) \
    }else{ \
        SOC_SBX_UTIL_WAIT_FOR_ACK(unit, CA, reg, nTimeoutInMs, nStatus) \
    }

#define SOC_SBX_UTIL_FE2000_WRITE_REQUEST(unit, reg, otherAddr)\
    (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) \
    ? SOC_SBX_UTIL_WRITE_REQUEST(unit, C2, reg, otherAddr)\
    : SOC_SBX_UTIL_WRITE_REQUEST(unit, CA, reg, otherAddr)

#define SOC_SBX_UTIL_FE2000_READ_REQUEST(unit, reg, otherAddr) \
    (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) \
    ? SOC_SBX_UTIL_READ_REQUEST(unit, C2, reg, otherAddr) \
    : SOC_SBX_UTIL_READ_REQUEST(unit, CA, reg, otherAddr)
#endif /* _SBX_UTIL_H_ */

