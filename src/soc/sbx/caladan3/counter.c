/*
 * $Id:  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    counter.c
 * Purpose: Caladan3 Contolled counter 
 * Requires:
 */

#include <soc/types.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/counter.h>
#include <soc/sbx/caladan3_counter.h>

/* Using controlled counter mechanism */

static uint32 cmic_tx_cnt[3];
static uint32 cmic_rx_cnt[3];

extern int
soc_sbx_caladan3_il_controlled_counter_get(int unit, int ctr_id, 
                                           int port, uint64 *counter);

int
soc_sbx_caladan3_controlled_counter_get(int unit, int ctr_id, int port, uint64 *counter);

soc_controlled_counter_t soc_caladan3_countrolled_counters[] = {

    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_TX_PKT_CNT,
        "Tx Packet Count",
        "TPKT",
        _SOC_CONTROLLED_COUNTER_FLAG_TX |  
           _SOC_CONTROLLED_COUNTER_FLAG_MAC | 
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        0
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_TX_BYTE_CNT,
        "Tx Byte Count",
        "TBYT",
        _SOC_CONTROLLED_COUNTER_FLAG_TX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        1
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_TX_BAD_PKT_CNT,
        "Packets with errors",
        "TERR",
        _SOC_CONTROLLED_COUNTER_FLAG_TX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        2
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_TX_GT_MTU_CNT,
        "Packets greater than MTU",
        "TGMTU",
        _SOC_CONTROLLED_COUNTER_FLAG_TX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        3
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_TX_EQ_MTU_CNT,
        "Packets equal to MTU",
        "TEMTU",
        _SOC_CONTROLLED_COUNTER_FLAG_TX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        4
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_PKT_CNT,
        "Rx Packet counter",
        "RPKT",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        5
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_BYTE_CNT,
        "Rx Byte counter ",
        "RBYT",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        6
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_BAD_PKT_CNT,
        "Packets with errors",
        "RERR",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        7
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_GT_MTU_CNT,
        "Packets greater than MTU",
        "RGMTU",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        8
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_EQ_MTU_CNT,
        "Packets equal to MTU",
        "REMTU",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        9
    },
    {
        soc_sbx_caladan3_controlled_counter_get,
        SOC_SBX_CALADAN3_RX_CRCERR_CNT,
        "Packets with CRC errors",
        "RCRC",
        _SOC_CONTROLLED_COUNTER_FLAG_RX |
            _SOC_CONTROLLED_COUNTER_FLAG_MAC |
              _SOC_CONTROLLED_COUNTER_FLAG_HIGH,
        10
    },
    {
         NULL,
         SOC_SBX_CALADAN3_LAST_CNT, 
         NULL, 
         NULL,
         0,
         0
    }
};

int
soc_sbx_caladan3_controlled_counters_num_get()
{
    return  (sizeof(soc_caladan3_countrolled_counters)/
               sizeof(soc_controlled_counter_t)) - 1;
}

soc_controlled_counter_t *
soc_sbx_caladan3_controlled_counters_get(int unit, int index) {
    if ((index >=0) && (index < soc_sbx_caladan3_controlled_counters_num_get())) {
        return &soc_caladan3_countrolled_counters[index];
    } else {
        return NULL;
    }
}

int
soc_sbx_caladan3_is_controlled_counter(int unit, char *str, int *ctrid)
{
    int i;
    soc_controlled_counter_t *ctr;

    for (i=0; i < 
               soc_sbx_caladan3_controlled_counters_num_get(); i++) {
        ctr = &soc_caladan3_countrolled_counters[i];
        if (ctr && sal_strcmp(str, ctr->short_cname) == 0) {
            if (ctrid) {
                    *ctrid = ctr->counter_id;
            }
            return TRUE;
        }
    }
    return FALSE;
}


int
soc_sbx_caladan3_cmic_controlled_counter_get(int unit, int ctr_id, int port, uint64 *counter)
{
    uint32 data = 0;
    uint32 count = 0;
    uint32 tmptx = 0, tmprx = 0;
    int cmc;
    /*
     * Note:
     * These registers are not CoR, so we fake counts
     */
    switch (ctr_id) {
        case SOC_SBX_CALADAN3_TX_PKT_CNT:
        for (cmc=0; cmc < 3; cmc++) {
            data = soc_pci_read(unit, CMIC_CMCx_PKT_COUNT_TXPKT_OFFSET(cmc));
            count = soc_reg_field_get(unit, CMIC_CMC0_PKT_COUNT_TXPKTr, data, COUNTf);
            if (count == cmic_tx_cnt[cmc]) {
                count =  0;
            } else if (count < cmic_tx_cnt[cmc]) {
                /* wrap ? */
                tmptx = (0xffffffff - cmic_tx_cnt[cmc] + 1);
                COMPILER_64_ADD_32(*counter, tmptx);
                COMPILER_64_ADD_32(*counter, count);
            } else {
                tmptx = count - cmic_tx_cnt[cmc];
                COMPILER_64_ADD_32(*counter, tmptx);
                cmic_tx_cnt[cmc] = count;
            }
        }
        break;
    case SOC_SBX_CALADAN3_RX_PKT_CNT:
        for (cmc=0; cmc < 3; cmc++) {
            data = soc_pci_read(unit, CMIC_CMCx_PKT_COUNT_RXPKT_OFFSET(cmc));
            count = soc_reg_field_get(unit, CMIC_CMC0_PKT_COUNT_RXPKTr, data, COUNTf);
            if (count == cmic_rx_cnt[cmc]) {
                count =  0;
            } else if (count < cmic_rx_cnt[cmc]) {
                /* wrap ? */
                tmprx = (0xffffffff - cmic_rx_cnt[cmc] + 1);
                COMPILER_64_ADD_32(*counter, tmprx);
                COMPILER_64_ADD_32(*counter, count);
            } else {
                tmprx = count - cmic_rx_cnt[cmc];
                COMPILER_64_ADD_32(*counter, tmprx);
                cmic_rx_cnt[cmc] = count;
            }
        }
        break;
    default:
        ;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_controlled_counter_get(int unit, int ctr_id, int port, uint64 *counter)
{
    COMPILER_64_ZERO(*counter);
    if (IS_CPU_PORT(unit, port)) {
        return soc_sbx_caladan3_cmic_controlled_counter_get(unit, ctr_id, port, counter);
    } else if (IS_IL_PORT(unit, port)) {
        return soc_sbx_caladan3_il_controlled_counter_get(unit, ctr_id, port, counter);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_controlled_counter_init(int unit)
{
    pbmp_t pbmp;
    int i, port;
    soc_controlled_counter_t *ctr;
    uint64 dummy[SOC_SBX_CALADAN3_IL_NUM_CHANNEL_STATS];

    if (SOC_CONTROL(unit)->controlled_counters == NULL) {
        SOC_CONTROL(unit)->controlled_counters = soc_caladan3_countrolled_counters;
        SOC_CONTROL(unit)->soc_controlled_counter_all_num =
            soc_sbx_caladan3_controlled_counters_num_get();
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_OR(pbmp, PBMP_IL_ALL(unit));
        SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
        SOC_PBMP_ITER(pbmp, port) {
            for (i=0; i < SOC_CONTROL(unit)->soc_controlled_counter_all_num; i++) {
                ctr = &soc_caladan3_countrolled_counters[i];
                if (ctr->controlled_counter_f) {
                    ctr->controlled_counter_f(unit, ctr->counter_id, port, &dummy[0]);
                }
            }
        }
    }
    return SOC_E_NONE;
}

#endif
