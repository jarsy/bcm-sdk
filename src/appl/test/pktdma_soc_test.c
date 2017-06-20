/*
 * $Id: pktdma_soc_test.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * CMIC Packet DMA performance test.
 * This test is intended to test the CMIC packet DMA performance. It can also
 * also be used as a feature test. The test can simultaneously exercise
 * multiple TX and RX channels across multiple CMCs.
 *
 */

#include <appl/diag/system.h>
#include <shared/alloc.h>
#include <sal/core/alloc.h>
#include <shared/bsl.h>

#include <soc/cm.h>
#include <soc/dma.h>
#include <soc/drv.h>
#include <soc/dcb.h>
#include <soc/cmicm.h>
#include <soc/cmic.h>

#include <sal/types.h>
#include <appl/diag/parse.h>
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/link.h>

#include "testlist.h"
#include "gen_pkt.h"

#define MAC_DA {0x12,0x34,0x56,0x78,0x9a,0xbc}
#define MAC_SA {0xfe,0xdc,0xba,0x98,0x76,0x54}
#define TPID 0x8100
#define VLAN 0x0abc
#define XLATE_VLAN 0xdef

#define DESC_SINGLE_PKT 0x0
#define DESC_SINGLE_PKT_SPLIT 0x1
#define DESC_CHAIN_PKT 0x2
#define DESC_CHAIN_PKT_SPLIT 0x3
#define DESC_CHAIN_PKT_RELOAD 0x4

#define NUM_BYTES_PER_DESC 64
#define MAX_MTU_CHAIN_LENGTH 30
#define MAX_DESC_CHAIN_LENGTH 150
#define MTU 9216
#define MAX_SPLITS 4
#define MAX_CHANNELS 12
#define POLL 0
#define CHAIN_DONE_INTR_ONLY 1
#define BOTH_DESC_CHAIN_INTR 2
#define ENET_IPG 12
#define ENET_PREAMBLE 8

#define TX_BITMAP_PARAM_DEFAULT 0x001
#define RX_BITMAP_PARAM_DEFAULT 0x002
#define PKT_SIZE_PARAM_DEFAULT 250
#define STREAM_TX_PARAM_DEFAULT 0
#define STREAM_RX_PARAM_DEFAULT 0
#define NUM_PKTS_TX_PARAM_DEFAULT 2
#define NUM_PKTS_RX_PARAM_DEFAULT 2
#define TEST_TYPE_TX_PARAM_DEFAULT 0x2
#define TEST_TYPE_RX_PARAM_DEFAULT 0x2
#define DISABLE_RX_FC_PARAM_DEFAULT 0x1
#define CHK_PKT_INTEG_PARAM_DEFAULT 0x0;
#define RATE_CALC_INT_PARAM_DEFAULT 10;
#define POLL_INTR_PARAM_DEFAULT 0
#define VERBOSE_PARAM_DEFAULT 0
#define SW_THREADS_OFF_PARAM_DEFAULT 1
#define SKIP_COS_CTRL_BMP_PARAM_DEFAULT 0
#define SV_OVERRIDE_PARAM_DEFAULT 0
#define PKT_CHK_INT_PARAM_DEFAULT 100
#define MAX_PKT_SIZE_PARAM_DEFAULT MTU
#define MIN_PKT_SIZE_PARAM_DEFAULT 64
#define NUM_LB_PORTS_PARAM_DEFAULT 20
#define CMICX_LOOPBACK_PARAM_DEFAULT 0
#define CPU_TIME_COST_PARAM_DEFAULT 0

#define NUM_CONT_DMA_DV 12
#define CH0_IN_HALT 27

#define MAX_COS 16
#define MIN_PKT_SIZE 64
#define MAX_PKT_SIZE MTU

#define NUM_SUBP_OBM 4
#define NUM_BYTES_MAC_ADDR 6
#define NUM_BYTES_CRC 4

#define TD2P_PGWS_PER_DEV 8

#define CMICM_MMU_BKP_OFF_THRESHOLD 0x20
#define CMICX_MMU_BKP_OFF_THRESHOLD 0xff

#if defined(BCM_ESW_SUPPORT) && defined(BCM_CMICM_SUPPORT)

typedef void (*isr_chain_done_t) (int, dv_t *);
typedef void (*isr_reload_done_t) (int, dv_t *);
typedef void (*isr_desc_done_t) (int, dv_t *, dcb_t *);

/* pktdma_t: struct for test */
typedef struct pktdma_s {
    uint32 tx_bitmap_param;
    uint32 rx_bitmap_param;
    uint32 pkt_size_param;
    uint32 stream_tx_param;
    uint32 stream_rx_param;
    uint32 num_pkts_tx_param;
    uint32 num_pkts_rx_param;
    uint32 test_type_tx_param;
    uint32 test_type_rx_param;
    uint32 chk_pkt_integ_param;
    uint32 rate_calc_int_param;
    uint32 poll_intr_param;
    uint32 verbose_param;
    uint32 sw_threads_off_param;
    uint32 skip_cos_ctrl_bmp_param;
    uint32 sv_override_param;
    uint32 pkt_chk_int_param;
    uint32 max_pkt_size_param;
    uint32 min_pkt_size_param;
    uint32 num_lb_ports_param;
    uint32 cmicx_loopback_param;
    uint32 cpu_time_cost_param;
    uint32 debug_param;
    uint8 ***tx_pkt_array;
    uint8 ***rx_pkt_array;
    volatile uint32 desc_done_count_tx[MAX_CHANNELS];
    volatile uint32 desc_done_count_rx[MAX_CHANNELS];
    volatile uint32 chain_done_count_tx[MAX_CHANNELS];
    volatile uint32 chain_done_count_rx[MAX_CHANNELS];
    volatile uint32 reload_done_count_tx[MAX_CHANNELS];
    volatile uint32 reload_done_count_rx[MAX_CHANNELS];
    volatile uint32 chain_done_tx[MAX_CHANNELS];
    volatile uint32 chain_done_rx[MAX_CHANNELS];
    volatile uint32 reload_done_tx[MAX_CHANNELS];
    volatile uint32 reload_done_rx[MAX_CHANNELS];
    uint32 **random_pkt_sizes;
    uint32 source_ch[MAX_CHANNELS];
    uint32 kill_dma;
    uint32 chain_tx;
    uint32 sg_tx;
    uint32 reload_tx;
    uint32 chain_rx;
    uint32 sg_rx;
    uint32 reload_rx;
    uint32 tx_ch_pkt_counters[MAX_CHANNELS];
    uint32 rx_ch_pkt_counters[MAX_CHANNELS];
    uint32 cont_dma;
    uint32 pkt_seed;
    isr_desc_done_t desc_done_intr_tx_table[MAX_CHANNELS];
    isr_desc_done_t desc_done_intr_rx_table[MAX_CHANNELS];
    isr_chain_done_t chain_done_intr_tx_table[MAX_CHANNELS];
    isr_chain_done_t chain_done_intr_rx_table[MAX_CHANNELS];
    isr_chain_done_t reload_done_intr_tx_table[MAX_CHANNELS];
    isr_chain_done_t reload_done_intr_rx_table[MAX_CHANNELS];
    sal_thread_t pid_tx;
    sal_thread_t pid_rx;
    dv_t *dv_tx;
    dv_t *dv_rx;
    dv_t ***dv_tx_array[NUM_CONT_DMA_DV];
    dv_t ***dv_rx_array[NUM_CONT_DMA_DV];
    uint32 bad_input;
    int test_fail;
    uint32 tx_thread_done;
    uint32 rx_thread_done;
    uint32 header_offset;
    uint32 tx_channel_en[MAX_CHANNELS];
    uint32 rx_channel_en[MAX_CHANNELS];
    uint32 num_tx_channels;
    uint32 num_rx_channels;
} pktdma_t;

/* rate_calc_t: struct for rate calculation */
typedef struct rate_calc_s {
    uint32 ch_tx_pkt[2][MAX_CHANNELS];
    uint32 ch_rx_pkt[2][MAX_CHANNELS];
    uint32 cmc_tx_pkt[2][MAX_CHANNELS];
    uint32 cmc_rx_pkt[2][MAX_CHANNELS];
    sal_usecs_t ch_tx_usec[2][MAX_CHANNELS];
    sal_usecs_t ch_rx_usec[2][MAX_CHANNELS];
    sal_usecs_t cmc_tx_usec[2][MAX_CHANNELS];
    sal_usecs_t cmc_rx_usec[2][MAX_CHANNELS];
} rate_calc_t;

static pktdma_t *pktdma_parray[SOC_MAX_NUM_DEVICES];


/* CMCS_PER_DEVICE */
static uint32
soc_get_cmcs_per_device(int unit)
{
    return (SOC_CMCS_NUM(unit)); /* SOC_PCI_CMCS_NUM(unit) */
}

/* CHANS_PER_DEVICE */
static uint32
soc_get_chans_per_cmc(int unit)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        return (N_DMA_CHAN);
    }

    return 1;
}

/* get cmc and ch of the given vchan */
static void
soc_get_cmc_ch_from_vchan(int unit, uint32 vchan, uint32 *cmc, uint32 *ch)
{
    if (soc_get_chans_per_cmc(unit) > 0) {
        *cmc = vchan / soc_get_chans_per_cmc(unit);
        *ch  = vchan % soc_get_chans_per_cmc(unit);
    } else {
        *cmc = 0;
        *ch  = 0;
    }
}

/* CHAN_PKT_COUNTER */
static void
soc_get_chan_pkt_counter(int unit, uint32 vchan, uint32 *tpkt, uint32 *rpkt)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        *tpkt = soc_pci_read(unit,
                    CMIC_CMCx_PKT_COUNT_CHy_TXPKT_OFFSET(cmc, ch));
        *rpkt = soc_pci_read(unit,
                    CMIC_CMCx_PKT_COUNT_CHy_RXPKT_OFFSET(cmc, ch));
    }
}

/* CMC_PKT_COUNTER */
static void
soc_get_cmc_pkt_counter(int unit, uint32 cmc, uint32 *tpkt, uint32 *rpkt)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        *tpkt = soc_pci_read(unit,
                             CMIC_CMCx_PKT_COUNT_TXPKT_OFFSET(cmc));
        *rpkt = soc_pci_read(unit,
                             CMIC_CMCx_PKT_COUNT_RXPKT_OFFSET(cmc));
    }
}

/* get CMIC_CMCx_CHy_COS_CTRL_RX_0_OFFSET and
       CMIC_CMCx_CHy_COS_CTRL_RX_1_OFFSET */
static void
soc_get_dma_chan_cos_ctrl(int unit, uint32 vchan,
                          uint32 *cos_bmp0, uint32 *cos_bmp1)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        (*cos_bmp0) = soc_pci_read(unit,
                        CMIC_CMCx_CHy_COS_CTRL_RX_0_OFFSET(cmc, ch));
        (*cos_bmp1) = soc_pci_read(unit,
                        CMIC_CMCx_CHy_COS_CTRL_RX_1_OFFSET(cmc, ch));
    }
}

/* set CMIC_CMCx_CHy_COS_CTRL_RX_0_OFFSET and
       CMIC_CMCx_CHy_COS_CTRL_RX_1_OFFSET */
static void
soc_set_dma_chan_cos_ctrl(int unit, uint32 vchan,
                          uint32 cos_bmp0, uint32 cos_bmp1)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        soc_pci_write(unit,
                      CMIC_CMCx_CHy_COS_CTRL_RX_0_OFFSET(cmc, ch), cos_bmp0);
        soc_pci_write(unit,
                      CMIC_CMCx_CHy_COS_CTRL_RX_1_OFFSET(cmc, ch), cos_bmp1);
    }
}

/* get CMIC_CMCx_CHy_DMA_CTRL_OFFSET */
static void
soc_get_dma_chan_ctrl_reg(int unit, uint32 vchan, uint32 *rdata)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        (*rdata) = soc_pci_read(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, ch));
    }
}

/* set CMIC_CMCx_CHy_DMA_CTRL_OFFSET */
static void
soc_set_dma_chan_ctrl_reg(int unit, uint32 vchan, uint32 rdata)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc, ch), rdata);
    }
}

/* clear CMIC_CMCx_DMA_STAT reg */
static void
soc_clear_dma_cmc_status(int unit, uint32 cmc)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        /* soc_dma_chan_status_clear(unit, (cmc * soc_get_chans_per_cmc(unit))); */
        soc_pci_write(unit, CMIC_CMCx_DMA_STAT_CLR_OFFSET(cmc), 0x0);
    }
}

/* clear CMIC_CMCx_PKT_COUNT_TXPKT_OFFSET and
         CMIC_CMCx_PKT_COUNT_RXPKT_OFFSET */
static void
soc_clear_dma_cmc_pkt_counter(int unit, uint32 cmc)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_pci_write(unit, CMIC_CMCx_PKT_COUNT_TXPKT_OFFSET(cmc), 0x0);
        soc_pci_write(unit, CMIC_CMCx_PKT_COUNT_RXPKT_OFFSET(cmc), 0x0);
    }
}

/* clear CMIC_CMCx_PKT_COUNT_CHy_TXPKT_OFFSET and
         CMIC_CMCx_PKT_COUNT_CHy_RXPKT_OFFSET */
static void
soc_clear_dma_ch_pkt_counter(int unit, uint32 vchan)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
            soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
            soc_pci_write(unit,
                    CMIC_CMCx_PKT_COUNT_CHy_TXPKT_OFFSET(cmc, ch), 0x0);
            soc_pci_write(unit,
                    CMIC_CMCx_PKT_COUNT_CHy_RXPKT_OFFSET(cmc, ch), 0x0);
    }
}

/*
 * Function:
 *      pktdma_soc_set_header_offset
 * Purpose:
 *      Set header offset for packets received by CMIC.
 *          -- CMICM -> 0
 *          -- CMICX -> 64
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_soc_set_header_offset(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 header_size = 0;

#ifdef BCM_CMICX_SUPPORT
    if (soc_feature(unit, soc_feature_cmicx)) {
        soc_dma_header_size_get(unit, &header_size);
    }
#endif
    pktdma_p->header_offset = header_size;
    cli_out("\n[CMIC_PKT_HEADER_OFFSET] %d\n", pktdma_p->header_offset);
}

/*
 * Function:
 *      pktdma_get_pkt_size
 * Purpose:
 *      Get packet size of the j-th packet of the i-th channel.
 * Parameters:
 *      unit    - StrataSwitch Unit #.
 *      dv_type - DV type (TX or RX).
 *      i       - virtual channel number.
 *      j       - packet number/index.
 *
 * Returns:
 *     packet size of the j-th packet of the i-th channel.
 */
static int
pktdma_get_pkt_size(int unit, dvt_t dv_type, int i, int j)
{
    int pkt_size = MIN_PKT_SIZE;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    if (pktdma_p->pkt_size_param == 0) {
        if (dv_type == DV_TX) {
            if (i < MAX_CHANNELS && j < MAX_DESC_CHAIN_LENGTH) {
                pkt_size = pktdma_p->random_pkt_sizes[i][j];
            } else {
                pkt_size = pktdma_p->min_pkt_size_param;
            }
        } else if (dv_type == DV_RX) {
            pkt_size = pktdma_p->max_pkt_size_param;
        }
    } else {
        pkt_size = pktdma_p->pkt_size_param;
    }

    if (dv_type == DV_RX) {
        pkt_size += pktdma_p->header_offset;
    }

    return pkt_size;
}

/*
 * Function:
 *      pktdma_parse_test_params
 * Purpose:
 *      Parse CLI parameters, create test structure and flag bad inputs.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      a - Pointer to arguments
 *
 * Returns:
 *     Nothing
 * Notes:
 *      pktdma_p->bad_input set from here - tells test to crash out in case CLI
 *      input combination is invalid.
 */
static void
pktdma_parse_test_params(int unit, args_t *a)
{
    parse_table_t parse_table;
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    char tr500_test_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nDocumentation too long to be displayed with -pedantic compiler\n";
#else
    "CMIC Packet DMA performance test.\n"
    "This test is intended to test the CMIC packet DMA performance. It can\n"
    "also be used as a feature test. The test can simultaneously exercise\n"
    "multiple TX and RX channels across multiple CMCs.\n\n"
    "Parameters passed from the CLI:\n"
    "TxBitmap : bitmap of TX channels. For example 0x333 signifies channels\n"
    "           0 and 1 of all 3 CMCs.\n"
    "RxBitmap : bitmap of RX channels. For example 0xccc signifies channels\n"
    "           2 and 3 of all 3 CMCs. Note that the Tx and Rx bitmaps\n"
    "           should be mutually exclusive and there should be at least\n"
    "           1 Tx channel enabled if Rx is enabled.\n"
    "PktSize  : Packet size in bytes. Use 0 for random packet sizes.\n"
    "StreamTx : Continuously stream packets on TX.\n"
    "StreamRx : Continuously stream packets on RX.\n"
    "NumPktsTx : Number of packets for which descriptors are set up at one time.\n"
    "            If StreamTx=0, this is the number of packets transmitted. If\n"
    "            chaining is enabled by the TestTypeTx parameter, this is also the\n"
    "            number of packets-per-chain.\n"
    "NumPktsRx : Number of packets for which descriptors are set up at one time.\n"
    "            If StreamRx=0, this is the number of packets transmitted. If\n"
    "            chaining is enabled by the TestTypeRx parameter, this is also the\n"
    "            number of packets-per-chain.\n"
    "TestTypeTx : Used to set the descriptor type or test type.\n"
    "             0: Simple Packet DMA: Each descriptor points to a single packet\n"
    "             1: Scatter/Gather without packet chaining: Each packet is split\n"
    "                among 2 descriptors. However there is only 1 packet per chain.\n"
    "             2: Chained DMA: Each descriptor points to 1 packet. However we\n"
    "                have descriptor chaining. Number of packets per chain is\n"
    "                specified by NumPktsTx.\n"
    "             3: Scatter/Gather with packet chaining: Each packet split among\n"
    "                2 descriptors. Number of packets per chain specified by\n"
    "                NumPktsTx.\n"
    "             4: Reload in infinite loop: Reload descriptor added at the end\n"
    "                that points to the top of the descriptor chain whose length\n"
    "                is specified by NumPktsTx. This creates an infinite HW loop.\n"
    "                This is very useful for measuring the raw HW performance of\n"
    "                the DMA engine..\n"
    "TestTypeRx : Used to set the descriptor type.\n"
    "             0: Simple Packet DMA: Each descriptor points to a single packet\n"
    "             1: Scatter/Gather without packet chaining: Each packet is split\n"
    "                among 2 descriptors. However there is only 1 packet per chain.\n"
    "             2: Chained DMA: Each descriptor points to 1 packet. However we\n"
    "                have descriptor chaining. Number of packets per chain\n"
    "                specified by NumPktsRx.\n"
    "             3: Scatter/Gather with packet chaining: Each packet is split\n"
    "                among 2 descriptors. Number of packets per chain specified by\n"
    "                NumPktsRx.\n"
    "             4: Reload in infinite loop: Reload descriptor added at the end\n"
    "                that points to the top of the descriptor chain whose length\n"
    "                is specified by NumPktsRx. This creates an infinite HW loop.\n"
    "                This is very useful for measuring the raw HW performance of\n"
    "                the DMA engine.\n"
    "ChkPktInteg : Enable packet integrity checks\n"
    "RateCalcInt : Interval in seconds over which TX/RX rates are calculated.\n"
    "PollIntr    : Set to 0 for polling mode, 1 to enable chain done interrupts and\n"
    "              2 to enable both descriptor done and chain done interrupts\n"
    "Verbose     : Print descriptors and packet pointers\n"
    "SwThOff     : Turn off memscan, linkscan and counter collection threads\n"
    "SkipCosBmp  : Skip COS Ctrl bitmap programming in CMIC\n"
    "SVOverride  : Special override for internal SV testing\n"
    "CmicxLoopBack: Perform cmicx loopback\n"
    "PktChkInt   : Interval between successive packet content checks in streaming\n"
    "              mode. It is specified in terms of NumPktsRx. For example setting\n"
    "              this parameterto 100 with NumPktsRx=20 means 20 pkts will be\n"
    "              checked for content every 20100=2000 received packets.\n"
    "MaxPktSize  : Max packet size if random packet sizes are used (PktSize=0).\n"
    "NumLbPorts  : Number of front panel ports used for looping back packets if\n"
    "              streaming enabled.\n";
#endif
    pktdma_p->bad_input = 0;

    /*Assign Default values */
    pktdma_p->tx_bitmap_param         = TX_BITMAP_PARAM_DEFAULT;
    pktdma_p->rx_bitmap_param         = RX_BITMAP_PARAM_DEFAULT;
    pktdma_p->pkt_size_param          = PKT_SIZE_PARAM_DEFAULT;
    pktdma_p->stream_tx_param         = STREAM_TX_PARAM_DEFAULT;
    pktdma_p->stream_rx_param         = STREAM_RX_PARAM_DEFAULT;
    pktdma_p->num_pkts_tx_param       = NUM_PKTS_TX_PARAM_DEFAULT;
    pktdma_p->num_pkts_rx_param       = NUM_PKTS_RX_PARAM_DEFAULT;
    pktdma_p->test_type_tx_param      = TEST_TYPE_TX_PARAM_DEFAULT;
    pktdma_p->test_type_rx_param      = TEST_TYPE_RX_PARAM_DEFAULT;
    pktdma_p->chk_pkt_integ_param     = CHK_PKT_INTEG_PARAM_DEFAULT;
    pktdma_p->rate_calc_int_param     = RATE_CALC_INT_PARAM_DEFAULT;
    pktdma_p->poll_intr_param         = POLL_INTR_PARAM_DEFAULT;
    pktdma_p->verbose_param           = VERBOSE_PARAM_DEFAULT;
    pktdma_p->sw_threads_off_param    = SW_THREADS_OFF_PARAM_DEFAULT;
    pktdma_p->skip_cos_ctrl_bmp_param = SKIP_COS_CTRL_BMP_PARAM_DEFAULT;
    pktdma_p->sv_override_param       = SV_OVERRIDE_PARAM_DEFAULT;
    pktdma_p->cmicx_loopback_param    = CMICX_LOOPBACK_PARAM_DEFAULT;
    pktdma_p->pkt_chk_int_param       = PKT_CHK_INT_PARAM_DEFAULT;
    pktdma_p->max_pkt_size_param      = MAX_PKT_SIZE_PARAM_DEFAULT;
    pktdma_p->min_pkt_size_param      = MIN_PKT_SIZE_PARAM_DEFAULT;
    pktdma_p->num_lb_ports_param      = NUM_LB_PORTS_PARAM_DEFAULT;
    pktdma_p->cpu_time_cost_param     = CPU_TIME_COST_PARAM_DEFAULT;
    pktdma_p->debug_param             = 0;

    /*Parse CLI opts */
    parse_table_init(unit, &parse_table);
    parse_table_add(&parse_table, "TxBitmap",        PQ_HEX|PQ_DFL, 0,
                    &(pktdma_p->tx_bitmap_param), NULL);
    parse_table_add(&parse_table, "RxBitmap",        PQ_HEX|PQ_DFL, 0,
                    &(pktdma_p->rx_bitmap_param), NULL);
    parse_table_add(&parse_table, "PktSize",         PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->pkt_size_param), NULL);
    parse_table_add(&parse_table, "StreamTx",        PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->stream_tx_param), NULL);
    parse_table_add(&parse_table, "StreamRx",        PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->stream_rx_param), NULL);
    parse_table_add(&parse_table, "NumPktsTx",       PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->num_pkts_tx_param), NULL);
    parse_table_add(&parse_table, "NumPktsRx",       PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->num_pkts_rx_param), NULL);
    parse_table_add(&parse_table, "TestTypeTx",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->test_type_tx_param), NULL);
    parse_table_add(&parse_table, "TestTypeRx",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->test_type_rx_param), NULL);
    parse_table_add(&parse_table, "ChkPktInteg",     PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->chk_pkt_integ_param), NULL);
    parse_table_add(&parse_table, "RateCalcInt",     PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->rate_calc_int_param), NULL);
    parse_table_add(&parse_table, "PollIntr",        PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->poll_intr_param), NULL);
    parse_table_add(&parse_table, "Verbose",         PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->verbose_param), NULL);
    parse_table_add(&parse_table, "SwThOff",         PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->sw_threads_off_param), NULL);
    parse_table_add(&parse_table, "SkipCosBmp",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->skip_cos_ctrl_bmp_param), NULL);
    parse_table_add(&parse_table, "SVOverride",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->sv_override_param), NULL);
    parse_table_add(&parse_table, "CmicxLoopBack",   PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->cmicx_loopback_param), NULL);
    parse_table_add(&parse_table, "PktChkInt",       PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->pkt_chk_int_param), NULL);
    parse_table_add(&parse_table, "MaxPktSize",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->max_pkt_size_param), NULL);
    parse_table_add(&parse_table, "MinPktSize",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->min_pkt_size_param), NULL);
    parse_table_add(&parse_table, "NumLbPorts",      PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->num_lb_ports_param), NULL);
    parse_table_add(&parse_table, "CpuTimeCost",     PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->cpu_time_cost_param), NULL);
    parse_table_add(&parse_table, "Debug",           PQ_INT|PQ_DFL, 0,
                    &(pktdma_p->debug_param), NULL);

    if (parse_arg_eq(a, &parse_table) < 0 || ARG_CNT(a) != 0) {
        test_msg(tr500_test_usage);
        test_error(unit, "%s: Invalid option: %s\n",
                   ARG_CMD(a), ARG_CUR(a) ? ARG_CUR(a) : "*");
        pktdma_p->bad_input = 1;
        parse_arg_eq_done(&parse_table);
    } else {
        cli_out("\n ------------- PRINTING TEST PARAMS ------------------");
        cli_out("\ntx_bitmap_param      = 0x%04x", pktdma_p->tx_bitmap_param);
        cli_out("\nrx_bitmap_param      = 0x%04x", pktdma_p->rx_bitmap_param);
        cli_out("\npkt_size_param       = %0d", pktdma_p->pkt_size_param);
        cli_out("\nstream_tx_param      = %0d", pktdma_p->stream_tx_param);
        cli_out("\nstream_rx_param      = %0d", pktdma_p->stream_rx_param);
        cli_out("\nnum_pkts_tx_param    = %0d", pktdma_p->num_pkts_tx_param);
        cli_out("\nnum_pkts_rx_param    = %0d", pktdma_p->num_pkts_rx_param);
        cli_out("\ntest_type_tx_param   = %0d", pktdma_p->test_type_tx_param);
        cli_out("\ntest_type_rx_param   = %0d", pktdma_p->test_type_rx_param);
        cli_out("\nchk_pkt_integ_param  = %0d", pktdma_p->chk_pkt_integ_param);
        cli_out("\nrate_calc_int_param  = %0d", pktdma_p->rate_calc_int_param);
        cli_out("\npoll_intr_param      = %0d", pktdma_p->poll_intr_param);
        cli_out("\nverbose_param        = %0d", pktdma_p->verbose_param);
        cli_out("\nsw_threads_off_param = %0d", pktdma_p->sw_threads_off_param);
        cli_out("\nskip_cos_ctrl_bmp_param = %0d",
                pktdma_p->skip_cos_ctrl_bmp_param);
        cli_out("\nsv_override_param    = %0d", pktdma_p->sv_override_param);
        cli_out("\ncmicx_loopback_param = %0d", pktdma_p->cmicx_loopback_param);
        cli_out("\npkt_chk_int_param    = %0d", pktdma_p->pkt_chk_int_param);
        cli_out("\nmax_pkt_size_param   = %0d", pktdma_p->max_pkt_size_param);
        cli_out("\nmin_pkt_size_param   = %0d", pktdma_p->min_pkt_size_param);
        cli_out("\nnum_lb_ports_param   = %0d", pktdma_p->num_lb_ports_param);
        cli_out("\ncpu_time_cost_param  = %0d", pktdma_p->cpu_time_cost_param);
        cli_out("\ndebug_param          = %0d", pktdma_p->debug_param);
        cli_out("\n -----------------------------------------------------");
    }

    /* pre-init: populate internal variables */
    pktdma_p->num_tx_channels = 0;
    pktdma_p->num_rx_channels = 0;
    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        pktdma_p->tx_channel_en[channel] = 0;
        pktdma_p->rx_channel_en[channel] = 0;
        if (((pktdma_p->tx_bitmap_param >> channel) & 0x1) != 0) {
            pktdma_p->tx_channel_en[channel] = 1;
            pktdma_p->num_tx_channels++;
        }
        if (((pktdma_p->rx_bitmap_param >> channel) & 0x1) != 0) {
            pktdma_p->rx_channel_en[channel] = 1;
            pktdma_p->num_rx_channels++;
        }
    }
    if (!(pktdma_p->pkt_size_param == 0)) {
        pktdma_p->max_pkt_size_param = MAX_PKT_SIZE;
        pktdma_p->min_pkt_size_param = MIN_PKT_SIZE;
    }

    /* check: continuous DMA mode */
    if (pktdma_p->cont_dma == 1) {
        if ((!(pktdma_p->test_type_tx_param == DESC_CHAIN_PKT ||
               pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_SPLIT)) ||
            (!(pktdma_p->test_type_rx_param == DESC_CHAIN_PKT ||
               pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_SPLIT))) {
            pktdma_p->bad_input = 1;
            test_error(unit, "\n*ERROR: Only TestTypeTx/Rx= 2 or 3 "
                             "supported by test in Cont DMA mode\n");
        }
    }
    /* check: reload mode */
    if (pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD ||
        pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD) {
        if (pktdma_p->stream_rx_param == 0 ||
            pktdma_p->stream_tx_param == 0) {
            test_error(unit,
                "\n*WARNING: TestTypeRx/Tx=4 sets up an infinite descriptor "
                "loop in hardware. Test will be treated as a performance test "
                "even though StreamRx/Tx=0\n");
        }
        pktdma_p->test_type_rx_param = DESC_CHAIN_PKT_RELOAD;
        pktdma_p->test_type_tx_param = DESC_CHAIN_PKT_RELOAD;
        pktdma_p->stream_rx_param = 1;
        pktdma_p->stream_tx_param = 1;
    }
    /* check: streaming mode */
    if (pktdma_p->stream_tx_param == 0 &&
        pktdma_p->stream_rx_param == 1 &&
        pktdma_p->rx_bitmap_param != 0) {
        pktdma_p->bad_input = 1;
        test_error(unit, "\n*ERROR: Streaming enabled on RX but not on TX\n");
    }
    /* check: packet size [min_pkt_size, max_pkt_size] */
    if (pktdma_p->pkt_size_param == 0) {
        cli_out("\nUsing random packet sizes");
        if (pktdma_p->max_pkt_size_param > MAX_PKT_SIZE ||
            pktdma_p->min_pkt_size_param < MIN_PKT_SIZE) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                   "\n*ERROR: Random Packet size cannot be higher than %0dB"
                   " or lower than %0dB\n",  MAX_PKT_SIZE, MIN_PKT_SIZE);
        }
    } else {
        if (pktdma_p->pkt_size_param < MIN_PKT_SIZE ||
            pktdma_p->pkt_size_param > MAX_PKT_SIZE) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                "\n*ERROR: Invalid packet size %0d, valid range [%0d, %0d]\n",
                pktdma_p->pkt_size_param, MIN_PKT_SIZE, MAX_PKT_SIZE);
        }
    }
    /* check: interrupt type */
    if ((pktdma_p->poll_intr_param != 0) &&
        (pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD ||
         pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD)) {
        pktdma_p->bad_input = 1;
        test_error(unit,
            "\n*ERROR: Interrupts cannot be enabled with TestTypeTx/Rx=4 "
            "since this sets up an infinite loop in hardware.\n");
    }
    /* check: TX/RX channel paring for feature test */
    if (!(pktdma_p->stream_tx_param == 1 ||
          pktdma_p->stream_rx_param == 1 ||
          pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD ||
          pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD)) {
        if (pktdma_p->num_tx_channels != pktdma_p->num_rx_channels) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                "\n*ERROR: Test is set up as a feature test with finite "
                "packet counts. Cannot do counter or packet integrity check "
                "if the number of active TX channels do not match the number "
                "of active RX channels, since they cannot be paired\n");
        }
    }
    /* check: integrity test */
    if (pktdma_p->chk_pkt_integ_param == 1) {
        if (pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD ||
            pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                "\n*ERROR: Cannot do packet integrity check "
                "in desc loop mode\n");
        }
        if (pktdma_p->num_pkts_tx_param != pktdma_p->num_pkts_rx_param) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                "\n*ERROR: Cannot do packet integrity check "
                "unless NumPktsTx==NumPktsRx\n");
        }
        if (pktdma_p->rx_bitmap_param == 0x0 ||
            pktdma_p->tx_bitmap_param == 0x0) {
            pktdma_p->bad_input = 1;
            test_error(unit,
                "\n*ERROR: Cannot check packet integrity, non-loopback\n");
        }
    }
}

/*
 * Function:
 *      pktdma_set_source_chan
 * Purpose:
 *      Fill up source_ch[] array. Provides the TX channel for correspoding
 *      RX channels when channels are paired in a non-streaming mode.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      tx_bitmap - bitmap of TX channels
 *      rx_bitmap - bitmap of RX channels
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_source_chan(int unit)
{
    uint32 tx_ch = 0, rx_ch = 0;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    for (rx_ch = 0; rx_ch < MAX_CHANNELS; rx_ch++) {
        pktdma_p->source_ch[rx_ch] = 99;
    }
    for (rx_ch = 0; rx_ch < MAX_CHANNELS; rx_ch++) {
        if (((pktdma_p->rx_bitmap_param >> rx_ch) & 0x1) != 0) {
            while (tx_ch < MAX_CHANNELS) {
                if (((pktdma_p->tx_bitmap_param >> tx_ch) & 0x1) != 0) {
                    pktdma_p->source_ch[rx_ch] = tx_ch;
                    tx_ch++;
                    break;
                }
                tx_ch++;
            }
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                     "\nrx_ch = %0d, tx_ch = %0d"),
                     rx_ch, pktdma_p->source_ch[rx_ch]));
        }
    }
}

/*
 * Function:
 *      pktdma_gen_random_l2_pkt
 * Purpose:
 *      Generate random L2 packet with seq ID
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
void
pktdma_gen_random_l2_pkt(uint8 *pkt_ptr, uint32 pkt_size,
    uint8 mac_da[NUM_BYTES_MAC_ADDR],
    uint8 mac_sa[NUM_BYTES_MAC_ADDR],
    uint16 tpid, uint16 vlan_id, uint32 seq_id, uint32 channel,
    uint32 cmicx_loopback_param)
{
    uint32 crc;

    tgp_gen_random_l2_pkt(pkt_ptr, pkt_size, mac_da, mac_sa, tpid, vlan_id);
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) +  6] = (seq_id >> 24) & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) +  7] = (seq_id >> 16) & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) +  8] = (seq_id >>  8) & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) +  9] = (seq_id)       & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) + 10] = (pkt_size >> 8) & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) + 11] = (pkt_size)      & 0xff;
    pkt_ptr[(2 * NUM_BYTES_MAC_ADDR) + 12] = (channel) & 0xff;

    tgp_populate_crc_table();
    crc = tgp_generate_calculate_crc(pkt_ptr, pkt_size);
    pkt_ptr[pkt_size - NUM_BYTES_CRC + 3] = (crc >> 24) & 0xff;
    pkt_ptr[pkt_size - NUM_BYTES_CRC + 2] = (crc >> 16) & 0xff;
    pkt_ptr[pkt_size - NUM_BYTES_CRC + 1] = (crc >>  8) & 0xff;
    pkt_ptr[pkt_size - NUM_BYTES_CRC]     = (crc)       & 0xff;

    if (cmicx_loopback_param) {
        pkt_ptr[4] = 0x0;
        /* use appropriate cos value */
        pkt_ptr[7] = 0x0;
    }
}

/*
 * Function:
 *      pktdma_gen_l2_pkt_vlan_tag
 * Purpose:
 *      Generate L2 packet VLAN tag (prio, cfi, vlan) based on priority info.
 * Parameters:
 *      unit    - StrataSwitch Unit #.
 *      prio    - priority
 *      vlan_id - vlan id
 *
 * Returns:
 *     16'b vlan tag
 */
static uint16
pktdma_gen_l2_pkt_vlan_tag(int unit, uint16 prio, uint16 vlan_id)
{
    uint16 vlan_tag, prio_cfi = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        prio_cfi = ((prio & 0x0007) << 13) |
                   (((prio >> 3) & 0x0001) << 12);
    }
    vlan_tag = prio_cfi | vlan_id;

    return (vlan_tag);
}

/*
 * Function:
 *      pktdma_store_l2_packets
 * Purpose:
 *      Store L2 packets for TX, allocate space for received packets
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_store_l2_packets(int unit)
{
    uint32 param_pkt_size_min, param_pkt_size_max,
           param_pkt_num_tx, param_pkt_num_rx;
    uint32 i, j, pkt_size;
    uint32 num_rx_channels = MAX_CHANNELS;
    uint32 **random_pkt_sizes = NULL;
    uint16 prio = 0, vlan_tag;
    uint8 mac_sa[] = MAC_SA;
    uint8 mac_da[] = MAC_DA;
    uint8 ***tx_pkt_array = NULL;
    uint8 ***rx_pkt_array = NULL;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit, "\nCalling pktdma_store_l2_packets")));
    param_pkt_size_min = MIN_PKT_SIZE;
    param_pkt_size_max = pktdma_p->max_pkt_size_param;
    param_pkt_num_tx = pktdma_p->num_pkts_tx_param;
    param_pkt_num_rx = pktdma_p->num_pkts_rx_param;

    if (pktdma_p->num_rx_channels > 0) {
        num_rx_channels = pktdma_p->num_rx_channels;
    }

    /* 1: generate random packet size array */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit, "\nPopulating random packet sizes")));
    random_pkt_sizes = (uint32 **) sal_alloc(MAX_CHANNELS * sizeof(uint32 *),
                                             "random_pkt_sizes**");
    for (i = 0; i < MAX_CHANNELS; i++) {
        random_pkt_sizes[i] = (uint32 *) sal_alloc(MAX_DESC_CHAIN_LENGTH *
                                         sizeof(uint32), "random_pkt_sizes*");
    }
    for (i = 0; i < MAX_CHANNELS; i++) {
        for (j = 0; j < MAX_DESC_CHAIN_LENGTH; j++) {
            /* coverity[dont_call : FALSE] */
            random_pkt_sizes[i][j] = param_pkt_size_min +
                    (sal_rand() % (param_pkt_size_max - param_pkt_size_min));
        }
    }
    pktdma_p->random_pkt_sizes = random_pkt_sizes;
    /* print */
    for (i = 0; i < MAX_CHANNELS; i++) {
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\nChannel %0d"), i));
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\n-----------")));
        for (j = 0; j < MAX_DESC_CHAIN_LENGTH; j++) {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "%4d "),
                     random_pkt_sizes[i][j]));
            if (j > 0 && j % 50 == 0) {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\n")));
            }
        }
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\n")));
    }

    /* 2: store tx packet array */
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\n\nStoring TX packets")));
    tx_pkt_array = (uint8 ***) soc_cm_salloc(unit,
                    MAX_CHANNELS * sizeof(uint8 **), "tx_pkt_array***");
    for (i = 0; i < MAX_CHANNELS; i++) {
        if (pktdma_p->tx_channel_en[i]) {
            tx_pkt_array[i] = (uint8 **) soc_cm_salloc(unit,
                    param_pkt_num_tx * sizeof(uint8 *), "tx_pkt_array**");
            for (j = 0; j < param_pkt_num_tx; j++) {
                /* populate packet: prio, cfi, content */
                vlan_tag = pktdma_gen_l2_pkt_vlan_tag(unit, prio, VLAN);
                pkt_size = pktdma_get_pkt_size(unit, DV_TX, i, j);
                tx_pkt_array[i][j] = (uint8 *) soc_cm_salloc(unit,
                                pkt_size * sizeof(uint8), "tx_pkt_array*");
                /* coverity[dont_call : FALSE] */
                sal_srand(pktdma_p->pkt_seed + i + j);
                pktdma_gen_random_l2_pkt(tx_pkt_array[i][j],
                        pkt_size, mac_da, mac_sa, TPID, vlan_tag,
                        j, i, pktdma_p->cmicx_loopback_param);
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                        "\n[TX_PKT_VALN_TAG] ch %2d, pkt_num %3d, "
                        "pkt_size %4d, vlan_tag 0x%04x"),
                        i, j, pkt_size, vlan_tag));
                /* update priority: for performance test, each TX channel
                 * sends packets to different RX channels by iterating cos
                 * value of all active RX channels.
                 */
                if (pktdma_p->stream_tx_param == 1) {
                    prio++;
                    prio = prio % num_rx_channels;
                }
            }
            /* update priority: for feature test, each TX channel
             * sends packets to a unique RX channel by setting a dedicated
             * cos value.
             */
            if (pktdma_p->stream_tx_param != 1) {
                prio++;
            }
        }
    }
    pktdma_p->tx_pkt_array = tx_pkt_array;

    /* 3: allocate memory for rx packet array */
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit, "\n\nAllocating memory for RX packets")));
    rx_pkt_array = (uint8 ***) soc_cm_salloc(unit,
                    MAX_CHANNELS * sizeof(uint8 **), "rx_pkt_array***");
    for (i = 0; i < MAX_CHANNELS; i++) {
        if (pktdma_p->rx_channel_en[i]) {
            rx_pkt_array[i] = (uint8 **) soc_cm_salloc(unit,
                    param_pkt_num_rx * sizeof(uint8 *), "rx_pkt_array**");
            for (j = 0; j < param_pkt_num_rx; j++) {
                pkt_size = pktdma_get_pkt_size(unit, DV_RX, i, j);
                rx_pkt_array[i][j] = (uint8 *) soc_cm_salloc(unit,
                        pkt_size * sizeof(uint8), "rx_pkt_array*");
            }
        }
    }
    pktdma_p->rx_pkt_array = rx_pkt_array;
}

/*
 * Functions:
 *      pktdma_desc_done_intr_tx
 * Purpose:
 *      Interrupt routine called from soc_dma_done_desc for TX descriptors.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *      dcb - Descriptor raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_desc_done_intr_tx(int unit, dv_t *dv, dcb_t *dcb)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->desc_done_count_tx[channel]++;
    }
}

/*
 * Functions:
 *      pktdma_desc_done_intr_rx
 * Purpose:
 *      Interrupt routine called from soc_dma_done_desc for RX descriptors.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *      dcb - Descriptor raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_desc_done_intr_rx(int unit, dv_t *dv, dcb_t *dcb)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->desc_done_count_rx[channel]++;
    }
}

/*
 * Functions:
 *      pktdma_chain_done_intr_tx
 * Purpose:
 *      Interrupt routine called from soc_dma_done_chain for TX descriptors.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chain_done_intr_tx(int unit, dv_t *dv)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->chain_done_count_tx[channel]++;
        pktdma_p->chain_done_tx[channel] = 1;
    }
}

/*
 * Functions:
 *      pktdma_chain_done_intr_rx
 * Purpose:
 *      Interrupt routine called from soc_dma_done_chain for RX descriptors.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chain_done_intr_rx(int unit, dv_t *dv)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->chain_done_count_rx[channel]++;
        pktdma_p->chain_done_rx[channel] = 1;
    }
}

/*
 * Functions:
 *      pktdma_reload_done_intr_tx
 * Purpose:
 *      Used only for continuous DMA.
 *      Interrupt routine called from soc_dma_done_desc for TX descriptors,
 *      when the reload desc at the end of a desc chain is done.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_reload_done_intr_tx(int unit, dv_t *dv)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->reload_done_count_tx[channel]++;
        pktdma_p->reload_done_tx[channel] = 1;
    }
}

/*
 * Functions:
 *      pktdma_reload_done_intr_rx
 * Purpose:
 *      Used only for continuous DMA.
 *      Interrupt routine called from soc_dma_done_desc for RX descriptors,
 *      when the reload desc at the end of a desc chain is done.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      dv - Descriptor vector raising interrupt
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_reload_done_intr_rx(int unit, dv_t *dv)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 channel;

    channel = dv->dv_public1.u32;
    if (channel < MAX_CHANNELS) {
        pktdma_p->reload_done_count_rx[channel]++;
        pktdma_p->reload_done_rx[channel] = 1;
    }
}

/*
 * Function:
 *      pktdma_set_up_isr_table
 * Purpose:
 *      Sets up interrupt related function pointers in test struct.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_isr_table(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    int vchan;

    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\nSetting up ISR table")));
    for (vchan = 0; vchan < MAX_CHANNELS; vchan++) {
        pktdma_p->desc_done_intr_tx_table[vchan] = &pktdma_desc_done_intr_tx;
        pktdma_p->desc_done_intr_rx_table[vchan] = &pktdma_desc_done_intr_rx;
        pktdma_p->chain_done_intr_tx_table[vchan] = &pktdma_chain_done_intr_tx;
        pktdma_p->chain_done_intr_rx_table[vchan] = &pktdma_chain_done_intr_rx;
        pktdma_p->reload_done_intr_tx_table[vchan] = &pktdma_reload_done_intr_tx;
        pktdma_p->reload_done_intr_rx_table[vchan] = &pktdma_reload_done_intr_rx;
    }
}

/*
 * Function:
 *      pktdma_set_global_desc_attr
 * Purpose:
 *      Sets variables in test struct for desc_attributes. Sets chain_tx,
 *      chain_rx (For chaining), sg_tx, sg_rx (Scatter/Gather) and reload_tx,
 *      reload_rx (Chain has reload desc at the end).
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_global_desc_attr(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit, "\nSetting global attributes")));

    pktdma_p->chain_tx =
            (pktdma_p->test_type_tx_param == DESC_SINGLE_PKT ||
             pktdma_p->test_type_tx_param == DESC_SINGLE_PKT_SPLIT) ? 0 : 1;
    pktdma_p->sg_tx =
            (pktdma_p->test_type_tx_param == DESC_SINGLE_PKT_SPLIT ||
             pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_SPLIT) ? 1 : 0;
    pktdma_p->reload_tx =
            (pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD) ? 1 : 0;

    pktdma_p->chain_rx =
            (pktdma_p->test_type_rx_param == DESC_SINGLE_PKT ||
             pktdma_p->test_type_rx_param == DESC_SINGLE_PKT_SPLIT) ? 0 : 1;
    pktdma_p->sg_rx =
            (pktdma_p->test_type_rx_param == DESC_SINGLE_PKT_SPLIT ||
             pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_SPLIT) ? 1 : 0;
    pktdma_p->reload_rx =
        (pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD) ? 1 : 0;
}

/*
 * Function:
 *      pktdma_set_up_dv
 * Purpose:
 *      Sets up DV arrays based on input params.
 * Parameters:
 *      unit      - StrataSwitch Unit #.
 *      dv_array  - Array of DV pointers (dv_t*).
 *      dv_type   - DV type (TX or RX).
 *      chain     - Chaining enabled if chain is 1
 *      sg        - Scatter/Gather, split each pkt into 2 descriptors if sg is 1
 *      reload    - Last desc in chain is a reload desc if set to 1
 *      poll_intr - Set to 0 for polling, 1 for chain_done interrupts, 0 for
 *                  desc_done interrupts.
 *      channel   - Virtual channel number
 *      pkt_array - 2D array of packet pointers generated by pktdma_store_l2_packets
 *      pkt_size  - Packet size in bytes
 *      num_pkts  - Number of packets in DV array
 *      random_pkt_sizes - Random packet size array created by pktdma_store_l2_packets
 *      source_ch - source_ch_array
 *      cont_dma  - 1 of cont_dma is enabled
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_dv(int unit, dv_t **dv_array, dvt_t dv_type, uint32 chain,
              uint32 sg, uint32 reload, uint32 poll_intr, uint32 channel,
              uint8 ***pkt_array, uint32 pkt_size, uint32 num_pkts,
              uint32 **random_pkt_sizes, uint32 *source_ch, uint32 cont_dma)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    pbmp_t lp_pbm, empty_pbm0, empty_pbm1;
    uint32 i, dv_num = 0, prefetch = 0;
    uint32 dma_bytes, pkt_bytes, flags = 0;
    dv_t *dv = 0;

    BCM_PBMP_CLEAR(lp_pbm);
    BCM_PBMP_CLEAR(empty_pbm0);
    BCM_PBMP_CLEAR(empty_pbm1);
    BCM_PBMP_PORT_ADD(lp_pbm, 1);

    for (i = 0; i < num_pkts; i++) {
        dv = dv_array[dv_num];
        pkt_bytes = pktdma_get_pkt_size(unit, dv_type, channel, i);
        if (sg == 0) {
            soc_dma_desc_add(dv, (sal_vaddr_t) (pkt_array[channel][i]),
                             pkt_bytes, lp_pbm, empty_pbm0, empty_pbm1,
                             flags, NULL);
        } else {
            /* coverity[dont_call : FALSE] */
            dma_bytes = ((sal_rand() % ((pkt_bytes / 4) - 4)) * 4) + 4;
            soc_dma_desc_add(dv, (sal_vaddr_t) (pkt_array[channel][i]),
                             dma_bytes, lp_pbm, empty_pbm0, empty_pbm1,
                             flags, NULL);
            soc_dma_desc_add(dv, (sal_vaddr_t) (pkt_array[channel][i] + dma_bytes),
                             pkt_bytes - dma_bytes, lp_pbm, empty_pbm0,
                             empty_pbm1, flags, NULL);
        }

        soc_dma_desc_end_packet(dv);
        if (poll_intr == POLL) {
            flags = 0;
        } else if (poll_intr == CHAIN_DONE_INTR_ONLY) {
            flags |= DV_F_NOTIFY_CHN;
        } else {
            flags |= DV_F_NOTIFY_DSC;
            flags |= DV_F_NOTIFY_CHN;
        }
        dv->dv_flags = flags;

        if (dv_type == DV_TX) {
            dv->dv_done_desc  = pktdma_p->desc_done_intr_tx_table[channel];
            dv->dv_done_chain = pktdma_p->chain_done_intr_tx_table[channel];
            dv->dv_done_reload= pktdma_p->reload_done_intr_tx_table[channel];
        } else {
            dv->dv_done_desc  = pktdma_p->desc_done_intr_rx_table[channel];
            dv->dv_done_chain = pktdma_p->chain_done_intr_rx_table[channel];
            dv->dv_done_reload= pktdma_p->reload_done_intr_rx_table[channel];
        }
        if (chain == 0) {
            dv_num++;
        }
        /* record channel number, to be used by isr functions */
        dv->dv_public1.u32 = channel;
    }
    if (cont_dma == 1) {
        soc_dma_rld_desc_add(dv, 0);
        SOC_DCB_CHAIN_SET(unit,
                        SOC_DCB_IDX2PTR(unit, dv->dv_dcb, dv->dv_vcnt - 1), 1);
        prefetch = 1;
    } else if (reload == 1) {
        soc_dma_rld_desc_add(dv, (sal_vaddr_t) dv->dv_dcb);
        SOC_DCB_CHAIN_SET(unit,
                        SOC_DCB_IDX2PTR(unit, dv->dv_dcb, dv->dv_vcnt - 1), 1);
        prefetch = 1;
    }

    if (!prefetch) {
#ifdef BCM_CMICX_SUPPORT
        soc_dma_contiguous_desc_add(dv);
#endif
    }
}

/*
 * Function:
 *      pktdma_set_up_all_dv
 * Purpose:
 *      Set up all DV arrays for all virtual channels.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_all_dv(int unit)
{
    uint32 channel;
    uint32 dv_cont;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    for (dv_cont = 0; dv_cont < NUM_CONT_DMA_DV; dv_cont++) {
        if (!((dv_cont > 0) && (pktdma_p->cont_dma == 0))) {
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (pktdma_p->tx_channel_en[channel]) {
                    pktdma_set_up_dv(unit,
                              pktdma_p->dv_tx_array[dv_cont][channel],
                              DV_TX, pktdma_p->chain_tx, pktdma_p->sg_tx,
                              pktdma_p->reload_tx, pktdma_p->poll_intr_param,
                              channel, pktdma_p->tx_pkt_array,
                              pktdma_p->pkt_size_param,
                              pktdma_p->num_pkts_tx_param,
                              pktdma_p->random_pkt_sizes,
                              pktdma_p->source_ch, pktdma_p->cont_dma);
                } else if (pktdma_p->rx_channel_en[channel]) {
                    pktdma_set_up_dv(unit,
                              pktdma_p->dv_rx_array[dv_cont][channel],
                              DV_RX, pktdma_p->chain_rx, pktdma_p->sg_rx,
                              pktdma_p->reload_rx, pktdma_p->poll_intr_param,
                              channel, pktdma_p->rx_pkt_array,
                              pktdma_p->pkt_size_param,
                              pktdma_p->num_pkts_rx_param,
                              pktdma_p->random_pkt_sizes,
                              pktdma_p->source_ch, pktdma_p->cont_dma);
                }
            }
        }
    }
}

/*
 * Function:
 *      pktdma_config_chan_intr
 * Purpose:
 *      Calls soc_dma_chan_config to configure DMA channels
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_config_chan_intr(int unit)
{
    uint32 channel;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit, "\nCalling pktdma_config_chan_intr")));
    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        if (pktdma_p->tx_channel_en[channel] != 0) {
            if (pktdma_p->poll_intr_param != POLL) {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                         "\nConfig channel %0d for interrupt"), channel));
                (void) soc_dma_chan_config(unit, channel, DV_TX, SOC_DMA_F_INTR);
            } else {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                         "\nConfig channel %0d for polling"), channel));
                (void) soc_dma_chan_config(unit, channel, DV_TX, SOC_DMA_F_POLL);
            }
        } else if (pktdma_p->rx_channel_en[channel] != 0) {
            if (pktdma_p->poll_intr_param != POLL) {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                         "\nConfig channel %0d for interrupt"), channel));
                (void) soc_dma_chan_config(unit, channel, DV_RX, SOC_DMA_F_INTR);
            } else {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                         "\nConfig channel %0d for polling"), channel));
                (void) soc_dma_chan_config(unit, channel, DV_RX, SOC_DMA_F_POLL);
            }
        }
    }
}

/*
 * Function:
 *      pktdma_soc_chk_cont_dma_halt
 * Purpose:
 *      Check DMA_STAT_HI register to see if channel is halted - used for
 *      continuous DMA
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      vchan - Virtual channel number
 *      detected - whether desc done or chain done was detected
 *
 * Returns:
 *    Nothing
 */
static void
pktdma_soc_chk_cont_dma_halt(int unit, uint32 vchan, int *detected)
{
    int halt;
    uint32 cmc = 0, ch = 0, rdata = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        rdata = soc_pci_read(unit, CMIC_CMCx_DMA_STAT_HI_OFFSET(cmc));
        halt = (((rdata >> (CH0_IN_HALT + ch)) % 2) != 0);
        *detected = (halt != 0) ? 1: 0;
    }
}

/*
 * Function:
 *      pktdma_soc_chk_dma_chan_done
 * Purpose:
 *      Check chain done or desc done bit in DMA status register.
 * Parameters:
 *      unit     - StrataSwitch Unit #.
 *      vchan    - Virtual channel number
 *      type     -  check desc done or chain done
 *      detected - whether desc done or chain done was detected
 *
 * Returns:
 *     SOC_E_XXXX
 *
 */
static int
pktdma_soc_chk_dma_chan_done(int unit, uint32 vchan, soc_dma_poll_type_t type,
                             int *detected)
{
    uint32 cmc = 0, ch = 0;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        soc_get_cmc_ch_from_vchan(unit, vchan, &cmc, &ch);
        switch (type) {
            case SOC_DMA_POLL_DESC_DONE:
                *detected = (soc_pci_read(unit,
                                          CMIC_CMCx_DMA_STAT_OFFSET(cmc))
                                          &DS_CMCx_DMA_DESC_DONE(ch));
                break;
            case SOC_DMA_POLL_CHAIN_DONE:
                *detected = (soc_pci_read(unit,
                                          CMIC_CMCx_DMA_STAT_OFFSET(cmc))
                                          &DS_CMCx_DMA_CHAIN_DONE(ch));
                break;
            default:
                break;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      pktdma_print_time_cost
 * Purpose:
 *      Print CPU time cost for the input channel
 * Parameters:
 *      unit    - StrataSwitch Unit #.
 *      vchan   - Virtual channel number
 *      stime   - Start time
 *      str     - prefixed string to be printed out.
 *
 * Returns:
 *    Nothing
 */
static void
pktdma_print_time_cost(int unit, int vchan, sal_usecs_t stime, char *str)
{
    sal_usecs_t etime, time_cost;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    if (pktdma_p->cpu_time_cost_param != 0) {
        etime = sal_time_usecs();
        time_cost = SAL_USECS_SUB(etime, stime);
        cli_out("\n%s channel %0d, time_cost(us) = %0d",
                str, vchan, (int) time_cost);
    }
}

/*
 * Function:
 *      pktdma_chk_pkt_content
 * Purpose:
 *      Check received packets for content
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      channel : RX channel
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chk_pkt_content(int unit, uint32 channel)
{
    uint32 j, k, match = 1, pkt_size_param;
    uint8 *packet, *exp_packet;
    uint32 seq_id, pkt_size, tx_channel;
    uint16 vlan_prio;
    uint8 mac_sa[] = MAC_SA;
    uint8 mac_da[] = MAC_DA;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    pkt_size_param = (pktdma_p->pkt_size_param == 0) ?
                     pktdma_p->max_pkt_size_param: pktdma_p->pkt_size_param;
    packet = sal_alloc(pkt_size_param * sizeof(uint8), "Packet");
    exp_packet = sal_alloc(pkt_size_param * sizeof(uint8), "Exp Packet");

    if (pktdma_p->rx_channel_en[channel]) {
        for (j = 0; j < pktdma_p->num_pkts_rx_param; j++) {
            for (k = 0; k < pkt_size_param; k++) {
                packet[k] = pktdma_p->rx_pkt_array[channel][j]
                                            [k + pktdma_p->header_offset];
            }
            packet[(2 * NUM_BYTES_MAC_ADDR) + 2] &= 0xf0;
            packet[(2 * NUM_BYTES_MAC_ADDR) + 2] |= (VLAN >> 8) & 0x000f;
            packet[(2 * NUM_BYTES_MAC_ADDR) + 3] = VLAN & 0x00ff;
            seq_id = 0x0;
            pkt_size = 0x0;
            tx_channel = 0x0;
            vlan_prio = 0x0;

            vlan_prio  |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 2] << 8);
            vlan_prio  |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 3]);
            seq_id     |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 6] << 24);
            seq_id     |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 7] << 16);
            seq_id     |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 8] << 8);
            seq_id     |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 9]);
            pkt_size   |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 10] << 8);
            pkt_size   |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 11]);
            tx_channel |= (packet[(2 * NUM_BYTES_MAC_ADDR) + 12]);

            sal_srand(pktdma_p->pkt_seed + tx_channel + seq_id);
            pktdma_gen_random_l2_pkt(exp_packet, pkt_size, mac_da, mac_sa,
                                         TPID, vlan_prio, seq_id, tx_channel,
                                         pktdma_p->cmicx_loopback_param);

            for (k = 0; k < pkt_size; k++) {
                if (packet[k] != exp_packet[k]) {
                    test_error(unit, "\nCorrupt packet received on "
                                     "channel %0d at location %p", channel,
                                     pktdma_p->rx_pkt_array[channel][j]);
                    pktdma_p->test_fail = 1;
                    match = 0;
                    break;
                }
            }
        }
    }

    if (match == 0) {
        test_error(unit, "\n*ERROR: Packet content check failed for channel %0d",
                   channel);
        pktdma_p->test_fail = 1;
    }
    sal_free(packet);
    sal_free(exp_packet);
}

/*
 * Function:
 *      pktdma_txdma_thread
 * Purpose:
 *      TX DMA thread - one thread takes care of all active channels
 * Parameters:
 *      up - Pointer to StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_txdma_thread(void *up)
{
    int unit = PTR_TO_INT(up);
    uint32 channel, performance_test = 0;
    uint32 first_time[MAX_CHANNELS];
    uint32 dv_num[MAX_CHANNELS];
    uint32 dv_cont[MAX_CHANNELS];
    uint32 channel_all_dv_done[MAX_CHANNELS];
    int poll_chain_done_detected = 0;
    uint32 all_channels_done = 0;
    sal_usecs_t stime = 0, db_stime = 0;
    uint32 reset_dv = 1;

    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nKick off TX thread");
    performance_test = (pktdma_p->stream_tx_param == 1) ? 1 : 0;
    if (pktdma_p->test_type_tx_param == DESC_CHAIN_PKT_RELOAD ||
        pktdma_p->stream_tx_param == 0) {
        reset_dv = 0;
    }

    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        first_time[channel] = 1;
        dv_cont[channel] = 0;
        dv_num[channel] = 0;
        channel_all_dv_done[channel] = 0;
    }
    if (pktdma_p->cpu_time_cost_param != 0) {
        stime = sal_time_usecs();
    }

    do {
        for (channel = 0; channel < MAX_CHANNELS; channel++) {
            if ((pktdma_p->tx_channel_en[channel]) &&
                (channel_all_dv_done[channel] == 0 || performance_test)) {
                /* if performance test, reset variables after one round done */
                if (performance_test && channel_all_dv_done[channel] == 1) {
                    channel_all_dv_done[channel] = 0;
                    dv_num[channel] = 0;
                    if (pktdma_p->cont_dma == 1) {
                        dv_cont[channel]++;
                        dv_cont[channel] = dv_cont[channel] % NUM_CONT_DMA_DV;
                    }
                }
                /* check DMA status for current channel */
                if ((pktdma_p->poll_intr_param == POLL || pktdma_p->cont_dma)
                    && (first_time[channel] == 0)
                    && (pktdma_p->test_type_tx_param != DESC_CHAIN_PKT_RELOAD)) {
                    if (pktdma_p->cont_dma) {
                        pktdma_soc_chk_cont_dma_halt(unit, channel,
                                                &poll_chain_done_detected);
                    } else {
                        pktdma_soc_chk_dma_chan_done(unit, channel,
                                                SOC_DMA_POLL_CHAIN_DONE,
                                                &poll_chain_done_detected);
                    }
                }
                /* kick off dma for one DV */
                if (first_time[channel] || poll_chain_done_detected ||
                    pktdma_p->chain_done_tx[channel]) {
                    pktdma_p->chain_done_tx[channel] = 0;
                    /* check abnormal status */
                    if (pktdma_p->poll_intr_param == POLL &&
                        poll_chain_done_detected &&
                        pktdma_p->cont_dma == 0) {
                        if (dv_num[channel] == 0) {
                            if (pktdma_p->chain_tx == 1) {
                                soc_dma_abort_dv(unit,
                                    pktdma_p->dv_tx_array[dv_cont[channel]]
                                                         [channel][0]);
                            } else {
                                soc_dma_abort_dv(unit,
                                    pktdma_p->dv_tx_array[dv_cont[channel]]
                                    [channel][pktdma_p->num_pkts_tx_param - 1]);
                            }
                        } else {
                            soc_dma_abort_dv(unit,
                                pktdma_p->dv_tx_array[dv_cont[channel]]
                                          [channel][dv_num[channel] - 1]);
                        }
                    }
                    /* refill DV */
                    if (performance_test && reset_dv) {
                    soc_dma_dv_reset(DV_TX,
                        pktdma_p->dv_tx_array[dv_cont[channel]][channel]
                                             [dv_num[channel]]);
                    pktdma_set_up_dv(unit,
                              pktdma_p->
                              dv_tx_array[dv_cont[channel]][channel], DV_TX,
                              pktdma_p->chain_tx, pktdma_p->sg_tx,
                              pktdma_p->reload_tx,
                              pktdma_p->poll_intr_param, channel,
                              pktdma_p->tx_pkt_array,
                              pktdma_p->pkt_size_param,
                              pktdma_p->num_pkts_tx_param,
                              pktdma_p->random_pkt_sizes,
                              pktdma_p->source_ch, pktdma_p->cont_dma);
                    }
                    /* call soc API to kick off dma for one DV */
                    if (pktdma_p->cpu_time_cost_param > 1) {
                        db_stime = sal_time_usecs();
                    }
                    soc_dma_start(unit, channel,
                        pktdma_p->dv_tx_array[dv_cont[channel]][channel]
                                             [dv_num[channel]]);
                    if (pktdma_p->cpu_time_cost_param > 1) {
                        pktdma_print_time_cost(unit, channel, db_stime,
                                               "[soc_dma_start] TX");
                    }
                    dv_num[channel]++;
                    if ((pktdma_p->chain_tx && dv_num[channel] == 1) ||
                        (dv_num[channel] == pktdma_p->num_pkts_tx_param)) {
                        channel_all_dv_done[channel] = 1;
                    }
                    /* print CPU time cost of each channel */
                    if (pktdma_p->cpu_time_cost_param != 0) {
                        pktdma_print_time_cost(unit, channel, stime,
                                               "[time_cost] TX");
                        stime = sal_time_usecs();
                    }
                    first_time[channel] = 0;
                }
            }
            poll_chain_done_detected = 0;
        }
        if (performance_test == 0) {
            all_channels_done = 1;
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (channel_all_dv_done[channel] == 0 &&
                    pktdma_p->tx_channel_en[channel]) {
                    all_channels_done = 0;
                }
            }
        }
    } while (all_channels_done != 1 && pktdma_p->kill_dma == 0);

    pktdma_p->tx_thread_done = 1;
}

/*
 * Function:
 *      pktdma_rxdma_thread
 * Purpose:
 *      RX DMA thread - one thread takes care of all active channels
 * Parameters:
 *      up - Pointer to StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_rxdma_thread(void *up)
{
    int unit = PTR_TO_INT(up);
    uint32 channel, performance_test = 0;
    int poll_chain_done_detected = 0;
    uint32 all_channels_done = 0;
    uint32 first_time[MAX_CHANNELS];
    uint32 dv_num[MAX_CHANNELS];
    uint32 channel_all_dv_done[MAX_CHANNELS];
    uint32 dv_cont[MAX_CHANNELS];
    uint32 skip_chain_cnt[MAX_CHANNELS];
    uint32 first_chain_done[MAX_CHANNELS];
    sal_usecs_t stime = 0, db_stime = 0;
    uint32 reset_dv = 1;

    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nKick off RX thread");
    performance_test = (pktdma_p->stream_rx_param == 1) ? 1 : 0;
    if (pktdma_p->test_type_rx_param == DESC_CHAIN_PKT_RELOAD ||
        pktdma_p->stream_rx_param == 0) {
        reset_dv = 0;
    }

    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        first_time[channel] = 1;
        dv_cont[channel] = 0;
        dv_num[channel] = 0;
        channel_all_dv_done[channel] = 0;
        skip_chain_cnt[channel] = 0;
        first_chain_done[channel] = 0;
    }

    if (pktdma_p->cpu_time_cost_param != 0) {
        stime = sal_time_usecs();
    }

    do {
        for (channel = 0; channel < MAX_CHANNELS; channel++) {
            if ((pktdma_p->rx_channel_en[channel]) &&
                (channel_all_dv_done[channel] == 0 || performance_test)){
                /* if performance test, reset variables after one round done */
                if (performance_test && channel_all_dv_done[channel] == 1) {
                    channel_all_dv_done[channel] = 0;
                    dv_num[channel] = 0;
                    if (pktdma_p->cont_dma == 1) {
                        dv_cont[channel]++;
                        dv_cont[channel] = dv_cont[channel] % NUM_CONT_DMA_DV;
                    }
                }
                /* check DMA status for current channel */
                if ((pktdma_p->poll_intr_param == POLL || pktdma_p->cont_dma)
                    && (first_time[channel] == 0)
                    && (pktdma_p->test_type_rx_param != DESC_CHAIN_PKT_RELOAD)) {
                    if (pktdma_p->cont_dma) {
                        pktdma_soc_chk_cont_dma_halt(unit, channel,
                                                &poll_chain_done_detected);
                    } else {
                        pktdma_soc_chk_dma_chan_done(unit, channel,
                                                SOC_DMA_POLL_CHAIN_DONE,
                                                &poll_chain_done_detected);
                    }
                }
                /* kick off dma for one DV */
                if (first_time[channel] || poll_chain_done_detected ||
                    pktdma_p->chain_done_rx[channel]) {
                    pktdma_p->chain_done_rx[channel] = 0;
                    /* check abnormal status */
                    if (pktdma_p->poll_intr_param == POLL &&
                        poll_chain_done_detected &&
                        pktdma_p->cont_dma == 0) {
                        if (dv_num[channel] == 0) {
                            if (pktdma_p->chain_rx == 1) {
                                soc_dma_abort_dv(unit,
                                    pktdma_p->dv_rx_array[dv_cont[channel]]
                                                         [channel][0]);
                            } else {
                                soc_dma_abort_dv(unit,
                                    pktdma_p->dv_rx_array[dv_cont[channel]]
                                    [channel][pktdma_p->num_pkts_rx_param - 1]);
                            }
                        } else {
                            soc_dma_abort_dv(unit,
                                pktdma_p->dv_rx_array[dv_cont[channel]]
                                          [channel][dv_num[channel] - 1]);
                        }
                    }
                    /* refill DV */
                    if (performance_test && reset_dv) {
                    soc_dma_dv_reset(DV_RX,
                        pktdma_p->dv_rx_array[dv_cont[channel]][channel]
                                             [dv_num[channel]]);
                    pktdma_set_up_dv(unit,
                              pktdma_p->
                              dv_rx_array[dv_cont[channel]][channel], DV_RX,
                              pktdma_p->chain_rx, pktdma_p->sg_rx,
                              pktdma_p->reload_rx,
                              pktdma_p->poll_intr_param, channel,
                              pktdma_p->rx_pkt_array,
                              pktdma_p->pkt_size_param,
                              pktdma_p->num_pkts_rx_param,
                              pktdma_p->random_pkt_sizes,
                              pktdma_p->source_ch, pktdma_p->cont_dma);
                    }
                    dv_num[channel]++;

                    if ((pktdma_p->chain_rx && dv_num[channel] == 1) ||
                        (dv_num[channel] == pktdma_p->num_pkts_rx_param)) {
                        channel_all_dv_done[channel] = 1;
                        if ((pktdma_p->chk_pkt_integ_param == 1) &&
                            (skip_chain_cnt[channel] == 0) &&
                            (first_time[channel] == 0) &&
                            (first_chain_done[channel] == 1)) {
                            pktdma_chk_pkt_content(unit, channel);
                        }
                        skip_chain_cnt[channel]
                                    = (skip_chain_cnt[channel] + 1)
                                            % pktdma_p->pkt_chk_int_param;
                        first_chain_done[channel] = 1;
                    }
                    /* call soc API to kick off dma for one DV */
                    if (pktdma_p->cpu_time_cost_param > 1) {
                        db_stime = sal_time_usecs();
                    }
                    soc_dma_start(unit, channel,
                                  pktdma_p->
                                  dv_rx_array[dv_cont[channel]][channel]
                                  [dv_num[channel] - 1]);
                    if (pktdma_p->cpu_time_cost_param > 1) {
                        pktdma_print_time_cost(unit, channel, db_stime,
                                               "[soc_dma_start] RX");
                    }
                    /* CPU time cost of each channel */
                    if (pktdma_p->cpu_time_cost_param != 0) {
                        pktdma_print_time_cost(unit, channel, stime,
                                               "[time_cost] RX");
                        stime = sal_time_usecs();
                    }
                    first_time[channel] = 0;
                }
            }
            poll_chain_done_detected = 0;
        }
        if (performance_test == 0) {
            all_channels_done = 1;
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (channel_all_dv_done[channel] == 0 &&
                    pktdma_p->rx_channel_en[channel]) {
                    all_channels_done = 0;
                }
            }
        }
    } while ((all_channels_done != 1) && pktdma_p->kill_dma == 0);

    pktdma_p->rx_thread_done = 1;
}

/*
 * Function:
 *      pktdma_soc_remap_hostmem_addr
 * Purpose:
 *      Program CMIC_CMCx_HOSTMEM_ADDR_REMAP regs for all CMCs
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_soc_remap_hostmem_addr(int unit)
{
    uint32 cmc, cmc_max;

    cmc_max = soc_get_cmcs_per_device(unit);
    if (SOC_IS_TOMAHAWK(unit)) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_0_OFFSET(cmc),
                        0x144d2450);
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_1_OFFSET(cmc),
                        0x19617595);
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_2_OFFSET(cmc),
                        0x1e75c6da);
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_3_OFFSET(cmc),
                        0x1f);
        }
    } else if (SOC_IS_TRIDENT2PLUS(unit)) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_0_OFFSET(cmc),
                        0x2b49ca30);
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_1_OFFSET(cmc),
                        0x37ace2f6);
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_2_OFFSET(cmc),
                        0xffbbc);
        }
    }
}

/*
 * Function:
 *      pktdma_turn_off_cmic_mmu_bkp_cmicx
 * Purpose:
 *      Turn off CMIC to MMU backpressure
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
#ifdef BCM_CMICX_SUPPORT
static void
pktdma_turn_off_cmic_mmu_bkp_cmicx(int unit)
{
    int cmc, ch, vchan, cmc_max, ch_max;
    uint32 threshold;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);

    if (soc_feature(unit, soc_feature_cmicx)) {
        threshold = CMICX_MMU_BKP_OFF_THRESHOLD;
        /* cmc */
        for (cmc = 0; cmc < cmc_max; cmc++) {
            soc_dma_cmc_rxbuf_threshold_cfg(unit, cmc, threshold);
        }
        /* channel */
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = cmc * ch_max + ch;
                soc_dma_chan_rxbuf_threshold_cfg(unit, vchan, threshold);
            }
        }
    }
}
#endif
/*
 * Function:
 *      pktdma_turn_off_cmic_mmu_bkp_cmicm
 * Purpose:
 *      Turn off CMIC to MMU backpressure
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_turn_off_cmic_mmu_bkp_cmicm(int unit)
{
    int cmc, ch, cmc_max, ch_max;
    /* int vchan; */
    uint32 threshold;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        threshold = CMICM_MMU_BKP_OFF_THRESHOLD;
        /* channel */
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
/* 
                vchan = cmc * ch_max + ch;
                soc_dma_chan_rxbuf_threshold_cfg(unit, vchan, threshold); */
                soc_pci_write(unit,
                        CMIC_CMCx_CHy_RXBUF_THRESHOLD_CONFIG(cmc, ch),
                        threshold);
            }
        }
    }
}

/*
 * Function:
 *      pktdma_turn_off_cmic_mmu_bkp
 * Purpose:
 *      Turn off CMIC to MMU backpressure
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_turn_off_cmic_mmu_bkp(int unit)
{
    /* CMICX */
#ifdef BCM_CMICX_SUPPORT
    if (soc_feature(unit, soc_feature_cmicx)) {
        pktdma_turn_off_cmic_mmu_bkp_cmicx(unit);
    } else
#endif
    /* CMICM */
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        pktdma_turn_off_cmic_mmu_bkp_cmicm(unit);
    }
}

/*
 * Function:
 *      pktdma_set_up_mac_lpbk
 * Purpose:
 *      Enable MAC loopback on all ports
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_mac_lpbk(int unit)
{
    uint32 p;

    PBMP_ITER(PBMP_PORT_ALL(unit), p) {
        (void) bcm_port_loopback_set(unit, p, BCM_PORT_LOOPBACK_MAC);
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_cpu_th
 * Purpose:
 *      Program CPU_COS_MAP table for 16 priorities. Maps each internal
 *      priority to an unique COS queue. (Tomahawk)
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_cpu_th(int unit)
{
    cpu_cos_map_entry_t cpu_cos_map_entry;
    uint32 entry;

    if (SOC_MEM_IS_VALID(unit, CPU_COS_MAPm)) {
        for (entry = 0; entry < MAX_COS; entry++) {
            (void) soc_mem_read(unit, CPU_COS_MAPm, COPYNO_ALL, entry,
                                cpu_cos_map_entry.entry_data);
            soc_mem_field32_set(unit, CPU_COS_MAPm,
                                cpu_cos_map_entry.entry_data,
                                VALIDf, 0x1);
            soc_mem_field32_set(unit, CPU_COS_MAPm,
                                cpu_cos_map_entry.entry_data,
                                INT_PRI_MASKf, 0xf);
            soc_mem_field32_set(unit, CPU_COS_MAPm,
                                cpu_cos_map_entry.entry_data,
                                INT_PRI_KEYf, entry);
            soc_mem_field32_set(unit, CPU_COS_MAPm,
                                cpu_cos_map_entry.entry_data,
                                COSf, entry);
            soc_mem_field32_set(unit, CPU_COS_MAPm,
                                cpu_cos_map_entry.entry_data,
                                RQE_CPU_COSf, (entry % 2));
            (void) soc_mem_write(unit, CPU_COS_MAPm, COPYNO_ALL, entry,
                                 cpu_cos_map_entry.entry_data);
        }
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_cpu
 * Purpose:
 *      Program CPU_COS_MAP table for 16 priorities. Maps each internal
 *      priority to an unique COS queue.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_cpu(int unit)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        pktdma_set_cos_4_cpu_th(unit);
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_ipep_th
 * Purpose:
 *      Misc table programming to map 16 internal priorities to corresponding
 *      COS queues. For example prio 0 is mapped to queue 0, priority 1 to
 *      queue 1, and so on. (Tomahawk)
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_ipep_th(int unit)
{
    uint32 i, j, p, entry_size, entry_num = 0;
    uint64 cpu_lo_rqe_q_num, cpu_hi_rqe_q_num, mirror_rqe_q_num;

    ing_pri_cng_map_entry_t ing_pri_cng_map_entry;
    ing_outer_dot1p_mapping_table_entry_t ing_outer_dot1p_mapping_table_entry;
    egr_pri_cng_map_entry_t egr_pri_cng_map_entry;
    port_cos_map_entry_t port_cos_map_entry;
    soc_field_t egr_vlan_control_1_fields[] =
                            {CFI_AS_CNGf, VT_MISS_DROPf, VT_ENABLEf};
    uint32 egr_vlan_control_1_values[] = { 0x1, 0x0, 0x0 };

    COMPILER_64_SET(cpu_lo_rqe_q_num, 0x0, 0x8);
    COMPILER_64_SET(cpu_hi_rqe_q_num, 0x0, 0x9);
    COMPILER_64_SET(mirror_rqe_q_num, 0x0, 0xa);

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        /* ING_PRI_CNG_MAPm */
        if (SOC_MEM_IS_VALID(unit, ING_PRI_CNG_MAPm)) {
            entry_num = soc_mem_index_max(unit, ING_PRI_CNG_MAPm);
            entry_size = sizeof(ing_pri_cng_map_entry.entry_data) /
                         sizeof(uint32);
            for (i = 0; i < entry_num; i++) {
                for (j = 0; j < entry_size; j++) {
                    ing_pri_cng_map_entry.entry_data[j] = 0x0;
                }
                soc_mem_field32_set(unit, ING_PRI_CNG_MAPm,
                                    ing_pri_cng_map_entry.entry_data, PRIf,
                                    ((8 * (i % 2)) + ((i % 16) / 2)));
                (void) soc_mem_write(unit, ING_PRI_CNG_MAPm, COPYNO_ALL, i,
                                     ing_pri_cng_map_entry.entry_data);
            }
        }
        /* ING_OUTER_DOT1P_MAPPING_TABLEm */
        if (SOC_MEM_IS_VALID(unit, ING_OUTER_DOT1P_MAPPING_TABLEm)) {
            entry_num = soc_mem_index_max(unit, ING_OUTER_DOT1P_MAPPING_TABLEm);
            entry_size = sizeof(ing_outer_dot1p_mapping_table_entry.entry_data) /
                         sizeof(uint32);
            for (i = 0; i < entry_num; i++) {
                for (j = 0; j < entry_size; j++) {
                    ing_outer_dot1p_mapping_table_entry.entry_data[j] = 0x0;
                }
                soc_mem_field32_set(unit, ING_OUTER_DOT1P_MAPPING_TABLEm,
                                    ing_outer_dot1p_mapping_table_entry.entry_data,
                                    NEW_CFIf, (i % 2));
                soc_mem_field32_set(unit, ING_OUTER_DOT1P_MAPPING_TABLEm,
                                    ing_outer_dot1p_mapping_table_entry.entry_data,
                                    NEW_DOT1Pf, ((i % 16) / 2));
                (void) soc_mem_write(unit, ING_OUTER_DOT1P_MAPPING_TABLEm,
                                     COPYNO_ALL, i,
                                     ing_outer_dot1p_mapping_table_entry.entry_data);
            }
        }
        /* EGR_PRI_CNG_MAPm */
        if (SOC_MEM_IS_VALID(unit, EGR_PRI_CNG_MAPm)) {
            entry_num = soc_mem_index_max(unit, EGR_PRI_CNG_MAPm);
            entry_size = sizeof(egr_pri_cng_map_entry.entry_data) / sizeof(uint32);
            for (i = 0; i < entry_num; i++) {
                for (j = 0; j < entry_size; j++) {
                    egr_pri_cng_map_entry.entry_data[j] = 0x0;
                }
                soc_mem_field32_set(unit, EGR_PRI_CNG_MAPm,
                                    egr_pri_cng_map_entry.entry_data,
                                    CFIf, ((i / 32) % 2));
                soc_mem_field32_set(unit, EGR_PRI_CNG_MAPm,
                                    egr_pri_cng_map_entry.entry_data,
                                    PRIf, ((i % 32) / 4));
                (void) soc_mem_write(unit, EGR_PRI_CNG_MAPm, COPYNO_ALL, i,
                                     egr_pri_cng_map_entry.entry_data);
            }
        }
        /* PORT_COS_MAPm */
        if (SOC_MEM_IS_VALID(unit, PORT_COS_MAPm)) {
            entry_num = soc_mem_index_max(unit, PORT_COS_MAPm);
            entry_size = sizeof(port_cos_map_entry.entry_data) / sizeof(uint32);
            for (i = 0; i < entry_num; i++) {
                for (j = 0; j < entry_size; j++) {
                    port_cos_map_entry.entry_data[j] = 0x0;
                }
                soc_mem_field32_set(unit, PORT_COS_MAPm,
                                    port_cos_map_entry.entry_data,
                                    UC_COS1f, ((i % 16) % 10));
                soc_mem_field32_set(unit, PORT_COS_MAPm,
                                    port_cos_map_entry.entry_data,
                                    MC_COS1f, ((i % 16) % 10));
                soc_mem_field32_set(unit, PORT_COS_MAPm,
                                    port_cos_map_entry.entry_data,
                                    RQE_Q_NUMf, ((i % 16) % 11));
                (void) soc_mem_write(unit, PORT_COS_MAPm, COPYNO_ALL, i,
                                     port_cos_map_entry.entry_data);
            }
        }
        /* regs */
        /* if (SOC_IS_TOMAHAWK(unit)) { */
            if (SOC_REG_IS_VALID(unit, CPU_LO_RQE_Q_NUMr)) {
                (void) soc_reg_set(unit, CPU_LO_RQE_Q_NUMr, 0, 0, cpu_lo_rqe_q_num);
            }
            if (SOC_REG_IS_VALID(unit, CPU_HI_RQE_Q_NUMr)) {
                (void) soc_reg_set(unit, CPU_HI_RQE_Q_NUMr, 0, 0, cpu_hi_rqe_q_num);
            }
            if (SOC_REG_IS_VALID(unit, MIRROR_RQE_Q_NUMr)) {
                (void) soc_reg_set(unit, MIRROR_RQE_Q_NUMr, 0, 0, mirror_rqe_q_num);
            }
        /* } */
        if (SOC_REG_IS_VALID(unit, EGR_VLAN_CONTROL_1r)) {
            PBMP_ITER(PBMP_PORT_ALL(unit), p) {
                soc_reg_fields32_modify(unit, EGR_VLAN_CONTROL_1r, p, 3,
                                        egr_vlan_control_1_fields,
                                        egr_vlan_control_1_values);
            }
        }
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_ipep
 * Purpose:
 *      Misc table programming to map 16 internal priorities to corresponding COS
 *      queues. For example prio 0 is mapped to queue 0, priority 1 to queue 1
 *      and so on.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_ipep(int unit)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        pktdma_set_cos_4_ipep_th(unit);
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_cmic_th
 * Purpose:
 *      Program COS_CTRL bitmaps. (Tomahawk)
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      rx_bitmap: bitmap of RX (virtual) channels
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_cmic_th(int unit, uint32 rx_bitmap)
{
    uint32 cos_cnt = 0, cos_bmp0 = 0, cos_bmp1 = 0, cos_bmp_left = 0;
    uint32 cmc, ch, vchan = 0, cmc_max, ch_max, vchan_max;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max  = soc_get_chans_per_cmc(unit);
    vchan_max = (cmc_max * ch_max) - 1;

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        /* set each RX channel with a unique cos value */
        cos_bmp_left = 0xffffffff;
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = cmc * ch_max + ch;
                soc_set_dma_chan_cos_ctrl(unit, vchan, 0x0, 0x0);
                if (((rx_bitmap >> vchan) & 0x1) != 0) {
                    cos_bmp0 = 0x1 << cos_cnt;
                    cos_bmp1 = 0x0;
                    soc_set_dma_chan_cos_ctrl(unit, vchan, cos_bmp0, cos_bmp1);
                    cos_cnt++;
                    cos_bmp_left &= ~cos_bmp0;
                }
            }
        }
        /* use the last RX channel to mask all unused cos values */
        vchan = vchan_max - 1;
        soc_get_dma_chan_cos_ctrl(unit, vchan, &cos_bmp0, &cos_bmp1);
        cos_bmp0 |= cos_bmp_left;
        cos_bmp1 = 0xffffffff;
        soc_set_dma_chan_cos_ctrl(unit, vchan, cos_bmp0, cos_bmp1);
    }
}

/*
 * Function:
 *      pktdma_set_cos_4_cmic
 * Purpose:
 *      Program COS_CTRL bitmaps.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      rx_bitmap: bitmap of RX (virtual) channels
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_cos_4_cmic(int unit, uint32 rx_bitmap)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        pktdma_set_cos_4_cmic_th(unit, rx_bitmap);
    }
}

/*
 * Function:
 *      pktdma_init_misc
 * Purpose:
 *      Basic initialization for packet DMA
 * Parameters:
 *      unit: StrataSwitch Unit #
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_init_misc(int unit)
{
    int disable_rx_fc = 1;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    /* set RX packet header off (CMICX) */
    pktdma_soc_set_header_offset(unit);

    /* remap hostmem address */
    cli_out("\nRemapping hostmem address");
    pktdma_soc_remap_hostmem_addr(unit);

    /* turn off cmic to mmu backpressue*/
    if (disable_rx_fc == 1) {
        cli_out("\nTurning off CMIC to MMU backpressue");
        pktdma_turn_off_cmic_mmu_bkp(unit);
    }

    /* set mac loopback */
    cli_out("\nSetting up MAC loopback on all ports");
    pktdma_set_up_mac_lpbk(unit);

    /* program CPU_COS_MAP */
    cli_out("\nProgramming CPU_COS_MAP");
    pktdma_set_cos_4_cpu(unit);

    /* program IPEP cos tables */
    cli_out("\nProgramming IP/EP COS_MAP for 16 priorities");
    pktdma_set_cos_4_ipep(unit);

    /* program 48-bit CMIC cos mapping */
    if (pktdma_p->skip_cos_ctrl_bmp_param != 1) {
        cli_out("\nProgramming CMIC RX channel to COS mapping");
        pktdma_set_cos_4_cmic(unit, pktdma_p->rx_bitmap_param);
    }
}

/*
 * Function:
 *      pktdma_clear_dma_ctrl_regs
 * Purpose:
 *      Clear all CMIC_CMCx_CHy_DMA_CTRL regs
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_clear_dma_ctrl_regs(int unit)
{
    uint32 cmc, ch, vchan, cmc_max, ch_max;
    uint32 rdata = 0, ctrl_reg = 0;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);

    soc_get_dma_chan_ctrl_reg(unit, 0, &rdata);
    rdata &= (PKTDMA_BIG_ENDIAN | PKTDMA_DESC_BIG_ENDIAN);
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "\nrdata = 0x%08x"), rdata));

    for (cmc = 0; cmc < cmc_max; cmc++) {
        for (ch = 0; ch < ch_max; ch++) {
            vchan = (cmc * ch_max) + ch;
            soc_set_dma_chan_ctrl_reg(unit, vchan, rdata);
            soc_get_dma_chan_ctrl_reg(unit, vchan, &ctrl_reg);
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                        "\nCMIC_CMC%0d_CH%0d_DMA_CTRL = 0x%08x"),
                        cmc, ch, ctrl_reg));
        }
    }
}

/*
 * Function:
 *      pktdma_clear_dma_status_regs
 * Purpose:
 *      Clear all CMIC_CMCx_DMA_STAT regs
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_clear_dma_status_regs(int unit)
{
    uint32 cmc, cmc_max;

    cmc_max = soc_get_cmcs_per_device(unit);

    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            soc_clear_dma_cmc_status(unit, cmc);
        }
    }
}

/*
 * Function:
 *      pktdma_clear_dma_pkt_counters
 * Purpose:
 *      Clear CMIC packet counters
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_clear_dma_pkt_counters(int unit)
{
    uint32 cmc, ch, vchan, cmc_max, ch_max;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);
    for (cmc = 0; cmc < cmc_max; cmc++) {
        soc_clear_dma_cmc_pkt_counter(unit, cmc);
        for (ch = 0; ch < ch_max; ch++) {
            vchan = (cmc * ch_max) + ch;
            soc_clear_dma_ch_pkt_counter(unit, vchan);
        }
    }
}

/*
 * Function:
 *      pktdma_clear_dma
 * Purpose:
 *      Cleanup routine. Aborts all packet DMAs and clears ctrl and status regs
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      abort_dma: Set to 1 to abort all packet DMA operations
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_clear_dma(int unit, int abort_dma)
{
    cli_out("\nClearing DMA status regs");
    pktdma_clear_dma_status_regs(unit);
    if (abort_dma != 0) {
        cli_out("\nAborting all DMA ops");
        soc_dma_abort(unit);
    }
    sal_usleep(100000);
    cli_out("\nClearing DMA ctrl regs");
    pktdma_clear_dma_ctrl_regs(unit);
    cli_out("\nClearing CMIC packet counters");
    pktdma_clear_dma_pkt_counters(unit);
    cli_out("\nClearing DMA status regs");
    pktdma_clear_dma_status_regs(unit);
}

/*
 * Function:
 *      pktdma_set_up_vlan
 * Purpose:
 *      VLAN programming to set up packet flows.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_vlan(int unit)
{
    pbmp_t pbm, ubm;
    uint32 port, port_cnt, *port_list;
    bcm_vlan_t test_vlan = VLAN;
    bcm_vlan_t xlate_vlan_start = XLATE_VLAN;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    port_list = (uint32 *)(sal_alloc(pktdma_p->num_lb_ports_param *
                                        sizeof(uint32), "port_list"));
    port_cnt = 0;
    PBMP_ITER(PBMP_PORT_ALL(unit), port) {
        if (port_cnt < pktdma_p->num_lb_ports_param) {
            port_list[port_cnt] = port;
            port_cnt++;
        }
    }
    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_CLEAR(ubm);

    cli_out("\nSetting up VLANs");
    if (pktdma_p->rx_bitmap_param == 0x0) {
        cli_out("\nrx_bitmap = 0x000, Test is a TX only test");
        BCM_PBMP_PORT_ADD(pbm, port_list[1]);
        BCM_PBMP_PORT_ADD(pbm, 0);
        bcm_vlan_create(unit, test_vlan);
        bcm_vlan_create(unit, xlate_vlan_start);
        bcm_vlan_port_add(unit, xlate_vlan_start, pbm, ubm);
        bcm_vlan_port_add(unit, test_vlan, pbm, ubm);
        bcm_vlan_control_set(unit, bcmVlanTranslate, TRUE);
        bcm_vlan_translate_add(unit, port_list[1], test_vlan,
                               xlate_vlan_start, 0);
    } else if (pktdma_p->tx_bitmap_param == 0x0) {
        cli_out("\ntx_bitmap = 0x000, Test is a RX only test."
                "The test needs to be done with a TG");
        BCM_PBMP_CLEAR(pbm);
        BCM_PBMP_PORT_ADD(pbm, port_list[1]);
        BCM_PBMP_PORT_ADD(pbm, 0);
        BCM_PBMP_PORT_ADD(pbm, port_list[2]);
        bcm_vlan_create(unit, test_vlan);
        bcm_vlan_port_add(unit, test_vlan, pbm, ubm);
    } else {
        cli_out("\nThis is a loopback test");
        /* case 1: performance test */
        if (pktdma_p->stream_tx_param == 1 ||
            pktdma_p->stream_rx_param == 1) {
            /* create translate vlan for each port */
            cli_out("\nSetting up VLAN xlates for perf test");
            bcm_vlan_control_set(unit, bcmVlanTranslate, TRUE);
            for (port = 0; port < pktdma_p->num_lb_ports_param; port++) {
                BCM_PBMP_CLEAR(pbm);
                BCM_PBMP_PORT_ADD(pbm, 0);
                BCM_PBMP_PORT_ADD(pbm, port_list[port]);
                bcm_vlan_create(unit, (xlate_vlan_start + port));
                bcm_vlan_port_add(unit, (xlate_vlan_start + port), pbm, ubm);
                bcm_vlan_translate_add(unit, port_list[port], test_vlan,
                                       xlate_vlan_start + port, 0);
            }
            for (port = 0; port < pktdma_p->num_lb_ports_param; port++) {
                cli_out("\n[vlan_translate] old_vlan 0x%04x, new_vlan 0x%04x,"
                        " ports [%0d, %0d]",
                        test_vlan, xlate_vlan_start + port, 0, port_list[port]);
            }

            /* add all ports into the test_vlan for vlan flooding */
            BCM_PBMP_CLEAR(pbm);
            for (port = 0; port < pktdma_p->num_lb_ports_param; port++) {
                BCM_PBMP_PORT_ADD(pbm, port_list[port]);
            }
            bcm_vlan_create(unit, test_vlan);
            bcm_vlan_port_add(unit, test_vlan, pbm, ubm);
        }
        /* case 1: feature test */
        else {
            BCM_PBMP_CLEAR(pbm);
            BCM_PBMP_PORT_ADD(pbm, port_list[1]);
            BCM_PBMP_PORT_ADD(pbm, 0);
            bcm_vlan_create(unit, test_vlan);
            bcm_vlan_port_add(unit, test_vlan, pbm, ubm);
        }
    }

    /* print all ports in test_vlan */
    cli_out("\n[vlan_setup] vlan_id 0x%04x, port [", test_vlan);
    PBMP_ITER(pbm, port) {
        cli_out("%d, ", port);
    }
    cli_out("]");

    sal_free(port_list);
}

/*
 * Function:
 *      pktdma_turn_off_idb_fc_handle
 * Purpose:
 *      Turn off IDB flow control.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      idb_fcc_regs: pionter to IDB flow control regs.
 *      a_size: num of regs in idb_fcc_regs
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_turn_off_idb_fc_handle(int unit, soc_reg_t *idb_fcc_regs, int a_size)
{
    int obm_subp, idx, rv;
    soc_reg_t reg;
    uint64 idb_fc;
    soc_field_t idb_flow_control_config_fields[] = {
        PORT_FC_ENf, LOSSLESS1_FC_ENf, LOSSLESS0_FC_ENf,
        LOSSLESS1_PRIORITY_PROFILEf, LOSSLESS0_PRIORITY_PROFILEf
    };
    uint32 idb_flow_control_config_values[] = { 0x0, 0x0, 0x0, 0xff, 0xff };

    COMPILER_64_SET(idb_fc, 0x0, 0x0);

    if (SOC_REG_IS_VALID(unit, IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE0r)) {
        reg = IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE0r;
        soc_reg_fields32_modify(unit, reg, REG_PORT_ANY, 5,
                                idb_flow_control_config_fields,
                                idb_flow_control_config_values);
        rv = soc_reg_get(unit, reg, 0, 0, &idb_fc);
        if (SOC_FAILURE(rv)) {
            test_error(unit, "\nError, failed to write reg %s",
                             SOC_REG_NAME(unit, reg));
        }
    }

    for (obm_subp = 0; obm_subp < NUM_SUBP_OBM; obm_subp++) {
        for(idx = 0; idx < a_size; idx++) {
            if(SOC_REG_IS_VALID(unit, idb_fcc_regs[idx])) {
                rv = soc_reg_set(unit, idb_fcc_regs[idx], 0, obm_subp, idb_fc);
                if (SOC_FAILURE(rv)) {
                    test_error(unit, "\nError, failed to write reg %s",
                                     SOC_REG_NAME(unit, idb_fcc_regs[idx]));
                }
            }
        }
    }
}

/*
 * Function:
 *      pktdma_turn_off_fc_th
 * Purpose:
 *      Turn off flow control at the MAC, IDB and MMU. (Tomahawk)
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_turn_off_fc_th(int unit)
{
    int i, p, idx, num_regs = 0;

    soc_reg_t idb_fcc_regs[] = {
        IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM1_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM2_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM3_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM4_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM5_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM6_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM7_FLOW_CONTROL_CONFIG_PIPE0r,
        IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM1_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM2_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM3_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM4_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM5_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM6_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM7_FLOW_CONTROL_CONFIG_PIPE1r,
        IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM1_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM2_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM3_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM4_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM5_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM6_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM7_FLOW_CONTROL_CONFIG_PIPE2r,
        IDB_OBM0_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM1_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM2_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM3_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM4_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM5_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM6_FLOW_CONTROL_CONFIG_PIPE3r,
        IDB_OBM7_FLOW_CONTROL_CONFIG_PIPE3r
    };

    soc_reg_t pgw_fcc_regs[] = {
        PGW_OBM_PORT0_FC_CONFIGr,
        PGW_OBM_PORT1_FC_CONFIGr,
        PGW_OBM_PORT2_FC_CONFIGr,
        PGW_OBM_PORT3_FC_CONFIGr,
        PGW_OBM_PORT4_FC_CONFIGr,
        PGW_OBM_PORT5_FC_CONFIGr,
        PGW_OBM_PORT6_FC_CONFIGr,
        PGW_OBM_PORT7_FC_CONFIGr,
        PGW_OBM_PORT8_FC_CONFIGr,
        PGW_OBM_PORT9_FC_CONFIGr,
        PGW_OBM_PORT10_FC_CONFIGr,
        PGW_OBM_PORT11_FC_CONFIGr,
        PGW_OBM_PORT12_FC_CONFIGr,
        PGW_OBM_PORT13_FC_CONFIGr,
        PGW_OBM_PORT14_FC_CONFIGr,
        PGW_OBM_PORT15_FC_CONFIGr
    };

    /* reg: THDI_INPUT_PORT_XON_ENABLESr */
    if (SOC_REG_IS_VALID(unit, THDI_INPUT_PORT_XON_ENABLESr)) {
        PBMP_ITER(PBMP_PORT_ALL(unit), p) {
            bcm_port_pause_set(unit, p, FALSE, FALSE);
            soc_reg_field32_modify(unit, THDI_INPUT_PORT_XON_ENABLESr, p,
                                   PORT_PAUSE_ENABLEf, 0x0);
        }
        soc_reg_field32_modify(unit, THDI_INPUT_PORT_XON_ENABLESr, 0,
                               PORT_PAUSE_ENABLEf, 0x0);
    } else {
        cli_out("\n*ERROR, invalid reg %s\n",
                SOC_REG_NAME(unit, THDI_INPUT_PORT_XON_ENABLESr));
    }

    /* reg: IDB_OBM_FLOW_CONTROL regs */
    num_regs = sizeof(idb_fcc_regs) / sizeof(soc_reg_t);
    pktdma_turn_off_idb_fc_handle(unit, idb_fcc_regs, num_regs);

    /* reg: PGW_OBM_PORT_FC regs */
    for (idx = 0; idx < (sizeof(pgw_fcc_regs) / sizeof(soc_reg_t)); idx++) {
        if (SOC_REG_IS_VALID(unit, pgw_fcc_regs[idx])) {
            for (i = 0; i < TD2P_PGWS_PER_DEV; i++) {
                soc_reg_field32_modify(unit, pgw_fcc_regs[idx],
                                       /* REG_PORT_ANY, */
                                       (i | SOC_REG_ADDR_INSTANCE_MASK),
                                       PORT_FC_ENABLEf, 0x0);
            }
        }
    }
}

/*
 * Function:
 *      pktdma_turn_off_fc
 * Purpose:
 *      Turn off flow control at the MAC, IDB and MMU.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_turn_off_fc(int unit)
{
    if (SOC_IS_TOMAHAWK(unit) || SOC_IS_TRIDENT2PLUS(unit)) {
        pktdma_turn_off_fc_th(unit);
    }
}

/*
 * Function:
 *      pktdma_get_all_ch_pkt_counters
 * Purpose:
 *      Gather CMIC TX and RX packet counters
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      tx_counters - TX packet counter array
 *      rx_counters - RX packet counter array
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_get_all_ch_pkt_counters(int unit, uint32 *tx_counters,
                               uint32 *rx_counters)
{
    uint32 cmc, ch, vchan, cmc_max, ch_max;

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);
    for (cmc = 0; cmc < cmc_max; cmc++) {
        for (ch = 0; ch < ch_max; ch++) {
            vchan = ((cmc * ch_max) + ch) % MAX_CHANNELS;
            soc_get_chan_pkt_counter(unit, vchan, &tx_counters[vchan],
                                                  &rx_counters[vchan]);
        }
    }
}

/*
 * Function:
 *      pktdma_chk_pkt_stall
 * Purpose:
 *      Check for stalls on TX/RX DMA channels
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      pkt_size: Packet size in bytes
 *      interval_in_seconds: Interval over which rate is measured
 *
 * Returns:
 *     BCM_E_XXX
 */
static int
pktdma_chk_pkt_stall(int unit)
{
    int result = BCM_E_NONE;
    uint32 cmc, ch, vchan, cmc_max, ch_max;
    uint32 tx_pkt_s[MAX_CHANNELS];
    uint32 rx_pkt_s[MAX_CHANNELS];
    uint32 tx_pkt_e[MAX_CHANNELS];
    uint32 rx_pkt_e[MAX_CHANNELS];

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max  = soc_get_chans_per_cmc(unit);

    cli_out("\nChecking packet stalls ...");
    pktdma_get_all_ch_pkt_counters(unit, tx_pkt_s, rx_pkt_s);
    /* sal_usleep(1000000); */
    sal_sleep(1);
    pktdma_get_all_ch_pkt_counters(unit, tx_pkt_e, rx_pkt_e);

    for (cmc = 0; cmc < cmc_max; cmc++) {
        for (ch = 0; ch < ch_max; ch++) {
            vchan = ((cmc * ch_max) + ch) % MAX_CHANNELS;
            if (tx_pkt_e[vchan] != 0 &&
                tx_pkt_e[vchan] == tx_pkt_s[vchan]) {
                test_error(unit, "\nTXDMA on vchan %0d stalled", vchan);
                result = BCM_E_FAIL;
            }
            if (rx_pkt_e[vchan] != 0 &&
                rx_pkt_e[vchan] == rx_pkt_s[vchan]) {
                test_error(unit, "\nRXDMA on vchan %0d stalled", vchan);
                result = BCM_E_FAIL;
            }
        }
    }

    return result;
}

/*
 * Function:
 *      pktdma_convert_pbs_to_mpbs
 * Purpose:
 *      Calculate TX and RX data rates in bps for fixed size packets and in
 *      pps (packets per second) for random sized packets
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      pkt_size: Packet size in bytes
 *      interval_in_seconds: Interval over which rate is measured
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_convert_pbs_to_mpbs(uint64 bps)
{
    uint64 meg_64;
    uint32 mbps_hi_32, mbps_lo_32;
    COMPILER_64_SET(meg_64, 0, 1000000);

    mbps_lo_32 = (uint32) COMPILER_64_LO(bps) % 1000000;
    COMPILER_64_UDIV_64(bps, meg_64);
    mbps_hi_32 = (uint32) COMPILER_64_LO(bps);

    if (mbps_lo_32 / 100000 > 0) {
        cli_out(" (%0d.%0d Mbps)", mbps_hi_32, mbps_lo_32);
    } else if (mbps_lo_32 / 10000 > 0) {
        cli_out(" (%0d.0%0d Mbps)", mbps_hi_32, mbps_lo_32);
    } else if (mbps_lo_32 / 1000 > 0) {
        cli_out(" (%0d.00%0d Mbps)", mbps_hi_32, mbps_lo_32);
    } else if (mbps_lo_32 / 100 > 0) {
        cli_out(" (%0d.000%0d Mbps)", mbps_hi_32, mbps_lo_32);
    } else {
        cli_out(" (%0d.0000 Mbps)", mbps_hi_32);
    }
}

/*
 * Function:
 *      pktdma_calc_rate_bps
 * Purpose:
 *      Calculate rate in bps from packet counter values
 * Parameters:
 *      pkt_num: number of packets
 *      bytes: bytes per pkt (includes IPG and preamble)
 *      interval_usecs: Interval over which rate is measured (usec)
 *
 * Returns:
 *     Rate in bps
 */
static void
pktdma_calc_rate_bps(uint32 pkt_num, uint32 bytes_per_pkt,
                     sal_usecs_t interval_usecs, uint64 *bps)
{
    uint64 rate_bps, time_usec;

    if (interval_usecs > 0) {
        COMPILER_64_SET(rate_bps, 0x0, pkt_num);
        COMPILER_64_SET(time_usec, 0x0, (uint32)interval_usecs);
        if (bytes_per_pkt > 1) {
            COMPILER_64_UMUL_32(rate_bps, (bytes_per_pkt * 8));
        }
        COMPILER_64_UMUL_32(rate_bps, 1000000);
        COMPILER_64_UDIV_64(rate_bps, time_usec);
        COMPILER_64_SET(*bps, COMPILER_64_HI(rate_bps),
                              COMPILER_64_LO(rate_bps));
    } else {
        COMPILER_64_ZERO(*bps);
    }
}

/*
 * Function:
 *      pktdma_chk_rate
 * Purpose:
 *      Calculate TX and RX data rates in bps for fixed size packets and in
 *      pps (packets per second) for random sized packets
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *      pkt_size: Packet size in bytes
 *      interval_in_seconds: Interval over which rate is measured
 *
 * Returns:
 *     Nothing
 */
static int
pktdma_chk_rate(int unit, uint32 pkt_size, uint32 interval_in_seconds,
                uint32 tx_bitmap, uint32 rx_bitmap)
{
    int i, n, result = BCM_E_NONE;
    uint32 cmc, ch, cmc_max, ch_max, vchan;
    uint32 random_pkt_size_en = 0;
    uint32 tpkt = 0, rpkt = 0;
    uint64 rate_64;
    char rate_str[32];
    char str_tx[] = "TX";
    char str_rx[] = "RX";
    char *str_ptr;
    sal_usecs_t usecs_td;
    uint32 pkt_num;
    uint32 bytes_per_pkt;
    uint64 total_rate_64;
    rate_calc_t *rate_calc;

    /* init */
    rate_calc = sal_alloc(sizeof(rate_calc_t), "rate_calc");
    sal_memset(rate_calc, 0, sizeof(rate_calc_t));

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);
    COMPILER_64_ZERO(rate_64);
    COMPILER_64_ZERO(total_rate_64);
    random_pkt_size_en = (pkt_size == 0);

    cli_out("\nStart measuring TX/RX rate");
    cli_out("\nMAX_CMC_NUM %0d", cmc_max);
    cli_out("\nMAX_CHAN_PER_CMC %0d", ch_max);

    /* record traffic info at beginning and ending of certain interval
     *         [0][] : data at begin
     *         [1][] : data at end
     */
    for (i = 0; i < 2; i++) {
        /* record num_of_pkts and cpu_time */
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = ((cmc * ch_max) + ch) % MAX_CHANNELS;
                soc_get_chan_pkt_counter(unit, vchan, &tpkt, &rpkt);
                rate_calc->ch_tx_usec[i][vchan] = sal_time_usecs();
                rate_calc->ch_rx_usec[i][vchan] = sal_time_usecs();
                rate_calc->ch_tx_pkt[i][vchan]  = tpkt;
                rate_calc->ch_rx_pkt[i][vchan]  = rpkt;
            }
            soc_get_cmc_pkt_counter(unit, cmc, &tpkt, &rpkt);
            rate_calc->cmc_tx_usec[i][cmc] = sal_time_usecs();
            rate_calc->cmc_rx_usec[i][cmc] = sal_time_usecs();
            rate_calc->cmc_tx_pkt[i][cmc]  = tpkt;
            rate_calc->cmc_rx_pkt[i][cmc]  = rpkt;
        }

        /* wait for interval_in_seconds */
        if (i == 0) {
            cli_out("\nWait %0ds for rate measurement", interval_in_seconds);
            sal_sleep(interval_in_seconds);
            /* sal_usleep(interval_in_seconds * 1000000); */
        }
    }

    /* calculate rate: n=0 -> TX; n=1 -> RX.
     *       -- rate_per_channel
     *       -- rate_per_cmc
     *       -- total_rate
     */
    for (n = 0; n < 2; n++) {
        str_ptr  = (n == 0) ? str_tx : str_rx;
        cli_out("\n----------------- %s Rates ----------------\n", str_ptr);
        for (cmc = 0; cmc < cmc_max; cmc++) {
            /* rate per channel */
            for (ch = 0; ch < ch_max; ch++) {
                vchan = ((cmc * ch_max) + ch) % MAX_CHANNELS;
                if (n == 0) {
                    pkt_num = rate_calc->ch_tx_pkt[1][vchan] -
                              rate_calc->ch_tx_pkt[0][vchan];
                    usecs_td = SAL_USECS_SUB(rate_calc->ch_tx_usec[1][vchan],
                                             rate_calc->ch_tx_usec[0][vchan]);
                } else {
                    pkt_num = rate_calc->ch_rx_pkt[1][vchan] -
                              rate_calc->ch_rx_pkt[0][vchan];
                    usecs_td = SAL_USECS_SUB(rate_calc->ch_rx_usec[1][vchan],
                                             rate_calc->ch_rx_usec[0][vchan]);
                }
                bytes_per_pkt = pkt_size + ENET_IPG + ENET_PREAMBLE;
                bytes_per_pkt = (random_pkt_size_en == 0) ? bytes_per_pkt : 1;
                pktdma_calc_rate_bps(pkt_num, bytes_per_pkt, usecs_td, &rate_64);
                format_uint64_decimal(rate_str, rate_64, 0);
                if (random_pkt_size_en == 0) {
                    cli_out("\nCMC%0d, CH%0d : %s bps", cmc, ch, rate_str);
                    pktdma_convert_pbs_to_mpbs(rate_64);
                } else {
                    cli_out("\nCMC%0d, CH%0d : %s pps", cmc, ch, rate_str);
                }
            }
            /* rate per CMC */
            if (n == 0) {
                pkt_num = rate_calc->cmc_tx_pkt[1][cmc] -
                          rate_calc->cmc_tx_pkt[0][cmc];
                usecs_td = SAL_USECS_SUB(rate_calc->cmc_tx_usec[1][cmc],
                                         rate_calc->cmc_tx_usec[0][cmc]);
            } else {
                pkt_num = rate_calc->cmc_rx_pkt[1][cmc] -
                          rate_calc->cmc_rx_pkt[0][cmc];
                usecs_td = SAL_USECS_SUB(rate_calc->cmc_rx_usec[1][cmc],
                                         rate_calc->cmc_rx_usec[0][cmc]);
            }
            bytes_per_pkt = pkt_size + ENET_IPG + ENET_PREAMBLE;
            bytes_per_pkt = (random_pkt_size_en == 0) ? bytes_per_pkt : 1;

            pktdma_calc_rate_bps(pkt_num, bytes_per_pkt, usecs_td, &rate_64);
            COMPILER_64_ADD_64(total_rate_64, rate_64);

            format_uint64_decimal(rate_str, rate_64, 0);
            if (random_pkt_size_en == 0) {
                cli_out("\n\nCMC%0d : %s bps", cmc, rate_str);
                pktdma_convert_pbs_to_mpbs(rate_64);
            } else {
                cli_out("\nCMC%0d, CH%0d : %s pps", cmc, ch, rate_str);
            }
            cli_out("\n\n");
        }
        /* total rate */
        format_uint64_decimal(rate_str, total_rate_64, 0);
        if (random_pkt_size_en == 0) {
            cli_out("\n****Total %s rate : %s bps", str_ptr, rate_str);
            pktdma_convert_pbs_to_mpbs(total_rate_64);
        } else {
            cli_out("\n****Total %s rate : %s pps", str_ptr, rate_str);
        }
    }

    /* check */
    for (cmc = 0; cmc < cmc_max; cmc++) {
        for (ch = 0; ch < ch_max; ch++) {
            vchan = (ch_max * cmc + ch) % MAX_CHANNELS;
            if ((((tx_bitmap >> vchan) & 0x1) != 0) &&
                (rate_calc->ch_tx_pkt[1][vchan] == 0)) {
                result = BCM_E_FAIL;
                test_error(unit, "\nTXDMA enabled on CMC %0d, CH %0d, but"
                                 " no packets sent.", cmc, ch);
            }
            if ((((rx_bitmap >> vchan) & 0x1) != 0) &&
                (rate_calc->ch_rx_pkt[1][vchan] == 0)) {
                result = BCM_E_FAIL;
                test_error(unit, "\nRXDMA enabled on CMC %0d, CH %0d, but"
                                 " no packets received.", cmc, ch);
            }
        }
    }

    if(pktdma_chk_pkt_stall(unit) != BCM_E_NONE) {
        result = BCM_E_FAIL;
    }

    sal_free(rate_calc);
    return result;
}

/*
 * Function:
 *      pktdma_dump_first_dv
 * Purpose:
 *      Dump first DV for each channel
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_dump_first_dv(int unit)
{
    uint32 channel;
    uint32 dv_cont;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nDumping first DV of each Channel:");
    cli_out("\n=================================");
    for (dv_cont = 0; dv_cont < NUM_CONT_DMA_DV; dv_cont++) {
        if (!((dv_cont > 0) && (pktdma_p->cont_dma == 0))) {
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                cli_out("\nChannel %0d", channel);
                cli_out("\n-----------\n");
                if (pktdma_p->tx_channel_en[channel]) {
                    soc_dma_dump_dv(unit, "[dv_tx_dump]",
                                    pktdma_p->dv_tx_array[dv_cont][channel][0]);
                }
                if (pktdma_p->rx_channel_en[channel]) {
                    soc_dma_dump_dv(unit, "[dv_rx_dump]",
                                    pktdma_p->dv_rx_array[dv_cont][channel][0]);
                }
            }
        }
    }
    cli_out("\nEnd dumping first DV");
    cli_out("\n=====================");
}

/*
 * Function:
 *      pktdma_dump_pkt_pointers
 * Purpose:
 *      Dump packet pointers for each channel
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_dump_pkt_pointers(int unit)
{
    uint32 channel, i;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nDumping packet pointers for each channel");
    cli_out("\n****************************************");

    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        cli_out("\nDumping TX packet pointers for channel %0d", channel);
        cli_out("\n------------------------------------------\n");
        if (pktdma_p->tx_channel_en[channel]) {
            for (i = 0; i < pktdma_p->num_pkts_tx_param; i++) {
                cli_out("%p ", pktdma_p->tx_pkt_array[channel][i]);
            }
        }
        cli_out("\nDumping RX packet pointers for channel %0d", channel);
        cli_out("\n------------------------------------------\n");
        if (pktdma_p->rx_channel_en[channel]) {
            for (i = 0; i < pktdma_p->num_pkts_rx_param; i++) {
                cli_out("%p ", pktdma_p->rx_pkt_array[channel][i]);
            }
        }
        cli_out("\n");
    }
    cli_out("\nDone dumping packet pointers");
    cli_out("\n*****************************************");
}

/*
 * Function:
 *      pktdma_dump_interrupt_counts
 * Purpose:
 *      Dump number of times each interrupt was raised.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_dump_interrupt_counts(int unit)
{
    uint32 channel;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nPrinting Interrupt counts");

    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        if (pktdma_p->tx_channel_en[channel]) {
            cli_out("\nChannel %0d -----------", channel);
            cli_out("\ndesc_done_count_tx[%0d] = %0d", channel,
                    pktdma_p->desc_done_count_tx[channel]);
            cli_out("\nchain_done_count_tx[%0d] = %0d", channel,
                    pktdma_p->chain_done_count_tx[channel]);
            cli_out("\nreload_done_count_tx[%0d] = %0d", channel,
                    pktdma_p->reload_done_count_tx[channel]);
            cli_out("\n");
        }

        if (pktdma_p->rx_channel_en[channel]) {
            cli_out("\nChannel %0d -----------", channel);
            cli_out("\ndesc_done_count_rx[%0d] = %0d", channel,
                    pktdma_p->desc_done_count_rx[channel]);
            cli_out("\nchain_done_count_rx[%0d] = %0d", channel,
                    pktdma_p->chain_done_count_rx[channel]);
            cli_out("\nreload_done_count_rx[%0d] = %0d", channel,
                    pktdma_p->reload_done_count_rx[channel]);
            cli_out("\n");
        }
    }
}

/*
 * Function:
 *      pktdma_free_all_memory
 * Purpose:
 *      Free all allocated memory
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_free_all_memory(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 i, j, dv_cont;

    cli_out("\nFreeing all allocated memory");
    /* random_pkt_sizes */
    for (i = 0; i < MAX_CHANNELS; i++) {
        sal_free(pktdma_p->random_pkt_sizes[i]);
    }
    sal_free(pktdma_p->random_pkt_sizes);
    /* tx_pkt_array */
    for (i = 0; i < MAX_CHANNELS; i++) {
        if (pktdma_p->tx_channel_en[i]) {
            for (j = 0; j < pktdma_p->num_pkts_tx_param; j++) {
                soc_cm_sfree(unit, pktdma_p->tx_pkt_array[i][j]);
            }
            soc_cm_sfree(unit, pktdma_p->tx_pkt_array[i]);
        }
    }
    soc_cm_sfree(unit, pktdma_p->tx_pkt_array);
    /* rx_pkt_array */
    for (i = 0; i < MAX_CHANNELS; i++) {
        if (pktdma_p->rx_channel_en[i]) {
            for (j = 0; j < pktdma_p->num_pkts_rx_param; j++) {
                soc_cm_sfree(unit, pktdma_p->rx_pkt_array[i][j]);
            }
            soc_cm_sfree(unit, pktdma_p->rx_pkt_array[i]);
        }
    }
    soc_cm_sfree(unit, pktdma_p->rx_pkt_array);
    /* dv_tx_array and dv_rx_array */
    for (dv_cont = 0; dv_cont < NUM_CONT_DMA_DV; dv_cont++) {
        if (!((dv_cont > 0) && (pktdma_p->cont_dma == 0))) {
            for (i = 0; i < MAX_CHANNELS; i++) {
                if (pktdma_p->tx_channel_en[i]) {
                    if (pktdma_p->chain_tx == 0) {
                        for (j = 0; j < pktdma_p->num_pkts_tx_param; j++) {
                            soc_dma_dv_free(unit,
                                    pktdma_p->dv_tx_array[dv_cont][i][j]);
                        }
                    } else {
                        soc_dma_dv_free(unit,
                                        pktdma_p->dv_tx_array[dv_cont][i][0]);
                    }
                    soc_at_free(unit, pktdma_p->dv_tx_array[dv_cont][i]);
                }
            }
            soc_at_free(unit, pktdma_p->dv_tx_array[dv_cont]);

            for (i = 0; i < MAX_CHANNELS; i++) {
                if (pktdma_p->rx_channel_en[i]) {
                    if (pktdma_p->chain_rx == 0) {
                        for (j = 0; j < pktdma_p->num_pkts_rx_param; j++) {
                            soc_dma_dv_free(unit,
                                    pktdma_p->dv_rx_array[dv_cont][i][j]);
                        }
                    } else {
                        soc_dma_dv_free(unit,
                                        pktdma_p->dv_rx_array[dv_cont][i][0]);
                    }
                    soc_at_free(unit, pktdma_p->dv_rx_array[dv_cont][i]);
                }
            }
            soc_at_free(unit, pktdma_p->dv_rx_array[dv_cont]);
        }
    }
}

/*
 * Function:
 *      pktdma_chk_pkt_integrity
 * Purpose:
 *      Check packet integrity for finite packet counts
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chk_pkt_integrity(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    uint32 j, k, num_bytes_compared, match = 1;
    uint32 header_offset;
    uint32 tx_ch, rx_ch;
    uint8 ***tx_pkts;
    uint8 ***rx_pkts;
    uint8 tx_pkt_byte, rx_pkt_byte;

    header_offset = pktdma_p->header_offset;
    tx_pkts = pktdma_p->tx_pkt_array;
    rx_pkts = pktdma_p->rx_pkt_array;

    cli_out("\nChecking packet integrity ...");
    for (rx_ch = 0; rx_ch < MAX_CHANNELS; rx_ch++) {
        tx_ch = pktdma_p->source_ch[rx_ch];
        if (pktdma_p->rx_channel_en[rx_ch] && tx_ch != 99) {
            for (j = 0; j < pktdma_p->num_pkts_rx_param; j++) {
                num_bytes_compared = pktdma_get_pkt_size(unit, DV_TX, tx_ch, j);
                for (k = 0; k < num_bytes_compared; k++) {
                    tx_pkt_byte =tx_pkts[tx_ch][j][k];
                    rx_pkt_byte =rx_pkts[rx_ch][j][k + header_offset];
                    if (tx_pkt_byte != rx_pkt_byte) {
                        match = 0;
                        cli_out("\nMismatch : rx_ch = %0d, pkt = %0d, "
                             "byte = %0d, tx_byte = %02x, rx_byte = %02x",
                             rx_ch, j, k, tx_pkt_byte, rx_pkt_byte);
                    }
                }
            }
        }
    }
    if (match == 0) {
        test_error(unit, "\n*ERROR: PACKET INTEGRITY CHECK FAILED\n");
        pktdma_p->test_fail = 1;
    } else {
        cli_out("\nPACKET INTEGRITY CHECK PASSED");
    }
}

/*
 * Function:
 *      pktdma_chk_pkt_counters
 * Purpose:
 *      Test end check for packet counter values.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chk_pkt_counters(int unit)
{
    uint32 cmc, ch, vchan, cmc_max, ch_max;
    uint32 match = 1;
    uint32 disable_tx_chk = 0;
    uint32 disable_rx_chk = 0;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nChecking packet counters ...");

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);

    pktdma_get_all_ch_pkt_counters(unit, pktdma_p->tx_ch_pkt_counters,
                                         pktdma_p->rx_ch_pkt_counters);

    if (pktdma_p->stream_tx_param == 1) {
        disable_tx_chk = 1;
        cli_out("\nStreaming enabled on TX, Disabling TX Pkt counter checks");
    }

    if (pktdma_p->stream_rx_param == 1) {
        disable_rx_chk = 1;
        cli_out("\nStreaming enabled on RX, Disabling RX Pkt counter checks");
    }

    if (disable_tx_chk == 0) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = (ch_max * cmc + ch) % MAX_CHANNELS;
                if ((pktdma_p->tx_channel_en[vchan]) &&
                    (pktdma_p->tx_ch_pkt_counters[vchan] !=
                     pktdma_p->num_pkts_tx_param)) {
                    cli_out("\n*ERROR: TX Counter mismatch for vchan=%0d "
                            "cmc=%0d, ch=%0d", vchan, cmc, ch);
                    match = 0;
                }
            }
        }
    }
    if (disable_rx_chk == 0) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = (ch_max * cmc + ch) % MAX_CHANNELS;
                if ((pktdma_p->rx_channel_en[vchan]) &&
                    (pktdma_p->rx_ch_pkt_counters[vchan] !=
                     pktdma_p->num_pkts_rx_param)) {
                    cli_out("\n*ERROR: RX Counter mismatch for vchan=%0d "
                            "cmc=%0d, ch=%0d", vchan, cmc, ch);
                    match = 0;
                }
            }
        }
    }

    if (disable_tx_chk == 0 || disable_rx_chk == 0) {
        if (match == 0) {
            test_error(unit, "\n*ERROR: PACKET COUNTER CHECKS FAILED\n");
            pktdma_p->test_fail = 1;
        } else {
            cli_out("\nPACKET COUNTER CHECKS PASSED");
        }
    }
}

/*
 * Function:
 *      pktdma_chk_interrupt_counters
 * Purpose:
 *      Test end check for interrupt count values.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_chk_interrupt_counters(int unit)
{
    uint32 cmc, ch, vchan, cmc_max, ch_max;
    uint32 match = 1;
    uint32 disable_tx_chk = 0;
    uint32 disable_rx_chk = 0;
    uint32 sg_factor_tx;
    uint32 sg_factor_rx;
    uint32 exp_desc_done_cnt_tx;
    uint32 exp_chain_done_cnt_tx;
    uint32 exp_desc_done_cnt_rx;
    uint32 exp_chain_done_cnt_rx;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    cli_out("\nChecking interrupt counters ...");

    cmc_max = soc_get_cmcs_per_device(unit);
    ch_max = soc_get_chans_per_cmc(unit);

    if (pktdma_p->stream_tx_param == 1) {
        disable_tx_chk = 1;
        cli_out("\nStreaming enabled on TX, Disabling TX Intr counter checks");
    }
    if (pktdma_p->stream_rx_param == 1) {
        disable_rx_chk = 1;
        cli_out("\nStreaming enabled on RX, Disabling RX Intr counter checks");
    }

    sg_factor_tx = (pktdma_p->sg_tx == 0) ? 1 : 2;
    sg_factor_rx = (pktdma_p->sg_rx == 0) ? 1 : 2;
    exp_desc_done_cnt_tx = pktdma_p->num_pkts_tx_param * sg_factor_tx;
    exp_desc_done_cnt_rx = pktdma_p->num_pkts_rx_param * sg_factor_rx;

    exp_chain_done_cnt_tx = (pktdma_p->chain_tx == 1) ? 1 :
                            (pktdma_p->num_pkts_tx_param);
    exp_chain_done_cnt_rx = (pktdma_p->chain_rx == 1) ? 1 :
                            (pktdma_p->num_pkts_rx_param);

    /* TX */
    if (disable_tx_chk == 0) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = (ch_max * cmc + ch) % MAX_CHANNELS;
                if ((pktdma_p->tx_channel_en[vchan]) &&
                    (pktdma_p->poll_intr_param == BOTH_DESC_CHAIN_INTR)) {
                    if (pktdma_p->desc_done_count_tx[vchan] !=
                        exp_desc_done_cnt_tx) {
                        cli_out("\n*WARN: TX Desc done interrupt count mismatch "
                                "for vchan %0d, intr_exp %0d, intr_act %0d",
                                vchan, exp_desc_done_cnt_tx,
                                pktdma_p->desc_done_count_tx[vchan]);
                        /* match = 0; */
                    }
                    if( pktdma_p->cont_dma == 0) {
                        if (pktdma_p->chain_done_count_tx[vchan]
                                != exp_chain_done_cnt_tx) {
                            cli_out("\n*ERROR: TX Chain done interrupt count "
                            "mismatch for vchan %0d, intr_exp %0d, intr_act %0d",
                            vchan, exp_chain_done_cnt_tx,
                            pktdma_p->chain_done_count_tx[vchan]);
                            match = 0;
                        }
                    }
                }
            }
        }
    }
    /* RX */
    if (disable_tx_chk == 0) {
        for (cmc = 0; cmc < cmc_max; cmc++) {
            for (ch = 0; ch < ch_max; ch++) {
                vchan = (ch_max * cmc + ch) % MAX_CHANNELS;
                if ((pktdma_p->rx_channel_en[vchan]) &&
                    (pktdma_p->poll_intr_param == BOTH_DESC_CHAIN_INTR)) {
                    if (pktdma_p->desc_done_count_rx[vchan] !=
                        exp_desc_done_cnt_rx) {
                        cli_out("\n*WARN: RX Desc done interrupt count mismatch"
                                "for vchan %0d, intr_exp %0d, intr_act %0d",
                                vchan, exp_desc_done_cnt_rx,
                                pktdma_p->desc_done_count_rx[vchan]);
                        /* match = 0; */
                    }
                    if( pktdma_p->cont_dma == 0) {
                        if (pktdma_p->chain_done_count_rx[vchan]
                                != exp_chain_done_cnt_rx) {
                            cli_out("\n*ERROR: RX Chain done interrupt count "
                            "mismatch for vchan %0d, intr_exp %0d, intr_act %0d",
                            vchan, exp_chain_done_cnt_rx,
                            pktdma_p->chain_done_count_rx[vchan]);
                            match = 0;
                        }
                    }
                }
            }
        }
    }

    if (disable_tx_chk == 0 || disable_rx_chk == 0) {
        if (match == 0) {
            test_error(unit, "\n*ERROR: INTERRUPT COUNT CHECKS FAILED\n");
            pktdma_p->test_fail = 1;
        } else {
            cli_out("\nINTERRUPT COUNT CHECKS PASSED");
        }
    }
}

/*
 * Function:
 *      pktdma_store_dv
 * Purpose:
 *      Set up test for all tx/rx channels.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_store_dv(int unit)
{
    uint32 i, channel = 0;
    uint32 dv_cont;
    int dcb_num;
    pktdma_t *pktdma_p = pktdma_parray[unit];

    /* allocate memory for TX/RX dvs */
    for (dv_cont = 0; dv_cont < NUM_CONT_DMA_DV; dv_cont++) {
        if (!((dv_cont > 0) && (pktdma_p->cont_dma == 0))) {
            pktdma_p->dv_tx_array[dv_cont] =
                (dv_t ***) soc_at_alloc(unit, MAX_CHANNELS * sizeof(dv_t **),
                                        "dv_tx_array[dv_cont]_alloc");
            pktdma_p->dv_rx_array[dv_cont] =
                (dv_t ***) soc_at_alloc(unit, MAX_CHANNELS * sizeof(dv_t **),
                                        "dv_rx_array[dv_cont]_alloc");

            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (pktdma_p->tx_channel_en[channel]) {
                    pktdma_p->dv_tx_array[dv_cont][channel] =
                        (dv_t **) soc_at_alloc(unit,
                                pktdma_p->num_pkts_tx_param * sizeof(dv_t *),
                                "dv_tx_array[dv_cont]_alloc");
                    if (pktdma_p->chain_tx == 0) {
                        dcb_num = (pktdma_p->sg_tx == 1) ? 3 : 2;
                        for (i = 0; i < pktdma_p->num_pkts_tx_param; i++) {
                            pktdma_p->dv_tx_array[dv_cont][channel][i] =
                                soc_dma_dv_alloc(unit, DV_TX, dcb_num);
                        }
                    } else {
                        dcb_num = (pktdma_p->sg_tx == 1) ?
                                  (2 * (pktdma_p->num_pkts_tx_param + 1)) :
                                  (pktdma_p->num_pkts_tx_param + 1);
                        pktdma_p->dv_tx_array[dv_cont][channel][0] =
                            soc_dma_dv_alloc(unit, DV_TX, dcb_num);
                    }
                }

                if (pktdma_p->rx_channel_en[channel]) {
                    pktdma_p->dv_rx_array[dv_cont][channel] =
                        (dv_t **) soc_at_alloc(unit,
                                pktdma_p->num_pkts_rx_param * sizeof(dv_t *),
                                "dv_rx_array[dv_cont]_alloc");
                    if (pktdma_p->chain_rx == 0) {
                        dcb_num = (pktdma_p->sg_rx == 1) ? 3 : 2;
                        for (i = 0; i < pktdma_p->num_pkts_rx_param; i++) {
                            pktdma_p->dv_rx_array[dv_cont][channel][i] =
                                soc_dma_dv_alloc(unit, DV_RX, dcb_num);
                        }
                    } else {
                        dcb_num = (pktdma_p->sg_rx == 1) ?
                                  (2 * (pktdma_p->num_pkts_rx_param + 1)) :
                                  (pktdma_p->num_pkts_rx_param + 1);
                        pktdma_p->dv_rx_array[dv_cont][channel][0] =
                            soc_dma_dv_alloc(unit, DV_RX, dcb_num);
                    }
                }
            }
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit, "\ndv_tx_array[%0d] = %p"),
                      dv_cont, pktdma_p->dv_tx_array[dv_cont]));
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (pktdma_p->tx_channel_en[channel]) {
                    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                                "\ndv_tx_array[%0d][%0d] = %p"),
                                dv_cont, channel,
                                pktdma_p->dv_tx_array[dv_cont][channel]));
                    if (pktdma_p->chain_tx == 0) {
                        for (i = 0; i < pktdma_p->num_pkts_tx_param; i++) {
                            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                                      "\ndv_tx_array[%0d][%0d][%0d] = %p"),
                                      dv_cont, channel, i, pktdma_p->
                                      dv_tx_array[dv_cont][channel][i]));
                        }
                    } else {
                        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                                  "\ndv_tx_array[%0d][%0d][%0d] = %p"),
                                  dv_cont, channel, 0,
                                  pktdma_p->dv_tx_array[dv_cont][channel][0]));
                    }
                }
            }

            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                        "\ndv_rx_array[%0d] = %p"),
                        dv_cont, pktdma_p->dv_rx_array[dv_cont]));
            for (channel = 0; channel < MAX_CHANNELS; channel++) {
                if (pktdma_p->rx_channel_en[channel]) {
                    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                              "\ndv_rx_array[%0d][%0d] = %p"),
                              dv_cont, channel,
                              pktdma_p->dv_rx_array[dv_cont][channel]));
                    if (pktdma_p->chain_rx == 0) {
                        for (i = 0; i < pktdma_p->num_pkts_rx_param; i++) {
                            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                                       "\ndv_rx_array[%0d][%0d][%0d] = %p"),
                                      dv_cont, channel, i, pktdma_p->
                                      dv_rx_array[dv_cont][channel][i]));
                        }
                    } else {
                        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit,
                                   "\ndv_rx_array[%0d][%0d][%0d] = %p"),
                                  dv_cont, channel, 0,
                                  pktdma_p->dv_rx_array[dv_cont][channel][0]));
                    }
                }
            }
        }
    }
    /* set up TX/RX dvs */
    pktdma_set_up_all_dv(unit);
}

/*
 * Function:
 *      pktdma_set_up_test
 * Purpose:
 *      Set up test for all tx/rx channels.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static void
pktdma_set_up_test(int unit)
{
    /* set up vlan */
    pktdma_set_up_vlan(unit);
    /* set up source channel for each RX channel */
    pktdma_set_source_chan(unit);
    /* set up interrupt service routing */
    pktdma_set_up_isr_table(unit);
    /* set up interrupt type */
    pktdma_config_chan_intr(unit);
    /* set up packets */
    pktdma_store_l2_packets(unit);
    /* set up descriptor vectors */
    pktdma_store_dv(unit);
}

/*
 * Function:
 *      pktdma_test_checker
 * Purpose:
 *      Test end checks.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static int
pktdma_test_checker(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];

    if (pktdma_p->bad_input == 1) {
        goto done;
    }

    cli_out("\n\nCalling pktdma_test_checker");
    cli_out("\nWait 5s for traffic to stabilize");
    sal_usleep(5000000);

    if (pktdma_p->stream_rx_param == 1 ||
        pktdma_p->stream_tx_param == 1) {
        if (pktdma_chk_rate(unit, pktdma_p->pkt_size_param,
                            pktdma_p->rate_calc_int_param,
                            pktdma_p->tx_bitmap_param,
                            pktdma_p->rx_bitmap_param) != BCM_E_NONE) {
            pktdma_p->test_fail = 1;
        }
        pktdma_chk_pkt_counters(unit);
        if (pktdma_p->poll_intr_param != POLL) {
            pktdma_chk_interrupt_counters(unit);
        }
    } else {
        pktdma_chk_pkt_counters(unit);
        if (pktdma_p->chk_pkt_integ_param == 1) {
            pktdma_chk_pkt_integrity(unit);
        }
        if (pktdma_p->poll_intr_param != POLL) {
            pktdma_chk_interrupt_counters(unit);
        }
    }

done:
    return 0;
}

/*
 * Function:
 *      pktdma_stop_tx_rx_threads
 * Purpose:
 *      Stop TX and RX threads
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
static int
pktdma_stop_tx_rx_threads(int unit)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];

    if (pktdma_p->bad_input == 1) {
        goto done;
    }

    cli_out("\nStopping TX/RX threads ...");
    /* kill TX and RX threads */
    pktdma_p->kill_dma = 1;
    sal_usleep(1000000);
    if(pktdma_p->tx_thread_done == 0) {
        test_error(unit, "\nTX thread still running");
        pktdma_p->test_fail = 1;
    }
    if(pktdma_p->rx_thread_done == 0) {
        test_error(unit, "\nRX thread still running");
        pktdma_p->test_fail = 1;
    }

    sal_thread_destroy(pktdma_p->pid_tx);
    sal_thread_destroy(pktdma_p->pid_rx);
done:
    return 0;
}

/*
 * Function:
 *      pktdma_soc_test_init
 * Purpose:
 *      Test init.Parses CLI params and allocates desc memory. Initializes most
 *      of test struct.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
int
pktdma_soc_test_init(int unit, args_t *a, void **pa)
{
    pktdma_t *pktdma_p;
    uint32 channel = 0;

    cli_out("\nStart pktdma_test");
    cli_out("\nCalling pktdma_soc_test_init\n");

    pktdma_p = pktdma_parray[unit];
    pktdma_p = sal_alloc(sizeof(pktdma_t), "pktdma_test");
    if (pktdma_p == NULL) {
        test_error(unit, "\nERROR: Memory allocation unsuccessful\n");
        return -1;
    }
    sal_memset(pktdma_p, 0, sizeof(pktdma_t));
    pktdma_parray[unit] = pktdma_p;

    /* pre-init: initialize internal variables */
    pktdma_p->test_fail = 0;
    pktdma_p->kill_dma  = 0;
    pktdma_p->tx_thread_done = 0;
    pktdma_p->rx_thread_done = 0;
    pktdma_p->num_tx_channels = 0;
    pktdma_p->num_rx_channels = 0;
    /* coverity[dont_call : FALSE] */
    pktdma_p->pkt_seed = sal_rand();

    if (soc_property_get(unit, spn_PDMA_CONTINUOUS_MODE_ENABLE, 0)) {
        cli_out("\nContinuous DMA enabled");
        pktdma_p->cont_dma = 1;
    } else {
        cli_out("\nLegacy DMA - Cont DMA not enabled");
        pktdma_p->cont_dma = 0;
    }

    for (channel = 0; channel < MAX_CHANNELS; channel++) {
        pktdma_p->chain_done_tx[channel] = 0;
        pktdma_p->chain_done_rx[channel] = 0;
        pktdma_p->reload_done_tx[channel] = 0;
        pktdma_p->reload_done_rx[channel] = 0;
        pktdma_p->chain_done_count_tx[channel] = 0;
        pktdma_p->chain_done_count_rx[channel] = 0;
        pktdma_p->reload_done_count_tx[channel] = 0;
        pktdma_p->reload_done_count_rx[channel] = 0;
        pktdma_p->tx_channel_en[channel] = 0;
        pktdma_p->rx_channel_en[channel] = 0;
    }

    /* parse parameters from CLI */
    pktdma_parse_test_params(unit, a);
    if (pktdma_p->bad_input == 1) {
        goto done;
    }

    /* post-init: initialize internal variables */
    pktdma_set_global_desc_attr(unit);

    /* set env: cleanup dma */
    if (pktdma_p->sv_override_param == 0) {
        bcm_vlan_destroy_all(unit);
        pktdma_clear_dma(unit, 0);
    }
    /* set env: turn off linkscan */
    if(pktdma_p->sw_threads_off_param == 1) {
        cli_out("\nTurning off memscan");
        soc_mem_scan_stop(unit);

        cli_out("\nPausing linkscan");
        bcm_linkscan_enable_set(unit, 0);
    }
    /* set env: turn off flow-control */
    cli_out("\nTurning off IDB/MMU FC");
    pktdma_turn_off_fc(unit);
    /* set env: turn off backgroup counter collection */
    if(pktdma_p->sw_threads_off_param == 1) {
        cli_out("\nStopping counter collection");
        soc_counter_stop(unit);
    }
    /* set env: mac loopback, cos mapping, etc. */
    if (pktdma_p->sv_override_param == 0) {
        pktdma_init_misc(unit);
    }

    if (pktdma_p->cmicx_loopback_param == 1) {
        soc_dma_attach(unit, 0);
    }

    soc_dma_init(unit);

done:
    return 0;
}

/*
 * Function:
 *      pktdma_soc_test
 * Purpose:
 *      Actual test. Kicks of TX and RX DMA threads.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
int
pktdma_soc_test(int unit, args_t *a, void *pa)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];

    if (pktdma_p->bad_input == 1) {
        goto done;
    }

    cli_out("\nCalling pktdma_soc_test");
    pktdma_set_up_test(unit);

    if (pktdma_p->verbose_param == 1) {
        pktdma_dump_first_dv(unit);
        pktdma_dump_pkt_pointers(unit);
    }

    pktdma_p->pid_tx =
        sal_thread_create("TXDMA thread", 16 * 1024 * 1024, 200,
                          pktdma_txdma_thread, INT_TO_PTR(unit));
    pktdma_p->pid_rx =
        sal_thread_create("RXDMA thread", 16 * 1024 * 1024, 200,
                          pktdma_rxdma_thread, INT_TO_PTR(unit));

    cli_out("\npid_tx = %p, pid_rx= %p", (pktdma_p->pid_tx),
                                         (pktdma_p->pid_rx));

    pktdma_test_checker(unit);
done:
    return 0;
}

/*
 * Function:
 *      pktdma_soc_test_cleanup
 * Purpose:
 *      Test cleanup. Called at test end. Frees all allocated memory.
 * Parameters:
 *      unit: StrataSwitch Unit #.
 *
 * Returns:
 *     Nothing
 */
int
pktdma_soc_test_cleanup(int unit, void *pa)
{
    pktdma_t *pktdma_p = pktdma_parray[unit];
    soc_control_t *soc = SOC_CONTROL(unit);
    uint32 vchan;
    sdc_t *sc;

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    uint32 alloc_curr = 0;
    uint32 alloc_max = 0;
#endif
#endif

    if (pktdma_p->bad_input == 1) {
        goto done;
    }

    cli_out("\nCalling pktdma_soc_test_cleanup");

    /* kill TX/RX threads */
    pktdma_stop_tx_rx_threads(unit);

    /* destroy all VLANs */
    bcm_vlan_destroy_all(unit);
    if (pktdma_p->poll_intr_param != 0) {
        pktdma_dump_interrupt_counts(unit);
    }

    /* cleanup DMA operations */
    if (pktdma_p->cont_dma == 0) {
        soc_dma_abort(unit);
    } else {
        for (vchan = 0; vchan < MAX_CHANNELS; vchan++) {
            sc = &soc->soc_channels[vchan];
            while (sc->sc_q != NULL) {
                sc->sc_q->dv_channel = -sc->sc_q->dv_channel;
                sc->sc_q = sc->sc_q->dv_next;
                sc->sc_q_cnt--;
            }
            sc->sc_dv_active = NULL;
            sc->sc_q_tail   = NULL;
            sc->sc_q = NULL;
            sc->sc_q_cnt = 0;
        }
        (void)soc_reset_init(unit);
    }

    /* free test allocated memories */
    pktdma_free_all_memory(unit);

done:
    sal_free(pktdma_p);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
    sal_alloc_resource_usage_get(&alloc_curr, &alloc_max);
    cli_out("\nalloc_curr = %0d, alloc_max = %0d", alloc_curr, alloc_max);
#endif
#endif

    cli_out("\n\n");
    return 0;
}
#endif /* BCM_ESW_SUPPORT && BCM_CMICM_SUPPORT */
