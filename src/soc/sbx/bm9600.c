/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: bm9600.c,v 1.35 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *   bm9600.c :   bm9600 driver
 *   This file contains Read/Write access routines for each of the Bm9600 tables
 *   It was derived from auto-generated code, so there's room for optimization
 *
 *-----------------------------------------------------------------------------*/


#include <shared/bsl.h>
#include <soc/cm.h>
#include <soc/debug.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600_soc_init.h>
#include "sbTypesGlue.h"
#include "glue.h"
#include "glue_dma.h"
#include "hal_user.h"
#include "bm9600.h"
#include "sbWrappers.h"
#include "sbZfFabBm9600InaSysportMapEntry.hx"

#ifdef BCM_BM9600_SUPPORT

#define printf  bsl_printf
#define BM9600_LOG_ERROR(format, arg1, arg2) printf(format, arg1, arg2)

static bm9600_NmCache_t *p_NmCache[SOC_MAX_NUM_DEVICES];
static bm9600_InaCache_t *pInaCache[SOC_MAX_NUM_DEVICES];

int
soc_bm9600_FoLinkStateTableRead(uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 *pData0,
                                uint32 *pData1)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ACK, nData);
    }
	
    if( !uAck ) {
	BM9600_LOG_ERROR("%s: Error reading memory for TableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData0 = SAND_HAL_READ(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA1);
    
    return SOC_E_NONE;
}

int
soc_bm9600_FoLinkStateTableWrite(uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 uData0,
                                 uint32 uData1)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA1, uData1);

    SAND_HAL_WRITE( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_FoLinkStateTableClear(uint32 uBaseAddress)
{
  uint32 uData[2];
  /*Init the values from the IMF or 0                                       */
  uData[0]=0;
  uData[1]=0;
  return soc_bm9600_FoLinkStateTableFillPattern(uBaseAddress, uData[0], uData[1]);
}
#endif /* FUNCTION_NOW_USED */

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_FoLinkStateTableFillPattern(uint32 uBaseAddress,
                                       uint32 uData0,
                                       uint32 uData1)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_DATA1, uData1);
    for (uAddress=0; (int)uAddress < (1<<7); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, FO_LINK_STATE_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, FO_LINK_STATE_MEM_ACC_CTRL,  nData);

    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_NmEgressRankerRead(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 *pData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 8) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    
    return SOC_E_NONE;
}

int
soc_bm9600_NmEgressRankerWrite(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0; 
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 8) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

int soc_bm9600_NmFullStatusRead(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 *pData0,
                              uint32 *pData1,
                              uint32 *pData2,
                              uint32 *pData3,
                              uint32 *pData4,
                              uint32 *pData5,
                              uint32 *pData6,
                              uint32 *pData7,
                              uint32 *pData8)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData;

    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 9) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData0 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA2);
    *pData3 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA3);
    *pData4 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA4);
    *pData5 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA5);
    *pData6 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA6);
    *pData7 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA7);
    *pData8 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA8);

    return SOC_E_NONE;
}

int
soc_bm9600_NmFullStatusWrite(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData0,
                               uint32 uData1,
                               uint32 uData2,
                               uint32 uData3,
                               uint32 uData4,
                               uint32 uData5,
                               uint32 uData6,
                               uint32 uData7,
                               uint32 uData8)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData;

    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA3, uData3);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA4, uData4);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA5, uData5);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA6, uData6);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA7, uData7);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA8, uData8);
  
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 9) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
	
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;

}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEgressRankerClear(uint32 uBaseAddress)
{
  uint32 uData[1];
  /*Init the values from the IMF or 0                                       */
  uData[0]=0;
  return soc_bm9600_NmEgressRankerFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_NmEgressRankerFillPattern(uint32 uBaseAddress,
                                     uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 8) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	
        nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/*
 * soc_bm9600_NmEmtRead - Read Eset Member Table
 */
int
soc_bm9600_NmEmtRead(uint32 uBaseAddress,
                     uint32 uAddress,
                     uint32 *pData0,
                     uint32 *pData1,
                     uint32 *pData2)
{
  int rc = SOC_E_NONE;


#ifndef _NO_NM_CACHE_FIX_
  rc = soc_bm9600_NmEmtCacheRead(uBaseAddress, uAddress, pData0, pData1, pData2);
#else /* _NO_NM_CACHE_FIX_ */
  rc = soc_bm9600_HwNmEmtRead(uBaseAddress, uAddress, pData0, pData1, pData2);
#endif /* !(_NO_NM_CACHE_FIX_ */
  return(rc);
}

int
soc_bm9600_HwNmEmtRead( uint32 uBaseAddress,
                           uint32 uAddress,
                           uint32 *pData0,
                           uint32 *pData1,
                           uint32 *pData2)
{
    int rc = SOC_E_NONE;
    int is_fo_disabled = TRUE;
    int fo_disable_workaround = TRUE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    if ( (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) &&
	 (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID) ) {
	fo_disable_workaround = FALSE;
    }

    if (fo_disable_workaround == TRUE) {
	is_fo_disabled = soc_bm9600_is_fo_disabled(uBaseAddress);
	
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_disable_fo(uBaseAddress);
	}
    }
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 3) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);


    if ( (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) &&
	 (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID) ) {
	fo_disable_workaround = FALSE;
    }
    
    if (fo_disable_workaround == TRUE) {
	is_fo_disabled = soc_bm9600_is_fo_disabled(uBaseAddress);
	
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_disable_fo(uBaseAddress);
	}
    }
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 3) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	rc = SOC_E_INTERNAL;
	goto err;
    }
    
    *pData0 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA2);
    
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
		soc_bm9600_enable_fo(uBaseAddress);
	}
    }

    return SOC_E_NONE;
    
err:
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	    }
    }
    
    return(rc);
}

/*
 * soc_bm9600_NmEmtWrite - Write Eset Member Table
 */
int
soc_bm9600_NmEmtWrite( uint32 uBaseAddress,
                            uint32 uAddress,
                            uint32 uData0,
                            uint32 uData1,
                            uint32 uData2)
{
  int rc = SOC_E_NONE;

  if (  (SOC_SBX_CFG_BM9600(uBaseAddress)->uDeviceMode==SOC_SBX_BME_XBAR_MODE) &&
        ((SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) || (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID))  ) {
      return rc;
  }
  rc = soc_bm9600_HwNmEmtWrite(uBaseAddress, uAddress, uData0, uData1, uData2);
  if (rc != SOC_E_NONE) {
      return(rc);
  }

  /* check if a backup device is setup. This will be only setup if Arbiter feature */
  /* is present.                                                                   */
  if ((SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != -1) &&
                      (SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != uBaseAddress)) {
    if (!SOC_IS_RELOADING(uBaseAddress)) {
      rc = soc_bm9600_HwNmEmtWrite(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit,
                                                   uAddress, uData0, uData1, uData2);
    }
  }

  return(rc);
}

int
soc_bm9600_HwNmEmtWrite(uint32 uBaseAddress,
                      uint32 uAddress,
                      uint32 uData0,
                      uint32 uData1,
                      uint32 uData2)
{
    int rc = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 3) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    
#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmEmtCacheWrite(uBaseAddress, uAddress, uData0, uData1, uData2);
#endif /* _NO_NM_CACHE_FIX_ */
    return(rc);
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtClear(uint32 uBaseAddress)
{
    uint32 uData[3];
    /*Init the values from the IMF or 0                                       */
    uData[0]=0;
    uData[1]=0;
    uData[2]=0;
    return soc_bm9600_NmEmtFillPattern(uBaseAddress, uData[0], uData[1], uData[2]);
}

int
soc_bm9600_NmEmtFillPattern(uint32 uBaseAddress,
                            uint32 uData0,
                            uint32 uData1,
                            uint32 uData2)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    for (uAddress = 0; (int)uAddress < (1 << 10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 3) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_NmEmtdebugbank0Read(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 *pData0,
                               uint32 *pData1,
                               uint32 *pData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL);
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData0 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA2);
    
    return SOC_E_NONE;
}

int
soc_bm9600_NmEmtdebugbank0Write(uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 uData0,
                                uint32 uData1,
                                uint32 uData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtdebugbank0Clear(uint32 uBaseAddress)
{
    uint32 uData[3];
    uData[0]=0;
    uData[1]=0;
    uData[2]=0;
    return soc_bm9600_NmEmtdebugbank0FillPattern(uBaseAddress, uData[0], uData[1], uData[2]);
}

int
soc_bm9600_NmEmtdebugbank0FillPattern(uint32 uBaseAddress,
                                      uint32 uData0,
                                      uint32 uData1,
                                      uint32 uData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    uint32 uAddress;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    
    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_NmEmtdebugbank1Read(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 *pData0,
                               uint32 *pData1,
                               uint32 *pData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 2) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
                              );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData0 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA2);
    
    return SOC_E_NONE;
}

int
soc_bm9600_NmEmtdebugbank1Write(uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 uData0,
                                uint32 uData1,
                                uint32 uData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 2) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	    

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
        uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtdebugbank1Clear(uint32 uBaseAddress)
{
    uint32 uData[3];
    uData[0]=0;
    uData[1]=0;
    uData[2]=0;
    return soc_bm9600_NmEmtdebugbank1FillPattern(uBaseAddress, uData[0], uData[1], uData[2]);
}

int
soc_bm9600_NmEmtdebugbank1FillPattern(uint32 uBaseAddress,
                                      uint32 uData0,
                                      uint32 uData1,
                                      uint32 uData2)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);

    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {

	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 2) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    nData = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	    
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
	
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_NmIngressRankerRead(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 *pData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 7) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);

    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_NmIngressRankerWrite(uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 uData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 7) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmIngressRankerClear(uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_NmIngressRankerFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_NmIngressRankerFillPattern(uint32 uBaseAddress,
                                      uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
 
    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 7) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/* For easy reload, this function must be read */
int
soc_bm9600_NmPortsetInfoRead(uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 *pData)
{
  int rc = SOC_E_NONE;

  
#ifndef _NO_NM_CACHE_FIX_
  rc = soc_bm9600_NmPortsetInfoCacheRead(uBaseAddress, uAddress, pData);
#else /* _NO_NM_CACHE_FIX_ */
  rc = soc_bm9600_HwNmPortsetInfoRead(uBaseAddress, uAddress, pData);
#endif /* !(_NO_NM_CACHE_FIX_) */
  return(rc);
}

int
soc_bm9600_HwNmPortsetInfoRead(uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 *pData)
{
    int rc = SOC_E_NONE;
    int is_fo_disabled = TRUE;
    int fo_disable_workaround = TRUE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    if ( (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) &&
	 (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID) ) {
	fo_disable_workaround = FALSE;
    }
    
    if (fo_disable_workaround == TRUE) {
	is_fo_disabled = soc_bm9600_is_fo_disabled(uBaseAddress);
	
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_disable_fo(uBaseAddress);
	}
    }
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 0) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	rc = SOC_E_INTERNAL;
	goto err;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);

    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }
    
    return SOC_E_NONE;

 err:
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }
    
    return(rc);
}

int
soc_bm9600_NmPortsetInfoWrite(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 uData)
{
  int rc = SOC_E_NONE;

  if (  (SOC_SBX_CFG_BM9600(uBaseAddress)->uDeviceMode==SOC_SBX_BME_XBAR_MODE) &&
        ((SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) || (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID))  ) {
      return rc;
  }

  rc = soc_bm9600_HwNmPortsetInfoWrite(uBaseAddress, uAddress, uData);
  if (rc != SOC_E_NONE) {
    return(rc);
  }

  /* check if a backup device is setup. This will be only setup if Arbiter feature */
  /* is present.                                                                   */
  if ((SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != -1) &&
                      (SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != uBaseAddress)) {
    if (!SOC_IS_RELOADING(uBaseAddress)) {
      rc = soc_bm9600_HwNmPortsetInfoWrite(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit,
                                                                           uAddress, uData);
    }
  }

  return(rc);
}

int
soc_bm9600_HwNmPortsetInfoWrite(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 uData)
{

    int rc = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmPortsetInfoCacheWrite(uBaseAddress, uAddress, uData);
#endif /* _NO_NM_CACHE_FIX_ */
    return(rc);
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmPortsetInfoClear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_NmPortsetInfoFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_NmPortsetInfoFillPattern(uint32 uBaseAddress,
                                    uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/* For easy reload, this table read must read from the hardware */
int
soc_bm9600_NmPortsetLinkRead(uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 *pData)
{
    int rc = SOC_E_NONE;


#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmPortsetLinkCacheRead(uBaseAddress, uAddress, pData);
#else /* _NO_NM_CACHE_FIX_ */
    rc = soc_bm9600_HwNmPortsetLinkRead(uBaseAddress, uAddress, pData);
#endif /* !(_NO_NM_CACHE_FIX_) */
    return(rc);
}

int
soc_bm9600_HwNmPortsetLinkRead(uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 *pData)
{
    int rc = SOC_E_NONE;
    int is_fo_disabled = TRUE;
    int fo_disable_workaround = TRUE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    if ( (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) &&
	 (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID) ) {
	fo_disable_workaround = FALSE;
    }

    if (fo_disable_workaround == TRUE) {
	is_fo_disabled = soc_bm9600_is_fo_disabled(uBaseAddress);
	
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_disable_fo(uBaseAddress);
	}
    }
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 5) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	rc = SOC_E_INTERNAL;
	goto err;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);

    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }
    
    return SOC_E_NONE;

 err:
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }
    
    return(rc);
}

int
soc_bm9600_NmPortsetLinkWrite(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 uData)
{
  int rc = SOC_E_NONE;

  if (  (SOC_SBX_CFG_BM9600(uBaseAddress)->uDeviceMode==SOC_SBX_BME_XBAR_MODE) &&
        ((SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) || (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID))  ) {
      return rc;
  }

    rc = soc_bm9600_HwNmPortsetLinkWrite(uBaseAddress, uAddress, uData);
    if (rc != SOC_E_NONE) {
	return(rc);
    }
    
    /* check if a backup device is setup. This will be only setup if Arbiter feature */
    /* is present.                                                                   */
    if ((SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != -1) &&
	(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != uBaseAddress)) {
	if (!SOC_IS_RELOADING(uBaseAddress)) {
	    rc = soc_bm9600_HwNmPortsetLinkWrite(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit,
                                                                             uAddress, uData);
	}
    }
    
    return(rc);
}

int
soc_bm9600_HwNmPortsetLinkWrite(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 uData)
{
    int rc = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 5) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmPortsetLinkCacheWrite(uBaseAddress, uAddress, uData);
#endif /* _NO_NM_CACHE_FIX_ */
    return(rc);
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmPortsetLinkClear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_NmPortsetLinkFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_NmPortsetLinkFillPattern(uint32 uBaseAddress,
                                    uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);

    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 5) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_NmRandomNumGenRead(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 *pData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 6) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_NmRandomNumGenWrite(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 6) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmRandomNumGenClear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_NmRandomNumGenFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_NmRandomNumGenFillPattern(uint32 uBaseAddress,
                                     uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData);

    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 6) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/* For easy reload, this function is required to actually read the indirect memory */
int
soc_bm9600_NmSysportArrayRead(uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 *pData0,
                              uint32 *pData1,
                              uint32 *pData2,
                              uint32 *pData3,
                              uint32 *pData4,
                              uint32 *pData5)
{
    int rc = SOC_E_NONE;


#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmSysportArrayCacheRead(uBaseAddress, uAddress,
					    pData0, pData1, pData2, pData3, pData4, pData5);
#else /* _NO_NM_CACHE_FIX_ */
    rc = soc_bm9600_HwNmSysportArrayRead(uBaseAddress, uAddress,
                                          pData0, pData1, pData2, pData3, pData4, pData5);
#endif /* !(_NO_NM_CACHE_FIX_) */
    return(rc);
}

int
soc_bm9600_HwNmSysportArrayRead(uint32 uBaseAddress,
				uint32 uAddress,
				uint32 *pData0,
				uint32 *pData1,
				uint32 *pData2,
				uint32 *pData3,
				uint32 *pData4,
				uint32 *pData5)
{
    int rc = SOC_E_NONE;
    int is_fo_disabled = TRUE;
    int fo_disable_workaround = TRUE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    

    if ( (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) &&
	 (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID) ) {
	fo_disable_workaround = FALSE;
    }
    
    if (fo_disable_workaround == TRUE) {
	is_fo_disabled = soc_bm9600_is_fo_disabled(uBaseAddress);
	
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_disable_fo(uBaseAddress);
	}
    }
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, NM_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 4) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
                                );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	rc = SOC_E_INTERNAL;
	goto err;
    }

    (*pData0) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA0);
    (*pData1) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA1);
    (*pData2) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA2);
    (*pData3) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA3);
    (*pData4) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA4);
    (*pData5) = SAND_HAL_READ(uBaseAddress, PL, NM_MEM_ACC_DATA5);
    
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }

    return(rc);

err:
    if (fo_disable_workaround == TRUE) {
	if (is_fo_disabled == FALSE) {
	    soc_bm9600_enable_fo(uBaseAddress);
	}
    }
    
    return(rc);
}

int
soc_bm9600_NmSysportArrayWrite(uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData0,
                               uint32 uData1,
                               uint32 uData2,
                               uint32 uData3,
                               uint32 uData4,
                               uint32 uData5)
{
    int rc = SOC_E_NONE;

  if (  (SOC_SBX_CFG_BM9600(uBaseAddress)->uDeviceMode==SOC_SBX_BME_XBAR_MODE) &&
        ((SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) || (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID))  ) {
      return rc;
  }
    rc = soc_bm9600_HwNmSysportArrayWrite(uBaseAddress, uAddress, uData0,
					  uData1, uData2, uData3, uData4, uData5);
    if (rc != SOC_E_NONE) {
	return(rc);
    }

    /* check if a backup device is setup. This will be only setup if Arbiter feature */
    /* is present.                                                                   */
    if ((SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != -1) &&
	(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit != uBaseAddress)) {
	if (!SOC_IS_RELOADING(uBaseAddress)) {
	    rc = soc_bm9600_HwNmSysportArrayWrite(SOC_SBX_CFG_BM9600(uBaseAddress)->BackupDeviceUnit,
						  uAddress, uData0, uData1, uData2, uData3, uData4, uData5);
	}
    }
    
    return(rc);
}

int
soc_bm9600_HwNmSysportArrayWrite(uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uData0,
				 uint32 uData1,
				 uint32 uData2,
				 uint32 uData3,
				 uint32 uData4,
				 uint32 uData5)
{
    int rc = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    /* write to it */
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA3, uData3);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA4, uData4);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA5, uData5);
    
    SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
                    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,SELECT, 4) |
                    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,REQ, 1) |
                    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, 1) |
                    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,RD_WR_N, 0) |
                    SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL,ADDRESS, uAddress)
                    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

#ifndef _NO_NM_CACHE_FIX_
    rc = soc_bm9600_NmSysportArrayCacheWrite(uBaseAddress, uAddress,
					     uData0, uData1, uData2, uData3, uData4, uData5);
#endif /* _NO_NM_CACHE_FIX_ */
    return(rc);
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmSysportArrayClear(uint32 uBaseAddress)
{
  uint32 uData[6];
  uData[0]=0;
  uData[1]=0;
  uData[2]=0;
  uData[3]=0;
  uData[4]=0;
  uData[5]=0;
  return soc_bm9600_NmSysportArrayFillPattern(uBaseAddress, uData[0], uData[1], uData[2], uData[3], uData[4], uData[5]);
}

int
soc_bm9600_NmSysportArrayFillPattern(uint32 uBaseAddress,
                                     uint32 uData0,
                                     uint32 uData1,
                                     uint32 uData2,
                                     uint32 uData3,
                                     uint32 uData4,
                                     uint32 uData5)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA3, uData3);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA4, uData4);
    SAND_HAL_WRITE(uBaseAddress, PL, NM_MEM_ACC_DATA5, uData5);
    for (uAddress=0; (int)uAddress < (1<<10); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, SELECT, 4) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, NM_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, NM_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, NM_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, NM_MEM_ACC_CTRL,  nData);
	
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapRead(uint32 uBaseAddress,
                           uint32 uAddress,
                           uint32 *pData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,SELECT, 0xff) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0xff Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_XbXcfgRemapWrite(uint32 uBaseAddress,
                            uint32 uAddress,
                            uint32 uData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,SELECT, 0xff) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
  
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, nData);
    }
    
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0xff Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

int
soc_bm9600_XbXcfgRemapClear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapFillPattern(uint32 uBaseAddress,
                                  uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<7); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, SELECT, 0xff) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0xff Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
        nData = SAND_HAL_MOD_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,  nData);

    }
    return SOC_E_NONE;
}

#endif /* FUNCTION_NOW_USED */

/* MCM base call for RemapSelectRead calls
*/
int
soc_bm9600_XbXcfgRemapSelectRead(uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 uSelect,
                                 uint32 *pData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,SELECT, uSelect) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA);

  return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0Read(uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,0,pData);
}
#endif /* FUNCTION_NOW_USED */

/*
 * MCM base call for RemapSelectWrite  calls
 */
int
soc_bm9600_XbXcfgRemapSelectWrite(uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 uSelect,
                                  uint32 uData)
{
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,SELECT, uSelect) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL,ACK, nData);
	
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

int
soc_bm9600_XbXcfgRemapSelect0Write(uint32 uBaseAddress,
                                   uint32 uAddress,
                                   uint32 uData)
{
    return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,0,uData);
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0Clear(uint32 uBaseAddress)
{
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_XbXcfgRemapSelect0FillPattern(uBaseAddress, uData[0]);
}

/*
 * MCM base call for RemapSelectFillPattern
 */
int
soc_bm9600_XbXcfgRemapSelectFillPattern(uint32 uBaseAddress,
                                        uint32 uSelect,
                                        uint32 uData)
{
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<7); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, SELECT, uSelect) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
        nData = SAND_HAL_MOD_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, XB_XCFG_REMAP_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, XB_XCFG_REMAP_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0FillPattern(uint32 uBaseAddress,
                                         uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,0,uData);
}

int
soc_bm9600_XbXcfgRemapSelect1Read(uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,1,pData);
}

int
soc_bm9600_XbXcfgRemapSelect1Write(uint32 uBaseAddress,
                                   uint32 uAddress,
                                   uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,1,uData);
}

int
soc_bm9600_XbXcfgRemapSelect1Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect1FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect1FillPattern(uint32 uBaseAddress,
                                         uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,1,uData);
}

int
soc_bm9600_XbXcfgRemapSelect10Read(uint32 uBaseAddress,
                                   uint32 uAddress,
                                   uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,10,pData);
}

int
soc_bm9600_XbXcfgRemapSelect10Write(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,10,uData);
}


int
soc_bm9600_XbXcfgRemapSelect100Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,100,pData);
}

int
soc_bm9600_XbXcfgRemapSelect100Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,100,uData);
}

int
soc_bm9600_XbXcfgRemapSelect100Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect100FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect100FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,100,uData);
}

int
soc_bm9600_XbXcfgRemapSelect101Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,101,pData);
}

int
soc_bm9600_XbXcfgRemapSelect101Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,101,uData);
}

int
soc_bm9600_XbXcfgRemapSelect101Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect101FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect101FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,101,uData);
}

int
soc_bm9600_XbXcfgRemapSelect102Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,102,pData);
}

int
soc_bm9600_XbXcfgRemapSelect102Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,102,uData);
}

int
soc_bm9600_XbXcfgRemapSelect102Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect102FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect102FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,102,uData);
}

int
soc_bm9600_XbXcfgRemapSelect103Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,103,pData);
}

int
soc_bm9600_XbXcfgRemapSelect103Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,103,uData);
}

int
soc_bm9600_XbXcfgRemapSelect103Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect103FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect103FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,103,uData);
}

int
soc_bm9600_XbXcfgRemapSelect104Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,104,pData);
}

int
soc_bm9600_XbXcfgRemapSelect104Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,104,uData);
}

int
soc_bm9600_XbXcfgRemapSelect104Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect104FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect104FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,104,uData);
}

int
soc_bm9600_XbXcfgRemapSelect105Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,105,pData);
}

int
soc_bm9600_XbXcfgRemapSelect105Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,105,uData);
}

int
soc_bm9600_XbXcfgRemapSelect105Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect105FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect105FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,105,uData);
}

int
soc_bm9600_XbXcfgRemapSelect106Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,106,pData);
}

int
soc_bm9600_XbXcfgRemapSelect106Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,106,uData);
}

int
soc_bm9600_XbXcfgRemapSelect106Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect106FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect106FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,106,uData);
}

int
soc_bm9600_XbXcfgRemapSelect107Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,107,pData);
}

int
soc_bm9600_XbXcfgRemapSelect107Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,107,uData);
}

int
soc_bm9600_XbXcfgRemapSelect107Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect107FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect107FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,107,uData);
}

int
soc_bm9600_XbXcfgRemapSelect108Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,108,pData);
}

int
soc_bm9600_XbXcfgRemapSelect108Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,108,uData);
}

int
soc_bm9600_XbXcfgRemapSelect108Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect108FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect108FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,108,uData);
}

int
soc_bm9600_XbXcfgRemapSelect109Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,109,pData);
}

int
soc_bm9600_XbXcfgRemapSelect109Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,109,uData);
}

int
soc_bm9600_XbXcfgRemapSelect109Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect109FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect109FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,109,uData);
}

int
soc_bm9600_XbXcfgRemapSelect11Read(uint32 uBaseAddress,
                                   uint32 uAddress,
                                   uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,11,pData);
}

int
soc_bm9600_XbXcfgRemapSelect11Write(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,11,uData);
}

int
soc_bm9600_XbXcfgRemapSelect11Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect11FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect11FillPattern(uint32 uBaseAddress,
                                          uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,11,uData);
}

int
soc_bm9600_XbXcfgRemapSelect110Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,110,pData);
}

int
soc_bm9600_XbXcfgRemapSelect110Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,110,uData);
}

int
soc_bm9600_XbXcfgRemapSelect110Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect110FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect110FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,110,uData);
}

int
soc_bm9600_XbXcfgRemapSelect111Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,111,pData);
}

int
soc_bm9600_XbXcfgRemapSelect111Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,111,uData);
}

int
soc_bm9600_XbXcfgRemapSelect111Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect111FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect111FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,111,uData);
}

int
soc_bm9600_XbXcfgRemapSelect112Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,112,pData);
}

int
soc_bm9600_XbXcfgRemapSelect112Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,112,uData);
}

int
soc_bm9600_XbXcfgRemapSelect112Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect112FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect112FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,112,uData);
}

int
soc_bm9600_XbXcfgRemapSelect113Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,113,pData);
}

int
soc_bm9600_XbXcfgRemapSelect113Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,113,uData);
}

int
soc_bm9600_XbXcfgRemapSelect113Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect113FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect113FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,113,uData);
}

int
soc_bm9600_XbXcfgRemapSelect114Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,114,pData);
}

int
soc_bm9600_XbXcfgRemapSelect114Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,114,uData);
}

int
soc_bm9600_XbXcfgRemapSelect114Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect114FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect114FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,114,uData);
}

int
soc_bm9600_XbXcfgRemapSelect115Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,115,pData);
}

int
soc_bm9600_XbXcfgRemapSelect115Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,115,uData);
}

int
soc_bm9600_XbXcfgRemapSelect115Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect115FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect115FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,115,uData);
}

int
soc_bm9600_XbXcfgRemapSelect116Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,116,pData);
}

int
soc_bm9600_XbXcfgRemapSelect116Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,116,uData);
}

int
soc_bm9600_XbXcfgRemapSelect116Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect116FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect116FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,116,uData);
}

int
soc_bm9600_XbXcfgRemapSelect117Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,117,pData);
}

int
soc_bm9600_XbXcfgRemapSelect117Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,117,uData);
}

int
soc_bm9600_XbXcfgRemapSelect117Clear(uint32 uBaseAddress)
{
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect117FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect117FillPattern(uint32 uBaseAddress,
                                           uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,117,uData);
}

int
soc_bm9600_XbXcfgRemapSelect118Read(uint32 uBaseAddress,
                                    uint32 uAddress,
                                    uint32 *pData)
{
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,118,pData);
}

int
soc_bm9600_XbXcfgRemapSelect118Write(uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uData)
{
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,118,uData);
}

int
soc_bm9600_XbXcfgRemapSelect118Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect118FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect118FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,118,uData);
}

int
soc_bm9600_XbXcfgRemapSelect119Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,119,pData);
}

int
soc_bm9600_XbXcfgRemapSelect119Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,119,uData);
}

int
soc_bm9600_XbXcfgRemapSelect119Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect119FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect119FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,119,uData);
}

int
soc_bm9600_XbXcfgRemapSelect12Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,12,pData);
}

int
soc_bm9600_XbXcfgRemapSelect12Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,12,uData);
}

int
soc_bm9600_XbXcfgRemapSelect12Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect12FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect12FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,12,uData);
}

int
soc_bm9600_XbXcfgRemapSelect120Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,120,pData);
}

int
soc_bm9600_XbXcfgRemapSelect120Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,120,uData);
}

int
soc_bm9600_XbXcfgRemapSelect120Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect120FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect120FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,120,uData);
}

int
soc_bm9600_XbXcfgRemapSelect121Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,121,pData);
}

int
soc_bm9600_XbXcfgRemapSelect121Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,121,uData);
}

int
soc_bm9600_XbXcfgRemapSelect121Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect121FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect121FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,121,uData);
}

int
soc_bm9600_XbXcfgRemapSelect122Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,122,pData);
}

int
soc_bm9600_XbXcfgRemapSelect122Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,122,uData);
}

int
soc_bm9600_XbXcfgRemapSelect122Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect122FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect122FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,122,uData);
}

int
soc_bm9600_XbXcfgRemapSelect123Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,123,pData);
}

int
soc_bm9600_XbXcfgRemapSelect123Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,123,uData);
}

int
soc_bm9600_XbXcfgRemapSelect123Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect123FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect123FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,123,uData);
}

int
soc_bm9600_XbXcfgRemapSelect124Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,124,pData);
}

int
soc_bm9600_XbXcfgRemapSelect124Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,124,uData);
}

int
soc_bm9600_XbXcfgRemapSelect124Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect124FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect124FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,124,uData);
}

int
soc_bm9600_XbXcfgRemapSelect125Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,125,pData);
}

int
soc_bm9600_XbXcfgRemapSelect125Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,125,uData);
}

int
soc_bm9600_XbXcfgRemapSelect125Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect125FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect125FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,125,uData);
}

int
soc_bm9600_XbXcfgRemapSelect126Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,126,pData);
}

int
soc_bm9600_XbXcfgRemapSelect126Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,126,uData);
}

int
soc_bm9600_XbXcfgRemapSelect126Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect126FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect126FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,126,uData);
}

int
soc_bm9600_XbXcfgRemapSelect127Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,127,pData);
}

int
soc_bm9600_XbXcfgRemapSelect127Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,127,uData);
}

int
soc_bm9600_XbXcfgRemapSelect127Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect127FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect127FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,127,uData);
}

int
soc_bm9600_XbXcfgRemapSelect128Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,128,pData);
}

int
soc_bm9600_XbXcfgRemapSelect128Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,128,uData);
}

int
soc_bm9600_XbXcfgRemapSelect128Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect128FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect128FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,128,uData);
}

int
soc_bm9600_XbXcfgRemapSelect129Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,129,pData);
}

int
soc_bm9600_XbXcfgRemapSelect129Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,129,uData);
}

int
soc_bm9600_XbXcfgRemapSelect129Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect129FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect129FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,129,uData);
}

int
soc_bm9600_XbXcfgRemapSelect13Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,13,pData);
}

int
soc_bm9600_XbXcfgRemapSelect13Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,13,uData);
}

int
soc_bm9600_XbXcfgRemapSelect13Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect13FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect13FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,13,uData);
}

int
soc_bm9600_XbXcfgRemapSelect130Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,130,pData);
}

int
soc_bm9600_XbXcfgRemapSelect130Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,130,uData);
}

int
soc_bm9600_XbXcfgRemapSelect130Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect130FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect130FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,130,uData);
}

int
soc_bm9600_XbXcfgRemapSelect131Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,131,pData);
}

int
soc_bm9600_XbXcfgRemapSelect131Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,131,uData);
}

int
soc_bm9600_XbXcfgRemapSelect131Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect131FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect131FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,131,uData);
}

int
soc_bm9600_XbXcfgRemapSelect132Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,132,pData);
}

int
soc_bm9600_XbXcfgRemapSelect132Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,132,uData);
}

int
soc_bm9600_XbXcfgRemapSelect132Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect132FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect132FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,132,uData);
}

int
soc_bm9600_XbXcfgRemapSelect133Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,133,pData);
}

int
soc_bm9600_XbXcfgRemapSelect133Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,133,uData);
}

int
soc_bm9600_XbXcfgRemapSelect133Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect133FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect133FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,133,uData);
}

int
soc_bm9600_XbXcfgRemapSelect134Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,134,pData);
}

int
soc_bm9600_XbXcfgRemapSelect134Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,134,uData);
}

int
soc_bm9600_XbXcfgRemapSelect134Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect134FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect134FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,134,uData);
}

int
soc_bm9600_XbXcfgRemapSelect135Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,135,pData);
}

int
soc_bm9600_XbXcfgRemapSelect135Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,135,uData);
}

int
soc_bm9600_XbXcfgRemapSelect135Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect135FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect135FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,135,uData);
}

int
soc_bm9600_XbXcfgRemapSelect136Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,136,pData);
}

int
soc_bm9600_XbXcfgRemapSelect136Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,136,uData);
}

int
soc_bm9600_XbXcfgRemapSelect136Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect136FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect136FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,136,uData);
}

int
soc_bm9600_XbXcfgRemapSelect137Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,137,pData);
}

int
soc_bm9600_XbXcfgRemapSelect137Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,137,uData);
}

int
soc_bm9600_XbXcfgRemapSelect137Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect137FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect137FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,137,uData);
}

int
soc_bm9600_XbXcfgRemapSelect138Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,138,pData);
}

int
soc_bm9600_XbXcfgRemapSelect138Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,138,uData);
}

int
soc_bm9600_XbXcfgRemapSelect138Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect138FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect138FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,138,uData);
}

int
soc_bm9600_XbXcfgRemapSelect139Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,139,pData);
}

int
soc_bm9600_XbXcfgRemapSelect139Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,139,uData);
}

int
soc_bm9600_XbXcfgRemapSelect139Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect139FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect139FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,139,uData);
}

int
soc_bm9600_XbXcfgRemapSelect14Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,14,pData);
}

int
soc_bm9600_XbXcfgRemapSelect14Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,14,uData);
}

int
soc_bm9600_XbXcfgRemapSelect14Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect14FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect14FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,14,uData);
}

int
soc_bm9600_XbXcfgRemapSelect140Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,140,pData);
}

int
soc_bm9600_XbXcfgRemapSelect140Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,140,uData);
}

int
soc_bm9600_XbXcfgRemapSelect140Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect140FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect140FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,140,uData);
}

int
soc_bm9600_XbXcfgRemapSelect141Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,141,pData);
}

int
soc_bm9600_XbXcfgRemapSelect141Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,141,uData);
}

int
soc_bm9600_XbXcfgRemapSelect141Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect141FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect141FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,141,uData);
}

int
soc_bm9600_XbXcfgRemapSelect142Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,142,pData);
}

int
soc_bm9600_XbXcfgRemapSelect142Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,142,uData);
}

int
soc_bm9600_XbXcfgRemapSelect142Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect142FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect142FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,142,uData);
}

int
soc_bm9600_XbXcfgRemapSelect143Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,143,pData);
}

int
soc_bm9600_XbXcfgRemapSelect143Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,143,uData);
}

int
soc_bm9600_XbXcfgRemapSelect143Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect143FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect143FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,143,uData);
}

int
soc_bm9600_XbXcfgRemapSelect144Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,144,pData);
}

int
soc_bm9600_XbXcfgRemapSelect144Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,144,uData);
}

int
soc_bm9600_XbXcfgRemapSelect144Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect144FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect144FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,144,uData);
}

int
soc_bm9600_XbXcfgRemapSelect145Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,145,pData);
}

int
soc_bm9600_XbXcfgRemapSelect145Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,145,uData);
}

int
soc_bm9600_XbXcfgRemapSelect145Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect145FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect145FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,145,uData);
}

int
soc_bm9600_XbXcfgRemapSelect146Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,146,pData);
}

int
soc_bm9600_XbXcfgRemapSelect146Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,146,uData);
}

int
soc_bm9600_XbXcfgRemapSelect146Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect146FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect146FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,146,uData);
}

int
soc_bm9600_XbXcfgRemapSelect147Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,147,pData);
}

int
soc_bm9600_XbXcfgRemapSelect147Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,147,uData);
}

int
soc_bm9600_XbXcfgRemapSelect147Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect147FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect147FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,147,uData);
}

int
soc_bm9600_XbXcfgRemapSelect148Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,148,pData);
}

int
soc_bm9600_XbXcfgRemapSelect148Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,148,uData);
}

int
soc_bm9600_XbXcfgRemapSelect148Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect148FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect148FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,148,uData);
}

int
soc_bm9600_XbXcfgRemapSelect149Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,149,pData);
}

int
soc_bm9600_XbXcfgRemapSelect149Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,149,uData);
}

int
soc_bm9600_XbXcfgRemapSelect149Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect149FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect149FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,149,uData);
}

int
soc_bm9600_XbXcfgRemapSelect15Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,15,pData);
}

int
soc_bm9600_XbXcfgRemapSelect15Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,15,uData);
}

int
soc_bm9600_XbXcfgRemapSelect15Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect15FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect15FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,15,uData);
}

int
soc_bm9600_XbXcfgRemapSelect150Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,150,pData);
}

int
soc_bm9600_XbXcfgRemapSelect150Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,150,uData);
}

int
soc_bm9600_XbXcfgRemapSelect150Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect150FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect150FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,150,uData);
}

int
soc_bm9600_XbXcfgRemapSelect151Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,151,pData);
}

int
soc_bm9600_XbXcfgRemapSelect151Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,151,uData);
}

int
soc_bm9600_XbXcfgRemapSelect151Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect151FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect151FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,151,uData);
}

int
soc_bm9600_XbXcfgRemapSelect152Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,152,pData);
}

int
soc_bm9600_XbXcfgRemapSelect152Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,152,uData);
}

int
soc_bm9600_XbXcfgRemapSelect152Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect152FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect152FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,152,uData);
}

int
soc_bm9600_XbXcfgRemapSelect153Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,153,pData);
}

int
soc_bm9600_XbXcfgRemapSelect153Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,153,uData);
}

int
soc_bm9600_XbXcfgRemapSelect153Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect153FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect153FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,153,uData);
}

int
soc_bm9600_XbXcfgRemapSelect154Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,154,pData);
}

int
soc_bm9600_XbXcfgRemapSelect154Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,154,uData);
}

int
soc_bm9600_XbXcfgRemapSelect154Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect154FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect154FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,154,uData);
}

int
soc_bm9600_XbXcfgRemapSelect155Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,155,pData);
}

int
soc_bm9600_XbXcfgRemapSelect155Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,155,uData);
}

int
soc_bm9600_XbXcfgRemapSelect155Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect155FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect155FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,155,uData);
}

int
soc_bm9600_XbXcfgRemapSelect156Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,156,pData);
}

int
soc_bm9600_XbXcfgRemapSelect156Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,156,uData);
}

int
soc_bm9600_XbXcfgRemapSelect156Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect156FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect156FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,156,uData);
}

int
soc_bm9600_XbXcfgRemapSelect157Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,157,pData);
}

int
soc_bm9600_XbXcfgRemapSelect157Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,157,uData);
}

int
soc_bm9600_XbXcfgRemapSelect157Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect157FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect157FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,157,uData);
}

int
soc_bm9600_XbXcfgRemapSelect158Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,158,pData);
}

int
soc_bm9600_XbXcfgRemapSelect158Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,158,uData);
}

int
soc_bm9600_XbXcfgRemapSelect158Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect158FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect158FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,158,uData);
}

int
soc_bm9600_XbXcfgRemapSelect159Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,159,pData);
}

int
soc_bm9600_XbXcfgRemapSelect159Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,159,uData);
}

int
soc_bm9600_XbXcfgRemapSelect159Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect159FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect159FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,159,uData);
}

int
soc_bm9600_XbXcfgRemapSelect16Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,16,pData);
}

int
soc_bm9600_XbXcfgRemapSelect16Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,16,uData);
}

int
soc_bm9600_XbXcfgRemapSelect16Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect16FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect16FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,16,uData);
}

int
soc_bm9600_XbXcfgRemapSelect160Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,160,pData);
}

int
soc_bm9600_XbXcfgRemapSelect160Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,160,uData);
}

int
soc_bm9600_XbXcfgRemapSelect160Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect160FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect160FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,160,uData);
}

int
soc_bm9600_XbXcfgRemapSelect161Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,161,pData);
}

int
soc_bm9600_XbXcfgRemapSelect161Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,161,uData);
}

int
soc_bm9600_XbXcfgRemapSelect161Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect161FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect161FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,161,uData);
}

int
soc_bm9600_XbXcfgRemapSelect162Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,162,pData);
}

int
soc_bm9600_XbXcfgRemapSelect162Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,162,uData);
}

int
soc_bm9600_XbXcfgRemapSelect162Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect162FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect162FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,162,uData);
}

int
soc_bm9600_XbXcfgRemapSelect163Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,163,pData);
}

int
soc_bm9600_XbXcfgRemapSelect163Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,163,uData);
}

int
soc_bm9600_XbXcfgRemapSelect163Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect163FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect163FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,163,uData);
}

int
soc_bm9600_XbXcfgRemapSelect164Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,164,pData);
}

int
soc_bm9600_XbXcfgRemapSelect164Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,164,uData);
}

int
soc_bm9600_XbXcfgRemapSelect164Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect164FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect164FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,164,uData);
}

int
soc_bm9600_XbXcfgRemapSelect165Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,165,pData);
}

int
soc_bm9600_XbXcfgRemapSelect165Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,165,uData);
}

int
soc_bm9600_XbXcfgRemapSelect165Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect165FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect165FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,165,uData);
}

int
soc_bm9600_XbXcfgRemapSelect166Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,166,pData);
}

int
soc_bm9600_XbXcfgRemapSelect166Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,166,uData);
}

int
soc_bm9600_XbXcfgRemapSelect166Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect166FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect166FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,166,uData);
}

int
soc_bm9600_XbXcfgRemapSelect167Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,167,pData);
}

int
soc_bm9600_XbXcfgRemapSelect167Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,167,uData);
}

int
soc_bm9600_XbXcfgRemapSelect167Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect167FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect167FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,167,uData);
}

int
soc_bm9600_XbXcfgRemapSelect168Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,168,pData);
}

int
soc_bm9600_XbXcfgRemapSelect168Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,168,uData);
}

int
soc_bm9600_XbXcfgRemapSelect168Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect168FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect168FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,168,uData);
}

int
soc_bm9600_XbXcfgRemapSelect169Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,169,pData);
}

int
soc_bm9600_XbXcfgRemapSelect169Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,169,uData);
}

int
soc_bm9600_XbXcfgRemapSelect169Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect169FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect169FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,169,uData);
}

int
soc_bm9600_XbXcfgRemapSelect17Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,17,pData);
}

int
soc_bm9600_XbXcfgRemapSelect17Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,17,uData);
}

int
soc_bm9600_XbXcfgRemapSelect17Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect17FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect17FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,17,uData);
}

int
soc_bm9600_XbXcfgRemapSelect170Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,170,pData);
}

int
soc_bm9600_XbXcfgRemapSelect170Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,170,uData);
}

int
soc_bm9600_XbXcfgRemapSelect170Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect170FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect170FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,170,uData);
}

int
soc_bm9600_XbXcfgRemapSelect171Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,171,pData);
}

int
soc_bm9600_XbXcfgRemapSelect171Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,171,uData);
}

int
soc_bm9600_XbXcfgRemapSelect171Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect171FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect171FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,171,uData);
}

int
soc_bm9600_XbXcfgRemapSelect172Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,172,pData);
}

int
soc_bm9600_XbXcfgRemapSelect172Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,172,uData);
}

int
soc_bm9600_XbXcfgRemapSelect172Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect172FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect172FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,172,uData);
}

int
soc_bm9600_XbXcfgRemapSelect173Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,173,pData);
}

int
soc_bm9600_XbXcfgRemapSelect173Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,173,uData);
}

int
soc_bm9600_XbXcfgRemapSelect173Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect173FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect173FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,173,uData);
}

int
soc_bm9600_XbXcfgRemapSelect174Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,174,pData);
}

int
soc_bm9600_XbXcfgRemapSelect174Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,174,uData);
}

int
soc_bm9600_XbXcfgRemapSelect174Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect174FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect174FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,174,uData);
}

int
soc_bm9600_XbXcfgRemapSelect175Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,175,pData);
}

int
soc_bm9600_XbXcfgRemapSelect175Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,175,uData);
}

int
soc_bm9600_XbXcfgRemapSelect175Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect175FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect175FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,175,uData);
}

int
soc_bm9600_XbXcfgRemapSelect176Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,176,pData);
}

int
soc_bm9600_XbXcfgRemapSelect176Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,176,uData);
}

int
soc_bm9600_XbXcfgRemapSelect176Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect176FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect176FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,176,uData);
}

int
soc_bm9600_XbXcfgRemapSelect177Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,177,pData);
}

int
soc_bm9600_XbXcfgRemapSelect177Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,177,uData);
}

int
soc_bm9600_XbXcfgRemapSelect177Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect177FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect177FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,177,uData);
}

int
soc_bm9600_XbXcfgRemapSelect178Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,178,pData);
}

int
soc_bm9600_XbXcfgRemapSelect178Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,178,uData);
}

int
soc_bm9600_XbXcfgRemapSelect178Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect178FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect178FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,178,uData);
}

int
soc_bm9600_XbXcfgRemapSelect179Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,179,pData);
}

int
soc_bm9600_XbXcfgRemapSelect179Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,179,uData);
}

int
soc_bm9600_XbXcfgRemapSelect179Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect179FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect179FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,179,uData);
}

int
soc_bm9600_XbXcfgRemapSelect18Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,18,pData);
}

int
soc_bm9600_XbXcfgRemapSelect18Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,18,uData);
}

int
soc_bm9600_XbXcfgRemapSelect18Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect18FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect18FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,18,uData);
}

int
soc_bm9600_XbXcfgRemapSelect180Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,180,pData);
}

int
soc_bm9600_XbXcfgRemapSelect180Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,180,uData);
}

int
soc_bm9600_XbXcfgRemapSelect180Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect180FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect180FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,180,uData);
}

int
soc_bm9600_XbXcfgRemapSelect181Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,181,pData);
}

int
soc_bm9600_XbXcfgRemapSelect181Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,181,uData);
}

int
soc_bm9600_XbXcfgRemapSelect181Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect181FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect181FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,181,uData);
}

int
soc_bm9600_XbXcfgRemapSelect182Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,182,pData);
}

int
soc_bm9600_XbXcfgRemapSelect182Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,182,uData);
}

int
soc_bm9600_XbXcfgRemapSelect182Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect182FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect182FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,182,uData);
}

int
soc_bm9600_XbXcfgRemapSelect183Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,183,pData);
}

int
soc_bm9600_XbXcfgRemapSelect183Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,183,uData);
}

int
soc_bm9600_XbXcfgRemapSelect183Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect183FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect183FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,183,uData);
}

int
soc_bm9600_XbXcfgRemapSelect184Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,184,pData);
}

int
soc_bm9600_XbXcfgRemapSelect184Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,184,uData);
}

int
soc_bm9600_XbXcfgRemapSelect184Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect184FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect184FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,184,uData);
}

int
soc_bm9600_XbXcfgRemapSelect185Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,185,pData);
}

int
soc_bm9600_XbXcfgRemapSelect185Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,185,uData);
}

int
soc_bm9600_XbXcfgRemapSelect185Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect185FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect185FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,185,uData);
}

int
soc_bm9600_XbXcfgRemapSelect186Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,186,pData);
}

int
soc_bm9600_XbXcfgRemapSelect186Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,186,uData);
}

int
soc_bm9600_XbXcfgRemapSelect186Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect186FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect186FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,186,uData);
}

int
soc_bm9600_XbXcfgRemapSelect187Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,187,pData);
}

int
soc_bm9600_XbXcfgRemapSelect187Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,187,uData);
}

int
soc_bm9600_XbXcfgRemapSelect187Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect187FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect187FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,187,uData);
}

int
soc_bm9600_XbXcfgRemapSelect188Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,188,pData);
}

int
soc_bm9600_XbXcfgRemapSelect188Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,188,uData);
}

int
soc_bm9600_XbXcfgRemapSelect188Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect188FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect188FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,188,uData);
}

int
soc_bm9600_XbXcfgRemapSelect189Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,189,pData);
}

int
soc_bm9600_XbXcfgRemapSelect189Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,189,uData);
}

int
soc_bm9600_XbXcfgRemapSelect189Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect189FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect189FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,189,uData);
}

int
soc_bm9600_XbXcfgRemapSelect19Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,19,pData);
}

int
soc_bm9600_XbXcfgRemapSelect19Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,19,uData);
}

int
soc_bm9600_XbXcfgRemapSelect19Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect19FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect19FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,19,uData);
}

int
soc_bm9600_XbXcfgRemapSelect190Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,190,pData);
}

int
soc_bm9600_XbXcfgRemapSelect190Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,190,uData);
}

int
soc_bm9600_XbXcfgRemapSelect190Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect190FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect190FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,190,uData);
}

int
soc_bm9600_XbXcfgRemapSelect191Read( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,191,pData);
}

int
soc_bm9600_XbXcfgRemapSelect191Write( uint32 uBaseAddress,
                                          uint32 uAddress,
                                          uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,191,uData);
}

int
soc_bm9600_XbXcfgRemapSelect191Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect191FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect191FillPattern( uint32 uBaseAddress,
                                                uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,191,uData);
}

int
soc_bm9600_XbXcfgRemapSelect2Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,2,pData);
}

int
soc_bm9600_XbXcfgRemapSelect2Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,2,uData);
}

int
soc_bm9600_XbXcfgRemapSelect2Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect2FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect2FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,2,uData);
}

int
soc_bm9600_XbXcfgRemapSelect20Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,20,pData);
}

int
soc_bm9600_XbXcfgRemapSelect20Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,20,uData);
}

int
soc_bm9600_XbXcfgRemapSelect20Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect20FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect20FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,20,uData);
}

int
soc_bm9600_XbXcfgRemapSelect21Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,21,pData);
}

int
soc_bm9600_XbXcfgRemapSelect21Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,21,uData);
}

int
soc_bm9600_XbXcfgRemapSelect21Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect21FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect21FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,21,uData);
}

int
soc_bm9600_XbXcfgRemapSelect22Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,22,pData);
}

int
soc_bm9600_XbXcfgRemapSelect22Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,22,uData);
}

int
soc_bm9600_XbXcfgRemapSelect22Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect22FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect22FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,22,uData);
}

int
soc_bm9600_XbXcfgRemapSelect23Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,23,pData);
}

int
soc_bm9600_XbXcfgRemapSelect23Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,23,uData);
}

int
soc_bm9600_XbXcfgRemapSelect23Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect23FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect23FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,23,uData);
}

int
soc_bm9600_XbXcfgRemapSelect24Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,24,pData);
}

int
soc_bm9600_XbXcfgRemapSelect24Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,24,uData);
}

int
soc_bm9600_XbXcfgRemapSelect24Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect24FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect24FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,24,uData);
}

int
soc_bm9600_XbXcfgRemapSelect25Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,25,pData);
}

int
soc_bm9600_XbXcfgRemapSelect25Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,25,uData);
}

int
soc_bm9600_XbXcfgRemapSelect25Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect25FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect25FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,25,uData);
}

int
soc_bm9600_XbXcfgRemapSelect26Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,26,pData);
}

int
soc_bm9600_XbXcfgRemapSelect26Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,26,uData);
}

int
soc_bm9600_XbXcfgRemapSelect26Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect26FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect26FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,26,uData);
}

int
soc_bm9600_XbXcfgRemapSelect27Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,27,pData);
}

int
soc_bm9600_XbXcfgRemapSelect27Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,27,uData);
}

int
soc_bm9600_XbXcfgRemapSelect27Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect27FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect27FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,27,uData);
}

int
soc_bm9600_XbXcfgRemapSelect28Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,28,pData);
}

int
soc_bm9600_XbXcfgRemapSelect28Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,28,uData);
}

int
soc_bm9600_XbXcfgRemapSelect28Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect28FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect28FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,28,uData);
}

int
soc_bm9600_XbXcfgRemapSelect29Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,29,pData);
}

int
soc_bm9600_XbXcfgRemapSelect29Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,29,uData);
}

int
soc_bm9600_XbXcfgRemapSelect29Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect29FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect29FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,29,uData);
}

int
soc_bm9600_XbXcfgRemapSelect3Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,3,pData);
}

int
soc_bm9600_XbXcfgRemapSelect3Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,3,uData);
}

int
soc_bm9600_XbXcfgRemapSelect3Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect3FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect3FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,3,uData);
}

int
soc_bm9600_XbXcfgRemapSelect30Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,30,pData);
}

int
soc_bm9600_XbXcfgRemapSelect30Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,30,uData);
}

int
soc_bm9600_XbXcfgRemapSelect30Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect30FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect30FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,30,uData);
}

int
soc_bm9600_XbXcfgRemapSelect31Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,31,pData);
}

int
soc_bm9600_XbXcfgRemapSelect31Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,31,uData);
}

int
soc_bm9600_XbXcfgRemapSelect31Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect31FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect31FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,31,uData);
}

int
soc_bm9600_XbXcfgRemapSelect32Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,32,pData);
}

int
soc_bm9600_XbXcfgRemapSelect32Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,32,uData);
}

int
soc_bm9600_XbXcfgRemapSelect32Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect32FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect32FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,32,uData);
}

int
soc_bm9600_XbXcfgRemapSelect33Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,33,pData);
}

int
soc_bm9600_XbXcfgRemapSelect33Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,33,uData);
}

int
soc_bm9600_XbXcfgRemapSelect33Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect33FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect33FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,33,uData);
}

int
soc_bm9600_XbXcfgRemapSelect34Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,34,pData);
}

int
soc_bm9600_XbXcfgRemapSelect34Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,34,uData);
}

int
soc_bm9600_XbXcfgRemapSelect34Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect34FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect34FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,34,uData);
}

int
soc_bm9600_XbXcfgRemapSelect35Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,35,pData);
}

int
soc_bm9600_XbXcfgRemapSelect35Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,35,uData);
}

int
soc_bm9600_XbXcfgRemapSelect35Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect35FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect35FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,35,uData);
}

int
soc_bm9600_XbXcfgRemapSelect36Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,36,pData);
}

int
soc_bm9600_XbXcfgRemapSelect36Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,36,uData);
}

int
soc_bm9600_XbXcfgRemapSelect36Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect36FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect36FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,36,uData);
}

int
soc_bm9600_XbXcfgRemapSelect37Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,37,pData);
}

int
soc_bm9600_XbXcfgRemapSelect37Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,37,uData);
}

int
soc_bm9600_XbXcfgRemapSelect37Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect37FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect37FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,37,uData);
}

int
soc_bm9600_XbXcfgRemapSelect38Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,38,pData);
}

int
soc_bm9600_XbXcfgRemapSelect38Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,38,uData);
}

int
soc_bm9600_XbXcfgRemapSelect38Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect38FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect38FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,38,uData);
}

int
soc_bm9600_XbXcfgRemapSelect39Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,39,pData);
}

int
soc_bm9600_XbXcfgRemapSelect39Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,39,uData);
}

int
soc_bm9600_XbXcfgRemapSelect39Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect39FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect39FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,39,uData);
}

int
soc_bm9600_XbXcfgRemapSelect4Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,4,pData);
}

int
soc_bm9600_XbXcfgRemapSelect4Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,4,uData);
}

int
soc_bm9600_XbXcfgRemapSelect4Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect4FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect4FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,4,uData);
}

int
soc_bm9600_XbXcfgRemapSelect40Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,40,pData);
}

int
soc_bm9600_XbXcfgRemapSelect40Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,40,uData);
}

int
soc_bm9600_XbXcfgRemapSelect40Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect40FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect40FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,40,uData);
}

int
soc_bm9600_XbXcfgRemapSelect41Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,41,pData);
}

int
soc_bm9600_XbXcfgRemapSelect41Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,41,uData);
}

int
soc_bm9600_XbXcfgRemapSelect41Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect41FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect41FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,41,uData);
}

int
soc_bm9600_XbXcfgRemapSelect42Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,42,pData);
}

int
soc_bm9600_XbXcfgRemapSelect42Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,42,uData);
}

int
soc_bm9600_XbXcfgRemapSelect42Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect42FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect42FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,42,uData);
}

int
soc_bm9600_XbXcfgRemapSelect43Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,43,pData);
}

int
soc_bm9600_XbXcfgRemapSelect43Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,43,uData);
}

int
soc_bm9600_XbXcfgRemapSelect43Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect43FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect43FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,43,uData);
}

int
soc_bm9600_XbXcfgRemapSelect44Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,44,pData);
}

int
soc_bm9600_XbXcfgRemapSelect44Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,44,uData);
}

int
soc_bm9600_XbXcfgRemapSelect44Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect44FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect44FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,44,uData);
}

int
soc_bm9600_XbXcfgRemapSelect45Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,45,pData);
}

int
soc_bm9600_XbXcfgRemapSelect45Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,45,uData);
}

int
soc_bm9600_XbXcfgRemapSelect45Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect45FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect45FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,45,uData);
}

int
soc_bm9600_XbXcfgRemapSelect46Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,46,pData);
}

int
soc_bm9600_XbXcfgRemapSelect46Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,46,uData);
}

int
soc_bm9600_XbXcfgRemapSelect46Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect46FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect46FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,46,uData);
}

int
soc_bm9600_XbXcfgRemapSelect47Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,47,pData);
}

int
soc_bm9600_XbXcfgRemapSelect47Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,47,uData);
}

int
soc_bm9600_XbXcfgRemapSelect47Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect47FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect47FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,47,uData);
}

int
soc_bm9600_XbXcfgRemapSelect48Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,48,pData);
}

int
soc_bm9600_XbXcfgRemapSelect48Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,48,uData);
}

int
soc_bm9600_XbXcfgRemapSelect48Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect48FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect48FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,48,uData);
}

int
soc_bm9600_XbXcfgRemapSelect49Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,49,pData);
}

int
soc_bm9600_XbXcfgRemapSelect49Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,49,uData);
}

int
soc_bm9600_XbXcfgRemapSelect49Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect49FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect49FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,49,uData);
}

int
soc_bm9600_XbXcfgRemapSelect5Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,5,pData);
}

int
soc_bm9600_XbXcfgRemapSelect5Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,5,uData);
}

int
soc_bm9600_XbXcfgRemapSelect5Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect5FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect5FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,5,uData);
}

int
soc_bm9600_XbXcfgRemapSelect50Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,50,pData);
}

int
soc_bm9600_XbXcfgRemapSelect50Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,50,uData);
}

int
soc_bm9600_XbXcfgRemapSelect50Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect50FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect50FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,50,uData);
}

int
soc_bm9600_XbXcfgRemapSelect51Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,51,pData);
}

int
soc_bm9600_XbXcfgRemapSelect51Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,51,uData);
}

int
soc_bm9600_XbXcfgRemapSelect51Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect51FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect51FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,51,uData);
}

int
soc_bm9600_XbXcfgRemapSelect52Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,52,pData);
}

int
soc_bm9600_XbXcfgRemapSelect52Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,52,uData);
}

int
soc_bm9600_XbXcfgRemapSelect52Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect52FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect52FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,52,uData);
}

int
soc_bm9600_XbXcfgRemapSelect53Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,53,pData);
}

int
soc_bm9600_XbXcfgRemapSelect53Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,53,uData);
}

int
soc_bm9600_XbXcfgRemapSelect53Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect53FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect53FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,53,uData);
}

int
soc_bm9600_XbXcfgRemapSelect54Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,54,pData);
}

int
soc_bm9600_XbXcfgRemapSelect54Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,54,uData);
}

int
soc_bm9600_XbXcfgRemapSelect54Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect54FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect54FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,54,uData);
}

int
soc_bm9600_XbXcfgRemapSelect55Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,55,pData);
}

int
soc_bm9600_XbXcfgRemapSelect55Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,55,uData);
}

int
soc_bm9600_XbXcfgRemapSelect55Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect55FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect55FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,55,uData);
}

int
soc_bm9600_XbXcfgRemapSelect56Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,56,pData);
}

int
soc_bm9600_XbXcfgRemapSelect56Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,56,uData);
}

int
soc_bm9600_XbXcfgRemapSelect56Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect56FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect56FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,56,uData);
}

int
soc_bm9600_XbXcfgRemapSelect57Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,57,pData);
}

int
soc_bm9600_XbXcfgRemapSelect57Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,57,uData);
}

int
soc_bm9600_XbXcfgRemapSelect57Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect57FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect57FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,57,uData);
}

int
soc_bm9600_XbXcfgRemapSelect58Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,58,pData);
}

int
soc_bm9600_XbXcfgRemapSelect58Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,58,uData);
}

int
soc_bm9600_XbXcfgRemapSelect58Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect58FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect58FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,58,uData);
}

int
soc_bm9600_XbXcfgRemapSelect59Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,59,pData);
}

int
soc_bm9600_XbXcfgRemapSelect59Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,59,uData);
}

int
soc_bm9600_XbXcfgRemapSelect59Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect59FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect59FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,59,uData);
}

int
soc_bm9600_XbXcfgRemapSelect6Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,6,pData);
}

int
soc_bm9600_XbXcfgRemapSelect6Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,6,uData);
}

int
soc_bm9600_XbXcfgRemapSelect6Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect6FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect6FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,6,uData);
}

int
soc_bm9600_XbXcfgRemapSelect60Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,60,pData);
}

int
soc_bm9600_XbXcfgRemapSelect60Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,60,uData);
}

int
soc_bm9600_XbXcfgRemapSelect60Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect60FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect60FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,60,uData);
}

int
soc_bm9600_XbXcfgRemapSelect61Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,61,pData);
}

int
soc_bm9600_XbXcfgRemapSelect61Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,61,uData);
}

int
soc_bm9600_XbXcfgRemapSelect61Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect61FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect61FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,61,uData);
}

int
soc_bm9600_XbXcfgRemapSelect62Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,62,pData);
}

int
soc_bm9600_XbXcfgRemapSelect62Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,62,uData);
}

int
soc_bm9600_XbXcfgRemapSelect62Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect62FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect62FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,62,uData);
}

int
soc_bm9600_XbXcfgRemapSelect63Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,63,pData);
}

int
soc_bm9600_XbXcfgRemapSelect63Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,63,uData);
}

int
soc_bm9600_XbXcfgRemapSelect63Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect63FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect63FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,63,uData);
}

int
soc_bm9600_XbXcfgRemapSelect64Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,64,pData);
}

int
soc_bm9600_XbXcfgRemapSelect64Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,64,uData);
}

int
soc_bm9600_XbXcfgRemapSelect64Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect64FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect64FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,64,uData);
}

int
soc_bm9600_XbXcfgRemapSelect65Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,65,pData);
}

int
soc_bm9600_XbXcfgRemapSelect65Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,65,uData);
}

int
soc_bm9600_XbXcfgRemapSelect65Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect65FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect65FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,65,uData);
}

int
soc_bm9600_XbXcfgRemapSelect66Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,66,pData);
}

int
soc_bm9600_XbXcfgRemapSelect66Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,66,uData);
}

int
soc_bm9600_XbXcfgRemapSelect66Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect66FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect66FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,66,uData);
}

int
soc_bm9600_XbXcfgRemapSelect67Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,67,pData);
}

int
soc_bm9600_XbXcfgRemapSelect67Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,67,uData);
}

int
soc_bm9600_XbXcfgRemapSelect67Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect67FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect67FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,67,uData);
}

int
soc_bm9600_XbXcfgRemapSelect68Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,68,pData);
}

int
soc_bm9600_XbXcfgRemapSelect68Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,68,uData);
}

int
soc_bm9600_XbXcfgRemapSelect68Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect68FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect68FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,68,uData);
}

int
soc_bm9600_XbXcfgRemapSelect69Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,69,pData);
}

int
soc_bm9600_XbXcfgRemapSelect69Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,69,uData);
}

int
soc_bm9600_XbXcfgRemapSelect69Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect69FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect69FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,69,uData);
}

int
soc_bm9600_XbXcfgRemapSelect7Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,7,pData);
}

int
soc_bm9600_XbXcfgRemapSelect7Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,7,uData);
}

int
soc_bm9600_XbXcfgRemapSelect7Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect7FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect7FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,7,uData);
}

int
soc_bm9600_XbXcfgRemapSelect70Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,70,pData);
}

int
soc_bm9600_XbXcfgRemapSelect70Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,70,uData);
}

int
soc_bm9600_XbXcfgRemapSelect70Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect70FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect70FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,70,uData);
}

int
soc_bm9600_XbXcfgRemapSelect71Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,71,pData);
}

int
soc_bm9600_XbXcfgRemapSelect71Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,71,uData);
}

int
soc_bm9600_XbXcfgRemapSelect71Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect71FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect71FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,71,uData);
}

int
soc_bm9600_XbXcfgRemapSelect72Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,72,pData);
}

int
soc_bm9600_XbXcfgRemapSelect72Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,72,uData);
}

int
soc_bm9600_XbXcfgRemapSelect72Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect72FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect72FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,72,uData);
}

int
soc_bm9600_XbXcfgRemapSelect73Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,73,pData);
}

int
soc_bm9600_XbXcfgRemapSelect73Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,73,uData);
}

int
soc_bm9600_XbXcfgRemapSelect73Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect73FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect73FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,73,uData);
}

int
soc_bm9600_XbXcfgRemapSelect74Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,74,pData);
}

int
soc_bm9600_XbXcfgRemapSelect74Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,74,uData);
}

int
soc_bm9600_XbXcfgRemapSelect74Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect74FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect74FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,74,uData);
}

int
soc_bm9600_XbXcfgRemapSelect75Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,75,pData);
}

int
soc_bm9600_XbXcfgRemapSelect75Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,75,uData);
}

int
soc_bm9600_XbXcfgRemapSelect75Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect75FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect75FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,75,uData);
}

int
soc_bm9600_XbXcfgRemapSelect76Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,76,pData);
}

int
soc_bm9600_XbXcfgRemapSelect76Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,76,uData);
}

int
soc_bm9600_XbXcfgRemapSelect76Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect76FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect76FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,76,uData);
}

int
soc_bm9600_XbXcfgRemapSelect77Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,77,pData);
}

int
soc_bm9600_XbXcfgRemapSelect77Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,77,uData);
}

int
soc_bm9600_XbXcfgRemapSelect77Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect77FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect77FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,77,uData);
}

int
soc_bm9600_XbXcfgRemapSelect78Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,78,pData);
}

int
soc_bm9600_XbXcfgRemapSelect78Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,78,uData);
}

int
soc_bm9600_XbXcfgRemapSelect78Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect78FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect78FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,78,uData);
}

int
soc_bm9600_XbXcfgRemapSelect79Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,79,pData);
}

int
soc_bm9600_XbXcfgRemapSelect79Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,79,uData);
}

int
soc_bm9600_XbXcfgRemapSelect79Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect79FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect79FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,79,uData);
}

int
soc_bm9600_XbXcfgRemapSelect8Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,8,pData);
}

int
soc_bm9600_XbXcfgRemapSelect8Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,8,uData);
}

int
soc_bm9600_XbXcfgRemapSelect8Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect8FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect8FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,8,uData);
}

int
soc_bm9600_XbXcfgRemapSelect80Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,80,pData);
}

int
soc_bm9600_XbXcfgRemapSelect80Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,80,uData);
}

int
soc_bm9600_XbXcfgRemapSelect80Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect80FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect80FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,80,uData);
}

int
soc_bm9600_XbXcfgRemapSelect81Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,81,pData);
}

int
soc_bm9600_XbXcfgRemapSelect81Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,81,uData);
}

int
soc_bm9600_XbXcfgRemapSelect81Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect81FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect81FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,81,uData);
}

int
soc_bm9600_XbXcfgRemapSelect82Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,82,pData);
}

int
soc_bm9600_XbXcfgRemapSelect82Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,82,uData);
}

int
soc_bm9600_XbXcfgRemapSelect82Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect82FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect82FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,82,uData);
}

int
soc_bm9600_XbXcfgRemapSelect83Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,83,pData);
}

int
soc_bm9600_XbXcfgRemapSelect83Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,83,uData);
}

int
soc_bm9600_XbXcfgRemapSelect83Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect83FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect83FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,83,uData);
}

int
soc_bm9600_XbXcfgRemapSelect84Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,84,pData);
}

int
soc_bm9600_XbXcfgRemapSelect84Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,84,uData);
}

int
soc_bm9600_XbXcfgRemapSelect84Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect84FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect84FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,84,uData);
}

int
soc_bm9600_XbXcfgRemapSelect85Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,85,pData);
}

int
soc_bm9600_XbXcfgRemapSelect85Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,85,uData);
}

int
soc_bm9600_XbXcfgRemapSelect85Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect85FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect85FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,85,uData);
}

int
soc_bm9600_XbXcfgRemapSelect86Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,86,pData);
}

int
soc_bm9600_XbXcfgRemapSelect86Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,86,uData);
}

int
soc_bm9600_XbXcfgRemapSelect86Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect86FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect86FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,86,uData);
}

int
soc_bm9600_XbXcfgRemapSelect87Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,87,pData);
}

int
soc_bm9600_XbXcfgRemapSelect87Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,87,uData);
}

int
soc_bm9600_XbXcfgRemapSelect87Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect87FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect87FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,87,uData);
}

int
soc_bm9600_XbXcfgRemapSelect88Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,88,pData);
}

int
soc_bm9600_XbXcfgRemapSelect88Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,88,uData);
}

int
soc_bm9600_XbXcfgRemapSelect88Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect88FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect88FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,88,uData);
}

int
soc_bm9600_XbXcfgRemapSelect89Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,89,pData);
}

int
soc_bm9600_XbXcfgRemapSelect89Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,89,uData);
}

int
soc_bm9600_XbXcfgRemapSelect89Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect89FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect89FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,89,uData);
}

int
soc_bm9600_XbXcfgRemapSelect9Read( uint32 uBaseAddress,
                                       uint32 uAddress,
                                       uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,9,pData);
}

int
soc_bm9600_XbXcfgRemapSelect9Write( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,9,uData);
}

int
soc_bm9600_XbXcfgRemapSelect9Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect9FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect9FillPattern( uint32 uBaseAddress,
                                              uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,9,uData);
}

int
soc_bm9600_XbXcfgRemapSelect90Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,90,pData);
}

int
soc_bm9600_XbXcfgRemapSelect90Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,90,uData);
}

int
soc_bm9600_XbXcfgRemapSelect90Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect90FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect90FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,90,uData);
}

int
soc_bm9600_XbXcfgRemapSelect91Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,91,pData);
}

int
soc_bm9600_XbXcfgRemapSelect91Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,91,uData);
}

int
soc_bm9600_XbXcfgRemapSelect91Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect91FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect91FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,91,uData);
}

int
soc_bm9600_XbXcfgRemapSelect92Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,92,pData);
}

int
soc_bm9600_XbXcfgRemapSelect92Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,92,uData);
}

int
soc_bm9600_XbXcfgRemapSelect92Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect92FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect92FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,92,uData);
}

int
soc_bm9600_XbXcfgRemapSelect93Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,93,pData);
}

int
soc_bm9600_XbXcfgRemapSelect93Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,93,uData);
}

int
soc_bm9600_XbXcfgRemapSelect93Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect93FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect93FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,93,uData);
}

int
soc_bm9600_XbXcfgRemapSelect94Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,94,pData);
}

int
soc_bm9600_XbXcfgRemapSelect94Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,94,uData);
}

int
soc_bm9600_XbXcfgRemapSelect94Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect94FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect94FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,94,uData);
}

int
soc_bm9600_XbXcfgRemapSelect95Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,95,pData);
}

int
soc_bm9600_XbXcfgRemapSelect95Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,95,uData);
}

int
soc_bm9600_XbXcfgRemapSelect95Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect95FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect95FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,95,uData);
}

int
soc_bm9600_XbXcfgRemapSelect96Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,96,pData);
}

int
soc_bm9600_XbXcfgRemapSelect96Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,96,uData);
}

int
soc_bm9600_XbXcfgRemapSelect96Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect96FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect96FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,96,uData);
}

int
soc_bm9600_XbXcfgRemapSelect97Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,97,pData);
}

int
soc_bm9600_XbXcfgRemapSelect97Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,97,uData);
}

int
soc_bm9600_XbXcfgRemapSelect97Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect97FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect97FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,97,uData);
}

int
soc_bm9600_XbXcfgRemapSelect98Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,98,pData);
}

int
soc_bm9600_XbXcfgRemapSelect98Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,98,uData);
}

int
soc_bm9600_XbXcfgRemapSelect98Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect98FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect98FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,98,uData);
}

int
soc_bm9600_XbXcfgRemapSelect99Read( uint32 uBaseAddress,
                                        uint32 uAddress,
                                        uint32 *pData) {
  return soc_bm9600_XbXcfgRemapSelectRead(uBaseAddress,uAddress,99,pData);
}

int
soc_bm9600_XbXcfgRemapSelect99Write( uint32 uBaseAddress,
                                         uint32 uAddress,
                                         uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectWrite(uBaseAddress,uAddress,99,uData);
}

int
soc_bm9600_XbXcfgRemapSelect99Clear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_XbXcfgRemapSelect99FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_XbXcfgRemapSelect99FillPattern( uint32 uBaseAddress,
                                               uint32 uData) {
  return soc_bm9600_XbXcfgRemapSelectFillPattern(uBaseAddress,99,uData);
}

#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwAllocCfgBaseRead( uint32 uBaseAddress,
                                   uint32 uAddress,
                                   uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 192) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
	
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=192 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwAllocCfgBaseWrite( uint32 uBaseAddress,
				uint32 uAddress,
				uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 192) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=192 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwAllocCfgBaseClear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_BwAllocCfgBaseFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwAllocCfgBaseFillPattern( uint32 uBaseAddress,
                                      uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 192) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=192 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
	
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwAllocRateRead( uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 224) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=224 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);
    
    return SOC_E_NONE;
}

int
soc_bm9600_BwAllocRateWrite( uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 224) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=224 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwAllocRateClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwAllocRateFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwAllocRateFillPattern( uint32 uBaseAddress,
                                       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    

    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 224) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
    }

    if( !uAck ) {
        BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=224 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
        return SOC_E_INTERNAL;
    }
    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);

    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


int
soc_bm9600_BwFetchDataRead( uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 64) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=64 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwFetchDataWrite( uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 64) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=64 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;

}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchDataClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwFetchDataFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwFetchDataFillPattern( uint32 uBaseAddress,
                                       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    

    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 64) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
	}
	;
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=64 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}

    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwFetchSumRead( uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 0) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);

  return SOC_E_NONE;
}

int
soc_bm9600_BwFetchSumWrite( uint32 uBaseAddress,
                                uint32 uAddress,
                                uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchSumClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwFetchSumFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwFetchSumFillPattern( uint32 uBaseAddress,
                                      uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}

        nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


int
soc_bm9600_BwFetchValidRead( uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 32) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=32 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);
    
    return SOC_E_NONE;
}

int
soc_bm9600_BwFetchValidWrite( uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 32) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=32 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchValidClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwFetchValidFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwFetchValidFillPattern( uint32 uBaseAddress,
                                        uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 32) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=32 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


/* We need to read this table for easy reload */
int
soc_bm9600_BwR0BagRead( uint32 uBaseAddress,
                            uint32 uAddress,
                            uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 9) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R0_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR0BagWrite( uint32 uBaseAddress,
			 uint32 uAddress,
			 uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 9) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0BagClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR0BagFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR0BagFillPattern( uint32 uBaseAddress,
			       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, SELECT, 9) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
        nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR0BwpRead( uint32 uBaseAddress,
                            uint32 uAddress,
                            uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 5) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R0_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR0BwpWrite( uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 5) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0BwpClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR0BwpFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR0BwpFillPattern( uint32 uBaseAddress,
                                   uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {

	SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, SELECT, 5) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, nData, 0);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData, 1);
	    SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,  nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/* We need to be able to read this table for easy reload functionality */
int
soc_bm9600_BwR0WdtRead( uint32 uBaseAddress,
                        uint32 uAddress,
                        uint32 *pData) {

    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 0) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R0_MEM_ACC_DATA);
    
    return SOC_E_NONE;
}

int
soc_bm9600_BwR0WdtWrite( uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,SELECT, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0WdtClear( uint32 uBaseAddress) {
  uint32 uData[1];
  uData[0]=0;
  return soc_bm9600_BwR0WdtFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR0WdtFillPattern( uint32 uBaseAddress,
                                   uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R0_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {

	SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, SELECT, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_R0_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R0_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


/* We nead to read this table for easy reload support */
int
soc_bm9600_BwR1BagRead( uint32 uBaseAddress,
                            uint32 uAddress,
                            uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 9) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR1BagWrite( uint32 uBaseAddress,
                             uint32 uAddress,
                             uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 9) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1BagClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1BagFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1BagFillPattern( uint32 uBaseAddress,
			       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {

	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 9) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


int
soc_bm9600_BwR1Wct0ARead( uint32 uBaseAddress,
			  uint32 uAddress,
			  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 11) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct0AWrite( uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 11) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct0AClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct0AFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1Wct0AFillPattern( uint32 uBaseAddress,
				 uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 11) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */


int
soc_bm9600_BwR1Wct0BRead( uint32 uBaseAddress,
			  uint32 uAddress,
			  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 27) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=27 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

  return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct0BWrite( uint32 uBaseAddress,
			   uint32 uAddress,
			   uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 27) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=27 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct0BClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct0BFillPattern(uBaseAddress, uData[0]);
}

 int
 soc_bm9600_BwR1Wct0BFillPattern( uint32 uBaseAddress,
				  uint32 uData) {
     uint32 uAddress;
     soc_timeout_t timeout;
     uint32 uAck = 0;
     int nData = 0;
     
     SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
     for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	 SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			 SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 27) |
			 SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			 SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			 SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			 SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			 );
	 
	 for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	      !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	     
	     nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	     uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	     nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
	     nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
	     SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
	 }
	 if( !uAck ) {
	     BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=27 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	     return SOC_E_INTERNAL;
	 }
     }
     return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR1Wct1ARead( uint32 uBaseAddress,
                              uint32 uAddress,
                              uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 43) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=43 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct1AWrite( uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 43) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=43 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct1AClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct1AFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1Wct1AFillPattern( uint32 uBaseAddress,
                                     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 43) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=43 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR1Wct1BRead( uint32 uBaseAddress,
			  uint32 uAddress,
			  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 59) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=59 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);
    return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct1BWrite( uint32 uBaseAddress,
			   uint32 uAddress,
			   uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 59) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=59 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct1BClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct1BFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1Wct1BFillPattern( uint32 uBaseAddress,
                                     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 59) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=59 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR1Wct2ARead( uint32 uBaseAddress,
			  uint32 uAddress,
			  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 75) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=75 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct2AWrite( uint32 uBaseAddress,
			   uint32 uAddress,
			   uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 75) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=75 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct2AClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct2AFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1Wct2AFillPattern( uint32 uBaseAddress,
                                     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 75) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=75 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR1Wct2BRead( uint32 uBaseAddress,
			  uint32 uAddress,
			  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 91) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=91 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);
    
    return SOC_E_NONE;
}

int
soc_bm9600_BwR1Wct2BWrite( uint32 uBaseAddress,
                               uint32 uAddress,
                               uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 91) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=91 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct2BClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1Wct2BFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1Wct2BFillPattern( uint32 uBaseAddress,
                                     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 91) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=91 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);

    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwR1WstRead( uint32 uBaseAddress,
			uint32 uAddress,
			uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 10) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_R1_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwR1WstWrite( uint32 uBaseAddress,
			 uint32 uAddress,
			 uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 10) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1WstClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwR1WstFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwR1WstFillPattern( uint32 uBaseAddress,
			       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_R1_MEM_ACC_DATA, uData);

    for (uAddress=0; (int)uAddress < (1<<15); uAddress++) {

	SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, SELECT, 10) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
        nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE( uBaseAddress, PL, BW_R1_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwWredCfgBaseRead( uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 96) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);

    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=96 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwWredCfgBaseWrite( uint32 uBaseAddress,
			       uint32 uAddress,
			       uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 96) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=96 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredCfgBaseClear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwWredCfgBaseFillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwWredCfgBaseFillPattern( uint32 uBaseAddress,
				     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 96) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=96 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwWredDropNPart1Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 128) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=128 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);

    return SOC_E_NONE;
}

int
soc_bm9600_BwWredDropNPart1Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    
    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 128) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=128 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredDropNPart1Clear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwWredDropNPart1FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwWredDropNPart1FillPattern( uint32 uBaseAddress,
                                            uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 128) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	    nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=128 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_BwWredDropNPart2Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_EASY_RELOAD( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 160) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 1) |
				SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
				);

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=160 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA);
    
    return SOC_E_NONE;
}

int
soc_bm9600_BwWredDropNPart2Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);

    SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,SELECT, 160) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,REQ, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, 1) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,RD_WR_N, 0) |
		    SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ADDRESS, uAddress)
		    );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=160 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredDropNPart2Clear( uint32 uBaseAddress) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_BwWredDropNPart2FillPattern(uBaseAddress, uData[0]);
}

int
soc_bm9600_BwWredDropNPart2FillPattern( uint32 uBaseAddress,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    

    SAND_HAL_WRITE(uBaseAddress, PL, BW_BAR_MEM_ACC_DATA, uData);
    for (uAddress=0; (int)uAddress < (1<<9); uAddress++) {
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, SELECT, 160) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, 1) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, RD_WR_N, 0) |
			SAND_HAL_SET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ADDRESS, uAddress)
			);
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_POLL( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=160 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, BW_BAR_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE( uBaseAddress, PL, BW_BAR_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaEsetPriRead( uint32 uBaseAddress,
			   uint32 uAddress,
			   uint32 uInstance,
			   uint32 *pData0,
			   uint32 *pData1,
			   uint32 *pData2,
			   uint32 *pData3) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    int retry_count = 0;

 retry:

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
      if (retry_count < 1) {
        retry_count++;
        goto retry;
      }
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData0 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2);
    *pData3 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3);   
    return SOC_E_NONE;
}

int
soc_bm9600_InaEsetPriWrite( uint32 uBaseAddress,
			    uint32 uAddress,
			    uint32 uInstance,
			    uint32 uData0,
			    uint32 uData1,
			    uint32 uData2,
			    uint32 uData3) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    int retry_count = 0;

 retry:
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3, uData3);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
      if (retry_count < 1) {
        retry_count++;
        goto retry;
      }
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

int
soc_bm9600_InaEsetPriClear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[4];
    uData[0]=0;
    uData[1]=0;
    uData[2]=0;
    uData[3]=0;
    return soc_bm9600_InaEsetPriFillPattern(uBaseAddress, uInstance, uData[0], uData[1], uData[2], uData[3]);
}

int
soc_bm9600_InaEsetPriFillPattern( uint32 uBaseAddress,
				  uint32 uInstance,
				  uint32 uData0,
				  uint32 uData1,
				  uint32 uData2,
				  uint32 uData3) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    int retry_count;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3, uData3);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
      retry_count = 0;
  retry:
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
          if (retry_count < 1) {
            retry_count++;
            goto retry;
          }
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=1 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}

int
soc_bm9600_InaHi1Selected_0Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 4) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi1Selected_0Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 4) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi1Selected_0Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi1Selected_0FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi1Selected_0FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 4) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=4 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi1Selected_1Read( uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uInstance,
                                     uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 8) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi1Selected_1Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 8) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi1Selected_1Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi1Selected_1FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi1Selected_1FillPattern( uint32 uBaseAddress,
                                        uint32 uInstance,
                                        uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 8) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
    
	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=8 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi2Selected_0Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 5) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi2Selected_0Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 5) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi2Selected_0Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi2Selected_0FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi2Selected_0FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 5) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=5 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}

        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi2Selected_1Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 9) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);
    
    return SOC_E_NONE;
}

int
soc_bm9600_InaHi2Selected_1Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 9) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi2Selected_1Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi2Selected_1FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi2Selected_1FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 9) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=9 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi3Selected_0Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 6) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi3Selected_0Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);

    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 6) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi3Selected_0Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi3Selected_0FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi3Selected_0FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 6) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=6 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi3Selected_1Read( uint32 uBaseAddress,
                                     uint32 uAddress,
                                     uint32 uInstance,
                                     uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 10) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
  
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);
    return SOC_E_NONE;
}

int
soc_bm9600_InaHi3Selected_1Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 10) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi3Selected_1Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi3Selected_1FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi3Selected_1FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 10) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=10 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }

    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi4Selected_0Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 7) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
  
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi4Selected_0Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 7) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi4Selected_0Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi4Selected_0FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi4Selected_0FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 7) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=7 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}

        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
        nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
        SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);    
    }
    
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaHi4Selected_1Read( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 11) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaHi4Selected_1Write( uint32 uBaseAddress,
				  uint32 uAddress,
				  uint32 uInstance,
				  uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
 
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 11) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }

    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi4Selected_1Clear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaHi4Selected_1FillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaHi4Selected_1FillPattern( uint32 uBaseAddress,
					uint32 uInstance,
					uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 11) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
      }
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=11 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
	
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaPortPriRead( uint32 uBaseAddress,
			   uint32 uAddress,
			   uint32 uInstance,
			   uint32 *pData0,
			   uint32 *pData1,
			   uint32 *pData2,
			   uint32 *pData3) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 0) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);

    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData0 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);
    *pData1 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1);
    *pData2 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2);
    *pData3 = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3);
    
    return SOC_E_NONE;
}

int
soc_bm9600_InaPortPriWrite( uint32 uBaseAddress,
			    uint32 uAddress,
			    uint32 uInstance,
			    uint32 uData0,
			    uint32 uData1,
			    uint32 uData2,
			    uint32 uData3) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3, uData3);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaPortPriClear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[4];
    uData[0]=0;
    uData[1]=0;
    uData[2]=0;
    uData[3]=0;
    return soc_bm9600_InaPortPriFillPattern(uBaseAddress, uInstance, uData[0], uData[1], uData[2], uData[3]);
}

int
soc_bm9600_InaPortPriFillPattern( uint32 uBaseAddress,
				  uint32 uInstance,
				  uint32 uData0,
				  uint32 uData1,
				  uint32 uData2,
				  uint32 uData3) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

  if (  (SOC_SBX_CFG_BM9600(uBaseAddress)->uDeviceMode==SOC_SBX_BME_XBAR_MODE) &&
        ((SOC_PCI_REVISION(uBaseAddress) != BCM88130_A0_REV_ID) || (SOC_PCI_REVISION(uBaseAddress) != BCM88130_A1_REV_ID))  ) {
      return SOC_E_NONE;
  }
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData0);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA1, uData1);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA2, uData2);
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA3, uData3);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );

	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}

	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=0 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

int
soc_bm9600_InaRandomNumGenRead( uint32 uBaseAddress,
				uint32 uAddress,
				uint32 uInstance,
				uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 3) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaRandomNumGenWrite( uint32 uBaseAddress,
				 uint32 uAddress,
				 uint32 uInstance,
				 uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    
    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 3) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaRandomNumGenClear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaRandomNumGenFillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaRandomNumGenFillPattern( uint32 uBaseAddress,
				       uint32 uInstance,
				       uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 3) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);
	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=3 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/* for easy_reload, this function must be read */
int
soc_bm9600_InaSysportMapRead( uint32 uBaseAddress,
			      uint32 uAddress,
			      uint32 uInstance,
			      uint32 *pData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE_EASY_RELOAD( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 2) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
				       );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error reading memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }
    *pData = SAND_HAL_READ_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0);

    return SOC_E_NONE;
}

int
soc_bm9600_InaUpdateCachedSysportMap(uint32 unit,
                                     uint32 ina_instance,
                                     uint32 sysport, 
                                     uint32 portset, 
                                     uint32 offset)

{
    int cached_ina;

    /* Sysport map update needs to be cached - even if moving! */
    if (!SOC_IS_RELOADING(uBaseAddress)) {
        /* check if cached entry needs to be updated */
        cached_ina = SOC_SBX_CFG_BM9600(unit)->cached_ina;
        if (cached_ina != -1) {
            if (ina_instance == cached_ina) {
                pInaCache[unit]->sysport.portset_index[sysport] = portset;
                pInaCache[unit]->sysport.offset[sysport] = offset;
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_bm9600_InaSysportMapWrite( uint32 uBaseAddress,
			       uint32 uAddress,
			       uint32 uInstance,
			       uint32 uData) {
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;
    sbZfFabBm9600InaSysportMapEntry_t zfInaSysportMap;
    int cached_ina;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    
    SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 2) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
			   SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uAddress)
			   );
    
    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	 !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	
	nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
    }
    if( !uAck ) {
	BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	return SOC_E_INTERNAL;
    }

  if (!SOC_IS_RELOADING(uBaseAddress)) {
      /* check if cached entry needs to be updated */
      cached_ina = SOC_SBX_CFG_BM9600(uBaseAddress)->cached_ina;
      if (cached_ina != -1) {
	  if (uInstance == cached_ina) {
	      sbZfFabBm9600InaSysportMapEntry_Unpack(&zfInaSysportMap, (uint8*)&uData, 4);
	      pInaCache[uBaseAddress]->sysport.portset_index[uAddress] = zfInaSysportMap.m_uPortsetAddr;
	      pInaCache[uBaseAddress]->sysport.offset[uAddress] = zfInaSysportMap.m_uOffset;
	  }
      }
  }

  return SOC_E_NONE;
}

int
soc_bm9600_InaSysportMapCacheRead(uint32 uBaseAddress,
			          uint32 uAddress,
			          uint32 uInstance,
			          uint32 *pData)
{
    sbZfFabBm9600InaSysportMapEntry_t   map_entry;
    int cached_ina;


    /* consistency check */
    cached_ina = SOC_SBX_CFG_BM9600(uBaseAddress)->cached_ina;
    if ( (cached_ina == -1) || (pInaCache[uBaseAddress] == NULL) ) {
	return(SOC_E_INTERNAL);
    }

    /* retreive entry from cache */
    map_entry.m_uPortsetAddr = pInaCache[uBaseAddress]->sysport.portset_index[uAddress];
    map_entry.m_uOffset =  pInaCache[uBaseAddress]->sysport.offset[uAddress];
    sbZfFabBm9600InaSysportMapEntry_Pack(&map_entry, (uint8 *)pData, 4);

    return(SOC_E_NONE);
}
                          

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaSysportMapClear( uint32 uBaseAddress,uint32 uInstance) {
    uint32 uData[1];
    uData[0]=0;
    return soc_bm9600_InaSysportMapFillPattern(uBaseAddress, uInstance, uData[0]);
}

int
soc_bm9600_InaSysportMapFillPattern( uint32 uBaseAddress,
				     uint32 uInstance,
				     uint32 uData) {
    uint32 uAddress;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int nData = 0;

    SAND_HAL_WRITE_STRIDE(uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_DATA0, uData);
    for (uAddress=0; (int)uAddress < (1<<12); uAddress++) {
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, SELECT, 2) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, REQ, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, 1) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, RD_WR_N, 0) |
			       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL, ADDRESS, uAddress)
			       );
	
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); 
	     !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    
	    nData = SAND_HAL_READ_STRIDE_POLL( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL);
	    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData);

	}
	if( !uAck ) {
	    BM9600_LOG_ERROR("%s:  Error writing memory for nTableId=2 Instance=0 uAddress=0x%08x\n", FUNCTION_NAME(), (uint32)uAddress);
	    return SOC_E_INTERNAL;
	}
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, REQ, nData, 0);
	nData = SAND_HAL_MOD_FIELD(PL, INA_MEM_ACC_CTRL, ACK, nData, 1);
	SAND_HAL_WRITE_STRIDE( uBaseAddress, PL, INA, uInstance, INA_MEM_ACC_CTRL,  nData);
    }
    return SOC_E_NONE;
}
#endif /* FUNCTION_NOW_USED */

/*
 * NM Memory Cache support functions
 */
int
soc_bm9600_NmMemoryCacheAllocate(int unit)
{
    int rc = SOC_E_NONE;
    int unit_no = 0, found_se = FALSE;
#ifdef BCM_EASY_RELOAD_SUPPORT
    int index;
#endif /* BCM_EASY_RELOAD_SUPPORT */


    if (p_NmCache[unit] == NULL) {
        p_NmCache[unit] = sal_alloc(sizeof(bm9600_NmCache_t), "nm cache");
        if (p_NmCache[unit] == NULL) {
            return(SOC_E_MEMORY);
        }
    }

    sal_memset(p_NmCache[unit], 0, sizeof(bm9600_NmCache_t));
    sal_memset(&(p_NmCache[unit]->emt.value[0][0]),
                 BM9600_NMEMTENTRY_DEFAULT_VALUE, sizeof(bm9600_NmEmtCache_t));
    sal_memset(&(p_NmCache[unit]->porsetInfo.value[0][0]),
                 BM9600_NMPORTSETINFOENTRY_DEFAULT_VALUE, sizeof(bm9600_NmPortsetInfoCache_t));
    sal_memset(&(p_NmCache[unit]->portsetlink.value[0][0]),
                 BM9600_NMPORTSETLINKENTRY_DEFAULT_VALUE, sizeof(bm9600_NmPortsetLinkCache_t));
    sal_memset(&(p_NmCache[unit]->sysportArray.value[0][0]),
                 BM9600_NMSYSPORTARRAYENTRY_DEFAULT_VALUE, sizeof(bm9600_NmSysportArrayCache_t));


    /* If Arbtier mode, determine if there is any XBAR device that can be */
    /* selected for backing up data. This requires that XBAR devices are  */
    /* on the same card as the Arbiter device and XBAR devices are        */
    /* initialized before Arbiter device. The initialization sequence     */
    /* should remain the same across system boots.                        */

    
    if (soc_bm9600_features(unit, soc_feature_arbiter)) {

        for (unit_no = 0; unit_no < SOC_MAX_NUM_DEVICES; unit_no++) {

            /* For Later revisions of Polaris set the backup unit to be the current unit. */
            /* This will effectively remove writing of memory to another backup device    */
            /* also put a fix in "HwNm*Read" routines to not disable the FO block         */
            if ( (SOC_PCI_REVISION(unit) != BCM88130_A0_REV_ID) &&
                 (SOC_PCI_REVISION(unit) != BCM88130_A1_REV_ID) ) {
                break;
            }

            /* verify that it is a different device from the current device */
            if (unit_no == unit) {
                continue;
            }

            /* verify that the device is initialized */
            if (SOC_CONTROL(unit_no) == NULL) {
                continue;
            }
            if (SOC_SBX_CONTROL(unit_no) == NULL) {
                continue;
            }

            /* determine that it is BM9600 device */
            if (!SOC_IS_SBX_BM9600(unit_no)) {
                continue;
            }

            /* determine if it is an SE only device */
            if (SOC_SBX_CFG_BM9600(unit_no)->uDeviceMode == SOC_SBX_BME_XBAR_MODE) {
                found_se = TRUE;
                break;
            }
        }
           
        if (found_se == TRUE) {
            /* store SE unit number */
            SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit = unit_no;
        }
        else {
            /* store the current device unit no */
            SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit = unit;
        }

        /* if easy reload state, then reconstruct Arbiter cache */
        if (SOC_IS_RELOADING(unit)) {

#ifdef BCM_EASY_RELOAD_SUPPORT
            for (index = 0; index < BM9600_NMEMTENTRY_MAX_INDEX; index++) {
                soc_bm9600_HwNmEmtRead(SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit,
                             index, &(p_NmCache[unit]->emt.value[index][0]),
                                    &(p_NmCache[unit]->emt.value[index][1]),
                                    &(p_NmCache[unit]->emt.value[index][2]));
            }

            for (index = 0; index < BM9600_NMPORTSETINFOENTRY_MAX_INDEX; index++) {
                soc_bm9600_HwNmPortsetInfoRead(SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit,
                             index, &(p_NmCache[unit]->porsetInfo.value[index][0]));
            }

            for (index = 0; index < BM9600_NMPORTSETLINKENTRY_MAX_INDEX; index++) {
                soc_bm9600_HwNmPortsetLinkRead(SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit,
                             index, &(p_NmCache[unit]->portsetlink.value[index][0]));
            }

            for (index = 0; index < BM9600_NMSYSPORTARRAYENTRY_MAX_INDEX; index++) {
                soc_bm9600_HwNmSysportArrayRead(SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit,
                             index, &(p_NmCache[unit]->sysportArray.value[index][0]),
                                    &(p_NmCache[unit]->sysportArray.value[index][1]),
                                    &(p_NmCache[unit]->sysportArray.value[index][2]),
                                    &(p_NmCache[unit]->sysportArray.value[index][3]),
                                    &(p_NmCache[unit]->sysportArray.value[index][4]),
                                    &(p_NmCache[unit]->sysportArray.value[index][5]));
            }
#endif /* BCM_EASY_RELOAD_SUPPORT */


        }

    }

    return(rc);
}

int
soc_bm9600_NmMemoryCacheDeAllocate(int unit)
{
    int rc = SOC_E_NONE;


    if (p_NmCache[unit] != NULL) {
         sal_free(p_NmCache[unit]);
         p_NmCache[unit] = NULL;
    }

    return(rc);
}

/*
 * INA Memory Cache support functions
 */
int
soc_bm9600_InaMemoryCacheAllocate(int unit)
{
    int rc = SOC_E_NONE;
    int cached_ina;
#ifndef _BM9600_INA_ENABLE_ON_
    int i;
#ifdef BCM_EASY_RELOAD_SUPPORT
    uint32 uData, ina_config;
    sbZfFabBm9600InaSysportMapEntry_t zfInaSysportMap;
    int is_ina_enabled;
#endif /* BCM_EASY_RELOAD_SUPPORT */
#endif /* _BM9600_INA_ENABLE_ON_ */

#ifndef _BM9600_INA_ENABLE_ON_

    /* check if this functionality is valid for this unit */
    if (!soc_bm9600_features(unit, soc_feature_arbiter)) {
        cached_ina = -1;
        SOC_SBX_CFG_BM9600(unit)->cached_ina = cached_ina;
        return(rc);
    }

    /* allocate memory if not already allocated */
    if (pInaCache[unit] == NULL) {
        pInaCache[unit] = sal_alloc(sizeof(bm9600_InaCache_t), "ina cache");
        if (pInaCache[unit] == NULL) {
            cached_ina = -1;
            SOC_SBX_CFG_BM9600(unit)->cached_ina = cached_ina;
            return(SOC_E_MEMORY);
        }
    }

    /* initialize cache to default value */
    sal_memset(pInaCache[unit], 0, sizeof(bm9600_InaCache_t));
    for (i = 0; i < BM9600_INASYSPORTENTRY_MAX_INDEX; i++) {
        pInaCache[unit]->sysport.portset_index[i] = BM9600_INASYSPORTENTRY_PORTSET_DEFAULT_VALUE;
        pInaCache[unit]->sysport.offset[i] = BM9600_INASYSPORTENTRY_OFFSET_DEFAULT_VALUE;
    }

    /* associate cache with a particular INA */
    for (i = 0; i < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; i++) {
        if (soc_property_port_get(unit, i, spn_PORT_IS_SCI, 0)) {
            break;
        }
    }
    cached_ina = (i < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS) ? i : -1;
    SOC_SBX_CFG_BM9600(unit)->cached_ina = cached_ina;

    /* if easy reload state, then reconstruct cache */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        if (cached_ina != -1) {
            /* check if INA needs to be enabled */
            ina_config = SAND_HAL_READ_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG);
            is_ina_enabled = SAND_HAL_GET_FIELD(PL, INA_CONFIG, ENABLE, ina_config) ? TRUE : FALSE;
            if (is_ina_enabled == FALSE) {
                ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 1);
                SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);
            }

            for (i = 0; i < BM9600_INASYSPORTENTRY_MAX_INDEX; i++) {
                rc = soc_bm9600_InaSysportMapRead(unit, i, cached_ina,  &uData);
                if (rc != SOC_E_NONE) {
                    /* restore previous INA state */
                    if (is_ina_enabled == FALSE) {
                        ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 0);
                        SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);

                        /* clear any errors */
                        SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_ERROR, 0xFF);
                    }
  
                    return(rc);
                }

                sbZfFabBm9600InaSysportMapEntry_Unpack(&zfInaSysportMap, (uint8*)&uData, 4);
                pInaCache[unit]->sysport.portset_index[i] = zfInaSysportMap.m_uPortsetAddr;
                pInaCache[unit]->sysport.offset[i] = zfInaSysportMap.m_uOffset;
            }

             /* restore previous INA state */
            if (is_ina_enabled == FALSE) {
                ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 0);
                SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_CONFIG, ina_config);

                /* clear any errors */
                SAND_HAL_WRITE_STRIDE(unit, PL, INA, cached_ina, INA_ERROR, 0xFF);
            }
        }
#endif /* BCM_EASY_RELOAD_SUPPORT */
    }

#else /* _BM9600_INA_ENABLE_ON_ */
    cached_ina = -1;
    SOC_SBX_CFG_BM9600(unit)->cached_ina = cached_ina;
    return(rc);
#endif /* !(_BM9600_INA_ENABLE_ON_) */

    return(rc);
}

int
soc_bm9600_InaMemoryCacheDeAllocate(int unit)
{
    int rc = SOC_E_NONE;


    if (pInaCache[unit] != NULL) {
         sal_free(pInaCache[unit]);
         pInaCache[unit] = NULL;
         SOC_SBX_CFG_BM9600(unit)->cached_ina = -1;
    }

    return(rc);
}

int
soc_bm9600_ina_sysport_sync(int unit, int ina)
{
    int rc = SOC_E_NONE;
    int sysport;
    uint32 uData;
    sbZfFabBm9600InaSysportMapEntry_t zfInaSysportMap;


    if (SOC_SBX_CFG_BM9600(unit)->cached_ina == -1) {
        return(rc);
    }

    /* sync up from cache */
    for (sysport = 0; sysport < BM9600_INASYSPORTENTRY_MAX_INDEX; sysport++) {
        zfInaSysportMap.m_uPortsetAddr = pInaCache[unit]->sysport.portset_index[sysport];
        zfInaSysportMap.m_uOffset = pInaCache[unit]->sysport.offset[sysport];
        sbZfFabBm9600InaSysportMapEntry_Pack(&zfInaSysportMap, (uint8*)&uData, 4);

        rc = soc_bm9600_InaSysportMapWrite(unit, sysport, ina, uData);
        if (rc != SOC_E_NONE) {
            break;
        }
    }

    return(rc);
}


int
soc_bm9600_NmEmtCacheRead(int unit, uint32 uAddress,
                          uint32 *pData0, uint32 *pData1, uint32 *pData2)
{
    int rc = SOC_E_NONE;


    if ( (uAddress >= BM9600_NMEMTENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    *pData0 = p_NmCache[unit]->emt.value[uAddress][0];
    *pData1 = p_NmCache[unit]->emt.value[uAddress][1];
    *pData2 = p_NmCache[unit]->emt.value[uAddress][2];

    return(rc);
}

int
soc_bm9600_NmEmtCacheWrite(int unit, uint32 uAddress,
                           uint32 uData0, uint32 uData1, uint32 uData2)
{
    int rc = SOC_E_NONE;


    if (SOC_IS_RELOADING(unit)) {
      return(rc);
    }

    if ( (uAddress >= BM9600_NMEMTENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    p_NmCache[unit]->emt.value[uAddress][0] = uData0;
    p_NmCache[unit]->emt.value[uAddress][1] = uData1;
    p_NmCache[unit]->emt.value[uAddress][2] = uData2;

    return(rc);
}

int
soc_bm9600_NmPortsetInfoCacheRead(int unit, uint32 uAddress, uint32 *pData)
{
    int rc = SOC_E_NONE;


    if ( (uAddress >= BM9600_NMPORTSETINFOENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) )  {
        return(SOC_E_INTERNAL);
    }

    *pData = p_NmCache[unit]->porsetInfo.value[uAddress][0];

    return(rc);
}

int
soc_bm9600_NmPortsetInfoCacheWrite(int unit, uint32 uAddress, uint32 uData)
{
    int rc = SOC_E_NONE;


    if (SOC_IS_RELOADING(unit)) {
      return(rc);
    }

    if ( (uAddress >= BM9600_NMPORTSETINFOENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    p_NmCache[unit]->porsetInfo.value[uAddress][0] = uData;

    return(rc);
}

int 
soc_bm9600_NmPortsetLinkCacheRead(int unit, uint32 uAddress, uint32 *pData)
{
    int rc = SOC_E_NONE;


    if ( (uAddress >= BM9600_NMPORTSETLINKENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    *pData = p_NmCache[unit]->portsetlink.value[uAddress][0];

    return(rc);
}

int
soc_bm9600_NmPortsetLinkCacheWrite(int unit, uint32 uAddress, uint32 uData)
{
    int rc = SOC_E_NONE;


    if (SOC_IS_RELOADING(unit)) {
      return(rc);
    }

    if ( (uAddress >= BM9600_NMPORTSETLINKENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    p_NmCache[unit]->portsetlink.value[uAddress][0] = uData;

    return(rc);
}

int
soc_bm9600_NmSysportArrayCacheRead(int unit, uint32 uAddress,
                                   uint32 *pData0, uint32 *pData1, uint32 *pData2,
                                   uint32 *pData3, uint32 *pData4, uint32 *pData5)
{
    int rc = SOC_E_NONE;


    if ( (uAddress >= BM9600_NMSYSPORTARRAYENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    *pData0 = p_NmCache[unit]->sysportArray.value[uAddress][0];
    *pData1 = p_NmCache[unit]->sysportArray.value[uAddress][1];
    *pData2 = p_NmCache[unit]->sysportArray.value[uAddress][2];
    *pData3 = p_NmCache[unit]->sysportArray.value[uAddress][3];
    *pData4 = p_NmCache[unit]->sysportArray.value[uAddress][4];
    *pData5 = p_NmCache[unit]->sysportArray.value[uAddress][5];

    return(rc);
}

int
soc_bm9600_NmSysportArrayCacheWrite(int unit, uint32 uAddress,
                                    uint32 uData0, uint32 uData1, uint32 uData2,
                                    uint32 uData3, uint32 uData4, uint32 uData5)
{
    int rc = SOC_E_NONE;


    if (SOC_IS_RELOADING(unit)) {
      return(rc);
    }

    if ( (uAddress >= BM9600_NMSYSPORTARRAYENTRY_MAX_INDEX) || (p_NmCache[unit] == NULL) ) {
        return(SOC_E_INTERNAL);
    }

    p_NmCache[unit]->sysportArray.value[uAddress][0] = uData0;
    p_NmCache[unit]->sysportArray.value[uAddress][1] = uData1;
    p_NmCache[unit]->sysportArray.value[uAddress][2] = uData2;
    p_NmCache[unit]->sysportArray.value[uAddress][3] = uData3;
    p_NmCache[unit]->sysportArray.value[uAddress][4] = uData4;
    p_NmCache[unit]->sysportArray.value[uAddress][5] = uData5;

    return(rc);
}

void
soc_bm9600_disable_fo(int unit)
{
    uint32 uData;


    uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FO_ENABLE, uData, 0);
    uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AC_ENABLE, uData, 0);
    SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);
}


void
soc_bm9600_enable_fo(int unit)
{
    uint32 uData;


    uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FO_ENABLE, uData, 1);
    uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AC_ENABLE, uData, 1);
    SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);
}

int
soc_bm9600_is_fo_disabled(int unit)
{
    uint32 uData;
    int is_disabled = FALSE;


    uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
    is_disabled = (SAND_HAL_GET_FIELD(PL, FO_CONFIG0, FO_ENABLE, uData) == 0) ? TRUE : FALSE;
    return(is_disabled);
}

int
soc_bm9600_epoch_in_timeslot_config_get(int unit, int num_queues, uint32 *epoch_in_timeslots)
{
    int rv = SOC_E_NONE;
    uint32 wred_deadline, dmnd_deadline, alloc_deadline;
    int32 nCosPerPort;

    /*
     * WRED processes 2 queues per timeslot, it also requires three
     * timeslots to prime the pipeline and three to drain it... If
     * WRED has not completed by its deadline, it will wind down,
     * if it has not completed within 8 timeslots of its deadline
     * it will be squelched...
     */
    wred_deadline = SB_FAB_DEVICE_BM9600_EPOCH_PRIME_PIPE + (num_queues + 1) / 2;
    wred_deadline = wred_deadline + SB_FAB_DEVICE_BM9600_EPOCH_WRED_KILL;

    /*
     * demand estimation is performed by the QEs in DMode. This
     * deadline merely determines when the BME expects that the
     * QEs will have completed their task. We set it greater than
     * the wred_deadline (arbitrarily).
     */
    dmnd_deadline = wred_deadline + 2;

    nCosPerPort = 1;

    /*
     * allocation runs at better than 2 queues per timeslot.
     * allocation must have completed by its deadline or it will be
     * squelched. allocation requires 16 timeslots to prime its
     * pipeline and 16 to wind down...
     */
    alloc_deadline = dmnd_deadline + SB_FAB_DEVICE_BM9600_EPOCH_ALLOC_PRIME_PIPE +
                                                        (num_queues * nCosPerPort) / 2;

    (*epoch_in_timeslots) = alloc_deadline + 1;

    return(rv);
}

#endif /* BCM_BM9600_SUPPORT */
