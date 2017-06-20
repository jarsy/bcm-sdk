/*
 * $Id: sbFe2000UcodeLoad.h,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbFe2000UcodeLoad.h : FE2000 Ucode Functions
 *
 *-----------------------------------------------------------------------------*/

#ifndef _SB_FE2000_UCODE_LOAD_H_
#define _SB_FE2000_UCODE_LOAD_H_

#include "soc/sbx/sbTypes.h"
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include "soc/sbx/glue.h"
#include "soc/sbx/fe2k/sbFe2000Asm.h"

uint32 sbFe2000LrpInstructionMemoryRead(sbhandle userDeviceHandle,
					  UINT uAddress,
					  UINT *pData0,
					  UINT *pData1,
					  UINT *pData2);

uint32 sbFe2000LrpGetInstruction(sbhandle userDeviceHandle,
                                   int stream, int pc,
                                   UINT *pData0, UINT *pData1, UINT *pData2,
                                   int *task);

uint32 sbFe2000LrpInstructionMemoryWrite(sbhandle userDeviceHandle,
					   uint32 uAddress,
					   uint32 uData0,
					   uint32 uData1,
					   uint32 uData2);

uint32 sbFe2000UcodeLoad(sbhandle userDeviceHandle, uint8 *ucode);

uint32 sbFe2000BringUpLr(sbhandle userDeviceHandle, uint8 *ucode);

uint32 sbFe2000EnableLr(sbhandle userDeviceHandle, uint8 *ucode);

uint32 sbFe2000DisableLr(sbhandle userDeviceHandle);

uint32 sbFe2000ReloadUcode(sbhandle userDeviceHandle, uint8 *ucode);
uint32 sbFe2000GetNamedConstant(sbhandle userDeviceHandle,
                                  fe2kAsm2IntD *ucode, char * name, uint32 *val);
uint32 sbFe2000SetNamedConstant(sbhandle userDeviceHandle, fe2kAsm2IntD *ucode, char * name, uint32 val);

uint32 sbFe2000UcodeLoadFromBuffer(sbhandle userDeviceHandle,
				     fe2kAsm2IntD *a_p,
                                     unsigned char *a_b,
                                     unsigned int a_l);

int32 sbFe2000LrpInstructionMemoryWriteCallback(void *a_pv,
						  unsigned int a_sn, 
						  unsigned int a_in, 
						  unsigned char *a_n);

int32 sbFe2000LrpInstructionMemoryReadCallback(void *a_pv,
						 unsigned int a_sn, 
						 unsigned int a_in, 
						 unsigned char *a_n);

uint32 sbFe2000SwapInstructionMemoryBank(sbhandle userDeviceHandle);
#endif
