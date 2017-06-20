/*
 *         
 * $Id:$
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 *
 */
#include <soc/portmod/portmod.h>
#include <soc/portmod/portmod_internal.h>
#include <soc/portmod/portmod_common.h>
#include <soc/portmod/portmod_system.h>
#include <soc/portmod/portmod_chain.h>
#include <soc/portmod/portmod_legacy_phy.h>
#include <soc/phy/phymod_port_control.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#include <soc/dcmn/dcmn_crash_recovery.h>
#endif /* CRASH_RECOVERY_SUPPORT */

#ifdef _ERR_MSG_MODULE_NAME 
#error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_PORT


#define PORT_MAX_PHY_ACCESS_STRUCTURES (6)
#define PM_MDIO_BUS_COUNT (8)

#include <soc/portmod/portmod_dispatch.h>

STATIC int _portmod_common_ext_phy_addr_get(int unit, soc_port_t port, uint32 *xphy_id);

int
portmod_common_phy_prbs_config_set(int unit, int port, pm_info_t pm_info, int flags, const phymod_prbs_t* config)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;


    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_prbs_config_set(unit, phy_access, chain_length, flags, config));

exit:
    SOC_FUNC_RETURN; 
}

int
portmod_common_phy_prbs_config_get(int unit, int port, pm_info_t pm_info, int flags, phymod_prbs_t* config)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_prbs_config_get(unit, phy_access, chain_length, flags, config));
    
exit:
    SOC_FUNC_RETURN; 
}

int portmod_commmon_portmod_to_phymod_loopback_type(int unit, portmod_loopback_mode_t loopback_type, phymod_loopback_mode_t *phymod_lb_type)
{
        
    SOC_INIT_FUNC_DEFS;
    switch(loopback_type){
    case portmodLoopbackPhyGloopPCS:
        *phymod_lb_type = phymodLoopbackGlobal;
        break;
    case portmodLoopbackPhyGloopPMD:
        *phymod_lb_type = phymodLoopbackGlobalPMD;
        break;
    case portmodLoopbackPhyRloopPCS:
        *phymod_lb_type = phymodLoopbackRemotePCS;
        break;
    case portmodLoopbackPhyRloopPMD:
        *phymod_lb_type = phymodLoopbackRemotePMD;
        break;
    default:
        _SOC_EXIT_WITH_ERR(SOC_E_PARAM, ("unsupported loopback type %d", loopback_type));
    }
exit:
    SOC_FUNC_RETURN; 
}

int
portmod_common_phy_loopback_set(int unit, int port, pm_info_t pm_info, portmod_loopback_mode_t loopback_type, int enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    phymod_loopback_mode_t phymod_lb_type;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_commmon_portmod_to_phymod_loopback_type(unit, loopback_type, &phymod_lb_type));
    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_loopback_set (unit, port, phy_access, chain_length, phymod_lb_type, enable));

exit:
    SOC_FUNC_RETURN; 
}

int
portmod_common_phy_loopback_get(int unit, int port, pm_info_t pm_info, portmod_loopback_mode_t loopback_type, int *enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    phymod_loopback_mode_t phymod_lb_type;
    uint32_t tmp_enable=0;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_commmon_portmod_to_phymod_loopback_type(unit, loopback_type, &phymod_lb_type));
    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_loopback_get(unit, port,  phy_access, chain_length, phymod_lb_type, &tmp_enable));
     *enable = tmp_enable;
exit:
    SOC_FUNC_RETURN; 
}


int portmod_common_phy_prbs_enable_set(int unit, int port, pm_info_t pm_info, int flags, int enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access,
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES,
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_prbs_enable_set(unit, phy_access, chain_length, flags, enable));

exit:
    SOC_FUNC_RETURN;
}


int portmod_common_phy_prbs_enable_get(int unit, int port, pm_info_t pm_info, int flags, int* enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    uint32_t tmp_enable;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access,
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES,
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_prbs_enable_get(unit, phy_access, chain_length, flags, &tmp_enable));
    *enable = tmp_enable;
exit:
    SOC_FUNC_RETURN;
}


int portmod_common_phy_prbs_status_get(int unit, int port, pm_info_t pm_info, int flags, phymod_prbs_status_t* status)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;

    SOC_INIT_FUNC_DEFS;
    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access,
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES,
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_phychain_prbs_status_get(unit, phy_access, chain_length, flags, status));
exit:
    SOC_FUNC_RETURN;
}

int portmod_common_phy_firmware_mode_set(int unit, int port, phymod_firmware_mode_t fw_mode)
{
    SOC_INIT_FUNC_DEFS;
    SOC_FUNC_RETURN; 
}


int portmod_common_phy_firmware_mode_get(int unit, int port, phymod_firmware_mode_t *fw_mode)
{
    SOC_INIT_FUNC_DEFS;
    SOC_FUNC_RETURN; 
}

/** 
 *  @brief generic function for register write for PM4X25,
 *         PM4X10, PM 12X10
 *  @param reg_access_mem - the memory that used for access PHY
 *                        registers. e.g: CLPORT_UCMEM_DATAm,
 *                        XLPORT_UCMEM_DATA
 *  @param user_acc - user data. this function expect to get
 *                  portmod_phy_op_data_t structure
 *  @param core_address -  only 5 bits of PHY id used in case
 *                       of sbus
 *  @param reg_addr - the reigister addrress
 *  @param val - the value to write to register. 16 bits value +
 *             16 bits mask

 */
int
portmod_common_phy_sbus_reg_write(soc_mem_t reg_access_mem, void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val)
{
    int rv= SOC_E_NONE;
    soc_reg_above_64_val_t mem_data;
    portmod_default_user_access_t *user_data = user_acc;
    uint32 data, data_mask;
    SOC_REG_ABOVE_64_CLEAR(mem_data);

    if(user_data == NULL){
        return SOC_E_PARAM;
    }

    /* If write mask (upper 16 bits) is empty, add full mask */
    if ((val & 0xffff0000) == 0) {
        val |= 0xffff0000;
    }

    /* assigning TSC register address to ucmem_data[31:0]  and write the 
     * data/datamask to to ucmem_data[63:32] */
    mem_data[0] = (reg_addr & 0xffffffff) | ((core_addr & 0x1f) << 19);
    /* data: ucmem_data[48:63]
       datamask: ucmem_data[32:47]
    */
    data = (val & 0xffff) << 16;
    data_mask = (~val & 0xffff0000) >> 16;
    mem_data[1] = data | data_mask;
    mem_data[2] = 1; /* for TSC register write */

#ifdef CRASH_RECOVERY_SUPPORT
        if (BCM_UNIT_DO_HW_READ_WRITE(user_data->unit)){
            soc_dcmn_cr_suppress(user_data->unit, dcmn_cr_no_support_portmod);
        }
#endif /* CRASH_RECOVERY_SUPPORT */

    rv = soc_mem_write(user_data->unit, reg_access_mem, user_data->blk_id, 0, mem_data); 

    LOG_DEBUG(BSL_LS_SOC_MII,
              (BSL_META_U(user_data->unit,
                          "_portmod_utils_sbus_reg_write[%d]: "
                          "addr=0x%x reg=0x%08x data=0x%08x mask=0x%08x(%d/%d)\n"),
               user_data->unit, core_addr, reg_addr, val , data_mask, user_data->blk_id, rv));

    return rv;
}


/** 
 *  @brief generic function for register read for PM4X25,
 *         PM4X10, PM 12X10
 *  @param reg_access_mem - the memory that used for access PHY
 *                        registers. e.g: CLPORT_UCMEM_DATAm,
 *                        XLPORT_UCMEM_DATA
 *  @param user_acc - user data. this function expect to get
 *                  portmod_phy_op_data_t structure
 *  @param core_address -  only 5 bits of PHY id used in case
 *                       of sbus
 *  @param reg_addr - the reigister addrress
 *  @param val - the value to write to register. no mask for
 *             read
 */
int
portmod_common_phy_sbus_reg_read(soc_mem_t reg_access_mem, void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t *val)
{
    int rv = SOC_E_NONE, reg_val_offset;
    soc_reg_above_64_val_t mem_data;
    portmod_default_user_access_t *user_data = user_acc;
    SOC_REG_ABOVE_64_CLEAR(mem_data);

    if(user_data == NULL){
        return SOC_E_PARAM;
    }
    
    /* assigning TSC register address to ucmem_data[31:0] */
    mem_data[0] = (reg_addr & 0xffffffff) | ((core_addr & 0x1f) << 19);
    mem_data[2] = 0; /* for TSC register READ */

#if defined(BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT)
#ifdef CRASH_RECOVERY_SUPPORT
    soc_hw_set_immediate_hw_access(user_data->unit);
#endif /* CRASH_RECOVERY_SUPPORT */
#endif /* defined(BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) */

    SOC_ALLOW_WB_WRITE(user_data->unit, soc_mem_write(user_data->unit, reg_access_mem, user_data->blk_id, 0, mem_data), rv);
   
    /* read data back from ucmem_data[47:32] */
    if (SOC_SUCCESS(rv)) {
        rv = soc_mem_read(user_data->unit, reg_access_mem, user_data->blk_id, 0, mem_data);
    }

#if defined(BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT)
#ifdef CRASH_RECOVERY_SUPPORT
    soc_hw_restore_immediate_hw_access(user_data->unit);
#endif /* CRASH_RECOVERY_SUPPORT */
#endif /* defined(BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) */

    if (PORTMOD_USER_ACCESS_REG_VAL_OFFSET_ZERO_GET(user_data)) {
        reg_val_offset = 0;
    } else {
        reg_val_offset = 1; /* default behaviour */
    }

    *val = mem_data[reg_val_offset];

    LOG_DEBUG(BSL_LS_SOC_MII, (BSL_META_U(user_data->unit,
                 "_portmod_utils_sbus_reg_read[%d]: "
                 "addr=0x%x reg=0x%08x data=0x%08x (%d/%d)\n"),
                 user_data->unit, core_addr, reg_addr, *val, user_data->blk_id, rv));

    return rv;
}

int
portmod_common_mutex_take(void* user_acc)
{
    portmod_default_user_access_t* user_data;
    SOC_INIT_FUNC_DEFS;

    user_data = (portmod_default_user_access_t*)user_acc;

    if(user_data->mutex != NULL) {
        _SOC_IF_ERR_EXIT(sal_mutex_take(user_data->mutex, sal_mutex_FOREVER));
    }

exit:
    SOC_FUNC_RETURN;
}

int 
portmod_common_mutex_give(void* user_acc)
{
    portmod_default_user_access_t* user_data;
    SOC_INIT_FUNC_DEFS;

    user_data = (portmod_default_user_access_t*)user_acc;

    if(user_data->mutex != NULL) {
        _SOC_IF_ERR_EXIT(sal_mutex_give(user_data->mutex));
    }

exit:
    SOC_FUNC_RETURN;
}

static
int _portmod_dma_buf_alloc(int unit, const uint8 *data, uint32 datalen,
                           portmod_ucode_buf_t *dmabuf, portmod_ucode_buf_order_t data_swap,
                           int endian, int entry_bytes)
{
    int count, extra_bytes, i, j;
    uint8 *dma_buf_ptr;
    const uint8 *array_ptr;
    int arr_pos_le[3] [16]
        = {{15, 14, 13, 12, 11, 10,  9,  8, 7, 6, 5, 4, 3, 2, 1, 0},
           { 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7},
           { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};
    int arr_pos_be[3] [16]
        = {{12, 13, 14, 15,  8,  9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3},
            {11, 10,  9,  8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4},
            {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12}};
    int *arr_pos;
    SOC_INIT_FUNC_DEFS;

    if(endian) {
        arr_pos = arr_pos_be[data_swap];
    } else {
        arr_pos = arr_pos_le[data_swap];
    }

    count = datalen / entry_bytes;
    extra_bytes = datalen % entry_bytes;
    dmabuf->ucode_alloc_size = datalen;

    /* align to entry_bytes */
    if (extra_bytes != 0) {
        dmabuf->ucode_alloc_size += entry_bytes - extra_bytes;
    }

    dmabuf->ucode_dma_buf = portmod_sys_dma_alloc(unit, dmabuf->ucode_alloc_size,
                                                  "WC ucode DMA buffer");
    if (dmabuf->ucode_dma_buf == NULL) {
        _SOC_EXIT_WITH_ERR(SOC_E_MEMORY, ("Failed to alloc WC ucode DMA buffer"));
    }

    /* copy image into dma buffer */
    array_ptr = data;
    dma_buf_ptr = dmabuf->ucode_dma_buf;
    for (i = 0; i < count; i++) {
        for (j = 0; j < 16; j++) {
            dma_buf_ptr[arr_pos[j]] = array_ptr[j];
        }
        array_ptr += entry_bytes;
        dma_buf_ptr += entry_bytes;
    }
    if (extra_bytes != 0) {
        sal_memset(dma_buf_ptr, 0, entry_bytes);
        for (j = 0; j < extra_bytes; j++) {
            dma_buf_ptr[arr_pos[j]] = array_ptr[j];
        }
    }

exit:
    SOC_FUNC_RETURN;
}

/**
 * this function will download ucode through UCMEM, if ucode size exceeds UCMEM limitation,
 * the second DMA buffer is required to download extra data
 */
int
portmod_firmware_set(int unit,
                     int blk_id,
                     const uint8 *array,
                     uint32 datalen,
                     portmod_ucode_buf_order_t data_swap,
                     portmod_ucode_buf_t* buf,
                     portmod_ucode_buf_t* buf_2nd,
                     soc_mem_t ucmem_data,
                     soc_reg_t ucmem_ctrl)
{
    int entry_bytes, entry_num, ucmem_size, endian;
    const uint8 *array_2nd;
    uint32 datalen_2nd = 0;
    int reg_access_idx = (blk_id | SOC_REG_ADDR_BLOCK_ID_MASK);
    uint64 reg_val;
    SOC_INIT_FUNC_DEFS;

    entry_bytes = soc_mem_entry_bytes(unit, ucmem_data);
    entry_num = soc_mem_index_count(unit, ucmem_data);
    ucmem_size = entry_bytes * entry_num;

    if (datalen > ucmem_size) {
        if (buf_2nd==NULL || datalen>2*ucmem_size) {
            _SOC_EXIT_WITH_ERR(SOC_E_RESOURCE, ("Can't fit all of the firmware into the device load table."));
        }

        array_2nd = array + ucmem_size;
        datalen_2nd = datalen - ucmem_size;
        datalen = ucmem_size;
    } else {
        array_2nd = NULL;
    }

    portmod_sys_get_endian(unit, &endian);

    if (buf->ucode_dma_buf == NULL) {
        _SOC_IF_ERR_EXIT(_portmod_dma_buf_alloc(unit, array, datalen, buf, data_swap, endian, entry_bytes));
    }

    if (datalen_2nd>0 && buf_2nd!=NULL && buf_2nd->ucode_dma_buf==NULL) {
        _SOC_IF_ERR_EXIT(_portmod_dma_buf_alloc(unit, array_2nd, datalen_2nd, buf_2nd, data_swap, endian, entry_bytes));
    }

    /* enable parallel bus access */
    _SOC_IF_ERR_EXIT(soc_reg_get(unit,ucmem_ctrl, reg_access_idx, 0, &reg_val));
    soc_reg64_field32_set(unit, ucmem_ctrl, &reg_val, ACCESS_MODEf, 1);
    _SOC_IF_ERR_EXIT(soc_reg_set(unit,ucmem_ctrl, reg_access_idx, 0, reg_val));

    /* coverity[stack_use_overflow] */
    _SOC_IF_ERR_EXIT
        (soc_mem_write_range(unit, ucmem_data, blk_id, 0,
                             buf->ucode_alloc_size / entry_bytes - 1,
                             buf->ucode_dma_buf));

    /* if ucode size exceeds UCMEM limitation, the second buffer is used to download the extra data */
    if (datalen_2nd>0 && buf_2nd!=NULL) {
        _SOC_IF_ERR_EXIT
            (soc_mem_write_range(unit, ucmem_data, blk_id, 0,
                                 buf_2nd->ucode_alloc_size / entry_bytes - 1,
                                 buf_2nd->ucode_dma_buf));
    }

    /* disable parallel bus access, and enable MDIO access */
    _SOC_IF_ERR_EXIT(soc_reg_get(unit,ucmem_ctrl, reg_access_idx, 0, &reg_val));
    soc_reg64_field32_set(unit, ucmem_ctrl, &reg_val, ACCESS_MODEf, 0);
    _SOC_IF_ERR_EXIT(soc_reg_set(unit,ucmem_ctrl, reg_access_idx, 0, reg_val));

exit:
    SOC_FUNC_RETURN;
}

void portmod_common_phy_to_port_ability (phymod_autoneg_ability_t *anAbility,
                                         portmod_port_ability_t *portAbility)
{
    int                       abil37;
    int                       abil73;
    _shr_port_mode_t          spd_fd = 0;  /* Speed full duplex */

    /* retrieve CL73 abilities */
    abil73 = anAbility->an_cap;
    spd_fd|= PHYMOD_AN_CAP_100G_CR10_GET(abil73) ?SOC_PA_SPEED_100GB:0;
    spd_fd|= PHYMOD_AN_CAP_100G_CR4_GET(abil73) ?SOC_PA_SPEED_100GB:0;
    spd_fd|= PHYMOD_AN_CAP_100G_KR4_GET(abil73) ?SOC_PA_SPEED_100GB:0;
    spd_fd|= PHYMOD_AN_CAP_40G_CR4_GET(abil73) ?SOC_PA_SPEED_40GB:0;
    spd_fd|= PHYMOD_AN_CAP_40G_KR4_GET(abil73) ?SOC_PA_SPEED_40GB:0;
    spd_fd|= PHYMOD_AN_CAP_10G_KR_GET(abil73)  ?SOC_PA_SPEED_10GB:0;
    spd_fd|= PHYMOD_AN_CAP_10G_KX4_GET(abil73) ?SOC_PA_SPEED_10GB:0;
    spd_fd|= PHYMOD_AN_CAP_1G_KX_GET(abil73)   ?SOC_PA_SPEED_1000MB:0;
    spd_fd|= PHYMOD_AN_CAP_100M_GET(abil73)   ?SOC_PA_SPEED_100MB:0;

    /* retrieve CL73bam abilities */
    abil73 = anAbility->cl73bam_cap;
    spd_fd|= PHYMOD_BAM_CL73_CAP_50G_CR4_GET(abil73)? SOC_PA_SPEED_50GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_50G_KR4_GET(abil73)? SOC_PA_SPEED_50GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_50G_CR2_GET(abil73)? SOC_PA_SPEED_50GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_50G_KR2_GET(abil73)? SOC_PA_SPEED_50GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_40G_CR2_GET(abil73)? SOC_PA_SPEED_40GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_40G_KR2_GET(abil73)? SOC_PA_SPEED_40GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_25G_CR1_GET(abil73)? SOC_PA_SPEED_25GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_25G_KR1_GET(abil73)? SOC_PA_SPEED_25GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_20G_CR2_GET(abil73)? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_20G_KR2_GET(abil73)? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_20G_CR1_GET(abil73)? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL73_CAP_20G_KR1_GET(abil73)? SOC_PA_SPEED_20GB:0;
    
    /* retrieve CL37 abilities */
    abil37 = anAbility->cl37bam_cap;
    spd_fd|= PHYMOD_BAM_CL37_CAP_40G_GET(abil37)  ? SOC_PA_SPEED_40GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_31P5G_GET(abil37)? SOC_PA_SPEED_30GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_32P7G_GET(abil37)? SOC_PA_SPEED_32GB:0;

    spd_fd|= PHYMOD_BAM_CL37_CAP_25P455G_GET(abil37)   ? SOC_PA_SPEED_25GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_21G_X4_GET(abil37)    ? SOC_PA_SPEED_21GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_20G_X2_CX4_GET(abil37)? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_20G_X2_GET(abil37)    ? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_20G_X4_GET(abil37)    ? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_20G_X4_CX4_GET(abil37)? SOC_PA_SPEED_20GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_16G_X4_GET(abil37)    ? SOC_PA_SPEED_16GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_15P75G_R2_GET(abil37) ? SOC_PA_SPEED_16GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_15G_X4_GET(abil37)?     SOC_PA_SPEED_15GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_13G_X4_GET(abil37)?     SOC_PA_SPEED_13GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_12P7_DXGXS_GET(abil37)? SOC_PA_SPEED_13GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_12P5_X4_GET(abil37)?    SOC_PA_SPEED_12P5GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_12G_X4_GET(abil37)?     SOC_PA_SPEED_12GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_10P5G_DXGXS_GET(abil37)?SOC_PA_SPEED_11GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_10G_X2_CX4_GET(abil37)? SOC_PA_SPEED_10GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_10G_DXGXS_GET(abil37)?  SOC_PA_SPEED_10GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_10G_CX4_GET(abil37)?    SOC_PA_SPEED_10GB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_10G_HIGIG_GET(abil37)?  SOC_PA_SPEED_10GB:0; /* 4-lane */
    spd_fd|= PHYMOD_BAM_CL37_CAP_6G_X4_GET(abil37)?      SOC_PA_SPEED_6000MB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_5G_X4_GET(abil37)?      SOC_PA_SPEED_5000MB:0;
    spd_fd|= PHYMOD_BAM_CL37_CAP_2P5G_GET(abil37)?       SOC_PA_SPEED_2500MB:0;

    if (PHYMOD_AN_CAP_CL37_GET(anAbility)){
        spd_fd|= SOC_PA_SPEED_1000MB;
    }

    switch (anAbility->sgmii_speed){
        case phymod_CL37_SGMII_10M:
            spd_fd|= SOC_PA_SPEED_10MB;
            break;
        case phymod_CL37_SGMII_100M:
            spd_fd|= SOC_PA_SPEED_100MB;
            break;
        case phymod_CL37_SGMII_1000M:
            spd_fd|= SOC_PA_SPEED_1000MB;
            break;
        default:
            break;
    }

    portAbility->pause = 0;
    if (anAbility->capabilities & PHYMOD_AN_CAP_ASYM_PAUSE) {
        if (anAbility->capabilities & PHYMOD_AN_CAP_SYMM_PAUSE) {
            portAbility->pause = SOC_PA_PAUSE_RX;
        } else {
            portAbility->pause = SOC_PA_PAUSE_TX;
        }
    } else if (anAbility->capabilities & PHYMOD_AN_CAP_SYMM_PAUSE) {
        portAbility->pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
    }

    portAbility->speed_full_duplex = spd_fd;
    portAbility->fec = anAbility->an_fec;
}


/* phy_tsce_ability_advert_set */
#define PHYMOD_INTF_CR4    (1 << SOC_PORT_IF_CR4)
void portmod_common_port_to_phy_ability (portmod_port_ability_t *portAbility, 
                                         phymod_autoneg_ability_t *anAbility,
                                         int port_num_lanes, 
                                         int line_interface, 
                                         int cx4_10g,
                                         int an_cl72,
                                         int an_fec,
                                         int hg_mode)
{
    uint32_t                  an_tech_ability   = 0;
    uint32_t                  an_bam37_ability  = 0;
    uint32_t                  an_bam73_ability  = 0;
    _shr_port_mode_t          speed_full_duplex = portAbility->speed_full_duplex;
    phymod_autoneg_ability_t_init(anAbility);

    /* an_tech_ability */
    if (port_num_lanes == 4  ||
        port_num_lanes == 10 ||
        port_num_lanes == 12 ) {
        if ((speed_full_duplex & SOC_PA_SPEED_100GB) ||
            (speed_full_duplex & SOC_PA_SPEED_106GB) ||
            (speed_full_duplex & SOC_PA_SPEED_120GB) ||
            (speed_full_duplex & SOC_PA_SPEED_127GB)) {
            if (SOC_PORT_IF_CR4 == line_interface) {
                PHYMOD_AN_CAP_100G_CR4_SET(an_tech_ability);
            } else if ((SOC_PORT_IF_KR4 == line_interface)
                       ||(SOC_PORT_IF_CAUI4 == line_interface)) {
                PHYMOD_AN_CAP_100G_KR4_SET(an_tech_ability);
            } else {
                if(port_num_lanes == 4){
                    PHYMOD_AN_CAP_100G_KR4_SET(an_tech_ability);
                } else if (port_num_lanes == 10){
                    PHYMOD_AN_CAP_100G_CR10_SET(an_tech_ability);
                } else {
                    PHYMOD_AN_CAP_100G_KR4_SET(an_tech_ability);
                } 
            }
        }
        if (speed_full_duplex & SOC_PA_SPEED_50GB) {
            if (SOC_PORT_IF_CR4 == line_interface) {
                PHYMOD_BAM_CL73_CAP_50G_CR4_SET(an_bam73_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_50G_KR4_SET(an_bam73_ability);
            }
        }
        if (speed_full_duplex & SOC_PA_SPEED_40GB) {
            if (SOC_PORT_IF_CR4 == line_interface) {
                PHYMOD_AN_CAP_40G_CR4_SET(an_tech_ability);
            } else {
                PHYMOD_AN_CAP_40G_KR4_SET(an_tech_ability);
            }
        }
        if(speed_full_duplex & SOC_PA_SPEED_20GB) {
            if (SOC_PORT_IF_CR4 == line_interface) {
                PHYMOD_BAM_CL73_CAP_20G_CR2_SET(an_bam73_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_20G_KR2_SET(an_bam73_ability);
            }
        }
        if(speed_full_duplex & SOC_PA_SPEED_10GB) {
            PHYMOD_AN_CAP_10G_KX4_SET(an_tech_ability);
        }
        if (speed_full_duplex & SOC_PA_SPEED_1000MB)
            PHYMOD_AN_CAP_1G_KX_SET(an_tech_ability);
    } else if (port_num_lanes == 2) {
        if(speed_full_duplex & SOC_PA_SPEED_50GB) {
            if (SOC_PORT_IF_CR2 == line_interface) {
                PHYMOD_BAM_CL73_CAP_50G_CR2_SET(an_bam73_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_50G_KR2_SET(an_bam73_ability);
            }
        }
        if(speed_full_duplex & SOC_PA_SPEED_40GB) {
            if (SOC_PORT_IF_CR2 == line_interface) {
                PHYMOD_BAM_CL73_CAP_40G_CR2_SET(an_bam73_ability);
            /*support AN ability set for external PHY with Gearbox mode*/
            } else if (SOC_PORT_IF_CR4 == line_interface) {
                PHYMOD_AN_CAP_40G_CR4_SET(an_tech_ability);
            } else if (SOC_PORT_IF_KR4 == line_interface) {
                PHYMOD_AN_CAP_40G_KR4_SET(an_tech_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_40G_KR2_SET(an_bam73_ability);
            }
        }
        if(speed_full_duplex & SOC_PA_SPEED_20GB) {
            if (SOC_PORT_IF_CR2 == line_interface) {
                PHYMOD_BAM_CL73_CAP_20G_CR2_SET(an_bam73_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_20G_KR2_SET(an_bam73_ability);
            }
        }
        if (speed_full_duplex & SOC_PA_SPEED_10GB)
            PHYMOD_AN_CAP_10G_KR_SET(an_tech_ability);
        if (speed_full_duplex & SOC_PA_SPEED_1000MB)
            PHYMOD_AN_CAP_1G_KX_SET(an_tech_ability);
    } else {
        if(speed_full_duplex & SOC_PA_SPEED_25GB) {
            if (SOC_PORT_IF_CR == line_interface) {
                PHYMOD_BAM_CL73_CAP_25G_CR1_SET(an_bam73_ability);
                PHYMOD_AN_CAP_25G_CR1_SET(an_tech_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_25G_KR1_SET(an_bam73_ability);
                PHYMOD_AN_CAP_25G_KR1_SET(an_tech_ability);
            }
        }
        if(speed_full_duplex & SOC_PA_SPEED_20GB) {
            if (SOC_PORT_IF_CR == line_interface) {
                PHYMOD_BAM_CL73_CAP_20G_CR1_SET(an_bam73_ability);
            } else {
                PHYMOD_BAM_CL73_CAP_20G_KR1_SET(an_bam73_ability);
            }
        }
        if (speed_full_duplex & SOC_PA_SPEED_10GB)
            PHYMOD_AN_CAP_10G_KR_SET(an_tech_ability);
        if (speed_full_duplex & SOC_PA_SPEED_1000MB)
            PHYMOD_AN_CAP_1G_KX_SET(an_tech_ability);        
    }
    
    /* an_bam37_ability */
    if (port_num_lanes == 4 ||
        port_num_lanes == 10 ||
        port_num_lanes == 12) {
        if(speed_full_duplex & SOC_PA_SPEED_40GB)
            PHYMOD_BAM_CL37_CAP_40G_SET(an_bam37_ability);
        /* if(speed_full_duplex & SOC_PA_SPEED_33GB)
            an_bam37_ability |= (1<<PHYMOD_BAM37ABL_32P7G); */
         if(speed_full_duplex & SOC_PA_SPEED_30GB)
            PHYMOD_BAM_CL37_CAP_31P5G_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_25GB)
            PHYMOD_BAM_CL37_CAP_25P455G_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_21GB)
            PHYMOD_BAM_CL37_CAP_21G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_20GB){
            PHYMOD_BAM_CL37_CAP_20G_X4_SET(an_bam37_ability);
            PHYMOD_BAM_CL37_CAP_20G_X4_CX4_SET(an_bam37_ability);
        }
        if(speed_full_duplex & SOC_PA_SPEED_16GB)
            PHYMOD_BAM_CL37_CAP_16G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_15GB)
            PHYMOD_BAM_CL37_CAP_15G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_13GB)
            PHYMOD_BAM_CL37_CAP_13G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_12P5GB)
            PHYMOD_BAM_CL37_CAP_12P5_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_12GB)
            PHYMOD_BAM_CL37_CAP_12G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_10GB) {
            if (cx4_10g) {
                PHYMOD_BAM_CL37_CAP_10G_CX4_SET(an_bam37_ability);
            } else {
                PHYMOD_BAM_CL37_CAP_10G_HIGIG_SET(an_bam37_ability);
            }
        }     
        if(speed_full_duplex & SOC_PA_SPEED_6000MB)
            PHYMOD_BAM_CL37_CAP_6G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_5000MB)
            PHYMOD_BAM_CL37_CAP_5G_X4_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_2500MB)
            PHYMOD_BAM_CL37_CAP_2P5G_SET(an_bam37_ability);
        if(speed_full_duplex & SOC_PA_SPEED_1000MB)
            PHYMOD_AN_CAP_1G_KX_SET(an_tech_ability);

    } else if (port_num_lanes == 2) {     /* 2 lanes */

        if(speed_full_duplex & SOC_PA_SPEED_20GB){
            PHYMOD_BAM_CL37_CAP_20G_X2_SET(an_bam37_ability);
            PHYMOD_BAM_CL37_CAP_20G_X2_CX4_SET(an_bam37_ability);
        }
        if (speed_full_duplex & SOC_PA_SPEED_16GB)
            PHYMOD_BAM_CL37_CAP_15P75G_R2_SET(an_bam37_ability);
        if (speed_full_duplex & SOC_PA_SPEED_13GB)
            PHYMOD_BAM_CL37_CAP_12P7_DXGXS_SET(an_bam37_ability);
        if (speed_full_duplex & SOC_PA_SPEED_11GB)
            PHYMOD_BAM_CL37_CAP_10P5G_DXGXS_SET(an_bam37_ability);
        if (speed_full_duplex & SOC_PA_SPEED_10GB){
            PHYMOD_BAM_CL37_CAP_10G_X2_CX4_SET(an_bam37_ability);
            PHYMOD_BAM_CL37_CAP_10G_DXGXS_SET(an_bam37_ability);
        }
        if (speed_full_duplex & SOC_PA_SPEED_2500MB)
            PHYMOD_BAM_CL37_CAP_2P5G_SET(an_bam37_ability);
        if (speed_full_duplex & SOC_PA_SPEED_1000MB)
            PHYMOD_AN_CAP_1G_KX_SET(an_tech_ability);

    } else {                              /* 1 lane */
        if (speed_full_duplex & SOC_PA_SPEED_2500MB)
            PHYMOD_BAM_CL37_CAP_2P5G_SET(an_bam37_ability);
    }
    anAbility->an_cap = an_tech_ability;
    anAbility->cl73bam_cap = an_bam73_ability; 
    anAbility->cl37bam_cap = an_bam37_ability; 

    

    switch (portAbility->pause & (SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX)) {
    case SOC_PA_PAUSE_TX:
        PHYMOD_AN_CAP_ASYM_PAUSE_SET(anAbility);
        break;
    case SOC_PA_PAUSE_RX:
        /* an_adv |= MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE; */
        PHYMOD_AN_CAP_ASYM_PAUSE_SET(anAbility);
        PHYMOD_AN_CAP_SYMM_PAUSE_SET(anAbility);
        break;
    case SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX:
        PHYMOD_AN_CAP_SYMM_PAUSE_SET(anAbility);
        break;
    }

    if ((speed_full_duplex & SOC_PA_SPEED_10MB) ||
        (speed_full_duplex & SOC_PA_SPEED_100MB) ||
        (speed_full_duplex & SOC_PA_SPEED_1000MB)) {
        if (SOC_PORT_IF_SGMII == line_interface) {
            /* also set the sgmii speed */
            if(portAbility->speed_full_duplex & SOC_PA_SPEED_1000MB) {
                PHYMOD_AN_CAP_SGMII_SET(anAbility);
                anAbility->sgmii_speed = phymod_CL37_SGMII_1000M;
            } else if(portAbility->speed_full_duplex & SOC_PA_SPEED_100MB) {
                PHYMOD_AN_CAP_SGMII_SET(anAbility);
                anAbility->sgmii_speed = phymod_CL37_SGMII_100M;
            } else if(portAbility->speed_full_duplex & SOC_PA_SPEED_10MB) {
                PHYMOD_AN_CAP_SGMII_SET(anAbility);
                anAbility->sgmii_speed = phymod_CL37_SGMII_10M;
            } else {
                PHYMOD_AN_CAP_SGMII_SET(anAbility);
                anAbility->sgmii_speed = phymod_CL37_SGMII_1000M;
            }
        } else {
            if(portAbility->speed_full_duplex & SOC_PA_SPEED_100MB) {
                PHYMOD_AN_CAP_100M_SET(anAbility->an_cap);
            }
        }
    }

    /* next check if we need to set cl37 attribute */
    if (an_cl72) {
        anAbility->an_cl72 = 1;
    }
    if (hg_mode) {
        anAbility->an_hg2 = 1;
    }

    /* check if Cl74/CL91 fec is to be set */
    if (portAbility->fec == SOC_PA_FEC_NONE) {
        PHYMOD_AN_FEC_OFF_SET(anAbility->an_fec);
    } else {
        if(portAbility->fec & SOC_PA_FEC_CL74) {
            PHYMOD_AN_FEC_CL74_SET(anAbility->an_fec);
        }
        if(portAbility->fec & SOC_PA_FEC_CL91) {
           PHYMOD_AN_FEC_CL91_SET(anAbility->an_fec);
        }
    }

    if (an_fec) {
        PHYMOD_AN_FEC_CL74_SET(anAbility->an_fec);
    }

}

/*!
 * portmod_port_main_core_access_get
 *
 * @brief get port main cores' phymod access
 *
 * @param [in]  unit               - unit id
 * @param [in]  port               - logical port
 * @param [in]  phyn               - the number of hops from the internal phy. 0 - internal, 1- first external PHY, etc.
 * @param [out]  core_access_arr   - port phymod cores array
 * @param [out]  nof_cores         - number of core access structutres filled by the function
 */
int portmod_port_main_core_access_get(int unit, int port, int phyn,
                                      phymod_core_access_t *core_access,
                                      int *nof_cores)
{
    int ncores = 0;
    if (IS_C_PORT(unit, port) || IS_CXX_PORT(unit, port)) {
        phymod_core_access_t core_acc_100g[3];
        portmod_port_core_access_get(unit, port, phyn, 3, core_acc_100g, &ncores, NULL);
        sal_memcpy(core_access, &core_acc_100g[0], sizeof(phymod_core_access_t));
    } else {
        phymod_core_access_t core_acc;
        portmod_port_core_access_get(unit, port, phyn, 1, &core_acc, &ncores, NULL);
        sal_memcpy(core_access, &core_acc, sizeof(phymod_core_access_t));
    }
    *nof_cores = ncores;
    return SOC_E_NONE;
}

/*!
 * portmod_port_to_phyaddr
 *
 * @brief Get Phy addr for a given port.
 *
 * @param [in]  unit            - unit id
 * @param [in]  port            - logical port
 */
int portmod_port_to_phyaddr(int unit, int port)
{
    int nof_cores = 0, phyaddr;

    phymod_core_access_t core_acc;
    portmod_port_main_core_access_get(unit, port, -1, &core_acc, &nof_cores);
    phyaddr = (nof_cores == 0)? -1 : core_acc.access.addr;

    return (phyaddr);
}

/*!
 * portmod_port_to_phyaddr_int
 *
 * @brief Get internal Phy addr for a given port.
 *
 * @param [in]  unit            - unit id
 * @param [in]  port            - logical port
 */
int portmod_port_to_phyaddr_int(int unit, int port)
{
    phymod_core_access_t internal_core;
    int nof_cores = 0;

    portmod_port_core_access_get(unit, port, 0, 1, &internal_core, &nof_cores, NULL);

    return ((nof_cores == 0)? -1 : internal_core.access.addr);
}

int portmod_intf_to_phymod_intf(int unit, int speed, soc_port_if_t interface,
                                       phymod_interface_t *phymod_interface)
{
    return soc_phymod_phy_intf_from_port_intf(unit, speed, interface, phymod_interface);
}

int portmod_intf_from_phymod_intf (int unit, 
                                   phymod_interface_t phymod_interface,
                                   soc_port_if_t *interface)
{
    return soc_phymod_phy_intf_to_port_intf(unit, phymod_interface, interface);
}

int portmod_line_intf_from_config_get(const portmod_port_interface_config_t *config, soc_port_if_t *interface)
{
    *interface = (config->line_interface != SOC_PORT_IF_NULL ? config->line_interface : config->interface);
    return SOC_E_NONE;
}

int
portmod_port_redirect_loopback_set(int unit, soc_port_t port, 
                                   int phyn, int phy_lane,
                                   int sys_side, uint32 enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int nof_phys = 0;
    portmod_access_get_params_t params;
    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE);

    _SOC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, 
                                                      PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                      phy_access, &nof_phys, NULL));

    _SOC_IF_ERR_EXIT(portmod_port_phychain_loopback_set (unit, port, phy_access, nof_phys, portmodLoopbackPhyGloopPCS, enable));

exit:
    SOC_FUNC_RETURN;

#if 0
    /* leave this code to look  sys_side need to be taken care of. The comment need to delete before commit. tlwin*/
    phymod_phy_access_t phy_access;
    int nof_phys;
    portmod_access_get_params_t params;

    portmod_access_get_params_t_init(unit, &params);
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side == 1) ? PORTMOD_SIDE_SYSTEM: PORTMOD_SIDE_LINE;

    SOC_IF_ERROR_RETURN(
        portmod_port_phy_lane_access_get(unit,
                                         port,
                                         &params,
                                         1, 
                                         &phy_access,
                                         &nof_phys));

    SOC_IF_ERROR_RETURN(phymod_phy_loopback_set(&phy_access, 
                                  portmodLoopbackPhyGloopPCS, enable));

    return SOC_E_NONE;
#endif
}

int
portmod_port_redirect_loopback_get(int unit, soc_port_t port, 
                                   int phyn, int phy_lane,
                                   int sys_side, uint32 *enable)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int nof_phys = 0;
    portmod_access_get_params_t params;
    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE);

    _SOC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, 
                                                      PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                      phy_access, &nof_phys, NULL));

    _SOC_IF_ERR_EXIT(portmod_port_phychain_loopback_get (unit, port, phy_access, nof_phys, portmodLoopbackPhyGloopPCS, enable));

exit:
    SOC_FUNC_RETURN;

#if 0
    /* need to review before deleting this */
    phymod_phy_access_t phy_access;
    int nof_phys;
    portmod_access_get_params_t params;

    portmod_access_get_params_t_init(unit, &params);
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side == 1) ? PORTMOD_SIDE_SYSTEM: PORTMOD_SIDE_LINE;

    SOC_IF_ERROR_RETURN(
        portmod_port_phy_lane_access_get(unit,
                                         port,
                                         &params,
                                         1, 
                                         &phy_access,
                                         &nof_phys));

    SOC_IF_ERROR_RETURN(phymod_phy_loopback_get(&phy_access, 
                                  portmodLoopbackPhyGloopPCS, enable));

    return SOC_E_NONE;
#endif
}

int
portmod_port_redirect_autoneg_set (int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, phymod_autoneg_control_t* an)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int nof_phys = 0;
    portmod_access_get_params_t params;
    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE);

    _SOC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, 
                                                      PORT_MAX_PHY_ACCESS_STRUCTURES,
                                                      phy_access, &nof_phys, NULL));

    _SOC_IF_ERR_EXIT(portmod_port_phychain_autoneg_set(unit, port, phy_access, nof_phys, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, an));

exit:
    SOC_FUNC_RETURN;
}

int
portmod_port_redirect_autoneg_get (int unit, soc_port_t port,
                                   int phyn, int phy_lane,
                                   int sys_side, phymod_autoneg_control_t* an)
{
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int nof_phys = 0;
    uint32_t an_done =0;
    portmod_access_get_params_t params;
    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));
    params.lane = phy_lane;
    params.phyn = phyn;
    params.sys_side = (sys_side ? PORTMOD_SIDE_SYSTEM : PORTMOD_SIDE_LINE);

    _SOC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, 
                                                      PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                      phy_access, &nof_phys, NULL));

    _SOC_IF_ERR_EXIT(portmod_port_phychain_autoneg_get(unit, port, phy_access, nof_phys, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, an, &an_done));

exit:
    SOC_FUNC_RETURN;
}


static int _portmod_port_medium_flags_test (int unit, soc_port_t port)
{
    static int prev_medium[128], flag = 0;
    portmod_port_diag_info_t diag_info;

    if (!flag) {
        sal_memset(prev_medium, -1, sizeof(prev_medium));
        flag = 1;
    }

    SOC_IF_ERROR_RETURN(portmod_port_diag_info_get(unit, port, &diag_info));

    if (diag_info.medium != prev_medium[port]) {
        prev_medium[port] = diag_info.medium;
        return (1);
    }
    return (0);
}

/*
 * This is a temporary  fix for the PHY_FLAGS_TST code which
 * gets updated in linkscan. Proper code will be added
 */
int portmod_port_flags_test(int unit, soc_port_t port, int flag)
{
    phymod_core_access_t core_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    phymod_autoneg_control_t an;
    portmod_port_interface_config_t cfg;
    phymod_operation_mode_t  op_mode = 0;
    int is_legacy_present;

    int chain_length = 0, is_most_ext;

    SOC_IF_ERROR_RETURN(portmod_port_core_access_get(unit, port, 0, PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                     core_access, &chain_length, &is_most_ext));
    SOC_IF_ERROR_RETURN(
        portmod_port_is_legacy_ext_phy_present(unit, port, &is_legacy_present));

    if (is_legacy_present) {
        /* call legacy driver functions */
        return portmod_port_legacy_phy_flags_test(unit, port, flag);
    }

    if (chain_length < 1) return (SOC_E_PARAM);

    switch (flag) {
        case PHY_FLAGS_MEDIUM_CHANGE:

             return (_portmod_port_medium_flags_test(unit, port));
             break;

        case PHY_FLAGS_C45:
             {
             return (PHYMOD_ACC_F_CLAUSE45_GET(&core_access[0].access));
             }
             break;

        case PHY_FLAGS_EXTERNAL_PHY:
             {
             return (is_most_ext ? 0 : 1);
             }
             break;

        case PHY_FLAGS_FORCED_SGMII:
             SOC_IF_ERROR_RETURN(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));
             if (an.enable) return (0);

             SOC_IF_ERROR_RETURN(portmod_port_interface_config_get(unit, port, &cfg, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
             return ((cfg.interface == SOC_PORT_IF_SGMII)? 1 : 0);
             break;

        case PHY_FLAGS_REPEATER:
             SOC_IF_ERROR_RETURN(portmod_port_phy_op_mode_get(unit, port, &op_mode));
             return (op_mode);
             break;

        default:
             break;
    }

    return (SOC_E_UNAVAIL);
}

int portmod_port_chain_core_access_get(int unit, int port, pm_info_t pm_info, phymod_core_access_t* core_access_arr, int max_buf, int* nof_cores)
{
    int phyn = 0, is_most_ext = 0, core_count;
    int arr_idx = 0, rv;
    SOC_INIT_FUNC_DEFS;
    
    while (!is_most_ext) {

        if (max_buf <= 0) { /* buffer protection */
            _SOC_EXIT_WITH_ERR(SOC_E_FULL, (_SOC_MSG("max buffer size exceeded.")));
        }

        rv = __portmod__dispatch__[pm_info->type]->f_portmod_port_core_access_get(unit, port, pm_info, phyn, max_buf, &(core_access_arr[arr_idx]), &core_count, &is_most_ext);
        _SOC_IF_ERR_EXIT(rv);
        arr_idx += core_count;
        phyn++;
        max_buf -= core_count;
    }

    *nof_cores = arr_idx;

exit:
    SOC_FUNC_RETURN; 
    
}

int portmod_port_chain_phy_access_get(int unit, int port, pm_info_t pm_info, phymod_phy_access_t* core_access_arr, int max_buf, int* nof_cores)
{
    int phyn = 0, is_most_ext = 0, core_count, arr_idx = 0, rv;
    portmod_access_get_params_t params;
    SOC_INIT_FUNC_DEFS;

    while (!is_most_ext) {

        if (max_buf <= 0) { /* buffer protection */
            _SOC_EXIT_WITH_ERR(SOC_E_FULL, (_SOC_MSG("max buffer size exceeded.")));
        }

        _SOC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));
        params.phyn = phyn;
        rv = __portmod__dispatch__[pm_info->type]->f_portmod_port_phy_lane_access_get(unit, port, pm_info, &params, max_buf, &(core_access_arr[arr_idx]), &core_count, &is_most_ext);
        _SOC_IF_ERR_EXIT(rv);

        arr_idx += core_count;
        phyn++;
        max_buf -= core_count;
    }

    *nof_cores = arr_idx;

exit:
    SOC_FUNC_RETURN; 
    
}
int portmod_ext_to_int_cmd_set(int unit, int port, portmod_ext_to_int_phy_ctrlcode_t cmd, void *data) {
    pm_info_t pm_info;
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0, flags = 0;
    phymod_autoneg_control_t *an;
    portmod_port_ability_t *port_ability;

    portmod_port_interface_config_t interface_config;

    int *cmd_data = (int *)data;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_pm_info_get(unit, port, &pm_info));

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                       PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                       &chain_length));
    sal_memset(&interface_config, 0, sizeof(interface_config));

   /* call portmod_port_chain function */
    switch(cmd) {
        case portmodExtToInt_CtrlCode_Link:
            break;
        case portmodExtToInt_CtrlCode_Enable:
            PORTMOD_PORT_ENABLE_PHY_SET(flags);
            PORTMOD_PORT_ENABLE_INTERNAL_PHY_ONLY_SET(flags);
            _SOC_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, *cmd_data));
            break;
        case portmodExtToInt_CtrlCode_Speed:
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &interface_config, 
                                              PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
            interface_config.speed = *cmd_data;    
            portmod_port_interface_config_set(unit, port, &interface_config, 
                                              PORTMOD_INIT_F_INTERNAL_SERDES_ONLY);
            break;
        case portmodExtToInt_CtrlCode_Interface:
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &interface_config,    
                                              PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));    
            interface_config.interface = *cmd_data;
            portmod_port_interface_config_set(unit, port, &interface_config, 
                                              PORTMOD_INIT_F_INTERNAL_SERDES_ONLY);
            break;
        case portmodExtToInt_CtrlCode_AN:
            an = (phymod_autoneg_control_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, an));
            break;  
        case portmodExtToInt_CtrlCode_AbilityAdvert:
            port_ability = (portmod_port_ability_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_ability_advert_set(unit, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, port, port_ability));
            break;      
        default:
            _SOC_EXIT_WITH_ERR(SOC_E_PARAM, ("Invalid command input %d\n", cmd));            
    }

   /* call portmod_port_chain function */
exit:
    SOC_FUNC_RETURN; 
   

}


int portmod_ext_to_int_cmd_get(int unit, int port, portmod_ext_to_int_phy_ctrlcode_t cmd, void *data) {
    pm_info_t pm_info;
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    uint32 flags = 0;
    portmod_port_interface_config_t interface_config;
    phymod_autoneg_control_t *an;
    portmod_port_ability_t *port_ability;

    int value;
    int *cmd_data = (int *)data;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_pm_info_get(unit, port, &pm_info));

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                       PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                       &chain_length));
    sal_memset(&interface_config, 0, sizeof(interface_config));

    /* call portmod_port_chain function */
    switch(cmd) {
        case portmodExtToInt_CtrlCode_Link:
            portmod_port_link_get(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, &value);
            *cmd_data = value;
            break;
    case portmodExtToInt_CtrlCode_Enable:
            PORTMOD_PORT_ENABLE_INTERNAL_PHY_ONLY_SET(flags);
            _SOC_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, &value));
            *cmd_data = value;
            break;
        case portmodExtToInt_CtrlCode_Speed:
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &interface_config, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));
            *cmd_data = interface_config.speed;
            break;
        case portmodExtToInt_CtrlCode_Interface:
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &interface_config, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));
            *cmd_data = interface_config.interface;
            break;
        case portmodExtToInt_CtrlCode_AN:
            an = (phymod_autoneg_control_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, an));
            break;
        case portmodExtToInt_CtrlCode_AbilityAdvert:
            port_ability = (portmod_port_ability_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_ability_advert_get(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, port_ability));
            break;
        case portmodExtToInt_CtrlCode_AbilityLocal:
            port_ability = (portmod_port_ability_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_ability_local_get(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, port_ability));
            break;                   
        case portmodExtToInt_CtrlCode_AbilityRemote:
            port_ability = (portmod_port_ability_t *)data;
            _SOC_IF_ERR_EXIT(portmod_port_ability_remote_get(unit, port, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY, port_ability));
            break;
        default:
            _SOC_EXIT_WITH_ERR(SOC_E_PARAM, ("Invalid command input %d\n", cmd));
    }

exit:
    SOC_FUNC_RETURN; 

}

int portmod_port_is_legacy_ext_phy_present(int unit, int port, int *is_legacy_present) {
    pm_info_t pm_info;
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    portmod_dispatch_type_t __type__;
    int ext_phy_threshold = 1;

    SOC_INIT_FUNC_DEFS;

    *is_legacy_present = 0;

    if (IS_IL_PORT(unit, port)) {
        /* ILKN does not support ext phy */
        SOC_EXIT;
    }
    _SOC_IF_ERR_EXIT(portmod_port_pm_type_get(unit, port, &port, &__type__));

    _SOC_IF_ERR_EXIT(portmod_pm_info_get(unit, port, &pm_info));

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));

#ifdef PORTMOD_PM12X10_SUPPORT
    if (__type__ == portmodDispatchTypePm12x10) {
        ext_phy_threshold = 3;
    }
#endif /*PORTMOD_PM12X10_SUPPORT  */

#ifdef PORTMOD_PM12X10_XGS_SUPPORT
     if (__type__ == portmodDispatchTypePm12x10_xgs) {
        ext_phy_threshold = 3;
    }
#endif /*PORTMOD_PM12X10_XGS_SUPPORT  */
    
    if (chain_length > ext_phy_threshold) {
        /* external phy is present in this system */
        _SOC_IF_ERR_EXIT(portmod_xphy_is_legacy_phy_get(unit, phy_access[chain_length - 1].access.addr, is_legacy_present));
    }   
exit:
    SOC_FUNC_RETURN; 

}

int portmod_port_ext_phy_control_set(int unit, int port, int phyn, int phy_lane, int sys_side,
                                     soc_phy_control_t control, uint32 value) {

    portmod_dispatch_type_t __type__;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_pm_type_get(unit, port, &port, &__type__));

    _SOC_IF_ERR_EXIT(portmod_port_legacy_phy_control_set(unit, port, phyn, phy_lane, sys_side,
                                               control, value));

    exit:
        SOC_FUNC_RETURN;
}

int portmod_port_ext_phy_control_get(int unit, int port, int phyn, int phy_lane, int sys_side, 
                                     soc_phy_control_t control, uint32* value) {

    portmod_dispatch_type_t __type__;

    SOC_INIT_FUNC_DEFS;

    _SOC_IF_ERR_EXIT(portmod_port_pm_type_get(unit, port, &port, &__type__));

    _SOC_IF_ERR_EXIT(portmod_port_legacy_phy_control_get(unit, port, phyn, phy_lane, sys_side,
                                               control, value));

    exit:
        SOC_FUNC_RETURN;
}

int portmod_port_ext_phy_mdix_set(int unit, int port, soc_port_mdix_t mode) {
    return portmod_port_legacy_phy_mdix_set(unit, port, mode);
}

int portmod_port_ext_phy_mdix_get(int unit, int port, soc_port_mdix_t *mode) {
    return portmod_port_legacy_phy_mdix_get(unit, port, mode);
}

int portmod_port_ext_phy_mdix_status_get(int unit, soc_port_t port,
                                         soc_port_mdix_status_t *status) {
    return portmod_port_legacy_phy_mdix_status_get(unit, port, status);
}


int portmod_port_status_notify(int unit, int port, int link)
{
    pm_info_t pm_info;
    phymod_phy_access_t phy_access[PORT_MAX_PHY_ACCESS_STRUCTURES];
    int chain_length = 0;
    portmod_port_interface_config_t interface_config;

    SOC_INIT_FUNC_DEFS;
 
    _SOC_IF_ERR_EXIT(portmod_pm_info_get(unit, port, &pm_info));

    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                        PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                        &chain_length));
    _SOC_IF_ERR_EXIT(portmod_port_chain_phy_access_get(unit, port, pm_info, phy_access, 
                                                       PORT_MAX_PHY_ACCESS_STRUCTURES, 
                                                       &chain_length));
  
    sal_memset(&interface_config, 0 , sizeof(interface_config));

    if (link) {
        if (chain_length > 1) {
            /* get the line side speed and program the internal phy 
            * to the same speed */
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_get(unit,
                                                               port,
                                                               &interface_config,
                                                               PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
            /* set the speed for internal phy */
            _SOC_IF_ERR_EXIT(portmod_port_interface_config_set(unit,
                                                               port,
                                                               &interface_config, 
                                                               PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));
        }
    } else {
        /* TBD Disable the internal PHY */
    }

exit:
    SOC_FUNC_RETURN; 
}
#define PORTMOD_IF_ERROR_RETURN(_op)   _SHR_E_IF_ERROR_RETURN(_op)

int portmod_common_timesync_config_set(const phymod_phy_access_t* phy_access,
                                       const portmod_phy_timesync_config_t* conf)
{
    phymod_timesync_config_t  ts_conf;

    PORTMOD_IF_ERROR_RETURN(
        phymod_timesync_config_get(phy_access, &ts_conf));

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_ENABLE) {
            phymod_timesync_enable_set(phy_access, TRUE);
        } else {
            phymod_timesync_enable_set(phy_access, FALSE);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE) {
            PHYMOD_TS_F_CAPTURE_TS_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_CAPTURE_TS_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE) {
            PHYMOD_TS_F_HEARTBEAT_TS_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_HEARTBEAT_TS_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE) {
            PHYMOD_TS_F_RX_CRC_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_RX_CRC_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE) {
            PHYMOD_TS_F_8021AS_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_8021AS_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_L2_ENABLE) {
            PHYMOD_TS_F_L2_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_L2_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP4_ENABLE) {
            PHYMOD_TS_F_IP4_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_IP4_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP6_ENABLE) {
            PHYMOD_TS_F_IP6_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_IP6_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT) {
            PHYMOD_TS_F_CLOCK_SRC_EXT_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_CLOCK_SRC_EXT_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_64BIT_TIMESTAMP_ENABLE) {
            PHYMOD_TS_F_64BIT_TIMESTAMP_ENABLE_SET(ts_conf.flags);
        } else {
            PHYMOD_TS_F_64BIT_TIMESTAMP_ENABLE_CLR(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_SYNC) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_SYNC_SET(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_DELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_DELAY_REQ_SET(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_PDELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_PDELAY_REQ_SET(ts_conf.flags);
        }

        if (conf->
            flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_PDELAY_RESP) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_PDELAY_RESP_SET(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_SYNC) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_SYNC_SET(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_DELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_DELAY_REQ_SET(ts_conf.flags);
        }

        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_PDELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_PDELAY_REQ_SET(ts_conf.flags);
        }

        if (conf->
            flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_PDELAY_RESP) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_PDELAY_RESP_SET(ts_conf.flags);
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        ts_conf.itpid = conf->itpid;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        ts_conf.otpid = conf->otpid;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        ts_conf.otpid2 = conf->otpid2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        ts_conf.ts_divider= conf->ts_divider;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        ts_conf.rx_link_delay= conf->rx_link_delay;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {
        sal_memcpy(&ts_conf.original_timecode, &conf->original_timecode,
                   sizeof(phymod_timesync_timespec_t));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        /* coverity[mixed_enums] */
        ts_conf.gmode = conf->gmode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_framesync_mode_set(phy_access,
                (portmod_timesync_framesync_t*) &conf->framesync));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        sal_memcpy(&ts_conf.syncout, &conf->syncout,
                   sizeof(phymod_timesync_syncout_t));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_rx_timestamp_offset_set(phy_access,
                                               conf->rx_timestamp_offset));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_tx_timestamp_offset_set(phy_access,
                                               conf->tx_timestamp_offset));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.tx_sync_mode = conf->tx_sync_mode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.tx_delay_req_mode = conf->tx_delay_request_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.tx_pdelay_req_mode = conf->tx_pdelay_request_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.tx_pdelay_resp_mode = conf->tx_pdelay_response_mode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.rx_sync_mode = conf->rx_sync_mode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.rx_delay_req_mode = conf->rx_delay_request_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.rx_pdelay_req_mode = conf->rx_pdelay_request_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        /* coverity[mixed_enums] */
        ts_conf.rx_pdelay_resp_mode = conf->rx_pdelay_response_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {
        ts_conf.phy_1588_dpll_ref_phase = conf->phy_1588_dpll_ref_phase;
    }

    if (conf->
        validity_mask &
        SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        ts_conf.phy_1588_dpll_ref_phase_delta = conf->phy_1588_dpll_ref_phase_delta;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        ts_conf.phy_1588_dpll_k1 = conf->phy_1588_dpll_k1;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        ts_conf.phy_1588_dpll_k2 = conf->phy_1588_dpll_k2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        ts_conf.phy_1588_dpll_k3 = conf->phy_1588_dpll_k3;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL) {
        sal_memcpy(&ts_conf.mpls_ctrl, &conf->mpls_control,
                   sizeof(phymod_timesync_mpls_ctrl_t));
    }

    PORTMOD_IF_ERROR_RETURN(
        phymod_timesync_config_set(phy_access, &ts_conf));

    return SOC_E_NONE;
}


int portmod_common_timesync_config_get(const phymod_phy_access_t* phy_access,
                                       portmod_phy_timesync_config_t* conf)
{
    uint32 enable;
    phymod_timesync_config_t  ts_conf;

    PORTMOD_IF_ERROR_RETURN(
        phymod_timesync_config_get(phy_access, &ts_conf));

    conf->flags = 0;

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_enable_get(phy_access, &enable));

        if(enable) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_ENABLE;
        } else {
            conf->flags &= ~SOC_PORT_PHY_TIMESYNC_ENABLE;
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE) {
            PHYMOD_TS_F_CAPTURE_TS_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_CAPTURE_TS_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE) {
            PHYMOD_TS_F_HEARTBEAT_TS_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_HEARTBEAT_TS_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE) {
            PHYMOD_TS_F_RX_CRC_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_RX_CRC_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE) {
            PHYMOD_TS_F_8021AS_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_8021AS_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_L2_ENABLE) {
            PHYMOD_TS_F_L2_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_L2_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_IP4_ENABLE) {
            PHYMOD_TS_F_IP4_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_IP4_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_IP6_ENABLE) {
            PHYMOD_TS_F_IP6_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_IP6_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT) {
            PHYMOD_TS_F_CLOCK_SRC_EXT_SET(conf->flags);
        } else {
            PHYMOD_TS_F_CLOCK_SRC_EXT_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_64BIT_TIMESTAMP_ENABLE) {
            PHYMOD_TS_F_64BIT_TIMESTAMP_ENABLE_SET(conf->flags);
        } else {
            PHYMOD_TS_F_64BIT_TIMESTAMP_ENABLE_CLR(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_SYNC) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_SYNC_SET(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_DELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_DELAY_REQ_SET(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_PDELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_PDELAY_REQ_SET(conf->flags);
        }

        if (ts_conf.flags
            & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_TX_PDELAY_RESP) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_TX_PDELAY_RESP_SET(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_SYNC) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_SYNC_SET(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_DELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_DELAY_REQ_SET(conf->flags);
        }

        if (ts_conf.flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_PDELAY_REQ) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_PDELAY_REQ_SET(conf->flags);
        }

        if (ts_conf.flags
            & SOC_PORT_PHY_TIMESYNC_CAPTURE_TIMESTAMP_RX_PDELAY_RESP) {
            PHYMOD_TS_F_CAPTURE_TIMESTAMP_RX_PDELAY_RESP_SET(conf->flags);
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        conf->itpid = ts_conf.itpid;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        conf->otpid = ts_conf.otpid;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        conf->otpid2 = ts_conf.otpid2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        conf->ts_divider= ts_conf.ts_divider;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        conf->rx_link_delay= ts_conf.rx_link_delay;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {
        sal_memcpy(&conf->original_timecode, &ts_conf.original_timecode,
                   sizeof(phymod_timesync_timespec_t));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        /* coverity[mixed_enums] */
        conf->gmode = ts_conf.gmode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_framesync_mode_get(phy_access,
                    (portmod_timesync_framesync_t*) &conf->framesync));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        sal_memcpy(&conf->syncout, &ts_conf.syncout,
                   sizeof(phymod_timesync_syncout_t));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_rx_timestamp_offset_get(phy_access,
                                                   &conf->rx_timestamp_offset));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_tx_timestamp_offset_get(phy_access,
                                                   &conf->tx_timestamp_offset));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        /* coverity[mixed_enums] */
        conf->tx_sync_mode = ts_conf.tx_sync_mode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        conf->tx_delay_request_mode = ts_conf.tx_delay_req_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        conf->tx_pdelay_request_mode = ts_conf.tx_pdelay_req_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        /* coverity[mixed_enums] */
        conf->tx_pdelay_response_mode = ts_conf.tx_pdelay_resp_mode;
    }


    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        /* coverity[mixed_enums] */
        conf->rx_sync_mode = ts_conf.rx_sync_mode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        conf->rx_delay_request_mode = ts_conf.rx_delay_req_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        /* coverity[mixed_enums] */
        conf->rx_pdelay_request_mode = ts_conf.rx_pdelay_req_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        /* coverity[mixed_enums] */
        conf->rx_pdelay_response_mode = ts_conf.rx_pdelay_resp_mode;
    }

    if (conf->
        validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {
        conf->phy_1588_dpll_ref_phase = ts_conf.phy_1588_dpll_ref_phase;
    }

    if (conf->
        validity_mask &
        SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        conf->phy_1588_dpll_ref_phase_delta = ts_conf.phy_1588_dpll_ref_phase_delta;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        conf->phy_1588_dpll_k1 = ts_conf.phy_1588_dpll_k1;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        conf->phy_1588_dpll_k2 = ts_conf.phy_1588_dpll_k2;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        conf->phy_1588_dpll_k3 = ts_conf.phy_1588_dpll_k3;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL) {
        sal_memcpy(&conf->mpls_control, &ts_conf.mpls_ctrl,
                   sizeof(phymod_timesync_mpls_ctrl_t));
    }

    return SOC_E_NONE;
}

int portmod_media_type_from_port_intf(int unit, soc_port_if_t interface,
                                       phymod_phy_inf_config_t *phy_interface_config)
{
    return soc_phymod_media_type_from_port_intf(unit, interface, phy_interface_config);
}

int portmod_intf_mode_from_phymod_intf(int unit, phymod_interface_t phymod_interface,
                                       phymod_phy_inf_config_t *phy_interface_config)
{
    return soc_phymod_intf_mode_from_phymod_intf(unit, phymod_interface, phy_interface_config);
}


int portmod_common_control_phy_timesync_set(const phymod_phy_access_t* phy_access, 
                                            const portmod_port_control_phy_timesync_t type, 
                                            const uint64 value)
{
    uint32 val_lo;
    uint32 once = 0, always = 0;
    uint32 value0;

    val_lo = COMPILER_64_LO(value);

    switch (type) {
    case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:
    case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:
        return SOC_E_NONE;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
        PORTMOD_IF_ERROR_RETURN(phymod_timesync_nco_addend_set(phy_access, val_lo));
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_nco_addend_set(phy_access, val_lo));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_FRAMESYNC:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_do_sync(phy_access));
        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOCAL_TIME:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_local_time_set(phy_access, value));
        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:
        if (val_lo & SOC_PORT_PHY_TIMESYNC_TN_LOAD) {
            PHYMOD_TS_LDCTL_TN_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_TN_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD) {
            PHYMOD_TS_LDCTL_TIMECODE_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_TIMECODE_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD) {
            PHYMOD_TS_LDCTL_SYNCOUT_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_SYNCOUT_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD) {
            PHYMOD_TS_LDCTL_NCO_DIVIDER_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_NCO_DIVIDER_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD) {
            PHYMOD_TS_LDCTL_LOCAL_TIME_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_LOCAL_TIME_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD) {
            PHYMOD_TS_LDCTL_NCO_ADDEND_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_NCO_ADDEND_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_LOOP_FILTER_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_LOOP_FILTER_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_REF_PHASE_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_REF_PHASE_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_REF_PHASE_DELTA_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_REF_PHASE_DELTA_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K3_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K3_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K2_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K2_LOAD_SET(always);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K1_LOAD_SET(once);
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD) {
            PHYMOD_TS_LDCTL_DPLL_K1_LOAD_SET(always);
        }

        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_load_ctrl_set(phy_access, once, always));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:

        if (val_lo & SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT) {
            PORTMOD_IF_ERROR_RETURN(
                phymod_phy_intr_status_clear(phy_access,
                                         PHYMOD_INTR_TIMESYNC_TIMESTAMP));
        }
        if (val_lo & SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT) {
            PORTMOD_IF_ERROR_RETURN(
                phymod_phy_intr_status_clear(phy_access,
                                           PHYMOD_INTR_TIMESYNC_FRAMESYNC));
        }
        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_enable_get(phy_access, &value0));

        if (val_lo & SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT){
            value0 |= PHYMOD_INTR_TIMESYNC_TIMESTAMP ;
        } else {
            value0 &= ~PHYMOD_INTR_TIMESYNC_TIMESTAMP ;
        } 

        if (val_lo & SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT){
            value0 |= PHYMOD_INTR_TIMESYNC_FRAMESYNC;
        } else {
            value0 &= ~PHYMOD_INTR_TIMESYNC_FRAMESYNC;
        }

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_enable_set(phy_access, value0));

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
        val_lo = COMPILER_64_LO(value);

        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_tx_timestamp_offset_set(phy_access, val_lo));

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
        val_lo = COMPILER_64_LO(value);
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_rx_timestamp_offset_set(phy_access, val_lo));

        break;
    default:
        return SOC_E_UNAVAIL;
    }
        return SOC_E_UNAVAIL;
}

int portmod_common_control_phy_timesync_get(const phymod_phy_access_t* phy_access, 
                                            const portmod_port_control_phy_timesync_t type, 
                                            uint64* value)
{
    uint32 val_lo;
    uint32 once = 0, always = 0;
    uint32 value0;
    uint32 intr_status;

    switch (type) {
    case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_heartbeat_timestamp_get(phy_access, value));

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_capture_timestamp_get(phy_access, value));

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_nco_addend_get(phy_access, &val_lo));
        COMPILER_64_SET((*value), 0, val_lo);

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOCAL_TIME:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_local_time_get(phy_access, value));
        break;

    case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:

        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_load_ctrl_get(phy_access, &once, &always));

        val_lo = 0;

        if (once & PHYMOD_TS_LDCTL_TN_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_TN_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_TN_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_TIMECODE_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_TIMECODE_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_SYNCOUT_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_SYNCOUT_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_NCO_DIVIDER_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_NCO_DIVIDER_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_LOCAL_TIME_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_LOCAL_TIME_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_NCO_ADDEND_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_NCO_ADDEND_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_LOOP_FILTER_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_LOOP_FILTER_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_REF_PHASE_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_REF_PHASE_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_REF_PHASE_DELTA_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_REF_PHASE_DELTA_LOAD) {
            val_lo |=
                SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_K3_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_K3_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_K2_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_K2_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD;
        }
        if (once & PHYMOD_TS_LDCTL_DPLL_K1_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD;
        }
        if (always & PHYMOD_TS_LDCTL_DPLL_K1_LOAD) {
            val_lo |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD;
        }
        COMPILER_64_SET((*value), 0, val_lo);

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:
        value0 = 0;

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_status_get(phy_access, &intr_status));
        if (intr_status) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT;
        }

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_status_get(phy_access, &intr_status));
        if (intr_status) {
            value0 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT;
        }

        COMPILER_64_SET((*value), 0, (uint32) value0);
        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:
        value0 = 0;

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_enable_get(phy_access, &intr_status));
        if (intr_status) {
            value0 |= SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT_MASK;
        }

        PORTMOD_IF_ERROR_RETURN(
            phymod_phy_intr_enable_get(phy_access, &intr_status));
        if (intr_status) {
            value0 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK;
        }

        COMPILER_64_SET((*value), 0, (uint32) value0);
        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_tx_timestamp_offset_get(phy_access, &val_lo));
        COMPILER_64_SET((*value), 0, val_lo);

        break;
    case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
        PORTMOD_IF_ERROR_RETURN(
            phymod_timesync_rx_timestamp_offset_get(phy_access, &val_lo));
        COMPILER_64_SET((*value), 0, val_lo);

        break;
    default:
       return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

STATIC
int _portmod_common_ext_phy_addr_get(int unit, soc_port_t port, uint32 *xphy_id)
{
    phymod_phy_access_t phy_access[1+MAX_PHYN];
    int nof_phys = 0;
    portmod_access_get_params_t params;
    int is_most_ext;

    *xphy_id = PORTMOD_XPHY_ID_INVALID;

    SOC_IF_ERROR_RETURN
        (portmod_access_get_params_t_init(unit, &params));
    params.phyn = 0;
    SOC_IF_ERROR_RETURN
        (portmod_port_phy_lane_access_get(unit, port,
                                &params, (1+MAX_PHYN), phy_access, &nof_phys, &is_most_ext));
    if(is_most_ext)
    {
        return SOC_E_NONE;
    }
    params.phyn = 1;
    SOC_IF_ERROR_RETURN
        (portmod_port_phy_lane_access_get(unit, port,
                                &params, (1+MAX_PHYN), phy_access, &nof_phys, NULL));

    *xphy_id = phy_access[0].access.addr;

    return SOC_E_NONE;
}

int portmod_common_ext_phy_fw_bcst(int unit, pbmp_t pbmp)
{
    phymod_core_status_t      core_status;
    phymod_firmware_load_method_t fw_load_method;
    phymod_core_access_t core_access;
    int is_initialized;
    int  force_fw_load, is_legacy_present;
    uint32 primary_core_num;

    phymod_core_init_config_t core_config;

    int     rv=0, dev_type, mdio_bus;
    uint32  xphy_id, is_identify = 0;
    uint8   ext_phy_bcst_done[PM_MDIO_BUS_COUNT][phymodDispatchTypeCount];
    soc_port_t port ;
    pbmp_t new_pbmp ;

    sal_memset(ext_phy_bcst_done, 0, sizeof(ext_phy_bcst_done));

    for(dev_type = 0; dev_type < phymodDispatchTypeCount; dev_type++) {
        for(mdio_bus =0; mdio_bus < PM_MDIO_BUS_COUNT; mdio_bus++) {

            /*set up new pbm of the same type in one mdio bus */
            SOC_PBMP_CLEAR(new_pbmp);
            SOC_IF_ERROR_RETURN(phymod_core_init_config_t_init(&core_config));
            PBMP_ITER(pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id);
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                rv = portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                if (fw_load_method != phymodFirmwareLoadMethodInternal)
                    continue;

                /* coverity[returned_value] */
                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                /*skip if it is legacy PHY*/
                rv = portmod_xphy_is_legacy_phy_get(unit, xphy_id, &is_legacy_present);
                if ((rv != SOC_E_NONE) || (is_legacy_present)) continue;

                /* check if this is bcast capable */
                SOC_IF_ERROR_RETURN(phymod_core_identify(&core_access, 0, &is_identify));

                /* if not broadcast capable - continue */
                if (!(is_identify & 0x80000000)) continue;

                if((dev_type == core_access.type) &&
                    (mdio_bus == PHY_ID_BUS_NUM(core_access.access.addr))){
                    SOC_PBMP_PORT_ADD(new_pbmp, port);
                }
            }

            /*step1: reset core for firmware load if the external phy need*/

            PBMP_ITER(new_pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id) ;
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */
                /* skip if it is not sesto. */
                if ( (core_access.type != phymodDispatchTypeSesto) 
                     && (core_access.type != phymodDispatchTypeDino)) {
                    continue;
                }

                SOC_IF_ERROR_RETURN(portmod_xphy_core_config_get(unit, xphy_id, &core_config));
                core_config.flags = 0;
                SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_get(unit, xphy_id, &is_initialized));
                SOC_IF_ERROR_RETURN(portmod_xphy_force_fw_load_get(unit, xphy_id, &force_fw_load));
                SOC_IF_ERROR_RETURN(portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method));

                if(is_initialized & PHYMOD_CORE_INIT_F_RESET_CORE_FOR_FW_LOAD)
                    continue;
                PHYMOD_CORE_INIT_F_RESET_CORE_FOR_FW_LOAD_SET(&core_config);
                if (force_fw_load == phymodFirmwareLoadForce) {
                    PHYMOD_CORE_INIT_F_FW_FORCE_DOWNLOAD_SET(&core_config);
                }
                else if (force_fw_load == phymodFirmwareLoadAuto) {
                    PHYMOD_CORE_INIT_F_FW_AUTO_DOWNLOAD_SET(&core_config);
                }

                core_config.firmware_load_method = fw_load_method;

                core_status.pmd_active = 0;
                SOC_IF_ERROR_RETURN(phymod_core_init(&core_access,
                                             &core_config, &core_status));
                is_initialized |= PHYMOD_CORE_INIT_F_RESET_CORE_FOR_FW_LOAD;

                SOC_IF_ERROR_RETURN(portmod_xphy_core_config_set(unit, xphy_id, &core_config));
                SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
            }

            /*step2:enable broadcast*/
            PBMP_ITER(new_pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id) ;
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                SOC_IF_ERROR_RETURN(portmod_xphy_primary_core_num_get(unit, xphy_id, &primary_core_num));
                if (primary_core_num == core_access.access.addr) {
                    SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_get(unit, xphy_id, &is_initialized));
                    if(is_initialized & PHYMOD_CORE_INIT_F_UNTIL_FW_LOAD)
                        continue;
                    SOC_IF_ERROR_RETURN(portmod_xphy_force_fw_load_get(unit, xphy_id, &force_fw_load));
                    SOC_IF_ERROR_RETURN(portmod_xphy_core_config_get(unit, xphy_id, &core_config));
                    SOC_IF_ERROR_RETURN(portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method));

                    core_config.firmware_load_method = fw_load_method;
                    core_config.flags = 0;
                    PHYMOD_CORE_INIT_F_UNTIL_FW_LOAD_SET(&core_config);
                    if (force_fw_load == phymodFirmwareLoadForce) {
                        PHYMOD_CORE_INIT_F_FW_FORCE_DOWNLOAD_SET(&core_config);
                    }
                    else if (force_fw_load == phymodFirmwareLoadAuto) {
                        PHYMOD_CORE_INIT_F_FW_AUTO_DOWNLOAD_SET(&core_config);
                    }

                    core_status.pmd_active = 0;
                    SOC_IF_ERROR_RETURN(phymod_core_init(&core_access,
                        &core_config, &core_status));
                    is_initialized |= PHYMOD_CORE_INIT_F_UNTIL_FW_LOAD;

                    SOC_IF_ERROR_RETURN(portmod_xphy_core_config_set(unit, xphy_id, &core_config));
                    SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
                }

            }

            /*step3:load firmware broadcast*/
            PBMP_ITER(new_pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id) ;
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                SOC_IF_ERROR_RETURN(portmod_xphy_primary_core_num_get(unit, xphy_id, &primary_core_num));
                SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_get(unit, xphy_id, &is_initialized));

                if (primary_core_num == core_access.access.addr) {
                    if(is_initialized & PHYMOD_CORE_INIT_F_EXECUTE_FW_LOAD)
                        continue;

                    if(!ext_phy_bcst_done[mdio_bus][dev_type]){
                        /* Do the broadcast*/
                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_get(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_force_fw_load_get(unit, xphy_id, &force_fw_load));
                        SOC_IF_ERROR_RETURN(portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method));

                        core_config.firmware_load_method = fw_load_method;
                        core_config.flags = 0;
                        PHYMOD_CORE_INIT_F_EXECUTE_FW_LOAD_SET(&core_config);
                        if (force_fw_load == phymodFirmwareLoadForce) {
                            PHYMOD_CORE_INIT_F_FW_FORCE_DOWNLOAD_SET(&core_config);
                        }
                        else if (force_fw_load == phymodFirmwareLoadAuto) {
                            PHYMOD_CORE_INIT_F_FW_AUTO_DOWNLOAD_SET(&core_config);
                        }

                        core_status.pmd_active = 0;
                        SOC_IF_ERROR_RETURN(phymod_core_init(&core_access,
                                                             &core_config, &core_status));
                        ext_phy_bcst_done[mdio_bus][dev_type] = 1;
                        is_initialized |= PHYMOD_CORE_INIT_F_EXECUTE_FW_LOAD;
                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_set(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
                    }else {
                        /* if the download is already done, mark it as fw load executed. */
                        is_initialized |= PHYMOD_CORE_INIT_F_EXECUTE_FW_LOAD;
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
                        break ;
                    }
                }
            }

            /*step4: resume and verify*/
            PBMP_ITER(new_pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id) ;
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                if (ext_phy_bcst_done[mdio_bus][dev_type]) {
                    SOC_IF_ERROR_RETURN(portmod_xphy_primary_core_num_get(unit, xphy_id, &primary_core_num));

                    if (primary_core_num == core_access.access.addr) {
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_get(unit, xphy_id, &is_initialized));
                        if((is_initialized & PHYMOD_CORE_INIT_F_RESUME_AFTER_FW_LOAD)&&
                            (is_initialized & PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY))
                        {
                            continue;
                        }

                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_get(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_force_fw_load_get(unit, xphy_id, &force_fw_load));
                        SOC_IF_ERROR_RETURN(portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method));

                        core_config.firmware_load_method = fw_load_method;
                        core_config.flags = 0;
                        /* verify firmware download and resume init */
                        PHYMOD_CORE_INIT_F_EXECUTE_FW_LOAD_CLR(&core_config);
                        PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY_SET(&core_config);
                        PHYMOD_CORE_INIT_F_RESUME_AFTER_FW_LOAD_SET(&core_config);

                        if (force_fw_load == phymodFirmwareLoadForce) {
                            PHYMOD_CORE_INIT_F_FW_FORCE_DOWNLOAD_SET(&core_config);
                        }
                        else if (force_fw_load == phymodFirmwareLoadAuto) {
                            PHYMOD_CORE_INIT_F_FW_AUTO_DOWNLOAD_SET(&core_config);
                        }

                        SOC_IF_ERROR_RETURN(phymod_core_init(&core_access,
                                                     &core_config, &core_status));
                        is_initialized |= PHYMOD_CORE_INIT_F_RESUME_AFTER_FW_LOAD;
                        is_initialized |= PHYMOD_CORE_INIT_F_FIRMWARE_LOAD_VERIFY;

                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_set(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
                    }
                 }
            }

            /*step5: firmware load end*/
            PBMP_ITER(new_pbmp, port){
                rv = _portmod_common_ext_phy_addr_get(unit, port, &xphy_id) ;
                if (rv != SOC_E_NONE) continue;

                if (xphy_id == PORTMOD_XPHY_ID_INVALID) {
                    continue;
                }

                rv = portmod_xphy_core_access_get(unit, xphy_id, &core_access, &is_legacy_present);
                if (rv == SOC_E_PARAM) continue;  /* no external phy exists */

                if (ext_phy_bcst_done[mdio_bus][dev_type]) {

                    SOC_IF_ERROR_RETURN(portmod_xphy_primary_core_num_get(unit, xphy_id, &primary_core_num));

                    if (primary_core_num == core_access.access.addr) {
                        /* finish up the configuration */
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_get(unit, xphy_id, &is_initialized));
                        if(is_initialized & PHYMOD_CORE_INIT_F_FW_LOAD_END)
                            continue;

                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_get(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_force_fw_load_get(unit, xphy_id, &force_fw_load));
                        SOC_IF_ERROR_RETURN(portmod_xphy_fw_load_method_get(unit, xphy_id, &fw_load_method));

                        core_config.firmware_load_method = fw_load_method;
                        core_config.flags = 0;
                        PHYMOD_CORE_INIT_F_FW_LOAD_END_SET(&core_config);
                        if (force_fw_load == phymodFirmwareLoadForce) {
                            PHYMOD_CORE_INIT_F_FW_FORCE_DOWNLOAD_SET(&core_config);
                        }
                        else if (force_fw_load == phymodFirmwareLoadAuto) {
                            PHYMOD_CORE_INIT_F_FW_AUTO_DOWNLOAD_SET(&core_config);
                        }

                        SOC_IF_ERROR_RETURN(phymod_core_init(&core_access,
                                                     &core_config, &core_status));
                        is_initialized |= PHYMOD_CORE_INIT_F_FW_LOAD_END;
                        SOC_IF_ERROR_RETURN(portmod_xphy_core_config_set(unit, xphy_id, &core_config));
                        SOC_IF_ERROR_RETURN(portmod_xphy_is_initialized_set(unit, xphy_id, is_initialized));
                    }
                 }
            }
        }
    }
    return (SOC_E_NONE);

}

/*!
 * portmod_port_phy_drv_name_get
 *
 * @brief Get the phy driver name attached to the port
 *
 * @param [in]  unit     - unit id
 * @param [in]  port     - port num
 * @param [out] name     - phy driver name.
 * @param [in]  len      - max length of the name buffer.
 */
int portmod_port_phy_drv_name_get(int unit, int port, char *name, int len)
{
    int is_ext_legacy_phy = 0;
    phymod_core_access_t core_acc;
    phymod_core_info_t core_info;
    char *pname = NULL, namelen = 0;
    int nof_cores = 0;

    SOC_IF_ERROR_RETURN(
       portmod_port_is_legacy_ext_phy_present(unit, port, &is_ext_legacy_phy));

    if (is_ext_legacy_phy) {
        SOC_IF_ERROR_RETURN(
            portmod_port_legacy_phy_drv_name_get(unit, port, name, len));
    } else {
        phymod_core_access_t_init(&core_acc);
        phymod_core_info_t_init(&core_info);

        SOC_IF_ERROR_RETURN(
            portmod_port_main_core_access_get(unit, port, -1, &core_acc,
                                              &nof_cores));
        if (nof_cores == 0) {
            /*coverity[buffer_size]*/
            strncpy(name, "<nophy>", strlen("<nophy>"));
            return SOC_E_NONE;
        }
        SOC_IF_ERROR_RETURN(
            phymod_core_info_get(&core_acc, &core_info));
            /*
             * get the phy name string from the mapping. The format is
             * phymodCoreVersion"PhyName", only extract Phy Name from
             * the string. The Phy name is also appended with the version
             * version string as the last 2 characters. Skip it.
             */
        pname =
           phymod_core_version_t_mapping[core_info.core_version].key;
        namelen = (len < (strlen(pname) - 2))? len: strlen(pname) - 2;
        strncpy(name, pname, namelen);
    }
    return (SOC_E_NONE);
}

/*!
 * portmod_common_default_interface_get
 *
 * @brief Get default interface type based on port speed, number of lanes, and medium
 *
 * @param [inout] interface_config     - interface config, must contain following info: port speed, number of lanes and medium
 */
int portmod_common_default_interface_get(portmod_port_interface_config_t* interface_config)
{
    int is_higig;
    int fiber_pref;

    is_higig =  ((interface_config->encap_mode == SOC_ENCAP_HIGIG2) ||
                 (interface_config->encap_mode == SOC_ENCAP_HIGIG)  ||
                 PHYMOD_INTF_MODES_HIGIG_GET(interface_config)) ? 1 : 0;
    fiber_pref = PHYMOD_INTF_MODES_FIBER_GET(interface_config);

    if (interface_config->port_num_lanes == 1) {
        if (interface_config->speed > 12000) {
            if (is_higig) {
                interface_config->interface = SOC_PORT_IF_CR;
            } else {
                interface_config->interface = fiber_pref ?
                    SOC_PORT_IF_SR : SOC_PORT_IF_CR;
            }
        } else if (interface_config->speed >= 10000) {
            if (is_higig) {
                interface_config->interface = SOC_PORT_IF_XFI;
            } else {
                interface_config->interface = fiber_pref ?
                    SOC_PORT_IF_SFI : SOC_PORT_IF_XFI;
            }
        } else if (interface_config->speed == 5000) {
            interface_config->interface = SOC_PORT_IF_GMII;
        } else {
            interface_config->interface = fiber_pref ?
                SOC_PORT_IF_GMII : SOC_PORT_IF_SGMII;
        }
    } else if (interface_config->port_num_lanes == 2) {
        if (interface_config->speed >= 40000) {
            interface_config->interface = SOC_PORT_IF_KR2;
        } else if (interface_config->speed >=20000) {
            interface_config->interface = SOC_PORT_IF_CR2;
        } else {
            /* 10G default interface type */
            interface_config->interface = SOC_PORT_IF_RXAUI;
        }
    } else if (interface_config->port_num_lanes == 3) {
        /* Add the interface later (if valid )*/
    } else if (interface_config->port_num_lanes == 4) {
        if (interface_config->speed >= 100000) {
            if (is_higig){
                interface_config->interface = SOC_PORT_IF_CR4;
            } else{
                interface_config->interface = SOC_PORT_IF_CAUI4;
            }
        } else if (interface_config->speed >= 40000) {
            if (is_higig){
                interface_config->interface = SOC_PORT_IF_CR4;
            } else{
                interface_config->interface = SOC_PORT_IF_XLAUI;
            }
        } else if (interface_config->speed >= 20000) {
            if (is_higig){
                interface_config->interface = SOC_PORT_IF_XGMII;
            } else{
                interface_config->interface = 
                      fiber_pref? SOC_PORT_IF_SR4 : SOC_PORT_IF_CR4;
            }
        } else if (interface_config->speed >= 10000) {
            if (is_higig){
                interface_config->interface = SOC_PORT_IF_XGMII;
            } else{
                interface_config->interface = SOC_PORT_IF_XAUI;
            }
           
        }
    } else if (interface_config->port_num_lanes == 10) {  /* 100G */
            interface_config->interface = SOC_PORT_IF_CAUI;
    } else if (interface_config->port_num_lanes == 12) {  /* 120G */
            interface_config->interface = SOC_PORT_IF_CAUI;
    } else {
        /* Do Nothing - let it be default */
    }
    
    return SOC_E_NONE;
}
#undef _ERR_MSG_MODULE_NAME
