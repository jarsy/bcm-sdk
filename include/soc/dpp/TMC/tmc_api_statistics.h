/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_STATISTICS_INCLUDED__
/* { */
#define __SOC_TMC_API_STATISTICS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_64cnt.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/TMC/tmc_api_ingress_traffic_mgmt.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/* $Id$
 * Mask to select counter groups
 */

/*
 * The value of the counter in case of an error with thte general timer
 */

/*
 *  The maximal value a gtimer period can take (in ms)
 *  because its field is 30 bit long
 */
      /* ( (soc_sand_power_of_2(31) - 1) / (SOC_TMC_DFLT_TICKS_PER_SEC/1000) ) */

/*     Number of programmable counters per Incoming FAP Port   */
#define  SOC_TMC_STAT_PER_IFP_NOF_CNTS 4

/*     If get_verify as the IFP-Id for the corresponding
*     counter, disables that counter                          */

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The index of the Incoming FAP Port counted by the
   *  corresponding programmable counter (ifp_id[0]
   *  corresponds to Counter 0). Get_verify
   *  STAT_PER_IFP_CNT_DISABLE to disable the counter.
   */
  uint32 ifp_id[SOC_TMC_STAT_PER_IFP_NOF_CNTS];

} SOC_TMC_STAT_IFP_SELECT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The Ingress Queue (VOQ) for which the number of
   *  enqueued/dequeued packets is counted
   */
  uint32 voq_id;
  /*
   *  An optional enable-mask. To set just one VOQ, can be set
   *  to '0'. Counte all packets for which (ActualQ |
   *  enable_also_mask) == (voq_id | enable_also_mask). For
   *  example, setting this mask to 0x7ff8 will result in
   *  counting all VOQs with traffic class 0 (assuming Queue 0
   *  is TC 0). Range: 0 - 32K-1.
   */
  uint32 enable_also_mask;

} SOC_TMC_STAT_VOQ_SELECT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Virtual Statistics Queue (VSQ) group of the VSQ for
   *  which the number of enqueued/dequeued packets is
   *  counted. VSQ group: 0 - Group A, VSQs 0->3, Category. 1
   *  - Group B, VSQs 4->35, Category & TC. 2 - Group C, VSQs
   *  36->99,Category & CC 2/3. 3 - Group D, VSQs 100 -> 355,
   *  STAG.
   */
  SOC_TMC_ITM_VSQ_GROUP vsq_grp_id;
  /*
   *  Virtual Statistics Queue (VSQ) in-group index of the VSQ
   *  for which the number of enqueued/dequeued packets is
   *  counted. VSQ in-group index: Group A (Category) Range:
   *  0-3. Group B (Category + TC) Range: 0-32. Group C
   *  (Category 2/3 + CC) Range: 0-64. Group D (STAG) Range:
   *  0-255.
   */
  uint32 vsq_in_grp_id;
  /*
   *  An optional enable-mask. To set just one VSQ, can be set
   *  to '0'. Counte all packets for which (ActualQ |
   *  enable_also_mask) == (vsq_id | enable_also_mask). For
   *  example, if the VSQ group is CTxTC, setting this mask to
   *  0x18 will result in counting all VSQs with traffic class
   *  0 (assuming Queue 0 is TC 0). Range: 0 - 255.
   */
  uint32 enable_also_mask;

} SOC_TMC_STAT_VSQ_SELECT_INFO;

/*
 * This enumerator describe the counter available in the driver.
 * Note, the *order* of the enumerator members is impotent!!
 * This order is used in 'soc_tmcdirect_counters', 'soc_tmcdeferred_counters'
 * and 'soc_tmccounters_info'. If one add/remove enumerator member, one should
 * do the same in 'soc_tmccounters_info'.
 */
typedef enum
{
  /*
   * First value should be ZERO.
   */

  /*
   * ReceivedPackets0: Number of received packets for
   * configured FAP port 0.
   */
  SOC_TMC_IDR_RECEIVED_PKT_PORT_0_CNT = 0,
  /*
   * ReceivedPackets1: Number of received packets for
   * configured FAP port 1.
   */
  SOC_TMC_IDR_RECEIVED_PKT_PORT_1_CNT,
  /*
   * ReceivedPackets2: Number of received packets for
   * configured FAP port 2.
   */
  SOC_TMC_IDR_RECEIVED_PKT_PORT_2_CNT,
  /*
   * ReceivedPackets3: Number of received packets for
   * configured FAP port 3.
   */
  SOC_TMC_IDR_RECEIVED_PKT_PORT_3_CNT,
  /*
   * ReassemblyErrors: Number of reassembly errors
   */
  SOC_TMC_IDR_REASSEMBLY_ERR_CNT,
 /*
  * FreeBdbCount: Number of free BDBs (buffer descriptors
  * buffers). The IQM uses this counter to generate/decide
  * on packet Reject and generate Flow-Control signal.
  */
  SOC_TMC_IQM_GLOBAL_RESOURCE_CNT,
  /*
   * QEnqPktCnt: Counts enqueued packets (does not include
   * discarded packets), for the specified VOQ (programmable)
   * This register is clear on read.
   */
  SOC_TMC_IQM_VOQ_ENQ_PROG_PACKET_CNT,
  /*
   * QDeqPktCnt: Counts dequeued packets (does not include
   * discarded packets), for the specified VOQ (programmable)
   * This register is clear on read.
   */
  SOC_TMC_IQM_VOQ_DEQ_PROG_PACKET_CNT,
  /*
   * VSQEnqPktCnt: Counts enqueued packets (does not include
   * discarded packets), for the specified VSQ (programmable)
   * This register is clear on read.
   */
  SOC_TMC_IQM_VSQ_ENQ_PROG_PACKET_CNT,
  /*
   * VSQDeqPktCnt: Counts dequeued packets (does not include
   * discarded packets), for the specified VSQ (programmable)
   * This register is clear on read.
   */
  SOC_TMC_IQM_VSQ_DEQ_PROG_PACKET_CNT,
 /*
  * EnqPktCnt: Counts enqueued packets (does not include
  * discarded packets) This register is clear on read.
  */
  SOC_TMC_IQM_ENQUEUE_PACKET_CNT,
 /*
  * DeqPktCnt: Counts dequeued packets (does not include
  * discarded packets) This register is clear on read.
  */
  SOC_TMC_IQM_DEQUEUE_PACKET_CNT,
 /*
  * TotDscrdPktCnt: Counts all the packets discarded at the
  * ENQ pipe. Tail Discarded. This register is clear on read
  */
 SOC_TMC_IQM_TOTAL_DISCARDED_PACKET_CNT,
 /*
  * DeqDeletePktCnt: Counts packets discarded in the DEQ
  * process (IPS signaled Discard for the packet). Head
  * Discarded.
  */
 SOC_TMC_IQM_DELETED_PACKET_CNT,
 /*
  * IspPktCnt: Counts ISP packets passed on IQM2IRR
  * interface. Note: these packets are counted also as DEQ
  * packets (DeqPktCnt) This register is clear on read.
  */
 SOC_TMC_IQM_ISP_PACKET_CNT,
  /*
   * RjctDbPktCnt: Counts packets reject in the ENQ process
   * due to lack of free-Dbuffs
   * (Unicast/Mini/Full-multicast). This register is clear on read
   */
 SOC_TMC_IQM_REJECTED_PACKET_LACK_RESOURCE_DBUFFS_CNT,
  /*
   * RjctBdbPktCnt: Counts packets reject in the ENQ process
   * due to lack of free-BDBs (free-bdb counter value reduced
   * bellow reject thresholds). This register is clear on read
   */
 SOC_TMC_IQM_REJECTED_PACKET_LACK_RESOURCE_DBD_CNT,
  /*
   * RjctBdbProtctPktCnt: Counts packets rejected in the ENQ
   * process due to panic mode protection of free-bdb
   * resource. Note: This type of discard is not related to
   * threholds and drop-p. This register is clear on read.
   */
   SOC_TMC_IQM_REJECTED_BDB_PROTCT_PKT_CNT,
  /*
   * RjctOcBdPktCnt: Counts packets rejected in the ENQ
   * process due to lack of free-BDs (QDR entrees). Note:
   * Packet is discarded if OcBdCounter (occupied BDs) is
   * over threshold. This register is clear on read.
   * range: 30:16, access type: RO, default value: 0x0
   */
   SOC_TMC_IQM_REJECTED_OC_BD_PKT_CNT,
  /*
   * RjctSnErrPktCnt: Counts packets rejected in the ENQ
   * process due to sequence number error in the IRR (oc-768c
   * re-sequencing). This register is clear on read.
   */
   SOC_TMC_IQM_IRR_RJCT_SN_ERR_PKT_CNT,
  /*
   * RjctMcErrPktCnt: Counts packets rejected in the ENQ
   * process due to multicast error in the IRR (IRR did not
   * have place in the MC FIFO). This register is clear on read
   */
   SOC_TMC_IQM_IRR_RJCT_MC_ERR_PKT_CNT,
  /*
   * RjctRsrcErrPktCnt: Counts packets rejected in the ENQ
   * process resource error signal in the PD: the IDR run out
   * of Dbuffs while re-assembling the packet. Note: in this
   * case the IQM reject the packet but does not release
   * Dbuffs.
   */
   SOC_TMC_IQM_IDR_RJCT_RSRC_ERR_PKT_CNT,
  /*
   * RjctQnvalidErrPktCnt: Counts packets rejected in the ENQ
   * process due to Q not valid in the ENQ command.
   */
   SOC_TMC_IQM_IDR_RJCT_QNVALID_ERR_PKT_CNT,
  /*
   * CrcErrCnt: CRC Error counter.
   */
   SOC_TMC_IPT_CRC_ERR_CNT,
  /*
   * CPUReceivedPacket: CPU received packet counter.
   */
   SOC_TMC_EGQ_CPU_PACKET_CNT,
  /*
   * IPTReceivedPacket: IPT received packet counter.
   */
   SOC_TMC_EGQ_IPT_PACKET_CNT,
  /*
   * FDRReceivedPacket: FDR received packet counter.
   */
   SOC_TMC_EGQ_FDR_PACKET_CNT,
  /*
   * RQBReceivedPacket: RQP2EHP packet counter.
   */
   SOC_TMC_EGQ_RQP_PACKET_CNT,
  /*
   * RQP2EHPDisc: RQP2EHP discarded packet counter.
   */
   SOC_TMC_EGQ_RQP_DISCARD_PACKET_CNT,
  /*
   * RQP2EHPUC: RQP2EHP Unicast packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_EHP_UNICAST_PACKET_CNT,
  /*
   * EHP2PQPMCHigh: EHP2PQP Multicast High packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_EHP_MULTICAST_HIGH_PACKET_CNT,
  /*
   * EHP2PQPMCLow: EHP2PQP Multicast Low packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_EHP_MULTICAST_LOW_PACKET_CNT,
  /*
   * EHP2PQPDiscard: EHP2PQP discarded packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_EHP_DISCARD_PACKET_CNT,
  /*
   * PQP2FQPUCHigh: PQP2FQP Unicast High packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_UNICAST_HIGH_PACKET_CNT,
  /*
   * PQP2FQPUCLow: PQP2FQP Unicast Low packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_UNICAST_LOW_PACKET_CNT,
  /*
   * PQP2FQPMCHigh: PQP2FQP Multicast High packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_MULTICAST_HIGH_PACKET_CNT,
  /*
   * PQP2FQPMCLow:PQP2FQP Multicast Low packet counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_MULTICAST_LOW_PACKET_CNT,
  /*
   * PQP2FQPUCHighBytes: PQP2FQP Unicast High bytes counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_UNICAST_HIGH_BYTES_CNT,
  /*
   * PQP2FQPUCLowBytes: PQP2FQP Unicast Low bytes counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_UNICAST_LOW_BYTES_CNT,
  /*
   * PQP2FQPMCHighBytes: PQP2FQP Multicast High bytes counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_MULTICAST_HIGH_BYTES_CNT,
  /*
   * PQP2FQPMCLowBytes: PQP2FQP Multicast Low bytes counter.
   * If CheckBwToOfp is set counts OfpToCheckBw
   * packets, otherwise counts all packets.
   */
   SOC_TMC_EGQ_PQP_MULTICAST_LOW_BYTES_CNT,
  /*
   * PQPDiscardUC: PQP discarded Unicast packet counter.
   */
   SOC_TMC_EGQ_PQP_DISCARD_UNICAST_PACKET_CNT,
  /*
   * PQPDiscardMC: PQP discarded Multicast packet counter.
   */
   SOC_TMC_EGQ_PQP_DISCARD_MULTICAST_PACKET_CNT,
  /*
   * FQP2EPEPackets: FQP2EPE packet counter.
   */
   SOC_TMC_EGQ_FQP_PACKET_CNT,
  /*
   * FsrSopCnt: Counts the number of Sop received from fabric
   * and whether there was counter overflow This register is clear on read
   */
   SOC_TMC_EGQ_FABRIC_ROUTE_SOP_CNT,
  /*
   * LsrSopCnt: Counts the number of Sop received from local
   */
   SOC_TMC_EGQ_LOCAL_ROUTE_SOP_CNT,
  /*
   * CsrSopCnt: Counts the number of Sop received from CPU
   * interface and whether there was counter overflow
   */
   SOC_TMC_EGQ_CPU_INTERFACE_SOP_CNT,
  /*
   * PrpSopCnt: Packet Reassembly Output Sop Counter: Indicates the
   * number of Sop packets completed packet reassembly
   * without errors and whether there was counter overflow
   */
   SOC_TMC_EGQ_PACEKT_REASSEMBLY_OUTPUT_SOP_CNT,
  /*
   * EpePacketCounter: EPE2PNI packet counter. If bit 32 is
   * set, counter overflowed. If
   * MaskCheckBwToPacketDescriptor is cleared counts
   * accordingly, if all packet descriptors are masked counts
   * all packets.
   */
   SOC_TMC_EGQ_PACEKT_EPE_PACKET_CNT,

 /*
  * PrgCellCnt: Programmable Cells Counter: Programmable-Cells-Counter
  * -count all the cells trapped by the match filter.
  */
  SOC_TMC_FCR_PROGRAMMABLE_CELLS_CNT,
 /*
  * CreditCellsCounter: Credit-Cells-Counter.
  */
  SOC_TMC_FCR_CREDIT_CELLS_CNT,
  /*
  * FsCellsCounter: Flow Status Cells Counter: Flow Status-Cells-Counter
  */
  SOC_TMC_FCT_FLOW_STATUS_CELLS_CNT,
  /*
  * ReachCellsCounter: Reachability-Cells-Counter.
  */
  SOC_TMC_FCT_REACHABILITY_CELLS_CNT,
 /*
  * TotalCellsCounter: Total-Cells-Counter.
  */
  SOC_TMC_FCT_TOTAL_CELLS_CNT,
 /*
  * CreditDropCount: Credit_drop_count -counts dropped cells
  * according to fifo overflow. This register is clear on read
  */
  SOC_TMC_FCT_CREDIT_DROP_CNT,
 /*
  * FsDropCount: Fs_drop_count -counts dropped cells
  * according to fifo overflow. This register is clear on read
  */
  SOC_TMC_FCT_FS_DROP_CNT,
 /*
  * ReachDropCount: Reach_drop_count -counts dropped cells
  * according to fifo overflow.
  */
  SOC_TMC_FCT_REACH_DROP_CNT,
 /*
  * DataCellCnt: Data-Cell-Counter. This 31-bit counter holds
  * the number of data cells transmitted to the MACT (in 32B
  * cell for FSC and 128B for VSC ). The counter does not
  * stop at saturation. The counter is reset when read.
  */
  SOC_TMC_FDT_DATA_CELL_CNT,
 /*
  * TdmDataCellCnt: TDM-Data-Cell-Counter. This 31-bit
  * counter holds the number of TDM cells transmitted to the
  * MACT .
  */
  SOC_TMC_FDT_TDM_DATA_CELL_CNT,
 /*
  * LocalTdmDataCellCnt: Local-TDM-Data-Cell-Counter. This
  * 31-bit counter holds the number of TDM cells transmitted
  * to the EGQ
  */
  SOC_TMC_FDT_LOCAL_TDM_DATA_CELL_CNT,
 /*
  * CreditCnt: Counts issued credits that match the filter.
  */
  SOC_TMC_SCH_CREDIT_CNT,
  /*
   * CpuPacketCounter: CPU ingress received packet counter.
   */
  SOC_TMC_IRE_CPU_PACKET_CNT,
  /*
   * OlpPacketCounter: OLP ingress received packet counter.
   */
  SOC_TMC_IRE_OLP_PACKET_CNT,
 /*
  * NifaPacketCounter: NIFA ingress received packet counter.
  */
  SOC_TMC_IRE_NIFA_PACKET_CNT,
 /*
  * NifbPacketCounter: NIFb ingress received packet counter.
  */
  SOC_TMC_IRE_NIFB_PACKET_CNT,
 /*
  * RcyPacketCounter: NIFb ingress received packet counter.
  */
  SOC_TMC_IRE_RCYCLE_PACKET_CNT,

  SOC_TMC_NOF_COUNTER_TYPES /*LAST SOC_PETRA-A TYPE*/,
  /*
   *  CnmPktCnt: Counts number of generated CNM packets. This
   *  register is clear on read. Not valid for Soc_petra-A.
   */
   SOC_TMC_IQM_CNM_PKT_CNT = SOC_TMC_NOF_COUNTER_TYPES,
  /*
   *  RjctCnmPktCnt: Counts number of rejected CNM packets
   *  (CNM FIFO towards IPT was full). This register is clear
   *  on read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_REJECTED_CNM_PKT_CNT,
  /*
   *  RjctDramDynPktCnt: Counts packets rejected due to DRAM
   *  dynamic space protection. The IQM rejects packets when
   *  the dynamic space is over a configurable threshold. This
   *  is needed to keep a static area for the Q's guaranteed
   *  bytes. This register is clear on read. Not valid for
   *  Soc_petra-A.
   */
  SOC_TMC_IQM_DRAM_DYNAMIC_REJECT_PKT_CNT,
  /*
   *  DramDynSize: Presents the DRAM dynamic space in 16 bytes
   *  units. The dynamic space is calculated according to
   *  number of bytes in each Q that exceeds the Q's
   *  guaranteed space threshold GrntBytes0->3). The counter
   *  is used for reject decisions (according to thresholds:
   *  DramDynSizeRjctSet/Clr), in order to keep a guaranteed
   *  static space of the DRAM free. Not valid for
   *  Soc_petra-A.
   */
  SOC_TMC_IQM_DRAM_DYNAMIC_SIZE_CNT,
   /*
   *  OcBdbCount: Number of
   *  occupied BDBs (buffer descriptors buffers). Not valid for
   *  Soc_petra-A.
   */
  SOC_TMC_IQM_BDB_CNT,
 /*
   *  CrpsaCmdCnt: The counter holds the counting-commands
   *  received ( by the design). Note: counter is cleared on
   *  read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_CNTING_CMD_REC_PROC_A_CNT,
  /*
   *  CrpsbCmdCnt: The counter holds the counting-commands
   *  received ( by the design). Note: counter is cleared on
   *  read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_CNTING_CMD_REC_PROC_B_CNT,
  /*
   *  CrpsaCpuReqCnt: The counter holds the CPU read requests
   *  received. Note: counter is cleared on read. Not valid
   *  for Soc_petra-A.
   */
  SOC_TMC_IQM_CPU_RD_REQ_PROC_A_CNT,
  /*
   *  CrpsbCpuReqCnt: The counter holds the CPU read requests
   *  received. Note: counter is cleared on read. Not valid
   *  for Soc_petra-A.
   */
  SOC_TMC_IQM_CPU_RD_REQ_PROC_B_CNT,
  /*
   *  CrpsaOvthCntrsCnt: The counter presents the number of
   *  counters that are over the threshold. Note: maximal
   *  value is 8192. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_NOF_CNTERS_OVER_TH_A_CNT,
  /*
   *  CrpsbOvthCntrsCnt: The counter presents the number of
   *  counters that are over the threshold. Note: maximal
   *  value is 8192. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_NOF_CNTERS_OVER_TH_B_CNT,
  /*
   *  CrpsaActCntrsCnt: The coutner presents the number of
   *  active (non-emtpy) pairs of counters. Note: maximal
   *  value is 8192. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_NOF_NON_ZERO_CNTERS_A_CNT,
  /*
   *  CrpsbActCntrsCnt: The coutner presents the number of
   *  active (non-emtpy) pairs of counters. Note: maximal
   *  value is 8192. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_NOF_NON_ZERO_CNTERS_B_CNT,
  /*
   *  MtrpaCblCnt: Number of bytes assigned by the committed
   *  leaky bucket of the configured index. This register is
   *  clear on read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_METER_A_CIR_LB_NOF_BYTES_CNT,
  /*
   *  MtrpbCblCnt: Number of bytes assigned by the committed
   *  leaky bucket of the configured index. This register is
   *  clear on read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_METER_B_CIR_LB_NOF_BYTES_CNT,
  /*
   *  MtrpaEblCnt: Number of bytes assigned by the excessed
   *  leaky bucket of the configured index. This register is
   *  clear on read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_METER_A_EIR_LB_NOF_BYTES_CNT,
  /*
   *  MtrpbEblCnt: Number of bytes assigned by the excessed
   *  leaky bucket of the configured index. This register is
   *  clear on read. Not valid for Soc_petra-A.
   */
  SOC_TMC_IQM_METER_B_EIR_LB_NOF_BYTES_CNT,

  /*
   *  CreditCellsCounter: Transmitted Cr Cells Counter: Credit
   *  status cells counter:counts all the transmitted Credit
   *  cell to the mac. The counter doesn't stop at
   *  saturation. The counter is cleared when read. Not valid
   *  for Soc_petra-A.
   */
  SOC_TMC_FCR_CREDIT_CELLS_TX_CNT,
  /*
   *  FsCellsCounter: Transmitted Fs Cells Counter: Flow
   *  status cells counter:counts all the transmitted flow
   *  status cell to the mac. The counter doesn't stop at
   *  saturation. The counter is cleared when read. Not valid
   *  for Soc_petra-A.
   */
  SOC_TMC_FCT_FLOW_STATUS_CELLS_TX_CNT,
  /*
   *  CnmPacketsCnt:
   *  Counts the number of CNM packets from the EGQ. Not valid
   *  for Soc_petra-A.
   */
  SOC_TMC_EGQ_CNM_PACKET_CNT,
  /*
   *  PktAgedCnt: Number
   *  of aged packets. Not valid for Soc_petra-A.
   */
  SOC_TMC_EGQ_PACKET_AGED_CNT

} SOC_TMC_STAT_COUNTER_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *
   */
  SOC_SAND_64CNT  counters[SOC_TMC_NOF_COUNTER_TYPES];
} SOC_TMC_STAT_ALL_STATISTIC_COUNTERS;

typedef enum
{
  /*
   *  Logical presentation of errors. Printed for all print levels.
   */
  SOC_TMC_STAT_PRINT_LEVEL_ERROR_ONLY=0,
  /*
   *  Counters that present the packet walk-through.
   */
  SOC_TMC_STAT_PRINT_LEVEL_PACKET_WALKTHROUGH=1,
  /*
   *  Represents resources counters and packet walk-through.
   */
  SOC_TMC_STAT_PRINT_LEVEL_ALL=2,
  /*
   *  Total number of counter print levels.
   */
  SOC_TMC_STAT_NOF_PRINT_LEVELS=3
}SOC_TMC_STAT_PRINT_LEVEL;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

void
  SOC_TMC_STAT_ALL_STATISTIC_COUNTERS_clear(
    SOC_SAND_OUT SOC_TMC_STAT_ALL_STATISTIC_COUNTERS *info
  );

void
  SOC_TMC_STAT_IFP_SELECT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_IFP_SELECT_INFO *info
  );

void
  SOC_TMC_STAT_VOQ_SELECT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_VOQ_SELECT_INFO *info
  );

void
  SOC_TMC_STAT_VSQ_SELECT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_STAT_VSQ_SELECT_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_STAT_COUNTER_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_STAT_COUNTER_TYPE  counter_type,
    SOC_SAND_IN uint32                 short_format
  );

uint32
  soc_tmcstat_all_counters_get_and_print(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                            sampling_period
  );

void
  SOC_TMC_STAT_IFP_SELECT_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_IFP_SELECT_INFO *info
  );

void
  SOC_TMC_STAT_VOQ_SELECT_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_VOQ_SELECT_INFO *info
  );

void
  SOC_TMC_STAT_VSQ_SELECT_INFO_print(
    SOC_SAND_IN  SOC_TMC_STAT_VSQ_SELECT_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_STATISTICS_INCLUDED__*/
#endif
