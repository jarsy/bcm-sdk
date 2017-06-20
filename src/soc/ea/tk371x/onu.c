/*
 * $Id: onu.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    onu.c
 * Purpose:     Tracks and manages onu chip.
 *      
 */
#include <assert.h>
#include <sal/core/libc.h>
#include <soc/drv.h>
#include <soc/ctrl_if.h>
#include <soc/debug.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/ea_drv.h>
#include "tk3715.h"
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/TkBrgApi.h>
#include <soc/ea/tk371x/TkOnuApi.h>
#include <soc/ea/tk371x/TkTmApi.h>
#include <soc/ea/tk371x/CtcMiscApi.h>
#include <soc/ea/tk371x/TkRuleApi.h>
#include <soc/ea/tk371x/onu.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/TkOamFwUpgradeApi.h>

int 
_soc_ea_load_info_get(int32 unit, soc_ea_load_info_t *load_info)
{
    int rv = SOC_E_NONE;
    
    if(NULL == load_info){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoLoadInfo, 
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    sal_memcpy(load_info, SOC_EA_PRIVATE(unit)->load_info, 
        sizeof(soc_ea_load_info_t)*socEaFileCount);
    
    return SOC_E_NONE;
}

/* here are the onu information part */
int
_soc_ea_firmware_ver_get(int unit, uint16 *ver)
{
    int rv = SOC_E_NONE;
    
    if(NULL == ver){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoOnuInfo, 
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    *ver = SOC_EA_PRIVATE(unit)->onu_info.firmware_ver;
    
    return SOC_E_NONE;
}

int 
_soc_ea_revision_id_get(int unit, uint32 *rev)
{
    int rv = SOC_E_NONE;
    
    if(NULL == rev){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoRevisionId,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    *rev = SOC_EA_PRIVATE(unit)->chip_revision;
    
    return SOC_E_NONE;
}

int 
_soc_ea_vendor_id_get(int unit, soc_ea_extend_id_t id)
{
    int rv = SOC_E_NONE;
    
    if(NULL == id){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoOnuInfo,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    sal_memcpy(id, SOC_EA_PRIVATE(unit)->extend_id, SOC_EA_EXTEND_ID_LEN);
    
    return SOC_E_NONE;
}


int
_soc_ea_ctc_oam_ver_get(int unit, uint32 *ver)
{
    int rv = SOC_E_NONE;
    
    if(NULL == ver){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoCtcOamVer,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    *ver = SOC_EA_PRIVATE(unit)->oam_ver;
    return SOC_E_NONE;
}

int
_soc_ea_real_llid_count_get(int unit, uint32 *llid_count)
{
    int rv = SOC_E_NONE;

    if(NULL == llid_count){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit, socEaChipInfoLllidCount,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE == rv){
        *llid_count = SOC_EA_PRIVATE(unit)->llid_count;
    }

    return rv;
}

int
_soc_ea_info_get(int unit, soc_ea_onu_info_t *info)
{
    int rv = SOC_E_NONE;

    if(NULL == info){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit, socEaChipInfoOnuInfo,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE == rv){
        sal_memcpy(info, &(SOC_EA_PRIVATE(unit)->onu_info), sizeof(soc_ea_onu_info_t));
    }

    return rv;
}

/*
 * Function:
 *      _soc_ea_mllid_set
 * Description:
 *      Multiple llid enable 0 and 1 means single llid while other means multiple
 *      llid 
 * Parameters:
 *      unit  - (IN) BCM device number
 *      count - (IN) 0 - 1 single llid,
 *              2 to max num means multiple llid
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_mllid_set(int unit, uint32 count)
{
    int rv;
    
    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;   
    }
    
    rv = _soc_ea_chip_control_sync(unit,socEaChipControlLlidCount,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv) {
        return rv;
    }

    if (!(count <= SOC_EA_PRIVATE(unit)->onu_info.max_link_count)){
        _soc_ea_chip_control_flag_update(unit, socEaChipControlLlidCount, FALSE);
        return SOC_E_FAIL;
    }
    rv = CtcExtOamSetMulLlidCtrl(unit, count);
    if(SOC_E_NONE != rv){
        _soc_ea_chip_control_flag_update(unit, socEaChipControlLlidCount, FALSE);
        return SOC_E_FAIL;
    }
    SOC_EA_CONTROL_MLLID_NUM(unit) = count;
    _soc_ea_chip_control_flag_update(unit, socEaChipControlLlidCount, TRUE);
    return SOC_E_NONE;
}

/*
 * Function:
 *      _soc_ea_mllid_get
 * Description:
 *      Multiple llid enable 0 and 1 means single llid while other means multiple
 *      llid 
 * Parameters:
 *      unit  - (IN) BCM device number
 *      count - (IN) 0 - 1 single llid,
 *              2 to max num means multiple llid
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_mllid_get(int unit, uint32 *count)
{
    int rv;
    
    if(NULL == count){
        return SOC_E_PARAM;
    }
    
    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;   
    }

    rv = _soc_ea_chip_control_sync(unit,socEaChipControlLlidCount,
            SOC_EA_SYNC_FLAG_NORMAL);
    if(SOC_E_NONE == rv){
        *count = SOC_EA_CONTROL_MLLID_NUM(unit);
    }
    
    return rv;
}

int
_soc_ea_queue_configuration_get(int unit, TkQueueConfigInfo * q_cfg)
{
    int rv;
    soc_ea_chip_q_config_t *p_q_cfg;
    int i, j;
    
    if(NULL == q_cfg){
        return SOC_E_PARAM;
    }
    
    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;   
    }
    
    rv = _soc_ea_chip_control_sync(unit,socEaChipControlQueueConfig,
            SOC_EA_SYNC_FLAG_NORMAL);
    if(SOC_E_NONE != rv){
        return rv;
    }

    p_q_cfg = SOC_EA_CONTROL_Q_CONFIG(unit);
    
    q_cfg->cntOfLink = p_q_cfg->link_count;
    
    if(p_q_cfg->link_count > MAX_CNT_OF_LINK){
        return SOC_E_FAIL;
    }

    for(i = 0; i < p_q_cfg->link_count; i++){

        q_cfg->linkInfo[i].cntOfUpQ = 
                p_q_cfg->link_info[i].q_count;
        
        if(p_q_cfg->link_info[i].q_count > MAX_CNT_OF_UP_QUEUE){
            return SOC_E_FAIL;
        }
        
        for(j = 0; j < p_q_cfg->link_info[i].q_count; j++){
            q_cfg->linkInfo[i].sizeOfUpQ[j] = 
                p_q_cfg->link_info[i].q_size[j];
        }
    }

    q_cfg->cntOfPort = p_q_cfg->port_count;
    
    if(q_cfg->cntOfPort > SDK_MAX_NUM_OF_PORT){
        return SOC_E_FAIL;
    }
    
    for(i = 0; i < MAX_CNT_OF_PORT; i++){
        /* coverity[overrun-local] */
        q_cfg->portInfo[i].cntOfDnQ = p_q_cfg->port_info[i].q_count;
        if(p_q_cfg->port_info[i].q_count > MAX_CNT_OF_UP_QUEUE){
            return SOC_E_FAIL;
        }
        for(j = 0; j < p_q_cfg->port_info[i].q_count;j++){
            q_cfg->portInfo[i].sizeOfDnQ[j] = p_q_cfg->port_info[i].q_size[j];    
        }  
    }

    q_cfg->mcastInfo.cntOfDnQ = p_q_cfg->mcast_info.q_count;
    if(q_cfg->mcastInfo.cntOfDnQ > MAX_CNT_OF_UP_QUEUE){
        return SOC_E_FAIL;
    }
    for(i = 0; i < p_q_cfg->mcast_info.q_count; i++){
        q_cfg->mcastInfo.sizeOfDnQ[i] = p_q_cfg->mcast_info.q_size[i];
    }
     
    return SOC_E_NONE;
}


int
_soc_ea_queue_configuration_set(int unit, TkQueueConfigInfo *q_cfg)
{
    int rv;
    soc_ea_chip_q_config_t *p_q_cfg;
    int i, j;
    uint8 path_id;
    
    if(NULL == q_cfg){
        return SOC_E_PARAM;
    }
    
    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;   
    }

    path_id = unit;
    rv = TkQueueSetConfiguration(path_id, q_cfg);
    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }
    
    p_q_cfg = SOC_EA_CONTROL_Q_CONFIG(unit);
    
    p_q_cfg->link_count = q_cfg->cntOfLink;
    if(p_q_cfg->link_count > MAX_CNT_OF_LINK){
        return SOC_E_FAIL;
    }

    for(i = 0; i < q_cfg->cntOfLink; i++){

        p_q_cfg->link_info[i].q_count = q_cfg->linkInfo[i].cntOfUpQ; 
        
        if(q_cfg->linkInfo[i].cntOfUpQ > MAX_CNT_OF_UP_QUEUE){
            return SOC_E_FAIL;
        }
        
        for(j = 0; j < q_cfg->linkInfo[i].cntOfUpQ; j++){
            p_q_cfg->link_info[i].q_size[j] 
                = q_cfg->linkInfo[i].sizeOfUpQ[j];    
        }
    }

    p_q_cfg->port_count = q_cfg->cntOfPort;
    
    if(p_q_cfg->port_count > SDK_MAX_NUM_OF_PORT){
        return SOC_E_FAIL;
    }
    
    for(i = 0; i < q_cfg->cntOfPort; i++){
        p_q_cfg->port_info[i].q_count = q_cfg->portInfo[i].cntOfDnQ; 
        if(q_cfg->portInfo[i].cntOfDnQ > MAX_CNT_OF_UP_QUEUE){
            return SOC_E_FAIL;
        }
        for(j = 0; j < p_q_cfg->port_info[i].q_count; j++){
            p_q_cfg->port_info[i].q_size[j] = q_cfg->portInfo[i].sizeOfDnQ[j]; 
        }   
    }

    p_q_cfg->mcast_info.q_count = q_cfg->mcastInfo.cntOfDnQ;
    if(q_cfg->mcastInfo.cntOfDnQ > MAX_CNT_OF_UP_QUEUE){
        return SOC_E_FAIL;
    }
    for(i = 0; i < q_cfg->mcastInfo.cntOfDnQ; i++){
        p_q_cfg->mcast_info.q_size[i] = q_cfg->mcastInfo.sizeOfDnQ[i];
    }

    _soc_ea_chip_control_flag_update(unit, socEaChipControlQueueConfig,
        TRUE);
    return SOC_E_NONE;
}




/*
 * Function:
 *      _soc_ea_gpio_write
 * Description:
 *      Write a GPIO value belonged to the bank, only the bits masked by the mask will be 
 *      write to the gpio register.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 *      data - (IN) Value to be write to the GPIO
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_gpio_write(int unit, uint32 flags, uint32 mask, uint32 data)
{
    int rv = SOC_E_NONE;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    if(GPIO_FLAGS_TK371X != flags){
        return SOC_E_PARAM;
    }

    flags = flags;
    rv = TkExtOamSetGpio(unit, 0, mask, data);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }
    
    return SOC_E_NONE; 
}

/*
 * Function:
 *      _soc_ea_gpio_read
 * Description:
 *      Read a GPIO value belonged to the bank
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      data - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_gpio_read(int unit, uint32 flags, uint32 *data)
{
    int rv = SOC_E_NONE;

    if(NULL == data){
        return SOC_E_PARAM;
    }

    if(GPIO_FLAGS_TK371X != flags){
        return SOC_E_PARAM;
    }
    
    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    flags = flags;
    rv = TkExtOamGetGpio(unit, 0, data);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }
    
    return SOC_E_NONE; 
}

/*
 * Function:
 *      _soc_ea_gpio_config_write
 * Description:
 *      Wirte the gpio configuration
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_gpio_config_write(int unit, uint32 flags, uint32 mask)
{
    int rv = SOC_E_NONE;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    if(GPIO_FLAGS_TK371X != flags){
        return SOC_E_PARAM;
    }
    
    flags = flags;
    rv = TkExtOamSetGpioConfig(unit, 0, mask);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }
    
    return SOC_E_NONE; 
}

/*
 * Function:
 *      _soc_ea_gpio_config_read
 * Description:
 *      Read the gpio configuration
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_gpio_config_read(int unit, uint32 flags, uint32 *mask)
{
    int rv = SOC_E_NONE;

    if(NULL == mask){
        return SOC_E_PARAM;
    }
    
    if(GPIO_FLAGS_TK371X != flags){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    flags = flags;
    rv = TkExtOamGetGpioConfig(unit, 0, mask);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }
    
    return SOC_E_NONE; 
}

int
_soc_ea_port_info_get(int unit, soc_port_t port, soc_ea_port_control_t type, 
    int *value)
{
    int rv;
    soc_ea_port_control_db_t *port_info;
    uint32 pkts_cnt;
    
    if(NULL == value){
        return SOC_E_PARAM;
    }

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }
    
    /* coverity[result_independent_of_operands] */
    if(!SOC_PORT_VALID_RANGE(unit,port)){
        /* coverity[dead_error_line] */
        return SOC_E_PORT;   
    }

    if(type >= socEaPortCount){
        return SOC_E_PARAM;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoPort,SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    } 

    port_info = &(SOC_EA_PRIVATE(unit)->port[port]);

    switch(type){
        case socEaPortInterface:
            *value = port_info[0].interface;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAutoneg:
            *value = port_info[0].autoneg;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortSpeed:
            *value = port_info[0].speed;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortDuplex:
            *value = port_info[0].duplex;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAdvertisement:
        case socEaPortAdvertisementRemote:
            rv = SOC_E_UNAVAIL;
            break;
        case socEaPortAbilitySpeedHalfDuplex:
            *value = port_info[0].ability_speed_half_duplex;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilitySpeedFullDuplex:
            *value = port_info[0].ability_speed_full_duplex;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilityPause:
            *value = port_info[0].ability_pause;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilityInterface:
            *value = port_info[0].ability_interface;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilityMedium:
            *value = port_info[0].ability_medium;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilityLoopback:
            *value = port_info[0].ability_loopback;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortAbilityEncap:
            *value = port_info[0].ability_encap;
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = SOC_E_NONE;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortMdix:
        case socEaPortMdixState:
            rv = SOC_E_UNAVAIL;
            break;
        case socEaPortBcastLimit:
        case socEaPortBcastLimitState:
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = TkExtOamGetBcastRateLimit(unit, 0, port, &pkts_cnt);
                ((uint32 *)value)[0] = pkts_cnt;
            }else{
                rv = SOC_E_UNAVAIL;
            }
            break;
        case socEaPortPhyMaster:
        case socEaPortPauseTx:
        case socEaPortPauseRx:
            rv = SOC_E_UNAVAIL;
            break;
        case socEaPortLoopback:
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_LLID == SOC_PORT_TYPE(unit, port))){
                ((uint32 *)value)[0] = port_info[0].loopback;
            }
            rv = SOC_E_NONE;
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    
    return rv;
}

int
_soc_ea_port_info_set(int unit, soc_port_t port, soc_ea_port_control_t type, 
    int value)
{
    int rv;
    soc_ea_port_control_db_t *port_info;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }
    
    /* coverity[result_independent_of_operands] */
    if(!SOC_PORT_VALID_RANGE(unit,port)){
        /* coverity[dead_error_line] */
        return SOC_E_PORT;   
    }

    if(type >= socEaPortCount){
        return SOC_E_PARAM;
    }

    port_info = &(SOC_EA_PRIVATE(unit)->port[port]);

    switch(type){
        case socEaPortInterface:
        case socEaPortAutoneg:
        case socEaPortSpeed:
        case socEaPortPauseTx:
        case socEaPortPauseRx:
        case socEaPortDuplex:
        case socEaPortAdvertisement:
        case socEaPortAdvertisementRemote:
        case socEaPortAbilitySpeedHalfDuplex:
        case socEaPortAbilitySpeedFullDuplex:
        case socEaPortAbilityPause:
        case socEaPortAbilityInterface:
        case socEaPortAbilityMedium:
        case socEaPortAbilityLoopback:
        case socEaPortAbilityEncap:
        case socEaPortMdix:
        case socEaPortMdixState:
        case socEaPortPhyMaster:
            rv = SOC_E_UNAVAIL;
            break;
        case socEaPortBcastLimit:
        case socEaPortBcastLimitState:
            rv = TkExtOamSetBcastRateLimit(unit, 0, port, value);
            if(SOC_E_NONE == rv){
                port_info[0].bcast_limit = value;
            }else{
                rv = SOC_E_NONE;
            } 
            break;
        case socEaPortLoopback:
            {
            OamLoopbackLoc loc;

            loc = value;
            
            if((SOC_EA_BLK_GE == SOC_PORT_TYPE(unit, port))
                ||(SOC_EA_BLK_FE == SOC_PORT_TYPE(unit, port))){
                rv = TkExtOamPortSetLoopback(unit,0, port, loc);
            }else if(SOC_EA_BLK_LLID == SOC_PORT_TYPE(unit, port)){
                rv = TkExtOamLlidSetLoopback(unit, (port - SOC_PORT_MIN(unit,llid)),
                    (port - SOC_PORT_MIN(unit,llid)), loc);
            }else{
                rv = SOC_E_UNAVAIL;
            }
            if(rv == SOC_E_NONE){
                port_info[0].loopback = ((uint32 *)(&value))[0];    
            }else{
               rv = SOC_E_FAIL;
            }
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    
    return rv;
}

void 
soc_ea_dbg_level_set(uint32 lvl)
{
    TkDbgLevelSet(lvl);
}

void 
soc_ea_dbg_level_dump(void)
{
    TkDbgLevelDump();
}

uint16 
soc_ea_firmware_ver_get(int unit)
{
    uint16 fw_ver;
    int rv;
    
    rv = _soc_ea_firmware_ver_get(unit, &fw_ver); 
    if(SOC_E_NONE != rv){
        fw_ver = 0;
    }
    return fw_ver;
}

uint16 
soc_ea_jedecid_get(int unit)
{
    int rv;
    soc_ea_onu_info_t info;

    rv = _soc_ea_info_get(unit, &info);
    
    if(SOC_E_NONE != rv)return 0;

    return info.jedec_id;
}

uint16 
soc_ea_chip_id_get(int unit)
{
    int rv;
    soc_ea_onu_info_t info;

    rv = _soc_ea_info_get(unit, &info);
    
    if(SOC_E_NONE != rv)return 0;

    return info.chip_id;
}

uint32 
soc_ea_chip_rev_get(int unit)
{
    int rv;
    soc_ea_onu_info_t info;

    rv = _soc_ea_info_get(unit, &info);
    
    if(SOC_E_NONE != rv)return 0;

    return info.chip_ver;
}

uint8* 
soc_ea_extend_id_get(int unit)
{
    int rv;
    static soc_ea_extend_id_t id;
   
    rv = _soc_ea_vendor_id_get(unit,id);  

    if(SOC_E_NONE != rv)return NULL;

    return id;
}

/*
 * Function:
 *      soc_gpio_write
 * Description:
 *      Write a GPIO value belonged to the bank, only the bits masked by the mask will be 
 *      write to the gpio register.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 *      data - (IN) Value to be write to the GPIO
 * Returns     : SOC_E_XXX
 */
int
soc_gpio_write(int unit, uint32 flags, uint32 mask, uint32 data)
{
    return _soc_ea_gpio_write(unit, flags, mask, data);
}

/*
 * Function:
 *      soc_gpio_read
 * Description:
 *      Read a GPIO value belonged to the bank
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      data - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
soc_gpio_read(int unit, uint32 flags, uint32 *data)
{
    return _soc_ea_gpio_read(unit, flags, data);
}

/*
 * Function:
 *      soc_gpio_config_write
 * Description:
 *      Set Bitmask of GPIO ports for which to generate alert.  Allow the host to be notified 
 *      of changes at the GPIO ports of an ONU through alerts
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 * Returns     : SOC_E_XXX
 */
int
soc_gpio_config_write(int unit, uint32 flags, uint32 mask)
{
    return _soc_ea_gpio_config_write(unit, flags, mask);
}

/*
 * Function:
 *      soc_gpio_config_read
 * Description:
 *      Get Bitmask of GPIO ports for which to generate alert.  Allow the host to be notified 
 *      of changes at the GPIO ports of an ONU through alerts
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
soc_gpio_config_read(int unit, uint32 flags, uint32 *mask)
{
    return _soc_ea_gpio_config_read(unit, flags, mask);
}

/*
 * Function:
 *      soc_base_mac_set
 * Description:
 *      Allow user to change the PON base mac which is used form MPCP registering and OAM 
 *      source MAC address. 
 *      If enable multiple llid feature, the llids' mac address will generated from this mac 
 *      address by +1.
 *      The uni base mac address is used for uni port such as the pause frame which come 
 *      from the uni port will pack this mac as the source mac. uni0 will have this mac while
 *      uni1 use uni_base_mac +1, so do other unis
 * Parameters:
 *      unit  - (IN) BCM device number
 *      mac - (IN) Including the PON base mac and UNI base mac
 * Returns     : SOC_E_XXX
 */
int 
soc_base_mac_set(int unit, soc_base_mac_t mac)
{
    int rv = SOC_E_NONE;
    OamEponBaseMac base_mac;
    
    if (!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }
    
    sal_memcpy(base_mac.PonBaseMac, mac.pon_base_mac, sizeof(sal_mac_addr_t));
    sal_memcpy(base_mac.UserBaseMac, mac.uni_base_mac, sizeof(sal_mac_addr_t));
   
    rv = TkExtOamSetEponBaseMac(unit, &base_mac);
    
    if (SOC_E_NONE == rv) {
        sal_memcpy(SOC_EA_PRIVATE(unit)->onu_info.pon_base_mac,mac.pon_base_mac, 
            sizeof(sal_mac_addr_t));
        sal_memcpy(SOC_EA_PRIVATE(unit)->onu_info.uni_base_mac,mac.uni_base_mac,
            sizeof(sal_mac_addr_t));   
        return SOC_E_NONE;
    } else {
        return SOC_E_FAIL;
    }
}

/*
 * Function:
 *      soc_base_mac_get
 * Description:
 *      Allow user to get the PON base mac which is used form MPCP registering and OAM 
 *      source MAC address. 
 *      If enable multiple llid feature, the llids' mac address will generated from this mac 
 *      address by +1.
 *      The uni base mac address is used for uni port such as the pause frame which come 
 *      from the uni port will pack this mac as the source mac. uni0 will have this mac while
 *      uni1 use uni_base_mac +1, so do other unis
 * Parameters:
 *      unit  - (IN) BCM device number
 *      mac - (IN) Including the PON base mac and UNI base mac
 * Returns     : SOC_E_XXX
 */
int 
soc_base_mac_get(int unit, soc_base_mac_t *mac)
{
    int rv = SOC_E_NONE;

    if (NULL == mac) {
        return SOC_E_PARAM;
    }
    
    if (!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    rv = _soc_ea_chip_info_sync(unit,socEaChipInfoOnuInfo,
        SOC_EA_SYNC_FLAG_NORMAL);

    if(SOC_E_NONE != rv){
        return rv;
    }

    sal_memcpy(mac->pon_base_mac,SOC_EA_PRIVATE(unit)->onu_info.pon_base_mac, 
        sizeof(sal_mac_addr_t));
    sal_memcpy(mac->uni_base_mac,SOC_EA_PRIVATE(unit)->onu_info.uni_base_mac,
        sizeof(sal_mac_addr_t));

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_chip_reset
 * Description:
 *      Reset the EPON chipset by sending OAM message
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : SOC_E_XXX
 */
int 
soc_chip_tk371x_reset(int unit)
{
    int rv; 
    uint8 path_id;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    path_id = (uint8)unit;

    rv = TkExtOamSetResetOnu(path_id, 0);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }else{
        return SOC_E_NONE;
    }
}

/*
 * Function:
 *      soc_nvs_erase
 * Description:
 *      Erase the EPON configuration that stored in the EPON chipset's NVS by 
 *      sending OAM message
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : SOC_E_XXX
 */
int
soc_nvs_erase(int unit)
{
    int rv;
    uint8 path_id;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    path_id = (uint8)unit;

    rv = TkExtOamSetEraseNvs(path_id, 0);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }else{
        return SOC_E_NONE;
    }
}

/*
 * Function:
 *      soc_auth_result_set
 * Description:
 *      Commit the LOID authentication result to EPON chipset
 * Parameters:
 *      unit  - (IN) BCM device number
 *      state - (IN) TRUE means fail while FALSE means success
 * Returns     : SOC_E_XXX
 */
int
soc_auth_result_set(int unit, int state)
{
    int rv;
    uint8 path_id;
    uint8 result;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    if(state != FALSE && state != TRUE){
        return SOC_E_PARAM;
    }

    result = (uint8)state;
    path_id = (uint8)unit;
    rv = TkExtOamSetCTCLoidAuthIfFail(path_id, 0, result);

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }else{
        return SOC_E_NONE;
    }
}

/*
 * Function:
 *      soc_auth_result_get
 * Description:
 *      Get the LOID authentication result from EPON chipset
 * Parameters:
 *      unit  - (IN) BCM device number
 *      state - (IN) TRUE means fail while FALSE means success
 * Returns     : SOC_E_XXX
 */
int 
soc_auth_result_get(int unit, int *state)
{
    int rv;
    uint8 path_id;
    uint8 result;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }

    if(NULL == state){
        return SOC_E_PARAM;
    }

    path_id = (uint8)unit;
    
    rv = TkExtOamGetCTCLoidAuthIfFail(path_id, 0, &result);

    if(!result){
        *state = FALSE;
    }else{
        *state = TRUE;
    }

    if(rv != SOC_E_NONE){
        return SOC_E_FAIL;
    }else{
        return SOC_E_NONE;
    }
}

int 
soc_ea_firmware_upgrade(int unit, int type, int length, uint8* buf)
{
    int rv;
    uint8 path_id;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }
    
    if(NULL == buf){
        return SOC_E_PARAM;
    }

    path_id = (uint8)unit;
    
    rv = TkExtFirmwareUpgrade(path_id, type, length, buf);

    if(SOC_E_NONE != rv)return SOC_E_FAIL;

    return SOC_E_NONE;
} 

