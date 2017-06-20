/*
 * $Id: ctrl_eth_if.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <shared/bsl.h>

#include <assert.h>
#include <sal/core/libc.h>
#include <shared/et/osl.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmenetmib.h>
#include <soc/etc.h>
#include <soc/drv.h>
#include <soc/ctrl_if.h>

#include <soc/debug.h>
#include <soc/ea/tk371x/TkInit.h>

static void ether_ctrl_init(int unit);
static void ether_ctrl_deinit(int unit);
static void ether_ctrl_request(int unit, uint8 *buf_ptr, uint32 buf_len);
static void ether_ctrl_response_reg(int unit, void *rtn_fn);

static int soc_ea_oam_ctrl_init_state[SOC_MAX_NUM_DEVICES] = {0x0};

soc_ea_oam_ctrl_if_t ea_oam_ether_ctrlops = {
    ether_ctrl_init,
    ether_ctrl_deinit,
    ether_ctrl_request,
    ether_ctrl_response_reg
};

typedef struct ea_rx_dma_s {
    eth_dv_t        *oam_dv;        /* Contains entry for 1 descriptor */
    uint8               *buf;           /* Buffer for one packet */
} ea_rx_dma_t;

#define EA_OAM_PKT_SIZE_DFLT 1600
#define SOC_EA_OAM_DMA 3
#define SOC_EA_OAM_RX_DMA_COUNT 16
ea_rx_dma_t oam_rx_dma[SOC_EA_OAM_RX_DMA_COUNT]; /* DMA control & buffer*/

void *oam_rx_rtn_func = NULL;

static void
ether_ctrl_init(int unit)
{
    int i;
    ea_rx_dma_t   *pd;
    eth_dv_t    *dv; 
    int attached_switch_unit;
    soc_control_t   *soc;
#if defined(KEYSTONE) && defined(BCM_ROBO_SUPPORT)
    int dma_drop_def = 0;
    int test_unit = -1;
#endif

    soc = SOC_CONTROL(unit);
    
    if ((soc->attached_port < 0) || (soc->attached_unit < 0)) {
        attached_switch_unit = unit;
    } else {
        attached_switch_unit = soc->attached_unit;
    }

    soc_eth_dma_start_channel(unit, SOC_EA_OAM_DMA);

    for (i = 0; i < SOC_EA_OAM_RX_DMA_COUNT; i++) {
        pd = &oam_rx_dma[i];
        if ((pd->oam_dv = soc_eth_dma_dv_alloc(attached_switch_unit, 
                DV_RX, 1)) == NULL) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: Out of memory at dv #%d\n"),i));  
            return;
        }
        if ((pd->buf = soc_cm_salloc(attached_switch_unit, 
                    EA_OAM_PKT_SIZE_DFLT, "oam rx"))==NULL) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: Out of memory at buf #%d\n"),i)); 
            return;
        }
        
        soc_eth_dma_dv_reset(DV_RX, pd->oam_dv);
        dv = pd->oam_dv;
        
        dv->dv_channel = SOC_EA_OAM_DMA;
        dv->dv_unit = unit;

        soc_eth_dma_desc_add(pd->oam_dv, 
            PTR_TO_INT(pd->buf), EA_OAM_PKT_SIZE_DFLT);

        pd->oam_dv->dv_done_chain = oam_rx_rtn_func;

        soc_eth_dma_start(attached_switch_unit, pd->oam_dv);

    }
        	
    soc_eth_dma_occupancy_set(SOC_EA_OAM_DMA, unit);
               
#if defined(KEYSTONE) && defined(BCM_ROBO_SUPPORT)
    if (SOC_IS_TBX(soc_mii_unit)) {
        soc_eth_dma_classify_setup(soc_mii_unit, 
            socEthDmaClassifyOam, SOC_EA_OAM_DMA);

        /* 
          * If the other DMAs are not used by any other unit, 
          * drop packets of other DMAs.
          */
        dma_drop_def = 0;
        for (i = 0; i < NUMRXQ; i++) {
            if (i == SOC_EA_OAM_DMA) {
                continue;
            }
            soc_eth_dma_occupancy_get(i, &test_unit);
            if (test_unit >= 0) {
                dma_drop_def = 0;
                break;
            }
        }
        soc_eth_dma_default_drop_enable(soc_mii_unit, dma_drop_def);

    }

#endif

    TkExtOamTaskInit(unit);
	
	soc_ea_oam_ctrl_state_set(unit, socEaOamCtrlInitTrue);
}



static void
ether_ctrl_deinit(int unit)
{
    int i;
    ea_rx_dma_t   *pd;

    
    for (i = 0; i < SOC_EA_OAM_RX_DMA_COUNT; i++) {
        pd = &oam_rx_dma[i];
        if (pd->oam_dv) {
            pd->oam_dv = NULL;
        }
    }

    soc_eth_dma_abort_channel(unit, SOC_EA_OAM_DMA);

    /* Update the DMA occupancy table */
    soc_eth_dma_occupancy_set(SOC_EA_OAM_DMA, -1);

#if defined(KEYSTONE)
#if defined (BCM_ROBO_SUPPORT)
#include <soc/robo/robo_drv.h>
    if (SOC_IS_TBX(soc_mii_unit)) {
        soc_eth_dma_classify_setup(soc_mii_unit, 
            socEthDmaClassifyOam, SOC_ETH_DMA_CLASSIFY_DISABLE);
        soc_eth_dma_default_drop_enable(soc_mii_unit, 0);
    }
#endif	
#endif

    TkExtOamTaskExit(unit);

    soc_ea_oam_ctrl_state_set(unit, socEaOamCtrlInitFalse);
}

STATIC void
_ea_oam_tx_done_chain(int unit, eth_dv_t *dv)
{
    soc_eth_dma_dv_free(unit, dv);
}

static void
ether_ctrl_request(int unit, uint8 *buf_ptr, uint32 buf_len)
{
    eth_dv_t    *dv;
	int attached_switch_unit;
    soc_control_t   *soc;
#if defined(BCM_ROBO_SUPPORT)		
    uint32 dst_port;
    uint8 *brcm_tag_ptr;
    uint32 tag_value,tag_value2;
    uint32 tmp_len, byte_offset;
#endif	


    soc = SOC_CONTROL(unit);
    
    if ((soc->attached_port < 0) || (soc->attached_unit < 0)) {
        attached_switch_unit = unit;
    } else {
        attached_switch_unit = soc->attached_unit;
    }

    if ((dv = soc_eth_dma_dv_alloc(attached_switch_unit, DV_TX, 3)) == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "TX dv allocate failed\n")));
        return;
    }
    dv->dv_next = NULL;
    dv->dv_done_chain = _ea_oam_tx_done_chain;
#if defined (BCM_ROBO_SUPPORT)
#include <soc/robo/robo_drv.h>
    if (SOC_IS_TBX(attached_switch_unit)) {
        /* Prepare Broadcom Internal Tag */

        dst_port = soc->attached_port;

        dv->dv_dstid_53280 = (dst_port & 0x1f);
        dv->dv_dnm_ctrl_53280 = 1;
        dv->dv_opcode_53280 = BRCM_OP_EGR_DIR_53280;
        dv->dv_tc_53280 = 7 << 1;
        dv->dv_dp_53280 = 1;
        dv->dv_ing_vid_53280 = 1;
        dv->dv_flow_id_53280 = 0;
        dv->dv_filter_53280 = SOC_FILTER_BYPASS_ALL;
    
        ENET_COPY_MACADDR(
            buf_ptr, SOC_ROBO_DV_DEST_MAC_53280(dv));
        ENET_COPY_MACADDR(
            buf_ptr+sizeof(sal_mac_addr_t), SOC_ROBO_DV_SRC_MAC_53280(dv));
        tmp_len = sizeof(sal_mac_addr_t) * 2;


        brcm_tag_ptr = SOC_ROBO_DV_BRCM_TAG_53280(dv);
#if defined(LE_HOST)
        tag_value = _shr_swap32(dv->dv_brcm_tag);
        tag_value2 = _shr_swap32(dv->dv_brcm_tag2);
#else
        tag_value = dv->dv_brcm_tag;
        tag_value2 = dv->dv_brcm_tag2;
#endif
        sal_memcpy(brcm_tag_ptr, (uint8 *)&tag_value, 4);
        sal_memcpy(brcm_tag_ptr+4, (uint8 *)&tag_value2, 4);
        tmp_len += ENET_BRCM_SHORT_TAG_SIZE * 2;

#ifdef SOC_EA_ADD_TAG
        SOC_ROBO_DV_VLAN_TAG_53280(dv)[0] = 0x81;
        SOC_ROBO_DV_VLAN_TAG_53280(dv)[1] = 0x00;
        SOC_ROBO_DV_VLAN_TAG_53280(dv)[2] = 0x00;
        SOC_ROBO_DV_VLAN_TAG_53280(dv)[3] = 0x01;
        tmp_len += ENET_TAG_SIZE;
#endif
    
        /* Append header */
        soc_eth_dma_desc_add(dv, (sal_vaddr_t)dv->dv_dmabufhdr, tmp_len);
    
        /* 
          * Append payload
          * byte_offset=12 if original buffer: DA(6)+DA(6)+ Paylaod; 
          * byte_offset=16 if original buffer: DA+SA+VLAN+Payload 
          */
        byte_offset = 12; 
        tmp_len = buf_len - byte_offset;
    
        soc_eth_dma_desc_add(dv,PTR_TO_INT(buf_ptr+byte_offset),tmp_len);
    } else {
#define SOC_EA_DV_DST_MAC(dv)    (&(((uint8 *)(dv)->dv_dmabufhdr)[0]))
#define SOC_EA_DV_SRC_MAC(dv)    (&(((uint8 *)(dv)->dv_dmabufhdr)[6]))
        ENET_COPY_MACADDR(
            buf_ptr, SOC_EA_DV_DST_MAC(dv));
        ENET_COPY_MACADDR(
            buf_ptr+sizeof(sal_mac_addr_t), SOC_EA_DV_SRC_MAC(dv));
        tmp_len = sizeof(sal_mac_addr_t) * 2;

        /* Append header */
        soc_eth_dma_desc_add(dv, (sal_vaddr_t)dv->dv_dmabufhdr, tmp_len);

        /* 
          * Append payload
          * byte_offset=12 if original buffer: DA(6)+DA(6)+ Paylaod; 
          * byte_offset=16 if original buffer: DA+SA+VLAN+Payload 
          */
        byte_offset = 12; 
        tmp_len = buf_len - byte_offset;
    
        soc_eth_dma_desc_add(dv,PTR_TO_INT(buf_ptr+byte_offset),tmp_len);
    }
#endif
    soc_eth_dma_start(attached_switch_unit, dv);

    return;
}

static void
ether_ctrl_response_reg(int unit, void *rtn_fn)
{
    int i;
    ea_rx_dma_t   *pd;
    
    oam_rx_rtn_func = rtn_fn;

    for (i = 0; i < SOC_EA_OAM_RX_DMA_COUNT; i++) {
        pd = &oam_rx_dma[i];
        if (pd->oam_dv) {
            pd->oam_dv->dv_done_chain = oam_rx_rtn_func;
        }
    }
}

int
soc_ea_oam_ctrl_state_get(int unit)
{
    if(SOC_IS_EA(unit)){
        return soc_ea_oam_ctrl_init_state[unit];
    }else{
        return SOC_E_UNIT;
    }
}

int
soc_ea_oam_ctrl_state_set(int unit, int state)
{
    if(SOC_IS_EA(unit)){
        if((state == socEaOamCtrlInitTrue) || (state == socEaOamCtrlInitFalse)){
            soc_ea_oam_ctrl_init_state[unit] = state;
        }else{
            return SOC_E_PARAM;
        }
    }else{
        return SOC_E_UNIT;
    }

    return SOC_E_NONE;
}

