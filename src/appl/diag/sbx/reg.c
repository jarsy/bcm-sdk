/*
 * $Id: reg.c,v 1.88.18.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        reg.c
 * Purpose:     SBX commands for register access
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/sync.h>
#include <sal/appl/pci.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/decode.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/dport.h>
#include <appl/diag/bslcons.h>
#include <appl/diag/bslfile.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/phyctrl.h>
#include <soc/l2x.h>
#include <soc/ll.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <bcm/stat.h>
#include <ibde.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/multicast.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/stack.h>


typedef struct {
    int reg;
    uint64 val0;
    uint64 val1;
    char *name;
    char *description;
} c3_count_reg_t;


c3_count_reg_t pr_sq[] = {
    { PR_IDP_STATS_GLOBAL_TOTAL_EOPS_RECEIVED_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "eops_received", "IDP Total EOPs Received" },
    { PR_IDP_STATS_GLOBAL_TOTAL_ACCEPTED_ENQUEUE_DONE_TO_QM_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
	    "accepted_enq_done_to_qm", "IDP Global Total Accepted Enqueue Done to QM count" },
    { PR_IDP_STATS_GLOBAL_TOTAL_PKTS_FULLY_DROPPED_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "pkts_fully_dropped", "IDP Total Pkts Fully Dropped : Pkts that did not generate an Enqueue to QM" },
    { PR_IDP_STATS_GLOBAL_CLIENT_IF_DROP_PKTS_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client_if_drop_pkts", "IDP Client IF Drop Pkt Count" },
    { PR_IDP_STATS_GLOBAL_DROP_PKTS_NO_FREE_PAGES_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "drop_pkts_no_free_pages", "IDP Dropped Pkts No Free Pages Count" },
    { PR_IDP_STATS_GLOBAL_DROP_PKTS_PB_ALMOST_FULL_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "drop_pkts_pb_almost_full", "IDP Dropped Pkts PB Almost Full Count"},
    { PR_IDP_STATS_GLOBAL_TOTAL_DROPPED_ENQUEUE_DONE_TO_QM_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "total_drop_enqueue_done_to_qm", "IDP Total Dropped Enqueue Done to QM count" },
};

c3_count_reg_t qm_sq[] = {
    { QM_INGRESS_SQ_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "sq received", "source queue packet accept counters" },
    { QM_EGRESS_SQ_ACCEPT_COUNTr, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "sq received", "source queue packet accept counters" },
    { QM_INGRESS_SQ_DROP_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "sq dropped", "source queue packet drop counters" },
    { QM_EGRESS_SQ_DROP_COUNTr,  COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "sq dropped", "source queue packet drop counters" },
};

c3_count_reg_t pt_sq[] = {
    { PT_HPTE_DEBUG_INGRESS_QM_AVAIL_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "qm avail", "Number of packets received by HPTE from QM" },
    { PT_HPTE_DEBUG_EGRESS_QM_AVAIL_COUNTr, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "qm avail", "Number of packets received by HPTE from QM" },
};

c3_count_reg_t ppe_sq[] = {
    { PP_SQ_DQ_00_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "normal pkts", "Counter for both SQUEUE and DQUEUE packets in same directions" },
    { PP_SQ_DQ_11_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "normal pkts", "Counter for both SQUEUE and DQUEUE packets in same directions" },
    { PP_SQ_DQ_01_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rc pkts", "Counter for both SQUEUE and DQUEUE packets in different directions" },
    { PP_SQ_DQ_10_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rc pkts", "Counter for both SQUEUE and DQUEUE packets in different directions" },
    { PP_RT_SQ_IG_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rt pkts", "Counter for SQUEUE packets with RT bit set" },
    { PP_RT_SQ_EG_PKT_COUNTERr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rt pkts", "Counter for SQUEUE packets with RT bit set" },
};

c3_count_reg_t ppe_sq_err[] = {
    { PP_ALL_PKT_COUNTERr,     COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "total pkts", "Aggregate Counter for all packets " },
    { PP_ERR_PKT_COUNTERr,     COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "err pkts for Ing&Egr", "Counter for Error Packets" },
};

c3_count_reg_t ped_dq[] = {
    { QM_PED_INGRESS_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "ped received", "HPRE packet accept counter" },
    { QM_PED_EGRESS_ACCEPT_COUNTr, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0), 
        "ped received", "HPRE packet accept counter" },
    { QM_PED_INGRESS_DROP_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ped dropped", "HPRE packet drop counter" },
    { QM_PED_EGRESS_DROP_COUNTr,  COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ped dropped", "HPRE packet drop counter" },
};

c3_count_reg_t pr_dq[] = {
    { PR_HDP_STATS_RCVD_PKT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rcvd_pkt", "HDP receive Pkt Count" },
    { PR_HDP_STATS_DROP_PKT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "drop_pkt", "HDP Drop Pkt Count" },
    { PR_HDP_STATS_DROP_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "dropped bytes", "HDP Drop Pkt Byte Count" },
};

c3_count_reg_t qm_dq[] = {
    { QM_AG_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "ag pkts", "Total packets accepted from HPRE with src_type=3, auto generated frames" },
    { QM_RT_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rt pkts", "Total packets accepted from HPRE with RT=1, packet redirection" },
    { QM_RP_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rp pkts", "Total packets accepted from HPRE with src_type=1, replicated frames" },
    { QM_RC_ACCEPT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rc pkts", "Total packets accepted from HPRE with src_type=2, recirculated frames" },
    { QM_TM_DROP_COUNT0r, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "ag & rt pkts dropped", "Dropped frame counts on TM, including auto generated and redirection frames" },
    { QM_TM_DROP_COUNT1r, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "rp & rc pkts dropped", "Dropped frame counts on TM, including replicated and recirculated frames" },
    { QM_AGED_DROP_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "aged pkts dropped", "IPTE packet drop counter: packets sent to IPTE with aged indication" },
};

c3_count_reg_t pt_dq[] = {
    { PT_IPTE_TX_PKT_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "tx pkts", "pkts tx - Aggregate count of all packets sent to client interfaces" },
    { PT_IPTE_DEBUG_AVAIL_IF_PARITY_ERROR_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "avail if_ parity_error", "Number of QM Avail transfers sent to IPTE with parity errors detected" },
    { PT_IPTE_DEBUG_DEQ_RESP_IF_PARITY_ERROR_COUNTr,COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "deq resp if parity err", "Number of QM Dequeue Response transfers sent to IPTE with parity errors detected" },
    { PT_IPTE_DEBUG_PB_RESP_IF_PARITY_ERROR_COUNTr, COMPILER_64_INIT(0,0),COMPILER_64_INIT(0,0),
        "PB resp if parity err", "Number of PB Response transfers sent to IPTE with parity errors detected" },
};

c3_count_reg_t ipre_adm_ctrl[] = {
    { PR_IPRE_ADM_CTRL_FR_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "frames received(FR)", "IPRE Admission Control Frame Receive Count" },
    { PR_IPRE_ADM_CTRL_FD_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "frames discarded(FD)", "IPRE Admission Control Frame Discard Count" },
    { PR_IPRE_ADM_CTRL_BD_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "bytes dropped(BD)", "IPRE Admission Control Bytes Dropped Count" },
    { PR_IPRE_ADM_CTRL_TD_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "tail discards(TD)", "IPRE Admission Control Frame Tail Discard Count" },
    { PR_IPRE_ADM_CTRL_RT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "runt discards(RT)", "IPRE Admission Control Runt Pkt Count : Runt pkts are discarded" },
    { PR_IPRE_STRICT_PRIORITY_WRITE_ERROR_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "SP write errors", "IPRE Strict Priority Write Error Count" },
    { PR_IPRE_STRICT_PRIORITY_WRITE_RUNT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "SP write runts", "IPRE Strict Priority Write Runt Count" }
};


c3_count_reg_t ipre_stat[] = {
    { PR_IPRE_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "pkts", "IPRE Client Received Pkt Count" },
    { PR_IPRE_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "bytes", "IPRE Client Received Pkt Byte Count" },
    { PR_IPRE_CL_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "CL pkts", "IPRE CL Client Received Pkt Count" },
    { PR_IPRE_CL_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "CL bytes", "IPRE CL Client Received Pkt Byte Count" },
    { PR_IPRE_XL_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XL pkts", "IPRE XL Client Received Pkt Count" },
    { PR_IPRE_XL_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XL bytes", "IPRE XL Client Received Pkt Byte Count" },
    { PR_IPRE_XT0_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT0 pkts", "IPRE XT0 Client Received Pkt Count" },
    { PR_IPRE_XT0_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT0 bytes", "IPRE XT0 Client Received Pkt Byte Count" },
    { PR_IPRE_XT1_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT1 pkts", "IPRE XT1 Client Received Pkt Count" },
    { PR_IPRE_XT1_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT1 bytes", "IPRE XT1 Client Received Pkt Byte Count" },
    { PR_IPRE_XT2_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT2 pkts", "IPRE XT2 Client Received Pkt Count" },
    { PR_IPRE_XT2_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "XT2 bytes", "IPRE XT2 Client Received Pkt Byte Count" },
    { PR_IPRE_IL_CLIENT_IF_STATS_RCVD_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "IL pkts", "IPRE Interlaken Client Received Pkt Count" },
    { PR_IPRE_IL_CLIENT_IF_STATS_RCVD_PKT_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "IL bytes", "IPRE Interlaken Client Received Pkt Byte Count" },
    { PR_IPRE_FRAME_DISCARD_ERROR_PORTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "frame discard error port", "IPRE Frame Discard Error Port" },
    { PR_IPRE_RUNT_ERROR_PORTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "runt error port", "IPRE RUNT Error Port" },
    { PR_IPRE_MOP_ERROR_PORTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "MOP error port", "IPRE MOP Error Port" },
};

uint64 PR_IDP_STATS_ACCEPT_PKT_COUNT0[64];
uint64 PR_IDP_STATS_ACCEPT_PKT_COUNT1[64];
uint64 PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[64];
uint64 PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[64];
uint64 PR_IDP_STATS_DROP_PKT_COUNT0[64];
uint64 PR_IDP_STATS_DROP_PKT_COUNT1[64];

uint64 PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[64];
uint64 PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[64];
uint64 PR_IDP_STATS_DROP_PKT_REASONS0[64];
uint64 PR_IDP_STATS_DROP_PKT_REASONS1[64];


c3_count_reg_t hpte_stat[] = {
    { PT_HPTE_DEBUG_INGRESS_DEQ_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ingress deq pkt", "Number of packets dequeued by HPTE Ingress" },
    { PT_HPTE_DEBUG_INGRESS_DEQ_RESP_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ingress deq resp pkt", "Number of packets received by HPTE Ingress on DEQ RESPONSE interface" },
    { PT_HPTE_DEBUG_EGRESS_DEQ_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "egress deq pkt", "Number of packets dequeued by HPTE Egress" },
    { PT_HPTE_DEBUG_EGRESS_DEQ_RESP_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "egress deq resp pkt", "Number of packets received by HPTE Egress on DEQ RESPONSE interface" },
    { PT_HPTE_DEBUG_PB_REQ_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "pb req", "Number of requests sent on HPTE to PB request interface" },
    { PT_HPTE_DEBUG_PB_LINE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "pb line", "Number of lines requested HPTE to PB request interface" },
    { PT_HPTE_DEBUG_PB_RESP_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "pb resp", "Number of lines received from PB response interface" },
};

c3_count_reg_t qm_stat[] = {
    { QM_DEBUG_HPRE_ENQr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "hpre enq", "HPRE enqueue debug" },
    { QM_DEBUG_HPRE_ENQDONEr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "hpre enqdone", "HPRE enqueue done debug" },
    { QM_DEBUG_HPTE_DEQr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "hpte_deq", "HPTE ingress dequeue debug" },
    { QM_DEBUG_IPTE_EG_DEQr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ipte_eg_deq", "IPTE egress dequeue fifo debug" },
    { QM_DEBUG_IPTE_EG_DEQDONEr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ipte_eg_deqdone", "IPTE egress dequeue done debug" },
    { QM_DEBUG_IPTE_IG_DEQr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ipte_ig_deq", "IPTE ingress dequeue fifo debug" },
    { QM_DEBUG_IPTE_IG_DEQDONEr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "ipte_iq_deqdone", "IPTE ingress dequeue done fifo debug" },
};


c3_count_reg_t ipte_stat[] = {
    { PT_IPTE_TX_BYTE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "tx bytes", "Aggregate count of all bytes sent to client interfaces" },
    { PT_IPTE_DEBUG_QM_AVAIL_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "qm avail", "Number of packets received by IPTE from QM" },
    { PT_IPTE_DEBUG_DEQ_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "deq pkts", "Number of packets dequeued by IPTE" },
    { PT_IPTE_DEBUG_DEQ_PAGE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "deq pages", "Number of pages dequeued by IPTE" },
    { PT_IPTE_DEBUG_DEQ_RESP_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "deq resp pkts", "Number of packets received by IPTE on DEQ RESPONSE interface" },
    { PT_IPTE_DEBUG_DEQ_RESP_PAGE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "deq resp pages", "Number of pages received by IPTE on DEQ RESPONSE interface" },
    { PT_IPTE_DEBUG_PB_REQ_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "PB req", "Number of requests sent on IPTE to PB request interface" },
    { PT_IPTE_DEBUG_PB_RESP_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "PB resp", "Number of lines received from PB response interface" },
    { PT_IPTE_DEBUG_PB_LINE_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "PB lines", "Number of lines requested IPTE to PB request interface" },
    { PT_IPTE_DEBUG_CLIENT0_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client0 pkts", "Number of packets sent from IPTE to Client 0 Interface (CL or ILKN)" },
    { PT_IPTE_DEBUG_CLIENT1_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client1 pkts", "Number of packets sent from IPTE to Client 1 Interface" },
    { PT_IPTE_DEBUG_CLIENT2_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client2 pkts", "Number of packets sent from IPTE to Client 2 Interface" },
    { PT_IPTE_DEBUG_CLIENT3_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client3 pkts", "Number of packets sent from IPTE to Client 3 Interface" },
    { PT_IPTE_DEBUG_CLIENT4_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client4 pkts", "Number of packets sent from IPTE to Client 4 Interface" },
    { PT_IPTE_DEBUG_CLIENT5_PKT_COUNTr, COMPILER_64_INIT(0,0), COMPILER_64_INIT(0,0), 
        "client5 pkts", "Number of packets sent from IPTE to Client 5 Interface" },
};

uint64 IPTE_TX_STATS_PKT_COUNT0[64];
uint64 IPTE_TX_STATS_PKT_COUNT1[64];
uint64 IPTE_TX_STATS_BYTE_COUNT0[64];
uint64 IPTE_TX_STATS_BYTE_COUNT1[64];




cmd_result_t
sbx_reg_info_print(int unit, soc_sbx_reg_info_t *reg_info, uint32 flags);

cmd_result_t  
sbx_pcim_print_regs(int unit, int pci_regs);

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
cmd_result_t
cmd_sbx_cmic_reg_get(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_set(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_mod(int unit, args_t * a);

cmd_result_t
cmd_sbx_cmic_reg_cmp(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_list(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_print_info(int unit, char *str, int clrok, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_edit(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_dump(int unit, args_t *a);

static void sbx_reg_convert_lower_case(int unit);
static void sbx_cmic_reg_print_sbx_format(int unit, soc_regaddrinfo_t *ainfo, uint64 val64, uint32 flags);

#else

cmd_result_t
cmd_sbx_cmic_reg_get(int unit, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_reg_set(int unit, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_reg_mod(int unit, args_t * a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_reg_cmp(int unit, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_reg_list(int unit, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_print_info(int unit, char *str, int clrok, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_reg_edit(int unit, args_t *a) {return CMD_NOTIMPL;};

cmd_result_t
cmd_sbx_cmic_dump(int unit, args_t *a) {return CMD_NOTIMPL;};

#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */



STATIC cmd_result_t 
sbx_cmic_reg_verify(int unit, args_t *a);

/*
 * Register Types - for getreg and dump commands
 */

typedef enum sbx_regtype_t {
    sbx_pci_cfg_reg,    /* PCI configuration space register */
    sbx_cpureg,         /* AKA PCI memory */
    sbx_sw              /* internal software state */
} sbx_regtype_t;

typedef struct sbx_regtype_entry_t {
    char *name;         /* Name of register for setreg/getreg commands */
    sbx_regtype_t type; /* Type of register (e.g. sbx_cpureg) */
    char *help;         /* Description of register for help command */
} sbx_regtype_entry_t;

STATIC sbx_regtype_entry_t sbx_regtypes[] = {
 { "PCIC",      sbx_pci_cfg_reg, "PCI Configuration space" },
 { "PCIM",      sbx_cpureg,      "PCI Memory space" },
 { "SW",        sbx_sw,          "Software state" },
};

int reg_sbx_format = 0;  /* display registers in SBX format */
int reg_skip_zero = 0;     /* skip zero registers */

/* Names of register types, indexed by sbx_regtype_t */
char *sbx_regtypenames[] = {
    "pci_cfg",        /* sbx_pci_cfg_reg,  */
    "cpu",            /* sbx_cpureg,       */
    "sw",             /* internal software state */
};

#define sbx_regtypes_count      COUNTOF(sbx_regtypes)

STATIC sbx_regtype_entry_t *sbx_regtype_lookup_name(char* str)
{
    int i;

    for (i = 0; i < sbx_regtypes_count; i++) {
        if (!sal_strcasecmp(str, sbx_regtypes[i].name)) {
            return &sbx_regtypes[i];
        }
    }
    
    return 0;
}

#ifdef   BCM_ICS
/* Do nothing on ICS */
static void
_pci_print_config(int dev)
{
}
#else
STATIC void
_pci_print_config(int dev)
{
    uint32              data;

    data = bde->pci_conf_read(dev, PCI_CONF_VENDOR_ID);
    cli_out("%04x: %08x  DeviceID=%04x  VendorID=%04x\n",
            PCI_CONF_VENDOR_ID, data,
            (data & 0xffff0000) >> 16,
            (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_COMMAND);
    cli_out("%04x: %08x  Status=%04x  Command=%04x\n",
            PCI_CONF_COMMAND, data,
            (data & 0xffff0000) >> 16,
            (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_REVISION_ID);
    cli_out("%04x: %08x  ClassCode=%06x  RevisionID=%02x\n",
            PCI_CONF_REVISION_ID, data,
            (data & 0xffffff00) >> 8,
            (data & 0x000000ff) >> 0);

    data = bde->pci_conf_read(dev, PCI_CONF_CACHE_LINE_SIZE);
    cli_out("%04x: %08x  BIST=%02x  HeaderType=%02x  "
            "LatencyTimer=%02x  CacheLineSize=%02x\n",
            PCI_CONF_CACHE_LINE_SIZE, data,
            (data & 0xff000000) >> 24,
            (data & 0x00ff0000) >> 16,
            (data & 0x0000ff00) >>  8,
            (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR0);
    cli_out("%04x: %08x  BaseAddress0=%08x\n",
            PCI_CONF_BAR0, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR1);
    cli_out("%04x: %08x  BaseAddress1=%08x\n",
            PCI_CONF_BAR1, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR2);
    cli_out("%04x: %08x  BaseAddress2=%08x\n",
            PCI_CONF_BAR2, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR3);
    cli_out("%04x: %08x  BaseAddress3=%08x\n",
            PCI_CONF_BAR3, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR4);
    cli_out("%04x: %08x  BaseAddress4=%08x\n",
            PCI_CONF_BAR4, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR5);
    cli_out("%04x: %08x  BaseAddress5=%08x\n",
            PCI_CONF_BAR5, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_CB_CIS_PTR);
    cli_out("%04x: %08x  CardbusCISPointer=%08x\n",
            PCI_CONF_CB_CIS_PTR, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_SUBSYS_VENDOR_ID);
    cli_out("%04x: %08x  SubsystemID=%02x  SubsystemVendorID=%02x\n",
            PCI_CONF_SUBSYS_VENDOR_ID, data,
            (data & 0xffff0000) >> 16,
            (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_EXP_ROM);
    cli_out("%04x: %08x  ExpansionROMBaseAddress=%08x\n",
            PCI_CONF_EXP_ROM, data, data);

    data = bde->pci_conf_read(dev, 0x34);
    cli_out("%04x: %08x  Reserved=%06x  CapabilitiesPointer=%02x\n",
            0x34, data,
            (data & 0xffffff00) >> 8,
            (data & 0x000000ff) >> 0);

    data = bde->pci_conf_read(dev, 0x38);
    cli_out("%04x: %08x  Reserved=%08x\n",
            0x38, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_INTERRUPT_LINE);
    cli_out("%04x: %08x  Max_Lat=%02x  Min_Gnt=%02x  "
            "InterruptLine=%02x  InterruptPin=%02x\n",
            PCI_CONF_INTERRUPT_LINE, data,
            (data & 0xff000000) >> 24,
            (data & 0x00ff0000) >> 16,
            (data & 0x0000ff00) >>  8,
            (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x40);
    cli_out("%04x: %08x  Reserved=%02x  "
            "RetryTimeoutValue=%02x  TRDYTimeoutValue=%02x\n",
            0x40, data,
            (data & 0xffff0000) >> 16,
            (data & 0x0000ff00) >>  8,
            (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x44);
    cli_out("%04x: %08x  PLLConf=%01x\n",
            0x44, data,
            (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x48);
    cli_out("%04x: %08x  -\n",
            0x48, data);

#ifdef VXWORKS
    
    {
#ifdef IDTRP334
        extern void sysBusErrDisable(void);
        extern void sysBusErrEnable(void);
#endif

        /* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID    0x3388
#define HINT_HB4_DEVICE_ID    0x0022

        int BusNo, DevNo, FuncNo;
        unsigned short tmp;

#ifdef IDTRP334
        sysBusErrDisable();
#endif

        /*
         * HINTCORP HB4 PCI-PCI Bridge
         */
        if (pciFindDevice(HINT_HB4_VENDOR_ID,
                          HINT_HB4_DEVICE_ID,
                          0,
                          &BusNo, &DevNo, &FuncNo) != ERROR) {

            cli_out("-------------------------------------\n");
            cli_out("HB4 PCI-PCI Bridge Status Registers  \n");
            cli_out("-------------------------------------\n");

            /* Dump the status registers */
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x06, &tmp);
            cli_out("Primary Status (%xh):   0x%x\n", 0x06, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x1e, &tmp);
            cli_out("Secondary Status (%xh): 0x%x\n", 0x1e, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x3e, &tmp);
            cli_out("Bridge Control (%xh):   0x%x\n", 0x3e, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x6a, &tmp);
            cli_out("P_SERR Status (%xh):    0x%x\n", 0x6a, tmp);
        }

#ifdef IDTRP334
        sysBusErrEnable();
#endif
    }
#endif
}
#endif /* ! ICS */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP

#define CMD_SW_DUMP_HELP_T(_bcm_module_) \
"  " #_bcm_module_ " -- dump internal " #_bcm_module_ " module state"


#if defined(BCM_QE2000_SUPPORT) || defined(BCM_BME3200) || defined(BCM_BME3200) || defined(BCM_SIRIUS_SUPPORT)
#define CMD_SBX_FABRIC_SUPPORT
#endif


#ifdef CMD_SBX_FABRIC_SUPPORT
#define SBX_FABRIC_SW_DUMP_CMD_T(_bcm_module_)          \
static cmd_result_t                                                           \
_cmd_bcm_sbx_wb_##_bcm_module_##_sw_dump(int unit, args_t *args)              \
{                                                                             \
    extern void bcm_sbx_wb_##_bcm_module_##_sw_dump(int unit);                \
    if (SOC_IS_SBX_QE(unit) || SOC_IS_SBX_BME(unit)) {                        \
        bcm_sbx_wb_##_bcm_module_##_sw_dump(unit);                            \
    } else {                                                                  \
        cli_out(\
                #_bcm_module_ " module sw_dump not supported on this unit\n"); \
        return CMD_FAIL;                                                      \
    }                                                                         \
    return CMD_OK;                                                            \
}

SBX_FABRIC_SW_DUMP_CMD_T(cosq)
SBX_FABRIC_SW_DUMP_CMD_T(fabric)
SBX_FABRIC_SW_DUMP_CMD_T(port)
SBX_FABRIC_SW_DUMP_CMD_T(stack)
SBX_FABRIC_SW_DUMP_CMD_T(trunk)
SBX_FABRIC_SW_DUMP_CMD_T(multicast)

#define SBX_FABRIC_SW_DUMP_OPTIONS "\tcosq|fabric|port|stack|trunk|multicast (for QE/BME)"
#else /* CMD_SBX_FABRIC_SUPPORT */
#define SBX_FABRIC_SW_DUMP_OPTIONS " "
#endif /* CMD_SBX_FABRIC_SUPPORT */

#if defined(CMD_SBX_FABRIC_SUPPORT)
/* cmd_sbx_sw_dump sub-command table */
static cmd_t _cmd_sw_mod_list[] = {

#ifdef CMD_SBX_FABRIC_SUPPORT
    {"cosq",    _cmd_bcm_sbx_wb_cosq_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(cosq), NULL},
    {"fabric",  _cmd_bcm_sbx_wb_fabric_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(fabric), NULL},
    {"port",    _cmd_bcm_sbx_wb_port_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(port), NULL},
    {"stack",   _cmd_bcm_sbx_wb_stack_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(stack), NULL},
    {"trunk",   _cmd_bcm_sbx_wb_trunk_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(trunk), NULL},
    {"multicast",_cmd_bcm_sbx_wb_multicast_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(multicast), NULL},
#endif /* CMD_SBX_FABRIC_SUPPORT */
};
#endif


/*
 * Function:
 *     cmd_sbx_sw_dump
 * Purpose:
 *     Provide interface to internal state of various bcm modules
 * Notes:
 *     Main driver to dump sw sub-commands
 */
cmd_result_t
cmd_sbx_sw_dump(int unit, args_t *args)
{
#if defined(CMD_SBX_FABRIC_SUPPORT)
    return subcommand_execute(unit, args, 
                              _cmd_sw_mod_list, COUNTOF(_cmd_sw_mod_list));
#else
    return BCM_E_UNAVAIL;
#endif
}

#else
#define SBX_FABRIC_SW_DUMP_OPTIONS "Unavailable"
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

char cmd_sbx_dump_usage[] = 
    "SIRIUS device only Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "DUMP [options]\n"
#else
    "DUMP [File=<name>] [Append=true|false] [raw] [hex] [all] [chg] [vert]\n\t"
    "        <TABLE>[.<COPYNO>] [<INDEX>] [<COUNT>]\n\t"
    "        [-filter <FIELD>=<VALUE>[,...]]\n\t"
    "      If raw is specified, show raw memory words instead of fields.\n\t"
    "      If hex is specified, show hex data only (for Expect parsing).\n\t"
    "      If all is specified, show even empty or invalid entries\n\t"
    "      If chg is specified, show only fields changed from defaults\n\t"
    "      If vert is specified, show fields in vertical format\n\t"
    "      (Use \"listmem\" command to show a list of valid tables)\n\t"
    "DUMP PCIC                     (PCI config space)\n\t"
    "DUMP PCIM [<START> [<COUNT>]] (CMIC PCI registers)\n\t"
    "DUMP SOC [ADDR | RVAL | DIFF] (All SOC registers)\n\t"
    "      ADDR shows only addresses, doesn't actually load.\n\t"
    "      RVAL shows reset defaults, doesn't actually load.\n\t"
    "      DIFF shows only regs not equal to their reset defaults.\n\t"
    "DUMP SOCMEM [DIFF] (All SOC memories)\n\t"
    "      DIFF shows only memories not equal to their reset defaults.\n\t"
    "DUMP MW [<START> [<COUNT>]]   (System memory, 32 bits)\n\t"
    "DUMP MH [<START> [<COUNT>]]   (System memory, 16 bits)\n\t"
    "DUMP MB [<START> [<COUNT>]]   (System memory, 8 bits)\n\t"
    "DUMP SA                       (ARL shadow table)\n\t"
    "DUMP DV ADDR                  (DMA vector)\n\t"
    "DUMP PHY [<PHYID>]            See also, the 'PHY' command.\n"
#endif
    "Other SBX device Usage:\n"
    "\n\t"
    "DUMP PCIC                     (PCI config space)\n\t"
    "DUMP PCIM                     (Directly addressable PCI registers)\n\t"
    "DUMP SW " SBX_FABRIC_SW_DUMP_OPTIONS ">\n";

cmd_result_t
cmd_sbx_dump(int unit, args_t *args)
{
    char *mem;
    int rv = CMD_OK;
    sbx_regtype_entry_t *rt;

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return cmd_sbx_cmic_dump(unit, args);
    }
#endif

    if((mem = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if ((rt = sbx_regtype_lookup_name(mem)) != NULL) {
        switch(rt->type) {
        case sbx_pci_cfg_reg:
            _pci_print_config(unit);
            break;

        case sbx_cpureg:
            rv  = sbx_pcim_print_regs(unit, 1);
            rv |= sbx_pcim_print_regs(unit, 0);
            break;
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
        case sbx_sw:
            rv = cmd_sbx_sw_dump(unit, args);
            break;
#endif
        default:
            rv = CMD_USAGE;
        }
    } else {
        rv = CMD_USAGE;
    }

    return rv;
}



cmd_result_t  
sbx_pcim_print_regs(int unit, int pci_regs)
{
    int idx = 0;
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_t *reg_info;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit (%d). \n", unit);
        return CMD_FAIL;
    }
    if (!chip_info) {
        return CMD_FAIL;
    }
    
    for (idx = 0; idx < chip_info->nregs; idx++) {
        reg_info = chip_info->regs[idx];
        /* only list direct accessible registes */
        if (pci_regs) {
            if(reg_info->offset & SOC_SBX_REG_IS_INDIRECT) {
                continue;
            } 
        } else {
            if(!(reg_info->offset & SOC_SBX_REG_IS_INDIRECT)) {
                continue;
            }
        }
        sbx_reg_info_print(unit, reg_info, REG_PRINT_RAW);
    }
    
    cli_out("\n"); /* new line in the end */
    
    return CMD_OK;
}
/*###################################################*/

static cmd_result_t
_sbx_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 *v)
{
    sbhandle  sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    int       hi COMPILER_ATTRIBUTE((unused));
    uint32  offset COMPILER_ATTRIBUTE((unused)), ioffset COMPILER_ATTRIBUTE((unused));

    offset = reg_info->offset;
    hi = !!(reg_info->ioffset & 0x08000000);
    ioffset = reg_info->ioffset & ~0x08000000;
    
    *v = 0;
    {
            *v = SAND_HAL_READ_OFFS(sbh, reg_info->offset);
    }

    return CMD_OK;
}

static cmd_result_t
_sbx_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 v)
{
    sbhandle  sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    int       hi COMPILER_ATTRIBUTE((unused));
    uint32  offset COMPILER_ATTRIBUTE((unused)), ioffset COMPILER_ATTRIBUTE((unused));

    offset = reg_info->offset;
    hi = !!(reg_info->ioffset & 0x08000000);
    ioffset = reg_info->ioffset & ~0x08000000;
    

    {
            SAND_HAL_WRITE_OFFS(sbh, reg_info->offset, v);
    }

    return CMD_OK;
}


int sbx_chip_info_show(void)
{
    int idx = 0;

    cli_out("soc_sbx_chip_count=%d\n\r", soc_sbx_chip_count);

    for (idx=0; idx<soc_sbx_chip_count; idx++)
    {
        cli_out("%-8d%-8d\n\r", idx, soc_sbx_chip_list[idx].id);
    }

    return 0;
}

/*###################################################*/
/*
 * Function:    sbx_chip_info_get
 * Purpose:     Returns a chip_info based on passed in chip_id
 *     NOTE: if not found, then create on for this chip_id
 */
cmd_result_t
sbx_chip_info_get(int chip_id, soc_sbx_chip_info_t **chip_info, int create)
{
    int idx = 0;
    int rv = CMD_FAIL;

    for (idx = 0; idx < soc_sbx_chip_count; idx++) {
        if (soc_sbx_chip_list[idx].id == chip_id) {
            *chip_info = &soc_sbx_chip_list[idx];
            return CMD_OK;
        }
    }

    if (create){
      /* create the list on the fly if not found */
      switch (chip_id){
#if defined(BCM_QE2000_SUPPORT)
      case SOC_INFO_CHIP_TYPE_QE2000: {
        extern int sbx_qe2000_reg_list_init(void);
        if ((rv = sbx_qe2000_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
#if defined(BCM_BME3200_SUPPORT)
      case SOC_INFO_CHIP_TYPE_BME3200: {
        extern int sbx_bme3200_reg_list_init(void);
        if ((rv = sbx_bme3200_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
#if defined(BCM_BM9600_SUPPORT)
      case SOC_INFO_CHIP_TYPE_BM9600:{
        extern int sbx_bm9600_reg_list_init(void);
        if ((rv = sbx_bm9600_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
      } /* switch */
      
      /* Try again to find the right info structure */
      for (idx = 0; idx < soc_sbx_chip_count; idx++) {
        if (soc_sbx_chip_list[idx].id == chip_id) {
          *chip_info = &soc_sbx_chip_list[idx];
          return CMD_OK;
        }
      }
    } /* if create */
    return rv;
}


/*
 * Function:    sbx_reg_info_list_get
 * Purpose:     Finds list of registers matching regname
 */
cmd_result_t 
sbx_reg_info_list_get(soc_sbx_chip_info_t *chip_info,
                      soc_sbx_reg_info_list_t *reginfo_l,
                      char *regname, int exactmatch)
{
    int idx = 0;
    soc_sbx_reg_info_t *reg_info = NULL;

    if ((!chip_info) || (!reginfo_l)) {
        return CMD_FAIL;
    }

    for (idx = 0; idx < chip_info->nregs; idx++) {
        reg_info = chip_info->regs[idx];
        if (exactmatch) {
            if (!sal_strcasecmp(reg_info->name, regname)) {
                reginfo_l->idx[reginfo_l->count++] = idx;
                break;
            }
        } else {
            if (strstr(reg_info->name, regname)) {
                reginfo_l->idx[reginfo_l->count++] = idx;
            }
        }
    }

    return CMD_OK;
}

void
sbx_reg_info_list_prune(soc_sbx_chip_info_t *chip_info,
                        soc_sbx_reg_info_list_t *reginfo_l,
                        char *regname)
{
    int i;
    int idx;
    soc_sbx_reg_info_t *reg_info = NULL;

    for (i = 0; i < reginfo_l->count; i++) {
        idx = reginfo_l->idx[i];
        if (idx >= 0) {
            reg_info = chip_info->regs[idx];
            if (strstr(reg_info->name, regname)) {
                reginfo_l->idx[i] = -1;
            }
        }
    }
}

/* 
 * Function:    sbx_reg_info_get
 * Purpose:     Find a reg given a name or an addr
 */
cmd_result_t 
sbx_reg_info_get(soc_sbx_chip_info_t *chip_info, soc_sbx_reg_info_t **reg_info,
                 char *reg)
{
    int     idx = 0;
    uint32  regaddr = 0;
    int     isaddr = 0;
    int     found = 0;

    if ((!chip_info) || (!reg_info)) {
        return CMD_FAIL;
    }

    if (isint(reg)) {
        regaddr = parse_integer(reg);
        isaddr = 1;
    }

    for (idx = 0; idx < chip_info->nregs; idx++) {
        if (isaddr) {
            if (chip_info->regs[idx]->offset == regaddr) {
                found = 1;
                break;
            }
        } else {
            if (!sal_strcasecmp(chip_info->regs[idx]->name, reg)) {
                found = 1;
                break;
            }
        }
    }

    if (found) {
        *reg_info = chip_info->regs[idx];
        return CMD_OK;
    }

    return CMD_FAIL;
}

/*
 * Function:    sbx_reg_fields_list
 * Purpose:     List all fields in the reg_info
 */
cmd_result_t
sbx_reg_fields_list(soc_sbx_reg_info_t *reg_info)
{
    int idx = 0;
    soc_sbx_field_info_t *fld_info = NULL;
    
    if (!reg_info) {
        return CMD_FAIL;
    }

    cli_out("%s: \n", reg_info->name);
    for (idx = 0; idx < reg_info->nfields; idx++) {
        fld_info = reg_info->fields[idx];
        if (fld_info) {
            cli_out("    FIELD:%s,  MASK:0x%x,  MSB:%d,  LSB:%d,  DEFAULT:%x \n", 
                    fld_info->name, fld_info->mask, fld_info->msb, 
                    fld_info->lsb, fld_info->default_val);
        }
    }
    return CMD_OK;
}


/*
 * Function:    sbx_reg_list_all
 * Purpose:     List all SBX registers for passed in chip_info
 */
cmd_result_t
sbx_reg_list_all(soc_sbx_chip_info_t *chip_info)
{
    int idx = 0;
    
    if (!chip_info) {
        return CMD_FAIL;
    }

    cli_out("List of all %s Registers: \n", chip_info->name);
    for (idx = 0; idx < chip_info->nregs; idx++) {
        if ((idx % 3) == 2) {
            cli_out("\n"); /* new line every 4 regs */
        }
        cli_out("%s  ", chip_info->regs[idx]->name);
    }

    cli_out("\n"); /* new line in the end */

    return CMD_OK;
}


/* 
 * Function:    sbx_reg_list
 * Purpose:     prints list of regs
 */
cmd_result_t 
sbx_reg_list(soc_sbx_chip_info_t *chip_info, char *regname)
{
    int idx = 0;
    int rv = CMD_OK;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;

    if (!chip_info) {
        return CMD_FAIL;
    }

    if (!regname) {
        return CMD_USAGE;
    }

    if (*regname == '*') {
        return sbx_reg_list_all(chip_info);
    }

    /* Now search for exact or partial matches */
    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        cli_out("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    /* Initialize to empty */
    reginfo_l->count = 0;

    /* first find exact match */
    if (sbx_reg_info_list_get(chip_info,reginfo_l ,regname , 1) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (reginfo_l->count == 1) {
        /* found exact match */
        rv = sbx_reg_fields_list(chip_info->regs[reginfo_l->idx[0]]);
        sal_free(reginfo_l);
        return rv;
    }

    reginfo_l->count = 0; /* zero out the count */
    /* Now find partial matches */
    if (sbx_reg_info_list_get(chip_info,reginfo_l ,regname , 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    /* now list all regs */
    for (idx = 0; idx < reginfo_l->count; idx++) {
        cli_out("%s  ", chip_info->regs[reginfo_l->idx[idx]]->name);
        if ((idx % 3) == 2) {
            cli_out("\n"); /* new line every 4 regs */
        }
    }

    if (idx % 3) {
        cli_out("\n"); /* new line in the end */
    }

    sal_free(reginfo_l);
    return CMD_OK;
}

STATIC void
_sbx_reg_print(uint32 regval, soc_sbx_reg_info_t *reg_info, uint32 flags)
{
    int         idx = 0;
    int         def_chg = 0;
    uint32      fld_val;
    soc_sbx_field_info_t    *fld_info = NULL;

    if (flags & REG_PRINT_HEX) {
        cli_out("0x%08x", regval);
    } else if (flags & REG_PRINT_RAW) {
        cli_out("%-30s: 0x%08x", reg_info->name, regval);
    } else {
      cli_out("%s: 0x%08x\n", reg_info->name, regval);
        for (idx = 0; idx < reg_info->nfields; idx++) {
            fld_info = reg_info->fields[idx];
            fld_val = ((regval & fld_info->mask) >> fld_info->shift);
            if ((!(flags & REG_PRINT_CHG)) 
                || (fld_val != fld_info->default_val)) {
                cli_out("\t%30s:  0x%08x\n", fld_info->name, fld_val);
                def_chg = 1;
            }
        }
        if ((flags & REG_PRINT_CHG) && (!def_chg)) {
            cli_out("ALL FIELDS SET TO DEFAULT VALUES");
        }
        cli_out("\n");
    }
}

/*
 * Function:    sbx_reg_print
 * Purpose:     Print the values of the register
 */
cmd_result_t
sbx_reg_info_print(int unit, soc_sbx_reg_info_t *reg_info, uint32 flags)
{
    uint32       regval;
    cmd_result_t rv;

    if (!reg_info) {
        return CMD_FAIL;
    }

    rv = _sbx_read_reg(unit, reg_info, &regval);
    if (rv != CMD_OK) {
        return rv;
    }
    
    _sbx_reg_print(regval, reg_info, flags);

    {
        return CMD_OK;
    }
}

/*
 * Function:    sbx_reg_addr_print
 * Purpose:     Check if a register address is valid and print it
 */
cmd_result_t
sbx_reg_print(int unit, char *reg, uint32 flags)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_t *reg_info = NULL;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info, 1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }
    
    if (sbx_reg_info_get(chip_info, &reg_info, reg) != CMD_OK) {
        soc_sbx_reg_info_list_t *reginfo_l = NULL;
        int idx;

        /* Now search for exact or partial matches */
        reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");

        if (!reginfo_l) {
            cli_out("ERROR: sbx_reg_list failed. Out of Memory \n");
            return CMD_FAIL;
        }
        /* Zero out the struct. */
        sal_memset(reginfo_l, 0, sizeof(soc_sbx_reg_info_list_t));
   
        reginfo_l->count = 0; /* zero out the count */
        /* Now find partial matches */
        if (sbx_reg_info_list_get(chip_info,reginfo_l ,reg , 0) != CMD_OK) {
            sal_free(reginfo_l);
            cli_out("ERROR: Invalid Register: %s on %s \n", reg, chip_info->name);
            return CMD_FAIL;
        }

        /* now dump values for all partial match regs */
        for (idx = 0; idx < reginfo_l->count; idx++) {
             if (sbx_reg_info_get(chip_info, 
                                  &reg_info, 
                                  chip_info->regs[reginfo_l->idx[idx]]->name) == CMD_OK) {
                 sbx_reg_info_print(unit, reg_info, flags);
             }
             cli_out("\n"); /* new line in the end */
        }
        sal_free(reginfo_l);

        return CMD_OK;
    }

    return sbx_reg_info_print(unit, reg_info, flags);
}


/*
 * Function:    sbx_reg_name_set
 * Purpose:     Check if a register name is valid and set it
 */
cmd_result_t
sbx_reg_set(int unit, char *reg, args_t *a, int mod)
{
    int         idx = 0;
    uint32      regval = 0;
    uint32      oldregval = 0;
    uint32      value = 0;
    char        *valstr = NULL;
    int         valstrlen = 0;
    uint32      regmask = 0;
    char        fldname [SBX_FIELD_NAME_LEN_MAX];
    char        *name = NULL;
    soc_sbx_chip_info_t     *chip_info = NULL;
    soc_sbx_reg_info_t      *reg_info = NULL;
    soc_sbx_field_info_t    *fld_info = NULL;
    cmd_result_t rv;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }

    if (sbx_reg_info_get (chip_info, &reg_info, reg) != CMD_OK) {
        cli_out("ERROR: Invalid Register: %s \n", reg);
        return CMD_FAIL;
    }

    name = ARG_GET(a);
    if (!name) {
        return CMD_USAGE;
    }

    if (isint(name)) {
        /* complete register value given */
        regval = parse_integer(name);
        return _sbx_write_reg(unit, reg_info, regval);
    }

    /* individual fields specified */
    for (;;) {
        if (!name) {
            break;
        }
        valstr = strchr(name, '=');
        if ((valstr == NULL) || (valstr >= (name + (sal_strlen(name) - 1)))) {
            return CMD_USAGE;
        }
        valstr++; /* value starts after = sign */
        if (!isint(valstr)) {
            return CMD_USAGE;
        }
        
        valstrlen = (sal_strlen(name) - sal_strlen(valstr) - 1);
        if (valstrlen > (SBX_FIELD_NAME_LEN_MAX - 1)) {
            return CMD_USAGE;
        }
        value =  parse_integer(valstr);
        strncpy(fldname, name, (sal_strlen(name) - sal_strlen(valstr) - 1));
        *(fldname + valstrlen) = '\0';

        /* check for valid field name */
        fld_info = NULL;
        for (idx = 0; idx < reg_info->nfields; idx++) {
            if (!sal_strcasecmp(reg_info->fields[idx]->name, fldname)) {
                fld_info = reg_info->fields[idx];
                break;
            }
        }
        if (!fld_info) {
            cli_out("ERROR: Invalid field: %s specified for register: %s  \n",
                    fldname, reg_info->name);
            return CMD_FAIL;
        }

        regval |= (value << fld_info->shift);
        regmask |= fld_info->mask;

        name = ARG_GET(a);
    }

    if (!regmask) {
        return CMD_USAGE;
    }

    rv = _sbx_read_reg(unit, reg_info, &oldregval);
    if (rv != CMD_OK) {
        return rv;
    }
    oldregval &= (~regmask); /* zero out the fields being modified */
    rv = _sbx_write_reg(unit, reg_info, ((mod?oldregval:0) | regval));
    return CMD_OK;
}

STATIC int
_sbx_reg_get_by_type(int unit, uint32 regaddr, sbx_regtype_t regtype,
                     uint64 *outval, uint32 flags)
{
    int           rv = CMD_OK;

    switch (regtype) {
    case sbx_pci_cfg_reg:
        if (regaddr & 3) {
            cli_out("ERROR: PCI config addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
        }
        break;

    case sbx_cpureg:
        if (regaddr & 3) {
            cli_out("ERROR: PCI memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
    
            COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));           
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }
    
    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
        if (flags & REG_PRINT_HEX) {
            cli_out("%08x\n",
                        COMPILER_64_LO(*outval));
        } else {
            char buf[80];
            
            format_uint64(buf, *outval);
            
            cli_out("%s[0x%x] = %s\n",
                    sbx_regtypenames[regtype], regaddr, buf);
        }
    }

    return rv;
}

char cmd_sbx_reg_get_usage[] =
  "SIRIUS only Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
  "Parameters: [hex|raw|chg] [<REGTYPE>] <REGISTER>\n\t"
  "If <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
  "<REGISTER> designates offset, address, or symbolic name.\n\t"
  "If hex is specified, dumps only a hex value (for Expect parsing).\n\t"
  "If raw is specified, no field decoding is done.\n\t"
  "If chg is specified, show only fields/regs changed from defaults.\n\t"
  "For a list of register types, use \"dump\".\n"
  "\n"
  "\n"
  "Other SBX device Usage:\n"
  "Parameters: [hex|raw|chg] <REGISTER> \n \t"
  "If hex is specified, dumps only a hex value (for Expect parsing).\n\t"
  "If raw is specified, no field decoding is done.\n\t"
  "If chg is specified, show only fields/regs changed from defaults.\n\t"
  "For a list of register types, use \"listreg\".\n"
#endif
;

/*
 * Function:    cmd_sbx_reg_get
 * Purpose:         Register Read command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_get(int unit, args_t *a)
{
    char                *name;
    uint32              flags = REG_PRINT_DO_PRINT;
    uint32              regaddr = 0;
    sbx_regtype_entry_t *rt;
    int                 rv = CMD_OK;
    uint64              rval;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    name = ARG_CUR(a);

    if (name != NULL && !sal_strcasecmp(name, "sbx")) {
#if defined(BCM_CALADAN3_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
      reg_sbx_format = (reg_sbx_format == 0);
      name = ARG_GET(a);
      sbx_reg_convert_lower_case(unit);
#endif
      return CMD_OK;
    } else if (name != NULL && !sal_strcasecmp(name, "nz")) {
      reg_skip_zero = (reg_skip_zero == 0);
      return CMD_OK;
    }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return cmd_sbx_cmic_reg_get(unit, a);
    }
#endif

    name = ARG_GET(a);

    /* 
     * If first arg specifies print options, take it and use the next argument
     * as the name or address
     */

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "chg")) {
            flags |= REG_PRINT_CHG;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "sbx")) {
	    reg_sbx_format = (reg_sbx_format == 0);
	    return CMD_OK;
        } else if (name != NULL && !sal_strcasecmp(name, "nz")) {
	    reg_skip_zero = (reg_skip_zero == 0);
	    return CMD_OK;
        } else {
            break;
        }
    }
    
    if (name == NULL) {
        return CMD_USAGE;
    }

    /* parse the type of register, default to PCIM */
    if ((rt = sbx_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_regtype_lookup_name("PCIM"); /* i.e. sbx_cpureg */
    }

    assert(rt);

    if (isint(name)) {
        /* Numerical address given */
        regaddr = parse_integer(name);
    } else {
        if (*name == '$') {
            name++;
        }
    }

    switch(rt->type) {
    case sbx_cpureg:
        rv = sbx_reg_print(unit, name, flags);
        break;
        
    default:
        if (!isint(name)) {
            cli_out("ERROR: Numeric register address expected\n");
            rv = CMD_FAIL;
        } else {
            rv = _sbx_reg_get_by_type(unit, regaddr, rt->type, &rval, flags);
        }
    }
    
    return rv;
}

char cmd_sbx_reg_set_usage[] = 
  "SIRIUS only Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
  "1. Parameters: [<REGTYPE>] <REGISTER> <VALUE>\n\t"
  "If <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
  "<REGISTER> is offset, address, or symbolic name.\n"
  "2. Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
  "<REGISTER> is SOC register offset or symbolic name.\n\t"
  "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
  "for example: L3_ENA=0,CPU_PRI=1.\n\t"
  "Fields not specified in the list are set to zero.\n\t"
  "For a list of register types, use \"help dump\".\n"
  "\n"
  "\n"
  "Other SBX device Usage:\n"
  "1. Parameters: <REGISTER> <VALUE>\n\t"
  "2. Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
  "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
  "Fields not specified in the list are set to zero.\n"
#endif
;

/*
 * Function:    cmd_sbx_reg_set
 * Purpose:         Register Write command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_set(int unit, args_t *a)
{
    char    *name;
    uint32  regaddr;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return cmd_sbx_cmic_reg_set(unit, a);
    }
#endif

    name = ARG_GET(a);

    if (name == NULL) {
    return CMD_USAGE;
    }

    if (isint(name)) {                 
        /* Numerical address given */
        regaddr = parse_integer(name);
        if (regaddr & 3) {
            cli_out("ERROR: Address must be multiple of 4 \n");
            return CMD_FAIL;
        }
    } else {
        if (*name == '$') {
            name++;
        }
    }

    return sbx_reg_set(unit, name, a, 0);
}

char cmd_sbx_reg_modify_usage[] = 
    "Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
    "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
    "Fields not specified in the list are left unchanged.\n";

/*
 * Function:    cmd_sbx_reg_modify
 * Purpose:         Register Read/Modify/Write command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_modify(int unit, args_t *a)
{
    char    *name;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return cmd_sbx_cmic_reg_mod(unit, a);
    }
#endif

    name = ARG_GET(a);

    if (name == NULL) {
    return CMD_USAGE;
    }

    if (*name == '$') {
        name++;
    }

    return sbx_reg_set(unit, name, a, 1);
}


char cmd_sbx_reg_verify_usage[] =
    "Parameters: <REGISTER> <VALUE>\n \t"
    "Write a register and read it back to verify the value written.\n";
/*
 * Function:    cmd_sbx_reg_verify
 * Purpose:         Write device register and read it back on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_verify(int unit, args_t *a)
{
    char                *name,*value;
    int                 rv = CMD_OK;
    uint32      regval = 0;
    soc_sbx_chip_info_t     *chip_info = NULL;
    soc_sbx_reg_info_t      *reg_info = NULL;
    uint32                  set_val;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (SOC_IS_SBX_SIRIUS(unit)) {
        cli_out("ERROR: Command not supported on Sirius devie \n");
        return CMD_FAIL;
    }

	
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return sbx_cmic_reg_verify(unit, a);
    }
#endif

    /* remember register name */
    name = ARG_GET(a);
    if (name == NULL) {
        return CMD_USAGE;
    }
    value = ARG_GET(a);
    if (value == NULL) {
        return CMD_USAGE;
    }

    ARG_PREV(a);
    ARG_PREV(a);

    /* write register */
    rv = cmd_sbx_reg_set(unit, a);
    if (rv < 0 ) {
        return rv;
    }

    set_val = parse_integer(value);

    /* read it back */
    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }
    
    if (sbx_reg_info_get (chip_info, &reg_info, name) != CMD_OK) {
        cli_out("ERROR: Invalid Register: %s \n", name);
        return CMD_FAIL;
    }
    
    _sbx_read_reg(unit, reg_info, &regval);
    if(set_val != regval) {
        cli_out("Error. Written Value 0x%08X != Read Value 0x%08X\n",
                set_val, regval);
    }
    return CMD_OK;
}


char cmd_sbx_reg_list_usage[] =
  "SIRIUS only Usage: listreg [options] regname [value]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
  "Options:\n"
  "     -alias/-a       display aliases\n"
  "     -summary/-s     display single line summary for each reg\n"
  "     -counters/-c    display counters\n"
  "     -ed/-e          display error/discard counters\n"
  "     -type/-t        display registers grouped by block type\n"
  "If regname is '*' or does not match a register name, a substring\n"
  "match is performed.  [] indicates register array.\n"
  "If regname is a numeric address, the register that resides at that\n"
  "address is displayed.\n"
  "\n"
  "\n"
  "Other SBX device Usage: Listreg regname \n"
  "If regname is '*' or does not match a register name, a substring\n"
  "match is performed.  [] indicates register array.\n"
  "If regname is a numeric address, the register that resides at that\n"
  "address is displayed.\n"
#endif
;

/*
 * Function:    cmd_sbx_reg_list
 * Purpose:         List of Registers
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_list(int unit, args_t *a)
{
    char *name;

    soc_sbx_chip_info_t *chip_info = NULL;

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit) || SOC_IS_SBX_CALADAN3(unit)) {
        return cmd_sbx_cmic_reg_list(unit, a);
    }
#endif

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit (%d). \n", unit);
        return CMD_FAIL;
    }

    name = ARG_GET(a);
    sbx_str_tolower(name);
    return sbx_reg_list(chip_info, name);
}

static cmd_result_t
_cmd_sbx_print_reg_class(int unit, args_t *a, soc_sbx_chip_info_t *chip_info,
                         soc_sbx_reg_info_list_t *reginfo_l, int flags)
{
    int raw = 0;
    int hex = 0;
    int clear = 0;
    soc_sbx_reg_info_t *reg_info;
    uint32 regval;
    int i;
    int idx;
    pbmp_t pbmp;
    bcm_port_t port;
    cmd_result_t rv;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "raw", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &raw, NULL);
        parse_table_add(&pt, "hex", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &hex, NULL);
        parse_table_add(&pt, "clear", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &clear, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    for (i = 0; i < reginfo_l->count; i++) {
        idx = reginfo_l->idx[i];
        if (idx >= 0) {
            reg_info = chip_info->regs[idx];
            rv = _sbx_read_reg(unit, reg_info, &regval);
            if (rv != CMD_OK) {
                return rv;
            }
            if (regval != 0) {
                _sbx_reg_print(regval, reg_info,
                               (flags
                                | (raw ? REG_PRINT_RAW : 0)
                                | (hex ? REG_PRINT_HEX : 0)));
		cli_out("\n");
                if (clear) {
                    /* Clear W1TC fields */
                    rv = _sbx_write_reg(unit, reg_info, regval);
                    if (rv != CMD_OK) {
                        return rv;
                    }
                    /* Clear other fields (e.g. counters) */
                    rv = _sbx_write_reg(unit, reg_info, 0);
                    if (rv != CMD_OK) {
                        return rv;
                    }
                }
            }
        }
    }

    /* Clear software counter statistics */
    if (clear) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        PBMP_ITER(pbmp, port) {
            bcm_stat_clear(unit, port);
        }
    }

    return CMD_OK;
}


char cmd_sbx_print_errors_usage[] =
    "Usage: PrintErrors [args]\n"
    "    hex    - prints in hex format\n"
    "    raw    - prints in raw format\n"
    "    clear  - writes 1s to clear\n";

cmd_result_t
cmd_sbx_print_errors(int unit, args_t *a)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;
    int rv;
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    char *str = "error";
#endif

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)){
        rv = cmd_sbx_cmic_print_info(unit, str, 1, a);
        return (rv);
    }
#endif

    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        cli_out("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    reginfo_l->count = 0;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit %d \n", unit);
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (sbx_reg_info_list_get(chip_info, reginfo_l, "error", 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    sbx_reg_info_list_prune(chip_info, reginfo_l, "_mask");

    rv = _cmd_sbx_print_reg_class(unit, a, chip_info, reginfo_l, 0);

    sal_free(reginfo_l);
    return rv;
}

#if defined(BCM_CALADAN3_SUPPORT)

void sbx_c3_gather_hw_counters(int unit){
    int i, result;
    uint32 n1, n2;
    ipte_tx_stats_entry_t meminfo0;
    ipte_tx_stats_entry_t meminfo1;

    int pt_blk0 = soc_sbx_block_find(unit, SOC_BLK_PT, 0);
    int pt_blk1 = soc_sbx_block_find(unit, SOC_BLK_PT, 1);

    for (i=0; i<sizeof(pr_sq)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, pr_sq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, pr_sq[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(pr_sq[i].val0, n1);
        COMPILER_64_ADD_32(pr_sq[i].val1, n2);
    }

    for (i=0; i<sizeof(qm_sq)/sizeof(c3_count_reg_t); i=i+2) {
        soc_reg32_get(unit, qm_sq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, qm_sq[i+1].reg, 0, 0, &n2);
        COMPILER_64_ADD_32(qm_sq[i].val0, n1);
        COMPILER_64_ADD_32(qm_sq[i+1].val1, n2);
    }

    for (i=0; i<sizeof(pt_sq)/sizeof(c3_count_reg_t); i=i+2) {
        soc_reg32_get(unit, pt_sq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, pt_sq[i+1].reg, 0, 0, &n2);
        COMPILER_64_ADD_32(pt_sq[i].val0, n1);
        COMPILER_64_ADD_32(pt_sq[i+1].val1, n2);
    }

    for (i=0; i<sizeof(ppe_sq)/sizeof(c3_count_reg_t); i=i+2) {
        soc_reg32_get(unit, ppe_sq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, ppe_sq[i+1].reg, 0, 0, &n2);
        COMPILER_64_ADD_32(ppe_sq[i].val0, n1);
        COMPILER_64_ADD_32(ppe_sq[i+1].val1, n2);
    }

    for (i=0; i<sizeof(ppe_sq_err)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, ppe_sq_err[i].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq_err[i].val0, n1);
    }

    for (i=0; i<sizeof(ped_dq)/sizeof(c3_count_reg_t); i=i+2) {
        soc_reg32_get(unit, ped_dq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, ped_dq[i+1].reg, 0, 0, &n2);
        COMPILER_64_ADD_32(ped_dq[i].val0, n1);
        COMPILER_64_ADD_32(ped_dq[i+1].val1, n2);
    }

    for (i=0; i<sizeof(pr_dq)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, pr_dq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, pr_dq[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(pr_dq[i].val0, n1);
        COMPILER_64_ADD_32(pr_dq[i].val1, n2);
    }

    for (i=0; i<sizeof(qm_dq)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, qm_dq[i].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_dq[i].val0, n1);
    }

    for (i=0; i<sizeof(pt_dq)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, pt_dq[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, pt_dq[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(pt_dq[i].val0, n1);
        COMPILER_64_ADD_32(pt_dq[i].val1, n2);
    }

    
    for (i=0; i<sizeof(ipre_adm_ctrl)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(ipre_adm_ctrl[i].val0, n1);
        COMPILER_64_ADD_32(ipre_adm_ctrl[i].val1, n2);
    }

    for (i=0; i<sizeof(ipre_stat)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, ipre_stat[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, ipre_stat[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(ipre_stat[i].val0, n1);
        COMPILER_64_ADD_32(ipre_stat[i].val1, n2);
    }

    for (i=0; i<64; i++) {
        soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 0, i, &n1);
        soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 1, i, &n2);
        COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT0[i],n1);
        COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT1[i],n2);
        
        soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 0, i, &n1);
        soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 1, i, &n2);
        COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i],n1);
        COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i],n2);
        
        soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 0, i, &n1);
        soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 1, i, &n2);
        COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT0[i],n1);
        COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT1[i],n2);
        
        soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 0, i, &n1);
        soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 1, i, &n2);
        COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i],n1);
        COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i],n2);
    }

    for (i=0; i<sizeof(hpte_stat)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, hpte_stat[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, hpte_stat[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(hpte_stat[i].val0, n1);
        COMPILER_64_ADD_32(hpte_stat[i].val1, n2);
    }

    
    for (i=0; i<sizeof(qm_stat)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, qm_stat[i].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_stat[i].val0, n1);
    }


    for (i=0; i<sizeof(ipte_stat)/sizeof(c3_count_reg_t); i++) {
        soc_reg32_get(unit, ipte_stat[i].reg, 0, 0, &n1);
        soc_reg32_get(unit, ipte_stat[i].reg, 1, 0, &n2);
        COMPILER_64_ADD_32(ipte_stat[i].val0, n1);
        COMPILER_64_ADD_32(ipte_stat[i].val1, n2);
    }
    for (i=0; i<64; i++) {
        result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk0, i, &meminfo0);
        if(SOC_E_NONE != result) {
            cli_out("fail to read IPTE_TX_STATS.pt0 [%d]\n", i);
        }
        result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk1, i, &meminfo1);
        if(SOC_E_NONE != result) {
            cli_out("fail to read IPTE_TX_STATS.pt1 [%d]\n", i);
        }
        
        soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, PKT_COUNTf, (uint32*)&n1);
        soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, PKT_COUNTf, (uint32*)&n2);
        
        
        COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT0[i], n1);
        COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT1[i], n2);
        
        soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, BYTE_COUNTf, (uint32*)&n1);
        soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, BYTE_COUNTf, (uint32*)&n2);
        
        COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT0[i], n1);
        COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT1[i], n2);
    }

}


static void sbx_c3_counters(int unit) {
    int i;
    uint32 n1, n2;
    uint32 last1, last2;
    char		buf_val1[32], buf_val2[32], tabby[120], line[120];
    int			tabwidth = soc_property_get(unit, spn_DIAG_TABS, 8);
    ipte_tx_stats_entry_t meminfo0;
    ipte_tx_stats_entry_t meminfo1;

    int pt_blk0 = soc_sbx_block_find(unit, SOC_BLK_PT, 0);
    int pt_blk1 = soc_sbx_block_find(unit, SOC_BLK_PT, 1);

    int result;
    uint32  c3_64bit_pc = SOC_SBX_CFG_CALADAN3(unit)->c3_64bit_pc;

    if (c3_64bit_pc) {
        cli_out("%-32s \t%25s\t%26s\n", "PR_IDP_STATS_GLOBAL", "PR0", "PR1");
    } else {
        cli_out("%-30s \t%17s\t%17s\n", "PR_IDP_STATS_GLOBAL", "PR0", "PR1");
    }
    for (i=0; i<sizeof(pr_sq)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", pr_sq[i].name);

	    soc_reg32_get(unit, pr_sq[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, pr_sq[i].reg, 1, 0, &n2);
	    COMPILER_64_ADD_32(pr_sq[i].val0, n1);
	    COMPILER_64_ADD_32(pr_sq[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, pr_sq[i].val0, 0);
	    format_uint64_decimal(buf_val2, pr_sq[i].val1, 0);
	    
	    COMPILER_64_ZERO(pr_sq[i].val0);
	    COMPILER_64_ZERO(pr_sq[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	} else {
	    soc_reg32_get(unit, pr_sq[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, pr_sq[i].reg, 1, 0, &n2);
	    if (!reg_skip_zero || n1 || n2) {
	        cli_out("%30s:\t%17u\t%17u\n", pr_sq[i].name, n1, n2);
	    }
	}
    }


    if (c3_64bit_pc) {
        cli_out("\n%-32s \t%25s\t%26s\n", "PR_IPRE_ADM_CTRL", "PR0", "PR1");
    } else {
        cli_out("\n%-30s \t%17s\t%17s\n", "PR_IPRE_ADM_CTRL", "PR0", "PR1");
    }
    for (i=0; i<sizeof(ipre_adm_ctrl)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", ipre_adm_ctrl[i].name);
	      
	    soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 1, 0, &n2);
	    
	    COMPILER_64_ADD_32(ipre_adm_ctrl[i].val0, n1);
	    COMPILER_64_ADD_32(ipre_adm_ctrl[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, ipre_adm_ctrl[i].val0, 0);
	    format_uint64_decimal(buf_val2, ipre_adm_ctrl[i].val1, 0);
	    
	    COMPILER_64_ZERO(ipre_adm_ctrl[i].val0);
	    COMPILER_64_ZERO(ipre_adm_ctrl[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	    
	} else {
	    soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 1, 0, &n2);
	    if (!reg_skip_zero || n1 || n2) {
	        cli_out("%30s:\t%17u\t%17u\n", ipre_adm_ctrl[i].name, n1, n2);
	    }
	}
    }
      
    if (c3_64bit_pc) {
        cli_out("\n%-32s \t%25s\t%26s\n", "IPRE stats", "PR0", "PR1");
    } else {
        cli_out("\n%-30s \t%17s\t%17s\n", "IPRE stats", "PR0", "PR1");
    }
    for (i=0; i<sizeof(ipre_stat)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", ipre_stat[i].name);
	    
	    soc_reg32_get(unit, ipre_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipre_stat[i].reg, 1, 0, &n2);
	    COMPILER_64_ADD_32(ipre_stat[i].val0, n1);
	    COMPILER_64_ADD_32(ipre_stat[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, ipre_stat[i].val0, 0);
	    format_uint64_decimal(buf_val2, ipre_stat[i].val1, 0);
	    
	    COMPILER_64_ZERO(ipre_stat[i].val0);
	    COMPILER_64_ZERO(ipre_stat[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	
	} else {
	    soc_reg32_get(unit, ipre_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipre_stat[i].reg, 1, 0, &n2);
	    if (!reg_skip_zero || n1 || n2) {
	        cli_out("%30s:\t%17u\t%17u\n", ipre_stat[i].name, n1, n2);
	    }
	}
    }

    for (i=0; i<64; i++) {
	if (c3_64bit_pc) {
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 1, i, &n2);
	    
	    COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT0[i],n1);
	    COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT1[i],n2);
	    format_uint64_decimal(buf_val1, PR_IDP_STATS_ACCEPT_PKT_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, PR_IDP_STATS_ACCEPT_PKT_COUNT1[i], 0);
	    if (!COMPILER_64_IS_ZERO( PR_IDP_STATS_ACCEPT_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO( PR_IDP_STATS_ACCEPT_PKT_COUNT1[i])){
	        COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_COUNT0[i]);
		COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_COUNT1[i]);
		
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "accepted pkts", i);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }
	    
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 1, i, &n2);
	    COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i],n1);
	    COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i],n2);
	    
	    format_uint64_decimal(buf_val1, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i], 0);

	    if(!COMPILER_64_IS_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i])){
	        COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i]);
		COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i]);
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "accepted bytes", i);	  
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }


	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 1, i, &n2);
	    COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT0[i],n1);
	    COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT1[i],n2);
	    
	    format_uint64_decimal(buf_val1, PR_IDP_STATS_DROP_PKT_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, PR_IDP_STATS_DROP_PKT_COUNT1[i], 0);
	    if (!COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_COUNT1[i])){
	        COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_COUNT0[i]);
		COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_COUNT1[i]);
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "dropped pkts", i);	  
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }


	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 1, i, &n2);
	    COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i],n1);
	    COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i],n2);
	    
	    format_uint64_decimal(buf_val1, PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i], 0);
	    
	    if(!COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i])){
	        COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i]);
		COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i]);
		
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "dropped bytes", i);	  
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }
	    
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, &n1);
	    soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, 0xffffffff);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, &n2);
	    soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, 0xffffffff);
	    if (n1 || n2) {
	        char buf[21];
		sprintf(buf, " %s%s%s%s",
			((n1>>3)&0x1)? "qm drop":"",
			((n1>>2)&0x1)? "policed":"",
			((n1>>1)&0x1)? "congestion ctrl":"",
			(n1&0x1)? "fifo overflow":"" );
		cli_out("%27s[%2d]:%33s", "drop reasons", i, buf);
		sprintf(buf, " %s%s%s%s",
			((n2>>3)&0x1)? "qm drop":"",
			((n2>>2)&0x1)? "policed":"",
			((n2>>1)&0x1)? "congestion ctrl":"",
			(n2&0x1)? "fifo overflow":"" );
		cli_out(" %32s\n", buf);
		
	    }
	}else {
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 1, i, &n2);
	    last1 = n1;
	    last2 = n2;
	    if (n1 || n2) {
	        cli_out("%25s [%2d]:\t%17u\t%17u\n", "accepted pkts", i, n1, n2);
	    }
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 1, i, &n2);
	    if (n1 || n2) {
	        cli_out("%25s [%2d]:\t%17u\t%17u\n", "accepted bytes", i, n1, n2);
	    }
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 1, i, &n2);
	    if (n1 || n2) {
		cli_out("%25s [%2d]:\t%17u", "dropped pkts", i , n1);
		if (n1 && last1) {
                    cli_out(" %2u%%", (unsigned int)((double)n1 * 100 / ((double) n1 + (double)last1)));
		}
		cli_out("\t%17u", n2);
		if (n2 && last2) {
                    cli_out(" %2u%%", (unsigned int)((double)n2 * 100 / ((double) n2 + (double)last2)));
		}
		cli_out("\n");

	    }
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 0, i, &n1);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 1, i, &n2);
	    if (n1 || n2) {
	        cli_out("%25s [%2d]:\t%17u\t%17u\n", "dropped bytes", i, n1, n2);
	    }
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, &n1);
	    soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, 0xffffffff);
	    soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, &n2);
	    soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, 0xffffffff);
	    if (n1 || n2) {
	      char buf[21];
	      sprintf(buf, " %s%s%s%s",
		      ((n1>>3)&0x1)? "qm drop":"",
		      ((n1>>2)&0x1)? "policed":"",
		      ((n1>>1)&0x1)? "congestion ctrl":"",
		      (n1&0x1)? "fifo overflow":"" );
	      cli_out("%25s [%2d]:%20s", "drop reasons", i, buf);
	      sprintf(buf, " %s%s%s%s",
		      ((n2>>3)&0x1)? "qm drop":"",
		      ((n2>>2)&0x1)? "policed":"",
		      ((n2>>1)&0x1)? "congestion ctrl":"",
		      (n2&0x1)? "fifo overflow":"" );
	      cli_out(" %20s\n", buf);
	    }
	}
    }
    

    if (c3_64bit_pc) {
        cli_out("\n%-32s \t%25s\t%26s\n", "PR_HDP_STATS", "PR0", "PR1");
    } else {
        cli_out("\n%-30s \t%17s\t%17s\n", "PR_HDP_STATS", "PR0", "PR1");
    }
    for (i=0; i<sizeof(pr_dq)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", pr_dq[i].name);
	    
	    soc_reg32_get(unit, pr_dq[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, pr_dq[i].reg, 1, 0, &n2);
	    
	    COMPILER_64_ADD_32(pr_dq[i].val0, n1);
	    COMPILER_64_ADD_32(pr_dq[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, pr_dq[i].val0, 0);
	    format_uint64_decimal(buf_val2, pr_dq[i].val1, 0);
	    
	    COMPILER_64_ZERO(pr_dq[i].val0);
	    COMPILER_64_ZERO(pr_dq[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
      } else {
	  soc_reg32_get(unit, pr_dq[i].reg, 0, 0, &n1);
	  soc_reg32_get(unit, pr_dq[i].reg, 1, 0, &n2);
	  cli_out("%30s:\t%17u\t%17u\n", pr_dq[i].name, n1, n2);
	}
    }

    if (c3_64bit_pc) {
        cli_out("\n%-32s \t%25s\t%26s\n", "PT_IPTE_DEBUG stats", "PT0", "PT1");
    } else {
        cli_out("\n%-30s \t%17s\t%17s\n", "PT_IPTE_DEBUG stats", "PT0", "PT1");
    }
    for (i=0; i<sizeof(pt_dq)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", pt_dq[i].name);

	    soc_reg32_get(unit, pt_dq[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, pt_dq[i].reg, 1, 0, &n2);
	    
	    COMPILER_64_ADD_32(pt_dq[i].val0, n1);
	    COMPILER_64_ADD_32(pt_dq[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, pt_dq[i].val0, 0);
	    format_uint64_decimal(buf_val2, pt_dq[i].val1, 0);
	    
	    COMPILER_64_ZERO(pt_dq[i].val0);
	    COMPILER_64_ZERO(pt_dq[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	} else {
	    soc_reg32_get(unit, pt_dq[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, pt_dq[i].reg, 1, 0, &n2);
	    if (!reg_skip_zero || n1 || n2) {
	        cli_out("%30s:\t%17u\t%17u\n", pt_dq[i].name, n1, n2);
	    }
	}
    }
    for (i=0; i<sizeof(ipte_stat)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", ipte_stat[i].name);

	    soc_reg32_get(unit, ipte_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipte_stat[i].reg, 1, 0, &n2);
	    
	    COMPILER_64_ADD_32(ipte_stat[i].val0, n1);
	    COMPILER_64_ADD_32(ipte_stat[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, ipte_stat[i].val0, 0);
	    format_uint64_decimal(buf_val2, ipte_stat[i].val1, 0);
	    
	    COMPILER_64_ZERO(ipte_stat[i].val0);
	    COMPILER_64_ZERO(ipte_stat[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	} else {
	    soc_reg32_get(unit, ipte_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, ipte_stat[i].reg, 1, 0, &n2);
	    if (!reg_skip_zero || n1 || n2) {
	        cli_out("%30s:\t%17u\t%17u\n", ipte_stat[i].name, n1, n2);
	    }
	}
    }

    for (i=0; i<64; i++) {
	if (c3_64bit_pc) {
	    result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk0, i, &meminfo0);
	    if(SOC_E_NONE != result) {
	      cli_out("fail to read IPTE_TX_STATS.pt0 [%d]\n", i);
	    }
	    result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk1, i, &meminfo1);
	    if(SOC_E_NONE != result) {
	      cli_out("fail to read IPTE_TX_STATS.pt1 [%d]\n", i);
	    }

	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, PKT_COUNTf, (uint32*)&n1);
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, PKT_COUNTf, (uint32*)&n2);
	    COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT0[i],n1);
	    COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT1[i],n2);

	    format_uint64_decimal(buf_val1, IPTE_TX_STATS_PKT_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, IPTE_TX_STATS_PKT_COUNT1[i], 0);
	    if (!COMPILER_64_IS_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO(IPTE_TX_STATS_PKT_COUNT1[i])){
	        COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
		COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
		
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "ipte_tx_stats pkt_count", i);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }

	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, BYTE_COUNTf, (uint32*)&n1);
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, BYTE_COUNTf, (uint32*)&n2);
	    
	    COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT0[i],n1);
	    COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT1[i],n2);

	    format_uint64_decimal(buf_val1, IPTE_TX_STATS_BYTE_COUNT0[i], 0);
	    format_uint64_decimal(buf_val2, IPTE_TX_STATS_BYTE_COUNT1[i], 0);
	    if (!COMPILER_64_IS_ZERO(IPTE_TX_STATS_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(IPTE_TX_STATS_BYTE_COUNT1[i])){
	        COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
		COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
		
                /* coverity[secure_coding] */
		sal_sprintf(line, "%27s[%2d]:", "ipte_tx_stats byte_count", i);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
                /* coverity[secure_coding] */
		sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
		tabify_line(tabby, line, tabwidth);
		cli_out("%s\n", tabby);
	    }

	    COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
	    COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
	    COMPILER_64_ZERO(IPTE_TX_STATS_BYTE_COUNT0[i]);
	    COMPILER_64_ZERO(IPTE_TX_STATS_BYTE_COUNT1[i]);
	}else {
	    result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk0, i, &meminfo0);
	    if(SOC_E_NONE != result) {
	      cli_out("fail to read IPTE_TX_STATS.pt0 [%d]\n", i);
	    }
	    result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk1, i, &meminfo1);
	    if(SOC_E_NONE != result) {
	      cli_out("fail to read IPTE_TX_STATS.pt1 [%d]\n", i);
	    }
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, PKT_COUNTf, (uint32*)&n1);
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, PKT_COUNTf, (uint32*)&n2);
	    if (n1 || n2) {
	        cli_out("%25s [%2d]:\t%17u\t%17u\n", "ipte_tx_stats pkt_count", i, n1, n2);
	    }
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, BYTE_COUNTf, (uint32*)&n1);
	    soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, BYTE_COUNTf, (uint32*)&n2);
	    if (n1 || n2) {
	        cli_out("%25s [%2d]:\t%17u\t%17u\n", "ipte_tx_stats pkt_count", i, n1, n2);
	    }
	}
    }
    
    if (c3_64bit_pc) {
	cli_out("\n%-32s \t%25s\t%26s\n", "PT_HPTE_DEBUG", "PT0", "PT1");
    } else {
        cli_out("\n%-30s \t%17s\t%17s\n", "PT_HPTE_DEBUG", "PT0", "PT1");
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ingress qm avail");        
        soc_reg32_get(unit, pt_sq[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(pt_sq[0].val0, n1);
        format_uint64_decimal(buf_val1, pt_sq[0].val0, 0);
        COMPILER_64_ZERO(pt_sq[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, pt_sq[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ingress qm avail", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "egress qm avail");        
        soc_reg32_get(unit, pt_sq[1].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(pt_sq[1].val0, n1);
        format_uint64_decimal(buf_val1, pt_sq[1].val0, 0);
        COMPILER_64_ZERO(pt_sq[1].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, pt_sq[1].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "egress qm avail", n1);
        }
    }

    for (i=0; i<sizeof(hpte_stat)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", hpte_stat[i].name);
	    
	    soc_reg32_get(unit, hpte_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, hpte_stat[i].reg, 1, 0, &n2);
	    
	    COMPILER_64_ADD_32(hpte_stat[i].val0, n1);
	    COMPILER_64_ADD_32(hpte_stat[i].val1, n2);
	    
	    format_uint64_decimal(buf_val1, hpte_stat[i].val0, 0);
	    format_uint64_decimal(buf_val2, hpte_stat[i].val1, 0);
	    
	    COMPILER_64_ZERO(hpte_stat[i].val0);
	    COMPILER_64_ZERO(hpte_stat[i].val1);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
      } else {
	    soc_reg32_get(unit, hpte_stat[i].reg, 0, 0, &n1);
	    soc_reg32_get(unit, hpte_stat[i].reg, 1, 0, &n2);
	    cli_out("%30s:\t%17u\t%17u\n", hpte_stat[i].name, n1, n2);
	}
    }

    cli_out("\nPPE counters\n");
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", ppe_sq_err[0].name);        
        soc_reg32_get(unit, ppe_sq_err[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq_err[0].val0, n1);
        format_uint64_decimal(buf_val1, ppe_sq_err[0].val0, 0);
        COMPILER_64_ZERO(ppe_sq_err[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ppe_sq_err[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", ppe_sq_err[0].name, n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ingress packets");        
        soc_reg32_get(unit, ppe_sq[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq[0].val0, n1);
        format_uint64_decimal(buf_val1, ppe_sq[0].val0, 0);
        COMPILER_64_ZERO(ppe_sq[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ppe_sq[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ingress packets", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "egress packets");        
        soc_reg32_get(unit, ppe_sq[1].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq[1].val0, n1);
        format_uint64_decimal(buf_val1, ppe_sq[1].val0, 0);
        COMPILER_64_ZERO(ppe_sq[1].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ppe_sq[1].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "egress packets", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "line side packets");        
        soc_reg32_get(unit, ppe_sq[2].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq[2].val0, n1);
        format_uint64_decimal(buf_val1, ppe_sq[2].val0, 0);
        COMPILER_64_ZERO(ppe_sq[2].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ppe_sq[2].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "line side packets", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "fabric side packets");        
        soc_reg32_get(unit, ppe_sq[3].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ppe_sq[3].val0, n1);
        format_uint64_decimal(buf_val1, ppe_sq[3].val0, 0);
        COMPILER_64_ZERO(ppe_sq[3].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ppe_sq[3].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "fabric side packets", n1);
        }
    }
    

    cli_out("\nQM\n");
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ped ingress accept");        
        soc_reg32_get(unit, ped_dq[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ped_dq[0].val0, n1);
        format_uint64_decimal(buf_val1, ped_dq[0].val0, 0);
        COMPILER_64_ZERO(ped_dq[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ped_dq[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ped ingress accept", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ped ingress drop");        
        soc_reg32_get(unit, ped_dq[2].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ped_dq[2].val0, n1);
        format_uint64_decimal(buf_val1, ped_dq[2].val0, 0);
        COMPILER_64_ZERO(ped_dq[2].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ped_dq[2].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ped ingress drop", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ped egress accept");        
        soc_reg32_get(unit, ped_dq[1].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ped_dq[1].val0, n1);
        format_uint64_decimal(buf_val1, ped_dq[1].val0, 0);
        COMPILER_64_ZERO(ped_dq[1].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ped_dq[1].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ped egress accept", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ped egress drop");        
        soc_reg32_get(unit, ped_dq[3].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(ped_dq[3].val0, n1);
        format_uint64_decimal(buf_val1, ped_dq[3].val0, 0);
        COMPILER_64_ZERO(ped_dq[3].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, ped_dq[3].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ped egress drop", n1);
        }
    }
    
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ingress sq acccept");        
        soc_reg32_get(unit, qm_sq[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_sq[0].val0, n1);
        format_uint64_decimal(buf_val1, qm_sq[0].val0, 0);
        COMPILER_64_ZERO(qm_sq[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_sq[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ingress sq acccept", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ingress sq drop");        
        soc_reg32_get(unit, qm_sq[2].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_sq[2].val0, n1);
        format_uint64_decimal(buf_val1, qm_sq[2].val0, 0);
        COMPILER_64_ZERO(qm_sq[2].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_sq[2].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ingress sq drop", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "egress sq acccept");        
        soc_reg32_get(unit, qm_sq[1].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_sq[1].val0, n1);
        format_uint64_decimal(buf_val1, qm_sq[1].val0, 0);
        COMPILER_64_ZERO(qm_sq[1].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_sq[1].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "egress sq acccept", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "egress sq drop");        
        soc_reg32_get(unit, qm_sq[3].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_sq[3].val0, n1);
        format_uint64_decimal(buf_val1, qm_sq[3].val0, 0);
        COMPILER_64_ZERO(qm_sq[3].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_sq[3].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "egress sq drop", n1);
        }
    }

    for (i=0; i<sizeof(qm_stat)/sizeof(c3_count_reg_t); i++) {
        if (c3_64bit_pc) {
            /* coverity[secure_coding] */
	    sal_sprintf(line, "%32s", qm_stat[i].name);

	    soc_reg32_get(unit, qm_stat[i].reg, 0, 0, &n1);
	    COMPILER_64_ADD_32(qm_stat[i].val0, n1);
	    
	    format_uint64_decimal(buf_val1, qm_stat[i].val0, 0);
	    COMPILER_64_ZERO(qm_stat[i].val0);
	    
            /* coverity[secure_coding] */
	    sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
	    tabify_line(tabby, line, tabwidth);
	    cli_out("%s\n", tabby);
	} else {
	    soc_reg32_get(unit, qm_stat[i].reg, 0, 0, &n1);
	    if (!reg_skip_zero || n1) {
	        cli_out("%30s:\t%17u\n", qm_stat[i].name, n1);
	    }
	}
    }

    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "aged drop");        
        soc_reg32_get(unit, qm_dq[6].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_dq[6].val0, n1);
        format_uint64_decimal(buf_val1, qm_dq[6].val0, 0);
        COMPILER_64_ZERO(qm_dq[6].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_dq[6].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "aged drop", n1);
        }
    }
    if (c3_64bit_pc) {
        sal_sprintf(line, "%32s", "ag_accept_count");        
        soc_reg32_get(unit, qm_dq[0].reg, 0, 0, &n1);
        COMPILER_64_ADD_32(qm_dq[0].val0, n1);
        format_uint64_decimal(buf_val1, qm_dq[0].val0, 0);
        COMPILER_64_ZERO(qm_dq[0].val0);
        sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
        tabify_line(tabby, line, tabwidth);
        cli_out("%s\n", tabby);
    } else {
        soc_reg32_get(unit, qm_dq[0].reg, 0, 0, &n1);
        if (!reg_skip_zero || n1) {
            cli_out("%30s:\t%17u\n", "ag_accept_count", n1);
        }
    }

}

char cmd_sbx_print_counts_ex_usage[] =
    "Usage: PrintCountseX [args]\n"
    "    more   - prints more detailed counts (only for Caladan3)\n"
    "    XXX    - prints specific module counts, like 'pc PR_SQ' (only for Caladan3)\n"
    "    XXX help - prints specific module counts help, like 'pc PR_SQ help' (only for Caladan3)\n";

cmd_result_t
cmd_sbx_print_counts_ex(int unit, args_t *a)
{
    int i;
    uint32 n1, n2;
    uint32 last1, last2;
    char        buf_val1[32], buf_val2[32], tabby[120], line[120];
    int         tabwidth = soc_property_get(unit, spn_DIAG_TABS, 8);
    ipte_tx_stats_entry_t meminfo0;
    ipte_tx_stats_entry_t meminfo1;
    char   *subcmd1 = NULL;
    char   *subcmd2 = NULL;
    int    pc_more = 0;

    int pt_blk0 = soc_sbx_block_find(unit, SOC_BLK_PT, 0);
    int pt_blk1 = soc_sbx_block_find(unit, SOC_BLK_PT, 1);

    int result;
    uint32  c3_64bit_pc = SOC_SBX_CFG_CALADAN3(unit)->c3_64bit_pc;

    
    subcmd1 = ARG_GET(a);
    subcmd2 = ARG_GET(a);
    
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "more") == 0) ) {
        pc_more = 1;
    }

    /* The following code section is for PR_SQ */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PR_SQ") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(pr_sq)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", pr_sq[i].name, pr_sq[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PR_SQ")==0)&&(subcmd2==NULL)) ){ 
        if (c3_64bit_pc) {
            cli_out("%-32s \t%25s\t%26s\n", "PR_SQ", "Ingress", "Egress");
        } else {
            cli_out("%-30s \t%17s\t%17s\n", "PR_SQ", "Ingress", "Egress");
        }
        for (i=0; i<sizeof(pr_sq)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", pr_sq[i].name);
    
            soc_reg32_get(unit, pr_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pr_sq[i].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(pr_sq[i].val0, n1);
            COMPILER_64_ADD_32(pr_sq[i].val1, n2);
            
            format_uint64_decimal(buf_val1, pr_sq[i].val0, 0);
            format_uint64_decimal(buf_val2, pr_sq[i].val1, 0);
            
            COMPILER_64_ZERO(pr_sq[i].val0);
            COMPILER_64_ZERO(pr_sq[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, pr_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pr_sq[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
                cli_out("%30s:\t%17u\t%17u\n", pr_sq[i].name, n1, n2);
            }
        }
        }
        
        {
            soc_reg32_get(unit, PR_IDP_ERROR_0r, 0, 0, &n1);
            soc_reg32_set(unit, PR_IDP_ERROR_0r, 0, 0, 0xffffffff);
            soc_reg32_get(unit, PR_IDP_ERROR_0r, 1, 0, &n2);
            soc_reg32_set(unit, PR_IDP_ERROR_0r, 1, 0, 0xffffffff);
            if (n1 || n2) {
                char buf[32];
            /* coverity[secure_coding] */
            sprintf(buf, " %s%s%s%s%s%s",
                ((n1>>5)&0x1)? "invalid_client_seq_err":"",
                ((n1>>4)&0x1)? "enq_req_eop_fifo_full_err":"",
                ((n1>>3)&0x1)? "free_cache_full_err":"",
                ((n1>>2)&0x1)? "s1_page_req_full_err":"",
                ((n1>>1)&0x1)? "enq_req_to_resp_fifo_full_err":"",
                (n1&0x1)?      "enq_resp_tag_seq_err":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s", "drop reasons", buf);
            }
            
            sprintf(buf, " %s%s%s%s%s%s",
                ((n2>>5)&0x1)? "invalid_client_seq_err":"",
                ((n2>>4)&0x1)? "enq_req_eop_fifo_full_err":"",
                ((n2>>3)&0x1)? "free_cache_full_err":"",
                ((n2>>2)&0x1)? "s1_page_req_full_err":"",
                ((n2>>1)&0x1)? "enq_req_to_resp_fifo_full_err":"",
                (n2&0x1)?      "enq_resp_tag_seq_err":"" );
            if (c3_64bit_pc){
                cli_out(" %33s\n", buf);
            } else {
                cli_out(" %20s\n", buf);
            }        
            }
        }
    
        {
            soc_reg32_get(unit, PR_IDP_ERROR_1r, 0, 0, &n1);
            soc_reg32_set(unit, PR_IDP_ERROR_1r, 0, 0, 0xffffffff);
            soc_reg32_get(unit, PR_IDP_ERROR_1r, 1, 0, &n2);
            soc_reg32_set(unit, PR_IDP_ERROR_1r, 1, 0, 0xffffffff);
            if (n1 || n2) {
                char buf[33];
            sprintf(buf, " %s%s%s%s%s%s%s%s",
                ((n1>>7)&0x1)? "enq_req_to_resp_fifo_uncorr":"",
                ((n1>>6)&0x1)? "enq_req_to_resp_fifo_corr":"",
                ((n1>>5)&0x1)? "enq_req_fifo_uncorrected":"",
                ((n1>>4)&0x1)? "enq_req_fifo_corrected":"",
                ((n1>>3)&0x1)? "free_cache_fifo_uncorrected":"",
                ((n1>>2)&0x1)? "free_cache_fifo_corrected":"",
                ((n1>>1)&0x1)? "page_req_fifo_uncorrected":"",
                (n1&0x1)?      "page_req_fifo_corrected":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s", "drop reasons", buf);
            }
            
            sprintf(buf, " %s%s%s%s%s%s%s%s",
                ((n2>>7)&0x1)? "enq_req_to_resp_fifo_uncor":"",
                ((n2>>6)&0x1)? "enq_req_to_resp_fifo_corr":"",
                ((n2>>5)&0x1)? "enq_req_fifo_uncorrected":"",
                ((n2>>4)&0x1)? "enq_req_fifo_corrected":"",
                ((n2>>3)&0x1)? "free_cache_fifo_uncorrected":"",
                ((n2>>2)&0x1)? "free_cache_fifo_corrected":"",
                ((n2>>1)&0x1)? "page_req_fifo_uncorrected":"",
                (n2&0x1)?      "page_req_fifo_corrected":"" );
    
            if (c3_64bit_pc){
                cli_out(" %33s\n", buf);
            } else {
                cli_out(" %20s\n", buf);
            } 
    
            
            }
        }
        
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PR_SQ")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    }    


    /* The following code section is for QM_SQ */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "QM_SQ") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(qm_sq)/sizeof(c3_count_reg_t); i=i+2){
                cli_out("%30s:\t%-s\n", qm_sq[i].name, qm_sq[i].description);
            }
            
            return CMD_OK;
        }
    }

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"QM_SQ")==0)&&(subcmd2==NULL)) ){    
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "QM_SQ");
        } else {
            cli_out("%-30s \n", "QM_SQ");
        }
        for (i=0; i<sizeof(qm_sq)/sizeof(c3_count_reg_t); i=i+2) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", qm_sq[i].name);
        
            soc_reg32_get(unit, qm_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, qm_sq[i+1].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(qm_sq[i].val0, n1);
            COMPILER_64_ADD_32(qm_sq[i+1].val1, n2);
            
            format_uint64_decimal(buf_val1, qm_sq[i].val0, 0);
            format_uint64_decimal(buf_val2, qm_sq[i+1].val1, 0);
            
            COMPILER_64_ZERO(qm_sq[i].val0);
            COMPILER_64_ZERO(qm_sq[i+1].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, qm_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, qm_sq[i+1].reg, 0, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
              cli_out("%30s:\t%17u\t%17u\n", qm_sq[i].name, n1, n2);
            
            }
        }
        }
    
        {
            soc_reg32_get(unit, QM_BUFFER_STATUS0r, 0, 0, &n1);
            soc_reg32_set(unit, QM_BUFFER_STATUS0r, 0, 0, 0xffffffff);
            if ((n1>>29)&0x7) {
                char buf[32];
            sprintf(buf, " %s%s%s",
                ((n1>>31)&0x1)? "total_buff_drop_hysteresis_max":"",
                ((n1>>30)&0x1)? "total_buff_drop_hysteresis_de2":"",
                ((n1>>29)&0x1)? "total_buff_drop_hysteresis_de1":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s\n", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s\n", "drop reasons", buf);
            }            
            }
        }
    
        {
            soc_reg32_get(unit, QM_BUFFER_STATUS2r, 0, 0, &n1);
            soc_reg32_set(unit, QM_BUFFER_STATUS2r, 0, 0, 0xffffffff);
            soc_reg32_get(unit, QM_BUFFER_STATUS3r, 0, 0, &n2);
            soc_reg32_set(unit, QM_BUFFER_STATUS3r, 0, 0, 0xffffffff);
    
            if (((n1>>29)&0x7)||((n2>>29)&0x7)) {
                char buf[32];
            sprintf(buf, " %s%s%s",
                ((n1>>31)&0x1)? "ingress_drop_hysteresis_max":"",
                ((n1>>30)&0x1)? "ingress_drop_hysteresis_de2":"",
                ((n1>>29)&0x1)? "ingress_drop_hysteresis_de1":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s", "drop reasons", buf);
            }            
    
            sprintf(buf, " %s%s%s",
                ((n2>>31)&0x1)? "egress_drop_hysteresis_max":"",
                ((n2>>30)&0x1)? "egress_drop_hysteresis_de2":"",
                ((n2>>29)&0x1)? "egress_drop_hysteresis_de1":"" );
            if (c3_64bit_pc){
                cli_out(" %33s\n", buf);
            } else {
                cli_out(" %20s\n", buf);
            }            
            }        
        }
    
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"QM_SQ")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 


        /* The following code section is for PT_SQ */
        if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PT_SQ") == 0) ) {
            if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
                for (i=0; i<sizeof(pt_sq)/sizeof(c3_count_reg_t); i=i+2) {
                    cli_out("%30s:\t%-s\n", pt_sq[i].name, pt_sq[i].description);
                }
                
                return CMD_OK;
            }
        } 
        
        if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PT_SQ")==0)&&(subcmd2==NULL)) ){     
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "PT_SQ");
        } else {
            cli_out("%-30s \n", "PT_SQ");
        }
        for (i=0; i<sizeof(pt_sq)/sizeof(c3_count_reg_t); i=i+2) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", pt_sq[i].name);
        
            soc_reg32_get(unit, pt_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pt_sq[i+1].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(pt_sq[i].val0, n1);
            COMPILER_64_ADD_32(pt_sq[i+1].val1, n2);
            
            format_uint64_decimal(buf_val1, pt_sq[i].val0, 0);
            format_uint64_decimal(buf_val2, pt_sq[i+1].val1, 0);
            
            COMPILER_64_ZERO(pt_sq[i].val0);
            COMPILER_64_ZERO(pt_sq[i+1].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, pt_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pt_sq[i+1].reg, 0, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
              cli_out("%30s:\t%17u\t%17u\n", pt_sq[i].name, n1, n2);
            }
        }
        }
    
    
        {
            soc_reg32_get(unit, PT_HPTE_DEBUG_ERROR0r, 0, 0, &n1);
            soc_reg32_set(unit, PT_HPTE_DEBUG_ERROR0r, 0, 0, 0xffffffff);
            soc_reg32_get(unit, PT_HPTE_DEBUG_ERROR0r, 1, 0, &n2);
            soc_reg32_set(unit, PT_HPTE_DEBUG_ERROR0r, 1, 0, 0xffffffff);
            if (n1 || n2) {
                char buf[35];
            sprintf(buf, " %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                ((n1>>13)&0x1)? "unexpected_pb_resp_err":"",
                ((n1>>12)&0x1)? "pp_read_resp_fifo_overflow_err":"",
                ((n1>>11)&0x1)? "pb_read_data_fifo_b_overflow_err":"",
                ((n1>>10)&0x1)? "pb_read_data_fifo_a_overflow_err":"",
                ((n1>>9)&0x1)?  "pb_read_resp_fifo_overflow_err":"",
                ((n1>>8)&0x1)?  "pb_read_req_fifo_overflow_err":"",
                ((n1>>7)&0x1)?  "deq_resp_if_qm_err":"",
                ((n1>>6)&0x1)?  "deq_resp_if_parity_err":"",
                ((n1>>5)&0x1)?  "avail_if_parity_err":"",
                ((n1>>4)&0x1)?  "eg_prefetch_fifo_overflow":"",
                ((n1>>3)&0x1)?  "ig_prefetch_fifo_overflow":"",
                ((n1>>2)&0x1)?  "pb_read_data_parity_err":"",
                ((n1>>1)&0x1)?  "ig_qm_prefetch_qid_err":"",
                (n1&0x1)?       "eg_qm_prefetch_qid_err":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s", "drop reasons", buf);
            }
            
            sprintf(buf, " %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                ((n2>>13)&0x1)? "unexpected_pb_resp_err":"",
                ((n2>>12)&0x1)? "pp_read_resp_fifo_overflow_err":"",
                ((n2>>11)&0x1)? "pb_read_data_fifo_b_overflow_err":"",
                ((n2>>10)&0x1)? "pb_read_data_fifo_a_overflow_err":"",
                ((n2>>9)&0x1)?  "pb_read_resp_fifo_overflow_err":"",
                ((n2>>8)&0x1)?  "pb_read_req_fifo_overflow_err":"",
                ((n2>>7)&0x1)?  "deq_resp_if_qm_err":"",
                ((n2>>6)&0x1)?  "deq_resp_if_parity_err":"",
                ((n2>>5)&0x1)?  "avail_if_parity_err":"",
                ((n2>>4)&0x1)?  "eg_prefetch_fifo_overflow":"",
                ((n2>>3)&0x1)?  "ig_prefetch_fifo_overflow":"",
                ((n2>>2)&0x1)?  "pb_read_data_parity_err":"",
                ((n2>>1)&0x1)?  "ig_qm_prefetch_qid_err":"",
                (n2&0x1)?       "eg_qm_prefetch_qid_err":"" );
            if (c3_64bit_pc){
                cli_out(" %33s\n", buf);
            } else {
                cli_out(" %20s\n", buf);
            }        
            }
        }
            if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PT_SQ")==0)&&(subcmd2==NULL)){
                return CMD_OK;
            }
        } 

    
    /* The following code section is for PPE */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PPE") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(ppe_sq)/sizeof(c3_count_reg_t); i=i+2) {
                cli_out("%30s:\t%-s\n", ppe_sq[i].name, ppe_sq[i].description);
            }
            for (i=0; i<sizeof(ppe_sq_err)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", ppe_sq_err[i].name, ppe_sq_err[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PPE")==0)&&(subcmd2==NULL)) ){    
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "PPE");
        } else {
            cli_out("%-30s \n", "PPE");
        }
        for (i=0; i<sizeof(ppe_sq)/sizeof(c3_count_reg_t); i=i+2) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ppe_sq[i].name);
        
            soc_reg32_get(unit, ppe_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ppe_sq[i+1].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(ppe_sq[i].val0, n1);
            COMPILER_64_ADD_32(ppe_sq[i+1].val1, n2);
            
            format_uint64_decimal(buf_val1, ppe_sq[i].val0, 0);
            format_uint64_decimal(buf_val2, ppe_sq[i+1].val1, 0);
            
            COMPILER_64_ZERO(ppe_sq[i].val0);
            COMPILER_64_ZERO(ppe_sq[i+1].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, ppe_sq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ppe_sq[i+1].reg, 0, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
              cli_out("%30s:\t%17u\t%17u\n", ppe_sq[i].name, n1, n2);
            }
        }
        }
    
        /* The register PP_ERR_PKT_COUNTERr needs to be processed alone */
        for (i=0; i<sizeof(ppe_sq_err)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ppe_sq_err[i].name);
        
            soc_reg32_get(unit, ppe_sq_err[i].reg, 0, 0, &n1);
            COMPILER_64_ADD_32(ppe_sq_err[i].val0, n1);
            
            format_uint64_decimal(buf_val1, ppe_sq_err[i].val0, 0);
            
            COMPILER_64_ZERO(ppe_sq_err[i].val0);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, ppe_sq_err[i].reg, 0, 0, &n1);
            if (!reg_skip_zero || n1 )  {
              cli_out("%30s:\t%17u\n", ppe_sq_err[i].name, n1);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PPE")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    }
    

    /* The following code section is for PED */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PED") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(ped_dq)/sizeof(c3_count_reg_t); i=i+2) {
                cli_out("%30s:\t%-s\n", ped_dq[i].name, ped_dq[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PED")==0)&&(subcmd2==NULL)) ){    
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "PED");
        } else {
            cli_out("%-30s \n", "PED");
        }
        for (i=0; i<sizeof(ped_dq)/sizeof(c3_count_reg_t); i=i+2) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ped_dq[i].name);
        
            soc_reg32_get(unit, ped_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ped_dq[i+1].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(ped_dq[i].val0, n1);
            COMPILER_64_ADD_32(ped_dq[i+1].val1, n2);
            
            format_uint64_decimal(buf_val1, ped_dq[i].val0, 0);
            format_uint64_decimal(buf_val2, ped_dq[i+1].val1, 0);
            
            COMPILER_64_ZERO(ped_dq[i].val0);
            COMPILER_64_ZERO(ped_dq[i+1].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, ped_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ped_dq[i+1].reg, 0, 0, &n2);
            if (!reg_skip_zero || n1 || n2) {
              cli_out("%30s:\t%17u\t%17u\n", ped_dq[i].name, n1, n2);
            }
        }
        }
            if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PED")==0)&&(subcmd2==NULL)){
                return CMD_OK;
            }
    }


    /* The following code section is for PR_DQ */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PR_DQ") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(pr_dq)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", pr_dq[i].name, pr_dq[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PR_DQ")==0)&&(subcmd2==NULL)) ){     
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "PR_DQ");
        } else {
            cli_out("%-30s \n", "PR_DQ");
        }
        for (i=0; i<sizeof(pr_dq)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", pr_dq[i].name);
        
            soc_reg32_get(unit, pr_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pr_dq[i].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(pr_dq[i].val0, n1);
            COMPILER_64_ADD_32(pr_dq[i].val1, n2);
            
            format_uint64_decimal(buf_val1, pr_dq[i].val0, 0);
            format_uint64_decimal(buf_val2, pr_dq[i].val1, 0);
            
            COMPILER_64_ZERO(pr_dq[i].val0);
            COMPILER_64_ZERO(pr_dq[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, pr_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pr_dq[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
                cli_out("%30s:\t%17u\t%17u\n", pr_dq[i].name, n1, n2);
            }
        }
        }
    
        {
            soc_reg32_get(unit, PR_HDP_HDR_DROP_STATUSr, 0, 0, &n1);
            soc_reg32_set(unit, PR_HDP_HDR_DROP_STATUSr, 0, 0, 0xffffffff);
            soc_reg32_get(unit, PR_HDP_HDR_DROP_STATUSr, 1, 0, &n2);
            soc_reg32_set(unit, PR_HDP_HDR_DROP_STATUSr, 1, 0, 0xffffffff);
            if (n1 || n2) {
                char buf[32];
            sprintf(buf, " %s%s%s%s%s%s%s%s%s",
                ((n1>>8)&0x1)? "ped_error_set":"",
                ((n1>>7)&0x1)? "hdr_error_set":"",
                ((n1>>6)&0x1)? "hdr_zero_xfer_length":"",
                ((n1>>5)&0x1)? "hdr_zero_hdr_length_set":"",
                ((n1>>4)&0x1)? "hdr_rt_rp_set":"",
                ((n1>>3)&0x1)? "hdr_rt_nzero_rc_set":"",
                ((n1>>2)&0x1)? "pb_almost_full_set":"",
                ((n1>>1)&0x1)? "hdr_drop_set":"",
                (n1&0x1)?      "pb_no_buffer_set":"" );
            if (c3_64bit_pc){
                cli_out("%30s:%33s", "drop reasons", buf);
            } else {
                cli_out("%28s:%20s", "drop reasons", buf);
            }
            
            sprintf(buf, " %s%s%s%s%s%s%s%s%s",
                ((n1>>8)&0x1)? "ped_error_set":"",
                ((n1>>7)&0x1)? "hdr_error_set":"",
                ((n1>>6)&0x1)? "hdr_zero_xfer_length":"",
                ((n1>>5)&0x1)? "hdr_zero_hdr_length_set":"",
                ((n1>>4)&0x1)? "hdr_rt_rp_set":"",
                ((n1>>3)&0x1)? "hdr_rt_nzero_rc_set":"",
                ((n1>>2)&0x1)? "pb_almost_full_set":"",
                ((n1>>1)&0x1)? "hdr_drop_set":"",
                (n1&0x1)?      "pb_no_buffer_set":"" );
            if (c3_64bit_pc){
                cli_out(" %33s\n", buf);
            } else {
                cli_out(" %20s\n", buf);
            }        
            }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PR_DQ")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    }    


    /* The following code section is for QM_DQ */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "QM_DQ") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(qm_dq)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", qm_dq[i].name, qm_dq[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"QM_DQ")==0)&&(subcmd2==NULL)) ){    
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "QM_DQ");
        } else {
            cli_out("%-30s \n", "QM_DQ");
        }
        for (i=0; i<sizeof(qm_dq)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", qm_dq[i].name);
        
            soc_reg32_get(unit, qm_dq[i].reg, 0, 0, &n1);
            COMPILER_64_ADD_32(qm_dq[i].val0, n1);
            
            format_uint64_decimal(buf_val1, qm_dq[i].val0, 0);
            
            COMPILER_64_ZERO(qm_dq[i].val0);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, qm_dq[i].reg, 0, 0, &n1);
            if (!reg_skip_zero || n1 )  {
                cli_out("%30s:\t%17u\n", qm_dq[i].name, n1);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"QM_DQ")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    }


    /* The following code section is for PT_DQ */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "PT_DQ") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(pt_dq)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", pt_dq[i].name, pt_dq[i].description);
            }
            
            return CMD_OK;
        }
    } 

    if ( (subcmd1==NULL) || (pc_more) || ((sal_strcasecmp(subcmd1,"PT_DQ")==0)&&(subcmd2==NULL)) ){     
        if (c3_64bit_pc) {
            cli_out("%-32s \n", "PT_DQ");
        } else {
            cli_out("%-30s \n", "PT_DQ");
        }
        for (i=0; i<sizeof(pt_dq)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", pt_dq[i].name);
        
            soc_reg32_get(unit, pt_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pt_dq[i].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(pt_dq[i].val0, n1);
            COMPILER_64_ADD_32(pt_dq[i].val1, n2);
            
            format_uint64_decimal(buf_val1, pt_dq[i].val0, 0);
            format_uint64_decimal(buf_val2, pt_dq[i].val1, 0);
            
            COMPILER_64_ZERO(pt_dq[i].val0);
            COMPILER_64_ZERO(pt_dq[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, pt_dq[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, pt_dq[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2)  {
                cli_out("%30s:\t%17u\t%17u\n", pt_dq[i].name, n1, n2);
            }
        }
        }
            if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"PT_DQ")==0)&&(subcmd2==NULL)){
                return CMD_OK;
            }
        }


    if (pc_more){
        cli_out("\n");
        cli_out("%-70s \n", "-------------------------------------------------------------------------");
    }


    /* The following code section is for ADM */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "ADM") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(ipre_adm_ctrl)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", ipre_adm_ctrl[i].name, ipre_adm_ctrl[i].description);
            }
            
            return CMD_OK;
        }
    } 
    
    if ( (pc_more) || ((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"ADM")==0)&&(subcmd2==NULL)) ){ 
        if (c3_64bit_pc) {
            cli_out("%-32s \t%25s\t%26s\n", "ADM", "Ingress", "Egress");
        } else {
            cli_out("%-30s \t%17s\t%17s\n", "ADM", "Ingress", "Egress");
        }
        for (i=0; i<sizeof(ipre_adm_ctrl)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ipre_adm_ctrl[i].name);
              
            soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 1, 0, &n2);
            
            COMPILER_64_ADD_32(ipre_adm_ctrl[i].val0, n1);
            COMPILER_64_ADD_32(ipre_adm_ctrl[i].val1, n2);
            
            format_uint64_decimal(buf_val1, ipre_adm_ctrl[i].val0, 0);
            format_uint64_decimal(buf_val2, ipre_adm_ctrl[i].val1, 0);
            
            COMPILER_64_ZERO(ipre_adm_ctrl[i].val0);
            COMPILER_64_ZERO(ipre_adm_ctrl[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            
        } else {
            soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipre_adm_ctrl[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2) {
                cli_out("%30s:\t%17u\t%17u\n", ipre_adm_ctrl[i].name, n1, n2);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"ADM")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 


    /* The following code section is for IPRE */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "IPRE") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(ipre_stat)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", ipre_stat[i].name, ipre_stat[i].description);
            }
            
            return CMD_OK;
        }
    } 
    
    if ( (pc_more) || ((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"IPRE")==0)&&(subcmd2==NULL)) ){ 
        if (c3_64bit_pc) {
            cli_out("\n%-32s \n", "IPRE");
        } else {
            cli_out("\n%-30s \n", "IPRE");
        }
        for (i=0; i<sizeof(ipre_stat)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ipre_stat[i].name);
            
            soc_reg32_get(unit, ipre_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipre_stat[i].reg, 1, 0, &n2);
            COMPILER_64_ADD_32(ipre_stat[i].val0, n1);
            COMPILER_64_ADD_32(ipre_stat[i].val1, n2);
            
            format_uint64_decimal(buf_val1, ipre_stat[i].val0, 0);
            format_uint64_decimal(buf_val2, ipre_stat[i].val1, 0);
            
            COMPILER_64_ZERO(ipre_stat[i].val0);
            COMPILER_64_ZERO(ipre_stat[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        
        } else {
            soc_reg32_get(unit, ipre_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipre_stat[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2) {
                cli_out("%30s:\t%17u\t%17u\n", ipre_stat[i].name, n1, n2);
            }
        }
        }
        
        for (i=0; i<64; i++) {
        if (c3_64bit_pc) {
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 1, i, &n2);
            
            COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT0[i],n1);
            COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_COUNT1[i],n2);
            format_uint64_decimal(buf_val1, PR_IDP_STATS_ACCEPT_PKT_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, PR_IDP_STATS_ACCEPT_PKT_COUNT1[i], 0);
            if (!COMPILER_64_IS_ZERO( PR_IDP_STATS_ACCEPT_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO( PR_IDP_STATS_ACCEPT_PKT_COUNT1[i])){
                COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_COUNT0[i]);
            COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_COUNT1[i]);
            
            sal_sprintf(line, "%27s[%2d]:", "accepted pkts", i);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
            
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 1, i, &n2);
            COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i],n1);
            COMPILER_64_ADD_32(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i],n2);
            
            format_uint64_decimal(buf_val1, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i], 0);
        
            if(!COMPILER_64_IS_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i])){
                COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT0[i]);
            COMPILER_64_ZERO(PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNT1[i]);
            sal_sprintf(line, "%27s[%2d]:", "accepted bytes", i);     
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
        
        
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 1, i, &n2);
            COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT0[i],n1);
            COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_COUNT1[i],n2);
            
            format_uint64_decimal(buf_val1, PR_IDP_STATS_DROP_PKT_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, PR_IDP_STATS_DROP_PKT_COUNT1[i], 0);
            if (!COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_COUNT1[i])){
                COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_COUNT0[i]);
            COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_COUNT1[i]);
            sal_sprintf(line, "%27s[%2d]:", "dropped pkts", i);   
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
        
        
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 1, i, &n2);
            COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i],n1);
            COMPILER_64_ADD_32(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i],n2);
            
            format_uint64_decimal(buf_val1, PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i], 0);
            
            if(!COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i])){
                COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT0[i]);
            COMPILER_64_ZERO(PR_IDP_STATS_DROP_PKT_BYTE_COUNT1[i]);
            
            sal_sprintf(line, "%27s[%2d]:", "dropped bytes", i);      
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
            
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, &n1);
            soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, 0xffffffff);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, &n2);
            soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, 0xffffffff);
            if (n1 || n2) {
                char buf[21];
            sprintf(buf, " %s%s%s%s",
                ((n1>>3)&0x1)? "qm drop":"",
                ((n1>>2)&0x1)? "policed":"",
                ((n1>>1)&0x1)? "congestion ctrl":"",
                (n1&0x1)? "fifo overflow":"" );
            cli_out("%27s[%2d]:%33s", "drop reasons", i, buf);
            sprintf(buf, " %s%s%s%s",
                ((n2>>3)&0x1)? "qm drop":"",
                ((n2>>2)&0x1)? "policed":"",
                ((n2>>1)&0x1)? "congestion ctrl":"",
                (n2&0x1)? "fifo overflow":"" );
            cli_out(" %32s\n", buf);
            
            }
        }else {
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_COUNTr, 1, i, &n2);
            last1 = n1;
            last2 = n2;
            if (n1 || n2) {
                cli_out("%25s [%2d]:\t%17u\t%17u\n", "accepted pkts", i, n1, n2);
            }
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_ACCEPT_PKT_BYTE_COUNTr, 1, i, &n2);
            if (n1 || n2) {
                cli_out("%25s [%2d]:\t%17u\t%17u\n", "accepted bytes", i, n1, n2);
            }
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_COUNTr, 1, i, &n2);
            if (n1 || n2) {
            cli_out("%25s [%2d]:\t%17u", "dropped pkts", i , n1);
            if (n1 && last1) {
                        cli_out(" %2u%%", (unsigned int)((double)n1 * 100 / ((double) n1 + (double)last1)));
            }
            cli_out("\t%17u", n2);
            if (n2 && last2) {
                        cli_out(" %2u%%", (unsigned int)((double)n2 * 100 / ((double) n2 + (double)last2)));
            }
            cli_out("\n");
        
            }
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 0, i, &n1);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_BYTE_COUNTr, 1, i, &n2);
            if (n1 || n2) {
                cli_out("%25s [%2d]:\t%17u\t%17u\n", "dropped bytes", i, n1, n2);
            }
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, &n1);
            soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 0, i, 0xffffffff);
            soc_reg32_get(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, &n2);
            soc_reg32_set(unit, PR_IDP_STATS_DROP_PKT_REASONSr, 1, i, 0xffffffff);
            if (n1 || n2) {
              char buf[21];
              sprintf(buf, " %s%s%s%s",
                  ((n1>>3)&0x1)? "qm drop":"",
                  ((n1>>2)&0x1)? "policed":"",
                  ((n1>>1)&0x1)? "congestion ctrl":"",
                  (n1&0x1)? "fifo overflow":"" );
              cli_out("%25s [%2d]:%20s", "drop reasons", i, buf);
              sprintf(buf, " %s%s%s%s",
                  ((n2>>3)&0x1)? "qm drop":"",
                  ((n2>>2)&0x1)? "policed":"",
                  ((n2>>1)&0x1)? "congestion ctrl":"",
                  (n2&0x1)? "fifo overflow":"" );
              cli_out(" %20s\n", buf);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"IPRE")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 



    /* The following code section is for HPTE */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "HPTE") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(hpte_stat)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", hpte_stat[i].name, hpte_stat[i].description);
            }
            
            return CMD_OK;
        }
    } 
    
    if ( (pc_more) || ((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"HPTE")==0)&&(subcmd2==NULL)) ){ 
        if (c3_64bit_pc) {
            cli_out("\n%-32s \t%25s\t%26s\n", "HPTE", "Ingress", "Egress");
        } else {
            cli_out("\n%-30s \t%17s\t%17s\n", "HPTE", "Ingress", "Egress");
        }
        for (i=0; i<sizeof(hpte_stat)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", hpte_stat[i].name);
            
            soc_reg32_get(unit, hpte_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, hpte_stat[i].reg, 1, 0, &n2);
            
            COMPILER_64_ADD_32(hpte_stat[i].val0, n1);
            COMPILER_64_ADD_32(hpte_stat[i].val1, n2);
            
            format_uint64_decimal(buf_val1, hpte_stat[i].val0, 0);
            format_uint64_decimal(buf_val2, hpte_stat[i].val1, 0);
            
            COMPILER_64_ZERO(hpte_stat[i].val0);
            COMPILER_64_ZERO(hpte_stat[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
          } else {
            soc_reg32_get(unit, hpte_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, hpte_stat[i].reg, 1, 0, &n2);
            cli_out("%30s:\t%17u\t%17u\n", hpte_stat[i].name, n1, n2);
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"HPTE")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 

    /* The following code section is for QM */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "QM") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(qm_stat)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", qm_stat[i].name, qm_stat[i].description);
            }
            
            return CMD_OK;
        }
    } 
    
    if ( (pc_more) || ((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"QM")==0)&&(subcmd2==NULL)) ){     
        cli_out("\nQM\n");
        for (i=0; i<sizeof(qm_stat)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", qm_stat[i].name);
    
            soc_reg32_get(unit, qm_stat[i].reg, 0, 0, &n1);
            COMPILER_64_ADD_32(qm_stat[i].val0, n1);
            
            format_uint64_decimal(buf_val1, qm_stat[i].val0, 0);
            COMPILER_64_ZERO(qm_stat[i].val0);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, qm_stat[i].reg, 0, 0, &n1);
            if (!reg_skip_zero || n1) {
                cli_out("%30s:\t%17u\n", qm_stat[i].name, n1);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"QM")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 


    /* The following code section is for IPTE */
    if ( (subcmd1 != NULL) && (sal_strcasecmp(subcmd1, "IPTE") == 0) ) {
        if ( (subcmd2 != NULL) && (sal_strcasecmp(subcmd2, "help") == 0) ){
            for (i=0; i<sizeof(ipte_stat)/sizeof(c3_count_reg_t); i++) {
                cli_out("%30s:\t%-s\n", ipte_stat[i].name, ipte_stat[i].description);
            }
            
            return CMD_OK;
        }
    } 
    
    if ( (pc_more) || ((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"IPTE")==0)&&(subcmd2==NULL)) ){    
        if (c3_64bit_pc) {
            cli_out("\n%-32s \t%25s\t%26s\n", "IPTE", "Ingress", "Egress");
        } else {
            cli_out("\n%-30s \t%17s\t%17s\n", "IPTE", "Ingress", "Egress");
        }
        for (i=0; i<sizeof(ipte_stat)/sizeof(c3_count_reg_t); i++) {
            if (c3_64bit_pc) {
            sal_sprintf(line, "%32s", ipte_stat[i].name);
    
            soc_reg32_get(unit, ipte_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipte_stat[i].reg, 1, 0, &n2);
            
            COMPILER_64_ADD_32(ipte_stat[i].val0, n1);
            COMPILER_64_ADD_32(ipte_stat[i].val1, n2);
            
            format_uint64_decimal(buf_val1, ipte_stat[i].val0, 0);
            format_uint64_decimal(buf_val2, ipte_stat[i].val1, 0);
            
            COMPILER_64_ZERO(ipte_stat[i].val0);
            COMPILER_64_ZERO(ipte_stat[i].val1);
            
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
        } else {
            soc_reg32_get(unit, ipte_stat[i].reg, 0, 0, &n1);
            soc_reg32_get(unit, ipte_stat[i].reg, 1, 0, &n2);
            if (!reg_skip_zero || n1 || n2) {
                cli_out("%30s:\t%17u\t%17u\n", ipte_stat[i].name, n1, n2);
            }
        }
        }
    
        for (i=0; i<64; i++) {
        if (c3_64bit_pc) {
            result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk0, i, &meminfo0);
            if(SOC_E_NONE != result) {
              cli_out("fail to read IPTE_TX_STATS.pt0 [%d]\n", i);
            }
            result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk1, i, &meminfo1);
            if(SOC_E_NONE != result) {
              cli_out("fail to read IPTE_TX_STATS.pt1 [%d]\n", i);
            }
    
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, PKT_COUNTf, (uint32*)&n1);
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, PKT_COUNTf, (uint32*)&n2);
            COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT0[i],n1);
            COMPILER_64_ADD_32(IPTE_TX_STATS_PKT_COUNT1[i],n2);
    
            format_uint64_decimal(buf_val1, IPTE_TX_STATS_PKT_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, IPTE_TX_STATS_PKT_COUNT1[i], 0);
            if (!COMPILER_64_IS_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]) || !COMPILER_64_IS_ZERO(IPTE_TX_STATS_PKT_COUNT1[i])){
                COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
            COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
            
            sal_sprintf(line, "%27s[%2d]:", "ipte_tx_stats pkt_count", i);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
    
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, BYTE_COUNTf, (uint32*)&n1);
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, BYTE_COUNTf, (uint32*)&n2);
            
            COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT0[i],n1);
            COMPILER_64_ADD_32(IPTE_TX_STATS_BYTE_COUNT1[i],n2);
    
            format_uint64_decimal(buf_val1, IPTE_TX_STATS_BYTE_COUNT0[i], 0);
            format_uint64_decimal(buf_val2, IPTE_TX_STATS_BYTE_COUNT1[i], 0);
            if (!COMPILER_64_IS_ZERO(IPTE_TX_STATS_BYTE_COUNT0[i]) || !COMPILER_64_IS_ZERO(IPTE_TX_STATS_BYTE_COUNT1[i])){
                COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
            COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
            
            sal_sprintf(line, "%27s[%2d]:", "ipte_tx_stats byte_count", i);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val1);
            sal_sprintf(line+strlen(line), "\t%24s", buf_val2);
            tabify_line(tabby, line, tabwidth);
            cli_out("%s\n", tabby);
            }
    
            COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT0[i]);
            COMPILER_64_ZERO(IPTE_TX_STATS_PKT_COUNT1[i]);
            COMPILER_64_ZERO(IPTE_TX_STATS_BYTE_COUNT0[i]);
            COMPILER_64_ZERO(IPTE_TX_STATS_BYTE_COUNT1[i]);
        }else {
            result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk0, i, &meminfo0);
            if(SOC_E_NONE != result) {
              cli_out("fail to read IPTE_TX_STATS.pt0 [%d]\n", i);
            }
            result = soc_mem_read(unit, IPTE_TX_STATSm, pt_blk1, i, &meminfo1);
            if(SOC_E_NONE != result) {
              cli_out("fail to read IPTE_TX_STATS.pt1 [%d]\n", i);
            }
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, PKT_COUNTf, (uint32*)&n1);
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, PKT_COUNTf, (uint32*)&n2);
            if (n1 || n2) {
                cli_out("%25s [%2d]:\t%17u\t%17u\n", "ipte_tx_stats pkt_count", i, n1, n2);
            }
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo0, BYTE_COUNTf, (uint32*)&n1);
            soc_mem_field_get(unit, IPTE_TX_STATSm,  (uint32*)&meminfo1, BYTE_COUNTf, (uint32*)&n2);
            if (n1 || n2) {
                cli_out("%25s [%2d]:\t%17u\t%17u\n", "ipte_tx_stats pkt_count", i, n1, n2);
            }
        }
        }
        if((subcmd1!=NULL)&&(sal_strcasecmp(subcmd1,"IPTE")==0)&&(subcmd2==NULL)){
            return CMD_OK;
        }
    } 

    return CMD_OK;
}


#endif /* BCM_CALADAN3_SUPPORT */


char cmd_sbx_print_counts_usage[] =
    "Usage: PrintCounts [args]\n"
    "    hex    - prints in hex format\n"
    "    raw    - prints in raw format\n"
    "    clear  - clears any counts that are not normally clear-on-read\n";

cmd_result_t
cmd_sbx_print_counts(int unit, args_t *a)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;
    int rv;
#if defined(BCM_SIRIUS_SUPPORT)
    char *str1 = "cnt";
    char *str2 = "count";
#endif

#if defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_CALADAN3(unit)){
      sbx_c3_counters(unit);
      return CMD_OK;
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)){
        rv = cmd_sbx_cmic_print_info(unit,str1,1, a);
        a->a_arg = 1; /* Reset args */
        rv = cmd_sbx_cmic_print_info(unit,str2,1, a);
        return (rv);
    }
#endif

    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        cli_out("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    reginfo_l->count = 0;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        cli_out("ERROR: Register info unknown for unit %d \n", unit);
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (sbx_reg_info_list_get(chip_info, reginfo_l, "cnt", 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    rv = _cmd_sbx_print_reg_class(unit, a, chip_info, reginfo_l,
                                  REG_PRINT_RAW);

    sal_free(reginfo_l);
    return rv;
}

char cmd_sbx_print_info_usage[] =
    "Usage: PrintInfo searchstr [args]\n"
    "    hex     - prints in hex format\n"
    "    raw     - prints in raw format\n"
    "    all     - prints all registers\n"
    "    pattern - find only pattern in searchstr\n"
    "   ~pattern - ignore pattern in searchstr\n";

cmd_result_t
cmd_sbx_print_info(int unit, args_t *a)
{
    int rv = CMD_NOTIMPL;
    char *str = ARG_GET(a);

    if (str == 0)
        return CMD_FAIL;

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)){
        rv = cmd_sbx_cmic_print_info(unit,str, 0, a);
        return (rv);
    }
#endif
    return rv;
}

/* routines used only for diags */
cmd_result_t
sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 *v)
{
  return _sbx_read_reg(unit,reg_info,v);
}

cmd_result_t
sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 v)
{
  return _sbx_write_reg(unit,reg_info,v);
}

cmd_result_t
sbx_clear_all(int unit, uint8 bErrorsOnly)
{
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_list_t *reginfo_l = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  int rv = CMD_OK;
  uint32 i;
  uint32 regval = 0;
  int idx;
  pbmp_t pbmp;
  bcm_port_t port;

  if (!soc_attached(unit)) {
    cli_out("Error Unit %d not attached\n",unit);
    return FALSE;
  }

  reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
  if (!reginfo_l) {
    cli_out("ERROR: sbx_reg_list failed. Out of Memory \n");
    return CMD_FAIL;
  }
  reginfo_l->count = 0;

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    cli_out("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(reginfo_l);
    return CMD_FAIL;
  }

  if (sbx_reg_info_list_get(chip_info, reginfo_l, "error", 0) != CMD_OK) {
    sal_free(reginfo_l);
    return CMD_FAIL;
  }

  if (!bErrorsOnly) {
    if (sbx_reg_info_list_get(chip_info, reginfo_l, "cnt", 0) != CMD_OK) {
      sal_free(reginfo_l);
      return CMD_FAIL;
    }
  }

  for (i = 0; i < reginfo_l->count; i++) {
    idx = reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = _sbx_read_reg(unit, reg_info, &regval);
      if (rv != CMD_OK) {
        return rv;
      }
      if (regval != 0) {
          /* Clear W1TC fields */
          rv = _sbx_write_reg(unit, reg_info, regval);
          if (rv != CMD_OK) {
            return rv;
          }
          /* Clear other fields (e.g. counters) */
          rv = _sbx_write_reg(unit, reg_info, 0);
          if (rv != CMD_OK) {
            return rv;
          }
      }
    }
  }

  /* Clear software counter statistics */
  BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
  PBMP_ITER(pbmp, port) {
    bcm_stat_clear(unit, port);
  }

  sal_free(reginfo_l);
  return rv;

}


#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)

/* 
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */

static void
collect_comma_args(args_t *a, char *valstr, char *first)
{
    char           *s;

    sal_strcpy(valstr, first);

    while ((s = ARG_GET(a)) != 0) {
        strcat(valstr, ",");
        strcat(valstr, s);
    }
}

/* 
 * modify_reg_fields
 *
 *   Takes a soc_reg_t 'regno', pointer to current value 'val',
 *   and a string 'mod' containing a field replacement spec of the form
 *   "FIELD=value".   The string may contain more than one spec separated
 *   by commas.  Updates the field in the register value accordingly,
 *   leaving all other unmodified fields intact.  If the string CLEAR is
 *   present in the list, the current register value is zeroed.
 *   If mask is non-NULL, it receives a mask of all fields modified.
 *
 *   Examples with modreg:
 *        modreg fe_mac1 lback=1        (modifies only lback field)
 *        modreg config ip_cfg=2        (modifies only ip_cfg field)
 *        modreg config clear,ip_cfg=2,cpu_pri=4  (zeroes all other fields)
 *
 *   Note that if "clear" appears in the middle of the list, the
 *   values in the list before "clear" are ignored.
 *
 *   Returns -1 on failure, 0 on success.
 */

static int
modify_reg_fields(int unit, soc_reg_t regno,
                  uint64 *val, uint64 *mask /* out param */, char *mod)
{
    soc_field_info_t *fld;
    char           *fmod, *fval;
    char           *modstr;
    soc_reg_info_t *reg = &SOC_REG_INFO(unit, regno);
    uint64          fvalue;
    uint64          fldmask;
    uint64          tmask;
    char            *tokstr=NULL;

    if ((modstr = sal_alloc(ARGS_BUFFER, "modify_reg")) == NULL) {
        cli_out("modify_reg_fields: Out of memory\n");
        return CMD_FAIL;
    }

    strncpy(modstr, mod, ARGS_BUFFER);/* Don't destroy input string */
    modstr[ARGS_BUFFER - 1] = 0;
    mod = modstr;

    if (mask) {
        COMPILER_64_ZERO(*mask);
    }

    while ((fmod = sal_strtok_r(mod, ",", &tokstr)) != 0) {
        mod = NULL;                    /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval) {                    /* Point fval to arg, NULL if none */
            *fval++ = 0;               /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0) {
            cli_out("Null field name\n");
            sal_free(modstr);
            return -1;
        }
        if (!sal_strcasecmp(fmod, "clear")) {
            COMPILER_64_ZERO(*val);
            if (mask) {
                COMPILER_64_ALLONES(*mask);
            }
            continue;
        }
        for (fld = &reg->fields[0]; fld < &reg->fields[reg->nFields]; fld++) {
            if (!sal_strcasecmp(fmod, SOC_FIELD_NAME(unit, fld->field))) {
                break;
            }
        }
        if (fld == &reg->fields[reg->nFields]) {
            cli_out("No such field \"%s\" in register \"%s\".\n",
                    fmod, SOC_REG_NAME(unit, regno));
            sal_free(modstr);
            return -1;
        }
        if (!fval) {
            cli_out("Missing %d-bit value to assign to \"%s\" "
                    "field \"%s\".\n",
                    fld->len, SOC_REG_NAME(unit, regno), SOC_FIELD_NAME(unit, fld->field));
            sal_free(modstr);
            return -1;
        }
        fvalue = parse_uint64(fval);

        /* Check that field value fits in field */
        COMPILER_64_MASK_CREATE(tmask, fld->len, 0);
        COMPILER_64_NOT(tmask);
        COMPILER_64_AND(tmask, fvalue);

        if (!COMPILER_64_IS_ZERO(tmask)) {
            cli_out("Value \"%s\" too large for %d-bit field \"%s\".\n",
                    fval, fld->len, SOC_FIELD_NAME(unit, fld->field));
            sal_free(modstr);
            return -1;
        }

        if (reg->flags & SOC_REG_FLAG_64_BITS) {
            soc_reg64_field_set(unit, regno, val, fld->field, fvalue);
        } else {
            uint32          tmp;
            uint32 ftmp;
            COMPILER_64_TO_32_LO(tmp, *val);
            COMPILER_64_TO_32_LO(ftmp, fvalue);
            soc_reg_field_set(unit, regno, &tmp, fld->field, ftmp);
            COMPILER_64_SET(*val, 0, tmp);
            COMPILER_64_SET(fvalue, 0, ftmp);
        }

        COMPILER_64_MASK_CREATE(fldmask, fld->len, fld->bp);
        if (mask) {
            COMPILER_64_OR(*mask, fldmask);
        }
    }

    sal_free(modstr);
    return 0;
}

static void sbx_reg_convert_lower_case(int unit)
{
#if 0
  soc_reg_t       reg;
  soc_regaddrlist_t alist;
  int j;

  if (soc_regaddrlist_alloc(&alist) < 0) {
    cli_out("Could not allocate address list.  Memory error.\n");
    return;
  }
    
  for (reg = 0; reg < NUM_SOC_REG; reg++) {
    if (!SOC_REG_IS_VALID(unit, reg)) {
      continue;
    }
    sbx_str_tolower(SOC_REG_NAME(unit, reg));
    if (parse_symbolic_reference(unit, &alist, SOC_REG_NAME(unit, reg)) < 0) {
      cli_out("%s:%d\n", __FILE__, __LINE__);
      break;
    }

    for (j = 0; j < alist.count; j++) {
      soc_regaddrinfo_t *ainfo;
      soc_reg_info_t *reginfo;
      int f;

      ainfo = &alist.ainfo[j];
      reginfo = &SOC_REG_INFO(unit, ainfo->reg);
      for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
	sbx_str_tolower(SOC_FIELD_NAME(unit, fld->field));
      }
    }
  }
  soc_regaddrlist_free(&alist);
#endif
}

static void
sbx_cmic_reg_print_sbx_format(int unit, soc_regaddrinfo_t *ainfo, uint64 val64, uint32 flags)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, ainfo->reg);
    int             f;
    uint64          resval, resfld;
    char            buf[80];
    uint32          val = COMPILER_64_LO(val64);  /* we don't have 64-bit registers*/

    soc_reg_sprint_addr(unit, buf, ainfo);
    cli_out("%s: 0x%08x\n", buf, val);

    if (flags & REG_PRINT_RAW) {
        return;
    }

    COMPILER_64_ZERO(resval);
    COMPILER_64_ZERO(resfld);
    for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
	uint64 field_value;
        field_value = soc_reg64_field_get(unit, ainfo->reg, val64, fld->field);

        if (flags & REG_PRINT_CHG) {
            resfld = soc_reg64_field_get(unit, ainfo->reg, resval, fld->field);
            if (COMPILER_64_EQ(field_value, resfld)) {
                continue;
            }
        }
	cli_out("\t%30s:  0x%08x\n", SOC_FIELD_NAME(unit, fld->field), COMPILER_64_LO(field_value));
    }
    cli_out("\n");
}

#define PRINT_COUNT(str, len, wrap, prefix) \
    if ((wrap > 0) && (len > wrap)) { \
        cli_out(\
                "\n%s", prefix); \
        len = sal_strlen(prefix); \
    } \
    cli_out(\
            "%s", str); \
    len += strlen(str)


/* 
 * Print a SOC internal register with fields broken out.
 */
void
sbx_cmic_reg_print(int unit, soc_regaddrinfo_t *ainfo, uint64 val, uint32 flags,
          char *fld_sep, int wrap)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, ainfo->reg);
    int             f;
    uint64          val64, resval, resfld;
    char            buf[80];
    char            line_buf[256];
    int             linelen = 0;
    int             nprint;

    if (flags & REG_PRINT_HEX) {
        if (SOC_REG_IS_64(unit, ainfo->reg)) {
            cli_out("%08x%08x\n",
                    COMPILER_64_HI(val),
                    COMPILER_64_LO(val));
        } else {
            cli_out("%08x\n",
                    COMPILER_64_LO(val));
        }
        return;
    }

    if (flags & REG_PRINT_CHG) {
        SOC_REG_RST_VAL_GET(unit, ainfo->reg, resval);
        if (COMPILER_64_EQ(val, resval)) {      /* no changed fields */
            return;
        }
    } else {
        COMPILER_64_ZERO(resval);
    }

    if (reg_sbx_format) {
      sbx_cmic_reg_print_sbx_format(unit, ainfo, val, flags);
      return;
    }

    soc_reg_sprint_addr(unit, buf, ainfo);

    sal_sprintf(line_buf, "%s[0x%x]=", buf, ainfo->addr);
    PRINT_COUNT(line_buf, linelen, wrap, "   ");

    format_uint64(line_buf, val);
    PRINT_COUNT(line_buf, linelen, -1, "");

    if (flags & REG_PRINT_RAW) {
        cli_out("\n");
        return;
    }

    PRINT_COUNT(": <", linelen, wrap, "   ");

    nprint = 0;
    for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
        val64 = soc_reg64_field_get(unit, ainfo->reg, val, fld->field);
        if (flags & REG_PRINT_CHG) {
            resfld = soc_reg64_field_get(unit, ainfo->reg, resval, fld->field);
            if (COMPILER_64_EQ(val64, resfld)) {
                continue;
            }
        }

        if (nprint > 0) {
            /* coverity[secure_coding] */
            sal_sprintf(line_buf, "%s", fld_sep);
            PRINT_COUNT(line_buf, linelen, -1, "");
        }
        /* coverity[secure_coding] */
        sal_sprintf(line_buf, "%s=", SOC_FIELD_NAME(unit, fld->field));
        PRINT_COUNT(line_buf, linelen, wrap, "   ");
        format_uint64(line_buf, val64);
        PRINT_COUNT(line_buf, linelen, -1, "");
        nprint += 1;
    }

    cli_out(">\n");
}

/* 
 * Reads and displays all SOC registers specified by alist structure.
 */
int
sbx_cmic_reg_print_all(int unit, soc_regaddrlist_t *alist, uint32 flags)
{
    int             j;
    uint64          value;
    int             r, rv = 0;
    soc_regaddrinfo_t *ainfo;

    assert(alist);

    for (j = 0; j < alist->count; j++) {
        ainfo = &alist->ainfo[j];
        if ((r = soc_anyreg_read(unit, ainfo, &value)) < 0) {
            char            buf[80];
            soc_reg_sprint_addr(unit, buf, ainfo);
            cli_out("ERROR: read from register %s failed: %s\n",
                    buf, soc_errmsg(r));
            rv = -1;
        } else {
            sbx_cmic_reg_print(unit, ainfo, value, flags, ",", 62);
        }
    }

    return rv;
}

/*
 * Register Types - for getreg and dump commands
 */

static regtype_entry_t regtypes[] = {
 { "PCIC",      soc_pci_cfg_reg,"PCI Configuration space" },
 { "PCIM",      soc_cpureg,     "PCI Memory space (CMIC)" },
 { "SOC",       soc_schan_reg,  "SOC internal registers" },
 { "SCHAN",     soc_schan_reg,  "SOC internal registers" },
 { "PHY",       soc_phy_reg,    "PHY registers via MII (phyID<<8|phyADDR)" },
 { "MW",        soc_hostmem_w,  "Host Memory 32-bit" },
 { "MH",        soc_hostmem_h,  "Host Memory 16-bit" },
 { "MB",        soc_hostmem_b,  "Host Memory 8-bit" },
 { "MEM",       soc_hostmem_w,  "Host Memory 32-bit" }, /* Backward compat */
};

#define regtypes_count  COUNTOF(regtypes)

regtype_entry_t *sbx_cmic_regtype_lookup_name(char* str)
{
    int i;

    for (i = 0; i < regtypes_count; i++) {
        if (!sal_strcasecmp(str,regtypes[i].name)) {
            return &regtypes[i];
        }
    }

    return 0;
}

void sirius_regtype_print_all(void)
{
    int i;

    cli_out("Register types supported by setreg, getreg, and dump:\n");

    for (i = 0; i < regtypes_count; i++)
        cli_out("\t%-10s -%s\n", regtypes[i].name, regtypes[i].help);
}

/* 
 * Get a register by type.
 *
 * doprint:  Boolean.  If set, display data.
 */
int
sbx_cmic_reg_get_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 *outval, uint32 flags)
{
    int             rv = CMD_OK;
    int             r;
    uint16          phy_rd_data;
    soc_regaddrinfo_t ainfo;
    int             is64 = FALSE;

    switch (regtype) {
    case soc_pci_cfg_reg:
        if (regaddr & 3) {
            cli_out("ERROR: PCI config addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
        }
        break;

    case soc_cpureg:
        if (regaddr & 3) {
            cli_out("ERROR: PCI memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));
        }
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_get(unit, &ainfo, regaddr);

        if (ainfo.reg >= 0) {
            is64 = SOC_REG_IS_64(unit, ainfo.reg);
        }

        r = soc_anyreg_read(unit, &ainfo, outval);
        if (r < 0) {
            cli_out("ERROR: soc_reg32_read failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }

        break;

    case soc_hostmem_w:
        COMPILER_64_SET(*outval, 0, *((uint32 *)INT_TO_PTR(regaddr)));
        break;
    case soc_hostmem_h:
        COMPILER_64_SET(*outval, 0, *((uint16 *)INT_TO_PTR(regaddr)));
        break;
    case soc_hostmem_b:
        COMPILER_64_SET(*outval, 0, *((uint8 *)INT_TO_PTR(regaddr)));
        break;

    case soc_phy_reg:
        /* Leave for MII debug reads */
        if ((r = soc_miim_read(unit,
                               (uint8) (regaddr >> 8 & 0xff),   /* Phy ID */
                               (uint8) (regaddr & 0xff),        /* Phy addr */
                               &phy_rd_data)) < 0) {
            cli_out("ERROR: soc_miim_read failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, (uint32) phy_rd_data);
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }

    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
        if (flags & REG_PRINT_HEX) {
            if (is64) {
                cli_out("%08x%08x\n",
                        COMPILER_64_HI(*outval),
                        COMPILER_64_LO(*outval));
            } else {
                cli_out("%08x\n",
                        COMPILER_64_LO(*outval));
            }
        } else {
            char buf[80];

            format_uint64(buf, *outval);

            cli_out("%s[0x%x] = %s\n",
                    soc_regtypenames[regtype], regaddr, buf);
        }
    }

    return rv;
}


/* 
 * Get a register by type.
 *
 * doprint:  Boolean.  If set, display data.
 */
int
sbx_cmic_reg_get_extended_by_type(int unit, int port, int block, uint32 regaddr, 
				  soc_regtype_t regtype,
				  uint64 *outval, uint32 flags)
{
    int             rv = CMD_OK;
    int             r;
    uint16          phy_rd_data;
    soc_regaddrinfo_t ainfo;
    int		    is64 = FALSE;

    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        return sbx_cmic_reg_get_by_type(unit, regaddr, regtype, outval, flags);
    }

    switch (regtype) {
    case soc_pci_cfg_reg:
	if (regaddr & 3) {
	    cli_out("ERROR: PCI config addr must be multiple of 4\n");
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
	}
	break;

    case soc_cpureg:
	if (regaddr & 3) {
	    cli_out("ERROR: PCI memory addr must be multiple of 4\n");
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));
	}
	break;
#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        if (regaddr & 3) {
            cli_out("ERROR: MCS memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, soc_pci_mcs_read(unit, regaddr));
        }
        break;
#endif
    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_extended_get(unit, &ainfo,
                                     SOC_BLOCK_INFO(unit, block).cmic, 0,
                                     regaddr);

        if (ainfo.reg >= 0) {
	    is64 = SOC_REG_IS_64(unit, ainfo.reg);
	}
        /* Set port value to handle TR3 AXP regs ..& C3 registers */
         if (((port >= 0) && (ainfo.port < 0)) || (port & SOC_REG_ADDR_INSTANCE_MASK))  {
            ainfo.port = port;
        } 
	r = soc_anyreg_read(unit, &ainfo, outval);
	if (r < 0) {
	    cli_out("ERROR: soc_reg32_read failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}

	break;

    case soc_hostmem_w:
        COMPILER_64_SET(*outval, 0, *((uint32 *)INT_TO_PTR(regaddr)));
        break;
    case soc_hostmem_h:
        COMPILER_64_SET(*outval, 0, *((uint16 *)INT_TO_PTR(regaddr)));
        break;
    case soc_hostmem_b:
        COMPILER_64_SET(*outval, 0, *((uint8 *)INT_TO_PTR(regaddr)));
        break;

    case soc_phy_reg:
	/* Leave for MII debug reads */
	if ((r = soc_miim_read(unit,
			       (uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
			       (uint8) (regaddr & 0xff),	/* Phy addr */
			       &phy_rd_data)) < 0) {
	    cli_out("ERROR: soc_miim_read failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, (uint32) phy_rd_data);
	}
	break;

    default:
	assert(0);
	rv = CMD_FAIL;
	break;
    }

    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
	if (flags & REG_PRINT_HEX) {
	    if (is64) {
		cli_out("%08x%08x\n",
                        COMPILER_64_HI(*outval),
                        COMPILER_64_LO(*outval));
	    } else {
		cli_out("%08x\n",
                        COMPILER_64_LO(*outval));
	    }
	} else {
	    char buf[80];

	    format_uint64(buf, *outval);

	    cli_out("%s[0x%x] = %s\n",
                    soc_regtypenames[regtype], regaddr, buf);
	}
    }

    return rv;
}

/* 
 * Set a register by type.  For SOC registers, is64 is used to
 * indicate if this is a 64 bit register.  Otherwise, is64 is
 * ignored.
 *
 */
int
sbx_cmic_reg_set_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 regval)
{
    int             rv = CMD_OK, r;
    uint32          val32;
    soc_regaddrinfo_t ainfo;

    COMPILER_64_TO_32_LO(val32, regval);

    switch (regtype) {
    case soc_pci_cfg_reg:
        bde->pci_conf_write(unit, regaddr, val32);
        break;

    case soc_cpureg:
        soc_pci_write(unit, regaddr, val32);
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_get(unit, &ainfo, regaddr);

        r = soc_anyreg_write(unit, &ainfo, regval);
        if (r < 0) {
            cli_out("ERROR: write reg failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }

        break;

    case soc_hostmem_w:
        *((uint32 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_hostmem_h:
        *((uint16 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_hostmem_b:
        *((uint8 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_phy_reg:
        /* Leave for MII debug writes */
        if ((r = soc_miim_write(unit,
                                (uint8) (regaddr >> 8 & 0xff),  /* Phy ID */
                                (uint8) (regaddr & 0xff),       /* Phy addr */
                                (uint16) val32)) < 0) {
            cli_out("ERROR: write miim failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }

    return rv;
}


/* 
 * Set a register by type.  For SOC registers, is64 is used to
 * indicate if this is a 64 bit register.  Otherwise, is64 is
 * ignored.
 *
 */
int
sbx_cmic_reg_set_extended_by_type(int unit, int port, int block, uint32 regaddr, 
				  soc_regtype_t regtype, uint64 regval)
{
    int             rv = CMD_OK, r;
    uint32          val32;
    soc_regaddrinfo_t ainfo;
    
    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        return sbx_cmic_reg_set_by_type(unit, regaddr, regtype, regval);
    }
    COMPILER_64_TO_32_LO(val32, regval);

    switch (regtype) {
    case soc_pci_cfg_reg:
	bde->pci_conf_write(unit, regaddr, val32);
	break;

    case soc_cpureg:
	soc_pci_write(unit, regaddr, val32);
	break;

#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        soc_pci_mcs_write(unit, regaddr, val32);
        break;
#endif

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_extended_get(unit, &ainfo,
	                             SOC_BLOCK_INFO(unit, block).cmic, 0,
                                     regaddr);
        ainfo.port = port; 
	r = soc_anyreg_write(unit, &ainfo, regval);
	if (r < 0) {
	    cli_out("ERROR: write reg failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}

	break;

    case soc_hostmem_w:
        *((uint32 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_hostmem_h:
        *((uint16 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_hostmem_b:
        *((uint8 *)INT_TO_PTR(regaddr)) = val32;
        break;

    case soc_phy_reg:
	/* Leave for MII debug writes */
	if ((r = soc_miim_write(unit,
				(uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
				(uint8) (regaddr & 0xff),	/* Phy addr */
				(uint16) val32)) < 0) {
	    cli_out("ERROR: write miim failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}
	break;

    default:
	assert(0);
	rv = CMD_FAIL;
	break;
    }

    return rv;
}

int ignore_reg(int unit, char *name)
{
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
    char *regs[] = {
      "CMIC_MMU_COSLC_",  /* hangs */
    };
    int i;

    for (i=0; i<sizeof(regs)/sizeof(char *); i++) {
      if (strcaseindex(name, regs[i]))
	return 1;
    }
  }
#endif /*BCM_CALADAN3_SUPPORT*/

  return 0;
}

/* 
 * Gets a memory value or register from the SOC.
 * Syntax: getreg [<regtype>] <offset|symbol>
 */

cmd_result_t
cmd_sbx_cmic_reg_get(int unit, args_t *a)
{
    uint64          regval;
    uint32          regaddr = 0;
    int             rv = CMD_OK;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist;
    char           *name;
    uint32          flags = REG_PRINT_DO_PRINT;
    int             all = 0;

    if (0 == sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    /* 
     * If first arg is a register type, take it and use the next argument
     * as the name or address, otherwise default to register type "soc."
     */
    name = ARG_GET(a);

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "chg")) {
            flags |= REG_PRINT_CHG;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "-all")) {
            all = 1;
            name = ARG_GET(a);
        } else {
            break;
        }
    }

    if (name == NULL) {
        return CMD_USAGE;
    }

    if ((rt = sbx_cmic_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_cmic_regtype_lookup_name("schan");
    }
    if (0 == rt) {
        cli_out("Unknown register.\n");
        return (CMD_FAIL);
    }

    if (soc_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (isint(name)) {                 /* Numerical address given */
        regaddr = parse_integer(name);
        rv = sbx_cmic_reg_get_by_type(unit, regaddr, rt->type, &regval, flags);
    } else {
        if (*name == '$') {
            name++;
        }

        /* Symbolic name given, print all or some values ... */
        if (rt->type == soc_cpureg) {
            if (parse_cmic_regname(unit, name, &regaddr) < 0) {
                cli_out("ERROR: bad argument to GETREG PCIM: %s\n", name);
                rv = CMD_FAIL;
            } else {
                rv = sbx_cmic_reg_get_by_type(unit, regaddr, rt->type,
                                      &regval, flags);
            }
        } else if (strchr(name, '.')) {
	    if (parse_symbolic_reference(unit, &alist, name) < 0) {
	      cli_out("Syntax error parsing \"%s\"\n", name);
	      rv = CMD_FAIL;
	    } else if (sbx_cmic_reg_print_all(unit, &alist, flags) < 0) {
	      rv = CMD_FAIL;
	    }
	} else {
	  int i;
	  for (i = 0; i < NUM_SOC_REG; i++) {
	    if (!SOC_REG_IS_VALID(unit, i)) {
	      continue;
	    }
	    if (!all && strcaseindex(SOC_REG_NAME(unit, i), name) == 0)
	      continue;
	    
	    if (ignore_reg(unit, SOC_REG_NAME(unit, i)))
		continue;
	    
	    if (parse_symbolic_reference(unit, &alist, SOC_REG_NAME(unit, i)) < 0) {
	      cli_out("Syntax error parsing \"%s\"\n", SOC_REG_NAME(unit, i));
              if (!all) {
                  break;
              }
	    }

	    if (sbx_cmic_reg_print_all(unit, &alist, flags) < 0) {
                if (!all) {
                    rv = CMD_FAIL;
                    break;
                }
	    }
	  }
        }
    }
    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Auxilliary routine to handle setreg and modreg.
 *      mod should be 0 for setreg, which takes either a value or a
 *              list of <field>=<value>, and in the latter case, sets all
 *              non-specified fields to zero.
 *      mod should be 1 for modreg, which does a read-modify-write of
 *              the register and permits value to be specified by a list
 *              of <field>=<value>[,...] only.
 */

STATIC cmd_result_t
sbx_cmic_do_reg_set(int unit, args_t *a, int mod)
{
    uint64          regval;
    uint32          regaddr = 0;
    int             rv = CMD_OK, i;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist = {0, NULL};
    soc_regaddrinfo_t *ainfo;
    char           *name;
    char           *s, *valstr = NULL;

    COMPILER_64_ALLONES(regval);

    if (0 == sh_check_attached(ARG_CMD(a), unit)) {
        return  CMD_FAIL;
    }

    if ((name = ARG_GET(a)) == 0) {
        return  CMD_USAGE;
    }

    /* 
     * If first arg is an access type, take it and use the next argument
     * as the name, otherwise use default access type.
     * modreg command does not allow this and assumes soc.
     */

    if ((0 == mod) && (rt = sbx_cmic_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_cmic_regtype_lookup_name("schan");
        if (0 == rt) {
            return CMD_FAIL;
        }
    }

    /* 
     * Parse the value field.  If there are more than one, string them
     * together with commas separating them (to make field-based setreg
     * inputs more convenient).
     */

    if ((s = ARG_GET(a)) == 0) {
        cli_out("Syntax error: missing value\n");
        return  CMD_USAGE;
    }

    if ((valstr = sal_alloc(ARGS_BUFFER, "reg_set")) == NULL) {
        cli_out("sbx_cmic_do_reg_set: Out of memory\n");
        return CMD_FAIL;
    }

    collect_comma_args(a, valstr, s);

    if (mod && isint(valstr)) {
        sal_free(valstr);
        return CMD_USAGE;
    }


    if (soc_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        sal_free(valstr);
        return CMD_FAIL;
    }

    if (!mod && isint(name)) {
        /* Numerical address given */
        regaddr = parse_integer(name);
        regval = parse_uint64(valstr);
        rv = sbx_cmic_reg_set_by_type(unit, regaddr, rt->type, regval);
    } else {                           /* Symbolic name given, set all or some 
                                * values ... */
        if (*name == '$') {
            name++;
        }
        if (rt->type == soc_cpureg) {
            if (parse_cmic_regname(unit, name, &regaddr) < 0) {
                cli_out("ERROR: bad argument to SETREG PCIM: %s\n", name);
                sal_free(valstr);
                soc_regaddrlist_free(&alist);
                return CMD_FAIL;
            }
            regval = parse_uint64(valstr);
            rv = sbx_cmic_reg_set_by_type(unit, regaddr, rt->type, regval);
        } else if (parse_symbolic_reference(unit, &alist, name) < 0) {
            cli_out("Syntax error parsing \"%s\"\n", name);
            sal_free(valstr);
            soc_regaddrlist_free(&alist);
            return CMD_FAIL;
        } else {
            if (isint(valstr)) {       /* valstr is numeric */
                regval = parse_uint64(valstr);
            }

            for (i = 0; i < alist.count; i++) {
                ainfo = &alist.ainfo[i];

                /* alist now holds list of registers to change */
                if (!isint(valstr)) {  /* Must modify registers */
                    /* 
                     * valstr must be a field replacement spec.
                     * In modreg mode, read the current register value,
                     * and modify it.  In setreg mode,
                     * assume a starting value of zero and modify it.
                     */
                    if (mod) {
                        rv = sbx_cmic_reg_get_extended_by_type(unit, ainfo->port, 
							       ainfo->block, 
							       ainfo->addr,
							       SOC_REG_INFO(unit, ainfo->reg).regtype,
							       &regval, 0);
                    } else {
                        COMPILER_64_ZERO(regval);
                    }

                    if (rv == CMD_OK) {
                        if ((rv = modify_reg_fields(unit, ainfo->reg, &regval,
                                                    (uint64 *) 0,
                                                    valstr)) < 0) {
                            cli_out("Syntax error, aborted\n");
                        }
                    }
                }

                if (rv == CMD_OK) {
                    rv = sbx_cmic_reg_set_extended_by_type(unit, ainfo->port, ainfo->block, ainfo->addr,
                                          SOC_REG_INFO(unit, ainfo->reg).regtype, regval);
                }
                if (rv != CMD_OK) {
                    break;
                }
            }
        }
    }
    sal_free(valstr);
    soc_regaddrlist_free(&alist);
    return rv;
}





STATIC cmd_result_t
sbx_cmic_reg_verify(int unit, args_t *a)
{
    uint64          regval;
    uint32			setval, getval;
    uint32          regaddr = 0, regtype = 0;
    int             rv = CMD_OK;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist = {0, NULL};
    soc_regaddrinfo_t *ainfo;
    char           *name, *value;

    COMPILER_64_ALLONES(regval);
    if (0 == sh_check_attached(ARG_CMD(a), unit)) {
        return  CMD_FAIL;
    }

    /* 
     * If first arg is a register type, take it and use the next argument
     * as the name or address, otherwise default to register type "soc."
     */
    name = ARG_GET(a);
    if ((rt = sbx_cmic_regtype_lookup_name(name)) != 0) 
    {
        if ((name = ARG_GET(a)) == 0) 
        {
            return CMD_USAGE;
        }
    }
    else
    {
        rt = sbx_cmic_regtype_lookup_name("schan");
    }
    if (0 == rt) 
    {
        cli_out("Unknown register.\n");
        return (CMD_FAIL);
    }
    
    value = ARG_GET(a);
    if (value == NULL) {
        return CMD_USAGE;
    }
    setval = parse_integer(value);

    regtype = rt->type;

    if (soc_regaddrlist_alloc(&alist) < 0) 
    {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (isint(name)) 
    {
        /* Numerical address given */
        regaddr = parse_integer(name);
        
        /*rv = sbx_cmic_reg_get_by_type(unit, regaddr, rt->type, &regval, 0);*/
    } 
    else 
    {
        if (*name == '$') 
        {
            name++;
        }

        /* Symbolic name given, print all or some values ... */
        if (rt->type == soc_cpureg) 
        {
            if (parse_cmic_regname(unit, name, &regaddr) < 0) 
            {
                cli_out("ERROR: bad argument to GETREG PCIM: %s\n", name);
                rv = CMD_FAIL;
            }
            else
            {
                COMPILER_64_SET(regval, 0, (uint32)setval);
                rv = sbx_cmic_reg_set_by_type(unit, regaddr, regtype, regval);
                rv = sbx_cmic_reg_get_by_type(unit, regaddr, regtype, &regval, 0);
                COMPILER_64_TO_32_LO(getval, regval);
   
                cli_out("cpu regaddr=%#X, regtype=%d, setval=%d, getval=%d\n\r",
                        regaddr, regtype, setval, getval);
            }
        } 
        else if (parse_symbolic_reference(unit, &alist, name) < 0) 
        {
            cli_out("Syntax error parsing \"%s\"\n", name);
            rv = CMD_FAIL;
        } 
        else 
        {
            ainfo = &alist.ainfo[0];
            COMPILER_64_SET(regval, 0, (uint32)setval);
            /* set reg value */
            rv = sbx_cmic_reg_set_extended_by_type(unit, ainfo->port, 
                                    ainfo->block, 
                                    ainfo->addr,
                                    SOC_REG_INFO(unit, ainfo->reg).regtype, regval);
            
            /* get back */
            rv = sbx_cmic_reg_get_extended_by_type(unit, ainfo->port, 
                                   ainfo->block, 
                                   ainfo->addr,
                                   SOC_REG_INFO(unit, ainfo->reg).regtype,
                                   &regval, 0);
            COMPILER_64_TO_32_LO(getval, regval);
                
            cli_out("cmic port=%d, block=%d, regaddr=%#X, regtype=%d, setval=%d, getval=%d\n\r",
                    ainfo->port, ainfo->block, ainfo->addr, 
                    SOC_REG_INFO(unit, ainfo->reg).regtype, setval, getval);
        }
    }
    

    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Sets a memory value or register on the SOC.
 * Syntax 1: setreg [<regtype>] <offset|symbol> <value>
 * Syntax 2: setreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_sbx_cmic_reg_set(int unit, args_t *a)
{
    return sbx_cmic_do_reg_set(unit, a, 0);
}

/* 
 * Read/modify/write a memory value or register on the SOC.
 * Syntax: modreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_sbx_cmic_reg_mod(int unit, args_t * a)
{
    return sbx_cmic_do_reg_set(unit, a, 1);
}

char cmd_sirius_regcmp_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "Usage: regcmp [args...]\n"
#else
    "Parameters: [-q] [<LOOPS>] <REG> [<VALUE>] [==|!=] <VALUE>\n\t"
    "If the optional <VALUE> on the left is given, starts by writing\n\t"
    "<VALUE> to <REG>.   Then loops up to <LOOPS> times reading <REG>,\n\t"
    "comparing if it is equal (==) or not equal (!=) to the <VALUE> on\n\t"
    "the right, and stopping if the compare fails.  If -q is specified, no\n\t"
    "output is displayed.  <LOOPS> defaults to 1 and may be * for\n\t"
    "indefinite.  If the compare fails, the command result is 1.  If the\n\t"
    "loops expire (compares succeed), the result is 0.  The result may be\n\t"
    "tested in an IF statement.  Also, each <VALUE> can consist of\n\t"
    "<FIELD>=<VALUE>[,...] to compare individual fields.  Examples:\n\t"
    "    if \"regcmp -q 1 rpkt.fe0 == 0\" \"echo RPKT IS ZERO\"\n\t"
    "    if \"regcmp -q config.e0 == fil_en=1\" \"echo FILTERING ENABLED\"\n"
#endif
    ;

cmd_result_t
cmd_sbx_cmic_reg_cmp(int unit, args_t *a)
{
    soc_reg_t       reg;
    soc_regaddrlist_t alist;
    soc_regaddrinfo_t *ainfo;
    char           *name = NULL, *count_str;
    char           *read_str, *write_str, *op_str;
    uint64          read_val, read_mask, write_val, reg_val, tmp_val;
    int             equal, i, quiet, loop;
    int             are_equal;
    int rv = CMD_OK;

    if (!(count_str = ARG_GET(a))) {
        return (CMD_USAGE);
    }

    if (!strcmp(count_str, "-q")) {
        quiet = TRUE;
        if (!(count_str = ARG_GET(a))) {
            return (CMD_USAGE);
        }
    } else {
        quiet = FALSE;
    }

    if (!strcmp(count_str, "*")) {
        loop = -1;
    } else if (isint(count_str)) {
        if ((loop = parse_integer(count_str)) < 0) {
            cli_out("%s: Invalid loop count: %s\n", ARG_CMD(a), count_str);
            return (CMD_FAIL);
        }
    } else {
        name = count_str;
        loop = 1;
    }

    if (!name && !(name = ARG_GET(a))) {
        return (CMD_USAGE);
    }

    write_str = ARG_GET(a);
    op_str = ARG_GET(a);
    read_str = ARG_GET(a);

    /* Must have WRITE ==|!= READ or ==|!= READ */

    if (!read_str) {
        read_str = op_str;
        op_str = write_str;
        write_str = NULL;
    } else if (ARG_CNT(a)) {
        return (CMD_USAGE);
    }

    if (!read_str || !op_str) {
        return (CMD_USAGE);
    }

    if (!strcmp(op_str, "==")) {
        equal = TRUE;
    } else if (!strcmp(op_str, "!=")) {
        equal = FALSE;
    } else {
        return (CMD_USAGE);
    }

    if (*name == '$') {
        name++;
    }

    /* Symbolic name given, set all or some values ... */

    if (soc_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (parse_symbolic_reference(unit, &alist, name) < 0) {
        cli_out("%s: Syntax error parsing \"%s\"\n", ARG_CMD(a), name);
        soc_regaddrlist_free(&alist);
        return (CMD_FAIL);
    }

    ainfo = &alist.ainfo[0];
    reg = ainfo->reg;

    COMPILER_64_ALLONES(read_mask);

    if (isint(read_str)) {
        read_val = parse_uint64(read_str);
    } else {
        COMPILER_64_ZERO(read_val);
        if (modify_reg_fields(unit, ainfo->reg, &read_val,
                              &read_mask, read_str) < 0) {
            cli_out("%s: Syntax error: %s\n", ARG_CMD(a), read_str);
            soc_regaddrlist_free(&alist);
            return (CMD_USAGE);
        }
    }

    if (write_str) {
        if (isint(write_str)) {
            write_val = parse_uint64(write_str);
        } else {
            COMPILER_64_ZERO(write_val);
            if (modify_reg_fields(unit, ainfo->reg, &write_val,
                                  (uint64 *) 0, write_str) < 0) {
                cli_out("%s: Syntax error: %s\n", ARG_CMD(a), write_str);
                soc_regaddrlist_free(&alist);
                return (CMD_USAGE);
            }
        }
    }

    do {
        for (i = 0; i < alist.count; i++) {
            int             r;

            ainfo = &alist.ainfo[i];
            if (write_str) {
                if ((r = soc_anyreg_write(unit, ainfo, write_val)) < 0) {
                    cli_out("%s: ERROR: Write register %s.%d failed: %s\n",
                            ARG_CMD(a), SOC_REG_NAME(unit, reg), i, soc_errmsg(r));
                    soc_regaddrlist_free(&alist);
                    return (CMD_FAIL);
                }
            }

            if ((r = soc_anyreg_read(unit, ainfo, &reg_val)) < 0) {
                cli_out("%s: ERROR: Read register %s.%d failed: %s\n",
                        ARG_CMD(a), SOC_REG_NAME(unit, reg), i, soc_errmsg(r));
                soc_regaddrlist_free(&alist);
                return (CMD_FAIL);
            }

            tmp_val = read_val;
            COMPILER_64_AND(tmp_val, read_mask);
            COMPILER_64_XOR(tmp_val, reg_val);
            are_equal = COMPILER_64_IS_ZERO(tmp_val);
            if ((!are_equal && equal) || (are_equal && !equal)) {
                if (!quiet) {
                    char buf1[80], buf2[80];
                    cli_out("%s: %s.%d ", ARG_CMD(a), SOC_REG_NAME(unit, reg), i);
                    format_uint64(buf1, read_val);
                    format_uint64(buf2, reg_val);
                    cli_out("%s %s %s\n", buf1, equal ? "!=" : "==", buf2);
                }
                soc_regaddrlist_free(&alist);
                return (1);
            }
        }

        if (loop > 0) {
            loop--;
        }
    } while (loop);

    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Lists registers containing a specific pattern
 *
 * If use_reset is true, ignores val and uses reset default value.
 */

static void
do_reg_list(int unit, soc_regaddrinfo_t *ainfo, int use_reset, uint64 regval)
{
    soc_reg_t       reg = ainfo->reg;
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);
    soc_field_info_t *fld;
    int             f;
    uint32          flags;
    uint64          mask, fldval, rval, rmsk;
    char            buf[80];
    char            rval_str[20], rmsk_str[20], dval_str[20];
    int             i, copies, disabled;

    if (!SOC_REG_IS_VALID(unit, reg)) {
        cli_out("Register %s is not valid for chip %s\n",
                SOC_REG_NAME(unit, reg), SOC_UNIT_NAME(unit));
        return;
    }

    flags = reginfo->flags;

    COMPILER_64_ALLONES(mask);

    SOC_REG_RST_VAL_GET(unit, reg, rval);
    SOC_REG_RST_MSK_GET(unit, reg, rmsk);
    format_uint64(rval_str, rval);
    format_uint64(rmsk_str, rmsk);
    if (use_reset) {
        regval = rval;
        mask = rmsk;
    } else {
        format_uint64(dval_str, regval);
    }

    soc_reg_sprint_addr(unit, buf, ainfo);

    cli_out("Register: %s", buf);
#if !defined(SOC_NO_ALIAS)
    if (soc_reg_alias[reg] && *soc_reg_alias[reg]) {
        cli_out(" alias %s", soc_reg_alias[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    cli_out(" %s register", soc_regtypenames[reginfo->regtype]);
    cli_out(" address 0x%08x\n", ainfo->addr);

    cli_out("Flags:");
    if (flags & SOC_REG_FLAG_64_BITS) {
        cli_out(" 64-bits");
    } else {
        cli_out(" 32-bits");
    }
    if (flags & SOC_REG_FLAG_COUNTER) {
        cli_out(" counter");
    }
    if (flags & SOC_REG_FLAG_ARRAY) {
        cli_out(" array[%d-%d]", 0, reginfo->numels-1);
    }
    if (flags & SOC_REG_FLAG_NO_DGNL) {
        cli_out(" no-diagonals");
    }
    if (flags & SOC_REG_FLAG_RO) {
        cli_out(" read-only");
    }
    if (flags & SOC_REG_FLAG_WO) {
        cli_out(" write-only");
    }
    if (flags & SOC_REG_FLAG_ED_CNTR) {
        cli_out(" error/discard-counter");
    }
    if (flags & SOC_REG_FLAG_SPECIAL) {
        cli_out(" special");
    }
    if (flags & SOC_REG_FLAG_EMULATION) {
        cli_out(" emulation");
    }
    if (flags & SOC_REG_FLAG_VARIANT1) {
        cli_out(" variant1");
    }
    if (flags & SOC_REG_FLAG_VARIANT2) {
        cli_out(" variant2");
    }
    if (flags & SOC_REG_FLAG_VARIANT3) {
        cli_out(" variant3");
    }
    if (flags & SOC_REG_FLAG_VARIANT4) {
        cli_out(" variant4");
    }
    cli_out("\n");

    cli_out("Blocks:");
    copies = disabled = 0;
    for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
        /*if (SOC_BLOCK_INFO(unit, i).type & reginfo->block) {*/
        if (SOC_BLOCK_IS_TYPE(unit, i, reginfo->block)) {
            if (SOC_INFO(unit).block_valid[i]) {
                cli_out(" %s", SOC_BLOCK_NAME(unit, i));
            } else {
                cli_out(" [%s]", SOC_BLOCK_NAME(unit, i));
                disabled += 1;
            }
            copies += 1;
        }
    }
    cli_out(" (%d cop%s", copies, copies == 1 ? "y" : "ies");
    if (disabled) {
        cli_out(", %d disabled", disabled);
    }
    cli_out(")\n");

#if !defined(SOC_NO_DESC)
    if (soc_reg_desc[reg] && *soc_reg_desc[reg]) {
        cli_out("Description: %s\n", soc_reg_desc[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    cli_out("Displaying:");
    if (use_reset) {
        cli_out(" reset defaults");
    } else {
        cli_out(" value %s", dval_str);
    }
    cli_out(", reset value %s mask %s\n", rval_str, rmsk_str);

    for (f = reginfo->nFields - 1; f >= 0; f--) {
        fld = &reginfo->fields[f];
        cli_out("  %s<%d", SOC_FIELD_NAME(unit, fld->field),
                fld->bp + fld->len - 1);
        if (fld->len > 1) {
            cli_out(":%d", fld->bp);
        }
        fldval = soc_reg64_field_get(unit, reg, mask, fld->field);
        if (use_reset && COMPILER_64_IS_ZERO(fldval)) {
            cli_out("> = x");
        } else {
            fldval = soc_reg64_field_get(unit, reg, regval, fld->field);
            format_uint64(buf, fldval);
            cli_out("> = %s", buf);
        }
        if (fld->flags & (SOCF_RO|SOCF_WO)) {
            cli_out(" [");
            i = 0;
            if (fld->flags & SOCF_RO) {
	      /* coverity[dead_error_line] */
                cli_out("%sRO", i++ ? "," : "");
            }
            if (fld->flags & SOCF_WO) {
                cli_out("%sWO", i++ ? "," : "");
            }
            cli_out("]");
        }
        cli_out("\n");
    }
}

#define PFLAG_ALIAS     0x01
#define PFLAG_SUMMARY   0x02

static void
_print_regname(int unit, soc_reg_t reg, int *col, int pflags)
{
    int             len;
    soc_reg_info_t *reginfo;

    reginfo = &SOC_REG_INFO(unit, reg);
    len = strlen(SOC_REG_NAME(unit, reg)) + 1;

    if (pflags & PFLAG_SUMMARY) {
        char    tname, *dstr1, *dstr2, *bname;
        int     dlen, copies, i;
        char    nstr[128], bstr[64];

        switch (reginfo->regtype) {
        case soc_schan_reg:     tname = 's'; break;
        case soc_cpureg:        tname = 'c'; break;
        case soc_genreg:        tname = 'g'; break;
        case soc_portreg:       tname = 'p'; break;
        case soc_cosreg:        tname = 'o'; break;
        case soc_hostmem_w:
        case soc_hostmem_h:
        case soc_hostmem_b:     tname = 'm'; break;
        case soc_phy_reg:       tname = 'f'; break;
        case soc_pci_cfg_reg:   tname = 'P'; break;
        default:                tname = '?'; break;
        }
#if !defined(SOC_NO_DESC)
        dstr2 = strchr(soc_reg_desc[reg], '\n');
        if (dstr2 == NULL) {
            dlen = strlen(soc_reg_desc[reg]);
        } else {
            dlen = dstr2 - soc_reg_desc[reg];
        }
        if (dlen > 30) {
            dlen = 30;
            dstr2 = "...";
        } else {
            dstr2 = "";
        }
        dstr1 = soc_reg_desc[reg];
#else /* defined(SOC_NO_DESC) */
        dlen = 1;
        dstr1 = "";
        dstr2 = "";
#endif /* defined(SOC_NO_DESC) */
        if (reginfo->flags & SOC_REG_FLAG_ARRAY) {
            /* coverity[secure_coding] */
            sal_sprintf(nstr, "%s[%d]", SOC_REG_NAME(unit, reg), reginfo->numels);
        } else {
            /* coverity[secure_coding] */
            sal_sprintf(nstr, "%s", SOC_REG_NAME(unit, reg));
        }

        copies = 0;
        bname = NULL;
        for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
            /*if (SOC_BLOCK_INFO(unit, i).type & reginfo->block) {*/
            if (SOC_BLOCK_IS_TYPE(unit, i, reginfo->block)) {
                if (bname == NULL) {
                    bname = SOC_BLOCK_NAME(unit, i);
                }
                copies += 1;
            }
        }
        if (copies > 1) {
            /* coverity[secure_coding] */
            sal_sprintf(bstr, "%d/%s", copies, bname);
        } else if (copies == 1) {
            /* coverity[secure_coding] */
            sal_sprintf(bstr, "%s", bname);
        } else {
            /* coverity[secure_coding] */
            sal_sprintf(bstr, "none");
        }
        cli_out(" %c%c%c%c%c  %-26s %-8.8s  %*.*s%s\n",
                tname,
                (reginfo->flags & SOC_REG_FLAG_64_BITS) ? '6' : '3',
                (reginfo->flags & SOC_REG_FLAG_COUNTER) ? 'c' : '-',
                (reginfo->flags & SOC_REG_FLAG_ED_CNTR) ? 'e' : '-',
                (reginfo->flags & SOC_REG_FLAG_RO) ? 'r' :
                (reginfo->flags & SOC_REG_FLAG_WO) ? 'w' : '-',
                nstr,
                bstr,
                dlen, dlen, dstr1, dstr2);
        return;
    }
    if (*col < 0) {
        cli_out("  ");
        *col = 2;
    }
    if (*col + len > ((pflags & PFLAG_ALIAS) ? 65 : 72)) {
        cli_out("\n  ");
        *col = 2;
    }
    cli_out("%s%s ", SOC_REG_NAME(unit, reg), SOC_REG_ARRAY(unit, reg) ? "[]" : "");
#if !defined(SOC_NO_ALIAS)
    if ((pflags & PFLAG_ALIAS) && soc_reg_alias[reg]) {
        len += strlen(soc_reg_alias[reg]) + 8;
        cli_out("(aka %s) ", soc_reg_alias[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    *col += len;
}

static void
_list_regs_by_type(int unit, soc_block_t blk, int *col, int pflag)
{
    soc_reg_t       reg;

    *col = -1;
    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        /*if (SOC_REG_INFO(unit, reg).block & blk) {*/
        if (SOC_BLOCK_IN_LIST(SOC_REG_INFO(unit, reg).block, blk)) {
            _print_regname(unit, reg, col, pflag);
        }
    }
    cli_out("\n");
}

cmd_result_t
cmd_sbx_cmic_reg_list(int unit, args_t *a)
{
    char           *str;
    char           *val;
    uint64          value;
    soc_regaddrinfo_t ainfo;
    int             found;
    int             rv = CMD_OK;
    int             all_regs;
    soc_reg_t       reg;
    int             col;
    int             pflag;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    ainfo.reg = INVALIDr;
    pflag = 0;
    col = -1;

    /* Parse options */
    while (((str = ARG_GET(a)) != NULL) && (str[0] == '-')) {
        while (str[0] && str[0] == '-') {
            str += 1;
        }
        if (sal_strcasecmp(str, "alias") == 0 ||
            sal_strcasecmp(str, "a") == 0) {    /* list w/ alias */
            pflag |= PFLAG_ALIAS;
            continue;
        }
        if (sal_strcasecmp(str, "summary") == 0 ||
            sal_strcasecmp(str, "s") == 0) {    /* list w/ summary */
            pflag |= PFLAG_SUMMARY;
            continue;
        }
        if (sal_strcasecmp(str, "counters") == 0 ||
            sal_strcasecmp(str, "c") == 0) {    /* list counters */
            cli_out("unit %d counters\n", unit);
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg))
                    continue;
                if (!SOC_REG_IS_COUNTER(unit, reg))
                    continue;
                _print_regname(unit, reg, &col, pflag);
            }
            cli_out("\n\n");
            return CMD_OK;
        }
        if (sal_strcasecmp(str, "ed") == 0 ||
            sal_strcasecmp(str, "e") == 0) {    /* error/discard */
            cli_out("unit %d error/discard counters\n", unit);
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg)) {
                    continue;
                }
                if (!(SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_ED_CNTR)) {
                    continue;
                }
                _print_regname(unit, reg, &col, pflag);
            }
            cli_out("\n\n");
            return CMD_OK;
        }
        if (sal_strcasecmp(str, "type") == 0 ||
            sal_strcasecmp(str, "t") == 0) {    /* list by type */
            int         i;
            soc_info_t *si = &SOC_INFO(unit);

            for (i = 0; i < COUNTOF(si->has_block); i++) {
                if (!(si->has_block[i])) {
                    continue;
                }
                cli_out("unit %d %s registers\n",
                        unit,
                        soc_block_name_lookup_ext(si->has_block[i], unit));
                col = -1;
                _list_regs_by_type(unit, si->has_block[i], &col, pflag);
            }
            cli_out("\n");
            return CMD_OK;
        }
        cli_out("ERROR: unrecognized option: %s\n", str);
        return CMD_FAIL;
    }

    if (!str) {
        return CMD_USAGE;
    }

    if ((val = ARG_GET(a)) != NULL) {
        value = parse_uint64(val);
    } else {
        COMPILER_64_ZERO(value);
    }


    if (isint(str)) {
        /* 
         * Address given, look up SOC register.
         */
        char            buf[80];
        uint32          addr;
        addr = parse_integer(str);
        soc_regaddrinfo_get(unit, &ainfo, addr);
        if (!ainfo.valid || (int)ainfo.reg < 0) {
            cli_out("Unknown register address: 0x%x\n", addr);
            rv = CMD_FAIL;
        } else {
            soc_reg_sprint_addr(unit, buf, &ainfo);
            cli_out("Address %s\n", buf);
        }
    } else {
        soc_regaddrlist_t alist;

        if (soc_regaddrlist_alloc(&alist) < 0) {
            cli_out("Could not allocate address list.  Memory error.\n");
            return CMD_FAIL;
        }

        /* 
         * Symbolic name.
         * First check if the register is there as exact match.
         * If not, list all substring matches.
         */

        all_regs = 0;
        if (*str == '$') {
            str++;
        } else if (*str == '*') {
            str++;
            all_regs = 1;
        }

        if (all_regs || parse_symbolic_reference(unit, &alist, str) < 0) {
            found = 0;
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg)) {
                    continue;
                }
                if (strcaseindex(SOC_REG_NAME(unit, reg), str) != 0) {
                    if (!found && !all_regs) {
                        cli_out("Unknown register; possible matches are:\n");
                    }
                    _print_regname(unit, reg, &col, pflag);
                    found = 1;
                }
            }
            if (!found) {
                cli_out("No matching register found");
            }
            cli_out("\n");
            rv = CMD_FAIL;
        } else {
            ainfo = alist.ainfo[0];
        }

        soc_regaddrlist_free(&alist);
    }

    /* 
     * Now have ainfo -- if reg is no longer INVALIDr
     */

    if (ainfo.reg != INVALIDr) {
        if (val) {
            do_reg_list(unit, &ainfo, 0, value);
        } else {
            COMPILER_64_ZERO(value);
            do_reg_list(unit, &ainfo, 1, value);
        }
    }

    return rv;
}

/* 
 * Editreg allows modifying register fields.
 * Works on fully qualified SOC registers only.
 */

cmd_result_t
cmd_sbx_cmic_reg_edit(int unit, args_t *a)
{
    soc_reg_info_t *reginfo;
    soc_field_info_t *fld;
    soc_regaddrlist_t alist;
    soc_reg_t       reg;
    uint64          v64;
    uint32          val, dfl, fv;
    char            ans[64], dfl_str[64];
    char           *name = ARG_GET(a);
    int             r, rv = CMD_FAIL;
    int             i, f;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return rv;
    }

    if (!name) {
        return CMD_USAGE;
    }

    if (*name == '$') {
        name++;
    }

    if (soc_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (parse_symbolic_reference(unit, &alist, name) < 0) {
        cli_out("Syntax error parsing \"%s\"\n", name);
        soc_regaddrlist_free(&alist);
        return (rv);
    }

    reg = alist.ainfo[0].reg;
    reginfo = &SOC_REG_INFO(unit, reg);

    /* 
     * If more than one register was specified, read the first one
     * and write the edited value to all of them.
     */

    if (soc_anyreg_read(unit, &alist.ainfo[0], &v64) < 0) {
        cli_out("ERROR: read reg failed\n");
        soc_regaddrlist_free(&alist);
        return (rv);
    }

    COMPILER_64_TO_32_LO(val, v64);

    cli_out("Current value: 0x%x\n", val);

    for (f = 0; f < (int)reginfo->nFields; f++) {
        fld = &reginfo->fields[f];
        dfl = soc_reg_field_get(unit, reg, val, fld->field);
        /* coverity[secure_coding] */
        sal_sprintf(dfl_str, "0x%x", dfl);
        /* coverity[secure_coding] */
        sal_sprintf(ans,                       /* Also use ans[] for prompt */
                "  %s<%d", SOC_FIELD_NAME(unit, fld->field), fld->bp + fld->len - 1);
        if (fld->len > 1) {
            /* coverity[secure_coding] */
            sal_sprintf(ans + strlen(ans), ":%d", fld->bp);
        }
        strcat(ans, ">? ");
        if (sal_readline(ans, ans, sizeof(ans), dfl_str) == 0 || ans[0] == 0) {
            cli_out("Aborted\n");
        soc_regaddrlist_free(&alist);
        return (rv);
        }
        fv = parse_integer(ans);
        if (fv & ~((1 << (fld->len - 1) << 1) - 1)) {
            cli_out("Value too big for %d-bit field, try again.\n",
                    fld->len);
            f--;
        } else {
            soc_reg_field_set(unit, reg, &val, fld->field, fv);
        }
    }

    cli_out("Writing new value: 0x%x\n", val);

    for (i = 0; i < alist.count; i++) {
        COMPILER_64_SET(v64, 0, val);

        if ((r = soc_anyreg_write(unit, &alist.ainfo[i], v64)) < 0) {
            cli_out("ERROR: write reg 0x%x failed: %s\n",
                    alist.ainfo[i].addr, soc_errmsg(r));
        soc_regaddrlist_free(&alist);
        return (rv);
        }
    }

    rv = CMD_OK;

    soc_regaddrlist_free(&alist);
    return rv;
}


#define DUMP_PHY_COLS   4
#define DUMP_MW_COLS    4
#define DUMP_MH_COLS    8
#define DUMP_MB_COLS    16

/*
 * Dump all of the CMIC registers.
 */

static void
do_dump_pcim(int unit, uint32 off_start, uint32 count)
{
    uint32 off, val;

    if ((off_start & 3) != 0) {
        cli_out("dump_pcim ERROR: offset must be a multiple of 4\n");
        return;
    }

    for (off = off_start; count--; off += 4) {
        val = soc_pci_read(unit, off);
        cli_out("0x%04x %s: 0x%x\n", off, soc_pci_off2name(unit, off), val);
    }
}

/*
 * Dump all of the SOC register addresses, and if do_values is true,
 * read and dump their values along with the addresses.
 */

#define DREG_ADR_SEL_MASK     0xf       /* Low order 4 bits */
#define DREG_ADDR               0       /* Address only */
#define DREG_RVAL               1       /* Address and reset default value */
#define DREG_REGS               2       /* Address and real value */
#define DREG_DIFF               3       /* Addr & real value if != default */
#define DREG_CNTR               4       /* Address and real value if counter */

#define DREG_PORT_ALL -1
#define DREG_BLOCK_ALL -1

struct dreg_data {
    int unit;
    int dreg_select;
    int only_port;    /* Select which port/block.  -1 ==> all */
    int only_block;
};

static int
dreg(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct dreg_data *dd = data;
    uint32 value = 0;
    uint64 val64, resetVal;
    char name[80];
    int is64, is_default, rv;
    int no_match = FALSE;  /* If specific port/block requested, turns true */
    char rval_str[20];

    /* Filter (set no_match) on ports and blocks if selected. */
    if (dd->only_port != DREG_PORT_ALL) {
        /* Only print ports that match */
        if (ainfo->port != dd->only_port) {
            no_match = TRUE;
        }
    }

    if (dd->only_block != DREG_BLOCK_ALL) {
        /* Only print blocks that match */
        if (ainfo->block != dd->only_block) {
            no_match = TRUE;
        } else { /* Match found; undo no_match */
            no_match = FALSE;
        }
    }

    if (no_match) {
        return 0;
    }

    soc_reg_sprint_addr(unit, name, ainfo);

    if (dd->dreg_select == DREG_ADDR) {
        cli_out("0x%08x %s\n", ainfo->addr, name);
        return 0;
    }

    SOC_REG_RST_VAL_GET(unit, ainfo->reg, resetVal);
    format_uint64(rval_str, resetVal);

    if (dd->dreg_select == DREG_RVAL) {
        cli_out("0x%08x %s = 0x%s\n", ainfo->addr, name, rval_str);
        return 0;
    }

    if (SOC_REG_INFO(unit, ainfo->reg).flags & SOC_REG_FLAG_WO) {
        cli_out("0x%08x %s = Write Only\n", ainfo->addr, name);
        return 0;
    }

    if (SOC_REG_IS_SPECIAL(unit, ainfo->reg)) {
        cli_out("0x%08x %s = Requires special processing\n",
                ainfo->addr, name);
        return 0;
    }

    if (reg_mask_subset(unit, ainfo, NULL)) {
        /* Register does not exist on this port/cos */
        return 0;
    }

    is64 = SOC_REG_IS_64(unit, ainfo->reg);

    if (is64) {
	rv = soc_reg64_get(dd->unit, ainfo->reg, ainfo->port, ainfo->idx, &val64);
        is_default = COMPILER_64_EQ(val64, resetVal);
    } else {
        rv = soc_reg32_get(dd->unit, ainfo->reg, ainfo->port, ainfo->idx, &value);
        is_default = (value == COMPILER_64_LO(resetVal));
    }

    if (rv < 0) {
        cli_out("0x%08x %s = ERROR\n", ainfo->addr, name);
        return 0;
    }

    if (dd->dreg_select == DREG_DIFF && is_default) {
        return 0;
    }

    if ((dd->dreg_select == DREG_CNTR)  &&
        (!(SOC_REG_INFO(unit, ainfo->reg).flags & SOC_REG_FLAG_COUNTER))) {
        return 0;
    }

    if (is64) {
        cli_out("0x%08x %s = 0x%08x%08x\n",
                ainfo->addr, name,
                COMPILER_64_HI(val64), COMPILER_64_LO(val64));
    } else {
        cli_out("0x%08x %s = 0x%08x\n", ainfo->addr, name, value);
    }

    return 0;
}

static cmd_result_t
do_dump_soc(int unit, int dreg_select, int only_port, int only_block)
{
    struct dreg_data dd;

    dd.unit = unit;
    dd.dreg_select = dreg_select;
    dd.only_port = only_port;
    dd.only_block = only_block;

    (void) soc_reg_iterate(unit, dreg, &dd);

    return CMD_OK;
}

/*
 * Dump registers, tables, or an address space.
 */

static cmd_result_t
sbx_cmic_do_dump_registers(int unit, regtype_entry_t *rt, args_t *a)
{
    int         i;
    sal_vaddr_t vaddr;
    uint32      t1, t2;
    pbmp_t      pbmp;
    soc_port_t  port, dport;
    int         rv = CMD_OK;
    uint32      flags = DREG_REGS;
    int         dump_port = DREG_PORT_ALL;
    int         dump_block = DREG_BLOCK_ALL;
    char        *an_arg;
    char        *count;

    an_arg = ARG_GET(a);
    count = ARG_GET(a);

    /* PCI config space does not take an offset */
    switch (rt->type) {
    case soc_pci_cfg_reg:
        _pci_print_config(unit);
        break;
    case soc_cpureg:
        if (an_arg) {
            if (parse_cmic_regname(unit, an_arg, &t1) < 0) {
                cli_out("ERROR: unknown CMIC register name: %s\n", an_arg);
                rv = CMD_FAIL;
                goto done;
            }
            t2 = count ? parse_integer(count) : 1;
        } else {
            t1 = CMIC_OFFSET_MIN;
            t2 = (CMIC_OFFSET_MAX - CMIC_OFFSET_MIN) / 4 + 1;
        }
        do_dump_pcim(unit, t1, t2);
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:

        while (an_arg) {
            if (sal_strcasecmp(an_arg, "addr") == 0) {
                flags = DREG_ADDR;
            } else if (sal_strcasecmp(an_arg, "rval") == 0) {
                flags = DREG_RVAL;
            } else if (sal_strcasecmp(an_arg, "diff") == 0) {
                flags = DREG_DIFF;
            } else if (sal_strcasecmp(an_arg, "counter") == 0) {
                flags = DREG_CNTR;
            } else if (sal_strcasecmp(an_arg, "port") == 0) {
                an_arg = ARG_GET(a);
                dump_port = an_arg ? parse_integer(an_arg) : 0;
            } else if (sal_strcasecmp(an_arg, "block") == 0) {
                an_arg = ARG_GET(a);
                dump_block = an_arg ? parse_integer(an_arg) : 0;
            } else {
                cli_out("ERROR: unrecognized argument to DUMP SOC: %s\n",
                        an_arg);
                return CMD_FAIL;
            }
            if (count != NULL) {
                an_arg = count;
                count = NULL;
            } else {
                an_arg = ARG_GET(a);
            }
        }
        rv = do_dump_soc(unit, flags, dump_port, dump_block);
        break;

    case soc_phy_reg:
        if (an_arg) {
            if (parse_pbmp(unit, an_arg, &pbmp)) {
                cli_out("Error: Invalid port identifier: %s\n", an_arg);
                rv = CMD_FAIL;
                break;
            }
        } else {
            pbmp = PBMP_PORT_ALL(unit);
        }
        SOC_PBMP_AND(pbmp, PBMP_PORT_ALL(unit));
        DPORT_SOC_PBMP_ITER(unit, pbmp, dport, port) {
            uint8       phy_id = PORT_TO_PHY_ADDR(unit, port);
            uint16      phy_data, phy_reg;
            cli_out("\nPort %d (Phy ID %d)", port + 1, phy_id);
            for (phy_reg = PHY_MIN_REG; phy_reg <= PHY_MAX_REG; phy_reg++) {
                rv = soc_miim_read(unit, phy_id, phy_reg, &phy_data);
                if (rv < 0) {
                    cli_out("Error: Port %d: cmic_read_miim failed: %s\n",
                            port + 1, soc_errmsg(rv));
                    rv = CMD_FAIL;
                    goto done;
                }
                cli_out("%s\t0x%02x: 0x%04x",
                        ((phy_reg % DUMP_PHY_COLS) == 0) ? "\n" : "",
                        phy_reg, phy_data);
            }
            cli_out("\n");
        }
        break;

    case soc_hostmem_w:
	if (!an_arg) {
	    cli_out("Dumping memory requires address and optional count\n");
	    rv = CMD_FAIL;
	    goto done;
	}
        vaddr = parse_address(an_arg) & ~3;
	t2 = count ? parse_integer(count) : 1;
	for (i = 0; i < (int)t2; i++, vaddr += 4) {
            uint32 *memptr = INT_TO_PTR(vaddr);
	    if ((i % DUMP_MW_COLS) == 0) {
		cli_out("%p: ", (void *)memptr);
            }
	    cli_out("%08x%c", *memptr,
                    ((i + 1) % DUMP_MW_COLS) == 0 ? '\n' : ' ');
	}
	if (i % DUMP_MW_COLS) {
	    cli_out("\n");
        }
	break;
    case soc_hostmem_h:
	if (!an_arg) {
	    cli_out("Dumping memory requires address and optional count\n");
	    rv = CMD_FAIL;
	    goto done;
	}
        vaddr = parse_address(an_arg) & ~1;
	t2 = count ? parse_integer(count) : 1;
	for (i = 0; i < (int)t2; i++, vaddr += 2) {
            uint16 *memptr = INT_TO_PTR(vaddr);
	    if ((i % DUMP_MH_COLS) == 0) {
 		cli_out("%p: ", (void *)memptr);
            }
	    cli_out("%04x%c", *memptr,
                    ((i + 1) % DUMP_MH_COLS) == 0 ? '\n' : ' ');
	}
	if (i % DUMP_MH_COLS) {
	    cli_out("\n");
        }
	break;
    case soc_hostmem_b:
	if (!an_arg) {
	    cli_out("Dumping memory requires address and optional count\n");
	    rv = CMD_FAIL;
	    goto done;
	}
        vaddr = parse_address(an_arg);
        t2 = count ? parse_integer(count) : 1;
        for (i = 0; i < (int)t2; i++, vaddr += 1) {
            uint8 *memptr = INT_TO_PTR(vaddr);
            if ((i % DUMP_MB_COLS) == 0) {
		cli_out("%p: ", memptr);
            }
            cli_out("%02x%c", *memptr,
                    ((i + 1) % DUMP_MB_COLS) == 0 ? '\n' : ' ');
        }
	if (i % DUMP_MB_COLS) {
	    cli_out("\n");
        }
	break;
    default:
        cli_out("Dumping register type %s is not yet implemented.\n",
                rt->name);
        rv = CMD_FAIL;
        break;
    }

 done:
    return rv;
}

cmd_result_t
cmd_sbx_cmic_do_dump_table(int unit, soc_mem_t mem,
              int copyno, int index, int count, int flags)
{
    int                 k, i;
    uint32              entry[SOC_MAX_MEM_WORDS];
    char                lineprefix[256];
    int                 entry_dw;
    int                 rv = CMD_FAIL;
    uint8             bWredStateModFormat = FALSE;
    uint32              uRegVal = 0;
    int32               voq_threshold = 0;
    

    assert(copyno >= 0);

    entry_dw = soc_mem_entry_words(unit, mem);

    /* special case when dumping WRED_STATE in tme or hybrid mode sdk-29998 */
    if (sal_strcasecmp(SOC_MEM_NAME(unit, mem),"WRED_STATE") == 0 
	&& ((SOC_SBX_CFG(unit)->bTmeMode) || SOC_SBX_CFG(unit)->bHybridMode)) {
      bWredStateModFormat = TRUE;
    }

    for (k = index; k < index + count; k++) {
        {
            i = soc_mem_read(unit, mem, copyno, k, entry);
        }
        if (i < 0) {
            cli_out("Read ERROR: table %s.%s[%d]: %s\n",
                    SOC_MEM_UFNAME(unit, mem),
                    SOC_BLOCK_NAME(unit, copyno), k, soc_errmsg(i));
            goto done;
        }

        if (!(flags & DUMP_TABLE_ALL)) {
            int         validf;

            validf = -1;

            if (soc_mem_field_length(unit, mem, ACTIVEf) > 0) {
                validf = ACTIVEf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALIDf) > 0) {
                validf = VALIDf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID0f) > 0) {
                validf = VALID0f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    if (soc_mem_field_length(unit, mem, VALID1f) > 0) {
                        validf = VALID1f;
                        if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                            continue;
                        }
                    }
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_0f) > 0) {
                validf = VALID_0f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_1f) > 0) {
                validf = VALID_1f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_2f) > 0) {
                validf = VALID_2f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_3f) > 0) {
                validf = VALID_3f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }
        }

	if (bWredStateModFormat) {
	  /* the data was read from WRED_STATE, valid for q < 32k,
	   * but when printing use the wred_avg_queue_length format if 
	   * this is a local queue */
	  if (SOC_SBX_CFG(unit)->bHybridMode) {
	    SOC_IF_ERROR_RETURN(READ_QMA_CONFIG1r(unit, &uRegVal));
	    voq_threshold = soc_reg_field_get(unit, QMA_CONFIG1r, uRegVal, VOQ_THRESHOLDf);
	    if (k >= voq_threshold) {
	      mem = WRED_AVG_QUEUE_LENGTHm;
	    }
	  } else {
	    mem = WRED_AVG_QUEUE_LENGTHm;
	  }
	}

        if (flags & DUMP_TABLE_HEX) {
            for (i = 0; i < entry_dw; i++) {
                cli_out("%08x\n", entry[i]);
            }
        } else if (flags & DUMP_TABLE_CHANGED) {
            /* coverity[secure_coding] */
            sal_sprintf(lineprefix, "%s.%s[%d]: ",
                    SOC_MEM_UFNAME(unit, mem),
                    SOC_BLOCK_NAME(unit, copyno),
                    k);
            soc_mem_entry_dump_if_changed(unit, mem, entry, lineprefix);
        } else {
            cli_out("%s.%s[%d]: ",
                    SOC_MEM_UFNAME(unit, mem),
                    SOC_BLOCK_NAME(unit, copyno),
                    k);

            if (flags & DUMP_TABLE_RAW) {
                for (i = 0; i < entry_dw; i++) {
                    cli_out("0x%08x ", entry[i]);
                }
            } else {
                if (flags & DUMP_TABLE_VERTICAL) {
                    soc_mem_entry_dump_vertical(unit, mem, entry);
                } else {
                    soc_mem_entry_dump(unit, mem, entry);
                }
            }

            cli_out("\n");
        }
    }

    rv = CMD_OK;

 done:
    return rv;
}

int
sbx_cmic_mem_dump_iter_callback(int unit, soc_mem_t mem, void *data)
{
    uint32     index_min, count, copyno;
    int        rv = SOC_E_NONE;
    int        flags = PTR_TO_INT(data);

    if (!SOC_MEM_IS_VALID(unit, mem) ||
        (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_DEBUG)) {
        return rv;
    }

    index_min = soc_mem_index_min(unit, mem);
    count = soc_mem_index_count(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
        /*
         * Bypass dumping MMU memories.
         */
        if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU) {
            continue;
        }

        if ((cmd_sbx_cmic_do_dump_table(unit, mem, copyno, index_min, 
                           count, flags)) != CMD_OK) {
            rv = SOC_E_INTERNAL;
            break;
        }
    }

    return rv;
}

static cmd_result_t
sbx_cmic_do_dump_memories(int unit, args_t *a)
{
    char        *an_arg;
    int          flags = DUMP_TABLE_ALL, rv = CMD_OK;

    an_arg = ARG_GET(a);

    while (an_arg) {
        if (sal_strcasecmp(an_arg, "diff") == 0) {
            flags = DUMP_TABLE_CHANGED;
        } else {
            cli_out("ERROR: unrecognized argument to DUMP SOC: %s\n",
                    an_arg);
            return CMD_FAIL;
        }
        an_arg = ARG_GET(a);
    }

    if ((soc_mem_iterate(unit, 
                         sbx_cmic_mem_dump_iter_callback, INT_TO_PTR(flags))) < 0) {
        rv = CMD_FAIL;
    }

    return rv;
}


cmd_result_t
cmd_sbx_cmic_dump(int unit, args_t *a)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    regtype_entry_t *rt;
    soc_mem_t mem;
    char *arg1, *arg2, *arg3;
    volatile int flags = 0;
    int copyno;
    volatile int rv = CMD_FAIL;
    parse_table_t pt;
    volatile char *fname = "";
    int append = FALSE;
    volatile int console_was_on = 0, console_disabled = 0, pushed_ctrl_c = 0;
    jmp_buf     ctrl_c;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "File", PQ_STRING, 0, &fname, 0);
    parse_table_add(&pt, "Append", PQ_BOOL, 0, &append, FALSE);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        goto done;
    }

    if (parse_arg_eq(a, &pt) < 0) {
        rv = CMD_USAGE;
        goto done;
    }

    console_was_on = bslcons_is_enabled();

    if (fname[0] != 0) {
        /*
         * Catch control-C in case if using file output option.
         */

#ifndef NO_CTRL_C
        if (setjmp(ctrl_c)) {
            rv = CMD_INTR;
            goto done;
        }
#endif

        sh_push_ctrl_c(&ctrl_c);

        pushed_ctrl_c = TRUE;

        if (bslfile_is_enabled()) {
            cli_out("%s: Can't dump to file while logging is enabled\n",
                    ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        if (bslfile_open((char *)fname, append) < 0) {
            cli_out("%s: Could not start log file\n", ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        bslcons_enable(FALSE);

        console_disabled = 1;
    }

    arg1 = ARG_GET(a);

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
    if ((arg1 != NULL) && !sal_strcasecmp(arg1, "sw")) {
        return (cmd_sbx_sw_dump(unit, a));
    }
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

    for (;;) {
        if (arg1 != NULL && !sal_strcasecmp(arg1, "raw")) {
            flags |= DUMP_TABLE_RAW;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "hex")) {
            flags |= DUMP_TABLE_HEX;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "all")) {
            flags |= DUMP_TABLE_ALL;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "chg")) {
            flags |= DUMP_TABLE_CHANGED;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "vert")) {
            flags |= DUMP_TABLE_VERTICAL;
            arg1 = ARG_GET(a);
        } else {
            break;
        }
    }

    if (arg1 == NULL) {
        rv = CMD_USAGE;
        goto done;
    }

    /* See if dumping internal ARL/L2 shadow copy */

    if (!sal_strcasecmp(arg1, "sarl") || !sal_strcasecmp(arg1, "sa")) {
        if (soc->arlShadow == NULL) {
            cli_out("No software ARL shadow table\n");
            rv = CMD_OK;
            goto done;
        }

        sal_mutex_take(soc->arlShadowMutex, sal_mutex_FOREVER);

        sal_mutex_give(soc->arlShadowMutex);

        rv = CMD_OK;
        goto done;
    }

    /* See if dumping a DV */
    if (!sal_strcasecmp(arg1, "dv")) {
        arg2 = ARG_GET(a);
        if (!arg2 || !isint(arg2)) {
            rv = CMD_USAGE;
            goto done;
        }
        soc_dma_dump_dv(unit, " ", (void *)parse_address(arg2));
        rv = CMD_OK;
        goto done;
    }

    /* See if dumping a packet */
    if (!sal_strcasecmp(arg1, "pkt")) {
        arg2 = ARG_GET(a);
        if (!arg2 || !isint(arg2)) {
            rv = CMD_USAGE;
            goto done;
        }
#ifdef  BROADCOM_DEBUG
        arg3 = ARG_GET(a);
        arg3 = arg3 ? arg3 : "0000";
        bcm_pkt_dump(unit,
                     INT_TO_PTR(parse_integer(arg2)),
                     parse_integer(arg3));
        rv = CMD_OK;
#else
        cli_out("cannot dump pkt in non-BROADCOM_DEBUG compilation\n");
        rv = CMD_FAIL;
#endif  /* BROADCOM_DEBUG */
        goto done;
    }

    /* See if dumping a memory table */

    if (parse_memory_name(unit, &mem, arg1, &copyno, 0) >= 0) {
        int index, count;

        arg2 = ARG_GET(a);
        arg3 = ARG_GET(a);
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            cli_out("Error: Memory %s not valid for chip %s.\n",
                    SOC_MEM_UFNAME(unit, mem), SOC_UNIT_NAME(unit));
            goto done;
        }
        if (copyno == COPYNO_ALL) {
            copyno = SOC_MEM_BLOCK_ANY(unit, mem);
        }
        if (arg2) {
            index = parse_memory_index(unit, mem, arg2);
            count = (arg3 ? parse_integer(arg3) : 1);
        } else {
            index = soc_mem_index_min(unit, mem);
            if (soc_mem_is_sorted(unit, mem) &&
                !(flags & DUMP_TABLE_ALL)) {
                count = soc_mem_entries(unit, mem, copyno);
            } else {
                count = soc_mem_index_max(unit, mem) - index + 1;
            }
        }
        rv = cmd_sbx_cmic_do_dump_table(unit, mem, copyno, index, count, flags);
        goto done;
    }

    if (!sal_strcasecmp(arg1, "socmem")) {
        rv = sbx_cmic_do_dump_memories(unit, a);
        goto done;
    }

    /*
     * See if dumping a register type
     */
    if ((rt = sbx_cmic_regtype_lookup_name(arg1)) != NULL) {
        rv = sbx_cmic_do_dump_registers(unit, rt, a);
        goto done;
    }

    cli_out("Unknown option or memory to dump "
            "(use 'help dump' for more info)\n");

    rv = CMD_FAIL;

 done:

    if (fname[0] != 0) {
        bslfile_close();
    }

    if (console_disabled && console_was_on) {
        bslcons_enable(TRUE);
    }

    if (pushed_ctrl_c) {
        sh_pop_ctrl_c();
    }

    parse_arg_eq_done(&pt);
    return rv;
}


/*
 * Function:    cmd_sbx_cmic_print_info
 * Purpose:         List of Registers
 * Parameters:  unit - unit number of device
 *              str  - Search string to key in on such as "error" or "count"
 *              a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_cmic_print_info(int unit, char *str, int clrok,  args_t *a)
{
    int             clear = 0, ignore = 0;
    soc_regaddrinfo_t *ainfo;
    soc_regaddrlist_t alist;
    int             i = 0, rv = CMD_OK;
    char           *name = ARG_GET(a);
    soc_reg_t       reg;
    int             flags = DUMP_TABLE_CHANGED;
    uint64          val;

    COMPILER_64_ALLONES(val);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "clear")) {
            if (clrok) clear = 1;
#if defined(BCM_CALADAN3_SUPPORT)
	  if (SOC_IS_CALADAN3(unit)){
	    cli_out("pe clear is not supported on C3.\n");
	    clear = 0;
	  }
#endif
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "all")) {
            flags &= ~DUMP_TABLE_CHANGED;
            name = ARG_GET(a);
        } else {
            break;
        }
    }

    if ((name) && (*name == '~')) {
        name++;
        ignore = 1;
    }
   
    if (soc_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }
    
    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        if (strcaseindex(SOC_REG_NAME(unit, reg), str) != 0) {
            if (parse_symbolic_reference(unit, &alist, SOC_REG_NAME(unit, reg)) < 0) {
                rv = CMD_FAIL;
            } else {
                /*
                 * Causes the hardware to hang
                 */
                if (strcaseindex(SOC_REG_NAME(unit, reg), "mmu_coslc_count_data") != 0) {
                    continue;
                }

                if (SAL_BOOT_BCMSIM && SOC_REG_IS_64(unit, reg)) {
		    /* ignore all 64 bits counters */
                    continue;
                }
                if (sal_strcasecmp(str, "error") == 0) {
		    if (SOC_IS_SIRIUS(unit)) {
			/* sirius specific register to skip for print error */
			if ((strcaseindex(SOC_REG_NAME(unit, reg), "mask") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "force") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "error_status") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "error_inject") != 0)) {
			    continue;
			}
		    } else if (SOC_IS_CALADAN3(unit)) {
			/* caladan3 specific register to skip */
		        if ((strcaseindex(SOC_REG_NAME(unit, reg), "mask") != 0)) {
			    continue;
		        }
		    }
                } else if (sal_strcasecmp(str, "cnt") == 0) {
		    if (SOC_IS_SIRIUS(unit)) {
			/* sirius specific register to skip for print count */
			if ((strcaseindex(SOC_REG_NAME(unit, reg), "maxsize") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cntr") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cntl") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt6") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt7") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "ecc_status") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "sfi_rx_sot") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "xp_xbode_cell_req") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "maxsz") != 0)) {
			    continue;
			}
		    } else if (SOC_IS_CALADAN3(unit)) {
			/* caladan3 specific register to skip */
		    }

                    if (SAL_BOOT_BCMSIM && 
                        (strcaseindex(SOC_REG_NAME(unit, reg), "mac") != 0)) {
                        continue;
                    }
                } else if (sal_strcasecmp(str, "count") == 0) {
		    if (SOC_IS_SIRIUS(unit)) {
			/* sirius specific register to skip for print count */
			if ((strcaseindex(SOC_REG_NAME(unit, reg), "addr") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cmic") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "rb_debug_edc_line") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "ecc") != 0)) {
			    continue;
			}
		    } else if (SOC_IS_CALADAN3(unit)) {
			/* caladan3 specific register to skip */

		    }
                }

                if (name != NULL) {
                    if (ignore) {
                        if (strcaseindex(SOC_REG_NAME(unit, reg), name) != 0) {
                            continue;
                        }
                    } else {
                        if (strcaseindex(SOC_REG_NAME(unit, reg), name) == 0) {
                            continue;
                        }
                    }
                }

		if (ignore_reg(unit, SOC_REG_NAME(unit, reg))) 
		  continue;

                if (sbx_cmic_reg_print_all(unit, &alist, flags) < 0) {
                    rv = CMD_FAIL;
                }
                if (clear) {
		    if (SOC_IS_SIRIUS(unit)) {
			/* sirius specific register to skip for register clear */
			if ((strcaseindex(SOC_REG_NAME(unit, reg), "cmic_ser_fail") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "eb_aging_dft") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "fr_flow_ctl_global") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "fr_flow_ctl_unicast") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "mac_pfc") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "mac_rxllfcmsg") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt6") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt7") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_pkt_count") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_slam_dma_entry") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_table_dma_entry") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_sw_pio_ack_data_beat") != 0) ||
			    (strcaseindex(SOC_REG_NAME(unit, reg), "txllfcmsg") != 0)) {
			    continue;
			}
		    }

                    for (i = 0; i < alist.count; i++) {
                        ainfo = &alist.ainfo[i];
                        rv = sbx_cmic_reg_set_extended_by_type(unit, ainfo->port, ainfo->block, ainfo->addr,
							       SOC_REG_INFO(unit, ainfo->reg).regtype, 
							       val);
                    }
                }
            }
        }
    }
    
    soc_regaddrlist_free(&alist);
    
    return rv;
}

#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */
