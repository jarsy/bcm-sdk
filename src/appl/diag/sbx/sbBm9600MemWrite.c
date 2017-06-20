
/*
 * $Id: sbBm9600MemWrite.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbBm9600MemWrites.c
 * Purpose:     sbx commands to write bm9600 indirect mems
 * Requires:
 */

#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>

#include <soc/defs.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_mem_access.h>
 
#ifdef BCM_BM9600_SUPPORT
/*
 * note: swap utilities used from bm9600 library 
 */
extern int sbBm9600WhichMem(char *memname);
extern int sbBm9600MemMax(int memindex);
extern void qeSwapWords(uint *pdata, int nwords);
extern bm9k6MemConfigRec bm9k6MemConfigTable[];

/*
 * sbBm9600MemSetFieldEntry - read and change contents of bm9600 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm9600MemSetFieldEntry(int unit, int memIndex, int uAddress, char *fieldname, int val, int instance)
{
  int  status;

  sbZfFabBm9600BwAllocCfgBaseEntry_t  BwAllocCfgBaseRecord;
  sbZfFabBm9600BwAllocRateEntry_t  BwAllocRateRecord;
  sbZfFabBm9600BwFetchDataEntry_t  BwFetchDataRecord;
  sbZfFabBm9600BwFetchSumEntry_t  BwFetchSumRecord;
  sbZfFabBm9600BwFetchValidEntry_t  BwFetchValidRecord;
  sbZfFabBm9600BwR0BagEntry_t  BwR0BagRecord;
  sbZfFabBm9600BwR0BwpEntry_t  BwR0BwpRecord;
  sbZfFabBm9600BwR0WdtEntry_t  BwR0WdtRecord;
  sbZfFabBm9600BwR1BagEntry_t  BwR1BagRecord;
  sbZfFabBm9600BwR1Wct0AEntry_t  BwR1Wct0ARecord;
  sbZfFabBm9600BwR1Wct0BEntry_t  BwR1Wct0BRecord;
  sbZfFabBm9600BwR1Wct1AEntry_t  BwR1Wct1ARecord;
  sbZfFabBm9600BwR1Wct1BEntry_t  BwR1Wct1BRecord;
  sbZfFabBm9600BwR1Wct2AEntry_t  BwR1Wct2ARecord;
  sbZfFabBm9600BwR1Wct2BEntry_t  BwR1Wct2BRecord;
  sbZfFabBm9600BwR1WstEntry_t  BwR1WstRecord;
  sbZfFabBm9600BwWredCfgBaseEntry_t  BwWredCfgBaseRecord;
  sbZfFabBm9600BwWredDropNPart1Entry_t  BwWredDropNPart1Record;
  sbZfFabBm9600BwWredDropNPart2Entry_t  BwWredDropNPart2Record;
  sbZfFabBm9600FoLinkStateTableEntry_t  FoLinkStateTableRecord;
  sbZfFabBm9600InaEsetPriEntry_t  InaEsetPriRecord;
  sbZfFabBm9600InaHi1Selected_0Entry_t  InaHi1Selected_0Record;
  sbZfFabBm9600InaHi1Selected_1Entry_t  InaHi1Selected_1Record;
  sbZfFabBm9600InaHi2Selected_0Entry_t  InaHi2Selected_0Record;
  sbZfFabBm9600InaHi2Selected_1Entry_t  InaHi2Selected_1Record;
  sbZfFabBm9600InaHi3Selected_0Entry_t  InaHi3Selected_0Record;
  sbZfFabBm9600InaHi3Selected_1Entry_t  InaHi3Selected_1Record;
  sbZfFabBm9600InaHi4Selected_0Entry_t  InaHi4Selected_0Record;
  sbZfFabBm9600InaHi4Selected_1Entry_t  InaHi4Selected_1Record;
  sbZfFabBm9600InaPortPriEntry_t  InaPortPriRecord;
  sbZfFabBm9600InaRandomNumGenEntry_t  InaRandomNumGenRecord;
  sbZfFabBm9600InaSysportMapEntry_t  InaSysportMapRecord;
  sbZfFabBm9600NmEgressRankerEntry_t  NmEgressRankerRecord;
  sbZfFabBm9600NmFullStatusEntry_t  NmFullStatusRecord;
  sbZfFabBm9600NmEmtEntry_t  NmEmtRecord;
  sbZfFabBm9600NmEmtdebugbank0Entry_t  NmEmtdebugbank0Record;
  sbZfFabBm9600NmEmtdebugbank1Entry_t  NmEmtdebugbank1Record;
  sbZfFabBm9600NmIngressRankerEntry_t  NmIngressRankerRecord;
  sbZfFabBm9600NmPortsetInfoEntry_t  NmPortsetInfoRecord;
  sbZfFabBm9600NmPortsetLinkEntry_t  NmPortsetLinkRecord;
  sbZfFabBm9600NmRandomNumGenEntry_t  NmRandomNumGenRecord;
  sbZfFabBm9600NmSysportArrayEntry_t  NmSysportArrayRecord;
  sbZfFabBm9600XbXcfgRemapEntry_t  XbXcfgRemapRecord;

   uint dbuf[40];

    cli_out("---  Entry index: %d ---\n", uAddress);
    switch(memIndex) {

    case BM9600_PLBWALLOCCFGBASE:
      status = soc_bm9600_BwAllocCfgBaseRead(unit, uAddress, (uint32 *)dbuf);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_READ;
      sbZfFabBm9600BwAllocCfgBaseEntry_Unpack(&BwAllocCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwAllocCfgBaseEntry_SetField(&BwAllocCfgBaseRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwAllocCfgBaseEntry_Pack(&BwAllocCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwAllocCfgBaseWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWALLOCRATE:
      status = soc_bm9600_BwAllocRateRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwAllocRateEntry_Unpack(&BwAllocRateRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwAllocRateEntry_SetField(&BwAllocRateRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwAllocRateEntry_Pack(&BwAllocRateRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwAllocRateWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWFETCHDATA:
      status = soc_bm9600_BwFetchDataRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwFetchDataEntry_Unpack(&BwFetchDataRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwFetchDataEntry_SetField(&BwFetchDataRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwFetchDataEntry_Pack(&BwFetchDataRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwFetchDataWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWFETCHSUM:
      status = soc_bm9600_BwFetchSumRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwFetchSumEntry_Unpack(&BwFetchSumRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwFetchSumEntry_SetField(&BwFetchSumRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwFetchSumEntry_Pack(&BwFetchSumRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwFetchSumWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWFETCHVALID:
      status = soc_bm9600_BwFetchValidRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwFetchValidEntry_Unpack(&BwFetchValidRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwFetchValidEntry_SetField(&BwFetchValidRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwFetchValidEntry_Pack(&BwFetchValidRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwFetchValidWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR0BAG:
      status = soc_bm9600_BwR0BagRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR0BagEntry_Unpack(&BwR0BagRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR0BagEntry_SetField(&BwR0BagRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR0BagEntry_Pack(&BwR0BagRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR0BagWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR0BWP:
      status = soc_bm9600_BwR0BwpRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR0BwpEntry_Unpack(&BwR0BwpRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR0BwpEntry_SetField(&BwR0BwpRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR0BwpEntry_Pack(&BwR0BwpRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR0BwpWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR0WDT:
      status = soc_bm9600_BwR0WdtRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR0WdtEntry_Unpack(&BwR0WdtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR0WdtEntry_SetField(&BwR0WdtRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR0WdtEntry_Pack(&BwR0WdtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR0WdtWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1BAG:
      status = soc_bm9600_BwR1BagRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1BagEntry_Unpack(&BwR1BagRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1BagEntry_SetField(&BwR1BagRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1BagEntry_Pack(&BwR1BagRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1BagWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT0A:
      status = soc_bm9600_BwR1Wct0ARead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct0AEntry_Unpack(&BwR1Wct0ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct0AEntry_SetField(&BwR1Wct0ARecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct0AEntry_Pack(&BwR1Wct0ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct0AWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT0B:
      status = soc_bm9600_BwR1Wct0BRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct0BEntry_Unpack(&BwR1Wct0BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct0BEntry_SetField(&BwR1Wct0BRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct0BEntry_Pack(&BwR1Wct0BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct0BWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT1A:
      status = soc_bm9600_BwR1Wct1ARead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct1AEntry_Unpack(&BwR1Wct1ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct1AEntry_SetField(&BwR1Wct1ARecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct1AEntry_Pack(&BwR1Wct1ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct1AWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT1B:
      status = soc_bm9600_BwR1Wct1BRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct1BEntry_Unpack(&BwR1Wct1BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct1BEntry_SetField(&BwR1Wct1BRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct1BEntry_Pack(&BwR1Wct1BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct1BWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT2A:
      status = soc_bm9600_BwR1Wct2ARead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct2AEntry_Unpack(&BwR1Wct2ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct2AEntry_SetField(&BwR1Wct2ARecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct2AEntry_Pack(&BwR1Wct2ARecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct2AWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WCT2B:
      status = soc_bm9600_BwR1Wct2BRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1Wct2BEntry_Unpack(&BwR1Wct2BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1Wct2BEntry_SetField(&BwR1Wct2BRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1Wct2BEntry_Pack(&BwR1Wct2BRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1Wct2BWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWR1WST:
      status = soc_bm9600_BwR1WstRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwR1WstEntry_Unpack(&BwR1WstRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwR1WstEntry_SetField(&BwR1WstRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwR1WstEntry_Pack(&BwR1WstRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwR1WstWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWWREDCFGBASE:
      status = soc_bm9600_BwWredCfgBaseRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwWredCfgBaseEntry_Unpack(&BwWredCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwWredCfgBaseEntry_SetField(&BwWredCfgBaseRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwWredCfgBaseEntry_Pack(&BwWredCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwWredCfgBaseWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWWREDDROPNPART1:
      status = soc_bm9600_BwWredDropNPart1Read(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwWredDropNPart1Entry_Unpack(&BwWredDropNPart1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwWredDropNPart1Entry_SetField(&BwWredDropNPart1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwWredDropNPart1Entry_Pack(&BwWredDropNPart1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwWredDropNPart1Write(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLBWWREDDROPNPART2:
      status = soc_bm9600_BwWredDropNPart2Read(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600BwWredDropNPart2Entry_Unpack(&BwWredDropNPart2Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600BwWredDropNPart2Entry_SetField(&BwWredDropNPart2Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600BwWredDropNPart2Entry_Pack(&BwWredDropNPart2Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_BwWredDropNPart2Write(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLFOLINKSTATETABLE:
      status = soc_bm9600_FoLinkStateTableRead(unit, uAddress, (uint32 *)dbuf,
					       (uint32 *)&dbuf[1]);
      sbZfFabBm9600FoLinkStateTableEntry_Unpack(&FoLinkStateTableRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600FoLinkStateTableEntry_SetField(&FoLinkStateTableRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600FoLinkStateTableEntry_Pack(&FoLinkStateTableRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_FoLinkStateTableWrite(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAESETPRI:
      status = soc_bm9600_InaEsetPriRead(unit, uAddress, instance, (uint32 *)dbuf,
					 (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					 (uint32 *)&dbuf[3]);
      sbZfFabBm9600InaEsetPriEntry_Unpack(&InaEsetPriRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaEsetPriEntry_SetField(&InaEsetPriRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaEsetPriEntry_Pack(&InaEsetPriRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaEsetPriWrite(unit, uAddress, instance, (uint32)dbuf[0], (uint32)dbuf[1],
					  (uint32)dbuf[2], (uint32)dbuf[3]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI1SELECTED_0:
      status = soc_bm9600_InaHi1Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi1Selected_0Entry_Unpack(&InaHi1Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi1Selected_0Entry_SetField(&InaHi1Selected_0Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi1Selected_0Entry_Pack(&InaHi1Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi1Selected_0Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI1SELECTED_1:
      status = soc_bm9600_InaHi1Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi1Selected_1Entry_Unpack(&InaHi1Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi1Selected_1Entry_SetField(&InaHi1Selected_1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi1Selected_1Entry_Pack(&InaHi1Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi1Selected_1Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI2SELECTED_0:
      status = soc_bm9600_InaHi2Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi2Selected_0Entry_Unpack(&InaHi2Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi2Selected_0Entry_SetField(&InaHi2Selected_0Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi2Selected_0Entry_Pack(&InaHi2Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi2Selected_0Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI2SELECTED_1:
      status = soc_bm9600_InaHi2Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi2Selected_1Entry_Unpack(&InaHi2Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi2Selected_1Entry_SetField(&InaHi2Selected_1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi2Selected_1Entry_Pack(&InaHi2Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi2Selected_1Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI3SELECTED_0:
      status = soc_bm9600_InaHi3Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi3Selected_0Entry_Unpack(&InaHi3Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi3Selected_0Entry_SetField(&InaHi3Selected_0Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi3Selected_0Entry_Pack(&InaHi3Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi3Selected_0Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI3SELECTED_1:
      status = soc_bm9600_InaHi3Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi3Selected_1Entry_Unpack(&InaHi3Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi3Selected_1Entry_SetField(&InaHi3Selected_1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi3Selected_1Entry_Pack(&InaHi3Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi3Selected_1Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI4SELECTED_0:
      status = soc_bm9600_InaHi4Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi4Selected_0Entry_Unpack(&InaHi4Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi4Selected_0Entry_SetField(&InaHi4Selected_0Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi4Selected_0Entry_Pack(&InaHi4Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi4Selected_0Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAHI4SELECTED_1:
      status = soc_bm9600_InaHi4Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaHi4Selected_1Entry_Unpack(&InaHi4Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaHi4Selected_1Entry_SetField(&InaHi4Selected_1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaHi4Selected_1Entry_Pack(&InaHi4Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaHi4Selected_1Write(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINAPORTPRI:
      status = soc_bm9600_InaPortPriRead(unit, uAddress, instance, (uint32 *)dbuf,
					 (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					 (uint32 *)&dbuf[3]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaPortPriEntry_Unpack(&InaPortPriRecord,(uint8 *)dbuf,sizeof(dbuf));
	  status = sbZfFabBm9600InaPortPriEntry_SetField(&InaPortPriRecord, fieldname, val);
	  if (status == BM9600_STATUS_OK) {
	      sbZfFabBm9600InaPortPriEntry_Pack(&InaPortPriRecord,(uint8 *)dbuf,sizeof(dbuf));
	      status = soc_bm9600_InaPortPriWrite( unit, uAddress, instance, dbuf[0], dbuf[1], dbuf[2], dbuf[3]);
	  }
      }
    break;
    case BM9600_PLINARANDOMNUMGEN:
      status = soc_bm9600_InaRandomNumGenRead(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaRandomNumGenEntry_Unpack(&InaRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaRandomNumGenEntry_SetField(&InaRandomNumGenRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaRandomNumGenEntry_Pack(&InaRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaRandomNumGenWrite(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLINASYSPORTMAP:
      status = soc_bm9600_InaSysportMapRead(unit, uAddress, instance, (uint32 *)dbuf);
      sbZfFabBm9600InaSysportMapEntry_Unpack(&InaSysportMapRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600InaSysportMapEntry_SetField(&InaSysportMapRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600InaSysportMapEntry_Pack(&InaSysportMapRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_InaSysportMapWrite(unit, uAddress, instance, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMEGRESSRANKER:
      status = soc_bm9600_NmEgressRankerRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmEgressRankerEntry_Unpack(&NmEgressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmEgressRankerEntry_SetField(&NmEgressRankerRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmEgressRankerEntry_Pack(&NmEgressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmEgressRankerWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMFULLSTATUS:
      status = soc_bm9600_NmFullStatusRead(unit, uAddress,
                                          (uint32 *)&dbuf[0], (uint32 *)&dbuf[1],
                                          (uint32 *)&dbuf[2], (uint32 *)&dbuf[3],
                                          (uint32 *)&dbuf[4], (uint32 *)&dbuf[5],
                                          (uint32 *)&dbuf[6], (uint32 *)&dbuf[7],
                                          (uint32 *)&dbuf[8]);
      sbZfFabBm9600NmFullStatusEntry_Unpack(&NmFullStatusRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmFullStatusEntry_SetField(&NmFullStatusRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmFullStatusEntry_Pack(&NmFullStatusRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmFullStatusWrite(unit, uAddress,
                                            (uint32)dbuf[0], (uint32)dbuf[1],
                                            (uint32)dbuf[2], (uint32)dbuf[3],
                                            (uint32)dbuf[4], (uint32)dbuf[5],
                                            (uint32)dbuf[6], (uint32)dbuf[7],
                                            (uint32)dbuf[8]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMEMT:
      status = soc_bm9600_NmEmtRead(unit, uAddress, (uint32 *)dbuf,
				    (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      sbZfFabBm9600NmEmtEntry_Unpack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmEmtEntry_SetField(&NmEmtRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmEmtEntry_Pack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmEmtWrite(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1], (uint32)dbuf[2]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMEMT_HW:
      status = soc_bm9600_HwNmEmtRead(unit, uAddress, (uint32 *)dbuf,
				    (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      sbZfFabBm9600NmEmtEntry_Unpack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmEmtEntry_SetField(&NmEmtRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmEmtEntry_Pack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmEmtWrite(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1], (uint32)dbuf[2]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMEMTDEBUGBANK0:
      status = soc_bm9600_NmEmtdebugbank0Read(unit, uAddress, (uint32 *)dbuf,
					      (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      sbZfFabBm9600NmEmtdebugbank0Entry_Unpack(&NmEmtdebugbank0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmEmtdebugbank0Entry_SetField(&NmEmtdebugbank0Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmEmtdebugbank0Entry_Pack(&NmEmtdebugbank0Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmEmtdebugbank0Write(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1], (uint32)dbuf[2]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMEMTDEBUGBANK1:
      status = soc_bm9600_NmEmtdebugbank1Read(unit, uAddress, (uint32 *)dbuf,
					      (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      sbZfFabBm9600NmEmtdebugbank1Entry_Unpack(&NmEmtdebugbank1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmEmtdebugbank1Entry_SetField(&NmEmtdebugbank1Record, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmEmtdebugbank1Entry_Pack(&NmEmtdebugbank1Record,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmEmtdebugbank1Write(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1], (uint32)dbuf[2]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMINGRESSRANKER:
      status = soc_bm9600_NmIngressRankerRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmIngressRankerEntry_Unpack(&NmIngressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmIngressRankerEntry_SetField(&NmIngressRankerRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmIngressRankerEntry_Pack(&NmIngressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmIngressRankerWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMPORTSETINFO:
      status = soc_bm9600_NmPortsetInfoRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmPortsetInfoEntry_Unpack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmPortsetInfoEntry_SetField(&NmPortsetInfoRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmPortsetInfoEntry_Pack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmPortsetInfoWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMPORTSETINFO_HW:
      status = soc_bm9600_HwNmPortsetInfoRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmPortsetInfoEntry_Unpack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmPortsetInfoEntry_SetField(&NmPortsetInfoRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmPortsetInfoEntry_Pack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmPortsetInfoWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMPORTSETLINK:
      status = soc_bm9600_NmPortsetLinkRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmPortsetLinkEntry_Unpack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmPortsetLinkEntry_SetField(&NmPortsetLinkRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmPortsetLinkEntry_Pack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmPortsetLinkWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMPORTSETLINK_HW:
      status = soc_bm9600_HwNmPortsetLinkRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmPortsetLinkEntry_Unpack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmPortsetLinkEntry_SetField(&NmPortsetLinkRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmPortsetLinkEntry_Pack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmPortsetLinkWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMRANDOMNUMGEN:
      status = soc_bm9600_NmRandomNumGenRead(unit, uAddress, (uint32 *)dbuf);
      sbZfFabBm9600NmRandomNumGenEntry_Unpack(&NmRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmRandomNumGenEntry_SetField(&NmRandomNumGenRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmRandomNumGenEntry_Pack(&NmRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmRandomNumGenWrite(unit, uAddress, (uint32)dbuf[0]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
    case BM9600_PLNMSYSPORTARRAY:
      status = soc_bm9600_NmSysportArrayRead(unit, uAddress, (uint32 *)dbuf,
					     (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					     (uint32 *)&dbuf[3], (uint32 *)&dbuf[4],
					     (uint32 *)&dbuf[5]);
      sbZfFabBm9600NmSysportArrayEntry_Unpack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmSysportArrayEntry_SetField(&NmSysportArrayRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmSysportArrayEntry_Pack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmSysportArrayWrite(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1],
					      (uint32)dbuf[2], (uint32)dbuf[3], (uint32)dbuf[4], (uint32)dbuf[5]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMSYSPORTARRAY_HW:
      status = soc_bm9600_HwNmSysportArrayRead(unit, uAddress, (uint32 *)dbuf,
                                             (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
                                             (uint32 *)&dbuf[3], (uint32 *)&dbuf[4],
                                             (uint32 *)&dbuf[5]);
      sbZfFabBm9600NmSysportArrayEntry_Unpack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = sbZfFabBm9600NmSysportArrayEntry_SetField(&NmSysportArrayRecord, fieldname, val);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_FIELD;
      sbZfFabBm9600NmSysportArrayEntry_Pack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
      status = soc_bm9600_NmSysportArrayWrite(unit, uAddress, (uint32)dbuf[0], (uint32)dbuf[1],
					      (uint32)dbuf[2], (uint32)dbuf[3], (uint32)dbuf[4], (uint32)dbuf[5]);
      if (status != BM9600_STATUS_OK) return BM9600_STATUS_BAD_WRITE;      
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLXBXCFGREMAP:
      status = soc_bm9600_XbXcfgRemapSelectRead(unit, uAddress, instance,
						(uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600XbXcfgRemapEntry_Unpack(&XbXcfgRemapRecord,(uint8 *)dbuf,sizeof(dbuf));
	  status = sbZfFabBm9600XbXcfgRemapEntry_SetField(&XbXcfgRemapRecord, fieldname, val);
	  if (status == BM9600_STATUS_OK) {
	      sbZfFabBm9600XbXcfgRemapEntry_Pack(&XbXcfgRemapRecord,(uint8 *)dbuf,sizeof(dbuf));
	      status = soc_bm9600_XbXcfgRemapSelectWrite( unit, uAddress, instance, dbuf[0]);
	  }
      }
    break;
    
    default:
        status = BM9600_STATUS_BAD_READ;
    break;
    }
    
    return status;
}

/*
 * sbBm9600MemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm9600MemSetField(int unit, char *memFieldName, int addr, int val, int instance)
{
    int i, memindex, status, doingfield, max;
    char *dptr, curch, memname[100], tmpfield[100], memfield[100];

    sal_memset(tmpfield, 0x0, sizeof(tmpfield));

   /* memfield comes in as mem.field, so split memname and field name */
    doingfield = 0;    /* needed to convert field name to lower chars */
    dptr = &memname[0];
    for(i = 0; i < 100; i++) {
        curch = memFieldName[i];
        if (memFieldName[i] == '\0') {
            *dptr = '\0';
            break;
        }
        if (curch == '.') {
            *dptr = '\0';
            dptr = &tmpfield[0];
            doingfield = 1;
        }
        else {
            if (doingfield)
                *dptr++ = tolower((unsigned char)curch);
            else
                *dptr++ = curch;
        }
    }

    memindex = sbBm9600WhichMem(memname);
    if(memindex == -1) {
        cli_out("Error: unrecognized Bm9600 memory: %s\n",memname);
        cli_out("Please pick from one of these:\n");
        sbBm9600ShowMemNames();
        return CMD_FAIL;
    }
    max = sbBm9600MemMax(memindex);
    if (addr >= max){
        cli_out("Error: Addr %d out of range for memory: %s max=%d\n",
                addr,memname,max);
        return CMD_FAIL;
    }
    if (bm9k6MemConfigTable[memindex].instances != 0 &&
	(instance < 0 || instance >= bm9k6MemConfigTable[memindex].instances)) {
	cli_out("Error: Mem %s is instanced and does not contain instance %d\n",memname, instance);
    }

    if ( ((memindex == BM9600_PLNMEMT) ||
	  (memindex == BM9600_PLNMEMTDEBUGBANK0) ||
	  (memindex == BM9600_PLNMEMTDEBUGBANK1)) &&
	 (SB_STRCMP(tmpfield, "esetmember0")==0) ) {
	SB_STRCPY(memfield, "m_uu");
    } else {
	SB_STRCPY(memfield, "m_u");
    }
    SB_STRCAT(memfield, tmpfield);

    cli_out("MemSetField unit=%d memIndex =%d instance=%d\n",unit,memindex, instance);

    status = sbBm9600MemSetFieldEntry(unit, memindex,addr,memfield, val, instance);
    switch(status) {
        case  BM9600_STATUS_BAD_READ:
           cli_out("Error Reading Bm9600 memory\n");
           break;
        case  BM9600_STATUS_BAD_WRITE:
           cli_out("Error Writing Bm9600 memory\n");
           break;
        case  BM9600_STATUS_BAD_FIELD:
           cli_out("Error: unrecognized field <%s> for this Bm9600 memory: %s\n",&memfield[3],memname);
           break;
    }
    if (status != BM9600_STATUS_OK) {
        status = CMD_USAGE;
    }
    return status;
}

#endif
