/*
 * $Id: drv.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <shared/bsl.h>

#include <assert.h>
#include <sal/core/libc.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/ea_drv.h>
#include <soc/ctrl_if.h>
#include <soc/debug.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOnuApi.h>
#include <soc/ea/tk371x/counter.h>
#include <soc/ea/tk371x/TkRuleApi.h>
#include <soc/ea/tk371x/onu.h>
#include "tk3715.h"

extern int ea_probe_thread_running;
extern void _TkDataProcessHandle(uint8 unit, char *data, uint16 len);

int soc_tk_ndev = 0;
int soc_tk_pre_attached = 0;

static soc_ea_private_db_t soc_ea_tk371x_a0_info;
int ea_probe_thread_running = FALSE;

soc_ea_chip_rev_map_t soc_ea_chip_rev_map[] = {
    {0x0,   0xA0100804, "TK371X_A0"},
    {0xff,  0xffffffff, NULL}
};

soc_block_name_t        soc_block_ea_port_names[] ={
    { SOC_EA_BLK_PON,     0,  "pon"   },
    { SOC_EA_BLK_GE,      0,  "ge"    },
    { SOC_EA_BLK_FE,      0,  "fe"    },
    { SOC_EA_BLK_LLID,    0,  "llid"  },
    { SOC_EA_BLK_NONE,    0,  ""      }
};

soc_block_name_t        soc_block_ea_names[] = {
    /*    blk  , isalias, name */
    { SOC_EA_BLK_PON,       0,  "pon" },
    { SOC_EA_BLK_GE,        0,  "ge"  },
    { SOC_EA_BLK_FE,        0,  "fe"  },
    { SOC_EA_BLK_LLID,      0,  "llid"},
    { SOC_EA_BLK_NONE,      0,  ""    }
};

uint8 
soc_ea_tk2bcm_revision_map(uint32 tk_rev)
{
    int i = 0;

    for (i = 0; soc_ea_chip_rev_map[i].bcm_revision != 0xFF; i++) {
        if(tk_rev == soc_ea_chip_rev_map[i].tk_revision){
            return soc_ea_chip_rev_map[i].bcm_revision;
        }
    }

    return soc_ea_chip_rev_map[i].bcm_revision;
}

uint32 
soc_ea_bcm2tk_revision_map(uint8 bcm_rev)
{
    int i = 0; 

    for (i = 0;soc_ea_chip_rev_map[i].bcm_revision != 0xFF; i++) {
        if (soc_ea_chip_rev_map[i].bcm_revision == bcm_rev) {
            return soc_ea_chip_rev_map[i].tk_revision;
        }
    }

    return soc_ea_chip_rev_map[i].tk_revision;
}

char *
soc_block_ea_port_name_lookup_ext(soc_block_t blk, int unit)
{
    int i;

    for(i = 0; soc_block_ea_port_names[i].blk != SOC_EA_BLK_NONE; i++){
        if(soc_block_ea_port_names[i].blk == blk){
            return soc_block_ea_port_names[i].name;
        }
    }

    return "?";
}

char *
soc_block_ea_name_lookup_ext(soc_block_t blk, int unit)
{
    int i;

    for(i = 0; soc_block_ea_names[i].blk != SOC_EA_BLK_NONE; i++){
        if(soc_block_ea_names[i].blk == blk){
            return soc_block_ea_names[i].name;
        }
    }

    return "?";
}

soc_ea_oam_ctrl_if_t *
soc_ea_oam_ctrl_attach(uint32 type)
{
    /* Attach control interface driver table */
    {
        extern soc_ea_oam_ctrl_if_t ea_oam_ether_ctrlops;
        if (type == socEaOamCtrlIfEther) {
            return (&ea_oam_ether_ctrlops);
        }
    }
    return (NULL);
}

int 
soc_ea_private_init(int unit)
{
    soc_control_t *soc;
    soc_info_t    *si;

    if (NULL == SOC_CONTROL(unit)) {
        return SOC_E_UNIT;
    }
    
    soc = SOC_CONTROL(unit);

    si = &soc->info;

    si->private = (void *)&soc_ea_tk371x_a0_info;

    SOC_EA_PRIVATE_FLAG_CLEAR(unit, 0xFFFFFFFF);

    return SOC_E_NONE;
}

int 
_soc_ea_chip_info_sync(int unit, soc_ea_chip_info_t type, int flag)
{
    int rv = SOC_E_NONE;
    uint32 buf[128] = {0};
    int index;

    if(NULL == SOC_EA_DB(unit))return SOC_E_INIT;
    if(!SOC_IS_EA(unit))return SOC_E_UNIT;
    if(socEaChipInfoCount <= type)return SOC_E_PARAM;

    switch(type){
        case socEaChipInfoLoadInfo:
            if((SOC_EA_PRIVATE_FLAG_IS_SET(unit, SOC_EA_CHIP_INFO_LOAD_INFO))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }

            if(SOC_IS_TK371X(unit)){
                rv = TkExtOamGetLoadInfo(unit, 0, (OamLoadInfo*)buf);
                if(OK != rv){
                    return SOC_E_FAIL;
                }
                
                SOC_EA_PRIVATE(unit)->load_info[socEaFileBoot].legacy_ver = 
                    ((OamLoadInfo *)buf)->bootVer;
                SOC_EA_PRIVATE(unit)->load_info[socEaFileBoot].crc = 
                    ((OamLoadInfo *)buf)->bootCrc;
                SOC_EA_PRIVATE(unit)->load_info[socEaFilePers].legacy_ver = 
                    ((OamLoadInfo *)buf)->persVer;
                SOC_EA_PRIVATE(unit)->load_info[socEaFilePers].crc = 
                    ((OamLoadInfo *)buf)->persCrc;
                SOC_EA_PRIVATE(unit)->load_info[socEaFileApp0].legacy_ver = 
                    ((OamLoadInfo *)buf)->app0Ver;
                SOC_EA_PRIVATE(unit)->load_info[socEaFileApp0].crc = 
                    ((OamLoadInfo *)buf)->app0Crc;
                SOC_EA_PRIVATE(unit)->load_info[socEaFileApp1].legacy_ver = 
                    ((OamLoadInfo *)buf)->app1Ver;
                SOC_EA_PRIVATE(unit)->load_info[socEaFileApp1].crc = 
                    ((OamLoadInfo *)buf)->app1Crc;
            }
            SOC_EA_PRIVATE_FLAG_SET(unit,SOC_EA_CHIP_INFO_LOAD_INFO);   
            break;
        case socEaChipInfoOnuInfo:
        case socEaChipInfoVendorId:
            if((SOC_EA_PRIVATE_FLAG_IS_SET(unit,SOC_EA_CHIP_INFO_ONU_INFO))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }
            rv = TkExtOamOnuInfo(unit, 0, (OamTkInfo*)buf);
            
            if(OK == rv){
                SOC_EA_PRIVATE(unit)->onu_info.firmware_ver = 
                    ((OamTkInfo*)buf)->firmwareVersion;
                sal_memcpy(SOC_EA_PRIVATE(unit)->onu_info.pon_base_mac, 
                    ((OamTkInfo*)buf)->baseMac, sizeof(sal_mac_addr_t));
                SOC_EA_PRIVATE(unit)->onu_info.port_count = 
                    ((OamTkInfo*)buf)->numLinks;
                SOC_EA_PRIVATE(unit)->onu_info.max_link_count = 
                    ((OamTkInfo*)buf)->numPorts;
                SOC_EA_PRIVATE(unit)->onu_info.up_queue_count = 
                    ((OamTkInfo*)buf)->numUpQueues;
                SOC_EA_PRIVATE(unit)->onu_info.max_queue_count_per_link = 
                    ((OamTkInfo*)buf)->maxUpQueuesPerLink;
                SOC_EA_PRIVATE(unit)->onu_info.up_queue_unit = 
                    ((OamTkInfo*)buf)->upQueueIncrement;
                SOC_EA_PRIVATE(unit)->onu_info.dn_queue_count = 
                    ((OamTkInfo*)buf)->numDnQueues;
                SOC_EA_PRIVATE(unit)->onu_info.max_queue_count_per_port = 
                    ((OamTkInfo*)buf)->maxDnQueuesPerLink;
                SOC_EA_PRIVATE(unit)->onu_info.dn_queue_unit = 
                    ((OamTkInfo*)buf)->dnQueueIncrement;
                SOC_EA_PRIVATE(unit)->onu_info.up_queue_total_size = 
                    ((OamTkInfo*)buf)->upBuffer;
                SOC_EA_PRIVATE(unit)->onu_info.dn_queue_total_size = 
                    ((OamTkInfo*)buf)->dnBuffer;
                SOC_EA_PRIVATE(unit)->onu_info.jedec_id = 
                    ((OamTkInfo*)buf)->jedecId;
                SOC_EA_PRIVATE(unit)->onu_info.chip_id = 
                    ((OamTkInfo*)buf)->chipId;
                SOC_EA_PRIVATE(unit)->onu_info.chip_ver = 
                    ((OamTkInfo*)buf)->chipVersion; 
                sal_memcpy(SOC_EA_PRIVATE(unit)->extend_id,
                    ((OamTkInfo*)buf)->extendedId,
                        sizeof(SOC_EA_PRIVATE(unit)->extend_id));
            }else {
                return SOC_E_FAIL;
            }
            
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_ONU_INFO); 
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_VERDOR_ID);
            break;
        case socEaChipInfoRevisionId:
            if((SOC_EA_PRIVATE_FLAG_IS_SET(unit, SOC_EA_CHIP_INFO_REVISION_ID))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }

            rv = TkExtOamGetChipRev(unit, 0, buf);
            if(OK != rv){
                return SOC_E_FAIL;
            }
            SOC_EA_PRIVATE(unit)->chip_revision = soc_ea_tk2bcm_revision_map(buf[0]);
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_REVISION_ID);
            break;
        
        case socEaChipInfoCtcOamVer:
            if((SOC_EA_PRIVATE_FLAG_IS_SET(unit, SOC_EA_CHIP_INFO_CTC_OAM_VER))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }

            if(SOC_IS_TK371X(unit)){
                rv = TkExtOamGetCtcOamVersion(unit,(OamVersion *)buf);
                if(OK != rv){
                    return SOC_E_FAIL;
                }
                
                SOC_EA_PRIVATE(unit)->oam_ver = 
                    ((OamVersion *)buf)->CTCOAMVer;
            }
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_CTC_OAM_VER);
            break;
        case socEaChipInfoLllidCount:
            if((SOC_EA_PRIVATE_FLAG_IS_SET(unit, SOC_EA_CHIP_INFO_LLID_COUNT))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }
            if(SOC_IS_TK371X(unit)){
                rv = TkExtOamGetEponPortInfo(unit, 0, (OamEponPortInfo *)buf);
                if(OK != rv){
                    SOC_EA_PRIVATE(unit)->llid_count = 0;
                    return SOC_E_FAIL;
                }
                
                /*if epon  happended, clear it to zero*/
                SOC_EA_PRIVATE(unit)->llid_count = 0;
                
                if(!(((OamEponPortInfo *)buf)->EponLosState)){
                    for(index = 0; index < SDK_MAX_NUM_OF_LINK; index++){ 
                        if(LlidWaitingForGates == 
                            ((OamEponPortInfo *)buf)->LinkInfo[index].RegState){
                            SOC_EA_PRIVATE(unit)->llid_count++; 
                        }
                    } 
                }else{
                    return SOC_E_NONE;
                }
            }
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_LLID_COUNT);
            break;

        case socEaChipInfoPort:
            if(SOC_EA_PRIVATE_FLAG_IS_SET(unit, SOC_EA_CHIP_INFO_PORT)
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }
            
            if(SOC_IS_TK371X(unit)){
                for(index = 0; index < SDK_MAX_NUM_OF_PORT; index++){
                    /* coverity[result_independent_of_operands] */
                    if(SOC_PORT_VALID_RANGE(unit, index)){
                        switch(SOC_PORT_TYPE(unit, index)){
                            case SOC_EA_BLK_PON:
                                SOC_EA_PRIVATE(unit)->port[index].interface = 
                                    SOC_PORT_IF_TBI;
                                SOC_EA_PRIVATE(unit)->port[index].autoneg = FALSE;
                                SOC_EA_PRIVATE(unit)->port[index].loopback = socEaLoopBackNone;
                                break;
                            case SOC_EA_BLK_GE:
                                SOC_EA_PRIVATE(unit)->port[index].autoneg = FALSE;
                                SOC_EA_PRIVATE(unit)->port[index].interface = 
                                    SOC_PORT_IF_GMII;
                                SOC_EA_PRIVATE(unit)->port[index].speed = 
                                    DRV_PORT_STATUS_SPEED_2500M;
                                SOC_EA_PRIVATE(unit)->port[index].duplex = 
                                    DRV_PORT_STATUS_DUPLEX_FULL;
                                SOC_EA_PRIVATE(unit)->port[index].advert_local = 
                                    -1;
                                SOC_EA_PRIVATE(unit)->port[index].advert_remote = 
                                    -1;
                                SOC_EA_PRIVATE(unit)->port[index].ability_speed_half_duplex = 
                                    SOC_PA_ABILITY_NONE;
                                SOC_EA_PRIVATE(unit)->port[index].ability_speed_full_duplex = 
                                    SOC_PA_SPEED_2500MB;
                                SOC_EA_PRIVATE(unit)->port[index].ability_pause = 
                                    (SOC_PA_PAUSE|SOC_PA_PAUSE_ASYMM);
                                SOC_EA_PRIVATE(unit)->port[index].ability_interface =
                                    (SOC_PA_INTF_GMII|SOC_PA_INTF_TBI);
                                SOC_EA_PRIVATE(unit)->port[index].ability_medium = 
                                    SOC_PA_ABILITY_NONE;
                                SOC_EA_PRIVATE(unit)->port[index].ability_loopback = 
                                    SOC_PA_LB_MAC;
                                SOC_EA_PRIVATE(unit)->port[index].ability_encap = 
                                    SOC_PA_ENCAP_IEEE;
                                /*without mdix*/
                                /*without mdix state*/
                                SOC_EA_PRIVATE(unit)->port[index].bcast_limit = 0xffffffff;
                                SOC_EA_PRIVATE(unit)->port[index].bcast_limit_enable = -1;
                                SOC_EA_PRIVATE(unit)->port[index].phy_master = -1;
                                SOC_EA_PRIVATE(unit)->port[index].loopback = socEaLoopBackNone;
                                break;
                            case SOC_EA_BLK_FE:
                                SOC_EA_PRIVATE(unit)->port[index].autoneg = FALSE;
                                SOC_EA_PRIVATE(unit)->port[index].interface = 
                                    SOC_PORT_IF_MII;
                                SOC_EA_PRIVATE(unit)->port[index].speed = 
                                    DRV_PORT_STATUS_SPEED_100M;
                                SOC_EA_PRIVATE(unit)->port[index].duplex = 
                                    DRV_PORT_STATUS_DUPLEX_FULL;
                                SOC_EA_PRIVATE(unit)->port[index].advert_local = 
                                    -1;
                                SOC_EA_PRIVATE(unit)->port[index].advert_remote = 
                                    -1;
                                SOC_EA_PRIVATE(unit)->port[index].ability_speed_half_duplex = 
                                    (SOC_PA_SPEED_10MB|SOC_PA_SPEED_100MB);
                                SOC_EA_PRIVATE(unit)->port[index].ability_speed_full_duplex = 
                                    (SOC_PA_SPEED_10MB|SOC_PA_SPEED_100MB);
                                SOC_EA_PRIVATE(unit)->port[index].ability_pause = 
                                    (SOC_PA_PAUSE|SOC_PA_PAUSE_ASYMM);
                                SOC_EA_PRIVATE(unit)->port[index].ability_interface =
                                    (SOC_PA_INTF_GMII|SOC_PA_INTF_TBI);
                                SOC_EA_PRIVATE(unit)->port[index].ability_medium = 
                                    SOC_PA_ABILITY_NONE;
                                SOC_EA_PRIVATE(unit)->port[index].ability_loopback = 
                                    SOC_PA_LB_MAC;
                                SOC_EA_PRIVATE(unit)->port[index].ability_encap = 
                                    SOC_PA_ENCAP_IEEE;
                                /*without mdix*/
                                /*without mdix state*/
                                SOC_EA_PRIVATE(unit)->port[index].bcast_limit = 0xffffffff;
                                SOC_EA_PRIVATE(unit)->port[index].bcast_limit_enable = -1;
                                SOC_EA_PRIVATE(unit)->port[index].phy_master = -1;
                                SOC_EA_PRIVATE(unit)->port[index].loopback = socEaLoopBackNone;
                                break;
                            case SOC_EA_BLK_LLID:
                                 SOC_EA_PRIVATE(unit)->port[index].interface = 
                                    SOC_PORT_IF_TBI;
                                 SOC_EA_PRIVATE(unit)->port[index].autoneg = FALSE;
                                 SOC_EA_PRIVATE(unit)->port[index].loopback = socEaLoopBackNone;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_INFO_PORT);
            break;
        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

int 
_soc_ea_chip_control_sync(int unit, soc_ea_chip_control_t type, int flag)
{
    int rv = SOC_E_NONE;
    uint32 index;
    uint32 i;

    if(!SOC_IS_EA(unit))return SOC_E_UNIT;
    if(NULL == SOC_EA_DB(unit))return SOC_E_INIT;
    if(socEaChipControlCount <= type)return SOC_E_PARAM;
    
    switch(type){
        case socEaChipControlLlidCount:
            if((SOC_EA_CONTROL_FLAG_IS_SET(unit,SOC_EA_CHIP_CONTROL_MLLID))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }
            if(!SOC_IS_TK371X(unit)){
                return SOC_E_UNIT;
            }
            rv = SOC_E_NONE;
            SOC_EA_CONTROL_FLAG_SET(unit, SOC_EA_CHIP_CONTROL_MLLID);
            break;
            
        case socEaChipControlQueueConfig:
            if((SOC_EA_CONTROL_FLAG_IS_SET(unit,SOC_EA_CHIP_CONTROL_QUEUE_CONFIG))
                && !(flag&SOC_EA_SYNC_FLAG_FORCE)){
                return SOC_E_NONE;
            }
            if(SOC_IS_TK371X(unit)){
                uint8 path_id = unit;
                TkQueueConfigInfo q_config;
                
                rv = TkQueueGetConfiguration(path_id, &q_config);
                if(rv != OK){
                    return SOC_E_FAIL;
                }
                
                sal_memset(&(SOC_EA_CONTROL(unit).q_config), 0x0, 
                    sizeof(soc_ea_chip_q_config_t));
                if(q_config.cntOfLink > MAX_CNT_OF_LINK){
                    return SOC_E_FAIL;
                }
                
                SOC_EA_CONTROL(unit).q_config.link_count 
                        = q_config.cntOfLink;
                
                for(index = 0; index < q_config.cntOfLink; index++){
                     SOC_EA_CONTROL(unit).q_config.link_info[index].q_count
                        = q_config.linkInfo[index].cntOfUpQ;
                     if(q_config.linkInfo[index].cntOfUpQ > MAX_CNT_OF_UP_QUEUE){
                        return SOC_E_FAIL;
                     }
                     
                     for(i = 0; i < q_config.linkInfo[index].cntOfUpQ; i++){
                        SOC_EA_CONTROL(unit).q_config.link_info[index].q_size[i] 
                            = q_config.linkInfo[index].sizeOfUpQ[i];
                     }
                }

                if(q_config.cntOfPort > 2){
                    return SOC_E_FAIL;
                }
                SOC_EA_CONTROL(unit).q_config.port_count 
                        = q_config.cntOfPort;
                for(index = 0; index < q_config.cntOfPort; index++){
                    SOC_EA_CONTROL(unit).q_config.port_info[index].q_count = 
                        q_config.portInfo[index].cntOfDnQ;
                    if(q_config.portInfo[index].cntOfDnQ > MAX_CNT_OF_UP_QUEUE){
                        return SOC_E_FAIL;
                    }
                    for(i = 0; i < q_config.portInfo[index].cntOfDnQ; i++){
                        SOC_EA_CONTROL(unit).q_config.port_info[index].q_size[i] =
                          q_config.portInfo[index].sizeOfDnQ[i];  
                    }
                }

                SOC_EA_CONTROL(unit).q_config.mcast_info.q_count 
                    = q_config.mcastInfo.cntOfDnQ;

                if(q_config.mcastInfo.cntOfDnQ > MAX_CNT_OF_UP_QUEUE){
                    return SOC_E_FAIL;
                }

                for(i = 0; i < q_config.mcastInfo.cntOfDnQ; i++){
                    SOC_EA_CONTROL(unit).q_config.mcast_info.q_size[i] 
                        = q_config.mcastInfo.sizeOfDnQ[i];
                }
            }
            rv = SOC_E_NONE;
            SOC_EA_CONTROL_FLAG_SET(unit, SOC_EA_CHIP_CONTROL_QUEUE_CONFIG);
            break;
            
        default:
            rv = SOC_E_PARAM;
            break;
    }
    
    return rv;
}

int 
_soc_ea_chip_control_flag_update(int unit, soc_ea_chip_control_t type, int dirty)
{
    int rv = SOC_E_NONE;

    if(!SOC_IS_EA(unit))return SOC_E_UNIT;
    if(NULL == SOC_EA_DB(unit))return SOC_E_INIT;
    if(socEaChipControlCount <= type)return SOC_E_PARAM;
    
    switch(type){
        case socEaChipControlLlidCount:
            if(dirty){
                SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_CONTROL_MLLID);
            }else{
                SOC_EA_PRIVATE_FLAG_CLEAR(unit, SOC_EA_CHIP_CONTROL_MLLID);
            } 
            break;
            
        case socEaChipControlQueueConfig:
            if(dirty){
                SOC_EA_PRIVATE_FLAG_SET(unit, SOC_EA_CHIP_CONTROL_QUEUE_CONFIG);
            }else{
                SOC_EA_PRIVATE_FLAG_CLEAR(unit, SOC_EA_CHIP_CONTROL_QUEUE_CONFIG);
            }
            break;
            
        default:
            rv = SOC_E_PARAM;
            break;
    }
    
    return rv;
}


/*
 * Function:
 *      soc_ea_private_db_sync
 * Purpose:
 *      EA Private database synchronization.
 * Parameters:
 *      unit - EA unit No.
 *      flag - 1 for force, 0 for normal.In force mode, the synchronization 
 *          function will sync the data regardless the flag of the items stored
 *          in the private data base.
 * Returns:
 *      SOC_E_NONE, SOC_E_XXXX
 */
int
soc_ea_private_db_sync(int unit, int flag)
{
    int db_type;
    int rv = SOC_E_NONE;

    if(!SOC_IS_EA(unit)){
        return SOC_E_UNIT;
    }
    
    if(soc_attached(unit)){
        db_type = socEaChipInfoLoadInfo;
        while(db_type < socEaChipInfoCount){
            rv = _soc_ea_chip_info_sync(unit, (soc_ea_chip_info_t)db_type, flag);
            if(rv != SOC_E_NONE){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Ea unit %d db %d sync failed.\n"),unit, 
                           db_type));
            }
            db_type++;
        }

        db_type = socEaChipControlLlidCount;
        /* coverity[mixed_enums] */
        while(db_type < socEaChipControlCount){
            rv = _soc_ea_chip_control_sync(unit, (soc_ea_chip_control_t)db_type, flag);
            if(rv != SOC_E_NONE){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Ea unit %d db %d sync failed.\n"),unit, 
                           db_type));
            }
            db_type++;
        }
    }
    return SOC_E_NONE;
}

void
soc_ea_private_db_dump(int unit)
{
    soc_control_t *soc;
    soc_info_t    *si;
    soc_ea_private_db_t *db;
    int i;
    int j;

    if (NULL == SOC_CONTROL(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Ea err code:%d"),SOC_E_UNIT));
        return;
    }

    if(!SOC_IS_EA(unit)){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Ea err code:%d"),SOC_E_UNIT));
        return ;
    }
    
    soc = SOC_CONTROL(unit);

    si = &soc->info;

    if(si->private == NULL){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Ea err code:%d"),SOC_E_INIT));
        return ;
    }

    db = (soc_ea_private_db_t *)si->private;

    LOG_CLI((BSL_META_U(unit,
                        "private info\n")));
    LOG_CLI((BSL_META_U(unit,
                        "info.flag = 0x%08x\n"),db->chip_info.flag));
    LOG_CLI((BSL_META_U(unit,
                        "loadinfo\t\n")));
    for(i = 0; i < socEaFileCount; i++){
        LOG_CLI((BSL_META_U(unit,
                            "load.%d.ver = 0x%04x\t\n"),i,
                 db->chip_info.load_info[i].legacy_ver));
        LOG_CLI((BSL_META_U(unit,
                            "load.%d.crc = 0x%08x\t\n"),i,
                 db->chip_info.load_info[i].crc));
    }

    LOG_CLI((BSL_META_U(unit,
                        "onu info\t\n")));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.firmware_ver = 0x%04x\t\n"),
             db->chip_info.onu_info.firmware_ver));
    LOG_CLI((BSL_META_U(unit,
                        "pon base mac::%02x:%02x:%02x:%02x:%02x:%02x\t\n"),
             db->chip_info.onu_info.pon_base_mac[0],
             db->chip_info.onu_info.pon_base_mac[1],
             db->chip_info.onu_info.pon_base_mac[2],
             db->chip_info.onu_info.pon_base_mac[3],
             db->chip_info.onu_info.pon_base_mac[4],
             db->chip_info.onu_info.pon_base_mac[5]));
    LOG_CLI((BSL_META_U(unit,
                        "uni base mac::%02x:%02x:%02x:%02x:%02x:%02x\t\n"),
             db->chip_info.onu_info.uni_base_mac[0],
             db->chip_info.onu_info.uni_base_mac[1],
             db->chip_info.onu_info.uni_base_mac[2],
             db->chip_info.onu_info.uni_base_mac[3],
             db->chip_info.onu_info.uni_base_mac[4],
             db->chip_info.onu_info.uni_base_mac[5]));
    
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.port_count = 0x%04x\t\n"),
             db->chip_info.onu_info.port_count));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.max_link_count = 0x%04x\t\n"),
             db->chip_info.onu_info.max_link_count));
    
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.up_queue_count = 0x%04x\t\n"),
             db->chip_info.onu_info.up_queue_count));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.max_queue_count_per_link = 0x%04x\t\n"),
             db->chip_info.onu_info.max_queue_count_per_link));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.up_queue_unit = 0x%04x\t\n"),
             db->chip_info.onu_info.up_queue_unit));

    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.dn_queue_count = 0x%04x\t\n"),
             db->chip_info.onu_info.dn_queue_count));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.max_queue_count_per_port = 0x%04x\t\n"),
             db->chip_info.onu_info.max_queue_count_per_port));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.dn_queue_unit = 0x%04x\t\n"),
             db->chip_info.onu_info.dn_queue_unit));

    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.up_queue_total_size = 0x%08x\t\n"),
             db->chip_info.onu_info.up_queue_total_size));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.max_queue_count_per_port = 0x%08x\t\n"),
             db->chip_info.onu_info.max_queue_count_per_port));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.jedec_id = 0x%04x\t\n"),
             db->chip_info.onu_info.jedec_id));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.chip_id = 0x%04x\t\n"),
             db->chip_info.onu_info.chip_id));
    LOG_CLI((BSL_META_U(unit,
                        "onuinfo.chip_ver = 0x%08x\t\n"),
             db->chip_info.onu_info.chip_ver));

    LOG_CLI((BSL_META_U(unit,
                        "chip_revision = 0x%08x\t\n"),db->chip_info.chip_revision));
    LOG_CLI((BSL_META_U(unit,
                        "extned id\t\n")));
    for(i = 0; i < sizeof(soc_ea_extend_id_t); i++){
        if(i != 0 && i%16 == 0){
            LOG_CLI((BSL_META_U(unit,
                                "\t\n%02x"),db->chip_info.extend_id[i]));
        }else{
            LOG_CLI((BSL_META_U(unit,
                                "%02x "),db->chip_info.extend_id[i]));
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "\t\noam ver = %x\t\n"),db->chip_info.oam_ver));
    LOG_CLI((BSL_META_U(unit,
                        "llid count = %d\t\n"), db->chip_info.llid_count));
    LOG_CLI((BSL_META_U(unit,
                        "port info\t\n")));
    for(i = 0; i < SDK_MAX_NUM_OF_PORT; i++){
        /* coverity[result_independent_of_operands] */
        if(!SOC_PORT_VALID(unit,i)){
            break;
        }
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.interface = %d\t\n"),i,db->chip_info.port[i].interface));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.autoneg = %d\t\n"),i,db->chip_info.port[i].autoneg));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.speed = %d\t\n"),i,db->chip_info.port[i].speed));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.duplex = %d\t\n"),i,db->chip_info.port[i].duplex));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.advert_local = %d\t\n"),i,
                 db->chip_info.port[i].advert_local));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.advert_remote = %d\t\n"),i,
                 db->chip_info.port[i].advert_remote));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_speed_half_duplex = %d\t\n"),i,
                 db->chip_info.port[i].ability_speed_half_duplex));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_speed_full_duplex = %d\t\n"),i,
                 db->chip_info.port[i].ability_speed_full_duplex));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_pause = %d\t\n"),i,
                 db->chip_info.port[i].ability_pause));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_medium = %d\t\n"),i,
                 db->chip_info.port[i].ability_medium));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_loopback = %d\t\n"),i,
                 db->chip_info.port[i].ability_loopback));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.ability_encap = %d\t\n"),i,
                 db->chip_info.port[i].ability_encap));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.mdix = %d\n"),i,db->chip_info.port[i].mdix));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.mdix_status = %d\t\n"),i,
                 db->chip_info.port[i].mdix_status));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.bcast_limit = %d\t\n"),i,
                 db->chip_info.port[i].bcast_limit));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.bcast_limit_enable = %d\t\n"),i,
                 db->chip_info.port[i].bcast_limit_enable));
        LOG_CLI((BSL_META_U(unit,
                            "port.%d.phy_master = %d\t\n"),i,
                 db->chip_info.port[i].phy_master)); 
    }

    LOG_CLI((BSL_META_U(unit,
                        "control info\t\n")));
    LOG_CLI((BSL_META_U(unit,
                        "control.flag = 0x%08x\t\n"),db->chip_control.flag));
    LOG_CLI((BSL_META_U(unit,
                        "control.mllid_num = %d\t\n"),db->chip_control.mllid_num));
    LOG_CLI((BSL_META_U(unit,
                        "qconfig info\t\n")));
    LOG_CLI((BSL_META_U(unit,
                        "qconfig.link_count = %d\t\n"),db->chip_control.q_config.link_count));
    for(i = 0; i < db->chip_control.q_config.link_count; i++){
        if(i >= MAX_CNT_OF_LINK)break;
        LOG_CLI((BSL_META_U(unit,
                            "link%d.qcount = %d\t\n"),i,
                 db->chip_control.q_config.link_info[i].q_count));
        for(j = 0; j < db->chip_control.q_config.link_info[i].q_count; j++){
            if(j >= MAX_CNT_OF_UP_QUEUE)break;
            LOG_CLI((BSL_META_U(unit,
                                "link%d.q%d.size = %d\t\n"),i,j,
                     db->chip_control.q_config.link_info[i].q_size[j])); 
        }
    }

    for(i = 0; i < db->chip_control.q_config.port_count; i++){
        if(i >= SDK_MAX_NUM_OF_PORT)break;
        LOG_CLI((BSL_META_U(unit,
                            "port%d.qcount = %d\t\n"),i,
                 db->chip_control.q_config.port_info[i].q_count));
        for(j = 0; j < db->chip_control.q_config.port_info[i].q_count; j++){
            if(j >= MAX_CNT_OF_UP_QUEUE)break;
            LOG_CLI((BSL_META_U(unit,
                                "port%d.q%d.size = %d\n"),i,j,
                     db->chip_control.q_config.port_info[i].q_size[j]));
        }
    }

    for(i = 0; i < db->chip_control.q_config.mcast_info.q_count; i++){
        if(i >= MAX_CNT_OF_UP_QUEUE)break;
        LOG_CLI((BSL_META_U(unit,
                            "mcast.q%d.szie = %d\n"),i,
                 db->chip_control.q_config.mcast_info.q_size[i]));
    }
}

/*
 * Function:
 *  soc_ea_info_config
 * Parameters:
 *  unit - eaSwitch unit number.
 *  soc  - soc_control_t associated with this unit
 * Purpose:
 *  Fill in soc_info structure for a newly attached unit.
 *  Generates bitmaps and various arrays based on block and
 *  ports that the hardware has enabled.
 */

static void
soc_ea_info_config(int unit, soc_control_t *soc)
{
    soc_info_t      *si;
    soc_pbmp_t      pbmp_valid;
    uint16      dev_id;
    uint8       rev_id;
    uint16      drv_dev_id;
    uint8       drv_rev_id;
    int         port, blk = 0, bindex = 0, pno = 0;
    char        *bname;
    int         blktype;
    int         disabled_port;

    si = &soc->info;
    sal_memset((void *)si, 0, sizeof(soc_info_t));

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &drv_dev_id, &drv_rev_id);

    if (CMDEV(unit).dev.info->dev_type & SOC_SPI_DEV_TYPE) {
        si->spi_device = TRUE;
    }

    si->driver_type = SOC_CHIP_TK371X_A0;
    si->driver_group = SOC_CHIP_TK371X;
    /*this field is no need for tk371x family*/
    si->num_cpu_cosq = 0;
    /*the tk371x_a0 support 11 ports*/
    si->port_addr_max = 11;
    /*only module*/
    si->modid_count = 1;

    si->modid_max = 1;
    SOC_PBMP_CLEAR(si->s_pbm);  /* 10/100/1000/2500 Mbps comboserdes */
    SOC_PBMP_CLEAR(si->gmii_pbm);

    /*
     * pbmp_valid is a bitmap of all ports that exist on the unit.
     */
    pbmp_valid = soc_property_get_pbmp(unit, spn_PBMP_VALID, 1);

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (drv_dev_id) {
    case TK371X_DEVICE_ID:
        si->chip_type = SOC_INFO_CHIP_TYPE_TK371X;
        break;
    default:
        si->chip = 0;
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "soc_info_config: driver device %04x unexpected\n"),
                  drv_dev_id));
        break;
    }

    si->ipic_port = -1;
    si->ipic_block = -1;
    si->exp_port = -1;
    si->exp_block = -1;
    si->cmic_port = -1;
    si->cmic_block = -1;
    si->spi_port = -1;
    si->spi_block = -1;
    si->fe.min = si->fe.max = -1;
    si->ge.min = si->ge.max = -1;
    si->xe.min = si->xe.max = -1;
    si->hg.min = si->hg.max = -1;
    si->llid.min = si->llid.max = -1;
    si->pon.min = si->pon.max = -1;
    si->ether.min = si->ether.max = -1;
    si->port.min = si->port.max = -1;
    si->all.min = si->all.max = -1;

    for (blk = 0; blk < SOC_EA_MAX_NUM_BLKS; blk++) {
        si->block_port[blk] = REG_PORT_ANY;
    }

    for (port = 0; ; port++) {
        disabled_port = FALSE;
        blk = SOC_PORT_INFO(unit, port).blk;
        bindex = SOC_PORT_INFO(unit, port).bindex;
        if (blk < 0 && bindex < 0) {            /* end of list */
            break;
        }
        if (blk < 0) {                  /* empty slot */
            disabled_port = TRUE;
            blktype = 0;
        } else {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
            if (!SOC_PBMP_MEMBER(pbmp_valid, port)) {   /* disabled port */
                disabled_port = TRUE;        
            }
        }

        if (disabled_port) {
            sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                 "?%d", port);
            si->port_offset[port] = port;
            continue;
        }

#define ADD_PORT(ptype, port) \
            si->ptype.port[si->ptype.num++] = port; \
            if (si->ptype.min < 0) { \
            si->ptype.min = port; \
            } \
            if (port > si->ptype.max) { \
            si->ptype.max = port; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.bitmap, port);

        bname = soc_block_ea_port_name_lookup_ext(blktype, unit);
        switch (blktype) {
        case SOC_EA_BLK_PON:
            pno = si->pon.num;
            ADD_PORT(pon, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_EA_BLK_GE:
            pno = si->ge.num;
            ADD_PORT(ge, port);
            ADD_PORT(ether, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_EA_BLK_FE:
            pno = si->fe.num;
            ADD_PORT(fe, port);
            ADD_PORT(ether, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_EA_BLK_LLID:
            pno = si->llid.num;
            ADD_PORT(llid, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        default:
            pno = 0;
            break;
        }
#undef  ADD_PORT

        sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                 "%s%d", bname, pno);
        si->port_type[port] = blktype;
        si->port_offset[port] = pno;
        si->block_valid[blk] += 1;
        if (si->block_port[blk] < 0) {
            si->block_port[blk] = port;
        }
        SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
    }
    si->port_num = port;

    /* some things need to be found in the block table */
    si->arl_block = -1;
    si->mmu_block = -1;
    si->mcu_block = -1;
    si->inter_block = -1;
    si->exter_block = -1;
    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
        blktype = SOC_BLOCK_INFO(unit, blk).type;
        si->has_block[blk] = blktype;
        sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                 "%s%d",
                 soc_block_ea_name_lookup_ext(blktype, unit),
                 SOC_BLOCK_INFO(unit, blk).number);
    }
    si->block_num = blk;
}

int
soc_ea_pre_attach(int unit)
{
    soc_control_t   *soc;
    soc_info_t      *si;
    char            prop[64], *s;
    int dev;
    int port;
#if !defined(KEYSTONE)
    int i;
#endif

    if(SOC_CONTROL(unit) == NULL){
        soc = SOC_CONTROL(unit) = sal_alloc(sizeof(soc_control_t), 
                                            "soc_control");
        if(NULL == soc){
            return SOC_E_MEMORY; 
        }else{
            sal_memset(soc, 0, sizeof(soc_control_t));   
        }
    }

    soc = SOC_CONTROL(unit);

    si = &soc->info;
    sal_memset((void *)si, 0, sizeof(soc_info_t));
    /* We don't know which TK chip is connected at current stage, assign a 
    dummy id */
    si->chip_type = SOC_INFO_CHIP_TYPE_TKDUMMY; 

    soc->chip_driver = &soc_driver_tk371x_a0;
    soc->soc_functions = NULL;

     

    for (dev = -1; dev < SOC_MAX_NUM_DEVICES; dev++) {
        for (port = 0; port < SOC_PBMP_PORT_MAX; port++) {
            sal_sprintf(prop, "ea_attach.port%d.%d", port,dev);
                if ((s = soc_property_get_str(unit, prop))) {
                    if (soc_tk_pre_attached == _shr_ctoi(s)) {
				        soc->attached_unit = dev;
                        soc->attached_port = port;
                        break;
                }
            }
        }
	}

    soc_tk_pre_attached++;

    /* TBD: decide which control interface is used */
#if defined(KEYSTONE)
    /* Attach GMII control interface drivers */
    soc->oam_ctrlops = soc_ea_oam_ctrl_attach(socEaOamCtrlIfEther);
    if (!soc->oam_ctrlops) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "EA unit %d attached control interface failed\n"),
                   unit));
        return SOC_E_INTERNAL;
    }
#else
    /* TBD: Defualt interface? */
    /* Search for all possible control interface types */
    for (i = 0; i < socEaOamCtrlIfCount; i++) {
        soc->oam_ctrlops = soc_ea_oam_ctrl_attach(i);
        if (soc->oam_ctrlops) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "EA unit %d attached control interface failed\n"), 
                       unit));
            break;
        }
    }
    if (!soc->oam_ctrlops) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "EA unit %d attached control interface failed\n"), 
                   unit));
        return SOC_E_INTERNAL;
    }
#endif

    soc_ea_info_config(unit, soc);

    soc_feature_init(unit);

    soc_ea_private_init(unit);

    _soc_ea_counter_attach(unit);

    return SOC_E_NONE;
}

int
soc_ea_detach(int unit)
{
    soc_control_t   *soc;
    soc_ea_oam_ctrl_if_t *ctrl_if;

    soc = SOC_CONTROL(unit);

    if (NULL == soc) {
        return SOC_E_NONE;
    }

    _soc_ea_counter_detach(unit);
    
    ctrl_if = soc->oam_ctrlops;
    ctrl_if->response_rtn_call_reg(unit, NULL);
    ctrl_if->deinit(unit);

    if (0 == (soc->soc_flags & SOC_F_ATTACHED)) {
        CMVEC(unit).init = 0;
        soc_tk_pre_attached = 0;
        return SOC_E_NONE;
    }

    SOC_FLAGS_CLR(soc, SOC_F_ATTACHED);

    sal_free(SOC_CONTROL(unit));
    SOC_CONTROL(unit) = NULL;

    soc_tk_pre_attached = 0;
    CMVEC(unit).init = 0;
    return SOC_E_NONE;
}

static void
_ea_device_probe_thread(void *param)
{
    int u = PTR_TO_INT(param);
    int cur_unit;
    soc_control_t   *soc;
    soc_ea_oam_ctrl_if_t *ctrl_if;
    uint8 *info_req_buf;
    uint32 info_req_len = 64;
    int buf_idx = 0;
    uint8 dst_mac[]={0x01,0x80,0xc2,0x00,0x00,0x02};
    uint8 src_mac[]={0x00,0x11,0x22,0x33,0x44,0x55};
    uint8 type[]={0x88,0x09};
    uint8 subtype[]={0x03};
    uint8 flags[]={0x00,0x50};
    uint8 code[]={0xfe};
    uint8 oui[]={0x00,0x0d,0xb6};
	int ea_probe_count = 0;

    info_req_buf = soc_cm_salloc(u, info_req_len, "ea probe");
    if(NULL == info_req_buf){
        return ;
    }
    
    sal_memset(info_req_buf, 0, info_req_len);

    buf_idx = 0;
    sal_memcpy(info_req_buf, dst_mac, sizeof(sal_mac_addr_t));
    buf_idx += sizeof(sal_mac_addr_t);
    sal_memcpy(info_req_buf+buf_idx, src_mac, sizeof(sal_mac_addr_t));
    buf_idx += sizeof(sal_mac_addr_t);
    sal_memcpy(info_req_buf+buf_idx, type, 2);
    buf_idx += 2;
    sal_memcpy(info_req_buf+buf_idx, subtype, 1);
    buf_idx += 1;
    sal_memcpy(info_req_buf+buf_idx, flags, 2);
    buf_idx += 2;
    sal_memcpy(info_req_buf+buf_idx, code, 1);
    buf_idx += 1;
    sal_memcpy(info_req_buf+buf_idx, oui, 3);

    while (ea_probe_thread_running) {
        /*
          * Check if the ONU device is up.
          * Start a thread and send info request packets every 5 seconds to all
          * known ONUs
          * If can not confirmed in one minutes, give up
          * If found any ONU devices, attach them.
          */
        for (cur_unit= 0; (cur_unit < soc_ndev); cur_unit++) { 
            if (SOC_IS_EA(SOC_NDEV_IDX2DEV(cur_unit))) {
                soc = SOC_CONTROL(SOC_NDEV_IDX2DEV(cur_unit));
        
                if (soc->soc_flags & SOC_F_ATTACHED) {
                    /* This unit is attached succefully */
                    continue;
                }
        
                ctrl_if = soc->oam_ctrlops;
        
                ctrl_if->request(SOC_NDEV_IDX2DEV(cur_unit), info_req_buf, info_req_len);
            }
        }
        ea_probe_count++;

        /* Probe time is up */
        if (ea_probe_count >= 3) {
            ea_probe_thread_running = FALSE;
        }

        sal_usleep(1000000);
    }

    if (info_req_buf) {
        soc_cm_sfree(u, info_req_buf);
    }

    for (cur_unit= 0; (cur_unit < soc_ndev); cur_unit++) { 
        if (SOC_IS_EA(SOC_NDEV_IDX2DEV(cur_unit))) {
            soc = SOC_CONTROL(SOC_NDEV_IDX2DEV(cur_unit));
    
            if (!(soc->soc_flags & SOC_F_ATTACHED)) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(u,
                                     "Unit %d attaching failed\n"), SOC_NDEV_IDX2DEV(cur_unit)));
                soc_ea_detach(SOC_NDEV_IDX2DEV(cur_unit));
            }
    
        }
    }

    sal_thread_exit(0);
}

static void
_oam_ctrl_temp_cbk_func(int unit, eth_dv_t *dv)
{
    char    *p;
    uint8   path_id = unit;
    
    p = &((char *) (INT_TO_PTR(dv->dv_dcb[0].dcb_vaddr)))[40];
    _TkDataProcessHandle(path_id, p, dv->dv_length);
    
#if defined(INCLUDE_KNET)
		dv->dv_dcb[0].desc_status = 0;
    dv->dv_dcb[0].len = 1600;
    dv->dv_dcb[0].dcb_paddr = soc_cm_l2p(0,(void *)dv->dv_dcb[0].dcb_vaddr);
#endif

    et_soc_rx_chain(unit, dv);
}

static void
_oam_ctrl_probe_cbk_func(int unit, eth_dv_t *dv)
{
    soc_control_t   *soc;
    soc_info_t      *si;
    soc_ea_oam_ctrl_if_t *ctrl_if;
    int test_unit;
    int attached_unit = 0;

#if defined(INCLUDE_KNET)
		dv->dv_dcb[0].desc_status = 0;
    dv->dv_dcb[0].len = 1600;
    dv->dv_dcb[0].dcb_paddr = soc_cm_l2p(0,(void *)dv->dv_dcb[0].dcb_vaddr);
#endif

		et_soc_rx_chain(unit, dv);
    
    soc = SOC_CONTROL(unit);

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        
        si = &soc->info;
        /* Now we know which TK chip connected, update the chip info */
        si->chip_type = SOC_INFO_CHIP_TYPE_TK371X;
    
        ctrl_if = soc->oam_ctrlops;

		/* Replace the control rx callback function by response_rtn_call_reg */
        ctrl_if->response_rtn_call_reg(unit, _oam_ctrl_temp_cbk_func);

        soc->soc_flags |= SOC_F_ATTACHED;
		
        LOG_CLI((BSL_META_U(unit,
                            "Attaching unit %d\n"),unit));
    }

    
    for (test_unit= 0; (test_unit < soc_ndev); test_unit++) {
        if (SOC_IS_EA(SOC_NDEV_IDX2DEV(test_unit))) {
            if (soc->soc_flags & SOC_F_ATTACHED) {
                /* This unit is attached succefully */
                attached_unit++;
            }
        }
    }

    if (attached_unit == soc_tk_ndev) {
        ea_probe_thread_running = FALSE;
    }

    return;
}

int
soc_ea_reset_init(int unit)
{
    soc_control_t   *soc;
    soc_ea_oam_ctrl_if_t *ctrl_if;
    int test_unit;
#if defined(BCM_ROBO_SUPPORT)	
    int rv;
    uint32 data;
    uint32 temp;
#endif	
    
    soc = SOC_CONTROL(unit);

    /* Count how many EA units are waiting for attachment */
    for (test_unit= 0; (test_unit < soc_ndev); test_unit++) {
        if (SOC_IS_EA(SOC_NDEV_IDX2DEV(test_unit))) {
            soc_tk_ndev++;
        }
    }
#if defined (BCM_ROBO_SUPPORT)    
    if (SOC_IS_TBX(soc->attached_unit)) {
        /* GE0 port force link up */   
        OVERRIDE_LOCK(soc->attached_unit);
        rv = REG_READ_STS_OVERRIDE_GPr(soc->attached_unit, 
                                        soc->attached_port, &data);
        if (rv != SOC_E_NONE) {
            OVERRIDE_UNLOCK(soc->attached_unit);
            return rv;
        }
        temp = 1;
        soc_STS_OVERRIDE_GPr_field_set(soc->attached_unit, &data, 
                                        LINK_STSf, &temp);
        soc_STS_OVERRIDE_GPr_field_set(soc->attached_unit, &data, 
                                        SW_OVERRIDEf, &temp);
        rv = REG_WRITE_STS_OVERRIDE_GPr(soc->attached_unit, 
                                        soc->attached_port, &data);
        if (rv != SOC_E_NONE) {
            OVERRIDE_UNLOCK(soc->attached_unit);
            return rv;
        }
        OVERRIDE_UNLOCK(soc->attached_unit);

        /* GE0 port forward status and tx/rx enable */   
        SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(soc->attached_unit, 
                                              soc->attached_port, &data));
        temp = 3; /* Forwarding State*/
        soc_G_PCTLr_field_set(soc->attached_unit, &data, G_STP_STATEf, &temp);
        temp = 0; /* Enable TX */
        soc_G_PCTLr_field_set(soc->attached_unit, &data, TX_DISf, &temp);
        temp = 0; /* Enable RX */
        soc_G_PCTLr_field_set(soc->attached_unit, &data, RX_DISf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr(soc->attached_unit, 
                                                soc->attached_port, &data));
    }

#endif
    ctrl_if = soc->oam_ctrlops;

    ctrl_if->response_rtn_call_reg(unit, _oam_ctrl_probe_cbk_func);
    ctrl_if->init(unit);
    
    return SOC_E_NONE;
}

int
soc_ea_do_init(int unit_count)
{
	int ea_unit_problint = 0;
	int i;
	sal_thread_t ea_probe_thread_id;
	char probe_thread_name[8];

	for(i = 0; i < unit_count; i++){
		if(SOC_IS_EA(i) && !soc_attached(i)){
			soc_ea_reset_init(i);
			ea_unit_problint = 1;
		}
	}

	if(ea_unit_problint){
		/* If there is any EA unit wait for probing */
		if (!ea_probe_thread_running) {
		/* Start a thread to polling EA units */
			ea_probe_thread_running = TRUE;
			ea_probe_thread_id = sal_thread_create("EAProbe",
										  SAL_THREAD_STKSZ,
										  200,
										  _ea_device_probe_thread, NULL);

			while (sal_thread_name(ea_probe_thread_id, probe_thread_name, 8) 
			       != NULL){
				sal_sleep(2);
			}
		}
	}

    for(i = 0; i < unit_count; i++){
        soc_ea_private_db_sync(i, SOC_EA_SYNC_FLAG_FORCE);    
    }
    
	return SOC_E_NONE;
}

int
soc_ea_event_generate(int unit,  soc_switch_event_t event, uint32 arg1,
                   uint32 arg2, uint32 arg3, soc_ea_event_userdata_t *data)
{
    soc_control_t       *soc;
    soc_event_cb_list_t  *curr;

    /* Input validation */
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    curr = soc->ev_cb_head;
    while (NULL != curr) {
        if((NULL != data) && (NULL != curr->userdata)){
            if(data->len < SOC_EA_EVENT_DATA_LEN){
                sal_memcpy(curr->userdata, data->data, data->len);
            }
        }
        
        curr->cb(unit, event, arg1, arg2, arg3, curr->userdata);
        curr = curr->next;
    }

    return (SOC_E_NONE);
}


