/*
 *         
 * $Id: tsce16.c,v 1 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 */

#include <phymod/phymod.h>
#include <phymod/phymod_debug.h>
#include <phymod/phymod_util.h>
#include <phymod/chip/bcmi_tsce16_xgxs_defs.h>
#include <phymod/chip/tsce16.h>
#include "tsce16/tier1/temod16_enum_defines.h"
#include "tsce16/tier1/temod16.h"
#include "tsce16/tier1/te16PCSRegEnums.h"
#include "merlin16/tier1/merlin16_cfg_seq.h"  
#include "merlin16/tier1/merlin16_common.h" 
#include "merlin16/tier1/merlin16_interface.h" 
#include "merlin16/tier1/merlin16_debug_functions.h" 
#include "merlin16/tier1/merlin16_dependencies.h" 
#include "merlin16/tier1/merlin16_internal.h"

#define TSCE16_ID0          0x600d
#define TSCE16_ID1          0x8770

#define TSCE16_MODEL        0x12
#define TSCE16_TECH_PROC    0x4     /* 16nm */

#define TSCE16_NOF_LANES_IN_CORE (4)
#define TSCE16_PHY_ALL_LANES (0xf)
#define TSCE16_CORE_TO_PHY_ACCESS(_phy_access, _core_access) \
    do{\
        PHYMOD_MEMCPY(&(_phy_access)->access, &(_core_access)->access, sizeof((_phy_access)->access));\
        (_phy_access)->type           = (_core_access)->type; \
        (_phy_access)->port_loc       = (_core_access)->port_loc; \
        (_phy_access)->device_op_mode = (_core_access)->device_op_mode; \
        (_phy_access)->access.lane_mask = TSCE16_PHY_ALL_LANES; \
    }while(0)

#define TSCE16_PMD_CRC_UCODE  1
/* uController's firmware */
extern unsigned char merlin16_ucode[];
extern unsigned short merlin16_ucode_ver;
extern unsigned short merlin16_ucode_crc;
extern unsigned short merlin16_ucode_len;

typedef int (*sequncer_control_f)(const phymod_access_t* core, uint32_t enable);
typedef int (*rx_DFE_tap_control_set_f)(const phymod_access_t* phy, uint32_t val);
extern int tsce16_phy_interface_config_get(const phymod_phy_access_t* phy, uint32_t flags, phymod_ref_clk_t ref_clock, phymod_phy_inf_config_t* config);


STATIC
int _tsce16_phy_firmware_lane_config_set(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t fw_config)
{
    struct merlin16_uc_lane_config_st serdes_firmware_config;
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane, i;
    /* uint32_t rst_status; */
    uint32_t is_warm_boot;

    PHYMOD_MEMSET(&serdes_firmware_config, 0x0, sizeof(serdes_firmware_config));
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        serdes_firmware_config.field.lane_cfg_from_pcs = fw_config.LaneConfigFromPCS;
        serdes_firmware_config.field.an_enabled        = fw_config.AnEnabled;
        serdes_firmware_config.field.dfe_on            = fw_config.DfeOn; 
        serdes_firmware_config.field.force_brdfe_on    = fw_config.ForceBrDfe;
        /* serdes_firmware_config.field.cl72_emulation_en = fw_config.Cl72Enable; */
        serdes_firmware_config.field.scrambling_dis    = fw_config.ScramblingDisable;
        serdes_firmware_config.field.unreliable_los    = fw_config.UnreliableLos;
        serdes_firmware_config.field.media_type        = fw_config.MediaType; 
        serdes_firmware_config.field.cl72_auto_polarity_en   = fw_config.Cl72AutoPolEn;
        serdes_firmware_config.field.cl72_restart_timeout_en = fw_config.Cl72RestTO;

        PHYMOD_IF_ERR_RETURN(PHYMOD_IS_WRITE_DISABLED(&phy_copy.access, &is_warm_boot));

		
        if (!is_warm_boot) {
            /* PHYMOD_IF_ERR_RETURN(merlin16_lane_soft_reset_read(&phy_copy.access, &rst_status)); */
            /* if (rst_status) */ PHYMOD_IF_ERR_RETURN (merlin16_lane_soft_reset_release(&phy_copy.access, 0));
            PHYMOD_IF_ERR_RETURN(merlin16_set_uc_lane_cfg(&phy_copy.access, serdes_firmware_config));
            /* if (rst_status) */ PHYMOD_IF_ERR_RETURN (merlin16_lane_soft_reset_release(&phy_copy.access, 1));
        }
    }
    return PHYMOD_E_NONE;
}

int tsce16_core_identify(const phymod_core_access_t* core, uint32_t core_id, uint32_t* is_identified)
{
    int ioerr = 0;
    const phymod_access_t *pm_acc = &core->access;
    PHYID2r_t id2;
    PHYID3r_t id3;
    MAIN0_SERDESIDr_t serdesid;
    /* DIG_REVID0r_t revid; */
    uint32_t model;
    int rv;
    *is_identified = 0;

#ifdef PHYMOD_DIAG
    /*
    LOG_ERROR(BSL_LS_SOC_PHYMOD,("%-22s: core_id=%0d adr=%0x lane_mask=%0x v_adr=%0x v_mask=%0x\n", 
    __func__, core_id, pm_acc->addr, pm_acc->lane_mask, phymod_dbg_addr, phymod_dbg_mask)); */
	PHYMOD_VDBG(DBG_CFG, pm_acc,("%-22s: core_id=%0d adr=%0"PRIx32" lane_mask=%0"PRIx32"\n", 
                                 __func__, (int)core_id, pm_acc->addr, pm_acc->lane_mask));
#endif
    if (core_id == 0) {
        ioerr += READ_PHYID2r(pm_acc, &id2);
        ioerr += READ_PHYID3r(pm_acc, &id3);
    } else {
        PHYID2r_SET(id2, ((core_id >> 16) & 0xffff));
        PHYID3r_SET(id3, core_id & 0xffff);
    }

    if (PHYID2r_GET(id2) == TSCE16_ID0 && PHYID3r_GET(id3) == TSCE16_ID1) {
        /* PHY IDs match - now check PCS model */
        ioerr += READ_MAIN0_SERDESIDr(pm_acc, &serdesid);
        model = MAIN0_SERDESIDr_MODEL_NUMBERf_GET(serdesid);
        if (model == TSCE16_MODEL) {
            if (MAIN0_SERDESIDr_TECH_PROCf_GET(serdesid) == TSCE16_TECH_PROC) {
                *is_identified = 1;
            }
        }
    }
    rv = ioerr ? PHYMOD_E_IO : PHYMOD_E_NONE;
#ifdef PHYMOD_DIAG
	PHYMOD_VDBG(DBG_CFG, pm_acc,("%-22s: core_id=%0d identified=%0d rv=%0d adr=%0"PRIx32" lmask=%0"PRIx32"\n", 
                                       __func__, (int)core_id, (int)*is_identified, rv, pm_acc->addr, pm_acc->lane_mask));
#endif
    return rv;
}

int tsce16_core_info_get(const phymod_core_access_t* phy, phymod_core_info_t* info)
{
    uint32_t serdes_id;
    PHYID2r_t id2;
    PHYID3r_t id3;
    const phymod_access_t *pm_acc = &phy->access;

    PHYMOD_IF_ERR_RETURN
        (temod16_revid_read(&phy->access, &serdes_id));
    info->serdes_id = serdes_id;

    if ((serdes_id & 0x3f) == TSCE16_MODEL) {
        info->core_version = phymodCoreVersionTsce16;
        PHYMOD_STRNCPY(info->name, "Tsce16", PHYMOD_STRLEN("Tsce16")+1);
    } 

    PHYMOD_IF_ERR_RETURN(READ_PHYID2r(pm_acc, &id2));
    PHYMOD_IF_ERR_RETURN(READ_PHYID3r(pm_acc, &id3));

    info->phy_id0 = (uint16_t) id2.v[0];
    info->phy_id1 = (uint16_t) id3.v[0];

    return PHYMOD_E_NONE;
}

/* set lane swapping for core
 */ 
int tsce16_core_lane_map_set(const phymod_core_access_t* core, const phymod_lane_map_t* lane_map)
{
    uint32_t pcs_swap = 0 , lane;
    uint8_t pmd_tx_lane_map[PHYMOD_MAX_LANES_PER_CORE];
    uint8_t pmd_rx_lane_map[PHYMOD_MAX_LANES_PER_CORE];
    uint8_t num_lanes = (uint8_t) lane_map->num_of_lanes;

#ifdef PHYMOD_DIAG
    const phymod_access_t *pm_acc;
    pm_acc = &core->access;
    PHYMOD_VDBG(DBG_TOP, pm_acc,
       ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" rx_lane_map=%0"PRIx32"%0"PRIx32"%0"PRIx32"%0"PRIx32" tx_lane_map=%0"PRIx32"%0"PRIx32"%0"PRIx32"%0"PRIx32"\n", 
        __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, 
        lane_map->lane_map_rx[3], lane_map->lane_map_rx[2], 
        lane_map->lane_map_rx[1], lane_map->lane_map_rx[0],
        lane_map->lane_map_tx[3], lane_map->lane_map_tx[2], 
        lane_map->lane_map_tx[1], lane_map->lane_map_tx[0]));
#endif

    /* PCS lane map; PCS lane map specifies which board-side lane a switch-side lane should
     * connect to. For instance, on the bring-up board, for TX, lane[0] on the switch is
     * actually routed to lane[2] on the board; lane[2] on the switch is routed to lane[0]
     * on the board. In this case, "2" should be put in lane_map_tx[0], and "0" should be
     * put in lane_map_tx[2].
    */
    if (lane_map->num_of_lanes != TSCE16_NOF_LANES_IN_CORE) {
        return PHYMOD_E_CONFIG;
    }

    for (lane = 0; lane < TSCE16_NOF_LANES_IN_CORE; lane++) {
        pcs_swap += lane_map->lane_map_tx[lane] << (lane*4);
        pcs_swap += lane_map->lane_map_rx[lane] << (lane*4 + 16);
    }

    if (!PHYMOD_DEVICE_OP_MODE_PCS_BYPASS_GET(core->device_op_mode)) {
        PHYMOD_IF_ERR_RETURN(temod16_pcs_lane_swap(&core->access, pcs_swap));
    }

    /* PMD lane address map; PCS lane map specifies the lane map from switch's point of
     * view, while PMD lane address map specifies the same lane map from board's point
     * of view.
     * For instance, if PCS specify that lane[0] on the switch TX side connects to lane[2]
     * on the board TX side, PCS would put "2" in lane_map_tx[0], and PMD would put "0"
     * in lane_map_tx[2]. From PMD's point of view, this means that board lane[2] should
     * connect to switch lane[0].
     */
    for (lane = 0; lane < TSCE16_NOF_LANES_IN_CORE; lane++) {
        pmd_tx_lane_map[(int)lane_map->lane_map_tx[lane]] = lane;
        pmd_rx_lane_map[(int)lane_map->lane_map_rx[lane]] = lane;
    }

    PHYMOD_IF_ERR_RETURN
        (merlin16_map_lanes(&core->access, num_lanes, pmd_tx_lane_map, pmd_rx_lane_map));

    return PHYMOD_E_NONE;
}

/* load tsce fw. the fw_loader parameter is valid just for external fw load */
STATIC
int _tsce16_core_firmware_load(const phymod_core_access_t* core, phymod_firmware_load_method_t load_method, phymod_firmware_loader_f fw_loader)
{
#ifdef PHYMOD_DIAG
    const phymod_access_t *pm_acc;
    pm_acc = &core->access;
    PHYMOD_VDBG(DBG_CFG, pm_acc,
       ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" load_meth=%0d",
        __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, (int)load_method));
#endif

    switch (load_method) {
    case phymodFirmwareLoadMethodInternal:
        PHYMOD_IF_ERR_RETURN(merlin16_ucode_mdio_load(&core->access, merlin16_ucode, merlin16_ucode_len));
        break;
    case phymodFirmwareLoadMethodExternal:
        PHYMOD_IF_ERR_RETURN(merlin16_ucode_pram_load(&core->access, (char*) merlin16_ucode, merlin16_ucode_len));
        break;
    case phymodFirmwareLoadMethodNone:
        break;
    default:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, (_PHYMOD_MSG("illegal fw load method %u"), load_method));
    }
    return PHYMOD_E_NONE;
}

int tsce16_phy_firmware_core_config_set(const phymod_phy_access_t* phy, phymod_firmware_core_config_t fw_config)
{
    struct merlin16_uc_core_config_st serdes_firmware_core_config;
    PHYMOD_MEMSET(&serdes_firmware_core_config, 0, sizeof(serdes_firmware_core_config));
    serdes_firmware_core_config.field.core_cfg_from_pcs = fw_config.CoreConfigFromPCS;
    serdes_firmware_core_config.field.vco_rate = fw_config.VcoRate; 

    PHYMOD_IF_ERR_RETURN(merlin16_INTERNAL_set_uc_core_config(&phy->access, serdes_firmware_core_config));
    return PHYMOD_E_NONE;
}

int tsce16_phy_firmware_core_config_get(const phymod_phy_access_t* phy, phymod_firmware_core_config_t* fw_config)
{
    struct merlin16_uc_core_config_st serdes_firmware_core_config;
    PHYMOD_IF_ERR_RETURN(merlin16_get_uc_core_config(&phy->access, &serdes_firmware_core_config));
    PHYMOD_MEMSET(fw_config, 0, sizeof(*fw_config));
    fw_config->CoreConfigFromPCS = serdes_firmware_core_config.field.core_cfg_from_pcs;
    fw_config->VcoRate = serdes_firmware_core_config.field.vco_rate;
    return PHYMOD_E_NONE; 
}

int tsce16_phy_firmware_lane_config_set(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t fw_config)
{
    phymod_phy_access_t phy_copy;
    int start_lane, num_lane, i;

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /*Hold the per lne soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (merlin16_lane_soft_reset_release(&phy_copy.access, 0));
    }

    PHYMOD_IF_ERR_RETURN
         (_tsce16_phy_firmware_lane_config_set(phy, fw_config));
    /*Hold the per lne soft reset bit*/
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (merlin16_lane_soft_reset_release(&phy_copy.access, 1));
    }

    PHYMOD_IF_ERR_RETURN
        (temod16_trigger_speed_change(&phy->access));

    return PHYMOD_E_NONE; 
}

int tsce16_phy_firmware_lane_config_get(const phymod_phy_access_t* phy, phymod_firmware_lane_config_t* fw_config)
{
    struct merlin16_uc_lane_config_st serdes_firmware_config;
    PHYMOD_MEMSET(&serdes_firmware_config, 0x0, sizeof(serdes_firmware_config));

    PHYMOD_IF_ERR_RETURN(merlin16_get_uc_lane_cfg(&phy->access, &serdes_firmware_config));
    PHYMOD_MEMSET(fw_config, 0, sizeof(*fw_config));
    fw_config->LaneConfigFromPCS = serdes_firmware_config.field.lane_cfg_from_pcs;
    fw_config->AnEnabled         = serdes_firmware_config.field.an_enabled;
    fw_config->DfeOn             = serdes_firmware_config.field.dfe_on;
    fw_config->ForceBrDfe        = serdes_firmware_config.field.force_brdfe_on;
    fw_config->Cl72AutoPolEn     = serdes_firmware_config.field.cl72_auto_polarity_en;
    fw_config->Cl72RestTO        = serdes_firmware_config.field.cl72_restart_timeout_en;
    fw_config->ScramblingDisable = serdes_firmware_config.field.scrambling_dis;
    fw_config->UnreliableLos     = serdes_firmware_config.field.unreliable_los;
    fw_config->MediaType         = serdes_firmware_config.field.media_type;
    fw_config->Cl72AutoPolEn     = serdes_firmware_config.field.cl72_auto_polarity_en;
    fw_config->Cl72RestTO        = serdes_firmware_config.field.cl72_restart_timeout_en;
     
    return PHYMOD_E_NONE; 
}

int tsce16_phy_polarity_set(const phymod_phy_access_t* phy, const phymod_polarity_t* polarity)
{
    PHYMOD_IF_ERR_RETURN
        (merlin16_polarity_set(&phy->access, polarity->tx_polarity, polarity->rx_polarity));

    return PHYMOD_E_NONE;
}

int tsce16_phy_polarity_get(const phymod_phy_access_t* phy, phymod_polarity_t* polarity)
{
    PHYMOD_IF_ERR_RETURN
        (merlin16_polarity_get(&phy->access, &polarity->tx_polarity, &polarity->rx_polarity));

    return PHYMOD_E_NONE;
}

int tsce16_phy_tx_set(const phymod_phy_access_t* phy, const phymod_tx_t* tx)
{
    PHYMOD_IF_ERR_RETURN
        (merlin16_apply_txfir_cfg(&phy->access, tx->pre, tx->main, tx->post, tx->post2));

    return PHYMOD_E_NONE;
}

int tsce16_phy_power_set(const phymod_phy_access_t* phy, const phymod_phy_power_t* power)
{
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i;

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    if ((power->tx == phymodPowerOff) && (power->rx == phymodPowerOff)) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(temod16_port_enable_set(&pm_phy_copy.access, 0));
        }
    }
    if ((power->tx == phymodPowerOn) && (power->rx == phymodPowerOn)) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(temod16_port_enable_set(&pm_phy_copy.access, 1));
        }
    }
    if ((power->tx == phymodPowerOff) && (power->rx == phymodPowerNoChange)) {
            /* disable tx on the PMD side */
            PHYMOD_IF_ERR_RETURN(merlin16_tx_disable(&phy->access, 1));
    }
    if ((power->tx == phymodPowerOn) && (power->rx == phymodPowerNoChange)) {
            /* enable tx on the PMD side */
            PHYMOD_IF_ERR_RETURN(merlin16_tx_disable(&phy->access, 0));
    }
    if ((power->tx == phymodPowerNoChange) && (power->rx == phymodPowerOff)) {
            /* disable rx on the PMD side */
            PHYMOD_IF_ERR_RETURN(temod16_rx_squelch_set(&phy->access, 1));
    }
    if ((power->tx == phymodPowerNoChange) && (power->rx == phymodPowerOn)) {
            /* enable rx on the PMD side */
            PHYMOD_IF_ERR_RETURN(temod16_rx_squelch_set(&phy->access, 0));
    }
    return PHYMOD_E_NONE; 
}

int tsce16_phy_tx_lane_control_set(const phymod_phy_access_t* phy, phymod_phy_tx_lane_control_t tx_control)
{   
    switch (tx_control) {
    case phymodTxTrafficDisable:
        PHYMOD_IF_ERR_RETURN(temod16_tx_lane_control_set(&phy->access, TEMOD16_TX_LANE_TRAFFIC_DISABLE));
        break;
    case phymodTxTrafficEnable:
        PHYMOD_IF_ERR_RETURN(temod16_tx_lane_control_set(&phy->access, TEMOD16_TX_LANE_TRAFFIC_ENABLE));
        break;
    case phymodTxReset:
        PHYMOD_IF_ERR_RETURN(temod16_tx_lane_control_set(&phy->access, TEMOD16_TX_LANE_RESET));
        break;
    case phymodTxSquelchOn:
        PHYMOD_IF_ERR_RETURN(temod16_tx_squelch_set(&phy->access, 1));
        break;
    case phymodTxSquelchOff:
        PHYMOD_IF_ERR_RETURN(temod16_tx_squelch_set(&phy->access, 0));
        break;
    default:
        break;
    }
    return PHYMOD_E_NONE;
}

int tsce16_phy_tx_lane_control_get(const phymod_phy_access_t* phy, phymod_phy_tx_lane_control_t* tx_control)
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

    PHYMOD_IF_ERR_RETURN(temod16_tx_squelch_get(&pm_phy_copy.access, &enable));

    /* next check if PMD loopback is on */ 
    if (enable) {                           
        PHYMOD_IF_ERR_RETURN(merlin16_pmd_loopback_get(&pm_phy_copy.access, &lb_enable));
        if (lb_enable) enable = 0;
    }

    if (enable) {
        *tx_control = phymodTxSquelchOn;
    } else {
        PHYMOD_IF_ERR_RETURN(temod16_tx_lane_control_get(&pm_phy_copy.access, &reset, &tx_lane));
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

int tsce16_phy_rx_lane_control_set(const phymod_phy_access_t* phy, phymod_phy_rx_lane_control_t rx_control)
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
        PHYMOD_IF_ERR_RETURN(temod16_rx_lane_control_set(&phy->access, 1));
        break;
    case phymodRxSquelchOn:
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(temod16_rx_squelch_set(&pm_phy_copy.access, 1));
        }
        break;
    case phymodRxSquelchOff:
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN(temod16_rx_squelch_set(&pm_phy_copy.access, 0));
        }
        break;
    default:
        break;
    }
    return PHYMOD_E_NONE;
}

int tsce16_phy_rx_lane_control_get(const phymod_phy_access_t* phy, phymod_phy_rx_lane_control_t* rx_control)
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

    PHYMOD_IF_ERR_RETURN(temod16_rx_squelch_get(&pm_phy_copy.access, &enable));
    /* next check if PMD loopback is on */ 
    if (enable) {                           
        PHYMOD_IF_ERR_RETURN(merlin16_pmd_loopback_get(&pm_phy_copy.access, &lb_enable));
        if (lb_enable) enable = 0;
    }
    if (enable) {
        *rx_control = phymodRxSquelchOn;
    } else {
        PHYMOD_IF_ERR_RETURN(temod16_rx_lane_control_get(&pm_phy_copy.access, &reset));
        if (reset == 0) {
            *rx_control = phymodRxReset;
        } else {
            *rx_control = phymodRxSquelchOff;
        }
    }
    return PHYMOD_E_NONE;
}


int tsce16_phy_interface_config_set(const phymod_phy_access_t* phy, uint32_t flags, const phymod_phy_inf_config_t* config)
{  
    uint32_t current_pll_div=0;
    uint32_t new_pll_div=0;
    int       new_speed_vec=0;
    int16_t  new_os_mode =-1;
    temod16_spd_intfc_type spd_intf = TEMOD16_SPD_ILLEGAL;
    phymod_phy_access_t pm_phy_copy;
    int start_lane=0, num_lane, i, pll_switch = 0;
    uint32_t os_dfeon=0;
    uint32_t scrambling_dis=0;
    uint32_t u_os_mode = 0;
    int      dfe_adjust = -1; 
    int cl72_allowed=0, cl72_req=0;
    phymod_phy_inf_config_t temp_config;
    temod16_pll_mode_type pll_mode;
    uint16_t or_val=0;

    /* sc_table_entry exp_entry; RAVI */
    phymod_firmware_lane_config_t firmware_lane_config;  
    phymod_firmware_core_config_t firmware_core_config;
#ifdef PHYMOD_DIAG
    const phymod_access_t *pm_acc;
    pm_acc = &phy->access;
    PHYMOD_VDBG(DBG_SPD, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" speed=%0d intf=%0d(%s) flags=%0x\n",
      __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, config->data_rate, config->interface_type, 
                                  phymod_interface_t_mapping[config->interface_type].key, flags));
#endif

    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    PHYMOD_MEMSET(&firmware_lane_config, 0x0, sizeof(firmware_lane_config));
    PHYMOD_MEMSET(&firmware_core_config, 0x0, sizeof(firmware_core_config));
    PHYMOD_MEMSET(&temp_config, 0x0, sizeof(temp_config));

    /* end of special mode */
    firmware_lane_config.MediaType = 0;

    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    /* reset pcs */
    temod16_disable_set(&phy->access);

    /* Hold the per lne soft reset bit */
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (merlin16_lane_soft_reset_release(&pm_phy_copy.access, 0));
    }
    /* deassert pmd_tx_disable_pin_dis if it is set by ILKn */
    /* remove pmd_tx_disable_pin_dis it may be asserted because of ILKn */
    if (config->interface_type != phymodInterfaceBypass) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN
              (merlin16_pmd_tx_disable_pin_dis_set(&phy->access, 0));    
        }
    } 
    /* disable CL72 */
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (temod16_clause72_control(&pm_phy_copy.access, 0));
    }

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;
     PHYMOD_IF_ERR_RETURN
        (tsce16_phy_firmware_lane_config_get(&pm_phy_copy, &firmware_lane_config)); 

    /* make sure that an and config from pcs is off */
    firmware_core_config.CoreConfigFromPCS = 0;
    firmware_lane_config.AnEnabled = 0;
    firmware_lane_config.LaneConfigFromPCS = 0;

    /* and also make sure the cl72 restart timeout is enabled */
    firmware_lane_config.Cl72RestTO = 1;
    firmware_lane_config.Cl72AutoPolEn = 0;
    if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    } else if (PHYMOD_INTF_MODES_COPPER_GET(config)) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
    } else {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
    }     

    PHYMOD_IF_ERR_RETURN
        (temod16_update_port_mode(&phy->access, (int *) &pll_switch));

    spd_intf = TEMOD16_SPD_10_X1_SGMII; /* to prevent undefinded TEMOD16_SPD_ILLEGAL accessing tables */

    /* find the speed */
    if (config->interface_type == phymodInterfaceXFI) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
    } else if (config->interface_type == phymodInterfaceSFI) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    } else if (config->interface_type == phymodInterfaceSFPDAC) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeCopperCable;
    } else if (config->interface_type == phymodInterface1000X) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    } else if (config->interface_type == phymodInterfaceSGMII) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
    } else if (config->interface_type == phymodInterfaceLR) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    } else if (config->interface_type == phymodInterfaceSR) {
        firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
    }
    
    cl72_req = (flags & (PHYMOD_INTF_F_CL72_REQUESTED_BY_CNFG|PHYMOD_INTF_F_CL72_REQUESTED_BY_API))?1:0;
    switch (config->data_rate) {
    case 10:
        spd_intf = TEMOD16_SPD_10_X1_SGMII;
        break;
    case 100:
        spd_intf = TEMOD16_SPD_100_X1_SGMII;
        break;
    case 1000:
        spd_intf = TEMOD16_SPD_1000_X1_SGMII;
        break;
    case 10000:
        cl72_allowed = 1;
        if (config->interface_type == phymodInterfaceSFI) {
            spd_intf = TEMOD16_SPD_10000_XFI;
            dfe_adjust = 0;
        } else if (config->interface_type == phymodInterfaceXFI) {
            spd_intf = TEMOD16_SPD_10000_XFI;
            dfe_adjust = 0;
        } else if (config->interface_type == phymodInterfaceXGMII) {
            spd_intf = TEMOD16_SPD_10000;
        } else if (config->interface_type == phymodInterfaceKR) {  
            spd_intf = TEMOD16_SPD_10000_XFI;
            dfe_adjust = 1;
        } else {
            spd_intf = TEMOD16_SPD_10000_XFI;
            if (PHYMOD_INTF_MODES_FIBER_GET(config)) {
                dfe_adjust = 0;
            }
        }
        break;
    default:
        PHYMOD_RETURN_WITH_ERR
            (PHYMOD_E_CONFIG, (_PHYMOD_MSG("this speed %d is not supported by tsce16"),
             config->data_rate));
        break;
    }

    if (config->data_rate==1000) {
        if (config->interface_type == phymodInterfaceSGMII) {
            firmware_lane_config.MediaType = phymodFirmwareMediaTypePcbTraceBackPlane;
        }
        if (config->interface_type == phymodInterface1000X) {
            firmware_lane_config.MediaType = phymodFirmwareMediaTypeOptics;
        }
    }

    /* get the current PLL setting */
    pm_phy_copy.access.lane_mask = 1 << start_lane;
    PHYMOD_IF_ERR_RETURN
        (temod16_pll_config_get(&pm_phy_copy.access, &pll_mode));
    current_pll_div = (uint32_t) pll_mode;     

    /* find the new os mode and new pll div */
    if (config->interface_type == phymodInterfaceBypass) {
        new_pll_div = TEMOD16_PLL_MODE_DIV_66;
        if (config->data_rate == 1000) {
            new_os_mode = 8;
        }
        /* the other supported speed could only be 10G */
        else {
            new_os_mode = 0;
        }
    } else {
        new_pll_div = TEMOD16_PLL_MODE_DIV_66;
        PHYMOD_IF_ERR_RETURN(temod16_get_mapped_speed(spd_intf, &new_speed_vec));
    }

    if (new_os_mode>=0) {
        /* 0x80000000 causes PMD OS mode to be configured based on new_os_mode,
         * otherwise the PMD OS mode will be based on spd_intf.
         */
        u_os_mode = new_os_mode | 0x80000000;
    } else {
        u_os_mode = 0;
    }

    
    PHYMOD_IF_ERR_RETURN
        (temod16_pmd_osmode_set(&phy->access, spd_intf, u_os_mode));
    PHYMOD_IF_ERR_RETURN(temod16_osdfe_on_lkup_get(&phy->access, spd_intf, &os_dfeon));
    PHYMOD_IF_ERR_RETURN(temod16_scrambling_dis_lkup_get(&phy->access, spd_intf, &scrambling_dis));
    firmware_lane_config.DfeOn = 0;
    firmware_lane_config.ScramblingDisable = scrambling_dis;
    /* Override scrambler settings in PCS when disable scrambler is needed*/
    if (scrambling_dis) {
        or_val = 0;
        PHYMOD_IF_ERR_RETURN
            (temod16_override_set(&phy->access, TEMOD16_OVERRIDE_SCR_MODE , or_val));
        PHYMOD_IF_ERR_RETURN
            (temod16_override_set(&phy->access, TEMOD16_OVERRIDE_DESCR_MODE, or_val));
    }
    else {
        PHYMOD_IF_ERR_RETURN
            (temod16_override_clear(&phy->access, TEMOD16_OVERRIDE_SCR_MODE));
        PHYMOD_IF_ERR_RETURN
            (temod16_override_clear(&phy->access, TEMOD16_OVERRIDE_DESCR_MODE));
    }

    if (os_dfeon == 0x1)
        firmware_lane_config.DfeOn = 1;

    if (dfe_adjust >=0) 
        firmware_lane_config.DfeOn = dfe_adjust;

    /* the speed is not supported */
    if (new_pll_div == TEMOD16_PLL_MODE_DIV_ILLEGAL) {
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, 
                               (_PHYMOD_MSG("this speed %d is not supported at this ref clock %d"),
                                 config->data_rate, config->ref_clock));
    }



    /* if pll change is enabled. new_pll_div is the reg vector value */
    if ((current_pll_div != new_pll_div) && (PHYMOD_INTF_F_DONT_TURN_OFF_PLL & flags)) {
        if (config->interface_type != phymodInterfaceBypass) {
            PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG, 
                                   (_PHYMOD_MSG("pll has to change for speed_set from %u to %u but DONT_TURN_OFF_PLL flag is enabled"),
                                     (unsigned int)current_pll_div, (unsigned int)new_pll_div));
        }
    }

    if (cl72_req & cl72_allowed) {
        for (i = 0; i < num_lane; i++) {
            if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
                continue;
            }
            pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
            PHYMOD_IF_ERR_RETURN
                (temod16_clause72_control(&pm_phy_copy.access, 1));
        }
    }

    if (config->interface_type != phymodInterfaceBypass) {
        if (flags & PHYMOD_INTF_F_SET_SPD_NO_TRIGGER) {
            (temod16_set_spd_intf(&phy->access, spd_intf, 1));
        } else {
            PHYMOD_IF_ERR_RETURN
                (temod16_set_spd_intf(&phy->access, spd_intf, 0));
        }
    }

    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 0x1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
             (_tsce16_phy_firmware_lane_config_set(&pm_phy_copy, firmware_lane_config));
    }
    
    /* release the per lne soft reset bit */
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (merlin16_lane_soft_reset_release(&pm_phy_copy.access, 1));
    }
#ifdef PHYMOD_DIAG
    PHYMOD_VDBG(DBG_SPD, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" spd=%0d(%s) media=%0d\n",
       __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, spd_intf, e2s_temod16_spd_intfc_type[spd_intf],
            firmware_lane_config.MediaType));
#endif

    return PHYMOD_E_NONE;
}



STATIC
int _tsce16_speed_id_interface_config_get(const phymod_phy_access_t* phy, int speed_id, 
                                        phymod_phy_inf_config_t* config, uint16_t an_enable, 
                                        phymod_firmware_lane_config_t *lane_config)
{
    switch (speed_id) {
    case 0x3:
        if (lane_config->MediaType == phymodFirmwareMediaTypeOptics) {
            config->interface_type = phymodInterface1000X;
        } else {
            config->interface_type = phymodInterfaceSGMII;
        }     
        config->data_rate = 1000;
        break;
    case 0x4:
        config->data_rate = 1000;
        config->interface_type = phymodInterfaceCX;
        break;
    case 0x5:
        config->data_rate = 1000;
        config->interface_type = phymodInterfaceKX;
        break;
    case 0x1c:
        config->data_rate = 10000;
        config->interface_type = phymodInterfaceKR;
        if (!an_enable) {
            if (lane_config->MediaType == phymodFirmwareMediaTypeOptics) {
                if (config->interface_type == phymodInterfaceSR) {
                    config->interface_type = phymodInterfaceSR;
                } else {
                    config->interface_type = phymodInterfaceSFI;
                }
            } else if (lane_config->MediaType == phymodFirmwareMediaTypeCopperCable) {
                config->interface_type = phymodInterfaceCR;
            } else if (lane_config->DfeOn == 1) {
                config->interface_type = phymodInterfaceKR;
            } else {
                config->interface_type = phymodInterfaceXFI;
            }
        }
        break;
    case 0x35:
        config->data_rate = 10;
        config->interface_type = phymodInterfaceSGMII;
        break;
    case 0x36:
        config->data_rate = 100;
        config->interface_type = phymodInterfaceSGMII;
        break;
    case 0x37:
        if (lane_config->MediaType == phymodFirmwareMediaTypeOptics) {
            config->interface_type = phymodInterface1000X;
        } else {
            config->interface_type = phymodInterfaceSGMII;
        }
        config->data_rate = 1000;
        break;
    default:
        config->data_rate = 0;
        config->interface_type = phymodInterfaceSGMII; 
        break;
    }

    return PHYMOD_E_NONE;
}


/* flags- unused parameter */
int tsce16_phy_interface_config_get(const phymod_phy_access_t* phy, uint32_t flags, phymod_ref_clk_t ref_clock, phymod_phy_inf_config_t* config)
{
    int speed_id;
    phymod_firmware_lane_config_t firmware_lane_config;  
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane;
    temod16_an_control_t an_control;
    int an_complete = 0;

#ifdef PHYMOD_DIAG
    const phymod_access_t *pm_acc;
    pm_acc = &phy->access;
    PHYMOD_VDBG(DBG_LNK, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" flags=%"PRIx32"\n",
                __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, flags));
#endif
    config->ref_clock = ref_clock;
    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    PHYMOD_IF_ERR_RETURN
        (temod16_speed_id_get(&phy->access, &speed_id));

    pm_phy_copy.access.lane_mask = 0x1 << start_lane;

    PHYMOD_MEMSET(&an_control, 0x0,  sizeof(temod16_an_control_t));
    PHYMOD_IF_ERR_RETURN
        (temod16_autoneg_control_get(&pm_phy_copy.access, &an_control, &an_complete));

    PHYMOD_IF_ERR_RETURN
        (tsce16_phy_firmware_lane_config_get(&pm_phy_copy, &firmware_lane_config));
    
    PHYMOD_IF_ERR_RETURN
        (_tsce16_speed_id_interface_config_get(phy, speed_id, config, an_control.enable, &firmware_lane_config));

    if (firmware_lane_config.MediaType == phymodFirmwareMediaTypeOptics) {
        PHYMOD_INTF_MODES_FIBER_SET(config);
    } else if (firmware_lane_config.MediaType == phymodFirmwareMediaTypeCopperCable) {
        PHYMOD_INTF_MODES_FIBER_CLR(config);
        PHYMOD_INTF_MODES_COPPER_SET(config);
    } else {
        PHYMOD_INTF_MODES_FIBER_CLR(config);
        PHYMOD_INTF_MODES_BACKPLANE_SET(config);
    }

    switch (config->interface_type) {
    case phymodInterfaceSGMII:
    {
        if (config->data_rate == 1000) {
            if (!PHYMOD_INTF_MODES_FIBER_GET(config)) {
                config->interface_type = phymodInterfaceSGMII;
            } else {
                config->interface_type = phymodInterface1000X;
            }
        } else {
            config->interface_type = phymodInterfaceSGMII;
        }
        break;
    }
    case phymodInterfaceKR:
    {
        phymod_autoneg_control_t an_control;
        int an_complete;
        /* first check if an is enable or not */
        PHYMOD_IF_ERR_RETURN
        (phymod_phy_autoneg_get(phy, &an_control, (uint32_t *) &an_complete));
        if (!an_control.enable) {
            if (config->data_rate == 10000) {
                if (!PHYMOD_INTF_MODES_FIBER_GET(config)) {
                   if (firmware_lane_config.DfeOn == 1) {        
                        config->interface_type = phymodInterfaceKR;
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
                    config->interface_type = phymodInterfaceKR;
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

#ifdef PHYMOD_DIAG
    PHYMOD_VDBG(DBG_CFG, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" sp_id=0x%0x rate=%0d phy_intf=%0d(%s) intf_md=%0x\n",
                __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, speed_id, config->data_rate, 
                config->interface_type, phymod_interface_t_mapping[config->interface_type].key, config->interface_modes));
#endif
    return PHYMOD_E_NONE;
}

int tsce16_phy_autoneg_ability_set(const phymod_phy_access_t* phy, const phymod_autoneg_ability_t* an_ability)
{
    return PHYMOD_E_NONE;
}

int tsce16_phy_autoneg_ability_get(const phymod_phy_access_t* phy, phymod_autoneg_ability_t* an_ability_get_type) {
    return PHYMOD_E_NONE;
}

int tsce16_phy_autoneg_set(const phymod_phy_access_t* phy, const phymod_autoneg_control_t* an)
{
    return PHYMOD_E_NONE;
}

int tsce16_phy_autoneg_get(const phymod_phy_access_t* phy, phymod_autoneg_control_t* an, uint32_t* an_done)
{
    an->enable  = 0;
    an->an_mode = phymod_AN_MODE_NONE;
    *an_done    = 0;

    return PHYMOD_E_NONE;
}


/* 
 * There is a significant change from TSCE 28nm to 16nm in terms of the
 * PMD programming sequence. Please be aware that the underlying PMD changed 
 * from eagle to merlin16. The PCS programming sequence remained the same.
 * Some comments are added below to highlight the PMD programming changes.
 */
STATIC 
int _tsce16_core_init_pass1(const phymod_core_access_t* core, 
                          const phymod_core_init_config_t* init_config, 
                          const phymod_core_status_t* core_status)
{
    phymod_phy_access_t phy_access;
    phymod_core_access_t  core_copy;
    uint32_t  uc_active = 0;
    int i, start_lane, num_lane;

    TSCE16_CORE_TO_PHY_ACCESS(&phy_access, core);
    PHYMOD_MEMCPY(&core_copy, core, sizeof(core_copy));
    core_copy.access.lane_mask = 0x1;

    /* 
     * First step to program merlin16 is to do power reset. That is to
     * program field POR_H_RSTB and CORE_DP_H_RSTB in register PMD_X1_CTL
     */
    PHYMOD_IF_ERR_RETURN
        (temod16_pmd_reset_seq(&core_copy.access, core_status->pmd_active));

    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy_access.access, &start_lane, &num_lane));

    /* 
     * Before programming the PMD lane address map register, the PMD lanes
     * have to be reset. Without do this, writing the PMD lane address map
     * regsiter will not take effect, meaning the reading value != writing
     * value.
     */
    for (i = 0; i < num_lane; i++) {
        phy_access.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (temod16_pmd_x4_reset(&phy_access.access));
    }

    PHYMOD_IF_ERR_RETURN(merlin16_uc_active_get(&core_copy.access, &uc_active));
    if (uc_active) {
        return(PHYMOD_E_NONE);
    }

    /* need to set the heart beat default is for 156.25M */
    PHYMOD_IF_ERR_RETURN (temod16_refclk_set(&core_copy.access,
                          init_config->interface.ref_clock));  

    /* 
     * In 28nm TSCE code the lane map programming was done in core_init_pass2.
     * Now it is moved to pass1 because merlin16 requires that PMD lane address
     * map be done before loading the uCode. Notice that before programming the
     * PMD lane address map, lane and lane DP has to be reset by calling
     * temod16_pmd_x4_reset().
     */
    PHYMOD_IF_ERR_RETURN
        (tsce16_core_lane_map_set(&core_copy, &init_config->lane_map));

    /* 
     * for bring-up we force the firmware load method to be internal. below is the code to restore after finishing bring-up.
     * if (_tsce16_core_firmware_load(&core_copy, init_config->firmware_load_method, init_config->firmware_loader)) {
     */
    if (init_config->firmware_load_method != phymodFirmwareLoadMethodNone) {
        if (_tsce16_core_firmware_load(&core_copy, phymodFirmwareLoadMethodInternal, init_config->firmware_loader)) {
                PHYMOD_DEBUG_ERROR(("devad 0x%"PRIx32" lane 0x%"PRIx32": UC firmware-load failed\n", core->access.addr, core->access.lane_mask));  
                PHYMOD_IF_ERR_RETURN (PHYMOD_E_INIT);
        }
    }

    return (PHYMOD_E_NONE);
}

STATIC 
int _tsce16_core_init_pass2(const phymod_core_access_t* core, 
                            const phymod_core_init_config_t* init_config, 
                            const phymod_core_status_t* core_status)
{
    phymod_phy_access_t phy_access, phy_access_copy;
    phymod_core_access_t  core_copy;
    phymod_firmware_core_config_t  firmware_core_config_tmp;

    TSCE16_CORE_TO_PHY_ACCESS(&phy_access, core);
    PHYMOD_MEMCPY(&core_copy, core, sizeof(core_copy));
    core_copy.access.lane_mask = 0x1;

    phy_access_copy = phy_access;
    phy_access_copy.access = core->access;
    phy_access_copy.access.lane_mask = 0x1;
    phy_access_copy.type = core->type;
    
    PHYMOD_IF_ERR_RETURN
        (merlin16_pmd_ln_h_rstb_pkill_override(&phy_access_copy.access, 0x1));
    PHYMOD_IF_ERR_RETURN
        (merlin16_uc_reset(&phy_access_copy.access, 0));
    PHYMOD_IF_ERR_RETURN
        (merlin16_wait_uc_active(&phy_access_copy.access));

    /* bring-up, may need to be added back laster
    if (PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY_GET(init_config)) {
        PHYMOD_IF_ERR_RETURN
            (merlin16_init_merlin16_info(&phy_access_copy.access));
        PHYMOD_IF_ERR_RETURN
            (merlin16_ucode_crc_verify(&core_copy.access, merlin16_ucode_len,merlin16_ucode_crc));
        PHYMOD_IF_ERR_RETURN
            (merlin16_start_ucode_crc_calc(&core_copy.access, merlin16_ucode_len));
        PHYMOD_IF_ERR_RETURN
            (merlin16_check_ucode_crc(&core_copy.access, merlin16_ucode_crc, 250));
    }
    */

    PHYMOD_IF_ERR_RETURN(
        merlin16_pmd_ln_h_rstb_pkill_override( &phy_access_copy.access, 0x0));

    /* next check if in pcs-bypass mode */
    if (PHYMOD_DEVICE_OP_MODE_PCS_BYPASS_GET(core->device_op_mode)) {
        phy_access_copy.access.lane_mask = 0xf; 
        PHYMOD_IF_ERR_RETURN(
            temod16_pcs_ilkn_mode_set(&phy_access_copy.access));
        phy_access_copy.access.lane_mask = 0x1;
    }             

    PHYMOD_IF_ERR_RETURN
        (temod16_autoneg_timer_init(&core->access));

    phy_access_copy.access.lane_mask = 0x1;

    PHYMOD_IF_ERR_RETURN
        (temod16_master_port_num_set(&core->access, 0));

    PHYMOD_IF_ERR_RETURN
        (merlin16_core_soft_reset_release(&core_copy.access, 0));
    
    /* After loading the PMD uCode, PLL needs to be configured. */
    PHYMOD_IF_ERR_RETURN
        (merlin16_configure_pll_refclk_div(&phy_access_copy.access, MERLIN16_PLL_REFCLK_156P25MHZ,
                                               MERLIN16_PLL_DIV_66));

    PHYMOD_IF_ERR_RETURN
        (tsce16_phy_firmware_core_config_get(&phy_access_copy, &firmware_core_config_tmp));
    firmware_core_config_tmp.CoreConfigFromPCS = 0;

    PHYMOD_IF_ERR_RETURN
        (tsce16_phy_firmware_core_config_set(&phy_access_copy, firmware_core_config_tmp)); 

   
    PHYMOD_IF_ERR_RETURN
        (temod16_cl74_chng_default (&core_copy.access));

    /* 
     * After programming the PMD PLL, we need to release the core reset and lane reset.
     * Core reset is done below, and lane reset is done in tsce16_phy_init() by calling
     * temod16_pmd_x4_reset().
     */
    PHYMOD_IF_ERR_RETURN
        (merlin16_core_soft_reset_release(&core_copy.access, 1));
        
    return PHYMOD_E_NONE;
}

int tsce16_core_init(const phymod_core_access_t* core, const phymod_core_init_config_t* init_config, const phymod_core_status_t* core_status)
{
    if ( (!PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config) &&
          !PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) ||
        PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config)) {
        PHYMOD_IF_ERR_RETURN
            (_tsce16_core_init_pass1(core, init_config, core_status));

        if (PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config)) {
            return PHYMOD_E_NONE;
        }
    }

    if ( (!PHYMOD_CORE_INIT_F_EXECUTE_PASS1_GET(init_config) &&
          !PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) ||
        PHYMOD_CORE_INIT_F_EXECUTE_PASS2_GET(init_config)) {
        PHYMOD_IF_ERR_RETURN
            (_tsce16_core_init_pass2(core, init_config, core_status));
    }

    return PHYMOD_E_NONE;

}

int tsce16_phy_init(const phymod_phy_access_t* phy, const phymod_phy_init_config_t* init_config)
{
    int pll_restart = 0;
    const phymod_access_t *pm_acc = &phy->access;
    phymod_phy_access_t pm_phy_copy;
    int start_lane, num_lane, i;
    int lane_bkup;
    phymod_polarity_t tmp_pol;
#ifdef PHYMOD_DIAG
    PHYMOD_VDBG(DBG_CFG, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32"\n",
                __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask));
#endif
    PHYMOD_MEMSET(&tmp_pol, 0x0, sizeof(tmp_pol));
    PHYMOD_MEMCPY(&pm_phy_copy, phy, sizeof(pm_phy_copy));

    /* next program the tx fir taps and driver current based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(pm_acc, &start_lane, &num_lane));
    /* per lane based reset release */
    PHYMOD_IF_ERR_RETURN
        (temod16_pmd_x4_reset(pm_acc));

    /* poll for per lane uc_dsc_ready */
    lane_bkup = pm_phy_copy.access.lane_mask;
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 1 << (start_lane + i);
        PHYMOD_IF_ERR_RETURN
            (merlin16_lane_soft_reset_release(&pm_phy_copy.access, 1));
    }
    pm_phy_copy.access.lane_mask = lane_bkup;

    /* program the rx/tx polarity */
    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        tmp_pol.tx_polarity = (init_config->polarity.tx_polarity) >> i & 0x1;
        tmp_pol.rx_polarity = (init_config->polarity.rx_polarity) >> i & 0x1;
        PHYMOD_IF_ERR_RETURN
            (tsce16_phy_polarity_set(&pm_phy_copy, &tmp_pol));
    }

    for (i = 0; i < num_lane; i++) {
        if (!PHYMOD_LANEPBMP_MEMBER(phy->access.lane_mask, start_lane + i)) {
            continue;
        }
        pm_phy_copy.access.lane_mask = 0x1 << (i + start_lane);
        PHYMOD_IF_ERR_RETURN
            (tsce16_phy_tx_set(&pm_phy_copy, &init_config->tx[i]));
    }

    /* next check if pcs-bypass mode  */
    if (PHYMOD_DEVICE_OP_MODE_PCS_BYPASS_GET(phy->device_op_mode)) {
        PHYMOD_IF_ERR_RETURN
            (merlin16_pmd_tx_disable_pin_dis_set(&phy->access, 1));
        PHYMOD_IF_ERR_RETURN
          (temod16_init_pcs_ilkn(&phy->access));   
    }


    pm_phy_copy.access.lane_mask = 0x1;

    PHYMOD_IF_ERR_RETURN
        (temod16_update_port_mode(pm_acc, &pll_restart));

    PHYMOD_IF_ERR_RETURN
        (temod16_rx_lane_control_set(pm_acc, 1));
    PHYMOD_IF_ERR_RETURN
        (temod16_tx_lane_control_set(pm_acc, TEMOD16_TX_LANE_RESET_TRAFFIC_ENABLE));         /* TX_LANE_CONTROL */

    return PHYMOD_E_NONE;
}

int tsce16_phy_cl72_set(const phymod_phy_access_t* phy, uint32_t cl72_en)
{
    struct merlin16_uc_lane_config_st serdes_firmware_config;
#ifdef PHYMOD_DIAG
    const phymod_access_t *pm_acc;
    pm_acc = &phy->access;
    PHYMOD_VDBG(DBG_CL72, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" cl72_en=%d\n",
                __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, (int)cl72_en));
#endif

    PHYMOD_IF_ERR_RETURN(merlin16_get_uc_lane_cfg(&phy->access, &serdes_firmware_config));

    if (serdes_firmware_config.field.dfe_on == 0) {
      PHYMOD_DEBUG_ERROR(("ERROR :: DFE is off : Can not start CL72 with no DFE\n"));  
      return PHYMOD_E_CONFIG;
    }

    PHYMOD_IF_ERR_RETURN
        (temod16_clause72_control(&phy->access, cl72_en));
    return PHYMOD_E_NONE;
}

int tsce16_phy_cl72_get(const phymod_phy_access_t* phy, uint32_t* cl72_en)
{
    uint32_t local_en;;
    PHYMOD_IF_ERR_RETURN
        (merlin16_pmd_cl72_enable_get(&phy->access, &local_en));
    *cl72_en = local_en;
    return PHYMOD_E_NONE;
}

int tsce16_phy_loopback_set(const phymod_phy_access_t* phy, phymod_loopback_mode_t loopback, uint32_t enable)
{
    int start_lane, num_lane;
    uint32_t cl72_en;
    phymod_phy_access_t phy_copy;
    const phymod_access_t *pm_acc;
    
    pm_acc = &phy->access;
#ifdef PHYMOD_DIAG
    PHYMOD_VDBG(DBG_LPK, pm_acc, ("%-22s: p=%p adr=%0"PRIx32" lmask=%0"PRIx32" lpbk=%0d(%s) en=%0d\n",
              __func__, (void *)pm_acc, pm_acc->addr, pm_acc->lane_mask, loopback,
                            phymod_loopback_mode_t_mapping[loopback].key, enable));
#endif
    PHYMOD_MEMCPY(&phy_copy, phy, sizeof(phy_copy));

    /* next figure out the lane num and start_lane based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    switch (loopback) {
    case phymodLoopbackGlobal:
        PHYMOD_IF_ERR_RETURN
            (tsce16_phy_cl72_get(phy, &cl72_en));
        if ((cl72_en == 1) && (enable == 1)) {
             PHYMOD_DEBUG_ERROR(("adr=%0"PRIx32",lane 0x%x: Error! pcs gloop not supported with cl72 enabled\n",  pm_acc->addr, start_lane));
             break;
        }
        PHYMOD_IF_ERR_RETURN(temod16_tx_loopback_control(&phy->access, enable, start_lane, num_lane));
        break;
    case phymodLoopbackGlobalPMD:
        merlin16_pmd_loopback_set(&phy->access, enable);
        break;
    case phymodLoopbackRemotePMD:
        return PHYMOD_E_UNAVAIL;
    case phymodLoopbackRemotePCS:
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG,
                               (_PHYMOD_MSG("PCS Remote LoopBack not supported")));
        break;
    default:
        break;
    }

    return PHYMOD_E_NONE;
}

int tsce16_phy_loopback_get(const phymod_phy_access_t* phy, phymod_loopback_mode_t loopback, uint32_t* enable)
{
    uint32_t enable_core;
    int start_lane, num_lane;

    *enable = 0;

    /* next figure out the lane num and start_lane based on the input */
    PHYMOD_IF_ERR_RETURN
        (phymod_util_lane_config_get(&phy->access, &start_lane, &num_lane));

    switch (loopback) {
    case phymodLoopbackGlobal :
        PHYMOD_IF_ERR_RETURN(temod16_tx_loopback_get(&phy->access, &enable_core));
        *enable = (enable_core >> start_lane) & 0x1; 
        break;
    case phymodLoopbackGlobalPMD :
        PHYMOD_IF_ERR_RETURN(merlin16_pmd_loopback_get(&phy->access, enable));
        break;
    case phymodLoopbackRemotePMD :
        PHYMOD_IF_ERR_RETURN(merlin16_rmt_lpbk_get(&phy->access, enable));
        break;
    case phymodLoopbackRemotePCS :
        PHYMOD_RETURN_WITH_ERR(PHYMOD_E_CONFIG,
                               (_PHYMOD_MSG("PCS Remote LoopBack not supported")));
        break;
    default :
        break;
    }             
    return PHYMOD_E_NONE;
}

int tsce16_phy_link_status_get(const phymod_phy_access_t* phy, uint32_t* link_status)
{
    PHYMOD_IF_ERR_RETURN(temod16_get_pcs_latched_link_status(&phy->access, link_status));

    return PHYMOD_E_NONE;
}

int tsce16_phy_reg_read(const phymod_phy_access_t* phy, uint32_t reg_addr, uint32_t *val)
{
    PHYMOD_IF_ERR_RETURN(phymod_tsc_iblk_read(&phy->access, reg_addr, val));
    return PHYMOD_E_NONE;
}

int tsce16_phy_reg_write(const phymod_phy_access_t* phy, uint32_t reg_addr, uint32_t val)
{
    PHYMOD_IF_ERR_RETURN(phymod_tsc_iblk_write(&phy->access, reg_addr, val));
    return PHYMOD_E_NONE;  
}

int tsce16_core_lane_map_get(const phymod_core_access_t* core, phymod_lane_map_t* lane_map)
{
    return PHYMOD_E_UNAVAIL;
}

int tsce16_phy_tx_get(const phymod_phy_access_t* phy, phymod_tx_t* tx)
{
    return PHYMOD_E_UNAVAIL;
}

int tsce16_phy_power_get(const phymod_phy_access_t* phy, phymod_phy_power_t* power)
{
    return PHYMOD_E_UNAVAIL;
}


