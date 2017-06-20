
/*
 * $Id: sbQe2000MemWrite.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbQe2000MemWrites.c
 * Purpose:     sbx commands to write fe2000 indirect mems
 * Requires:
 */

#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <soc/defs.h>

#ifdef BCM_QE2000_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/qe2k/KaminoDriver.h>

#include <soc/sbx/qe2k/sbZfQeIncludes.h>
#include <soc/sbx/qe2k/qe2k.h>
 
/*
 * note: swap utilities used from fe2000 library 
 */
extern int sbQe2000WhichMem(char *memname);
extern int sbQe2000MemMax(int memindex);
extern void qeSwapWords(uint *pdata, int nwords);
extern qe2kMemConfigRec qe2kMemConfigTable[2];

/*
 * sbQe2000MemSetFieldEntry - read and change contents of qe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbQe2000MemSetFieldEntry(int unit, int memIndex, int uAddress, char *fieldname, int val)
{
   int  status, uTableId;
   int  spi = 0;      /* FIX THIS */
   uint32 dbuf[40];
   uint32     *pData  = &dbuf[0];

    sbZfKaQsLastSentPriEntry_t       sbZfKaQsLastSentPriEntryRecord;
    sbZfKaQmQueueArrivalsEntry_t     sbZfKaQmQueueArrivalsEntryRecord;
    sbZfKaQmWredParamEntry_t         sbZfKaQmWredParamEntryRecord;
    sbZfKaRbClassPortEnablesEntry_t  sbZfKaRbClassPortEnablesEntryRecord;
    sbZfKaQmPortBwCfgTableEntry_t    sbZfKaQmPortBwCfgTableEntryRecord;
    sbZfKaQsLastSentPriAddr_t        sbZfKaQsLastSentPriAddrRecord;
    sbZfKaEpCrTableEntry_t           sbZfKaEpCrTableEntryRecord;
    sbZfKaEpIpPortVridSmacTableEntry_t     sbZfKaEpIpPortVridSmacTableEntryRecord;
    sbZfKaQsShapeRateEntry_t        sbZfKaQsShapeRateEntryRecord;
    sbZfKaQsLnaPortRemapEntry_t     sbZfKaQsLnaPortRemapEntryRecord;
    sbZfKaEgMemFifoParamEntry_t     sbZfKaEgMemFifoParamEntryRecord;
    sbZfKaQsShapeTableEntry_t       sbZfKaQsShapeTableEntryRecord;
    sbZfKaQmQueueAgeEntry_t         sbZfKaQmQueueAgeEntryRecord;
    sbZfKaRbClassDefaultQEntry_t    sbZfKaRbClassDefaultQEntryRecord;
    sbZfKaQmWredCfgTableEntry_t     sbZfKaQmWredCfgTableEntryRecord;
    sbZfKaQsPriAddr_t               sbZfKaQsPriAddrRecord;
    sbZfKaRbClassHashVlanIPv6_t     sbZfKaRbClassHashVlanIPv6Record;
    sbZfKaQsShapeMaxBurstEntry_t    sbZfKaQsShapeMaxBurstEntryRecord;
    sbZfKaQmQueueStateEntry_t       sbZfKaQmQueueStateEntryRecord;
    sbZfKaQmFbLine_t     sbZfKaQmFbLineRecord;
    sbZfKaEpIp16BitRewrite_t     sbZfKaEpIp16BitRewriteRecord;
    sbZfKaQmIngressPortEntry_t     sbZfKaQmIngressPortEntryRecord;
    sbZfKaEbMvtEntry_t     sbZfKaEbMvtEntryRecord;
    sbZfKaQsRateTableEntry_t     sbZfKaQsRateTableEntryRecord;
    sbZfKaEgMemFifoControlEntry_t     sbZfKaEgMemFifoControlEntryRecord;
    sbZfKaEpIpTciDmac_t     sbZfKaEpIpTciDmacRecord;
    sbZfKaRbClassHashIPv6Only_t     sbZfKaRbClassHashIPv6OnlyRecord;
    sbZfKaEpVlanIndRecord_t     sbZfKaEpVlanIndRecordRecord;
    sbZfKaRbClassHashSVlanIPv6_t     sbZfKaRbClassHashSVlanIPv6Record;
    sbZfKaRbClassHashSVlanIPv4_t     sbZfKaRbClassHashSVlanIPv4Record;
    sbZfKaRbPolicePortQMapEntry_t     sbZfKaRbPolicePortQMapEntryRecord;
    sbZfKaRbPoliceCBSEntry_t     sbZfKaRbPoliceCBSEntryRecord;
    sbZfKaEpIpV6Tci_t     sbZfKaEpIpV6TciRecord;
    sbZfKaEpIpMplsLabels_t     sbZfKaEpIpMplsLabelsRecord;
    sbZfKaEpBfPriTableAddr_t     sbZfKaEpBfPriTableAddrRecord;
    sbZfKaQsE2QEntry_t     sbZfKaQsE2QEntryRecord;
    sbZfKaRbPoliceCfgCtrlEntry_t     sbZfKaRbPoliceCfgCtrlEntryRecord;
    sbZfKaQsQueueTableEntry_t     sbZfKaQsQueueTableEntryRecord;
    sbZfKaQmBaaCfgTableEntry_t     sbZfKaQmBaaCfgTableEntryRecord;
    sbZfKaQmWredCurvesTableEntry_t     sbZfKaQmWredCurvesTableEntryRecord;
    sbZfKaRbClassProtocolEntry_t     sbZfKaRbClassProtocolEntryRecord;
    sbZfKaQmWredAvQlenTableEntry_t     sbZfKaQmWredAvQlenTableEntryRecord;
    sbZfKaEpIpPriExpTosRewrite_t     sbZfKaEpIpPriExpTosRewriteRecord;
    sbZfKaPmLastLine_t     sbZfKaPmLastLineRecord;
    sbZfKaEgTmePortRemapAddr_t     sbZfKaEgTmePortRemapAddrRecord;
    sbZfKaQsAgeThreshLutEntry_t     sbZfKaQsAgeThreshLutEntryRecord;
    sbZfKaRbClassSourceIdEntry_t     sbZfKaRbClassSourceIdEntryRecord;
    sbZfKaEpVlanIndTableEntry_t     sbZfKaEpVlanIndTableEntryRecord;
    sbZfKaEgSrcId_t     sbZfKaEgSrcIdRecord;
    sbZfKaRbClassIPv6ClassEntry_t     sbZfKaRbClassIPv6ClassEntryRecord;
    sbZfKaEpIpTtlRange_t     sbZfKaEpIpTtlRangeRecord;
    sbZfKaRbClassDmacMatchEntry_t     sbZfKaRbClassDmacMatchEntryRecord;
    sbZfKaRbClassIPv4TosEntry_t     sbZfKaRbClassIPv4TosEntryRecord;
    sbZfKaRbPoliceEBSEntry_t     sbZfKaRbPoliceEBSEntryRecord;
    sbZfKaEiMemDataEntry_t     sbZfKaEiMemDataEntryRecord;
    sbZfKaQmSlqCntrsEntry_t     sbZfKaQmSlqCntrsEntryRecord;
    sbZfKaSrManualDeskewEntry_t     sbZfKaSrManualDeskewEntryRecord;
    sbZfKaEpSlimVlanRecord_t     sbZfKaEpSlimVlanRecordRecord;
    sbZfKaQsE2QAddr_t     sbZfKaQsE2QAddrRecord;
    sbZfKaEbMvtAddress_t     sbZfKaEbMvtAddressRecord;
    sbZfKaEgMemShapingEntry_t     sbZfKaEgMemShapingEntryRecord;
    sbZfKaEpPortTableEntry_t     sbZfKaEpPortTableEntryRecord;
    sbZfKaEgPortRemapEntry_t     sbZfKaEgPortRemapEntryRecord;
    sbZfKaEpIpPrepend_t     sbZfKaEpIpPrependRecord;
    sbZfKaEpInstruction_t     sbZfKaEpInstructionRecord;
    sbZfKaQsAgeEntry_t     sbZfKaQsAgeEntryRecord;
    sbZfKaEpBfPriTableEntry_t     sbZfKaEpBfPriTableEntryRecord;
    sbZfKaRbClassHashInputW0_t     sbZfKaRbClassHashInputW0Record;
    sbZfKaEgNotTmePortRemapAddr_t     sbZfKaEgNotTmePortRemapAddrRecord;
    sbZfKaEgMemQCtlEntry_t     sbZfKaEgMemQCtlEntryRecord;
    sbZfKaEiRawSpiReadEntry_t     sbZfKaEiRawSpiReadEntryRecord;
    sbZfKaQsQ2EcEntry_t     sbZfKaQsQ2EcEntryRecord;
    sbZfKaQsPriLutEntry_t     sbZfKaQsPriLutEntryRecord;
    sbZfKaEpIpCounter_t     sbZfKaEpIpCounterRecord;
    sbZfKaQmDemandCfgDataEntry_t     sbZfKaQmDemandCfgDataEntryRecord;
    sbZfKaQsQueueParamEntry_t     sbZfKaQsQueueParamEntryRecord;
    sbZfKaQsPriEntry_t     sbZfKaQsPriEntryRecord;
    sbZfKaEpIp32BitRewrite_t     sbZfKaEpIp32BitRewriteRecord;
    sbZfKaQsDepthHplenEntry_t     sbZfKaQsDepthHplenEntryRecord;
    sbZfKaRbClassHashVlanIPv4_t     sbZfKaRbClassHashVlanIPv4Record;
    sbZfKaQmQueueByteAdjEntry_t     sbZfKaQmQueueByteAdjEntryRecord;
    sbZfKaQsPriLutAddr_t     sbZfKaQsPriLutAddrRecord;

    uTableId = qe2kMemConfigTable[memIndex].tableId;
    switch(memIndex) {

    case QE2000_KAQSLASTSENTPRIENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
        sbZfKaQsLastSentPriEntry_Unpack(&sbZfKaQsLastSentPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsLastSentPriEntry_SetField(&sbZfKaQsLastSentPriEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsLastSentPriEntry_Pack(&sbZfKaQsLastSentPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;

    case QE2000_KAQMQUEUEARRIVALSENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmQueueArrivalsEntry_Unpack(&sbZfKaQmQueueArrivalsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmQueueArrivalsEntry_SetField(&sbZfKaQmQueueArrivalsEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmQueueArrivalsEntry_Pack(&sbZfKaQmQueueArrivalsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;

    case QE2000_KAQMWREDPARAMENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
        sbZfKaQmWredParamEntry_Unpack(&sbZfKaQmWredParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmWredParamEntry_SetField(&sbZfKaQmWredParamEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmWredParamEntry_Pack(&sbZfKaQmWredParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSPORTENABLESENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassPortEnablesEntry_Unpack(&sbZfKaRbClassPortEnablesEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassPortEnablesEntry_SetField(&sbZfKaRbClassPortEnablesEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassPortEnablesEntry_Pack(&sbZfKaRbClassPortEnablesEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMPORTBWCFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmPortBwCfgTableEntry_Unpack(&sbZfKaQmPortBwCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmPortBwCfgTableEntry_SetField(&sbZfKaQmPortBwCfgTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmPortBwCfgTableEntry_Pack(&sbZfKaQmPortBwCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSLASTSENTPRIADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsLastSentPriAddr_Unpack(&sbZfKaQsLastSentPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsLastSentPriAddr_SetField(&sbZfKaQsLastSentPriAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsLastSentPriAddr_Pack(&sbZfKaQsLastSentPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPCRTABLEENTRY:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpCrTableEntry_Unpack(&sbZfKaEpCrTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpCrTableEntry_SetField(&sbZfKaEpCrTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpCrTableEntry_Pack(&sbZfKaEpCrTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAEPIP_PORT_VRID_SMAC_TABLEENTRY:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpPortVridSmacTableEntry_Unpack(&sbZfKaEpIpPortVridSmacTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpPortVridSmacTableEntry_SetField(&sbZfKaEpIpPortVridSmacTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpPortVridSmacTableEntry_Pack(&sbZfKaEpIpPortVridSmacTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQSSHAPERATEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
        qeSwapWords(pData,2);
        sbZfKaQsShapeRateEntry_Unpack(&sbZfKaQsShapeRateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsShapeRateEntry_SetField(&sbZfKaQsShapeRateEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsShapeRateEntry_Pack(&sbZfKaQsShapeRateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        qeSwapWords(pData,2);
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSPORTREMAPENTRY:
        status = sandDrvKaQsMemLnaRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsLnaPortRemapEntry_Unpack(&sbZfKaQsLnaPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsLnaPortRemapEntry_SetField(&sbZfKaQsLnaPortRemapEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsLnaPortRemapEntry_Pack(&sbZfKaQsLnaPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemLnaWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEGMEMFIFOPARAMENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgMemFifoParamEntry_Unpack(&sbZfKaEgMemFifoParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgMemFifoParamEntry_SetField(&sbZfKaEgMemFifoParamEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgMemFifoParamEntry_Pack(&sbZfKaEgMemFifoParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSSHAPETABLEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsShapeTableEntry_Unpack(&sbZfKaQsShapeTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsShapeTableEntry_SetField(&sbZfKaQsShapeTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsShapeTableEntry_Pack(&sbZfKaQsShapeTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMQUEUEAGEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmQueueAgeEntry_Unpack(&sbZfKaQmQueueAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmQueueAgeEntry_SetField(&sbZfKaQmQueueAgeEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmQueueAgeEntry_Pack(&sbZfKaQmQueueAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSDEFAULTQENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassDefaultQEntry_Unpack(&sbZfKaRbClassDefaultQEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassDefaultQEntry_SetField(&sbZfKaRbClassDefaultQEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassDefaultQEntry_Pack(&sbZfKaRbClassDefaultQEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMWREDCFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmWredCfgTableEntry_Unpack(&sbZfKaQmWredCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmWredCfgTableEntry_SetField(&sbZfKaQmWredCfgTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmWredCfgTableEntry_Pack(&sbZfKaQmWredCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSPRIADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsPriAddr_Unpack(&sbZfKaQsPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsPriAddr_SetField(&sbZfKaQsPriAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsPriAddr_Pack(&sbZfKaQsPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHVLANIPV6:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashVlanIPv6_Unpack(&sbZfKaRbClassHashVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashVlanIPv6_SetField(&sbZfKaRbClassHashVlanIPv6Record, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashVlanIPv6_Pack(&sbZfKaRbClassHashVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSSHAPEMAXBURSTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsShapeMaxBurstEntry_Unpack(&sbZfKaQsShapeMaxBurstEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsShapeMaxBurstEntry_SetField(&sbZfKaQsShapeMaxBurstEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsShapeMaxBurstEntry_Pack(&sbZfKaQsShapeMaxBurstEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMQUEUESTATEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmQueueStateEntry_Unpack(&sbZfKaQmQueueStateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmQueueStateEntry_SetField(&sbZfKaQmQueueStateEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmQueueStateEntry_Pack(&sbZfKaQmQueueStateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMFBLINE:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmFbLine_Unpack(&sbZfKaQmFbLineRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmFbLine_SetField(&sbZfKaQmFbLineRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmFbLine_Pack(&sbZfKaQmFbLineRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIP16BITREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIp16BitRewrite_Unpack(&sbZfKaEpIp16BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIp16BitRewrite_SetField(&sbZfKaEpIp16BitRewriteRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIp16BitRewrite_Pack(&sbZfKaEpIp16BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KAQMINGRESSPORTENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmIngressPortEntry_Unpack(&sbZfKaQmIngressPortEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmIngressPortEntry_SetField(&sbZfKaQmIngressPortEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmIngressPortEntry_Pack(&sbZfKaQmIngressPortEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEBMVTENTRY:
        status = sandDrvKaEbMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEbMvtEntry_Unpack(&sbZfKaEbMvtEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEbMvtEntry_SetField(&sbZfKaEbMvtEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEbMvtEntry_Pack(&sbZfKaEbMvtEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEbMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQSRATETABLEA:
    case QE2000_KAQSRATETABLEB:
      /* Writing to these tables shouldn't have effect as they are calculated and refreshed by QE */
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsRateTableEntry_Unpack(&sbZfKaQsRateTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsRateTableEntry_SetField(&sbZfKaQsRateTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsRateTableEntry_Pack(&sbZfKaQsRateTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEGMEMFIFOCONTRILENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgMemFifoControlEntry_Unpack(&sbZfKaEgMemFifoControlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgMemFifoControlEntry_SetField(&sbZfKaEgMemFifoControlEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgMemFifoControlEntry_Pack(&sbZfKaEgMemFifoControlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIPTCIDMAC:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpTciDmac_Unpack(&sbZfKaEpIpTciDmacRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpTciDmac_SetField(&sbZfKaEpIpTciDmacRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpTciDmac_Pack(&sbZfKaEpIpTciDmacRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHIPV6ONLY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashIPv6Only_Unpack(&sbZfKaRbClassHashIPv6OnlyRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashIPv6Only_SetField(&sbZfKaRbClassHashIPv6OnlyRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashIPv6Only_Pack(&sbZfKaRbClassHashIPv6OnlyRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPVLANINDRECORD:
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpVlanIndRecord_Unpack(&sbZfKaEpVlanIndRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpVlanIndRecord_SetField(&sbZfKaEpVlanIndRecordRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpVlanIndRecord_Pack(&sbZfKaEpVlanIndRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHSVLANIPV6:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashSVlanIPv6_Unpack(&sbZfKaRbClassHashSVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashSVlanIPv6_SetField(&sbZfKaRbClassHashSVlanIPv6Record, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashSVlanIPv6_Pack(&sbZfKaRbClassHashSVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHSVLANIPV4:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashSVlanIPv4_Unpack(&sbZfKaRbClassHashSVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashSVlanIPv4_SetField(&sbZfKaRbClassHashSVlanIPv4Record, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashSVlanIPv4_Pack(&sbZfKaRbClassHashSVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBPOLPORTQMAPENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbPolicePortQMapEntry_Unpack(&sbZfKaRbPolicePortQMapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbPolicePortQMapEntry_SetField(&sbZfKaRbPolicePortQMapEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbPolicePortQMapEntry_Pack(&sbZfKaRbPolicePortQMapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbPolMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBPOLCBSENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbPoliceCBSEntry_Unpack(&sbZfKaRbPoliceCBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbPoliceCBSEntry_SetField(&sbZfKaRbPoliceCBSEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbPoliceCBSEntry_Pack(&sbZfKaRbPoliceCBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbPolMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIPV6TCI:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpV6Tci_Unpack(&sbZfKaEpIpV6TciRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpV6Tci_SetField(&sbZfKaEpIpV6TciRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpV6Tci_Pack(&sbZfKaEpIpV6TciRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAEPIPMPLSLABELS:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpMplsLabels_Unpack(&sbZfKaEpIpMplsLabelsRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpMplsLabels_SetField(&sbZfKaEpIpMplsLabelsRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpMplsLabels_Pack(&sbZfKaEpIpMplsLabelsRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAEPBFPRITABLEADDR:
        status = sandDrvKaEpMmBfMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpBfPriTableAddr_Unpack(&sbZfKaEpBfPriTableAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpBfPriTableAddr_SetField(&sbZfKaEpBfPriTableAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpBfPriTableAddr_Pack(&sbZfKaEpBfPriTableAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmBfMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQSE2QENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsE2QEntry_Unpack(&sbZfKaQsE2QEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsE2QEntry_SetField(&sbZfKaQsE2QEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsE2QEntry_Pack(&sbZfKaQsE2QEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBPOLCFGCTRLENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbPoliceCfgCtrlEntry_Unpack(&sbZfKaRbPoliceCfgCtrlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbPoliceCfgCtrlEntry_SetField(&sbZfKaRbPoliceCfgCtrlEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbPoliceCfgCtrlEntry_Pack(&sbZfKaRbPoliceCfgCtrlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbPolMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSQUEUETABLEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsQueueTableEntry_Unpack(&sbZfKaQsQueueTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsQueueTableEntry_SetField(&sbZfKaQsQueueTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsQueueTableEntry_Pack(&sbZfKaQsQueueTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMBAACFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmBaaCfgTableEntry_Unpack(&sbZfKaQmBaaCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmBaaCfgTableEntry_SetField(&sbZfKaQmBaaCfgTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmBaaCfgTableEntry_Pack(&sbZfKaQmBaaCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMWREDCURVESTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmWredCurvesTableEntry_Unpack(&sbZfKaQmWredCurvesTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmWredCurvesTableEntry_SetField(&sbZfKaQmWredCurvesTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmWredCurvesTableEntry_Pack(&sbZfKaQmWredCurvesTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSPROTOCOLENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassProtocolEntry_Unpack(&sbZfKaRbClassProtocolEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassProtocolEntry_SetField(&sbZfKaRbClassProtocolEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassProtocolEntry_Pack(&sbZfKaRbClassProtocolEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMWREDAVQLENTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmWredAvQlenTableEntry_Unpack(&sbZfKaQmWredAvQlenTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmWredAvQlenTableEntry_SetField(&sbZfKaQmWredAvQlenTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmWredAvQlenTableEntry_Pack(&sbZfKaQmWredAvQlenTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIPPRIEXPTOSREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpPriExpTosRewrite_Unpack(&sbZfKaEpIpPriExpTosRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpPriExpTosRewrite_SetField(&sbZfKaEpIpPriExpTosRewriteRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpPriExpTosRewrite_Pack(&sbZfKaEpIpPriExpTosRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAPMLASTLINE:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaPmLastLine_Unpack(&sbZfKaPmLastLineRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaPmLastLine_SetField(&sbZfKaPmLastLineRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaPmLastLine_Pack(&sbZfKaPmLastLineRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEGTMEPORTREMAPADDR:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgTmePortRemapAddr_Unpack(&sbZfKaEgTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgTmePortRemapAddr_SetField(&sbZfKaEgTmePortRemapAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgTmePortRemapAddr_Pack(&sbZfKaEgTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSAGETHRESHLUTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsAgeThreshLutEntry_Unpack(&sbZfKaQsAgeThreshLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsAgeThreshLutEntry_SetField(&sbZfKaQsAgeThreshLutEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsAgeThreshLutEntry_Pack(&sbZfKaQsAgeThreshLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSAGETHRESHKEY:
        *(int *)pData = val;
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;

    case QE2000_KARBCLASSSOURCEIDENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassSourceIdEntry_Unpack(&sbZfKaRbClassSourceIdEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassSourceIdEntry_SetField(&sbZfKaRbClassSourceIdEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassSourceIdEntry_Pack(&sbZfKaRbClassSourceIdEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
/*
    case QE2000_KAEPPCTEENTRY:
* MCM resolve which EP table *
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpPctEntry_Unpack(&sbZfKaEpPctEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpPctEntry_SetField(&sbZfKaEpPctEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpPctEntry_Pack(&sbZfKaEpPctEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit,  uAddress,   pData);
    break;
    
*/
    case QE2000_KAEPVLANINDTABLEENTRY:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpVlanIndTableEntry_Unpack(&sbZfKaEpVlanIndTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpVlanIndTableEntry_SetField(&sbZfKaEpVlanIndTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpVlanIndTableEntry_Pack(&sbZfKaEpVlanIndTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAEGSRCID:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgSrcId_Unpack(&sbZfKaEgSrcIdRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgSrcId_SetField(&sbZfKaEgSrcIdRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgSrcId_Pack(&sbZfKaEgSrcIdRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    
    break;
    case QE2000_KARBCLASSIPV6CLASSENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassIPv6ClassEntry_Unpack(&sbZfKaRbClassIPv6ClassEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassIPv6ClassEntry_SetField(&sbZfKaRbClassIPv6ClassEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassIPv6ClassEntry_Pack(&sbZfKaRbClassIPv6ClassEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    
    break;
    case QE2000_KAEPIPTTLRANGE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpTtlRange_Unpack(&sbZfKaEpIpTtlRangeRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpTtlRange_SetField(&sbZfKaEpIpTtlRangeRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpTtlRange_Pack(&sbZfKaEpIpTtlRangeRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSDMACMATCHENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassDmacMatchEntry_Unpack(&sbZfKaRbClassDmacMatchEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassDmacMatchEntry_SetField(&sbZfKaRbClassDmacMatchEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassDmacMatchEntry_Pack(&sbZfKaRbClassDmacMatchEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSIPV4TOSENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassIPv4TosEntry_Unpack(&sbZfKaRbClassIPv4TosEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassIPv4TosEntry_SetField(&sbZfKaRbClassIPv4TosEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassIPv4TosEntry_Pack(&sbZfKaRbClassIPv4TosEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBPOLEBSENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbPoliceEBSEntry_Unpack(&sbZfKaRbPoliceEBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbPoliceEBSEntry_SetField(&sbZfKaRbPoliceEBSEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbPoliceEBSEntry_Pack(&sbZfKaRbPoliceEBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbPolMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEIMEMDATAENTRY:
        status = sandDrvKaEiMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEiMemDataEntry_Unpack(&sbZfKaEiMemDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEiMemDataEntry_SetField(&sbZfKaEiMemDataEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEiMemDataEntry_Pack(&sbZfKaEiMemDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
	/* write all three EI tables */
        status = sandDrvKaEiMemWrite( unit, 0, uAddress,   pData);
        status = sandDrvKaEiMemWrite( unit, 1, uAddress,   pData);
        status = sandDrvKaEiMemWrite( unit, 2, uAddress,   pData);
    break;
    
    case QE2000_KAQMSLQCNTRSENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmSlqCntrsEntry_Unpack(&sbZfKaQmSlqCntrsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmSlqCntrsEntry_SetField(&sbZfKaQmSlqCntrsEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmSlqCntrsEntry_Pack(&sbZfKaQmSlqCntrsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KASRMANUALDESKEWENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaSrManualDeskewEntry_Unpack(&sbZfKaSrManualDeskewEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaSrManualDeskewEntry_SetField(&sbZfKaSrManualDeskewEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaSrManualDeskewEntry_Pack(&sbZfKaSrManualDeskewEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPSLIMVLANRECORD:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpSlimVlanRecord_Unpack(&sbZfKaEpSlimVlanRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpSlimVlanRecord_SetField(&sbZfKaEpSlimVlanRecordRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpSlimVlanRecord_Pack(&sbZfKaEpSlimVlanRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KAQSE2QADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsE2QAddr_Unpack(&sbZfKaQsE2QAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsE2QAddr_SetField(&sbZfKaQsE2QAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsE2QAddr_Pack(&sbZfKaQsE2QAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEBMVTADDRESS:
        status = sandDrvKaEbMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEbMvtAddress_Unpack(&sbZfKaEbMvtAddressRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEbMvtAddress_SetField(&sbZfKaEbMvtAddressRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEbMvtAddress_Pack(&sbZfKaEbMvtAddressRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEbMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KAEGMEMSHAPINGENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgMemShapingEntry_Unpack(&sbZfKaEgMemShapingEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgMemShapingEntry_SetField(&sbZfKaEgMemShapingEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgMemShapingEntry_Pack(&sbZfKaEgMemShapingEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPPORTTABLEENTRY:
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpPortTableEntry_Unpack(&sbZfKaEpPortTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpPortTableEntry_SetField(&sbZfKaEpPortTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpPortTableEntry_Pack(&sbZfKaEpPortTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAEGPORTREMAPENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgPortRemapEntry_Unpack(&sbZfKaEgPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgPortRemapEntry_SetField(&sbZfKaEgPortRemapEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgPortRemapEntry_Pack(&sbZfKaEgPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIPPREPEND:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpPrepend_Unpack(&sbZfKaEpIpPrependRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpPrepend_SetField(&sbZfKaEpIpPrependRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpPrepend_Pack(&sbZfKaEpIpPrependRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KAEPINSTRUCTION:
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpInstruction_Unpack(&sbZfKaEpInstructionRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpInstruction_SetField(&sbZfKaEpInstructionRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpInstruction_Pack(&sbZfKaEpInstructionRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpAmClMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQSAGEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsAgeEntry_Unpack(&sbZfKaQsAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsAgeEntry_SetField(&sbZfKaQsAgeEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsAgeEntry_Pack(&sbZfKaQsAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPBFPRITABLEENTRY:
        status = sandDrvKaEpMmBfMemRead( unit, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpBfPriTableEntry_Unpack(&sbZfKaEpBfPriTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpBfPriTableEntry_SetField(&sbZfKaEpBfPriTableEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpBfPriTableEntry_Pack(&sbZfKaEpBfPriTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmBfMemWrite( unit, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHINPUTW0:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashInputW0_Unpack(&sbZfKaRbClassHashInputW0Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashInputW0_SetField(&sbZfKaRbClassHashInputW0Record, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashInputW0_Pack(&sbZfKaRbClassHashInputW0Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit, spi, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEGNOTTMEPORTREMAPADDR:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgNotTmePortRemapAddr_Unpack(&sbZfKaEgNotTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgNotTmePortRemapAddr_SetField(&sbZfKaEgNotTmePortRemapAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgNotTmePortRemapAddr_Pack(&sbZfKaEgNotTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEGMEMQCTLENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEgMemQCtlEntry_Unpack(&sbZfKaEgMemQCtlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEgMemQCtlEntry_SetField(&sbZfKaEgMemQCtlEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEgMemQCtlEntry_Pack(&sbZfKaEgMemQCtlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEgMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEIRAWSPIREADENTRY:
        status = sandDrvKaEiMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEiRawSpiReadEntry_Unpack(&sbZfKaEiRawSpiReadEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEiRawSpiReadEntry_SetField(&sbZfKaEiRawSpiReadEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEiRawSpiReadEntry_Pack(&sbZfKaEiRawSpiReadEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEiMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSQ2ECENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsQ2EcEntry_Unpack(&sbZfKaQsQ2EcEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsQ2EcEntry_SetField(&sbZfKaQsQ2EcEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsQ2EcEntry_Pack(&sbZfKaQsQ2EcEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSPRILUTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsPriLutEntry_Unpack(&sbZfKaQsPriLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsPriLutEntry_SetField(&sbZfKaQsPriLutEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsPriLutEntry_Pack(&sbZfKaQsPriLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIPCOUNTER:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIpCounter_Unpack(&sbZfKaEpIpCounterRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIpCounter_SetField(&sbZfKaEpIpCounterRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIpCounter_Pack(&sbZfKaEpIpCounterRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQMDEMANDCFGDATAENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmDemandCfgDataEntry_Unpack(&sbZfKaQmDemandCfgDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmDemandCfgDataEntry_SetField(&sbZfKaQmDemandCfgDataEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmDemandCfgDataEntry_Pack(&sbZfKaQmDemandCfgDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSQUEUEPARAMENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsQueueParamEntry_Unpack(&sbZfKaQsQueueParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsQueueParamEntry_SetField(&sbZfKaQsQueueParamEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsQueueParamEntry_Pack(&sbZfKaQsQueueParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSPRIENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsPriEntry_Unpack(&sbZfKaQsPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsPriEntry_SetField(&sbZfKaQsPriEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsPriEntry_Pack(&sbZfKaQsPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAEPIP32BITREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaEpIp32BitRewrite_Unpack(&sbZfKaEpIp32BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaEpIp32BitRewrite_SetField(&sbZfKaEpIp32BitRewriteRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaEpIp32BitRewrite_Pack(&sbZfKaEpIp32BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaEpMmIpMemWrite( unit,  uAddress,   pData);
    break;
    
    case QE2000_KAQSDEPTHHPLENENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsDepthHplenEntry_Unpack(&sbZfKaQsDepthHplenEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsDepthHplenEntry_SetField(&sbZfKaQsDepthHplenEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsDepthHplenEntry_Pack(&sbZfKaQsDepthHplenEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KARBCLASSHASHVLANIPV4:
        status = sandDrvKaRbClassMemRead( unit,spi,  uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaRbClassHashVlanIPv4_Unpack(&sbZfKaRbClassHashVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaRbClassHashVlanIPv4_SetField(&sbZfKaRbClassHashVlanIPv4Record, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaRbClassHashVlanIPv4_Pack(&sbZfKaRbClassHashVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaRbClassMemWrite( unit,spi,  uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQMQUEUEBYTEADJENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQmQueueByteAdjEntry_Unpack(&sbZfKaQmQueueByteAdjEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQmQueueByteAdjEntry_SetField(&sbZfKaQmQueueByteAdjEntryRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQmQueueByteAdjEntry_Pack(&sbZfKaQmQueueByteAdjEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQmMemWrite( unit, uTableId, uAddress,   pData);
    break;
    
    case QE2000_KAQSPRILUTADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_READ;
         sbZfKaQsPriLutAddr_Unpack(&sbZfKaQsPriLutAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sbZfKaQsPriLutAddr_SetField(&sbZfKaQsPriLutAddrRecord, fieldname, val);
        if (status != SAND_DRV_KA_STATUS_OK) return QE2000_STATUS_BAD_FIELD;
        sbZfKaQsPriLutAddr_Pack(&sbZfKaQsPriLutAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
        status = sandDrvKaQsMemWrite( unit, uTableId, uAddress,   pData);
    break;

    default:
        status = QE2000_STATUS_BAD_READ;
    break;
    }
    
    if (status == QE2000_STATUS_BAD_READ) {
        cli_out("Error %d while reading Qe2000 memory\n",status);
    } else if (status != 0) {
        cli_out("Error %d while writing Qe2000 memory\n",status);
    }
    return 0;
}

/*
 * sbQe2000MemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbQe2000MemSetField(int unit, char *memFieldName, int addr, int val)
{
    int i, memindex, status, doingfield, uTableId;
    char *dptr, curch, memname[100], memfield[100];

   /* memfield comes in as mem.field, so split memname and field name */
    doingfield = 0;    /* needed to convert field name to lower chars */
    dptr = &memname[0];
    memfield[0] = 'm';
    memfield[1] = '_';
    memfield[2] = 'n';
    for(i = 0; i < 100; i++) {
        curch = memFieldName[i];
        if (memFieldName[i] == '\0') {
            *dptr = '\0';
            break;
        }
        if (curch == '.') {
            *dptr = '\0';
            dptr = &memfield[3];
            doingfield = 1;
        }
        else {
            if (doingfield)
                *dptr++ = tolower((unsigned char)curch);
            else
                *dptr++ = curch;
        }
    }

    memindex = sbQe2000WhichMem(memname);
    if(memindex == -1) {
        cli_out("Error: unrecognized Qe2000 memory: %s\n",memname);
        cli_out("Please pick from one of these:\n");
        sbQe2000ShowMemNames();
        return CMD_FAIL;
    }
/*
    max = sbQe2000MemMax(memindex);
    if (addr > max){
        cli_out("Error: Addr %d out of range for memory: %s max=%d\n",
                addr,memname,max);
        return CMD_FAIL;
    }
*/
    uTableId = qe2kMemConfigTable[memindex].tableId;
    cli_out("MemSetField unit=%d memIndex =%d  tableId=%d\n",unit,memindex, uTableId);

    status = sbQe2000MemSetFieldEntry(unit, memindex,addr,memfield, val);
    switch(status) {
        case  QE2000_STATUS_BAD_READ:
           cli_out("Error Reading Qe2000 memory\n");
           break;
        case  QE2000_STATUS_BAD_WRITE:
           cli_out("Error Writing Qe2000 memory\n");
           break;
        case  QE2000_STATUS_BAD_FIELD:
           cli_out("Error: unrecognized field <%s> for this Qe2000 memory: %s\n",&memfield[3],memname);
           break;
    }
    if (status != QE2000_STATUS_OK) {
        status = CMD_USAGE;
    }
    return status;
}

#endif /* BCM_QE2000_SUPPORT */
