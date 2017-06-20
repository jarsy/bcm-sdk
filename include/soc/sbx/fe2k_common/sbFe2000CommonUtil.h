/* -*-  Mode:C; c-basic-offset:4 -*- */
/******************************************************************************
 *
 * $Id: sbFe2000CommonUtil.h,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 ******************************************************************************/
#ifndef _SB_FE_2000_COMMON_UTIL_H_
#define _SB_FE_2000_COMMON_UTIL_H_

#include "soc/sbx/sbTypes.h"
#include "soc/sbx/fe2k_common/sbFe2000Common.h"

/*
 * Generic indirect read function
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param raw              Flag to perform no integer interpretation
 * @param descending       Flag indicates data registers are in n->0 order
 * @param ctrlOffset       Offset of _ctrl register
 * @param address          Memory address to access
 * @param words            Number of (consecutive) _data registers for memory
 * @param data             Returns data read
 *
 * @return                 failure flag (TRUE == timeout)
 */
int
sbFe2000UtilReadIndir(sbhandle userDeviceHandle, int raw, int descending,
                      uint32 ctrlOffset, uint32 address, uint32 words,
                      uint32 *data);

/*
 * Generic indirect write function
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param raw              Flag to perform no integer interpretation
 * @param descending       Flag indicates data registers are in n->0 order
 * @param ctrlOffset       Offset of _ctrl register
 * @param address          Memory address to access
 * @param words            Number of (consecutive) _data registers for memory
 * @param data             Data to write
 *
 * @return                 failure flag (TRUE == timeout)
 */
int
sbFe2000UtilWriteIndir(sbhandle userDeviceHandle, int raw, int descending,
                       uint32 ctrlOffset, uint32 address, uint32 words,
                       uint32 *data);

/*
 * Write data to an integrated Tri-Speed MAC (port specific or general reg)
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param uData            Data to write.
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32
sbFe2000UtilAgmWrite(sbhandle userDeviceHandle, int32 nAgmNum, int32 nPort,
                 uint32 uRegAddr, uint32 uData);

/*
 * Read data from an integrated Tri-Speed MAC (port specific or general reg)
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to read
 * @param puData           Location in which to place read data.
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32
sbFe2000UtilAgmRead(sbhandle userDeviceHandle, int32 nAgmNum, int32 nPort,
                uint32 uRegAddr, uint32 *puData);

void 
sbFe2000UtilApplySoftReset(sbhandle userDeviceHandle);

void 
sbFe2000UtilReleaseSoftReset(sbhandle userDeviceHandle);

uint32
sbFe2000UtilAgmMiiWrite(sbhandle userDeviceHandle, uint8 bInternalPhy,uint32 uPhyAddr, uint32 uRegAddr, uint32 uData);

uint32
sbFe2000UtilAgmMiiRead(sbhandle userDeviceHandle, uint8 bInternalPhy, uint32 uPhyAddr, uint32 uRegAddr, uint32 *puData);

uint32
sbFe2000UtilAgmGportWrite(sbhandle userDeviceHandle, uint32 nAgmNum, uint32 uPort, 
		      uint32 uRegAddr, uint32 uData);
uint32
sbFe2000UtilAgmGportRead(sbhandle userDeviceHandle, uint32 nAgmNum, uint32 uPort, 
		     uint32 uRegAddr);
unsigned int
sbFe2000UtilIICRead(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int *data);

unsigned int
sbFe2000UtilIICWrite(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int data);

unsigned int
sbFe2000UtilPciIICRead(int unit, unsigned int uRegAddr, unsigned int *puData);

unsigned int
sbFe2000UtilPciIICWrite(int unit, unsigned int uRegAddr, unsigned int uData);

uint32 
sbFe2000UtilAgmMacErrorMaskCalcAddr(uint16 nNum, uint16 nPort,sbhandle userDeviceHandle);

void
sbFe2000UtilSetupCmSegment(  sbhandle userDeviceHandle, uint32 uCmSegment, uint32 uCntrType, uint32 uMmuDestBank, uint32 uEjectLocation,
			     uint32 uMmuBankBase, uint32 uLimit, uint32 uMemAddrConsumed );

/*
 * Write data to an integrated 10G MAC
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param uDataHi          Data to write (bits [63:32]).
 * @param uDataLo          Data to write (bits [31:0]).
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32
sbFe2000UtilXgmWrite(sbhandle userDeviceHandle, int32 nXgmNum, uint32 uRegAddr,
                 uint32 uDataHi, uint32 uDataLo);

/*
 * Read data from an integrated 10G MAC
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param puDataHi         Location in which to place read data (bits [63:32]).
 * @param puDataLo         Location in which to place read data (bits [31:0]).
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */
uint32
sbFe2000UtilXgmRead(sbhandle userDeviceHandle, int32 nXgmNum,
                uint32 uRegAddr, uint32 *puDataHi, uint32 *puDataLo);
uint32
sbFe2000UtilXgmBigMacRead(sbhandle userDeviceHandle,uint32 uXgmNum, uint32 uRegAddr,
		      uint32 *pDataHi, uint32 *pDataLo);
uint32
sbFe2000UtilXgmBigMacWrite(sbhandle userDeviceHandle,uint32 uXgmNum, uint32 uRegAddr, 
		       uint32 uDataHi, uint32 uDataLo);
uint32
sbFe2000UtilXgmMiimRead(sbhandle userDeviceHandle, uint8 bInternalPhy, uint8 bClause45,
			uint32 uDevAddr, uint32 uPhyOrPortAddr, uint32 uRegAddr,
			uint32 *pData);

uint32
sbFe2000UtilXgmMiimWrite(sbhandle userDeviceHandle,uint8 bInternalPhy, uint8 bClause45,
			 uint32 uDevAddr, uint32 uPhyOrPortAddr, uint32 uRegAddr, 
			 uint32 uData);

/* helper routines */
void CaSetDataWord(uint32 * pWord, uint8 *pDataBytes);
void CaSetDataBytes( uint32 uData, uint8 *pBytes);
#endif /* _SB_FE_2000_UTIL_H_ */
