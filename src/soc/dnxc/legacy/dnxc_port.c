/*
 * $Id:$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_BCM_PORT
#include <shared/bsl.h>
#ifdef PORTMOD_SUPPORT

#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_port.h>
#include <soc/portmod/portmod.h>
#include <soc/portmod/portmod_common.h>
#include <sal/core/sync.h>
#include <soc/phyreg.h>

#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/legacy/drv.h>
#endif /* BCM_DNX_SUPPORT */

#include <soc/phy/phymod_port_control.h>
#include <phymod/phymod_acc.h>
#include <phymod/phymod.h>

/*simulator includes*/
#include <soc/phy/phymod_sim.h>

#define SOC_DNXC_PORT_QMX_FIRST_FSRD_CORE 2

typedef struct dnxc_port_user_access_s {
    int unit; 
    int fsrd_blk_id; 
    int fsrd_internal_quad;
    sal_mutex_t mutex; 
} dnxc_port_user_access_t;


/****************************************************************************** 
 DNXC MDIO access
*******************************************************************************/

static
int cl45_bus_read(void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t* val){
    
    DNXC_LEGACY_FIXME_ASSERT;

#ifdef FIXME_DNX_LEGACY
    dnxc_port_user_access_t *cl45_user_data;
    uint16 val16;
    int rv = 0;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    cl45_user_data = user_acc;

    
    rv = soc_dnxc_miim_cl45_read(cl45_user_data->unit, core_addr, reg_addr,  &val16);
    (*val) = val16;

    return rv;
#endif 

    return -1;
}


static
int cl45_bus_write(void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val){
    
    DNXC_LEGACY_FIXME_ASSERT;

#ifdef FIXME_DNX_LEGACY

    dnxc_port_user_access_t *cl45_user_data;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    cl45_user_data = user_acc;

    return soc_dnxc_miim_cl45_write(cl45_user_data->unit, core_addr, reg_addr, val);
#endif 

    return -1;
}



static
int cl22_bus_read(void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t* val){
    
    DNXC_LEGACY_FIXME_ASSERT;

#ifdef FIXME_DNX_LEGACY

    uint16 val16;
    int rv;
    dnxc_port_user_access_t *cl22_user_data;

    (*val) = 0;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    cl22_user_data = user_acc;

    rv = soc_dnxc_miim_cl22_read(cl22_user_data->unit, core_addr, reg_addr,&val16);
    (*val) = val16;

    return rv;
#endif 

    return -1;

}

static
int cl22_bus_write(void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val){
    
    DNXC_LEGACY_FIXME_ASSERT;

#ifdef FIXME_DNX_LEGACY

    dnxc_port_user_access_t *cl22_user_data;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    cl22_user_data = user_acc;

    return soc_dnxc_miim_cl22_write(cl22_user_data->unit, core_addr, reg_addr, val);
#endif 

    return -1;
}



static
int mdio_bus_mutex_take(void* user_acc){
    dnxc_port_user_access_t *user_data;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    user_data = (dnxc_port_user_access_t *) user_acc;

    return sal_mutex_take(user_data->mutex, sal_mutex_FOREVER); 
}

static
int mdio_bus_mutex_give(void* user_acc){
    dnxc_port_user_access_t *user_data;

    if(user_acc == NULL){
        return SOC_E_PARAM;
    }
    user_data = (dnxc_port_user_access_t *) user_acc;

    return sal_mutex_give(user_data->mutex); 
}

static int soc_dnxc_port_bus_write_disabled(void* user_acc, uint32_t* val) {
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_EASY_RELOAD_WB_COMPAT_SUPPORT)
    dnxc_port_user_access_t *user_data;
    if(user_acc == NULL){
        return SOC_E_PARAM;
    }

    user_data = (dnxc_port_user_access_t *) user_acc;
    *val = 0;
    if (SOC_IS_RELOADING(user_data->unit)) {
        *val = 1;
    }

#else
    *val = 0;
#endif

    return SOC_E_NONE;
}

static phymod_bus_t cl22_bus = {
    "dnxc_cl22_with_mutex",
    cl22_bus_read,
    cl22_bus_write,
    soc_dnxc_port_bus_write_disabled,
    mdio_bus_mutex_take,
    mdio_bus_mutex_give,
    0
};


static phymod_bus_t cl45_bus = {
    "dnxc_cl45_with_mutex",
    cl45_bus_read,
    cl45_bus_write,
    soc_dnxc_port_bus_write_disabled,
    mdio_bus_mutex_take,
    mdio_bus_mutex_give,
    0
};


static phymod_bus_t cl22_no_mutex_bus = {
    "dnxc_cl22",
    cl22_bus_read,
    cl22_bus_write,
    soc_dnxc_port_bus_write_disabled,
    NULL,
    NULL,
    0
};


static phymod_bus_t cl45_no_mutex_bus = {
    "dnxc_cl45",
    cl45_bus_read,
    cl45_bus_write,
    soc_dnxc_port_bus_write_disabled,
    NULL,
    NULL,
    0
};


/** 
 *  @brief read SoC properties to get extenal phy chain of
 *         specified phy.
 *  @param unit-
 *  @param phy - lane number
 *  @param addresses_array_size - the maximum number of phy
 *                              addresses that allowed
 *  @param addresses - (output)core addresses
 *  @param phys_in_chain - (output) number of phys read from the
 *                       SoC properties
 *  @param is_clause45 - (output) clause 45 else clause 22.
 *  @note assume that all phys in the chain work with same MDIO
 *        type.
 *  
 */
soc_error_t
soc_dnxc_external_phy_chain_info_get(int unit, int phy, int addresses_array_size, int *addresses, int *phys_in_chain, int *is_clause45)
{
    int clause;
    DNXC_INIT_FUNC_DEFS;

    clause = soc_property_port_get(unit, phy, spn_PORT_PHY_CLAUSE, 22);
    if((clause != 22) && (clause != 45)){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG_STR("invalid Clause value %d"), clause));
    }
    *phys_in_chain = soc_property_port_get_csv(unit, phy, spn_PORT_PHY_ADDR, addresses_array_size, addresses);

    *is_clause45 = (clause == 45);

exit:
    DNXC_FUNC_RETURN; 
}



/** 
 *  @brief get phymod access structure for mdio core
 *  @param unit-
 *  @param acc_data - access structure allocated in the caller
 *  @param is_clause45 - clause 45/22
 *  @param addr - core address
 *  @param access - function output. the phymod core access
 */
static soc_error_t
soc_dnxc_mdio_phy_access_get(int unit, dnxc_port_user_access_t *acc_data, int is_clause45, uint16 addr, phymod_access_t *access, int *is_sim)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(acc_data);
    DNXC_NULL_CHECK(access);
    phymod_access_t_init(access);

    PHYMOD_ACC_USER_ACC(access) = acc_data;
    PHYMOD_ACC_ADDR(access) = addr;
    if(!is_clause45){
        if(acc_data->mutex != NULL){
            PHYMOD_ACC_BUS(access) = &cl22_bus;
        }else{
            PHYMOD_ACC_BUS(access) = &cl22_no_mutex_bus;
        }
    } else{
        if(acc_data->mutex != NULL){
            PHYMOD_ACC_BUS(access) = &cl45_bus;    
        }else{
            PHYMOD_ACC_BUS(access) = &cl45_no_mutex_bus;
        }
         
        PHYMOD_ACC_F_CLAUSE45_SET(access);
    }

    PHYMOD_ACC_DEVAD(access) = 0 | PHYMOD_ACC_DEVAD_FORCE_MASK; 

    DNXC_IF_ERR_EXIT(soc_physim_check_sim(unit, phymodDispatchTypeFalcon, access, 0, is_sim));

exit:
    DNXC_FUNC_RETURN;  
}



/** 
 *  @brief get phymod access structure for mdio core via function "soc_dnxc_mdio_phy_access_get"
 *  @param unit-
 *  @param addr - core address
 *  @param access - function output. the phymod core access
 */

soc_error_t
soc_dnxc_external_phy_core_access_get(int unit, uint32 addr, phymod_access_t *access)
{
    int cl = 0, is_sim = 0, is_cl45 = 0;
    dnxc_port_user_access_t *local_user_access = NULL;
    DNXC_INIT_FUNC_DEFS;

    DNXC_ALLOC(local_user_access, dnxc_port_user_access_t, 1, "dnx_fabric_ext_phy_specific_db");
    sal_memset(local_user_access, 0, sizeof(dnxc_port_user_access_t));
    local_user_access->unit = unit;
    
    cl = soc_property_suffix_num_get(unit, -1, spn_PORT_PHY_CLAUSE  , "fabric", 45);
    if(45 == cl){
        is_cl45 = TRUE;
    } 
    else if(22 == cl){
        is_cl45 = FALSE;
    } 
    else{
        DNXC_FREE(local_user_access);
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid clause %d"), cl));
    }

    soc_dnxc_mdio_phy_access_get(unit, local_user_access, is_cl45, addr, access, &is_sim);

exit:
    DNXC_FUNC_RETURN; 
}

/****************************************************************************** 
 DNXC to portmod mapping
*******************************************************************************/

soc_error_t
dnxc_soc_to_phymod_ref_clk(int unit, int  ref_clk, phymod_ref_clk_t *phymod_ref_clk){
    DNXC_INIT_FUNC_DEFS;

    *phymod_ref_clk = phymodRefClkCount;
    switch(ref_clk){
        case soc_dnxc_init_serdes_ref_clock_disable: break;
        case soc_dnxc_init_serdes_ref_clock_125:
        case 125:
            *phymod_ref_clk = phymodRefClk125Mhz;
            break;
        case soc_dnxc_init_serdes_ref_clock_156_25:
        case 156:
            *phymod_ref_clk = phymodRefClk156Mhz;
            break;
        default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid ref clk %d"), ref_clk));
    }
exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
soc_dnxc_to_portmod_lb(int unit, soc_port_t port, soc_dnxc_loopback_mode_t dmnn_lb_mode, portmod_loopback_mode_t *portmod_lb_mode){
    DNXC_INIT_FUNC_DEFS;

    *portmod_lb_mode = portmodLoopbackCount;
    switch(dmnn_lb_mode){
    case soc_dnxc_loopback_mode_none:
        *portmod_lb_mode = portmodLoopbackCount;
        break;
    case soc_dnxc_loopback_mode_mac_async_fifo:
        *portmod_lb_mode = portmodLoopbackMacAsyncFifo;
        break;
    case soc_dnxc_loopback_mode_mac_outer:
        *portmod_lb_mode = portmodLoopbackMacOuter;
        break;
    case soc_dnxc_loopback_mode_mac_pcs:
        *portmod_lb_mode = portmodLoopbackMacPCS;
        break;
    case soc_dnxc_loopback_mode_phy_gloop:
        if(IS_SFI_PORT(unit, port) || IS_IL_PORT(unit, port)) {
            *portmod_lb_mode = portmodLoopbackPhyGloopPMD;
        } else {
            *portmod_lb_mode = portmodLoopbackPhyGloopPCS;
        }
        break;
    case soc_dnxc_loopback_mode_phy_rloop:
        *portmod_lb_mode = portmodLoopbackPhyRloopPMD;
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid lb type %d"), dmnn_lb_mode));
    }
exit:
    DNXC_FUNC_RETURN;
}


/****************************************************************************** 
 External Firmware loader
*******************************************************************************/
#define UC_TABLE_ENTRY_SIZE (4)


/** 
 *  @brief This function load firmware to all Fabric cores. IF
 *         firmware already loaded this function will do
 *         nothing.
 *         This is callback function that called during phymod
 *         core_init
 *  @param core- phymod core access
 *  @param fw_version - the firmware verision 
 *  @param fw_crc - the firmware crc
 *  @param length -  the firmware length
 *  @param data - the uCode
 *  @note assumes that all cores use the same firmware
 */
int
soc_dnxc_fabric_broadcast_firmware_loader(int unit,  uint32_t length, const uint8_t* data)
{
    soc_reg_above_64_val_t wr_data;
    int i=0;
    int word_index = 0;


    PHYMOD_IF_ERR_RETURN(WRITE_BRDC_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, 0x7)); /*all FSRDs*/

    for (i = 0 ; i < length ; i+= UC_TABLE_ENTRY_SIZE){
        SOC_REG_ABOVE_64_CLEAR(wr_data);
        if(i + UC_TABLE_ENTRY_SIZE < length){
            sal_memcpy((uint8 *)wr_data, data + i, UC_TABLE_ENTRY_SIZE);
        }else{ /*last time*/
            sal_memcpy((uint8 *)wr_data, data + i, length - i);
        }
        /*swap every 4 bytes in case of big endian*/
        for(word_index = 0 ; word_index < sizeof(soc_reg_above_64_val_t)/4; word_index++) {
            wr_data[word_index] = _shr_uint32_read((uint8 *)&wr_data[word_index]);
        }  
        /*we write to index 0 allways*/
        PHYMOD_IF_ERR_RETURN(WRITE_BRDC_FSRD_FSRD_WL_EXT_MEMm(unit, MEM_BLOCK_ALL, 0, wr_data)); 
        
    }
    
    PHYMOD_IF_ERR_RETURN(WRITE_BRDC_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, 0x0));
    

    return SOC_E_NONE;
}

static int
soc_dnxc_fabric_firmware_loader_callback(const phymod_core_access_t* core,  uint32_t length, const uint8_t* data)
{
    dnxc_port_user_access_t *user_data;
    soc_reg_above_64_val_t wr_data;
    int i=0, unit, reg_access_blk_id;
    int word_index = 0;
    uint32 quad;

    user_data = core->access.user_acc;
    unit = user_data->unit;
    reg_access_blk_id = user_data->fsrd_blk_id | SOC_REG_ADDR_BLOCK_ID_MASK;
    quad = (1 << user_data->fsrd_internal_quad);

    PHYMOD_IF_ERR_RETURN(WRITE_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, reg_access_blk_id, quad)); 
    for (i = 0 ; i < length ; i+= UC_TABLE_ENTRY_SIZE){
        SOC_REG_ABOVE_64_CLEAR(wr_data);
        if(i + UC_TABLE_ENTRY_SIZE < length){
            sal_memcpy((uint8 *)wr_data, data + i, UC_TABLE_ENTRY_SIZE);
        }else{ /*last time*/
            sal_memcpy((uint8 *)wr_data, data + i, length - i);
        }
        /*swap every 4 bytes in case of big endian*/
        for(word_index = 0 ; word_index < sizeof(soc_reg_above_64_val_t)/4; word_index++) {
            wr_data[word_index] = _shr_uint32_read((uint8 *)&wr_data[word_index]);
        }  
        /*we write to index 0 allways*/
        PHYMOD_IF_ERR_RETURN(WRITE_FSRD_FSRD_WL_EXT_MEMm(user_data->unit, user_data->fsrd_blk_id  , 0, wr_data));
    }
    PHYMOD_IF_ERR_RETURN(WRITE_FSRD_WC_UC_MEM_MASK_BITMAPr(user_data->unit, reg_access_blk_id, 0x0));

    return PHYMOD_E_NONE;
}

/****************************************************************************** 
 Fabric PMs init
*******************************************************************************/

soc_error_t
soc_dnxc_fabric_pms_add(int unit, int cores_num , int first_port, int use_mutex, int quads_in_fsrd, dnxc_core_address_get_f address_get_func, void **alloced_buffer)
{
    int i = 0, is_sim;
    int fmac_block_id, fsrd_block_id;
    int core_port_index = 0;
    int phy = first_port, port = first_port;
    soc_error_t rv;
    uint16 addr = 0;
    portmod_pm_create_info_t pm_info;
    soc_dnxc_init_serdes_ref_clock_t ref_clk = phymodRefClkCount;
    dnxc_port_user_access_t *user_data = NULL;
    uint32 tx_lane_map, rx_polarity, tx_polarity;
    int cl, is_cl45, serdes_core, mac_core, j;
    DNXC_INIT_FUNC_DEFS;

    
    user_data = sal_alloc(sizeof(dnxc_port_user_access_t)*cores_num , "user_data");
    DNXC_NULL_CHECK(user_data);
    for(i = 0 ; i < cores_num ; i++){
        user_data[i].unit = unit;
        if(use_mutex){
            user_data[i].mutex = sal_mutex_create("core mutex");
        }else {
            user_data[i].mutex = NULL;
        }
    }

    /*Clause*/
    cl = soc_property_suffix_num_get(unit, -1, spn_PORT_PHY_CLAUSE  , "fabric", 45);
    if (cl == 45)
    {
        is_cl45 = TRUE;
    } else if (cl == 22) {
        is_cl45 = FALSE;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid clause %d"), cl));
    }





    for(i = 0; i < cores_num ; i++){

        DNXC_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));

        /*no lane swap for fabric rx links*/
        pm_info.pm_specific_info.dnx_fabric.lane_map.num_of_lanes = 4;
        for( j = 0 ; j < 4 ; j++){
            pm_info.pm_specific_info.dnx_fabric.lane_map.lane_map_rx[j] = j;
        }

        pm_info.pm_specific_info.dnx_fabric.core_index = i;
            {
                pm_info.type = portmodDispatchTypeDnx_fabric;
                pm_info.pm_specific_info.dnx_fabric.first_phy_offset = 0;
                pm_info.pm_specific_info.dnx_fabric.is_over_nif = 0;
            }
        pm_info.pm_specific_info.dnx_fabric.fw_load_method = soc_property_suffix_num_get(unit, -1, spn_LOAD_FIRMWARE, "fabric", phymodFirmwareLoadMethodExternal);
        pm_info.pm_specific_info.dnx_fabric.fw_load_method &= 0xff;
        if( pm_info.pm_specific_info.dnx_fabric.fw_load_method == phymodFirmwareLoadMethodExternal){
            pm_info.pm_specific_info.dnx_fabric.external_fw_loader =  soc_dnxc_fabric_firmware_loader_callback;
        }


        port = first_port + i*4;

        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy = SOC_INFO(unit).port_l2p_mapping[port];
        } else{
            phy = port;
        }
        
        serdes_core = i;
        mac_core = i;
        if (SOC_IS_QMX(unit))
        {
            serdes_core += SOC_DNXC_PORT_QMX_FIRST_FSRD_CORE;
        }
            
        
        fmac_block_id = FMAC_BLOCK(unit, mac_core);
        fsrd_block_id = FSRD_BLOCK(unit, (serdes_core/quads_in_fsrd));
        user_data[i].fsrd_blk_id = fsrd_block_id; 
        user_data[i].fsrd_internal_quad = (serdes_core % quads_in_fsrd);

        pm_info.pm_specific_info.dnx_fabric.fmac_schan_id = SOC_BLOCK_INFO(unit, fmac_block_id).schan;
        pm_info.pm_specific_info.dnx_fabric.fsrd_schan_id = SOC_BLOCK_INFO(unit, fsrd_block_id).schan;
        pm_info.pm_specific_info.dnx_fabric.fsrd_internal_quad = user_data[i].fsrd_internal_quad;
        DNXC_IF_ERR_EXIT(address_get_func(unit, serdes_core, &addr));
        rv = soc_dnxc_mdio_phy_access_get(unit, &user_data[i], is_cl45, addr, &pm_info.pm_specific_info.dnx_fabric.access, &is_sim);
        DNXC_IF_ERR_EXIT(rv);

        if(is_sim) {
            pm_info.pm_specific_info.dnx_fabric.fw_load_method = phymodFirmwareLoadMethodNone;
        }

        ref_clk = SOC_INFO(unit).port_refclk_int[port];
        rv = dnxc_soc_to_phymod_ref_clk(unit, ref_clk, &pm_info.pm_specific_info.dnx_fabric.ref_clk);
        DNXC_IF_ERR_EXIT(rv);

        /*tx lane_swap*/
        if (!SOC_WARM_BOOT(unit))
        {
            if (SOC_IS_RAMON(unit) && soc_property_suffix_num_only_suffix_str_get(unit, i, spn_PHY_TX_LANE_MAP, "quad"))
            {
                tx_lane_map = soc_property_suffix_num_get(unit, i, spn_PHY_TX_LANE_MAP, "quad", SOC_DNXC_PORT_NO_LANE_SWAP); 
            }
            else
            {
                tx_lane_map = soc_property_suffix_num_get(unit, i, spn_PHY_TX_LANE_MAP, "fabric_quad", SOC_DNXC_PORT_NO_LANE_SWAP); 
            }
            for(core_port_index = 0 ;  core_port_index < 4 ; core_port_index++) {
                pm_info.pm_specific_info.dnx_fabric.lane_map.lane_map_tx[core_port_index] = ((tx_lane_map >> (core_port_index * 4)) & 0xf);
            }    
        } else {
            pm_info.pm_specific_info.dnx_fabric.lane_map.lane_map_tx[core_port_index] = 0xFFFFFFFF; /*invalid*/
        }
        
        /*polarity*/
        PORTMOD_PBMP_CLEAR(pm_info.phys);
        for(core_port_index = 0 ;  core_port_index < 4 ; core_port_index++){
            if (!SOC_WARM_BOOT(unit))
            {
                rx_polarity = soc_property_port_get(unit, port, spn_PHY_RX_POLARITY_FLIP, 0);
                tx_polarity = soc_property_port_get(unit, port, spn_PHY_TX_POLARITY_FLIP, 0);
                pm_info.pm_specific_info.dnx_fabric.polarity.rx_polarity |= ((rx_polarity & 0x1) << core_port_index);
                pm_info.pm_specific_info.dnx_fabric.polarity.tx_polarity |= ((tx_polarity & 0x1) << core_port_index);

            } 

            PORTMOD_PBMP_PORT_ADD(pm_info.phys, phy);
            phy++;
            port++;
        }
        DNXC_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));
    }
    *alloced_buffer = user_data;

exit:
    if(_rv != SOC_E_NONE){
        if(user_data != NULL){
            for(i = 0 ; i < cores_num ; i++){
                if(user_data[i].mutex != NULL){
                    sal_mutex_destroy(user_data[i].mutex);
                }
            }
            sal_free(user_data);
        }
    }
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_fabric_pcs_parse(int unit, char *pcs_str, int pcs_def, int *pcs)
{
    DNXC_INIT_FUNC_DEFS;

    if (pcs_str == NULL)
    {
        *pcs = pcs_def;
    } else if (!sal_strcmp(pcs_str, "KR_FEC") || !sal_strcmp(pcs_str, "2")) {
        *pcs = PORTMOD_PCS_64B66B_FEC;
    } else if (!sal_strcmp(pcs_str, "64_66") || !sal_strcmp(pcs_str, "4")) {
        *pcs = PORTMOD_PCS_64B66B;
    } else if (!sal_strcmp(pcs_str, "RS_FEC")) {
        *pcs = PORTMOD_PCS_64B66B_RS_FEC;
    } else if (!sal_strcmp(pcs_str, "LL_RS_FEC")) {
        *pcs = PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG_STR("invalid pcs %s"), pcs_str));
    }

exit:
    DNXC_FUNC_RETURN;
}
soc_error_t
soc_dnxc_fabric_port_probe(int unit, int port, dnxc_port_init_stage_t init_stage, int fw_verify, dnxc_port_fabric_init_config_t* port_config)
{
    portmod_port_add_info_t info;
    uint32 encoding_properties = 0;
    int phy = 0;
    uint32 value, value_def;
    uint32 pre_curr, main_curr, post_curr;
    uint32 pre_curr_def, main_curr_def, post_curr_def;
    uint32 is_fiber_perf;
    int enable;
    
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_WARM_BOOT(unit)) {

        DNXC_IF_ERR_EXIT(portmod_port_add_info_t_init(unit, &info));

        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy = SOC_INFO(unit).port_l2p_mapping[port];
        } else{
            phy = port;
        }
        PORTMOD_PBMP_PORT_ADD(info.phys, phy);
        info.interface_config.interface = SOC_PORT_IF_SFI;
        info.interface_config.speed = port_config->speed;
        info.interface_config.interface_modes = 0;
        info.interface_config.flags = 0;
        info.link_training_en = port_config->cl72;

        if(init_stage == dnxc_port_init_until_fw_load) {
            PORTMOD_PORT_ADD_F_INIT_CORE_PROBE_SET(&info);
            PORTMOD_PORT_ADD_F_INIT_PASS1_SET(&info);
        }

        if(init_stage == dnxc_port_init_resume_after_fw_load) {
            PORTMOD_PORT_ADD_F_INIT_PASS2_SET(&info);
        }

        if(fw_verify) {
            PORTMOD_PORT_ADD_F_FIRMWARE_LOAD_VERIFY_SET(&info);
        } else {
            PORTMOD_PORT_ADD_F_FIRMWARE_LOAD_VERIFY_CLR(&info);
        }


        DNXC_IF_ERR_EXIT(portmod_port_add(unit, port, &info));

        if(init_stage == dnxc_port_init_until_fw_load) {
            SOC_EXIT;
        }

        if (port_config->pcs == PORTMOD_PCS_64B66B_FEC)
        {
            PORTMOD_ENCODING_EXTRCT_CIG_FROM_LLFC_SET(encoding_properties);
        } else if (port_config->pcs == PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC || port_config->pcs == PORTMOD_PCS_64B66B_RS_FEC)
        {
            PORTMOD_ENCODING_LOW_LATENCY_LLFC_SET(encoding_properties);
        }

        DNXC_IF_ERR_EXIT(portmod_port_encoding_set(unit, port, encoding_properties, port_config->pcs));

        DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_get(unit, port, -1, -1, 0, SOC_PHY_CONTROL_DRIVER_CURRENT, &value_def));
        value = soc_property_port_get(unit, port, spn_SERDES_DRIVER_CURRENT, value_def);
        if (value != value_def)
        {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_DRIVER_CURRENT, value));
        }

        DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_get(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_PRE, &pre_curr_def));
        DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_get(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_MAIN, &main_curr_def));
        DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_get(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_POST, &post_curr_def));
        value_def = pre_curr_def << SOC_DNXC_PORT_SERDES_PRE_CURRENT_FIRST;
        value_def = value_def | (main_curr_def << SOC_DNXC_PORT_SERDES_MAIN_CURRENT_FIRST);
        value_def = value_def | (post_curr_def << SOC_DNXC_PORT_SERDES_POST_CURRENT_FIRST);

        value = soc_property_port_get(unit, port, spn_SERDES_PREEMPHASIS, value_def);

        pre_curr = (value & SOC_DNXC_PORT_SERDES_PRE_CURRENT_MASK) >> SOC_DNXC_PORT_SERDES_PRE_CURRENT_FIRST;
        if (pre_curr != pre_curr_def)
        {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_PRE, pre_curr));
        }

        main_curr = (value & SOC_DNXC_PORT_SERDES_MAIN_CURRENT_MASK) >> SOC_DNXC_PORT_SERDES_MAIN_CURRENT_FIRST;
        if (main_curr != main_curr_def)
        {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_MAIN, main_curr));
        }

        post_curr = (value & SOC_DNXC_PORT_SERDES_POST_CURRENT_MASK) >> SOC_DNXC_PORT_SERDES_POST_CURRENT_FIRST;
        if (post_curr != post_curr_def)
        {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_TX_FIR_POST, post_curr));
        }

        is_fiber_perf = soc_property_port_get(unit, port, spn_SERDES_FIBER_PREF, 0);
        if(is_fiber_perf) {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_MEDIUM_TYPE, SOC_PORT_MEDIUM_FIBER));
        }

        DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_get(unit, port, -1, -1, 0, SOC_PHY_CONTROL_FIRMWARE_MODE, &value_def));
        value = soc_property_port_get(unit, port, spn_SERDES_FIRMWARE_MODE, value_def);
        if (value != value_def)
        {
            DNXC_IF_ERR_EXIT(soc_dnxc_port_enable_get(unit, port , &enable));
            DNXC_IF_ERR_EXIT(soc_dnxc_port_enable_set(unit, port, 0)); 
            DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_control_set(unit, port, -1, -1, 0, SOC_PHY_CONTROL_FIRMWARE_MODE, value));
            DNXC_IF_ERR_EXIT(soc_dnxc_port_enable_set(unit, port, enable)); 
            
        }
    }


exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
soc_dnxc_port_cl72_set(int unit, soc_port_t port, int enable)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(portmod_port_cl72_set(unit, port, (enable ? 1 : 0))); 

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_cl72_get(int unit, soc_port_t port, int *enable)
{
    uint32 local_enable;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_cl72_get(unit, port, &local_enable)); 
    *enable = (local_enable ? 1 : 0);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_phy_control_set(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 value)
{
    phymod_phy_access_t phys[SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT];
    int phys_returned;
    portmod_access_get_params_t params;
    int is_lane_control, rv;
    phymod_ref_clk_t ref_clk;
    int is_legacy_ext_phy = 0;
    uint32_t port_dynamic_state = 0;
    DNXC_INIT_FUNC_DEFS;

    rv = dnxc_soc_to_phymod_ref_clk(unit, SOC_INFO(unit).port_refclk_int[port], &ref_clk);
    DNXC_IF_ERR_EXIT(rv);

    portmod_access_get_params_t_init(unit, &params);
    params.lane = lane;
    params.phyn = (phyn == -1 ? PORTMOD_PHYN_LAST_ONE : phyn);
    params.sys_side = is_sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE;

    DNXC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT, phys, &phys_returned, NULL));
    switch (type)
    {
        /*not per lane control*/
        case SOC_PHY_CONTROL_LANE_SWAP:
            is_lane_control = 0;
            break;
        default:
            is_lane_control = 1;
    }

    if(IS_IL_PORT(unit, port) || IS_SFI_PORT(unit, port)) { 
        is_legacy_ext_phy = 0;
    } else if( params.phyn != 0) { /* only check if phy is legacy when phyn is not 0 (in portmod internal phy is always not legacy) */
        DNXC_IF_ERR_EXIT(portmod_port_is_legacy_ext_phy_present(unit, port, &is_legacy_ext_phy));
    }

    if (!is_legacy_ext_phy) { 
        if(type != SOC_PHY_CONTROL_AUTONEG_MODE){
            DNXC_IF_ERR_EXIT(soc_port_control_set_wrapper(unit, ref_clk, is_lane_control, phys, phys_returned, type, value));
        }
        else{
            port_dynamic_state |= 0x2;
            port_dynamic_state |= value << 16;
            portmod_port_update_dynamic_state(unit, port, port_dynamic_state);
        }
    } else {
        DNXC_IF_ERR_EXIT(portmod_port_ext_phy_control_set(unit, port, phyn, lane, is_sys_side, type, value));
    }

exit:
    DNXC_FUNC_RETURN;
}



soc_error_t
soc_dnxc_port_phy_control_get(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 *value)
{
    phymod_phy_access_t phys[SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT];
    int phys_returned;
    portmod_access_get_params_t params; 
    int is_lane_control, rv;
    phymod_ref_clk_t ref_clk;
    int is_legacy_ext_phy = 0;
    DNXC_INIT_FUNC_DEFS;

    rv = dnxc_soc_to_phymod_ref_clk(unit, SOC_INFO(unit).port_refclk_int[port], &ref_clk);
    DNXC_IF_ERR_EXIT(rv);

    portmod_access_get_params_t_init(unit, &params);
    params.lane = lane;
    params.phyn = (phyn == -1 ? PORTMOD_PHYN_LAST_ONE : phyn);
    params.sys_side = is_sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE;

    DNXC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT, phys, &phys_returned, NULL));
    switch (type)
    {
        /*not per lane control*/
        case SOC_PHY_CONTROL_LANE_SWAP:
            is_lane_control = 0;
            break;
        default:
            is_lane_control = 1;
    }

    if(IS_IL_PORT(unit, port) || IS_SFI_PORT(unit, port)) { 
        is_legacy_ext_phy = 0;
    } else if( params.phyn != 0) { /* only check if phy is legacy when phyn is not 0 (in portmod internal phy is always not legacy) */
        DNXC_IF_ERR_EXIT(portmod_port_is_legacy_ext_phy_present(unit, port, &is_legacy_ext_phy));
    }

    if (!is_legacy_ext_phy) {
        DNXC_IF_ERR_EXIT(soc_port_control_get_wrapper(unit, ref_clk, is_lane_control, phys, phys_returned, type, value));
    } else {
        DNXC_IF_ERR_EXIT(portmod_port_ext_phy_control_get(unit, port, phyn, lane, is_sys_side, type, value));
    }

exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
soc_dnxc_port_config_get(int unit, soc_port_t port, dnxc_port_fabric_init_config_t* port_config)
{
    int speed;
    char *pcs_str;
    DNXC_INIT_FUNC_DEFS;

    /* Update according to soc properties */
    pcs_str = soc_property_port_get_str(unit, port, spn_BACKPLANE_SERDES_ENCODING);
    DNXC_IF_ERR_EXIT(soc_dnxc_fabric_pcs_parse(unit, pcs_str, port_config->pcs, &(port_config->pcs)));
    speed = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, port_config->speed);
    if((speed != -1) && (speed != 0))
    {
        port_config->speed = speed;
    }
    port_config->cl72 = soc_property_port_get(unit, port, spn_PORT_INIT_CL72, port_config->cl72) ? 1 : 0;

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_phy_reg_get(int unit, soc_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 *phy_data)
{
    uint16 phy_rd_data = 0;
    uint32 reg_flag;
    int    rv;
    int nof_phys_structs = 0;
    phymod_phy_access_t phy_access[SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT];
    portmod_access_get_params_t params;
    DNXC_INIT_FUNC_DEFS;

    portmod_access_get_params_t_init(unit, &params);
    if(flags & SOC_PHY_INTERNAL){
        params.phyn = 0;
    }

    /*clause = (flags & SOC_PHY_CLAUSE45 ? 45 : 22);*/
    if ((flags & SOC_PHY_NOMAP) == 0){
        rv = portmod_port_phy_lane_access_get(unit, port, &params, SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT, phy_access, &nof_phys_structs, NULL);
        DNXC_IF_ERR_EXIT(rv);
    }

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported flags for ramon")));
    } else {
        reg_flag = SOC_PHY_REG_FLAGS(phy_reg_addr);
        if (reg_flag & SOC_PHY_REG_INDIRECT) {
            if (flags & BCM_PORT_PHY_NOMAP) {
                /* Indirect register access is performed through PHY driver.
                 * Therefore, indirect register access is not supported if
                 * BCM_PORT_PHY_NOMAP flag is set.
                 */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("indirect register access is not supported if SOC_PHY_NOMAP flag is set")));
            }
            phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
            rv = phymod_phy_reg_read(phy_access, phy_reg_addr, phy_data);
            DNXC_IF_ERR_EXIT(rv);
        } else {
            if (flags & SOC_PHY_NOMAP) {
                
#ifdef FIXME_DNX_LEGACY
                phy_id = port;
                phy_reg = phy_reg_addr;
                rv = soc_dnxc_miim_read(unit, clause, phy_id, phy_reg, &phy_rd_data);
                DNXC_IF_ERR_EXIT(rv);
#endif 
                DNXC_LEGACY_FIXME_ASSERT;
                *phy_data = phy_rd_data;
            } else{
                PHYMOD_LOCK_TAKE(phy_access);
                rv = PHYMOD_BUS_READ(&phy_access[0].access, phy_reg_addr, phy_data);
                PHYMOD_LOCK_GIVE(phy_access);
                DNXC_IF_ERR_EXIT(rv);
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_phy_reg_set(int unit, soc_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 phy_data)
{ 
    uint32 reg_flag;
    int    rv;
    int nof_phys_structs = 0;
    phymod_phy_access_t phy_access[SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT];
    portmod_access_get_params_t params;
    DNXC_INIT_FUNC_DEFS;

    portmod_access_get_params_t_init(unit, &params);
    if(flags & SOC_PHY_INTERNAL){
        params.phyn = 0;
    }
    
#ifdef FIXME_DNX_LEGACY
    clause = (flags & SOC_PHY_CLAUSE45 ? 45 : 22);
#endif 
    DNXC_LEGACY_FIXME_ASSERT;
    if ((flags & SOC_PHY_NOMAP) == 0){
        rv = portmod_port_phy_lane_access_get(unit, port, &params, SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT, phy_access, &nof_phys_structs, NULL);
        DNXC_IF_ERR_EXIT(rv);
    }

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported flags for ramon")));
    } else {
        reg_flag = SOC_PHY_REG_FLAGS(phy_reg_addr);
        if (reg_flag & SOC_PHY_REG_INDIRECT) {
            if (flags & BCM_PORT_PHY_NOMAP) {
                /* Indirect register access is performed through PHY driver.
                 * Therefore, indirect register access is not supported if
                 * SOC_PHY_NOMAP flag is set.
                 */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("indirect register access is not supported if SOC_PHY_NOMAP flag is set")));
            }
            phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
            rv = phymod_phy_reg_write(phy_access, phy_reg_addr, phy_data);
            DNXC_IF_ERR_EXIT(rv);
        } else {
            if (flags & BCM_PORT_PHY_NOMAP) {
                
#ifdef FIXME_DNX_LEGACY
                phy_id = port;
                phy_reg = phy_reg_addr;
                phy_wr_data = phy_data & 0xffff;
                rv = soc_dnxc_miim_write(unit, clause, phy_id, phy_reg, phy_wr_data);
                DNXC_IF_ERR_EXIT(rv);
#endif 
                DNXC_LEGACY_FIXME_ASSERT;
            } else{
                PHYMOD_LOCK_TAKE(phy_access);
                rv = PHYMOD_BUS_WRITE(&phy_access[0].access, phy_reg_addr, phy_data);
                PHYMOD_LOCK_GIVE(phy_access);
                DNXC_IF_ERR_EXIT(rv);
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_phy_reg_modify(int unit, soc_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint32 phy_rd_data;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_reg_get(unit, port, flags, phy_reg_addr, &phy_rd_data));
    phy_data |= (phy_rd_data & ~phy_mask);
    DNXC_IF_ERR_EXIT(soc_dnxc_port_phy_reg_set(unit, port, flags, phy_reg_addr, phy_data));

exit:
    DNXC_FUNC_RETURN;
}

/****************************************************************************** 
 Fabric Port Controls
*******************************************************************************/

soc_error_t 
soc_dnxc_port_control_pcs_set(int unit, soc_port_t port, soc_dnxc_port_pcs_t pcs)
{
    uint32 properties = 0;
    DNXC_INIT_FUNC_DEFS;
    if (pcs == soc_dnxc_port_pcs_64_66_fec)
    {
        PORTMOD_ENCODING_EXTRCT_CIG_FROM_LLFC_SET(properties); /*CIG From llfc is enabled by default*/
    } else if (pcs == soc_dnxc_port_pcs_64_66_rs_fec|| pcs == soc_dnxc_port_pcs_64_66_ll_rs_fec)
    {
        PORTMOD_ENCODING_LOW_LATENCY_LLFC_SET(properties);
    }
    DNXC_IF_ERR_EXIT(portmod_port_encoding_set(unit, port, properties, (_shr_port_pcs_t) pcs));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_control_pcs_get(int unit, soc_port_t port, soc_dnxc_port_pcs_t* pcs)
{
    uint32 properties = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, (portmod_port_pcs_t *) pcs));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_control_low_latency_llfc_set(int unit, soc_port_t port, int value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));

    if ((encoding != PORTMOD_PCS_64B66B_FEC)
        && (encoding != PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC)
        && (encoding != PORTMOD_PCS_64B66B_RS_FEC))
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Low latency LLFC contorl is supported only by KR_FEC, RS_FEC or LL_RS_FEC")));
    }
    if(value){
        properties |= PORTMOD_ENCODING_LOW_LATENCY_LLFC;
    } else{
        properties &= ~PORTMOD_ENCODING_LOW_LATENCY_LLFC;
    }
    DNXC_IF_ERR_EXIT(portmod_port_encoding_set(unit, port, properties, encoding));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_control_low_latency_llfc_get(int unit, soc_port_t port, int *value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));
    if ((encoding != PORTMOD_PCS_64B66B_FEC)
        && (encoding != PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC)
        && (encoding != PORTMOD_PCS_64B66B_RS_FEC))
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Low latency LLFC contorl is supported only by KR_FEC, RS_FEC or LL_RS_FEC")));
    }
    *value = PORTMOD_ENCODING_LOW_LATENCY_LLFC_GET(properties);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_control_fec_error_detect_set(int unit, soc_port_t port, int value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));
    if (encoding != PORTMOD_PCS_64B66B_FEC && encoding != PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC && encoding != PORTMOD_PCS_64B66B_RS_FEC)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error detect contorl is supported only by KR_FEC, RS_FEC or LL_RS_FEC")));
    }

    if(value){
        properties |= PORTMOD_ENCODING_FEC_ERROR_DETECT;
    } else{
        properties &= ~PORTMOD_ENCODING_FEC_ERROR_DETECT;
    }

    DNXC_IF_ERR_EXIT(portmod_port_encoding_set(unit, port, properties, encoding));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_control_fec_error_detect_get(int unit, soc_port_t port, int *value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));
    if (encoding != PORTMOD_PCS_64B66B_FEC && encoding != PORTMOD_PCS_64B66B_LOW_LATENCY_RS_FEC && encoding != PORTMOD_PCS_64B66B_RS_FEC)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error detect is supported only by KR_FEC, RS_FEC or LL_RS_FEC")));
    }

    *value = PORTMOD_ENCODING_FEC_ERROR_DETECT_GET(properties);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_control_power_set(int unit, soc_port_t port, uint32 flags, soc_dnxc_port_power_t power)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_set(unit, port, 0, soc_dnxc_port_power_on == power ? 1 : 0));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_control_power_get(int unit, soc_port_t port, soc_dnxc_port_power_t* power)
{
    int enable;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_get(unit, port, 0, &enable));
    *power =  enable ? soc_dnxc_port_power_on : soc_dnxc_port_power_off;

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_control_rx_enable_set(int unit, soc_port_t port, uint32 flags, int enable)
{
    /*Maybe has to set PORTMOD_PORT_ENABLE_MAC flag*/
    int portmod_enable_flags = PORTMOD_PORT_ENABLE_RX;
    DNXC_INIT_FUNC_DEFS;

    if(flags & SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE){
        portmod_enable_flags |= PORTMOD_PORT_ENABLE_MAC ;
    }
    DNXC_IF_ERR_EXIT(portmod_port_enable_set(unit, port, portmod_enable_flags, enable));

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
soc_dnxc_port_control_tx_enable_set(int unit, soc_port_t port, int enable)
{
    int portmod_enable_flags = PORTMOD_PORT_ENABLE_TX;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_set(unit, port, portmod_enable_flags, enable));
exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
soc_dnxc_port_control_rx_enable_get(int unit, soc_port_t port, int* enable)
{
    int portmod_enable_flags = PORTMOD_PORT_ENABLE_RX | PORTMOD_PORT_ENABLE_MAC;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_get(unit, port, portmod_enable_flags, enable));
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_control_tx_enable_get(int unit, soc_port_t port, int* enable)
{
    int portmod_enable_flags = PORTMOD_PORT_ENABLE_TX | PORTMOD_PORT_ENABLE_MAC;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_get(unit, port, portmod_enable_flags, enable));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_prbs_tx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int flags = 0;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_TX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_enable_set(unit, port, portmod_mode, flags, value));

exit:
    DNXC_FUNC_RETURN;
}


soc_error_t 
soc_dnxc_port_prbs_rx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int flags = 0;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_RX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_enable_set(unit, port, portmod_mode, flags, value));

exit:
    DNXC_FUNC_RETURN;
    
}

soc_error_t 
soc_dnxc_port_prbs_tx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *value)
{
    int flags = 0;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_TX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_enable_get(unit, port, portmod_mode, flags, value));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_prbs_rx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *value)
{
    int flags = 0;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_RX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_enable_get(unit, port, portmod_mode, flags, value));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_prbs_rx_status_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    phymod_prbs_status_t status;
    int flags = PHYMOD_PRBS_STATUS_F_CLEAR_ON_READ;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    DNXC_IF_ERR_EXIT(phymod_prbs_status_t_init(&status));
    DNXC_IF_ERR_EXIT(portmod_port_prbs_status_get(unit, port, portmod_mode, flags, &status));
    if(status.prbs_lock_loss) {
        *value = -2;
    } else if(!status.prbs_lock) {
        *value = -1;
    } else {
        *value = status.error_count;
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_prbs_tx_invert_data_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *invert)
{
    int flags = 0;
    phymod_prbs_t config;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_TX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_get(unit, port, portmod_mode, flags, &config));
    *invert = config.invert ? 1: 0;

exit:
    DNXC_FUNC_RETURN;
}


soc_error_t 
soc_dnxc_port_prbs_tx_invert_data_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int invert)
{
    int flags = 0;
    phymod_prbs_t config;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    PHYMOD_PRBS_DIRECTION_TX_SET(flags);
    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_get(unit, port, portmod_mode, flags, &config));
    config.invert = invert ? 1: 0;
    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_set(unit, port, portmod_mode, flags, &config));

exit:
    DNXC_FUNC_RETURN;
}


soc_error_t 
soc_dnxc_port_prbs_polynomial_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int flags = 0;
    phymod_prbs_poly_t poly;
    phymod_prbs_t config;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_get(unit, port, portmod_mode, flags, &config));
    DNXC_IF_ERR_EXIT(soc_prbs_poly_to_phymod(value, &poly));
    config.poly = poly;
    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_set(unit, port, portmod_mode, flags, &config));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_prbs_polynomial_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int *value)
{
    int flags = 0;
    phymod_prbs_t config;
    portmod_prbs_mode_t portmod_mode;
    DNXC_INIT_FUNC_DEFS;

    portmod_mode = (mode == soc_dnxc_port_prbs_mode_phy ? portmodPrbsModePhy : portmodPrbsModeMac);

    DNXC_IF_ERR_EXIT(portmod_port_prbs_config_get(unit, port, portmod_mode, flags, &config));
    DNXC_IF_ERR_EXIT(phymod_prbs_poly_to_soc(config.poly, (uint32*) value));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_loopback_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback)
{
    soc_dnxc_loopback_mode_t current_lb = soc_dnxc_loopback_mode_none;
    portmod_loopback_mode_t portmod_lb_mode = portmodLoopbackCount;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_dnxc_port_loopback_get(unit, port, &current_lb));
    if(current_lb == loopback){
        /*nothing to do*/
        SOC_EXIT;
    }
    if(current_lb != soc_dnxc_loopback_mode_none){
        /*open exist loopback*/
        DNXC_IF_ERR_EXIT(soc_dnxc_to_portmod_lb(unit, port, current_lb, &portmod_lb_mode));
        DNXC_IF_ERR_EXIT(portmod_port_loopback_set(unit, port, portmod_lb_mode, 0));
    }
    if(loopback != soc_dnxc_loopback_mode_none){
        /*define the new loopback*/
        DNXC_IF_ERR_EXIT(soc_dnxc_to_portmod_lb(unit, port, loopback, &portmod_lb_mode));
        DNXC_IF_ERR_EXIT(portmod_port_loopback_set(unit, port, portmod_lb_mode, 1));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_loopback_get(int unit, soc_port_t port, soc_dnxc_loopback_mode_t* loopback)
{
    portmod_loopback_mode_t supported_lb_modes[] = { portmodLoopbackMacAsyncFifo, portmodLoopbackMacOuter, portmodLoopbackMacPCS,
        portmodLoopbackPhyGloopPMD,portmodLoopbackPhyGloopPCS,portmodLoopbackPhyRloopPMD};
    soc_dnxc_loopback_mode_t mapped_moeds[] = {soc_dnxc_loopback_mode_mac_async_fifo, soc_dnxc_loopback_mode_mac_outer, soc_dnxc_loopback_mode_mac_pcs,
        soc_dnxc_loopback_mode_phy_gloop, soc_dnxc_loopback_mode_phy_gloop, soc_dnxc_loopback_mode_phy_rloop};
    int i = 0, rv;
    int enable = 0;
    DNXC_INIT_FUNC_DEFS;

    *loopback = soc_dnxc_loopback_mode_none;
    for(i = 0 ; i < COUNTOF(supported_lb_modes); i++){

        if(IS_SFI_PORT(unit, port) || IS_IL_PORT(unit, port)) {
            if(supported_lb_modes[i] == portmodLoopbackPhyGloopPCS) {
                continue;
            }
        }
        rv = portmod_port_loopback_get(unit, port, supported_lb_modes[i], &enable);
        if(rv == SOC_E_UNAVAIL) {
            /* looback type is not supported for the PM */
            continue; 
        }
        DNXC_IF_ERR_EXIT(rv);
        if(enable){
            *loopback = mapped_moeds[i];
            break;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_enable_set(int unit, soc_port_t port, int enable)
{

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_set(unit, port, 0, enable? 1 : 0));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_enable_get(int unit, soc_port_t port, int *enable)
{

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(portmod_port_enable_get(unit, port, 0, enable));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_dnxc_port_rx_locked_get(int unit, soc_port_t port, uint32 *rx_locked)
{
    phymod_phy_access_t phys[SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT];
    int phys_returned;
    portmod_access_get_params_t params;
    DNXC_INIT_FUNC_DEFS;

    portmod_access_get_params_t_init(unit, &params);
    params.lane = -1;
    params.phyn = PORTMOD_PHYN_LAST_ONE;
    params.sys_side = PORTMOD_SIDE_LINE;
    DNXC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, SOC_DNXC_PORT_MAX_CORE_ACCESS_PER_PORT, phys, &phys_returned, NULL));

    DNXC_IF_ERR_EXIT(phymod_phy_rx_pmd_locked_get(phys, rx_locked));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxc_port_extract_cig_from_llfc_enable_set(int unit, soc_port_t port, int value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));
    if (encoding != PORTMOD_PCS_64B66B_FEC)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Extract Congrstion Ind from LLFC cells contorl is supported only by KR_FEC")));
    }
    if(value){
        properties |= PORTMOD_ENCODING_EXTRCT_CIG_FROM_LLFC;
    } else{
        properties &= ~PORTMOD_ENCODING_EXTRCT_CIG_FROM_LLFC;
    }
    DNXC_IF_ERR_EXIT(portmod_port_encoding_set(unit, port, properties, encoding));
exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
soc_dnxc_port_extract_cig_from_llfc_enable_get(int unit, soc_port_t port, int *value)
{
    uint32 properties = 0;
    portmod_port_pcs_t encoding;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(portmod_port_encoding_get(unit, port, &properties, &encoding));
    if (encoding != PORTMOD_PCS_64B66B_FEC)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Extract Congrstion Ind from LLFC cells contorl is supported only by KR_FEC")));
    }
    *value = PORTMOD_ENCODING_EXTRCT_CIG_FROM_LLFC_GET(properties);
exit:
    DNXC_FUNC_RETURN;
}


#endif /*PORTMOD_SUPPORT*/

#undef _ERR_MSG_MODULE_NAME

