/*
 * $Id: ilmac.c,v 1.1.2.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Caladan3 ILMAC Driver
 */

#ifdef BCM_CALADAN3_SUPPORT

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/spl.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/portmode.h>
#include <soc/ll.h>
#include <soc/counter.h>
#include <soc/phyctrl.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3_counter.h>
#include <soc/mem.h>

#define JUMBO_MAXSZ              0x3fe8 /* Max legal value (per regsfile) */

#define SOC_SBX_CALADAN3_IL_BURST_SHORT_64B   (1) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_96B   (2) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_128B  (3) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_160B  (4) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_192B  (5) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_224B  (6) 
#define SOC_SBX_CALADAN3_IL_BURST_SHORT_256B  (7) 

#define SOC_SBX_CALADAN3_IL_BURST_MAX_64B   (0) 
#define SOC_SBX_CALADAN3_IL_BURST_MAX_128B  (1) 
#define SOC_SBX_CALADAN3_IL_BURST_MAX_192B  (2) 
#define SOC_SBX_CALADAN3_IL_BURST_MAX_256B  (3) 

#define SOC_SBX_CALADAN3_IL_LOOPBACK_NONE (0)
#define SOC_SBX_CALADAN3_IL_LOOPBACK_L1   (1)
#define SOC_SBX_CALADAN3_IL_LOOPBACK_RMT  (2)
#define SOC_SBX_CALADAN3_IL_LOOPBACK_L2   (3)
#define SOC_SBX_CALADAN3_IL_LOOPBACK_FC   (4)

#define SOC_SBX_CALADAN3_IL_LANES_MAX     (12)
#define SOC_SBX_CALADAN3_IL_CHANNELS_MAX  (64) /* 256 supported but enables only for 64 */ 

/* Operating modes */
#define SOC_SBX_CALADAN3_IL_50G_N   (0)    
#define SOC_SBX_CALADAN3_IL_50G_W   (1)
#define SOC_SBX_CALADAN3_IL_100G    (2)

/* Max lanes */
#define SOC_SBX_CALADAN3_IL_50G_N_LANE_MAX  (6) 
#define SOC_SBX_CALADAN3_IL_50G_W_LANE_MAX  (10)    
#define SOC_SBX_CALADAN3_IL_100G_LANE_MAX   (12)

/* Link Level FC */
#define SOC_SBX_CALADAN3_IL_LINK_FC_DISABLED  (0) 
#define SOC_SBX_CALADAN3_IL_LINK_FC_ON_64     (1)    
#define SOC_SBX_CALADAN3_IL_LINK_FC_ON_32     (2)    

/* FC mode */
#define SOC_SBX_CALADAN3_IL_FC_INBAND   (0) 
#define SOC_SBX_CALADAN3_IL_FC_OOB      (1)    

/* Meta frame len  */
#define SOC_SBX_CALADAN3_IL_METAFRAME_MIN      (64)
#define SOC_SBX_CALADAN3_IL_METAFRAME_DEFAULT  (2048)
#define SOC_SBX_CALADAN3_IL_METAFRAME_MAX      (16*1024)

#define SOC_SBX_CALADAN3_ILKN_MAX 2

#define IL_INSTANCE(inst) ((SOC_REG_ADDR_INSTANCE_MASK)|(inst))


int
soc_sbx_caladan3_il_interface_status_override_set(int unit, int ifnum,
                                                  int intf_status, int intf_status_ov,
                                                  int lane_status, int lane_status_ov,
                                                  int tx_status, int tx_status_ov);

/* current config */
typedef struct {
    int num_lanes;
    int num_channels;
    int fc_type;
    int oob_status_ignore;
    int mtu;
    uint32 *tx_stat_mem;     /* Memories are COR so we need to buffer last seen data */
    uint32 *rx0_stat_mem;
    uint32 *rx1_stat_mem;
    int tx_stat_mem_size;
    int rx0_stat_mem_size;
    int rx1_stat_mem_size;
    int counters_available;  /* A0 counter read is not reliable, fixed in A1 */
} soc_c3_ilkn_cfg_t;

soc_c3_ilkn_cfg_t  ilkn_config[SOC_MAX_NUM_DEVICES][SOC_SBX_CALADAN3_ILKN_MAX];


int
soc_sbx_caladan3_il_force_resync(int unit, int ifnum)
{
    uint32 data0 = 0;
    ifnum = IL_INSTANCE(ifnum);

    soc_reg_field_set(unit, IL_RX_CORE_CONTROL0r, &data0,
                      CTL_RX_FORCE_RESYNC_PULSEf, 1);
    soc_reg32_set(unit, IL_RX_CORE_CONTROL0r, ifnum, 0, data0);
    soc_reg_field_set(unit, IL_RX_CORE_CONTROL0r, &data0,
                      CTL_RX_FORCE_RESYNC_PULSEf, 0);
    soc_reg32_set(unit, IL_RX_CORE_CONTROL0r, ifnum, 0, data0);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_get_lane_status(int unit, int ifnum, int *status)
{
    uint32 data0 = 0;
    ifnum = IL_INSTANCE(ifnum);
    if (status) {
        soc_reg32_get(unit, IL_RX_CORE_STATUS1r, ifnum, 0, &data0);
        *status = soc_reg_field_get(unit, 
                      IL_RX_CORE_STATUS1r, data0, STAT_RX_SYNCEDf);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_get_lanes(int unit, int ifnum, int *nlanes)
{
    uint32 data = 0;
    ifnum = IL_INSTANCE(ifnum);
    soc_reg32_get(unit, IL_RX_CORE_CONFIG1r, ifnum, 0, &data);
    if (nlanes) {
        *nlanes = soc_reg_field_get(unit, IL_RX_CORE_CONFIG1r, data,
                                   CTL_RX_LAST_LANEf);
        if (*nlanes > 0) (*nlanes)++;
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_set_lanes(int unit, int ifnum, int nlanes)
{
    uint32 data0 = 0;
    soc_reg32_get(unit, IL_TX_CORE_CONFIG2r, IL_INSTANCE(ifnum), 0, &data0);
    nlanes -= 1;
    soc_reg_field_set(unit, IL_TX_CORE_CONFIG2r, &data0,
                      CTL_TX_LAST_LANEf, nlanes);
    soc_reg32_set(unit, IL_TX_CORE_CONFIG2r, IL_INSTANCE(ifnum), 0, data0);
    soc_reg32_get(unit, IL_RX_CORE_CONFIG1r, IL_INSTANCE(ifnum), 0, &data0);
    soc_reg_field_set(unit, IL_RX_CORE_CONFIG1r, &data0,
                      CTL_RX_LAST_LANEf, nlanes);
    soc_reg32_set(unit, IL_RX_CORE_CONFIG1r, IL_INSTANCE(ifnum), 0, data0);
    ilkn_config[unit][ifnum].num_lanes = nlanes;
    return SOC_E_NONE;
}

int soc_sbx_caladan3_il_clear(int unit, int ifnum)
{
    ifnum = IL_INSTANCE(ifnum);
    /* Clear Err Status */
    soc_reg32_set(unit, IL_RX_ERRDET0_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_RX_ERRDET1_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_RX_ERRDET2_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_RX_ERRDET3_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_RX_ERRDET4_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_RX_ERRDET5_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_TX_ERRDET0_L2_INTRr, ifnum, 0, 0xffffffff);

    /* Clear FC Status */
    soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_STS0r, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_STS1r, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_STS0r, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_STS1r, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_FLOWCONTROL_L2_INTRr, ifnum, 0, 0xffffffff);
    soc_reg32_set(unit, IL_HCFC_INTRr, ifnum, 0, 1);
    return SOC_E_NONE; 
}

int 
soc_sbx_caladan3_il_flow_control_mode_get(int unit, int ifnum, int *mode)
{
    uint32 data = 0;
    ifnum = IL_INSTANCE(ifnum);
    if (!mode) {
        return SOC_E_PARAM;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_GLOBAL_CONFIGr, ifnum, 0, &data));
    *mode = soc_reg_field_get(unit, IL_GLOBAL_CONFIGr, data, CFG_FLOWCONTROL_TYPEf);
    if (*mode) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_TX_CONFIG0r,  ifnum, 0, &data));
        if (soc_reg_field_get(unit, IL_HCFC_TX_CONFIG0r, data, HCFC_TX_ENABLEf)) {
            *mode = SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB;
        } else {
            *mode = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
        }
    } else {
        *mode = SOC_SBX_CALADAN3_FC_TYPE_ILKN;
    }
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_il_flow_control_calendar_compression_set(int unit, int ifnum, int mode,
                                                          int tx_fc_len, int rx_fc_len)
{
    int tx_fc = 0;
    int rx_fc = 0;
    uint32 data = 0;

    ifnum = IL_INSTANCE(ifnum);

    /* TX FC Compression:    
     *  64/32 bits carrying info for 64/32 queues  => 0  
     *  16 bits carrying info for queues(0-7,32-39)=> 1    
     *   8 bits carrying summary of 8 queues each  => 2
     *   4 bits carrying summary of 8 queues each  => 3
     * RX FC Expansion
     *   64 bits to 64 queues => 0
     *   8 bits to 64 queues  => 1
     */

    switch (rx_fc_len) {
    case 8:
    case 4:
        rx_fc = 1;
        break;
    default:
        rx_fc = 0;
    }
    switch (tx_fc_len) {
    case 16:
        tx_fc = 1;
        break;
    case 8:
        tx_fc = 2;
        break;
    case 4:
        tx_fc = 3;
        break;
    case 64:
    case 32:
    default:
        tx_fc = 0;
        break;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_GLOBAL_CONFIGr, ifnum, 0, &data));
    soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, CFG_TX_CHANNEL_MODEf, tx_fc);
    soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, CFG_RX_CHANNEL_MODEf, rx_fc);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_GLOBAL_CONFIGr, ifnum, 0, data));

    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_il_flow_control_calendar_set(int unit, int ifnum, int mode, int fc_calendar_length)
{
    uint32 data = 0;
    int  ctl_tx_fc_callen = 0;

    ifnum = IL_INSTANCE(ifnum);

    /* Program the calendar length */
    if (mode == SOC_SBX_CALADAN3_FC_TYPE_ILKN) {
        switch (fc_calendar_length)
        {
        case 16:
            ctl_tx_fc_callen = 0;
            break;
        case 32:
            ctl_tx_fc_callen = 1;
            break;
        case 64:
            ctl_tx_fc_callen = 3;
            break;
        case 128:
            ctl_tx_fc_callen = 7;
            break;
        case 256:
            ctl_tx_fc_callen = 0xf;
            break;
        case 0: /* if set to 0, set calendar size to 3 by default */
        default:
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Invalid ilkn fc_calendar_length(%d) setting to 64\n"), fc_calendar_length));
            ctl_tx_fc_callen = 3;
            break;
        }
        /* Inband Calendar */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_TX_CORE_CONFIG0r, ifnum, 0, &data));
        soc_reg_field_set(unit, IL_TX_CORE_CONFIG0r, &data, CTL_TX_FC_CALLENf, ctl_tx_fc_callen);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CORE_CONFIG0r, ifnum, 0, data));

    } else {

        /* upto 64 is valid in out of band case */
        if ((fc_calendar_length < 0) || (fc_calendar_length > 64)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Invalid ilkn fc_calendar_length(%d) setting to 64\n"),
                      fc_calendar_length));
            ctl_tx_fc_callen = 63;
        } else {
            ctl_tx_fc_callen = fc_calendar_length -1;
        }
        /* OOB Calendar */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_TX_CORE_CONFIG2r, ifnum, 0, &data));
        soc_reg_field_set(unit, IL_TX_CORE_CONFIG2r, &data, 
                          CTL_TX_CALLEN_MINUS1f, ctl_tx_fc_callen);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CORE_CONFIG2r, ifnum, 0, data));
    }

    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_flow_control_loopback_enable(int unit, int ifnum, int mode, int en)
{
    uint32 loop = 0;

    ifnum = IL_INSTANCE(ifnum);
    if (mode == SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_DEBUGr, ifnum, 0, &loop));
        soc_reg_field_set(unit, IL_HCFC_DEBUGr, &loop, 
            HCFC_LOOPBACK_ENf, en);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_HCFC_DEBUGr, ifnum, 0, loop));
        return SOC_E_NONE;
    } else {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_LOOPBACK_CONFIGr, ifnum, 0, &loop));
        soc_reg_field_set(unit, IL_LOOPBACK_CONFIGr, &loop, 
            IL_LOOPBACK_FC_ENABLEf, en);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_LOOPBACK_CONFIGr, ifnum, 0, loop));
    }
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_il_flow_control_mode_set(int unit, int ifnum, int mode)
{
    uint32 data = 0;
    uint32 hcfc = 0;

    ifnum = IL_INSTANCE(ifnum);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_GLOBAL_CONFIGr, ifnum, 0, &data));

    if (mode == SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB) {
        soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data,
                          CFG_FLOWCONTROL_TYPEf, SOC_SBX_CALADAN3_IL_FC_OOB);
        /* Enable HCFC */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_TX_CONFIG0r, ifnum, 0, &hcfc));
        soc_reg_field_set(unit, IL_HCFC_TX_CONFIG0r, &hcfc, HCFC_TX_ENABLEf, 1);
        soc_reg_field_set(unit, IL_HCFC_TX_CONFIG0r, &hcfc, HCFC_TX_CHANNEL_BASE_MINf, 0);
        soc_reg_field_set(unit, IL_HCFC_TX_CONFIG0r, &hcfc, HCFC_TX_CHANNEL_BASE_MAXf, 0);
        soc_reg_field_set(unit, IL_HCFC_TX_CONFIG0r, &hcfc, HCFC_TX_BITS_PER_CHANNELf, 0);
        soc_reg32_set(unit, IL_HCFC_TX_CONFIG0r, ifnum, 0, hcfc);
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_RX_CONFIG0r,  ifnum, 0, &hcfc));
        soc_reg_field_set(unit, IL_HCFC_RX_CONFIG0r, &hcfc, HCFC_RX_ENABLEf, 1);
        soc_reg_field_set(unit, IL_HCFC_RX_CONFIG0r, &hcfc, HCFC_RX_CHANNEL_BASE_MINf, 0);
        soc_reg_field_set(unit, IL_HCFC_RX_CONFIG0r, &hcfc, HCFC_RX_CHANNEL_BASE_MAXf, 0);
        soc_reg_field_set(unit, IL_HCFC_RX_CONFIG0r, &hcfc, HCFC_RX_BITS_PER_CHANNELf, 0);
        soc_reg32_set(unit, IL_HCFC_RX_CONFIG0r, ifnum, 0, hcfc);

    } else if (mode == SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB) {

        soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data,
                          CFG_FLOWCONTROL_TYPEf, SOC_SBX_CALADAN3_IL_FC_OOB);
        /* OOB HCFC shares the same pins with OOB interlaken FC. They are mutually
           exclusive. Disable HCFC */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_TX_CONFIG0r, ifnum, 0, &hcfc));
        soc_reg_field_set(unit, IL_HCFC_TX_CONFIG0r, &hcfc, HCFC_TX_ENABLEf, 0);
        soc_reg32_set(unit, IL_HCFC_TX_CONFIG0r, ifnum, 0, hcfc);
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_RX_CONFIG0r,  ifnum, 0, &hcfc));
        soc_reg_field_set(unit, IL_HCFC_RX_CONFIG0r, &hcfc, HCFC_RX_ENABLEf, 0);
        soc_reg32_set(unit, IL_HCFC_RX_CONFIG0r, ifnum, 0, hcfc);

    } else {

        /* Default inband */
        soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, 
                          CFG_FLOWCONTROL_TYPEf, SOC_SBX_CALADAN3_IL_FC_INBAND); 
    }
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_GLOBAL_CONFIGr, ifnum, 0, data));

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_il_oob_init
 * Purpose
 *     Set up HCFC OOB/ILKN OOB 
 */
int 
soc_sbx_caladan3_il_oob_init(int unit, int ifnum, int fc_type)
{
    int loop;
    int fc_calendar_length;
    uint32 data = 0;


    /* Not required to setup interlaken interface fully, 
     * enable only hcfc oob/ilkn oob part 
     */
    soc_sbx_caladan3_il_flow_control_mode_set(unit, ifnum, fc_type);

    /* Tx Disable, Override Lane and Interface status to indicate all ok */
    soc_sbx_caladan3_il_interface_status_override_set(unit, ifnum,
                                                      1, /* Intf override value*/
                                                      1, /*  Overide set */
                                                      0xfff, /* lane status value */
                                                      1, /* override set */
                                                      0, /* Tx disable */
                                                      0  /* based on sv */);

    /* Update calendar len */
    if (ifnum == 0) {
       fc_calendar_length = 
           soc_property_get(unit, "fc_calendar_length_il_line", 64);
    } else {
       fc_calendar_length = 
           soc_property_get(unit, "fc_calendar_length_il_fabric", 64);
    }
    soc_sbx_caladan3_il_flow_control_calendar_set(unit, ifnum, fc_type, fc_calendar_length);


    soc_reg32_get(unit, IL_GLOBAL_CONTROLr, IL_INSTANCE(ifnum), 0, &data);
    soc_reg_field_set(unit, IL_GLOBAL_CONTROLr, &data, SOFT_RESETf, 0);
    soc_reg32_set(unit, IL_GLOBAL_CONTROLr, IL_INSTANCE(ifnum), 0, data);

    
    loop = soc_property_port_get(unit, ifnum, "fc_oob_loopback", 0);
    if (loop) {
        soc_sbx_caladan3_il_flow_control_loopback_enable(unit, ifnum, fc_type, loop);
    }

    return SOC_E_NONE;
}

/*
 * function:
 *     soc_sbx_caladan3_il_oob_hcfc_remap_default
 * purpose:
 *     Setup HCFC remap table one to one mapping
 *     Returns SOC_E_NONE
 */
int
soc_sbx_caladan3_il_oob_hcfc_remap_default(int unit, int ifnum, int dir)
{
    int idx, channel;
    uint32 txr = 0, rxr = 0;
    soc_reg_t txregs[]= {
        IL_HCFC_TX_REMAP0r, IL_HCFC_TX_REMAP1r, IL_HCFC_TX_REMAP2r,
        IL_HCFC_TX_REMAP3r, IL_HCFC_TX_REMAP4r, IL_HCFC_TX_REMAP5r,
        IL_HCFC_TX_REMAP6r, IL_HCFC_TX_REMAP7r, IL_HCFC_TX_REMAP8r,
        IL_HCFC_TX_REMAP9r, IL_HCFC_TX_REMAP10r, IL_HCFC_TX_REMAP11r,
        IL_HCFC_TX_REMAP12r
    };
    soc_reg_t rxregs[]= {
        IL_HCFC_RX_REMAP0r, IL_HCFC_RX_REMAP1r, IL_HCFC_RX_REMAP2r,
        IL_HCFC_RX_REMAP3r, IL_HCFC_RX_REMAP4r, IL_HCFC_RX_REMAP5r,
        IL_HCFC_RX_REMAP6r, IL_HCFC_RX_REMAP7r, IL_HCFC_RX_REMAP8r,
        IL_HCFC_RX_REMAP9r, IL_HCFC_RX_REMAP10r, IL_HCFC_RX_REMAP11r,
        IL_HCFC_RX_REMAP12r
    };

    for (channel = 0; channel < 64; channel++) {
        idx = channel / 5;
        if (dir & 1) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, txregs[idx],
                                              IL_INSTANCE(ifnum), 0, &txr));
            txr &= ~(0x3f << (channel % 5) * 6);
            txr |= (channel & 0x3f) << ((channel % 5) * 6);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, txregs[idx],
                                            IL_INSTANCE(ifnum), 0, txr));
        } 
        if (dir & 2) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, rxregs[idx],
                                            IL_INSTANCE(ifnum), 0, &rxr));
            rxr &= ~(0x3f << (channel % 5) * 6);
            rxr |= (channel & 0x3f) << ((channel % 5) * 6);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, rxregs[idx],
                                            IL_INSTANCE(ifnum), 0, rxr));
        }
    }
    return SOC_E_NONE;
}

/*
 * function:
 *     soc_sbx_caladan3_il_oob_hcfc_remap_get
 * purpose:
 *     Setup HCFC remap table
 *     direction = 1 -> Tx, 2 -> Rx
 *     Specify the queue and get the channel_id
 *     Returns SOC_E_NONE on success
 */
int
soc_sbx_caladan3_il_oob_hcfc_remap_get(int unit, int ifnum, int queue, int dir, int *channel)
{
    int idx, ch, found;
    uint32 txr = 0, rxr = 0;
    soc_reg_t txregs[]= {
        IL_HCFC_TX_REMAP0r, IL_HCFC_TX_REMAP1r, IL_HCFC_TX_REMAP2r,
        IL_HCFC_TX_REMAP3r, IL_HCFC_TX_REMAP4r, IL_HCFC_TX_REMAP5r,
        IL_HCFC_TX_REMAP6r, IL_HCFC_TX_REMAP7r, IL_HCFC_TX_REMAP8r,
        IL_HCFC_TX_REMAP9r, IL_HCFC_TX_REMAP10r, IL_HCFC_TX_REMAP11r,
        IL_HCFC_TX_REMAP12r
    };
    soc_reg_t rxregs[]= {
        IL_HCFC_RX_REMAP0r, IL_HCFC_RX_REMAP1r, IL_HCFC_RX_REMAP2r,
        IL_HCFC_RX_REMAP3r, IL_HCFC_RX_REMAP4r, IL_HCFC_RX_REMAP5r,
        IL_HCFC_RX_REMAP6r, IL_HCFC_RX_REMAP7r, IL_HCFC_RX_REMAP8r,
        IL_HCFC_RX_REMAP9r, IL_HCFC_RX_REMAP10r, IL_HCFC_RX_REMAP11r,
        IL_HCFC_RX_REMAP12r
    };

    if ((channel == NULL) || ((dir != 1) && (dir != 2))) {
        return (SOC_E_PARAM);
    }
    for (ch = 0, found = 0, idx = 0;
            (!found && (idx < sizeof(txregs)/sizeof(txregs[0])));
                 idx++) {

        if (dir & 0x2) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, rxregs[idx],
                                              IL_INSTANCE(ifnum), 0, &rxr));
            while(rxr != 0) {
                if (queue == (rxr & 0x3f)) {
                    found = TRUE;
                    break;
                }
                ch++;
                rxr >>= 6;
            }
        } else if (dir & 0x1) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, txregs[idx],
                                              IL_INSTANCE(ifnum), 0, &txr));
            while(txr != 0) {
                if (queue == (txr & 0x3f)) {
                    found = TRUE;
                    break;
                }
                ch++;
                txr >>= 6;
            }
        }
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Unit %d, oob_hcfc_remap_get Dir(%s) Queue(%x,%d) Channel(%d)\n"),
                 unit, (dir & 1) ? "tx" : "rx",
                 queue, queue, ch));
    if (found) {
        *channel = ch;
        return SOC_E_NONE;
    } else {
        *channel = -1;
        return SOC_E_NOT_FOUND;
    }
}

/*
 * function:
 *     soc_sbx_caladan3_il_oob_hcfc_remap_set
 * purpose:
 *     Setup HCFC remap table
 *     direction = 1 -> Tx, 2 -> Rx, 3 -> Both
 *     Specify the channel and the queue
 *     Returns SOC_E_NONE on success
 */
int
soc_sbx_caladan3_il_oob_hcfc_remap_set(int unit, int ifnum, int queue, int dir, int channel)
{
    int idx;
    uint32 txr = 0, rxr = 0;
    soc_reg_t txregs[]= {
        IL_HCFC_TX_REMAP0r, IL_HCFC_TX_REMAP1r, IL_HCFC_TX_REMAP2r,
        IL_HCFC_TX_REMAP3r, IL_HCFC_TX_REMAP4r, IL_HCFC_TX_REMAP5r,
        IL_HCFC_TX_REMAP6r, IL_HCFC_TX_REMAP7r, IL_HCFC_TX_REMAP8r,
        IL_HCFC_TX_REMAP9r, IL_HCFC_TX_REMAP10r, IL_HCFC_TX_REMAP11r,
        IL_HCFC_TX_REMAP12r
    };
    soc_reg_t rxregs[]= {
        IL_HCFC_RX_REMAP0r, IL_HCFC_RX_REMAP1r, IL_HCFC_RX_REMAP2r,
        IL_HCFC_RX_REMAP3r, IL_HCFC_RX_REMAP4r, IL_HCFC_RX_REMAP5r,
        IL_HCFC_RX_REMAP6r, IL_HCFC_RX_REMAP7r, IL_HCFC_RX_REMAP8r,
        IL_HCFC_RX_REMAP9r, IL_HCFC_RX_REMAP10r, IL_HCFC_RX_REMAP11r,
        IL_HCFC_RX_REMAP12r
    };

    if ((channel < 0) || (channel > 63)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, oob_hcfc_remap invalid channel(%d)\n"),
                   unit, channel));
        return (SOC_E_PARAM);
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d, oob_hcfc_%s_remap Queue(%x,%d) Channel(%d)\n"),
                     unit, ((dir & 3) ? "tx_rx" : ((dir & 1) ? "tx" : "rx")),
                     queue, queue, channel));
    }
    idx = channel / 5;
    if (dir & 0x2) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, rxregs[idx],
                                        IL_INSTANCE(ifnum), 0, &rxr));
        rxr &= ~(0x3f << (channel % 5) * 6);
        rxr |= (queue & 0x3f) << ((channel % 5) * 6);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, rxregs[idx],
                                        IL_INSTANCE(ifnum), 0, rxr));
    }
    if (dir & 0x1) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, txregs[idx],
                                        IL_INSTANCE(ifnum), 0, &txr));
        txr &= ~(0x3f << (channel % 5) * 6);
        txr |= (queue & 0x3f) << ((channel % 5) * 6);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, txregs[idx],
                                        IL_INSTANCE(ifnum), 0, txr));
    }

   return SOC_E_NONE;
}

int 
soc_sbx_caladan3_il_enable_channels(int unit, int ifnum, int ch_31_0, int ch_63_32)
{
    ifnum = IL_INSTANCE(ifnum);
    if (ch_31_0) {
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_RX_CHAN_ENABLE0r, ifnum, 0, ch_31_0));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CHAN_ENABLE0r, ifnum, 0, ch_31_0));
    } 
    if (ch_63_32) {
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_RX_CHAN_ENABLE1r, ifnum, 0, ch_63_32));
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CHAN_ENABLE1r, ifnum, 0, ch_63_32));
    }
    return SOC_E_NONE;
}


int soc_sbx_caladan3_il_set_mtu(int unit, int ifnum, int mtu )
{
    uint32 data = 0;
    if ((mtu < 0) || (mtu > JUMBO_MAXSZ)) {
        return SOC_E_PARAM;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_RX_CONFIGr, IL_INSTANCE(ifnum), 0, &data));
    soc_reg_field_set(unit, IL_RX_CONFIGr, &data, IL_RX_MAX_PACKET_SIZEf, mtu); 
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_RX_CONFIGr, IL_INSTANCE(ifnum), 0, data));

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_TX_CONFIGr, IL_INSTANCE(ifnum), 0, &data));
    soc_reg_field_set(unit, IL_TX_CONFIGr, &data, IL_TX_MAX_PACKET_SIZEf, mtu);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CONFIGr, IL_INSTANCE(ifnum), 0, data));

    ilkn_config[unit][ifnum].mtu = mtu;
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_loopback_set(int unit, int port, int lb) 
{
    int ifnum;
    uint32 loop = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    ifnum = IL_INSTANCE(ifnum);
    soc_reg32_get(unit, IL_LOOPBACK_CONFIGr, ifnum, 0, &loop);
    if (lb == SOC_SBX_CALADAN3_IL_LOOPBACK_NONE) {
        loop = 0;
    } else if (lb == SOC_SBX_CALADAN3_IL_LOOPBACK_L1){ 
        soc_reg_field_set(unit, IL_LOOPBACK_CONFIGr, &loop, IL_LOOPBACK_L1_ENABLEf, 1);
    } else if (lb == SOC_SBX_CALADAN3_IL_LOOPBACK_RMT) {
        soc_reg_field_set(unit, IL_LOOPBACK_CONFIGr, &loop, IL_LOOPBACK_R1_ENABLEf, 1);
    } else if (lb == SOC_SBX_CALADAN3_IL_LOOPBACK_L2) {
        soc_reg_field_set(unit, IL_LOOPBACK_CONFIGr, &loop, IL_LOOPBACK_L2_ENABLEf, 1);
    } else if (lb == SOC_SBX_CALADAN3_IL_LOOPBACK_FC) {
        soc_reg_field_set(unit, IL_LOOPBACK_CONFIGr, &loop, IL_LOOPBACK_FC_ENABLEf, 1);
    }
    soc_reg32_set(unit, IL_LOOPBACK_CONFIGr, ifnum, 0, loop);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_loopback_get(int unit, soc_port_t port, int *lb) 
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    ifnum = IL_INSTANCE(ifnum);
    soc_reg32_get(unit, IL_LOOPBACK_CONFIGr, ifnum, 0, &data);
    if (lb) {
        if (soc_reg_field_get(unit, IL_LOOPBACK_CONFIGr, data, IL_LOOPBACK_L1_ENABLEf)) {
            *lb = SOC_SBX_CALADAN3_IL_LOOPBACK_L1;
        } else if (soc_reg_field_get(unit, IL_LOOPBACK_CONFIGr, data, 
                                     IL_LOOPBACK_R1_ENABLEf)) {
            *lb = SOC_SBX_CALADAN3_IL_LOOPBACK_RMT;
        } else if (soc_reg_field_get(unit, IL_LOOPBACK_CONFIGr, 
                                     data, IL_LOOPBACK_L2_ENABLEf)) {
            *lb = SOC_SBX_CALADAN3_IL_LOOPBACK_L2;
        } else if (soc_reg_field_get(unit, IL_LOOPBACK_CONFIGr, 
                                     data, IL_LOOPBACK_FC_ENABLEf)) {
            *lb = SOC_SBX_CALADAN3_IL_LOOPBACK_FC;
        } else {
            *lb = SOC_SBX_CALADAN3_IL_LOOPBACK_NONE;
        }
    }
    return SOC_E_NONE;
}


int 
soc_sbx_caladan3_il_uninit(int unit, int port)
{
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;

    if (ilkn_config[unit][ifnum].tx_stat_mem) {
            soc_cm_sfree(unit, ilkn_config[unit][ifnum].tx_stat_mem);
    }
    if (ilkn_config[unit][ifnum].rx0_stat_mem) {
            soc_cm_sfree(unit, ilkn_config[unit][ifnum].rx0_stat_mem);
    }
    if (ilkn_config[unit][ifnum].rx1_stat_mem) {
            soc_cm_sfree(unit, ilkn_config[unit][ifnum].rx1_stat_mem);
    }
    sal_memset(&ilkn_config[unit][ifnum], 0, sizeof(soc_c3_ilkn_cfg_t));
    return SOC_E_NONE;
}


int 
soc_sbx_caladan3_il_init(int unit, int port)
{

    int ifnum = 0;
    uint32 data = 0;
    int nlanes = 0;
    int mflen = 0;
    int fc, fc_calendar_length, loop;
    uint16 devid;
    uint8  revid;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;

    nlanes = soc_property_port_get(unit, port, 
                 spn_ILKN_NUM_LANES, 0);
    if ((nlanes  <= 0) || 
        (nlanes  > SOC_SBX_CALADAN3_IL_LANES_MAX)) {
        nlanes = SOC_INFO(unit).port_num_lanes[port];
    }
        

    /* Metaframe */
    mflen = soc_property_port_get(unit, port, 
                 spn_ILKN_METAFRAME_SYNC_PERIOD, SOC_SBX_CALADAN3_IL_METAFRAME_DEFAULT);
    if ((mflen < SOC_SBX_CALADAN3_IL_METAFRAME_MIN) || 
          (mflen > SOC_SBX_CALADAN3_IL_METAFRAME_MAX)) {
        mflen = SOC_SBX_CALADAN3_IL_METAFRAME_DEFAULT;
    }
    mflen--;

    /* RX */
    soc_reg32_get(unit, IL_RX_CORE_CONFIG0r, IL_INSTANCE(ifnum), 0, &data);
    /* The IL Transmitter and Receiver on Caladan3 will support 
     * Non-interleaved packet transmission only.
     */
    soc_reg_field_set(unit, IL_RX_CORE_CONFIG0r, &data, 
        CTL_RX_PACKET_MODEf, 1);
    soc_reg_field_set(unit, IL_RX_CORE_CONFIG0r, &data, 
        CTL_RX_BURSTMAXf, SOC_SBX_CALADAN3_IL_BURST_MAX_256B);
    soc_reg32_set(unit, IL_RX_CORE_CONFIG0r, IL_INSTANCE(ifnum), 0, data);


    /* Since IL protects with CRC we could forgo FCS */
    soc_reg32_set(unit, IL_IEEE_CRC32_CONFIGr, IL_INSTANCE(ifnum), 0, 0);

    /* TX */
    soc_reg32_get(unit, IL_TX_CORE_CONFIG0r, IL_INSTANCE(ifnum), 0, &data);
    soc_reg_field_set(unit, IL_TX_CORE_CONFIG0r, &data, 
        CTL_TX_BURSTSHORTf, SOC_SBX_CALADAN3_IL_BURST_SHORT_64B);
    soc_reg_field_set(unit, IL_TX_CORE_CONFIG0r, &data, 
        CTL_TX_BURSTMAXf, SOC_SBX_CALADAN3_IL_BURST_MAX_256B);
    soc_reg_field_set(unit, IL_TX_CORE_CONFIG0r, &data, 
        CTL_TX_MFRAMELEN_MINUS1f, mflen);
    soc_reg32_set(unit, IL_TX_CORE_CONFIG0r, IL_INSTANCE(ifnum), 0, data);

    soc_reg32_get(unit, IL_TX_CONFIGr, IL_INSTANCE(ifnum), 0, &data);
    soc_reg_field_set(unit, IL_TX_CONFIGr, &data, 
        IL_TX_ENHANCED_SCHEDULING_ENf, 1);
    soc_reg32_set(unit, IL_TX_CONFIGr, IL_INSTANCE(ifnum), 0, data);


    fc = soc_property_port_get(unit, port, spn_FC_OOB_TYPE, -1);
    if (fc < 0) {
        fc = SOC_SBX_CFG_CALADAN3(unit)->fc_type[ifnum];
        /* Supported values are defined in port.h
         * ilkn=0, ilkn_oob=1/3
         */
        if ((fc==3) || (fc==1)) {
            fc = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
        } else {
            fc = SOC_SBX_CALADAN3_FC_TYPE_ILKN;
        }
    } else {
        /* 
         * Supported values are defined in property.h
         * We dont support SPI, 0=none/inband,2=ilkn_oob,3=hcfc_oob
         */
        if ((fc <= 1) || (fc > 3)) {
            fc = SOC_SBX_CALADAN3_FC_TYPE_ILKN;
        } else if (fc == 2) {
            fc = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
        }
    }
    soc_sbx_caladan3_il_flow_control_mode_set(unit, ifnum, fc);
    ilkn_config[unit][ifnum].fc_type = fc;

    /* Update calendar len */
    if (ifnum == 0) {
       fc_calendar_length = 
           soc_property_get(unit, "fc_calendar_length_il_line", 64);
    } else {
       fc_calendar_length = 
           soc_property_get(unit, "fc_calendar_length_il_fabric", 64);
    }
    soc_sbx_caladan3_il_flow_control_calendar_set(unit, ifnum, fc, fc_calendar_length);

    /* Compression defaults to uncompressed */
    soc_sbx_caladan3_il_flow_control_calendar_compression_set(unit, ifnum, fc, 
         SOC_SBX_CALADAN3_IL_CHANNELS_MAX, SOC_SBX_CALADAN3_IL_CHANNELS_MAX);


    /* Global Config */
    soc_reg32_get(unit, IL_GLOBAL_CONFIGr, IL_INSTANCE(ifnum), 0, &data);
    soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, 
            CFG_LINK_XOFFf, SOC_SBX_CALADAN3_IL_LINK_FC_DISABLED);
    soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, CFG_RX_WC_BITFLIPf, 1);
    soc_reg_field_set(unit, IL_GLOBAL_CONFIGr, &data, CFG_TX_WC_BITFLIPf, 1);
    soc_reg32_set(unit, IL_GLOBAL_CONFIGr, IL_INSTANCE(ifnum), 0, data);

    soc_sbx_caladan3_il_enable_channels(unit, ifnum, 0xffffffff, 0xffffffff);
    soc_sbx_caladan3_il_set_mtu(unit, ifnum, JUMBO_MAXSZ);
    soc_sbx_caladan3_il_set_lanes(unit, ifnum, nlanes);

    /* take it out of reset */
    soc_reg32_get(unit, IL_GLOBAL_CONTROLr, IL_INSTANCE(ifnum), 0, &data);
    soc_reg_field_set(unit, IL_GLOBAL_CONTROLr, &data, SOFT_RESETf, 0);
    soc_reg32_set(unit, IL_GLOBAL_CONTROLr, IL_INSTANCE(ifnum), 0, data);

    /* init stats memory */
    soc_reg32_set(unit, IL_MEMORY_INITr, IL_INSTANCE(ifnum), 0, 0x1f);

    
    loop = soc_property_port_get(unit, port, "fc_oob_loopback", 0);
    if (loop) {
        soc_sbx_caladan3_il_flow_control_loopback_enable(unit, ifnum, fc, loop);
    }

    fc = soc_property_port_get(unit, port, spn_ILKN_INTERFACE_STATUS_OOB_IGNORE, 0);
    if (fc) {
        ilkn_config[unit][ifnum].oob_status_ignore = 1;
        soc_sbx_caladan3_il_interface_status_override_set(unit, ifnum, 0, 0, 0, 0, 1, 1);
    }
    soc_sbx_caladan3_il_force_resync(unit, ifnum);
    soc_sbx_caladan3_il_clear(unit, ifnum);

    ilkn_config[unit][ifnum].num_lanes = nlanes;
    ilkn_config[unit][ifnum].num_channels = SOC_SBX_CALADAN3_IL_CHANNELS_MAX;
    ilkn_config[unit][ifnum].tx_stat_mem_size =   
        (soc_mem_entry_words(unit, IL_STAT_MEM_3m) * sizeof(uint32)) *
            (soc_mem_index_max(unit, IL_STAT_MEM_3m) + 1);
    ilkn_config[unit][ifnum].rx0_stat_mem_size =   
        (soc_mem_entry_words(unit, IL_STAT_MEM_0m) * sizeof(uint32)) *
            (soc_mem_index_max(unit, IL_STAT_MEM_0m) + 1);
    ilkn_config[unit][ifnum].rx1_stat_mem_size =   
        (soc_mem_entry_words(unit, IL_STAT_MEM_1m) * sizeof(uint32)) *
            (soc_mem_index_max(unit, IL_STAT_MEM_1m) + 1);
    if (!ilkn_config[unit][ifnum].tx_stat_mem) {
        ilkn_config[unit][ifnum].tx_stat_mem = 
            soc_cm_salloc(unit, ilkn_config[unit][ifnum].tx_stat_mem_size, "ilkn tx stats");
    }
    if (!ilkn_config[unit][ifnum].rx0_stat_mem) {
        ilkn_config[unit][ifnum].rx0_stat_mem = 
            soc_cm_salloc(unit, ilkn_config[unit][ifnum].rx0_stat_mem_size, "ilkn rx0 stats");
    }
    if (!ilkn_config[unit][ifnum].rx1_stat_mem) {
        ilkn_config[unit][ifnum].rx1_stat_mem = 
            soc_cm_salloc(unit, ilkn_config[unit][ifnum].rx1_stat_mem_size, "ilkn rx1 stats");
    }

    /* A0 counters are not reliable, issue fixed in A1 */
    soc_cm_get_id(unit, &devid, &revid);
    ilkn_config[unit][ifnum].counters_available = 
          ((((devid == BCM88030_DEVICE_ID) && (revid == BCM88030_A0_REV_ID)) || 
             ((devid == BCM88034_DEVICE_ID) && (revid == BCM88034_A0_REV_ID))) ? 0 : 1);


    return SOC_E_NONE;
}

/*
 * Function: soc_sbx_caladan3_il_intf_status_get
 *
 * Purpose: Get the interface and lane status
 */
int
soc_sbx_caladan3_il_intf_status_get(int unit, int port, int *intf_status, int *lane_status)
{
    uint32 data = 0xffffffff;
    int ifnum = 0;
    int s = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if (ilkn_config[unit][ifnum].oob_status_ignore) {
         SOC_IF_ERROR_RETURN(
             soc_reg32_get(unit, IL_RX_CORE_STATUS1r, IL_INSTANCE(ifnum), 0, &data));
         if (intf_status) {
             *intf_status = soc_reg_field_get(unit, IL_RX_CORE_STATUS1r, data,
                                STAT_RX_ALIGNEDf);
         }
         if (lane_status) {
             *lane_status = soc_reg_field_get(unit, IL_RX_CORE_STATUS1r, data,
                                STAT_RX_SYNCEDf);
         }
    } else {
        SOC_IF_ERROR_RETURN(
            soc_reg32_set(unit, IL_FLOWCONTROL_L2_INTRr, IL_INSTANCE(ifnum), 0, data));
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_L2_INTRr, IL_INSTANCE(ifnum), 0, &data));
        if (intf_status) {
            s = soc_reg_field_get(unit, IL_FLOWCONTROL_L2_INTRr, data, 
                                  IL_FLOWCONTROL_INTSTS_IB_RX_INTF_DOWNf);
            s |= soc_reg_field_get(unit, IL_FLOWCONTROL_L2_INTRr, data, 
                                   IL_FLOWCONTROL_INTSTS_OOB_RX_INTF_DOWNf);
            *intf_status = (s == 0) ? 1 : 0;
        }
        if (lane_status) {
            s = soc_reg_field_get(unit, IL_FLOWCONTROL_L2_INTRr, data, 
                                  IL_FLOWCONTROL_INTSTS_IB_RX_LANE_DOWNf);
            s |= soc_reg_field_get(unit, IL_FLOWCONTROL_L2_INTRr, data, 
                                   IL_FLOWCONTROL_INTSTS_OOB_RX_LANE_DOWNf);
            *lane_status = (s == 0) ? 1 : 0;
        }
    }
    return  SOC_E_NONE;
}

int
soc_sbx_caladan3_il_intf_flowcontrol_status_get(int unit, int port, int *intf_status, int *lane_status)
{
    uint32 data = 0;
    int ifnum = 0;
    int d = 0, mode = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    SOC_IF_ERROR_RETURN(
        soc_sbx_caladan3_il_flow_control_mode_get(unit, port, &mode));
    if (mode == SOC_SBX_CALADAN3_FC_TYPE_ILKN) {
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_RX_CORE_STATUS0r, IL_INSTANCE(ifnum), 0, &data));
        if (intf_status) {
            d = soc_reg_field_get(unit, IL_RX_CORE_STATUS0r, data, 
                                        STAT_RX_DIAGWORD_INTFSTATf);
            *intf_status = (d == 0xfff) ? (1) : (0);
        }
        if (lane_status) {
            *lane_status = soc_reg_field_get(unit, IL_RX_CORE_STATUS0r,
                                             data, STAT_RX_DIAGWORD_LANESTATf);
        }
    } else {
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_OOB_RX_STSr, IL_INSTANCE(ifnum), 0, &data));
        if (intf_status) {
            *intf_status = soc_reg_field_get(unit, IL_FLOWCONTROL_OOB_RX_STSr, data, 
                                         IL_FLOWCONTROL_OOB_RX_INTF_STATUSf);
        }
        if (lane_status) {
            *lane_status = soc_reg_field_get(unit, IL_FLOWCONTROL_OOB_RX_STSr, data, 
                                         IL_FLOWCONTROL_OOB_RX_LANE_STATUSf);
        }
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_channel_flowcontrol_status_get(int unit, int port, 
                                                   int *tx_ch0_ch31, int *tx_ch32_ch63,
                                                   int *rx_ch0_ch31, int *rx_ch32_ch63)
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_STS0r, IL_INSTANCE(ifnum), 0, &data));
    if (rx_ch0_ch31) {
        *rx_ch0_ch31 = soc_reg_field_get(unit, IL_FLOWCONTROL_RXFC_STS0r, data, 
                                         IL_FLOWCONTROL_RXFC_STS0f);
    }
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_STS1r, IL_INSTANCE(ifnum), 0, &data));
    if (rx_ch32_ch63) {
        *rx_ch32_ch63 = soc_reg_field_get(unit, IL_FLOWCONTROL_RXFC_STS1r, data, 
                                          IL_FLOWCONTROL_RXFC_STS1f);
    }
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_STS0r, IL_INSTANCE(ifnum), 0, &data));
    if (rx_ch0_ch31) {
        *rx_ch0_ch31 = soc_reg_field_get(unit, IL_FLOWCONTROL_TXFC_STS0r, data, 
                                         IL_FLOWCONTROL_TXFC_STS0f);
    }
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_STS1r, IL_INSTANCE(ifnum), 0, &data));
    if (rx_ch32_ch63) {
        *rx_ch32_ch63 = soc_reg_field_get(unit, IL_FLOWCONTROL_TXFC_STS1r, data, 
                                          IL_FLOWCONTROL_TXFC_STS1f);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_channel_flowcontrol_is_override_on(int unit, int port, 
                                                       int *tx, int *rx)
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if ((tx != NULL) || (rx != NULL)) {
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_CONFIGr, IL_INSTANCE(ifnum), 0, &data));
    }
    if (tx) {
        *tx = soc_reg_field_get(unit, IL_FLOWCONTROL_CONFIGr, data, 
                                IL_FLOWCONTROL_TXFC_OVERRIDE_ENABLEf);
    }
    if (rx) {
        *rx = soc_reg_field_get(unit, IL_FLOWCONTROL_CONFIGr, data, 
                                IL_FLOWCONTROL_RXFC_OVERRIDE_ENABLEf);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_channel_flowcontrol_override_get(int unit, int port, 
                                                    int *tx_ch0_ch31, int *tx_ch32_ch63,
                                                    int *rx_ch0_ch31, int *rx_ch32_ch63)
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    ifnum = IL_INSTANCE(ifnum);
    if (rx_ch0_ch31) {
        data = 0;
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, ifnum, 0, &data));
        *rx_ch0_ch31 = soc_reg_field_get(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, 
                          data, IL_FLOWCONTROL_RXFC_OVRVAL0f);
    }
    if (rx_ch32_ch63) {
        data = 0;
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, ifnum, 0, &data));
       *rx_ch32_ch63 = soc_reg_field_get(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, 
                          data, IL_FLOWCONTROL_RXFC_OVRVAL1f);
    }
    if (tx_ch0_ch31) {
        data = 0;
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL0r, ifnum, 0, &data));
        *tx_ch0_ch31 = soc_reg_field_get(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL0r, 
                          data, IL_FLOWCONTROL_TXFC_OVRVAL0f);
    }
    if (tx_ch32_ch63) {
        data = 0;
        SOC_IF_ERROR_RETURN(
            soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL1r, ifnum, 0, &data));
        *tx_ch32_ch63 = soc_reg_field_get(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL1r,
                           data, IL_FLOWCONTROL_TXFC_OVRVAL1f);
    }
    return SOC_E_NONE;
}
                               
int
soc_sbx_caladan3_il_channel_flowcontrol_rx_override_set(int unit, int port, 
                                                    int rx_ch0_ch31, int rx_ch32_ch63,
                                                    int rx_ch0_ch31_en, int rx_ch32_ch63_en)
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    ifnum = IL_INSTANCE(ifnum);
    data = 0;
    soc_reg_field_set(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, &data, 
                      IL_FLOWCONTROL_RXFC_OVRVAL0f, rx_ch0_ch31);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL0r, ifnum, 0, data));
    data = 0;
    soc_reg_field_set(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL1r, &data, 
                      IL_FLOWCONTROL_RXFC_OVRVAL1f, rx_ch32_ch63);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_OVERRIDE_VAL1r, ifnum, 0, data));
    rx_ch0_ch31_en |= rx_ch32_ch63_en;
    data = 0;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_CONFIGr, ifnum, 0, &data));
    soc_reg_field_set(unit, IL_FLOWCONTROL_CONFIGr, &data, 
                      IL_FLOWCONTROL_RXFC_OVERRIDE_ENABLEf, rx_ch0_ch31_en);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_CONFIGr, ifnum, 0, data));
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_channel_flowcontrol_tx_override_set(int unit, int port, 
                                                    int tx_ch0_ch31, int tx_ch32_ch63,
                                                    int tx_ch0_ch31_en, int tx_ch32_ch63_en)
{
    uint32 data = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    ifnum = IL_INSTANCE(ifnum);
    data = 0;
    soc_reg_field_set(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL0r, &data, 
                      IL_FLOWCONTROL_TXFC_OVRVAL0f, tx_ch0_ch31);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL0r, ifnum, 0, data));
    
    data = 0;
    soc_reg_field_set(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL1r, &data, 
                      IL_FLOWCONTROL_TXFC_OVRVAL1f, tx_ch32_ch63);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_OVERRIDE_VAL1r, ifnum, 0, data));

    data = 0;
    tx_ch0_ch31_en |= tx_ch32_ch63_en;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, IL_FLOWCONTROL_CONFIGr, ifnum, 0, &data));
    soc_reg_field_set(unit, IL_FLOWCONTROL_CONFIGr, &data, 
        IL_FLOWCONTROL_TXFC_OVERRIDE_ENABLEf, tx_ch0_ch31_en);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, IL_FLOWCONTROL_CONFIGr, ifnum, 0, data));
    return SOC_E_NONE;
}

/*
 * Function
 *     soc_sbx_caladan3_il_interface_status_override_set
 * Purpose
 *     Override the ilkn status.
 *     Setting *_ov will trigger the override
 *     The value in *_status will be used to override
 */
int
soc_sbx_caladan3_il_interface_status_override_set(int unit, int ifnum, 
                                                  int intf_status, int intf_status_ov,
                                                  int lane_status, int lane_status_ov,
                                                  int tx_status, int tx_status_ov)
{

    uint32 data = 0;

    ifnum = IL_INSTANCE(ifnum);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_TX_CORE_CONTROL0r, ifnum, 0, &data));
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, SWO_CTL_TX_ENABLEf, tx_status_ov);
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, SWCTRL_CTL_TX_ENABLEf, tx_status);
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, 
                      SWO_CTL_TX_DIAGWORD_LANESTATf, lane_status_ov);
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, 
                      SWCTRL_CTL_TX_DIAGWORD_LANESTATf, lane_status);
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, 
                      SWO_CTL_TX_DIAGWORD_INTFSTATf, intf_status_ov);
    soc_reg_field_set(unit, IL_TX_CORE_CONTROL0r, &data, 
                      SWCTRL_CTL_TX_DIAGWORD_INTFSTATf, intf_status);
    SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_TX_CORE_CONTROL0r, ifnum, 0, data));
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_interface_status_override_get(int unit, int port, 
                                                  int *intf_status, int *intf_status_ov,
                                                  int *lane_status, int *lane_status_ov,
                                                  int *tx_status, int *tx_status_ov)
{

    uint32 data = 0;
    int ifnum;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0: 1;
    ifnum = IL_INSTANCE(ifnum);

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_TX_CORE_CONTROL0r, ifnum, 0, &data));
    if (tx_status_ov) {
        *tx_status_ov = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r,
                                          data, SWO_CTL_TX_ENABLEf);
    }
    if (tx_status) {
       *tx_status = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r, data, 
                                      SWCTRL_CTL_TX_ENABLEf);
    }
    if (lane_status_ov) {
       *lane_status_ov = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r, data,
                                           SWO_CTL_TX_DIAGWORD_LANESTATf);
    }
    if (lane_status) {
       *lane_status = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r, data,
                                        SWCTRL_CTL_TX_DIAGWORD_LANESTATf);
    }
    if (intf_status_ov) {
       *intf_status_ov = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r, data,
                                           SWO_CTL_TX_DIAGWORD_INTFSTATf);
    }
    if (intf_status) {
       *intf_status = soc_reg_field_get(unit, IL_TX_CORE_CONTROL0r, data,
                                        SWCTRL_CTL_TX_DIAGWORD_INTFSTATf);
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_tx_cnt_get(int unit, int port, int channel, 
                               uint64 *tx, uint64 *txb, uint64 *txe)
{
    il_stat_mem_3_entry_t stat_mem;
    int intf;
    uint64 e;
    intf = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0: 1;

    if ((channel < 0) || (channel >= SOC_SBX_CALADAN3_IL_CHANNELS_MAX)) {
       return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, IL_STAT_MEM_3m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     channel, &stat_mem));
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_3m, &stat_mem.entry_data[0],
                      TX_STAT_PKT_COUNTf, (uint32*)tx);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_3m, &stat_mem.entry_data[0],
                      TX_STAT_BYTE_COUNTf, (uint32*)txb);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      TX_STAT_BAD_PKT_PERR_COUNTf, (uint32*)&e);
    COMPILER_64_ADD_64(*txe, e);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      TX_STAT_GTMTU_PKT_COUNTf, (uint32*)&e);
    COMPILER_64_ADD_64(*txe, e);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      TX_STAT_EQMTU_PKT_COUNTf, (uint32*)&e);
    COMPILER_64_ADD_64(*txe, e);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_counters_tx_get(int unit, int port, int ctr_id, uint64 *counter)
{

    il_stat_mem_3_entry_t *entry;
    int max, i;
    int intf;
    uint64 p;
    uint32 u, l;

    
    intf = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0: 1;

    /* NOTE: not collecting size specific statistics */
    max = SOC_SBX_CALADAN3_IL_NUM_CHANNELS;

    for (i = 0; i < max; i++) {
        COMPILER_64_ZERO(counter[i]);
    }

    COMPILER_64_ZERO(p);
    if (ilkn_config[unit][intf].tx_stat_mem) {
        if (ctr_id == SOC_SBX_CALADAN3_TX_PKT_CNT) {
            sal_memset(ilkn_config[unit][intf].tx_stat_mem, 0, 
                       ilkn_config[unit][intf].tx_stat_mem_size);
            SOC_IF_ERROR_RETURN(soc_mem_read_range(unit, IL_STAT_MEM_3m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     0, max, ilkn_config[unit][intf].tx_stat_mem));
        }

        for (i=0; i < max; i++) {
            entry = soc_mem_table_idx_to_pointer(unit,
                              IL_STAT_MEM_3m,
                              il_stat_mem_3_entry_t *,
                              ilkn_config[unit][intf].tx_stat_mem, i);
            switch (ctr_id) {
            case  SOC_SBX_CALADAN3_TX_PKT_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_3m, &entry->entry_data[0],
                      TX_STAT_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case  SOC_SBX_CALADAN3_TX_BYTE_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_3m, &entry->entry_data[0],
                      TX_STAT_BYTE_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case  SOC_SBX_CALADAN3_TX_BAD_PKT_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_3m, &entry->entry_data[0],
                      TX_STAT_BAD_PKT_PERR_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case  SOC_SBX_CALADAN3_TX_GT_MTU_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_3m, &entry->entry_data[0],
                      TX_STAT_GTMTU_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case  SOC_SBX_CALADAN3_TX_EQ_MTU_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_3m, &entry->entry_data[0],
                      TX_STAT_EQMTU_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            default:
                ;
            }
        }
    }
    return SOC_E_NONE;
}


int
soc_sbx_caladan3_il_rx_cnt_get(int unit, int port, int channel, 
                               uint64 *rx, uint64 *rxb, uint64 *rxe)
{
    il_stat_mem_0_entry_t stat_mem;
    il_stat_mem_1_entry_t stat_mem1;
    int intf;
    uint64 rxerr = COMPILER_64_INIT(0,0);
    intf = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0: 1;

    if ((channel < 0) || (channel >= SOC_SBX_CALADAN3_IL_CHANNELS_MAX)) {
       return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, IL_STAT_MEM_0m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     channel, &stat_mem));
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      RX_STAT_PKT_COUNTf, (uint32*)rx);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      RX_STAT_BYTE_COUNTf, (uint32*)rxb);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_0m, &stat_mem.entry_data[0],
                      RX_STAT_BAD_PKT_ILERR_COUNTf, (uint32*)&rxerr);
    COMPILER_64_ADD_64(*rxe, rxerr);
    COMPILER_64_ZERO(rxerr);
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, IL_STAT_MEM_1m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     channel, &stat_mem1));
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_1m, &stat_mem1.entry_data[0],
                      RX_STAT_IEEE_CRCERR_PKT_COUNTf, (uint32*)&rxerr);
    COMPILER_64_ADD_64(*rxe, rxerr);

    COMPILER_64_ZERO(rxerr);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_1m, &stat_mem1.entry_data[0],
                      RX_STAT_GTMTU_PKT_COUNTf, (uint32*)&rxerr);
    COMPILER_64_ADD_64(*rxe, rxerr);

    COMPILER_64_ZERO(rxerr);
    /* coverity[incompatible_cast] */
    soc_mem_field_get(unit, IL_STAT_MEM_1m, &stat_mem1.entry_data[0],
                      RX_STAT_EQMTU_PKT_COUNTf, (uint32*)&rxerr);
    COMPILER_64_ADD_64(*rxe, rxerr);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_counters_rx_get(int unit, int port, int ctr_id, uint64 *counter) 
{
    int i, intf;
    uint64 p;
    uint32 u, l;
    il_stat_mem_0_entry_t *entry;
    il_stat_mem_1_entry_t *entry1;
    int max = 0;

    intf = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0: 1;

    /* NOTE: not collecting size specific statistics */
    max = SOC_SBX_CALADAN3_IL_NUM_CHANNELS; /* Both 0 and 1 are of same size */

    for (i = 0; i < max; i++) {
        COMPILER_64_ZERO(counter[i]);
    }

    if ((ilkn_config[unit][intf].rx0_stat_mem) && 
           (ilkn_config[unit][intf].rx1_stat_mem)) {
        if (ctr_id == SOC_SBX_CALADAN3_RX_PKT_CNT) {
            sal_memset(ilkn_config[unit][intf].rx0_stat_mem, 0, 
                       ilkn_config[unit][intf].rx0_stat_mem_size);
            SOC_IF_ERROR_RETURN(soc_mem_read_range(unit, IL_STAT_MEM_0m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     0, max, ilkn_config[unit][intf].rx0_stat_mem));
            sal_memset(ilkn_config[unit][intf].rx1_stat_mem, 0, 
                       ilkn_config[unit][intf].rx1_stat_mem_size);
            SOC_IF_ERROR_RETURN(soc_mem_read_range(unit, IL_STAT_MEM_1m,
                                     soc_sbx_block_find(unit, SOC_BLK_IL, intf),
                                     0, max, ilkn_config[unit][intf].rx1_stat_mem));
        }

        
        for (i=0; i < max; i++) {
            entry = soc_mem_table_idx_to_pointer(unit,
                          IL_STAT_MEM_0m, il_stat_mem_0_entry_t *,
                          ilkn_config[unit][intf].rx0_stat_mem, i);
            entry1 = soc_mem_table_idx_to_pointer(unit,
                          IL_STAT_MEM_1m, il_stat_mem_1_entry_t *,
                          ilkn_config[unit][intf].rx1_stat_mem, i);
            switch (ctr_id) {
            case SOC_SBX_CALADAN3_RX_PKT_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_0m, &entry->entry_data[0],
                      RX_STAT_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case SOC_SBX_CALADAN3_RX_BYTE_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_0m, &entry->entry_data[0],
                      RX_STAT_BYTE_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case SOC_SBX_CALADAN3_RX_BAD_PKT_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_0m, &entry->entry_data[0],
                      RX_STAT_BAD_PKT_ILERR_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case SOC_SBX_CALADAN3_RX_CRCERR_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_1m, &entry1->entry_data[0],
                      RX_STAT_IEEE_CRCERR_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case SOC_SBX_CALADAN3_RX_GT_MTU_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_1m, &entry1->entry_data[0],
                      RX_STAT_GTMTU_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            case SOC_SBX_CALADAN3_RX_EQ_MTU_CNT:
                /* coverity[incompatible_cast] */
                soc_mem_field_get(unit, IL_STAT_MEM_1m, &entry1->entry_data[0],
                      RX_STAT_EQMTU_PKT_COUNTf, (uint32*)&p);
                u = COMPILER_64_HI(p); l = COMPILER_64_LO(p);
                COMPILER_64_SET(p, l, u);
                COMPILER_64_ADD_64(*counter, p);
                COMPILER_64_ADD_64(*(counter+i+1), p);
                break;
            default:
                ;
            }
        }
    }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_il_controlled_counter_get(int unit, int ctr_id, int port, uint64 *counter)
{
    int ifnum;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if (!ilkn_config[unit][ifnum].counters_available) {
        return SOC_E_NONE;
    }
    if ((ctr_id < SOC_SBX_CALADAN3_TX_PKT_CNT) ||
          (ctr_id >= SOC_SBX_CALADAN3_LAST_CNT)) {
        return SOC_E_PARAM;
    } else if (ctr_id < SOC_SBX_CALADAN3_RX_PKT_CNT) {
        return soc_sbx_caladan3_il_counters_tx_get(unit, port, ctr_id, counter);
    } else {
        return soc_sbx_caladan3_il_counters_rx_get(unit, port, ctr_id, counter);
    }
}


/*
 * IL MAC Driver Follows 
 */


/*
 * Function:
 *      mac_ilkn_init
 * Purpose:
 *      Initialize Xmac into a known good state.
 * Parameters:
 *      unit -  unit #.
 *      port - Port number on unit.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *
 */
static int
mac_ilkn_init(int unit, soc_port_t port)
{
    return (soc_sbx_caladan3_il_init(unit, port));
}

/*
 * Function:
 *      mac_ilkn_enable_set
 * Purpose:
 *      Enable or disable MAC
 * Parameters:
 *      unit -  unit #.
 *      port - Port number on unit.
 *      enable - TRUE to enable, FALSE to disable
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_enable_set(int unit, soc_port_t port, int enable)
{
    int ifnum = 0;
    uint32 data0 = 0, data1 = 0;
    uint32 mask0 = 0, mask1 = 0;
    int nc = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    nc = ilkn_config[unit][ifnum].num_channels;

    soc_reg32_get(unit, IL_RX_CHAN_ENABLE0r, IL_INSTANCE(ifnum), 0, &data0);
    soc_reg32_get(unit, IL_RX_CHAN_ENABLE1r, IL_INSTANCE(ifnum), 0, &data1);
    if (enable) {
        mask0 = (nc > 31) ? (0xffffffff) : ((1 << nc)-1);
        if (data0 != mask0) {
            soc_reg32_set(unit, IL_RX_CHAN_ENABLE0r, IL_INSTANCE(ifnum), 0, mask0);
            soc_reg32_set(unit, IL_TX_CHAN_ENABLE0r, IL_INSTANCE(ifnum), 0, mask0);
        }
        if (nc > 31) {
            mask1 = (nc == 64) ? (0xffffffff) : ((1 << (nc - 32))-1);
            if (data1 != mask1) {
                soc_reg32_set(unit, IL_RX_CHAN_ENABLE1r, IL_INSTANCE(ifnum), 0, mask1);
                soc_reg32_set(unit, IL_TX_CHAN_ENABLE1r, IL_INSTANCE(ifnum), 0, mask1);
            }
        }
        soc_reg32_set(unit, IL_MEMORY_INITr, IL_INSTANCE(ifnum), 0, 0x1f);
        soc_sbx_caladan3_il_force_resync(unit, ifnum);
        soc_sbx_caladan3_il_clear(unit, ifnum);
    } else {
        data0 = 0;
        soc_reg32_set(unit, IL_RX_CHAN_ENABLE0r, IL_INSTANCE(ifnum), 0, data0);
        soc_reg32_set(unit, IL_RX_CHAN_ENABLE1r, IL_INSTANCE(ifnum), 0, data0);
        soc_reg32_set(unit, IL_TX_CHAN_ENABLE0r, IL_INSTANCE(ifnum), 0, data0);
        soc_reg32_set(unit, IL_TX_CHAN_ENABLE1r, IL_INSTANCE(ifnum), 0, data0);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_enable_get
 * Purpose:
 *      Get MAC enable state
 * Parameters:
 *      unit -  unit #.
 *      port - Port number on unit.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_enable_get(int unit, soc_port_t port, int *enable)
{
    uint32 data = 0;
    int ifnum;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;

    soc_reg32_get(unit, IL_RX_CORE_STATUS1r, IL_INSTANCE(ifnum), 0, &data);
    if (soc_reg_field_get(unit, IL_RX_CORE_STATUS1r, data, STAT_RX_ALIGNEDf)) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_duplex_set
 * Purpose:
 *      Set  XMAC in the specified duplex mode.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      duplex - Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 */
static int
mac_ilkn_duplex_set(int unit, soc_port_t port, int duplex)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_duplex_get
 * Purpose:
 *      Get XMAC duplex mode.
 * Parameters:
 *      unit -  unit #.
 *      duplex - (OUT) Boolean: true --> full duplex, false --> half duplex.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_duplex_get(int unit, soc_port_t port, int *duplex)
{
    *duplex = TRUE; /* Always */
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_pause_set
 * Purpose:
 *      There is no Pause on the interlaken interface. Not sure if its ok to resuse the same interface
 *      This routine overrides the FC information. if pause_tx or pause_rx is 0, this routine
 *      sets up an override to ignore the TX/RX flow control information and vice versa.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pause_tx - Boolean: transmit pause or -1 (don't change)
 *      pause_rx - Boolean: receive pause or -1 (don't change)
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    int rv = SOC_E_NONE;
    int tx0_en, tx1_en, rx0_en, rx1_en;
    int tx0_ov = 0, tx1_ov = 0, rx0_ov = 0, rx1_ov = 0;
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if (pause_tx >= 0) {
        if (pause_tx) {
            tx0_en = tx1_en = 0;
        } else {
            tx0_en = tx1_en = 1;
            tx1_ov = 0;
            tx0_ov = (ilkn_config[unit][ifnum].num_channels >= 32) ? (0xffffffff) : 
                         ((1 << ilkn_config[unit][ifnum].num_channels)-1);
            if (ilkn_config[unit][ifnum].num_channels > 32) {
                tx1_ov = (ilkn_config[unit][ifnum].num_channels >= 64) ? (0xffffffff) : 
                         ((1 << (ilkn_config[unit][ifnum].num_channels-32))-1);
            }
        }
        rv = soc_sbx_caladan3_il_channel_flowcontrol_tx_override_set(unit, port, 
                                                    tx0_ov, tx1_ov,
                                                    tx0_en, tx1_en);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Pause Tx set failed on port %d\n"),
                       unit, port));
            return rv;
        }
    }
    if (pause_rx >= 0) {
        if (pause_rx) {
            rx0_en = rx1_en = 0;
        } else {
            rx0_en = rx1_en = 1;
            rx1_ov = 0;
            rx0_ov = (ilkn_config[unit][ifnum].num_channels >= 32) ? (0xffffffff) : 
                         ((1 << ilkn_config[unit][ifnum].num_channels)-1);
            if (ilkn_config[unit][ifnum].num_channels > 32) {
                rx1_ov = (ilkn_config[unit][ifnum].num_channels >= 64) ? (0xffffffff) : 
                         ((1 << (ilkn_config[unit][ifnum].num_channels-32))-1);
            }
        }
        rv = soc_sbx_caladan3_il_channel_flowcontrol_rx_override_set(unit, port, 
                                                    rx0_ov, rx1_ov,
                                                    rx0_en, rx1_en);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Pause Rx set failed on port %d\n"),
                       unit, port));
            return rv;
        }
    }
    return rv;
}

/*
 * Function:
 *      mac_ilkn_pause_get
 * Purpose:
 *      There is no Pause on the interlaken interface. Not sure if its ok to resuse the same interface
 *      This routine returns the current state of FC override
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pause_tx - Boolean: transmit pause
 *      pause_rx - Boolean: receive pause
 *      pause_mac - MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    int rv = SOC_E_NONE;
    rv  = soc_sbx_caladan3_il_channel_flowcontrol_is_override_on(unit, 
                port, pause_tx, pause_rx);
    if (SOC_SUCCESS(rv)) {
        *pause_tx = (*pause_tx == 0);
        *pause_rx = (*pause_rx == 0);
    }
    return rv;
}

/*
 * Function:
 *      mac_ilkn_speed_set
 * Purpose:
 *      Set XMAC in the specified speed.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      speed - Per Lane speed
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_speed_set(int unit, soc_port_t port, int speed)
{
    int ifnum;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    SOC_IF_ERROR_RETURN(soc_phyctrl_notify(unit, port, phyEventSpeed, speed));
    soc_sbx_caladan3_il_force_resync(unit, ifnum);

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_speed_get
 * Purpose:
 *      Get XMAC speed
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      speed - (OUT) speed in Mb (per Lane)
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_speed_get(int unit, soc_port_t port, int *speed)
{
    *speed = 10312; /* Default to 10.3G, use phy data */

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_loopback_set
 * Purpose:
 *      Set a XMAC into/out-of loopback mode
 * Parameters:
 *      unit -  unit #.
 *      port -  unit # on unit.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Note:
 *      On Xmac, when setting loopback, we enable the TX/RX function also.
 *      Note that to test the PHY, we use the remote loopback facility.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_loopback_set(int unit, soc_port_t port, int lb)
{
    if (lb == 1) {
        lb = SOC_SBX_CALADAN3_IL_LOOPBACK_L1;
    } else if (lb == 3) {
        lb = SOC_SBX_CALADAN3_IL_LOOPBACK_RMT;
    } else if (lb == 4) {
        lb = SOC_SBX_CALADAN3_IL_LOOPBACK_FC;
    } else {
        lb = SOC_SBX_CALADAN3_IL_LOOPBACK_NONE;
    }
    return soc_sbx_caladan3_il_loopback_set(unit, port, lb);
}

/*
 * Function:
 *      mac_ilkn_loopback_get
 * Purpose:
 *      Get current XMAC loopback mode setting.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      loopback - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_loopback_get(int unit, soc_port_t port, int *lb)
{
    int rv = 0;
    rv = soc_sbx_caladan3_il_loopback_get(unit, port, lb);
    if (SOC_SUCCESS(rv)) {
        if (*lb == SOC_SBX_CALADAN3_IL_LOOPBACK_NONE) {
            *lb = 0;
        } else if (*lb == SOC_SBX_CALADAN3_IL_LOOPBACK_L1) {
            *lb = 1;
        } else if (*lb == SOC_SBX_CALADAN3_IL_LOOPBACK_RMT) {
            *lb = 3;
        } else if (*lb == SOC_SBX_CALADAN3_IL_LOOPBACK_FC) {
            *lb = 4;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_pause_addr_set
 * Purpose:
 *      Configure PAUSE frame source address.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pause_mac - (OUT) MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t m)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_pause_addr_get
 * Purpose:
 *      Retrieve PAUSE frame source address.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pause_mac - (OUT) MAC address used for pause transmission.
 * Returns:
 *      SOC_E_XXX
 * NOTE: We always write the same thing to TX & RX SA
 *       so, we just return the contects on RXMACSA.
 */
static int
mac_ilkn_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t m)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_interface_set
 * Purpose:
 *      Set a XMAC interface type
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pif - one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_UNAVAIL - requested mode not supported.
 * Notes:
 *
 */
static int
mac_ilkn_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_interface_get
 * Purpose:
 *      Retrieve XMAC interface type
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 */
static int
mac_ilkn_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);

    *pif = SOC_PORT_IF_ILKN;

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_frame_max_set
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
static int
mac_ilkn_frame_max_set(int unit, soc_port_t port, int size)
{
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if (size > JUMBO_MAXSZ) {
        return SOC_E_PARAM;
    }
    return soc_sbx_caladan3_il_set_mtu(unit, ifnum, size);
}

/*
 * Function:
 *      mac_ilkn_frame_max_get
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
static int
mac_ilkn_frame_max_get(int unit, soc_port_t port, int *size)
{
    int ifnum = 0;

    ifnum = (soc_sbx_caladan3_is_line_port(unit, port)) ? 0 : 1;
    if (size) {
        *size = ilkn_config[unit][ifnum].mtu;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_ifg_set
 * Description:
 *      Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      ifg - number of bits to use for average inter-frame gap
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      The function makes sure the IFG value makes sense and updates the
 *      IPG register in case the speed/duplex match the current settings
 */
static int
mac_ilkn_ifg_set(int unit, soc_port_t port, int speed,
                soc_port_duplex_t duplex, int ifg)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_ifg_get
 * Description:
 *      Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      The function makes sure the IFG value makes sense and updates the
 *      IPG register in case the speed/duplex match the current settings
 */
static int
mac_ilkn_ifg_get(int unit, soc_port_t port, int speed,
                soc_port_duplex_t duplex, int *ifg)
{
    return SOC_E_NONE;
}


/*
 * Function:
 *      mac_ilkn_encap_set
 * Purpose:
 *      Set the XMAC port encapsulation mode.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      mode - (IN) encap bits (defined above)
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_encap_set(int unit, soc_port_t port, int mode)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_encap_get
 * Purpose:
 *      Get the XMAC port encapsulation mode.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      mode - (INT) encap bits (defined above)
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_encap_get(int unit, soc_port_t port, int *mode)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_control_set
 * Purpose:
 *      To configure MAC control properties.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      type - MAC control property to set.
 *      int  - New setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_control_set(int unit, soc_port_t port, soc_mac_control_t type,
                  int value)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_control_get
 * Purpose:
 *      To get current MAC control setting.
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      type - MAC control property to set.
 *      int  - New setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_control_get(int unit, soc_port_t port, soc_mac_control_t type,
                  int *value)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_ilkn_ability_local_get
 * Purpose:
 *      Return the abilities of XMAC
 * Parameters:
 *      unit -  unit #.
 *      port -  port # on unit.
 *      mode - (OUT) Supported operating modes as a mask of abilities.
 * Returns:
 *      SOC_E_XXX
 */
static int
mac_ilkn_ability_local_get(int unit, soc_port_t port,
                          soc_port_ability_t *ability)
{
    
    return SOC_E_NONE;
}

/* Exported ILKN Core driver structure */
mac_driver_t soc_mac_ilkn = {
    "Interlaken Driver for C3",    /* drv_name */
    mac_ilkn_init,                   /* md_init  */
    mac_ilkn_enable_set,             /* md_enable_set */
    mac_ilkn_enable_get,             /* md_enable_get */
    mac_ilkn_duplex_set,             /* md_duplex_set */
    mac_ilkn_duplex_get,             /* md_duplex_get */
    mac_ilkn_speed_set,              /* md_speed_set */
    mac_ilkn_speed_get,              /* md_speed_get */
    mac_ilkn_pause_set,              /* md_pause_set */
    mac_ilkn_pause_get,              /* md_pause_get */
    mac_ilkn_pause_addr_set,         /* md_pause_addr_set */
    mac_ilkn_pause_addr_get,         /* md_pause_addr_get */
    mac_ilkn_loopback_set,           /* md_lb_set */
    mac_ilkn_loopback_get,           /* md_lb_get */
    mac_ilkn_interface_set,          /* md_interface_set */
    mac_ilkn_interface_get,          /* md_interface_get */
    NULL,                            /* md_ability_get - Deprecated */
    mac_ilkn_frame_max_set,          /* md_frame_max_set */
    mac_ilkn_frame_max_get,          /* md_frame_max_get */
    mac_ilkn_ifg_set,                /* md_ifg_set */
    mac_ilkn_ifg_get,                /* md_ifg_get */
    mac_ilkn_encap_set,              /* md_encap_set */
    mac_ilkn_encap_get,              /* md_encap_get */
    mac_ilkn_control_set,            /* md_control_set */
    mac_ilkn_control_get,            /* md_control_get */
    mac_ilkn_ability_local_get       /* md_ability_local_get */
 };



#endif /* defined(BCM_CALADAN3_SUPPORT) */
