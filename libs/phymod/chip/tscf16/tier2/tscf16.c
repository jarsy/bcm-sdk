/*
 *         
 * $Id: tscf16.c,v 1.1.2.5 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 */

#ifndef _DV_TB_
 #define _SDK_TEFMOD16_ 1
#endif

#include <phymod/phymod.h>
#include <phymod/phymod_util.h>
#include <phymod/chip/bcmi_tscf_16nm_xgxs_defs.h>
#include <phymod/chip/falcon16.h>
#include <phymod/chip/tscf16.h>
#include "../../tscf16/tier1/tefmod16.h"
#include "../../tscf16/tier1/tefmod16_enum_defines.h"
#include "../../tscf16/tier1/tefmod16_device.h"
#include "../../tscf16/tier1/tefmod16_sc_lkup_table.h"
#include "../../tscf16/tier1/tf16PCSRegEnums.h"
#include "../../falcon16/tier1/src/falcon16_cfg_seq.h"
#include "../../falcon16/tier1/include/falcon16_tsc_common.h"
#include "../../falcon16/tier1/include/falcon16_tsc_interface.h"
#include "../../falcon16/tier1/include/falcon16_tsc_internal.h"
#include "../../falcon16/tier1/include/falcon16_tsc_dependencies.h"
#include "../../falcon16/tier1/include/falcon16_tsc_types.h"
#include "../../falcon16/tier1/include/falcon16_tsc_config.h"
#include "../../falcon16/tier1/include/common/srds_api_enum.h"
#include "../../falcon16/tier1/include/falcon16_tsc_debug_functions.h"

#define TSCF16_ID0                 0x600d
#define TSCF16_ID1                 0x8770
#define TSC4F_GEN2_MODEL           0x15
#define TSCF16_TECH_PROC           0x4
#define FALCON16_MODEL             0x1b
#define TSCF16_NOF_LANES_IN_CORE   4
#define TSCF16_LANE_SWAP_LANE_MASK 3
#define TSCF16_NOF_DFES            5
#define TSCF16_PHY_ALL_LANES       0xf

#define TSCF16_CORE_TO_PHY_ACCESS(_phy_access, _core_access) \
    do{\
        PHYMOD_MEMCPY(&(_phy_access)->access, &(_core_access)->access, sizeof((_phy_access)->access));\
        (_phy_access)->type           = (_core_access)->type; \
        (_phy_access)->port_loc       = (_core_access)->port_loc; \
        (_phy_access)->device_op_mode = (_core_access)->device_op_mode; \
        (_phy_access)->access.lane_mask = TSCF16_PHY_ALL_LANES; \
    }while(0)


#define TSCF16_MAX_FIRMWARES (5)

#define TSCF16_PMD_CRC_UCODE_VERIFY 1

/* uController's firmware */
extern unsigned char falcon16_ucode[];
extern unsigned short falcon16_ucode_ver;
extern unsigned short falcon16_ucode_crc;
extern unsigned short falcon16_ucode_len;

STATIC
int _tscf16_pll_multiplier_get(uint32_t pll_div, uint32_t *pll_multiplier)
{
    switch (pll_div) {
    case FALCON16_TSC_PLL_DIV_80:
        *pll_multiplier = 80;
        break;
    case FALCON16_TSC_PLL_DIV_96:
        *pll_multiplier = 96;
        break;
    case FALCON16_TSC_PLL_DIV_100:
        *pll_multiplier = 100;
        break;
    case FALCON16_TSC_PLL_DIV_128:
        *pll_multiplier = 128;
        break;
    case FALCON16_TSC_PLL_DIV_132:
        *pll_multiplier = 132;
        break;
    case FALCON16_TSC_PLL_DIV_140:
        *pll_multiplier = 140;
        break;
    case FALCON16_TSC_PLL_DIV_160:
        *pll_multiplier = 160;
        break;
    case FALCON16_TSC_PLL_DIV_165:
        *pll_multiplier = 165;
        break;
    case FALCON16_TSC_PLL_DIV_168:
        *pll_multiplier = 168;
        break;
    case FALCON16_TSC_PLL_DIV_170:
        *pll_multiplier = 170;
        break;
    case FALCON16_TSC_PLL_DIV_175:
        *pll_multiplier = 175;
        break;
    case FALCON16_TSC_PLL_DIV_180:
        *pll_multiplier = 180;
        break;
    case FALCON16_TSC_PLL_DIV_184:
        *pll_multiplier = 184;
        break;
    case FALCON16_TSC_PLL_DIV_200:
        *pll_multiplier = 200;
        break;
    case FALCON16_TSC_PLL_DIV_224:
        *pll_multiplier = 224;
        break;
    case FALCON16_TSC_PLL_DIV_264:
        *pll_multiplier = 264;
        break;
    case FALCON16_TSC_PLL_DIV_120:
        *pll_multiplier = 120;
        break;
    case FALCON16_TSC_PLL_DIV_144:
        *pll_multiplier = 144;
        break;
    case FALCON16_TSC_PLL_DIV_198:
        *pll_multiplier = 198;
        break;
    default:
        *pll_multiplier = 165;
        break;
    }

    return PHYMOD_E_NONE;
}

STATIC
int _tscf16_phy_firmware_lane_config_set(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t fw_config)
{
    struct falcon16_tsc_uc_lane_config_st serdes_firmware_config;
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane, i;
    uint32_t rst_status;
    uint32_t is_warm_boot;

    PHYMOD_MEMSET(&serdes_firmware_config, 0x0, sizeof(serdes_firmware_config));
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    for (i = 0; i < num_lane; i++) {
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        serdes_firmware_config.field.lane_cfg_from_pcs = fw_config.LaneConfigFromPCS;
        serdes_firmware_config.field.an_enabled        = fw_config.AnEnabled;
        serdes_firmware_config.field.dfe_on            = fw_config.DfeOn;
        serdes_firmware_config.field.force_brdfe_on    = fw_config.ForceBrDfe;
        serdes_firmware_config.field.scrambling_dis    = fw_config.ScramblingDisable;
        serdes_firmware_config.field.unreliable_los    = fw_config.UnreliableLos;
        serdes_firmware_config.field.media_type        = fw_config.MediaType;
        serdes_firmware_config.field.dfe_lp_mode       = fw_config.LpDfeOn;
        serdes_firmware_config.field.cl72_auto_polarity_en   = fw_config.Cl72AutoPolEn;
        serdes_firmware_config.field.cl72_restart_timeout_en = fw_config.Cl72RestTO;

        PHYMOD_IF_ERR_RETURN(PHYMOD_IS_WRITE_DISABLED(&phy_copy.access, &is_warm_boot));

        if (!is_warm_boot) {
            PHYMOD_IF_ERR_RETURN(falcon16_lane_soft_reset_read(&phy_copy.access, &rst_status));
            if (rst_status) PHYMOD_IF_ERR_RETURN (falcon16_lane_soft_reset_release(&phy_copy.access, 0));
            PHYMOD_IF_ERR_RETURN(falcon16_tsc_set_uc_lane_cfg(&phy_copy.access, serdes_firmware_config));
            if (rst_status) PHYMOD_IF_ERR_RETURN (falcon16_lane_soft_reset_release(&phy_copy.access, 1));
        }
    }
    return PHYMOD_E_NONE;
}

/*
 * Identify PHYID2 and PHYID3 Read From register 0x2 and 0x3 
 * param core core access information
 * param core_id predetermined core id 
 * param is_identified  if identification IDS match 
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_core_identify(const phymod_core_access_t* core, uint32_t core_id, uint32_t* is_identified)
{
    int ioerr = 0;
    const phymod_access_t *pm_acc = &core->access;
    PHYID2r_t id2;
    PHYID3r_t id3;
    MAIN0_SERDESIDr_t serdesid;
    /* DIG_REVID0r_t revid; */
    uint32_t model;

    *is_identified = 0;

    if (core_id == 0){
        ioerr += READ_PHYID2r(pm_acc, &id2);
        ioerr += READ_PHYID3r(pm_acc, &id3);
    }
    else{
        PHYID2r_SET(id2, ((core_id >> 16) & 0xffff));
        PHYID3r_SET(id3, core_id & 0xffff);
    }

    if (PHYID2r_REGID1f_GET(id2) == TSCF16_ID0 &&
       (PHYID3r_REGID2f_GET(id3) == TSCF16_ID1)) {
        /* PHY IDs match - now check PCS model */
        ioerr += READ_MAIN0_SERDESIDr(pm_acc, &serdesid);
        model = MAIN0_SERDESIDr_MODEL_NUMBERf_GET(serdesid);
        if (model == TSC4F_GEN2_MODEL)  {
            if (MAIN0_SERDESIDr_TECH_PROCf_GET(serdesid) == TSCF16_TECH_PROC) {
                *is_identified = 1;
            }
        }
    }

    return ioerr ? PHYMOD_E_IO : PHYMOD_E_NONE;
}

/*
 * Retrive core information 
 * param core core access information
 * param info core information (core_version,serdes_id,PHYID2,PHYID3) 
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_core_info_get(const phymod_core_access_t* core, phymod_core_info_t* info)
{
    uint32_t serdes_id;
    PHYID2r_t id2;
    PHYID3r_t id3;
    const phymod_access_t *pm_acc = &core->access;
    uint32_t rev_number;
    PHYMOD_IF_ERR_RETURN
        (tefmod16_revid_read(&core->access, &serdes_id));
    info->serdes_id = serdes_id;
    rev_number = (serdes_id & 0xc000) >> 14;
    if ((serdes_id & 0x3f) == TSC4F_GEN2_MODEL) {
        /* for rev a*/
        if (rev_number == 0)
            info->core_version = phymodCoreVersionTscf16;
    }
    PHYMOD_IF_ERR_RETURN(READ_PHYID2r(pm_acc, &id2));
    PHYMOD_IF_ERR_RETURN(READ_PHYID3r(pm_acc, &id3));

    info->phy_id0 = (uint16_t) id2.v[0];
    info->phy_id1 = (uint16_t) id3.v[0];
        
    return PHYMOD_E_NONE;
}

/*
 * set lane swapping for core
 * The pcs tx/rx swap - logical to physical mapping
 * The pmd tx/rx addr - logical address associated with the PMD lane with physcal index at the PCS interface
 *
 * lane_map_tx and lane_map_rx[lane=logic_lane] are logic-lane base.
 */

int tscf16_core_lane_map_set(const phymod_core_access_t* core, const phymod_lane_map_t* lane_map)
{
    uint32_t lane, pcs_tx_swap = 0, pcs_rx_swap = 0;
    uint8_t pmd_tx_addr[4], pmd_rx_addr[4];

    if (lane_map->num_of_lanes != TSCF16_NOF_LANES_IN_CORE){
        return PHYMOD_E_CONFIG;
    }
    for (lane = 0; lane < TSCF16_NOF_LANES_IN_CORE; lane++){
        if ((lane_map->lane_map_tx[lane] >= TSCF16_NOF_LANES_IN_CORE)||
             (lane_map->lane_map_rx[lane] >= TSCF16_NOF_LANES_IN_CORE)){
            return PHYMOD_E_CONFIG;
        }
        /*encode each lane as four bits*/
        pcs_tx_swap += lane_map->lane_map_tx[lane]<<(lane*4);
        pcs_rx_swap += lane_map->lane_map_rx[lane]<<(lane*4);
    }
    /* PMD lane addr is based on PCS logical to physical mapping*/
    for (lane = 0; lane < TSCF16_NOF_LANES_IN_CORE; lane++){
        pmd_tx_addr[((pcs_tx_swap >> (lane*4)) & 0xf)] = lane;
        pmd_rx_addr[((pcs_rx_swap >> (lane*4)) & 0xf)] = lane;
    }

    PHYMOD_IF_ERR_RETURN
        (tefmod16_pcs_tx_lane_swap(&core->access, pcs_tx_swap));
    PHYMOD_IF_ERR_RETURN
        (tefmod16_pcs_rx_lane_swap(&core->access, pcs_rx_swap));

    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_map_lanes(&core->access, TSCF16_NOF_LANES_IN_CORE, pmd_tx_addr, pmd_rx_addr));

    return PHYMOD_E_NONE;
}

/*
 * Get lane map information 
 * param core core access information
 * param lane_map (num_of_lanes, lane_map_rx, lane_map_tx) 
 * Returns PHYMOD_E_NONE if successful else  PHYMOD_E_ERROR 
 */
int tscf16_core_lane_map_get(const phymod_core_access_t* core, phymod_lane_map_t* lane_map)
{
    uint32_t pcs_tx_swap = 0 , pcs_rx_swap = 0, lane;

    PHYMOD_IF_ERR_RETURN(tefmod16_pcs_tx_lane_swap_get(&core->access, &pcs_tx_swap)); 
    PHYMOD_IF_ERR_RETURN(tefmod16_pcs_rx_lane_swap_get(&core->access, &pcs_rx_swap));

    for (lane = 0; lane < TSCF16_NOF_LANES_IN_CORE ; lane++){
        /*decode each lane from four bits*/
        lane_map->lane_map_tx[lane] = (pcs_tx_swap>>(lane*4)) & TSCF16_LANE_SWAP_LANE_MASK;
        lane_map->lane_map_rx[lane] = (pcs_rx_swap>>(lane*4)) & TSCF16_LANE_SWAP_LANE_MASK;
    }
    lane_map->num_of_lanes = TSCF16_NOF_LANES_IN_CORE;
        
    return PHYMOD_E_NONE;
}


int tscf16_core_reset_set(const phymod_core_access_t* core, phymod_reset_mode_t reset_mode, phymod_reset_direction_t direction)
{
        
    return PHYMOD_E_UNAVAIL;
    
}

int tscf16_core_reset_get(const phymod_core_access_t* core, phymod_reset_mode_t reset_mode, phymod_reset_direction_t* direction)
{
        
    return PHYMOD_E_UNAVAIL;
    
}


int tscf16_core_firmware_info_get(const phymod_core_access_t* core, phymod_core_firmware_info_t* fw_info)
{
        
    return PHYMOD_E_NONE;
    
}


/* 
 * Tscf16 firmware load
 * param core core access information
 * param load_method firmware load method: external fw load is valid
 * fw_loader  firmware loader
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */

STATIC
int _tscf16_core_firmware_load(const phymod_core_access_t* core, phymod_firmware_load_method_t load_method, phymod_firmware_loader_f fw_loader)
{
    switch(load_method){
    case phymodFirmwareLoadMethodInternal:
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_ucode_mdio_load(&core->access, falcon16_ucode, falcon16_ucode_len));
        break;
    case phymodFirmwareLoadMethodExternal:
        PHYMOD_NULL_CHECK(fw_loader);
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_ucode_init(&core->access));
        PHYMOD_IF_ERR_RETURN
            (falcon16_pram_firmware_enable(&core->access, 1, 0));
        PHYMOD_IF_ERR_RETURN(fw_loader(core, falcon16_ucode_len, falcon16_ucode));
        PHYMOD_IF_ERR_RETURN
            (falcon16_pram_firmware_enable(&core->access, 0, 0));
        break;
    case phymodFirmwareLoadMethodNone:
        break;
    default:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, (_PHYMOD_MSG("illegal fw load method %u"), load_method));
    }
    if (load_method != phymodFirmwareLoadMethodNone){
        /* PHYMOD_IF_ERR_RETURN(tscf_core_firmware_info_get(core, &actual_fw));
        if ((falcon16_ucode_crc != actual_fw.fw_crc) || (falcon16_ucode_ver != actual_fw.fw_version)){
            PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, (_PHYMOD_MSG("fw load validation was failed")));
        } */
    }

    return PHYMOD_E_NONE;

}

/* 
 * Set firmware configure 
 * param phy phy access information
 * param fw_core_config  firmare core config  
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_firmware_core_config_set(const phymod_phy_access_t* phy, phymod_firmware_core_config_t fw_core_config)
{
    struct falcon16_tsc_uc_core_config_st serdes_firmware_core_config;
    uint32_t rst_status;

    PHYMOD_MEMSET(&serdes_firmware_core_config, 0, sizeof(serdes_firmware_core_config));
    serdes_firmware_core_config.field.core_cfg_from_pcs = fw_core_config.CoreConfigFromPCS;
    serdes_firmware_core_config.field.vco_rate = fw_core_config.VcoRate;

    PHYMOD_IF_ERR_RETURN(falcon16_core_soft_reset_read(&phy->access, &rst_status));
    if (rst_status) PHYMOD_IF_ERR_RETURN (falcon16_tsc_core_dp_reset(&phy->access, 1));
    PHYMOD_IF_ERR_RETURN(falcon16_tsc_INTERNAL_set_uc_core_config(&phy->access, serdes_firmware_core_config));
    if (rst_status) PHYMOD_IF_ERR_RETURN (falcon16_tsc_core_dp_reset(&phy->access, 0));

    return PHYMOD_E_NONE;
}

/* 
 * Get firmware configure 
 * param phy phy access information
 * param fw_core_config  firmare core config  
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_firmware_core_config_get(const phymod_phy_access_t* phy, phymod_firmware_core_config_t* fw_core_config)
{
    struct falcon16_tsc_uc_core_config_st serdes_firmware_core_config;

    PHYMOD_IF_ERR_RETURN(falcon16_tsc_get_uc_core_config(&phy->access, &serdes_firmware_core_config));
    PHYMOD_MEMSET(fw_core_config, 0, sizeof(*fw_core_config));
    fw_core_config->CoreConfigFromPCS = serdes_firmware_core_config.field.core_cfg_from_pcs;
    fw_core_config->VcoRate = serdes_firmware_core_config.field.vco_rate;

    return PHYMOD_E_NONE;
}

/* 
 * Set firmware lane configure 
 * param phy phy access information
 * param fw_lane_config  firmare lane config  
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */

int tscf16_phy_firmware_lane_config_set(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t fw_lane_config)
{
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane, i;

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*Hold the per lne soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_lane_soft_reset_release(&phy_copy.access, 0));
    }

    PHYMOD_IF_ERR_RETURN
         (_tscf16_phy_firmware_lane_config_set(phy, fw_lane_config));
    /*Hold the per lane soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_lane_soft_reset_release(&phy_copy.access, 1));
    }

    /* next we need to toggle the pcs data path reset */
    PHYMOD_IF_ERR_RETURN
        (tefmod16_trigger_speed_change(&phy->access));
 
    return PHYMOD_E_NONE;
}

/* 
 * Get firmware lane configure 
 * param phy phy access information
 * param fw_lane_config  firmare lane config  
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_firmware_lane_config_get(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t* fw_lane_config)
{
    struct falcon16_tsc_uc_lane_config_st serdes_firmware_config;

    PHYMOD_MEMSET(&serdes_firmware_config, 0x0, sizeof(serdes_firmware_config));
    PHYMOD_IF_ERR_RETURN(falcon16_tsc_get_uc_lane_cfg(&phy->access, &serdes_firmware_config));
    PHYMOD_MEMSET(fw_lane_config, 0, sizeof(*fw_lane_config));
    fw_lane_config->LaneConfigFromPCS = serdes_firmware_config.field.lane_cfg_from_pcs;
    fw_lane_config->AnEnabled         = serdes_firmware_config.field.an_enabled;
    fw_lane_config->DfeOn             = serdes_firmware_config.field.dfe_on;
    fw_lane_config->LpDfeOn           = serdes_firmware_config.field.dfe_lp_mode;
    fw_lane_config->ForceBrDfe        = serdes_firmware_config.field.force_brdfe_on;
    fw_lane_config->ScramblingDisable = serdes_firmware_config.field.scrambling_dis;
    fw_lane_config->UnreliableLos     = serdes_firmware_config.field.unreliable_los;
    fw_lane_config->MediaType         = serdes_firmware_config.field.media_type;
    fw_lane_config->Cl72AutoPolEn     = serdes_firmware_config.field.cl72_auto_polarity_en;
    fw_lane_config->Cl72RestTO        = serdes_firmware_config.field.cl72_restart_timeout_en;

    return PHYMOD_E_NONE;
}


int tscf16_core_pll_sequencer_restart(const phymod_core_access_t* core, uint32_t flags, phymod_sequencer_operation_t operation)
{
    return PHYMOD_E_UNAVAIL;
    
}


int tscf16_core_wait_event(const phymod_core_access_t* core, phymod_core_event_t event, uint32_t timeout)
{
    switch(event){
    case phymodCoreEventPllLock:
        /* PHYMOD_IF_ERR_RETURN(tefmod16_pll_lock_wait(&core->access, timeout)); */
        break;
    default:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, (_PHYMOD_MSG("illegal wait event %u"), event));
    }
     
    return PHYMOD_E_NONE;
    
}

/*
 * Re-tune rx path
 * param phy phy access information
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_rx_restart(const phymod_phy_access_t* phy)
{
    PHYMOD_IF_ERR_RETURN(falcon16_tsc_rx_restart(&phy->access, 1));     
        
    return PHYMOD_E_NONE;
    
}

/*
 * Set phy polarity
 * param phy phy access information
 * param polarity   
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_polarity_set(const phymod_phy_access_t* phy, const phymod_polarity_t* polarity)
{
    PHYMOD_IF_ERR_RETURN
        (tefmod16_tx_rx_polarity_set(&phy->access, polarity->tx_polarity, polarity->rx_polarity));
    
    return PHYMOD_E_NONE;
    
}

/*
 * Get phy polarity
 * param phy phy access information
 * param polarity   
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_polarity_get(const phymod_phy_access_t* phy, phymod_polarity_t* polarity)
{
    PHYMOD_IF_ERR_RETURN
        (tefmod16_tx_rx_polarity_get(&phy->access, &polarity->tx_polarity, &polarity->rx_polarity));
        
    return PHYMOD_E_NONE;
}

/*
 * Set tx parameters 
 * param phy phy access information
 * param tx struct tx parameter (pre, main, post, post2, post3)
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_tx_set(const phymod_phy_access_t* phy, const phymod_tx_t* tx)
{
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_write_tx_afe(&phy->access, TX_AFE_PRE, tx->pre));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_write_tx_afe(&phy->access, TX_AFE_MAIN, tx->main));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_write_tx_afe(&phy->access, TX_AFE_POST1, tx->post));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_write_tx_afe(&phy->access, TX_AFE_POST2, tx->post2));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_write_tx_afe(&phy->access, TX_AFE_POST3, tx->post3));
        
    return PHYMOD_E_NONE;
}

/*
 * Get tx parameters 
 * param phy phy access information
 * param tx struct tx parameter (pre, main, post, post2, post3, amp)   
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_tx_get(const phymod_phy_access_t* phy, phymod_tx_t* tx)
{
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_PRE, &tx->pre));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_MAIN, &tx->main));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_POST1, &tx->post));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_POST2, &tx->post2));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_POST3, &tx->post3));
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_tx_afe(&phy->access, TX_AFE_RPARA, &tx->amp));
        
    return PHYMOD_E_NONE;
}

/*
 * Set TX parameters using override   
 * param phy phy access information
 * param tx struct tx override (enable, value)   
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_tx_override_set(const phymod_phy_access_t* phy, const phymod_tx_override_t* tx_override)
{
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_tx_pi_freq_override(&phy->access,
                                    tx_override->phase_interpolator.enable,
                                    tx_override->phase_interpolator.value));
    
    return PHYMOD_E_NONE;
    
}

/*
 * Get TX override    
 * param phy phy access information
 * param tx struct tx override (enable, value)   
 * Returns PHYMOD_E_NONE if successful else PHYMOD_E_ERROR 
 */
int tscf16_phy_tx_override_get(const phymod_phy_access_t* phy, phymod_tx_override_t* tx_override)
{
    uint16_t pi_value;

    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_tx_pi_control_get(&phy->access, &pi_value));

    tx_override->phase_interpolator.value = (int32_t) pi_value;

    return PHYMOD_E_NONE;
    
}

int tscf16_phy_rx_set(const phymod_phy_access_t* phy, const phymod_rx_t* rx)
{
    uint32_t i;
    uint8_t uc_lane_stopped;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, k;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    /*params check*/
    if ((rx->num_of_dfe_taps == 0) || (rx->num_of_dfe_taps > TSCF16_NOF_DFES)){
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, (_PHYMOD_MSG("illegal number of DFEs to set %u"), (unsigned int)rx->num_of_dfe_taps));
    }

       for (k = 0; k < num_lane; k++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + k)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + k);

        /*vga set*/
        /* first check if uc lane is stopped already */
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_stop_uc_lane_status(&pm_phy_copy.access, &uc_lane_stopped));
        if (!uc_lane_stopped) {
            PHYMOD_IF_ERR_RETURN(falcon16_tsc_stop_rx_adaptation(&pm_phy_copy.access, 1));
        }
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_VGA, rx->vga.value));
        /*dfe set*/
        for (i = 0 ; i < rx->num_of_dfe_taps ; i++){
            switch (i) {
                case 0:
                        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_DFE1, rx->dfe[i].value));
                    break;
                case 1:
                        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_DFE2, rx->dfe[i].value));
                    break;
                case 2:
                        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_DFE3, rx->dfe[i].value));
                    break;
                case 3:
                        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_DFE4, rx->dfe[i].value));
                    break;
                case 4:
                        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_DFE5, rx->dfe[i].value));
                    break;
                default:
                    return PHYMOD_E_PARAM;
            }
        }
        /*peaking filter set*/
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_PF, rx->peaking_filter.value));

        /* low freq peak filter */
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_write_rx_afe(&pm_phy_copy.access, RX_AFE_PF2, (int8_t)rx->low_freq_peaking_filter.value));
    }
 
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_rx_get(const phymod_phy_access_t* phy, phymod_rx_t* rx)
{
    int8_t tmpData;

    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_VGA,  &tmpData));
    rx->vga.value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_DFE1,  &tmpData));
    rx->dfe[0].value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_DFE2,  &tmpData));
    rx->dfe[1].value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_DFE3,  &tmpData));
    rx->dfe[2].value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_DFE4,  &tmpData));
    rx->dfe[3].value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_DFE5,  &tmpData));
    rx->dfe[4].value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_PF,  &tmpData));
    rx->peaking_filter.value = tmpData;
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_read_rx_afe(&phy->access, RX_AFE_PF2,  &tmpData));
    rx->low_freq_peaking_filter.value = tmpData;

    rx->num_of_dfe_taps = 5;
    rx->dfe[0].enable = 1;
    rx->dfe[1].enable = 1;
    rx->dfe[2].enable = 1;
    rx->dfe[3].enable = 1;
    rx->dfe[4].enable = 1;
    rx->vga.enable = 1;
    rx->low_freq_peaking_filter.enable = 1;
    rx->peaking_filter.enable = 1;
    
    return PHYMOD_E_NONE;
}


int tscf16_phy_rx_adaptation_resume(const phymod_phy_access_t* phy)
{
        
    uint8_t uc_lane_stopped;

    PHYMOD_IF_ERR_RETURN(falcon16_tsc_stop_uc_lane_status(&phy->access, &uc_lane_stopped));
    if (uc_lane_stopped) {
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_stop_rx_adaptation(&phy->access, 0));
    }
 
    return PHYMOD_E_NONE;
}


int tscf16_phy_reset_set(const phymod_phy_access_t* phy, const phymod_phy_reset_t* reset)
{
    phymod_firmware_lane_config_t fw_lane_config;

    PHYMOD_IF_ERR_RETURN (tscf16_phy_firmware_lane_config_get(phy, &fw_lane_config));

    if (fw_lane_config.LaneConfigFromPCS == 0) {
        PHYMOD_IF_ERR_RETURN (falcon16_phy_reset_set(phy, reset));
    }else{
        return PHYMOD_E_UNAVAIL; /*Not implemented, yet*/
    }

    return PHYMOD_E_NONE;
}

int tscf16_phy_reset_get(const phymod_phy_access_t* phy, phymod_phy_reset_t* reset)
{
    phymod_firmware_lane_config_t fw_lane_config;

    PHYMOD_IF_ERR_RETURN (tscf16_phy_firmware_lane_config_get(phy, &fw_lane_config));

    if (fw_lane_config.LaneConfigFromPCS == 0) {
         PHYMOD_IF_ERR_RETURN (falcon16_phy_reset_get(phy, reset));
     }else{
         return PHYMOD_E_UNAVAIL; /*Not implemented, yet*/
     }
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_power_set(const phymod_phy_access_t* phy, const phymod_phy_power_t* power)
{
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    if ((power->tx == phymodPowerOff) && (power->rx == phymodPowerOff)) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(tefmod16_port_enable_set(&pm_phy_copy.access, 0));
        }
    }
    if ((power->tx == phymodPowerOn) && (power->rx == phymodPowerOn)) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(tefmod16_port_enable_set(&pm_phy_copy.access, 1));
            PHYMOD_IF_ERR_RETURN(tefmod16_power_control(&phy->access, 0, 0));
        }
    }
    if ((power->tx == phymodPowerOff) && (power->rx == phymodPowerNoChange)) {
            /*disable tx on the PMD side */
            PHYMOD_IF_ERR_RETURN(falcon16_tsc_tx_disable(&phy->access, 1));
    }
    if ((power->tx == phymodPowerOn) && (power->rx == phymodPowerNoChange)) {
            /*enable tx on the PMD side */
            PHYMOD_IF_ERR_RETURN(falcon16_tsc_tx_disable(&phy->access, 0));
    }
    if ((power->tx == phymodPowerNoChange) && (power->rx == phymodPowerOff)) {
            /* disable rx on the PMD side */
            PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_set(&phy->access, 1));
    }
    if ((power->tx == phymodPowerNoChange) && (power->rx == phymodPowerOn)) {
            /*enable rx on the PMD side */
            PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_set(&phy->access, 0));
    }
 
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_power_get(const phymod_phy_access_t* phy, phymod_phy_power_t* power)
{
    int enable;
    uint32_t lb_enable;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_get(&pm_phy_copy.access, &enable));

    /* next check if PMD loopback is on */
    if (enable) {
        PHYMOD_IF_ERR_RETURN(falcon16_pmd_loopback_get(&pm_phy_copy.access, &lb_enable));
        if (lb_enable) enable = 0;
    }

    power->rx = (enable == 1)? phymodPowerOff: phymodPowerOn;
    /* Commented the following line. Because if in PMD loopback mode, we squelch the
           xmit, and we should still see the correct port status */
    /* PHYMOD_IF_ERR_RETURN(tefmod16_tx_squelch_get(&pm_phy_copy.access, &enable)); */
    power->tx = (enable == 1)? phymodPowerOff: phymodPowerOn;
     
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_hg2_codec_control_set(const phymod_phy_access_t* phy, phymod_phy_hg2_codec_t hg2_codec)
            /* disable rx on the PMD side */
{
    tefmod16_hg2_codec_t local_copy;

    switch (hg2_codec) {
        case phymodBcmHG2CodecOff: local_copy = TEFMOD16_HG2_CODEC_OFF;
                break;
        case phymodBcmHG2CodecOnWith8ByteIPG:  local_copy = TEFMOD16_HG2_CODEC_ON_8BYTE_IPG;
                break;
        case phymodBcmHG2CodecOnWith9ByteIPG:  local_copy = TEFMOD16_HG2_CODEC_ON_9BYTE_IPG;
                break;
        default: local_copy = TEFMOD16_HG2_CODEC_OFF;
                break;
        }
    PHYMOD_IF_ERR_RETURN(tefmod16_hg2_codec_set(&phy->access, local_copy));
        
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_hg2_codec_control_get(const phymod_phy_access_t* phy, phymod_phy_hg2_codec_t* hg2_codec)
{
    tefmod16_hg2_codec_t local_copy;

    PHYMOD_IF_ERR_RETURN(tefmod16_hg2_codec_get(&phy->access, &local_copy));

    switch (local_copy) {
        case TEFMOD16_HG2_CODEC_OFF: *hg2_codec = phymodBcmHG2CodecOff;
                break;
        case TEFMOD16_HG2_CODEC_ON_8BYTE_IPG:  *hg2_codec = phymodBcmHG2CodecOnWith8ByteIPG;
                break;
        case TEFMOD16_HG2_CODEC_ON_9BYTE_IPG:  *hg2_codec = phymodBcmHG2CodecOnWith9ByteIPG;
                break;
        default: *hg2_codec = phymodBcmHG2CodecOff;
                break;
        }
    
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_tx_lane_control_set(const phymod_phy_access_t* phy, phymod_phy_tx_lane_control_t tx_control)
{
    phymod_firmware_lane_config_t fw_lane_config;
    PHYMOD_IF_ERR_RETURN (tscf16_phy_firmware_lane_config_get(phy, &fw_lane_config));

    switch (tx_control) {
    case phymodTxTrafficDisable:
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_lane_control_set(&phy->access, TEFMOD16_TX_LANE_TRAFFIC_DISABLE));
        break;
    case phymodTxTrafficEnable:
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_lane_control_set(&phy->access, TEFMOD16_TX_LANE_TRAFFIC_ENABLE));
        break;
    case phymodTxReset:
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_lane_control_set(&phy->access, TEFMOD16_TX_LANE_RESET));
        break;
    case phymodTxSquelchOn:
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_squelch_set(&phy->access, 1));
        break;
    case phymodTxSquelchOff:
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_squelch_set(&phy->access, 0));
        break;
    case phymodTxElectricalIdleEnable:
        if (fw_lane_config.LaneConfigFromPCS == 0) {
            PHYMOD_IF_ERR_RETURN(falcon16_electrical_idle_set(&phy->access, 1));
        }else{
            return PHYMOD_E_UNAVAIL; /*autoneg */
        }
        break;
    case phymodTxElectricalIdleDisable:
        if (fw_lane_config.LaneConfigFromPCS == 0) {
            PHYMOD_IF_ERR_RETURN(falcon16_electrical_idle_set(&phy->access, 0));
        }else{
            return PHYMOD_E_UNAVAIL; 
        }
        break;
    default:
        break;
    }
    
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_tx_lane_control_get(const phymod_phy_access_t* phy, phymod_phy_tx_lane_control_t* tx_control)
{
    int enable, reset, tx_lane;
    uint32_t lb_enable;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_IF_ERR_RETURN(tefmod16_tx_squelch_get(&pm_phy_copy.access, &enable));

    /* next check if PMD loopback is on */
    if (enable) {
        PHYMOD_IF_ERR_RETURN(falcon16_pmd_loopback_get(&pm_phy_copy.access, &lb_enable));
        if (lb_enable) enable = 0;
    }

    if (enable) {
        *tx_control = phymodTxSquelchOn;
    } else {
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_lane_control_get(&pm_phy_copy.access, &reset, &tx_lane));
        if (!reset) {
            *tx_control = phymodTxReset;
        } else if (!tx_lane) {
            *tx_control = phymodTxTrafficDisable;
        } else {
            *tx_control = phymodTxTrafficEnable;
        }
    }
    
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_rx_lane_control_set(const phymod_phy_access_t* phy, phymod_phy_rx_lane_control_t rx_control)
{
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;

    switch (rx_control) {
    case phymodRxReset:
        PHYMOD_IF_ERR_RETURN(tefmod16_rx_lane_control_set(&phy->access, 1));
        break;
    case phymodRxSquelchOn:
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_set(&pm_phy_copy.access, 1));
        }
        break;
    case phymodRxSquelchOff:
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_set(&pm_phy_copy.access, 0));
        }
        break;
    default:
        break;
    }
    
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_rx_lane_control_get(const phymod_phy_access_t* phy, phymod_phy_rx_lane_control_t* rx_control)
{
    int enable, reset;
    uint32_t lb_enable;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_IF_ERR_RETURN(tefmod16_rx_squelch_get(&pm_phy_copy.access, &enable));
    /* next check if PMD loopback is on */
    if (enable) {
        PHYMOD_IF_ERR_RETURN(falcon16_pmd_loopback_get(&pm_phy_copy.access, &lb_enable));
        if (lb_enable) enable = 0;
    }
    if (enable) {
        *rx_control = phymodRxSquelchOn;
    } else {
        PHYMOD_IF_ERR_RETURN(tefmod16_rx_lane_control_get(&pm_phy_copy.access, &reset));
        if (reset == 0) {
            *rx_control = phymodRxReset;
        } else {
            *rx_control = phymodRxSquelchOff;
        }
    }
     
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_fec_enable_set(const phymod_phy_access_t* phy, uint32_t enable)
{
    int fec_en;
    tefmod16_fec_type_t fec_type;

    /* first check FEC type */
    if (PHYMOD_FEC_CL91_GET(enable)) {
        fec_type = TEFMOD16_CL91;
    } else {
        fec_type = TEFMOD16_CL74;
    }

    /* check FEC on/off */
    fec_en   = enable & TEFMOD16_PHY_CONTROL_FEC_MASK;
    PHYMOD_IF_ERR_RETURN(
      tefmod16_FEC_control(&phy->access, fec_type, fec_en, 0));

    return PHYMOD_E_NONE;
    
}

int tscf16_phy_fec_enable_get(const phymod_phy_access_t* phy, uint32_t* enable)
{
    int fec_en;
    tefmod16_fec_type_t fec_type;

    /* first check FEC type */
    if (PHYMOD_FEC_CL91_GET(*enable)) {
        fec_type = TEFMOD16_CL91;
    } else {
        fec_type = TEFMOD16_CL74;
    }

    PHYMOD_IF_ERR_RETURN(
      tefmod16_FEC_get(&phy->access, fec_type, &fec_en));
    PHYMOD_DEBUG_VERBOSE(("FEC enable state :: %x :: fec_type :: %x\n", fec_en, fec_type));
    *enable = (uint32_t) fec_en;

    return PHYMOD_E_NONE;
}


int tscf16_phy_autoneg_oui_set(const phymod_phy_access_t* phy, phymod_autoneg_oui_t an_oui)
{
    tefmod16_an_oui_t oui;

    oui.oui                    = an_oui.oui;
    oui.oui_override_hpam_adv  = an_oui.oui_override_hpam_adv;
    oui.oui_override_hpam_det  = an_oui.oui_override_hpam_det;
    oui.oui_override_bam73_adv = an_oui.oui_override_bam73_adv;
    oui.oui_override_bam73_det = an_oui.oui_override_bam73_det;
    PHYMOD_IF_ERR_RETURN(tefmod16_an_oui_set(&phy->access, oui));

    return PHYMOD_E_NONE;
    
}

int tscf16_phy_autoneg_oui_get(const phymod_phy_access_t* phy, phymod_autoneg_oui_t* an_oui)
{
    tefmod16_an_oui_t oui;

    PHYMOD_IF_ERR_RETURN(tefmod16_an_oui_get(&phy->access, &oui));
    an_oui->oui_override_hpam_adv  = oui.oui_override_hpam_adv;
    an_oui->oui_override_hpam_det  = oui.oui_override_hpam_det;
    an_oui->oui_override_bam73_adv = oui.oui_override_bam73_adv;
    an_oui->oui_override_bam73_det = oui.oui_override_bam73_det;

    return PHYMOD_E_NONE;
    
}


int tscf16_phy_eee_set(const phymod_phy_access_t* phy, uint32_t enable)
{
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_eee_get(const phymod_phy_access_t* phy, uint32_t* enable)
{
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_interface_config_set(const phymod_phy_access_t* phy, uint32_t flags, const phymod_phy_inf_config_t* config)
{
    uint32_t current_pll_div = 0;
    uint32_t new_pll_div = 0;
    int16_t  new_os_mode =-1;
    tefmod16_spd_intfc_type_t spd_intf = TEFMOD16_SPD_ILLEGAL;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i, pll_switch=0;
    int lane_bkup, os_mode;
    enum falcon16_tsc_pll_refclk_enum refclk = FALCON16_TSC_PLL_REFCLK_156P25MHZ;
    uint32_t  vco_rate;

    phymod_firmware_lane_config_t firmware_lane_config;
    phymod_firmware_core_config_t firmware_core_config;

    PHYMOD_MEMSET(&firmware_lane_config, 0x0, sizeof(firmware_lane_config));
    PHYMOD_MEMSET(&firmware_core_config, 0x0, sizeof(firmware_core_config));

    /*next program the tx fir taps and driver current based on the input*/
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));

    /* first hold the pcs lane reset */
    tefmod16_disable_set(&phy->access);

    /*Hold the per lane PMD soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_lane_soft_reset_release(&pm_phy_copy.access, 0));
    }
    /* remove pmd_tx_disable_pin_dis it may be asserted because of ILKn */
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_pmd_tx_disable_pin_dis_set(&pm_phy_copy.access, 0));
    }

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;
    PHYMOD_IF_ERR_RETURN
        (tscf16_phy_firmware_lane_config_get(&pm_phy_copy, &firmware_lane_config));
    
    /*make sure that an and config from pcs is off*/
    firmware_core_config.CoreConfigFromPCS = 0;
    firmware_lane_config.AnEnabled = 0;
    firmware_lane_config.LaneConfigFromPCS = 0;
    firmware_lane_config.DfeOn = 1;
    firmware_lane_config.LpDfeOn = 0;
    /* enable pmd_rx_restart after 600ms if the link has faield to complete training */
    firmware_lane_config.Cl72RestTO = 1;
    firmware_lane_config.Cl72AutoPolEn = 0; 

    if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    } else if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
    } else {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
    }    
    /* this field is used only when MediaType is optical */
    if (PHYMOD_INTF_MODES_UNRELIABLE_LOS_GET(config)) {
        firmware_lane_config.UnreliableLos = 1;
    } else {
        /* los is reliable */
        firmware_lane_config.UnreliableLos = 0;
    }
    if (config->data_rate == 1000) {
       firmware_lane_config.DfeOn = 0;
    }

    PHYMOD_IF_ERR_RETURN
        (tefmod16_update_port_mode(&phy->access, (int *) &pll_switch));

    if (config->interface_type == phymodInterface1000X) {
         spd_intf = TEFMOD16_SPD_1G_20G;
    } else if (config->interface_type == phymodInterfaceSGMII) {
         spd_intf = TEFMOD16_SPD_1G_20G;
    } else if ((config->interface_type == phymodInterfaceBypass) ||
               (num_lane == 1)) {
        switch (config->data_rate) {
         case 10000:
         case 12000:
           spd_intf = TEFMOD16_SPD_10000_XFI;
           if (!PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.DfeOn = 0;
           }
           break;
         case 10600:
         case 11000:
           spd_intf = TEFMOD16_SPD_10600_XFI_HG;
           if (!PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.DfeOn = 0;
           }
           break;
         case 20000:
           spd_intf = TEFMOD16_SPD_20000_XFI;
           break;
         case 21200:
           spd_intf = TEFMOD16_SPD_21200_XFI_HG;
           break;
         case 25000:
           spd_intf = TEFMOD16_SPD_25000_XFI;
           break;
         case 27000:
           spd_intf = TEFMOD16_SPD_26500_XFI_HG;
           firmware_lane_config.DfeOn = 1;
           firmware_lane_config.LpDfeOn = 0;
            if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
           } else {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           }
           break;
         default:
           /* if the interface is pcs bypass mode and check the speed */
           if (config->interface_type == phymodInterfaceBypass) {
               /* disable DFE for lower speed */
               if (config->data_rate < 10000) {
                   firmware_lane_config.DfeOn = 0;
               }
           } else {
               spd_intf = TEFMOD16_SPD_10000_XFI;
               firmware_lane_config.DfeOn = 0;
           }
           break;
        }
    } else if ((config->interface_type == phymodInterfaceKR2) ||
               (config->interface_type == phymodInterfaceCR2) ||
               (config->interface_type == phymodInterfaceXLAUI2) ||
               (config->interface_type == phymodInterfaceRXAUI) ||
               (config->interface_type == phymodInterfaceX2) ||
               (num_lane == 2))  {
       switch (config->data_rate) {
         case 20000:
           spd_intf = TEFMOD16_SPD_20G_MLD_X2;
           if (PHYMOD_INTF_MODES_HIGIG_GET(config)) {
               firmware_lane_config.DfeOn = 0;
               firmware_lane_config.LpDfeOn = 0;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           } else if (PHYMOD_INTF_MODES_FIBER_GET(config)) { 
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
           } else if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
           } else {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           }
           break;
         case 21000:
         case 21200:
           firmware_lane_config.DfeOn = 0;
           firmware_lane_config.LpDfeOn = 0;
           spd_intf = TEFMOD16_SPD_21G_MLD_HG_X2;
           firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           break;
         case 40000:
           spd_intf = TEFMOD16_SPD_40G_MLD_X2;
           firmware_lane_config.DfeOn = 1;
           firmware_lane_config.LpDfeOn = 1;
           if (PHYMOD_INTF_MODES_FIBER_GET(config)) { 
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
           } else if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
           } else {
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           }
           break;
         case 42000:
           spd_intf = TEFMOD16_SPD_42G_MLD_HG_X2;
           firmware_lane_config.DfeOn = 1;
           firmware_lane_config.LpDfeOn = 1;
           firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           break;
         case 50000:
           spd_intf = TEFMOD16_SPD_50G_MLD_X2;
           break;
         case 53000:
           spd_intf = TEFMOD16_SPD_53G_MLD_HG_X2;
           break;
         default:
           spd_intf = TEFMOD16_SPD_20G_MLD_X2;
           break;
       }
    } else {
       switch (config->data_rate) {
         case 40000:
            spd_intf = TEFMOD16_SPD_40G_MLD_X4;
            if ((config->interface_type == phymodInterfaceXLAUI) || (PHYMOD_INTF_MODES_FIBER_GET(config))){
               firmware_lane_config.DfeOn = 0;
               firmware_lane_config.LpDfeOn = 0;
            } else {
               firmware_lane_config.DfeOn = 1;
               firmware_lane_config.LpDfeOn = 1;
               if (PHYMOD_INTF_MODES_HIGIG_GET(config)){
                   spd_intf = TEFMOD16_SPD_42G_MLD_HG_X4;
               }
           }
           break;
         case 42000:
           spd_intf = TEFMOD16_SPD_42G_MLD_HG_X4;
           firmware_lane_config.DfeOn = 1;
           firmware_lane_config.LpDfeOn = 1;
           break;
         case 48000:
           spd_intf = TEFMOD16_SPD_40G_MLD_X4;
           firmware_lane_config.DfeOn = 0;
           firmware_lane_config.LpDfeOn = 0;
           break;
         case 50000:
           spd_intf = TEFMOD16_SPD_50G_MLD_X4;
           break;
         case 53000:
           spd_intf = TEFMOD16_SPD_53G_MLD_HG_X4;
           break;
         case 100000:
           spd_intf = TEFMOD16_SPD_100G_MLD_X4;
           /* need to enable lp_dfe and set the medium type to be backplane for optical module */
           if (config->interface_type == phymodInterfaceCAUI4 || config->interface_type == phymodInterfaceCAUI) {
               firmware_lane_config.DfeOn = 1;
               firmware_lane_config.LpDfeOn = 1;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           } else if (PHYMOD_INTF_MODES_HIGIG_GET(config)) {
               spd_intf = TEFMOD16_SPD_106G_MLD_HG_X4;
               firmware_lane_config.DfeOn = 1;
               firmware_lane_config.LpDfeOn = 0;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           } else if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
               firmware_lane_config.DfeOn = 0;
               firmware_lane_config.LpDfeOn = 0;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
           } else if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
               firmware_lane_config.DfeOn = 1;
               firmware_lane_config.LpDfeOn = 0;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
           } else {
               firmware_lane_config.DfeOn = 1;
               firmware_lane_config.LpDfeOn = 0;
               firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           }
           break;
         case 106000:
           spd_intf = TEFMOD16_SPD_106G_MLD_HG_X4;
           firmware_lane_config.DfeOn = 1;
           firmware_lane_config.LpDfeOn = 0;
           firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
           break;
         default:
           spd_intf = TEFMOD16_SPD_40G_MLD_X4;
           firmware_lane_config.DfeOn = 0;
           break;
       }
    }

    /* get current pll */ 
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_INTERNAL_read_pll_div(&phy->access, &current_pll_div));
    
    if (config->interface_type == phymodInterfaceBypass) {
        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_get_pll_vco_osmode(config, &vco_rate, &new_pll_div, &new_os_mode));
    } else {
        PHYMOD_IF_ERR_RETURN
            (tefmod16_plldiv_lkup_get(&phy->access, spd_intf, config->ref_clock, &new_pll_div));

        /* For 12GG XFI , data rate specified is 12000, and pll_div is set to 160 ( 6 )  */
        if ((config->data_rate == 12000) && (num_lane == 1)) {
            if (config->ref_clock == phymodRefClk125Mhz) {
                new_pll_div = TEFMOD16_PLL_MODE_DIV_200; 
            } else {       
                new_pll_div = TEFMOD16_PLL_MODE_DIV_160;
            }     
        }
        if ((config->data_rate == 40000) && 
            PHYMOD_INTF_MODES_HIGIG_GET(config) &&
            (num_lane == 4)) {
            if (config->ref_clock == phymodRefClk125Mhz) {
                new_pll_div = TEFMOD16_PLL_MODE_DIV_165; 
           } else {
                new_pll_div = TEFMOD16_PLL_MODE_DIV_132;
           }
        }
        if ((config->data_rate == 48000) && (num_lane == 4)) {
            if (config->ref_clock == phymodRefClk125Mhz) {
                new_pll_div = TEFMOD16_PLL_MODE_DIV_200; 
            } else {       
                new_pll_div = TEFMOD16_PLL_MODE_DIV_160;
            }     
        }
        if ((config->data_rate == 100000) &&
            PHYMOD_INTF_MODES_HIGIG_GET(config) &&
            (num_lane == 4)) {
            new_pll_div = TEFMOD16_PLL_MODE_DIV_160;
        }
    }

    if (new_os_mode>=0) {
        os_mode = new_os_mode | 0x80000000 ;
    } else {
        os_mode = 0 ;
    }
    PHYMOD_IF_ERR_RETURN
        (tefmod16_pmd_osmode_set(&phy->access, spd_intf, config->ref_clock, os_mode));

    /*if pll change is enabled*/
    if ((current_pll_div != new_pll_div) && (PHYMOD_INTF_F_DONT_TURN_OFF_PLL & flags)){
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG,
                               (_PHYMOD_MSG("pll has to change for speed_set from %u to %u but DONT_TURN_OFF_PLL flag is enabled"),
                                 (unsigned int)current_pll_div, (unsigned int)new_pll_div));
    }

    /*pll switch is required and expected */
    if ((current_pll_div != new_pll_div) && !(PHYMOD_INTF_F_DONT_TURN_OFF_PLL & flags)) {

        /* Change in PLL, so reset all the ports first. */
        lane_bkup = pm_phy_copy.access.lane_mask;
        pm_phy_copy.access.lane_mask = 0xf;
        tefmod16_disable_set(&pm_phy_copy.access);
        pm_phy_copy.access.lane_mask = lane_bkup;

        /*set the PLL divider */
        if (config->ref_clock == phymodRefClk125Mhz){ 
            refclk = FALCON16_TSC_PLL_REFCLK_125MHZ;
        }
        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_core_dp_reset(&pm_phy_copy.access, 1));

        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_configure_pll_refclk_div(&pm_phy_copy.access, refclk, new_pll_div));

        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_core_dp_reset(&pm_phy_copy.access, 0));

        PHYMOD_IF_ERR_RETURN
            (tefmod16_master_port_num_set(&phy->access, start_lane));

        PHYMOD_IF_ERR_RETURN
            (tefmod16_pll_reset_enable_set(&phy->access, 1));

         
    }
    if (config->interface_type != phymodInterfaceBypass) {
      PHYMOD_IF_ERR_RETURN
            (tefmod16_set_spd_intf(&phy->access, spd_intf));
    }

    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 0x1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
             (_tscf16_phy_firmware_lane_config_set(&pm_phy_copy, firmware_lane_config));
    }

    /*release the per lne soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_lane_soft_reset_release(&pm_phy_copy.access, 1));
    }
        
    return PHYMOD_E_NONE;
    
}



STATIC
int _tscf16_speed_id_interface_config_get(const phymod_phy_access_t* phy, int speed_id, phymod_phy_inf_config_t* config)
{
    int ilkn_set;
    int osr_mode;
    int div_osr_value;
    uint32_t vco_rate;
    uint32_t pll_multiplier;
    uint32_t plldiv_r_val;

    PHYMOD_IF_ERR_RETURN(tefmod16_pcs_ilkn_chk(&phy->access, &ilkn_set));
    PHYMOD_IF_ERR_RETURN(tefmod16_get_plldiv(&phy->access, &plldiv_r_val));

    if (ilkn_set) {
        config->interface_type = phymodInterfaceBypass;
        PHYMOD_IF_ERR_RETURN(tefmod16_get_plldiv(&phy->access, &plldiv_r_val));
        PHYMOD_IF_ERR_RETURN
            (_tscf16_pll_multiplier_get(plldiv_r_val, &pll_multiplier));
        PHYMOD_IF_ERR_RETURN
            (falcon16_osr_mode_get(&phy->access, &osr_mode));

        switch (config->ref_clock) {
            case phymodRefClk156Mhz:
                vco_rate = pll_multiplier * 156 + pll_multiplier * 25 / 100;
                break;
            case phymodRefClk125Mhz:
                vco_rate = pll_multiplier * 125;
                break;
            default:
                vco_rate = pll_multiplier * 156 + pll_multiplier * 25 / 100;
                break;
        }
        div_osr_value = 1 << osr_mode;
        config->data_rate = vco_rate/div_osr_value;
    } else {
        switch (speed_id) {
        case 0x0:
            config->data_rate = 10000;
            config->interface_type = phymodInterfaceCR;
            break;
        case 0x1:
            config->data_rate = 10000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0x2:
            config->data_rate = 10000;
            config->interface_type = phymodInterfaceKR;
            /*next check the PLL divider to see if 10G or 12G */
            PHYMOD_IF_ERR_RETURN(tefmod16_get_plldiv(&phy->access, &plldiv_r_val));
            if ((config->ref_clock == phymodRefClk125Mhz)  &&
                (plldiv_r_val == 0xd)) {
                config->data_rate = 12000;
            }

            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x6)) {
                config->data_rate = 12000;
            }
            break;
        case 0x4:
            config->data_rate = 11000;
            config->interface_type = phymodInterfaceCR;
            break;
        case 0x5:
            config->data_rate = 11000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0x6:
            config->data_rate = 11000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0x8:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceCR;
            break;
        case 0x9:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0xa:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0xc:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceCR;
            break;
        case 0xd:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0xe:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceKR;
            break;
           /* 25G_CR1_MSA */
        case 0x10:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceCR;
            break;
        case 0x11:
           /* 25G_KR1_MSA */
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceKR;
            break;
           /* 25G_X1_MSA */
        case 0x12:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceKR;
            break;
           /* 25G_HG2_CR1_MSA */
        case 0x14:
            config->data_rate = 27000;
            config->interface_type = phymodInterfaceCR;
            break;
           /* 25G_HG2_KR1_MSA */
        case 0x15:
            config->data_rate = 27000;
            config->interface_type = phymodInterfaceKR;
            break;
           /* 25G_HG2_X1_MSA */
        case 0x16:
            config->data_rate = 27000;
            config->interface_type = phymodInterfaceKR;
            break;
        case 0x18:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x19:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x1a:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x1c:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x1d:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x1e:
            config->data_rate = 21000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x20:
            config->data_rate = 40000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x21:
            config->data_rate = 40000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x22:
            config->data_rate = 40000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x24:
            config->data_rate = 42000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x25:
            config->data_rate = 42000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x26:
            config->data_rate = 42000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x28:
            config->data_rate = 40000;
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x5)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x6)) {
                config->data_rate =48000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xa)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xd)) {
                config->data_rate =48000;
            }

            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x29:
            config->data_rate = 40000;
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x5)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x6)) {
                config->data_rate =48000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xa)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xd)) {
                config->data_rate =48000;
            }
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x2a:
            config->data_rate = 40000;
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x5)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x6)) {
                config->data_rate =48000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xa)) {
                config->data_rate =42000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0xd)) {
                config->data_rate =48000;
            }
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x2c:
            config->data_rate = 42000;
            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x2d:
            config->data_rate = 42000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x2e:
            config->data_rate = 42000;
            if ((config->ref_clock == phymodRefClk156Mhz) &&
                (plldiv_r_val == 0x4)) {
                config->data_rate =40000;
            }
            if ((config->ref_clock == phymodRefClk125Mhz) &&
                (plldiv_r_val == 0x7)) {
                config->data_rate =40000;
            }
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x30:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x31:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x32:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x34:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceCR2;
            break;
        case 0x35:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x36:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x38:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x39:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x3a:
            config->data_rate = 50000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x3c:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x3d:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x3e:
            config->data_rate = 53000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x40:
            config->data_rate = 100000;
            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x41:
            config->data_rate = 100000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x42:
            config->data_rate = 100000;
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x44:
            config->data_rate = 106000;
            if (plldiv_r_val == 6) {
                config->data_rate = 100000;
            }
            config->interface_type = phymodInterfaceCR4;
            break;
        case 0x45:
            config->data_rate = 106000;
            if (plldiv_r_val == 6) {
                config->data_rate = 100000;
            }
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x46:
            config->data_rate = 106000;
            if (plldiv_r_val == 6) {
                config->data_rate = 100000;
            }
            config->interface_type = phymodInterfaceKR4;
            break;
        case 0x48:
            config->data_rate = 20000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x50:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceKR2;
            break;
        case 0x58:
            config->data_rate = 1000;
            config->interface_type = phymodInterfaceSGMII;
            break;
        case 0x60:
            config->data_rate = 1000;
            config->interface_type = phymodInterfaceSGMII;
            break;
        case 0x62:
            config->data_rate = 10000;
            config->interface_type = phymodInterfaceXAUI;
            break;
            /* 25G_CR_IEEE*/
        case 0x70:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceCR;
            break;
            /* 25G_CRS_IEEE*/
        case 0x71:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceCR;
            break;
            /* 25G_KR_IEEE*/
        case 0x72:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceKR;
            break;
            /* 25G_KRS_IEEE*/
        case 0x73:
            config->data_rate = 25000;
            config->interface_type = phymodInterfaceKR;
            break;
        default:
            config->data_rate = 10000;
            config->interface_type = phymodInterfaceKR;
            break;
        }
    }
    return PHYMOD_E_NONE;
}

 

int tscf16_phy_interface_config_get(const phymod_phy_access_t* phy, uint32_t flags /*unused */, phymod_ref_clk_t ref_clock, phymod_phy_inf_config_t* config)
{
    int speed_id;
    phymod_firmware_lane_config_t firmware_lane_config;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane;

    config->ref_clock = ref_clock;
    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_IF_ERR_RETURN
        (tefmod16_speed_id_get(&phy->access, &speed_id));
    PHYMOD_IF_ERR_RETURN
        (_tscf16_speed_id_interface_config_get(phy, speed_id, config));

    /* read the current media type */
    pm_phy_copy.access.lane_mask = 0x1 << start_lane;
    PHYMOD_IF_ERR_RETURN
        (tscf16_phy_firmware_lane_config_get(&pm_phy_copy, &firmware_lane_config));
    if (firmware_lane_config.MediaType == phymodFirmwareMediaTypeOptics) {
        PHYMOD_INTF_MODES_FIBER_SET(config);
    } else if (firmware_lane_config.MediaType == phymodFirmwareMediaTypeCopperCable) {
        PHYMOD_INTF_MODES_FIBER_CLR(config);
        PHYMOD_INTF_MODES_COPPER_SET(config);
    } else {
        PHYMOD_INTF_MODES_FIBER_CLR(config);
        PHYMOD_INTF_MODES_BACKPLANE_SET(config);
    }
        /* next need to check for 40G 4 lanes mode, it's for XLAUI or KR4 */
    if ((config->data_rate == 40000) && (!(firmware_lane_config.DfeOn)) &&
         (firmware_lane_config.MediaType == phymodFirmwareMediaTypePcbTraceBackPlane)) {
        config->interface_type = phymodInterfaceXLAUI;
    }

    /* next need to check if we are CAUI4 100G mode */
    if ((config->data_rate == 100000) && (firmware_lane_config.LpDfeOn) &&
         (firmware_lane_config.MediaType == phymodFirmwareMediaTypePcbTraceBackPlane)) {
        config->interface_type = phymodInterfaceCAUI4;
    }

    switch (config->interface_type) {
      case phymodInterfaceSGMII:
        if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
            config->interface_type = phymodInterface1000X;
        } else {
            config->interface_type = phymodInterfaceSGMII;
        }
        break;
      case phymodInterfaceXFI:
        if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
            config->interface_type = phymodInterfaceCR;
        } else {
            config->interface_type = phymodInterfaceXFI;
        }
        break;
      case phymodInterfaceKR2:
        if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
            config->interface_type = phymodInterfaceSR2;
        } else {
            if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
                config->interface_type = phymodInterfaceCR2;
            } else {
                config->interface_type = phymodInterfaceKR2;
            }
        }
        break;
      case phymodInterfaceKR4:
        if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
            config->interface_type = phymodInterfaceSR4;
        } else {
            if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
                config->interface_type = phymodInterfaceCR4;
            } else {
                config->interface_type = phymodInterfaceKR4;
            }
        }
        break;
      case phymodInterfaceKR:
        {
          phymod_autoneg_control_t an_control;
          int an_complete;
          /* first check if an is enable or not */
          PHYMOD_IF_ERR_RETURN
               (phymod_phy_autoneg_get(phy, &an_control, (uint32_t *) &an_complete));
           if (!an_control.enable) {
             if (config->data_rate == 10000) {
                if ( !PHYMOD_INTF_MODES_FIBER_GET(config)) {
                    if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
                        config->interface_type = phymodInterfaceCR;
                    } else {
                        config->interface_type = phymodInterfaceXFI;
                    }
                } else {
                    config->interface_type = phymodInterfaceSFI;
                }
             } else {
                if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
                    config->interface_type = phymodInterfaceSR;
                } else {
                    if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
                        config->interface_type = phymodInterfaceCR;
                    } else {
                        config->interface_type = phymodInterfaceKR;
                    }
                }
             }
          } else {
            config->interface_type = phymodInterfaceKR;
          }
          break;
        }

      default:
        break;
    }

    return PHYMOD_E_NONE;
}


int tscf16_phy_cl72_set(const phymod_phy_access_t* phy, uint32_t cl72_en)
{
    struct falcon16_tsc_uc_lane_config_st serdes_firmware_config;
    PHYMOD_IF_ERR_RETURN(falcon16_tsc_get_uc_lane_cfg(&phy->access, &serdes_firmware_config));

    if (serdes_firmware_config.field.dfe_on == 0) {
      PHYMOD_DEBUG_ERROR(("ERROR :: DFE is off : Can not start CL72/CL93 with no DFE\n"));
      return PHYMOD_E_CONFIG;
    }

    PHYMOD_IF_ERR_RETURN
        (tefmod16_clause72_control(&phy->access, cl72_en));
        
        
    return PHYMOD_E_NONE;
    
}

int tscf16_phy_cl72_get(const phymod_phy_access_t* phy, uint32_t* cl72_en)
{
    PHYMOD_IF_ERR_RETURN
        (falcon16_clause72_control_get(&phy->access, cl72_en));
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_cl72_status_get(const phymod_phy_access_t* phy, phymod_cl72_status_t* status)
{
    uint32_t local_status;

    PHYMOD_IF_ERR_RETURN
        (falcon16_pmd_cl72_receiver_status(&phy->access, &local_status));
    status->locked = local_status;
    
       
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_autoneg_ability_set(const phymod_phy_access_t* phy, const phymod_autoneg_ability_t* an_ability)
{
         
    tefmod16_an_adv_ability_t value;
    int start_lane, num_lane;
    phymod_phy_access_t phy_copy;
    phymod_core_info_t core_info;

    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_MEMSET(&value, 0x0, sizeof(value));
    PHYMOD_MEMSET(&core_info, 0x0, sizeof(core_info));

    PHYMOD_IF_ERR_RETURN
        (tscf16_core_info_get((phymod_core_access_t *)phy, &core_info));

    value.an_cl72 = an_ability->an_cl72;

    /* check FEC/CL74/CL91 support */
    if (PHYMOD_AN_FEC_OFF_GET(an_ability->an_fec)) {
        value.an_fec = TEFMOD16_FEC_SUPRTD_NOT_REQSTD;
    } else {
        if (PHYMOD_AN_FEC_CL74_GET(an_ability->an_fec))
            value.an_fec = TEFMOD16_FEC_CL74_SUPRTD_REQSTD;
        if (PHYMOD_AN_FEC_CL91_GET(an_ability->an_fec))
            value.an_fec |= TEFMOD16_FEC_CL91_SUPRTD_REQSTD;
    }

    value.an_hg2 = an_ability->an_hg2;

    /* next check pause */
    if (PHYMOD_AN_CAP_SYMM_PAUSE_GET(an_ability) && !PHYMOD_AN_CAP_ASYM_PAUSE_GET(an_ability)) {
        value.an_pause = TEFMOD16_SYMM_PAUSE;
    }
    if (PHYMOD_AN_CAP_ASYM_PAUSE_GET(an_ability) && !PHYMOD_AN_CAP_SYMM_PAUSE_GET(an_ability)) {
        value.an_pause = TEFMOD16_ASYM_PAUSE;
    }
    if (PHYMOD_AN_CAP_ASYM_PAUSE_GET(an_ability) && PHYMOD_AN_CAP_SYMM_PAUSE_GET(an_ability)) {
        value.an_pause = TEFMOD16_ASYM_SYMM_PAUSE;
    }

        /* check cl73 and cl73 bam ability */
    if (PHYMOD_AN_CAP_1G_KX_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_1GBASE_KX;
    if (PHYMOD_AN_CAP_10G_KR_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_10GBASE_KR1;
    if (PHYMOD_AN_CAP_40G_KR4_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_40GBASE_KR4;
    if (PHYMOD_AN_CAP_40G_CR4_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_40GBASE_CR4;
    if (PHYMOD_AN_CAP_100G_KR4_GET(an_ability->an_cap)) 
        value.an_base_speed |= 1 << TEFMOD16_CL73_100GBASE_KR4;
    if (PHYMOD_AN_CAP_100G_CR4_GET(an_ability->an_cap)) 
        value.an_base_speed |= 1 << TEFMOD16_CL73_100GBASE_CR4;
 
    if (PHYMOD_AN_CAP_25G_CR1_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_25GBASE_CR1;
    if (PHYMOD_AN_CAP_25G_KR1_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_25GBASE_KR1;
    if (PHYMOD_AN_CAP_25G_CRS1_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_25GBASE_CRS1;
    if (PHYMOD_AN_CAP_25G_KRS1_GET(an_ability->an_cap))
        value.an_base_speed |= 1 << TEFMOD16_CL73_25GBASE_KRS1;

    /* check cl73 bam ability */
    if (PHYMOD_BAM_CL73_CAP_20G_KR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_20GBASE_KR2;
    if (PHYMOD_BAM_CL73_CAP_20G_CR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_20GBASE_CR2;
    if (PHYMOD_BAM_CL73_CAP_40G_KR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_40GBASE_KR2;
    if (PHYMOD_BAM_CL73_CAP_40G_CR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_40GBASE_CR2;
    if (PHYMOD_BAM_CL73_CAP_50G_KR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_50GBASE_KR2;
    if (PHYMOD_BAM_CL73_CAP_50G_CR2_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_50GBASE_CR2;
    if (PHYMOD_BAM_CL73_CAP_50G_KR4_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_50GBASE_KR4;
    if (PHYMOD_BAM_CL73_CAP_50G_CR4_GET(an_ability->cl73bam_cap))
        value.an_bam_speed |= 1 << TEFMOD16_CL73_BAM_50GBASE_CR4;

    if (PHYMOD_BAM_CL73_CAP_20G_KR1_GET(an_ability->cl73bam_cap))
        value.an_bam_speed1 |= 1 << TEFMOD16_CL73_BAM_20GBASE_KR1;
    if (PHYMOD_BAM_CL73_CAP_20G_CR1_GET(an_ability->cl73bam_cap))
        value.an_bam_speed1 |= 1 << TEFMOD16_CL73_BAM_20GBASE_CR1;
    if (PHYMOD_BAM_CL73_CAP_25G_KR1_GET(an_ability->cl73bam_cap))
        value.an_bam_speed1 |= 1 << TEFMOD16_CL73_BAM_25GBASE_KR1;
    if (PHYMOD_BAM_CL73_CAP_25G_CR1_GET(an_ability->cl73bam_cap))
        value.an_bam_speed1 |= 1 << TEFMOD16_CL73_BAM_25GBASE_CR1;

    PHYMOD_IF_ERR_RETURN
        (tefmod16_autoneg_set(&phy_copy.access, &value));

    return PHYMOD_E_NONE;
    
}

int tscf16_phy_autoneg_ability_get(const phymod_phy_access_t* phy, phymod_autoneg_ability_t* an_ability_get_type)
{
    tefmod16_an_adv_ability_t value;
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane;

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    phy_copy.access.lane_mask = 0x1 << start_lane;
    PHYMOD_MEMSET(&value, 0x0, sizeof(value));

    PHYMOD_IF_ERR_RETURN
        (tefmod16_autoneg_local_ability_get(&phy_copy.access, &value));
    an_ability_get_type->an_cl72 = value.an_cl72;
    an_ability_get_type->an_fec = value.an_fec;

    if (value.an_pause == TEFMOD16_ASYM_PAUSE) {
        PHYMOD_AN_CAP_ASYM_PAUSE_SET(an_ability_get_type);
    } else if (value.an_pause == TEFMOD16_SYMM_PAUSE) {
        PHYMOD_AN_CAP_SYMM_PAUSE_SET(an_ability_get_type);
    } else if (value.an_pause == TEFMOD16_ASYM_SYMM_PAUSE) {
        PHYMOD_AN_CAP_ASYM_PAUSE_SET(an_ability_get_type);
        PHYMOD_AN_CAP_SYMM_PAUSE_SET(an_ability_get_type);
    }

    /* first check cl73 base  ability */
    if (value.an_base_speed &  1 << TEFMOD16_CL73_100GBASE_CR4)
        PHYMOD_AN_CAP_100G_CR4_SET(an_ability_get_type->an_cap);
    if (value.an_base_speed & 1 << TEFMOD16_CL73_100GBASE_KR4)
        PHYMOD_AN_CAP_100G_KR4_SET(an_ability_get_type->an_cap);
    if (value.an_base_speed & 1 << TEFMOD16_CL73_40GBASE_CR4)
        PHYMOD_AN_CAP_40G_CR4_SET(an_ability_get_type->an_cap);
    if (value.an_base_speed & 1 << TEFMOD16_CL73_40GBASE_KR4)
        PHYMOD_AN_CAP_40G_KR4_SET(an_ability_get_type->an_cap);
    if (value.an_base_speed & 1 << TEFMOD16_CL73_10GBASE_KR1)
        PHYMOD_AN_CAP_10G_KR_SET(an_ability_get_type->an_cap);
    if (value.an_base_speed & 1 << TEFMOD16_CL73_1GBASE_KX)
        PHYMOD_AN_CAP_1G_KX_SET(an_ability_get_type->an_cap);

    /* next check cl73 bam ability */
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_20GBASE_KR2)
        PHYMOD_BAM_CL73_CAP_20G_KR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_20GBASE_CR2)
        PHYMOD_BAM_CL73_CAP_20G_CR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_40GBASE_KR2)
        PHYMOD_BAM_CL73_CAP_40G_KR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_40GBASE_CR2)
        PHYMOD_BAM_CL73_CAP_40G_CR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_50GBASE_KR2)
        PHYMOD_BAM_CL73_CAP_50G_KR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_50GBASE_CR2)
        PHYMOD_BAM_CL73_CAP_50G_CR2_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_50GBASE_KR4)
        PHYMOD_BAM_CL73_CAP_50G_KR4_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed & 1 << TEFMOD16_CL73_BAM_50GBASE_CR4)
        PHYMOD_BAM_CL73_CAP_50G_CR4_SET(an_ability_get_type->cl73bam_cap);

    if (value.an_bam_speed1 & 1 << TEFMOD16_CL73_BAM_20GBASE_KR1)
        PHYMOD_BAM_CL73_CAP_20G_KR1_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed1 & 1 << TEFMOD16_CL73_BAM_20GBASE_CR1)
        PHYMOD_BAM_CL73_CAP_20G_CR1_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed1 & 1 << TEFMOD16_CL73_BAM_25GBASE_KR1)
        PHYMOD_BAM_CL73_CAP_25G_KR1_SET(an_ability_get_type->cl73bam_cap);
    if (value.an_bam_speed1 & 1 << TEFMOD16_CL73_BAM_25GBASE_CR1)
        PHYMOD_BAM_CL73_CAP_25G_CR1_SET(an_ability_get_type->cl73bam_cap);
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_autoneg_remote_ability_get(const phymod_phy_access_t* phy, phymod_autoneg_ability_t* an_ability_get_type)
{
    tefmod16_an_adv_ability_t value;

    PHYMOD_MEMSET(&value, 0x0, sizeof(value));
    PHYMOD_IF_ERR_RETURN
       (tefmod16_autoneg_lp_status_get(&phy->access, &value));

      /* cl73 technology abilities A0 - D21 */
    if (value.an_base_speed & ( 1 << TEFMOD16_CL73_1GBASE_KX)){
        PHYMOD_AN_CAP_1G_KX_SET(an_ability_get_type->an_cap);
    }

    /* cl73 technology abilities A2 - D23 */
    if (value.an_base_speed & (1 << TEFMOD16_CL73_10GBASE_KR1)){
        PHYMOD_AN_CAP_10G_KR_SET(an_ability_get_type->an_cap);
    }
    /* cl73 technology abilities A3 - D24 */
    if (value.an_base_speed & ( 1 << TEFMOD16_CL73_40GBASE_KR4)){
        PHYMOD_AN_CAP_40G_KR4_SET(an_ability_get_type->an_cap);
    }
    /* cl73 technology abilities A4 - D25 */
    if (value.an_base_speed & ( 1 << TEFMOD16_CL73_40GBASE_CR4)){
        PHYMOD_AN_CAP_40G_CR4_SET(an_ability_get_type->an_cap);
    }
    /* cl73 technology abilities A7 - D28 */
    if (value.an_base_speed & (1 << TEFMOD16_CL73_100GBASE_KR4)){
        PHYMOD_AN_CAP_100G_KR4_SET(an_ability_get_type->an_cap);
    }
    /* cl73 technology abilities A8 - D29 */
    if (value.an_base_speed & (1 << TEFMOD16_CL73_100GBASE_CR4)){
        PHYMOD_AN_CAP_100G_CR4_SET(an_ability_get_type->an_cap);
    }

    /* FEC[F0:F1] is encoded in bits [D46:D47] */
    an_ability_get_type->an_fec = value.an_fec; 

    /* advertise pause capabilities C[0:1] - D[10:11]*/
    if (value.an_pause & TEFMOD16_SYMM_PAUSE) {
        PHYMOD_AN_CAP_SYMM_PAUSE_SET(an_ability_get_type);
    }
    if (value.an_pause & TEFMOD16_ASYM_PAUSE) {
       PHYMOD_AN_CAP_ASYM_PAUSE_SET(an_ability_get_type);
    }

    /* BAM73 OUI UP bit 16 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_20GBASE_KR2)) {
        PHYMOD_BAM_CL73_CAP_20G_KR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 17 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_20GBASE_CR2)) {
        PHYMOD_BAM_CL73_CAP_20G_CR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 18 */
    if (value.an_bam_speed1 & TEFMOD16_CL73_BAM_20GBASE_KR1) {
        PHYMOD_BAM_CL73_CAP_20G_KR1_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 19 */
    if (value.an_bam_speed1 & TEFMOD16_CL73_BAM_20GBASE_CR1) {
        PHYMOD_BAM_CL73_CAP_20G_CR1_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 20 */
    if (value.an_bam_speed & TEFMOD16_CL73_BAM_25GBASE_KR1) {
        PHYMOD_BAM_CL73_CAP_25G_KR1_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 21 */
    if (value.an_bam_speed & TEFMOD16_CL73_BAM_25GBASE_CR1) {
        PHYMOD_BAM_CL73_CAP_25G_CR1_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 22 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_40GBASE_KR2)) {
        PHYMOD_BAM_CL73_CAP_40G_KR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 23 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_40GBASE_CR2)) {
        PHYMOD_BAM_CL73_CAP_40G_CR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 24 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_50GBASE_KR2)) {
        PHYMOD_BAM_CL73_CAP_50G_KR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 25 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_50GBASE_CR2)) {
        PHYMOD_BAM_CL73_CAP_50G_CR2_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 32 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_50GBASE_KR4)) {
        PHYMOD_BAM_CL73_CAP_50G_KR4_SET(an_ability_get_type->cl73bam_cap);
    }
    /* BAM73 OUI UP bit 33 */
    if (value.an_bam_speed & (1 << TEFMOD16_CL73_BAM_50GBASE_CR4)) {
        PHYMOD_BAM_CL73_CAP_50G_CR4_SET(an_ability_get_type->cl73bam_cap);
    }

    return PHYMOD_E_NONE;
    
}


int tscf16_phy_autoneg_set(const phymod_phy_access_t* phy, const phymod_autoneg_control_t* an)
{
    int num_lane_adv_encoded;
    int start_lane, num_lane;
    int i, an_enable_bitmap, single_port_mode = 0;
    int do_lane_config_set, do_core_config_set;
    phymod_firmware_lane_config_t firmware_lane_config;
    phymod_firmware_core_config_t firmware_core_config_tmp;
    tefmod16_an_control_t an_control;
    phymod_phy_access_t phy_copy;

    PHYMOD_MEMSET(&firmware_lane_config, 0x0, sizeof(firmware_lane_config));
    PHYMOD_MEMSET(&firmware_core_config_tmp, 0x0, sizeof(firmware_core_config_tmp));

    PHYMOD_MEMSET(&an_control, 0x0, sizeof(an_control));
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    if (PHYMOD_AN_F_ALLOW_PLL_CHANGE_GET(an)){
        single_port_mode = 1;
    }

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    phy_copy.access.lane_mask = 0x1 << start_lane;

    switch (an->num_lane_adv) {
        case 1:
            num_lane_adv_encoded = 0;
            break;
        case 2:
            num_lane_adv_encoded = 1;
            break;
        case 4:
            num_lane_adv_encoded = 2;
            break;
        case 10:
            num_lane_adv_encoded = 3;
            break;
        default:
            return PHYMOD_E_PARAM;
    }
    
   /* first check if cl72 is allowed to change */
    if (PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE_GET(an)) {
        an_control.cl72_config_allowed = 1;
    } else {
        an_control.cl72_config_allowed = 0;
    }

    if (PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE_GET(an)) {
        an_control.pd_kx_en = 1 ;
    }
    an_control.num_lane_adv = num_lane_adv_encoded;
    an_control.enable       = an->enable;
    an_control.an_property_type = 0x0;   /* for now disable */
    switch (an->an_mode) {
    case phymod_AN_MODE_CL73:
        an_control.an_type = TEFMOD16_AN_MODE_CL73;
        break;
    case phymod_AN_MODE_CL73BAM:
        an_control.an_type = TEFMOD16_AN_MODE_CL73BAM;
        break;
    case phymod_AN_MODE_HPAM:
        an_control.an_type = TEFMOD16_AN_MODE_HPAM;
        break;
    default:
        an_control.an_type = TEFMOD16_AN_MODE_CL73;
        break;
    }

    tefmod16_disable_set(&phy->access);

    if (single_port_mode) {
        PHYMOD_IF_ERR_RETURN
            (tefmod16_set_an_single_port_mode(&phy->access, an->enable));
    }
    /* first check if any other lane has An on */
    an_enable_bitmap = 0;
    if (!an->enable) {
        for (i = 0; i < 4; i++ ) {
            phy_copy.access.lane_mask = 0x1 << i;
            if (phy_copy.access.lane_mask & phy->access.lane_mask)  continue;
            PHYMOD_IF_ERR_RETURN
                (tscf16_phy_firmware_lane_config_get(&phy_copy, &firmware_lane_config));
            if (firmware_lane_config.AnEnabled) {
                an_enable_bitmap |= 1 << i;
            }
        }

    }

    phy_copy.access.lane_mask = 0x1 << start_lane;

    /* make sure the firmware config is set to an enabled */
    PHYMOD_IF_ERR_RETURN
        (tscf16_phy_firmware_lane_config_get(&phy_copy, &firmware_lane_config));
    /* make sure the firmware config is set to an enabled */
    PHYMOD_IF_ERR_RETURN
        (tscf16_phy_firmware_core_config_get(&phy_copy, &firmware_core_config_tmp));
    do_lane_config_set = 0;
    do_core_config_set = 0;

    if (an->enable) {
        if (firmware_lane_config.AnEnabled != 1) {
          firmware_lane_config.AnEnabled = 1;
          do_lane_config_set = 1;
        }
        if (firmware_lane_config.LaneConfigFromPCS != 1) {
          firmware_lane_config.LaneConfigFromPCS = 1;
          do_lane_config_set = 1;
        }
        if (firmware_core_config_tmp.CoreConfigFromPCS != 1) {
          firmware_core_config_tmp.CoreConfigFromPCS = 1;
          do_core_config_set = 1;
        }
        firmware_lane_config.Cl72RestTO = 0;
    } else {
        if (firmware_lane_config.AnEnabled != 0) {
          firmware_lane_config.AnEnabled = 0;
          do_lane_config_set = 1;
        }
        if (firmware_lane_config.LaneConfigFromPCS != 0) {
          firmware_lane_config.LaneConfigFromPCS = 0;
          do_lane_config_set = 1;
        }
        if (firmware_core_config_tmp.CoreConfigFromPCS != 0) {
            if (!an_enable_bitmap) {
                firmware_core_config_tmp.CoreConfigFromPCS = 0;
                do_core_config_set = 1;
            }
        }
        firmware_lane_config.Cl72RestTO = 1;
    }

    if (do_core_config_set && single_port_mode) {
        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_core_dp_reset(&phy_copy.access, 1));
        PHYMOD_USLEEP(1000);

        PHYMOD_IF_ERR_RETURN
            (tscf16_phy_firmware_core_config_set(&phy_copy, firmware_core_config_tmp));

        PHYMOD_IF_ERR_RETURN
            (falcon16_tsc_core_dp_reset(&phy_copy.access, 0));
    }

    if (do_lane_config_set) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            phy_copy.access.lane_mask = 0x1 << (i + start_lane);
            PHYMOD_IF_ERR_RETURN
                (falcon16_lane_soft_reset_release(&phy_copy.access, 0));
        }
        PHYMOD_USLEEP(1000);
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            phy_copy.access.lane_mask = 0x1 << (i + start_lane);
            PHYMOD_IF_ERR_RETURN
                (_tscf16_phy_firmware_lane_config_set(&phy_copy, firmware_lane_config));
        }

        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            phy_copy.access.lane_mask = 0x1 << (i + start_lane);
            PHYMOD_IF_ERR_RETURN
                (falcon16_lane_soft_reset_release(&phy_copy.access, 1));
        }
    }

    if (!an->enable) {
        tefmod16_enable_set(&phy->access);
    }

    phy_copy.access.lane_mask = 0x1 << start_lane;

    if (an->enable && single_port_mode) {
        PHYMOD_IF_ERR_RETURN
            (tefmod16_master_port_num_set(&phy_copy.access, start_lane));
    }

    PHYMOD_IF_ERR_RETURN
        (tefmod16_autoneg_control(&phy_copy.access, &an_control));

    return PHYMOD_E_NONE;
    
}

int tscf16_phy_autoneg_get(const phymod_phy_access_t* phy, phymod_autoneg_control_t* an, uint32_t* an_done)
{
    tefmod16_an_control_t an_control;
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane;
    int an_complete = 0;

    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_MEMSET(&an_control, 0x0,  sizeof(tefmod16_an_control_t));
    PHYMOD_IF_ERR_RETURN
        (tefmod16_autoneg_control_get(&phy_copy.access, &an_control, &an_complete));

    if (an_control.enable) {
        an->enable = 1;
        *an_done = an_complete;
    } else {
        an->enable = 0;
    }

    switch (an_control.an_type) {
    case TEFMOD16_AN_MODE_CL73:
        an->an_mode = phymod_AN_MODE_CL73;
        break;
    case TEFMOD16_AN_MODE_CL73BAM:
        an->an_mode = phymod_AN_MODE_CL73BAM;
        break;
    case TEFMOD16_AN_MODE_HPAM:
        an->an_mode = phymod_AN_MODE_HPAM;
        break;
    default:
        an->an_mode = phymod_AN_MODE_NONE;
        break;
    }

    return PHYMOD_E_NONE;
    
}


int tscf16_phy_autoneg_status_get(const phymod_phy_access_t* phy, phymod_autoneg_status_t* status)
{
    int speed_id, an_en, an_done;
    phymod_phy_inf_config_t config;
    phymod_ref_clk_t ref_clock;
    const phymod_access_t *pm_acc = &phy->access;

    PHYMOD_IF_ERR_RETURN
       (tefmod16_autoneg_status_get(&phy->access, &an_en, &an_done));

    PHYMOD_IF_ERR_RETURN
       (tefmod16_speed_id_get(&phy->access, &speed_id));

    PHYMOD_IF_ERR_RETURN
       (tefmod16_refclk_get(pm_acc, &ref_clock));

    config.ref_clock = ref_clock;
    PHYMOD_IF_ERR_RETURN
       (_tscf16_speed_id_interface_config_get(phy, speed_id, &config));

    status->enabled   = an_en;
    status->locked    = an_done;
    status->data_rate = config.data_rate;
    status->interface = config.interface_type;      
            
    return PHYMOD_E_NONE;
}


/* Core initialization
 * 1. De-assert PMD core and PMD lane reset
 * 2. Set heartbeat for comclk
 * 3. Configure lane mapping
 * 4. Micro code load and verify
 * 5. De-assert micro reset
 * 6. Wait for uc_active = 1
 * 7. Initialize software information table for the micro
 * 8. PLL configuration
 * 9. Release core DP soft reset
 */
STATIC
int _tscf16_core_init_pass1(const phymod_core_access_t* core, const phymod_core_init_config_t* init_config, const phymod_core_status_t* core_status)
{
    int rv, lane;
    phymod_phy_access_t phy_access, phy_access_copy;
    phymod_core_access_t  core_copy;
    uint32_t uc_enable = 0;

    TSCF16_CORE_TO_PHY_ACCESS(&phy_access, core);
    PHYMOD_MEMCPY(&core_copy, core, sizeof(core_copy));
    core_copy.access.lane_mask = 0x1;

    phy_access_copy = phy_access;
    phy_access_copy.access = core->access;
    phy_access_copy.access.lane_mask = 0x1;
    phy_access_copy.type = core->type;

    /* 1. De-assert PMD core power and core data path reset */
    PHYMOD_IF_ERR_RETURN
        (tefmod16_pmd_reset_seq(&core_copy.access, core_status->pmd_active));

    /* De-assert PMD lane reset */
    for (lane = 0; lane < TSCF16_NOF_LANES_IN_CORE; lane++) {
     phy_access.access.lane_mask = 1 << lane;
    PHYMOD_IF_ERR_RETURN
        (tefmod16_pmd_x4_reset(&phy_access.access));
    }

    PHYMOD_IF_ERR_RETURN
        (falcon16_uc_active_get(&phy_access.access, &uc_enable)); 
    if (uc_enable) return PHYMOD_E_NONE;

    /* 2. Set the heart beat, default is for 156.25M */
    if (init_config->interface.ref_clock == phymodRefClk125Mhz) {
        PHYMOD_IF_ERR_RETURN
            (tefmod16_refclk_set(&core_copy.access, phymodRefClk125Mhz));
    }

    /* 3. Configure lane mapping */
    PHYMOD_IF_ERR_RETURN
        (tscf16_core_lane_map_set(&core_copy, &init_config->lane_map));

    /* 4. Micro code load and verify */
    rv = _tscf16_core_firmware_load(&core_copy, init_config->firmware_load_method, init_config->firmware_loader);
    if (rv != PHYMOD_E_NONE) {
        PHYMOD_DEBUG_ERROR(("devad 0x%"PRIx32" lane 0x%"PRIx32": UC firmware-load failed\n", core->access.addr, core->access.lane_mask));
        PHYMOD_IF_ERR_RETURN(rv);
    }

     /* need to check if the ucode load is correct or not */
#ifndef TSCF16_PMD_CRC_UCODE_VERIFY
    if (init_config->firmware_load_method != phymodFirmwareLoadMethodNone) {
        rv = falcon16_tsc_ucode_load_verify(&core_copy.access, (uint8_t *) &falcon16_ucode, falcon16_ucode_len);
        if (rv != PHYMOD_E_NONE) {
            PHYMOD_DEBUG_ERROR(("devad 0x%x lane 0x%x: UC load-verify failed\n", core->access.addr, core->access.lane_mask));
            PHYMOD_IF_ERR_RETURN(rv);
        }
    }
#endif

    /* 5. De-assert micro reset */
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_uc_reset(&core_copy.access, 0));

    /* 6. Wait for uc_active = 1 */
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_wait_uc_active(&phy_access.access));

    /* pmd lane hard reset */
    PHYMOD_IF_ERR_RETURN
        (falcon16_pmd_ln_h_rstb_pkill_override( &phy_access_copy.access, 0x1));

    /* 7. Initialize software information table for the micro */
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_init_falcon16_tsc_info(&core_copy.access));

    if (PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY_GET(init_config)) {
#ifdef TSCF16_PMD_CRC_UCODE_VERIFY
        PHYMOD_IF_ERR_RETURN(
            falcon16_tsc_start_ucode_crc_calc(&core_copy.access, falcon16_ucode_len));
#endif
    }

    return PHYMOD_E_NONE;
}


STATIC
int _tscf16_core_init_pass2(const phymod_core_access_t* core, const phymod_core_init_config_t* init_config, const phymod_core_status_t* core_status)
{
    phymod_phy_access_t phy_access, phy_access_copy;
    phymod_core_access_t  core_copy;
    enum falcon16_tsc_pll_refclk_enum refclk;
    enum falcon16_tsc_pll_div_enum div;

    TSCF16_CORE_TO_PHY_ACCESS(&phy_access, core);
    phy_access_copy = phy_access;
    PHYMOD_MEMCPY(&core_copy, core, sizeof(core_copy));
    core_copy.access.lane_mask = 0x1;
    phy_access_copy = phy_access;
    phy_access_copy.access = core->access;
    phy_access_copy.access.lane_mask = 0x1;
    phy_access_copy.type = core->type;

    refclk = FALCON16_TSC_PLL_REFCLK_156P25MHZ;
    div = FALCON16_TSC_PLL_DIV_175;

    if (PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY_GET(init_config)) { 
#ifndef TSCF16_PMD_CRC_UCODE
        /* poll the ready bit in 10 ms */
        enum srds_pmd_uc_cmd_enum cmd; 
        cmd = CMD_CALC_CRC;
        PHYMOD_IF_ERR_RETURN(
            falcon16_tsc_INTERNAL_poll_uc_dsc_ready_for_cmd_equals_1(&core_copy.access, 100, cmd));
#else
        PHYMOD_IF_ERR_RETURN(
            falcon16_tsc_check_ucode_crc(&core_copy.access, falcon16_ucode_crc, 200));
#endif
    }

    /* release pmd lane hard reset */
    PHYMOD_IF_ERR_RETURN(
        falcon16_pmd_ln_h_rstb_pkill_override( &phy_access_copy.access, 0x0));

    /* set charge pump current */
    PHYMOD_IF_ERR_RETURN
        (falcon16_afe_pll_reg_set(&core_copy.access, &init_config->afe_pll));

    /* 8. PLL configuration */
    if (init_config->interface.ref_clock == phymodRefClk125Mhz) {
        refclk = FALCON16_TSC_PLL_REFCLK_125MHZ;
    } 

    /* reset core DP */
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_core_dp_reset(&core_copy.access, 1));

    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_configure_pll_refclk_div(&core_copy.access, refclk, div)); 

    /* select tsc_clk_credit */
    PHYMOD_IF_ERR_RETURN
        (tefmod16_default_init(&core->access)); 
   
    PHYMOD_IF_ERR_RETURN
        (tefmod16_autoneg_timer_init(&core->access));

    PHYMOD_IF_ERR_RETURN
        (tefmod16_master_port_num_set(&core->access, 0));

    PHYMOD_IF_ERR_RETURN
        (tefmod16_cl74_chng_default (&core_copy.access));

    /* 9. Release core DP soft reset */
    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_core_dp_reset(&core_copy.access, 0));

   return PHYMOD_E_NONE;  

}

int tscf16_core_init(const phymod_core_access_t* core, const phymod_core_init_config_t* init_config, const phymod_core_status_t* core_status)
{
    
    if ( (!PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config) &&
          !PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) ||
        PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config)) {
        PHYMOD_IF_ERR_RETURN
            (_tscf16_core_init_pass1(core, init_config, core_status));

        if (PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config)) {
            return PHYMOD_E_NONE;
        }
    }

    if ( (!PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config) &&
          !PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) ||
        PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) {
        PHYMOD_IF_ERR_RETURN
            (_tscf16_core_init_pass2(core, init_config, core_status));
    }
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_pll_multiplier_get(const phymod_phy_access_t* phy, uint32_t* core_vco_pll_multiplier)
{
    phymod_phy_access_t pm_phy_copy;
    uint32_t pll_mode = 0;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    pm_phy_copy.access.lane_mask = 0x1;

    PHYMOD_IF_ERR_RETURN
        (falcon16_pll_mode_get(&pm_phy_copy.access, &pll_mode));
    _tscf16_pll_multiplier_get(pll_mode, core_vco_pll_multiplier);

    return PHYMOD_E_NONE;
    
}


int tscf16_phy_init(const phymod_phy_access_t* phy, const phymod_phy_init_config_t* init_config)
{
    int pll_restart = 0;
    const phymod_access_t *pm_acc = &phy->access;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i;
    int lane_bkup;
    phymod_polarity_t tmp_pol;
    phymod_firmware_lane_config_t firmware_lane_config;

    PHYMOD_MEMSET(&tmp_pol, 0x0, sizeof(tmp_pol));
    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    PHYMOD_MEMSET(&firmware_lane_config, 0x0, sizeof(firmware_lane_config));

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(pm_acc, &start_lane, &num_lane));
    /* per lane based reset release */
    PHYMOD_IF_ERR_RETURN
        (tefmod16_pmd_x4_reset(pm_acc));

    lane_bkup = pm_phy_copy.access.lane_mask;
    for (i = 0; i < num_lane; i++) {
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (falcon16_lane_soft_reset_release(&pm_phy_copy.access, 1));
    }
    pm_phy_copy.access.lane_mask = lane_bkup;
 
    /* clearing all the lane config */
    PHYMOD_MEMSET(&firmware_lane_config, 0x0, sizeof(firmware_lane_config));

    /* program the rx/tx polarity */
    for (i = 0; i < num_lane; i++) {
        pm_phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        tmp_pol.tx_polarity = (init_config->polarity.tx_polarity) >> i & 0x1;
        tmp_pol.rx_polarity = (init_config->polarity.rx_polarity) >> i & 0x1;
        PHYMOD_IF_ERR_RETURN
            (tscf16_phy_polarity_set(&pm_phy_copy, &tmp_pol));
    }

    for (i = 0; i < num_lane; i++) {
        pm_phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        /* set tx parameters */
        PHYMOD_IF_ERR_RETURN
            (tscf16_phy_tx_set(&pm_phy_copy, &init_config->tx[i]));
    }

    for (i = 0; i < num_lane; i++) {
        pm_phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        PHYMOD_IF_ERR_RETURN
             (_tscf16_phy_firmware_lane_config_set(&pm_phy_copy, firmware_lane_config));
    }

    /* next check if pcs-bypass mode  */
    if (PHYMOD_DEVICE_OP_MODE_PCS_BYPASS_GET(phy->device_op_mode)) {
        PHYMOD_IF_ERR_RETURN
            (falcon16_pmd_tx_disable_pin_dis_set(&phy->access, 1));
        PHYMOD_IF_ERR_RETURN
          (tefmod16_init_pcs_ilkn(&phy->access));
    }

    PHYMOD_IF_ERR_RETURN
        (tefmod16_update_port_mode(pm_acc, &pll_restart));

    PHYMOD_IF_ERR_RETURN
        (tefmod16_rx_lane_control_set(pm_acc, 1));
    PHYMOD_IF_ERR_RETURN
        (tefmod16_tx_lane_control_set(pm_acc, TEFMOD16_TX_LANE_RESET_TRAFFIC_ENABLE));         /* TX_LANE_CONTROL */

    return PHYMOD_E_NONE;
    
}


int tscf16_phy_loopback_set(const phymod_phy_access_t* phy, phymod_loopback_mode_t loopback, uint32_t enable)
{
    int i;
    int start_lane, num_lane;
    int rv = PHYMOD_E_NONE;
    uint32_t cl72_en;
    phymod_phy_access_t phy_copy;

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /* next figure out the lane num and start_lane based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    switch (loopback) {
    case phymodLoopbackGlobal :
        PHYMOD_IF_ERR_RETURN
            (tscf16_phy_cl72_get(phy, &cl72_en));
        if (cl72_en == 1) {
             PHYMOD_DEBUG_ERROR(("adr=%0x,lane 0x%x: Error! pcs gloop not supported with cl72 enabled\n",  phy_copy.access.addr, start_lane));
             break;
        }
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_loopback_control(&phy->access, enable, start_lane, num_lane));
        break;
    case phymodLoopbackGlobalPMD :
        PHYMOD_IF_ERR_RETURN
            (tscf16_phy_cl72_get(phy, &cl72_en));
        if (cl72_en == 1) {
             PHYMOD_DEBUG_ERROR(("adr=%0x,lane 0x%x: Error! pmd gloop not supported with cl72 enabled\n",  phy_copy.access.addr, start_lane));
             break;
        }
            for (i = 0; i < num_lane; i++) {
            phy_copy.access.lane_mask = 0x1 << (i + start_lane);
            PHYMOD_IF_ERR_RETURN(falcon16_tsc_dig_lpbk(&phy_copy.access, (uint8_t) enable));
            PHYMOD_IF_ERR_RETURN(falcon16_pmd_force_signal_detect(&phy_copy.access, (int) enable));
            PHYMOD_IF_ERR_RETURN(tefmod16_rx_lane_control_set(&phy->access, 1));
        }
        break;
    case phymodLoopbackRemotePMD :
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_rmt_lpbk(&phy->access, (uint8_t)enable));
        break;
    case phymodLoopbackRemotePCS :
        /* PHYMOD_IF_ERR_RETURN(tefmod16_rx_loopback_control(&phy->access, enable, enable, enable)); */ /* RAVI */
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG,
                               (_PHYMOD_MSG("PCS Remote LoopBack not supported")));
        break;
    default :
        break;
    }

    return rv;
}

int tscf16_phy_loopback_get(const phymod_phy_access_t* phy, phymod_loopback_mode_t loopback, uint32_t* enable)
{
    uint32_t enable_core;
    int start_lane, num_lane;

    *enable = 0;

    /* next figure out the lane num and start_lane based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    switch (loopback) {
    case phymodLoopbackGlobal :
        PHYMOD_IF_ERR_RETURN(tefmod16_tx_loopback_get(&phy->access, &enable_core));
        *enable = (enable_core >> start_lane) & 0x1;
        break;
    case phymodLoopbackGlobalPMD :
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_dig_lpbk_get(&phy->access, enable));
        break;
    case phymodLoopbackRemotePMD :
        PHYMOD_IF_ERR_RETURN(falcon16_tsc_rmt_lpbk_get(&phy->access, enable));
        break;
    case phymodLoopbackRemotePCS :
        /* PHYMOD_IF_ERR_RETURN(tefmod16_rx_loopback_control(&phy->access, enable, enable, enable)); */ /* RAVI */
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG,
                               (_PHYMOD_MSG("PCS Remote LoopBack not supported")));
        break;
    default :
        break;
    }
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_rx_pmd_locked_get(const phymod_phy_access_t* phy, uint32_t* rx_pmd_locked)
{
    PHYMOD_IF_ERR_RETURN(tefmod16_pmd_lock_get(&phy->access, rx_pmd_locked)); 
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_link_status_get(const phymod_phy_access_t* phy, uint32_t* link_status)
{
    if (1) {
        PHYMOD_IF_ERR_RETURN(tefmod16_get_pcs_latched_link_status(&phy->access, link_status));
    } else {
        PHYMOD_IF_ERR_RETURN(tefmod16_get_pcs_link_status(&phy->access, link_status));
    }
    
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_pcs_userspeed_set(const phymod_phy_access_t* phy, const phymod_pcs_userspeed_config_t* config)
{
        
    return PHYMOD_E_UNAVAIL;    
    
}

int tscf16_phy_pcs_userspeed_get(const phymod_phy_access_t* phy, phymod_pcs_userspeed_config_t* config)
{
    return PHYMOD_E_UNAVAIL;    

}


int tscf16_phy_reg_read(const phymod_phy_access_t* phy, uint32_t reg_addr, uint32_t* val)
{
    PHYMOD_IF_ERR_RETURN(phymod_tsc_iblk_read(&phy->access, reg_addr, val));    
        
    return PHYMOD_E_NONE;
    
}


int tscf16_phy_reg_write(const phymod_phy_access_t* phy, uint32_t reg_addr, uint32_t val)
{
    PHYMOD_IF_ERR_RETURN(phymod_tsc_iblk_write(&phy->access, reg_addr, val));
        
    return PHYMOD_E_NONE;
    
}

#if 0
int tscf16_phy_eye_margin_est_get(const phymod_phy_access_t* phy, phymod_eye_margin_mode_t eye_margin_mode, uint32_t* value)
{
    int start_lane, num_lane;
    phymod_phy_access_t phy_copy;
    int hz_l, hz_r, vt_u, vt_d;


    /* first get the start_lane */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));
    phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_IF_ERR_RETURN
        (falcon16_tsc_get_eye_margin_est(&phy_copy.access, &hz_l, &hz_r, &vt_u, &vt_d));

    switch (eye_margin_mode) {
    case phymod_eye_marign_HZ_L:
        *value = hz_l;
        break;
    case phymod_eye_marign_HZ_R:
        *value = hz_r;
        break;
    case phymod_eye_marign_VT_U:
        *value = vt_u;
        break;
    case phymod_eye_marign_VT_D:
        *value = vt_d;
        break;
    default:
        *value = 0;
        break;
    }

    return PHYMOD_E_NONE;
}
#endif

int tscf16_phy_rescal_set(const phymod_phy_access_t* phy, uint32_t enable, uint32_t value)
{
    PHYMOD_IF_ERR_RETURN(falcon16_rescal_val_set(&phy->access, enable, value));

    return PHYMOD_E_NONE;

}


int tscf16_phy_rescal_get(const phymod_phy_access_t* phy, uint32_t* value)
{
    PHYMOD_IF_ERR_RETURN(falcon16_rescal_val_get(&phy->access, value));

    return PHYMOD_E_NONE;

}


