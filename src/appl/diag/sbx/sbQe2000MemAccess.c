/*
 * $Id: sbQe2000MemAccess.c,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbQe2000MemAccess.c
 * Purpose:     sbx commands to read/write qe2000 indirect mems
 * Requires:
 */

#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <soc/defs.h>
#include <appl/diag/sbx/sbx.h>

#ifdef BCM_QE2000_SUPPORT

#include <appl/diag/sbx/imfswap32.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/qe2k/qe2k.h>

#include <soc/sbx/qe2k/sbZfQeIncludes.h>
#include <soc/sbx/qe2k/KaminoDriver.h>
#include <bcm/error.h>

extern int sbQe2kPmMemRead(sbhandle tKaAddr,
			   uint32 ulAddr,
			   uint32 *pulData3,
			   uint32 *pulData2,
			   uint32 *pulData1,
			   uint32 *pulData0);
int sbQe2000PmMemDump(int unit, int channel, int rangemin, int rangemax, int pause);
extern void qeSwapWords(uint *pdata, int nwords);

qe2kMemConfigRec qe2kMemConfigTable[] =
{
    {"KaEbMvtEntry",               QE2000_KAEBMVTENTRY,0x5, 49152},
    {"KaEbMvtAddress",             QE2000_KAEBMVTADDRESS,0x5, 49152},
    {"KaEgMemFifoParamEntry",      QE2000_KAEGMEMFIFOPARAMENTRY,0x2, 200},
    {"KaEgMemFifoControlEntry",    QE2000_KAEGMEMFIFOCONTRILENTRY,0x1, 200},
    {"KaEgMemShapingEntry",        QE2000_KAEGMEMSHAPINGENTRY,0x5, 152},
    {"KaEgMemQCtlEntry",           QE2000_KAEGMEMQCTLENTRY,0x8, 100},
    {"KaEgNotTmePortRemapAddr",    QE2000_KAEGNOTTMEPORTREMAPADDR,0x0, 1024},
    {"KaEgPortRemapEntry",         QE2000_KAEGPORTREMAPENTRY,0x0, 1024},
    {"KaEgSrcId",                  QE2000_KAEGSRCID,0x3, 1024},
    {"KaEgTmePortRemapAddr",       QE2000_KAEGTMEPORTREMAPADDR,0x0, 1024},
    {"KaEiMemDataEntry",           QE2000_KAEIMEMDATAENTRY,0x2, 49},
    {"KaEiRawSpiReadEntry",        QE2000_KAEIRAWSPIREADENTRY,0x5, 49},
    {"KaEpBfPriTableEntry",        QE2000_KAEPBFPRITABLEENTRY,0x5, 10},
    {"KaEpBfPriTableAddr",         QE2000_KAEPBFPRITABLEADDR,0x0, 10},
    {"KaEpCiTableEntry",           QE2000_KAEPCITABLEENTRY,0x5, 10},
    {"KaEpCrTableEntry",           QE2000_KAEPCRTABLEENTRY,0x0, 10},
    {"KaEpIpTciDmac",              QE2000_KAEPIPTCIDMAC,0x0, 10},
    {"KaEpIpTwoBitEntry",          QE2000_KAEPIPTWOBITENTRY,0x0, 10},
    {"KaEpIpPortVridSmacTableEntry", QE2000_KAEPIP_PORT_VRID_SMAC_TABLEENTRY,0x0, 10},
    {"KaEpIp16BitRewrite",         QE2000_KAEPIP16BITREWRITE,0x0, 10},
    {"KaEpIp32BitRewrite",         QE2000_KAEPIP32BITREWRITE,0x5, 10},
    {"KaEpIpV6Tci",                QE2000_KAEPIPV6TCI,0x0, 10},
    {"KaEpIpMplsLabels",           QE2000_KAEPIPMPLSLABELS,0x0, 10},
    {"KaEpIpPrepend",              QE2000_KAEPIPPREPEND,0x5, 10},
    {"KaEpInstruction",            QE2000_KAEPINSTRUCTION,0x5, 10},
    {"KaEpIpCounter",              QE2000_KAEPIPCOUNTER,0x5, 10},
    {"KaEpIpFourBitEntry",         QE2000_KAEPIPFOURBITENTRY,0x5, 10},
    {"KaEpIpPriExpTosRewrite",     QE2000_KAEPIPPRIEXPTOSREWRITE,0x0, 10},
    {"KaEpIpOneBitEntry",          QE2000_KAEPIPONEBITENTRY,0x0, 10},
    {"KaEpIpTtlRange",             QE2000_KAEPIPTTLRANGE,0x5, 10},
    {"KaEpPortTableEntry",         QE2000_KAEPPORTTABLEENTRY,0x5, 10},
    {"KaEpPredictState",           QE2000_KAEPPREDICTSTATE,0x5, 10},
    {"KaEpPctEntry",               QE2000_KAEPPCTEENTRY,0x0, 10},
    {"KaEpSlimVlanRecord",         QE2000_KAEPSLIMVLANRECORD,0x5, 10},
    {"KaEpVlanIndRecord",          QE2000_KAEPVLANINDRECORD,0x0, 10},
    {"KaEpVlanIndTableEntry",      QE2000_KAEPVLANINDTABLEENTRY,0x5, 10},
    {"KaQmBaaCfgTableEntry",       QE2000_KAQMBAACFGTABLEENTRY,0xD, 0x3FFF},
    {"KaQmDemandCfgDataEntry",     QE2000_KAQMDEMANDCFGDATAENTRY,0x8, 0x3FFF},
    {"KaQmFbLine",                 QE2000_KAQMFBLINE,0x7, 10},
    {"KaQmIngressPortEntry",       QE2000_KAQMINGRESSPORTENTRY,0x1, 0x3FFF},
    {"KaQmPortBwCfgTableEntry",    QE2000_KAQMPORTBWCFGTABLEENTRY,0xC, 0xFFF},
    {"KaQmQueueArrivalsEntry",     QE2000_KAQMQUEUEARRIVALSENTRY,0x4, 0x3FFF},
    {"KaQmQueueByteAdjEntry",      QE2000_KAQMQUEUEBYTEADJENTRY,0x6, 0x3FFF},
    {"KaQmQueueAgeEntry",          QE2000_KAQMQUEUEAGEENTRY,0x2, 0x3FFF},
    {"KaQmQueueStateEntry",        QE2000_KAQMQUEUESTATEENTRY,0x0, 0x3FFF},
    {"KaQmWredParamEntry",         QE2000_KAQMWREDPARAMENTRY,0x3, 0x3FFF},
    {"KaQmWredCfgTableEntry",      QE2000_KAQMWREDCFGTABLEENTRY,0xF, 0x3FFF},
    {"KaQmWredCurvesTableEntry",   QE2000_KAQMWREDCURVESTABLEENTRY,0x10, 47},
    {"KaQmWredAvQlenTableEntry",   QE2000_KAQMWREDAVQLENTABLEENTRY,0xE, 0x3FFF},
    {"KaQmSlqCntrsEntry",          QE2000_KAQMSLQCNTRSENTRY,0xA, 191},
    {"KaQmRateDeltaMaxEntry",      QE2000_KAQMRATEDELTAMAXENTRY,0x9, 64},
    {"KaQsAgeThreshLutEntry",      QE2000_KAQSAGETHRESHLUTENTRY,0xB, 32},
    {"KaQsAgeThreshKey",           QE2000_KAQSAGETHRESHKEY,0xA, 16384},
    {"KaQsAgeEntry",               QE2000_KAQSAGEENTRY,0x9, 16384},
    {"KaQsCredit",                 QE2000_KAQSCREDIT,0x2, 16384},
    {"KaQsDepthHplenEntry",        QE2000_KAQSDEPTHHPLENENTRY,0x3, 16384},
    {"KaQsE2QEntry",               QE2000_KAQSE2QENTRY,0xE, 4224},
    {"KaQsE2QAddr",                QE2000_KAQSE2QADDR,0xE, 4224},
    {"KaQsLastSentPriEntry",       QE2000_KAQSLASTSENTPRIENTRY,0xf, 4224},
    {"KaQsLastSentPriAddr",        QE2000_KAQSLASTSENTPRIADDR,0xf, 4224},
    {"KaQsLnaPortRemapEntry",      QE2000_KAQSPORTREMAPENTRY,0x2, 16384},
    {"KaQsLnaPriEntry",            QE2000_KAQSLNAPRIENTRY,0x0, 70},
    {"KaQsLnaNextPriEntry",        QE2000_KAQSLNANEXTPRIENTRY,0x1, 70},
    {"KaQsPriAddr",                QE2000_KAQSPRIADDR,0xd, 16*4224},
    {"KaQsPriEntry",               QE2000_KAQSPRIENTRY,0xD, 16*4224},
    {"KaQsPriLutEntry",            QE2000_KAQSPRILUTENTRY,0xC, 8192},
    {"KaQsPriLutAddr",             QE2000_KAQSPRILUTADDR,0xC, 8192},
    {"KaQsQueueTableEntry",        QE2000_KAQSQUEUETABLEENTRY,0x5, 16384},
    {"KaQsQueueParamEntry",        QE2000_KAQSQUEUEPARAMENTRY,0x5, 16384},
    {"KaQsQ2EcEntry",              QE2000_KAQSQ2ECENTRY,0x4, 16384},
    {"KaQsRateA",                  QE2000_KAQSRATETABLEA,0x0, 16384},
    {"KaQsRateB",                  QE2000_KAQSRATETABLEB,0x1, 16384},
    {"KaQsShapeRateEntry",         QE2000_KAQSSHAPERATEENTRY,0x6, 16384},
    {"KaQsShapeTableEntry",        QE2000_KAQSSHAPETABLEENTRY,0x8, 16384},
    {"KaQsShapeMaxBurstEntry",     QE2000_KAQSSHAPEMAXBURSTENTRY,0x7, 16384},
    {"KaRbClassDefaultQEntry",     QE2000_KARBCLASSDEFAULTQENTRY,0x1, 0x3f},
    {"KaRbClassDmacMatchEntry",    QE2000_KARBCLASSDMACMATCHENTRY,0x5, 0x1f},
    {"KaRbClassHashVlanIPv6",      QE2000_KARBCLASSHASHVLANIPV6,0x2, 0x7F},
    {"KaRbClassHashIPv6Only",      QE2000_KARBCLASSHASHIPV6ONLY,0x2, 0x7F},
    {"KaRbClassHashSVlanIPv6",     QE2000_KARBCLASSHASHSVLANIPV6,0x5, 0x1f},
    {"KaRbClassHashSVlanIPv4",     QE2000_KARBCLASSHASHSVLANIPV4,0x3, 0x7F},
    {"KaRbClassHashInputW0",       QE2000_KARBCLASSHASHINPUTW0,0x0, 0x3f},
    {"KaRbClassHashVlanIPv4",      QE2000_KARBCLASSHASHVLANIPV4,0x0, 0x3f},
    {"KaRbClassProtocolEntry",     QE2000_KARBCLASSPROTOCOLENTRY,0x4, 0x7F},
    {"KaRbClassIPv4TosEntry",      QE2000_KARBCLASSIPV4TOSENTRY,0x3, 0x7F},
    {"KaRbClassIPv6ClassEntry",    QE2000_KARBCLASSIPV6CLASSENTRY,0x2, 0x7F},
    {"KaRbClassPortEnablesEntry",  QE2000_KARBCLASSPORTENABLESENTRY,0x0, 0x3f},
    {"KaRbClassSourceIdEntry",     QE2000_KARBCLASSSOURCEIDENTRY,0x6, 0x1f},
    {"KaRbPoliceEBSEntry",         QE2000_KARBPOLEBSENTRY,0x2, 0x1ff},
    {"KaRbPolicePortQMapEntry",    QE2000_KARBPOLPORTQMAPENTRY,0x0, 0x1ff},
    {"KaRbPoliceCBSEntry",         QE2000_KARBPOLCBSENTRY,0x1, 0x1ff},
    {"KaRbPoliceCfgCtrlEntry",     QE2000_KARBPOLCFGCTRLENTRY,0x3, 0x1ff},
    {"KaPmLastLine",               QE2000_KAPMLASTLINE,0x5, 10},
    {"KaSrManualDeskewEntry",      QE2000_KASRMANUALDESKEWENTRY,0x5, 10},
    {"KaDDR",                      QE2000_KADDRMEM,0, 0x1ffffff},
    {"KaTestStat",                 QE2000_KATESTSTAT,0x5, 10},
    {"END",                        QE2000_MEM_MAX_INDEX, 0x05, 0xff},
};

/*
 * just return for big endian 
 */
void qeSwapWords(uint *pdata, int nwords)
{
  int i;

   for(i = 0; i < nwords; i++) {
       *pdata = IMFSWAP32(*pdata);
       pdata++;
   }
}
int sbQe2000PmMemDump(int unit, int channel, int rangemin, int rangemax, int pause)
{
    int status = 0;
    int ch;
    uint32 uRdData[4];
    sbhandle tKaAddr = (sbhandle)unit;
    int jj;
    uint32 addr;

    cli_out("CH:%-8s  %8s %8s %8s %8s %8s\n",
            "addr", "ctrl", "data0", "data1", "data2", "data3");
    for (ch = 0; status == 0 && ch < 8; ch++) {
        if (channel != -1 && ch != channel){
            continue;
        }
      
	for (jj=rangemin; jj<=rangemax; jj=jj+8) {
	    addr = jj + ch;
	    if (sbQe2kPmMemRead(tKaAddr, addr, &uRdData[3], &uRdData[2],
                                &uRdData[1], &uRdData[0]) != 0) {
		cli_out("Can't read memory location 0x%08x\n", addr);
		status = -1;
		break;
	    }
	    cli_out("%02d:%08x  %08x %08x %08x %08x %08x\n",
                    ch, jj,
                    SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_CTRL),
                    uRdData[0], uRdData[1], uRdData[2], uRdData[2]);
	    if (pause > 0)
	      sal_udelay(pause);
	}
    }
    return status;
}


/*
 * scan qe2kMemConfigTable for designated name and return memIndex
 */
int
sbQe2000WhichMem(char *memname)
{
   int i;

   for(i = 0; i < QE2000_MEM_MAX_INDEX; i++) {
       if(strcmp(memname,qe2kMemConfigTable[i].memname) == 0)
          return qe2kMemConfigTable[i].memindex;
   }
   return -1;

}

int 
sbQe2000MemMax(int memindex)
{
    return qe2kMemConfigTable[memindex].rangemax;
}

void
sbQe2000ShowMemNames(void)
{
   int i;

   for(i = 0; i < QE2000_MEM_MAX_INDEX; i++) {
       cli_out("%-32s",qe2kMemConfigTable[i].memname);
       if ((i+1)%3 == 0)
	 cli_out("\n");
   }
   cli_out("\n");
}





/*
 * sbQe2000MemShowEntry - read and display contents of qe2000 indirect memory
 *	inputs:	 unit 
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbQe2000MemShowEntry(int unit, int memindex, int entryindex)
{
    int  status = BCM_E_NOT_FOUND;

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
    sbZfKaQsLnaPriEntry_t           sbZfKaQsLnaPriEntryRecord;
    sbZfKaQsLnaNextPriEntry_t       sbZfKaQsLnaNextPriEntryRecord;
    sbZfKaEgMemFifoParamEntry_t     sbZfKaEgMemFifoParamEntryRecord;
    sbZfKaQsShapeTableEntry_t       sbZfKaQsShapeTableEntryRecord;
    sbZfKaQmQueueAgeEntry_t         sbZfKaQmQueueAgeEntryRecord;
    sbZfKaRbClassDefaultQEntry_t    sbZfKaRbClassDefaultQEntryRecord;
    sbZfKaQmWredCfgTableEntry_t     sbZfKaQmWredCfgTableEntryRecord;
    sbZfKaQsPriAddr_t               sbZfKaQsPriAddrRecord;
    sbZfKaRbClassHashVlanIPv6_t     sbZfKaRbClassHashVlanIPv6Record;
    sbZfKaQsShapeMaxBurstEntry_t    sbZfKaQsShapeMaxBurstEntryRecord;
    sbZfKaQmRateDeltaMaxTableEntry_t sbZfKaQmRateDeltaMaxEntryRecord;
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

    /*uint dbuf[40];*/
   uint8 dbuf[50]; /* larger size required for HashVlanIPv6 table */
   uint    uTableId,  uAddress, *pData  = (uint *)&dbuf[0];
   uint    spi = 0;   /*  FIX THIS */

    uAddress = entryindex;
    uTableId = qe2kMemConfigTable[memindex].tableId;
    cli_out("---  Entry index: %d ---\n",entryindex);
    switch(memindex) {

    case QE2000_KAQSLASTSENTPRIENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsLastSentPriEntry_Unpack(&sbZfKaQsLastSentPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsLastSentPriEntry_Print(&sbZfKaQsLastSentPriEntryRecord);
        }
    break;
    case QE2000_KAQMQUEUEARRIVALSENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmQueueArrivalsEntry_Unpack(&sbZfKaQmQueueArrivalsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmQueueArrivalsEntry_Print(&sbZfKaQmQueueArrivalsEntryRecord);
        }
    break;
    case QE2000_KAQMWREDPARAMENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmWredParamEntry_Unpack(&sbZfKaQmWredParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmWredParamEntry_Print(&sbZfKaQmWredParamEntryRecord);
        }
    break;
    case QE2000_KARBCLASSPORTENABLESENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassPortEnablesEntry_Unpack(&sbZfKaRbClassPortEnablesEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassPortEnablesEntry_Print(&sbZfKaRbClassPortEnablesEntryRecord);
        }
    break;
    case QE2000_KAQMPORTBWCFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmPortBwCfgTableEntry_Unpack(&sbZfKaQmPortBwCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmPortBwCfgTableEntry_Print(&sbZfKaQmPortBwCfgTableEntryRecord);
        }
    break;
    case QE2000_KAQSLASTSENTPRIADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsLastSentPriAddr_Unpack(&sbZfKaQsLastSentPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsLastSentPriAddr_Print(&sbZfKaQsLastSentPriAddrRecord);
        }
    break;
    case QE2000_KAEPCRTABLEENTRY:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpCrTableEntry_Unpack(&sbZfKaEpCrTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpCrTableEntry_Print(&sbZfKaEpCrTableEntryRecord);
        }
    break;
    case QE2000_KAEPIP_PORT_VRID_SMAC_TABLEENTRY:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpPortVridSmacTableEntry_Unpack(&sbZfKaEpIpPortVridSmacTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpPortVridSmacTableEntry_Print(&sbZfKaEpIpPortVridSmacTableEntryRecord);
        }
    break;
    case QE2000_KAQSSHAPERATEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsShapeRateEntry_Unpack(&sbZfKaQsShapeRateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsShapeRateEntry_Print(&sbZfKaQsShapeRateEntryRecord);
        }
    break;
    case QE2000_KAQSPORTREMAPENTRY:
        status = sandDrvKaQsMemLnaRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsLnaPortRemapEntry_Unpack(&sbZfKaQsLnaPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsLnaPortRemapEntry_Print(&sbZfKaQsLnaPortRemapEntryRecord);
        }
    break;
    case QE2000_KAQSLNAPRIENTRY:
        status = sandDrvKaQsMemLnaRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
	    if ((pData[0]) || (pData[1]) || (pData[2]) || (pData[3]) || (pData[4])) {
	       sbZfKaQsLnaPriEntry_Unpack(&sbZfKaQsLnaPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
	       sbZfKaQsLnaPriEntry_Print(&sbZfKaQsLnaPriEntryRecord);
	    }
        }
    break;
    case QE2000_KAQSLNANEXTPRIENTRY:
        status = sandDrvKaQsMemLnaRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
	    if ((pData[0]) || (pData[1]) || (pData[2]) || (pData[3])) {
		sbZfKaQsLnaNextPriEntry_Unpack(&sbZfKaQsLnaNextPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
		sbZfKaQsLnaNextPriEntry_Print(&sbZfKaQsLnaNextPriEntryRecord);
	    }
        }
    break;
    case QE2000_KAEGMEMFIFOPARAMENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgMemFifoParamEntry_Unpack(&sbZfKaEgMemFifoParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgMemFifoParamEntry_Print(&sbZfKaEgMemFifoParamEntryRecord);
        }
    break;
    case QE2000_KAQSSHAPETABLEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsShapeTableEntry_Unpack(&sbZfKaQsShapeTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsShapeTableEntry_Print(&sbZfKaQsShapeTableEntryRecord);
        }
    break;
    case QE2000_KAQMQUEUEAGEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmQueueAgeEntry_Unpack(&sbZfKaQmQueueAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmQueueAgeEntry_Print(&sbZfKaQmQueueAgeEntryRecord);
        }
    break;
    case QE2000_KARBCLASSDEFAULTQENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassDefaultQEntry_Unpack(&sbZfKaRbClassDefaultQEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassDefaultQEntry_Print(&sbZfKaRbClassDefaultQEntryRecord);
        }
    break;
    case QE2000_KAQMWREDCFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmWredCfgTableEntry_Unpack(&sbZfKaQmWredCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmWredCfgTableEntry_Print(&sbZfKaQmWredCfgTableEntryRecord);
        }
    break;
    case QE2000_KAQSPRIADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsPriAddr_Unpack(&sbZfKaQsPriAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsPriAddr_Print(&sbZfKaQsPriAddrRecord);
        }
    break;
    case QE2000_KARBCLASSHASHVLANIPV6:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashVlanIPv6_Unpack(&sbZfKaRbClassHashVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashVlanIPv6_Print(&sbZfKaRbClassHashVlanIPv6Record);
        }
    break;
    case QE2000_KAQSSHAPEMAXBURSTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsShapeMaxBurstEntry_Unpack(&sbZfKaQsShapeMaxBurstEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsShapeMaxBurstEntry_Print(&sbZfKaQsShapeMaxBurstEntryRecord);
        }
    break;
    case QE2000_KAQMRATEDELTAMAXENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmRateDeltaMaxTableEntry_Unpack(&sbZfKaQmRateDeltaMaxEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmRateDeltaMaxTableEntry_Print(&sbZfKaQmRateDeltaMaxEntryRecord);
        }
    break;
    case QE2000_KAQMQUEUESTATEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmQueueStateEntry_Unpack(&sbZfKaQmQueueStateEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmQueueStateEntry_Print(&sbZfKaQmQueueStateEntryRecord);
        }
    break;
    case QE2000_KAQMFBLINE:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmFbLine_Unpack(&sbZfKaQmFbLineRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmFbLine_Print(&sbZfKaQmFbLineRecord);
        }
    break;
    case QE2000_KAEPIP16BITREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIp16BitRewrite_Unpack(&sbZfKaEpIp16BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIp16BitRewrite_Print(&sbZfKaEpIp16BitRewriteRecord);
        }
    break;
    case QE2000_KAQMINGRESSPORTENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmIngressPortEntry_Unpack(&sbZfKaQmIngressPortEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmIngressPortEntry_Print(&sbZfKaQmIngressPortEntryRecord);
        }
    break;
    case QE2000_KAEBMVTENTRY:
        status = sandDrvKaEbMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEbMvtEntry_Unpack(&sbZfKaEbMvtEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEbMvtEntry_Print(&sbZfKaEbMvtEntryRecord);
        }
    break;
    case QE2000_KAQSRATETABLEA:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsRateTableEntry_Unpack(&sbZfKaQsRateTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsRateTableEntry_Print(&sbZfKaQsRateTableEntryRecord);
        }
    break;
    case QE2000_KAQSRATETABLEB:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsRateTableEntry_Unpack(&sbZfKaQsRateTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsRateTableEntry_Print(&sbZfKaQsRateTableEntryRecord);
        }
    break;
    case QE2000_KAEGMEMFIFOCONTRILENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgMemFifoControlEntry_Unpack(&sbZfKaEgMemFifoControlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgMemFifoControlEntry_Print(&sbZfKaEgMemFifoControlEntryRecord);
        }
    break;
    case QE2000_KAEPIPTCIDMAC:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpTciDmac_Unpack(&sbZfKaEpIpTciDmacRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpTciDmac_Print(&sbZfKaEpIpTciDmacRecord);
        }
    break;
    case QE2000_KARBCLASSHASHIPV6ONLY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashIPv6Only_Unpack(&sbZfKaRbClassHashIPv6OnlyRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashIPv6Only_Print(&sbZfKaRbClassHashIPv6OnlyRecord);
        }
    break;
    case QE2000_KAEPVLANINDRECORD:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpVlanIndRecord_Unpack(&sbZfKaEpVlanIndRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpVlanIndRecord_Print(&sbZfKaEpVlanIndRecordRecord);
        }
    break;
    case QE2000_KARBCLASSHASHSVLANIPV6:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashSVlanIPv6_Unpack(&sbZfKaRbClassHashSVlanIPv6Record,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashSVlanIPv6_Print(&sbZfKaRbClassHashSVlanIPv6Record);
        }
    break;
    case QE2000_KARBCLASSHASHSVLANIPV4:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashSVlanIPv4_Unpack(&sbZfKaRbClassHashSVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashSVlanIPv4_Print(&sbZfKaRbClassHashSVlanIPv4Record);
        }
    break;
    case QE2000_KARBPOLPORTQMAPENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbPolicePortQMapEntry_Unpack(&sbZfKaRbPolicePortQMapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbPolicePortQMapEntry_Print(&sbZfKaRbPolicePortQMapEntryRecord);
        }
    break;
    case QE2000_KARBPOLCBSENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbPoliceCBSEntry_Unpack(&sbZfKaRbPoliceCBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbPoliceCBSEntry_Print(&sbZfKaRbPoliceCBSEntryRecord);
        }
    break;
    case QE2000_KAEPIPV6TCI:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpV6Tci_Unpack(&sbZfKaEpIpV6TciRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpV6Tci_Print(&sbZfKaEpIpV6TciRecord);
        }
    break;
    case QE2000_KAEPIPMPLSLABELS:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpMplsLabels_Unpack(&sbZfKaEpIpMplsLabelsRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpMplsLabels_Print(&sbZfKaEpIpMplsLabelsRecord);
        }
    break;
    case QE2000_KAEPBFPRITABLEADDR:
        status = sandDrvKaEpMmBfMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpBfPriTableAddr_Unpack(&sbZfKaEpBfPriTableAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpBfPriTableAddr_Print(&sbZfKaEpBfPriTableAddrRecord);
        }
    break;
    case QE2000_KAQSE2QENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsE2QEntry_Unpack(&sbZfKaQsE2QEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsE2QEntry_Print(&sbZfKaQsE2QEntryRecord);
        }
    break;
    case QE2000_KARBPOLCFGCTRLENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbPoliceCfgCtrlEntry_Unpack(&sbZfKaRbPoliceCfgCtrlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbPoliceCfgCtrlEntry_Print(&sbZfKaRbPoliceCfgCtrlEntryRecord);
        }
    break;
    case QE2000_KAQSQUEUETABLEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsQueueTableEntry_Unpack(&sbZfKaQsQueueTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsQueueTableEntry_Print(&sbZfKaQsQueueTableEntryRecord);
        }
    break;
    case QE2000_KAQMBAACFGTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmBaaCfgTableEntry_Unpack(&sbZfKaQmBaaCfgTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmBaaCfgTableEntry_Print(&sbZfKaQmBaaCfgTableEntryRecord);
        }
    break;
    case QE2000_KAQMWREDCURVESTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmWredCurvesTableEntry_Unpack(&sbZfKaQmWredCurvesTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmWredCurvesTableEntry_Print(&sbZfKaQmWredCurvesTableEntryRecord);
        }
    break;
    case QE2000_KARBCLASSPROTOCOLENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassProtocolEntry_Unpack(&sbZfKaRbClassProtocolEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassProtocolEntry_Print(&sbZfKaRbClassProtocolEntryRecord);
        }
    break;
    case QE2000_KAQMWREDAVQLENTABLEENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmWredAvQlenTableEntry_Unpack(&sbZfKaQmWredAvQlenTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmWredAvQlenTableEntry_Print(&sbZfKaQmWredAvQlenTableEntryRecord);
        }
    break;
    case QE2000_KAEPIPPRIEXPTOSREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpPriExpTosRewrite_Unpack(&sbZfKaEpIpPriExpTosRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpPriExpTosRewrite_Print(&sbZfKaEpIpPriExpTosRewriteRecord);
        }
    break;
    case QE2000_KAPMLASTLINE:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaPmLastLine_Unpack(&sbZfKaPmLastLineRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaPmLastLine_Print(&sbZfKaPmLastLineRecord);
        }
    break;
    case QE2000_KAEGTMEPORTREMAPADDR:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgTmePortRemapAddr_Unpack(&sbZfKaEgTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgTmePortRemapAddr_Print(&sbZfKaEgTmePortRemapAddrRecord);
        }
    break;
    case QE2000_KAQSAGETHRESHLUTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsAgeThreshLutEntry_Unpack(&sbZfKaQsAgeThreshLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsAgeThreshLutEntry_Print(&sbZfKaQsAgeThreshLutEntryRecord);
        }
    break;
    case QE2000_KAQSAGETHRESHKEY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
	  cli_out("QsAgeThreshKey: %d\n", *(int *)dbuf);
        }
    break;
    case QE2000_KARBCLASSSOURCEIDENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassSourceIdEntry_Unpack(&sbZfKaRbClassSourceIdEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassSourceIdEntry_Print(&sbZfKaRbClassSourceIdEntryRecord);
        }
    break;
/*
    case QE2000_KAEPPCTEENTRY:
* MCM resolve which EP table *
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpPctEntry_Unpack(&sbZfKaEpPctEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpPctEntry_Print(&sbZfKaEpPctEntryRecord);
        }
    break;
*/
    case QE2000_KAEPVLANINDTABLEENTRY:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpVlanIndTableEntry_Unpack(&sbZfKaEpVlanIndTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpVlanIndTableEntry_Print(&sbZfKaEpVlanIndTableEntryRecord);
        }
    break;
    case QE2000_KAEGSRCID:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgSrcId_Unpack(&sbZfKaEgSrcIdRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgSrcId_Print(&sbZfKaEgSrcIdRecord);
        }
    break;
    case QE2000_KARBCLASSIPV6CLASSENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassIPv6ClassEntry_Unpack(&sbZfKaRbClassIPv6ClassEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassIPv6ClassEntry_Print(&sbZfKaRbClassIPv6ClassEntryRecord);
        }
    break;
    case QE2000_KAEPIPTTLRANGE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpTtlRange_Unpack(&sbZfKaEpIpTtlRangeRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpTtlRange_Print(&sbZfKaEpIpTtlRangeRecord);
        }
    break;
    case QE2000_KARBCLASSDMACMATCHENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassDmacMatchEntry_Unpack(&sbZfKaRbClassDmacMatchEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassDmacMatchEntry_Print(&sbZfKaRbClassDmacMatchEntryRecord);
        }
    break;
    case QE2000_KARBCLASSIPV4TOSENTRY:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassIPv4TosEntry_Unpack(&sbZfKaRbClassIPv4TosEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassIPv4TosEntry_Print(&sbZfKaRbClassIPv4TosEntryRecord);
        }
    break;
    case QE2000_KARBPOLEBSENTRY:
        status = sandDrvKaRbPolMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbPoliceEBSEntry_Unpack(&sbZfKaRbPoliceEBSEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbPoliceEBSEntry_Print(&sbZfKaRbPoliceEBSEntryRecord);
        }
    break;
    case QE2000_KAEIMEMDATAENTRY:
        status = sandDrvKaEiMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEiMemDataEntry_Unpack(&sbZfKaEiMemDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEiMemDataEntry_Print(&sbZfKaEiMemDataEntryRecord);
        }
    break;

    case QE2000_KAQMSLQCNTRSENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmSlqCntrsEntry_Unpack(&sbZfKaQmSlqCntrsEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmSlqCntrsEntry_Print(&sbZfKaQmSlqCntrsEntryRecord);
        }
    break;
    case QE2000_KASRMANUALDESKEWENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaSrManualDeskewEntry_Unpack(&sbZfKaSrManualDeskewEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaSrManualDeskewEntry_Print(&sbZfKaSrManualDeskewEntryRecord);
        }
    break;

    case QE2000_KAEPSLIMVLANRECORD:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpSlimVlanRecord_Unpack(&sbZfKaEpSlimVlanRecordRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpSlimVlanRecord_Print(&sbZfKaEpSlimVlanRecordRecord);
        }
    break;
    case QE2000_KAQSE2QADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsE2QAddr_Unpack(&sbZfKaQsE2QAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsE2QAddr_Print(&sbZfKaQsE2QAddrRecord);
        }
    break;
    case QE2000_KAEBMVTADDRESS:
        status = sandDrvKaEbMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEbMvtAddress_Unpack(&sbZfKaEbMvtAddressRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEbMvtAddress_Print(&sbZfKaEbMvtAddressRecord);
        }
    break;
    case QE2000_KAEGMEMSHAPINGENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgMemShapingEntry_Unpack(&sbZfKaEgMemShapingEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgMemShapingEntry_Print(&sbZfKaEgMemShapingEntryRecord);
        }
    break;
    case QE2000_KAEPPORTTABLEENTRY:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpPortTableEntry_Unpack(&sbZfKaEpPortTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpPortTableEntry_Print(&sbZfKaEpPortTableEntryRecord);
        }
    break;
    case QE2000_KAEGPORTREMAPENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgPortRemapEntry_Unpack(&sbZfKaEgPortRemapEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgPortRemapEntry_Print(&sbZfKaEgPortRemapEntryRecord);
        }
    break;
    case QE2000_KAEPIPPREPEND:
        status = sandDrvKaEpMmIpMemRead( unit, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpPrepend_Unpack(&sbZfKaEpIpPrependRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpPrepend_Print(&sbZfKaEpIpPrependRecord);
        }
    break;
    case QE2000_KAEPINSTRUCTION:
/* MCM resolve which EP table */
        status = sandDrvKaEpAmClMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpInstruction_Unpack(&sbZfKaEpInstructionRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpInstruction_Print(&sbZfKaEpInstructionRecord);
        }
    break;
    case QE2000_KAQSAGEENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsAgeEntry_Unpack(&sbZfKaQsAgeEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsAgeEntry_Print(&sbZfKaQsAgeEntryRecord);
        }
	break;
    
    case QE2000_KAQSCREDIT:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
	  cli_out("credit: 0x%08x\n", *pData);
        }
	break;

    case QE2000_KAEPBFPRITABLEENTRY:
        status = sandDrvKaEpMmBfMemRead( unit, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpBfPriTableEntry_Unpack(&sbZfKaEpBfPriTableEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpBfPriTableEntry_Print(&sbZfKaEpBfPriTableEntryRecord);
        }
    break;
    case QE2000_KARBCLASSHASHINPUTW0:
        status = sandDrvKaRbClassMemRead( unit, spi, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashInputW0_Unpack(&sbZfKaRbClassHashInputW0Record,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashInputW0_Print(&sbZfKaRbClassHashInputW0Record);
        }
    break;
    case QE2000_KAEGNOTTMEPORTREMAPADDR:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgNotTmePortRemapAddr_Unpack(&sbZfKaEgNotTmePortRemapAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgNotTmePortRemapAddr_Print(&sbZfKaEgNotTmePortRemapAddrRecord);
        }
    break;
    case QE2000_KAEGMEMQCTLENTRY:
        status = sandDrvKaEgMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEgMemQCtlEntry_Unpack(&sbZfKaEgMemQCtlEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEgMemQCtlEntry_Print(&sbZfKaEgMemQCtlEntryRecord);
        }
    break;
    case QE2000_KAEIRAWSPIREADENTRY:
        status = sandDrvKaEiMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEiRawSpiReadEntry_Unpack(&sbZfKaEiRawSpiReadEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEiRawSpiReadEntry_Print(&sbZfKaEiRawSpiReadEntryRecord);
        }
    break;
    case QE2000_KAQSQ2ECENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsQ2EcEntry_Unpack(&sbZfKaQsQ2EcEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsQ2EcEntry_Print(&sbZfKaQsQ2EcEntryRecord);
        }
    break;
    case QE2000_KAQSPRILUTENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsPriLutEntry_Unpack(&sbZfKaQsPriLutEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsPriLutEntry_Print(&sbZfKaQsPriLutEntryRecord);
        }
    break;
    case QE2000_KAEPIPCOUNTER:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIpCounter_Unpack(&sbZfKaEpIpCounterRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIpCounter_Print(&sbZfKaEpIpCounterRecord);
        }
    break;
    case QE2000_KAQMDEMANDCFGDATAENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmDemandCfgDataEntry_Unpack(&sbZfKaQmDemandCfgDataEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmDemandCfgDataEntry_Print(&sbZfKaQmDemandCfgDataEntryRecord);
        }
    break;
    case QE2000_KAQSQUEUEPARAMENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsQueueParamEntry_Unpack(&sbZfKaQsQueueParamEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsQueueParamEntry_Print(&sbZfKaQsQueueParamEntryRecord);
        }
    break;
    case QE2000_KAQSPRIENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsPriEntry_Unpack(&sbZfKaQsPriEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsPriEntry_Print(&sbZfKaQsPriEntryRecord);
        }
    break;
    case QE2000_KAEPIP32BITREWRITE:
        status = sandDrvKaEpMmIpMemRead( unit,  uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaEpIp32BitRewrite_Unpack(&sbZfKaEpIp32BitRewriteRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaEpIp32BitRewrite_Print(&sbZfKaEpIp32BitRewriteRecord);
        }
    break;
    case QE2000_KAQSDEPTHHPLENENTRY:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsDepthHplenEntry_Unpack(&sbZfKaQsDepthHplenEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsDepthHplenEntry_Print(&sbZfKaQsDepthHplenEntryRecord);
        }
    break;
    case QE2000_KARBCLASSHASHVLANIPV4:
        status = sandDrvKaRbClassMemRead( unit,spi,  uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaRbClassHashVlanIPv4_Unpack(&sbZfKaRbClassHashVlanIPv4Record,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaRbClassHashVlanIPv4_Print(&sbZfKaRbClassHashVlanIPv4Record);
        }
    break;
    case QE2000_KAQMQUEUEBYTEADJENTRY:
        status = sandDrvKaQmMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQmQueueByteAdjEntry_Unpack(&sbZfKaQmQueueByteAdjEntryRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQmQueueByteAdjEntry_Print(&sbZfKaQmQueueByteAdjEntryRecord);
        }
    break;
    case QE2000_KAQSPRILUTADDR:
        status = sandDrvKaQsMemRead( unit, uTableId, uAddress,   pData);
        if (status == SAND_DRV_KA_STATUS_OK) {
            sbZfKaQsPriLutAddr_Unpack(&sbZfKaQsPriLutAddrRecord,(uint8 *)dbuf,sizeof(dbuf));
            sbZfKaQsPriLutAddr_Print(&sbZfKaQsPriLutAddrRecord);
        }
      break;
    case QE2000_KADDRMEM:
      status = sbQe2000PmMemDump(unit, -1, uAddress, uAddress, 0);
      break;
    break;
    }
    
    if (status != SAND_DRV_KA_STATUS_OK) {
        cli_out("Error %d while reading Qe2000 memory\n",status);
    } 
    return 0;
}

/*
 * sbQe2000MemShowRange - call ShowEntry for a range of entries
 *	inputs:	unit
 *		index of memory 
 *              start of range
 *              end of range
    if (memindex == QE2000_KADDRMEM) {
      status = sbQe2000PmMemDump(unit, -1, rangemin, rangemax, 0);
      return 0;
    }

 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbQe2000MemShowRange(int unit, int memindex, int rangemin, int rangemax)
{
    int i,  status;

    for(i = rangemin; i <= rangemax; i++) {
        status = sbQe2000MemShowEntry(unit, memindex,i);
        if(status != 0)
            return status;
    }
    return 0;
}

/* #include "sbQe2000MemWrite.txt"*/

/*
 * sbQe2000MemShow - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbQe2000MemShow(int unit, char *memname, int rangemin, int rangemax)
{
    int i, cnt, len, tempmax;
    int status = CMD_FAIL;

    cnt = 0;
    len = strlen(memname);
    for(i = 0; i < QE2000_MEM_MAX_INDEX; i++) {
        if ((memname[0] == '*') || (strncmp(memname,qe2kMemConfigTable[i].memname,len) == 0)) {
             cnt++;
             cli_out("------------- %s ------------\n",qe2kMemConfigTable[i].memname);
             tempmax = rangemax;
             if (rangemax > qe2kMemConfigTable[i].rangemax){
                  tempmax = qe2kMemConfigTable[i].rangemax;
                  cli_out("Warning: Max range for mem %s is %d\n",memname,tempmax);
             }
             status = sbQe2000MemShowRange(unit,qe2kMemConfigTable[i].memindex,rangemin,tempmax);
        }
    }
    if(cnt == 0) {
        cli_out("Error: unrecognized Qe2000 memory: %s\n",memname);
        cli_out("Please pick from one of these:\n");
        sbQe2000ShowMemNames();
    } 
    return status;
}

#endif /* BCM_QE2000_SUPPORT */
