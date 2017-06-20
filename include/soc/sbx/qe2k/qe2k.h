/*
 * $Id: qe2k.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains aggregated definitions for Guadalupe 2.x microcode
 */

#ifndef _SOC_SBX_QE2K_H
#define _SOC_SBX_QE2K_H


#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/g2eplib/sbG2Eplib.h>

#define QE2000_STATUS_OK         0
#define QE2000_STATUS_BAD_UNIT  -1
#define QE2000_STATUS_BAD_READ  -2
#define QE2000_STATUS_BAD_WRITE -3
#define QE2000_STATUS_BAD_FIELD -4


typedef struct {
      char memname[30];
      int  memindex;
      int  tableId;
      int  rangemax;
} qe2kMemConfigRec;

extern void sbQe2000ShowMemNames(void);
extern sbG2EplibCtxt_st *gu2_unit2ep(int unit);

/*
 * include other specific structs defined for zframes
 */
#if 0
#include "sbZfKaDramPhysicalAddr.hx"
#include "sbZfKaDramPhysicalAddrConsole.hx"
#include "sbZfKaEbMvtAddress.hx"
#include "sbZfKaEbMvtAddressConsole.hx"
#include "sbZfKaEbMvtEntry.hx"
#include "sbZfKaEbMvtEntryConsole.hx"
#include "sbZfKaEgMemFifoControlEntry.hx"
#include "sbZfKaEgMemFifoControlEntryConsole.hx"
#include "sbZfKaEgMemFifoParamEntry.hx"
#include "sbZfKaEgMemFifoParamEntryConsole.hx"
#include "sbZfKaEgMemQCtlEntry.hx"
#include "sbZfKaEgMemQCtlEntryConsole.hx"
#include "sbZfKaEgMemShapingEntry.hx"
#include "sbZfKaEgMemShapingEntryConsole.hx"
#include "sbZfKaEgNotTmePortRemapAddr.hx"
#include "sbZfKaEgNotTmePortRemapAddrConsole.hx"
#include "sbZfKaEgPortRemapEntry.hx"
#include "sbZfKaEgPortRemapEntryConsole.hx"
#include "sbZfKaEgSrcId.hx"
#include "sbZfKaEgSrcIdConsole.hx"
#include "sbZfKaEgTmePortRemapAddr.hx"
#include "sbZfKaEgTmePortRemapAddrConsole.hx"
#include "sbZfKaEiMemDataEntry.hx"
#include "sbZfKaEiMemDataEntryConsole.hx"
#include "sbZfKaEiRawSpiReadEntry.hx"
#include "sbZfKaEiRawSpiReadEntryConsole.hx"
#include "sbZfKaEpBfPriTableAddr.hx"
#include "sbZfKaEpBfPriTableAddrConsole.hx"
#include "sbZfKaEpBfPriTableEntry.hx"
#include "sbZfKaEpBfPriTableEntryConsole.hx"
#include "sbZfKaEpCrTableEntry.hx"
#include "sbZfKaEpCrTableEntryConsole.hx"
#include "sbZfKaEpInstruction.hx"
#include "sbZfKaEpInstructionConsole.hx"
#include "sbZfKaEpIp16BitRewrite.hx"
#include "sbZfKaEpIp16BitRewriteConsole.hx"
#include "sbZfKaEpIp32BitRewrite.hx"
#include "sbZfKaEpIp32BitRewriteConsole.hx"
#include "sbZfKaEpIpCounter.hx"
#include "sbZfKaEpIpCounterConsole.hx"
#include "sbZfKaEpIpFourBitEntry.hx"
#include "sbZfKaEpIpFourBitEntryConsole.hx"
#include "sbZfKaEpIpMplsLabels.hx"
#include "sbZfKaEpIpMplsLabelsConsole.hx"
#include "sbZfKaEpIpOneBitEntry.hx"
#include "sbZfKaEpIpOneBitEntryConsole.hx"
#include "sbZfKaEpIpPortVridSmacTableEntry.hx"
#include "sbZfKaEpIpPortVridSmacTableEntryConsole.hx"
#include "sbZfKaEpIpPrepend.hx"
#include "sbZfKaEpIpPrependConsole.hx"
#include "sbZfKaEpIpPriExpTosRewrite.hx"
#include "sbZfKaEpIpPriExpTosRewriteConsole.hx"
#include "sbZfKaEpIpTciDmac.hx"
#include "sbZfKaEpIpTciDmacConsole.hx"
#include "sbZfKaEpIpTtlRange.hx"
#include "sbZfKaEpIpTtlRangeConsole.hx"
#include "sbZfKaEpIpTwoBitEntry.hx"
#include "sbZfKaEpIpTwoBitEntryConsole.hx"
#include "sbZfKaEpIpV6Tci.hx"
#include "sbZfKaEpIpV6TciConsole.hx"
#include "sbZfKaEpPortTableEntry.hx"
#include "sbZfKaEpPortTableEntryConsole.hx"
#include "sbZfKaEpSlimVlanRecord.hx"
#include "sbZfKaEpSlimVlanRecordConsole.hx"
#include "sbZfKaEpVlanIndRecord.hx"
#include "sbZfKaEpVlanIndRecordConsole.hx"
#include "sbZfKaEpVlanIndTableEntry.hx"
#include "sbZfKaEpVlanIndTableEntryConsole.hx"
#include "sbZfKaPmLastLine.hx"
#include "sbZfKaPmLastLineConsole.hx"
#include "sbZfKaQmBaaCfgTableEntry.hx"
#include "sbZfKaQmBaaCfgTableEntryConsole.hx"
#include "sbZfKaQmDemandCfgDataEntry.hx"
#include "sbZfKaQmDemandCfgDataEntryConsole.hx"
#include "sbZfKaQmFbLine.hx"
#include "sbZfKaQmFbLineConsole.hx"
#include "sbZfKaQmIngressPortEntry.hx"
#include "sbZfKaQmIngressPortEntryConsole.hx"
#include "sbZfKaQmPortBwCfgTableEntry.hx"
#include "sbZfKaQmPortBwCfgTableEntryConsole.hx"
#include "sbZfKaQmQueueAgeEntry.hx"
#include "sbZfKaQmQueueAgeEntryConsole.hx"
#include "sbZfKaQmQueueArrivalsEntry.hx"
#include "sbZfKaQmQueueArrivalsEntryConsole.hx"
#include "sbZfKaQmQueueByteAdjEntry.hx"
#include "sbZfKaQmQueueByteAdjEntryConsole.hx"
#include "sbZfKaQmQueueStateEntry.hx"
#include "sbZfKaQmQueueStateEntryConsole.hx"
#include "sbZfKaQmSlqCntrsEntry.hx"
#include "sbZfKaQmSlqCntrsEntryConsole.hx"
#include "sbZfKaQmWredAvQlenTableEntry.hx"
#include "sbZfKaQmWredAvQlenTableEntryConsole.hx"
#include "sbZfKaQmWredCfgTableEntry.hx"
#include "sbZfKaQmWredCfgTableEntryConsole.hx"
#include "sbZfKaQmWredCurvesTableEntry.hx"
#include "sbZfKaQmWredCurvesTableEntryConsole.hx"
#include "sbZfKaQmWredParamEntry.hx"
#include "sbZfKaQmWredParamEntryConsole.hx"
#include "sbZfKaQsAgeEntry.hx"
#include "sbZfKaQsAgeEntryConsole.hx"
#include "sbZfKaQsAgeThreshLutEntry.hx"
#include "sbZfKaQsAgeThreshLutEntryConsole.hx"
#include "sbZfKaQsDepthHplenEntry.hx"
#include "sbZfKaQsDepthHplenEntryConsole.hx"
#include "sbZfKaQsE2QAddr.hx"
#include "sbZfKaQsE2QAddrConsole.hx"
#include "sbZfKaQsE2QEntry.hx"
#include "sbZfKaQsE2QEntryConsole.hx"
#include "sbZfKaQsLastSentPriAddr.hx"
#include "sbZfKaQsLastSentPriAddrConsole.hx"
#include "sbZfKaQsLastSentPriEntry.hx"
#include "sbZfKaQsLastSentPriEntryConsole.hx"
#include "sbZfKaQsLnaPortRemapEntry.hx"
#include "sbZfKaQsLnaPortRemapEntryConsole.hx"
#include "sbZfKaQsLnaPriEntry.hx"
#include "sbZfKaQsLnaPriEntryConsole.hx"
#include "sbZfKaQsLnaNextPriEntry.hx"
#include "sbZfKaQsLnaNextPriEntryConsole.hx"
#include "sbZfKaQsPriAddr.hx"
#include "sbZfKaQsPriAddrConsole.hx"
#include "sbZfKaQsPriEntry.hx"
#include "sbZfKaQsPriEntryConsole.hx"
#include "sbZfKaQsPriLutAddr.hx"
#include "sbZfKaQsPriLutAddrConsole.hx"
#include "sbZfKaQsPriLutEntry.hx"
#include "sbZfKaQsPriLutEntryConsole.hx"
#include "sbZfKaQsQ2EcEntry.hx"
#include "sbZfKaQsQ2EcEntryConsole.hx"
#include "sbZfKaQsQueueParamEntry.hx"
#include "sbZfKaQsQueueParamEntryConsole.hx"
#include "sbZfKaQsQueueTableEntry.hx"
#include "sbZfKaQsQueueTableEntryConsole.hx"
#include "sbZfKaQsRateTableEntry.hx"
#include "sbZfKaQsRateTableEntryConsole.hx"
#include "sbZfKaQsShapeMaxBurstEntry.hx"
#include "sbZfKaQsShapeMaxBurstEntryConsole.hx"
#include "sbZfKaQsShapeRateEntry.hx"
#include "sbZfKaQsShapeRateEntryConsole.hx"
#include "sbZfKaQsShapeTableEntry.hx"
#include "sbZfKaQsShapeTableEntryConsole.hx"
#include "sbZfKaRbClassDefaultQEntry.hx"
#include "sbZfKaRbClassDefaultQEntryConsole.hx"
#include "sbZfKaRbClassDmacMatchEntry.hx"
#include "sbZfKaRbClassDmacMatchEntryConsole.hx"
#include "sbZfKaRbClassHashIPv4Only.hx"
#include "sbZfKaRbClassHashIPv4OnlyConsole.hx"
#include "sbZfKaRbClassHashIPv6Only.hx"
#include "sbZfKaRbClassHashIPv6OnlyConsole.hx"
#include "sbZfKaRbClassHashInputW0.hx"
#include "sbZfKaRbClassHashInputW0Console.hx"
#include "sbZfKaRbClassHashSVlanIPv4.hx"
#include "sbZfKaRbClassHashSVlanIPv4Console.hx"
#include "sbZfKaRbClassHashSVlanIPv6.hx"
#include "sbZfKaRbClassHashSVlanIPv6Console.hx"
#include "sbZfKaRbClassHashVlanIPv4.hx"
#include "sbZfKaRbClassHashVlanIPv4Console.hx"
#include "sbZfKaRbClassHashVlanIPv6.hx"
#include "sbZfKaRbClassHashVlanIPv6Console.hx"
#include "sbZfKaRbClassIPv4TosEntry.hx"
#include "sbZfKaRbClassIPv4TosEntryConsole.hx"
#include "sbZfKaRbClassIPv6ClassEntry.hx"
#include "sbZfKaRbClassIPv6ClassEntryConsole.hx"
#include "sbZfKaRbClassPortEnablesEntry.hx"
#include "sbZfKaRbClassPortEnablesEntryConsole.hx"
#include "sbZfKaRbClassProtocolEntry.hx"
#include "sbZfKaRbClassProtocolEntryConsole.hx"
#include "sbZfKaRbClassSourceIdEntry.hx"
#include "sbZfKaRbClassSourceIdEntryConsole.hx"
#include "sbZfKaRbPoliceCBSEntry.hx"
#include "sbZfKaRbPoliceCBSEntryConsole.hx"
#include "sbZfKaRbPoliceCfgCtrlEntry.hx"
#include "sbZfKaRbPoliceCfgCtrlEntryConsole.hx"
#include "sbZfKaRbPoliceEBSEntry.hx"
#include "sbZfKaRbPoliceEBSEntryConsole.hx"
#include "sbZfKaRbPolicePortQMapEntry.hx"
#include "sbZfKaRbPolicePortQMapEntryConsole.hx"
#include "sbZfKaSrManualDeskewEntry.hx"
#include "sbZfKaSrManualDeskewEntryConsole.hx"
#endif


enum {
  QE2000_KAEBMVTENTRY = 0,
  QE2000_KAEBMVTADDRESS,
  QE2000_KAEGMEMFIFOPARAMENTRY,
  QE2000_KAEGMEMFIFOCONTRILENTRY,
  QE2000_KAEGMEMSHAPINGENTRY,
  QE2000_KAEGMEMQCTLENTRY,
  QE2000_KAEGNOTTMEPORTREMAPADDR,
  QE2000_KAEGPORTREMAPENTRY,
  QE2000_KAEGSRCID,
  QE2000_KAEGTMEPORTREMAPADDR,
  QE2000_KAEIMEMDATAENTRY,
  QE2000_KAEIRAWSPIREADENTRY,
  QE2000_KAEPBFPRITABLEENTRY,
  QE2000_KAEPBFPRITABLEADDR,
  QE2000_KAEPCITABLEENTRY,
  QE2000_KAEPCRTABLEENTRY,
  QE2000_KAEPIPTCIDMAC,
  QE2000_KAEPIPTWOBITENTRY,
  QE2000_KAEPIP_PORT_VRID_SMAC_TABLEENTRY,
  QE2000_KAEPIP16BITREWRITE,
  QE2000_KAEPIP32BITREWRITE,
  QE2000_KAEPIPV6TCI,
  QE2000_KAEPIPMPLSLABELS,
  QE2000_KAEPIPPREPEND,
  QE2000_KAEPINSTRUCTION,
  QE2000_KAEPIPCOUNTER,
  QE2000_KAEPIPFOURBITENTRY,
  QE2000_KAEPIPPRIEXPTOSREWRITE,
  QE2000_KAEPIPONEBITENTRY,
  QE2000_KAEPIPTTLRANGE,
  QE2000_KAEPPORTTABLEENTRY,
  QE2000_KAEPPREDICTSTATE,
  QE2000_KAEPPCTEENTRY,
  QE2000_KAEPSLIMVLANRECORD,
  QE2000_KAEPVLANINDRECORD,
  QE2000_KAEPVLANINDTABLEENTRY,
  QE2000_KAQMBAACFGTABLEENTRY,
  QE2000_KAQMDEMANDCFGDATAENTRY,
  QE2000_KAQMFBLINE,
  QE2000_KAQMINGRESSPORTENTRY,
  QE2000_KAQMPORTBWCFGTABLEENTRY,
  QE2000_KAQMQUEUEARRIVALSENTRY,
  QE2000_KAQMQUEUEBYTEADJENTRY,
  QE2000_KAQMQUEUEAGEENTRY,
  QE2000_KAQMQUEUESTATEENTRY,
  QE2000_KAQMWREDPARAMENTRY,
  QE2000_KAQMWREDCFGTABLEENTRY,
  QE2000_KAQMWREDCURVESTABLEENTRY,
  QE2000_KAQMWREDAVQLENTABLEENTRY,
  QE2000_KAQMSLQCNTRSENTRY,
  QE2000_KAQMRATEDELTAMAXENTRY,
  QE2000_KAQSAGETHRESHLUTENTRY,
  QE2000_KAQSAGETHRESHKEY,
  QE2000_KAQSAGEENTRY,
  QE2000_KAQSCREDIT,
  QE2000_KAQSDEPTHHPLENENTRY,
  QE2000_KAQSE2QENTRY,
  QE2000_KAQSE2QADDR,
  QE2000_KAQSLASTSENTPRIENTRY,
  QE2000_KAQSLASTSENTPRIADDR,
  QE2000_KAQSPORTREMAPENTRY,
  QE2000_KAQSLNAPRIENTRY,
  QE2000_KAQSLNANEXTPRIENTRY,
  QE2000_KAQSPRIADDR,
  QE2000_KAQSPRIENTRY,
  QE2000_KAQSPRILUTENTRY,
  QE2000_KAQSPRILUTADDR,
  QE2000_KAQSQUEUETABLEENTRY,
  QE2000_KAQSQUEUEPARAMENTRY,
  QE2000_KAQSQ2ECENTRY,
  QE2000_KAQSRATETABLEA,
  QE2000_KAQSRATETABLEB,
  QE2000_KAQSSHAPERATEENTRY,
  QE2000_KAQSSHAPETABLEENTRY,
  QE2000_KAQSSHAPEMAXBURSTENTRY,
  QE2000_KARBCLASSDEFAULTQENTRY,
  QE2000_KARBCLASSDMACMATCHENTRY,
  QE2000_KARBCLASSHASHVLANIPV6,
  QE2000_KARBCLASSHASHIPV6ONLY,
  QE2000_KARBCLASSHASHSVLANIPV6,
  QE2000_KARBCLASSHASHSVLANIPV4,
  QE2000_KARBCLASSHASHINPUTW0,
  QE2000_KARBCLASSHASHVLANIPV4,
  QE2000_KARBCLASSPROTOCOLENTRY,
  QE2000_KARBCLASSIPV4TOSENTRY,
  QE2000_KARBCLASSIPV6CLASSENTRY,
  QE2000_KARBCLASSPORTENABLESENTRY,
  QE2000_KARBCLASSSOURCEIDENTRY,
  QE2000_KARBPOLEBSENTRY,
  QE2000_KARBPOLPORTQMAPENTRY,
  QE2000_KARBPOLCBSENTRY,
  QE2000_KARBPOLCFGCTRLENTRY,
  QE2000_KAPMLASTLINE,
  QE2000_KASRMANUALDESKEWENTRY,
  QE2000_KATESTSTAT,
  QE2000_KADDRMEM,
  QE2000_MEM_MAX_INDEX
};

#endif  /* !_SOC_SBX_QE2K_H */
