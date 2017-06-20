/*
 * $Id: sbBm9600MemAccess.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbBm9600MemAccess.c
 * Purpose:     sbx commands to read/write BM9600 indirect mems
 * Requires:
 */

#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <soc/defs.h>

#ifdef BCM_BM9600_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_mem_access.h>
#include <bcm/error.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/imfswap32.h>

extern void bmSwapWords(uint *pdata, int nwords);

bm9k6MemConfigRec bm9k6MemConfigTable[] =
{
    {"PlBwAllocCfgBaseEntry", BM9600_PLBWALLOCCFGBASE, 1, 0},
    {"PlBwAllocRateEntry", BM9600_PLBWALLOCRATE, 384, 0},
    {"PlBwFetchDataEntry", BM9600_PLBWFETCHDATA, 384, 0},
    {"PlBwFetchSumEntry", BM9600_PLBWFETCHSUM, 16, 0},
    {"PlBwFetchValidEntry", BM9600_PLBWFETCHVALID, 8, 0},
    {"PlBwR0BagEntry", BM9600_PLBWR0BAG, 4096, 0},
    {"PlBwR0BwpEntry", BM9600_PLBWR0BWP, 16384, 0},
    {"PlBwR0WdtEntry", BM9600_PLBWR0WDT, 8192, 0},
    {"PlBwR1BagEntry", BM9600_PLBWR1BAG, 4096, 0},
    {"PlBwR1Wct0AEntry", BM9600_PLBWR1WCT0A, 256, 0},
    {"PlBwR1Wct0BEntry", BM9600_PLBWR1WCT0B, 256, 0},
    {"PlBwR1Wct1AEntry", BM9600_PLBWR1WCT1A, 256, 0},
    {"PlBwR1Wct1BEntry", BM9600_PLBWR1WCT1B, 256, 0},
    {"PlBwR1Wct2AEntry", BM9600_PLBWR1WCT2A, 256, 0},
    {"PlBwR1Wct2BEntry", BM9600_PLBWR1WCT2B, 256, 0},
    {"PlBwR1WstEntry", BM9600_PLBWR1WST, 16384, 0},
    {"PlBwWredCfgBaseEntry", BM9600_PLBWWREDCFGBASE, 1, 0},
    {"PlBwWredDropNPart1Entry", BM9600_PLBWWREDDROPNPART1, 2, 0},
    {"PlBwWredDropNPart2Entry", BM9600_PLBWWREDDROPNPART2, 2, 0},
    {"PlFoLinkStateTableEntry", BM9600_PLFOLINKSTATETABLE, 72, 0},
    {"PlInaEsetPriEntry", BM9600_PLINAESETPRI, 64, 192},
    {"PlInaHi1Selected_0Entry", BM9600_PLINAHI1SELECTED_0, 72, 192},
    {"PlInaHi1Selected_1Entry", BM9600_PLINAHI1SELECTED_1, 72, 192},
    {"PlInaHi2Selected_0Entry", BM9600_PLINAHI2SELECTED_0, 72, 192},
    {"PlInaHi2Selected_1Entry", BM9600_PLINAHI2SELECTED_1, 72, 192},
    {"PlInaHi3Selected_0Entry", BM9600_PLINAHI3SELECTED_0, 72, 192},
    {"PlInaHi3Selected_1Entry", BM9600_PLINAHI3SELECTED_1, 72, 192},
    {"PlInaHi4Selected_0Entry", BM9600_PLINAHI4SELECTED_0, 72, 192},
    {"PlInaHi4Selected_1Entry", BM9600_PLINAHI4SELECTED_1, 72, 192},
    {"PlInaPortPriEntry", BM9600_PLINAPORTPRI, 176, 192},
    {"PlInaRandomNumGenEntry", BM9600_PLINARANDOMNUMGEN, 55, 192},
    {"PlInaSysportMapEntry", BM9600_PLINASYSPORTMAP, 2816, 192},
    {"PlNmEgressRankerEntry", BM9600_PLNMEGRESSRANKER, 73, 0},
    {"PlNmFullStatusEntry", BM9600_PLNMFULLSTATUS, 72, 0},
    {"PlNmEmtEntry", BM9600_PLNMEMT, 1024, 0},
    {"PlNmEmtdebugbank0Entry", BM9600_PLNMEMTDEBUGBANK0, 1024, 0},
    {"PlNmEmtdebugbank1Entry", BM9600_PLNMEMTDEBUGBANK1, 1024, 0},
    {"PlNmIngressRankerEntry", BM9600_PLNMINGRESSRANKER, 73, 0},
    {"PlNmPortsetInfoEntry", BM9600_PLNMPORTSETINFO, 176, 0},
    {"PlNmPortsetLinkEntry", BM9600_PLNMPORTSETLINK, 176, 0},
    {"PlNmRandomNumGenEntry", BM9600_PLNMRANDOMNUMGEN, 55, 0},
    {"PlNmSysportArrayEntry", BM9600_PLNMSYSPORTARRAY, 176, 0},
    {"PlXbXcfgRemapEntry", BM9600_PLXBXCFGREMAP, 72, 192},
#ifndef _NO_NM_CACHE_FIX_
    {"PlNmEmtHwEntry", BM9600_PLNMEMT_HW, 1024},
    {"PlNmPortsetInfoHwEntry", BM9600_PLNMPORTSETINFO_HW, 176},
    {"PlNmPortsetLinkHwEntry", BM9600_PLNMPORTSETLINK_HW, 176},
    {"PlNmSysportArrayHwEntry", BM9600_PLNMSYSPORTARRAY_HW, 176},
#endif /* _NO_NM_CACHE_FIX_ */
    
    {"END", BM9600_MEM_MAX_INDEX, 0xff, 0},
};

/*
 * just return for big endian 
 */
void bmSwapWords(uint *pdata, int nwords)
{
  int i;

   for(i = 0; i < nwords; i++) {
       *pdata = IMFSWAP32(*pdata);
       pdata++;
   }
}


/*
 * scan bm9k6MemConfigTable for designated name and return memIndex
 */
int
sbBm9600WhichMem(char *memname)
{
   int i;

   for(i = 0; i < BM9600_MEM_MAX_INDEX; i++) {
       if(strcmp(memname,bm9k6MemConfigTable[i].memname) == 0)
          return bm9k6MemConfigTable[i].memindex;
   }
   return -1;

}

int 
sbBm9600MemMax(int memindex)
{
    return bm9k6MemConfigTable[memindex].rangemax;
}

void
sbBm9600ShowMemNames()
{
   int i;

   for(i = 0; i < BM9600_MEM_MAX_INDEX; i++) {
       cli_out("--- %s\n",bm9k6MemConfigTable[i].memname);
   }
}





/*
 * sbBm9600MemShowEntry - read and display contents of BM9600 indirect memory
 *	inputs:	 unit 
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm9600MemShowEntry(int unit, int memindex, int entryindex, int instance)
{
  int  status = BCM_E_NOT_FOUND;

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
  sbZfFabBm9600NmEmtEntry_t  NmEmtRecord;
  sbZfFabBm9600NmEmtdebugbank0Entry_t  NmEmtdebugbank0Record;
  sbZfFabBm9600NmEmtdebugbank1Entry_t  NmEmtdebugbank1Record;
  sbZfFabBm9600NmIngressRankerEntry_t  NmIngressRankerRecord;
  sbZfFabBm9600NmPortsetInfoEntry_t  NmPortsetInfoRecord;
  sbZfFabBm9600NmPortsetLinkEntry_t  NmPortsetLinkRecord;
  sbZfFabBm9600NmRandomNumGenEntry_t  NmRandomNumGenRecord;
  sbZfFabBm9600NmSysportArrayEntry_t  NmSysportArrayRecord;
  sbZfFabBm9600XbXcfgRemapEntry_t  XbXcfgRemapRecord;
  sbZfFabBm9600NmFullStatusEntry_t NmFullStatusRecord;

   uint dbuf[40];
   uint    uAddress, *pData  = &dbuf[0];

    uAddress = entryindex;
    cli_out("---  Entry index: %d ---\n",entryindex);
    switch(memindex) {

    case BM9600_PLBWALLOCCFGBASE:
      status = soc_bm9600_BwAllocCfgBaseRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwAllocCfgBaseEntry_Unpack(&BwAllocCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwAllocCfgBaseEntry_Print(&BwAllocCfgBaseRecord);
      }
    break;
    case BM9600_PLBWALLOCRATE:
      status = soc_bm9600_BwAllocRateRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwAllocRateEntry_Unpack(&BwAllocRateRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwAllocRateEntry_Print(&BwAllocRateRecord);
      }
    break;
    case BM9600_PLBWFETCHDATA:
      status = soc_bm9600_BwFetchDataRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwFetchDataEntry_Unpack(&BwFetchDataRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwFetchDataEntry_Print(&BwFetchDataRecord);
      }
    break;
    case BM9600_PLBWFETCHSUM:
      status = soc_bm9600_BwFetchSumRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwFetchSumEntry_Unpack(&BwFetchSumRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwFetchSumEntry_Print(&BwFetchSumRecord);
      }
    break;
    case BM9600_PLBWFETCHVALID:
      status = soc_bm9600_BwFetchValidRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwFetchValidEntry_Unpack(&BwFetchValidRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwFetchValidEntry_Print(&BwFetchValidRecord);
      }
    break;
    case BM9600_PLBWR0BAG:
      status = soc_bm9600_BwR0BagRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR0BagEntry_Unpack(&BwR0BagRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR0BagEntry_Print(&BwR0BagRecord);
      }
    break;
    case BM9600_PLBWR0BWP:
      status = soc_bm9600_BwR0BwpRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR0BwpEntry_Unpack(&BwR0BwpRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR0BwpEntry_Print(&BwR0BwpRecord);
      }
    break;
    case BM9600_PLBWR0WDT:
      status = soc_bm9600_BwR0WdtRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR0WdtEntry_Unpack(&BwR0WdtRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR0WdtEntry_Print(&BwR0WdtRecord);
      }
    break;
    case BM9600_PLBWR1BAG:
      status = soc_bm9600_BwR1BagRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1BagEntry_Unpack(&BwR1BagRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1BagEntry_Print(&BwR1BagRecord);
      }
    break;
    case BM9600_PLBWR1WCT0A:
      status = soc_bm9600_BwR1Wct0ARead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct0AEntry_Unpack(&BwR1Wct0ARecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct0AEntry_Print(&BwR1Wct0ARecord);
      }
    break;
    case BM9600_PLBWR1WCT0B:
      status = soc_bm9600_BwR1Wct0BRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct0BEntry_Unpack(&BwR1Wct0BRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct0BEntry_Print(&BwR1Wct0BRecord);
      }
    break;
    case BM9600_PLBWR1WCT1A:
      status = soc_bm9600_BwR1Wct1ARead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct1AEntry_Unpack(&BwR1Wct1ARecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct1AEntry_Print(&BwR1Wct1ARecord);
      }
    break;
    case BM9600_PLBWR1WCT1B:
      status = soc_bm9600_BwR1Wct1BRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct1BEntry_Unpack(&BwR1Wct1BRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct1BEntry_Print(&BwR1Wct1BRecord);
      }
    break;
    case BM9600_PLBWR1WCT2A:
      status = soc_bm9600_BwR1Wct2ARead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct2AEntry_Unpack(&BwR1Wct2ARecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct2AEntry_Print(&BwR1Wct2ARecord);
      }
    break;
    case BM9600_PLBWR1WCT2B:
      status = soc_bm9600_BwR1Wct2BRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1Wct2BEntry_Unpack(&BwR1Wct2BRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1Wct2BEntry_Print(&BwR1Wct2BRecord);
      }
    break;
    case BM9600_PLBWR1WST:
      status = soc_bm9600_BwR1WstRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwR1WstEntry_Unpack(&BwR1WstRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwR1WstEntry_Print(&BwR1WstRecord);
      }
    break;
    case BM9600_PLBWWREDCFGBASE:
      status = soc_bm9600_BwWredCfgBaseRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwWredCfgBaseEntry_Unpack(&BwWredCfgBaseRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwWredCfgBaseEntry_Print(&BwWredCfgBaseRecord);
      }
    break;
    case BM9600_PLBWWREDDROPNPART1:
      status = soc_bm9600_BwWredDropNPart1Read(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwWredDropNPart1Entry_Unpack(&BwWredDropNPart1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwWredDropNPart1Entry_Print(&BwWredDropNPart1Record);
      }
    break;
    case BM9600_PLBWWREDDROPNPART2:
      status = soc_bm9600_BwWredDropNPart2Read(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600BwWredDropNPart2Entry_Unpack(&BwWredDropNPart2Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600BwWredDropNPart2Entry_Print(&BwWredDropNPart2Record);
      }
    break;
    case BM9600_PLFOLINKSTATETABLE:
      status = soc_bm9600_FoLinkStateTableRead(unit, uAddress, (uint32 *)dbuf,
					       (uint32 *)&dbuf[1]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600FoLinkStateTableEntry_Unpack(&FoLinkStateTableRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600FoLinkStateTableEntry_Print(&FoLinkStateTableRecord);
      }
    break;
    case BM9600_PLINAESETPRI:
      status = soc_bm9600_InaEsetPriRead(unit, uAddress, instance, (uint32 *)dbuf,
					 (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					 (uint32 *)&dbuf[3]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaEsetPriEntry_Unpack(&InaEsetPriRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaEsetPriEntry_Print(&InaEsetPriRecord);
      }
    break;
    case BM9600_PLINAHI1SELECTED_0:
      status = soc_bm9600_InaHi1Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi1Selected_0Entry_Unpack(&InaHi1Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi1Selected_0Entry_Print(&InaHi1Selected_0Record);
      }
    break;
    case BM9600_PLINAHI1SELECTED_1:
      status = soc_bm9600_InaHi1Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi1Selected_1Entry_Unpack(&InaHi1Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi1Selected_1Entry_Print(&InaHi1Selected_1Record);
      }
    break;
    case BM9600_PLINAHI2SELECTED_0:
      status = soc_bm9600_InaHi2Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi2Selected_0Entry_Unpack(&InaHi2Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi2Selected_0Entry_Print(&InaHi2Selected_0Record);
      }
    break;
    case BM9600_PLINAHI2SELECTED_1:
      status = soc_bm9600_InaHi2Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi2Selected_1Entry_Unpack(&InaHi2Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi2Selected_1Entry_Print(&InaHi2Selected_1Record);
      }
    break;
    case BM9600_PLINAHI3SELECTED_0:
      status = soc_bm9600_InaHi3Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi3Selected_0Entry_Unpack(&InaHi3Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi3Selected_0Entry_Print(&InaHi3Selected_0Record);
      }
    break;
    case BM9600_PLINAHI3SELECTED_1:
      status = soc_bm9600_InaHi3Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi3Selected_1Entry_Unpack(&InaHi3Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi3Selected_1Entry_Print(&InaHi3Selected_1Record);
      }
    break;
    case BM9600_PLINAHI4SELECTED_0:
      status = soc_bm9600_InaHi4Selected_0Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi4Selected_0Entry_Unpack(&InaHi4Selected_0Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi4Selected_0Entry_Print(&InaHi4Selected_0Record);
      }
    break;
    case BM9600_PLINAHI4SELECTED_1:
      status = soc_bm9600_InaHi4Selected_1Read(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaHi4Selected_1Entry_Unpack(&InaHi4Selected_1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaHi4Selected_1Entry_Print(&InaHi4Selected_1Record);
      }
    break;
    case BM9600_PLINAPORTPRI:
      status = soc_bm9600_InaPortPriRead(unit, uAddress, instance, (uint32 *)dbuf,
					 (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					 (uint32 *)&dbuf[3]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaPortPriEntry_Unpack(&InaPortPriRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaPortPriEntry_Print(&InaPortPriRecord);
      }
    break;
    case BM9600_PLINARANDOMNUMGEN:
      status = soc_bm9600_InaRandomNumGenRead(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaRandomNumGenEntry_Unpack(&InaRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaRandomNumGenEntry_Print(&InaRandomNumGenRecord);
      }
    break;
    case BM9600_PLINASYSPORTMAP:
      status = soc_bm9600_InaSysportMapRead(unit, uAddress, instance, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600InaSysportMapEntry_Unpack(&InaSysportMapRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600InaSysportMapEntry_Print(&InaSysportMapRecord);
      }
    break;
    case BM9600_PLNMEGRESSRANKER:
      status = soc_bm9600_NmEgressRankerRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmEgressRankerEntry_Unpack(&NmEgressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmEgressRankerEntry_Print(&NmEgressRankerRecord);
      }
    break;
    case BM9600_PLNMFULLSTATUS:
      status = soc_bm9600_NmFullStatusRead(unit, uAddress,
                             (uint32 *)&dbuf[0], (uint32 *)&dbuf[1],
                             (uint32 *)&dbuf[2], (uint32 *)&dbuf[3],
                             (uint32 *)&dbuf[4], (uint32 *)&dbuf[5],
                             (uint32 *)&dbuf[6], (uint32 *)&dbuf[7],
                             (uint32 *)&dbuf[8]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmFullStatusEntry_Unpack(&NmFullStatusRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmFullStatusEntry_Print(&NmFullStatusRecord);
      }
    break;
    case BM9600_PLNMEMT:
      status = soc_bm9600_NmEmtRead(unit, uAddress, (uint32 *)dbuf,
				    (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmEmtEntry_Unpack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmEmtEntry_Print(&NmEmtRecord);
      }
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMEMT_HW:
      status = soc_bm9600_HwNmEmtRead(unit, uAddress, (uint32 *)dbuf,
                                    (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmEmtEntry_Unpack(&NmEmtRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmEmtEntry_Print(&NmEmtRecord);
      }
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMEMTDEBUGBANK0:
      status = soc_bm9600_NmEmtdebugbank0Read(unit, uAddress, (uint32 *)dbuf,
					      (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmEmtdebugbank0Entry_Unpack(&NmEmtdebugbank0Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmEmtdebugbank0Entry_Print(&NmEmtdebugbank0Record);
      }
    break;
    case BM9600_PLNMEMTDEBUGBANK1:
      status = soc_bm9600_NmEmtdebugbank1Read(unit, uAddress, (uint32 *)dbuf,
					      (uint32 *)&dbuf[1], (uint32 *)&dbuf[2]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmEmtdebugbank1Entry_Unpack(&NmEmtdebugbank1Record,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmEmtdebugbank1Entry_Print(&NmEmtdebugbank1Record);
      }
    break;
    case BM9600_PLNMINGRESSRANKER:
      status = soc_bm9600_NmIngressRankerRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmIngressRankerEntry_Unpack(&NmIngressRankerRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmIngressRankerEntry_Print(&NmIngressRankerRecord);
      }
    break;
    case BM9600_PLNMPORTSETINFO:
      status = soc_bm9600_NmPortsetInfoRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmPortsetInfoEntry_Unpack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmPortsetInfoEntry_Print(&NmPortsetInfoRecord);
      }
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMPORTSETINFO_HW:
      status = soc_bm9600_HwNmPortsetInfoRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmPortsetInfoEntry_Unpack(&NmPortsetInfoRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmPortsetInfoEntry_Print(&NmPortsetInfoRecord);
      }
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMPORTSETLINK:
      status = soc_bm9600_NmPortsetLinkRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmPortsetLinkEntry_Unpack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmPortsetLinkEntry_Print(&NmPortsetLinkRecord);
      }
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMPORTSETLINK_HW:
      status = soc_bm9600_HwNmPortsetLinkRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmPortsetLinkEntry_Unpack(&NmPortsetLinkRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmPortsetLinkEntry_Print(&NmPortsetLinkRecord);
      }
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLNMRANDOMNUMGEN:
      status = soc_bm9600_NmRandomNumGenRead(unit, uAddress, (uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmRandomNumGenEntry_Unpack(&NmRandomNumGenRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmRandomNumGenEntry_Print(&NmRandomNumGenRecord);
      }
    break;
    case BM9600_PLNMSYSPORTARRAY:
      status = soc_bm9600_NmSysportArrayRead(unit, uAddress, (uint32 *)dbuf,
					     (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
					     (uint32 *)&dbuf[3], (uint32 *)&dbuf[4],
					     (uint32 *)&dbuf[5]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmSysportArrayEntry_Unpack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmSysportArrayEntry_Print(&NmSysportArrayRecord);
      }
    break;
#ifndef _NO_NM_CACHE_FIX_
    case BM9600_PLNMSYSPORTARRAY_HW:
      status = soc_bm9600_HwNmSysportArrayRead(unit, uAddress, (uint32 *)dbuf,
                                             (uint32 *)&dbuf[1], (uint32 *)&dbuf[2],
                                             (uint32 *)&dbuf[3], (uint32 *)&dbuf[4],
                                             (uint32 *)&dbuf[5]);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600NmSysportArrayEntry_Unpack(&NmSysportArrayRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600NmSysportArrayEntry_Print(&NmSysportArrayRecord);
      }
    break;
#endif /* _NO_NM_CACHE_FIX_ */
    case BM9600_PLXBXCFGREMAP:
      status = soc_bm9600_XbXcfgRemapSelectRead(unit, uAddress, instance,
						(uint32 *)dbuf);
      if (status == BM9600_STATUS_OK) {
          sbZfFabBm9600XbXcfgRemapEntry_Unpack(&XbXcfgRemapRecord,(uint8 *)dbuf,sizeof(dbuf));
          sbZfFabBm9600XbXcfgRemapEntry_Print(&XbXcfgRemapRecord);
      }
    break;
    }

    if (status != BM9600_STATUS_OK) {
            bmSwapWords(pData,1);
        cli_out("Error %d while reading Bm9600 memory\n",status);
    } 
    return 0;
}

/*
 * sbBm9600MemShowRange - call ShowEntry for a range of entries
 *	inputs:	unit
 *		index of memory 
 *              start of range
 *              end of range
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm9600MemShowRange(int unit, int memindex, int rangemin, int rangemax, int instance)
{
    int i,  status;

    for(i = rangemin; i <= rangemax; i++) {
        status = sbBm9600MemShowEntry(unit, memindex, i, instance);
        if(status != 0)
            return status;
    }
    return 0;
}

/*
 * sbBm9600MemShow - read and display contents of BM9600 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm9600MemShow(int unit, char *memname, int rangemin, int rangemax, int instance)
{
    int i, cnt, len, tempmax, tempinstance;
    int status = CMD_FAIL;

    cnt = 0;
    len = strlen(memname);
    for(i = 0; i < BM9600_MEM_MAX_INDEX; i++) {
        if ((memname[0] == '*') || (strncmp(memname,bm9k6MemConfigTable[i].memname,len) == 0)) {
             cnt++;
             cli_out("------------- %s ------------\n",bm9k6MemConfigTable[i].memname);
             tempmax = rangemax;
             if (rangemax >= bm9k6MemConfigTable[i].rangemax){
                  tempmax = bm9k6MemConfigTable[i].rangemax-1;
                  cli_out("Max range for mem %s is %d\n",memname,tempmax);
		  break;
             }
	     tempinstance = instance;
             if (bm9k6MemConfigTable[i].instances != 0 &&
		 (instance < 0 || instance >= bm9k6MemConfigTable[i].instances)) {
                  tempinstance = 0;
                  cli_out("Warning: Mem %s is instanced: showing instance %d\n",memname,tempinstance);
             }
             status = sbBm9600MemShowRange(unit,bm9k6MemConfigTable[i].memindex,rangemin,tempmax,tempinstance);
	     break;
        }
    }
    if(cnt == 0) {
        cli_out("Error: unrecognized BM9600 memory: %s\n",memname);
        cli_out("Please pick from one of these:\n");
        sbBm9600ShowMemNames();
    } 
    return status;
}

#endif /* BCM_BM9600_SUPPORT */
